/**
 * @file Reai.c
 * @date 10th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#include <Reai/Common.h>

/* curl */
#include <curl/curl.h>

/* reai */
#include <Reai/Api/Api.h>
#include <Reai/Db.h>
#include <Reai/Log.h>
#include <Reai/Util/Vec.h>

/* libc */
#include <memory.h>

struct Reai {
    CURL*              curl;
    struct curl_slist* headers;

    CString host;
    CString api_key;

    ReaiDb*  db;
    ReaiLog* logger;
};

HIDDEN Size reai_response_write_callback (void* ptr, Size size, Size nmemb, ReaiResponse* response);
HIDDEN ReaiResponse* reai_response_init_for_type (ReaiResponse* response, ReaiResponseType type);
HIDDEN CString       reai_request_to_json_cstr (ReaiRequest* request);
HIDDEN ReaiResponse* reai_response_reset (ReaiResponse* request);
Reai*                reai_init_conn (Reai* reai);
Reai*                reai_deinit_conn (Reai* reai);

#define ENDPOINT_URL_STR_SIZE 100

/**
 * @b Create a new @c Reai object to handle connection with RevEngAI servers.
 * 
 * @param reai @c Reai object to be initialized with connection data.
 * @param host Base address of reai api in use
 * @param api_key API key provided for using RevEngAI.
 *
 * @return @c Reai.
 * */
Reai* reai_create (CString host, CString api_key) {
    RETURN_VALUE_IF (!host || !api_key, Null, ERR_INVALID_ARGUMENTS);

    Reai* reai = NEW (Reai);
    RETURN_VALUE_IF (!reai, Null, ERR_INVALID_ARGUMENTS);

    GOTO_HANDLER_IF (
        !(reai->host = strdup (host)) || !(reai->api_key = strdup (api_key)),
        CREATE_FAILED,
        ERR_OUT_OF_MEMORY
    );

    GOTO_HANDLER_IF (!reai_init_conn (reai), CREATE_FAILED, "Failed to create connection.");

    return reai;

CREATE_FAILED:
    reai_destroy (reai);
    return Null;
}

/**
 * Initialize the curl handle as if it was created recently.
 * */
