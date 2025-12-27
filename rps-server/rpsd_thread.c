#include "rpsd_thread.h"
#include "rpsd.h"
#include <pthread.h>
#include <stdint.h>
#include <sys/poll.h>

//Assertion: player names will not exceed 64 characters. This is for memory safety. The limit is arbitrary; it can be increased freely.
//Returns -1 on fail, 0 on success.

int client_is_connected(int fd) {
    if (fd == -1) return 0;

    struct pollfd pfd = {
        .fd = fd,
        .events = POLLOUT,
        .revents = 0
    };

    // Non-blocking poll to check connection status
    return (poll(&pfd, 1, 0) == 1 &&
           (pfd.revents & POLLOUT) &&
           !(pfd.revents & (POLLHUP | POLLERR)));
}

ssize_t recv_msg(int client_fd, char msg[MSG_MAXSIZE]) {
    struct pollfd pollfd = {
        .fd = client_fd,
        .events = POLLIN,
        .revents = 0
    };

    if (poll(&pollfd, 1, -1) != 1 || !(pollfd.revents & POLLIN) || pollfd.revents & (POLLERR|POLLHUP)) {
        msg[0] = '\0';
        return -1;
    }
    ssize_t bytes_read = read(client_fd, msg, MSG_MAXSIZE);
    if (bytes_read < 2 || (msg[0] != 'P' && msg[0] != 'M' && msg[0] != 'C' && msg[0] != 'Q') || msg[1] != '|') {
        //Either we are in the middle of a message (we dont want it) or an error happened.
        msg[0] = '\0';
        return -1;
    }
    else { //Get a full message
        ssize_t read_result = 0;
        while ( (msg[bytes_read -2] != '|' || msg[bytes_read-1] != '|') && bytes_read < MSG_MAXSIZE) {
            pollfd.revents = 0;
            if (poll(&pollfd, 1, -1) != 1 || !(pollfd.revents & POLLIN) || (read_result = read(client_fd, msg+bytes_read, MSG_MAXSIZE-bytes_read)) == -1) {
                msg[0] = '\0';
                return -1;
            }
            bytes_read += read_result;
        }
    }
    return bytes_read;
}

int server_send_wait(int client_fd) {
    struct pollfd pfd;
    pfd.fd = client_fd;
    pfd.events = POLLOUT;
    pfd.revents = 0;
    if (poll(&pfd, 1, -1) < 1 || !(pfd.revents & POLLOUT) || (pfd.revents & (POLLHUP | POLLERR)) || send(client_fd, "W|1||", 5, MSG_NOSIGNAL) < (ssize_t)(5*sizeof(char))) { //Send Wait message to the client (only required when client joins)
        return -1;
    }
    return 0;
}

void * client_get_name(void * struct_client_info) { //This function can be ran using pthread
    struct client_info * info = struct_client_info;
    int client_fd = info->player_fd;
    char * name = info->player_name;
    char msg[MSG_MAXSIZE];
    ssize_t bytes_read = recv_msg(client_fd, msg);
    if (bytes_read < 5 || msg[0] != 'P') {
        name[0] = '\0';
        return (void *)(intptr_t)-1;
    }
    memcpy(name, msg+2, (bytes_read-4)*sizeof(char)); //bytes_read is at max 68.
    name[bytes_read-4] = '\0';
    if (player_list_ispresent(name) || player_count >= MAXPLAYERS) {
        send(client_fd, "R|L|Logged in||", 15*sizeof(char), 0);
        name[0] = '\0';
        return (void *)(intptr_t)-1;
    }
    if (server_send_wait(client_fd) == -1) {
        player_list_remove(name);
        name[0] = '\0';
        return (void *)(intptr_t)-1;
    }
    player_list_add(name);
    return (void *)(intptr_t)0;
}

//Returns -1 on fail, 0 on success.
int begin_match(struct lobby *lobby) {
    for (int i = 0; i < 2; i++) {
        char begin_msg[MSG_MAXSIZE]; //B|(2) + name(64) + ||(2)
        begin_msg[0] = 'B', begin_msg[1] = '|';
        char *opponent_name = lobby->player_info[1-i].player_name;
        size_t opponent_name_length = strlen(opponent_name);
        memcpy(&begin_msg[2], opponent_name, opponent_name_length*sizeof(char));
        begin_msg[2+opponent_name_length] = '|';
        begin_msg[3+opponent_name_length] = '|';
        //ssize_t cast to satisfy compiler; cast is safe as opponent_name_length can be at max 64.
        struct pollfd pollfd = {
            .fd = lobby->player_info[i].player_fd,
            .events = POLLOUT,
            .revents = 0
        };

        if (poll(&pollfd, 1, -1) < 1 || !(pollfd.revents & POLLOUT) || pollfd.revents & (POLLHUP|POLLERR) || send(lobby->player_info[i].player_fd, begin_msg, (4+opponent_name_length)*sizeof(char), 0) < (ssize_t)((4+opponent_name_length)*sizeof(char))) return -1;
    }
    return 0;
}

