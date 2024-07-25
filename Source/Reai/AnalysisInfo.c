/**
 * @file AnalysisInfo.c
 * @date 23rd July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

/* reai */
#include <Reai/AnalysisInfo.h>

/* libc */
#include <string.h>

#define CREATE_CSTR_CLONE(d, s)                                                                    \
    GOTO_HANDLER_IF (!(d = strdup (s)), CLONE_FAILED, ERR_OUT_OF_MEMORY);

#define DESTROY_CSTR_CLONE(c)                                                                      \
    if (c) {                                                                                       \
        memset ((Char*)c, 0, strlen (c));                                                          \
        FREE (c);                                                                                  \
    }

/**
 * @b Method to destroy cloned items.
 *
 * @param clone.
 *
 * @return @c clone on success.
 * @return @c Null otherwise.
 * */
PUBLIC ReaiAnalysisInfo* reai_analysis_info_clone_deinit (ReaiAnalysisInfo* clone) {
    RETURN_VALUE_IF (!clone, Null, ERR_INVALID_ARGUMENTS);

    DESTROY_CSTR_CLONE (clone->binary_name);
    DESTROY_CSTR_CLONE (clone->model_name);
    DESTROY_CSTR_CLONE (clone->creation);
    DESTROY_CSTR_CLONE (clone->sha_256_hash);

    memset (clone, 0, sizeof (*clone));
    return clone;
}

/**
 * @b Method to clone function info items.
 *
 * @param dst Memory pointer where cloned data must be placed
 * @param src Memory pointer where source data is stored.
 *
 * @return @c dst on success.
 * @return @c Null otherwise.
 * */
PUBLIC ReaiAnalysisInfo*
    reai_analysis_info_clone_init (ReaiAnalysisInfo* dst, ReaiAnalysisInfo* src) {
    RETURN_VALUE_IF (!dst || !src, Null, ERR_INVALID_ARGUMENTS);

    CREATE_CSTR_CLONE (dst->binary_name, src->binary_name);
    CREATE_CSTR_CLONE (dst->creation, src->creation);
    CREATE_CSTR_CLONE (dst->model_name, src->model_name);
    CREATE_CSTR_CLONE (dst->sha_256_hash, src->sha_256_hash);

    dst->binary_id = src->binary_id;
    dst->model_id  = src->model_id;
    dst->status    = src->status;

    return dst;

CLONE_FAILED:
    reai_analysis_info_clone_deinit (dst);
    return Null;
}

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

#undef DESTROY_CSTR_CLONE
#undef MAKE_CSTR_CLONE
