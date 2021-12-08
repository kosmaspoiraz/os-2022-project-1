CC=gcc

CFLAGS= -c -Wall -ggdb

output: main.o client.o server.o
	$(CC) -pthread main.o common.c -o main
	$(CC) -pthread client.o common.c -o client
	$(CC) -pthread server.o common.c -o server

main.o: main.c
	$(CC) $(CFLAGS) main.c common.c

client.o: client.c
	$(CC) $(CFLAGS) client.c common.c

server.o: server.c
	$(CC) $(CFLAGS) server.c common.c

clean:	
	rm *.o main client server 
	clear

run:main.o  client.o server.o
	clear
	$(CC) -pthread main.o common.c -o main
	$(CC) -pthread client.o common.c -o client
	$(CC) -pthread server.o common.c -o server
	./main sample.txt 2 3

gdb:main.o
	gdb ./main



