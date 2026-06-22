#include "file/file_utils.h"
#include "png/png_utils.h"

#include "opencl/cl_init.h"
#include <time.h>

int main() {
    ComputeContext ctx = init();
    print_context_info(&ctx, CL_TRUE);

    if (
        IGPU_CL30_SPIRV_IMAGE_PROFILE(&ctx) &&

        is_image2d_format_available(&ctx, CL_R, CL_UNSIGNED_INT8) &&
        is_image2d_format_available(&ctx, CL_RGBA, CL_UNSIGNED_INT8)
    ) {
        PNGImage img = png_read(PNG_FILE("watch"));

        cl_int err;
        cl_command_queue queue = clCreateCommandQueue(ctx.context, ctx.device, 0, NULL);

        const uint8_t *spirv = file_read_bytes(SPIRV_PROGRAM("grayscale_2"));
        const size_t spirv_sz = file_get_size_bytes(SPIRV_PROGRAM("grayscale_2"));
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
            CL_MEM_WRITE_ONLY,
            &gs_format,
            img.width, img.height,
            0,
            NULL,
            NULL
        );

        clSetKernelArg(to_grayscale, 0, sizeof(cl_mem), &rgb_img);
        clSetKernelArg(to_grayscale, 1, sizeof(cl_mem), &gs_img);

        size_t gwg[2] = { img.width, img.height };
        size_t lwg[2] = { ctx.max_workgroup_size / 2, 1 };

        struct timespec before, after;
        clock_gettime(CLOCK_MONOTONIC, &before);
        clEnqueueNDRangeKernel(queue, to_grayscale, 2, NULL, gwg, NULL, 0, NULL, NULL);        

        // Finish before mapping
        clFinish(queue);
        clock_gettime(CLOCK_MONOTONIC, &after);        
        uint64_t delta_ns = (after.tv_sec - before.tv_sec) * 1000000000 +
            (after.tv_nsec - before.tv_nsec);
        printf("Took %lu micros to compute.\n", delta_ns / 1000);

        size_t origin[3] = { 0, 0, 0 };
        size_t region[3] = { img.width, img.height, 1 };
        size_t row_pitch;

        uint8_t *gray_pixels = malloc(img.width * img.height);
        clEnqueueReadImage(
            queue, gs_img, CL_TRUE,
            origin, region,
            0, 0,           // pitch=0 → runtime packs tightly into host buffer
            gray_pixels,
            0, NULL, NULL
        );

        png_write_grayscale(PNG_FILE("watch_gray"), &img, gray_pixels);
        free(gray_pixels);
        clFinish(queue);

        png_free_pixels(img);

        clReleaseMemObject(rgb_img);
        clReleaseMemObject(gs_img);
        clReleaseKernel(to_grayscale);
        clReleaseProgram(program);
        clReleaseCommandQueue(queue);
    }

    if (DGPU_IMAGE_PROFILE(&ctx))
        printf("dGPU detected! Nothing to do since code is incompatible!\n");
    
    free_compute_context(ctx);
    return 0;
}