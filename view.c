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
#define RESET   "\x1b[0m" // Resetear color


int main(int argc, char *argv[]){
    if(argc != 3){ 
        perror("argumentos incorrectos");
        exit(1);
    }
    char * colors[9] = {PLAYER1, PLAYER2, PLAYER3, PLAYER4, PLAYER5, PLAYER6, PLAYER7, PLAYER8, PLAYER9};
    int width = atoi(argv[1]);
    int height = atoi(argv[2]);

    game_t * game = (game_t*)createSHM(SHM_GAME_NAME, O_RDONLY |  O_CREAT, sizeof(game_t) + sizeof(int) * height * width, 0);   
    sync_t * sems = (sync_t*)createSHM(SHM_SYNC_NAME, O_RDWR |  O_CREAT, sizeof(sync_t), 0);    
    while(1){
        mySemWait(&sems->haveToPrint);
        for(int i = 0; i < width * height; i++){
            
            if(i % width == 0){
                printf("\n");
            }
            int board = game->board[i]; 
            if(board > 0){
                printf(" %d ", board);
            }else{
                unsigned short x,y;
                x = game->players[-board].posX;
                y = game->players[-board].posY;
                printf("%s" "%s" RESET, colors[-board],(i == y * width + x)?" ██":" ⯀ ");
            }
            
        }
        printf("\n");

        printf("\x1b[1mPuntajes:   Score / ValidMoves  / InvalidMoves  / Blocked\x1b[0m\n"); 
        for(int i = 0; i < game->cantPlayers; i++){
            printf("%sJugador %d %s: %5d /\t %5d  /\t %5d  / %s\n", colors[i], i, RESET, game->players[i].score, game->players[i].validMoves, game->players[i].invalidMoves,game->players[i].blocked?"Yes":"No");
        }
        printf("-------------------------------");
        printf("\n");
        if(game->finished){
            mySemPost(&sems->finishedPrinting); 
            break;
        }
        mySemPost(&sems->finishedPrinting); 
    }
    int winner = 0;
    int winnerArray[9];
    int cantWinners = 0;
    for (int i = 1; i < game->cantPlayers; i++) {
        if (game->players[i].score > game->players[winner].score ||
        (game->players[i].score == game->players[winner].score &&
         game->players[i].validMoves < game->players[winner].validMoves) ||
        (game->players[i].score == game->players[winner].score &&
         game->players[i].validMoves == game->players[winner].validMoves &&
         game->players[i].invalidMoves < game->players[winner].invalidMoves)) {
         winner = i;
        }else if(game->players[i].score == game->players[winner].score &&
         game->players[i].validMoves == game->players[winner].validMoves &&
         game->players[i].invalidMoves == game->players[winner].invalidMoves){
            winnerArray[++cantWinners] = i;
        }
    }
    winnerArray[0] = winner;
    if(cantWinners > 0){
        printf("\x1b[1mEmpate entre los jugadores:\x1b[0m\n");
        for(int i = 0; i <= cantWinners; i++){
            printf("%sJugador %d%s\n",colors[winnerArray[i]], winnerArray[i], RESET);
        }
    }else{
        printf("\x1b[1mGanador:\x1b[0m %sJugador %d%s\n",colors[winner], winner, RESET);
    }
    
    closeSHM(SHM_GAME_NAME, (void *)game, sizeof(game_t) + sizeof(int)* width * height, 0);
    closeSHM(SHM_SYNC_NAME, (void *)sems, sizeof(sync_t), 0);
    return 0;
}

