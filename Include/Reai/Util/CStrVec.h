/**
 * @file CStrVec.h
 * @date 24th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 *
 *
 * @brief Provides container to store CString values.
 * */

#ifndef REAI_UTIL_CSTRVEC_H
#define REAI_UTIL_CSTRVEC_H

#include <Reai/Util/Vec.h>

#include "Reai/Common.h"

/**
 * Create another CString* from given one.
 *
 * @return @c dst on success.
 * @return @c Null otherwise.
 * */
PRIVATE CString *cstr_clone_init (CString *dst, CString *src) {
    RETURN_VALUE_IF (!dst || !src || !*src, Null, ERR_INVALID_ARGUMENTS);

    CString clone = strdup (*src);
    RETURN_VALUE_IF (!clone, Null, ERR_INVALID_ARGUMENTS);

    *dst = clone;

    return dst;
}

/**
 * De-initialize pointer to CString.
 *
 * @return @c clone on success.
 * @return @c Null otherwise.
 * */
PRIVATE CString *cstr_clone_deinit (CString *clone) {
    RETURN_VALUE_IF (!clone || !*clone, Null, ERR_INVALID_ARGUMENTS);

    FREE (*clone);
    *clone = Null;

    return clone;
}

REAI_MAKE_VEC (CStrVec, cstr, CString, cstr_clone_init, cstr_clone_deinit);

#endif // REAI_UTIL_CSTRVEC_H
