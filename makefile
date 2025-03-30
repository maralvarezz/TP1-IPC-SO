# Variables
CC = gcc
CFLAGS = -std=gnu99 -Wall
LDFLAGS = -lrt -lpthread -lm

all: player view

player: player.c utils.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) 

master: master.c utils.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)


view: view.c utils.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) 

clean:
	rm -f player  view  master
.PHONY: all clean