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
 * @return @c NULL otherwise.
 * */
PUBLIC ReaiAnalysisInfo* reai_analysis_info_clone_deinit (ReaiAnalysisInfo* clone) {
    RETURN_VALUE_IF (!clone, NULL, ERR_INVALID_ARGUMENTS);

    DESTROY_CSTR_CLONE (clone->creation);
    DESTROY_CSTR_CLONE (clone->binary_name);
    DESTROY_CSTR_CLONE (clone->sha_256_hash);
    DESTROY_CSTR_CLONE (clone->username);

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
 * @return @c NULL otherwise.
 * */
PUBLIC ReaiAnalysisInfo*
    reai_analysis_info_clone_init (ReaiAnalysisInfo* dst, ReaiAnalysisInfo* src) {
    RETURN_VALUE_IF (!dst || !src, NULL, ERR_INVALID_ARGUMENTS);

    CREATE_CSTR_CLONE (dst->creation, src->creation);
    CREATE_CSTR_CLONE (dst->binary_name, src->binary_name);
    CREATE_CSTR_CLONE (dst->sha_256_hash, src->sha_256_hash);
    CREATE_CSTR_CLONE (dst->username, src->username);

    dst->binary_id                 = src->binary_id;
    dst->analysis_id               = src->analysis_id;
    dst->is_public                 = src->is_public;
    dst->model_id                  = src->model_id;
    dst->status                    = src->status;
    dst->is_owner                  = src->is_owner;
    dst->binary_size               = src->binary_size;
    dst->dynamic_execution_status  = src->dynamic_execution_status;
    dst->dynamic_execution_task_id = src->dynamic_execution_task_id;

    return dst;

CLONE_FAILED:
    reai_analysis_info_clone_deinit (dst);
    return NULL;
}

/**
 * @b Convert given @c ReaiAnalysisStatus to @c CString.
 *
 * @param status
 * @return static @c CString value. Must not be freed by caller.
 * @return @c NULL if status is invalid
 * */
CString reai_analysis_status_to_cstr (ReaiAnalysisStatus status) {
    RETURN_VALUE_IF (!status || status >= REAI_ANALYSIS_STATUS_MAX, NULL, ERR_INVALID_ARGUMENTS);

    static const CString status_strings[] = {
        [REAI_ANALYSIS_STATUS_QUEUED]     = "Queued",
        [REAI_ANALYSIS_STATUS_PROCESSING] = "Processing",
        [REAI_ANALYSIS_STATUS_COMPLETE]   = "Complete",
        [REAI_ANALYSIS_STATUS_UPLOADED]   = "Uploaded",
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
        {  "Uploaded",   REAI_ANALYSIS_STATUS_UPLOADED},
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

CString reai_dyn_exec_status_to_cstr (ReaiDynExecStatus status) {
    RETURN_VALUE_IF (!status, NULL, ERR_INVALID_ARGUMENTS);

    switch (status) {
        case REAI_DYN_EXEC_STATUS_PENDING :
            return "PENDING";
        case REAI_DYN_EXEC_STATUS_ERROR :
            return "ERROR";
        case REAI_DYN_EXEC_STATUS_SUCCESS :
            return "SUCCESS";
        case REAI_DYN_EXEC_STATUS_ALL :
            return "ALL";
        default :
            return NULL;
    }
}

ReaiDynExecStatus reai_dyn_exec_status_from_cstr (CString status) {
    RETURN_VALUE_IF (!status, REAI_DYN_EXEC_STATUS_ERROR, ERR_INVALID_ARGUMENTS);

    if (strcmp (status, "PENDING") == 0)
        return REAI_DYN_EXEC_STATUS_PENDING;
    if (strcmp (status, "ERROR") == 0)
        return REAI_DYN_EXEC_STATUS_ERROR;
    if (strcmp (status, "SUCCESS") == 0)
        return REAI_DYN_EXEC_STATUS_SUCCESS;
    if (strcmp (status, "ALL") == 0)
        return REAI_DYN_EXEC_STATUS_ALL;

    return REAI_DYN_EXEC_STATUS_ERROR;
}

#undef DESTROY_CSTR_CLONE
#undef MAKE_CSTR_CLONE
