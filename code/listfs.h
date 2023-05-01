#ifndef CREATEFS_H_
#define CREATEFS_H_
#define MAX_NUM_ARGUMENTS 256

#define DRIVE_SIZE 226
#define BLOCK_SIZE 64
#define NUM_BLOCKS (DRIVE_SIZE / BLOCK_SIZE)
#define FILE_NAME_SIZE 16
#define MAX_NUM_FILES (DRIVE_SIZE - NUM_BLOCKS * 2) / (FILE_NAME_SIZE + 2)
typedef struct FileEntry
{
    char name[FILE_NAME_SIZE];
    unsigned char start_block;
    unsigned char num_blocks;
    struct FileEntry *next;
} FileEntry;

void list(char tokens[MAX_NUM_ARGUMENTS], FileEntry *directory, unsigned char *inodes);

#endif