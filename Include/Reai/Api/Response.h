/**
 * @file Response.h
 * @date 9th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_API_RESPONSE_H
#define REAI_API_RESPONSE_H

#include <Reai/AnalysisInfo.h>
#include <Reai/AnnFnMatch.h>
#include <Reai/Api/Request.h>
#include <Reai/ApiError.h>
#include <Reai/FnInfo.h>
#include <Reai/QueryResult.h>

#ifdef __cplusplus
extern "C" {
#endif

    /* fwd declarations */
    typedef struct ReaiResponseBuf ReaiResponseBuf;

    typedef enum ReaiResponseType {
        REAI_RESPONSE_TYPE_UNKNOWN_ERR = 0,

        /* health api */
        REAI_RESPONSE_TYPE_HEALTH_CHECK = REAI_REQUEST_TYPE_HEALTH_CHECK,

        /* authentication api */
        REAI_RESPONSE_TYPE_AUTH_CHECK = REAI_REQUEST_TYPE_AUTH_CHECK,

        /* utility api */
        REAI_RESPONSE_TYPE_UPLOAD_FILE = REAI_REQUEST_TYPE_UPLOAD_FILE,
        /* REAI_RESPONSE_TYPE_GET_CONFIG  = REAI_REQUEST_TYPE_GET_CONFIG, */
        REAI_RESPONSE_TYPE_SEARCH     = REAI_REQUEST_TYPE_SEARCH,
        REAI_RESPONSE_TYPE_GET_MODELS = REAI_REQUEST_TYPE_GET_MODELS,

        /* analysis api */
        REAI_RESPONSE_TYPE_CREATE_ANALYSIS     = REAI_REQUEST_TYPE_CREATE_ANALYSIS,
        REAI_RESPONSE_TYPE_DELETE_ANALYSIS     = REAI_REQUEST_TYPE_DELETE_ANALYSIS,
        REAI_RESPONSE_TYPE_BASIC_FUNCTION_INFO = REAI_REQUEST_TYPE_BASIC_FUNCTION_INFO,
        REAI_RESPONSE_TYPE_RECENT_ANALYSIS     = REAI_REQUEST_TYPE_RECENT_ANALYSIS,
        REAI_RESPONSE_TYPE_ANALYSIS_STATUS     = REAI_REQUEST_TYPE_ANALYSIS_STATUS,

        /* analysis info api */
        /* REAI_RESPONSE_TYPE_GET_ANALYSIS_LOGS = REAI_REQUEST_TYPE_GET_ANALYSIS_LOGS, */
        REAI_RESPONSE_TYPE_BATCH_RENAMES_FUNCTIONS = REAI_REQUEST_TYPE_BATCH_RENAMES_FUNCTIONS,
        REAI_RESPONSE_TYPE_RENAME_FUNCTION         = REAI_REQUEST_TYPE_RENAME_FUNCTION,
        /* REAI_RESPONSE_TYPE_GET_FUNCTION_DISASSEMBLY_DUMPS = REAI_REQUEST_TYPE_GET_FUNCTION_DISASSEMBLY_DUMPS, */

        /* ann api */
        REAI_RESPONSE_TYPE_BATCH_BINARY_SYMBOL_ANN   = REAI_REQUEST_TYPE_BATCH_BINARY_SYMBOL_ANN,
        REAI_RESPONSE_TYPE_BATCH_FUNCTION_SYMBOL_ANN = REAI_REQUEST_TYPE_BATCH_FUNCTION_SYMBOL_ANN,

        /* ai decompilation */
        REAI_RESPONSE_TYPE_BEGIN_AI_DECOMPILATION = REAI_REQUEST_TYPE_BEGIN_AI_DECOMPILATION,
        REAI_RESPONSE_TYPE_POLL_AI_DECOMPILATION  = REAI_REQUEST_TYPE_POLL_AI_DECOMPILATION,

        REAI_RESPONSE_TYPE_VALIDATION_ERR,
        REAI_RESPONSE_TYPE_MAX, /* enum value less than this is valid */
    } ReaiResponseType;

    typedef enum ReaiAiDecompilationStatus {
        REAI_AI_DECOMPILATION_STATUS_ERROR,
        REAI_AI_DECOMPILATION_STATUS_UNINITIALIZED,
        REAI_AI_DECOMPILATION_STATUS_PENDING,
        REAI_AI_DECOMPILATION_STATUS_SUCCESS,
    } ReaiAiDecompilationStatus;

    static inline ReaiAiDecompilationStatus reai_ai_decompilation_status_from_cstr (CString status
    ) {
        if (!status) {
            return REAI_AI_DECOMPILATION_STATUS_ERROR;
        } else if (!strcmp (status, "uninitialized")) {
            return REAI_AI_DECOMPILATION_STATUS_UNINITIALIZED;
        } else if (!strcmp (status, "pending")) {
            return REAI_AI_DECOMPILATION_STATUS_PENDING;
        } else if (!strcmp (status, "success")) {
            return REAI_AI_DECOMPILATION_STATUS_SUCCESS;
        } else {
            return REAI_AI_DECOMPILATION_STATUS_ERROR;
        }
    }

    static inline CString reai_ai_decompilation_status_to_cstr (ReaiAiDecompilationStatus status) {
        switch (status) {
            case REAI_AI_DECOMPILATION_STATUS_UNINITIALIZED :
                return "uninitialized";
            case REAI_AI_DECOMPILATION_STATUS_PENDING :
                return "pending";
            case REAI_AI_DECOMPILATION_STATUS_SUCCESS :
                return "success";
            default :
                return "error";
        }
    }

    /**
     * @b Structure returned and taken by reai_request calls that get
     * a response from an API endpoint.
     *
     * User must initialize a `ReaiResponse` structure and must pass it
     * when making a call to `reai_request`. When a response is returned,
     * the `reai_request` method will parse the response data and store
     * all the returned details into this structure with proper response type
     * information in `type` member.
     * */
    typedef struct ReaiResponse {
        ReaiResponseType type; /**< @b Received response type. */

        /**
         * This is where write callback writes raw response data.
         * Usually this is JSON data when interacting with Reai API,
         * but this is treated as raw data.
         *
         * Internally code uses `reai_response_init_for_type(response, type)`
         * to parse this raw data as JSON data based on given type.
         *
         * This is initialized when `reai_response_init` is called with a
         * preallocated response object. The idea is to initialize a response
         * object only once and keep reusing it throughout the lifetime of application,
         * and de-initialize it only when you're done with it finally.
         * */
        struct {
            Char* data;
            Size  length;
            Size  capacity;
        } raw;

        /**
         * Like raw data this is initialized/allocated when init is called and
         * de-initialized/deallocated when deinit is called. This is again to reduce
         * the total number of allocations/reallocations throught program lifetime.
         *
         * Ideally this should've been part of the union below, but it is because
         * of the above reason that we place it here.
         * */
        struct {
            CStrVec* locations;
            CString  message; /**< @b Error message. */
            CString  type;    /**< @b Error type. */
        } validation_error;

        union {
            struct {
                Bool    success; /**< @b Is true when request was successful */
                CString message; /**< @b Message returned by request */
            } health_check, delete_analysis;

            struct {
                CString message;
            } auth_check;

            struct {
                Bool    success;      /**< @b Is true when request was successful */
                CString message;      /**< @b Message returned by request */
                CString sha_256_hash; /**< @b Contains sha256 hash value if upload was successful */
            } upload_file;

            struct {
                Bool         success;   /**< @b true when analysis created successfully */
                ReaiBinaryId binary_id; /**< @b Binary id returned in response */
            } create_analysis;

            struct {
                Bool success; /**< @b true on success */
                ReaiFnInfoVec*
                    fn_infos; /**< @b Contains array of (id, name, vaddr, size) records */
            } basic_function_info;

            struct {
                Bool                 success;
                ReaiAnalysisInfoVec* analysis_infos;
            } recent_analysis;

            struct {
                Bool               success;
                ReaiAnalysisStatus status;
            } analysis_status;

            struct {
                Bool                success;
                ReaiQueryResultVec* query_results;
            } search;

            struct {
                Bool    success;
                CString msg;
            } rename_function;

            struct {
                Bool success;
                struct {
                    CStrVec* collections;
                    Bool     debug_mode;
                    Float64  distance;
                    Size     result_per_function;
                } settings;
                ReaiAnnFnMatchVec* function_matches;
            } batch_binary_symbol_ann, batch_function_symbol_ann;

            struct {
                Bool     success;
                CStrVec* models;
            } get_models;

            struct {
                Bool           status;
                CString        message;
                ReaiApiErrors* errors;
            } begin_ai_decompilation;

            struct {
                Bool status;
                struct {
                    ReaiAiDecompilationStatus status;
                    CString                   decompilation;
                    // TODO: function mapping?
                } data;
                CString        message;
                ReaiApiErrors* errors;
            } poll_ai_decompilation;
        };
    } ReaiResponse;

    ReaiResponse* reai_response_init (ReaiResponse* response);
    ReaiResponse* reai_response_deinit (ReaiResponse* response);

#ifdef __cplusplus
}
#endif

#endif // REAI_API_RESPONSE_H