Reai* reai_init_conn (Reai* reai) {
    RETURN_VALUE_IF (!reai, Null, ERR_INVALID_ARGUMENTS);

    /* initialize curl isntance and set url */
    reai->curl = curl_easy_init();
    RETURN_VALUE_IF (!reai->curl, Null, "Failed to easy init curl");

    curl_easy_setopt (reai->curl, CURLOPT_WRITEFUNCTION, reai_response_write_callback);
    curl_easy_setopt (reai->curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt (reai->curl, CURLOPT_USERAGENT, "creait");
    curl_easy_setopt (reai->curl, CURLOPT_MAXREDIRS, 50);

    /* cache the CA cert bundle in memory for a week */
    curl_easy_setopt (reai->curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);
    /* curl_easy_setopt (reai->curl, CURLOPT_VERBOSE, 1); */

    Char auth_hdr_str[80];
    snprintf (auth_hdr_str, sizeof (auth_hdr_str), "Authorization: %s", reai->api_key);
    reai->headers = curl_slist_append (reai->headers, auth_hdr_str);

    /* reai->headers = curl_slist_append (reai->headers, "Expect:"); */
    RETURN_VALUE_IF (!reai->headers, Null, "Failed to prepare initial headers");

    /* set headers */
    curl_easy_setopt (reai->curl, CURLOPT_HTTPHEADER, reai->headers);

    return reai;
}

Reai* reai_deinit_conn (Reai* reai) {
    RETURN_VALUE_IF (!reai, Null, ERR_INVALID_ARGUMENTS);

    if (reai->headers) {
        curl_slist_free_all (reai->headers);
        reai->headers = Null;
    }
    if (reai->curl) {
        curl_easy_cleanup (reai->curl);
        reai->curl = Null;
    }

    return reai;
}

/**
 * @b De-initialize given @c Reai object.
 *
 * @param reai Connection to be de-initialized
 * */
void reai_destroy (Reai* reai) {
    RETURN_IF (!reai, ERR_INVALID_ARGUMENTS);

    reai_deinit_conn (reai);

    if (reai->db) {
        reai_db_destroy (reai->db);
        reai->db = Null;
    }

    if (reai->logger) {
        reai_log_destroy (reai->logger);
        reai->logger = Null;
    }

    if (reai->host) {
        FREE (reai->host);
        reai->host = Null;
    }

    if (reai->api_key) {
        FREE (reai->api_key);
        reai->api_key = Null;
    }

    FREE (reai);
}

/**
 * @b Perform an API request to RevEngAI.
 *
 * @param[in] reai
 * @param[in] request
 * @param[out] response
 *
 * @return @c response on successful request and response.
 * @return @c Null otherwise.
 * */
ReaiResponse* reai_request (Reai* reai, ReaiRequest* request, ReaiResponse* response) {
    RETURN_VALUE_IF (!reai || !request || !response, Null, ERR_INVALID_ARGUMENTS);

    // BUG: Not able to properly reuse the curl handle.
    // HACK: Fixed this for now by recreating it one every connection

    reai_deinit_conn (reai);
    reai_init_conn (reai);
    reai_response_reset (response);

    /* curl will now write raw data to this response */
    curl_easy_setopt (reai->curl, CURLOPT_WRITEDATA, response);

    /* temporary buffer to hold endpoint url */
    Char endpoint_str[ENDPOINT_URL_STR_SIZE];

    /* from data for uploading file */
    struct curl_mime*     mime     = Null;
    struct curl_mimepart* mimepart = Null;

#define SET_ENDPOINT(fmtstr, ...)                                                                  \
    do {                                                                                           \
        snprintf (endpoint_str, sizeof (endpoint_str), fmtstr, __VA_ARGS__);                       \
        curl_easy_setopt (reai->curl, CURLOPT_URL, endpoint_str);                                  \
        if (reai->logger) {                                                                        \
            REAI_LOG_TRACE (reai->logger, "ENDPOINT : '%s'", endpoint_str);                        \
        }                                                                                          \
    } while (0)

#define SET_METHOD(method)                                                                         \
    do {                                                                                           \
        curl_easy_setopt (reai->curl, CURLOPT_CUSTOMREQUEST, method);                              \
        if (reai->logger) {                                                                        \
            REAI_LOG_TRACE (reai->logger, "METHOD : '%s'", method);                                \
        }                                                                                          \
    } while (0)

#define MAKE_REQUEST(expected_retcode, expected_response)                                          \
    do {                                                                                           \
        CURLcode retcode = curl_easy_perform (reai->curl);                                         \
        if (retcode == CURLE_OK) {                                                                 \
            Uint32 http_code = 0;                                                                  \
            curl_easy_getinfo (reai->curl, CURLINFO_RESPONSE_CODE, &http_code);                    \
                                                                                                   \
            if (reai->logger) {                                                                    \
                if (response->raw.data && response->raw.length) {                                  \
                    REAI_LOG_TRACE (reai->logger, "RESPONSE.JSON : '%s'", response->raw.data);     \
                } else {                                                                           \
                    REAI_LOG_TRACE (reai->logger, "RESPONSE.JSON : INVALID");                      \
                }                                                                                  \
            }                                                                                      \
                                                                                                   \
            response = reai_response_init_for_type (                                               \
                response,                                                                          \
                http_code == expected_retcode ? expected_response :                                \
                http_code == 422              ? REAI_RESPONSE_TYPE_VALIDATION_ERR :                \
                                                REAI_RESPONSE_TYPE_UNKNOWN_ERR                     \
            );                                                                                     \
        } else {                                                                                   \
            PRINT_ERR ("Interaction with API endpoint '%s' failed\n", endpoint_str);               \
            response = reai_response_init_for_type (response, REAI_RESPONSE_TYPE_UNKNOWN_ERR);     \
        }                                                                                          \
    } while (0)

#define MAKE_JSON_REQUEST(expected_retcode, expected_response)                                     \
    do {                                                                                           \
        /* add json header info */                                                                 \
        curl_slist_append (reai->headers, "Content-Type: application/json");                       \
                                                                                                   \
        /* convert request to json string */                                                       \
        CString json = reai_request_to_json_cstr (request);                                        \
        GOTO_HANDLER_IF (!json, REQUEST_FAILED, "Failed to convert request to JSON");              \
        if (reai->logger) {                                                                        \
            REAI_LOG_TRACE (reai->logger, "REQUEST.JSON : '%s'", json);                            \
        }                                                                                          \
                                                                                                   \
        /* set json data */                                                                        \
        curl_easy_setopt (reai->curl, CURLOPT_POSTFIELDS, json);                                   \
                                                                                                   \
        /* WARN: this method will fail if MAKE request makes any jump */                           \
        MAKE_REQUEST (expected_retcode, expected_response);                                        \
                                                                                                   \
        /* free after use */                                                                       \
        FREE (json);                                                                               \
                                                                                                   \
        /* unset json data */                                                                      \
        curl_easy_setopt (reai->curl, CURLOPT_POSTFIELDS, Null);                                   \
    } while (0)

#define MAKE_UPLOAD_REQUEST(expected_retcode, expected_response)                                   \
    do {                                                                                           \
        /* create a new mime */                                                                    \
        mime = curl_mime_init (reai->curl);                                                        \
        GOTO_HANDLER_IF (!mime, REQUEST_FAILED, "Failed to create mime data");                     \
                                                                                                   \
        /* create mimepart for multipart data */                                                   \
        mimepart = curl_mime_addpart (mime);                                                       \
        GOTO_HANDLER_IF (!mimepart, REQUEST_FAILED, "Failed to add mime part to mime data");       \
                                                                                                   \
        /* set part info */                                                                        \
        curl_mime_name (mimepart, "file");                                                         \
        curl_mime_filedata (mimepart, request->upload_file.file_path);                             \
        if (reai->logger) {                                                                        \
            REAI_LOG_TRACE (reai->logger, "UPLOAD FILE : '%s'", request->upload_file.file_path);   \
        }                                                                                          \
                                                                                                   \
        /* set the mime data for post */                                                           \
        curl_easy_setopt (reai->curl, CURLOPT_MIMEPOST, mime);                                     \
                                                                                                   \
        MAKE_REQUEST (200, REAI_RESPONSE_TYPE_UPLOAD_FILE);                                        \
                                                                                                   \
        /* remove mime data */                                                                     \
        curl_easy_setopt (reai->curl, CURLOPT_MIMEPOST, Null);                                     \
    } while (0)

    switch (request->type) {
        /* GET : api.local/v1 */
        case REAI_REQUEST_TYPE_HEALTH_CHECK : {
            SET_ENDPOINT ("%s", reai->host);
            SET_METHOD ("GET");
            MAKE_REQUEST (200, REAI_RESPONSE_TYPE_HEALTH_CHECK);
            break;
        }

        /* GET : api.local/v1/authenticate */
        case REAI_REQUEST_TYPE_AUTH_CHECK : {
            SET_ENDPOINT ("%s/authenticate", reai->host);
            SET_METHOD ("GET");
            MAKE_REQUEST (200, REAI_RESPONSE_TYPE_AUTH_CHECK);
            break;
        }

        /* POST : api.local/v1/upload */
        case REAI_REQUEST_TYPE_UPLOAD_FILE : {
            SET_ENDPOINT ("%s/upload", reai->host);
            SET_METHOD ("POST");
            MAKE_UPLOAD_REQUEST (200, REAI_RESPONSE_TYPE_UPLOAD_FILE);
            break;
        }

        /* POST : api.local/v1/analyse */
        case REAI_REQUEST_TYPE_CREATE_ANALYSIS : {
            SET_ENDPOINT ("%s/analyse", reai->host);
            SET_METHOD ("POST");
            MAKE_JSON_REQUEST (201, REAI_RESPONSE_TYPE_CREATE_ANALYSIS);
            break;
        }

        /* DELETE : api.local/v1/analyse/binary_id */
        case REAI_REQUEST_TYPE_DELETE_ANALYSIS : {
            SET_ENDPOINT ("%s/analyse/%llu", reai->host, request->delete_analysis.binary_id);
            SET_METHOD ("DELETE");
            MAKE_REQUEST (200, REAI_RESPONSE_TYPE_DELETE_ANALYSIS);
            break;
        }

        /* GET : api.local/v1/analyse/functions/binary_id */
        case REAI_REQUEST_TYPE_BASIC_FUNCTION_INFO : {
            SET_ENDPOINT (
                "%s/analyse/functions/%llu",
                reai->host,
                request->basic_function_info.binary_id
            );
            SET_METHOD ("GET");
            MAKE_REQUEST (200, REAI_RESPONSE_TYPE_BASIC_FUNCTION_INFO);
            break;
        }

        /* GET : api.local/v1/analyse/recent */
        case REAI_REQUEST_TYPE_RECENT_ANALYSIS : {
            SET_ENDPOINT ("%s/analyse/recent", reai->host);
            SET_METHOD ("GET");
            MAKE_JSON_REQUEST (200, REAI_RESPONSE_TYPE_RECENT_ANALYSIS);
            break;
        }

            /* GET : api.local/v1/analyse/status/{binary_id} */
        case REAI_REQUEST_TYPE_ANALYSIS_STATUS : {
            SET_ENDPOINT ("%s/analyse/status/%llu", reai->host, request->analysis_status.binary_id);
            SET_METHOD ("GET");
            MAKE_REQUEST (200, REAI_RESPONSE_TYPE_ANALYSIS_STATUS);
            break;
        }

        case REAI_REQUEST_TYPE_SEARCH : {
            SET_ENDPOINT ("%s/search", reai->host);
            SET_METHOD ("GET");
            MAKE_JSON_REQUEST (200, REAI_RESPONSE_TYPE_SEARCH);
            break;
        }

        case REAI_REQUEST_TYPE_BATCH_RENAMES_FUNCTIONS : {
            SET_ENDPOINT ("%s/functions/batch/rename", reai->host);
            SET_METHOD ("POST");
            MAKE_JSON_REQUEST (200, REAI_RESPONSE_TYPE_BATCH_RENAMES_FUNCTIONS);
            break;
        }

        case REAI_REQUEST_TYPE_RENAME_FUNCTION : {
            SET_ENDPOINT (
                "%s/functions/rename/%llu",
                reai->host,
                request->rename_function.function_id
            );
            SET_METHOD ("POST");
            MAKE_JSON_REQUEST (200, REAI_RESPONSE_TYPE_RENAME_FUNCTION);
            break;
        }

        case REAI_REQUEST_TYPE_BATCH_BINARY_SYMBOL_ANN : {
            SET_ENDPOINT (
                "%s/ann/symbol/%llu",
                reai->host,
                request->batch_binary_symbol_ann.binary_id
            );
            SET_METHOD ("POST");
            MAKE_JSON_REQUEST (200, REAI_RESPONSE_TYPE_BATCH_BINARY_SYMBOL_ANN);
            break;
        }

        default :
            PRINT_ERR ("Invalid request.");
            break;
    }

#undef MAKE_JSON_REQUEST
#undef MAKE_UPLOAD_REQUEST
#undef MAKE_REQUEST
#undef SET_METHOD
#undef SET_ENDPOINT

DEFAULT_RETURN:
    if (mime) {
        curl_mime_free (mime);
    }

    return response;

REQUEST_FAILED:
    response = Null;
    goto DEFAULT_RETURN;
};

/**
 * @b Set a database in Reai. This will make sure that database
 * is automatically updated in case of file upload or analysis creation.
 *
 * The given DB object is now owned by Reai and will automatically be
 * destroyed when given Reai object is destroyed.
 *
 * @param reai
 * @param db
 *
 * @return @c reai On success.
 * @return @c Null otherwise.
 * */
Reai* reai_set_db (Reai* reai, ReaiDb* db) {
    RETURN_VALUE_IF (!reai || !db, Null, ERR_INVALID_ARGUMENTS);

    reai->db = db;

    return reai;
}

/**
 * @b Set a logger in Reai. This will increase verbosity in the logger.
 * Every request and respose will be dumped into the log file.
 *
 * The given Log object is now owned by Reai and will automatically be
 * destroyed when given Reai object is destroyed.
 *
 * @param reai
 * @param logger
 *
 * @return @c reai On success.
 * @return @c Null otherwise.
 * */
Reai* reai_set_logger (Reai* reai, ReaiLog* logger) {
    RETURN_VALUE_IF (!reai || !logger, Null, ERR_INVALID_ARGUMENTS);

    reai->logger = logger;

    return reai;
}

/**
 * @b Update analysis status of all created analyses in database.
 *
 * This call requires that Reai already has a set database.
 *
 * @param reai
 *
 * @return @c reai On success.
 * @return @c Null otherwise.
 * */
Reai* reai_update_all_analyses_status_in_db (Reai* reai) {
    RETURN_VALUE_IF (!reai, Null, ERR_INVALID_ARGUMENTS);

    /* no need to continue if we don't need an update */
    if (!reai_db_require_analysis_status_update (reai->db)) {
        return reai;
    }

    U64Vec* bin_ids_in_processing =
        reai_db_get_analyses_with_status (reai->db, REAI_ANALYSIS_STATUS_PROCESSING);
    U64Vec* bin_ids_in_queue =
        reai_db_get_analyses_with_status (reai->db, REAI_ANALYSIS_STATUS_QUEUED);
    RETURN_VALUE_IF (!bin_ids_in_processing, Null, "Failed to get all created analyses from DB.");

    ReaiResponse response;
    GOTO_HANDLER_IF (
        !reai_response_init (&response),
        STATUS_UPDATE_FAILED,
        "Failed to create response structure."
    );

#define UPDATE_STATUS_FOR(bin_ids)                                                                 \
    REAI_VEC_FOREACH (bin_ids, bin_id, {                                                           \
        ReaiRequest request               = {0};                                                   \
        request.type                      = REAI_REQUEST_TYPE_ANALYSIS_STATUS;                     \
        request.analysis_status.binary_id = *bin_id;                                               \
                                                                                                   \
        GOTO_HANDLER_IF (                                                                          \
            !reai_request (reai, &request, &response),                                             \
            STATUS_UPDATE_FAILED,                                                                  \
            "Failed to make request to RevEngAI servers : %s.",                                    \
            response.raw.data                                                                      \
        );                                                                                         \
                                                                                                   \
        GOTO_HANDLER_IF (                                                                          \
            !response.analysis_status.status,                                                      \
            STATUS_UPDATE_FAILED,                                                                  \
            "Invalid analysis status."                                                             \
        );                                                                                         \
                                                                                                   \
        GOTO_HANDLER_IF (                                                                          \
            !reai_db_set_analysis_status (reai->db, *bin_id, response.analysis_status.status),     \
            STATUS_UPDATE_FAILED,                                                                  \
            "Failed to update analysis status in db."                                              \
        );                                                                                         \
    });

    UPDATE_STATUS_FOR (bin_ids_in_processing);
    UPDATE_STATUS_FOR (bin_ids_in_queue);

#undef UPDATE_STATUS_FOR

DEFAULT_RETURN:
    reai_u64_vec_destroy (bin_ids_in_processing);
    reai_u64_vec_destroy (bin_ids_in_queue);
    reai_response_deinit (&response);
    return reai;

STATUS_UPDATE_FAILED:
    reai = Null;
    goto DEFAULT_RETURN;
}

/**
 * Upload given file.
 *
 * The returned string is owned the @c response provided. When response
 * is reset or destroyed, the returned string will automatically be freed as well.
 *
 * @parma reai
 * @param response Where complete response will be stored.
 * @param file_path
 *
 * @return @c CString containing returned sh256 hash value in response.
 * @return @c Null on failure.
 * */
CString reai_upload_file (Reai* reai, ReaiResponse* response, CString file_path) {
    RETURN_VALUE_IF (!reai || !file_path, Null, ERR_INVALID_ARGUMENTS);

    /* prepare request */
    ReaiRequest request           = {0};
    request.type                  = REAI_REQUEST_TYPE_UPLOAD_FILE;
    request.upload_file.file_path = file_path;

    /* make request */
    if ((response = reai_request (reai, &request, response))) {
        switch (response->type) {
            case REAI_RESPONSE_TYPE_UPLOAD_FILE : {
                if (reai->db) {
                    if (!reai_db_add_upload (
                            reai->db,
                            file_path,
                            response->upload_file.sha_256_hash
                        )) {
                        PRINT_ERR ("Failed to add uploaded file into to Reai DB.");
                    }
                }

                return response->upload_file.sha_256_hash;
            }
            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                return Null;
            }
            default : {
                RETURN_VALUE_IF_REACHED (Null, "Unexpected type.");
            }
        }
    } else {
        return Null;
    }
}

