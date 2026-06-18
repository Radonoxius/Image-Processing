#pragma once

#include "cl_init_utils.h"
#include "cl_utils.h"

// Holds OpenCL primitives and important info required
// by the program to function properly
typedef struct ComputeContext {
    cl_platform_id platform;    // The chosen OpenCL Platform
    cl_device_id device;        // The chosen Platform Device
    cl_context context;         // Corresponding Device Context
    
    char *spirv_version_str;    // SPIRV version

    size_t max_workgroup_size;  // Max number of Threads per WorkGroup
    cl_uint max_compute_units;  // Number of Compute Units

    cl_bool cl_30_support;      // OpenCL 3 Support
    cl_bool cl_21_support;      // OpenCL 2.1 Support
    cl_bool cl_20_support;      // OpenCL 2 Support

    cl_bool uses_spirv_khr;     // Represents whether SPIRV uses the KHR extension

    cl_bool image_support;      // Image Support
    cl_bool image2d_r_support;    // CL_R channel 2D-Image Support
    cl_bool image2d_rgb_support;  // CL_RGB channel 2D-Image Support
    cl_bool image2d_rgba_support; // CL_RGBA channel 2D-Image Support

    cl_bool usm_support;        // Represents whether the device supports shared memory
} ComputeContext;

/**
 * Initializes a simple OpenCL environment.
 * 
 * Lets the user dynamically choose the Platform and
 * Device (if there are multiple available)
 * 
 * @return Valid `ComputeContext` on success, Zeroed struct on failure.
 */
static ComputeContext cl_init() {
    const cl_platform_id platform = cl_choose_platform_id();
    cl_device_id device = cl_choose_device_id(platform);
    
    const cl_context_properties context_properties[3] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties) platform,
        0
    };
    const cl_context context = clCreateContext(context_properties, 1, &device, NULL, NULL, NULL);

    ComputeContext ctx = { 0 };
    ctx.platform = platform;
    ctx.device = device;
    ctx.context = context;

    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &ctx.max_workgroup_size, NULL);
    clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &ctx.max_compute_units, NULL);

    clGetDeviceInfo(device, CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool), &ctx.image_support, NULL);
    clGetDeviceInfo(device, CL_DEVICE_HOST_UNIFIED_MEMORY, sizeof(cl_bool), &ctx.usm_support, NULL);

    size_t il_str_len;
    clGetDeviceInfo(device, CL_DEVICE_IL_VERSION, 0, NULL, &il_str_len);
    char *il_str = (char *) malloc(il_str_len);
    cl_int errcode = clGetDeviceInfo(device, CL_DEVICE_IL_VERSION, il_str_len, il_str, NULL);

    if (errcode != CL_SUCCESS || il_str_len < 4) {
        cl_bool r = is_device_extension_available(device, "cl_khr_il_program");

        if (il_str != NULL) {
            free(il_str);
            il_str = NULL;
        }

        if (r == CL_TRUE) {
            clGetDeviceInfo(device, CL_DEVICE_IL_VERSION_KHR, 0, NULL, &il_str_len);
            il_str = (char *) malloc(il_str_len);
            clGetDeviceInfo(device, CL_DEVICE_IL_VERSION_KHR, il_str_len, il_str, NULL);

            ctx.spirv_version_str = il_str;
            ctx.uses_spirv_khr = CL_TRUE;
        }
    } else {
        ctx.spirv_version_str = il_str;
        ctx.cl_21_support = CL_TRUE;
        ctx.cl_20_support = CL_TRUE;
    }

    cl_version device_version;
    errcode = clGetDeviceInfo(device, CL_DEVICE_NUMERIC_VERSION, sizeof(cl_version), &device_version, NULL);
    if (errcode == CL_SUCCESS && CL_VERSION_MAJOR(device_version) == 3)
        ctx.cl_30_support = CL_TRUE;

    if (ctx.cl_30_support == CL_TRUE)
        ctx.image_support = ctx.image_support &&
            is_device_feature_available(ctx.device, "__opencl_c_images");

    if (ctx.image_support == CL_TRUE) {
        cl_uint format_count;
        clGetSupportedImageFormats(
            ctx.context,
            CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY,
            CL_MEM_OBJECT_IMAGE2D,
            0,
            NULL,
            &format_count
        );
        cl_image_format *image_formats = (cl_image_format *) malloc(format_count * sizeof(cl_image_format));
        clGetSupportedImageFormats(
            ctx.context,
            CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY,
            CL_MEM_OBJECT_IMAGE2D,
            format_count,
            image_formats,
            NULL
        );

        for (cl_uint i = 0; i < format_count; i++) {
            if (
                image_formats[i].image_channel_order == CL_R &&
                image_formats[i].image_channel_data_type == CL_UNSIGNED_INT8
            )
                ctx.image2d_r_support = CL_TRUE;
            
            else if (
                image_formats[i].image_channel_order == CL_RGB &&
                image_formats[i].image_channel_data_type == CL_UNSIGNED_INT8
            )
                ctx.image2d_rgb_support = CL_TRUE;

            else if (
                image_formats[i].image_channel_order == CL_RGBA &&
                image_formats[i].image_channel_data_type == CL_UNSIGNED_INT8
            )
                ctx.image2d_rgba_support = CL_TRUE;
        }
    }

    return ctx;
}

