#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "errno.h"

#include "constants.h"
#include "signal_handler.h"
#include "executor.h"
#include "built_in.h"


void split_redirect_command(char *command_args[], char *redirections[2][2], char *left_redir_file_inputs[])
{
    int left_redir_ctr = 1;
    int n_args = 0;
    for(int i=0; i<N_ARGS && command_args[i]!=NULL; i++) {
        n_args++;
    }

    for (int i=0; i<n_args; i++)
    {
        if(!strcmp(command_args[i], ">") || !strcmp(command_args[i], ">>"))
        {
            redirections[1][0] = command_args[i];
            if(i+1 < n_args)
            {
                redirections[1][1] = command_args[++i];
            }

        } 
        else if (!strcmp(command_args[i], "<"))
        {
            redirections[0][0] = command_args[i];
            if (i+1 < n_args)
            {
                redirections[0][1] = command_args[++i];
            }
    
        }
        // NOTE: The line below is not a proper way of identifying if the command outputs to STDOUT
        else if (!strcmp(command_args[i], "ls") || !strcmp(command_args[i], "echo") || !strcmp(command_args[i], "cat") || !strcmp(command_args[i], "grep") || \
        !strcmp(command_args[i], "head") || !strcmp(command_args[i], "diff") || !strcmp(command_args[i], "pwd") || !strcmp(command_args[i], "sleep"))
        {
            left_redir_file_inputs[0] = command_args[i];
        }
        else
        {
            left_redir_file_inputs[left_redir_ctr++] = command_args[i];
        }
    }
}


void run_command(char *command[])
{
    if(is_built_in(command[0]))
    {
        exec_built_in(command);
        exit(0);
    
    } else {
        execvp(command[0], command);
        fprintf(stderr, "%s: command not found\n", command[0]);
        exit(1);
    }
}



bool exec_command(char *command_args[])
{
    char *left_redir_file_inputs[N_ARGS] = {NULL};
    // redirections[0] is for left and [1] for right
    char *redirections[2][2] = {{NULL}};

    // splt command into parts
    split_redirect_command(command_args, redirections, left_redir_file_inputs); 
    
    // check if the main command is missing
    if(!left_redir_file_inputs[0] && command_args[0]!=NULL &&(command_args[0][0] == '>' || command_args[0][0] == '<' || command_args[0][0] == '|'))
    {
        fprintf(stderr, "error: missing program\n");
        exit(1);
    }

    // if there are redirections to both left and right in the command
    if(left_redir_file_inputs[0] && redirections[1][0] && redirections[1][1])
    {
        // open output file
        int file_write_fd = -1;
        if(!strcmp(redirections[1][0], ">"))
        {
            file_write_fd = open(redirections[1][1], O_WRONLY | O_CREAT | O_TRUNC, FILE_PERMISSIONS);
        
        } else {

            file_write_fd = open(redirections[1][1], O_WRONLY | O_CREAT | O_APPEND, FILE_PERMISSIONS);
        }
        if(file_write_fd==-1)
        {
            switch (errno) {
                case 2:
                    fprintf(stderr, "%s: No such file or directory\n", redirections[1][1]);
                default:
                    fprintf(stderr, "%s: Permission denied\n", redirections[1][1]);
            }
            exit(1);
        }
        
        
        // set STDOUT to output file
        dup2(file_write_fd, STDOUT_FILENO);
        close(file_write_fd);

        if(redirections[0][0] && redirections[0][1])
        {
            int file_fd = open(redirections[0][1], O_RDONLY);
            if(file_fd==-1)
            {
                perror(redirections[0][1]);
                exit(1);
            }
            dup2(file_fd, STDIN_FILENO);
            close(file_fd);

            run_command(left_redir_file_inputs);
        
        } else {
            run_command(left_redir_file_inputs);
        }
    }
    else if (left_redir_file_inputs[0] && redirections[0][0] && redirections[0][1])
    {
        // if there is a redirection to left only
        int file_fd = open(redirections[0][1], O_RDONLY);
        if(file_fd==-1)
        {
            perror(redirections[0][1]);
            exit(1);
        }
        dup2(file_fd, STDIN_FILENO);
        close(file_fd);

        run_command(left_redir_file_inputs);

    // no redirections
    } else if (command_args[0]) {
        run_command(command_args);
    }

    return true;
}



void single_command_exec(char *command_args[])
{
    if(!strcmp(command_args[0], "cd"))
    {
        exec_built_in(command_args);
    }
    else
    {
        // init signal mask
        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGINT);
        sigprocmask(SIG_BLOCK, &sigset, NULL);

        pid_t pid = fork();
        if(pid == 0)
        {
            sigprocmask(SIG_UNBLOCK, &sigset, NULL);
            reset_signal_handlers();
            exec_command(command_args);
        }
        else
        {
            int status=-1;
            waitpid(pid, &status, 0);
            sigprocmask(SIG_UNBLOCK, &sigset, NULL);
        }
    }
}


