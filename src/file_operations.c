#include "file_operations.h"
#include "disk_manager.h"
#include "fat.h"

int create_file(const char *name, const char *content) {
    // Access the current directory
    Directory *current_directory = &directories[current_directory_index];
    
    // Check if the directory has space for a new file
    if (current_directory->file_count >= DIRECTORY_SIZE) {
        printf("Directory is full.\n");
        return -1;
    }

    // Check if a file with the same name exists in the current directory
    for (int i = 0; i < current_directory->file_count; i++) {
        if (strcmp(current_directory->files[i].name, name) == 0) {
            printf("A file with this name already exists in the current directory.\n");
            return -1;
        }
    }

    // Find a free block to store the file content
    int start_block = find_free_block();
    if (start_block == -1) {
        printf("Error: Not enough space to create the file.\n");
        return -1;
    }

    // Create a new file entry
    File new_file;
    strncpy(new_file.name, name, MAX_FILE_NAME_SIZE);
    new_file.name[MAX_FILE_NAME_SIZE - 1] = '\0'; // Ensure null termination
    new_file.size = strlen(content); // Simplified to the length of content
    new_file.start_block = start_block;

    // Add the file entry to the current directory
    current_directory->files[current_directory->file_count] = new_file;
    current_directory->file_count++;

    // Write the file content to the simulated disk
    FILE *disk = fopen(DISK_FILE, "rb+");
    if (disk == NULL) {
        printf("Error: Unable to access the disk.\n");
        return -1;
    }
    fseek(disk, sizeof(FAT) + sizeof(Directory) * MAX_DIRECTORIES + start_block * BLOCK_SIZE, SEEK_SET);
    fwrite(content, 1, new_file.size, disk);
    fclose(disk);

    // Update the FAT to mark the block as used
    FAT[start_block] = -2; // End-of-file marker
    write_to_disk();

    printf("File '%s' created successfully in the current directory.\n", name);
    return 0;
}

void write_to_file(const char *name, const char *new_content) {
    Directory *current_directory = &directories[current_directory_index];

    for (int i = 0; i < current_directory->file_count; i++) {
        if (strcmp(current_directory->files[i].name, name) == 0) {
            File *file = &current_directory->files[i];

            int new_content_size = strlen(new_content);

            if (new_content_size > MAX_FILE_SIZE * BLOCK_SIZE) {
                printf("Error: File size exceeds maximum limit of 128 KB.\n");
                return;
            }

            // Overwrite the content of the file
            int current_block = file->start_block;
            int bytes_written = 0;

            while (current_block != FREE && bytes_written < new_content_size) {
                int bytes_to_write = (new_content_size - bytes_written < BLOCK_SIZE)
                                     ? new_content_size - bytes_written
                                     : BLOCK_SIZE;

                memcpy(virtual_disk[current_block], &new_content[bytes_written], bytes_to_write);

                bytes_written += bytes_to_write;
                current_block = FAT[current_block];
            }

            // If new content is smaller, clear the remaining blocks
            if (bytes_written < file->size) {
                int remaining_bytes = file->size - bytes_written;
                while (current_block != FREE && remaining_bytes > 0) {
                    int bytes_to_clear = (remaining_bytes < BLOCK_SIZE) ? remaining_bytes : BLOCK_SIZE;
                    memset(virtual_disk[current_block], 0, bytes_to_clear);
                    remaining_bytes -= bytes_to_clear;
                    current_block = FAT[current_block];
                }
            }

            file->size = new_content_size;
            write_to_disk();
            printf("File '%s' overwritten successfully with new content.\n", name);
            return;
        }
    }
    printf("Error: File '%s' not found.\n", name);
}

void read_from_file(const char *name) {
    Directory *current_directory = &directories[current_directory_index];

    for (int i = 0; i < current_directory->file_count; i++) {
        if (strcmp(current_directory->files[i].name, name) == 0) {
            File *file = &current_directory->files[i];

            printf("Reading from file '%s':\n", file->name);
            printf("- Start Block: %d\n", file->start_block);
            printf("- File Size: %d bytes\n", file->size);

            int current_block = file->start_block;
            int bytes_read = 0;

            printf("File Content:\n");
            while (current_block != FREE && bytes_read < file->size) {
                int bytes_to_read = (file->size - bytes_read < BLOCK_SIZE)
                                    ? file->size - bytes_read
                                    : BLOCK_SIZE;

                // Print the content of the current block
                fwrite(virtual_disk[current_block], sizeof(char), bytes_to_read, stdout);
                bytes_read += bytes_to_read;
                current_block = FAT[current_block];
            }

            printf("\nFinished reading file '%s'.\n", file->name);
            return;
        }
    }
    printf("Error: File '%s' not found.\n", name);
}

void truncate_file(const char *name, int new_size) {
    Directory *current_directory = &directories[current_directory_index];

    for (int i = 0; i < current_directory->file_count; i++) {
        if (strcmp(current_directory->files[i].name, name) == 0) {
            File *file = &current_directory->files[i];

            if (new_size > file->size) {
                printf("Error: New size is larger than the current file size.\n");
                return;
            }

            int current_block = file->start_block;
            int total_bytes_processed = 0;
            int bytes_to_keep = new_size; // Amount of data to preserve
            int last_block_to_keep = -1;

            // Traverse blocks to find the point of truncation
            while (current_block != FREE) {
                if (total_bytes_processed + BLOCK_SIZE > bytes_to_keep) {
                    // We found the block where truncation happens
                    int truncate_offset = bytes_to_keep % BLOCK_SIZE;
                    memset(&virtual_disk[current_block * BLOCK_SIZE + truncate_offset], 0,
                           BLOCK_SIZE - truncate_offset);

                    last_block_to_keep = current_block;
                    break;
                }

                total_bytes_processed += BLOCK_SIZE;
                last_block_to_keep = current_block;
                current_block = FAT[current_block];
            }

            // Free remaining blocks in FAT after truncation point
            current_block = FAT[last_block_to_keep];
            FAT[last_block_to_keep] = FREE; // End the file's block chain

            while (current_block != FREE) {
                int next_block = FAT[current_block];
                FAT[current_block] = FREE;
                current_block = next_block;
            }

            // Update file size
            file->size = new_size;
            write_to_disk();
            printf("File '%s' truncated successfully.\n", name);
            return;
        }
    }
    printf("Error: File '%s' not found.\n", name);
}
