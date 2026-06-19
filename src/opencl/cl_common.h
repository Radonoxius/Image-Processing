#pragma once

#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>
#include <CL/cl_ext.h>

// Holds OpenCL primitives and important info required
// by the program to function properly
typedef struct ComputeContext {
    cl_platform_id platform;    // The chosen OpenCL Platform
    cl_device_id device;        // The chosen Platform Device
    cl_context context;         // Corresponding Device Context

    size_t max_workgroup_size;  // Max number of Threads per WorkGroup
    cl_uint max_compute_units;  // Number of Compute Units

    cl_bool cl_30_support;      // OpenCL 3.0 Support
    cl_bool cl_21_support;      // OpenCL 2.1 Support
    cl_bool cl_20_support;      // OpenCL 2.0 Support

    float spirv_version;        // SPIRV version, should be >= 1.0 if supported
    cl_bool uses_spirv_khr;     // Represents whether SPIRV uses the KHR extension

    cl_bool image_support;      // Image Support

    cl_bool usm_support;        // Represents whether the device supports shared memory
} ComputeContext;