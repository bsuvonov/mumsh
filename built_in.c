#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "built_in.h"
#include "parser.h"


enum { N_CHARS = 1024 };



void pwd(void)
{
    char path[N_CHARS];
    printf("%s\n", getcwd(path, N_CHARS));
}

void cd(char *command[])
{
    static char prev_cwd[N_CHARS] = {0};
    if (command[1] == NULL || !strcmp(command[1], "~"))
    {
        char *path = getenv("HOME");
        char curr_path[N_CHARS] = {0};
        getcwd(curr_path, N_CHARS);
        if(path)
        {
            chdir(path);
        }
        strncpy_safe(prev_cwd, curr_path, N_CHARS);
    }
    else if (!strcmp(command[1], "-"))
    {
        if(prev_cwd[0] != '\0')
        {
            char curr_path[N_CHARS] = {0};
            getcwd(curr_path, N_CHARS);
            chdir(prev_cwd);
            pwd();
            strncpy_safe(prev_cwd, curr_path, N_CHARS);
        }
    }
    else
    {
        char curr_path[N_CHARS] = {0};
        getcwd(curr_path, N_CHARS);
        if(chdir(command[1])==0)
        {
            strncpy_safe(prev_cwd, curr_path, N_CHARS);
        } else {
            fprintf(stderr, "%s: No such file or directory\n", command[1]);
        }
    }
}


bool is_built_in(char *command)
{
    if(!strcmp(command, "cd") || !strcmp(command, "pwd")) {
        return true;
    }
    return false;
}

void exec_built_in(char *command[])
{
    if(!strcmp(command[0], "pwd")) {
        pwd();
    }
    else if (!strcmp(command[0], "cd"))
    {
        cd(command);
    }
}
    

