/**
 * @file AiDecompilation.h
 * @date 18th May 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef AI_DECOMPILATION_H
#define AI_DECOMPILATION_H

#include <Reai/Api/Types/Common.h>
#include <Reai/Api/Types/Status.h>
#include <Reai/Api/Types/SymbolInfo.h>
#include <Reai/Types.h>

typedef struct AiDecompilation {
    Str         decompilation;
    Str         raw_decompilation;
    Str         ai_summary;     // optional (if requested)
    Str         raw_ai_summary; // optional (if requested)
    SymbolInfos strings;
    SymbolInfos functions;
    struct {
        SymbolInfos strings;
        SymbolInfos functions;
        SymbolInfos vars;
        SymbolInfos external_vars;
        SymbolInfos custom_types;
        SymbolInfos go_to_labels;
        SymbolInfos custom_function_pointers;
        SymbolInfos variadic_lists;
    } unmatched;
    // TODO: fields??
} AiDecompilation;

#ifdef __cplusplus
extern "C" {
#endif

    REAI_API void AiDecompilationDeinit (AiDecompilation* clone);

    ///
    /// Init clone of AiDecompilation object
    /// Vectors in inited in clone will create new copies of data instead of sharing ownersip
    ///
    /// dst[out] : Destination of cloned data
    /// src[in]  : Cloning source
    ///
    /// SUCCESS : true
    /// FAILURE : Does not return
    ///
    REAI_API bool AiDecompilationInitClone (AiDecompilation* dst, AiDecompilation* src);

#ifdef __cplusplus
}
#endif

#endif // AI_DECOMPILATION_H
