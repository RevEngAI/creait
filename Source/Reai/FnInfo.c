/**
 * @file FnInfo.h
 * @date 20 August 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#include <Reai/FnInfo.h>

/* libc */
#include <string.h>

/**
 * @b Method to clone function info items.
 *
 * @param dst Memory pointer where cloned data must be placed
 * @param src Memory pointer where source data is stored.
 *
 * @return @c dst on success.
 * @return @c NULL otherwise.
 * */
ReaiFnInfo* reai_fn_info_clone_init (ReaiFnInfo* dst, ReaiFnInfo* src) {
    RETURN_VALUE_IF (!dst || !src, (ReaiFnInfo*)NULL, ERR_INVALID_ARGUMENTS);

    dst->name = strdup (src->name);
    RETURN_VALUE_IF (!dst->name, (ReaiFnInfo*)NULL, ERR_OUT_OF_MEMORY);
    dst->id    = src->id;
    dst->vaddr = src->vaddr;
    dst->size  = src->size;

    return dst;
}

/**
 * @b Method to destroy cloned items.
 *
 * @param clone.
 *
 * @return @c clone on success.
 * @return @c NULL otherwise.
 * */
ReaiFnInfo* reai_fn_info_clone_deinit (ReaiFnInfo* clone) {
    RETURN_VALUE_IF (!clone, (ReaiFnInfo*)NULL, ERR_INVALID_ARGUMENTS);

    if (clone->name) {
        memset ((Char*)clone->name, 0, strlen (clone->name));
        FREE (clone->name);
    }

    memset (clone, 0, sizeof (*clone));
    return clone;
}
