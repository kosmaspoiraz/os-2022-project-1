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
int main(int argc, char *argv[])
{
    int numLines = 0;
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
    sharedMemory->numberOfLines = numLines;
    // sharedMemory->foundLine = malloc(sizeof(FSIZE * numLines));

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
        sharedMemory->numberOfLines = countLines(fptr);
    }
    fclose(fptr);

    char str[100];
    sprintf(str, "%d", shm_id);

    sem_close(semClientWrite);
    sem_close(semServer);
    sem_close(semClientRead);

    // Create children processes
    int i;
    for (i = 0; i < numChildren; i++)
    {
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
        execlp("./client", "./client", str, NULL);
    }
    else
    {
        // Parent process waits requests one at a time and responds
        execlp("./server", "./server", str, NULL);
    }

    // Wait for everyone to finish
    while (wait(NULL) > 0)
        ;

    // Free and unlink semaphores

    sem_unlink("/ClientRead");
    sem_unlink("/ClientWrite");
    sem_unlink("/Server");

    // Detach and free memory
    // free(sharedMemory->foundLine);
    shmdt(sharedMemory);
    shmctl(shm_id, IPC_RMID, NULL);
    printf("Bla\n");

    return 0;
}