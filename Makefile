CC=gcc

CFLAGS= -c -Wall -ggdb

output: main.o client.o server.o
	$(CC) -pthread main.o -o main
	$(CC) -pthread client.o -o client
	$(CC) -pthread server.o -o server

main.o: main.c
	$(CC) $(CFLAGS) main.c

client.o: client.c
	$(CC) $(CFLAGS) client.c

server.o: server.c
	$(CC) $(CFLAGS) server.c

clean:	
	rm *.o main client server
	clear

run:main.o  client.o server.o
	clear
	$(CC) -pthread main.o -o main
	$(CC) -pthread client.o -o client
	$(CC) -pthread server.o -o server
	./main sample.txt 2 3

gdb:main.o
	gdb ./main



