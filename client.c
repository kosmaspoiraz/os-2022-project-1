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
    char *buffer;
    int lineNum;
};

int main()
{
    printf("Client is running...\n");
    return 0;
}