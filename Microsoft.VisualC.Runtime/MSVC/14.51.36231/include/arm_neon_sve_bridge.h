/***
*   arm_neon_sve_bridge.h - declarations/definitions for ARM64 SVE/NEON bridge specific intrinsics
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This include file contains the declarations for ARM64 SVE/NEON bridge intrinsic functions.
*
****/

#pragma once

#if !defined (_M_ARM64) && !defined(_M_ARM64EC)
#   error "This header is specific to ARM64 target"
#endif  // !_M_ARM64 && !_M_ARM64EC

#ifdef  __cplusplus
#   include <cstdint>
#else
#   include <stdbool.h>
#   include <stdint.h>
#endif

#include <arm_neon.h>
#include <arm_sve.h>

#ifdef  __cplusplus
extern "C" {
#endif

// These intrinsics set the first 128 bits of SVE vector vec to subvec.
svint8_t svset_neonq_s8(svint8_t vec, int8x16_t subvec);
svint16_t svset_neonq_s16(svint16_t vec, int16x8_t subvec);
svint32_t svset_neonq_s32(svint32_t vec, int32x4_t subvec);
svint64_t svset_neonq_s64(svint64_t vec, int64x2_t subvec);
svuint8_t svset_neonq_u8(svuint8_t vec, uint8x16_t subvec);
svuint16_t svset_neonq_u16(svuint16_t vec, uint16x8_t subvec);
svuint32_t svset_neonq_u32(svuint32_t vec, uint32x4_t subvec);
svuint64_t svset_neonq_u64(svuint64_t vec, uint64x2_t subvec);
svfloat32_t svset_neonq_f32(svfloat32_t vec, float32x4_t subvec);
svfloat64_t svset_neonq_f64(svfloat64_t vec, float64x2_t subvec);
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define svset_neonq(vec, subvec) _Generic((vec), \
    svint64_t: svset_neonq_s64, \
    svint32_t: svset_neonq_s32, \
    svint16_t: svset_neonq_s16, \
    svint8_t: svset_neonq_s8, \
    svuint64_t: svset_neonq_u64, \
    svuint32_t: svset_neonq_u32, \
    svuint16_t: svset_neonq_u16, \
    svuint8_t: svset_neonq_u8, \
    svfloat64_t: svset_neonq_f64, \
    svfloat32_t: svset_neonq_f32, \
    default: __assume(0) \
    )(vec,subvec)
#endif

// These intrinsics get the first 128 bit subvector of SVE vector vec as a NEON vector.
int8x16_t svget_neonq_s8(svint8_t vec);
int16x8_t svget_neonq_s16(svint16_t vec);
int32x4_t svget_neonq_s32(svint32_t vec);
int64x2_t svget_neonq_s64(svint64_t vec);
uint8x16_t svget_neonq_u8(svuint8_t vec);
uint16x8_t svget_neonq_u16(svuint16_t vec);
uint32x4_t svget_neonq_u32(svuint32_t vec);
uint64x2_t svget_neonq_u64(svuint64_t vec);
float32x4_t svget_neonq_f32(svfloat32_t vec);
float64x2_t svget_neonq_f64(svfloat64_t vec);
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define svget_neonq(vec) _Generic((vec), \
    svint64_t: svget_neonq_s64, \
    svint32_t: svget_neonq_s32, \
    svint16_t: svget_neonq_s16, \
    svint8_t: svget_neonq_s8, \
    svuint64_t: svget_neonq_u64, \
    svuint32_t: svget_neonq_u32, \
    svuint16_t: svget_neonq_u16, \
    svuint8_t: svget_neonq_u8, \
    svfloat64_t: svget_neonq_f64, \
    svfloat32_t: svget_neonq_f32, \
    default: __assume(0) \
    )(vec)
#endif

// These intrinsics return an SVE vector with all SVE subvectors containing the duplicated NEON vector vec.
svint8_t svdup_neonq_s8(int8x16_t vec);
svint16_t svdup_neonq_s16(int16x8_t vec);
svint32_t svdup_neonq_s32(int32x4_t vec);
svint64_t svdup_neonq_s64(int64x2_t vec);
svuint8_t svdup_neonq_u8(uint8x16_t vec);
svuint16_t svdup_neonq_u16(uint16x8_t vec);
svuint32_t svdup_neonq_u32(uint32x4_t vec);
svuint64_t svdup_neonq_u64(uint64x2_t vec);
svfloat32_t svdup_neonq_f32(float32x4_t vec);
svfloat64_t svdup_neonq_f64(float64x2_t vec);
#if defined(_ARM64_DISTINCT_NEON_TYPES) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define svdup_neonq(vec) _Generic((vec), \
    int64x2_t: svdup_neonq_s64, \
    int32x4_t: svdup_neonq_s32, \
    int16x8_t: svdup_neonq_s16, \
    int8x16_t: svdup_neonq_s8, \
    uint64x2_t: svdup_neonq_u64, \
    uint32x4_t: svdup_neonq_u32, \
    uint16x8_t: svdup_neonq_u16, \
    uint8x16_t: svdup_neonq_u8, \
    float64x2_t: svdup_neonq_f64, \
    float32x4_t: svdup_neonq_f32, \
    default: __assume(0) \
    )(vec)
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#if defined(__cplusplus)
__forceinline svint8_t svset_neonq(svint8_t vec, int8x16_t subvec) { return svset_neonq_s8(vec, subvec); }
__forceinline svint16_t svset_neonq(svint16_t vec, int16x8_t subvec) { return svset_neonq_s16(vec, subvec); }
__forceinline svint32_t svset_neonq(svint32_t vec, int32x4_t subvec) { return svset_neonq_s32(vec, subvec); }
__forceinline svint64_t svset_neonq(svint64_t vec, int64x2_t subvec) { return svset_neonq_s64(vec, subvec); }
__forceinline svuint8_t svset_neonq(svuint8_t vec, uint8x16_t subvec) { return svset_neonq_u8(vec, subvec); }
__forceinline svuint16_t svset_neonq(svuint16_t vec, uint16x8_t subvec) { return svset_neonq_u16(vec, subvec); }
__forceinline svuint32_t svset_neonq(svuint32_t vec, uint32x4_t subvec) { return svset_neonq_u32(vec, subvec); }
__forceinline svuint64_t svset_neonq(svuint64_t vec, uint64x2_t subvec) { return svset_neonq_u64(vec, subvec); }
__forceinline svfloat32_t svset_neonq(svfloat32_t vec, float32x4_t subvec) { return svset_neonq_f32(vec, subvec); }
__forceinline svfloat64_t svset_neonq(svfloat64_t vec, float64x2_t subvec) { return svset_neonq_f64(vec, subvec); }

