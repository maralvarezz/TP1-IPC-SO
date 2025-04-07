# ChompChamps - TP1 Sistemas Operativos

Trabajo Práctico de Sistemas Operativos - Juego multijugador en C con procesos, memoria compartida, semáforos y sincronización a través de pipes y `select()`.

## 🧠 Descripción

Este proyecto implementa un juego por turnos donde múltiples procesos representan jugadores que se mueven en un tablero compartido. La sincronización se maneja mediante memoria compartida y semáforos POSIX. El juego incluye:

- Vista visual opcional (modo interactivo).
- Múltiples jugadores controlados por procesos.
- Comunicación entre procesos usando pipes y `select()`.
- Lógica central implementada en `master.c`.

## 🧩 Estructura del proyecto