/**
 * @file AnnFnMatch.h
 * @date 2nd August 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */


#ifndef REAI_ANN_FN_MATCH_H
#define REAI_ANN_FN_MATCH_H

#include "Reai/Common.h"
#ifdef __cplusplus
extern "C" {
#endif

#include <Reai/Types.h>
#include <Reai/Util/Vec.h>

    typedef struct ReaiAnnFnMatch {
        Float64        confidence;
        ReaiBinaryId   nn_binary_id;
        CString        nn_binary_name;
        Bool           nn_debug;
        ReaiFunctionId nn_function_id;
        CString        nn_function_name;
        CString        nn_sha_256_hash;
        ReaiFunctionId origin_function_id;
    } ReaiAnnFnMatch;

    ReaiAnnFnMatch* reai_ann_fn_match_clone_deinit (ReaiAnnFnMatch* clone);
    ReaiAnnFnMatch* reai_ann_fn_match_clone_init (ReaiAnnFnMatch* dst, ReaiAnnFnMatch* src);

    REAI_MAKE_VEC (
        ReaiAnnFnMatchVec,
        ann_fn_match,
        ReaiAnnFnMatch,
        reai_ann_fn_match_clone_init,
        reai_ann_fn_match_clone_deinit
    );

#ifdef __cplusplus
}
#endif

#endif // REAI_ANN_FN_MATCH_H