/**
 * @b Create a new analysis by providing as minimum context as possible.
 *
 * @param reai
 * @param response
 * @param model
 * @param fn_info_vec Must be provided if functions don't have default boundaries.
 * @param is_private @c True if new analysis created will be private.
 * @param sha_256_hash SHA-256 hash value for previously uploaded binary.
 * @param file_name Name of file (will be name of analysis as well)
 * @param size_in_bytes Size of file in bytes.
 *
 * @return binary id on success.
 * @return 0 on failure.
 * */
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
) {
    RETURN_VALUE_IF (
        !reai || !response || !model || !sha_256_hash || !file_name || !size_in_bytes,
        0,
        ERR_INVALID_ARGUMENTS
    );

    /* prepare new request to perform analysis */
    ReaiRequest request = {0};
    request.type        = REAI_REQUEST_TYPE_CREATE_ANALYSIS;

    request.create_analysis.model        = model;
    request.create_analysis.platform_opt = Null;
    request.create_analysis.isa_opt      = Null;
    request.create_analysis.file_opt     = REAI_FILE_OPTION_DEFAULT;
    request.create_analysis.dyn_exec     = False;
    request.create_analysis.tags         = Null;
    request.create_analysis.tags_count   = 0;
    request.create_analysis.base_addr    = base_addr;
    request.create_analysis.functions    = fn_info_vec;
    request.create_analysis.bin_scope =
        is_private ? REAI_BINARY_SCOPE_PRIVATE : REAI_BINARY_SCOPE_PUBLIC;
    request.create_analysis.file_name     = file_name;
    request.create_analysis.cmdline_args  = cmdline_args;
    request.create_analysis.priority      = 0;
    request.create_analysis.sha_256_hash  = sha_256_hash;
    request.create_analysis.debug_hash    = Null;
    request.create_analysis.size_in_bytes = size_in_bytes;

    if ((response = reai_request (reai, &request, response))) {
        switch (response->type) {
            case REAI_RESPONSE_TYPE_CREATE_ANALYSIS : {
                if (reai->db) {
                    if (!reai_db_add_analysis (
                            reai->db,
                            response->create_analysis.binary_id,
                            sha_256_hash,
                            model,
                            file_name,
                            cmdline_args
                        )) {
                        PRINT_ERR ("Failed to add created analysis info to database.");
                    }
                }

                return response->create_analysis.binary_id;
            }
            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                return 0;
            }
            default : {
                RETURN_VALUE_IF_REACHED (0, "Unexpected response type.");
            }
        }
    } else {
        return 0;
    }
}

