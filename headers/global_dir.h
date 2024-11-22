#ifndef GLOBAL_DIR_H
#define GLOBAL_DIR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DISK_SIZE (64 * 1024 * 1024)  // 64 MB
#define BLOCK_SIZE 1024               // 1 KB block
#define MAX_BLOCKS (DISK_SIZE / BLOCK_SIZE)  // Number of blocks on the disk
#define MAX_FILE_NAME_SIZE 64
#define MAX_FILE_SIZE 128  // Max file size in KB
#define DIRECTORY_SIZE 128  // Max number of entries in the root directory
#define FREE -1  // Representing free blocks
#define MAX_FILES 100
#define DISK_FILE "disk.fs"

// File Allocation Table (FAT)
extern int FAT[MAX_BLOCKS];
extern char virtual_disk[MAX_BLOCKS][BLOCK_SIZE];

// File and Directory Structures
typedef struct {
    char name[MAX_FILE_NAME_SIZE];
    int size;
    int start_block;
} File;

typedef struct {
    int file_count;
    File files[DIRECTORY_SIZE];
} Directory;

// Global root directory
extern Directory root_directory;

// Function prototypes
void initialize_fat();
int find_free_block();
void write_to_disk();
void load_from_disk();


#endif
