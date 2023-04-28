#ifndef TOOLS_H
#define TOOLS_H

void display_usage();

void trim();

void read_command(char **command_string);

void parser(char *token[], char *command_string);

void command_center(char *command);

#endif