/**
 * @file AnalysisInfo.h
 * @date 18th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_ANALYSIS_INFO_H
#define REAI_ANALYSIS_INFO_H

#include <Reai/Common.h>
#include <Reai/Types.h>
#include <Reai/Util/Vec.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum ReaiAnalysisStatus {
        REAI_ANALYSIS_STATUS_INVALID,
        REAI_ANALYSIS_STATUS_QUEUED,
        REAI_ANALYSIS_STATUS_PROCESSING,
        REAI_ANALYSIS_STATUS_COMPLETE,
        REAI_ANALYSIS_STATUS_ERROR,
        REAI_ANALYSIS_STATUS_ALL,
        REAI_ANALYSIS_STATUS_MAX
    } ReaiAnalysisStatus;

    CString            reai_analysis_status_to_cstr (ReaiAnalysisStatus status);
    ReaiAnalysisStatus reai_analysis_status_from_cstr (CString status);

    /**
     * @b Contains parsed JSON response for GET `/analyse/recent` endpoint,
     * representing analysis status for associated binary id.
     * */
    typedef struct ReaiAnalysisInfo {
        Uint64             binary_id;
        CString            binary_name;
        CString            creation;
        Uint64             model_id;
        CString            model_name;
        CString            sha_256_hash;
        ReaiAnalysisStatus status;
    } ReaiAnalysisInfo;

    ReaiAnalysisInfo* reai_analysis_info_clone_deinit (ReaiAnalysisInfo* clone);
    ReaiAnalysisInfo* reai_analysis_info_clone_init (ReaiAnalysisInfo* dst, ReaiAnalysisInfo* src);

    REAI_MAKE_VEC (
        ReaiAnalysisInfoVec,
        analysis_info,
        ReaiAnalysisInfo,
        reai_analysis_info_clone_init,
        reai_analysis_info_clone_deinit
    );


#ifdef __cplusplus
}
#endif

#endif // REAI_ANALYSIS_INFO_H
