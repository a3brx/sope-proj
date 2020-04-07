#include "simpledu.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <zconf.h>
#include <wait.h>

#define MAXLINE 1000
#define READ 0
#define WRITE 1

char flags[32] = "";
char argument[128] = "";
int in_backup, out_backup;

void parentCode() {

}

unsigned simpledu(int fd[2], DIR *dirp, char **argv) {
    struct dirent *direntp;
    struct stat stat_buf;
    char path[384];
    char line[MAXLINE];
    unsigned total_size = 0;
    while ((direntp = readdir(dirp)) != NULL) {
        if (strcmp(direntp->d_name, ".") == 0 || strcmp(direntp->d_name, "..") == 0)
            continue;
        sprintf(path, "%s/%s", argument, direntp->d_name);
        if (lstat(path, &stat_buf) == -1) {
            perror("lstat ERROR");
            exit(3);
        }
        if (S_ISDIR(stat_buf.st_mode)) {
            dup2(fd[READ], STDIN_FILENO);
            dup2(fd[WRITE], STDOUT_FILENO);
            if (fork() == 0)
                execl(argv[0], argv[0], flags, path, NULL);
            read(STDIN_FILENO, line, MAXLINE);
            dup2(atoi(getenv("BACKUP_STDOUT_FILENO")), STDOUT_FILENO);
            printf("%-25s %d\n", path, atoi(line));
        }
    }
    int n = sprintf(line, "%d", total_size);
    line[n] = 0;
    write(STDOUT_FILENO, line, n);
    return total_size;
}

int main(int argc, char **argv) {
    argv[1] = "-";
    argv[2] = ".";
    DIR *dirp;

    // Handle argument
    strcpy(flags, argv[1]);
    strcpy(argument, argv[2]);

    in_backup = dup(STDIN_FILENO);
    out_backup = dup(STDOUT_FILENO);

    // Create pipe
    int fd[2];
    pipe(fd);

    if ((dirp = opendir(argument)) == NULL) {
        perror(argument);
        exit(2);
    }

    simpledu(fd, dirp, argv);

    close(fd[WRITE]);
    close(fd[READ]);
    int ret;
    while (waitpid(-1, &ret, WNOHANG) >= 0);
    closedir(dirp);
    exit(0);
}