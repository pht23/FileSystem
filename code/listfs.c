#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "createfs.h"
#include "listsfs.h"
#include "tools.h"

#define MAX_NUM_ARGUMENTS 256



void list(char tokens[MAX_NUM_ARGUMENTS], FileEntry *directory, unsigned char *inodes)
{
    bool empty = true;
    bool list_hidden = false, list_attrib = false;

    // Parse options
    for (int i = 1; i < MAX_NUM_ARGUMENTS && tokens[i] != NULL; ++i)
    {
        if (tokens[i] == '-')
        {
            char opt = tokens[i][1];
            switch (opt)
            {
            case 'h':
                list_hidden = true;
                break;
            case 'a':
                list_attrib = true;
                break;
            case '\0':
                fprintf(stderr, "list: ERROR: missing option parameter");
                break;
            default:
                fprintf(stderr, "list: unrecognized option %c", opt);
            }
        }
    }

    FileEntry *current = directory;
    while (current != NULL)
    {
        struct inode this = inodes[current->start_block];
        if ((this.attrib & ATTRIB_HIDDEN) && !list_hidden)
        {
            current = current->next;
            continue;
        }

        char temp[65];
        char *cpy = current->name;

        int j = 0;
        while (cpy && j < 64)
        {
            temp[j++] = *(cpy++);
        }
        temp[j] = '\0';

        empty = false;
        if (list_attrib)
            printf("%s\t%hhu\n", temp, this.attrib);
        else
            printf("%s\n", temp);

        current = current->next;
    }

    if (empty)
    {
        printf("list: No files found.\n");
    }
}
