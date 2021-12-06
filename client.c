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

int main(int argc, char **argv)
{
    printf("Client is running...\n");

    // Initialize shared memory
    struct shared_memory *sharedMemory;
    void *bla = (void *)0;
    int shm_id;
    sscanf(argv[1], "%d", &shm_id);
    bla = shmat(shm_id, (void *)0, 0);
    sharedMemory = (struct shared_memory *)bla;

    // Initialize semaphores
    sem_t *semClientRead, *semClientWrite, *semServer;
    semServer = sem_open("/sem_server", O_RDWR);
    semClientRead = sem_open("/sem_ClientRead", O_RDWR);
    semClientWrite = sem_open("/sem_ClientWrite", O_RDWR);

    printf("%d\n", *(int *)sharedMemory->numberOfLines);

    // srand(getpid());
    for (int actions = 0; actions < numActions; actions++)
    {
        printf("(%d) -- Wait on semClientWrite\n", getpid());
        sem_wait(semClientWrite);
        // Request line from server
        printf("%p\n", sharedMemory->foundLine);
        printf("%d\n", *((int *)sharedMemory->requestedLine));
        printf("%d\n", *((int *)sharedMemory->numberOfLines));
        printf("%p\n", sharedMemory->requestedLine);
        printf("%p\n", sharedMemory->numberOfLines);

        printf("\n%p\n", sharedMemory->foundLine);
        printf("%p = %d\n", sharedMemory->requestedLine, *(int *)sharedMemory->requestedLine);
        printf("%p = %d\n", sharedMemory->numberOfLines, *(int *)sharedMemory->numberOfLines);

        int lines = *(int *)sharedMemory->numberOfLines;
        int random = rand() % lines + 1;
        sharedMemory->requestedLine = &random;
        printf("Child (%d) in Action (%d) writing to memory: %d \n", getpid(), actions, *(int *)sharedMemory->requestedLine);

        // Send Server signal to respond
        sem_post(semServer);
        printf("(%d) -- Post on semServer\n", getpid());
        sem_wait(semClientRead);
        printf("(%d) -- Wait on semClientRead\n", getpid());

        // Receive signal from Server that he responded
        sem_wait(semClientRead);
        printf("(%d) -- Wait on semClientRead\n", getpid());

        // Print line
        printf("Child (%d) in Action (%d) requested line number (%d): \n<- %s ->\n", getpid(), actions, *(int *)sharedMemory->requestedLine, sharedMemory->foundLine);
    }

    shmdt(sharedMemory);
    return 0;
}