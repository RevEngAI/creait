/**
 * @file Common.h
 * @date Sun, 21st January 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) 2024 RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_COMMON_H
#define REAI_COMMON_H

#include <Reai/Log.h>
#include <stdio.h>
#include <stdlib.h>

/********************************** CONVINIENT WRAPPER MACROS *************************************/


#define CONSTRUCTOR  __attribute__ ((constructor))
#define DESTRUCTOR   __attribute__ ((destructor))
#define FORCE_INLINE __attribute__ ((always_inline))
#define UNUSED(x)    ((void)(x))
#ifdef __cplusplus
#    define NEW(type)                reinterpret_cast<type *> (calloc (1, sizeof (type)))
#    define ALLOCATE(type, n)        reinterpret_cast<type *> (calloc (n, sizeof (type)))
#    define REALLOCATE(ptr, type, n) reinterpret_cast<type *> (realloc (ptr, n * sizeof (type)))
#    define FREE(x)                  free (const_cast<void *> (reinterpret_cast<const void *> (x)))
#else
#    define NEW(type)                (type *)calloc (1, sizeof (type))
#    define ALLOCATE(type, n)        (type *)calloc (n, sizeof (type))
#    define REALLOCATE(ptr, type, n) (type *)realloc (ptr, n * sizeof (type));
#    define FREE(x)                                                                                \
        free ((void *)(x));                                                                        \
        x = NULL
#endif
#define PACKED __attribute__ ((packed))

#define ALIGN_UP(x, y) (x + (y - (x % y)))
#define ALIGN_DOWN(x, y) (x - (x % y)))

#ifndef OFFSET_OF
#    define OFFSET_OF(t, f) ((Size)(&((t *)0)->f))
#endif // OFFSET_OF

/************************************* MISC UTILITY MACROS ****************************************/




#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define MIN3(x, y, z) MIN (x, MIN (y, z))
#define MAX3(x, y, z) MAX (x, MAX (y, z))

#define CLAMP(x, y, z)   MIN (MAX (x, y), z)
#define ARRAY_SIZE(arr)  sizeof (arr) / sizeof (arr[0])
#define CHECK_ITER(b, e) (b && e && (b <= e))

#define HIDDEN  /* functions that are hidden and can be extern-ed to call them */
#define PUBLIC  /* functions that are public and can be used directly by including a header file */
#define PRIVATE static inline /* functions not visible outside compilation unit */




/************************************* ERROR HANDLER MACROS ***************************************/




#define ERR_INVALID_ARGUMENTS              "Invalid Arguments"
#define ERR_OUT_OF_MEMORY                  "Out of memory (allocation failed)"
#define ERR_INVALID_SIZE                   "Invalid size (zero)"
#define ERR_INVALID_OBJECT_REF             "Invalid object reference (NULL)"
#define ERR_INVALID_OBJECT_CONTENTS        "Invalid contents inside provided object (not what expected)"
#define ERR_INVALID_ITERATOR               "Invalid iterator (NULL)"
#define ERR_OBJECT_INITIALIZATION_FAILED   "Failed to initialze object"
#define ERR_OBJECT_DEINITIALIZATION_FAILED "Failed to de-initialze object"

#define ERR_FILE_OPEN_FAILED        "Failed to open file (NULL returned)"
#define ERR_FILE_SEEK_FAILED        "Failed to seek/tell file position"
#define ERR_FILE_READ_FAILED        "Failed to read file"
#define ERR_UNSUPPORTED_FILE_FORMAT "Unsupported file format"

/**
 * @brief Macro to return a value if a condition is met, with error message.
 * @param cond Condition to evaluate.
 * @param value Value to return if condition is true.
 * @param ... Variadic arguments for error message.
 */
#ifndef RETURN_VALUE_IF
#    define RETURN_VALUE_IF(cond, value, ...)                                                      \
        do {                                                                                       \
            if ((cond)) {                                                                          \
                REAI_LOG_ERROR (__VA_ARGS__);                                                      \
                return value;                                                                      \
            }                                                                                      \
        } while (0)
#endif

