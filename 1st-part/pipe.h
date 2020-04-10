#pragma once
#define MAXLINE 1000

void create_pipe();

void close_pipe();

void write_on_log(char *action, char *info);

int write_on_console(unsigned size, char *path);

int read_child_size();

void write_size(unsigned size);

char get_character();