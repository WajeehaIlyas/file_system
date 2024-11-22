#include "global_dir.h"

// Function prototypes
void initialize_disk();
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
    FILE *disk = fopen(DISK_FILE, "rb");
    if (disk == NULL) {
        // Disk does not exist, create and initialize it
        disk = fopen(DISK_FILE, "wb");
        initialize_fat();
        root_directory.file_count = 0;

        // Write initial FAT and directory to the disk
        fwrite(FAT, sizeof(FAT), 1, disk);
        fwrite(&root_directory, sizeof(root_directory), 1, disk);

        // Write empty blocks
        char empty_block[BLOCK_SIZE] = {0};
        for (int i = 0; i < MAX_BLOCKS; i++) {
            fwrite(empty_block, BLOCK_SIZE, 1, disk);
        }
    } else {
        // Load existing FAT and directory
        load_from_disk();
    }
    fclose(disk);
}

/*
 * Simulate a simple file system where we can create and manage files.
 */
void simulate_fs_operations() {
    char command[100];
    printf("Simple FAT File System Simulator\n");
    printf("Available commands: create <filename>, list, delete <filename>,write <filename> <content>, read <filename>, truncate <filename> <new_size>, exit\n");

    while (1) {
        printf("Enter command: ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;  // Remove newline

        if (strncmp(command, "create ", 7) == 0) {
            char filename[MAX_FILE_NAME_SIZE];
            sscanf(command + 7, "%s", filename);
            create_file(filename, "");
        } else if (strcmp(command, "list") == 0) {
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
        else if (strcmp(command, "exit") == 0) {
            break;
        } else {
            printf("Invalid command.\n");
        }
    }
}

