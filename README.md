File system in C 

Code design: 
1. Disk initialization: 
- If disk already exists,it is opened in read-write mode by using load_from_disk function, else new disk is cretaed and opened in write mode. 
- FAT and directory structure is initialized. An array is created for blocks where all blocks and there entires are set to zero.

2. FAT initialization: 
- All blocks in the FAT are initially set as free

3. Directory structure initialization:
- All bytes for each directory are set to zero. Directory count is intialized to one to indicate root directory. Root directory initially has no parent or child and contains no files. Initially, current directory is root directory. 

4. 

Operations:

1. Create file:
- Accesses current directory, check if there's space in it. If yes, then creates a file only if any file of similar name doesn't exist already. Finds free block, creates a new file structure, copies name of file, ensures it's null terminated. 
- The new file entry is added to the current directory and file count is incremented. The file contents are written to disk and FAT is updated. 

2. Write File:
- The size of new conetnt is calculated. If it is greater than already existing content, then file size is updated to new content. 
- If new content is smaller, then new_content size of bytes are overwritten. 
-The changes are written to the disk

3. Read File: 
- Accesses the file, and prints all content, start block and size of file. ( might shift start block and size info into a separate function for file metadata)

4. Truncate file: 
- Truncate offset is calculated and teh size of the file is reduced to that. All bytes after that are marked as free. 
- Changes are written to disk. 

5. List files: 
- Checks if the current directory has any children, if yes iterates through tehm and prnts their names
- If current directory contains any files, iterates through them and prints their names

6. Change directory:
- '..' indicates parent directory. If -1, then already in root directory. If moving to child directory, the children array is searched for, and current directory index is set to the child index

7. Delete: 
- If it's a directory then all files/subdirectories in it are deleted recursively.
- If it is a file, then blocks associated with it are marked as free and the files are moved one position earlier in array so that no gaps are left. 

8. Rename:
- Renames files and directories. Ensures that two files/directories of same name do not exist in the same directory.

9. Read Block: 
- Receives the index of the block to read and prints the contents of that block along with the free space available in it. 

10. Write block: 
- Receives index of block and content to be written. Checks if the block is already in use, if yes then doesn't write to it, else writes the new content and updates the block on disk. 