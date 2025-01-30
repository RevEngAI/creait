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

#ifdef __cplusplus
extern "C" {
#endif

#include <Reai/Util/Vec.h>
#include <string.h>

    /**
     * Create another CString* from given one.
     *
     * @param dst Pointer to CString variable where clone must be stored.
     * @param src Pointer to CString variable to-be-cloned is stored.
     *
     * @return @c dst on success.
     * @return @c NULL otherwise.
     * */
    PRIVATE CString *cstr_clone_init (CString *dst, CString *src) {
        RETURN_VALUE_IF (!dst || !src || !*src, (CString *)NULL, ERR_INVALID_ARGUMENTS);

        CString clone = strdup (*src);
        RETURN_VALUE_IF (!clone, (CString *)NULL, ERR_INVALID_ARGUMENTS);

        *dst = clone;

        return dst;
    }

    /**
     * De-initialize pointer to CString.
     *
     * @param clone Pointer to cloned string to be destroyed.
     *
     * @return @c clone on success.
     * @return @c NULL otherwise.
     * */
    PRIVATE CString *cstr_clone_deinit (CString *clone) {
        RETURN_VALUE_IF (!clone, (CString *)NULL, ERR_INVALID_ARGUMENTS);

        if (*clone) {
            FREE (*clone);
        }

        return clone;
    }

    REAI_MAKE_VEC (CStrVec, cstr, CString, cstr_clone_init, cstr_clone_deinit);

#ifdef __cplusplus
}
#endif

#endif // REAI_UTIL_CSTRVEC_H
