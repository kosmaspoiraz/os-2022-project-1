#include "common.h"

// Main program
int main(int argvc, char *argvv[])
{
    printf("\n\n <--- START OF PROGRAM --->\n\n");

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
    int const shm_id = shmget(MEM_KEY, sizeof(struct shared_memory), SHM_FLAGS);
    if (shm_id == -1)
    {
        perror("Error with shmget");
        exit(EXIT_FAILURE);
    }
    bla = shmat(shm_id, NULL, 0);
    if (bla == NULL)
    {
        perror("Error with shmat");
        exit(EXIT_FAILURE);
    }
    sharedMemory = (struct shared_memory *)bla;
    sharedMemory->requestedLine = 0;

    // Create semaphores
    sem_t *semChildWrite, *semParent, *semChildRead;

    semParent = sem_open("/sem_parent", SEM_PERMS, 0);
    if (semParent == SEM_FAILED)
    {
        perror("Failed to open semParent");
        exit(EXIT_FAILURE);
    }

    semChildRead = sem_open("/sem_child_read", SEM_PERMS, 0);
    if (semChildRead == SEM_FAILED)
    {
        perror("Failed to open semChildRead");
        exit(EXIT_FAILURE);
    }

    semChildWrite = sem_open("/sem_child_write", SEM_PERMS, 1);
    if (semChildWrite == SEM_FAILED)
    {
        perror("Failed to open semChildWrite");
        exit(EXIT_FAILURE);
    }

    // Initialize semaphores
    if (sem_init(semChildWrite, 1, 1) != 0) // Unlocked
    {
        perror("Failed to initialize semChildWrite");
        exit(EXIT_FAILURE);
    }
    if (sem_init(semParent, 1, 0) != 0) // Locked
    {
        perror("Failed to initialize semParent");
        exit(EXIT_FAILURE);
    }
    if (sem_init(semChildRead, 1, 0)) // Locked
    {
        perror("Failed to initialize semChildRead");
        exit(EXIT_FAILURE);
    }

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

    // Close sems as we won't be using them here
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
        execlp("./child", "./child", argv1, argv3, argv4, NULL);
    }
    else
    {
        // Parent process
        execlp("./parent", "./parent", argv1, argv2, argv3, argv4, argv5, NULL);
    }

    return 0;
}