#include "./utils.h"
#define WRITEFD 1
#define CANTDIR 8

void move(unsigned char * direction,sync_t *sems);
int getMove(game_t * game, int playerNum);

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
    unsigned char dir;
    int aux = 15;
    int seedAux = rand();
    srand(playerNum*seedAux);

    while(aux){// el master atiendo preguntando si hay algun moviemiento en orden, si no hay va a al siguiente
        //if (playerNum==4){
           // sleep(4);
        //}
        dir = rand()%8;
        //dir = getMove(game,playerNum);
        //if(dir == -1){
        //   game->players[playerNum].blocked = true;
         //   break;
       // }
        move(&dir,sems);
        aux--;
    }
    close(WRITEFD);
    //write(1, EOF, sizeof(unsigned char));
    return 0;
}

//struct fd_pair pipe();
//int pipe(int pipefd[2]);
//int pipe2(int pipefd[2], int flags);
void move(unsigned char * direction,sync_t *sems){
    sem_wait(&sems->C);
    // Escribir la solicitud en el pipe
    if (write(WRITEFD, direction, sizeof(unsigned char)) == -1) {
        perror("write");
        sem_post(&sems->C); 
        return;
    }
    sem_post(&sems->C);
    return;
}

/**
    * Devuelve la direccion de movimiento y -1 si no hay direccion disponible.
 */

int getMove(game_t * game, int playerNum){
    int max = 0;
    int dir = -1;
    unsigned short y = game->players[playerNum].posY;
    unsigned short x = game->players[playerNum].posX;
    
    int w = game->width;
    //Sacamos cual es el adyacente mayor
    for(int a1 = -1; a1 <= 1; a1++){
        for(int a2 = -1; a2 <= 1; a2++){
            if(game->board[y*(w+a1)+x+a2] > max){
                max = game->board[y*(w+a1)+x+a2];
                dir = (y*(w+a1)+x+a2) % 9;
                if(max == 9){
                    return dir;
                }
            }
        }
    }
    return dir;
}
