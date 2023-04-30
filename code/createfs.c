#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "createfs.h"
#include "tools.h"

void createfs()
{
    FileSystem fs;
    memset(&fs, 0, sizeof(fs));

    // Setting all the blocks to be free
    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        fs.free_block_list[i] = 1;
    }

    FILE *fp = fopen("disk.img", "wb");
    if (!fp)
    {
        printf("Error creating disk image\n");
        return;
    }

    // Writing file system to disk image
    fwrite(&fs, sizeof(fs), 1, fp);

    fclose(fp);

    printf("Created file system\n");
}
