#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define FSIZE 100

// Function to count how many lines in File
int countLines(FILE *file)
{
    char c;
    int lines;
    for (c = getc(file); c != EOF; c = getc(file))
        if (c == '\n') // Increment count if this character is newline
            lines = lines + 1;
    fclose(file);
    return lines;
}

// Function to print requested line of File
void printLine(int lineNum, FILE *file)
{
    char line[100];
    int count = 0;

    while (getline(line, sizeof(line), file) != NULL)
    {
        if (count == lineNum)
        {
            printf("%s\n", line);
            break;
        }
        else
        {
            count++;
        }
    }
}

int main(int argc, char *argv[])
{
    int numChildren = 4;
    int numActions = 10;
    int numLines = 0;
    FILE *fptr;
    char fname = "K22_CW1_2021-2022.docx";

    // Open File and find how many lines
    fptr = fopen(fname, "r");
    if (fptr == NULL)
    {
        printf("Error opening file\n");
    }
    else
    {
        numLines = countLines(fptr);
    }

    // Open pipe for communication between Parent Process and Children Processes
    int fd[2];
    if (pipe(fd) == -1)
    {
        printf("Error opening the pipe\n");
        return 1;
    }

    // Create children processes
    srand(NULL);
    for (int i = 0; i < numChildren; i++)
    {
        if (fork() == 0)
        {
            // Use pipe to return random line number from child to parent
            close(fd[0]);
            int returnNumber = rand() % numLines;
            if (write(fd[1], &returnNumber, sizeof(int)) == -1)
            {
                printf("Error writing to pipe\n");
                return 2;
            }
            else
            {
                close(fd[1]);
            }
        }
        else
        {
            // Use pipe to read random line number from child and print that line
            close(fd[1]);
            int returnLine;
            read(fd[0], &returnLine, sizeof(int));
            close(fd[0]);
            printLine(returnLine, fptr);
            wait(NULL);
        }
    }

    fclose(fptr);
    return 0;
}