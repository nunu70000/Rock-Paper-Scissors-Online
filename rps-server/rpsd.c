#include "rpsd_game_thread.h"
#include "rpsd.h"
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>

#define BACKLOG 1024
#define PLAYERNAME_SIZE 64

int player_count;
char player_list[MAXPLAYERS][PLAYERNAME_SIZE+1];

int player_list_add(char *player_name) {
    if (player_name[0] == '\0') return -1;
    for (int i = 0; i < MAXPLAYERS; i++) if (player_list[i][0] == '\0') {
        size_t cpy_size = strlen(player_name) < 64 ? strlen(player_name) : 64;
        memcpy(player_list[i], player_name, cpy_size*sizeof(char));
        player_list[i][cpy_size] = '\0';
        player_count++;
        return 0;
    }
    return -1;
}
int player_list_remove(char *player_name) {
    if (player_name[0] == '\0') return -1;
    for (int i = 0; i < MAXPLAYERS; i++) if (strcmp(player_name, player_list[i]) == 0) {
        player_list[i][0] = '\0';
        player_count--;
        return 0;
    }
    return -1;
}
int player_list_ispresent(char *player_name) {
    if (player_name[0] == '\0') return 0;
    for (int i = 0; i < MAXPLAYERS; i++) if (strcmp(player_name, player_list[i]) == 0) {
        return 1;
    }
    return 0;
}

void server_loop_thread(int server_fd, struct lobby * lobby) {
    struct lobby global_lobby = *lobby;
    pthread_mutex_t queue_mutex;
    pthread_cond_t condvar;
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_cond_init(&condvar, NULL);
    pthread_t thread = -1;
    struct pollfd pfd = {
        .fd = server_fd,
        .events = POLLIN,
        .revents = 0
    };
    while (1) {
        //Reset variables
        pfd.revents = 0;
        for (int i = 0; i < 2; i++) {
            global_lobby.player_info[i].player_fd = -1;
            global_lobby.player_info[i].player_name[0] = '\0';
            global_lobby.player_info[i].msg[0] = '\0';
        }

        if (poll(&pfd, 1, -1) == -1 || !(pfd.revents & POLLIN)) continue;

        //open the new lobby
        struct rps_thread_info * info = malloc(sizeof(struct rps_thread_info));
        if (info == NULL) {
            perror("Failed to allocate memory while connecting");
            continue;
        }
        info->condvar = &condvar; info->lobby = &global_lobby; info->queuemutex = &queue_mutex; info->server_fd = server_fd; //probably something else for playerlist
        if (pthread_create(&thread, NULL, openLobby, info) != 0) {
            perror("Failed to create server thread");
            free(info);
            continue;
        }
        pthread_detach(thread);
        pthread_cond_wait(&condvar, &queue_mutex); //Wait until the lobby is full and done executing
    }
}

int is_valid_port(char *str) {
    /*
    Verifies the input string meets the following conditions:
        Each character is a number
        There are no more than 5 characters
        The number is not greater than 65535
    */
    size_t len = strlen(str);
    if (len > 5) return 0;
    for (size_t i = 0; i < len; i++) {
        if (str[i] < '0'|| str[i] > '9') return 0;
    }
    int port_num = atoi(str);
    if (port_num < 1 || port_num > 65535) return -1;
    return port_num;
}

/*
 * main - Main routine for the proxy program
 */
int main(int argc, char **argv) {
    /* Check arguments */
    int port = -1;

    struct lobby global_lobby = {
        (struct client_info){-1, {'\0'}, {'\0'}},
        (struct client_info){-1, {'\0'}, {'\0'}}
    };

    memset(player_list, '\0', sizeof(player_list));

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if ( (port = is_valid_port(argv[1])) == -1 ) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Failed to create socket, exitting...");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr = { //Proxy server address
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = {INADDR_ANY}
    };

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        close(server_fd);
        perror("Failed to bind to socket, exiting...");
        return EXIT_FAILURE;
    }

    if (listen(server_fd, BACKLOG) == -1) {
        close(server_fd);
        perror("Failed to listen on socket, exiting...");
        return EXIT_FAILURE;
    }

    server_loop_thread(server_fd, &global_lobby);

    return EXIT_SUCCESS;
}