/**
 * Prints basic info about the given context.
 * 
 * @param ctx A pointer to the context whose info
 * is printed.
 */
static void cl_print_context_info(ComputeContext *ctx) {
    printf("\nCompute Context Info:\n--------\n");

    size_t vendor_name_len;
    clGetPlatformInfo(ctx -> platform, CL_PLATFORM_VENDOR, 0, NULL, &vendor_name_len);
    char vendor_name[vendor_name_len];
    clGetPlatformInfo(ctx -> platform, CL_PLATFORM_VENDOR, vendor_name_len, vendor_name, NULL);
    size_t device_name_len;
    clGetDeviceInfo(ctx -> device, CL_DEVICE_NAME, 0, NULL, &device_name_len);
    char device_name[device_name_len];
    clGetDeviceInfo(ctx -> device, CL_DEVICE_NAME, device_name_len, device_name, NULL);

    printf("[%s] %s\n\n", vendor_name, device_name);
    printf("# Compute Units:             %u\n", ctx -> max_compute_units);
    printf("Max # Threads per WorkGroup: %lu\n\n", ctx -> max_workgroup_size);

    if (ctx -> spirv_version_str != NULL) {
        printf("SPIRV version:               %s\n", ctx -> spirv_version_str);
        printf("SPIRV KHR Extension:         %s\n\n", STRINGIFY(ctx -> uses_spirv_khr));
    }

    printf("OpenCLv3.0 Support:          %s\n", STRINGIFY(ctx -> cl_30_support));
    printf("OpenCLv2.1 Support:          %s\n", STRINGIFY(ctx -> cl_21_support));
    printf("OpenCLv2.0 Support:          %s\n\n", STRINGIFY(ctx -> cl_20_support));

    printf("Image Support:               %s\n", STRINGIFY(ctx -> image_support));
    if (ctx -> image_support) {
        printf("Image2D CL_R Support:        %s\n", STRINGIFY(ctx -> image2d_r_support));
        printf("Image2D CL_RGB Support:      %s\n", STRINGIFY(ctx -> image2d_rgb_support));
        printf("Image2D CL_RGBA Support:     %s\n\n", STRINGIFY(ctx -> image2d_rgba_support));
    }

    printf("Unified Memory Support:      %s\n--------", STRINGIFY(ctx -> usm_support));

    printf("\n\n");
}

/**
 * Frees the created `ComputeContext`.
 * 
 * @param ctx The context to be freed
 */
static void cl_free_compute_context(ComputeContext ctx) {
    if (ctx.spirv_version_str != NULL)
        free(ctx.spirv_version_str);

    DEVICE_FEATURE_COUNT = 0;
    free(ALL_DEVICE_FEATURES);
    free(ALL_DEVICE_EXTENSIONS);

    clReleaseContext(ctx.context);
    clReleaseDevice(ctx.device);
}