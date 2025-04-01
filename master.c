#include "./utils.h"

#define SHARED 1



void setGame(game_t * game,unsigned int cantJug, unsigned int w, unsigned int h);
void getBoard(game_t * game, unsigned int seed);
void setPlayersPos(game_t * game);
void setPlayers(game_t * game,int pipefds[][2],int cantJug);
void createSems(sync_t * sems);
void closeSems(sync_t * sems);
void safeSem_init(sem_t* sem, int shared, int value);


int main(int argc, char *argv[]){
    if(argc < 3){
        perror("Cantidad de argumentos incorrectos");
        exit(1);
    }
    //pruebas
    //FILE * view = NULL;
    char * view = NULL;
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
            if((aux = atoi(argv[i+1])) < 10){
                perror("El ancho del tablero no puede ser menor a 10");
                exit(1);
            }
            w = aux;
            i++;
        }else if(strcmp(argv[i], "-h") == 0){
            if((aux = atoi(argv[i+1])) < 10){
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
            //view = fopen(argv[i+1], "r"); // ver si estan bien los permisos
            view=argv[i+1];
            i++;
        }else if(strcmp(argv[i], "-p") == 0){
            i++;
            int parametros[] = {w, h, 0};
            firtsPlayer=i;
            for(int j = i ; j < argc; j++){
                if((j - i) > 9){
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
    printf("termine de procesar args\n");
    game_t * game = (game_t*)createSHM(SHM_GAME_NAME,O_RDWR |  O_CREAT, sizeof(game_t)+sizeof(int)*w*h, 1);
    sync_t * sems = (sync_t*)createSHM(SHM_SYNC_NAME,O_RDWR |  O_CREAT, sizeof(sync_t), 1);
    //falta inicializar los semaforos
    createSems(sems);
    setGame(game, cantJug, w, h);
    getBoard(game, seed);
    
    

    //seria solo si pasan una view
    int viewPID=fork();
    if(viewPID==0){
        printf("\033[H\033[J");
        printf("width = %d\nheight = %d\n delay = %dms\ntimeout = %ds\nseed=%d\nview = a\n",w,h,delay,timeout,seed);
        return 0;
        //execve()
    }
    waitpid(viewPID,NULL,0);//esto es momentaneo!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // ya ejecutado la view
    int pipefds[cantJug][2];
    setPlayers(game,pipefds,cantJug); // ahora quedan bien todos los pipes precreados y ya ubicados en el tablero
    
    //ESPACIO PARA TERMINAR DE SETTEAR LAS COSAS DEL MASTER ANTES DE ARRANCAR EL JUEGO

    //PREPARACION DE LOS PLAYERS Y EXECVE DE C/U
    for(int i=0; i < cantJug; i++){ 
        pid_t childPID = fork();
        if(childPID == -1){
            perror("fork");
            exit(EXIT_FAILURE);
        }else if(childPID == 0){ 
            close(pipefds[i][0]);
            dup2(pipefds[i][1], STDOUT_FILENO);
            close(pipefds[i][1]);
            return 0;//CAMBIAR ESTE RETURN POR EL EXECVE DE CADA PLAYER
            // execve() falta ver que ejecuta
        }else{

            waitpid(childPID,NULL,0);
            close(pipefds[i][1]);
            game->players[i].pid = childPID;
        }
    }



    //falta cerrar semaforos antes de cerrar las shms
    closeSHM(SHM_GAME_NAME,(void *)game, sizeof(game_t)+sizeof(int)*w*h,1);
    closeSHM(SHM_SYNC_NAME,(void *)sems, sizeof(sync_t),1);



    

    /*
   
                    game->players[cantJug].score = 0;
                    game->players[cantJug].validMoves = 0;
                    game->players[cantJug].invalidMoves = 0;
                    game->players[cantJug].blocked = false;
                    game->cantPlayers = cantJug;
                    execve(argv[j], parametros, NULL);
    */

    /*if(game->cantPlayers == 0){
        perror("At least one player must be specified using -p");
        exit(1);
    }*/
   return 0;
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
            if (game->players[i].posX >= 0 && game->players[i].posX < game->width && game->players[i].posY >= 0 && game->players[i].posY < game->height) {
                game->board[game->players[i].posY * game->width + game->players[i].posX] = -i; // Colocar al jugador en el tablero
            }
        }
    }
}

void setPlayers(game_t * game,int pipefds[][2],int cantJug){
    for(int i=0;i<cantJug;i++){
        char aux[] = {'P','l','a','y','e','r',' ',i + '0','\0'};
        strcpy(game->players[i].playerName,aux);
        game->players[i].score = 0;
        game->players[i].validMoves = 0;
        game->players[i].invalidMoves = 0;
        game->players[i].blocked = false;
        if(pipe(pipefds[i])==-1){
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }
    setPlayersPos(game);
}

//falte ver en que arrancan los semaforos
void createSems(sync_t * sems){
    safeSem_init(&sems->haveToPrint,SHARED,0);
    safeSem_init(&sems->finishedPrinting,SHARED,0);
    safeSem_init(&sems->masterMutex,SHARED,0);
    safeSem_init(&sems->gameStatusMutex,SHARED,0);
    safeSem_init(&sems->playersReadingMutex,SHARED,0);
    sems->playersReading = 0;
}

void safeSem_init(sem_t* sem, int shared, int value){
    if(sem_init(sem,shared,value)==-1){
        perror("sem_init ");
        exit(EXIT_FAILURE);
    }
}