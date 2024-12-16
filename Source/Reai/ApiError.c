/**
 * @file ApiError.c
 * @date 5th Dec 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#include <Reai/ApiError.h>

/**
 * @b Create clone of given Api error object.
 *
 * @param dst
 * @param src
 *
 * @return dst on success.
 * @return NULL otherwise.
 * */
ReaiApiError* reai_api_error_clone_init (ReaiApiError* dst, ReaiApiError* src) {
    if (!dst || !src) {
        REAI_LOG_ERROR (ERR_INVALID_ARGUMENTS);
        return NULL;
    }

    reai_api_error_clone_deinit (dst);

    dst->code    = src->code ? strdup (src->code) : NULL;
    dst->message = src->message ? strdup (src->message) : NULL;

    return dst;
}

/**
 * @b Destroy clone of given API object.
 *
 * @param clone 
 *
 * @return deinited clone object on success. 
 * @return NULL otherwise.
 * */
ReaiApiError* reai_api_error_clone_deinit (ReaiApiError* clone) {
    if (!clone) {
        REAI_LOG_ERROR (ERR_INVALID_ARGUMENTS);
        return NULL;
    }

    if (clone->code) {
        FREE (clone->code);
        clone->code = NULL;
    }

    if (clone->message) {
        FREE (clone->message);
        clone->message = NULL;
    }

    return clone;
}
