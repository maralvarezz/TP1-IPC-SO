#include "utils.h"

void setGame(game_t * game,unsigned int cantJug, unsigned int w, unsigned int h);
void getBoard(game_t * game, unsigned int seed);

int main(int argc, char *argv[]){
    if(argc < 3){
        perror("Cantidad de argumentos incorrectos");
        exit(1);
    }
    unsigned int delay, timeout, seed;
    FILE * view = NULL;
    //auxiliar para los players y sus files porque no se como se manejan
    //FILE * Fplayers[];
    int firtsPlayer;
    unsigned int w = 10;
    unsigned int h = 10;
    unsigned int delay = 200;
    unsigned int timeout = 10;
    unsigned int seed = time(NULL);
    int aux;
    int cantJug = 0;
    for(int i = 1; i < argc; i++){ // unico detalle -p ULTIMO
        if(strcmp(argv[i], "-w") == 0){
            if(aux = atoi(argv[i+1]) < 10){
                perror("El ancho del tablero no puede ser menor a 10");
                exit(1);
            }
            w = aux;
            i++;
        }else if(strcmp(argv[i], "-h") == 0){
            if(aux = atoi(argv[i+1]) < 10){
                perror("El alto del tablero no puede ser menor a 10");
                exit(1);
            }
            h = aux;
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
            int parametros[] = {w, h, NULL};
            firtsPlayer=i;
            for(int j = i ; j < argc; j++){
                if(j > 9){
                    perror("Maximo 9 jugadores");
                    exit(1);
                }
                cantJug++;
            }
        }
    }

    if(cantJug < 1){
        perror("Debe haber minimo un jugador");
        exit(1);
    }
    
    game_t * game = (game_t*)createSHM("/game_state",O_RDONLY |  O_CREAT, sizeof(game_t)+sizeof(int)*w*h, 0);
    sync_t * sems = (sync_t*)createSHM("/game_sync",O_RDWR |  O_CREAT, sizeof(sync_t), 0);


    setGame(game, cantJug, w, h);
    getBoard(game, seed);
    setPlayersPos(game);

    // ya ejecutado la view

    for(int i=0; i < cantJug; i++){ 
        char aux[] = {i + '0','/0'};
        strcopy(game->players[i].playerName, strconcat("Player", aux));
        game->players[i].score = 0;
        game->players[i].validMoves = 0;
        game->players[i].invalidMoves = 0;
        game->players[i].blocked = false;
    }

    

    /*
   
                    game->players[cantJug].score = 0;
                    game->players[cantJug].validMoves = 0;
                    game->players[cantJug].invalidMoves = 0;
                    game->players[cantJug].blocked = false;
                    game->cantPlayers = cantJug;
                    execve(argv[j], parametros, NULL);
    */

    if(game->cantPlayers == 0){
        perror("At least one player must be specified using -p");
        exit(1);
    }
}

void setGame(game_t * game,unsigned int cantJug,  unsigned int  w, unsigned int  h){
    game->width = w;
    game->height = h;
    game->cantPlayers = cantJug;
    game->finished = false;
    /* *delay = 200;
    *timeout = 10;
    *seed = time(NULL);
    *view = NULL;*/
}

void getBoard(game_t * game, unsigned int seed){
    srand(seed);
    int aux;
    for(int i = 0; i < (game->height * (game->width)); i++){
        game->board[i] = (rand() % 9) + 1;
    }
}

void setPlayersPos(game_t * game){
    unsigned short a = (game->height / 2) * 0.8; // Semieje vertical reducido
    unsigned short b = (game->width / 2) * 0.8;  // Semieje horizontal reducido

    if(game->cantPlayers == 1){
        game->players[0].posX = game->width / 2; // Centro del tablero
        game->players[0].posY = game->height / 2;
    } else {
        for(int i = 0; i < game->cantPlayers; i++){
            double theta = (2.0 * 3.14 * i) / game->cantPlayers; // Ángulo en radianes para cada jugador

            // Calcular las posiciones en el borde del óvalo reducido
            game->players[i].posX = (unsigned short)((game->width / 2) + b * cos(theta));
            game->players[i].posY = (unsigned short)((game->height / 2) + a * sin(theta));

            printf("Player %d: (%d, %d)\n", i, game->players[i].posX, game->players[i].posY);

            // Verificar que las posiciones estén dentro de los límites del tablero
            if (game->players[i].posX >= 0 && game->players[i].posX < game->width &&
                game->players[i].posY >= 0 && game->players[i].posY < game->height) {
                game->board[game->players[i].posY * game->width + game->players[i].posX] = -i - 1; // Colocar al jugador en el tablero
            }
        }
    }
}
