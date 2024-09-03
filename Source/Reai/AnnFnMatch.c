/**
 * @file AnnFnMatch.c
 * @date 20 August 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#include <Reai/AnnFnMatch.h>

/**
 * @b Deinitialize a cloned ReaiAnnFnMatch object.
 *
 * @param clone
 *
 * @return @c clone on success.
 * @return @c NULL otherwise.
 * */
ReaiAnnFnMatch* reai_ann_fn_match_clone_deinit (ReaiAnnFnMatch* clone) {
    RETURN_VALUE_IF (!clone, (ReaiAnnFnMatch*)NULL, ERR_INVALID_ARGUMENTS);

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
ReaiAnnFnMatch* reai_ann_fn_match_clone_init (ReaiAnnFnMatch* dst, ReaiAnnFnMatch* src) {
    RETURN_VALUE_IF (!dst || !src, (ReaiAnnFnMatch*)NULL, ERR_INVALID_ARGUMENTS);

    dst->confidence         = src->confidence;
    dst->nn_binary_id       = src->nn_binary_id;
    dst->nn_binary_name     = src->nn_binary_name ? strdup (src->nn_binary_name) : NULL;
    dst->nn_debug           = src->nn_debug;
    dst->nn_function_id     = src->nn_function_id;
    dst->nn_function_name   = src->nn_function_name ? strdup (src->nn_function_name) : NULL;
    dst->nn_sha_256_hash    = src->nn_sha_256_hash ? strdup (src->nn_sha_256_hash) : NULL;
    dst->origin_function_id = src->origin_function_id;

    if (!dst->nn_binary_name || !dst->nn_function_name || !dst->nn_sha_256_hash) {
        PRINT_ERR ("Out of memory or invalid function match object");
        reai_ann_fn_match_clone_deinit (dst);
        return (ReaiAnnFnMatch*)NULL;
    }

    return dst;
}
