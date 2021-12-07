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

// Function to count how many lines in File
int countLines(FILE *file)
{
    char c;
    int lines = 0;
    for (c = getc(file); c != EOF; c = getc(file))
        if (c == '\n') // Increment count if this character is newline
            lines = lines + 1;
    return lines;
}

// Main program
int main(int argvc, char *argvv[])
{
    int numLines = 0;
    int numChildren, numActions;
    const char *fname;
    fname = argvv[1];
    sscanf(argvv[2], "%d", &numChildren);
    sscanf(argvv[3], "%d", &numActions);

    int pid[numChildren];
    FILE *fptr;

    // Initialize shared memory
    void *bla = (void *)0;
    struct shared_memory *sharedMemory;
    int const shm_id = shmget(IPC_PRIVATE, sizeof(struct shared_memory), IPC_CREAT | 0666);
    if (shm_id == -1)
    {
        perror("Error with shmget");
        exit(EXIT_FAILURE);
    }
    bla = shmat(shm_id, NULL, 0);
    sharedMemory = (struct shared_memory *)bla;
    sharedMemory->requestedLine = 0;
    // sharedMemory->foundLine = malloc(sizeof(FSIZE));

    // Initialize semaphores
    sem_t *semClientWrite, *semServer;
    semServer = sem_open("/sem_server", O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 0);
    sem_t *semClientRead;
    semClientRead = sem_open("/sem_client_read", O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 0);
    semClientWrite = sem_open("/sem_client_write", O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 1);

    sem_init(semClientWrite, 1, 1); // Unlocked
    sem_init(semServer, 1, 0);      // Locked
    sem_init(semClientRead, 1, 0);  // Locked

    // Open File and find how many lines
    fptr = fopen(fname, "r");
    if (fptr == NULL)
    {
        perror("Error opening file\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        numLines = countLines(fptr);
    }
    fclose(fptr);

    char argv1[100], argv2[100], argv3[100], argv4[100], argv5[100];
    sprintf(argv1, "%d", shm_id);      // argvv[1]
    sprintf(argv2, "%d", numChildren); // argvv[2]
    sprintf(argv3, "%d", numActions);  // argvv[3]
    sprintf(argv4, "%d", numLines);    // argvv[4]
    sprintf(argv5, "%s", fname);       // argvv[5]

    sem_close(semClientWrite);
    sem_close(semServer);
    sem_close(semClientRead);

    // Create children processes
    int i;
    for (i = 0; i < numChildren; i++)
    {
        fflush(stdout);
        pid[i] = fork();
        // Child proccess writes numActions times to buffer
        if (pid[i] == -1)
        {
            perror("Error with fork");
            exit(EXIT_FAILURE);
        }
        else if (pid[i] == 0)
        {
            break;
        }
    }
    if (pid[i] == 0)
    {
        execlp("./client", "./client", argv1, argv3, argv4, NULL);
    }
    else
    {
        // Parent process waits requests one at a time and responds
        execlp("./server", "./server", argv1, argv2, argv3, argv4, argv5, NULL);
    }

    // Wait for everyone to finish
    while (wait(NULL) > 0)
        ;

    // Free and unlink semaphores

    sem_unlink("/sem_client_read");
    sem_unlink("/sem_client_write");
    sem_unlink("/sem_server");

    // Detach and free memory
    shmdt(sharedMemory);
    shmctl(shm_id, IPC_RMID, NULL);
    printf("Bla\n");

    return 0;
}