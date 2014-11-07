CC=gcc

all:
	$(CC) -o client.o client.c
	$(CC) -o server.o server.c

clean:
	rm -f ./*out
