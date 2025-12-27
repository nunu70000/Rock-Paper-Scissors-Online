#include <pthread.h> //Required for mutex

//Assertion: player names will not exceed 64 characters. This is for memory safety. The limit is arbitrary; it can be increased freely.
#define PLAYERNAME_SIZE 64

//Definitions with the intent of increasing modularity of the code
#define MSGTYPE_SIZE 1
#define MSGDELIMITER_SIZE 1
#define MSGTERMINATOR_SIZE 2

//The max size of a client message is 68 chars. 64(name) + 2(P| at the start) + 2(|| at end of message).
//The max size of a server message is thus also 68 chars. 64(name) + 2(B| at the start) + 2(|| at the end of message)
//Assertion: There exists at most one argument in the client's message to the server.
#define MSG_MAXSIZE (MSGTYPE_SIZE + MSGDELIMITER_SIZE + PLAYERNAME_SIZE + MSGTERMINATOR_SIZE)

//It is possible to establish stricter limits on different message types (like the name string being 65 chars, choice string 9), but it isn't worth the hassle unless
//memory efficiency is of high importance.

struct client_info {
    int player_fd;
    char player_name[MSG_MAXSIZE];
    char msg[MSG_MAXSIZE];
};

struct lobby {
    struct client_info player_info[2];
};

struct rps_thread_info {
    struct lobby * lobby;
    int server_fd;
    pthread_mutex_t *queuemutex;
    pthread_cond_t *condvar;
};

int get_players(int server_fd, pthread_mutex_t * mutex, struct lobby * lobby);

void * rps_game_thread(void * struct_rps_thread_info);

void * openLobby(void * struct_lobby_info);
