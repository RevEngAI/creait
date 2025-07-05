/**
 * @file FnInfo.h
 * @date 20 August 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#include <Reai/Api/Types/FunctionInfo.h>
#include <Reai/Log.h>

/* libc */
#include <string.h>

void FunctionInfoDeinit (FunctionInfo* fi) {
    if (!fi) {
        LOG_FATAL ("Invalid object provided. Cannot deinit. Aborting...");
    }

    SymbolInfoDeinit (&fi->symbol);

    memset (fi, 0, sizeof (FunctionInfo));
}

bool FunctionInfoInitClone (FunctionInfo* dst, FunctionInfo* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid objects provided. Cannot init clone. Aborting...");
    }

    dst->id   = src->id;
    dst->size = src->size;
    SymbolInfoInitClone (&dst->symbol, &src->symbol);
    dst->debug = src->debug;

    return true;
}
