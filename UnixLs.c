#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

// Function to print details of a file
void printFileDetails(char *filename, struct stat fileStat, int showInode) {
    // Print inode number if requested
    if (showInode) {
        printf("%lu ", (unsigned long)fileStat.st_ino);
    }

    // Print file permissions
    printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
    printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
    printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
    printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf((fileStat.st_mode & S_IXOTH) ? "x " : "- ");

    // Print number of hard links
    printf("%ld ", (long)fileStat.st_nlink);

    // Print owner name
    struct passwd *pw = getpwuid(fileStat.st_uid);
    printf("%s ", pw ? pw->pw_name : "Invalid");

    // Print group name
    struct group *gr = getgrgid(fileStat.st_gid);
    printf("%s ", gr ? gr->gr_name : "Invalid");

    // Print file size
    printf("%lld ", (long long)fileStat.st_size);

    // Print last modified time
    struct tm *tm;
    char buf[100];
    tm = localtime(&fileStat.st_mtime);
    strftime(buf, sizeof(buf), "%b %d %Y %H:%M", tm);
    printf("%s ", buf);

    // Print filename
    printf("%s\n", filename);
}

// Function to check if a path is a directory
int isDirectory(const char *path) {
    struct stat path_stat;
    if (stat(path, &path_stat) == -1) {
        perror("stat");
        exit(1); // Exit with error
    }
    return S_ISDIR(path_stat.st_mode);
}

// Function to list files in a directory
void listFiles(char *dirname, int showInode, int longListing) {
    DIR *dir;
    struct dirent *ent;
    struct stat fileStat;
    char path[1024];

    if ((dir = opendir(dirname)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            // Construct full path to file
            sprintf(path, "%s/%s", dirname, ent->d_name);

            // Skip current and parent directory entries
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }

            // Get file details
            if (stat(path, &fileStat) != -1) {
                // Print file details based on options
                if (longListing) {
                    printFileDetails(ent->d_name, fileStat, showInode);
                } else if (showInode) {
                    printf("%lu %s\n", (unsigned long)fileStat.st_ino, ent->d_name);
                } else {
                    printf("%s\n", ent->d_name);
                }
            } else {
                perror("stat");
            }
        }
        closedir(dir);
    } else {
        perror("opendir");
    }
}

int main(int argc, char *argv[]) {
    int showInode = 0;
    int longListing = 0;

    // Command-line cases

    int i;
    int dir_specified = 0; // Flag to track if directory argument is provided
    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            // Parse options
            int j;
            for (j = 1; argv[i][j] != '\0'; j++) {
                if (argv[i][j] == 'i') {
                    showInode = 1; // Enable inode option
                } else if (argv[i][j] == 'l') {
                    longListing = 1; // Enable long listing option
                } else {
                    fprintf(stderr, "Invalid option: %c\n", argv[i][j]);
                    return 1; // Exit with error
                }
            }
        } else {
            // Process directory argument
            dir_specified = 1; // Directory argument is provided
            struct stat fileStat;
            if (isDirectory(argv[i])) {  // If path is a dir
                listFiles(argv[i], showInode, longListing);
            } else { // If path is a file
                if (stat(argv[i], &fileStat) == -1) {
                    perror("stat");
                    exit(1); // Exit with error
                }
                printFileDetails(argv[i], fileStat, showInode);
            }
        }
    }

    // If no directory argument is provided, list contents of current directory
    if (!dir_specified) {
        listFiles(".", showInode, longListing);
    }

    return 0;
}