__forceinline int8x16_t svget_neonq(svint8_t vec) { return svget_neonq_s8(vec); }
__forceinline int16x8_t svget_neonq(svint16_t vec) { return svget_neonq_s16(vec); }
__forceinline int32x4_t svget_neonq(svint32_t vec) { return svget_neonq_s32(vec); }
__forceinline int64x2_t svget_neonq(svint64_t vec) { return svget_neonq_s64(vec); }
__forceinline uint8x16_t svget_neonq(svuint8_t vec) { return svget_neonq_u8(vec); }
__forceinline uint16x8_t svget_neonq(svuint16_t vec) { return svget_neonq_u16(vec); }
__forceinline uint32x4_t svget_neonq(svuint32_t vec) { return svget_neonq_u32(vec); }
__forceinline uint64x2_t svget_neonq(svuint64_t vec) { return svget_neonq_u64(vec); }
__forceinline float32x4_t svget_neonq(svfloat32_t vec) { return svget_neonq_f32(vec); }
__forceinline float64x2_t svget_neonq(svfloat64_t vec) { return svget_neonq_f64(vec); }

#if defined(_ARM64_DISTINCT_NEON_TYPES)
__forceinline svint8_t svdup_neonq(int8x16_t vec) { return svdup_neonq_s8(vec); }
__forceinline svint16_t svdup_neonq(int16x8_t vec) { return svdup_neonq_s16(vec); }
__forceinline svint32_t svdup_neonq(int32x4_t vec) { return svdup_neonq_s32(vec); }
__forceinline svint64_t svdup_neonq(int64x2_t vec) { return svdup_neonq_s64(vec); }
__forceinline svuint8_t svdup_neonq(uint8x16_t vec) { return svdup_neonq_u8(vec); }
__forceinline svuint16_t svdup_neonq(uint16x8_t vec) { return svdup_neonq_u16(vec); }
__forceinline svuint32_t svdup_neonq(uint32x4_t vec) { return svdup_neonq_u32(vec); }
__forceinline svuint64_t svdup_neonq(uint64x2_t vec) { return svdup_neonq_u64(vec); }
__forceinline svfloat32_t svdup_neonq(float32x4_t vec) { return svdup_neonq_f32(vec); }
__forceinline svfloat64_t svdup_neonq(float64x2_t vec) { return svdup_neonq_f64(vec); }
#endif

#endif
