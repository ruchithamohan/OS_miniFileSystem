//include the header files

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h> // Include the <stdlib.h> header for memory allocation functions
#include <stddef.h> // Include the <stddef.h> header for NULL pointer constant
#include <unistd.h>
#include <sys/stat.h>


//define MACROS

#define MAX_FILE_NAME_LENGTH 50
#define MAX_FILE_CONTENT_LENGTH 1000
#define MAX_DIR_NAME_LENGTH 50
#define MAX_FILES 100
#define MAX_DIRS 100

#define true 1
#define false 0

// Forward declaration of struct Inode
struct Inode;

//Directory
struct Directory {
    char name[MAX_DIR_NAME_LENGTH];
    int numFiles;
    struct Inode *files[MAX_FILES]; // Now an array of pointers to Inode
};

//Superblock
struct Superblock {
    //file system metadata
    int totalFiles;
    struct Directory directories[MAX_DIRS];
    char currentDirectory[MAX_DIR_NAME_LENGTH]; // Variable to store the current directory path
};

// Definition of struct Inode
struct Inode {
    //metadata
    char name[MAX_FILE_NAME_LENGTH];
    char content[MAX_FILE_CONTENT_LENGTH];
    int size;
    int isDirectory;
};

// Function declarations

void createFile(struct Superblock *sb, const char *dirName, const char *fileName);
void makeDirectory(struct Superblock *sb, const char *dirName);
void echo(struct Superblock *sb, const char *dirName, const char *fileName, const char *content);
void readFile(struct Superblock *sb, const char *dirName, const char *fileName);
void listFiles(struct Superblock *sb, const char *dirName);
void changeDirectory(struct Superblock *sb, const char *dirName) ;
void copyFile(struct Superblock *sb, const char *srcDir, const char *destDir, const char *fileName) ;
void renameFile(struct Superblock *sb, const char *dirName, const char *oldFileName, const char *newFileName);
void renameDirectory(struct Superblock *sb, const char *oldDirName, const char *newDirName);
void findFile(struct Superblock *sb, const char *fileName);
void printCurrentDirectoryPath(const char *currentDir) ;
void removeDirectory(struct Superblock *sb, const char *dirName);
void removeFile(struct Superblock *sb, const char *dirName, const char *fileName) ;


// Function definitions

//CREATE A FILE
void createFile(struct Superblock *sb, const char *dirName, const char *fileName) {
    // Find the directory
    for (int i = 0; i < sb->totalFiles; i++) {
        if (strcmp(sb->directories[i].name, dirName) == 0) {
            // Check if there's space for a new file in this directory
            if (sb->directories[i].numFiles < MAX_FILES) {
                // Construct the full path
                char fullPath[MAX_DIR_NAME_LENGTH + MAX_FILE_NAME_LENGTH + 2]; // 2 for '/' and null terminator
                snprintf(fullPath, sizeof(fullPath), "%s/%s", dirName, fileName);

                // Open the file for writing
                FILE *file = fopen(fullPath, "w");
                if (file == NULL) {
                    printf("Failed to create file '%s'.\n", fileName);
                    return;
                }

                // Close the file
                fclose(file);

                // Allocate memory for a new Inode instance
                struct Inode *newFile = malloc(sizeof(struct Inode));
                if (newFile == NULL) {
                    printf("Memory allocation failed.\n");
                    return;
                }

                // Initialize the new Inode instance
                strcpy(newFile->name, fileName);
                newFile->size = 0;
                newFile->isDirectory = false;

                // Add the file to the directory
                sb->directories[i].files[sb->directories[i].numFiles++] = newFile;
                sb->totalFiles++;

                printf("File '%s' created in directory '%s'.\n", fileName, dirName);
                return;
            } else {
                printf("Directory '%s' is full.\n", dirName);
            }
            break;
        }
    }
    printf("Directory '%s' not found.\n", dirName);
}



//CREATE A DIRECTORY
void makeDirectory(struct Superblock *sb, const char *dirName) {
  
        struct Directory newDir;
        strcpy(newDir.name, dirName);
        newDir.numFiles = 0;
        sb->directories[sb->totalFiles++] = newDir;

        // Create the directory
        if (mkdir(dirName, 0777) == 0) { // 0777 gives full permissions, adjust as needed
            printf("Directory '%s' created.\n", dirName);
        } else {
            printf("Failed to create directory '%s'.\n", dirName);
        }
    
}

