#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include "global_dir.h"

int create_file(const char *name, const char *content);
void write_to_file(const char *name, const char *new_content);
void read_from_file(const char *name);
void truncate_file(const char *name, int new_size);

#endif