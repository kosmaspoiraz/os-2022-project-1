CC=gcc

CFLAGS= -c -Wall

output: main.o 
	$(CC) -pthread main.o -o main

main.o: main.c
	$(CC) $(CFLAGS) main.c

clean:	
	rm *.o main
	clear

run:main.o 
	clear
	$(CC) -pthread main.o -o main
	./main



