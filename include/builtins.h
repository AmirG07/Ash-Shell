#ifndef BUILTINS_H

#define BUILTINS_H


void print_dir(const char *dir_path, char **filenames, int count, bool show_long, int max_width);

int ash_cd(char **args);
int ash_ls(char **args);
int ash_help(char **args);
int ash_exit(char **args);

typedef struct {
    char *name;
    int (*ash_builtin)(char **args);
} Builtin;

extern Builtin builtin_funcs[];

int ash_num_builtins(void);

#endif
