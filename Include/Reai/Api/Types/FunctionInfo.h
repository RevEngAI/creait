/**
 * @file FnInfo.h
 * @date 17th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_FN_INFO_H
#define REAI_FN_INFO_H

#include <Reai/Api/Types/Common.h>
#include <Reai/Api/Types/SymbolInfo.h>
#include <Reai/Types.h>
#include <Reai/Util/Str.h>

/* libc */
#include <memory.h>

typedef struct FunctionInfo {
    FunctionId id;
    u64        size;
    SymbolInfo symbol;
    bool       debug;
} FunctionInfo;

typedef Vec (FunctionInfo) FunctionInfos;

#ifdef __cplusplus
extern "C" {
#endif

    ///
    /// Clone a function info object from `src` to `dst`
    ///
    /// dst[out] : Destination object.
    /// src[in]  : Source object.
    ///
    /// SUCCESS : True
    /// FAILURE : Does not return
    ///
    REAI_API bool FunctionInfoInitClone (FunctionInfo* dst, FunctionInfo* src);

    ///
    /// Deinit cloned FunctionInfo object. Provided pointer is not freed.
    /// That must be taken care of by the owner.
    ///
    /// fi[in,out] : Object to be destroyed.
    ///
    REAI_API void FunctionInfoDeinit (FunctionInfo* fi);

#ifdef __cplusplus
}
#endif

#endif // REAI_FN_INFO_H
