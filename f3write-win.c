#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <windows.h>

#define VERSION "9.0-win"
#define DEFAULT_BLOCK_SIZE (1 * 1024 * 1024)  // 1MB blocks
#define MAX_PATH_LENGTH 256

// Fixed-size buffer that is filled with pseudo-random data
static unsigned char *g_buffer;
static size_t g_buffer_size;

// Initialize buffer with pseudo-random data and a marker
void init_buffer(unsigned char *buffer, size_t size, int block_number) {
    // Use block_number as seed for reproducibility
    srand(block_number);
    
    // Fill most of the buffer with pseudo-random data
    for (size_t i = 0; i < size; i++) {
        buffer[i] = rand() % 256;
    }
    
    // Write a marker at the beginning of the buffer that includes the block number
    // This allows f3read to verify the block
    char marker[64];
    sprintf(marker, "F3 Block %d", block_number);
    memcpy(buffer, marker, strlen(marker) + 1);
}

// Format size with appropriate unit
const char* format_size(double *size) {
    static const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    
    while (*size >= 1024 && unit < 4) {
        *size /= 1024;
        unit++;
    }
    
    return units[unit];
}

// Write files to test flash memory
int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: f3write.exe <PATH> [NUM_BLOCKS_MB]\n");
        printf("F3 Write - Test flash memory capacity\n");
        printf("Example: f3write.exe E:\\ 2000\n");
        printf("         (writes 2000MB worth of test data)\n");
        return 1;
    }

    // Parse arguments
    char *path = argv[1];
    int num_blocks = 0;  // 0 means fill the drive
    
    if (argc >= 3) {
        num_blocks = atoi(argv[2]);
        if (num_blocks <= 0) {
            printf("Error: Invalid number of blocks: %s\n", argv[2]);
            return 1;
        }
    }

    // Print header
    printf("F3 Write - Test flash memory capacity v%s\n", VERSION);
    printf("Copyright (C) 2010 Digirati Internet LTDA.\n");
    printf("This is free software; see the source for copying conditions.\n\n");

    // Check if the path exists and is a directory
    DWORD attr = GetFileAttributes(path);
    if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        printf("Error: %s is not a valid directory\n", path);
        return 1;
    }

    // Make sure path ends with backslash
    char full_path[MAX_PATH_LENGTH];
    strncpy(full_path, path, MAX_PATH_LENGTH - 1);
    full_path[MAX_PATH_LENGTH - 1] = '\0';
    
    size_t len = strlen(full_path);
    if (len > 0 && full_path[len - 1] != '\\' && full_path[len - 1] != '/') {
        if (len < MAX_PATH_LENGTH - 1) {
            full_path[len] = '\\';
            full_path[len + 1] = '\0';
        } else {
            printf("Error: Path too long\n");
            return 1;
        }
    }

    // Check available space
    ULARGE_INTEGER free_bytes_available, total_bytes, total_free_bytes;
    if (!GetDiskFreeSpaceEx(full_path, &free_bytes_available, &total_bytes, &total_free_bytes)) {
        printf("Error: Failed to get disk space information for %s\n", full_path);
        return 1;
    }
    
    double free_space = (double)free_bytes_available.QuadPart;
    const char *free_unit = format_size(&free_space);
    
    double total_space = (double)total_bytes.QuadPart;
    const char *total_unit = format_size(&total_space);
    
    printf("Free space: %.2f %s\n", free_space, free_unit);
    printf("Available to write: %.2f %s\n\n", free_space, free_unit);
    
    // Determine number of blocks to write
    uint64_t available_bytes = free_bytes_available.QuadPart;
    uint64_t bytes_to_write;
    
    if (num_blocks == 0) {
        // Use 95% of available space to avoid filling the drive completely
        bytes_to_write = (uint64_t)(available_bytes * 0.95);
    } else {
        bytes_to_write = (uint64_t)num_blocks * 1024 * 1024;
        if (bytes_to_write > available_bytes) {
            printf("Warning: Requested %d MB but only %.2f %s available\n", 
                   num_blocks, free_space, free_unit);
            bytes_to_write = available_bytes;
        }
    }

    // Calculate number of blocks and block size
    g_buffer_size = DEFAULT_BLOCK_SIZE;
    uint64_t num_blocks_to_write = bytes_to_write / g_buffer_size;
    
    // Adjust block size if too many blocks (for progress reporting)
    if (num_blocks_to_write > 10000) {
        g_buffer_size = (size_t)((bytes_to_write + 9999) / 10000);
        // Round to nearest MB for cleaner sizes
        g_buffer_size = ((g_buffer_size + (1024 * 1024) - 1) / (1024 * 1024)) * (1024 * 1024);
        num_blocks_to_write = bytes_to_write / g_buffer_size;
    }
    
    // Allocate buffer
    g_buffer = (unsigned char *)malloc(g_buffer_size);
    if (!g_buffer) {
        printf("Error: Out of memory\n");
        return 1;
    }
    
    // Write blocks
    printf("Writing blocks...\n");
    time_t start_time = time(NULL);
    uint64_t total_written = 0;
    int file_count = 0;
    
    // Progress reporting variables
    int progress_percent = 0;
    int prev_progress = -1;
    
    for (uint64_t i = 0; i < num_blocks_to_write; i++) {
        // Create filename: F3_NNN.txt (NNN = file number)
        char filename[MAX_PATH_LENGTH];
        sprintf(filename, "%sF3_%03d.txt", full_path, file_count);
        
        // Initialize buffer with unique pattern for this block
        init_buffer(g_buffer, g_buffer_size, file_count);
        
        // Open file
        FILE *f = fopen(filename, "wb");
        if (!f) {
            printf("\nError: Could not create file %s\n", filename);
            break;
        }
        
        // Write data
        size_t written = fwrite(g_buffer, 1, g_buffer_size, f);
        fclose(f);
        
        if (written != g_buffer_size) {
            printf("\nError: Could not write full block to %s\n", filename);
            break;
        }
        
        total_written += written;
        file_count++;
        
        // Progress reporting (update every 1%)
        progress_percent = (int)((i * 100) / num_blocks_to_write);
        if (progress_percent != prev_progress) {
            printf("\rProgress: %d%% (%d files)", 
                  progress_percent, file_count);
            fflush(stdout);
            prev_progress = progress_percent;
        }
    }
    printf("\rProgress: 100%% (%d files)   \n", file_count);
    
    // Print summary
    time_t end_time = time(NULL);
    double elapsed = difftime(end_time, start_time);
    
    double written_mb = total_written / (1024.0 * 1024.0);
    double speed_mbps = elapsed > 0 ? written_mb / elapsed : 0;
    
    printf("\nWrote %.2f MB in %.1f seconds, %.2f MB/s\n", 
           written_mb, elapsed, speed_mbps);
    
    // Clean up
    free(g_buffer);
    
    return 0;
}