//WRITE CONTENT ONTO A FILE
void echo(struct Superblock *sb, const char *dirName, const char *fileName, const char *content) {
    // Find the directory
    for (int i = 0; i < sb->totalFiles; i++) {
        if (strcmp(sb->directories[i].name, dirName) == 0) {
            // Search for the file in the directory
            for (int j = 0; j < sb->directories[i].numFiles; j++) {
                if (strcmp(sb->directories[i].files[j]->name, fileName) == 0) {
                    // Construct the full path to the file
                    char fullPath[MAX_DIR_NAME_LENGTH + MAX_FILE_NAME_LENGTH + 2]; // 2 for '/' and null terminator
                    snprintf(fullPath, sizeof(fullPath), "%s/%s", dirName, fileName);

                    // Open the file for writing
                    FILE *file = fopen(fullPath, "w");
                    if (file == NULL) {
                        printf("Failed to open file '%s' in directory '%s'.\n", fileName, dirName);
                        return;
                    }

                    // Write content to the file
                    fprintf(file, "%s", content);

                    // Close the file
                    fclose(file);

                    printf("Content written to file '%s' in directory '%s'.\n", fileName, dirName);
                    return;
                }
            }
            // File not found in the directory
            printf("File '%s' not found in directory '%s'.\n", fileName, dirName);
            return;
        }
    }
    // Directory not found
    printf("Directory '%s' not found.\n", dirName);
}

//READ A FILE'S CONTENTS
void readFile(struct Superblock *sb, const char *dirName, const char *fileName) {
    // Find the directory
    for (int i = 0; i < sb->totalFiles; i++) {
        if (strcmp(sb->directories[i].name, dirName) == 0) {
            // Check if the file exists in the directory
            for (int j = 0; j < sb->directories[i].numFiles; j++) {
                if (strcmp(sb->directories[i].files[j]->name, fileName) == 0) {
                    // Construct the full file path
                    char filePath[MAX_DIR_NAME_LENGTH + MAX_FILE_NAME_LENGTH + 2]; // +2 for '/' and '\0'
                    snprintf(filePath, sizeof(filePath), "%s/%s", dirName, fileName);

                    // Open the file for reading
                    FILE *file = fopen(filePath, "r");
                    if (file == NULL) {
                        printf("Failed to open file '%s'.\n", fileName);
                        return;
                    }

                    // Read content from the file
                    char content[MAX_FILE_CONTENT_LENGTH];
                    while (fgets(content, MAX_FILE_CONTENT_LENGTH, file) != NULL) {
                        printf("%s", content);
                    }

                    // Close the file
                    fclose(file);
                    return;
                }
            }
            printf("File '%s' not found in directory '%s'.\n", fileName, dirName);
            return;
        }
    }
    printf("Directory '%s' not found.\n", dirName);
}

//LIST FILES IN A DIRECTORY
void listFiles(struct Superblock *sb, const char *dirName) {
    printf("Attempting to list files in directory '%s'\n", dirName);
    for (int i = 0; i < sb->totalFiles; i++) {
        printf("Checking directory '%s'\n", sb->directories[i].name);
        if (strcmp(sb->directories[i].name, dirName) == 0) {
            printf("Directory '%s' found\n", dirName);
            printf("Files in directory '%s':\n", dirName);
            for (int j = 0; j < sb->directories[i].numFiles; j++) {
                printf("- %s\n", sb->directories[i].files[j]->name);
            }
            return;
        }
    }
    printf("Directory '%s' not found.\n", dirName);
}

//CHANGE DIRECTORY
void changeDirectory(struct Superblock *sb, const char *dirName) {
    for (int i = 0; i < sb->totalFiles; i++) {
        if (strcmp(sb->directories[i].name, dirName) == 0) {
            // Update the current directory path
            strcpy(sb->currentDirectory, dirName);
            printf("Changed directory to '%s'\n", dirName);
            return;
        }
    }
    printf("Directory '%s' not found.\n", dirName);
}

