#include "strategy.h"

extern int dirs[][2];

void getMove(game_t * game, int width, int height, int playerNum, unsigned char * direction, int totalMoves){
    *direction = playerNum % 8;
    usleep(10000);
}