/* --- Standard Headers and Custom Definitions --- */
#include "ash.h"

/* --- Custom Headers --- */
#include "builtins.h"
#include "help.h"
#include "colors.h"

builtin_help help_table[] = {
    {
	"cd",
        "Change the current working directory.\n"
	"----\n"
	"Usage: cd [DIRECTORY]\n"
    },
    {
	"ls",
	"Shows the files in the current directory\n"
	"----\n"
	"Usage: ls [-OPTIONS]\n\n"
	"-Options:\n"
	"\t-l: Lists the files with details\n"
	"\t-h: Shows the manual of the command\n"
    }
};

const int help_table_size = sizeof(help_table) / sizeof(help_table[0]);

void print_help(char **args)
{
    bool has_help = false;
    for(int i = 0; i < help_table_size; i++) {
	if(strcmp(args[0], help_table[i].name) == 0) {
	    printf("%s", help_table[i].help);
	    has_help = true;
	    break;
	}
    }

    if(!has_help) {
	fprintf(stderr, "ash-> no help description was found");
    }
}
