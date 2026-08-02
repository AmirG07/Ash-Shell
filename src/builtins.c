/* Standard Headers and Custom Definitions */
#include "ash.h"

/* Custom Headers */
#include "builtins.h"
#include "help.h"
#include "colors.h"

Builtin builtin_funcs[] = {
    {
	"cd", 
	ash_cd
    },
    {
	"ls",
	ash_ls
    },
    {
	"help",
	ash_help
    },
    {
	"exit",
	ash_exit
    }
};

int ash_num_builtins()
{
    return sizeof(builtin_funcs) / sizeof(Builtin);
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

void merge(char **filenames, int l, int m, int r)
{
    int n1 = m - l + 1;
    int n2 = r - m;

    char *L[n1]; 
    char *R[n2];
    
    for(int i = 0; i < n1; i++) {
	L[i] = filenames[l + i];
    }
    for(int j = 0; j < n2; j++) {
	R[j] = filenames[m + 1 + j];
    }

    int i = 0, j = 0;
    int k = l;
    while(i < n1 && j < n2) {
	if(strcasecmp(L[i], R[j]) <= 0) {
	    filenames[k] = L[i];
	    i++;
	}
	else {
	    filenames[k] = R[j];
	    j++;
	}
	k++;
    }

    while(i < n1) {
	filenames[k] = L[i];
	i++;
	k++;
    }

    while(j < n2) {
	filenames[k] = R[j];
	j++;
	k++;
    }
}

void merge_sort(char **filenames, int l, int r)
{
    if(l < r) {
	int m = (l + r) / 2;

	merge_sort(filenames, l, m);
	merge_sort(filenames, m + 1, r);

	merge(filenames, l, m, r);
    }
}

void print_dir(const char *dir_path, char **filenames, int count, bool show_long, int max_width)
{
    for(int i = 0; i < count; i++) {
	char full_path[1024];
	snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, filenames[i]);

	if(strcmp(filenames[i], ".") != 0 && strcmp(filenames[i], "..") != 0) {
	    if(show_long) {
		struct stat st;

		if(lstat(full_path, &st) == 0) {
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

		    printf((st.st_mode & S_IROTH) ? "r" : "-");
		    printf((st.st_mode & S_IWOTH) ? "w" : "-");
		    printf((st.st_mode & S_IXOTH) ? "x" : "-");

		    printf(ASH_GREEN " %-*s " ASH_RESET, max_width, filenames[i]);

		    struct tm *tm = localtime(&st.st_mtim.tv_sec);
		    char time_buffer[64];
		    strftime(time_buffer, sizeof(time_buffer), "%b %d %H:%M", tm);
		    printf(ASH_YELLOW " %s\n" ASH_RESET, time_buffer);
		}


	    }
	    else {
		printf(ASH_GREEN "%s\n" ASH_RESET, filenames[i]);	
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
    
    int normal_buffer_size = ASH_FILES_BUFSIZE;
    int dot_buffer_size = ASH_FILES_BUFSIZE;
    int normal_count = 0, dot_count = 0;
    char **normal_filenames = malloc(sizeof(char*) * normal_buffer_size);
    char **dot_filenames = malloc(sizeof(char*) * dot_buffer_size);
    
    int max_width = 0;
    const char *dir_path = ".";
    
    bool show_long = false;
    bool show_all = false;
    for(int i = 1; args[i] != NULL; i++) {
	if(args[i][0] == '-') {
	    if(strchr(args[i], 'l'))
		show_long = true;
	    if(strchr(args[i], 'a'))
		show_all = true;
	}
	else {
	    dir_path = args[i];
	}
    }
        
    DIR *dr = opendir(dir_path);
    if(dr == NULL) {
	perror("opendir");
	return 1;
    }
    
    struct dirent *de;
    while((de = readdir(dr)) != NULL) {
	if(strlen((*de).d_name) > max_width) {
	    max_width = strlen((*de).d_name);
	}

	char full_path[1024];
	snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, de->d_name);

	if(normal_count == normal_buffer_size) {
	    normal_buffer_size *= 2;
	    normal_filenames = realloc(normal_filenames, sizeof(char*) * normal_buffer_size);
	}
	if(dot_count == dot_buffer_size) {
	    dot_buffer_size *= 2;
	    dot_filenames = realloc(dot_filenames, sizeof(char*) * dot_buffer_size);
	}


	if((*de).d_name[0] == '.')
	    dot_filenames[dot_count++] = strdup((*de).d_name); 
	else
	    normal_filenames[normal_count++] = strdup((*de).d_name);    
    }
    
    closedir(dr);
   
    
    merge_sort(normal_filenames, 0, normal_count - 1);
    merge_sort(dot_filenames, 0, dot_count - 1);


    if(show_all) {
	print_dir(dir_path, dot_filenames, dot_count, show_long, max_width);
	
    }
    print_dir(dir_path, normal_filenames, normal_count, show_long, max_width);
    

    free(normal_filenames);
    free(dot_filenames);

    return 1;
}

int ash_help(char **args)
{
    if(args[1] != NULL) {
	print_help(args);
    }
    else {
	printf("Amir Reza Gohari's ASH\n");
	printf("Type program names and arguments, and hit enter.\n");
	printf("The following are built in:\n");

	for (int i = 0; i < ash_num_builtins(); i++) {
	    printf("\t%s\n", builtin_funcs[i].name);
	}

	printf("Use the man command for information on other programs.\n");
    }
    return 1;
}

int ash_exit(char **args)
{
    return 0;
}


