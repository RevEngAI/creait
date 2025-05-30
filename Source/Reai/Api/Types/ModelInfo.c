/**
 * @file Symboladdr.c
 * @date 23rd April 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

/* reai */
#include <Reai/Api/Types/ModelInfo.h>
#include <Reai/Log.h>
#include <Reai/Util/Str.h>

/* libc */
#include <string.h>

void ModelInfoDeinit (ModelInfo* mi) {
    if (!mi) {
        LOG_FATAL ("Invalid ModelInfo object provided. Aborting...");
    }

    StrDeinit (&mi->name);

    memset (mi, 0, sizeof (*mi));
}

bool ModelInfoInitClone (ModelInfo* dst, ModelInfo* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid ModelInfo object provided. Aborting...");
    }

    StrInitCopy (&dst->name, &src->name);
    dst->id = src->id;

    return true;
}
