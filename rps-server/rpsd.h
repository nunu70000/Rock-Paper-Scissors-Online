#define PLAYERNAME_SIZE 64
#define MAXPLAYERS 2048 //Arbitrary, can be increased freely
extern int player_count;
extern char player_list[MAXPLAYERS][PLAYERNAME_SIZE+1];
int player_list_add(char *player_name);
int player_list_remove(char *player_name);
int player_list_ispresent(char *player_name);
