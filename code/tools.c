#include "tools.h"


void display_usage()
{
    printf("USAGE: [filename] [token[0]]\n");
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

    for (int i = 0; i < MAX_NUM_ARGUMENTS; i++)
    {
        token[i] = NULL;
    }

    while (((argument_ptr = strsep(&working_string, WHITESPACE)) != NULL) 
                                    && (token_count < MAX_NUM_ARGUMENTS))
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



void command_center(char *command)
{
    char *token[MAX_NUM_ARGS];
    parser(token, command);

    if (strcmp(token[0], "createfs") == 0)
        createfs();

    else if (strcmp(token[0], "open") == 0)
        openfs();

    else if (strcmp(token[0], "close") == 0)
        closefs();

    else if (strcmp(token[0], "savefs") == 0)
        savefs();

    else if (strcmp(token[0], "insert") == 0)
        insert();

    else if (strcmp(token[0], "delete") == 0)
        delete();

    else if (strcmp(token[0], "undelete") == 0)
        undelete();

    else if (strcmp(token[0], "retrieve") == 0)
        retrieve();

    else if (strcmp(token[0], "list") == 0)
        list();

    else if (strcmp(token[0], "attrib") == 0)
        attribute();

    else if (strcmp(token[0], "df") == 0)
        display_free();
    
    else if (strcmp(token[0], "encrypt") == 0)
        encrypt();

    else if (strcmp(token[0], "decrypt") == 0)
        decrypt();
    
    else
        display_usage();
}