/**
 * @file ApiError.c
 * @date 5th Dec 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#include <Reai/Api/Types/Error.h>
#include <Reai/Log.h>

bool ErorrInitClone (Error* dst, Error* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid objects provided. Cannot init clone. Aborting...");
    }

    StrInitCopy (&dst->code, &src->code);
    StrInitCopy (&dst->message, &src->message);

    return true;
}

void ErrorDeinit (Error* e) {
    if (!e) {
        LOG_FATAL ("Invalid arguments to clone method. Aborting...");
    }

    StrDeinit (&e->code);
    StrDeinit (&e->message);
}
