/**
 * @file SimilarFn.h
 * @date 31st March 2025 
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */


#ifndef REAI_SIMILAR_FN_H
#define REAI_SIMILAR_FN_H

#include "Reai/Common.h"
#ifdef __cplusplus
extern "C" {
#endif

#include <Reai/Types.h>
#include <Reai/Util/IntVec.h>

    typedef struct ReaiSimilarFn {
        ReaiFunctionId function_id;
        CString        function_name;
        ReaiBinaryId   binary_id;
        CString        binary_name;
        Float64        distance;
        F64Vec*        projection;
        CString        sha_256_hash;
    } ReaiSimilarFn;

    ReaiSimilarFn* reai_similar_fn_clone_deinit (ReaiSimilarFn* clone);
    ReaiSimilarFn* reai_similar_fn_clone_init (ReaiSimilarFn* dst, ReaiSimilarFn* src);

    REAI_MAKE_VEC (
        ReaiSimilarFnVec,
        similar_fn,
        ReaiSimilarFn,
        reai_similar_fn_clone_init,
        reai_similar_fn_clone_deinit
    );

#ifdef __cplusplus
}
#endif

#endif // REAI_SIMILAR_FN_H
