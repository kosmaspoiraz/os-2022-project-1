#include "common.h"

int main(int argc, char **argv)
{
    printf("Parent is running...\n");

    int shm_id, numChildren, numActions, numberOfLines;
    char *fname;
    sscanf(argv[1], "%d", &shm_id);
    sscanf(argv[2], "%d", &numChildren);
    sscanf(argv[3], "%d", &numActions);
    sscanf(argv[4], "%d", &numberOfLines);
    fname = argv[5];

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
    sem_t *semParent, *semChildRead, *semChildWrite;

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

    int i = 0;
    while (i < numChildren * numActions)
    {
        // Entering Critical Section
        if (sem_wait(semParent) < 0)
        {
            perror("Failed to wait semParent");
            exit(EXIT_FAILURE);
        }

        printf("Parent read from memory: %d\n", sharedMemory->requestedLine);
        findLine(&sharedMemory->requestedLine, &numberOfLines, sharedMemory, fname);
        i++;

        fflush(stdout);
        // Exiting Critical Section 2
        if (sem_post(semChildRead) < 0)
        {
            perror("Failed to post semChildRead");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for everyone to finish
    while (wait(NULL) > 0)
        ;

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

    // Unlink semaphores
    if (sem_unlink("/sem_child_read") < 0)
    {
        perror("Failed to unlink semChildRead");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink("/sem_child_write") < 0)
    {
        perror("Failed to unlink semChildWrite");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink("/sem_parent") < 0)
    {
        perror("Failed to unlink semParent");
        exit(EXIT_FAILURE);
    }

    // Detach sharedMemory
    if (shmdt(sharedMemory) == -1)
    {
        perror("Failed with shmdt");
        exit(EXIT_FAILURE);
    }

    // Remove sharedMemory
    if (shmctl(shm_id, IPC_RMID, NULL) == -1)
    {
        perror("Failed with shmctl");
        exit(EXIT_FAILURE);
    }

    printf("\n<--- END OF PROGRAM --->\n\n");

    return 0;
}