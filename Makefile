CC=gcc

CFLAGS= -c -Wall -ggdb

output: main.o child.o parent.o
	$(CC) -pthread main.o common.c -o main
	$(CC) -pthread child.o common.c -o child
	$(CC) -pthread parent.o common.c -o parent

main.o: main.c
	$(CC) $(CFLAGS) main.c common.c

child.o: child.c
	$(CC) $(CFLAGS) child.c common.c

parent.o: parent.c
	$(CC) $(CFLAGS) parent.c common.c

clean:	
	rm *.o main child parent 
	clear

run:main.o  child.o parent.o
	clear
	$(CC) -pthread main.o common.c -o main
	$(CC) -pthread child.o common.c -o child
	$(CC) -pthread parent.o common.c -o parent
	./main sample.txt 2 3

gdb:main.o
	gdb ./main



