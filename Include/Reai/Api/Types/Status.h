/**
 * @file AiDecompilation.h
 * @date 18th May 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_API_TYPES_STATUS_H
#define REAI_API_TYPES_STATUS_H

#include <Reai/Util/Str.h>

#define STATUS_MASK 0xf

#ifdef __cplusplus
extern "C" {
#endif

    ///
    /// A common enum to represent statuses of an existing analysis or a dynamic execution
    ///
    /// Enum values are flagged with corresponding source. If the source is an analysis status
    /// then it'll be or'ed with "ANALYSIS_STATUS" and if it's dynamic execution status then it'll
    /// be or'red with 'DYN_EXEC_STATUS'
    ///
    /// This is a clever way to merge status enums from two different endpoints
    ///
    typedef enum Status {
        STATUS_INVALID = 0,

        STATUS_PROCESSING = 1,
        STATUS_COMPLETE   = 2,
        STATUS_UPLOADED   = 3,
        STATUS_ERROR      = 4,
        STATUS_ALL        = 5,
        STATUS_QUEUED     = 6,

        STATUS_MAX
    } Status;

#ifndef STATUS_ENUM_ALIASES_AND_MASK_DEFINED
#define STATUS_ENUM_ALIASES_AND_MASK_DEFINED

#define STATUS_PENDING STATUS_PROCESSING
#define STATUS_RUNNING STATUS_PROCESSING
#define STATUS_SUCCESS STATUS_COMPLETE
#define STATUS_UNINITIALIZED STATUS_QUEUED

/// Flagged in Status if the source is an analysis status
#define ANALYSIS_STATUS (1 << 5)

/// Flagged if source is dynamic execution status
#define DYN_EXEC_STATUS (1 << 6)

/// Flagged if source is dynamic execution status
#define AI_DECOMP_STATUS (1 << 7)

#endif

    ///
    /// Convert given status to a string and store it in given Str object.
    /// status[in] : Status enum to be converted.
    /// str[out]   : Str object to return the status string into.
    ///
    /// RETURN : Does not return on failure
    ///
    REAI_API void StatusToStr (Status status, Str* str);

    ///
    /// Convert the value given in Str object to corresponding Status enum.
    ///
    /// str[out]   : Str object to read the status string from.
    ///
    /// RETURN : Does not return on failure
    ///
    REAI_API Status StatusFromStr (Str* str);

#ifdef __cplusplus
}
#endif

#endif // REAI_API_TYPES_STATUS_H
