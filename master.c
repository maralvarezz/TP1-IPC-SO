#include "./masterLib.h"

int main(int argc, char *argv[]){
    if(argc < 3){
        perror("Cantidad de argumentos incorrectos");
        exit(1);
    }
    char * view = NULL;
    int firtsPlayer, param, option;
    unsigned int width = 10;
    unsigned int height = 10;
    unsigned int delay = 200;
    unsigned int timeout = 10;
    unsigned int seed = time(NULL);
    int cantPlayers = 0;
    while ((option = getopt(argc, argv, "w:h:d:t:s:v:p:")) != -1) {
        switch (option) {
            case 'w':
                if (optarg == NULL || strncmp(optarg, "-", 1) == 0) {
                    break;
                }
                if ((param = atoi(optarg)) < 10) {
                    perror("El ancho del tablero no puede ser menor a 10");
                    exit(1);
                }
                width = param;
                break;
            case 'h':
                if (optarg == NULL || strncmp(optarg, "-", 1) == 0) {
                    break;
                }
                if ((param = atoi(optarg)) < 10) {
                    perror("El alto del tablero no puede ser menor a 10");
                    exit(1);
                }
                height = param;
                ;
                break;
            case 'd':
                if (optarg != NULL && strncmp(optarg, "-", 1) != 0) {
                    delay = atoi(optarg);
                }
                else{
                    printf("%s\n", optarg);
                }
                break;
            case 't':
                if (optarg == NULL || strncmp(optarg, "-", 1) == 0) {
                    break;
                }
                timeout = atoi(optarg);
                break;
            case 's':
                if (optarg == NULL || strncmp(optarg, "-", 1) == 0) {
                    break;
                }
                seed = atoi(optarg);
                break;
            case 'v':
                if (optarg == NULL || strncmp(optarg, "-", 1) == 0) {
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
                    cantPlayers++;
                }
                optind = argc;
                break;
            default:
                fprintf(stderr, "Opción no reconocida: -%c\n", optopt);
                exit(1);
        }
    }
    if(cantPlayers < 1){
        perror("Debe haber minimo un jugador");
        exit(1);
    }
    game_t * game = (game_t *) createSHM(SHM_GAME_NAME, O_RDWR |  O_CREAT, sizeof(game_t) + sizeof(int) * width * height, 1);
    sync_t * sems = (sync_t *) createSHM(SHM_SYNC_NAME, O_RDWR |  O_CREAT, sizeof(sync_t), 1);
    createSems(sems);
    setGame(game, cantPlayers, width, height);
    getBoard(game, seed);
    char * argv2[4] = {0};
    argv2[3] = NULL;
    int viewRet;
    pid_t viewPID;
    char argvHeight[MAXDIGITS] = {0};
    char argvWidth[MAXDIGITS] = {0};
    sprintf(argvHeight, "%d", height);
    sprintf(argvWidth, "%d", width);
    argv2[1] = argvWidth;
    argv2[2] = argvHeight;
    printf(CLEAR);
    printf("width = %d\nheight = %d\ndelay = %dms\ntimeout = %ds\nseed=%d\nview = %s\nplayers:\n", width, height, delay, timeout, seed, view != NULL?view : " - ");
    for(int i = 0; i < cantPlayers; i++){
        printf("Jugador %d: %s\n", i, argv[firtsPlayer + i]);
    }
    sleep(3);
    if(view != NULL){
        if (access(view, X_OK) == 0) {
            viewPID = fork();
            if(viewPID == 0){
                argv2[0] = view;
                if(execve(view, argv2, NULL)==-1){
                    perror("execve view");
                    exit(EXIT_FAILURE);
                }
            }
        }else{
            printf("view NO ejecutable\n");
            view=NULL;
        }
    }
    int pipefds[cantPlayers][2];
    initializePlayers(game, pipefds, cantPlayers); 
    for(int i = 0; i < cantPlayers; i++){ 
        if(access(argv[firtsPlayer + i], X_OK) == 0){
            pid_t childPID = fork();
            if(childPID == -1){
                perror("fork");
                exit(EXIT_FAILURE);
            }else if(childPID == 0){
                closeAllNotNeededFD(pipefds, cantPlayers, i);
                safeClose(pipefds[i][0]);
                dup2(pipefds[i][1], STDOUT_FILENO);
                safeClose(pipefds[i][1]);
                argv2[0] = argv[firtsPlayer + i];
                execve(argv[firtsPlayer + i], argv2, NULL);
            }else{ 
                game->players[i].pid = childPID;
            }
        }else{
            game->players[i].pid = 0;
        }
    }
    
    
    fd_set readFDS, masterFDS;
    FD_ZERO(&masterFDS); 
    for(int i = 0; i < cantPlayers; i++) {
        if(game->players[i].pid == 0){
            safeClose(pipefds[i][0]);
        }else{
            FD_SET(pipefds[i][0], &masterFDS);
        }
        safeClose(pipefds[i][1]);
    }
    struct timeval timeoutForSelect;
    char finished = 0;
    int haveToPrintFlag = 1;
    time_t lastValidMoveTime = time(NULL);
    while(!finished){
        if(view != NULL && haveToPrintFlag){
            mySemPost(&sems->haveToPrint);
            mySemWait(&sems->finishedPrinting);
            haveToPrintFlag = 0;
        }
        readFDS = masterFDS; 
        timeoutForSelect.tv_sec = timeout;
        timeoutForSelect.tv_usec = 0;
        int status = select(pipefds[cantPlayers - 1][0] + 1, &readFDS, NULL, NULL, &timeoutForSelect);
        if(status == -1){
            perror("select");
            exit(EXIT_FAILURE);
        }else if(status == 0){
            finishGame(game, sems);
            finished = 1;
            
        }else{
            if(makeMove(game, sems, &readFDS, &masterFDS, pipefds, cantPlayers, &finished, &lastValidMoveTime,timeout) && view != NULL){
                haveToPrintFlag = 1;
                usleep(delay * 1000);
            }
            if(finished){
                finishGame(game, sems);
            }
        }
    }
    if(view != NULL){
        mySemPost(&sems->haveToPrint);
        mySemWait(&sems->finishedPrinting);
        waitpid(viewPID, &viewRet, 0);
        printf("El view (%s) devolvio el valor %d\n", view ,viewRet);
    }

    int playersReturns[cantPlayers];

    for(int i=0; i < cantPlayers; i++){
        if(game->players[i].pid != 0){
            safeClose(pipefds[i][0]);
            waitpid(game->players[i].pid, &playersReturns[i], 0);
        }else{
            playersReturns[i] = 256;
        }
        printf("El jugador %d (%s) retorno el valor %3d con puntaje %5d / %5d / %5d \n", i, argv[firtsPlayer + i], playersReturns[i], game->players[i].score, game->players[i].validMoves, game->players[i].invalidMoves);
    }
    closeSems(sems);
    closeSHM(SHM_GAME_NAME, (void *)game, sizeof(game_t) + sizeof(int) * width * height, 1);
    closeSHM(SHM_SYNC_NAME, (void *)sems, sizeof(sync_t), 1);
    return 0;
}
