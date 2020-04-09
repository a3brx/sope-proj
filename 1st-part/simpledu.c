#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <zconf.h>
#include <wait.h>
#include <stdbool.h>
#include <signal.h>
#include "pipe.h"

char program[128] = "";
char flags[128] = "";
char argument[128] = "";
int max_depth;
unsigned block_size;
bool files_flag, bytes_flag, symls_flag, sizes_flag;

void sigint_handler(int sig) {
    char temp;
    kill(atoi(getenv("SIMPLEDU_GROUP_ID")), SIGSTOP);
    write_on_console(0, "Do you want to terminate the program? (y/n):");
    temp = get_character();

    if (temp == 'y' || temp == 'Y') {
        kill(getpid(), SIGTERM);
        write_on_console(0, "Terminating all processes...");
    } else if (temp == 'n' || temp == 'N') {
        kill(-atoi(getenv("SIMPLEDU_GROUP_ID")), SIGCONT);
        write_on_console(0, "Resuming all processes...");
    } else {
        write_on_console(0, "Invalid Character! \n");
    }

    return;

}

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
    max_depth = atoi(argv[3]);
    block_size = atoi(argv[4]);
    files_flag = isItem(flags, 'a');
    bytes_flag = isItem(flags, 'b');
    symls_flag = isItem(flags, 'L');
    sizes_flag = isItem(flags, 'S');
}

void get_dir_stat(char *path, struct stat *stat_buf) {
    if (lstat(path, stat_buf) == -1) {
        perror("lstat ERROR");
        exit(3);
    }
}

void print_size(struct stat stat, char *path) {
    if (max_depth == 0 || max_depth == -1)
        return;
    if (bytes_flag)
        write_on_console(stat.st_size, path);
    write_on_console((int) (stat.st_blocks / (block_size / 512.0)), path);
}

void print_dir_size(unsigned size, char *path) {
    if (max_depth == -1)
        return;
    if (bytes_flag)
        write_on_console(size % block_size ? size / block_size + 1 : size / block_size, path);
    else
        write_on_console((int) (size / (block_size / 512.0)), path);
}

unsigned get_size(struct stat stat_buf) {
    if (bytes_flag)
        return stat_buf.st_size;
    return stat_buf.st_blocks;// / (1024 / 512);
}

void recursive_call(char *path) {
    char new_max_depth[10];
    int n = sprintf(new_max_depth, "%d", max_depth == -1 ? -1 : max_depth - 1);
    new_max_depth[n] = 0;
    char block_string[20];
    n = sprintf(block_string, "%d", block_size);
    block_string[n] = 0;
    execl(program, program, flags, path, new_max_depth, block_string, NULL);
}

unsigned simpledu(DIR *dirp) {
    struct dirent *direntp;
    struct stat stat_buf;
    char path[384];
    get_dir_stat(argument, &stat_buf);
    unsigned total_size = get_size(stat_buf);
    while ((direntp = readdir(dirp)) != NULL) {
        if (strcmp(direntp->d_name, ".") == 0 || strcmp(direntp->d_name, "..") == 0)
            continue;
        sprintf(path, "%s/%s", argument, direntp->d_name);
        get_dir_stat(path, &stat_buf);
        if (S_ISDIR(stat_buf.st_mode)) {
            if (fork() == 0) {
                recursive_call(path);
            } else if (!sizes_flag) {
                total_size += read_child_size();
            }
        } else {
            total_size += get_size(stat_buf);
            if (files_flag)
                print_size(stat_buf, path);
        }
    }
    write_size(total_size);
    return total_size;
}

int main(int argc, char **argv) {
    setgid(atoi(getenv("SIMPLEDU_GROUP_ID")));
    signal(SIGINT, sigint_handler);
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