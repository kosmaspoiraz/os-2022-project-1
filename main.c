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

// Function to count how many lines in File
int countLines(FILE *file)
{
    char c;
    int lines = 0;
    for (c = getc(file); c != EOF; c = getc(file))
        if (c == '\n') // Increment count if this character is newline
            lines = lines + 1;
    return lines;
}

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

// Main program
int main(int argc, char *argv[])
{
    int numLines = 0;
    int pid[numChildren];
    FILE *fptr;

    // Initialize semaphores
    sem_t *semaphoreRead, *semaphoreWrite;
    semaphoreRead = sem_open("Read", O_CREAT, 0660, 1);
    semaphoreWrite = sem_open("Write", O_CREAT, 0660, 1);

    // Open File and find how many lines
    fptr = fopen(fname, "r");
    if (fptr == NULL)
    {
        perror("Error opening file\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        numLines = countLines(fptr);
    }
    fclose(fptr);

    // Initialize shared memory
    struct shared_memory *sharedMemory;
    int shm_id = shmget(IPC_PRIVATE, sizeof(struct shared_memory), IPC_CREAT | 0666);
    if (shm_id == -1)
    {
        perror("Error with shmget");
        exit(EXIT_FAILURE);
    }
    sharedMemory = shmat(shm_id, NULL, 0);
    sharedMemory->lineNum = 0;

    // Create children processes
    int i;
    for (i = 0; i < numChildren; i++)
    {
        pid[i] = fork();
        // Child proccess writes numActions times to buffer
        if (pid[i] == -1)
        {
            perror("Error with fork");
            exit(EXIT_FAILURE);
        }
        else if (pid[i] == 0)
        {
            execv("./client", 0);
        }
        else
        {
            // Parent process waits requests one at a time and responds
            execv("./server", 0);
        }
    }

    // Wait for everyone to finish
    while (wait(NULL) > 0)
        ;

    // Free and unlink semaphores
    sem_close(semaphoreRead);
    sem_close(semaphoreWrite);
    sem_unlink("Read");
    sem_unlink("Write");

    // Detach and free memory
    shmdt(sharedMemory);
    shmctl(shm_id, IPC_RMID, NULL);
    printf("Bla\n");

    return 0;
}