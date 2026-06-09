#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#define COLOR_BLUE    "\033[1;34m"
#define COLOR_DEFAULT "\033[0m"

void list_directory(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("nodus: Cannot open directory");
        return;
    }

    struct dirent *entry;
    struct stat file_stat;
    char full_path[512];

    printf("Listing: %s\n\n", path);

    // Loop through every entry inside the directory stream
    while ((entry = readdir(dir)) != NULL) {
        // Skip the working directory shortcuts "." and ".." to keep the UI clean
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construct the full relative path to check file metadata
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        // stat() reads file attributes (like whether it's a folder or file) from the disk
        if (stat(full_path, &file_stat) == 0) {
            if (S_ISDIR(file_stat.st_mode)) {
                // If it's a directory, paint it blue and append a trailing slash
                printf(COLOR_BLUE " %s/\n" COLOR_DEFAULT, entry->d_name);
            } else {
                // If it's a regular file, print it standard
                printf("  %s\n", entry->d_name);
            }
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    // If no path is provided, default to the current working directory "."
    const char *target_dir = (argc > 1) ? argv[1] : ".";
    
    list_directory(target_dir);
    
    return 0;
}
