/**
 * @file BinaryInfo.h
 * @date 1st April 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_BINARY_SEARCH_RESULT_H
#define REAI_BINARY_SEARCH_RESULT_H

#include <Reai/Common.h>
#include <Reai/Types.h>
#include <Reai/Util/CStrVec.h>

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @b Contains parsed JSON response for GET `/v2/binaries` endpoint
     * */
    typedef struct ReaiBinarySearchResult {
        ReaiBinaryId   binary_id;
        CString        binary_name;
        ReaiAnalysisId analysis_id;
        CString        sha_256_hash;
        CStrVec*       tags;
        CString        created_at;
        ReaiModelId    model_id;
        CString        model_name;
        CString        owned_by;
    } ReaiBinarySearchResult;

    ReaiBinarySearchResult* reai_binary_search_result_clone_deinit (ReaiBinarySearchResult* clone);
    ReaiBinarySearchResult* reai_binary_search_result_clone_init (
        ReaiBinarySearchResult* dst,
        ReaiBinarySearchResult* src
    );

    REAI_MAKE_VEC (
        ReaiBinarySearchResultVec,
        binary_search_result,
        ReaiBinarySearchResult,
        reai_binary_search_result_clone_init,
        reai_binary_search_result_clone_deinit
    );


#ifdef __cplusplus
}
#endif

#endif // REAI_BINARY_SEARCH_RESULT_H
