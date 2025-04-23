/**
 * @file AiDecompFnMap.c
 * @date 23rd April 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

/* reai */
#include <Reai/Api/Types/AiDecompFnMap.h>

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
PUBLIC ReaiAiDecompFnMap* reai_ai_decomp_fn_map_clone_deinit (ReaiAiDecompFnMap* clone) {
    RETURN_VALUE_IF (!clone, NULL, ERR_INVALID_ARGUMENTS);

    DESTROY_CSTR_CLONE (clone->name);

    memset (clone, 0, sizeof (*clone));
    return clone;
}

/**
 * @b Method to clone function mapping entries from poll-ai-decompilation endpoint.
 *
 * @param dst Memory pointer where cloned data must be placed
 * @param src Memory pointer where source data is stored.
 *
 * @return @c dst on success.
 * @return @c NULL otherwise.
 * */
PUBLIC ReaiAiDecompFnMap*
    reai_ai_decomp_fn_map_clone_init (ReaiAiDecompFnMap* dst, ReaiAiDecompFnMap* src) {
    RETURN_VALUE_IF (!dst || !src, NULL, ERR_INVALID_ARGUMENTS);

    CREATE_CSTR_CLONE (dst->name, src->name);
    dst->addr = src->addr;

    return dst;

CLONE_FAILED:
    reai_ai_decomp_fn_map_clone_deinit (dst);
    return NULL;
}

#undef DESTROY_CSTR_CLONE
#undef MAKE_CSTR_CLONE
