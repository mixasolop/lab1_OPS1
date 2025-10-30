#define _XOPEN_SOURCE 700

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

// join 2 path. returned pointer is for newly allocated memory and must be freed
char* join_paths(const char* path1, const char* path2)
{
    char* res;
    const int l1 = strlen(path1);
    if (path1[l1 - 1] == '/')
    {
        res = malloc(strlen(path1) + strlen(path2) + 1);
        if (!res)
            ERR("malloc");
        strcpy(res, path1);
    }
    else
    {
        res = malloc(strlen(path1) + strlen(path2) + 2);  // additional space for "/"
        if (!res)
            ERR("malloc");
        strcpy(res, path1);
        res[l1] = '/';
        res[l1 + 1] = 0;
    }
    return strcat(res, path2);
}

void usage(int argc, char** argv)
{
    (void)argc;
    fprintf(stderr, "USAGE: %s path\n", argv[0]);
    exit(EXIT_FAILURE);
}


int walk(const char *name, const struct stat *s, int type, struct FTW *f)
{
    if(type == FTW_F){
        if(chdir("index/by_visible_title") == -1){
            ERR("chdir");
        }
        char* str = join_paths("../../",name);
        symlink(str, &name[f->base]);
        if(chdir("../..") == -1){
            ERR("chdir");
        }
        free(str);
    }
    return 0;
}

void parser(FILE* fptr){
    char* lineptr = NULL;
    size_t n = -1;
    char* token;
    char* author = NULL;
    char* title = NULL;
    char* genre = NULL;
    while(getline(&lineptr, &n, fptr) != -1){
        size_t size = strlen(lineptr);
        lineptr[size-1] = '\0';
        token = strtok(lineptr, ":");
        if((token = strtok(NULL, ":")) == NULL){
            continue;
        }
        
        if(strcmp(lineptr, "author") == 0){
            author = strdup(token);
        }
        if(strcmp(lineptr, "title") == 0){
            title = strdup(token);
        }
        if(strcmp(lineptr, "genre") == 0){
            genre = strdup(token);
        }
    }
    free(lineptr);

    
    if(author != NULL){
        printf("author: %s\n", author);
    }
    else{
        printf("author: missing!\n");
    }
    if(title != NULL){
        printf("title: %s\n", title);
    }
    else{
        printf("title: missing!\n");
    }
    if(genre != NULL){
        printf("genre: %s\n", genre);
    }
    else{
        printf("genre missing!\n");
    }
    free(author);
    free(title);
    free(genre);
}

int main(int argc, char** argv) { 
    if(mkdir("index", 0755) == -1)
        ERR("mkdir");
    if(mkdir("index/by_visible_title", 0755) == -1)
        ERR("mkdir");
    if(nftw("library", walk, 100, FTW_PHYS) != 0){
        ERR("nftw");
    }
}
