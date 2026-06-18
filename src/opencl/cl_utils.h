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
static cl_bool is_device_extension_available(cl_device_id device, char *required_extension_name) {
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
 * @param device The OpenCL device whose features are
 * checked against
 * @param required_feature_name The name of the feature
 * whose support status is required.
 * 
 * @return `CL_TRUE` if supported, else `CL_FALSE`
 */
static cl_bool is_device_feature_available(cl_device_id device, char *required_feature_name) {
    if (ALL_DEVICE_FEATURES == NULL) {
        clGetDeviceInfo(device, CL_DEVICE_OPENCL_C_FEATURES, 0, NULL, &DEVICE_FEATURE_COUNT);
        ALL_DEVICE_FEATURES = (cl_name_version *) malloc(DEVICE_FEATURE_COUNT * sizeof(cl_name_version));
        clGetDeviceInfo(device, CL_DEVICE_OPENCL_C_FEATURES, DEVICE_FEATURE_COUNT, ALL_DEVICE_FEATURES, NULL);
    }

    for (size_t i = 0; i < DEVICE_FEATURE_COUNT; i++) {
        char* res = strstr(ALL_DEVICE_FEATURES[i].name, required_feature_name);
        if (res != NULL)
            return CL_TRUE;
    }
}