#include "./utils.h"

#define SHARED 1



void setGame(game_t * game,unsigned int cantJug, unsigned int w, unsigned int h);
void getBoard(game_t * game, unsigned int seed);
void setPlayersPos(game_t * game);
void setPlayers(game_t * game,int pipefds[][2],int cantJug);
void createSems(sync_t * sems);
void closeSems(sync_t * sems);
void checkAndBlockPlayer(game_t * game,int playerNum);
void makeMove(game_t* game, sync_t * sems,fd_set * readFDS, int pipefds[][2], int cantJug,int viewPID);
void move(game_t * game, sync_t sems,int playerNum, unsigned char dirIdx);



void safeSem_init(sem_t* sem, int shared, int value);
void safeClose(int fd);

void closeAllNotNeededFD(int pipefds[][2],int cantJug,int playerNum);
extern int dirs[][2];



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
    int option;

    /*CORREGIR EL FORMATP DE ESTO*/
    while ((option = getopt(argc, argv, "w:h:d:t:s:v:p:")) != -1) {
        printf("Procesando opción: -%c, optarg: %s, optind: %d\n", option, optarg, optind);
        switch (option) {
            case 'w':
                if (optarg == NULL) {
                    perror("Falta el argumento para -w");
                    exit(1);
                }
                if ((aux = atoi(optarg)) < 10) {
                    perror("El ancho del tablero no puede ser menor a 10");
                    exit(1);
                }
                w = aux;
                break;
            case 'h':
                if (optarg == NULL) {
                    perror("Falta el argumento para -h");
                    exit(1);
                }
                if ((aux = atoi(optarg)) < 10) {
                    perror("El alto del tablero no puede ser menor a 10");
                    exit(1);
                }
                h = aux;
                break;
            case 'd':
                if (optarg == NULL) {
                    perror("Falta el argumento para -d");
                    exit(1);
                }
                delay = atoi(optarg);
                break;
            case 't':
                if (optarg == NULL) {
                    perror("Falta el argumento para -t");
                    exit(1);
                }
                timeout = atoi(optarg);
                break;
            case 's':
                if (optarg == NULL) {
                    perror("Falta el argumento para -s");
                    exit(1);
                }
                seed = atoi(optarg);
                break;
            case 'v':
                if (optarg == NULL) {
                    perror("Falta el argumento para -v");
                    exit(1);
                }
                view = optarg;
                break;
            case 'p':
                firtsPlayer = optind - 1;
                if (firtsPlayer >= argc) {
                    perror("Debe especificar al menos un jugador después de -p");
                    exit(1);
                }
                for (int j = firtsPlayer; j < argc; j++) {
                    if ((j - firtsPlayer) >= 9) {
                        perror("Máximo 9 jugadores");
                        exit(1);
                    }
                    cantJug++;
                }
                optind = argc; // Saltar el resto de los argumentos
                break;
            default:
                fprintf(stderr, "Opción no reconocida: -%c\n", optopt);
                exit(1);
        }
    }
    
    /*for(int i = 1; i < argc; i++){ // unico detalle -p ULTIMO
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
    }*/

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
    
    
    int viewRet;
    pid_t viewPID;
    //seria solo si pasan una view
    if(view != NULL){
        viewPID=fork();
        if(viewPID==0){
            printf(CLEAR);
            printf("width = %d\nheight = %d\n delay = %dms\ntimeout = %ds\nseed=%d\nview = a\n",w,h,delay,timeout,seed);
            return 0;
            //execve()
        }
        //esto es momentaneo!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    }
    

    // ya ejecutado la view
    int pipefds[cantJug][2];
    setPlayers(game,pipefds,cantJug); // ahora quedan bien todos los pipes precreados y  los jugadores ya ubicados en el tablero
    
    //ESPACIO PARA TERMINAR DE SETTEAR LAS COSAS DEL MASTER ANTES DE ARRANCAR EL JUEGO

    //PREPARACION DE LOS PLAYERS Y EXECVE DE C/U
    for(int i=0; i < cantJug; i++){ 
        pid_t childPID = fork();
        if(childPID == -1){
            perror("fork");
            exit(EXIT_FAILURE);
        }else if(childPID == 0){ 
            closeAllNotNeededFD(pipefds,cantJug,i);
            safeClose(pipefds[i][0]);
            dup2(pipefds[i][1], STDOUT_FILENO);
            safeClose(pipefds[i][1]);
            printf("%d",i);
            return 0;//CAMBIAR ESTE RETURN POR EL EXECVE DE CADA PLAYER
            // execve() falta ver que ejecuta
        }else{
            game->players[i].pid = childPID;
        }
    }
    int playersReturns[cantJug];
    fd_set readFDS;
    FD_ZERO(&readFDS); // inicializo en 0 el fd_set

    for(int i = 0; i < cantJug; i++) {
        safeClose(pipefds[i][1]);
        FD_SET(pipefds[i][0], &readFDS);
    }
    
    struct timeval timeoutForSelect;
    char finished  = 0;
    while(!finished){
        timeoutForSelect.tv_sec = timeout;
        timeoutForSelect.tv_usec = 0;
        int status = select(pipefds[cantJug - 1][0] + 1, &readFDS, NULL, NULL,&timeoutForSelect);
        if(status == -1){
            perror("select");
            exit(EXIT_FAILURE);
        }else if(status == 0){
            finished = 1;
            //espero a que todos los jugadores hayan salido del game board y aviso que termino el juego
            sem_wait(&sems->masterMutex);
            sem_wait(&sems->gameStatusMutex);
            game->finished=true;
            sem_post(&sems->masterMutex);
            sem_post(&sems->gameStatusMutex);
        }else{
            
            //PLAY


        }
    }

    if(view != NULL){
        waitpid(viewPID,&viewRet,0);
    }
    
    for(int i=0; i < cantJug; i++){
        waitpid(game->players[i].pid, &playersReturns[i], 0);
    }
    
    for(int i = 0; i < cantJug; i++){
        printf("El jugador %d (%s) devolvio el valor %d\n", i, game->players[i].playerName, playersReturns[i]);
    }
    if(view != NULL){
        printf("El view devolvio el valor %d\n", viewRet);
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

void makeMove(game_t* game, sync_t * sems,fd_set * readFDS, int pipefds[][2], int cantJug,int viewPID){ //el PID de la view es para saber si se ejecuto o no
    static int playerTurn = 0;
    for(int i = 0; i < cantJug; i++){
        if(!game->players[i].blocked){
            unsigned char dir;
            int bytesRead = read(pipefds[i][0], &aux, sizeof(unsigned char));
            if (bytesRead < 0) {
                perror("read");
                exit(EXIT_FAILURE);
            }else if(bytesRead == 0){
                game->players[i].blocked = true;
                FD_CLR(pipefds[i][0], readFDS);
            }else{ //salgo del ciclo unicamente si se proceso un movimiento
                
                playerTurn= (playerTurn + 1)%cantJug;
                break;
            }
        }
        playerTurn= (playerTurn + 1)%cantJug;
    }
}

void move(game_t * game, sync_t sems,int playerNum, unsigned char dirIdx){
    
    sem_wait(&sems->masterMutex);
    sem_wait(&sems->gameStatusMutex);
    int newPosValue;
    unsigned short w,h;
    w = game->width;
    h = game->height;
    player_t* playerToMove= &game->players[playerNum]
    if ((newPosValue=game->board[w*(playerToMove->posY+dirs[dirIdx][1])+dirs[dirIdx][0]+playerToMove->posX])>0){
        playerToMove->score+=newPosValue;
        playertoMove->posX+=dirs[dirIdx][0];
        playertoMove->posY+=dirs[dirIdx][1];
        playerToMove->validMoves++;
        checkAndBlockPlayers(game,playerToMove->posX,playerToMove->posY,cantJug);//tengo q cheackear si hay jugadores bloqueados unicamente si 
                                                                                 // hubo un cambio en el tablero, sino no es necesario porque 
                                                                                 //no cambio el estado  del tablero.
    }else{
        playerToMove->invalidMoves++;
    }
    sem_post(&sems->masterMutex);
    sem_post(&sems->gameStatusMutex);
}
//falte ver en que arrancan los semaforos
void createSems(sync_t * sems){
    safeSem_init(&sems->haveToPrint,SHARED,0);
    safeSem_init(&sems->finishedPrinting,SHARED,0);
    safeSem_init(&sems->masterMutex,SHARED,1);
    safeSem_init(&sems->gameStatusMutex,SHARED,1);
    safeSem_init(&sems->playersReadingMutex,SHARED,1);
    sems->playersReading = 0;
}

void checkAndBlockPlayer(game_t * game,int playerNum,int notFirstTime){ // me falta ver si alguno de los adyacentes al lugar, no es otra cabeza y tengo q ponerla bloqueada
    unsigned short x,y;
    int cantPosOcuppied=0;
    int isOtherPlayer = 0;
    int aux;
    int pos;
    x= game->players[playerNum].posX;
    y= game->players[playerNum].posY;
    for(int i = 0; i < 8; i++){
        pos =w*(y+dirs[i][1])+dirs[i][0]+x;
        if(y+dirs[i][1]>=0 && y+dirs[i][1]<h && dirs[i][0]+x >=0 && dirs[i][0]+x < w ){
            if((aux = game->board[pos] ) < 0){ 
                cantPosOcuppied++;
                if(y+dirs[i][1] == game->players[-aux].posY && x+dirs[i][0] == game->players[-aux].posX){
                    checkAndBlockPlayer(game, -aux, 1);
                }
            }else if(notFirstTime){
                break;
            }
        }
    }
    if(cantPosOcuppied == 8){
        game->players[playerNum].blocked=true;
    }
}


void safeSem_init(sem_t* sem, int shared, int value){
    if(sem_init(sem,shared,value)==-1){
        perror("sem_init ");
        exit(EXIT_FAILURE);
    }
}

void closeAllNotNeededFD(int pipefds[][2],int cantJug,int playerNum){
    for(int i=0;i<cantJug;i++){
        if(i!=playerNum){
            safeClose(pipefds[i][0]);
            safeClose(pipefds[i][1]);
        }
    }
}

void safeClose(int fd){
    if(close(fd)==-1){
        perror("close");
        exit(EXIT_FAILURE);
    }
}