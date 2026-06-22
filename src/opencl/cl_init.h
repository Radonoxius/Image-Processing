#pragma once

#include "cl_init_utils.h"
#include "cl_profiles.h"

/**
 * Initializes a simple OpenCL environment.
 * 
 * Lets the user dynamically choose the Platform and
 * Device (if there are multiple available)
 * 
 * @return Valid `ComputeContext` on success, Zeroed struct on failure.
 */
static ComputeContext init() {
    const cl_platform_id platform = choose_platform_id();
    cl_device_id device = choose_device_id(platform);
    
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

    cl_bool img_s;
    clGetDeviceInfo(device, CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool), &img_s, NULL);
    ctx.image_support = (uint8_t) img_s;
    cl_bool usm_s;
    clGetDeviceInfo(device, CL_DEVICE_HOST_UNIFIED_MEMORY, sizeof(cl_bool), &usm_s, NULL);
    ctx.usm_support = (uint8_t) usm_s;

    size_t il_str_len;
    cl_int errcode = clGetDeviceInfo(device, CL_DEVICE_IL_VERSION, 0, NULL, &il_str_len);
    char *il_str = NULL;

    if (errcode != CL_SUCCESS || il_str_len < 7) {
        cl_bool r = is_device_extension_available(device, "cl_khr_il_program");

        if (r == CL_TRUE) {
            clGetDeviceInfo(device, CL_DEVICE_IL_VERSION_KHR, 0, NULL, &il_str_len);
            il_str = (char *) malloc(il_str_len);
            clGetDeviceInfo(device, CL_DEVICE_IL_VERSION_KHR, il_str_len, il_str, NULL);

            ctx.uses_spirv_khr = CL_TRUE;
        }
    } else {
        il_str = (char *) malloc(il_str_len);
        clGetDeviceInfo(device, CL_DEVICE_IL_VERSION, il_str_len, il_str, NULL);

        ctx.cl_21_support = CL_TRUE;
        ctx.cl_20_support = CL_TRUE;
    }

    if (il_str != NULL && il_str_len > 7)
        ctx.spirv_versions_str = il_str;

    cl_version device_version;
    errcode = clGetDeviceInfo(device, CL_DEVICE_NUMERIC_VERSION, sizeof(cl_version), &device_version, NULL);
    if (errcode == CL_SUCCESS && CL_VERSION_MAJOR(device_version) == 3)
        ctx.cl_30_support = CL_TRUE;

    if (ctx.cl_30_support == CL_TRUE) {
        ctx.image_support = ctx.image_support &&
            is_device_feature_available(&ctx, "__opencl_c_images");

        cl_bool nuw_s;
        clGetDeviceInfo(device, CL_DEVICE_HOST_UNIFIED_MEMORY, sizeof(cl_bool), &nuw_s, NULL);
        ctx.non_uniform_workgroup_support = (uint8_t) nuw_s;
    }

    return ctx;
}

/**
 * Prints basic info about the given context.
 * 
 * @param ctx A pointer to the context whose info
 * is printed.
 */
static void print_context_info(const ComputeContext *const ctx, uint8_t show_profile_support) {
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
    printf("Max # Threads per WorkGroup: %lu\n", ctx -> max_workgroup_size);
    printf("Non-Uniform WorkGroups:      %s\n\n", STRINGIFY(ctx -> non_uniform_workgroup_support));

    if (ctx -> spirv_versions_str != NULL) {
        printf("SPIRV versions:              %s\n", ctx -> spirv_versions_str);
        printf("SPIRV KHR Extension:         %s\n\n", STRINGIFY(ctx -> uses_spirv_khr));
    }

    printf("OpenCLv3.0 Support:          %s\n", STRINGIFY(ctx -> cl_30_support));
    printf("OpenCLv2.1 Support:          %s\n", STRINGIFY(ctx -> cl_21_support));
    printf("OpenCLv2.0 Support:          %s\n\n", STRINGIFY(ctx -> cl_20_support));

    printf("Image Support:               %s\n", STRINGIFY(ctx -> image_support));

    if (show_profile_support == CL_TRUE) {
        printf("Unified Memory Support:      %s\n\n", STRINGIFY(ctx -> usm_support));
        printf("Supported Profiles:\n");
        printf("IGPU_CL30_SPIRV_IMAGE_PROFILE:    %s\n", STRINGIFY(IGPU_CL30_SPIRV_IMAGE_PROFILE(ctx)));
        printf("IGPU_CL30_KHRSPIRV_IMAGE_PROFILE: %s\n", STRINGIFY(IGPU_CL30_KHRSPIRV_IMAGE_PROFILE(ctx)));
        printf("IGPU_CL30_IMAGE_PROFILE:          %s\n", STRINGIFY(IGPU_CL30_IMAGE_PROFILE(ctx)));
        printf("IGPU_IMAGE_PROFILE:               %s\n\n", STRINGIFY(IGPU_IMAGE_PROFILE(ctx)));
        printf("DGPU_CL30_SPIRV_IMAGE_PROFILE:    %s\n", STRINGIFY(DGPU_CL30_SPIRV_IMAGE_PROFILE(ctx)));
        printf("DGPU_CL30_KHRSPIRV_IMAGE_PROFILE: %s\n", STRINGIFY(DGPU_CL30_KHRSPIRV_IMAGE_PROFILE(ctx)));
        printf("DGPU_CL30_IMAGE_PROFILE:          %s\n", STRINGIFY(DGPU_CL30_IMAGE_PROFILE(ctx)));
        printf("DGPU_IMAGE_PROFILE:               %s\n--------", STRINGIFY(DGPU_IMAGE_PROFILE(ctx)));
    }
    else
        printf("Unified Memory Support:      %s\n--------", STRINGIFY(ctx -> usm_support));

    printf("\n\n");
}

/**
 * Frees the created `ComputeContext`.
 * 
 * @param ctx The context to be freed
 */
static void free_compute_context(ComputeContext ctx) {
    IMAGE2D_FORMAT_COUNT = 0;
    free(ALL_IMAGE2D_FORMATS);

    DEVICE_FEATURE_COUNT = 0;
    free(ALL_DEVICE_FEATURES);

    free(ALL_DEVICE_EXTENSIONS);

    if (ctx.spirv_versions_str != NULL)
        free(ctx.spirv_versions_str);

    clReleaseContext(ctx.context);
    clReleaseDevice(ctx.device);
}