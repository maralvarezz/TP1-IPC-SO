#include "./utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define PLAYER1     "\x1b[31m" //Rojo
#define PLAYER2   "\x1b[38;2;0;100;0m" //Verde oscuro
#define PLAYER3  "\x1b[38;2;255;255;102m" //Amarillo patito
#define PLAYER4    "\x1b[34m" //Azul
#define PLAYER5 "\x1b[35m" //Magenta
#define PLAYER6    "\x1b[36m" //Cyan
#define PLAYER7 "\x1b[38;2;255;0;255m" //Fucsia
#define PLAYER8     "\x1b[38;2;255;165;0m"  // Naranja
#define PLAYER9    "\x1b[38;2;144;238;144m"  // Verde claro
#define RESET   "\x1b[0m"


int main(int argc, char *argv[]){
    if(argc != 3){ // siempre hay al menos un argumento que es el llamado al programa ./programa siempre es el primer argumento
        perror("argumentos incorrectos");
        exit(1);
    }
    char *colors[9] = {PLAYER1, PLAYER2, PLAYER3, PLAYER4, PLAYER5, PLAYER6, PLAYER7, PLAYER8, PLAYER9};
    int w = atoi(argv[1]);
    int h = atoi(argv[2]);
    printf("in view\n");
    // pruebas ---------------------
    //inicio: 
    game_t* game = (game_t*)createSHM("/game_state",O_RDONLY |  O_CREAT, sizeof(game_t), 0);    
    sync_t *sems = (sync_t*)createSHM("/game_sync",O_RDWR |  O_CREAT, sizeof(sync_t), 0);    
    while(1){
        sem_wait(&sems->haveToPrint);
        for(int i=0;i<w*h;i++){
            if(i%w == 0){
                printf("\n");
            }
            int aux = game->board[i]; 
            if(aux>0){
                printf(" %d ",aux);
            }else{
                unsigned short x,y;
                x = game->players[-aux].posX;
                y = game->players[-aux].posY;
                printf("%s" "%s" RESET, colors[-aux],(i==y*w+x)?" ██":" ⯀ ");//⯀ ██
            }
        }
        printf("\n");

        printf("\x1b[1mPuntajes:\x1b[0m\n"); 
        for(int i=0; i < game->cantPlayers; i++){
            printf("Jugador %d: %d\n", i+1, game->players[i].score);
        }
        printf("-------------------------------");
        printf("\n");
        sem_post(&sems->finishedPrinting); 
        if(game->finished){
            break;
        }
    }
    //goto inicio;
    //-------------------------------
    return 0;
}

