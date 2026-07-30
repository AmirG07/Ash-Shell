/* Standard Headers and Custom Definitions */
#include "ash.h"

/* Custom Headers */
#include "builtins.h"
#include "help.h"
#include "colors.h"

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
    else if(strcmp(args[1], "-h") == 0 || strcmp(args[1], "--help") == 0) {
	print_help(args);
    }
    else {
	if(chdir(args[1]) != 0) {
	    perror("ash");
	}
    }

    return 1;
}

void print_dir(char **args, char **filenames, int count, int max_width)
{
    for(int i = 0; i < count; i++) {
	struct stat st;
	
	if(strcmp(filenames[i], ".") != 0 && strcmp(filenames[i], "..") != 0) {
	    if(stat(filenames[i], &st) == 0) {
		if(args[1] != NULL && (strchr(args[1], 'l') && args[1][0] == '-')) {
		    if (S_ISREG(st.st_mode))
			printf("-");
		    else if (S_ISDIR(st.st_mode))
			printf("d");
		    else if (S_ISLNK(st.st_mode))
			printf("l");
		    else if (S_ISCHR(st.st_mode))
			printf("c");
		    else if (S_ISBLK(st.st_mode))
			printf("b");
		    else if (S_ISFIFO(st.st_mode))
			printf("p");
		    else if (S_ISSOCK(st.st_mode))
			printf("s");

		    printf((st.st_mode & S_IRUSR) ? "r" : "-");
		    printf((st.st_mode & S_IWUSR) ? "w" : "-");
		    printf((st.st_mode & S_IXUSR) ? "x" : "-");

		    printf((st.st_mode & S_IRGRP) ? "r" : "-");
		    printf((st.st_mode & S_IWGRP) ? "w" : "-");
		    printf((st.st_mode & S_IXGRP) ? "x" : "-");

		    printf((st.st_mode & S_IRGRP) ? "r" : "-");
		    printf((st.st_mode & S_IWGRP) ? "w" : "-");
		    printf((st.st_mode & S_IXGRP) ? "x" : "-");

		    printf(ASH_GREEN " %-*s " ASH_RESET, max_width, filenames[i]);

		    struct tm *tm = localtime(&st.st_mtim.tv_sec);
		    char time_buffer[64];
		    strftime(time_buffer, sizeof(time_buffer), "%b %d %H:%M", tm);
		    printf(ASH_YELLOW " %s\n" ASH_RESET, time_buffer);

		}
		else {
		    printf(ASH_GREEN "%s\n" ASH_RESET, filenames[i]);	
		}	
	    }
	}   
	free(filenames[i]);
    }
}

int ash_ls(char **args)
{
    if(args[1] != NULL && (strcmp(args[1], "-h") == 0 || strcmp(args[1], "--help") == 0)) {
	print_help(args);
	return 1;
    }
    
    int buffer_size = ASH_FILES_BUFSIZE;
    int normal_count = 0, dot_count = 0;
    char **normal_filenames = malloc(sizeof(char*) * buffer_size);
    char **dot_filenames = malloc(sizeof(char*) * buffer_size);
    
    int max_width = 0;
    struct dirent *de;
    DIR *dr = opendir(".");

    if(dr == NULL) {
	perror("opendir");
	exit(EXIT_FAILURE);
    }

    while((de = readdir(dr)) != NULL) {
	if(strlen((*de).d_name) > max_width)
	    max_width = strlen((*de).d_name);
	
	if(normal_count == buffer_size)	    
	    normal_filenames = realloc(normal_filenames, sizeof(char*) * (buffer_size * 2));
	if(dot_count == buffer_size)   
	    dot_filenames = realloc(dot_filenames, sizeof(char*) * (buffer_size * 2));


	if((*de).d_name[0] == '.')
	    dot_filenames[dot_count++] = strdup((*de).d_name); 
	else
	    normal_filenames[normal_count++] = strdup((*de).d_name);    
    }
    closedir(dr);
    
    if(args[1] != NULL && strchr(args[1], 'a')) {
	print_dir(args, dot_filenames, dot_count, max_width);
	
    }
    print_dir(args, normal_filenames, normal_count, max_width);
    

    free(normal_filenames);
    free(dot_filenames);

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
