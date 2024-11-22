#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"


char* strncat_safe(char *dst, const char *src, size_t dstsize)
{
    size_t dst_len = 0;
    while (dst_len < dstsize && dst[dst_len] != '\0') {
        dst_len++;
    }

    if (dst_len == dstsize) {
        return dst;
    }
    size_t ind = 0;
    while (src[ind] != '\0' && (dst_len + ind) < (dstsize - 1))
    {
        dst[dst_len + ind] = src[ind];
        ind++;
    }

    dst[dst_len + ind] = '\0';
    return dst;
}


size_t strncpy_safe(char * dst, const char * src, size_t maxlen)
{
	if (maxlen == 0) {
        return 0;
    }
    size_t ind = 0;
    for (ind = 0; ind < maxlen - 1 && src[ind] != '\0'; ind++) {
        dst[ind] = src[ind];
    }

    dst[ind] = '\0';  // Ensure null-termination
    return ind; 
}

void parse_cli(char *command_args[], char *line, const bool *str_map, bool *args_map)
{
    // Parse the input into command arguments.
    int ctr = 0;  // Counter for the number of arguments parsed.
    int ptr = 0;  // Pointer to track the start of the current argument.


    for (int i = 0; i < (int)strlen(line); i++)
    {
        // Check if the current character is a space or null terminator.
        if (line[i] == ' ' || line[i] == '\0')
        {
            // If there is a valid segment between ptr and i, allocate memory and store it.
            if (i > ptr)
            {
                size_t len = (size_t)(i - ptr);
                command_args[ctr] = (char *)malloc((len + 1) * sizeof(char));
                if (command_args[ctr] == NULL)
                {
                    perror("malloc failed");
                    exit(1);
                }
                strncpy_safe(command_args[ctr], line + ptr, len + 1);

                if(str_map[i-1])
                {
                    args_map[ctr] = true; // mark this spot as string, will be used in exec() in main.c
                }
                ctr++;
            }
            ptr = i + 1;  // Move the pointer to the next character after the space.
        
        // Check if the current character is a redirection symbol '<'.
        }
        else if (line[i] == '<' && !str_map[i])
        {
            // If there is a valid segment before '<', allocate memory and store it.
            if (i > ptr)
            {
                size_t len = (size_t)(i - ptr);
                command_args[ctr] = (char *)malloc((len + 1) * sizeof(char));
                if (command_args[ctr] == NULL)
                {
                    perror("malloc failed");
                    exit(1);
                }
                strncpy_safe(command_args[ctr], line + ptr, len + 1);
                ctr++;
            }
            command_args[ctr] = (char *)malloc(2 * sizeof(char));
            if (command_args[ctr] == NULL)
            {
                perror("malloc failed");
                exit(1);
            }
            strncpy_safe(command_args[ctr], "<", 2);
            ctr++;
            ptr = i + 1;  // Move the pointer to the next character after '<'.

        // Check if the current character is a redirection symbol '>'.
        }
        else if (line[i] == '>' && !str_map[i])
        {
            // If there is a valid segment before '>', allocate memory and store it.
            if (i > ptr)
            {
                size_t len = (size_t)(i - ptr);
                command_args[ctr] = (char *)malloc((len + 1) * sizeof(char));
                if (command_args[ctr] == NULL)
                {
                    perror("malloc failed");
                    exit(1);
                }
                strncpy_safe(command_args[ctr], line + ptr, len + 1);
                ctr++;
            }
            // Check if the next character is also '>', indicating '>>' append redirection.
            if (line[i + 1] == '>')
            {
                command_args[ctr] = (char *)malloc(3 * sizeof(char));
                if (command_args[ctr] == NULL)
                {
                    perror("malloc failed");
                    exit(1);
                }
                strncpy_safe(command_args[ctr], ">>", 3);
                ctr++;
                i++;  // Skip the next '>' character.
            }
            else
            {
                // Allocate memory for the '>' symbol and store it as an argument.
                command_args[ctr] = (char *)malloc(2 * sizeof(char));
                if (command_args[ctr] == NULL)
                {
                    perror("malloc failed");
                    exit(1);
                }
                strncpy_safe(command_args[ctr], ">", 2);
                ctr++;
            }
            ptr = i + 1;  // Move the pointer to the next character after '>'.
        }
    }

    // Add the last part of the input if it exists.
    if (ptr < (int)strlen(line))
    {
        size_t len = (size_t)(strlen(line) - (uint64_t)ptr);
        command_args[ctr] = (char *)malloc((len + 1) * sizeof(char));
        if (command_args[ctr] == NULL)
        {
            perror("malloc failed");
            exit(1);
        }
        strncpy_safe(command_args[ctr], line + ptr, len + 1);  // Copy the last argument.
        ctr++;
    }

    // Set the last argument to NULL to indicate the end of the arguments.
    command_args[ctr] = NULL;
}


