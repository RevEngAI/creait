/**
 * @file SymbolInfo.h
 * @date 23rd April 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef AI_SYMBOL_INFO_H
#define AI_SYMBOL_INFO_H

#include <Reai/Api/Types/Common.h>
#include <Reai/Types.h>
#include <Reai/Util/Str.h>

/**
 * \b Function map provided in AI decompilation results.
 * */
typedef struct {
    union {
        Str name;
        Str string;
    };
    bool is_addr;
    bool is_external;
    union {
        Str str;
        u64 addr;
    } value;
} SymbolInfo;

typedef Vec (SymbolInfo) SymbolInfos;

#ifdef __cplusplus
extern "C" {
#endif

    ///
    /// Deinitalize given symbol addr object.
    ///
    /// sa[in,out] : Object to be deinitalized.
    ///
    /// RETURN : Does not return on failure.
    ///
    REAI_API void SymbolInfoDeinit (SymbolInfo* sa);

    ///
    /// Init clone of given `src` object into `dst` object
    ///
    /// dst[out] : Object into which clone is to be initialized
    /// src[in]  : Object to initalize clone for.
    ///
    /// SUCCESS : Return true on success
    /// FAILURE : Does not return on failure
    ///
    REAI_API bool SymbolInfoInitClone (SymbolInfo* dst, SymbolInfo* src);

#ifdef __cplusplus
}
#endif

#endif // AI_SYMBOL_INFO_H
