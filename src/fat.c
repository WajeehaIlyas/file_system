#include "global_dir.h"

int FAT[MAX_BLOCKS];

// Initialize the FAT, marking all blocks as free.
void initialize_fat() {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        FAT[i] = FREE;
    }
}

// Find a free block in the FAT to allocate for a new file.
int find_free_block() {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (FAT[i] == FREE) {
            return i;
        }
    }
    return -1;  // No free blocks available
}

