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
void rename_file(const char *old_name, const char *new_name);
void read_block(int block_index);
void write_block(int block_index, const char *content);
void partition_file_system();
void simulate_fs_operations();

// Main function to interact with the system
int main() {
    initialize_disk();
    simulate_fs_operations();
    return 0;
}

/*
 * Initialize the FAT and root directory and ensure the disk file exists.
 */
void initialize_disk() {
    FILE *disk = fopen(DISK_FILE, "rb+");
    if (disk == NULL) {
        // Disk does not exist, create and initialize it
        printf("Disk does not exist. Initializing a new file system...\n");
        disk = fopen(DISK_FILE, "wb+");
        if (!disk) {
            perror("Error creating disk file");
            exit(1);
        }

        initialize_fat();
        initialize_dir_structure();

        // Write initial FAT and directories to the disk
        fwrite(FAT, sizeof(FAT), 1, disk);
        fwrite(directories, sizeof(directories), 1, disk);

        // Write empty blocks (initialize them to 0)
        char empty_block[BLOCK_SIZE] = {0};  // Initialize to all zeroes
        for (int i = 0; i < MAX_BLOCKS; i++) {
            fwrite(empty_block, BLOCK_SIZE, 1, disk);
        }

        printf("File system initialized and written to disk.\n");
    } else {
        // Load existing FAT and directories
        printf("Existing file system found. Loading from disk...\n");
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

void rename_file(const char *old_name, const char *new_name) {
    Directory *current_directory = &directories[current_directory_index];

    // Check if it's a directory
    for (int i = 0; i < current_directory->child_count; i++) {
        int child_index = current_directory->children[i];
        if (strcmp(directories[child_index].name, old_name) == 0) {
            strcpy(directories[child_index].name, new_name);
            write_to_disk();
            printf("Directory '%s' renamed to '%s'.\n", old_name, new_name);
            return;
        }
    }

    // Check if it's a file
    for (int i = 0; i < current_directory->file_count; i++) {
        if (strcmp(current_directory->files[i].name, old_name) == 0) {
            strcpy(current_directory->files[i].name, new_name);
            write_to_disk();
            printf("File '%s' renamed to '%s'.\n", old_name, new_name);
            return;
        }
    }
    printf("File or directory not found.\n");
}


void read_block(int block_index) {
    if (block_index < 0 || block_index >= MAX_BLOCKS) {
        printf("Error: Invalid block index.\n");
        return;
    }

    int free_bytes = 0;

    printf("Block %d Content:\n", block_index);
    for (int i = 0; i < BLOCK_SIZE; i++) {
        char current_char = virtual_disk[block_index][i];

        // Replace unreadable characters with a placeholder (e.g., '.')
        if (current_char == '\0' || (current_char < 32 || current_char > 126)) {
            free_bytes = BLOCK_SIZE - i;  // Calculate free bytes
            break;
        } else {
            printf("%c", current_char);
        }
    }
    printf("\n");

    printf("Block %d Free Bytes: %d/%d\n", block_index, free_bytes, BLOCK_SIZE);
}

void write_block(int block_index, const char *content) {
    if (block_index < 0 || block_index >= MAX_BLOCKS) {
        printf("Error: Invalid block index.\n");
        return;
    }

    if (FAT[block_index] != FREE) {
        printf("Error: Block %d is already in use by a file.\n", block_index);
        return;
    }

    int content_length = strlen(content);

    // Check if the content fits in the block
    if (content_length > BLOCK_SIZE) {
        printf("Error: Content is too large for the block.\n");
        return;
    }

    // Write content to the virtual disk (update the specified block)
    memset(virtual_disk[block_index], 0, BLOCK_SIZE);
    strncpy(virtual_disk[block_index], content, content_length);

    FAT[block_index] = -2;  // Mark block as used

    // After writing to virtual_disk, persist the changes to the actual disk file
    write_to_disk();  // This will save the changes to the disk

    printf("Block %d successfully updated with content: '%s'.\n", block_index, content);
}



void partition_file_system() {
    // Step 1: Clear Memory Structures
    memset(FAT, FREE, sizeof(FAT)); // Set all FAT entries to FREE
    memset(directories, 0, sizeof(directories)); // Clear all directory entries
    memset(virtual_disk, 0xFF, sizeof(virtual_disk)); // Fill virtual disk with garbage data (0xFF)

    // Step 2: Write Cleared Data to Disk
    FILE *disk = fopen(DISK_FILE, "wb");
    if (disk == NULL) {
        printf("Error: Could not open disk file for partitioning.\n");
        return;
    }
    fwrite(FAT, sizeof(FAT), 1, disk);
    fwrite(directories, sizeof(directories), 1, disk);
    fwrite(virtual_disk, sizeof(virtual_disk), 1, disk);
    fclose(disk);

    // Step 3: Reinitialize the Filesystem
    initialize_fat(); // Reset FAT with initial structure
    initialize_dir_structure(); // Reset directory structure

    // Step 4: Inform the User
    printf("Filesystem partitioned successfully. All data has been cleared.\n");
}


/*
 * Simulate a simple file system where we can create and manage files.
 */
void simulate_fs_operations() {
    char command[100];
    printf("Simple FAT File System Simulator\n");
    printf("Type 'help' to see available commands.\n");

    while (1) {
        printf("Enter command: ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;  // Remove newline

        if (strcmp(command, "help") == 0) {
            printf("Available commands:\n");
            printf("  touch\n");
            printf("  ls\n");
            printf("  rm\n");
            printf("  write\n");
            printf("  read\n");
            printf("  tcate\n");
            printf("  mkdir\n");
            printf("  cd\n");
            printf("  rblock\n");
            printf("  wblock\n");
            printf("  part\n");
            printf("  rname\n");
            printf("  exit\n");
        } else if (strncmp(command, "touch ", 6) == 0) {
            char filename[MAX_FILE_NAME_SIZE];
            sscanf(command + 6, "%s", filename);
            create_file(filename, "");
        } else if (strcmp(command, "ls") == 0) {
            list_files();
        } else if (strncmp(command, "rm ", 3) == 0) {
            char filename[MAX_FILE_NAME_SIZE];
            sscanf(command + 3, "%s", filename);
            delete_file(filename);
        } else if (strncmp(command, "write ", 6) == 0) {
            char name[MAX_FILE_NAME_SIZE];
            char new_content[1024];
            sscanf(command + 6, "%s %[^\n]", name, new_content); // Extract filename and content
            write_to_file(name, new_content);
        } else if (strncmp(command, "read ", 5) == 0) {
            char name[MAX_FILE_NAME_SIZE];
            sscanf(command + 5, "%s", name);
            read_from_file(name);
        } else if (strncmp(command, "tcate ", 6) == 0) {
            char name[MAX_FILE_NAME_SIZE];
            int new_size;
            sscanf(command + 6, "%s %d", name, &new_size);
            truncate_file(name, new_size);
        } else if (strncmp(command, "mkdir ", 6) == 0) {
            char dir_name[MAX_FILE_NAME_SIZE];
            sscanf(command + 6, "%s", dir_name); // Extract directory name
            create_directory(dir_name);
        } else if (strncmp(command, "cd ", 3) == 0) {
            char dir_name[MAX_FILE_NAME_SIZE];
            sscanf(command + 3, "%s", dir_name); // Extract directory name
            change_directory(dir_name);
        } else if (strncmp(command, "rblock ", 7) == 0) {
            int block_index;
            sscanf(command + 7, "%d", &block_index);
            read_block(block_index);
        } else if (strncmp(command, "wblock ", 7) == 0) {
            int block_index;
            char content[1024];
            sscanf(command + 7, "%d %[^\n]", &block_index, content);
            write_block(block_index, content);
        } else if (strcmp(command, "part") == 0) {
            partition_file_system();
        } else if (strncmp(command, "rname ", 6) == 0) {
            char old_name[MAX_FILE_NAME_SIZE];
            char new_name[MAX_FILE_NAME_SIZE];
            sscanf(command + 7, "%s %s", old_name, new_name);
            rename_file(old_name, new_name);
        } else if (strcmp(command, "exit") == 0) {
            break;
        } else {
            printf("Invalid command. Type 'help' to see available commands.\n");
        }
    }
}
