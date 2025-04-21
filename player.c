#include "./strategy.h"
#define WRITEFD 1


void move(unsigned char * direction);
int isBlocked(game_t * game, int playerNum);

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
    int isBlockedFlag=0;
    while(!isBlockedFlag){
        mySemWait(&sems->wantToModifyMutex);
        mySemPost(&sems->wantToModifyMutex);
        mySemWait(&sems->playersReadingMutex);
        if(++sems->playersReading == 1){
            mySemWait(&sems->gameStatusMutex);
        }
        mySemPost(&sems->playersReadingMutex);
        if(!(isBlockedFlag=isBlocked(game,playerNum))){
            getMove(game, width, height, playerNum, &direction, totalMoves);
            if(direction != 15){
                move(&direction);
                totalMoves++;
            }
        }
        mySemWait(&sems->playersReadingMutex);
        if(--sems->playersReading == 0){
            mySemPost(&sems->gameStatusMutex);
        }
        mySemPost(&sems->playersReadingMutex);
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

int isBlocked(game_t * game, int playerNum){
    return game->players[playerNum].blocked || game->finished;
}