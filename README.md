# ChompChamps - TP1 Sistemas Operativos

Trabajo Práctico de Sistemas Operativos - Juego multijugador en C con procesos, memoria compartida, semáforos y sincronización a través de pipes y `select()`.

## Descripción

Este proyecto implementa un juego por turnos donde múltiples procesos representan jugadores que se mueven en un tablero compartido. La sincronización se maneja mediante memoria compartida y semáforos POSIX. El juego incluye:

- Vista visual opcional (modo interactivo).
- Múltiples jugadores controlados por procesos.
- Comunicación entre procesos usando pipes y `select()`.
- Lógica central implementada en `master.c`.

## Requisitos:

Para utilizarlo, debe contar con Docker instalado y utilizar la imagen provista por la cátedra: ```agodio/itba-so-multi-platform:3.0```

## Instrucciones:

Para compilar el programa realizar:
``` bash 
make all
```

## Instrucciones para ejecutar el juego

La forma correcta de correr el juego es:

```bash
./master [opciones]  -p <jugador1> [jugador2 ... jugador9] 
```

### Argumento obligatorio

- `-p`: Indica los ejecutables de los jugadores.  
  Debés pasar al menos 1 y como máximo 9 jugadores.  
  Ejemplo:
  ```bash
  ./master -p player player player
  ```

---

### Argumentos opcionales

- `-v`: Habilita la vista visual del juego (por defecto: deshabilitada)
- `-w <ancho>`: Ancho del tablero (por defecto: 10)
- `-h <alto>`: Alto del tablero (por defecto: 10)
- `-d <delay>`: Tiempo de delay entre turnos en milisegundos (por defecto: 200)
- `-t <timeout>`: Tiempo máximo para que un jugador responda en segundos (por defecto: 10)
- `-s <semilla>`: Semilla para el generador aleatorio (por defecto: aleatoria)

> Si no se especifican, estos parámetros toman los siguientes valores por defecto:
> 
> - `width`: 10  
> - `height`: 10  
> - `delay`: 200 ms  
> - `timeout`: 10 s  
> - `seed`: aleatoria  
> - `view`: desactivada  

---

## Integrantes:

María Victoria Alvarez (63165) - maralvarez@itba.edu.ar

Santiago Sanchez Marostica (64056) - ssanchezmarostica@itba.edu.ar

Martín Gallardo Bertuzzi (63510) - mgallardobertuzzi@itba.edu.ar


