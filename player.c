#include "./utils.h"
#define WRITEFD 1

extern int dirs[][2];

void move(unsigned char * direction);
void getMove(game_t * game, int width, int height, int playerNum, sync_t * sems, unsigned char * direction, int totalMoves);
int isBlocked(game_t * game, int playerNum, sync_t * sems);

int main(int argc, char *argv[]){ 
    if(argc != 3){ 
        perror("argumentos incorrectos");
        exit(1);
    }
    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    game_t * game = (game_t*)createSHM(SHM_GAME_NAME, O_RDONLY |  O_CREAT, sizeof(game_t) + sizeof(int) * height * width, 0);
    sync_t *sems = (sync_t*)createSHM(SHM_SYNC_NAME, O_RDWR |  O_CREAT, sizeof(sync_t), 0);
    int totalMoves = 0;
    pid_t pid = getpid();
    int playerNum;
    for(int i = 0; i < game->cantPlayers; i++){
        if(game->players[i].pid == pid){
            playerNum = i;
            break;
        }
    }
    unsigned char direction=0;
    while(!isBlocked(game, playerNum, sems)){
        getMove(game, width, height, playerNum, sems, &direction, totalMoves);
        if(direction != 15){
            move(&direction);
            totalMoves++;
        }
    }
    close(WRITEFD);
    closeSHM(SHM_GAME_NAME, (void *)game, sizeof(game_t) + sizeof(int)* width * height, 0);
    closeSHM(SHM_SYNC_NAME, (void *)sems, sizeof(sync_t), 0);
    return 0;
}

void move(unsigned char * direction){
    if (write(WRITEFD, direction, sizeof(unsigned char)) == -1) {
        perror("write");
    }
}

 void getMove(game_t * game, int width, int height, int playerNum, sync_t * sems, unsigned char * direction, int totalMoves){
    int max = 0;
    *direction = 15; 
    mySemWait(&sems->wantToModifyMutex);
    mySemPost(&sems->wantToModifyMutex);
    mySemWait(&sems->playersReadingMutex);
    if(++sems->playersReading == 1){
        mySemWait(&sems->gameStatusMutex);
    }
    mySemPost(&sems->playersReadingMutex);
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
    mySemWait(&sems->playersReadingMutex);
    if(--sems->playersReading == 0){
        mySemPost(&sems->gameStatusMutex);
    }
    mySemPost(&sems->playersReadingMutex);
}

int isBlocked(game_t * game, int playerNum, sync_t * sems){
    int ret = 0;
    mySemWait(&sems->wantToModifyMutex);
    mySemPost(&sems->wantToModifyMutex);

    mySemWait(&sems->playersReadingMutex);
    if(++sems->playersReading == 1){
        mySemWait(&sems->gameStatusMutex);
    }
    mySemPost(&sems->playersReadingMutex);
    if(game->players[playerNum].blocked || game->finished){
        ret = 1;
    }
    mySemWait(&sems->playersReadingMutex);
    if(--sems->playersReading == 0){
        mySemPost(&sems->gameStatusMutex);
    }
    mySemPost(&sems->playersReadingMutex);
    return ret;
}