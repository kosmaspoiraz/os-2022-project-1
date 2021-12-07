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

#define FSIZE 1024

// Shared memory struct
struct shared_memory
{
    char foundLine[FSIZE];
    int requestedLine;
};

// Function to print requested line number
void findLine(int *lineNum, int *numLines, struct shared_memory *sharedMemory, char *fname)
{
    FILE *file = fopen(fname, "r");
    if (file == NULL)
    {
        perror("Error opening file\n");
        exit(EXIT_FAILURE);
    }
    int count = 1;
    char line[FSIZE];

    while (fgets(line, sizeof(line), file) != NULL)
    {
        line[strlen(line) - 1] = '\0';
        if (count == *lineNum)
        {
            printf("Server returned line number %d \n", sharedMemory->requestedLine);
            // sharedMemory->foundLine = line;
            strcpy(sharedMemory->foundLine, line);
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

    int shm_id, numChildren, numActions, numberOfLines;
    char *fname;
    sscanf(argv[1], "%d", &shm_id);
    sscanf(argv[2], "%d", &numChildren);
    sscanf(argv[3], "%d", &numActions);
    sscanf(argv[4], "%d", &numberOfLines);
    fname = argv[5];

    printf("Server: %s, %d , %d\n", fname, numChildren, numActions);

    // Initialize shared memory
    void *bla = (void *)0;
    struct shared_memory *sharedMemory;
    bla = shmat(shm_id, NULL, 0);
    sharedMemory = (struct shared_memory *)bla;

    // Initialize semaphores
    sem_t *semServer;
    semServer = sem_open("/sem_server", O_RDWR);
    sem_t *semClientRead;
    semClientRead = sem_open("/sem_client_read", O_RDWR);

    int i = 0;
    while (i < numChildren * numActions)
    {
        // Entering Critical Section
        sem_wait(semServer);

        printf("Server read from memory: %d\n", sharedMemory->requestedLine);
        findLine(&sharedMemory->requestedLine, &numberOfLines, sharedMemory, fname);
        i++;

        // Exiting Critical Section
        fflush(stdout);
        sem_post(semClientRead);
    }

    sem_close(semServer);
    sem_close(semClientRead);

    shmdt(sharedMemory);
    return 0;
}