/**
 * @b Get basic function info for given binary id.
 *
 * Returned vector is owned by the given @c response object. The vector
 * will automatically be destroyed when response is reset or destroyed.
 *
 * @param reai
 * @param response
 * @parma bin_id
 *
 * @return @c ReaiFnInfoVec* on success.
 * @return @c Null otherwise.
 * */
ReaiFnInfoVec*
    reai_get_basic_function_info (Reai* reai, ReaiResponse* response, ReaiBinaryId bin_id) {
    RETURN_VALUE_IF (!reai || !response || !bin_id, Null, ERR_INVALID_ARGUMENTS);

    /* prepare new request to get function info list for given binary id */
    ReaiRequest request = {0};
    request.type        = REAI_REQUEST_TYPE_BASIC_FUNCTION_INFO;

    request.basic_function_info.binary_id = bin_id;

    if ((response = reai_request (reai, &request, response))) {
        switch (response->type) {
            case REAI_RESPONSE_TYPE_BASIC_FUNCTION_INFO : {
                return response->basic_function_info.fn_infos;
            }
            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                return Null;
            }
            default : {
                RETURN_VALUE_IF_REACHED (Null, "Unexpected response type.");
            }
        }
    } else {
        return Null;
    }
}


/**
 * @b Get recently created analysis.
 *
 * The returned vector is owned the @c response provided. When response
 * is reset or destroyed, the returned vector will automatically be destroyed as well.
 *
 * @param reai
 * @param response
 * @param status Complete, Queued, etc...
 * @param scope Public, Private, Default.
 * @param count Default value is 10 (if zero is provided).
 *
 * @return @c ReaiAnalysisInfoVec reference on success.
 * @return @c Null otherwise.
 * */
