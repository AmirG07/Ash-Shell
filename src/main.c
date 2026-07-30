#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>

/* --- Custom Headers --- */
#include "../include/builtins.h"
#include "../include/help.h"
#include "../include/colors.h"

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
