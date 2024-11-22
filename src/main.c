#include "global_dir.h"
#include "disk_manager.h"
#include "file_operations.h"
#include "dir_operations.h"
#include "fat.h"


// Function prototypes
void initialize_disk();
void list_files();
void change_directory(const char *name);
void delete_file(const char *name);
void delete_directory_recursive(int dir_index);
void simulate_fs_operations();

// Main function to interact with the system
int main() {
    initialize_disk();
    initialize_dir_structure();
    simulate_fs_operations();
    return 0;
}

/*
 * Initialize the FAT and root directory and ensure the disk file exists.
 */
void initialize_disk() {
    FILE *disk = fopen(DISK_FILE, "rb");
    if (disk == NULL) {
        // Disk does not exist, create and initialize it
        disk = fopen(DISK_FILE, "wb");
        initialize_fat();

        // Initialize the directories array with empty directories
        for (int i = 0; i < MAX_DIRECTORIES; i++) {
            directories[i].file_count = 0;  // Initialize each directory with 0 files
        }

        // Write initial FAT and directories to the disk
        fwrite(FAT, sizeof(FAT), 1, disk);
        fwrite(directories, sizeof(directories), 1, disk);

        // Write empty blocks
        char empty_block[BLOCK_SIZE] = {0};
        for (int i = 0; i < MAX_BLOCKS; i++) {
            fwrite(empty_block, BLOCK_SIZE, 1, disk);
        }
    } else {
        // Load existing FAT and directories
        load_from_disk();
    }
    fclose(disk);
}

void list_files() {
    Directory *current_directory = &directories[current_directory_index];

    if (current_directory->child_count == 0) {
        printf("No directories in the current directory.\n");
    } else {
        printf("Directories in current directory:\n");
        for (int i = 0; i < current_directory->child_count; i++) {
            int child_index = current_directory->children[i];
            printf("- %s (Directory)\n", directories[child_index].name);
        }
    }

    if (current_directory->file_count == 0) {
        printf("No files in the directory.\n");
        return;
    }

    printf("Files in root directory:\n");
    for (int i = 0; i < current_directory->file_count; i++) {
        printf("- %s (Size: %d bytes)\n", current_directory->files[i].name, current_directory->files[i].size);
    }
}

void change_directory(const char *name) {
    Directory *current_directory = &directories[current_directory_index];

    if (strcmp(name, "..") == 0) {
        // Move to parent directory
        if (current_directory->parent_index == -1) {
            printf("Already in root directory.\n");
            return;
        }
        current_directory_index = current_directory->parent_index;
        printf("Moved to parent directory.\n");
    } else {
        // Move to child directory
        for (int i = 0; i < current_directory->child_count; i++) {
            int child_index = current_directory->children[i];
            if (strcmp(directories[child_index].name, name) == 0) {
                current_directory_index = child_index;
                printf("Moved to directory '%s'.\n", name);
                return;
            }
        }
        printf("Error: Directory '%s' not found.\n", name);
    }
}

void delete_file(const char *name) {
    Directory *current_directory = &directories[current_directory_index];

    // Check if it's a directory
    for (int i = 0; i < current_directory->child_count; i++) {
        int child_index = current_directory->children[i];
        if (strcmp(directories[child_index].name, name) == 0) {
            // Recursively delete all files and subdirectories
            delete_directory_recursive(child_index);

            // Shift remaining directories down
            for (int j = i; j < current_directory->child_count - 1; j++) {
                current_directory->children[j] = current_directory->children[j + 1];
            }

            current_directory->child_count--;
            write_to_disk();
            printf("Directory '%s' deleted successfully.\n", name);
            return;
        }
    }

    // Check if it's a file
    for (int i = 0; i < current_directory->file_count; i++) {
        if (strcmp(current_directory->files[i].name, name) == 0) {
            // Mark the block as free
            FAT[current_directory->files[i].start_block] = FREE;

            // Shift remaining files down
            for (int j = i; j < current_directory->file_count - 1; j++) {
                current_directory->files[j] = current_directory->files[j + 1];
            }

            current_directory->file_count--;
            write_to_disk();
            printf("File '%s' deleted successfully.\n", name);
            return;
        }
    }
    printf("File or directory not found.\n");
}

void delete_directory_recursive(int dir_index) {
    Directory *dir = &directories[dir_index];

    // Delete all files in the directory
    for (int i = 0; i < dir->file_count; i++) {
        FAT[dir->files[i].start_block] = FREE;
    }

    // Recursively delete all subdirectories
    for (int i = 0; i < dir->child_count; i++) {
        delete_directory_recursive(dir->children[i]);
    }

    // Mark the directory as empty
    dir->file_count = 0;
    dir->child_count = 0;
}


/*
 * Simulate a simple file system where we can create and manage files.
 */
void simulate_fs_operations() {
    char command[100];
    printf("Simple FAT File System Simulator\n");
    printf("Available commands: touch <filename>, ls, delete <filename>,write <filename> <content>, read <filename>, truncate <filename> <new_size>,mkdir <dir_name>, cd <dir_name>, exit\n");

    while (1) {
        printf("Enter command: ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;  // Remove newline

        if (strncmp(command, "touch ", 6) == 0) {
            char filename[MAX_FILE_NAME_SIZE];
            sscanf(command + 6, "%s", filename);
            create_file(filename, "");
        } else if (strcmp(command, "ls") == 0) {
            list_files();
        } else if (strncmp(command, "delete ", 7) == 0) {
            char filename[MAX_FILE_NAME_SIZE];
            sscanf(command + 7, "%s", filename);
            delete_file(filename);
        }else if (strncmp(command, "write ", 6) == 0) {
    char name[MAX_FILE_NAME_SIZE];
    char new_content[1024];
    sscanf(command + 6, "%s %[^\n]", name, new_content); // Extract filename and content
    write_to_file(name, new_content);
} else if (strncmp(command, "read ", 5) == 0) {
    char name[MAX_FILE_NAME_SIZE];
    sscanf(command + 5, "%s", name);
    read_from_file(name);
}
else if (strncmp(command, "truncate ", 9) == 0) {
    char name[MAX_FILE_NAME_SIZE];
    int new_size;
    sscanf(command + 9, "%s %d", name, &new_size);
    truncate_file(name, new_size);
}
else if (strncmp(command, "mkdir ", 6) == 0) {
            char dir_name[MAX_FILE_NAME_SIZE];
            sscanf(command + 6, "%s", dir_name); // Extract directory name
            create_directory(dir_name);
        }
        else if (strncmp(command, "cd ", 3) == 0) {
            char dir_name[MAX_FILE_NAME_SIZE];
            sscanf(command + 3, "%s", dir_name); // Extract directory name
            change_directory(dir_name);
        } 
        else if (strcmp(command, "exit") == 0) {
            break;
        }
        else {
            printf("Invalid command. Please try again.\n");
        }
    }
}



