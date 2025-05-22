/**
 * @file AnnFnMatch.c
 * @date 20 August 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#include <Reai/Api/Types/AnnFnMatch.h>
#include <Reai/Log.h>

/* libc */
#include <string.h>

void AnnFnMatchDeinit (AnnFnMatch* afn) {
    if (!afn) {
        LOG_FATAL ("Invalid arguments. Aborting...");
    }

    StrDeinit (&afn->binary_name);
    StrDeinit (&afn->function_name);
    StrDeinit (&afn->sha256);

    memset (afn, 0, sizeof (AnnFnMatch));
}

AnnFnMatch* ann_fn_match_clone_init (AnnFnMatch* dst, AnnFnMatch* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid arguments. Aborting...");
    }

    StrInitCopy (&dst->binary_name, &src->binary_name);
    StrInitCopy (&dst->function_name, &src->function_name);
    StrInitCopy (&dst->sha256, &src->sha256);

    dst->confidence         = src->confidence;
    dst->binary_id          = src->binary_id;
    dst->debug              = src->debug;
    dst->function_id        = src->function_id;
    dst->origin_function_id = src->origin_function_id;

    return dst;
}
