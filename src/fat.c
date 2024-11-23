#include "fat.h"
char virtual_disk[MAX_BLOCKS][BLOCK_SIZE];
Directory directories[MAX_DIRECTORIES];
int FAT[MAX_BLOCKS];
int directory_count;
int current_directory_index;

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

void initialize_dir_structure() {
    // Initialize the directories array with empty directories
    for (int i = 0; i < MAX_DIRECTORIES; i++) {
        memset(&directories[i], 0, sizeof(Directory));  // Ensure all fields are zeroed
        directories[i].file_count = 0;  // Explicitly set file count
    }

    directory_count = 1; // Start with only the root directory

    // Initialize the root directory
    Directory *root = &directories[0];
    strcpy(root->name, "/");       // Root directory name
    root->parent_index = -1;      // Root has no parent
    root->file_count = 0;         // No files initially
    root->child_count = 0;        // No child directories initially

    // Set the current directory to root
    current_directory_index = 0;
}
