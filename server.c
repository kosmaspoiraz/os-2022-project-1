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

#define FSIZE 100
#define numChildren 2
#define numActions 2
#define fname "sample.txt"
#define MAXSIZE numChildren *numActions

// Shared memory struct containg buffer[numChildren][numActions]
struct shared_memory
{
    char *buffer;
    int lineNum;
};

// Function to print requested line number
void findLine(int *lineNum, int *numLines, struct shared_memory *sharedMemory)
{
    FILE *file = fopen(fname, "r");
    if (file == NULL)
    {
        perror("Error opening file\n");
        exit(EXIT_FAILURE);
    }
    int count = 1;
    char line[FSIZE * (*numLines)];

    while (fgets(line, sizeof(line), file) != NULL)
    {
        line[strlen(line) - 1] = '\0';
        if (count == *lineNum)
        {
            sharedMemory->buffer = 0;
            printf("Child (%d) requested Line Number %d\n", getpid(), sharedMemory->lineNum);
            sharedMemory->buffer = line;
            fclose(file);
            return;
        }
        else
        {
            count++;
        }
    }
}

int main()
{
    printf("Server is running...\n");

        return 0;
}