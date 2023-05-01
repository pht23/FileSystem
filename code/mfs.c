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
#define FIRST_DATA_BLOCK 1001
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
    for (i = 0; i < NUM_BLOCKS; i++)
    {
        if (free_blocks[i])
        {
            return i + 1001;
        }
    }
    return -1;
}

int32_t findFreeInode()
{
    int i;
    for (i = 0; i < NUM_FILES; i++)
    {
        if ( free_inodes[i] )
        {
            return i;
        }
    }
    return -1;
}

int32_t findFreeInodeBlock(int32_t inode)
{
    int i;
    for (i = 0; i < BLOCK_PER_FILE; i++)
    {
        if (inodes[inode].blocks[i] == -1)
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

void initialize()
{
    directory = (struct directoryEntry*)&data[0][0];
    inodes = (struct inode*)&data[20][0];
    free_blocks = (uint8_t *)&data[1000][0];
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
    fp = fopen(filename, "r");

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
    int directory_entry = -1;
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
        return;
    }

    // Open the input file read-only 
    FILE *ifp = fopen (filename, "r"); 
    printf("Reading %d bytes from %s\n", (int)buf.st_size, filename);
 
    // Save off the size of the input file since we'll use it in a couple of places and 
    // also initialize our index variables to zero. 
    int32_t copy_size = buf.st_size;

    // We want to copy and write in chunks of BLOCK_SIZE. So to do this 
    // we are going to use fseek to move along our file stream in chunks of BLOCK_SIZE.
    // We will copy bytes, increment our file pointer by BLOCK_SIZE and repeat.
    int32_t offset = 0;               

    // We are going to copy and store our file in BLOCK_SIZE chunks instead of one big 
    // memory pool. Why? We are simulating the way the file system stores file data in
    // blocks of space on the disk. block_index will keep us pointing to the area of
    // the area that we will read from or write to.
    int32_t block_index = -1;

    // Find free inode
        int32_t inode_index = findFreeInode();
        if(inode_index == -1)
        {
            printf("ERROR: Can not find a free inode.\n");
            return;
        }


        // Place the file info in the directory
        directory[directory_entry].in_use = 1;
        directory[directory_entry].inode = inode_index;
        strncpy(directory[directory_entry].filename, filename, strlen(filename));

        inodes[inode_index].file_size = buf.st_size;
 
    // copy_size is initialized to the size of the input file so each loop iteration we
    // will copy BLOCK_SIZE bytes from the file then reduce our copy_size counter by
    // BLOCK_SIZE number of bytes. When copy_size is less than or equal to zero we know
    // we have copied all the data from the input file.
    while(copy_size > 0)
    {
        fseek(ifp, offset, SEEK_SET);
 
        // Read BLOCK_SIZE number of bytes from the input file and store them in our
        // data array. 

        // Find a free block
        block_index = findFreeBlock();

        if (block_index == -1)
        {
            printf("ERROR: Can not find a free block.\n");
            return;
        }

        int32_t bytes = fread(data[block_index], BLOCK_SIZE, 1, ifp );

        // Save blocks in the inode
        int32_t inode_block = findFreeInodeBlock(inode_index);
        inodes[inode_index].blocks[inode_block] = block_index;


        // If bytes == 0 and we haven't reached the end of the file then something is 
        // wrong. If 0 is returned and we also have the EOF flag set then that is OK.
        // It means we've reached the end of our input file.
        if(bytes == 0 && !feof(ifp))
        {
            printf("ERROR: An error occured reading from the input file.\n");
            return;
        }

        // Clear the EOF file flag.
        clearerr( ifp );

        // Reduce copy_size by the BLOCK_SIZE bytes.
        copy_size -= BLOCK_SIZE;
      
        // Increase the offset into our input file by BLOCK_SIZE.  This will allow
        // the fseek at the top of the loop to position us to the correct spot.
        offset    += BLOCK_SIZE;

        block_index = findFreeBlock();
    }

    // We are done copying from the input file so close it out.
    fclose(ifp);
    

    // find free inodes and place the file
}

// line 130 to 200 of the code from examples is our retrieve
// read to second argument instead of argv[2]
// iterate through inode instead of block_index
void retrieve(char *filename, char *newfilename)
{
    if (!image_open)
    {
        printf("ERROR: Disk image is not opened.\n");
        return;
    }

    if (filename == NULL)
    {
        printf("ERROR: No filename specified.\n");
        return;
    }

    // Find the file in the directory
    int directory_entry = -1;
    for (int i = 0; i < NUM_FILES; i++)
    {
        if (directory[i].in_use && strcmp(directory[i].filename, filename) == 0)
        {
            directory_entry = i;
            break;
        }
    }

    if (directory_entry == -1)
    {
        printf("ERROR: File not found in the directory.\n");
        return;
    }

    int32_t inode_index = directory[directory_entry].inode;

    // Open the output file with the given newfilename or the original filename
    FILE *ofp = fopen(newfilename ? newfilename : filename, "w");

    if (ofp == NULL)
    {
        printf("ERROR: Unable to create the output file.\n");
        return;
    }

    int32_t remaining_size = inodes[inode_index].file_size;
    int32_t offset = 0;

    for (int i = 0; i < BLOCK_PER_FILE && remaining_size > 0; i++)
    {
        if (inodes[inode_index].blocks[i] != -1)
        {
            int32_t bytes_to_write = remaining_size < BLOCK_SIZE ? remaining_size : BLOCK_SIZE;
            fwrite(data[inodes[inode_index].blocks[i]], bytes_to_write, 1, ofp);
            remaining_size -= bytes_to_write;
            offset += bytes_to_write;
        }
    }

    fclose(ofp);
}

// set directory in_use to false
// set inode in_use to false
// set the blocks as free (free_blocks)
void delete(char *filename)
{
    if (image_open == 0)
    {
        printf ("ERROR: Disk image is not opened.\n");
        return;
    }

    int i;
    int dir_entry = -1;

    for (i = 0; i < NUM_FILES; i++)
    {
        if (directory[i].in_use && strcmp(directory[i].filename, filename) == 0)
        {
            dir_entry = i;
            break;
        }
    }

    if (dir_entry == -1)
    {
        printf("ERROR: File not found.\n");
        return;
    }

    int32_t inode_index = directory[dir_entry].inode;
    
    for (i = 0; i < BLOCK_PER_FILE; i++)
    {
        if (inodes[inode_index].blocks[i] != -1)
        {
            free_blocks[inodes[inode_index].blocks[i] - 1001] = 1;
            inodes[inode_index].blocks[i] = -1;
        }
    }
    // Free the inode
    inodes[inode_index].in_use = 0;
    inodes[inode_index].attribute = 0;
    inodes[inode_index].file_size = 0;
    free_inodes[inode_index] = 1;

    // Free the directory entry
    directory[dir_entry].in_use = 0;
    directory[dir_entry].inode = -1;
    //memset(directory[dir_entry].filename, 0, 64);
}

// exact opposite of delete
void undel(char *filename)
{
  if (image_open == 0)
    {
        printf ("ERROR: Disk image is not opened.\n");
        return;
    }

    int i;
    int dir_entry = -1;

    for (i = 0; i < NUM_FILES; i++)
    {
        if (directory[i].in_use == 0 && strcmp(directory[i].filename, filename) == 0)
        {
            dir_entry = i;
            break;
        }
    }

    if (dir_entry == -1)
    {
        printf("ERROR: File not found.\n");
        return;
    }

    int32_t inode_index = directory[dir_entry].inode;
    
    for (i = 0; i < BLOCK_PER_FILE; i++)
    {
        if (inodes[inode_index].blocks[i] != -1)
        {
            free_blocks[inodes[inode_index].blocks[i] - 1001] = 1;
            inodes[inode_index].blocks[i] = -1;
        }
    }
    // Free the inode
    inodes[inode_index].in_use = 1;
    inodes[inode_index].attribute = 0;
    inodes[inode_index].file_size = 0;
    free_inodes[inode_index] = 0;

    // Free the directory entry
    directory[dir_entry].in_use = 1;
    directory[dir_entry].inode = -1;
    //memset(directory[dir_entry].filename, 0, 64);
}

/*
    encrypt	encrypt <filename> <cipher>	
    XOR encrypt the file using the given cipher. 
    The cipher is limited to a 1-byte value
*/
//void enc(char * filename, uint8_t key);
void enc(char *filename, uint8_t key)
{
    if (!image_open)
    {
        printf("ERROR: Disk image is not opened.\n");
        return;
    }

    if (filename == NULL)
    {
        printf("ERROR: No filename specified.\n");
        return;
    }

    int directory_entry = - 1;
    for (int i = 0; i < NUM_FILES; i++)
    {
        if (directory[i].in_use && strcmp(directory[i].filename, filename) == 0)
        {
            directory_entry = i;
        }
    }

    if (directory_entry == -1)
    {
        printf("ERROR: File not found in the directory.\n");
        return;
    }

    int32_t inode_index = directory[directory_entry].inode;

    for (int i = 0; i < BLOCK_PER_FILE; i++)
    {
        if (inodes[inode_index].blocks[i] != -1)
        {
            for (int j = 0; j < BLOCK_SIZE; j++)
            {
                data[inodes[inode_index].blocks[i]][j] ^= key;
            }
        }
    }
}

void dec(char *filename, uint8_t key)
{
    // XOR decryption is the same as encryption
    enc(filename, key);
}

/*
    decrypt	encrypt <filename> <cipher>	
    XOR decrypt the file using the given cipher. 
    The cipher is limited to a 1-byte value
*/



/*
    read	read <filename> <starting byte> <number of bytes>	
    Print <number of bytes> bytes from the file, in hexadecimal, 
    starting at <starting byte>
*/
//void read(char *filename, /*starting byte*/ /*number of bytes*/);


/*
    attrib	attrib [+attribute] [-attribute] <filename>	
    Set or remove the attribute for the file
*/
void attrib();


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
    initialize();

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

        else if (strcmp("savefs", token[0]) == 0)
        {
            savefs();
        }

        else if (strcmp("open", token[0]) == 0)
        {
            if (token[1] == NULL)
            {
                printf("ERROR: No filename specified\n");
                continue;
            }
            openfs(token[1]);
        }

        else if (strcmp("close", token[0]) == 0)
        {
            closefs();
        }

        else if (strcmp("list", token[0]) == 0)
        {
            if (!image_open)
            {
                printf("ERROR: Disk image is not opened\n");
                continue;
            }
            list();
        }

        else if (strcmp("df", token[0]) == 0)
        {
            if (!image_open)
            {
                printf("ERROR: Disk image is not opened.\n");
                continue;
            }
            printf("%d bytes free\n", df());
        }

        else if (strcmp("insert", token[0]) == 0)
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

        else if (strcmp("delete", token[0]) == 0)
        {
            if (token[1] == NULL)
            {
                printf("ERROR: No filename specified.\n");
                continue;
            }
            delete(token[1]);
        }

        else if (strcmp("undelete", token[0]) == 0)
        {
            if (token[1] == NULL)
            {
                printf("ERROR: No filename specified.\n");
                continue;
            }
            undel(token[1]);
        }
        
        else if (strcmp("retrieve", token[0]) == 0)
        {
            if (token[1] == NULL)
            {
                printf("ERROR: No filename specified.\n");
                continue;
            }
            retrieve(token[1], token[2]);
        }

        else if (strcmp("encrypt", token[0]) == 0)
        {
            if (token[1] == NULL)
            {
                printf("ERROR: No filename specified.\n");
                continue;
            }
               
            uint8_t key;
            printf("Enter the cipher value to encrypt: ");
            scanf("%hhu", &key);
    
            enc(token[1], key);
            //enc(token[1], (unsigned char)cipher); // Call the enc function with the given filename and cipher
        }
        
        else if (strcmp("decrypt", token[0]) == 0)
        {
            if (token[1] == NULL)
            {
                printf("ERROR: No filename specified.\n");
                continue;
            }
            uint8_t key;
            printf("Enter the cipher value to decrypt: ");
            scanf("%hhu", &key);
            dec(token[1], key);
        }

        else if (strcmp("attrib", token[0]) == 0)
        {
            printf("This feature hasn't been implemented. Check back soon.\n");
            continue;
        }

        else if (strcmp("read", token[0]) == 0)
        {
            printf("This feature hasn't been implemented. Check back soon.\n");
            continue;
        }

        else if (strcmp("exit", token[0]) == 0 || strcmp("quit", token[0]) == 0 )
        {
            exit(EXIT_SUCCESS);
        }

        else
        {
            printf("Command: %s not found.\n", token[0]);
            continue;
        }

    }
    return 0;
}