#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"

int ash_cd(char **args);
int ash_ls(char **args);
int ash_help(char **args);
int ash_exit(char **args);

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
	fprintf(stderr, "ash-> expected argument to \"cd\"\n");
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

		printf(GREEN " %-*s " RESET, max_width, (*de).d_name);

		struct tm *tm = localtime(&(*st).st_mtim.tv_sec);
		char time_buffer[64];
		strftime(time_buffer, sizeof(time_buffer), "%b %d %H:%M", tm);
		printf(YELLOW " %s\n" RESET, time_buffer);

	    }
	    else {
		printf(GREEN "%s\n" RESET, (*de).d_name);	
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


#define ASH_RL_BUFSIZE 1024
#define ASH_TOKEN_BUFSIZE 64
#define ASH_TOKEN_DELIM " \t\r\n\a"

char *ash_read_line()
{
    int buffer_size = ASH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * buffer_size);
    int c;

    if(!buffer) {
	fprintf(stderr, "ash-> allocation error\n");
	exit(EXIT_FAILURE);
    }

    while(1) {
	c = getchar();

	if(c == EOF || c == '\n') {
	    buffer[position] = '\0';
	    return buffer;
	}
	else {
	    buffer[position] = c;
	}
	position++;

	if(position >= buffer_size) {
	    buffer_size += ASH_RL_BUFSIZE;
	    buffer = realloc(buffer, buffer_size);
	    if(!buffer) {
		fprintf(stderr, "ash-> allocation error\n");
		exit(EXIT_FAILURE);
	    }
	}
    }
}

char **ash_split_line(char *line)
{
    int buffer_size = ASH_TOKEN_BUFSIZE;
    int position = 0;
    char **tokens = malloc(sizeof(char*) * buffer_size);
    char *token;

    if(!tokens) {
	fprintf(stderr, "ash-> allocation error\n");
	exit(EXIT_FAILURE);
    } 

    token = strtok(line, ASH_TOKEN_DELIM);
    while(token != NULL) {
	tokens[position] = token;
	position++;

	if(position >= buffer_size) {
	    buffer_size += ASH_TOKEN_BUFSIZE;
	    tokens = realloc(tokens, buffer_size * sizeof(char*));
	    if(!tokens) {
		fprintf(stderr, "ash-> allocation error\n");
		exit(EXIT_FAILURE);
	    }
	}

	token = strtok(NULL, ASH_TOKEN_DELIM);
    }

    tokens[position] = NULL;
    return tokens;
}

int ash_launch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid == 0){
	if(execvp(args[0], args) == -1) {
	    perror("ash");
	    
	}
	exit(EXIT_FAILURE);
    }
    else if(pid < 0) {
	perror("ash");
    }
    else {
	do {
	    wpid = waitpid(pid, &status, WUNTRACED);
	} while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int ash_execute(char **args)
{
    if(args[0] == NULL) {
	return 1;
    }

    for(int i = 0; i < ash_num_builtins(); i++) {
	if(strcmp(args[0], builtin_str[i]) == 0) {
	    return (*builtin_func[i])(args);
	}
    }

    return ash_launch(args);
}

#define ASH_CWD_BUFSIZE 512

char *ash_print_cwd(char *cwd)
{
    int buffer_size = ASH_CWD_BUFSIZE;
    
    cwd = malloc(sizeof(char) * ASH_CWD_BUFSIZE);

    if(!cwd) {
	perror("malloc");
	exit(EXIT_FAILURE);
    }
    
    while(getcwd(cwd, buffer_size) == NULL) {
	if(errno == ERANGE) {
	    buffer_size += buffer_size;

	    cwd = realloc(cwd, buffer_size);
	    if(!cwd) {
		free(cwd);
		perror("realloc");
		exit(EXIT_FAILURE);
	    }
	}
	else {
	    free(cwd);
	    perror("getcwd");
	    exit(EXIT_FAILURE);
	}
    }

    return cwd;
}

void ash_loop()
{
    char *cwd;
    char *line;
    char **args;
    int status;

    do {
	cwd = ash_print_cwd(cwd);
	printf("%s\n", cwd);
	printf("> ");
	line = ash_read_line();
	args = ash_split_line(line);
	status = ash_execute(args);

	free(line);
	free(args);
    } while (status);
}

int main(int argc, char **argv)
{
    ash_loop();
    return EXIT_SUCCESS;
}
