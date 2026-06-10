#pragma once

#include "cl_common.h"

#include <stdio.h>

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