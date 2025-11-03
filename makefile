CC = gcc
CFLAGS = -Wall -Werror -pedantic -g

all: mymalloc

mymalloc: mymalloc.c
	$(CC) $(CFLAGS) -o mymalloc mymalloc.c 

clean:
	rm mymalloc.o mymalloc