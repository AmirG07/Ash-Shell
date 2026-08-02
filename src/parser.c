/* --- Standard Headers & Custom Definitions --- */
#include "ash.h"

/* --- Custom Headers --- */
#include "parser.h"
#include "colors.h"

Token **ash_tokenizer(char *line)
{
    int buffer_size = ASH_TOKEN_BUFSIZE;
    Token **tokens = malloc(sizeof(Token*) * buffer_size);
    int position = 0;
    
    if(!tokens) {
	fprintf(stderr, "ash-> allocation error\n");
	exit(EXIT_FAILURE);
    }

    while(*line != '\0') {
	if(*line == ' ' || *line == '\t' || *line == '\n') {
	    line++;
	    continue;
	}

	if(position >= buffer_size) {
	    buffer_size += buffer_size;
	    tokens = realloc(tokens, sizeof(Token*) * buffer_size);
	}
	
	Token *token = malloc(sizeof(Token));

	if(!token) {
	    fprintf(stderr, "ash-> allocation error\n");
	    exit(EXIT_FAILURE);
	}

	if(*line == '|') {
	    if(*(line + 1) == '|') {
		token->type = TOKEN_OR;
		token->value = strdup("||");
		line += 2;
	    }
	    else {
		token->type = TOKEN_PIPE;
		token->value = strdup("|");
		line+=1;
	    }
	}
	else if(*line == '&') {
	    if(*(line + 1) == '&') {
		token->type = TOKEN_AND;
		token->value = strdup("&&");
		line += 2;
	    }
	    else {
		token->type = TOKEN_BACKGROUND;
		token->value = strdup("&");
		line += 1;
	    }
	}
	else if(*line == '>') {
	    if(*(line + 1) == '>') {
		token->type = TOKEN_APPEND_OUT;
		token->value = strdup(">>");
		line += 2;
	    }
	    else {
		token->type = TOKEN_REDIRECT_OUT;
		token->value = strdup(">");
		line += 1;
	    }
	}
	else if(*line == '<') {
	    if(*(line + 1) == '<') {
		token->type = TOKEN_APPEND_IN;
		token->value = strdup("<<");
		line += 2;
	    }
	    else {
		token->type = TOKEN_REDIRECT_IN;
		token->value = strdup("<");
		line += 1;
	    }
	}
	else {
	    char *start = line;
	    while(*line != '\0' && *line != '\t' && *line != '\n' &&
		  *line != '\r' && *line != '|' && *line != '&' && 
		  *line != ' ' && *line != '<' && *line != '>') {
		line++;
	    }
	    int length = line - start;
	    token->type = TOKEN_WORD;
	    token->value = strndup(start, length);
	}
	tokens[position++] = token;
    }

    tokens[position] = NULL;
    return tokens;
}

Pipeline *ash_parser(Token **tokens)
{
    if(tokens == NULL || tokens[0] == NULL) {
	return NULL;
    }

    Pipeline *pipeline = malloc(sizeof(Pipeline));

    int pipe_buffer_size = ASH_PIPE_BUFSIZE;
    pipeline->cmds = malloc(sizeof(Command) * pipe_buffer_size);
    pipeline->cmd_count = 0;
    pipeline->is_background = 0;

    int cmd_buffer_size = ASH_CMD_BUFSIZE;
    int position = 0;

    Command current_cmd;
    current_cmd.args = malloc(sizeof(char*) * cmd_buffer_size);
    current_cmd.input_file = NULL;
    current_cmd.output_file = NULL;
    current_cmd.append = 0;

    for(int i = 0; tokens[i] != NULL; i++) {
	Token *t = tokens[i];
	if(t->type == TOKEN_WORD) {
	    if(position >= cmd_buffer_size - 1) {
		cmd_buffer_size += cmd_buffer_size;
		current_cmd.args = realloc(current_cmd.args, sizeof(char*) * cmd_buffer_size);
	    }
	    current_cmd.args[position++] = t->value;
	}
	else if(t->type == TOKEN_REDIRECT_IN) {
	    if(tokens[i+1] != NULL && tokens[i+1]->type == TOKEN_WORD){
		i++;
		current_cmd.input_file = t->value;
	    }
	    else {
		fprintf(stderr, "ash-> syntax error near '<'\n");
	    }
	}
	else if(t->type == TOKEN_REDIRECT_OUT || t->type == TOKEN_APPEND_OUT) {
	    current_cmd.append = (t->type == TOKEN_APPEND_OUT) ? 1 : 0;
	    if(tokens[i+1] != NULL && tokens[i+1]->type == TOKEN_WORD){
		i++;
		current_cmd.output_file = tokens[i]->value;
	    }
	    else {
		fprintf(stderr, "ash-> syntax error near '>'\n");
	    }
	}
	else if(t->type == TOKEN_PIPE) {
	    current_cmd.args[position] = NULL;

	    if (pipeline->cmd_count >= pipe_buffer_size) {
                pipe_buffer_size += pipe_buffer_size;     
		pipeline->cmds = realloc(pipeline->cmds, sizeof(Command) * pipe_buffer_size);
            }
            pipeline->cmds[pipeline->cmd_count++] = current_cmd;

	    cmd_buffer_size = 8;
            position = 0;
            current_cmd.args = malloc(sizeof(char *) * cmd_buffer_size);
            current_cmd.input_file = NULL;
            current_cmd.output_file = NULL;
            current_cmd.append = 0;
	}
	else if(t->type == TOKEN_BACKGROUND) {
	    pipeline->is_background = 1;
	}
    }

    if(position > 0 || current_cmd.input_file || current_cmd.output_file) {
	current_cmd.args[position] = NULL;
	if(pipeline->cmd_count >= pipe_buffer_size - 1) {
	    pipeline->cmds = realloc(pipeline->cmds, sizeof(Command) * pipe_buffer_size); 
	}
	pipeline->cmds[pipeline->cmd_count++] = current_cmd;	
    }
    else {
	free(current_cmd.args);
    }

    return pipeline;
}

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



