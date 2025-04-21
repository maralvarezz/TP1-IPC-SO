#include "./utils.h"

int dirs[][2]=   {{0,-1},
                    {1,-1},
                    {1,0},
                    {1,1},
                    {0,1},
                    {-1,1},
                    {-1,0},
                    {-1,-1}};

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
    close(fd);
    return toRet;
}

void closeSHM(char * name,void * dir,size_t size,char haveToUnlink){
    if (munmap(dir,size) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
    if(haveToUnlink){
        if (shm_unlink(name) == -1) {
            perror("shm_unlink");
            exit(EXIT_FAILURE);
        }
    }
}

void mySemWait(sem_t * sem){
    if(sem_wait(sem) == -1){
        perror("sem_wait ");
        exit(EXIT_FAILURE);
    }
}

void mySemPost(sem_t * sem){
    if(sem_post(sem) == -1){
        perror("sem_post ");
        exit(EXIT_FAILURE);
    }
}	

void mySemDestroy(sem_t * sem){
    if(sem_destroy(sem) == -1){
        perror("sem_destroy ");
        exit(EXIT_FAILURE);
    }
}	
void safeSem_init(sem_t* sem, int shared, int value){
    if(sem_init(sem,shared,value) == -1){
        perror("sem_init ");
        exit(EXIT_FAILURE);
    }
}