#pragma once

#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>
#include <CL/cl_ext.h>

#include <stdint.h>

// Holds OpenCL primitives and important info required
// by the program to function properly
typedef struct ComputeContext {
    cl_platform_id platform;    // The chosen OpenCL Platform
    cl_device_id device;        // The chosen Platform Device
    cl_context context;         // Corresponding Device Context

    size_t max_workgroup_size;  // Max number of Threads per WorkGroup
    cl_uint max_compute_units;  // Number of Compute Units

    float spirv_version;        // SPIRV version, should be >= 1.0 if supported
    uint8_t uses_spirv_khr;     // Represents whether SPIRV uses the KHR extension

    uint8_t cl_30_support;      // OpenCL 3.0 Support
    uint8_t cl_21_support;      // OpenCL 2.1 Support
    uint8_t cl_20_support;      // OpenCL 2.0 Support

    uint8_t image_support;      // Image Support
    uint8_t usm_support;        // Represents whether the device supports shared memory
} ComputeContext;