//COPY FILE & ITS CONTENTS FROM ONE DIRECTORY TO ANOTHER
void copyFile(struct Superblock *sb, const char *srcDir, const char *destDir, const char *fileName) {
    struct Inode *srcFile = NULL;
    int fileFound = false;

    // Find the source file
    for (int i = 0; i < sb->totalFiles; i++) {
        if (strcmp(sb->directories[i].name, srcDir) == 0) {
            for (int j = 0; j < sb->directories[i].numFiles; j++) {
                if (strcmp(sb->directories[i].files[j]->name, fileName) == 0) {
                    srcFile = sb->directories[i].files[j];
                    fileFound = true;
                    break;
                }
            }
            if (fileFound) break;
        }
    }

    if (!fileFound) {
        printf("File '%s' not found in directory '%s'.\n", fileName, srcDir);
        return;
    }

    // Check if the destination directory exists
    int destDirIndex = -1;
    for (int i = 0; i < sb->totalFiles; i++) {
        if (strcmp(sb->directories[i].name, destDir) == 0) {
            destDirIndex = i;
            break;
        }
    }

    if (destDirIndex == -1) {
        printf("Directory '%s' not found.\n", destDir);
        return;
    }

    // Open the source file for reading
    char srcFilePath[MAX_DIR_NAME_LENGTH + MAX_FILE_NAME_LENGTH + 2];  // 1 for '/' and 1 for null terminator
    snprintf(srcFilePath, sizeof(srcFilePath), "%s/%s", srcDir, fileName);
    FILE *srcFilePtr = fopen(srcFilePath, "r");
    if (srcFilePtr == NULL) {
        printf("Failed to open source file '%s' in directory '%s'.\n", fileName, srcDir);
        return;
    }

    // Open the destination file for writing
    char destFilePath[MAX_DIR_NAME_LENGTH + MAX_FILE_NAME_LENGTH + 2];  // 1 for '/' and 1 for null terminator
    snprintf(destFilePath, sizeof(destFilePath), "%s/%s", destDir, fileName);
    FILE *destFilePtr = fopen(destFilePath, "w");
    if (destFilePtr == NULL) {
        printf("Failed to create file '%s' in directory '%s'.\n", fileName, destDir);
        fclose(srcFilePtr); // Close the source file
        return;
    }

    // Copy contents from source file to destination file
    int ch;
    while ((ch = fgetc(srcFilePtr)) != EOF) {
        fputc(ch, destFilePtr);
    }

    // Close the files
    fclose(srcFilePtr);
    fclose(destFilePtr);

    // Update the directory structure in the superblock
    sb->directories[destDirIndex].files[sb->directories[destDirIndex].numFiles++] = srcFile;

    printf("File '%s' copied from directory '%s' to directory '%s'.\n", fileName, srcDir, destDir);
}

// RENAME A FILE
void renameFile(struct Superblock *sb, const char *dirName, const char *oldFileName, const char *newFileName) {
    for (int i = 0; i < sb->totalFiles; i++) {
        if (strcmp(sb->directories[i].name, dirName) == 0) {
            for (int j = 0; j < sb->directories[i].numFiles; j++) {
                if (strcmp(sb->directories[i].files[j]->name, oldFileName) == 0) {
                    // Rename the file on disk
                    char oldPath[MAX_DIR_NAME_LENGTH + MAX_FILE_NAME_LENGTH + 2]; // 2 for '/' and null terminator
                    char newPath[MAX_DIR_NAME_LENGTH + MAX_FILE_NAME_LENGTH + 2];
                    snprintf(oldPath, sizeof(oldPath), "%s/%s", dirName, oldFileName);
                    snprintf(newPath, sizeof(newPath), "%s/%s", dirName, newFileName);

                    if (rename(oldPath, newPath) != 0) {
                        printf("Failed to rename file '%s' to '%s' on disk.\n", oldFileName, newFileName);
                        return;
                    }

                    // Update the file's name in metadata
                    strcpy(sb->directories[i].files[j]->name, newFileName);

                    printf("File '%s' renamed to '%s' in directory '%s'.\n", oldFileName, newFileName, dirName);
                    return;
                }
            }
            printf("File '%s' not found in directory '%s'.\n", oldFileName, dirName);
            return;
        }
    }
    printf("Directory '%s' not found.\n", dirName);
}

//FIND A FILE
void findFile(struct Superblock *sb, const char *fileName) {
    int found = false;
    for (int i = 0; i < sb->totalFiles; i++) {
        for (int j = 0; j < sb->directories[i].numFiles; j++) {
            if (strcmp(sb->directories[i].files[j]->name, fileName) == 0) {
                printf("File '%s' found in directory '%s'.\n", fileName, sb->directories[i].name);
                found = true;
            }
        }
    }
    if (!found) {
        printf("File '%s' not found.\n", fileName);
    }
}

//PRINT LATEST DIRECTORY ACCESSED
void printCurrentDirectoryPath(const char *currentDir) {
    printf("Current directory path: %s\n", currentDir);
}

