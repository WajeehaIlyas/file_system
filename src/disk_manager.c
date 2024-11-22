#include "disk_manager.h"
#include "fat.h"

void write_to_disk() {
    FILE *disk = fopen(DISK_FILE, "rb+");
    if (disk == NULL) {
        disk = fopen(DISK_FILE, "wb");
    }
    fseek(disk, 0, SEEK_SET);
    fwrite(FAT, sizeof(FAT), 1, disk);
    fwrite(&directory_count, sizeof(directory_count), 1, disk);
    fwrite(directories, sizeof(Directory), MAX_DIRECTORIES, disk);
    fwrite(virtual_disk, sizeof(virtual_disk), 1, disk);
    fclose(disk);
}

void load_from_disk() {
    FILE *disk = fopen(DISK_FILE, "rb");
    if (!disk) {
        printf("No existing file system found. Initializing new...\n");

        // Initialize FAT and directory structure
        initialize_fat();
        initialize_dir_structure();
        return;
    }

    // Load FAT
    fread(FAT, sizeof(FAT), 1, disk);

    // Load directory structures
    fread(&directory_count, sizeof(directory_count), 1, disk);
    fread(directories, sizeof(Directory), MAX_DIRECTORIES, disk);

    // Load virtual disk
    fread(virtual_disk, sizeof(virtual_disk), 1, disk);

    fclose(disk);
    printf("File system loaded successfully.\n");
}
