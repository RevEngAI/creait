/**
 * @file CollectionInfo.c
 * @date 1st April 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

/* reai */
#include <Reai/Api/Types/CollectionInfo.h>
#include <Reai/Log.h>

/* libc */
#include <string.h>

void CollectionInfoDeinit (CollectionInfo* ci) {
    if (!ci) {
        LOG_FATAL ("Invalid object provided. Cannot deinit. Aborting...");
    }

    StrDeinit (&ci->name);
    StrDeinit (&ci->description);
    StrDeinit (&ci->owned_by);
    StrDeinit (&ci->created_at);
    StrDeinit (&ci->model_name);
    VecDeinit (&ci->tags);

    memset (ci, 0, sizeof (*ci));
}

bool CollectionInfoInitClone (CollectionInfo* dst, CollectionInfo* src) {
    if (!src || !dst) {
        LOG_FATAL ("Invalid objects provided. Cannot init clone. Aborting...");
    }

    StrInitCopy (&dst->name, &src->name);
    StrInitCopy (&dst->description, &src->description);
    StrInitCopy (&dst->owned_by, &src->owned_by);
    dst->is_official = src->is_official;
    VecInitClone (&dst->tags, &src->tags);
    dst->size = src->size;
    dst->id   = src->id;
    StrInitCopy (&dst->created_at, &src->created_at);
    dst->team_id = src->team_id;
    StrInitCopy (&dst->model_name, &src->model_name);

    return true;
}