int client_get_choice(int client_fd, char choice[MSG_MAXSIZE]) {
    char msg[MSG_MAXSIZE];
    ssize_t bytes_read = recv_msg(client_fd, msg);
    if (bytes_read < 8 || msg[0] != 'M') {
        choice[0] = '\0';
        return 0;
    }
    memcpy(choice, msg+2, (bytes_read-4)*sizeof(char)); //bytes_read is at max 68.
    choice[bytes_read-4] = '\0';

    //Disallow choices other than ROCK, PAPER, or SCISSORS. They will become an empty string.
    if (strcmp(choice, "ROCK") != 0 && strcmp(choice, "PAPER") && strcmp(choice, "SCISSORS") != 0) {
        choice[0] = '\0';
    }
    return 0;
}

int client_get_rematch_status(int client_fd, char rematch_status[MSG_MAXSIZE]) {
    char msg[MSG_MAXSIZE];
    ssize_t bytes_read = recv_msg(client_fd, msg);
    rematch_status[0] = bytes_read != 3 || msg[0] != 'C' ? 'Q' : 'C';
    rematch_status[1] = '\0';
    return 0;
}

int client_getboth_message(struct lobby * lobby, int message_function (int, char[])) {
    //Gets a messaage from both clients using the given message_function.
    struct pollfd pfd[2] = {
        {lobby->player_info[0].player_fd, POLLIN, 0},
        {lobby->player_info[1].player_fd, POLLIN, 0}
    };
    for (int j = 0; j < 2; j++) {
        if (poll(pfd, 2, -1) < 1) return -1;
        //At least one fd is ready. Read from it/them. If only one is read, then we must poll again for the other. That explains the outer for loop.
        for (int i = 0; i < 2; i++) {
            if (!(pfd[i].revents & POLLIN)) continue;
            if (message_function(pfd[i].fd, lobby->player_info[i].msg) == -1) return -1;
            pfd[i].fd = ~pfd[i].fd; //tell poll to not poll this again. Caveat: assumes fd is never 0.
        }
        if (pfd[0].fd < 0 && pfd[1].fd < 0) break;
    }
    return 0;
}

int get_match_outcome(char *p1_attack, char *p2_attack) {
    /*
    Input strings are assumed to be acquired by client_get_choice().
    Returns:
        1 if the first attack wins against the second
        2 if the second attack beats the first
        0 if draw
        -1 if the first attack is invalid (P1-forfeit)
        -2 if the second attack is invalid (P2-forfeit)
        -3 if both attacks invalid (both forfeit)
    */
    char player1_attack = p1_attack[0], player2_attack = p2_attack[0];

    //Cases of forfeit
    if (player1_attack == '\0') return player2_attack == '\0' ? -3 : -1;
    else if (player2_attack == '\0') return -2;

    //Draw case
    if (player1_attack == player2_attack) return 0;

    if (player1_attack == 'R') return player2_attack == 'S' ? 1 : 2;
    if (player1_attack == 'P') return player2_attack == 'R' ? 1 : 2;
    if (player1_attack == 'S') return player2_attack == 'P' ? 1 : 2;

    return 0; //Satisfy compiler's wish for return in all control paths. Should not occur though
}

void conclude_match(struct lobby * lobby) {
    int match_outcome = get_match_outcome(lobby->player_info[0].msg, lobby->player_info[1].msg);
    char result[2];

    switch (match_outcome) {
        case 1:
            result[0] = 'W', result[1] = 'L';
            break;
        case 2:
            result[0] = 'L', result[1] = 'W';
            break;
        case -1:
            result[0] = 'L', result[1] = 'F';
            break;
        case -2:
            result[0] = 'F', result[1] = 'L';
            break;
        case -3:
            result[0] = 'L', result[1] = 'L';
            break;
        default:
            result[0] = 'D', result[1] = 'D';
    }
    for (int i = 0; i < 2; i++) {
        char *opponent_choice = lobby->player_info[1-i].msg;
        size_t choice_len = strlen(opponent_choice);
        char result_msg[MSG_MAXSIZE]; //The result message includes another argument so we must add 2 chars to hold it
        result_msg[0] = 'R';
        result_msg[1] = '|';
        result_msg[2] = result[i];
        result_msg[3] = '|';
        memcpy(result_msg+4, opponent_choice, choice_len*sizeof(char));
        result_msg[4+choice_len] = '|';
        result_msg[5+choice_len] = '|';
        send(lobby->player_info[i].player_fd, result_msg, (6+choice_len)*sizeof(char), 0);
    }
}


