#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#define VERSION "9.0-win"
#define SECTOR_SIZE 512
#define BLOCK_SIZE (1 << 20)  // 1MB blocks
#define PATTERN_SIZE 64

// Simplified fake type enum
typedef enum {
    FAKE_TYPE_GOOD,
    FAKE_TYPE_POSSIBLY_FAKE,
    FAKE_TYPE_DAMAGED
} FakeType;

// Random pattern generator
void fill_pattern(unsigned char *buffer, size_t size, uint64_t seed) {
    srand((unsigned int)seed);
    for (size_t i = 0; i < size; i++) {
        buffer[i] = rand() % 256;
    }
}

// Get drive size in bytes
uint64_t get_drive_size(HANDLE hDevice) {
    DISK_GEOMETRY_EX diskGeometry;
    DWORD bytesReturned;

    if (DeviceIoControl(
        hDevice,
        IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
        NULL,
        0,
        &diskGeometry,
        sizeof(diskGeometry),
        &bytesReturned,
        NULL))
    {
        return diskGeometry.DiskSize.QuadPart;
    }

    // Alternative method if the above fails
    GET_LENGTH_INFORMATION lengthInfo;
    if (DeviceIoControl(
        hDevice,
        IOCTL_DISK_GET_LENGTH_INFO,
        NULL,
        0,
        &lengthInfo,
        sizeof(lengthInfo),
        &bytesReturned,
        NULL))
    {
        return lengthInfo.Length.QuadPart;
    }

    // Fall back to seeking to end if both methods fail
    LARGE_INTEGER distanceToMove;
    distanceToMove.QuadPart = 0;
    SetFilePointerEx(hDevice, distanceToMove, &distanceToMove, FILE_END);
    return distanceToMove.QuadPart;
}

