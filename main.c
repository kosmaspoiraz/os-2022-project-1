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
    int pid[numChildren];
    FILE *fptr;

    // Initialize shared memory
    struct shared_memory *sharedMemory;
    void *bla = (void *)0;
    int const shm_id = shmget((key_t)1996, sizeof(struct shared_memory), IPC_CREAT | 0666);
    if (shm_id == -1)
    {
        perror("Error with shmget");
        exit(EXIT_FAILURE);
    }
    bla = shmat(shm_id, (void *)0, 0);
    sharedMemory = (struct shared_memory *)bla;

    // Initialize semaphores
    sem_t *semClientRead, *semClientWrite, *semServer;
    semServer = sem_open("/sem_server", O_CREAT, 0660, 0);           // LOCKED
    semClientRead = sem_open("/sem_ClientRead", O_CREAT, 0660, 0);   // LOCKED
    semClientWrite = sem_open("/sem_ClientWrite", O_CREAT, 0660, 1); // UNLOCKED

    sem_init(semServer, 1, 0);      // LOCKED
    sem_init(semClientRead, 1, 0);  // LOCKED
    sem_init(semClientWrite, 1, 1); // UNLOCKED

    // Open File and find how many lines
    fptr = fopen(fname, "r");
    if (fptr == NULL)
    {
        perror("Error opening file\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        int lines = countLines(fptr);
        sharedMemory->numberOfLines = malloc(sizeof(int));
        sharedMemory->numberOfLines = &lines;
    }
    fclose(fptr);

    sharedMemory->foundLine = malloc(sizeof(FSIZE * (*(int *)sharedMemory->numberOfLines)));
    sharedMemory->requestedLine = malloc(sizeof(int));
    *(int *)sharedMemory->requestedLine = 0;

    char str[100];
    sprintf(str, "%d", shm_id);

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
        fflush(stdout);
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
    sem_close(semClientRead);
    sem_close(semClientWrite);
    sem_close(semServer);
    sem_unlink("ClientRead");
    sem_unlink("ClientWrite");
    sem_unlink("Server");

    // Detach and free memory
    free(sharedMemory->foundLine);
    shmdt(sharedMemory);
    shmctl(shm_id, IPC_RMID, NULL);
    printf("Bla\n");

    return 0;
}