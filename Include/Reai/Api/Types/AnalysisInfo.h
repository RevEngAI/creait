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

    typedef enum ReaiDynExecStatus {
        REAI_DYN_EXEC_STATUS_PENDING,
        REAI_DYN_EXEC_STATUS_ERROR,
        REAI_DYN_EXEC_STATUS_SUCCESS,
        REAI_DYN_EXEC_STATUS_ALL,
        REAI_DYN_EXEC_STATUS_MAX
    } ReaiDynExecStatus;

    CString           reai_dyn_exec_status_to_cstr (ReaiDynExecStatus status);
    ReaiDynExecStatus reai_dyn_exec_status_from_cstr (CString status);

    typedef enum ReaiAnalysisStatus {
        REAI_ANALYSIS_STATUS_INVALID,
        REAI_ANALYSIS_STATUS_QUEUED,
        REAI_ANALYSIS_STATUS_PROCESSING,
        REAI_ANALYSIS_STATUS_COMPLETE,
        REAI_ANALYSIS_STATUS_UPLOADED,
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
        ReaiBinaryId       binary_id;
        ReaiAnalysisId     analysis_id;
        Bool               is_public;
        Uint64             model_id;
        ReaiAnalysisStatus status;
        CString            creation;
        Bool               is_owner;
        CString            binary_name;
        CString            sha_256_hash;
        Size               binary_size;
        CString            username;
        ReaiDynExecStatus  dynamic_execution_status;
        Uint64             dynamic_execution_task_id;
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