/**
 * @brief Macro to return if a condition is met, with error message.
 * @param cond Condition to evaluate.
 * @param ... Variadic arguments for error message.
 */
#ifndef RETURN_IF
#    define RETURN_IF(cond, ...)                                                                   \
        do {                                                                                       \
            if ((cond)) {                                                                          \
                REAI_LOG_ERROR (__VA_ARGS__);                                                      \
                return;                                                                            \
            }                                                                                      \
        } while (0)
#endif

/**
 * @brief Macro to goto a handler with error message.
 * @param handler Handler to goto.
 * @param ... Variadic arguments for error message.
 */
#ifndef GOTO_HANDLER_IF_REACHED
#    define GOTO_HANDLER_IF_REACHED(handler, ...)                                                  \
        do {                                                                                       \
            REAI_LOG_ERROR (__VA_ARGS__);                                                          \
            goto handler;                                                                          \
        } while (0)
#endif

/**
 * @brief Macro to goto a handler if a condition is met, with error message.
 * @param cond Condition to evaluate.
 * @param handler Handler to goto.
 * @param ... Variadic arguments for error message.
 */
#ifndef GOTO_HANDLER_IF
#    define GOTO_HANDLER_IF(cond, handler, ...)                                                    \
        do {                                                                                       \
            if ((cond)) {                                                                          \
                REAI_LOG_ERROR (__VA_ARGS__);                                                      \
                goto handler;                                                                      \
            }                                                                                      \
        } while (0)
#endif

/**
 * @brief Macro to call a handler if a condition is met, with error message.
 * @param cond Condition to evaluate.
 * @param handler Handler to call.
 * @param ... Variadic arguments for error message.
 */
#ifndef CALL_HANDLER_IF
#    define CALL_HANDLER_IF(cond, handler, ...)                                                    \
        do {                                                                                       \
            if ((cond)) {                                                                          \
                REAI_LOG_ERROR (__VA_ARGS__);                                                      \
                handler;                                                                           \
            }                                                                                      \
        } while (0)
#endif

/**
 * @brief Macro to abort if a condition is met, with error message.
 * @param cond Condition to evaluate.
 * @param ... Variadic arguments for error message.
 */
#ifndef ABORT_IF
#    define ABORT_IF(cond, ...)                                                                    \
        do {                                                                                       \
            if ((cond)) {                                                                          \
                REAI_LOG_ERROR (__VA_ARGS__);                                                      \
                abort();                                                                           \
            }                                                                                      \
        } while (0)
#endif

/**
 * @brief Macro to return a value if unreachable code is reached, with error message.
 * @param val Value to return.
 * @param ... Variadic arguments for error message.
 */
#ifndef RETURN_VALUE_IF_REACHED
#    define RETURN_VALUE_IF_REACHED(val, ...)                                                      \
        do {                                                                                       \
            REAI_LOG_ERROR (__VA_ARGS__);                                                          \
            return val;                                                                            \
        } while (0)
#endif

/**
 * @brief Macro to return if unreachable code is reached, with error message.
 * @param ... Variadic arguments for error message.
 */
#ifndef RETURN_IF_REACHED
#    define RETURN_IF_REACHED(...)                                                                 \
        do {                                                                                       \
            REAI_LOG_ERROR (__VA_ARGS__);                                                          \
            return;                                                                                \
        } while (0)
#endif

/**
 * @brief Macro to abort if unreachable code is reached, with error message.
 * @param ... Variadic arguments for error message.
 */
#ifndef ABORT_IF_REACHED
#    define ABORT_IF_REACHED(...)                                                                  \
        do {                                                                                       \
            REAI_LOG_ERROR (__VA_ARGS__);                                                          \
            abort();                                                                               \
        } while (0)
#endif

/**
 * @brief Macro to print an error message.
 * @param ... Variadic arguments for error message.
 */
#ifndef PRINT_ERR
#    define PRINT_ERR(...)                                                                         \
        do {                                                                                       \
            REAI_LOG_ERROR (__VA_ARGS__);                                                          \
        } while (0)
#endif

#endif // REAI_COMMON_H
