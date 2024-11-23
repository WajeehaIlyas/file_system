#include "file_operations.h"
#include "disk_manager.h"
#include "fat.h"

int create_file(const char *name, const char *content) {
    Directory *current_directory = &directories[current_directory_index];
    if (current_directory->file_count >= DIRECTORY_SIZE) {
        printf("Directory is full.\n");
        return -1;
    }

    // Check if file already exists
    for (int i = 0; i < current_directory->file_count; i++) {
        if (strcmp(current_directory->files[i].name, name) == 0) {
            printf("File already exists.\n");
            return -1;
        }
    }

    // Find free blocks for the file
    int start_block = find_free_block();
    if (start_block == -1) {
        printf("Not enough space to create file.\n");
        return -1;
    }

    // Create the file entry
    File new_file;
    strncpy(new_file.name, name, MAX_FILE_NAME_SIZE);
    new_file.size = strlen(content);  // Simplified to length of the content
    new_file.start_block = start_block;

    // Save file to root directory
    current_directory->files[current_directory->file_count] = new_file;
    current_directory->file_count++;

    // Write content to the disk
    FILE *disk = fopen(DISK_FILE, "rb+");
    fseek(disk, sizeof(FAT) + sizeof(Directory) + start_block * BLOCK_SIZE, SEEK_SET);
    fwrite(content, 1, new_file.size, disk);
    fclose(disk);

    // Update FAT and write back to disk
    FAT[start_block] = 0; // Mark block as used
    write_to_disk();

    printf("File '%s' created successfully.\n", name);
    return 0;
}

/*
 * List all files in the root directory.
 */

/*
 * Delete a file from the root directory and free its associated blocks in FAT.
 */

void write_to_file(const char *name, const char *new_content) {
    Directory *current_directory = &directories[current_directory_index];

    for (int i = 0; i < current_directory->file_count; i++) {
        if (strcmp(current_directory->files[i].name, name) == 0) {
            File *file = &current_directory->files[i];

            int new_content_size = strlen(new_content);
            if (new_content_size > MAX_FILE_SIZE) {
                printf("Error: File size exceeds the maximum allowed size of 1 KB.\n");
                return;
            }

            int current_block = file->start_block;
            int bytes_written = 0;

            while (bytes_written < new_content_size) {
                int offset = bytes_written % BLOCK_SIZE;
                int bytes_to_write = BLOCK_SIZE - offset; // Write as much as possible in current block
                if (bytes_written + bytes_to_write > new_content_size) {
                    bytes_to_write = new_content_size - bytes_written;
                }

                memcpy(&virtual_disk[current_block][offset], 
                       &new_content[bytes_written], bytes_to_write);
                bytes_written += bytes_to_write;

                // Allocate next block if needed
                if (bytes_written < new_content_size) {
                    if (FAT[current_block] == FREE) {
                        int new_block = find_free_block();
                        if (new_block == -1) {
                            printf("Error: Not enough space to complete the write.\n");
                            file->size = bytes_written; // Update to reflect written content
                            write_to_disk();
                            return;
                        }
                        FAT[current_block] = new_block;
                        FAT[new_block] = FREE;
                    }
                    current_block = FAT[current_block];
                }
            }

            // Truncate unused blocks if new content is smaller
            int next_block = FAT[current_block];
            FAT[current_block] = FREE;
            while (next_block != FREE) {
                int temp = FAT[next_block];
                FAT[next_block] = FREE; // Mark block as free
                next_block = temp;
            }

            file->size = new_content_size; // Update file size
            write_to_disk(); // Persist changes
            printf("File '%s' content overwritten successfully.\n", name);
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
