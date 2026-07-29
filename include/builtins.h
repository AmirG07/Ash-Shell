#ifndef BUILTINS_H

#define BUILTINS_H

void print_dir(char **args, struct dirent *de, struct stat *st, int max_width);

int ash_cd(char **args);
int ash_ls(char **args);
int ash_help(char **args);
int ash_exit(char **args);

extern char *builtin_str[];
extern int (*builtin_func[]) (char **);

int ash_num_builtins(void);

#endif
