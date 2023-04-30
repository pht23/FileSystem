#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "createfs.h"
#include "tools.h"
#include "openfs.h"

#define MAX_PATH_SIZE 255

FileSystem *fs = NULL;

void openfs(char *path)
{
    /*
    if (fs != NULL)
    {
        printf("A file system is already open\n");
        return;
    }
    */

    // char *path = (char *) malloc(MAX_PATH_SIZE * sizeof(char));
    FILE *fp = fopen(path, "rb");
    if (!fp)
    {
        printf("File not found\n");
        return;
    }
    else if (fp)
    {
        printf("Opening file: %s\n", path);
    }

    fs = (FileSystem*) malloc(sizeof(FileSystem));
    fread(fs, sizeof(FileSystem), 1, fp);
    fclose(fp);

    printf("Opened file system %s\n", path);
}