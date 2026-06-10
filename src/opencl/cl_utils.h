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