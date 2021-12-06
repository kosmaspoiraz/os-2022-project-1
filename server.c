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
    char line[1024];

    while (fgets(line, sizeof(line), file) != NULL)
    {
        line[strlen(line) - 1] = '\0';
        if (count == *lineNum)
        {
            printf("Server returned line number %d \n", sharedMemory->requestedLine);
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
    void *bla = (void *)0;
    struct shared_memory *sharedMemory;
    int shm_id;
    sscanf(argv[1], "%d", &shm_id);
    bla = shmat(shm_id, NULL, 0);
    sharedMemory = (struct shared_memory *)bla;

    // Initialize semaphores
    sem_t *semServer, *semClientWrite;
    semServer = sem_open("/sem_server", O_RDWR);
    sem_t *semClientRead;
    semClientRead = sem_open("/sem_client_read", O_RDWR);
    semClientWrite = sem_open("/sem_client_write", O_RDWR);

    int i = 0;
    while (i < numChildren * numActions)
    {
        // Entering Critical Section
        sem_wait(semServer);
        printf("Server entered Critical Section \n");

        printf("Server read from memory: %d\n", sharedMemory->requestedLine);
        findLine(&sharedMemory->requestedLine, &sharedMemory->numberOfLines, sharedMemory);
        i++;

        // Exiting Critical Section
        sem_post(semClientRead);
    }

    sem_close(semClientWrite);
    sem_close(semServer);
    sem_close(semClientRead);

    // free(sharedMemory->foundLine);
    shmdt(sharedMemory);
    return 0;
}