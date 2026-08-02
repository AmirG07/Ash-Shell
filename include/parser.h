#ifndef PARSER_H

#define PARSER_H

typedef enum {
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_REDIRECT_IN,
    TOKEN_REDIRECT_OUT,
    TOKEN_APPEND_IN,
    TOKEN_APPEND_OUT,
    TOKEN_BACKGROUND,
    TOKEN_EOF
} token_type;

typedef struct {
    token_type type;
    char *value;
} Token;

typedef struct {
    char **args;

    char *input_file;
    char *output_file;

    int append;
} Command;

typedef struct {
    Command *cmds;
    int cmd_count;

    int is_background;
} Pipeline;

char *ash_read_line();
Token **ash_tokenizer(char *line);
Pipeline *ash_parser(Token **tokens);

#endif
