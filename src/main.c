#include "png/png_utils.h"

#include "opencl/cl_init.h"

int main() {
    ComputeContext ctx = cl_init();
    cl_print_context_info(&ctx);
    
    PNGImage img = png_read(PNG_FILE("watch"));

    uint8_t *gray_pixels = (uint8_t *) malloc(grayscale_pixel_data_len(&img));

    if (img.pixels != NULL) {
        for(uint64_t i = 0; i < grayscale_pixel_data_len(&img); i++) {
            gray_pixels[i] = (uint8_t) (
                (
                    77 * img.pixels[img.channels * i + 0] +
                    150 * img.pixels[img.channels * i + 1] +
                    29 * img.pixels[img.channels * i + 2]
                ) >> 8
            );
        }

        grayscale_png_write(PNG_FILE("watch_gray"), &img, gray_pixels);

        free_png_image(img);
        free(gray_pixels);
    } else {
        printf("Failed to read PNG image.\n");
    }

    cl_free_compute_context(ctx);
    return 0;
}