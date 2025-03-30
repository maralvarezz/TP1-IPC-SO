#include "./utils.h"
//crea una memoria compartida o la abre y retorna un puntero a la misma
void * createSHM(char * name,int flags, size_t size, char haveToTruncate){
    int fd = shm_open(name , flags , 0666);
    if(fd ==-1){
        perror("shm_open");
        exit(EXIT_FAILURE);
    }if(haveToTruncate){
        if(-1 == ftruncate(fd, size)){
            perror("ftruncate");
            exit(EXIT_FAILURE);
        }
    }
    int f;
    if(flags == (O_RDWR |  O_CREAT)){
        f=PROT_WRITE | PROT_READ;
    }else{
        f=PROT_READ;
    }
    void *toRet= mmap(NULL, size, f , MAP_SHARED, fd, 0);
    if(toRet == MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    
    return toRet;
}