ReaiAnalysisInfoVec* reai_get_recent_analyses (
    Reai*              reai,
    ReaiResponse*      response,
    ReaiAnalysisStatus status,
    ReaiBinaryScope    scope,
    Size               count
) {
    RETURN_VALUE_IF (!reai || !response, Null, ERR_INVALID_ARGUMENTS);

    /* prepare new request to get recent analysis */
    ReaiRequest request = {0};
    request.type        = REAI_REQUEST_TYPE_RECENT_ANALYSIS;

    request.recent_analysis.count  = count;
    request.recent_analysis.scope  = scope;
    request.recent_analysis.status = status;

    if ((response = reai_request (reai, &request, response))) {
        switch (response->type) {
            case REAI_RESPONSE_TYPE_RECENT_ANALYSIS : {
                return response->recent_analysis.analysis_infos;
            }
            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                return Null;
            }
            default : {
                RETURN_VALUE_IF_REACHED (Null, "Unexpected response type.");
            }
        }
    } else {
        return Null;
    }
}

/**
 * @b Rename many functions at once.
 *
 * Ownership of given vector is not transferred and caller is responsible
 * for managing it's lifetime.
 *
 * @param reai
 * @param response
 * @param new_name_mapping
 *
 * @return @c True if response type is same as request type.
 *         Note that this does not mean all functions are renamed correctly.
 * @return @c False otherwise.
 * */
