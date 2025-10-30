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


struct Book{
    char* title;
    char* author;
    char* genre;

}typedef book_t;

struct DBentry{
    int size;
    char arr[64];
}typedef DBentry_t;

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

book_t parser(FILE* fptr){
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
    book_t book;
    book.title = title;
    book.author = author;
    book.genre = genre;
    return book;
}


int walk(const char *path, const struct stat *s, int type, struct FTW *f)
{
    if(type == FTW_F){
        if(chdir("index/by_visible_title") == -1){
            ERR("chdir");
        }
        char* str = join_paths("../../",path);
        symlink(str, &path[f->base]);
        if(chdir("../..") == -1){
            ERR("chdir");
        }
        free(str);
        FILE *fptr;
        fptr = fopen(path, "r");
        if(fptr == NULL){
            ERR("fopen");
        }
        book_t book = parser(fptr);
        fclose(fptr);
        char trun_title[65];
        if(book.title != NULL){
            if(chdir("index/by-title") == -1){
                ERR("chdir");
            }
            strncpy(trun_title, book.title, 64);
            trun_title[64] = '\0';
            char* str2 = join_paths("../../",path);
            symlink(str2, trun_title);
            if(chdir("../..") == -1){
                ERR("chdir");
            }
            free(str2);
        }
        free(book.author);
        free(book.title);
        free(book.genre);

        
    }
    return 0;
}


void open_DB(char* database_path){
    int DBfd;
    DBfd = open(database_path, O_RDONLY);
    if(DBfd == -1){
        ERR("open");
    }
    struct stat s;
    stat(database_path, &s);
    size_t size = s.st_size;
    if(size%68 != 0){
        printf("%ld\n", size);
        ERR("size!");
    }
    else{
        int num_entries = size/68;
        struct iovec* storage = malloc(num_entries*sizeof(struct iovec));
        DBentry_t* storage_entry = malloc(num_entries*sizeof(DBentry_t));
        for(int i = 0; i < num_entries; i++){
            storage[i].iov_base = &storage_entry[i];
            storage[i].iov_len = 68;
        }
        readv(DBfd, storage, num_entries);
        if(chdir("index/by-title"))
            ERR("chdir");
        for(int i = 0; i < num_entries; i++){
            struct stat s2;
            int res = stat(storage_entry[i].arr, &s2);
            
            // int fd = open(storage_entry[i].arr, O_CREAT | O_EXCL);
            if(res == -1){
                if(errno == ENOENT){
                    printf("Book %s is missing\n", storage_entry[i].arr);
                }
            }
            else{
                if(s2.st_size != storage_entry[i].size){
                    printf("Book %s size mismatch (%ld vs %d).\n", storage_entry[i].arr, s2.st_size, storage_entry[i].size);
                }
            }
        }
        free(storage);
        free(storage_entry);
    }
    close(DBfd);
}

int main(int argc, char** argv) { 
    if(argc != 2){
        usage(argc, argv);
    }
    if(mkdir("index", 0755) == -1)
        ERR("mkdir");
    if(mkdir("index/by_visible_title", 0755) == -1)
        ERR("mkdir");
    if(mkdir("index/by-title", 0755) == -1)
        ERR("mkdir");
    if(mkdir("index/by-genre", 0755) == -1)
        ERR("mkdir");
    if(nftw("library", walk, 100, FTW_PHYS) != 0){
        ERR("nftw");
    }
    open_DB(argv[1]);
}
