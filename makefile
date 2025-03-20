# Variables
CC = gcc
CFLAGS = -std=gnu99 -Wall
LDFLAGS = -lrt -lpthread

all: player view

player: player.c utils.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) 

#app: application.c utils.c
#	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) application

view: view.c utils.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) 

clean:
	rm -f player  view
.PHONY: all clean