Bool reai_batch_renames_functions (
    Reai*          reai,
    ReaiResponse*  response,
    ReaiFnInfoVec* new_name_mapping
) {
    RETURN_VALUE_IF (!reai || !response || !new_name_mapping, False, ERR_INVALID_ARGUMENTS);

    /* prepare new request to rename functions in batch */
    ReaiRequest request = {0};
    request.type        = REAI_REQUEST_TYPE_BATCH_RENAMES_FUNCTIONS;

    request.batch_renames_functions.new_name_mapping = new_name_mapping;

    if ((response = reai_request (reai, &request, response))) {
        switch (response->type) {
            case REAI_RESPONSE_TYPE_BATCH_RENAMES_FUNCTIONS : {
                return True;
            }
            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                return False;
            }
            default : {
                RETURN_VALUE_IF_REACHED (False, "Unexpected response type.");
            }
        }
    } else {
        return False;
    }
}

/**
 * @b Rename function with given function id to given new name.
 *
 * @param reai
 * @param response
 * @param fn_id
 * @param new_name
 *
 * @return @c True on success.
 * @return @c False otherwise.
 * */
Bool reai_rename_function (
    Reai*          reai,
    ReaiResponse*  response,
    ReaiFunctionId fn_id,
    CString        new_name
) {
    RETURN_VALUE_IF (!reai || !response || !fn_id || !new_name, False, ERR_INVALID_ARGUMENTS);

    /* prepare new request to rename functions in batch */
    ReaiRequest request = {0};
    request.type        = REAI_REQUEST_TYPE_RENAME_FUNCTION;

    request.rename_function.function_id = fn_id;
    request.rename_function.new_name    = new_name;

    if ((response = reai_request (reai, &request, response))) {
        switch (response->type) {
            case REAI_RESPONSE_TYPE_RENAME_FUNCTION : {
                return response->rename_function.success;
            }
            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                return False;
            }
            default : {
                RETURN_VALUE_IF_REACHED (False, "Unexpected response type.");
            }
        }
    } else {
        return False;
    }
}


