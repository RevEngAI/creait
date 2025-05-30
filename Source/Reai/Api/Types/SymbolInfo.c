/**
 * @file Symboladdr.c
 * @date 23rd April 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

/* reai */
#include <Reai/Api/Types/SymbolInfo.h>
#include <Reai/Log.h>
#include <Reai/Util/Str.h>

void SymbolInfoDeinit (SymbolInfo* sa) {
    if (!sa) {
        LOG_FATAL ("Invalid SymbolInfo object provided. Aborting...");
    }

    StrDeinit (&sa->name);
    if (!sa->is_addr) {
        StrDeinit (&sa->value.str);
    }

    memset (sa, 0, sizeof (*sa));
}

bool SymbolInfoInitClone (SymbolInfo* dst, SymbolInfo* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid SymbolInfo object provided. Aborting...");
    }

    StrInitCopy (&dst->name, &src->name);
    dst->is_addr     = src->is_addr;
    dst->is_external = src->is_external;
    if (!src->is_addr) {
        StrInitCopy (&dst->value.str, &src->value.str);
    }

    return true;
}
