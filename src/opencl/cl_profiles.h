#pragma once

#include "cl_common.h"
#include "cl_utils.h"

/**
 * Checks whether the given context supports SVM, OpenCL 3.0, (Non-KHR) SPIRV-1.0 and Images.
 * 
 * @param ctx The ComputeContext struct
 * 
 * @return `CL_TRUE` if supported, else `CL_FALSE`
 */
static uint8_t IGPU_CL30_SPIRV_IMAGE_PROFILE(const ComputeContext *const ctx) {
    return
        ctx -> cl_20_support == CL_TRUE &&
        ctx -> cl_21_support == CL_TRUE &&
        ctx -> cl_30_support == CL_TRUE &&

        ctx -> spirv_versions_str != NULL &&
        ctx -> uses_spirv_khr == CL_FALSE &&

        ctx -> usm_support == CL_TRUE &&

        ctx -> image_support == CL_TRUE;
}

/**
 * Checks whether the given context supports SVM, OpenCL 3.0, (KHR) SPIRV-1.0 and Images.
 * 
 * @param ctx The ComputeContext struct
 * 
 * @return `CL_TRUE` if supported, else `CL_FALSE`
 */
static uint8_t IGPU_CL30_KHRSPIRV_IMAGE_PROFILE(const ComputeContext *const ctx) {
    return
        ctx -> cl_20_support == CL_TRUE &&
        ctx -> cl_30_support == CL_TRUE &&

        ctx -> spirv_versions_str != NULL &&
        is_device_extension_available(ctx -> device, "cl_khr_il_program") &&

        ctx -> usm_support == CL_TRUE &&

        ctx -> image_support == CL_TRUE;
}

/**
 * Checks whether the given context supports SVM, OpenCL 3.0 and Images.
 * 
 * @param ctx The ComputeContext struct
 * 
 * @return `CL_TRUE` if supported, else `CL_FALSE`
 */
static uint8_t IGPU_CL30_IMAGE_PROFILE(const ComputeContext *const ctx) {
    return
        ctx -> cl_20_support == CL_TRUE &&
        ctx -> cl_30_support == CL_TRUE &&

        ctx -> usm_support == CL_TRUE &&

        ctx -> image_support == CL_TRUE;
}

/**
 * Checks whether the given context supports SVM and Images.
 * 
 * @param ctx The ComputeContext struct
 * 
 * @return `CL_TRUE` if supported, else `CL_FALSE`
 */
static uint8_t IGPU_IMAGE_PROFILE(const ComputeContext *const ctx) {
    return
        ctx -> cl_20_support == CL_TRUE &&
        
        ctx -> usm_support == CL_TRUE &&

        ctx -> image_support == CL_TRUE;
}

/**
 * Checks whether the given context supports OpenCL 3.0, (Non-KHR) SPIRV-1.0 and Images.
 * 
 * @param ctx The ComputeContext struct
 * 
 * @return `CL_TRUE` if supported, else `CL_FALSE`
 */
static uint8_t DGPU_CL30_SPIRV_IMAGE_PROFILE(const ComputeContext *const ctx) {
    return
        ctx -> cl_20_support == CL_TRUE &&
        ctx -> cl_21_support == CL_TRUE &&
        ctx -> cl_30_support == CL_TRUE &&

        ctx -> spirv_versions_str != NULL &&
        ctx -> uses_spirv_khr == CL_FALSE &&

        ctx -> usm_support == CL_FALSE &&

        ctx -> image_support == CL_TRUE;
}

/**
 * Checks whether the given context supports OpenCL 3.0, (KHR) SPIRV-1.0 and Images.
 * 
 * @param ctx The ComputeContext struct
 * 
 * @return `CL_TRUE` if supported, else `CL_FALSE`
 */
static uint8_t DGPU_CL30_KHRSPIRV_IMAGE_PROFILE(const ComputeContext *const ctx) {
    return
        ctx -> cl_30_support == CL_TRUE &&

        ctx -> spirv_versions_str != NULL &&
        is_device_extension_available(ctx -> device, "cl_khr_il_program") &&

        ctx -> usm_support == CL_FALSE &&

        ctx -> image_support == CL_TRUE;
}

/**
 * Checks whether the given context supports OpenCL 3.0 and Images.
 * 
 * @param ctx The ComputeContext struct
 * 
 * @return `CL_TRUE` if supported, else `CL_FALSE`
 */
static uint8_t DGPU_CL30_IMAGE_PROFILE(const ComputeContext *const ctx) {
    return
        ctx -> cl_30_support == CL_TRUE &&

        ctx -> usm_support == CL_FALSE &&

        ctx -> image_support == CL_TRUE;
}

/**
 * Checks whether the given context supports Images.
 * 
 * @param ctx The ComputeContext struct
 * 
 * @return `CL_TRUE` if supported, else `CL_FALSE`
 */
static uint8_t DGPU_IMAGE_PROFILE(const ComputeContext *const ctx) {
    return
        ctx -> usm_support == CL_FALSE &&

        ctx -> image_support == CL_TRUE;
}