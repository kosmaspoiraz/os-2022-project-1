CC=gcc

CFLAGS= -c -Wall

output: main.o 
	$(CC) -pthread main.o -o output

main.o: main.c
	$(CC) $(CFLAGS) main.c

clean:	
	rm *.o output
	clear

run:
	./output



