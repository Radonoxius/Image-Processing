#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <png.h>

// Helps you to specify the name of the `PNG` file
#define PNG_FILE(filename) "../imgs/" filename ".png"

// Holds `PNG` metadata and raw pixel data
typedef struct PNGImage {
    char *filename;    // The name of the PNG associated with the struct
    uint32_t width;    // Width of the image in pixels
    uint32_t height;   // Height of the image in pixels
    uint8_t *pixels;   // Array of raw pixel values
    uint8_t channels;  // 3 for RGB, 4 for RGBA
} PNGImage;

// Returns the length of the raw pixel data
static uint64_t png_pixel_data_len(const PNGImage *const img) {
    return img->channels * img->width * img->height;
}

// Returns the length of the equivalent grayscale pixel data
static uint64_t png_grayscale_pixel_data_len(const PNGImage *const img) {
    return img->width * img->height;
}

// Free the PNGImage struct
static void png_free_struct(PNGImage img) {
    free(img.filename);
    if (img.pixels != NULL)
        free(img.pixels);
}

/**
 * Reads a PNG file and returns the `PNGImage` struct
 * @param filename The name of the image that will be read.
 * @return Valid `PNGImage` on success, Zeroed struct on failure.
 */
static PNGImage png_read(const char *const filename) {
    PNGImage img = { 0 };

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Error opening file");
        return img;
    }

    // Verify the PNG signature (first 8 bytes)
    unsigned char header[8];
    if (fread(header, 1, 8, fp) != 8 || png_sig_cmp(header, 0, 8)) {
        fprintf(stderr, "Error: File %s is not a valid PNG.\n", filename);
        fclose(fp);
        return img;
    }

    // Initialize libpng structures
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fprintf(stderr, "Error: png_create_read_struct failed.\n");
        fclose(fp);
        return img;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fprintf(stderr, "Error: png_create_info_struct failed.\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(fp);
        return img;
    }

    // Set up error handling (libpng uses setjmp/longjmp)
    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "Error during PNG creation/reading.\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return img;
    }

    // Initialize PNG I/O
    png_init_io(png_ptr, fp);
    // Tell libpng we already read the first 8 bytes signature
    png_set_sig_bytes(png_ptr, 8);

    // Read the image metadata
    png_read_info(png_ptr, info_ptr);

    // Get basic image info
    png_uint_32 width = png_get_image_width(png_ptr, info_ptr);
    png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    // --- Standardizing the Image Format ---
    // Convert palette images to RGB
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
    }
    // Convert low-bit grayscale (1, 2, 4 bits) to 8 bits
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }
    // Expand transparency chunks to a full alpha channel
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr);
    }
    // Reduce 16-bit depth per channel to 8-bit depth
    if (bit_depth == 16) {
        png_set_strip_16(png_ptr);
    }
    // Convert grayscale images to RGB
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png_ptr);
    }

    // Update the image info after setting transformations
    png_read_update_info(png_ptr, info_ptr);
    
    // Re-read color type to see what we transformed it into (typically RGB or RGBA)
    color_type = png_get_color_type(png_ptr, info_ptr);
    uint8_t channels = (uint8_t) png_get_channels(png_ptr, info_ptr);

    img.width = width;
    img.height = height;
    img.channels = channels;
    img.filename = filename;

    // Allocate memory for the raw pixel buffer
    size_t row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    img.pixels = (unsigned char *)malloc(row_bytes * height);
    if (!img.pixels) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return img;
    }

    // Setup an array of pointers pointing to the beginning of each row in the flat pixels array
    png_bytepp row_pointers = (png_bytepp)malloc(sizeof(png_bytep) * height);
    for (png_uint_32 i = 0; i < height; i++) {
        row_pointers[i] = img.pixels + (i * row_bytes);
    }

    // Read the actual image data into our buffer
    png_read_image(png_ptr, row_pointers);

    // Clean up
    free(row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);

    return img;
}

