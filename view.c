#include "./utils.h"


int main(int argc, char *argv[]){
    if(argc != 3){ // siempre hay almenos un argumento que es el llamado al programa ./programa siempre es el primer argumento
        perror("argumentos incorrectos");
        exit(1);
    }
    int  w =  atoi(argv[1]);
    int h = atoi(argv[2]);
    printf("in view\n");
    // pruebas ---------------------
    int fd = shm_open("/game_state" , O_RDONLY |  O_CREAT, 0666);
    if(fd ==-1){
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    game_t *game = (game_t*) mmap(NULL, sizeof(game_t) , PROT_READ, MAP_SHARED, fd, 0);
    if( game == MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    for(int i=0;i<w*h;i++){
        if(i%w == 0){
            printf("\n");
        }
        int aux = game->board[i];
        if(aux>0){
            printf(" %d ",aux);
        }else{
            printf("p%d ",-aux+1);
        }
        
    }
    

    //-------------------------------
    return 0;
}