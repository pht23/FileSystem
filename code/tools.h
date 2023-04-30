#ifndef TOOLS_H_
#define TOOLS_H_

#define MAX_NUM_ARGS 5
#define MAX_CMD_SIZE 255
#define WHITESPACE " \t\n"

void display_usage();

void trim();

void read_command(char **command_string);

void parser(char *token[], char *command_string);

void command_center(char *command);

void createfs();

void openfs(char *path);

#endif