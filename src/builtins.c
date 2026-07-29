#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

/* --- Source Headers */
#include "../include/builtins.h"
#include "../include/colors.h"


char *builtin_str[] = {
    "cd",
    "ls",
    "help",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &ash_cd,
    &ash_ls,
    &ash_help,
    &ash_exit
};

int ash_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char*);
}

int ash_cd(char **args)
{
    if(args[1] == NULL) {
	fprintf(stderr, "ash -> expected argument to \"cd\"\n");
    }
    else {
	if(chdir(args[1]) != 0) {
	    perror("ash");
	}
    }
    return 1;
}

void print_dir(char **args, struct dirent *de, struct stat *st, int max_width)
{
    if(strcmp((*de).d_name, ".") != 0 && strcmp((*de).d_name, "..") != 0) {
	if(stat((*de).d_name, st) == 0) {
	    if(args[1] != NULL && strcmp(args[1], "-l") == 0) {
		if (S_ISREG((*st).st_mode))
		    printf("-");
		else if (S_ISDIR((*st).st_mode))
		    printf("d");
		else if (S_ISLNK((*st).st_mode))
		    printf("l");
		else if (S_ISCHR((*st).st_mode))
		    printf("c");
		else if (S_ISBLK((*st).st_mode))
		    printf("b");
		else if (S_ISFIFO((*st).st_mode))
		    printf("p");
		else if (S_ISSOCK((*st).st_mode))
		    printf("s");

		printf(((*st).st_mode & S_IRUSR) ? "r" : "-");
		printf(((*st).st_mode & S_IWUSR) ? "w" : "-");
		printf(((*st).st_mode & S_IXUSR) ? "x" : "-");

		printf(((*st).st_mode & S_IRGRP) ? "r" : "-");
		printf(((*st).st_mode & S_IWGRP) ? "w" : "-");
		printf(((*st).st_mode & S_IXGRP) ? "x" : "-");

		printf(((*st).st_mode & S_IRGRP) ? "r" : "-");
		printf(((*st).st_mode & S_IWGRP) ? "w" : "-");
		printf(((*st).st_mode & S_IXGRP) ? "x" : "-");

		printf(ASH_GREEN " %-*s " ASH_RESET, max_width, (*de).d_name);

		struct tm *tm = localtime(&(*st).st_mtim.tv_sec);
		char time_buffer[64];
		strftime(time_buffer, sizeof(time_buffer), "%b %d %H:%M", tm);
		printf(ASH_YELLOW " %s\n" ASH_RESET, time_buffer);

	    }
	    else {
		printf(ASH_GREEN "%s\n" ASH_RESET, (*de).d_name);	
	    }	
	}
    }   
}

int ash_ls(char **args)
{
    struct dirent *de;

    DIR *dr = opendir(".");

    if(dr == NULL) {
	perror("opendir");
	exit(EXIT_FAILURE);
    }

    int max_width = 0;
    while((de = readdir(dr)) != NULL) {
	if(strlen((*de).d_name) > max_width)
	    max_width = strlen((*de).d_name);
    }

    rewinddir(dr);

    while((de = readdir(dr)) != NULL) {
	struct stat *st = malloc(sizeof(struct stat));

	if(!st) {
	    fprintf(stderr, "ash-> allocation error");
	    exit(EXIT_FAILURE);
	}

	print_dir(args, de, st, max_width);

	free(st);
    }

    closedir(dr);

    return 1;
}

int ash_help(char **args)
{
    printf("Amir Reza Gohari's ASH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (int i = 0; i < ash_num_builtins(); i++) {
	printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int ash_exit(char **args)
{
    return 0;
}
