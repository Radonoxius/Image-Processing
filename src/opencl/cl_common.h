#pragma once

#define CL_TARGET_OPENCL_VERSION 300

#include <CL/cl.h>
#include <CL/cl_ext.h>

#include <stdint.h>

// Holds OpenCL primitives and important info required
// by the program to function optimally
typedef struct ComputeContext {
    cl_platform_id platform;    // The chosen OpenCL Platform
    cl_device_id device;        // The chosen Platform Device
    cl_context context;         // Corresponding Device Context

    char *spirv_versions_str;   // String containing all supported SPIRV-IL version(s). NULL if no SPIRV support

    size_t max_workgroup_size;  // Max number of Threads per WorkGroup
    size_t profile_timer_res;   // Profiling Timer Resolution in nanoseconds
    cl_uint max_compute_units;  // Number of Compute Units
    cl_uint cacheline_size;     // Size of the cache-line in bytes

    cl_uint image_pitch_alignment;         // Row pitch alignment size in pixels for 2D Images created from buffers
    cl_uint image_address_alignment;       // Minimum alignment in pixels for 2D Images created from buffers

    uint8_t non_uniform_workgroup_support; // Non-Uniform Workgroup Support
    uint8_t host_queue_profiling_support;  // Represents whether Host side Command Queue supports profiling

    uint8_t uses_spirv_khr;     // Represents whether `spirv_versions_str` was obtained using KHR extensions, if non-NULL

    uint8_t cl_30_support;      // OpenCL 3.0 Support
    uint8_t cl_21_support;      // OpenCL 2.1 Support
    uint8_t cl_20_support;      // OpenCL 2.0 Support

    uint8_t image_support;      // Image Support
    uint8_t usm_support;        // Represents whether the device supports shared memory
} ComputeContext;