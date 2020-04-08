#include "pipe.h"
#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <string.h>

#define READ 0
#define WRITE 1
#define MAXLINE 1000

int parent_in, parent_out, in, out, console_out;

void create_pipe() {
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
    dup2(in, STDIN_FILENO);
    dup2(out, STDOUT_FILENO);
}

void close_pipe() {
    close(in);
    close(out);
}

int write_on_console(unsigned size, char *path) {
    char line[MAXLINE];
    sprintf(line, "%-15d %s\n", size, path);
    return write(console_out, line, strlen(line));
}

int read_child_size() {
    char line[MAXLINE];
    read(in, line, MAXLINE);
    return atoi(line);;
}

void write_size(int size) {
    char line[MAXLINE];
    int n = sprintf(line, "%d\n", size);
    line[n] = 0;
    write(parent_out, line, n);
}