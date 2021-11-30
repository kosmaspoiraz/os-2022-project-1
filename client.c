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

int main(int argc, char **argv)
{
    printf("Client is running...\n");

    // Initialize shared memory
    struct shared_memory *sharedMemory;
    // int shm_id = shmget(IPC_PRIVATE, sizeof(struct shared_memory), 0666);
    // if (shm_id == -1)
    // {
    //     perror("Error with shmget");
    //     exit(EXIT_FAILURE);
    // }
    int shm_id;
    sscanf(argv[1], " % d", &shm_id);
    sharedMemory = shmat(shm_id, NULL, 0);

    // Initialize semaphores
    sem_t *semClientRead, *semClientWrite, *semServer;
    semServer = sem_open("Server", O_RDWR);
    semClientRead = sem_open("ClientRead", O_RDWR);
    semClientWrite = sem_open("ClientWrite", O_RDWR);

    srand(getpid());
    for (int actions = 0; actions < numActions; actions++)
    {
        // Lock semClientWrite
        sem_wait(semClientWrite);
        printf("Locked semClientWrite\n");

        // Request line from server
        printf("Child (%d) in Action (%d) writing to memory...\n", getpid(), actions);
        sharedMemory->requestedLine = rand() % sharedMemory->numberOfLines + 1;
        printf("Child (%d) in Action (%d) wrote to memory: %d\n", getpid(), actions, sharedMemory->requestedLine);

        // Send Server signal to respond
        // Unlock semServer
        sem_post(semServer);
        printf("UnLocked semServer\n");

        // Receive signal from Server that he responded
        // Lock semClientRead
        sem_wait(semClientRead);
        printf("Locked semClientRead\n");

        // Print line
        printf("Child (%d) in Action (%d) requested line number (%d): /n%s\n", getpid(), actions, sharedMemory->requestedLine, sharedMemory->foundLine);

        // Unlock
        // sem_post(semClientRead);
        // printf("UnLocked semClientRead\n");
        // sem_post(semClientWrite);
        // printf("UnLocked semClientWrite\n");
    }

    shmdt(sharedMemory);
    return 0;
}