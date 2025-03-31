/**
 * @file SimilarFn.c
 * @date 31st March 2025 
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#include <Reai/SimilarFn.h>

/* libc */
#include <string.h>

/**
 * @b Deinitialize a cloned ReaiSimilarFn object.
 *
 * @param clone
 *
 * @return @c clone on success.
 * @return @c NULL otherwise.
 * */
ReaiSimilarFn* reai_similar_fn_clone_deinit (ReaiSimilarFn* clone) {
    RETURN_VALUE_IF (!clone, (ReaiSimilarFn*)NULL, ERR_INVALID_ARGUMENTS);

    if (clone->binary_name) {
        FREE (clone->binary_name);
    }

    if (clone->function_name) {
        FREE (clone->function_name);
    }

    if (clone->sha_256_hash) {
        FREE (clone->function_name);
    }

    if (clone->projection) {
        reai_f64_vec_destroy (clone->projection);
        clone->projection = NULL;
    }

    memset (clone, 0, sizeof (ReaiSimilarFn));
    return clone;
}

/**
 * @b Create clone of given ReaiSimilarFn object.
 *
 * @param dst
 * @param src
 *
 * @return @c dst on success.
 * @return @c src otherwise.
 * */
ReaiSimilarFn* reai_similar_fn_clone_init (ReaiSimilarFn* dst, ReaiSimilarFn* src) {
    RETURN_VALUE_IF (!dst || !src, (ReaiSimilarFn*)NULL, ERR_INVALID_ARGUMENTS);

    dst->function_id   = src->function_id;
    dst->function_name = src->function_name ? strdup (src->function_name) : NULL;
    dst->binary_id     = src->binary_id;
    dst->binary_name   = src->binary_name ? strdup (src->binary_name) : NULL;
    dst->distance      = src->distance;
    dst->projection    = src->projection ? reai_f64_vec_clone_create (src->projection) : NULL;
    dst->sha_256_hash  = src->sha_256_hash ? strdup (src->sha_256_hash) : NULL;

    if (!dst->binary_name || !dst->function_name || !dst->sha_256_hash ||
        (!!dst->projection ^ !!src->projection)) {
        PRINT_ERR ("Out of memory or invalid function match object");
        reai_similar_fn_clone_deinit (dst);
        return (ReaiSimilarFn*)NULL;
    }

    return dst;
}
