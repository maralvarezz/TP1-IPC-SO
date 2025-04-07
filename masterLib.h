#ifndef MASTERLIB_H
#define MASTERLIB_H
#include "./utils.h"

#define MAXDIGITS 11
#define SHARED 1
#define POSITIONING 0.8
#define PI 3.14

void setGame(game_t * game,unsigned int cantPlayers, unsigned int width, unsigned int height);
void getBoard(game_t * game, unsigned int seed);
void setPlayersPosition(game_t * game);
void initializePlayers(game_t * game, int pipefds[][2], int cantPlayers);
void createSems(sync_t * sems);
void closeSems(sync_t * sems);
void checkAndBlockPlayer(game_t * game, int playerNum, int firstTime);
int makeMove(game_t * game, sync_t * sems, fd_set * readFDS, fd_set * masterFDS, int pipefds[][2], int cantPlayers, char * finished);
void move(game_t * game, sync_t * sems, int playerNum, int cantJug, unsigned char dirIdx);
int isGameEnded(game_t * game, int cantPlayers);
void finishGame(game_t * game, sync_t * sems);
void safeSem_init(sem_t* sem, int shared, int value);
void safeClose(int fd);
void closeAllNotNeededFD(int pipefds[][2], int cantJug, int playerNum);

extern int dirs[][2];

#endif 