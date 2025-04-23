/**
 * @file ApiError.h
 * @date 5th Dec 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_API_ERROR_H
#define REAI_API_ERROR_H

#include <Reai/Types.h>
#include <Reai/Util/Vec.h>

typedef struct {
    CString code;
    CString message;
} ReaiApiError;

ReaiApiError* reai_api_error_clone_init (ReaiApiError* dst, ReaiApiError* src);
ReaiApiError* reai_api_error_clone_deinit (ReaiApiError* clone);

REAI_MAKE_VEC (
    ReaiApiErrors,
    api_error,
    ReaiApiError,
    reai_api_error_clone_init,
    reai_api_error_vec_init
);

#endif // REAI_API_ERROR_H
