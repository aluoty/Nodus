#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define MAX_FILES 1024
#define PATH_MAX_SIZE 512

#define COLOR_BLUE    "\033[1;34m"
#define COLOR_DEFAULT "\033[0m"
#define STYLE_INVERT  "\033[7m"

// Structure to hold our file metadata in memory
typedef struct {
    char name[256];
    int is_dir;
} FileEntry;

// Turn off terminal buffering and mirroring
void enable_raw_mode(struct termios *orig_termios) {
    struct termios raw = *orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// Restore terminal settings
void disable_raw_mode(struct termios *orig_termios) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, orig_termios);
}

// Draws the interface on every keypress
void draw_interface(const char *current_path, FileEntry files[], int file_count, int selected) {
    // Clear screen (\033[2J) and reset cursor to top-left (\033[H)
    printf("\033[2J\033[H");
    printf("--- nodus v0.1.0 | Location: %s ---\n", current_path);
    printf("Use 'j' (Down), 'k' (Up) to navigate | 'q' to quit\n\n");

    if (file_count == 0) {
        printf("  [Empty Directory]\n");
        return;
    }

    for (int i = 0; i < file_count; i++) {
        // Step 1: Handle Selection Highlight
        if (i == selected) {
            printf(" -> " STYLE_INVERT);
        } else {
            printf("    ");
        }

        // Step 2: Handle Color-coding based on file type
        if (files[i].is_dir) {
            printf(COLOR_BLUE "%s/" COLOR_DEFAULT, files[i].name);
        } else {
            printf("%s", files[i].name);
        }

        // Close the inversion style block if selected
        if (i == selected) {
            printf(COLOR_DEFAULT "\n");
        } else {
            printf("\n");
        }
    }
}

int main(int argc, char *argv[]) {
    const char *target_dir = (argc > 1) ? argv[1] : ".";
    
    // Core memory allocation for directory contents
    FileEntry files[MAX_FILES];
    int file_count = 0;

    // 1. Scan the target directory and load it into our array structure
    DIR *dir = opendir(target_dir);
    if (!dir) {
        perror("nodus: Cannot open directory");
        return 1;
    }

    struct dirent *entry;
    struct stat file_stat;
    char full_path[PATH_MAX_SIZE];

    // Include a manual item to go backward up the directory tree
    strncpy(files[file_count].name, "..", sizeof(files[file_count].name));
    files[file_count].is_dir = 1;
    file_count++;

    while ((entry = readdir(dir)) != NULL && file_count < MAX_FILES) {
        // Skip current dir "." and the default ".." layout loop to avoid duplicates
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construct full tracking path for stat() lookup
        snprintf(full_path, sizeof(full_path), "%s/%s", target_dir, entry->d_name);

        if (stat(full_path, &file_stat) == 0) {
            strncpy(files[file_count].name, entry->d_name, sizeof(files[file_count].name));
            files[file_count].is_dir = S_ISDIR(file_stat.st_mode);
            file_count++;
        }
    }
    closedir(dir);

    // 2. Set up raw terminal environment
    struct termios orig_termios;
    tcgetattr(STDIN_FILENO, &orig_termios);
    enable_raw_mode(&orig_termios);

    int selected = 0;
    char ch;

    // 3. Interactive runtime loop
    while (1) {
        draw_interface(target_dir, files, file_count, selected);

        ch = getchar();

        if (ch == 'q') {
            break;
        } else if (ch == 'j') { // Move Down
            if (selected < file_count - 1) selected++;
        } else if (ch == 'k') { // Move Up
            if (selected > 0) selected--;
        }
    }

    // 4. Graceful Cleanup
    printf("\033[2J\033[H");
    disable_raw_mode(&orig_termios);
    printf("Exiting nodus safely.\n");

    return 0;
}
