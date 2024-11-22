#ifndef FAT_H
#define FAT_H

#include "global_dir.h"

void initialize_fat();
int find_free_block();
void initialize_dir_structure();

#endif