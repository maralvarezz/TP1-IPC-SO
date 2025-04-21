#include "./masterLib.h"

void setGame(game_t * game, unsigned int cantPlayers, unsigned int  width, unsigned int height){
    game->width = width;
    game->height = height;
    game->cantPlayers = cantPlayers;
    game->finished = false;
}

void finishGame(game_t * game, sync_t * sems){
    mySemWait(&sems->wantToModifyMutex);
    mySemWait(&sems->gameStatusMutex);
    game->finished = true;
    mySemPost(&sems->wantToModifyMutex);
    mySemPost(&sems->gameStatusMutex);
}

void getBoard(game_t * game, unsigned int seed){
    srand(seed);
    for(int i = 0; i < (game->height * (game->width)); i++){
        game->board[i] = (rand() % 9) + 1;
    }
}

void setPlayersPosition(game_t * game){
    unsigned short a = (game->height / 2) * POSITIONING;
    unsigned short b = (game->width / 2) * POSITIONING;  

    if(game->cantPlayers == 1){
        game->players[0].posX = game->width / 2; 
        game->players[0].posY = game->height / 2;
        game->board[game->players[0].posY * game->width + game->players[0].posX] = 0;
    } 
    else {
        for(int i = 0; i < game->cantPlayers; i++){
            double theta = (2.0 * PI * i) / game->cantPlayers; 

            game->players[i].posX = (unsigned short)((game->width / 2) + b * cos(theta));
            game->players[i].posY = (unsigned short)((game->height / 2) + a * sin(theta));
            
            if (game->players[i].posX >= 0 && game->players[i].posX < game->width && game->players[i].posY >= 0 && game->players[i].posY < game->height) {
                game->board[game->players[i].posY * game->width + game->players[i].posX] = -i; 
            }
        }
    }
}

void initializePlayers(game_t * game, int pipefds[][2], int cantPlayers){
    for(int i = 0; i < cantPlayers; i++){
        char playerName[] = {'P', 'l', 'a', 'y', 'e', 'r', ' ', i + '0', '\0'};
        strcpy(game->players[i].playerName, playerName);
        game->players[i].score = 0;
        game->players[i].validMoves = 0;
        game->players[i].invalidMoves = 0;
        game->players[i].blocked = false;
        if(pipe(pipefds[i]) == -1){
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }
    setPlayersPosition(game);
}

int makeMove(game_t * game, sync_t * sems, fd_set * readFDS, fd_set * masterFDS, int pipefds[][2], int cantPlayers, char * finished, time_t * lastValidMoveTime, int  timeout ){ 
    static int playerTurn = 0;
    unsigned char dir;
    int toRet = 1;
    int timeoutFlag = 0;
    if(time(NULL) - *lastValidMoveTime < timeout){
        for(int i = 0; i < cantPlayers; i++){
            int current = (playerTurn + i) % cantPlayers;
            if(!game->players[current].blocked && FD_ISSET(pipefds[current][0], readFDS)){
                int bytesRead = read(pipefds[current][0], &dir, sizeof(unsigned char));
                if (bytesRead < 0) {
                    perror("read");
                    exit(EXIT_FAILURE);
                }else if(bytesRead > 0){
                   if(move(game, sems, current, cantPlayers, dir)==1){
                        *lastValidMoveTime = time(NULL);
                    }
                    playerTurn = (current + 1) % cantPlayers;
                    break;
                }else{
                    FD_CLR(pipefds[current][0], masterFDS);
                    game->players[current].blocked = true;
                    toRet = 0;
                }
            }
        }
    }
    else{
        timeoutFlag++;
    }
    if(isGameEnded(game, cantPlayers) || timeoutFlag){
        game->finished = true;
        *finished = 1;
    }
    return toRet;
}

int isGameEnded(game_t * game, int cantPlayers){
    int ret = 1;
    for(int i = 0; i < cantPlayers && ret ; i++){
        if(!game->players[i].blocked){
            ret = 0;
        }
    }
    return ret;
}



int move(game_t * game, sync_t * sems, int playerNum, int cantPlayers, unsigned char dirIdx){  
    int toRet = 0;
    mySemWait(&sems->wantToModifyMutex);
    mySemWait(&sems->gameStatusMutex);
    int newPosValue;
    unsigned short width = game->width;
    player_t * playerToMove= &game->players[playerNum];
    int idx = width * (playerToMove->posY+dirs[dirIdx][1]) + dirs[dirIdx][0] + playerToMove->posX;
    if ((idx >= 0 && idx < (width * game->height)) && ((newPosValue=game->board[idx]) > 0)){
        playerToMove->score += newPosValue;
        playerToMove->posX += dirs[dirIdx][0];
        playerToMove->posY += dirs[dirIdx][1];
        game->board[width * (playerToMove->posY) + playerToMove->posX] = -playerNum; 
        playerToMove->validMoves++;
        toRet = 1;
    }else{
        playerToMove->invalidMoves++;
    }
    checkAndBlockPlayer(game, playerNum, 1);
    mySemPost(&sems->wantToModifyMutex);
    mySemPost(&sems->gameStatusMutex);
    return toRet;
}

void createSems(sync_t * sems){
    safeSem_init(&sems->haveToPrint, SHARED, 0);
    safeSem_init(&sems->finishedPrinting, SHARED, 0);
    safeSem_init(&sems->wantToModifyMutex, SHARED, 1);
    safeSem_init(&sems->gameStatusMutex, SHARED, 1);
    safeSem_init(&sems->playersReadingMutex, SHARED, 1);
    sems->playersReading = 0;
}

void checkAndBlockPlayer(game_t * game, int playerNum, int firstTime){ 
    int cantPosOcuppied = 0;
    int boardValue;
    int pos;
    int x = game->players[playerNum].posX;
    int y = game->players[playerNum].posY;
    for(int i = 0; i < 8; i++){
        pos = game->width * (y+dirs[i][1]) + dirs[i][0] + x;
        int newX = x + dirs[i][0];
        int newY = y + dirs[i][1];
        if(newY >= 0 && newY < game->height && newX >= 0 && newX < game->width){
            pos = game->width * newY + newX;
            boardValue = game->board[pos];
            if(boardValue <= 0){
                cantPosOcuppied++;
                if(firstTime && boardValue < 0 && !game->players[-boardValue].blocked && 
                game->players[-boardValue].posY == newY && game->players[-boardValue].posX == newX){
                    checkAndBlockPlayer(game, -boardValue, 0);
                }
            }
        } else {
            cantPosOcuppied++;
        }
    }
    if(cantPosOcuppied == 8){
        game->players[playerNum].blocked = true;
    }
}

void closeAllNotNeededFD(int pipefds[][2], int cantPlayers, int playerNum){
    for(int i = 0; i < cantPlayers; i++){
        if(i != playerNum){
            safeClose(pipefds[i][0]);
            safeClose(pipefds[i][1]);
        }
    }
}

void safeClose(int fd){
    if(close(fd) == -1){
        perror("close");
        exit(EXIT_FAILURE);
    }
}

void closeSems(sync_t * sems){
    mySemDestroy(&sems->haveToPrint);
    mySemDestroy(&sems->finishedPrinting);
    mySemDestroy(&sems->wantToModifyMutex);
    mySemDestroy(&sems->gameStatusMutex);
    mySemDestroy(&sems->playersReadingMutex);
}