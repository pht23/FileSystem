#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>

#include "tools.h"

#define BLOCK_SIZE 1024
#define NUM_BLOCKS 65536
#define BLOCK_PER_FILE 1024
#define NUM_FILES 256
#define FIRST_DATA_BLOCK 790
#define MAX_FILE_SIZE 1048576

#define WHITESPACE " \t\n"

uint8_t data[NUM_BLOCKS][BLOCK_SIZE];

// 512 blocks just for free block map
uint8_t *free_blocks;
uint8_t *free_inodes;


// directory
struct directoryEntry
{
    char     filename[64];
    short    in_use;
    int32_t  inode;
};


struct directoryEntry *directory;

// inode
struct inode 
{
    int32_t blocks[BLOCK_PER_FILE];
    short in_use;
    uint8_t attribute;
    uint32_t file_size;
};

struct inode *inodes;

FILE *fp;
char image_name[64];
uint8_t image_open;

#define WHITESPACE " \t\n"

#define MAX_COMMAND_SIZE 255

#define MAX_NUM_ARGUMENTS 5

int32_t findFreeBlock()
{
    int i;
    for ( i = 0; i < NUM_BLOCKS; i++ )
    {
        if ( free_blocks[i] )
        {
            return i + 790;
        }
    }
    return -1;
}

int32_t findFreeInode()
{
    int i;
    for ( i = 0; i < NUM_FILES; i++)
    {
        if ( free_inodes[i] )
        {
            return i;
        }
    }
    return -1;
}

/*
|-------------------------------------------------------------------------|
    init()
|-------------------------------------------------------------------------|
*/

void init()
{
    directory = (struct directoryEntry*)&data[0][0];
    inodes = (struct inode*)&data[20][0];
    free_blocks = (uint8_t *)&data[277][0];
    free_inodes = (uint8_t *)&data[19][0];

    memset(image_name, 0, 64);
    image_open = 0;

    for(int i = 0; i < NUM_FILES; i++)
    {
        directory[i].in_use = 0;
        directory[i].inode = -1;
        free_inodes[i] = 1;
        memset(directory[i].filename, 0, 64);

        for (int j = 0; j < NUM_BLOCKS; j++)
        {
            inodes[i].blocks[j] = -1;
            inodes[i].in_use = 0;
            inodes[i].attribute = 0;
            inodes[i].file_size = 0;

        }
    }
    for (int j = 0; j < NUM_BLOCKS; j++)
    {
        free_blocks[j] = 1;
    }
}



/*
|-------------------------------------------------------------------------|
    df()
|-------------------------------------------------------------------------|
*/


uint32_t df()
{
    int j;
    int count = 0;
    for (j = FIRST_DATA_BLOCK; j < NUM_BLOCKS; j++)
    {
        if (free_blocks[j])
        {
            count++;
        }
    }
    return (count * BLOCK_SIZE);
}



/*
|-------------------------------------------------------------------------|
    createfs()
|-------------------------------------------------------------------------|
*/


void createfs(char *filename)
{
    fp = fopen(filename, "w");
    strncpy(image_name, filename, strlen(filename));

    memset(data, 0, NUM_BLOCKS * BLOCK_SIZE);
    image_open = 1;

    fclose(fp);
}



/*
|-------------------------------------------------------------------------|
    savefs()
|-------------------------------------------------------------------------|
*/


void savefs()
{

    if (image_open == 0)
    {
        printf("ERROR: Disk image is not open\n");
    }
    fp = fopen(image_name, "w");

    fwrite(&data[0][0], BLOCK_SIZE, NUM_BLOCKS, fp);

    memset(image_name, 0, 64);

    fclose(fp);
}



/*
|-------------------------------------------------------------------------|
    openfs()
|-------------------------------------------------------------------------|
*/


void openfs(char *filename)
{
    fp = fopen(filename, "w");

    strncpy(image_name, filename, strlen(filename));

    fread(&data[0][0], BLOCK_SIZE, NUM_BLOCKS, fp);

    image_open = 1;

    fclose(fp);
}



/*
|-------------------------------------------------------------------------|
    closefs()
|-------------------------------------------------------------------------|
*/

void closefs()
{
    if (image_open == 0)
    {
        printf("ERROR: Disk image is not open\n");
        return;
    }

    fclose(fp);
    image_open = 0;
    memset(image_name, 0, 64);
}



/*
|-------------------------------------------------------------------------|
    list()
|-------------------------------------------------------------------------|
*/

void list ()
{
    int i;
    int not_found = 1;

    for (i = 0; i < NUM_FILES; i++)
    {
        // TODO add a check to not list if the file is hidden
        if (directory[i].in_use)
        {
            not_found = 0;
            char filename[65];
            memset(filename, 0, 65);
            strncpy(filename, directory[i].filename, strlen(directory[i].filename));
            printf("%s\n", filename);
        }
    }

    if (not_found)
    {
        printf("ERROR: No files found.\n");
    }
}

/*
|-------------------------------------------------------------------------|
    insert()
|-------------------------------------------------------------------------|
*/

void insert(char *filename)
{
    // verify filename isn't null
    if (filename == NULL)
    {
        printf("ERROR: Filename is NULL\n");
        return;
    }
    // verify the file exists
    struct stat buf;
    int ret = stat(filename, &buf);

    if (ret == -1)
    {
        printf("ERROR: File does not exist.\n");
        return;
    }

    // verify the file isn't too big
    if (buf.st_size > MAX_FILE_SIZE)
    {
        printf("ERROR: File is too large.\n");
        return;
    }

    // verify theres engouh space
    if (buf.st_size > df())
    {
        printf("ERROR: Not enough free disk space.\n");
        return;
    }
    // find empty directory entry
    int i;
    for (i = )

    // find free inodes and place the file
}




/*
|-------------------------------------------------------------------------|
    main()
|-------------------------------------------------------------------------|
*/

int main ()
{
    char *command = (char*)malloc(MAX_CMD_SIZE);
    char *token[MAX_NUM_ARGS];

    fp = NULL;
    init();

    while(1)
    {
        printf("mfs> ");
        read_command(&command);
        parser(token, command);

        // process the filesystem commands
        if (strcmp("createfs", token[0]) == 0)
        {
            if (token[1] == NULL)
            {
                printf("ERROR: no filename specified\n");
                continue;
            }
            createfs (token[1]);
        }

        if (strcmp("savefs", token[0]) == 0)
        {
            savefs();
        }

        if (strcmp("open", token[0]) == 0)
        {
            if (token[1] == NULL)
            {
                printf("ERROR: No filename specified\n");
                continue;
            }
            openfs(token[1]);
        }

        if (strcmp("close", token[0]) == 0)
        {
            closefs();
        }

        if (strcmp("list", token[0]) == 0)
        {
            if (!image_open)
            {
                printf("ERROR: Disk image is not opened\n");
                continue;
            }
            list();
        }

        if (strcmp("df", token[0]) == 0)
        {
            if (!image_open)
            {
                printf("ERROR: Disk image is not opened.\n");
                continue;
            }
            printf("%d bytes free\n", df());
        }

        if (strcmp("insert", token[0]) == 0)
        {               
            if (!image_open)
            {
                printf("ERROR: Disk image is not opened.\n");
                continue;
            }

            if (token[1] == NULL)
            {
                printf("ERROR: No filename specified.\n");
                continue;
            }
            insert(token[1]);

        }





        if (strcmp("exit", token[0]) == 0 || strcmp("quit", token[0]) == 0 )
        {
            exit(EXIT_SUCCESS);
        }

    }
    return 0;
}