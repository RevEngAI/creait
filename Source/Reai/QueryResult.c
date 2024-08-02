/**
 * @file QueryResult.c
 * @date 24rd July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */


#include <Reai/QueryResult.h>

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
PUBLIC ReaiQueryResult* reai_query_result_clone_deinit (ReaiQueryResult* clone) {
    RETURN_VALUE_IF (!clone, Null, ERR_INVALID_ARGUMENTS);

    DESTROY_CSTR_CLONE (clone->binary_name);
    reai_cstr_vec_destroy (clone->collections);
    DESTROY_CSTR_CLONE (clone->creation);
    DESTROY_CSTR_CLONE (clone->model_name);
    DESTROY_CSTR_CLONE (clone->sha_256_hash);
    reai_cstr_vec_destroy (clone->tags);

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
PUBLIC ReaiQueryResult* reai_query_result_clone_init (ReaiQueryResult* dst, ReaiQueryResult* src) {
    RETURN_VALUE_IF (!dst || !src, Null, ERR_INVALID_ARGUMENTS);

    CREATE_CSTR_CLONE (dst->binary_name, src->binary_name);

    GOTO_HANDLER_IF (
        !(dst->collections = reai_cstr_vec_clone_create (src->collections)),
        CLONE_FAILED,
        "Failed to clone cstr vec."
    );

    CREATE_CSTR_CLONE (dst->creation, src->creation);
    CREATE_CSTR_CLONE (dst->model_name, src->model_name);
    CREATE_CSTR_CLONE (dst->sha_256_hash, src->sha_256_hash);

    GOTO_HANDLER_IF (
        !(dst->tags = reai_cstr_vec_clone_create (src->tags)),
        CLONE_FAILED,
        "Failed to clone cstr vec."
    );

    dst->binary_id = src->binary_id;
    dst->model_id  = src->model_id;
    dst->status    = src->status;

    return dst;

CLONE_FAILED:
    reai_query_result_clone_deinit (dst);
    return Null;
}
