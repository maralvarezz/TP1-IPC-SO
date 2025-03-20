#include "./utils.h"

void * createSHM(char * name, size_t size){
    int fd = shm_open(name , O_RDWR |  O_CREAT, 0666);
    if(fd ==-1){
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    if(-1 == ftruncate(fd, size)){
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }
    void *toRet= mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    if(toRet == MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    return toRet;
}