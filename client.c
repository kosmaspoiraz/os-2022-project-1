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
#define fname "sample.txt"

// Shared memory struct
struct shared_memory
{
    char foundLine[FSIZE];
    int requestedLine;
};

int main(int argc, char **argv)
{
    printf("Client (%d) is running...\n", getpid());

    int shm_id, numActions, numberOfLines;
    sscanf(argv[1], "%d", &shm_id);
    sscanf(argv[2], "%d", &numActions);
    sscanf(argv[3], "%d", &numberOfLines);

    // Initialize shared memory
    void *bla = (void *)0;
    struct shared_memory *sharedMemory;
    bla = shmat(shm_id, NULL, 0);
    sharedMemory = (struct shared_memory *)bla;

    // Initialize semaphores
    sem_t *semServer, *semClientWrite;
    semServer = sem_open("/sem_server", O_RDWR);
    sem_t *semClientRead;
    semClientRead = sem_open("/sem_client_read", O_RDWR);
    semClientWrite = sem_open("/sem_client_write", O_RDWR);

    srand(getpid());
    for (int actions = 0; actions < numActions; actions++)
    {
        // Entering Critical Section 1
        sem_wait(semClientWrite);

        // Request line from server
        sharedMemory->requestedLine = rand() % numberOfLines + 1;
        printf("Client (%d) in Action (%d) writing to memory: %d...\n", getpid(), actions, sharedMemory->requestedLine);
        fflush(stdout);

        // Exiting Critical Section 1
        sem_post(semServer);

        // Entering Critical Section 2
        sem_wait(semClientRead);

        printf("Client (%d) in Action (%d) printing line: %d \n%s\n", getpid(), actions,
               sharedMemory->requestedLine, sharedMemory->foundLine);
        fflush(stdout);

        // Exiting Critical Section 2
        sem_post(semClientWrite);
    }

    sem_close(semClientWrite);
    sem_close(semServer);
    sem_close(semClientRead);

    shmdt(sharedMemory);
    return 0;
}