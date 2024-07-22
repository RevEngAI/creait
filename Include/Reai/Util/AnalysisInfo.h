/**
 * @file AnalysisInfo.h
 * @date 18th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_UTIL_ANALYSIS_INFO_H
#define REAI_UTIL_ANALYSIS_INFO_H

#include <Reai/Common.h>
#include <Reai/Types.h>

/* libc */
#include <string.h>

C_SOURCE_BEGIN

typedef enum ReaiAnalysisStatus {
    REAI_ANALYSIS_STATUS_INVALID,
    REAI_ANALYSIS_STATUS_QUEUED,
    REAI_ANALYSIS_STATUS_PROCESSING,
    REAI_ANALYSIS_STATUS_COMPLETE,
    REAI_ANALYSIS_STATUS_ERROR,
    REAI_ANALYSIS_STATUS_ALL,
    REAI_ANALYSIS_STATUS_MAX
} ReaiAnalysisStatus;

/**
 * @b Contains parsed JSON response for GET `/analyse/recent` endpoint,
 * representing analysis status for associated binary id.
 * */
typedef struct ReaiAnalysisInfo {
    Uint64             binary_id;
    CString            binary_name;
    CString            creation;
    Uint64             model_id;
    CString            model_name;
    CString            sha_256_hash;
    ReaiAnalysisStatus status;
} ReaiAnalysisInfo;

/**
 * @b Method to destroy cloned items.
 *
 * @param clone.
 * 
 * @return @c clone on success.
 * @return @c Null otherwise.
 * */
PRIVATE ReaiAnalysisInfo* reai_analysis_info_clone_deinit (ReaiAnalysisInfo* clone) {
    RETURN_VALUE_IF (!clone, Null, ERR_INVALID_ARGUMENTS);

#define DESTROY_CSTR_CLONE(c)                                                                      \
    if (c) {                                                                                       \
        memset ((Char*)c, 0, strlen (c));                                                          \
        FREE (c);                                                                                  \
    }

    DESTROY_CSTR_CLONE (clone->binary_name);
    DESTROY_CSTR_CLONE (clone->model_name);
    DESTROY_CSTR_CLONE (clone->creation);
    DESTROY_CSTR_CLONE (clone->sha_256_hash);

#undef DESTROY_CSTR_CLONE

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
PRIVATE ReaiAnalysisInfo*
    reai_analysis_info_clone_init (ReaiAnalysisInfo* dst, ReaiAnalysisInfo* src) {
    RETURN_VALUE_IF (!dst || !src, Null, ERR_INVALID_ARGUMENTS);

#define CREATE_CSTR_CLONE(d, s)                                                                    \
    GOTO_HANDLER_IF (!(d = strdup (s)), CLONE_FAILED, ERR_OUT_OF_MEMORY);

    CREATE_CSTR_CLONE (dst->binary_name, src->binary_name);
    CREATE_CSTR_CLONE (dst->creation, src->creation);
    CREATE_CSTR_CLONE (dst->model_name, src->model_name);
    CREATE_CSTR_CLONE (dst->sha_256_hash, src->sha_256_hash);

#undef MAKE_CSTR_CLONE

    dst->binary_id = src->binary_id;
    dst->model_id  = src->model_id;
    dst->status    = src->status;

    return dst;

CLONE_FAILED:
    reai_analysis_info_clone_deinit (dst);
    return Null;
}

#include <Reai/Util/Vec.h>
REAI_MAKE_VEC (
    ReaiAnalysisInfoVec,
    analysis_info,
    ReaiAnalysisInfo,
    reai_analysis_info_clone_init,
    reai_analysis_info_clone_deinit
);


C_SOURCE_END

#endif // REAI_UTIL_ANALYSIS_INFO_H