/**
 * @b Perform ANN Auto analysis and get similar function names.
 *
 * The returned vector is not to be explicitly freed as it is owned by
 * the provided response object. The returned vector will automatically
 * be freed when given response is deinitialized.
 *
 * @param reai
 * @param response
 * @param bin_id
 * @param max_results_per_function
 * @param min_distance
 * @param collection Can be @c Null.
 *
 * @return @c ReaiAnnFnMatch on success.
 * @return @c Null otherwise.
 * */
ReaiAnnFnMatchVec* reai_batch_binary_symbol_ann (
    Reai*         reai,
    ReaiResponse* response,
    ReaiBinaryId  bin_id,
    Size          max_results_per_function,
    Float64       max_distance,
    CStrVec*      collection
) {
    RETURN_VALUE_IF (!reai || !response || !bin_id, Null, ERR_INVALID_ARGUMENTS);

    /* prepare new request to rename functions in batch */
    ReaiRequest request = {0};
    request.type        = REAI_REQUEST_TYPE_BATCH_BINARY_SYMBOL_ANN;

    request.batch_binary_symbol_ann.binary_id            = bin_id;
    request.batch_binary_symbol_ann.collection           = collection ? collection->items : Null;
    request.batch_binary_symbol_ann.collection_count     = collection ? collection->count : 0;
    request.batch_binary_symbol_ann.results_per_function = max_results_per_function;
    request.batch_binary_symbol_ann.distance             = max_distance;

    if ((response = reai_request (reai, &request, response))) {
        switch (response->type) {
            case REAI_RESPONSE_TYPE_BATCH_BINARY_SYMBOL_ANN : {
                return response->batch_binary_symbol_ann.success ?
                           response->batch_binary_symbol_ann.function_matches :
                           Null;
            }

            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                return Null;
            }

            default : {
                RETURN_VALUE_IF_REACHED (Null, "Unexpected response type.");
            }
        }
    } else {
        return Null;
    }
}


