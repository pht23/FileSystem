#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools.h"

void display_usage()
{
    printf("USAGE: [filename] [command]]\n");
}

void trim(char *str)
{
    int len = strlen(str);
    if (str[len - 1] == '\n')   
        str[len - 1] = 0;
}


void read_command(char** command_string)
{
    while (!fgets(*command_string, MAX_CMD_SIZE, stdin));
}


void parser(char* token[], char *command_string)
{    
    int token_count = 0;                            
    char* argument_ptr = NULL;                      
    char* working_string = strdup(command_string);  
    char* head_ptr = working_string;                

    for (int i = 0; i < MAX_NUM_ARGS; i++)
    {
        token[i] = NULL;
    }

    while (((argument_ptr = strsep(&working_string, WHITESPACE)) != NULL) 
                                    && (token_count < MAX_NUM_ARGS))
    {
        token[token_count] = strndup(argument_ptr, MAX_CMD_SIZE);
        if(strlen( token[token_count] ) == 0)
        {
            token[token_count] = NULL;
        }
        token_count++;
    }
    free(head_ptr);
}