// Test for fake flash by writing and reading pattern
FakeType test_drive(HANDLE hDevice, uint64_t drive_size, int destructive, int time_ops) {
    const uint64_t test_interval = drive_size / 64;  // Test at 64 points
    const uint64_t min_test_interval = 64 * 1024 * 1024; // Min 64MB between tests
    
    unsigned char *write_buffer = (unsigned char *)malloc(BLOCK_SIZE);
    unsigned char *read_buffer = (unsigned char *)malloc(BLOCK_SIZE);
    
    if (!write_buffer || !read_buffer) {
        printf("Error: Out of memory\n");
        if (write_buffer) free(write_buffer);
        if (read_buffer) free(read_buffer);
        return FAKE_TYPE_DAMAGED;
    }
    
    LARGE_INTEGER li;
    DWORD bytesWritten, bytesRead;
    int mismatch_count = 0;
    int test_count = 0;
    uint64_t first_mismatch_pos = 0;
    int error_count = 0;
    
    // Time tracking
    LARGE_INTEGER frequency, start, end;
    double write_seconds = 0, read_seconds = 0;
    
    QueryPerformanceFrequency(&frequency);
    
    printf("Testing drive with %s mode...\n", 
           destructive ? "destructive" : "non-destructive");
    printf("Drive size: %.2f GB\n", (double)drive_size / (1024*1024*1024));
    
    // Skip first 1MB which might contain partition table/filesystem data
    uint64_t pos = 1024 * 1024;
    uint64_t step = test_interval > min_test_interval ? test_interval : min_test_interval;

    while (pos + BLOCK_SIZE <= drive_size) {
        // Generate test pattern
        fill_pattern(write_buffer, BLOCK_SIZE, pos);
        
        // Set file pointer
        li.QuadPart = pos;
        if (!SetFilePointerEx(hDevice, li, NULL, FILE_BEGIN)) {
            printf("Error setting file pointer to position %llu\n", pos);
            error_count++;
            pos += step;
            continue;
        }
        
        if (destructive) {
            // Write pattern
            QueryPerformanceCounter(&start);
            
            if (!WriteFile(hDevice, write_buffer, BLOCK_SIZE, &bytesWritten, NULL) || 
                bytesWritten != BLOCK_SIZE) {
                printf("Error writing at position %llu\n", pos);
                error_count++;
                pos += step;
                continue;
            }
            
            QueryPerformanceCounter(&end);
            write_seconds += (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;

            // Flush to ensure data is written
            FlushFileBuffers(hDevice);
        }
        
        // Set file pointer again for reading
        li.QuadPart = pos;
        if (!SetFilePointerEx(hDevice, li, NULL, FILE_BEGIN)) {
            printf("Error setting file pointer for reading at position %llu\n", pos);
            error_count++;
            pos += step;
            continue;
        }
        
        // Read data
        QueryPerformanceCounter(&start);
        
        if (!ReadFile(hDevice, read_buffer, BLOCK_SIZE, &bytesRead, NULL) || 
            bytesRead != BLOCK_SIZE) {
            printf("Error reading at position %llu\n", pos);
            error_count++;
            pos += step;
            continue;
        }
        
        QueryPerformanceCounter(&end);
        read_seconds += (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
        
        if (destructive) {
            // Compare data
            int mismatch = 0;
            for (size_t i = 0; i < BLOCK_SIZE; i++) {
                if (write_buffer[i] != read_buffer[i]) {
                    mismatch = 1;
                    break;
                }
            }
            
            if (mismatch) {
                mismatch_count++;
                if (first_mismatch_pos == 0) {
                    first_mismatch_pos = pos;
                }
                printf("X");  // Indicates mismatch
            } else {
                printf(".");  // Indicates match
            }
        } else {
            printf(".");  // Just to show progress
        }
        
        test_count++;
        if (test_count % 32 == 0) {
            printf(" %llu MB\n", pos / (1024 * 1024));
        }
        
        pos += step;
        
        // If we've found several mismatches, we can conclude it's fake
        if (mismatch_count >= 3) {
            break;
        }
    }
    
    printf("\n\nTest complete. %d points tested.\n", test_count);
    
    if (time_ops) {
        printf("Write time: %.2f seconds\n", write_seconds);
        printf("Read time: %.2f seconds\n", read_seconds);
        printf("Average write speed: %.2f MB/s\n", 
               (BLOCK_SIZE * test_count) / (write_seconds * 1024 * 1024));
        printf("Average read speed: %.2f MB/s\n", 
               (BLOCK_SIZE * test_count) / (read_seconds * 1024 * 1024));
    }
    
    free(write_buffer);
    free(read_buffer);
    
    if (error_count > test_count / 2) {
        printf("Drive appears to be DAMAGED (too many I/O errors)\n");
        return FAKE_TYPE_DAMAGED;
    } else if (mismatch_count > 0) {
        printf("Drive appears to be COUNTERFEIT\n");
        printf("First mismatch at: %llu MB\n", first_mismatch_pos / (1024 * 1024));
        printf("Estimated real capacity: approximately %llu MB\n", 
               first_mismatch_pos / (1024 * 1024));
        return FAKE_TYPE_POSSIBLY_FAKE;
    } else {
        printf("Drive appears to be GENUINE\n");
        return FAKE_TYPE_GOOD;
    }
}

void print_usage(const char* program_name) {
    printf("F3 Probe for Windows %s - probe a flash drive for counterfeit\n", VERSION);
    printf("Copyright (C) 2010 Digirati Internet LTDA.\n");
    printf("This is free software; see the source for copying conditions.\n\n");
    
    printf("Usage: %s [options] drive_letter:\n", program_name);
    printf("Options:\n");
    printf("  --destructive       Perform destructive testing (will overwrite data)\n");
    printf("  --time-ops          Time read and write operations\n");
    printf("  --help              Display this help text\n");
    printf("\nExample: %s --destructive J:\n", program_name);
    printf("\nWARNING: Destructive mode will overwrite data on the drive.\n");
    printf("         Please backup your data before using this option.\n");
}

int main(int argc, char **argv) {
    int destructive = 0;
    int time_ops = 0;
    char drive_letter = 0;
    
    // Parse command line args
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--destructive") == 0) {
            destructive = 1;
        } else if (strcmp(argv[i], "--time-ops") == 0) {
            time_ops = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strlen(argv[i]) >= 1) {
            // Assume it's a drive letter
            drive_letter = argv[i][0];
            // Check if it has a colon
            if (strlen(argv[i]) >= 2 && argv[i][1] != ':') {
                printf("Error: Drive specification should end with a colon (e.g., J:)\n");
                print_usage(argv[0]);
                return 1;
            }
        }
    }
    
    if (!drive_letter) {
        printf("Error: Drive letter not specified\n");
        print_usage(argv[0]);
        return 1;
    }
    
    // Construct physical drive path
    char physical_drive_path[128];
    sprintf(physical_drive_path, "\\\\.\\%c:", drive_letter);
    
    printf("F3 Probe for Windows %s\n", VERSION);
    printf("Copyright (C) 2010 Digirati Internet LTDA.\n");
    printf("This is free software; see the source for copying conditions.\n\n");
    
    printf("Probing drive %c:\n", drive_letter);
    
    // Open the drive
    HANDLE hDevice = CreateFile(
        physical_drive_path,
        GENERIC_READ | (destructive ? GENERIC_WRITE : 0),
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_NO_BUFFERING | FILE_FLAG_RANDOM_ACCESS,
        NULL
    );
    
    if (hDevice == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        printf("Error opening drive %c: (code %lu)\n", drive_letter, error);
        printf("Make sure you run this program with administrator privileges\n");
        printf("and that the drive is not in use by another program.\n");
        return 1;
    }
    
    uint64_t drive_size = get_drive_size(hDevice);
    if (drive_size == 0) {
        printf("Error: Could not determine drive size\n");
        CloseHandle(hDevice);
        return 1;
    }
    
    FakeType result = test_drive(hDevice, drive_size, destructive, time_ops);
    
    // Close the drive
    CloseHandle(hDevice);
    
    return result == FAKE_TYPE_GOOD ? 0 : 1;
}