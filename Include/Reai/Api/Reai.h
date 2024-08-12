/**
 * @file Reai.h
 * @date 9th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_API_REAI_H
#define REAI_API_REAI_H

#include <Reai/AnalysisInfo.h>
#include <Reai/AnnFnMatch.h>
#include <Reai/Api/Request.h>
#include <Reai/Common.h>
#include <Reai/Types.h>
#include <Reai/Util/CStrVec.h>


#ifdef __cplusplus
extern "C" {
#endif

    /* fwd declarations */
    typedef void                CURL;
    typedef struct Reai         Reai; /* opaque object */
    typedef struct ReaiResponse ReaiResponse;
    typedef struct ReaiRequest  ReaiRequest;
    typedef struct ReaiDb       ReaiDb;
    typedef struct ReaiLog      ReaiLog;

    Reai*         reai_create (CString host, CString api_key, CString model);
    void          reai_destroy (Reai* reai);
    ReaiResponse* reai_request (Reai* reai, ReaiRequest* req, ReaiResponse* response);
    Reai*         reai_set_db (Reai* reai, ReaiDb* db);
    Reai*         reai_set_logger (Reai* reai, ReaiLog* logger);
    Reai*         reai_update_all_analyses_status_in_db (Reai* reai);

    CString      reai_upload_file (Reai* reai, ReaiResponse* response, CString file_path);
    ReaiBinaryId reai_create_analysis (
        Reai*          reai,
        ReaiResponse*  response,
        ReaiModel      model,
        Uint64         base_addr,
        ReaiFnInfoVec* fn_info_vec,
        Bool           is_private,
        CString        sha_256_hash,
        CString        file_name,
        CString        cmdline_args,
        Size           size_in_bytes
    );
    ReaiAnalysisInfoVec* reai_get_recent_analyses (
        Reai*              reai,
        ReaiResponse*      response,
        ReaiAnalysisStatus status,
        ReaiBinaryScope    scope,
        Size               count
    );
    ReaiFnInfoVec*
         reai_get_basic_function_info (Reai* reai, ReaiResponse* response, ReaiBinaryId bin_id);
    Bool reai_batch_renames_functions (
        Reai*          reai,
        ReaiResponse*  response,
        ReaiFnInfoVec* new_name_mappings
    );
    Bool reai_rename_function (
        Reai*          reai,
        ReaiResponse*  response,
        ReaiFunctionId fn_id,
        CString        new_name
    );
    ReaiAnalysisStatus
        reai_get_analysis_status (Reai* reai, ReaiResponse* response, ReaiBinaryId bin_id);
    ReaiAnnFnMatchVec* reai_batch_binary_symbol_ann (
        Reai*         reai,
        ReaiResponse* response,
        ReaiBinaryId  bin_id,
        Size          max_results_per_function,
        Float64       min_distance,
        CStrVec*      collection
    );

#ifdef __cplusplus
}
#endif

#endif // REAI_API_REAI_H
