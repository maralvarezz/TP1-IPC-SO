#include "./utils.h"
#define WRITEFD 1
#define CANTDIR 8

void move(unsigned char * direction,sync_t *sems);
void getMove(game_t * game,int w,int h, int playerNum,sync_t *sems, unsigned char * direc);


int main(int argc, char *argv[]){ // reciben w y h;
    if(argc != 3){ // siempre hay almenos un argumento que es el llamado al programa ./programa siempre es el primer argumento
        perror("argumentos incorrectos");
        exit(1);
    }
    int w = atoi(argv[1]);
    int h = atoi(argv[2]);

    int pipefd[2];
    game_t* game = (game_t*)createSHM("/game_state",O_RDONLY |  O_CREAT, sizeof(game_t), 0);
    sync_t *sems = (sync_t*)createSHM("/game_sync",O_RDWR |  O_CREAT, sizeof(sync_t), 0);
    //if (pipe(pipefd) == -1) {
        //perror("pipe");
        //exit(EXIT_FAILURE);
    //} 
    //close(pipefd[0]); // Cierra el extremo de lectura del pipe
    //dup2(pipefd[1], 1); // Redirige stdout al pipe
    //close(pipefd[1]); // Ya no se necesita este fd
    pid_t pid = getpid();
    int found = 0;
    int playerNum;
    for(int i= 0; i<game->cantPlayers && !found;i++){ //busca el jugador en el arreglo de jugadores
        if(game->players[i].pid==pid){
            found++;
            playerNum=i;
        }
    }
    //move(1,pipefd, sems);

    
    // Escribir la solicitud en el pipe
    //int seedAux = rand();
    //srand(playerNum*seedAux);
    unsigned char dir;
    while(1){// el master atiendo preguntando si hay algun moviemiento en orden, si no hay va a al siguiente
        //if (playerNum==4){
           // sleep(4);
        //}
        getMove(game,w,h,playerNum, sems, &dir);
        if(dir == 15){
        //   game->players[playerNum].blocked = true;
            break;
        } 
        
        move(&dir,sems);
        usleep(300000*game->cantPlayers);
    }
    close(WRITEFD);
    //write(1, EOF, sizeof(unsigned char));
    return 0;
}

//struct fd_pair pipe();
//int pipe(int pipefd[2]);
//int pipe2(int pipefd[2], int flags);
void move(unsigned char * direction,sync_t *sems){
    //sem_wait(&sems->C);
    // Escribir la solicitud en el pipe
    if (write(WRITEFD, direction, sizeof(unsigned char)) == -1) {
        perror("write");
       //sem_post(&sems->C);
        return;
    }
    //sem_post(&sems->C);
    return;
}

/**
    * Devuelve la direccion de movimiento y 15 si no hay direccion disponible.
 */

 void getMove(game_t * game,int w,int h, int playerNum,sync_t *sems,unsigned char * direct){
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
    sem_wait(&sems->E);
    if(++sems->playersReading==1){
        sem_wait(&sems->D);
    }
    sem_post(&sems->E);
    
    unsigned short y = game->players[playerNum].posY;
    unsigned short x = game->players[playerNum].posX;
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
    
    sem_wait(&sems->E);
    if(--sems->playersReading==0){
        sem_post(&sems->D);
    }
    sem_post(&sems->E);

}
