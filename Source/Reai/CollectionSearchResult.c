/**
 * @file CollectionSearchResult.c
 * @date 1st April 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

/* reai */
#include <Reai/CollectionSearchResult.h>

/* libc */
#include <string.h>

#define CREATE_CSTR_CLONE(d, s)                                                                    \
    GOTO_HANDLER_IF (!(s) || !(d = strdup (s)), CLONE_FAILED, ERR_OUT_OF_MEMORY);

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
PUBLIC ReaiCollectionSearchResult* reai_collection_search_result_clone_deinit (
    ReaiCollectionSearchResult* clone
) {
    RETURN_VALUE_IF (!clone, NULL, ERR_INVALID_ARGUMENTS);

    DESTROY_CSTR_CLONE (clone->collection_name);
    DESTROY_CSTR_CLONE (clone->scope);
    DESTROY_CSTR_CLONE (clone->owned_by);
    DESTROY_CSTR_CLONE (clone->last_updated_at);
    DESTROY_CSTR_CLONE (clone->model_name);

    if (clone->tags) {
        reai_cstr_vec_destroy (clone->tags);
    }

    memset (clone, 0, sizeof (*clone));
    return clone;
}

/**
 * @b Method to clone collection info items.
 *
 * @param dst Memory pointer where cloned data must be placed
 * @param src Memory pointer where source data is stored.
 *
 * @return @c dst on success.
 * @return @c NULL otherwise.
 * */
PUBLIC ReaiCollectionSearchResult* reai_collection_search_result_clone_init (
    ReaiCollectionSearchResult* dst,
    ReaiCollectionSearchResult* src
) {
    RETURN_VALUE_IF (!dst || !src, NULL, ERR_INVALID_ARGUMENTS);

    dst->collection_id = src->collection_id;
    CREATE_CSTR_CLONE (dst->collection_name, src->collection_name);
    CREATE_CSTR_CLONE (dst->scope, src->scope);
    CREATE_CSTR_CLONE (dst->owned_by, src->owned_by);
    CREATE_CSTR_CLONE (dst->last_updated_at, src->last_updated_at);
    dst->model_id = src->model_id;
    CREATE_CSTR_CLONE (dst->model_name, src->model_name);
    dst->tags = src->tags ? reai_cstr_vec_clone_create (src->tags) : NULL;

    return dst;

CLONE_FAILED:
    reai_collection_search_result_clone_deinit (dst);
    return NULL;
}

#undef DESTROY_CSTR_CLONE
#undef MAKE_CSTR_CLONE
