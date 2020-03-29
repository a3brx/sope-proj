#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

DIR *openDirectory(char *name) {
    DIR *dirp;
    if ((dirp = opendir(name)) == NULL) {
        perror(name);
        exit(2);
    }
    return dirp;
}

void function(DIR *dirp, char *name) {
    struct dirent *direntp;
    struct stat stat_buf;
    char *str;
    char path[257];
    while ((direntp = readdir(dirp)) != NULL) {
        sprintf(path, "%s/%s", name, direntp->d_name); // <----- NOTAR
        // alternativa a chdir(); ex: anterior
        if (lstat(path, &stat_buf) == -1) {
            perror("lstat ERROR");
            exit(3);
        }
        printf("%10d - ", (int) stat_buf.st_ino);
        if (S_ISREG(stat_buf.st_mode))
            str = "regular";
        else if (S_ISDIR(stat_buf.st_mode)) {
            str = "directory";
            DIR *aux = openDirectory(path);
            function(aux, direntp->d_name);
        } else str = "other";
        printf("%-25s - %s\n", direntp->d_name, str);
    }
}

int main(int argc, char **argv, char **envp) {
    DIR *dirp;
    if (argc < 2)
        argv[1] = ".";
    dirp = openDirectory(argv[1]);
    function(dirp, argv[1]);
    closedir(dirp);
    exit(0);
}