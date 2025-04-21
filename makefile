
CC = gcc
CFLAGS = -std=gnu99 -Wall
LDFLAGS = -lrt -lpthread -lm

OBJS_PLAYER = player.o utils.o strategy.o
OBJS_DUMMY = player.o utils.o strategyDummy.o
OBJS_MASTER = master.o utils.o masterLib.o
OBJS_VIEW = view.o utils.o

all: player playerDummy master view

player: $(OBJS_PLAYER)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

playerDummy: $(OBJS_DUMMY)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

master: $(OBJS_MASTER)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

view: $(OBJS_VIEW)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	rm -f *.o player playerDummy master view

.PHONY: all clean