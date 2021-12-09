#include "common.h"

int main(int argc, char **argv)
{
    printf("Client (%d) is running...\n", getpid());

    int shm_id, numActions, numberOfLines;
    sscanf(argv[1], "%d", &shm_id);
    sscanf(argv[2], "%d", &numActions);
    sscanf(argv[3], "%d", &numberOfLines);

    // Attach shared memory
    void *bla = (void *)0;
    struct shared_memory *sharedMemory;
    bla = shmat(shm_id, NULL, 0);
    if (bla == NULL)
    {
        perror("Error with shmat");
        exit(EXIT_FAILURE);
    }
    sharedMemory = (struct shared_memory *)bla;

    // Open semaphores
    sem_t *semServer, *semClientWrite, *semClientRead;

    semServer = sem_open("/sem_server", O_RDWR);
    if (semServer == SEM_FAILED)
    {
        perror("Failed to open semServer");
        exit(EXIT_FAILURE);
    }

    semClientRead = sem_open("/sem_client_read", O_RDWR);
    if (semClientRead == SEM_FAILED)
    {
        perror("Failed to open semClientRead");
        exit(EXIT_FAILURE);
    }

    semClientWrite = sem_open("/sem_client_write", O_RDWR);
    if (semClientWrite == SEM_FAILED)
    {
        perror("Failed to open semClientWrite");
        exit(EXIT_FAILURE);
    }

    srand(getpid());
    double times[numActions];
    for (int actions = 0; actions < numActions; actions++)
    {
        clock_t begin = clock();
        // Entering Critical Section 1
        if (sem_wait(semClientWrite) < 0)
        {
            perror("Failed to wait semClientWrite");
            exit(EXIT_FAILURE);
        }

        // Request line from server
        sharedMemory->requestedLine = rand() % numberOfLines + 1;
        printf("Client (%d) in Action (%d) writing to memory: %d...\n", getpid(), actions, sharedMemory->requestedLine);
        fflush(stdout);

        if (sem_post(semServer) < 0)
        {
            perror("Failed to post semServer");
            exit(EXIT_FAILURE);
        }

        // Entering Critical Section 2
        if (sem_wait(semClientRead) < 0)
        {
            perror("Failed to wait semClientRead");
            exit(EXIT_FAILURE);
        }

        printf("Client (%d) in Action (%d) printing line: %d \n%s\n", getpid(), actions,
               sharedMemory->requestedLine, sharedMemory->foundLine);
        fflush(stdout);

        // Exiting Critical Section 1
        if (sem_post(semClientWrite) < 0)
        {
            perror("Failed to post semClientWrite");
            exit(EXIT_FAILURE);
        }

        // Print transaction time
        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        times[actions] = time_spent;
        printf("Transaction lasted %f seconds\n\n", time_spent);
    }

    double avgTime = 0;
    for (int i = 0; i < numActions; i++)
        avgTime += times[i];

    // Close semaphores
    if (sem_close(semClientWrite) < 0)
    {
        perror("Failed to close semClientWrite");
        exit(EXIT_FAILURE);
    }

    if (sem_close(semClientRead) < 0)
    {
        perror("Failed to close semClientRead");
        exit(EXIT_FAILURE);
    }

    if (sem_close(semServer) < 0)
    {
        perror("Failed to close semServer");
        exit(EXIT_FAILURE);
    }

    // Detach sharedMemory
    if (shmdt(sharedMemory) == -1)
    {
        perror("Failed with shmdt");
        exit(EXIT_FAILURE);
    }

    printf("\n|------------------------------------------------------------------------|\n");
    printf("|Client (%d) terminated with average transaction time: %f seconds|\n", getpid(), avgTime / (double)numActions);
    printf("|------------------------------------------------------------------------|\n\n");

    return 0;
}