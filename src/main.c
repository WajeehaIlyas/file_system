#include "global_dir.h"
#include "disk_manager.h"
#include "file_operations.h"
#include "dir_operations.h"
#include "fat.h"


// Function prototypes
void initialize_disk();
void list_files();
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

/*
 * Simulate a simple file system where we can create and manage files.
 */
void simulate_fs_operations() {
    char command[100];
    printf("Simple FAT File System Simulator\n");
    printf("Available commands: touch <filename>, ls, delete <filename>,write <filename> <content>, read <filename>, truncate <filename> <new_size>,mkdir <dir_name>, exit\n");

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
        } else if (strcmp(command, "exit") == 0) {
            break;
        }
        else {
            printf("Invalid command. Please try again.\n");
        }
    }
}