void handle_pipeline(char *commands[][N_ARGS], int n_commands)
{
    int pipe_fd[2] = {-1};
    int prev_fd = -1;
    int pids[N_ARGS] = {-1};

    // init signal mask
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    for (int i = 0; i < n_commands; i++)
    {
        // create a pipe for next pair
        if (i != n_commands-1)
        {
            if (pipe(pipe_fd) < 0)
            {
                perror("pipe creation failed");
                exit(1);
            }
        }

        pid_t pid = fork();
        if(pid == 0)
        {
            sigprocmask(SIG_UNBLOCK, &sigset, NULL);
            reset_signal_handlers();

            if (i == 0)
            {
                dup2(pipe_fd[1], STDOUT_FILENO);

            } else if (i == n_commands-1) {
                dup2(prev_fd, STDIN_FILENO);
            
            } else {
                dup2(prev_fd, STDIN_FILENO);
                dup2(pipe_fd[1], STDOUT_FILENO);
            }
            if (i != n_commands-1)
            {
                close(pipe_fd[0]);
                close(pipe_fd[1]);
            }
            if(prev_fd!=-1)
            {
                close(prev_fd);
            }
            exec_command(commands[i]);

        }
        else
        {
            pids[i] = pid;
            if (prev_fd!=-1) {
                close(prev_fd);
            }
            
            if (i!=n_commands-1)
            {
                prev_fd = dup(pipe_fd[0]);
                close(pipe_fd[0]);
                close(pipe_fd[1]);
            }
        }
    }

    for(int i=0; i<n_commands && pids[i]!=-1; i++)
    {
        int status=0;
        waitpid(pids[i], &status, 0);
    }
    sigprocmask(SIG_UNBLOCK, &sigset, NULL);
}



bool has_duplicate_redirections(const char *command_args[], int n_args)
{
    bool saw_pipe = false;
    bool saw_right_redir = false;
    bool saw_left_redir = false;

    for(int i=0; i<n_args; i++)
    {
        if(!strcmp(command_args[i], "<"))
        {
            if(saw_left_redir || saw_pipe)
            {
                fprintf(stderr, "error: duplicated input redirection\n");
                return true;
            }
            saw_left_redir = true;
        }
        else if (!strcmp(command_args[i], ">") || !strcmp(command_args[i], ">>"))
        {
            if(saw_right_redir)
            {
                fprintf(stderr, "error: duplicated output redirection\n");
                return true;
            }
            saw_right_redir = true;
        }
        else if (!strcmp(command_args[i], "|"))
        {
            if(saw_right_redir)
            {
                fprintf(stderr, "error: duplicated output redirection\n");
                return true;
            }
            saw_pipe = true;
        }
    }
    return false;
}



bool has_syntax_error(const char *command_args[], int n_args, const bool *args_map)
{
    for(int i=0; i<n_args-1; i++)
    {
        if(!args_map[i] && !args_map[i+1] && command_args[i][1] == '\0' && command_args[i+1][1] == '\0')
        {
            if((command_args[i][0] == '>' || command_args[i][0] == '<' || command_args[i][0] == '|')  && \
            (command_args[i+1][0] == '>' || command_args[i+1][0] == '<' || command_args[i+1][0] == '|'))
            {
                if(command_args[i][0] == '|' && command_args[i+1][0] == '|')
                {
                    fprintf(stderr, "error: missing program\n");
                    return true;
                }
                fprintf(stderr, "syntax error near unexpected token `%c'\n", command_args[i+1][0]);
                return true;
            }
        }
    }
    return false;
}






void exec(char *command_args[], int n_args, const bool *args_map)
{    
    if (!has_syntax_error((const char **)command_args, n_args, args_map) && !has_duplicate_redirections((const char **)command_args, n_args))
    {
        char *commands[N_ARGS][N_ARGS] = {{NULL}};
        int start = 0;
        int n_commands = 0;

        for (int i=0; i<n_args; i++)
        {
            if (i == n_args-1 || (!strcmp(command_args[i+1], "|") && !args_map[i+1]))
            {
                for (int j=start; j<=i; j++) {
                    commands[n_commands][j-start] = command_args[j];
                }
                start = i+2;
                n_commands++;
            }
        }
        if (n_commands==1)
        {
            single_command_exec(commands[0]);
        
        } else {
            handle_pipeline(commands, n_commands);
        }
    }
}