/**
 * @b Get analysis status for given binary.
 *
 * @param reai
 * @param response
 * @param bin_id Binary ID to get analysis status for.
 *
 * @return @c ReaiAnaylsisStatus < @c REAI_ANALYSIS_STATUS_MAX enum on success.
 * @return @c REAI_ANALYSIS_STATUS_INVALID otherwise.
 * */
ReaiAnalysisStatus
    reai_get_analysis_status (Reai* reai, ReaiResponse* response, ReaiBinaryId bin_id) {
    RETURN_VALUE_IF (!reai || !response, REAI_ANALYSIS_STATUS_INVALID, ERR_INVALID_ARGUMENTS);

    /* prepare new request to get analysis status */
    ReaiRequest request = {0};
    request.type        = REAI_REQUEST_TYPE_ANALYSIS_STATUS;

    request.analysis_status.binary_id = bin_id;

    if ((response = reai_request (reai, &request, response))) {
        switch (response->type) {
            case REAI_RESPONSE_TYPE_ANALYSIS_STATUS : {
                return response->analysis_status.status;
            }
            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                return REAI_ANALYSIS_STATUS_INVALID;
            }
            default : {
                RETURN_VALUE_IF_REACHED (REAI_ANALYSIS_STATUS_INVALID, "Unexpected response type.");
            }
        }
    } else {
        return REAI_ANALYSIS_STATUS_INVALID;
    }
}
