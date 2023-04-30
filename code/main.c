#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "createfs.h"
#include "openfs.h"
/*
#include "savefs.h"
#include "closefs.h"
#include "insert.h"
#include "retrieve.h"
#include "delete.h"
#include "undelete.h"
#include "list.h"
#include "df.h"
#include "attribute.h"
#include "encrypt.h"
#include "decrypt.h"
*/
#include "tools.h" // moving the parser and read token function here






int main (void)
{
    char *command = (char*)malloc(MAX_CMD_SIZE);

    while (1)
    {
        printf("mfs> ");
        read_command(&command);
        command_center(command);
    }

    return 0;
}