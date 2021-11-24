#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/types.h>

#define FSIZE 100
#define numChildren 2
#define numActions 4
#define fname "sample.txt"
#define MAXSIZE numChildren *numActions

// Shared memory struct containg buffer[numChildren][numActions]
struct shared_memory
{
    char buffer[numChildren][numActions];
    int child_index;
    int action_index;
    int size;
};

void writeToBuffer(int numLine, struct shared_memory *sharedMemory)
{
    sharedMemory->buffer[sharedMemory->child_index][sharedMemory->action_index] = numLine;
    sharedMemory->size++;
    sharedMemory->action_index++;
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
    int lines = 0;
    for (c = getc(file); c != EOF; c = getc(file))
        if (c == '\n') // Increment count if this character is newline
            lines = lines + 1;
    return lines;
}

// Function to print requested line number
void printLine(int *lineNum, int *numLines, int *childIndex, int *actionIndex, struct shared_memory *sharedMemory)
{
    FILE *file = fopen(fname, "r");
    if (file == NULL)
    {
        perror("Error opening file\n");
        exit(EXIT_FAILURE);
    }
    int count = 1;
    int child = *childIndex;
    int action = *actionIndex;
    char line[FSIZE * (*numLines)];

    while (fgets(line, sizeof(line), file) != NULL)
    {
        line[strlen(line) - 1] = '\0';
        if (count == *lineNum)
        {
            printf("Child (%d) in action (%d) requested Line Number %d: %s\n", child, action, sharedMemory->buffer[child][action], line);
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
    sharedMemory->action_index = sharedMemory->child_index = sharedMemory->size = 0;

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
                if (sharedMemory->size <= MAXSIZE)
                {
                    sem_wait(&semaphore);
                    int returnNumber = rand() % numLines + 1;
                    printf("Child (%d) in Action (%d) writing to memory...\n", sharedMemory->child_index, sharedMemory->action_index);
                    printf("Child (%d) in Action (%d) wrote to memory: %d\n", sharedMemory->child_index, sharedMemory->action_index, returnNumber);
                    writeToBuffer(returnNumber, sharedMemory);
                    sem_post(&semaphore);
                    sleep(5);
                }
                else
                {
                    printf("No more space in sharedMemory\n");
                    break;
                }
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
                    printf("Server trying to read from memory...\n");
                    if (sharedMemory->size <= 0)
                    {
                        printf("Nothing yet to read. Waiting...\n");
                    }
                    else
                    {
                        int *returnLine = malloc(sizeof(int));
                        *returnLine = readFromBuffer(sharedMemory, &childIndex, &actionIndex);
                        if (*returnLine == 0)
                        {
                            printf("Nothing yet to read. Waiting...\n");
                            free(returnLine);
                            continue;
                        }
                        printf("Server read from memory: %d...\n", *(int *)returnLine);
                        printLine(returnLine, &numLines, &childIndex, &actionIndex, sharedMemory);
                        free(returnLine);
                    }
                    sem_post(&semaphore);
                    sleep(1);
                }
            }
        }
    }

    for (int i = 0; i < numChildren; i++)
        wait(NULL);

    sem_destroy(&semaphore);
    shmdt(sharedMemory);
    shmctl(shm_id, IPC_RMID, NULL);
    fclose(fptr);
    return 0;
}