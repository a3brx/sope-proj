#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <zconf.h>
#include <wait.h>
#include "pipe.h"

char program[128] = "";
char flags[32] = "";
char argument[128] = "";

void get_dir_stat(char *path, struct stat *stat_buf) {
    if (lstat(path, stat_buf) == -1) {
        perror("lstat ERROR");
        exit(3);
    }
}

unsigned simpledu(DIR *dirp) {
    struct dirent *direntp;
    struct stat stat_buf;
    char path[384];
    unsigned total_size = 0;
    while ((direntp = readdir(dirp)) != NULL) {
        if (strcmp(direntp->d_name, ".") == 0 || strcmp(direntp->d_name, "..") == 0)
            continue;
        sprintf(path, "%s/%s", argument, direntp->d_name);
        get_dir_stat(path, &stat_buf);
        total_size += stat_buf.st_size;
        if (S_ISDIR(stat_buf.st_mode)) {
            if (fork() == 0) {
                execl(program, program, flags, path, NULL);
            } else {
                total_size += read_child_size();
            }
        }
    }
    write_size();
    return total_size;
}

int main(int argc, char **argv) {
    // Handle argument
    strcpy(program, argv[0]);
    strcpy(flags, argv[1]);
    strcpy(argument, argv[2]);
    create_pipe();

    DIR *dirp;
    if ((dirp = opendir(argument)) == NULL) {
        perror(argument);
        exit(2);
    }

    write_on_console(simpledu(dirp), argument);

    close_pipe();
    int ret;
    while (waitpid(-1, &ret, WNOHANG) >= 0);
    closedir(dirp);
    exit(0);
}