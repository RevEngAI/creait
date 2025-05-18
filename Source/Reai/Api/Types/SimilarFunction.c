/**
 * @file SimilarFunction.c
 * @date 31st March 2025 
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#include <Reai/Api/Types/SimilarFunction.h>
#include <Reai/Log.h>

/* libc */
#include <string.h>

void SimilarFunctionDeinit (SimilarFunction* sfn) {
    if (!sfn) {
        LOG_FATAL ("Invalid object provided. Cannot deinit. Aborting...");
    }

    StrDeinit (&sfn->binary_name);
    StrDeinit (&sfn->name);
    VecDeinit (&sfn->projection);

    memset (sfn, 0, sizeof (SimilarFunction));
}

bool SimilarFunctionInitClone (SimilarFunction* dst, SimilarFunction* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid objects provided. Cannot init clone. Aborting...");
    }

    dst->id = src->id;
    StrInitCopy (&dst->name, &src->name);
    dst->binary_id = src->binary_id;
    StrInitCopy (&dst->binary_name, &src->binary_name);
    dst->distance = src->distance;
    VecInitClone (&dst->projection, &src->projection);
    StrInitCopy (&dst->sha256, &src->sha256);

    return true;
}
