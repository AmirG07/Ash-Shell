#ifndef HELP_H

#define HELP_H

typedef struct
{
    const char *name;
    const char *help;
} builtin_help;

extern builtin_help help_table[];
extern const int help_table_size;

void print_help(char **args);

#endif
