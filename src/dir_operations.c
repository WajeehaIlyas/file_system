#include "dir_operations.h"
#include "disk_manager.h"

void create_directory(const char *name) {
    // Check if max directory limit is reached
    if (directory_count >= MAX_DIRECTORIES) {
        printf("Error: Maximum directory limit reached.\n");
        return;
    }

    // Check for duplicate directory name
    Directory *current_dir = &directories[current_directory_index];
    for (int i = 0; i < current_dir->child_count; i++) {
        if (strcmp(directories[current_dir->children[i]].name, name) == 0) {
            printf("Error: Directory '%s' already exists.\n", name);
            return;
        }
    }

    // Create a new directory
    Directory *new_dir = &directories[directory_count];
    strncpy(new_dir->name, name, MAX_FILE_NAME_SIZE);
    new_dir->parent_index = current_directory_index;
    new_dir->file_count = 0;
    new_dir->child_count = 0;

    // Add to current directory's child list
    current_dir->children[current_dir->child_count] = directory_count;
    current_dir->child_count++;

    directory_count++;

    printf("Directory '%s' created successfully.\n", name);
    write_to_disk(); // Save changes to disk
}
