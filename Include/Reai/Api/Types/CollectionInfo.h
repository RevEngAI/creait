/**
 * @file CollectionInfo.h
 * @date 1st April 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_COLLECTION_BASIC_INFO_H
#define REAI_COLLECTION_BASIC_INFO_H

#include <Reai/Api/Types/Common.h>
#include <Reai/Types.h>
#include <Reai/Util/Str.h>

typedef struct CollectionInfo {
    CollectionId id;
    Str          name;
    bool         is_private;
    Str          description;
    Str          owned_by;
    bool         is_official;
    Tags         tags;
    u64          size;
    Str          created_at;
    Str          last_updated_at;
    TeamId       team_id;
    Str          model_name;
    ModelId      model_id;
} CollectionInfo;

typedef Vec (CollectionInfo) CollectionInfos;

#ifdef __cplusplus
extern "C" {
#endif

    ///
    /// Deinit cloned CollectionInfo object. Provided pointer is not freed.
    /// That must be taken care of by the owner.
    ///
    /// fi[in,out] : Object to be destroyed.
    ///
    void CollectionInfoDeinit (CollectionInfo* clone);

    ///
    /// Clone a CollectionInfo object from `src` to `dst`
    ///
    /// dst[out] : Destination object.
    /// src[in]  : Source object.
    ///
    /// SUCCESS : True
    /// FAILURE : Does not return
    ///
    bool CollectionInfoInitClone (CollectionInfo* dst, CollectionInfo* src);

#ifdef __cplusplus
}
#endif

#endif // REAI_COLLECTION_BASIC_INFO_H
