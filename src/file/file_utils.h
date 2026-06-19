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
 * @return Valid file contents (byte array) on success.
 * 
 * #### NOTE: Returned contents are invalid if an internal
 * failure was encountered
 */
static uint8_t *read_file_bytes(const char *const filename) {
    FILE *fp = fopen(filename, "rb");

    int size = fseek(fp, 0, SEEK_END);
    fseek(fp, 0, SEEK_SET);

    uint8_t *bin = (uint8_t *) malloc(size);
    fread(bin, size, 1, fp);

    fclose(fp);

    return bin;
}

/**
 * Returns the size of the file in bytes
 * @param filename The name of the file that will be read.
 * @return Valid file size on success.
 * 
 * #### NOTE: Returned size is invalid if an internal
 * failure was encountered
 */
static size_t get_file_size_bytes(const char *const filename) {
    FILE *fp = fopen(filename, "rb");

    int size = fseek(fp, 0, SEEK_END);
    fseek(fp, 0, SEEK_SET);

    fclose(fp);

    return size;
}