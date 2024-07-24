/**
 * @file AnalysisInfo.c
 * @date 23rd July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#include <Reai/Util/AnalysisInfo.h>

#include "Reai/Common.h"

/**
 * @b Convert given @c ReaiAnalysisStatus to @c CString.
 *
 * @param status
 * @return static @c CString value. Must not be freed by caller.
 * @return @c Null if status is invalid
 * */
CString reai_analysis_status_to_cstr (ReaiAnalysisStatus status) {
    RETURN_VALUE_IF (!status || status >= REAI_ANALYSIS_STATUS_MAX, Null, ERR_INVALID_ARGUMENTS);

    static const CString status_strings[] = {
        [REAI_ANALYSIS_STATUS_QUEUED]     = "Queued",
        [REAI_ANALYSIS_STATUS_PROCESSING] = "Processing",
        [REAI_ANALYSIS_STATUS_COMPLETE]   = "Complete",
        [REAI_ANALYSIS_STATUS_ERROR]      = "Error",
        [REAI_ANALYSIS_STATUS_ALL]        = "All"
    };
    return status_strings[status];
}

/**
 * @b Convert given @c CString to @c ReaiAnalysisStatus.
 *
 * @param str
 * @return @c ReaiAnalysisStatus value corresponding to the string.
 * @return @c REAI_ANALYSIS_STATUS_INVALID if the string is invalid.
 * */
ReaiAnalysisStatus reai_analysis_status_from_cstr (CString str) {
    RETURN_VALUE_IF (!str, REAI_ANALYSIS_STATUS_INVALID, ERR_INVALID_ARGUMENTS);

    static const struct {
        CString            str;
        ReaiAnalysisStatus status;
    } status_map[] = {
        {    "Queued",     REAI_ANALYSIS_STATUS_QUEUED},
        {"Processing", REAI_ANALYSIS_STATUS_PROCESSING},
        {  "Complete",   REAI_ANALYSIS_STATUS_COMPLETE},
        {     "Error",      REAI_ANALYSIS_STATUS_ERROR},
        {       "All",        REAI_ANALYSIS_STATUS_ALL}
    };

    for (Size i = 0; i < ARRAY_SIZE (status_map); ++i) {
        if (!strcmp (str, status_map[i].str)) {
            return status_map[i].status;
        }
    }

    return REAI_ANALYSIS_STATUS_INVALID;
}
