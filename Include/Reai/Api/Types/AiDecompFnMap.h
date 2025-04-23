/**
 * @file AiDecompFnMap.h
 * @date 23rd April 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_AI_DECOMP_FN_MAP_H
#define REAI_AI_DECOMP_FN_MAP_H

#include <Reai/Common.h>
#include <Reai/Types.h>
#include <Reai/Util/CStrVec.h>

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * \b Function map provided in AI decompilation results.
     * */
    typedef struct {
        CString name;
        Uint64  addr;
    } ReaiAiDecompFnMap;

    ReaiAiDecompFnMap* reai_ai_decomp_fn_map_clone_deinit (ReaiAiDecompFnMap* clone);
    ReaiAiDecompFnMap*
        reai_ai_decomp_fn_map_clone_init (ReaiAiDecompFnMap* dst, ReaiAiDecompFnMap* src);

    REAI_MAKE_VEC (
        ReaiAiDecompFnMapVec,
        ai_decomp_fn_map,
        ReaiAiDecompFnMap,
        reai_ai_decomp_fn_map_clone_init,
        reai_ai_decomp_fn_map_clone_deinit
    );


#ifdef __cplusplus
}
#endif

#endif // REAI_AI_DECOMP_FN_MAP_H
