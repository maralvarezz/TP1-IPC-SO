#include "./utils.h"

int move(unsigned char direction,int pipefd[],sync_t *sems);

int main(int argc, char *argv[]){
    printf("soy el pj nashe\n");
    if(argc != 3){ // siempre hay almenos un argumento que es el llamado al programa ./programa siempre es el primer argumento
        perror("argumentos incorrectos");
        exit(1);
    }
    int w = atoi(argv[1]);
    int h = atoi(argv[2]);

    int pipefd[2];
    game_t* game = (game_t*)createSHM("/game_state",O_RDONLY |  O_CREAT, sizeof(game_t), 0);    
    sync_t *sems = (sync_t*)createSHM("/game_sync",O_RDWR |  O_CREAT, sizeof(sync_t), 0);    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    close(pipefd[0]); // Cierra el extremo de lectura del pipe
    dup2(pipefd[1], 1); // Redirige stdout al pipe
    close(pipefd[1]); // Ya no se necesita este fd
    pid_t pid = getpid();
    int found = 0;
    int playerNum;
    for(int i= 0; i<game->cantPlayers && !found;i++){
        if(game->players[i].pid==pid){
            found++;
            playerNum=i;
        }
    }
    printf("soy el jugador %d",playerNum);
    move(1,pipefd, sems);
    
    return 0;

}

//struct fd_pair pipe();
//int pipe(int pipefd[2]);
//int pipe2(int pipefd[2], int flags);
//move() returns 0 if the move is completed, and returns 1 if the move is invalid
int move(unsigned char direction,int pipefd[],sync_t *sems){
    sem_wait(&sems->C);
    // Escribir la solicitud en el pipe
    printf("estoy re loco\n");
    if (write(pipefd[0], &direction, sizeof(direction)) == -1) {
        perror("write");
        sem_post(&sems->C); // Liberar el semáforo si hay error
        return 1;
    }
    sem_post(&sems->D);
    sem_post(&sems->C);

    return 0;
}

