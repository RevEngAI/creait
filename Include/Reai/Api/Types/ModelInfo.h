/**
 * @file ModelInfo.h
 * @date 23rd April 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef AI_MODEL_INFO_H
#define AI_MODEL_INFO_H

#include <Reai/Api/Types/Common.h>
#include <Reai/Types.h>
#include <Reai/Util/Str.h>

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * \b Function map provided in AI decompilation results.
     * */
    typedef struct {
        ModelId id;
        Str     name;
    } ModelInfo;
    typedef Vec (ModelInfo) ModelInfos;


    ///
    /// Deinitalize given model info object.
    ///
    /// sa[in,out] : Object to be deinitalized.
    ///
    /// RETURN : Does not return on failure.
    ///
    void ModelInfoDeinit (ModelInfo* sa);

    ///
    /// Init clone of given `src` object into `dst` object
    ///
    /// dst[out] : Object into which clone is to be initialized
    /// src[in]  : Object to initalize clone for.
    ///
    /// SUCCESS : Return true on success
    /// FAILURE : Does not return on failure
    ///
    bool ModelInfoInitClone (ModelInfo* dst, ModelInfo* src);

    //TODO : vec

#ifdef __cplusplus
}
#endif

#endif // AI_MODEL_INFO_H
