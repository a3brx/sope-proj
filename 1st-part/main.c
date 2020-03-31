#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <zconf.h>
#include <wait.h>

char flags[32] = "-";
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
    DIR *dirp;
    struct dirent *direntp;
    struct stat stat_buf;
    char path[384];

    separateArgs(argc, argv);

    if ((dirp = opendir(argument)) == NULL) {
        perror(argument);
        exit(2);
    }
    while ((direntp = readdir(dirp)) != NULL) {
        if (strcmp(direntp->d_name, ".") == 0 || strcmp(direntp->d_name, "..") == 0)
            continue;
        sprintf(path, "%s/%s", argument, direntp->d_name);
        if (lstat(path, &stat_buf) == -1) {
            perror("lstat ERROR");
            exit(3);
        }
        if (S_ISDIR(stat_buf.st_mode)) {
            printf("%-25s\n", path);
            if (fork() == 0)
                execl(argv[0], argv[0], flags, path, NULL);
        }
    }
    int ret;
    while (waitpid(-1, &ret, WNOHANG) >= 0);
    closedir(dirp);
    exit(0);
}