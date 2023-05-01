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


int32_t findFreeBlock()
{
    int i;
    for ( i = FIRST_DATA_BLOCK; i < NUM_BLOCKS; i++ )
    {
        if ( free_blocks[i] )
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
    int directory_entry = -1
    for (i = 0; i < NUM_FILES; i++)
    {
        if (directory[i].in_use == 0)
        {
            directory_entry = i;
            break;
        }
    }

    if (directory_entry == -1)
    {
        printf("ERROR: Could not find a free directory entry\n.");
    }

    // Open the input file read-only 
    FILE *ifp = fopen (filename, "r"); 
    printf("Reading %d bytes from %s\n", (int)buf.st_size, filename);
 
    // Save off the size of the input file since we'll use it in a couple of places and 
    // also initialize our index variables to zero. 
    int copy_size = buf.st_size;

    // We want to copy and write in chunks of BLOCK_SIZE. So to do this 
    // we are going to use fseek to move along our file stream in chunks of BLOCK_SIZE.
    // We will copy bytes, increment our file pointer by BLOCK_SIZE and repeat.
    int offset = 0;               

    // We are going to copy and store our file in BLOCK_SIZE chunks instead of one big 
    // memory pool. Why? We are simulating the way the file system stores file data in
    // blocks of space on the disk. block_index will keep us pointing to the area of
    // the area that we will read from or write to.
    int block_index = -1;
 
    // copy_size is initialized to the size of the input file so each loop iteration we
    // will copy BLOCK_SIZE bytes from the file then reduce our copy_size counter by
    // BLOCK_SIZE number of bytes. When copy_size is less than or equal to zero we know
    // we have copied all the data from the input file.
    while(copy_size > 0)
    {
        fseek( ifp, offset, SEEK_SET );
 
      // Read BLOCK_SIZE number of bytes from the input file and store them in our
      // data array. 

      // Find a free block
      block_index = findFreeBlock();

      if (block_index = -1)
      {
        printf("ERROR: Can not find a free block.\n");
        return;
      }

      int bytes  = fread( data[block_index], BLOCK_SIZE, 1, ifp );

      // If bytes == 0 and we haven't reached the end of the file then something is 
      // wrong. If 0 is returned and we also have the EOF flag set then that is OK.
      // It means we've reached the end of our input file.
      if( bytes == 0 && !feof( ifp ) )
      {
        printf("An error occured reading from the input file.\n");
        return -1;
      }

      // Clear the EOF file flag.
      clearerr( ifp );

      // Reduce copy_size by the BLOCK_SIZE bytes.
      copy_size -= BLOCK_SIZE;
      
      // Increase the offset into our input file by BLOCK_SIZE.  This will allow
      // the fseek at the top of the loop to position us to the correct spot.
      offset    += BLOCK_SIZE;

      // Increment the index into the block array 
      // DO NOT just increment block index in your file system
      block_index ++;
    }

    // We are done copying from the input file so close it out.
    fclose( ifp );
    

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