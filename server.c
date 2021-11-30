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
    char *foundLine;
    int numberOfLines;
    int requestedLine;
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
            sharedMemory->foundLine = 0;
            printf("Server returned line number %d \n", sharedMemory->requestedLine);
            // sharedMemory->foundLine = malloc(sizeof(FSIZE * (*numLines)));
            sharedMemory->foundLine = line;
            fclose(file);
            return;
        }
        else
        {
            count++;
        }
    }
}

int main(int argc, char **argv)
{
    printf("Server is running...\n");

    // Initialize shared memory
    struct shared_memory *sharedMemory;
    int shm_id;
    sscanf(argv[1], "%d", &shm_id);
    sharedMemory = shmat(shm_id, NULL, 0);

    // Initialize semaphores
    sem_t *semClientRead, *semClientWrite, *semServer;
    semServer = sem_open("Server", O_RDWR);
    semClientRead = sem_open("ClientRead", O_RDWR);
    semClientWrite = sem_open("ClientWrite", O_RDWR);

    int i = 0;
    while (i < numChildren * numActions)
    {
        // Lock semServer
        sem_wait(semServer);
        printf("Locked semServer\n");

        printf("Server read from memory: %d\n", sharedMemory->requestedLine);
        findLine(&sharedMemory->requestedLine, &sharedMemory->numberOfLines, sharedMemory);

        // Unlock semClientRead
        sem_post(semClientRead);
        printf("UnLocked semClientRead\n");

        // Unlock semClientWrite
        sem_post(semClientWrite);
        printf("UnLocked semClientWrite\n");
    }

    free(sharedMemory->foundLine);
    shmdt(sharedMemory);
    return 0;
}