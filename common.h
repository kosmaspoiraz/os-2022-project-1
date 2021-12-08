#ifndef COMMON_H_
#define COMMON_H_

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#define FSIZE 1024
#define SEM_PERMS O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP
#define MEM_KEY IPC_PRIVATE
#define SHM_FLAGS IPC_CREAT | 0666

// Shared memory struct
struct shared_memory
{
    char foundLine[FSIZE];
    int requestedLine;
};

// Function to count how many lines in File
int countLines(FILE *file);

// Function to print requested line number
void findLine(int *lineNum, int *numLines, struct shared_memory *sharedMemory, char *fname);

#endif
