/**
 * @file AnnFnMatch.h
 * @date 2nd August 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */


#ifndef REAI_ANN_FN_MATCH_H
#define REAI_ANN_FN_MATCH_H

#include <Reai/Api/Types/Common.h>
#include <Reai/Types.h>
#include <Reai/Util/Str.h>
#include <Reai/Util/Vec.h>

typedef struct AnnFnMatch {
    f64        confidence;
    BinaryId   binary_id;
    Str        binary_name;
    bool       debug;
    FunctionId function_id;
    Str        function_name;
    Str        sha256;
    FunctionId origin_function_id;
} AnnFnMatch;

#ifdef __cplusplus
extern "C" {
#endif

    REAI_API void AnnFnMatchDeinit (AnnFnMatch* clone);
    REAI_API bool AnnFnMatchInitClone (AnnFnMatch* dst, AnnFnMatch* src);

#ifdef __cplusplus
}
#endif

#endif // REAI_ANN_FN_MATCH_H
