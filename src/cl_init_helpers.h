#pragma once

#include "cl_common.h"

#include <stdio.h>
#include <string.h>

#define STRINGIFY(b) ((b) ? "Yes" : "No")

static char *ALL_DEVICE_EXTENSIONS = NULL;
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

static cl_platform_id cl_choose_platform_id() {
    cl_platform_id chosen_platform = NULL;

    cl_uint num_platforms;
    clGetPlatformIDs(0, NULL, &num_platforms);
    cl_platform_id all_platforms[num_platforms];
    cl_int errcode = clGetPlatformIDs(num_platforms, all_platforms, NULL);

    if (errcode != CL_SUCCESS) {
        fprintf(stderr, "An Error occured while obtaining OpenCL platforms!\n");
        exit(1);
    }

    if (num_platforms == 1)
        return all_platforms[0];

    for (cl_uint i = 0; i < num_platforms; i++) {
        size_t vendor_name_len;
        clGetPlatformInfo(all_platforms[i], CL_PLATFORM_VENDOR, 0, NULL, &vendor_name_len);
        char vendor_name[vendor_name_len];
        clGetPlatformInfo(all_platforms[i], CL_PLATFORM_VENDOR, vendor_name_len, vendor_name, NULL);

        size_t platform_name_len;
        clGetPlatformInfo(all_platforms[i], CL_PLATFORM_NAME, 0, NULL, &platform_name_len);
        char platform_name[platform_name_len];
        clGetPlatformInfo(all_platforms[i], CL_PLATFORM_NAME, platform_name_len, platform_name, NULL);

        printf(
            "%u] %s: %s\n",
            i + 1,
            vendor_name,
            platform_name
        );
    }

    printf("\nSelect Platform: ");
    cl_uint platform_no;
    scanf("%u", &platform_no);

    chosen_platform = all_platforms[platform_no - 1];
    return chosen_platform;
}

static cl_device_id cl_choose_device_id(cl_platform_id chosen_platform) {
    cl_device_id chosen_device = NULL;

    cl_uint num_devices;
    clGetDeviceIDs(chosen_platform, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
    cl_device_id all_devices[num_devices];
    cl_int errcode = clGetDeviceIDs(chosen_platform, CL_DEVICE_TYPE_ALL, num_devices, all_devices, NULL);

    if (errcode != CL_SUCCESS) {
        fprintf(stderr, "An Error occured while obtaining Platform devices!\n");
        exit(1);
    }

    if (num_devices == 1)
        return all_devices[0];

    for (cl_uint i = 0; i < num_devices; i++) {
        size_t device_name_len;
        clGetDeviceInfo(all_devices[i], CL_DEVICE_NAME, 0, NULL, &device_name_len);
        char device_name[device_name_len];
        clGetDeviceInfo(all_devices[i], CL_DEVICE_NAME, device_name_len, device_name, NULL);

        printf(
            "%u] %s\n",
            i + 1,
            device_name
        );
    }

    printf("\nSelect Device: ");
    cl_uint device_no;
    scanf("%u", &device_no);

    chosen_device = all_devices[device_no - 1];
    return chosen_device;
}

typedef struct {
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    
    char *spirv_version_str;

    size_t max_workgroup_size;
    cl_uint max_compute_units;

    cl_bool cl_30_support;
    cl_bool cl_21_support;
    cl_bool cl_20_support;

    cl_bool is_spirv_khr;

    cl_bool image_support;
    cl_bool usm_support;
} ComputeContext;

static void cl_print_context_info(ComputeContext *ctx) {
    printf("\nCompute Context Info:\n");

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
        printf("SPIRV-KHR Support:           %s\n\n", STRINGIFY(ctx -> is_spirv_khr));
    }

    printf("OpenCLv3.0 Support:          %s\n", STRINGIFY(ctx -> cl_30_support));
    printf("OpenCLv2.1 Support:          %s\n", STRINGIFY(ctx -> cl_21_support));
    printf("OpenCLv2.0 Support:          %s\n\n", STRINGIFY(ctx -> cl_20_support));

    printf("Image Support:               %s\n", STRINGIFY(ctx -> image_support));
    printf("Unified Memory Support:      %s\n", STRINGIFY(ctx -> usm_support));

    printf("\n");
}