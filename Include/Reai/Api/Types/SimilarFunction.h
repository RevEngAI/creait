/**
 * @file SimilarFunction.h
 * @date 31st March 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */


#ifndef REAI_SIMILAR_FN_H
#define REAI_SIMILAR_FN_H

#include <Reai/Api/Types/Common.h>
#include <Reai/Types.h>
#include <Reai/Util/Str.h>

typedef struct SimilarFunction {
    FunctionId id;
    Str        name;
    BinaryId   binary_id;
    Str        binary_name;
    f64        distance;
    Str        sha256;
    Vec (f64) projection;
} SimilarFunction;

typedef Vec (SimilarFunction) SimilarFunctions;

#ifdef __cplusplus
extern "C" {
#endif

    ///
    /// Deinit cloned SimilarFunction object. Provided pointer is not freed.
    /// That must be taken care of by the owner.
    ///
    /// fi[in,out] : Object to be destroyed.
    ///
    REAI_API void SimilarFunctionDeinit (SimilarFunction* clone);

    ///
    /// Clone a SimilarFunction object from `src` to `dst`
    ///
    /// dst[out] : Destination object.
    /// src[in]  : Source object.
    ///
    /// SUCCESS : True
    /// FAILURE : Does not return
    ///
    REAI_API bool SimilarFunctionInitClone (SimilarFunction* dst, SimilarFunction* src);

#ifdef __cplusplus
}
#endif

#endif // REAI_SIMILAR_FN_H