/**
 * Reads a PNG file's metadata and returns the `PNGImage` struct without fetching pixel data.
 * @param filename The name of the image that will be read.
 * @return Valid `PNGImage` (pixels = NULL) on success, Zeroed struct on failure.
 * 
 * #### Use this when dealing with iGPUs
 */
static PNGImage png_get_info(const char *const filename) {
    PNGImage img = { 0 };

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Error opening file");
        return img;
    }

    // Verify the PNG signature (first 8 bytes)
    unsigned char header[8];
    if (fread(header, 1, 8, fp) != 8 || png_sig_cmp(header, 0, 8)) {
        fprintf(stderr, "Error: File %s is not a valid PNG.\n", filename);
        fclose(fp);
        return img;
    }

    // Initialize libpng structures
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fprintf(stderr, "Error: png_create_read_struct failed.\n");
        fclose(fp);
        return img;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fprintf(stderr, "Error: png_create_info_struct failed.\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(fp);
        return img;
    }

    // Set up error handling (libpng uses setjmp/longjmp)
    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "Error during PNG info reading.\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return img;
    }

    // Initialize PNG I/O
    png_init_io(png_ptr, fp);
    // Tell libpng we already read the first 8 bytes signature
    png_set_sig_bytes(png_ptr, 8);

    // Read the image metadata
    png_read_info(png_ptr, info_ptr);

    // Get basic image info
    png_uint_32 width = png_get_image_width(png_ptr, info_ptr);
    png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    // --- Standardizing the Image Format ---
    // These must be kept so `channels` reflects the format png_read would output
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr);
    }
    if (bit_depth == 16) {
        png_set_strip_16(png_ptr);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png_ptr);
    }

    // Update the image info after setting transformations
    png_read_update_info(png_ptr, info_ptr);
    
    // Re-read channels to see what we transformed it into (typically 3 or 4)
    uint8_t channels = (uint8_t) png_get_channels(png_ptr, info_ptr);

    // Populate the struct fields
    img.width = width;
    img.height = height;
    img.channels = channels;
    img.pixels = NULL; // Explicitly set to NULL since no data is read
    img.filename = filename;

    // Clean up libpng structures and close the file
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);

    return img;
}

/**
 * Reads pixel data from a PNG file and populates a pre-allocated buffer.
 * @param img Pointer to your `PNGImage` struct.
 * @param pixel_buf Pre-allocated memory block to receive the raw pixel data.
 * @return 1 on success, 0 on failure.
 * 
 * #### Use this when dealing with iGPUs
 */
static int png_get_pixeldata(const PNGImage *const img, uint8_t *pixel_buf) {
    if (!img || !img->filename || !pixel_buf) {
        fprintf(stderr, "Error: Invalid arguments passed to png_get_pixeldata.\n");
        return 0;
    }

    FILE *fp = fopen(img->filename, "rb");
    if (!fp) {
        perror("Error opening file");
        return 0;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fclose(fp);
        return 0;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(fp);
        return 0;
    }

    // Standard libpng error recovery
    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "Error during PNG pixel reading.\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return 0;
    }

    // OPTIMIZATION 1: Let libpng read from byte 0. 
    // We already know it's a valid PNG from png_get_info. If the file changed or corrupted
    // underneath us, libpng's internal signature check will safely trip the setjmp block.
    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);

    // Get format parameters needed to recreate the normalization pipeline
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr);
    }
    if (bit_depth == 16) {
        png_set_strip_16(png_ptr);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png_ptr);
    }

    png_read_update_info(png_ptr, info_ptr);

    // OPTIMIZATION 2: Calculate stride directly from metadata instead of querying libpng again.
    size_t row_bytes = (size_t)img->width * img->channels;

    // OPTIMIZATION 3: Stream rows sequentially via `png_read_row`.
    // Eliminates malloc/free overhead for `row_pointers` completely, keeping execution fast 
    // and removing a potential heap allocation failure path. Perfect for iGPU zero-copy alignment layouts.
    for (uint32_t i = 0; i < img->height; i++) {
        png_read_row(png_ptr, pixel_buf + (i * row_bytes), NULL);
    }

    // Clean up
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);

    return 1;
}