//REMOVE A DIRECTORY
void removeDirectory(struct Superblock *sb, const char *dirName) {
    // Find the directory
    for (int i = 0; i < sb->totalFiles; i++) {
        if (strcmp(sb->directories[i].name, dirName) == 0) {
            // Remove all files in the directory
            for (int j = 0; j < sb->directories[i].numFiles; j++) {
                char filePath[MAX_DIR_NAME_LENGTH + MAX_FILE_NAME_LENGTH + 2]; // +2 for '/' and '\0'
                snprintf(filePath, sizeof(filePath), "%s/%s", dirName, sb->directories[i].files[j]->name);
                if (remove(filePath) != 0) {
                    printf("Failed to remove file '%s' from directory '%s'.\n", sb->directories[i].files[j]->name, dirName);
                    return;
                }
                free(sb->directories[i].files[j]); // Free the memory allocated for the file
            }
            sb->directories[i].numFiles = 0; // Reset the number of files in the directory

            // Remove the directory from the Superblock
            for (int k = i; k < sb->totalFiles - 1; k++) {
                sb->directories[k] = sb->directories[k + 1];
            }
            sb->totalFiles--;

            // Delete the directory itself from the file system
            if (remove(dirName) != 0) {
                printf("Failed to remove directory '%s'.\n", dirName);
                return;
            }

            printf("Directory '%s' removed.\n", dirName);
            return;
        }
    }
    printf("Directory '%s' not found.\n", dirName);
}

//REMOVE A FILE
void removeFile(struct Superblock *sb, const char *dirName, const char *fileName) {
    for (int i = 0; i < sb->totalFiles; i++) {
        if (strcmp(sb->directories[i].name, dirName) == 0) {
            for (int j = 0; j < sb->directories[i].numFiles; j++) {
                if (strcmp(sb->directories[i].files[j]->name, fileName) == 0) {
                    // Remove the file from the directory by shifting elements
                    free(sb->directories[i].files[j]);
                    for (int k = j; k < sb->directories[i].numFiles - 1; k++) {
                        sb->directories[i].files[k] = sb->directories[i].files[k + 1];
                    }
                    sb->directories[i].numFiles--;
                    sb->totalFiles--;

                    // Remove the file from the filesystem
                    char filePath[MAX_DIR_NAME_LENGTH + MAX_FILE_NAME_LENGTH + 2]; // +2 for '/' and '\0'
                    snprintf(filePath, sizeof(filePath), "%s/%s", dirName, fileName);
                    if (remove(filePath) != 0) {
                        printf("Failed to remove file '%s' from directory '%s'.\n", fileName, dirName);
                        return;
                    }

                    printf("File '%s' removed from directory '%s'.\n", fileName, dirName);
                    return;
                }
            }
            printf("File '%s' not found in directory '%s'.\n", fileName, dirName);
            return;
        }
    }
    printf("Directory '%s' not found.\n", dirName);
}

//RENAME A DIRECTORY
void renameDirectory(struct Superblock *sb, const char *oldDirName, const char *newDirName) {
    for (int i = 0; i < sb->totalFiles; i++) {
        if (strcmp(sb->directories[i].name, oldDirName) == 0) {
            // Rename the directory on disk
            char oldPath[MAX_DIR_NAME_LENGTH + 1]; // 1 for null terminator
            char newPath[MAX_DIR_NAME_LENGTH + 1];
            snprintf(oldPath, sizeof(oldPath), "%s", oldDirName);
            snprintf(newPath, sizeof(newPath), "%s", newDirName);

            if (rename(oldPath, newPath) != 0) {
                printf("Failed to rename directory '%s' to '%s' on disk.\n", oldDirName, newDirName);
                return;
            }

            // Update the directory name in metadata
            strcpy(sb->directories[i].name, newDirName);

            printf("Directory '%s' renamed to '%s'.\n", oldDirName, newDirName);
            return;
        }
    }
    printf("Directory '%s' not found.\n", oldDirName);
}

