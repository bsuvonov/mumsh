#ifndef PARCER
#define PARCER
#include <stddef.h>
#include <stdbool.h>

//char* strncat_safe(char *dst, const char *src, size_t dstsize);
size_t strncpy_safe(char * dst, const char * src, size_t maxlen);
void parse_cli(char *command_args[], char *line, const bool *str_map, bool *args_map);
char* strncat_safe(char *dst, const char *src, size_t dstsize);
#endif

