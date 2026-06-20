#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// Helps you to specify the name of the `SPIRV` program
#define SPIRV_PROGRAM(filename) "kernels/spirv/" filename ".cl.spv"

// Helps you to specify the name of the `OpenCL C` program
#define CLC_PROGRAM(filename) "kernels/" filename ".cl"

/**
 * Reads the specified file as raw bytes
 * @param filename The name of the file that will be read.
 * @return Valid file contents (byte array) on success, or NULL on failure.
 */
static uint8_t *file_read_bytes(const char *const filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("Failed to open file for reading");
        return NULL;
    }

    // Seek to end to determine size
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }
    
    long res = ftell(fp);
    if (res < 0) {
        fclose(fp);
        return NULL;
    }
    size_t size = (size_t)res;
    
    // Rewind back to the beginning
    fseek(fp, 0, SEEK_SET);

    uint8_t *bin = (uint8_t *) malloc(size);
    if (bin == NULL) {
        fclose(fp);
        return NULL;
    }

    // Read elements of 1 byte up to 'size' count
    size_t bytes_read = fread(bin, 1, size, fp);
    if (bytes_read < size) {
        // Handle partial read or error if necessary
        free(bin);
        fclose(fp);
        return NULL;
    }

    fclose(fp);
    return bin;
}

/**
 * Returns the size of the file in bytes
 * @param filename The name of the file that will be read.
 * @return Valid file size on success, or 0 on failure.
 */
static size_t file_get_size_bytes(const char *const filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("Failed to open file for size check");
        return 0;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }
    
    long res = ftell(fp);
    fclose(fp);

    return (res < 0) ? 0 : (size_t)res;
}