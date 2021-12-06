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
#define numClientren 2
#define numActions 2
#define fname "sample.txt"
#define MAXSIZE numClientren *numActions

// Shared memory struct containg buffer[numClientren][numActions]
struct shared_memory
{
    char *foundLine;
    int numberOfLines;
    int requestedLine;
};

int main(int argc, char **argv)
{
    printf("Client (%d) is running...\n", getpid());

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

    srand(getpid());
    for (int actions = 0; actions < numActions; actions++)
    {
        // Entering Critical Section 1
        sem_wait(semClientWrite);
        printf("(%d) entered Critical Section 1\n", getpid());
        fflush(stdout);

        // Request line from server
        printf("%p\n", sharedMemory->foundLine);
        printf("%d\n", sharedMemory->requestedLine);
        printf("%d\n", sharedMemory->numberOfLines);

        sharedMemory->requestedLine = rand() % sharedMemory->numberOfLines + 1;
        printf("Client (%d) in Action (%d) writing to memory: %d...\n", getpid(), actions, sharedMemory->requestedLine);
        fflush(stdout);

        // Exiting Critical Section 1
        sem_post(semServer);

        // Entering Critical Section 2
        sem_wait(semClientRead);
        printf("%s", sharedMemory->foundLine);

        // Exiting Critical Section 2
        sem_post(semClientWrite);
    }

    sem_close(semClientWrite);
    sem_close(semServer);
    sem_close(semClientRead);

    shmdt(sharedMemory);
    return 0;
}