#pragma once

#include "cl_init_helpers.h"

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

    if (errcode != CL_SUCCESS) {
        cl_bool r = is_device_extension_available(device, "cl_khr_il_program");

        if (il_str != NULL)
            free(il_str);

        if (r == CL_TRUE) {
            clGetDeviceInfo(device, CL_DEVICE_IL_VERSION_KHR, 0, NULL, &il_str_len);
            il_str = (char *) malloc(il_str_len);
            clGetDeviceInfo(device, CL_DEVICE_IL_VERSION_KHR, il_str_len, il_str, NULL);

            ctx.spirv_version_str = il_str;
            ctx.is_spirv_khr = CL_TRUE;
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

    return ctx;
}

static void cl_free_compute_context(ComputeContext ctx) {
    if (ctx.spirv_version_str != NULL)
        free(ctx.spirv_version_str);

    free(ALL_DEVICE_EXTENSIONS);

    clReleaseContext(ctx.context);
    clReleaseDevice(ctx.device);
}