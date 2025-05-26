/**
 * @file BinaryInfo.h
 * @date 1st April 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef BINARY_SEARCH_RESULT_H
#define BINARY_SEARCH_RESULT_H

#include <Reai/Api/Types/AnalysisInfo.h>
#include <Reai/Api/Types/Common.h>
#include <Reai/Types.h>
#include <Reai/Util/Str.h>

/// can be used as response for query result as well

/**
 * @b Contains parsed JSON response for GET `/v2/binaries` endpoint
 * */
typedef struct BinaryInfo {
    BinaryId   binary_id;
    Str        binary_name;
    AnalysisId analysis_id;
    Str        sha256;
    Tags       tags;
    Str        created_at;
    ModelId    model_id;
    Str        model_name;
    Str        owned_by;

    /// XXX:
    Collections collections;
    Status      status;
} BinaryInfo;

typedef Vec (BinaryInfo) BinaryInfos;

#ifdef __cplusplus
extern "C" {
#endif

    ///
    /// Deinit BinaryInfo object clone. This won't free provided pointer.
    /// That must be done by owner.
    ///
    /// bsr[in] : BinaryInfo object.
    ///
    /// RETURN : Does not return on failure
    ///
    void BinaryInfoDeinit (BinaryInfo* bsr);

    ///
    /// Create clone of given `src` object into `dst` object
    ///
    /// dst[out] : Destination BinaryInfo object.
    /// src[in]  : Source BinaryInfo object.
    ///
    /// SUCCESS : true
    /// FAILURE : Does not return on failure
    ///
    bool BinaryInfoInitClone (BinaryInfo* dst, BinaryInfo* src);

#ifdef __cplusplus
}
#endif

#endif // BINARY_SEARCH_RESULT_H
