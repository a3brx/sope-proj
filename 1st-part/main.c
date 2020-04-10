#include <sys/times.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <zconf.h>

#define READ 0
#define WRITE 1

char flags[32] = "";
char argument[128] = "";
char max_depth[10] = "-2";
char block_size[20] = "1024";

bool isItem(char *word, char letter) {
    for (int i = 0; i < strlen(word); ++i)
        if (word[i] == letter)
            return true;
    return false;
}

bool separateArgs(int argc, char **argv) {
    char *arg;
    int flag_position = 0;
    bool block_size_changed = false;
    for (int i = 1; i < argc; ++i) {
        arg = argv[i];
        if (strncmp(arg, "--", 2) == 0) {
            if (strncmp(arg + 2, "max-depth=", 10) == 0)
                strcpy(max_depth, arg + 12);
            else if (strncmp(arg + 2, "block-size=", 11) == 0) {
                strcpy(block_size, arg + 13);
                block_size_changed = true;
            } else if (strcmp(arg + 2, "all") == 0)
                flags[flag_position++] = 'a';
            else if (strcmp(arg + 2, "bytes") == 0) {
                flags[flag_position++] = 'b';
                if (!block_size_changed)
                    strcpy(block_size, "1");
            } else if (strcmp(arg + 2, "count-links") == 0)
                flags[flag_position++] = 'l';
            else if (strcmp(arg + 2, "dereference") == 0)
                flags[flag_position++] = 'L';
            else if (strcmp(arg + 2, "separate-dirs") == 0)
                flags[flag_position++] = 'S';
        } else if (arg[0] == '-') {
            if (strcmp(arg, "-B") == 0) {
                strcpy(block_size, argv[++i]);
                block_size_changed = true;
            }
            for (int j = 1; j < strlen(arg); ++j)
                if (!isItem(flags, arg[j])) {
                    flags[flag_position++] = arg[j];
                    if (arg[j] == 'b')
                        strcpy(block_size, "1");
                }
        } else {
            if (strcmp(argument, "") == 0)
                strcpy(argument, arg);
            else
                return false;
        }
    }
    if (strcmp(argument, "") == 0)
        strcpy(argument, ".");
    return true;
}

int main(int argc, char **argv) {
    char line[128];
    struct tms t;
    clock_t time = times(&t);
    int n = sprintf(line, "%ld", time);
    line[n] = 0;
    setenv("SIMPLEDU_PARENT_START", line, 0);
    int out_backup = dup(STDOUT_FILENO);
    n = sprintf(line, "%d", out_backup);
    line[n] = 0;
    setenv("BACKUP_STDOUT_FILENO", line, 0);
    int in_backup = dup(STDIN_FILENO);
    n = sprintf(line, "%d", in_backup);
    line[n] = 0;
    setenv("BACKUP_STDIN_FILENO", line, 0);
    n = sprintf(line, "%d", getgid());
    line[n] = 0;
    setenv("SIMPLEDU_GROUP_ID", line, 0);

    int first_pipe[2];
    pipe(first_pipe);

    if (separateArgs(argc, argv)) {
        dup2(first_pipe[READ], STDIN_FILENO);
        dup2(first_pipe[WRITE], STDOUT_FILENO);
        execl("simpledu_rec", "simpledu_rec", flags, argument, max_depth, block_size, NULL);
    }

    unsetenv("BACKUP_STDOUT_FILENO");
    exit(0);
}
