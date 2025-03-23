#include "./utils.h"
#define WRITEFD 1

void move(unsigned char * direction,sync_t *sems);

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
    srand(playerNum*234);

    while(aux){// el master atiendo preguntando si hay algun moviemiento en orden, si no hay va a al siguiente
        if (playerNum==4){
            sleep(4);
        }
        dir = rand()%8;
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

