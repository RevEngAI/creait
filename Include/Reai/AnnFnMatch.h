/**
 * @file AnnFnMatch.h
 * @date 2nd August 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */


#ifndef REAI_ANN_FN_MATCH_H
#define REAI_ANN_FN_MATCH_H

#include "Reai/Common.h"
#ifdef __cplusplus
extern "C" {
#endif

#include <Reai/Types.h>
#include <Reai/Util/Vec.h>

    typedef struct ReaiAnnFnMatch {
        Float64        confidence;
        ReaiBinaryId   nn_binary_id;
        CString        nn_binary_name;
        Bool           nn_debug;
        ReaiFunctionId nn_function_id;
        CString        nn_function_name;
        CString        nn_sha_256_hash;
        ReaiFunctionId origin_function_id;
    } ReaiAnnFnMatch;

    /**
     * @b Deinitialize a cloned ReaiAnnFnMatch object.
     *
     * @param clone
     *
     * @return @c clone on success.
     * @return @c Null otherwise.
     * */
    PRIVATE ReaiAnnFnMatch* reai_ann_fn_match_clone_deinit (ReaiAnnFnMatch* clone) {
        RETURN_VALUE_IF (!clone, Null, ERR_INVALID_ARGUMENTS);

        if (clone->nn_binary_name) {
            FREE (clone->nn_binary_name);
        }

        if (clone->nn_function_name) {
            FREE (clone->nn_function_name);
        }

        if (clone->nn_sha_256_hash) {
            FREE (clone->nn_sha_256_hash);
        }

        memset (clone, 0, sizeof (ReaiAnnFnMatch));
        return clone;
    }

    /**
     * @b Create clone of given ReaiAnnFnMatch object.
     *
     * @param dst
     * @param src
     *
     * @return @c dst on success.
     * @return @c src otherwise.
     * */
    PRIVATE ReaiAnnFnMatch*
        reai_ann_fn_match_clone_init (ReaiAnnFnMatch* dst, ReaiAnnFnMatch* src) {
        RETURN_VALUE_IF (!dst || !src, Null, ERR_INVALID_ARGUMENTS);

        dst->confidence         = src->confidence;
        dst->nn_binary_id       = src->nn_binary_id;
        dst->nn_binary_name     = src->nn_binary_name ? strdup (src->nn_binary_name) : Null;
        dst->nn_debug           = src->nn_debug;
        dst->nn_function_id     = src->nn_function_id;
        dst->nn_function_name   = src->nn_function_name ? strdup (src->nn_function_name) : Null;
        dst->nn_sha_256_hash    = src->nn_sha_256_hash ? strdup (src->nn_sha_256_hash) : Null;
        dst->origin_function_id = src->origin_function_id;

        if (!dst->nn_binary_name || !dst->nn_function_name || !dst->nn_sha_256_hash) {
            PRINT_ERR ("Out of memory or invalid function match object");
            reai_ann_fn_match_clone_deinit (dst);
            return Null;
        }

        return dst;
    }

    REAI_MAKE_VEC (
        ReaiAnnFnMatchVec,
        ann_fn_match,
        ReaiAnnFnMatch,
        reai_ann_fn_match_clone_init,
        reai_ann_fn_match_clone_deinit
    );

#ifdef __cplusplus
}
#endif

#endif // REAI_ANN_FN_MATCH_H
