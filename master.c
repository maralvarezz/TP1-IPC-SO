#include "utils.h"

void preSet(game_t * game, unsigned int * delay, unsigned int * timeout, unsigned int * seed, FILE ** view);
void getBoard(game_t * game, unsigned int seed);

int main(int argc, char *argv[]){

    if(argc < 3){
        perror("Cantidad de argumentos incorrectos");
        exit(1);
    }
    
    game_t* game = (game_t*)createSHM("/game_state",O_RDONLY |  O_CREAT, sizeof(game_t), 0);
    sync_t *sems = (sync_t*)createSHM("/game_sync",O_RDWR |  O_CREAT, sizeof(sync_t), 0);

    unsigned int delay, timeout, seed;
    FILE * view;
    //auxiliar para los players y sus files porque no se como se manejan
    //FILE * Fplayers[];
    preSet(game, &delay, &timeout, &seed, &view);

    /*FALTA PROG DEFENSIVA (dio paja)*/
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-w") == 0){
            game->width = atoi(argv[i+1]);
            i++;
        }else if(strcmp(argv[i], "-h") == 0){
            game->height = atoi(argv[i+1]);
            i++;
        }else if(strcmp(argv[i], "-d") == 0){
            delay = atoi(argv[i+1]);
            i++;
        }else if(strcmp(argv[i], "-t") == 0){
            timeout = atoi(argv[i+1]);
            i++;
        }else if(strcmp(argv[i], "-s") == 0){
            seed = atoi(argv[i+1]);
            i++;
        }else if(strcmp(argv[i], "-v") == 0){
            view = fopen(argv[i+1], "r"); // ver si estan bien los permisos
            i++;
        }else if(strcmp(argv[i], "-p") == 0){
            i++;
            //falta ver como sacar el pid teniendo solo el binario
            /* Esto supongo que esta mal 
            strcmp(argv[i][0], "-") == 0 --> me guardo esto
            for(int j = 0; j < argc; j++){
                if(j > 9){
                    perror("Maximo 9 jugadores");
                    exit(1);
                }
                if(sizeof(argv[i]) == sizeof(FILE*)){
                    Fplayers[j] = fopen(argv[i], "r");
                    game->players[j].playerName = strconcat("Player", j);
                    game->players[j].score = 0;
                    game->players[j].validMoves = 0;
                    game->players[j].invalidMoves = 0;
                    game->players[j].blocked = false;
                    i++;
                }
                else{
                    break;
                }
            }
            game->cantPlayers = j;
            */
        }
    }

    getBoard(game, seed);
    setPlayersPos(game);


    if(game->cantPlayers == 0){
        perror("At least one player must be specified using -p");
        exit(1);
    }

}

void preSet(game_t * game, unsigned int * delay, unsigned int * timeout, unsigned int * seed, FILE ** view){
    game->width = 10;
    game->height = 10;
    game->cantPlayers = 0;
    game->finished = false;
    *delay = 200;
    *timeout = 10;
    *seed = time(NULL);
    *view = NULL;
}

void getBoard(game_t * game, unsigned int seed){
    srand(seed);
    int aux;
    for(int i = 0; i < (game->height * (game->width)); i++){
        do{
            aux = rand() % 10;
        } while (aux == 0);
        game->board[i] = aux;
    }
}

void setPlayersPos(game_t * game){

}
