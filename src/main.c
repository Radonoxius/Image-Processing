#include "file/file_utils.h"
#include "png/png_utils.h"
#include "opencl/cl_init.h"

int main() {
    ComputeContext ctx = cl_init();
    cl_print_context_info(&ctx);

    PNGImage img = png_read(PNG_FILE("watch"));
    if (img.pixels == NULL) {
        printf("Failed to read PNG image.\n");
        cl_free_compute_context(ctx);
        return 1;
    }

    cl_int err;
    cl_command_queue queue = clCreateCommandQueue(ctx.context, ctx.device, 0, NULL);

    const void *spirv = read_file_bytes(SPIRV_PROGRAM("grayscale"));
    size_t spirv_sz = get_file_size_bytes(SPIRV_PROGRAM("grayscale"));
    cl_program program = clCreateProgramWithIL(ctx.context, spirv, spirv_sz, NULL);
    clBuildProgram(program, 1, &ctx.device, NULL, NULL, NULL);
    cl_kernel to_grayscale = clCreateKernel(program, "to_grayscale", NULL);

    cl_image_format rgb_format = {
        .image_channel_order     = CL_RGBA,
        .image_channel_data_type = CL_UNSIGNED_INT8
    };
    cl_image_format gs_format = {
        .image_channel_order     = CL_R,
        .image_channel_data_type = CL_UNSIGNED_INT8
    };

    cl_mem rgb_img = clCreateImage2D(
        ctx.context,
        CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
        &rgb_format,
        img.width, img.height,
        img.width * 4,
        img.pixels,
        NULL
    );

    // ALLOC_HOST_PTR: driver places this in CPU-accessible memory on iGPU
    // so MapImage gives a zero-copy pointer into the same physical page
    cl_mem gs_img = clCreateImage2D(
        ctx.context,
        CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
        &gs_format,
        img.width, img.height,
        0, NULL,
        NULL
    );

    clSetKernelArg(to_grayscale, 0, sizeof(cl_mem), &rgb_img);
    clSetKernelArg(to_grayscale, 1, sizeof(cl_mem), &gs_img);

    // One work item per pixel; no width/4 truncation
    size_t gwg[2] = { img.width, img.height };
    clEnqueueNDRangeKernel(queue, to_grayscale, 2, NULL, gwg, NULL, 0, NULL, NULL);

    // Finish before mapping
    clFinish(queue);

    size_t origin[3] = { 0, 0, 0 };
    size_t region[3] = { img.width, img.height, 1 };
    size_t row_pitch;

    // Zero-copy map on iGPU: no malloc, no ReadImage transfer
    uint8_t *gray_pixels = clEnqueueMapImage(
        queue, gs_img, CL_TRUE,
        CL_MAP_READ,
        origin, region,
        &row_pitch, NULL,
        0, NULL, NULL, NULL
    );

    grayscale_png_write(PNG_FILE("watch_gray"), &img, gray_pixels);

    clEnqueueUnmapMemObject(queue, gs_img, gray_pixels, 0, NULL, NULL);
    clFinish(queue);

    free_png_image(img);
    clReleaseMemObject(rgb_img);
    clReleaseMemObject(gs_img);
    clReleaseKernel(to_grayscale);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    cl_free_compute_context(ctx);
    return 0;
}