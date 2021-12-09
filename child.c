#include "common.h"

int main(int argc, char **argv)
{
    printf("Child (%d) is running...\n", getpid());

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
    sem_t *semParent, *semChildWrite, *semChildRead;

    semParent = sem_open("/sem_parent", O_RDWR);
    if (semParent == SEM_FAILED)
    {
        perror("Failed to open semParent");
        exit(EXIT_FAILURE);
    }

    semChildRead = sem_open("/sem_child_read", O_RDWR);
    if (semChildRead == SEM_FAILED)
    {
        perror("Failed to open semChildRead");
        exit(EXIT_FAILURE);
    }

    semChildWrite = sem_open("/sem_child_write", O_RDWR);
    if (semChildWrite == SEM_FAILED)
    {
        perror("Failed to open semChildWrite");
        exit(EXIT_FAILURE);
    }

    srand(getpid());
    double times[numActions];
    for (int actions = 0; actions < numActions; actions++)
    {
        clock_t begin = clock();
        // Entering Critical Section 1
        if (sem_wait(semChildWrite) < 0)
        {
            perror("Failed to wait semChildWrite");
            exit(EXIT_FAILURE);
        }

        // Request line from Parent
        sharedMemory->requestedLine = rand() % numberOfLines + 1;
        printf("Child (%d) in Action (%d) writing to memory: %d...\n", getpid(), actions, sharedMemory->requestedLine);
        fflush(stdout);

        if (sem_post(semParent) < 0)
        {
            perror("Failed to post semParent");
            exit(EXIT_FAILURE);
        }

        // Entering Critical Section 2
        if (sem_wait(semChildRead) < 0)
        {
            perror("Failed to wait semChildRead");
            exit(EXIT_FAILURE);
        }

        printf("Child (%d) in Action (%d) printing line: %d \n%s\n", getpid(), actions,
               sharedMemory->requestedLine, sharedMemory->foundLine);
        fflush(stdout);

        // Exiting Critical Section 1
        if (sem_post(semChildWrite) < 0)
        {
            perror("Failed to post semChildWrite");
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
    if (sem_close(semChildWrite) < 0)
    {
        perror("Failed to close semChildWrite");
        exit(EXIT_FAILURE);
    }

    if (sem_close(semChildRead) < 0)
    {
        perror("Failed to close semChildRead");
        exit(EXIT_FAILURE);
    }

    if (sem_close(semParent) < 0)
    {
        perror("Failed to close semParent");
        exit(EXIT_FAILURE);
    }

    // Detach sharedMemory
    if (shmdt(sharedMemory) == -1)
    {
        perror("Failed with shmdt");
        exit(EXIT_FAILURE);
    }

    printf("\n|-----------------------------------------------------------------------|\n");
    printf("|Child (%d) terminated with average transaction time: %f seconds|\n", getpid(), avgTime / (double)numActions);
    printf("|-----------------------------------------------------------------------|\n\n");

    return 0;
}