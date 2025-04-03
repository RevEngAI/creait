/**
 * @file CollectionInfo.h
 * @date 1st April 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_COLLECTION_SEARCH_RESULT_H
#define REAI_COLLECTION_SEARCH_RESULT_H

#include <Reai/Common.h>
#include <Reai/Types.h>
#include <Reai/Util/CStrVec.h>

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @b Contains parsed JSON response for GET `/v2/collections` endpoint
     * */
    typedef struct ReaiCollectionSearchResult {
        ReaiCollectionId collection_id;
        CString          collection_name;
        CString          scope;
        CString          last_updated_at;
        ReaiModelId      model_id;
        CString          model_name;
        CString          owned_by;
        CStrVec*         tags;
    } ReaiCollectionSearchResult;

    ReaiCollectionSearchResult* reai_collection_search_result_clone_deinit (
        ReaiCollectionSearchResult* clone
    );
    ReaiCollectionSearchResult* reai_collection_search_result_clone_init (
        ReaiCollectionSearchResult* dst,
        ReaiCollectionSearchResult* src
    );

    REAI_MAKE_VEC (
        ReaiCollectionSearchResultVec,
        collection_search_result,
        ReaiCollectionSearchResult,
        reai_collection_search_result_clone_init,
        reai_collection_search_result_clone_deinit
    );


#ifdef __cplusplus
}
#endif

#endif // REAI_COLLECTION_SEARCH_RESULT_H
