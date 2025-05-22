/**
 * @file BinaryInfo.c
 * @date 1st April 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

/* reai */
#include <Reai/Api/Types/BinaryInfo.h>
#include <Reai/Log.h>

/* libc */
#include <string.h>

void BinaryInfoDeinit (BinaryInfo* bsr) {
    if (!bsr) {
        LOG_FATAL ("Invalid object provided. Cannot deinit. Aborting...");
    }

    StrDeinit (&bsr->binary_name);
    StrDeinit (&bsr->sha256);
    StrDeinit (&bsr->owned_by);
    StrDeinit (&bsr->created_at);
    StrDeinit (&bsr->model_name);
    VecDeinit (&bsr->tags);

    memset (bsr, 0, sizeof (*bsr));
}

bool BinaryInfoInitClone (BinaryInfo* dst, BinaryInfo* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid objects provided for cloning. Cannot clone. Aborting...");
    }

    dst->binary_id = src->binary_id;
    StrInitCopy (&dst->binary_name, &src->binary_name);
    dst->analysis_id = src->analysis_id;
    StrInitCopy (&dst->sha256, &src->sha256);
    VecInitClone (&dst->tags, &src->tags);
    StrInitCopy (&dst->created_at, &src->created_at);
    dst->model_id = src->model_id;
    StrInitCopy (&dst->model_name, &src->model_name);
    StrInitCopy (&dst->owned_by, &src->owned_by);

    return true;
}
