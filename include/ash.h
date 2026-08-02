#ifndef ASH_H

#define ASH_H

/* --- Neccessary Headers --- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>

/* --- Definitions --- */

// -- main.c --
#define ASH_RL_BUFSIZE 1024
#define ASH_CWD_BUFSIZE 512

// -- builtins.c --
#define ASH_FILES_BUFSIZE 16

// -- parser.c --
#define ASH_TOKEN_BUFSIZE 64
#define ASH_PIPE_BUFSIZE 8
#define ASH_CMD_BUFSIZE 8

#endif
