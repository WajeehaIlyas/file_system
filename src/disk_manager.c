#include "global_dir.h"

char virtual_disk[MAX_BLOCKS][BLOCK_SIZE];
Directory root_directory;

void write_to_disk() {
    FILE *disk = fopen(DISK_FILE, "rb+");
    if (disk == NULL) {
        disk = fopen(DISK_FILE, "wb");
    }
    fseek(disk, 0, SEEK_SET);
    fwrite(FAT, sizeof(FAT), 1, disk);
    fwrite(&root_directory, sizeof(root_directory), 1, disk);
    fwrite(virtual_disk, sizeof(virtual_disk), 1, disk);
    fclose(disk);
}

// Load the FAT and root directory from the disk file.
void load_from_disk() {
    FILE *disk = fopen(DISK_FILE, "rb");
    if(disk !=NULL){
    fread(FAT, sizeof(FAT), 1, disk);
    fread(&root_directory, sizeof(root_directory), 1, disk);
    fread(virtual_disk, sizeof(virtual_disk), 1, disk);
    fclose(disk);
}
}