#include "strategy.h"

extern int dirs[][2];

void getMove(game_t * game, int width, int height, int playerNum, unsigned char * direction, int totalMoves){
    int max = 0;
    *direction = 15; 
    unsigned short y = game->players[playerNum].posY;
    unsigned short x = game->players[playerNum].posX;
    if(totalMoves == game->players[playerNum].validMoves + game->players[playerNum].invalidMoves){
        int boardPosition;
        for(int i = 0; i < 8; i++){
            if(y + dirs[i][1] >= 0 && y + dirs[i][1] < height && dirs[i][0] + x >=0 && dirs[i][0] + x < width ){
                if((boardPosition = game->board[width*(y+dirs[i][1])+dirs[i][0] + x] ) > max){ 
                    max = boardPosition;
                    *direction = i;
                    if(max == 9){
                        break;
                    }
                }
            }
        }
    }
}