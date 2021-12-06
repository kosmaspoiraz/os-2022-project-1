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
#define numChildren 2
#define numActions 2
#define fname "sample.txt"
#define MAXSIZE numChildren *numActions

// Shared memory struct containg buffer[numChildren][numActions]
struct shared_memory
{
    char *foundLine;
    int *numberOfLines;
    int *requestedLine;
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
            printf("Server returned line number %d \n", *(int *)sharedMemory->requestedLine);
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
    void *bla = (void *)0;
    int shm_id;
    sscanf(argv[1], "%d", &shm_id);
    bla = shmat(shm_id, (void *)0, 0);
    sharedMemory = (struct shared_memory *)bla;

    // Initialize semaphores
    sem_t *semClientRead, *semServer;
    semServer = sem_open("/sem_server", O_RDWR);
    semClientRead = sem_open("/sem_ClientRead", O_RDWR);
    // semClientWrite = sem_open("/sem_ClientWrite", O_RDWR);

    int i = 0;
    while (i < numChildren * numActions)
    {
        printf("Server -- Wait on semServer\n");
        sem_wait(semServer);

        printf("Server read from memory: %d\n", *(int *)sharedMemory->requestedLine);
        findLine(sharedMemory->requestedLine, sharedMemory->numberOfLines, sharedMemory);

        printf("%s\n", sharedMemory->foundLine);

        sem_post(semClientRead);
        printf("Server -- Post on semClientRead\n");

        sem_wait(semServer);
        printf("Server -- Wait on semServer\n");

        i++;
    }

    // free(sharedMemory->foundLine);
    shmdt(sharedMemory);
    return 0;
}