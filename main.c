#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <unistd.h>

#define FSIZE 100
#define numChildren 2
#define numActions 10
#define fname "sample.txt"

// Shared memory struct containg buffer[numChildren][numActions]
struct shared_memory
{
    char buffer[numChildren][numActions];
    int child_index;
    int action_index;
};

void writeToBuffer(int numLine, struct shared_memory *sharedMemory)
{
    sharedMemory->buffer[sharedMemory->child_index][sharedMemory->action_index++] = numLine;
}

int readFromBuffer(struct shared_memory *sharedMemory, int *childIndex, int *actionIndex)
{
    int i = *childIndex;
    int j = *actionIndex;
    return sharedMemory->buffer[i][j];
}

// Function to count how many lines in File
int countLines(FILE *file)
{
    char c;
    int lines;
    for (c = getc(file); c != EOF; c = getc(file))
        if (c == '\n') // Increment count if this character is newline
            lines = lines + 1;
    return lines;
}

// Function to print requested line of File
void printLine(int lineNum, FILE *file, int childIndex, int actionIndex)
{
    char *line = NULL;
    size_t bla;
    int count = 0;
    size_t size = 100;
    bla = getline(&line, &size, file);

    while (bla != EOF)
    {
        if (count == lineNum)
        {
            printf("Child (%d) in action (%d): %s\n", childIndex, actionIndex, line);
            break;
        }
        else
        {
            count++;
        }
    }
}

int main(int argc, char *argv[])
{
    int numLines = 0;
    int pid[numChildren];
    FILE *fptr;

    sem_t semaphore;
    sem_init(&semaphore, 0, 1);

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

    // Initialize shared memory
    struct shared_memory *sharedMemory;
    int shm_id = shmget(IPC_PRIVATE, sizeof(struct shared_memory), 0600);
    if (shm_id == -1)
    {
        perror("Error with shmget");
        exit(EXIT_FAILURE);
    }
    sharedMemory = shmat(shm_id, NULL, 0);
    sharedMemory->action_index = sharedMemory->child_index = 0;

    // Create children processes
    for (int i = 0; i < numChildren; i++)
    {
        // int pidi = fork();
        pid[i] = fork();
        // Child proccess writes numActions times to buffer
        if (pid[i] == -1)
        {
            perror("Error with fork");
            exit(EXIT_FAILURE);
        }
        else if (pid[i] == 0)
        {
            for (int actions = 0; actions < numActions; actions++)
            {
                sem_wait(&semaphore);
                int returnNumber = rand() % numLines;
                writeToBuffer(returnNumber, sharedMemory);
                sem_post(&semaphore);
            }
            sharedMemory->child_index++;
        }
        else
        {
            // Parent process reads lineNumber from buffer one at a time and prints line
            int childIndex, actionIndex;
            for (childIndex = 0; childIndex < numChildren; childIndex++)
            {
                for (actionIndex = 0; actionIndex < numActions; actionIndex++)
                {
                    sem_wait(&semaphore);
                    int returnLine = readFromBuffer(sharedMemory, &childIndex, &actionIndex);
                    printLine(returnLine, fptr, childIndex, actionIndex);
                    sem_post(&semaphore);
                }
            }
        }
        usleep(5000);
    }

    for (int i = 0; i < numChildren; i++)
        wait(NULL);

    // for (int i = 0; i < numChildren; i++)
    //     for (int j = 0; j < numActions; j++)
    //         printf("%d\n", sharedMemory->buffer[i][j]);

    sem_destroy(&semaphore);
    shmdt(sharedMemory);
    shmctl(shm_id, IPC_RMID, 0);
    fclose(fptr);
    return 0;
}