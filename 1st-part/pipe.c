#include "pipe.h"
#include <sys/times.h>
#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <string.h>

#define READ 0
#define WRITE 1
#define MAXLINE 1000

int parent_in, parent_out, in, out, console_out, console_in;
char *log_file;

void create_pipe() {
    // Saving parent descriptors
    console_out = atoi(getenv("BACKUP_STDOUT_FILENO"));
    console_in = atoi(getenv("BACKUP_STDIN_FILENO"));
    log_file = getenv("LOG_FILENAME");
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
    sprintf(line, "%d\t%s\n", size, path);
    return write(console_out, line, strlen(line));
}

char get_character() {
    char buf;
    read(console_in, &buf, 1);
    return buf;
}

void write_on_log(char *action, char *info) {
    struct tms t;
    clock_t end = times(&t);
    clock_t start = atol(getenv("SIMPLEDU_PARENT_START"));
    long ticks = sysconf(_SC_CLK_TCK);
    FILE *file = fopen(log_file, "a+");
    fprintf(file, "%05.2f - %8d - %s - %s\n", (float) ((end - start) / ticks), getpid(), action, info);
    fflush(file);
}

int read_child_size() {
    char line[MAXLINE + 1];
    int n = read(in, line, MAXLINE);
    line[n] = 0;
    write_on_log("RECV_PIPE", line);
    return atoi(line);
}

void write_size(unsigned size) {
    char line[MAXLINE];
    int n = sprintf(line, "%d", size);
    line[n] = 0;
    write(parent_out, line, n);
    write_on_log("SEND_PIPE", line);
}