//TODO use poll to stop blocking from occurring
void * rps_game_thread(void * struct_rps_thread_info) { //RPS game. At this point, both players are connected. This function is meant to be run as a pthread.
    intptr_t return_status = 0;
    struct rps_thread_info * info = (struct rps_thread_info *) struct_rps_thread_info;

    int player1 = info->lobby->player_info[0].player_fd, player2 = info->lobby->player_info[1].player_fd;
    //Copy the lobby before letting main overwrite it
    struct lobby * lobbyCopy = malloc(sizeof(struct lobby));
    if (lobbyCopy == NULL) goto conclusion;
    memcpy(lobbyCopy, info->lobby, sizeof(struct lobby));
    pthread_cond_signal(info->condvar);
    info->lobby = lobbyCopy;

    char *p1_msg = info->lobby->player_info[0].msg;
    char *p2_msg = info->lobby->player_info[1].msg;

    //At this point, both players are connected and we have their names. So we begin the death battle.

    if (!client_is_connected(player1) || !client_is_connected(player2)) goto conclusion;
    while (1) { //Game loop
        if (begin_match(info->lobby) == -1) break;

        client_getboth_message(info->lobby, client_get_choice);

        //Both players have submitted their moves (or did not, in which case they forfeit). Determine who won.
        conclude_match(info->lobby);
        client_getboth_message(info->lobby, client_get_rematch_status);
        fprintf(stderr, "P1:%s\nP2:%s\n", p1_msg, p2_msg);
        for (int i = 0; i < 2; i++) {
           if (info->lobby->player_info[i].msg[0] == 'Q') {
               close(info->lobby->player_info[i].player_fd);
               info->lobby->player_info[i].player_fd = -1;
           }
        }
        if (info->lobby->player_info[0].player_fd == -1 || info->lobby->player_info[1].player_fd == -1) break; //destroy lobby
    }
    //Maybe implement something

    conclusion: //Free all memory and close all open sockets
    pthread_mutex_lock(info->queuemutex);
    player_list_remove(info->lobby->player_info[0].player_name);
    player_list_remove(info->lobby->player_info[1].player_name);
    player_count -= 2;
    pthread_mutex_unlock(info->queuemutex);
    free(lobbyCopy);
    free(info);
    if (player1 != -1) close(player1);
    if (player2 != -1) close(player2);
    return (void *)return_status;
}

void * openLobby(void * struct_lobby_info) {
    struct rps_thread_info * info = struct_lobby_info;
    int player1_connected = (info->lobby->player_info[0].player_fd == -1) ? 0 : 1;
    int player2_connected = (info->lobby->player_info[1].player_fd == -1) ? 0 : 1;
    struct pollfd pfd = {
        .fd = info->server_fd,
        .events = POLLIN,
        .revents = 0
    };
    waitingForPlayers:
    while (!player1_connected || !player2_connected) {

        pfd.revents = 0;
        if (poll(&pfd, 1, -1) < 1) continue;

        int which_client = player1_connected ? 1 : 0;
        info->lobby->player_info[which_client].player_fd = accept(info->server_fd, NULL, NULL);
        if (info->lobby->player_info[which_client].player_fd != -1) {
            if (which_client == 0) player1_connected = 1;
            else player2_connected = 1;
            client_get_name(&info->lobby->player_info[which_client]);
            player_count++;
        }
    }
    //Final check before starting game: Are both players connected?
    /* 3 strats
    1. Poll to see if sendable
    2. Send W|1|| again
    3. Send a single byte of begin message
    */
    for (int i = 0; i < 2; i++) {
        if (server_send_wait(info->lobby->player_info[i].player_fd) == -1 || info->lobby->player_info[i].player_name[0] == '\0' || !client_is_connected(info->lobby->player_info[i].player_fd)) {
            if (i == 0) {
                player1_connected = 0;
            }
            else player2_connected = 0;
            close(info->lobby->player_info[i].player_fd);
            player_list_remove(info->lobby->player_info[i].player_name);
            info->lobby->player_info[i].player_fd = -1;
            player_count--;
        }
    }
    if (!player1_connected || !player2_connected) goto waitingForPlayers;
    rps_game_thread(struct_lobby_info);
    return 0;
}
