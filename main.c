#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "signal_handler.h"
#include "parser.h"
#include "executor.h"
#include "constants.h"




bool handle_quotes(char *line, bool map[N_CLI_CHARS])
{
    int curr = 0;
    int next = 0;

    bool in_dbl_quotes = false;
    bool in_sngl_quotes = false;

    for(curr=0; next< (int)strlen(line); curr++, next++)
    {
        if(line[next] == '"' && !in_sngl_quotes)
        {
            curr--;
            in_dbl_quotes = !in_dbl_quotes;
        }
        else if (line[next] == '\'' && !in_dbl_quotes)
        {
            curr--;
            in_sngl_quotes = !in_sngl_quotes;
        }
        else
        {
            if(in_sngl_quotes || in_dbl_quotes){
                map[curr] = true;
            }
            line[curr] = line[next];
        }
    }
    if(curr-1 >= 0){
        line[curr-1] = '\0';       // replace the \n with \0
    }
    bool pass = true;

    for(int i = curr-2; i>=0; i--)
    {
        if(!map[i] && (line[i] == '>' || line[i] == '<' || line[i] == '|'))
        {
            if(!pass) {
                pass = true;    // this is an error and will be caught later in exec()
                break;
            }
            pass = false;
        }
        else if(line[i]!=' ')
        {
            break;
        }
    }

    if(!(in_sngl_quotes) && !(in_dbl_quotes))
    {
        if(pass) {
            return true;
        }
        return false;
    }
    return false;
}

void print_help(void)
{
    printf("Help? I think you want to see the author.\nAuthor: Bunyod Suvonov\n");
}



int main(int argc, const char *argv[])
{
    if (argc > 1)
    {
        if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
        {
            print_help();
            exit(0);
        }
    }

    setup_signal_handlers();

    while (1)
    {
        char line[N_CLI_CHARS];
        char line_cpy[N_CLI_CHARS]; // used for handling quotes
        char *command_args[N_ARGS] = {NULL};
        bool args_map[N_ARGS] = {false};

        printf("mumsh $ ");
        fflush(stdout);

        // Read a line of input as well as handle CTRL+D
        if (fgets(line, sizeof(line), stdin) == NULL)
        {
            if (feof(stdin))
            {
                printf("exit\n");
                fflush(stdout);
                break;
            }
            continue;
        }

        bool str_map[N_CLI_CHARS] = {false};
        strncpy_safe(line_cpy, line, N_CLI_CHARS);

        while(!handle_quotes(line_cpy, str_map))
        {
            printf("> ");
            fflush(stdout);
            char line_continued[N_CLI_CHARS];
            fgets(line_continued, sizeof(line_continued), stdin);
            
            // if it was > or < or |, not an unfinished quote
            if(!str_map[strlen(line_cpy)-1])
            {
                strncpy_safe(line_cpy, line, N_CLI_CHARS);
                line_cpy[strlen(line_cpy)-1] = ' '; // ' ' instead of '\0' because we want to have a space between | and the rest
            }
            else    // if it was an unfinished quote
            {
                strncpy_safe(line_cpy, line, N_CLI_CHARS);
            }

            strncat_safe(line_cpy, line_continued, N_CLI_CHARS); // concatenate the input
            strncpy_safe(line, line_cpy, N_CLI_CHARS); // save the line to backup array
        }

        parse_cli(command_args, line_cpy, str_map, args_map);    // parse line

        int n_args = 0;
        for (n_args = 0; n_args < N_ARGS && command_args[n_args]!=NULL; n_args++){} // count the number of argument tokens


        // If exit command is input, then exit.
        if (command_args[0] != NULL && !strcmp(command_args[0], "exit"))
        {
            printf("exit\n");
            fflush(stdout);

            // Free the allocated memory for command_args.
            for (int i = 0; i < n_args; i++)
            {
                if(command_args[i])
                {
                    free(command_args[i]);
                    command_args[i] = NULL;
                }
            }
            exit(0);
        }

        // Execute the parsed command.
        exec(command_args, n_args, args_map);

        // Free the allocated memory for command_args.
        for (int i = 0; i < n_args; i++)
        {
            if (command_args[i])
            {
                free(command_args[i]);
                command_args[i] = NULL;
            }
        }
    }

    return 0;
}


