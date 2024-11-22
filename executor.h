#ifndef EXEC_H
#define EXEC_H

#include <stdbool.h>
#include "constants.h"

void split_redirect_command(char *command_args[], char *redirections[2][2], char *left_redir_file_inputs[]);
void run_command(char *command[]);
bool exec_command(char *command_args[]);
void single_command_exec(char *command_args[]);
void handle_pipeline(char *commands[][N_ARGS], int n_commands);
void exec(char *command_args[], int n_args, const bool *args_map);
#endif


