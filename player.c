#include "./utils.h"
#define WRITEFD 1
#define CANTDIR 8

void move(unsigned char * direction);
void getMove(game_t * game,int w,int h, int playerNum,sync_t *sems,unsigned char * direct, int  totalMoves);
int isBlocked(game_t * game, int playerNum, sync_t *sems);


int main(int argc, char *argv[]){ // reciben w y h;
    if(argc != 3){ 
        perror("argumentos incorrectos");
        exit(1);
    }
    int w = atoi(argv[1]);
    int h = atoi(argv[2]);

    game_t * game = (game_t*)createSHM("/game_state",O_RDONLY |  O_CREAT, sizeof(game_t) + sizeof(int)*h*w, 0);
    sync_t *sems = (sync_t*)createSHM("/game_sync",O_RDWR |  O_CREAT, sizeof(sync_t), 0);
    int totalMoves = 0;
    
    pid_t pid = getpid();
    int found = 0;
    int playerNum;
    for(int i= 0; i<game->cantPlayers && !found;i++){ //busca el jugador en el arreglo de jugadores
        if(game->players[i].pid==pid){
            found++;
            playerNum=i;
        }
    }
    unsigned char dir;
    while(!isBlocked(game,playerNum,sems)){
        getMove(game,w,h,playerNum, sems, &dir,totalMoves);
        if(dir != 15){
            move(&dir);
            totalMoves++;
        }
    }
    close(WRITEFD);
    return 0;
}

void move(unsigned char * direction){
    // Escribir la solicitud en el pipe
    if (write(WRITEFD, direction, sizeof(unsigned char)) == -1) {
        perror("write");
    }
    return;
}

/**
    * Deja en direct la direccion de movimiento, 14 si no hay direccion disponible, y 15 si el juego termino
 */

 void getMove(game_t * game,int w,int h, int playerNum,sync_t *sems,unsigned char * direct, int  totalMoves){
    int dirs[][2]= {{0,-1},
                    {1,-1},
                    {1,0},
                    {1,1},
                    {0,1},
                    {-1,1},
                    {-1,0},
                    {-1,-1}};
    int max = 0;
    *direct = 15; 
    sem_wait(&sems->C);
    sem_wait(&sems->E);
    if(++sems->playersReading==1){
        sem_wait(&sems->D);
    }
    sem_post(&sems->E);
    sem_post(&sems->C);
    
    unsigned short y = game->players[playerNum].posY;
    unsigned short x = game->players[playerNum].posX;
    if(totalMoves == game->players[playerNum].validMoves+game->players[playerNum].invalidMoves){
    //Sacamos cual es el adyacente mayor
        int aux;
        for(int i = 0; i < 8; i++){
            if(y+dirs[i][1]>=0 && y+dirs[i][1]<h && dirs[i][0]+x >=0 && dirs[i][0]+x < w ){
                if((aux = game->board[w*(y+dirs[i][1])+dirs[i][0]+x] ) > max){ 
                    max = aux;
                    *direct = i;
                    if(max == 9){
                        break;
                    }
                }
         }
        }
    }
    
    sem_wait(&sems->E);
    if(--sems->playersReading==0){
        sem_post(&sems->D);
    }
    sem_post(&sems->E);
}


int isBlocked(game_t * game, int playerNum, sync_t *sems){
    int ret = 0;
    sem_wait(&sems->C);
    sem_wait(&sems->E);
    if(++sems->playersReading==1){
        sem_wait(&sems->D);
    }
    sem_post(&sems->E);
    sem_post(&sems->C);
    if(game->players[playerNum].blocked){
        ret = 1;
    }
    sem_wait(&sems->E);
    if(--sems->playersReading==0){
        sem_post(&sems->D);
    }
    sem_post(&sems->E);
    return ret;
}