/**
 * @file AnalysisInfo.h
 * @date 18th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef ANALYSIS_INFO_H
#define ANALYSIS_INFO_H

#include <Reai/Api/Types/Common.h>
#include <Reai/Api/Types/Status.h>
#include <Reai/Types.h>
#include <Reai/Util/Str.h>
#include <Reai/Util/Vec.h>

/**
 * @b Contains parsed JSON response for GET `/analyses/recent` endpoint,
 * representing analysis status for associated binary id.
 * */
typedef struct AnalysisInfo {
    BinaryId   binary_id;
    AnalysisId analysis_id;
    bool       is_private; /// named as scope in response json
    ModelId    model_id;
    Status     status;
    Str        creation;
    bool       is_owner;
    Str        binary_name;
    Str        sha256;
    size       binary_size;
    Str        username;
    Status     dyn_exec_status;
    u64        dyn_exec_task_id;
} AnalysisInfo;

typedef Vec (AnalysisInfo) AnalysisInfos;

#ifdef __cplusplus
extern "C" {
#endif

    void AnalysisInfoDeinit (AnalysisInfo* clone);
    bool AnalysisInfoInitClone (AnalysisInfo* dst, AnalysisInfo* src);

#ifdef __cplusplus
}
#endif

#endif // ANALYSIS_INFO_H
