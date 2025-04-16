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
#include <Reai/Api/Response.h>
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

    Reai* reai_create (CString host, CString api_key);
    Reai* reai_set_mock_handler (
        Reai* reai,
        ReaiResponse* (*mock_handler) (
            Reai*         reai,
            ReaiRequest*  req,
            ReaiResponse* response,
            CString       endpoint,
            Uint32*       http_code
        )
    );
    void          reai_destroy (Reai* reai);
    ReaiResponse* reai_request (Reai* reai, ReaiRequest* req, ReaiResponse* response);

    Bool    reai_auth_check (Reai* reai, ReaiResponse* response, CString host, CString api_key);
    CString reai_upload_file (Reai* reai, ReaiResponse* response, CString file_path);
    ReaiBinaryId reai_create_analysis (
        Reai*          reai,
        ReaiResponse*  response,
        CString        ai_model,
        Uint64         base_addr,
        ReaiFnInfoVec* fn_info_vec,
        Bool           is_private,
        CString        sha_256_hash,
        CString        file_name,
        CString        cmdline_args,
        Size           size_in_bytes,
        Bool           dynamic_execution,
        Bool           skip_scraping,
        Bool           skip_cves,
        Bool           skip_sbom,
        Bool           skip_capabilities,
        Bool           ignore_cache,
        Bool           do_advanced_analysis
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
        Float64       max_distance,
        CStrVec*      collection,
        Bool          debug_mode
    );

    ReaiAnnFnMatchVec* reai_batch_function_symbol_ann (
        Reai*          reai,
        ReaiResponse*  response,
        ReaiFunctionId fn_id,
        U64Vec*        speculative_fn_ids,
        Size           max_results_per_function,
        Float64        max_distance,
        CStrVec*       collection,
        Bool           debug_mode
    );

    CStrVec* reai_get_available_models (Reai* reai, ReaiResponse* response);

    Reai* reai_begin_ai_decompilation (Reai* reai, ReaiResponse* response, ReaiFunctionId fn_id);
    ReaiAiDecompilationStatus reai_poll_ai_decompilation (
        Reai*          reai,
        ReaiResponse*  response,
        ReaiFunctionId fn_id,
        Bool           summarise
    );

    ReaiSimilarFnVec* reai_get_similar_functions (
        Reai*          reai,
        ReaiResponse*  response,
        ReaiFunctionId fn_id,
        Uint64         limit,
        Float64        distance,
        U64Vec*        collection_ids,
        Bool           debug,
        U64Vec*        binary_ids
    );

    ReaiCollectionBasicInfoVec* reai_get_basic_collection_info (
        Reai*                              reai,
        ReaiResponse*                      response,
        CString                            search_term,
        ReaiCollectionBasicInfoFilterFlags filters,
        Uint64                             limit,
        Uint64                             offset,
        ReaiCollectionBasicInfoOrderBy     order_by,
        ReaiCollectionBasicInfoOrderIn     order_in
    );

    ReaiCollectionSearchResultVec* reai_collection_search (
        Reai*         reai,
        ReaiResponse* response,
        CString       partial_collection_name,
        CString       partial_binary_name,
        CString       partial_binary_sha256,
        CStrVec*      tags,
        CString       model_name
    );

    ReaiBinarySearchResultVec* reai_binary_search (
        Reai*         reai,
        ReaiResponse* response,
        CString       partial_name,
        CString       partial_sha256,
        CStrVec*      tags,
        CString       model_name
    );

    ReaiAnalysisId reai_analysis_id_from_binary_id (
        Reai*         reai,
        ReaiResponse* respomse,
        ReaiBinaryId  binary_id
    );

    CString reai_get_analysis_logs (Reai* reai, ReaiResponse* respomse, ReaiAnalysisId analysis_id);

#ifdef __cplusplus
}
#endif

#endif // REAI_API_REAI_H
