#pragma once

#include "cl_common.h"

#include <string.h>

// Converts `cl_bool` to a string (Yes/No)
#define STRINGIFY(b) ((b) ? "Yes" : "No")

static char *ALL_DEVICE_EXTENSIONS = NULL;

/**
 * Checks whether the given device extension is supported.
 * 
 * @param device The OpenCL device whose extensions are
 * checked against
 * @param required_extension_name The name of the extension
 * whose support status is required.
 * 
 * @return `CL_TRUE` if supported, else `CL_FALSE`
 */
static cl_bool cl_device_extension_available(const cl_device_id device, const char *const required_extension_name) {
    if (ALL_DEVICE_EXTENSIONS == NULL) {
        size_t extensions_str_len;
        clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, 0, NULL, &extensions_str_len);
        ALL_DEVICE_EXTENSIONS = (char *) malloc(extensions_str_len);
        clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, extensions_str_len, ALL_DEVICE_EXTENSIONS, NULL);
    }
    
    char* res = strstr(ALL_DEVICE_EXTENSIONS, required_extension_name);
    if (res == NULL)
        return CL_FALSE;
    else
        return CL_TRUE;
}

static cl_name_version *ALL_DEVICE_FEATURES = NULL;
static size_t DEVICE_FEATURE_COUNT = 0;

/**
 * Checks whether the given device (OpenCL C) feature is supported.
 * 
 * @param ctx The ComputeContext struct
 * @param required_feature_name The name of the feature
 * whose support status is required.
 * 
 * @return `CL_TRUE` if supported, else `CL_FALSE`
 */
static cl_bool cl_device_feature_available(const ComputeContext *const ctx, const char *const required_feature_name) {
    if (ctx -> cl_30_support == CL_TRUE) {
        if (ALL_DEVICE_FEATURES == NULL) {
            clGetDeviceInfo(ctx -> device, CL_DEVICE_OPENCL_C_FEATURES, 0, NULL, &DEVICE_FEATURE_COUNT);
            ALL_DEVICE_FEATURES = (cl_name_version *) malloc(DEVICE_FEATURE_COUNT * sizeof(cl_name_version));
            clGetDeviceInfo(ctx -> device, CL_DEVICE_OPENCL_C_FEATURES, DEVICE_FEATURE_COUNT, ALL_DEVICE_FEATURES, NULL);
        }

        for (size_t i = 0; i < DEVICE_FEATURE_COUNT; i++) {
            char* res = strstr(ALL_DEVICE_FEATURES[i].name, required_feature_name);
            if (res != NULL)
                return CL_TRUE;
        }
    }

    return CL_FALSE;
}

static cl_image_format *ALL_IMAGE2D_FORMATS = NULL;
static cl_uint IMAGE2D_FORMAT_COUNT = 0;

/**
 * Checks whether the required `image2d_t` format is supported.
 * 
 * @param ctx The ComputeContext struct
 * @param channel The type of channel required,
 * eg: CL_RGBA, CL_RGB etc.
 * @param channel_datatype The datatype of the specified channel.
 * eg: CL_UNSIGNED_INT8 etc.
 * 
 * @return `CL_TRUE` if supported, else `CL_FALSE`
 */
static cl_bool cl_image2d_format_available(
    const ComputeContext *const ctx,
    const cl_channel_order channel,
    const cl_channel_type channel_datatype
) {
    if (ctx -> image_support == CL_TRUE) {
        if (ALL_IMAGE2D_FORMATS == NULL) {
            clGetSupportedImageFormats(
                ctx -> context,
                CL_MEM_READ_WRITE,
                CL_MEM_OBJECT_IMAGE2D,
                0,
                NULL,
                &IMAGE2D_FORMAT_COUNT
            );
            ALL_IMAGE2D_FORMATS = (cl_image_format *) malloc(IMAGE2D_FORMAT_COUNT * sizeof(cl_image_format));
            clGetSupportedImageFormats(
                ctx -> context,
                CL_MEM_READ_WRITE,
                CL_MEM_OBJECT_IMAGE2D,
                IMAGE2D_FORMAT_COUNT,
                ALL_IMAGE2D_FORMATS,
                NULL
            );
        }
    
        for (cl_uint i = 0; i < IMAGE2D_FORMAT_COUNT; i++) {
            if (
                ALL_IMAGE2D_FORMATS[i].image_channel_order == channel &&
                ALL_IMAGE2D_FORMATS[i].image_channel_data_type == channel_datatype
            )
                return CL_TRUE;
        }
    }

    return CL_FALSE;
}