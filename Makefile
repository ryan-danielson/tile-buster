CURRENT_DIR := $(shell pwd)
INCLUDE_DIRS := /usr/include/SDL2/
LIBRARY := /usr/include/


CC 			 := gcc
CFLAGS 		 := -I$(INCLUDE_DIRS) -L$(LIBRARY) -lSDL2 -lSDL2_image -lSDL2_mixer

OBJS 		 := tile_buster.o
DEPS 		 := ${INCLUDE_DIRS}/

.PHONY: clean all

all: tile_buster.o tile_buster

clean:
	rm $(OBJS) tile_buster

tile_buster.o: tilebuster.c ${DEPS}
	$(CC) -c -o $@ $(CFLAGS) $<

tile_buster: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)
