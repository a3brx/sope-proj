#pragma once

#include <stdbool.h>

void create_pipe();

void close_pipe();

int write_on_console(unsigned size, char *path);

int read_child_size();

void write_size(unsigned size);

char get_character();