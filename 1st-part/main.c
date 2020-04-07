#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <zconf.h>

char flags[32] = "";
char argument[128] = "";

bool isItem(char *word, char letter) {
    for (int i = 0; i < strlen(word); ++i)
        if (word[i] == letter)
            return true;
    return false;
}

bool separateArgs(int argc, char **argv) {
    char *arg;
    int flagPosition = 1;
    for (int i = 1; i < argc; ++i) {
        arg = argv[i];
        if (arg[0] == '-') {
            for (int j = 1; j < strlen(arg); ++j)
                if (!isItem(flags, arg[j]))
                    flags[flagPosition++] = arg[j];
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
    int out_backup = dup(STDOUT_FILENO);
    char line[128];
    int n = sprintf(line, "%d", out_backup);
    line[n] = 0;
    setenv("BACKUP_STDOUT_FILENO", line, 0);

    if (separateArgs(argc, argv))
        execl("simpledu", "simpledu", flags, argument, NULL);

    exit(0);
}