/**
 * Writes a PNG image to disk.
 * @param img Pointer to your `PNGImage` struct.
 * @return 0 on success, -1 on failure.
 */
static int png_write(const PNGImage *const img) {
    if (!img || !img->pixels) {
        fprintf(stderr, "Error: Invalid image data or null pixel buffer.\n");
        return -1;
    }

    // 1. Open the file for writing in binary mode
    FILE *fp = fopen(img -> filename, "wb");
    if (!fp) {
        perror("Error opening file for writing");
        return -1;
    }

    // 2. Initialize the write structure
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fclose(fp);
        return -1;
    }

    // 3. Initialize the info structure
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, NULL);
        fclose(fp);
        return -1;
    }

    // 4. Set up error handling (libpng uses setjmp/longjmp for errors)
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return -1;
    }

    // 5. Set up the output stream
    png_init_io(png_ptr, fp);

    // 6. Map your struct's channel count to libpng color types
    int color_type;
    if (img->channels == 3) {
        color_type = PNG_COLOR_TYPE_RGB;
    } else if (img->channels == 4) {
        color_type = PNG_COLOR_TYPE_RGBA;
    } else {
        fprintf(stderr, "Error: Unsupported number of channels (%d). Only 3 (RGB) or 4 (RGBA) are supported.\n", img->channels);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return -1;
    }

    // 7. Write the PNG header (IHDR chunk)
    // We assume 8 bits per channel since pixels are stored as unsigned char.
    png_set_IHDR(
        png_ptr,
        info_ptr,
        img->width,
        img->height,
        8, 
        color_type,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE
    );

    png_write_info(png_ptr, info_ptr);

    // 8. Allocate memory for row pointers and map them to your flattened array
    png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * img->height);
    if (!row_pointers) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return -1;
    }

    size_t row_stride = (size_t)img->width * img->channels;
    for (unsigned int y = 0; y < img->height; y++) {
        row_pointers[y] = (png_bytep)&img->pixels[y * row_stride];
    }

    // 9. Write the actual pixel data
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, NULL);

    // 10. Clean up everything
    free(row_pointers);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);

    return 0; // Success!
}

/**
 * Writes an 8-bit greyscale PNG image to disk.
 * @param new_filename The path/name where the new image will be saved.
 * @param img Pointer to your `PNGImage` struct.
 * @param gray_pixels Pointer to the new greyscale pixels.
 * @return 0 on success, -1 on failure.
 */
static int png_write_grayscale(
    const char *const new_filename,
    const PNGImage *const img,
    const uint8_t *const gray_pixels
) {
    if (!gray_pixels || img->width == 0 || img->height == 0) {
        fprintf(stderr, "Error: Invalid raw pixel data or dimensions.\n");
        return -1;
    }

    FILE *fp = fopen(new_filename, "wb");
    if (!fp) {
        perror("Error opening file for writing");
        return -1;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fclose(fp);
        return -1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, NULL);
        fclose(fp);
        return -1;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return -1;
    }

    png_init_io(png_ptr, fp);

    // Set configuration to 8-bit Greyscale (PNG_COLOR_TYPE_GRAY)
    png_set_IHDR(
        png_ptr,
        info_ptr,
        img->width,
        img->height,
        8,                    // 8 bits per pixel
        PNG_COLOR_TYPE_GRAY,  // 1-channel greyscale flag
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE
    );

    png_write_info(png_ptr, info_ptr);

    // Map row pointers directly into your parameter buffer offsets
    png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * img->height);
    if (!row_pointers) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return -1;
    }

    for (uint32_t y = 0; y < img->height; y++) {
        row_pointers[y] = (png_bytep) &gray_pixels[y * img->width];
    }

    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, NULL);

    free(row_pointers);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);

    return 0;
}