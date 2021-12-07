#include "common.h"

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
    sem_t *semServer, *semClientRead;

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

    int i = 0;
    while (i < numChildren * numActions)
    {
        // Entering Critical Section
        if (sem_wait(semServer) < 0)
        {
            perror("Failed to wait semServer");
            exit(EXIT_FAILURE);
        }

        printf("Server read from memory: %d\n", sharedMemory->requestedLine);
        findLine(&sharedMemory->requestedLine, &numberOfLines, sharedMemory, fname);
        i++;

        // Exiting Critical Section
        fflush(stdout);

        if (sem_post(semClientRead) < 0)
        {
            perror("Failed to post semClientRead");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for everyone to finish
    while (wait(NULL) > 0)
        ;

    // Close semaphores
    if (sem_close(semClientWrite) < 0)
    {
        perror("Failed to close semClientWrite");
        exit(EXIT_FAILURE);
    }

    if (sem_close(semClienRead) < 0)
    {
        perror("Failed to close semClientRead");
        exit(EXIT_FAILURE);
    }

    if (sem_close(semServer) < 0)
    {
        perror("Failed to close semServer");
        exit(EXIT_FAILURE);
    }

    // Unlink semaphores
    if (sem_unlink("/sem_client_read") < 0)
    {
        perror("Failed to unlink semClientRead");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink("/sem_client_write") < 0)
    {
        perror("Failed to unlink semClientWrite");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink("/sem_server") < 0)
    {
        perror("Failed to unlink semServer");
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