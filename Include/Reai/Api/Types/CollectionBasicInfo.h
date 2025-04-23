/**
 * @file CollectionInfo.h
 * @date 1st April 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_COLLECTION_BASIC_INFO_H
#define REAI_COLLECTION_BASIC_INFO_H

#include <Reai/Common.h>
#include <Reai/Types.h>
#include <Reai/Util/CStrVec.h>

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @b Contains parsed JSON response for GET `/v2/collections` endpoint
     * */
    typedef struct ReaiCollectionBasicInfo {
        CString          collection_name;
        CString          description;
        CString          collection_scope;
        CString          collection_owner;
        Bool             official_collection;
        CStrVec*         collection_tags;
        Uint64           collection_size;
        ReaiCollectionId collection_id;
        CString          creation;
        ReaiTeamId       team_id;
        CString          model_name;
    } ReaiCollectionBasicInfo;

    ReaiCollectionBasicInfo* reai_collection_basic_info_clone_deinit (ReaiCollectionBasicInfo* clone
    );
    ReaiCollectionBasicInfo* reai_collection_basic_info_clone_init (
        ReaiCollectionBasicInfo* dst,
        ReaiCollectionBasicInfo* src
    );

    REAI_MAKE_VEC (
        ReaiCollectionBasicInfoVec,
        collection_basic_info,
        ReaiCollectionBasicInfo,
        reai_collection_basic_info_clone_init,
        reai_collection_basic_info_clone_deinit
    );


#ifdef __cplusplus
}
#endif

#endif // REAI_COLLECTION_BASIC_INFO_H
