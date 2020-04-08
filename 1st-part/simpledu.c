#include <stdio.h>
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
int parent_in, parent_out, in, out, console_out;

void create_pipe(){
    // Saving parent descriptors
    console_out = atoi(getenv("BACKUP_STDOUT_FILENO"));
    parent_in = dup(STDIN_FILENO);
    parent_out = dup(STDOUT_FILENO);
    close(parent_in);
    // Creating a new pipe
    int fd[2];
    pipe(fd);
    in = fd[READ];
    out = fd[WRITE];
}

void get_dir_stat(char *path, struct stat *stat_buf){
    if (lstat(path, stat_buf) == -1) {
        perror("lstat ERROR");
        exit(3);
    }
}

unsigned simpledu(DIR *dirp, char **argv) {
    struct dirent *direntp;
    struct stat stat_buf;
    char path[384];
    char line[MAXLINE];
    unsigned total_size = 0;
    while ((direntp = readdir(dirp)) != NULL) {
        if (strcmp(direntp->d_name, ".") == 0 || strcmp(direntp->d_name, "..") == 0)
            continue;
        sprintf(path, "%s/%s", argument, direntp->d_name);

        get_dir_stat(path, &stat_buf);

        if (S_ISDIR(stat_buf.st_mode)) {
            if (fork() == 0) {
                execl(argv[0], argv[0], flags, path, NULL);
            } else {
                read(in, line, MAXLINE);
            }
        }
        total_size += stat_buf.st_size;
    }
    int n = sprintf(line, "%d", total_size);
    line[n] = 0;
    write(parent_out, " ", 1);
    write(parent_out, line, n);
    write(parent_out, "\n", 1);
    return total_size;
}

int main(int argc, char **argv) {
    // Handle argument
    strcpy(flags, argv[1]);
    strcpy(argument, argv[2]);
    create_pipe();

    DIR *dirp;
    if ((dirp = opendir(argument)) == NULL) {
        perror(argument);
        exit(2);
    }

    dup2(in, STDIN_FILENO);
    dup2(out, STDOUT_FILENO);

    unsigned result = simpledu(dirp, argv);

    char line[MAXLINE];
    sprintf(line, "%-25d %s\n", result, argument);
    write(console_out, line, strlen(line));

    close(in);
    close(out);
    int ret;
    while (waitpid(-1, &ret, WNOHANG) >= 0);
    closedir(dirp);
    exit(0);
}