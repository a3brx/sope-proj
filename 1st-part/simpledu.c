#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <zconf.h>
#include <wait.h>
#include <stdbool.h>
#include "pipe.h"

char program[128] = "";
char flags[128] = "";
char argument[128] = "";
bool files_flag, bytes_flag, links_flag, symls_flag, sizes_flag;

bool isItem(char *word, char letter) {
    for (int i = 0; i < strlen(word); ++i)
        if (word[i] == letter)
            return true;
    return false;
}

void handle_arguments(char **argv) {
    strcpy(program, argv[0]);
    strcpy(flags, argv[1]);
    strcpy(argument, argv[2]);
    files_flag = isItem(flags, 'a');
    bytes_flag = isItem(flags, 'b');
    // block_size
    links_flag = isItem(flags, 'l');
    symls_flag = isItem(flags, 'L');
    sizes_flag = !isItem(flags, 'S');
}

void get_dir_stat(char *path, struct stat *stat_buf) {
    if (lstat(path, stat_buf) == -1) {
        perror("lstat ERROR");
        exit(3);
    }
}

void print_size(struct stat stat, char *path) {
    //if (bytes_flag)
    write_on_console(stat.st_size, path);
}

void print_dir_size(unsigned size, char *path) {
    //if (bytes_flag)
    write_on_console(size, path);
}

unsigned simpledu(DIR *dirp) {
    struct dirent *direntp;
    struct stat stat_buf;
    char path[384];
    get_dir_stat(argument, &stat_buf);
    unsigned total_size = stat_buf.st_size;
    while ((direntp = readdir(dirp)) != NULL) {
        if (strcmp(direntp->d_name, ".") == 0 || strcmp(direntp->d_name, "..") == 0)
            continue;
        sprintf(path, "%s/%s", argument, direntp->d_name);
        get_dir_stat(path, &stat_buf);
        if (S_ISDIR(stat_buf.st_mode)) {
            if (fork() == 0) {
                execl(program, program, flags, path, NULL);
            } else {
                total_size += read_child_size();
            }
        } else {
            total_size += stat_buf.st_size;
            print_size(stat_buf, path);
        }
    }
    write_size(total_size);
    return total_size;
}

int main(int argc, char **argv) {
    handle_arguments(argv);
    create_pipe();

    DIR *dirp;
    if ((dirp = opendir(argument)) == NULL) {
        perror(argument);
        exit(2);
    }

    print_dir_size(simpledu(dirp), argument);
    close_pipe();
    int ret;
    while (waitpid(-1, &ret, WNOHANG) >= 0);
    closedir(dirp);
    exit(0);
}