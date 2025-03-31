#include <math.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <poll.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>


typedef struct {
    char playerName[16]; // Nombre del jugador
    unsigned int score; // Puntaje
    unsigned int invalidMoves; // Cantidad de solicitudes de movimientos inválidas realizadas
    unsigned int validMoves; // Cantidad de solicitudes de movimientos válidas realizadas
    unsigned short posX, posY; // Coordenadas x e y en el tablero
    pid_t pid; // Identificador de proceso
    bool blocked; // Indica si el jugador esta bloqueado
} player_t;

typedef struct {
    unsigned short width; // Ancho del tablero
    unsigned short height; // Alto del tablero
    unsigned int cantPlayers; // Cantidad de jugadores
    player_t players[9]; // Lista de jugadores
    bool finished; // Indica si el juego se ha terminado
    int board[]; // Puntero al comienzo del tablero. fila-0, fila-1, ..., fila-n-1
} game_t;

typedef struct {
    sem_t haveToPrint; // Se usa para indicarle a la vista que hay cambios por imprimir
    sem_t finishedPrinting; // Se usa para indicarle al master que la vista terminó de imprimir
    sem_t C; // Mutex para evitar inanición del master al acceder al estado
    sem_t D; // Mutex para ver el estado del juego(a quién le toca)
    sem_t E; // Mutex para la siguiente variable 
    unsigned int playersReading; // Cantidad de jugadores leyendo el estado
} sync_t;
    

void * createSHM(char * name,int flags, size_t size, char haveToTruncate);
int closeSHM(char * name,void * dir,size_t size);

