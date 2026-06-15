#include "file/file_utils.h"
#include "png/png_utils.h"

#include "opencl/cl_init.h"

int main() {
    ComputeContext ctx = cl_init();
    cl_print_context_info(&ctx);

    PNGImage img = png_read(PNG_FILE("watch"));

    cl_command_queue queue = clCreateCommandQueue(ctx.context, ctx.device, 0, NULL);

    cl_program program = clCreateProgramWithIL(
        ctx.context,
        read_file_bytes(SPIRV_PROGRAM("grayscale")),
        get_file_size_bytes(SPIRV_PROGRAM("grayscale")),
        NULL
    );
    clBuildProgram(program, 1, &ctx.device, NULL, NULL, NULL);
    cl_kernel to_grayscale = clCreateKernel(program, "to_grayscale", NULL);

    cl_image_format rgb_format = {
        .image_channel_order = CL_RGB,
        .image_channel_data_type = CL_UNSIGNED_INT8
    };
    cl_image_format gs_format = {
        .image_channel_order = CL_LUMINANCE,
        .image_channel_data_type = CL_UNSIGNED_INT8
    };

    size_t origin[2] = { 0, 0 };
    size_t region[2] = { img.width, img.height };
    size_t row_pitch = img.width * 3;
    size_t gs_row_pitch = img.width;
    size_t slice_pitch = pixel_data_len(&img);
    size_t gs_slice_pitch = grayscale_pixel_data_len(&img);

    cl_mem rgb_img = clCreateImage2D(
        ctx.context,
        CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
        &rgb_format,
        img.width,
        img.height,
        img.width * 3,
        NULL,
        NULL
    );
    uint8_t *rgb_img_ptr = (uint8_t *) clEnqueueMapImage(queue, rgb_img, CL_TRUE, CL_MAP_WRITE, origin, region, &row_pitch, &slice_pitch, 0, NULL, NULL, NULL);
    memmove(rgb_img_ptr, img.pixels, pixel_data_len(&img));
    clEnqueueUnmapMemObject(queue, rgb_img, rgb_img_ptr, 0, NULL, NULL);

    cl_mem gs_img = clCreateImage2D(
        ctx.context,
        CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
        &rgb_format,
        img.width,
        img.height,
        img.width,
        NULL,
        NULL
    );

    clSetKernelArg(to_grayscale, 0, sizeof(cl_mem), rgb_img);
    clSetKernelArg(to_grayscale, 1, sizeof(cl_mem), gs_img);

    if (img.pixels != NULL) {
        size_t gwg[2] = { img.width / 4, img.height };
        size_t lwg[2] = { ctx.max_workgroup_size / 2, 1 };
        clEnqueueNDRangeKernel(queue, to_grayscale, 2, NULL, gwg, lwg, 0, NULL, NULL);

        uint8_t *gray_pixels = (uint8_t *) clEnqueueMapImage(queue, gs_img, CL_TRUE, CL_MAP_READ, origin, region, &gs_row_pitch, &gs_slice_pitch, 0, NULL, NULL, NULL);
        grayscale_png_write(PNG_FILE("watch_gray"), &img, gray_pixels);
        
        free_png_image(img);
        clEnqueueUnmapMemObject(queue, gs_img, gray_pixels, 0, NULL, NULL);
    } else {
        printf("Failed to read PNG image.\n");
    }

    clFinish(queue);
    cl_free_compute_context(ctx);
    return 0;
}