/**
 * @file FnInfo.h
 * @date 20 August 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#include <Reai/Api/Types/AnnSymbol.h>
#include <Reai/Log.h>

/* libc */
#include <string.h>

void AnnSymbolDeinit (AnnSymbol* sym) {
    if (!sym) {
        LOG_FATAL ("Invalid object provided. Cannot deinit. Aborting...");
    }

    StrDeinit (&sym->analysis_name);
    StrDeinit (&sym->function_name);
    StrDeinit (&sym->function_mangled_name);
    StrDeinit (&sym->sha256);

    memset (sym, 0, sizeof (AnnSymbol));
}

bool AnnSymbolInitClone (AnnSymbol* dst, AnnSymbol* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid objects provided. Cannot init clone. Aborting...");
    }

    dst->source_function_id = src->source_function_id;
    dst->target_function_id = src->target_function_id;
    dst->distance           = src->distance;
    dst->analysis_id        = src->analysis_id;
    StrInitCopy (&dst->analysis_name, &src->analysis_name);
    StrInitCopy (&dst->function_name, &src->function_name);
    StrInitCopy (&dst->function_mangled_name, &src->function_mangled_name);
    dst->binary_id = src->binary_id;
    StrInitCopy (&dst->sha256, &src->sha256);
    dst->debug = src->debug;

    return true;
}
