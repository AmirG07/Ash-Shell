/* --- Standard Headers and Custom Definitions --- */
#include "ash.h"

/* --- Custom Headers --- */
#include "parser.h"
#include "builtins.h"
#include "help.h"
#include "colors.h"

int is_builtin(char *arg)
{
    if (arg == NULL) {
        return 0;
    }
    for(int i = 0; i < ash_num_builtins(); i++) {
        if(strcmp(arg, builtin_funcs[i].name) == 0) {
            return 1;	
        }
    }
    return 0;
}

int builtin_run(char **args)
{
    for(int i = 0; i < ash_num_builtins(); i++) {
        if(strcmp(args[0], builtin_funcs[i].name) == 0) {
            return builtin_funcs[i].ash_builtin(args);
        }
    }
    return 1;
}

static void handle_redirections(Command *cmd) 
{
    if (cmd->input_file != NULL) {
        int fd_in = open(cmd->input_file, O_RDONLY);
        if (fd_in < 0) {
            perror("ash: input file error");
            exit(EXIT_FAILURE);
        }
        if (dup2(fd_in, STDIN_FILENO) < 0) {
            perror("ash: dup2 input error");
            close(fd_in);
            exit(EXIT_FAILURE);
        }
        close(fd_in);
    }

    if (cmd->output_file != NULL) {
        int flags = O_WRONLY | O_CREAT | (cmd->append ? O_APPEND : O_TRUNC);
        
        int fd_out = open(cmd->output_file, flags, 0644);
        if (fd_out < 0) {
            perror("ash: output file error");
            exit(EXIT_FAILURE);
        }
        if (dup2(fd_out, STDOUT_FILENO) < 0) {
            perror("ash: dup2 output error");
            close(fd_out);
            exit(EXIT_FAILURE);
        }
        close(fd_out);
    }
}

int ash_launch(Pipeline *pipeline)
{
    int fd[2];
    int pre_fd = -1;

    pid_t *pids = malloc(sizeof(pid_t) * pipeline->cmd_count);

    if(!pids) {
        fprintf(stderr, "ash-> allocation error\n");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < pipeline->cmd_count; i++) {
        Command *cmd = &pipeline->cmds[i];

        if (i < pipeline->cmd_count - 1) {
            if(pipe(fd) == -1) {
                perror("pipe");
                free(pids);
                exit(EXIT_FAILURE);
            }
        }
        
        pid_t pid = fork();
        switch(pid) {
            case -1:
                perror("fork");
                free(pids);
                exit(EXIT_FAILURE);

            case 0: 
                if(i > 0) {
                    dup2(pre_fd, STDIN_FILENO);
                    close(pre_fd);
                }
                if(i < pipeline->cmd_count - 1) {
                    close(fd[0]);
                    dup2(fd[1], STDOUT_FILENO);
                    close(fd[1]);
                }
                
                handle_redirections(cmd);

                if(is_builtin(cmd->args[0])) {
                    int status = builtin_run(cmd->args);
                    exit(status == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
                }
                else {
                    if (execvp(cmd->args[0], cmd->args) == -1) {
                        perror("ash");
                        exit(EXIT_FAILURE);
                    }
                }
                break;

            default: 
		pids[i] = pid;
                if (i > 0) {
                    close(pre_fd);
                }
                if (i < pipeline->cmd_count - 1) {
                    pre_fd = fd[0];
                    close(fd[1]);
                }
                break;
        }
    }

    if (!pipeline->is_background) {
        for (int i = 0; i < pipeline->cmd_count; i++) {
            waitpid(pids[i], NULL, 0);
        }
    }

    free(pids);

    return 1;
}

int ash_execute(Pipeline *pipeline)
{
    if(pipeline == NULL || pipeline->cmd_count == 0 || pipeline->cmds[0].args[0] == NULL) {
        return 1;
    }
    
    if(pipeline->cmd_count == 1 && is_builtin(pipeline->cmds[0].args[0])) {
        Command *cmd = &pipeline->cmds[0];

        int saved_stdin = dup(STDIN_FILENO);
        int saved_stdout = dup(STDOUT_FILENO);

        if (cmd->input_file) {
            int fd_in = open(cmd->input_file, O_RDONLY);
            if (fd_in >= 0) {
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            } 
            else { 
                perror("ash input error"); 
            }
        }
        if (cmd->output_file) {
            int flags = O_WRONLY | O_CREAT | (cmd->append ? O_APPEND : O_TRUNC);
            int fd_out = open(cmd->output_file, flags, 0644);
            if (fd_out >= 0) {
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            } 
            else { 
                perror("ash output error"); 
            }
        }

        int status = builtin_run(cmd->args);	 
        
        dup2(saved_stdin, STDIN_FILENO);
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdin);
        close(saved_stdout);

        return status;
    }

    return ash_launch(pipeline);
}

void ash_print_cwd()
{
    int buffer_size = ASH_CWD_BUFSIZE;

    char *cwd = malloc(sizeof(char) * ASH_CWD_BUFSIZE);

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

    char *home = getenv("HOME");

    if (home && strncmp(cwd, home, strlen(home)) == 0) {
        printf("~%s\n", cwd + strlen(home));
    }
    else {
        printf("%s\n", cwd);
    }

    free(cwd); 
}

static void free_tokens(Token **tokens)
{
    if (!tokens) return;
    for (int i = 0; tokens[i] != NULL; i++) {
        if (tokens[i]->value) {
            free(tokens[i]->value);
        }
        free(tokens[i]);
    }
    free(tokens);
}

static void free_pipeline(Pipeline *pipeline)
{
    if (!pipeline) 
	return;
    for (int i = 0; i < pipeline->cmd_count; i++) {
        if (pipeline->cmds[i].args) {
            free(pipeline->cmds[i].args);
        }
    }
    if (pipeline->cmds) {
        free(pipeline->cmds);
    }
    free(pipeline);
}

void ash_loop()
{
    char *line;
    Token **tokens;
    Pipeline *pipeline;
    int status;

    do {
        ash_print_cwd();
        printf("> ");
        line = ash_read_line(); 
        tokens = ash_tokenizer(line);
        pipeline = ash_parser(tokens);
        status = ash_execute(pipeline);

        free(line);
        free_tokens(tokens);
        free_pipeline(pipeline);
    } while (status);
}

int main(int argc, char **argv)
{
    ash_loop();
    return EXIT_SUCCESS;
}
