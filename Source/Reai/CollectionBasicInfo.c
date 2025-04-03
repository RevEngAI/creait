/**
 * @file CollectionBasicInfo.c
 * @date 1st April 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

/* reai */
#include <Reai/CollectionBasicInfo.h>

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
PUBLIC ReaiCollectionBasicInfo* reai_collection_basic_info_clone_deinit (
    ReaiCollectionBasicInfo* clone
) {
    RETURN_VALUE_IF (!clone, NULL, ERR_INVALID_ARGUMENTS);

    DESTROY_CSTR_CLONE (clone->collection_name);
    DESTROY_CSTR_CLONE (clone->description);
    DESTROY_CSTR_CLONE (clone->collection_scope);
    DESTROY_CSTR_CLONE (clone->collection_owner);
    DESTROY_CSTR_CLONE (clone->creation);
    DESTROY_CSTR_CLONE (clone->model_name);

    if (clone->collection_tags) {
        reai_cstr_vec_destroy (clone->collection_tags);
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
PUBLIC ReaiCollectionBasicInfo* reai_collection_basic_info_clone_init (
    ReaiCollectionBasicInfo* dst,
    ReaiCollectionBasicInfo* src
) {
    RETURN_VALUE_IF (!dst || !src, NULL, ERR_INVALID_ARGUMENTS);

    CREATE_CSTR_CLONE (dst->collection_name, src->collection_name);
    CREATE_CSTR_CLONE (dst->description, src->description);
    CREATE_CSTR_CLONE (dst->collection_scope, src->collection_scope);
    CREATE_CSTR_CLONE (dst->collection_owner, src->collection_owner);
    dst->official_collection = src->official_collection;
    dst->collection_tags =
        src->collection_tags ? reai_cstr_vec_clone_create (src->collection_tags) : NULL;
    dst->collection_size = src->collection_size;
    dst->collection_id   = src->collection_id;
    CREATE_CSTR_CLONE (dst->creation, src->creation);
    dst->team_id = src->team_id;
    CREATE_CSTR_CLONE (dst->model_name, src->model_name);

    return dst;

CLONE_FAILED:
    reai_collection_basic_info_clone_deinit (dst);
    return NULL;
}

#undef DESTROY_CSTR_CLONE
#undef MAKE_CSTR_CLONE
