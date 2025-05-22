/**
 * @file Types.h
 * @date Mon, 8th January 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) 2024 RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_TYPE_H
#define REAI_TYPE_H

// clang-format off
typedef signed char      i8;
typedef signed short     i16;
typedef signed int       i32;
typedef signed long long i64;

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef unsigned long size;

typedef float  f32;
typedef double f64;

#ifndef bool
typedef i8 bool;
#endif

#define MIN2(x, y) ((x) < (y) ? (x) : (y))
#define MAX2(x, y) ((x) > (y) ? (x) : (y))
#define CLAMP(x, lo, hi) MIN2(MAX2(lo, x), hi)

// for any general alignment value (13, 8, 17, 144, etc...)
#define ALIGN_UP(value, alignment)                                                                 \
    ((alignment) > 1 ? (((value) + (alignment) - 1) / (alignment) * (alignment)) : (value))

#define ALIGN_DOWN(value, alignment)                                                               \
    ((alignment) > 1 ? ((value) / (alignment) * (alignment)) : (value))

// for alignment value that is power of two, (2, 4, 8, 16, 32, ...)
#define ALIGN_UP_POW2(value, alignment)                                                            \
    ((alignment) > 1 ? (((value) + (alignment) - 1) & ~((alignment) - 1)) : (value))

#define ALIGN_DOWN_POW2(value, alignment)                                                          \
    ((alignment) > 1 ? ((value) & ~((alignment) - 1)) : (value))

#ifndef true
#    define true 1
#endif

#ifndef false
#    define false 0
#endif

#ifndef NULL
#    define NULL 0
#endif

#define NEW(tname) calloc(1, sizeof(tname))
#define FREE(x)    (free((void *)(x)), (x) = NULL)

/// Compatibility macro between MSVC and GCC/Clang
#if defined(_MSC_VER)
#    define FORMAT_STRING(fmt_pos, va_arg_pos)
#else
#    define FORMAT_STRING(fmt_pos, va_arg_pos) __attribute((format(printf, fmt_pos, va_arg_pos)))
#endif

// clang-format on

#ifndef SIZE_MAX
#    define SIZE_MAX ((size)1 << (sizeof (size) * 8 - 1))
#endif

#ifndef UINT32_MAX
#    define UINT32_MAX ((u32)1 << (sizeof (u32) * 8 - 1))
#endif

#endif // REAI_TYPE_H
