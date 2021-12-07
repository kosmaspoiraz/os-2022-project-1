#include "common.h"

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

// Function to print requested line number
void findLine(int *lineNum, int *numLines, struct shared_memory *sharedMemory, char *fname)
{
    FILE *file = fopen(fname, "r");
    if (file == NULL)
    {
        perror("Error opening file\n");
        exit(EXIT_FAILURE);
    }
    int count = 1;
    char line[FSIZE];

    while (fgets(line, sizeof(line), file) != NULL)
    {
        line[strlen(line) - 1] = '\0';
        if (count == *lineNum)
        {
            printf("Server returned line number %d \n", sharedMemory->requestedLine);
            // sharedMemory->foundLine = line;
            strcpy(sharedMemory->foundLine, line);
            fclose(file);
            return;
        }
        else
        {
            count++;
        }
    }
}