//MAIN PROGRAM
int main() {

    //initialise superblock
    struct Superblock sb;
    sb.totalFiles = 0;

    
    printf("Initialization complete.\n");

    while (true) {
        printf("\n\nWaiting for user input...\n");
        sleep(2);

        //variables

        int option;

        char dirName[MAX_DIR_NAME_LENGTH], fileName[MAX_FILE_NAME_LENGTH], content[MAX_FILE_CONTENT_LENGTH];

        char srcDir[MAX_DIR_NAME_LENGTH], destDir[MAX_DIR_NAME_LENGTH], oldFileName[MAX_FILE_NAME_LENGTH], newFileName[MAX_FILE_NAME_LENGTH];
        printf("           _       _       _____ _ _      ____            _                 \n");
        printf(" _ __ ___ (_)_ __ (_)     |  ___(_) | ___/ ___| _   _ ___| |_ ___ _ __ ___  \n");
        printf("| '_ ` _ \\| | '_ \\| |_____| |_  | | |/ _ \\___ \\| | | / __| __/ _ \\ '_ ` _ \\ \n");
        printf("| | | | | | | | | | |_____|  _| | | |  __/___) | |_| \\__ \\ | | __/ | | | | |\n");
        printf("|_| |_| |_|_|_| |_|_|     |_|   |_|_|\\___|____/ \\__, |___/\\__\\___|_| |_| |_|\n");
        printf("                                                |___/\n");

        // Display the command list

        printf("1. ls [Dir]...\t\t\tlist files under directories.\n");
        printf("2. cd [Dir]\t\t\tchange to [Dir].\n");
        printf("3. cp [file]...[Dir]\t\tcopy files to [Dir].\n");
        printf("4. rm [file]...\t\t\tremove files.\n");
        printf("5. rmdir [Dir]...\t\tremove directories.\n");
        printf("6. touch [file]...\t\tcreate files.\n");
        printf("7. mkdir [Dir]...\t\tcreate directories.\n");
        printf("8. mv [file] [filename]\t\trename the file.\n");
        printf("9. mvdir [Dir] [directoryname]\trename the directory.\n");
        printf("10. cat [file]...\t\tread files.\n");
        printf("11. pwd\t\t\t\tprint the current directory path.\n");
        printf("12. echo [content] > [file]\twrite content to the file.\n");
        printf("13. find [Dir] [file/Dir]\tfind specific file or directory.\n");
        printf("0. Exit\n");
       

        printf("Enter your option: ");
        scanf("%d", &option);

        // Debugging print statement
        printf("Option selected: %d\n", option);

        switch (option) {
            // Cases for different options
            case 1:
                printf("Enter directory name: ");
                scanf("%s", dirName);
                listFiles(&sb, dirName);
                break;

            case 2:
                printf("Enter directory name: ");
                scanf("%s", dirName);
                changeDirectory(&sb, dirName);
                break;

            case 3:
                printf("Enter source directory name: ");
                scanf("%s", srcDir);
                printf("Enter destination directory name: ");
                scanf("%s", destDir);
                printf("Enter file name: ");
                scanf("%s", fileName);
                copyFile(&sb, srcDir, destDir, fileName);
                break;

            case 4:
                printf("Enter directory name: ");
                scanf("%s", dirName);
                printf("Enter file name: ");
                scanf("%s", fileName);
                removeFile(&sb, dirName, fileName);
                break;

            case 5:
                printf("Enter directory name: ");
                scanf("%s", dirName);
                removeDirectory(&sb, dirName);
                break;

            case 6:
                printf("Enter directory name: ");
                scanf("%s", dirName);
                printf("Enter file name: ");
                scanf("%s", fileName);
                createFile(&sb, dirName, fileName);
                break;

            case 7:
                printf("Enter directory name: ");
                scanf("%s", dirName);
                makeDirectory(&sb, dirName);
                break;

            case 8:
                printf("Enter directory name: ");
                scanf("%s", dirName);
                printf("Enter old file name: ");
                scanf("%s", oldFileName);
                printf("Enter new file name: ");
                scanf("%s", newFileName);
                renameFile(&sb, dirName, oldFileName, newFileName);
                break;

            case 9:
                printf("Enter old directory name: ");
                scanf("%s", dirName);
                printf("Enter new directory name: ");
                scanf("%s", newFileName);
                renameDirectory(&sb, dirName, newFileName);
                break;

            case 10:
                printf("Enter directory name: ");
                scanf("%s", dirName);
                printf("Enter file name: ");
                scanf("%s", fileName);
                readFile(&sb, dirName, fileName);
                break;
                
            case 11:
                
                printCurrentDirectoryPath(dirName);
                break;

            case 12:
                printf("Enter directory name: ");
                scanf("%s", dirName);
                printf("Enter file name: ");
                scanf("%s", fileName);
                printf("Enter content: ");
                scanf("%s", content);
                echo(&sb, dirName, fileName, content);
                break;

            case 13:
                printf("Enter directory name: ");
                scanf("%s", dirName);
                printf("Enter file/directory name: ");
                scanf("%s", fileName);
                findFile(&sb, fileName);
                break;

            case 0:
                printf("Exiting...\n");
                return 0;

            default:
                printf("Invalid option!\n");
                break;

        }

    }

    return 0;
}
