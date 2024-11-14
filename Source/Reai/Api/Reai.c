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
#include <Reai/Log.h>
#include <Reai/Util/Vec.h>

/* libc */
#include <memory.h>

struct Reai {
    CURL*              curl;
    struct curl_slist* headers;

    CString host;
    CString api_key;
    CString model;
};

HIDDEN Size reai_response_write_callback (void* ptr, Size size, Size nmemb, ReaiResponse* response);
HIDDEN ReaiResponse* reai_response_init_for_type (ReaiResponse* response, ReaiResponseType type);
HIDDEN CString       reai_request_to_json_cstr (ReaiRequest* request, CString model);
HIDDEN ReaiResponse* reai_response_reset (ReaiResponse* request);
Reai*                reai_deinit_curl_headers (Reai* reai);
Reai*                reai_init_curl_headers (Reai* reai, CString api_key);
Reai*                reai_init_conn (Reai* reai);
Reai*                reai_deinit_conn (Reai* reai);

#define ENDPOINT_URL_STR_SIZE 100

/**
 * @b Create a new @c Reai object to handle connection with RevEngAI servers.
 * 
 * @param host Base address of reai api in use
 * @param api_key API key provided for using RevEngAI.
 * @param model AI model to use to perform analysis.
 *
 * @return @c Reai.
 * */
Reai* reai_create (CString host, CString api_key, CString model) {
    RETURN_VALUE_IF (!host || !api_key || !model, NULL, ERR_INVALID_ARGUMENTS);

    Reai* reai = NEW (Reai);
    RETURN_VALUE_IF (!reai, NULL, ERR_INVALID_ARGUMENTS);

    GOTO_HANDLER_IF (
        !(reai->host = strdup (host)) || !(reai->api_key = strdup (api_key)) ||
            !(reai->model = strdup (model)),
        CREATE_FAILED,
        ERR_OUT_OF_MEMORY
    );

    GOTO_HANDLER_IF (!reai_init_conn (reai), CREATE_FAILED, "Failed to create connection.");

    return reai;

CREATE_FAILED:
    reai_destroy (reai);
    return NULL;
}

/**
 * Initialize the curl handle as if it was created recently.
 * */
Reai* reai_init_conn (Reai* reai) {
    RETURN_VALUE_IF (!reai, NULL, ERR_INVALID_ARGUMENTS);

    /* initialize curl isntance and set url */
    reai->curl = curl_easy_init();
    RETURN_VALUE_IF (!reai->curl, NULL, "Failed to easy init curl");

    curl_easy_setopt (reai->curl, CURLOPT_WRITEFUNCTION, reai_response_write_callback);
    curl_easy_setopt (reai->curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt (reai->curl, CURLOPT_USERAGENT, "creait");
    curl_easy_setopt (reai->curl, CURLOPT_MAXREDIRS, 50);

    /* cache the CA cert bundle in memory for a week */
    curl_easy_setopt (reai->curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);
    /* curl_easy_setopt (reai->curl, CURLOPT_VERBOSE, 1); */

    reai_init_curl_headers (reai, reai->api_key);

    return reai;
}

Reai* reai_deinit_conn (Reai* reai) {
    RETURN_VALUE_IF (!reai, NULL, ERR_INVALID_ARGUMENTS);

    reai_deinit_curl_headers (reai);
    if (reai->curl) {
        curl_easy_cleanup (reai->curl);
        reai->curl = NULL;
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

    if (reai->host) {
        FREE (reai->host);
        reai->host = NULL;
    }

    if (reai->api_key) {
        FREE (reai->api_key);
        reai->api_key = NULL;
    }

    if (reai->model) {
        FREE (reai->model);
        reai->model = NULL;
    }

    FREE (reai);
}

Reai* reai_deinit_curl_headers (Reai* reai) {
    RETURN_VALUE_IF (!reai, NULL, ERR_INVALID_ARGUMENTS);

    if (reai->headers) {
        REAI_LOG_TRACE ("headers deinited");
        curl_slist_free_all (reai->headers);
        reai->headers = NULL;
    }

    return reai;
}

Reai* reai_init_curl_headers (Reai* reai, CString api_key) {
    RETURN_VALUE_IF (!reai || !api_key, NULL, ERR_INVALID_ARGUMENTS);

    REAI_LOG_TRACE ("headers init");

    Char auth_hdr_str[80];
    snprintf (auth_hdr_str, sizeof (auth_hdr_str), "Authorization: %s", api_key);
    reai->headers = curl_slist_append (reai->headers, auth_hdr_str);

    REAI_LOG_TRACE ("auth header : %s", auth_hdr_str);

    /* reai->headers = curl_slist_append (reai->headers, "Expect:"); */
    RETURN_VALUE_IF (!reai->headers, NULL, "Failed to prepare initial headers");

    /* set headers */
    curl_easy_setopt (reai->curl, CURLOPT_HTTPHEADER, reai->headers);


    return reai;
}

/**
 * @b Perform an API request to RevEngAI.
 *
 * @param[in] reai
 * @param[in] request
 * @param[out] response
 *
 * @return @c response on successful request and response.
 * @return @c NULL otherwise.
 * */
ReaiResponse* reai_request (Reai* reai, ReaiRequest* request, ReaiResponse* response) {
    RETURN_VALUE_IF (!reai || !request || !response, NULL, ERR_INVALID_ARGUMENTS);

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
    struct curl_mime*     mime     = NULL;
    struct curl_mimepart* mimepart = NULL;

#define SET_ENDPOINT(fmtstr, ...)                                                                  \
    do {                                                                                           \
        snprintf (endpoint_str, sizeof (endpoint_str), fmtstr, __VA_ARGS__);                       \
        curl_easy_setopt (reai->curl, CURLOPT_URL, endpoint_str);                                  \
        REAI_LOG_TRACE ("ENDPOINT : '%s'", endpoint_str);                                          \
    } while (0)

#define SET_METHOD(method)                                                                         \
    do {                                                                                           \
        curl_easy_setopt (reai->curl, CURLOPT_CUSTOMREQUEST, method);                              \
        REAI_LOG_TRACE ("METHOD : '%s'", method);                                                  \
    } while (0)

#define MAKE_REQUEST(expected_retcode, expected_response)                                          \
    do {                                                                                           \
        CURLcode retcode = curl_easy_perform (reai->curl);                                         \
        if (retcode == CURLE_OK) {                                                                 \
            Uint32 http_code = 0;                                                                  \
            curl_easy_getinfo (reai->curl, CURLINFO_RESPONSE_CODE, &http_code);                    \
                                                                                                   \
            REAI_LOG_TRACE ("Response code : %u", http_code);                                      \
            if (response->raw.data && response->raw.length) {                                      \
                REAI_LOG_TRACE ("RESPONSE.JSON : '%s'", response->raw.data);                       \
            } else {                                                                               \
                REAI_LOG_TRACE ("RESPONSE.JSON : INVALID");                                        \
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
        CString json = reai_request_to_json_cstr (request, reai->model);                           \
        GOTO_HANDLER_IF (!json, REQUEST_FAILED, "Failed to convert request to JSON");              \
        REAI_LOG_TRACE ("REQUEST.JSON : '%s'", json);                                              \
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
        curl_easy_setopt (reai->curl, CURLOPT_POSTFIELDS, NULL);                                   \
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
        REAI_LOG_TRACE ("UPLOAD FILE : '%s'", request->upload_file.file_path);                     \
                                                                                                   \
        /* set the mime data for post */                                                           \
        curl_easy_setopt (reai->curl, CURLOPT_MIMEPOST, mime);                                     \
                                                                                                   \
        MAKE_REQUEST (200, REAI_RESPONSE_TYPE_UPLOAD_FILE);                                        \
                                                                                                   \
        /* remove mime data */                                                                     \
        curl_easy_setopt (reai->curl, CURLOPT_MIMEPOST, NULL);                                     \
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
            reai_deinit_curl_headers (reai); // NOTE: Not the best solution here but it works...
            reai_init_curl_headers (reai, request->auth_check.api_key);
            SET_ENDPOINT ("%s/authenticate", request->auth_check.host);
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

        case REAI_REQUEST_TYPE_GET_MODELS : {
            SET_ENDPOINT ("%s/models", reai->host);
            SET_METHOD ("GET");
            MAKE_REQUEST (200, REAI_RESPONSE_TYPE_GET_MODELS);
            break;
        }

        /* POST : api.local/v1/analyse */
        case REAI_REQUEST_TYPE_CREATE_ANALYSIS : {
            SET_ENDPOINT ("%s/analyse/", reai->host);
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

        case REAI_REQUEST_TYPE_BATCH_FUNCTION_SYMBOL_ANN : {
            SET_ENDPOINT ("%s/ann/symbol/batch", reai->host);
            SET_METHOD ("POST");
            MAKE_JSON_REQUEST (200, REAI_RESPONSE_TYPE_BATCH_FUNCTION_SYMBOL_ANN);
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
    // restore api key in headers
    if (request->type == REAI_REQUEST_TYPE_AUTH_CHECK) {
        reai_deinit_curl_headers (reai);
        reai_init_curl_headers (reai, reai->api_key);
    }

    if (mime) {
        curl_mime_free (mime);
    }

    return response;

REQUEST_FAILED:
    response = NULL;
    goto DEFAULT_RETURN;
};

/**
 * @b Make call to "/authenticate" endpoint to check whether the given
 * authentication key works or not.
 *
 * @param reai
 * @param response
 * @param api_key Api key to check against
 *
 * @return true if auth check is successful
 * @return false otherwise
 * */
Bool reai_auth_check (Reai* reai, ReaiResponse* response, CString host, CString api_key) {
    RETURN_VALUE_IF (!reai || !response || !api_key || !host, false, ERR_INVALID_ARGUMENTS);


    /* prepare request */
    ReaiRequest request        = {0};
    request.type               = REAI_REQUEST_TYPE_AUTH_CHECK;
    request.auth_check.api_key = api_key;
    request.auth_check.host    = host;

    /* make request */
    if ((response = reai_request (reai, &request, response))) {
        switch (response->type) {
            case REAI_RESPONSE_TYPE_AUTH_CHECK : {
                return true;
            }
            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                REAI_LOG_ERROR ("reveng.ai request returned validation error.");
                return false;
            }
            default : {
                RETURN_VALUE_IF_REACHED (false, "Unexpected type.");
            }
        }
    } else {
        REAI_LOG_ERROR ("Failed to make reveng.ai request");
        return false;
    }
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
 * @return @c NULL on failure.
 * */
CString reai_upload_file (Reai* reai, ReaiResponse* response, CString file_path) {
    RETURN_VALUE_IF (!reai || !file_path, NULL, ERR_INVALID_ARGUMENTS);

    /* prepare request */
    ReaiRequest request           = {0};
    request.type                  = REAI_REQUEST_TYPE_UPLOAD_FILE;
    request.upload_file.file_path = file_path;

    /* make request */
    if ((response = reai_request (reai, &request, response))) {
        switch (response->type) {
            case REAI_RESPONSE_TYPE_UPLOAD_FILE : {
                return response->upload_file.sha_256_hash;
            }
            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                REAI_LOG_ERROR ("reveng.ai request returned validation error.");
                return NULL;
            }
            default : {
                RETURN_VALUE_IF_REACHED (NULL, "Unexpected type.");
            }
        }
    } else {
        REAI_LOG_ERROR ("Failed to make reveng.ai request");
        return NULL;
    }
}

/**
 * @b Create a new analysis by providing as minimum context as possible.
 *
 * @param reai
 * @param response
 * @param model
 * @param fn_info_vec Must be provided if functions don't have default boundaries.
 * @param is_private @c true if new analysis created will be private.
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
    CString        ai_model,
    Uint64         base_addr,
    ReaiFnInfoVec* fn_info_vec,
    Bool           is_private,
    CString        sha_256_hash,
    CString        file_name,
    CString        cmdline_args,
    Size           size_in_bytes
) {
    RETURN_VALUE_IF (
        !reai || !response || !ai_model || !strlen (ai_model) || !sha_256_hash ||
            !(strlen (sha_256_hash)) || !file_name || !strlen (file_name) || !size_in_bytes,
        0,
        ERR_INVALID_ARGUMENTS
    );

    /* prepare new request to perform analysis */
    ReaiRequest request = {0};
    request.type        = REAI_REQUEST_TYPE_CREATE_ANALYSIS;

    request.create_analysis.ai_model     = ai_model;
    request.create_analysis.platform_opt = NULL;
    request.create_analysis.isa_opt      = NULL;
    request.create_analysis.file_opt     = REAI_FILE_OPTION_DEFAULT;
    request.create_analysis.dyn_exec     = false;
    request.create_analysis.tags         = NULL;
    request.create_analysis.tags_count   = 0;
    request.create_analysis.base_addr    = base_addr;
    request.create_analysis.functions    = fn_info_vec;
    request.create_analysis.bin_scope =
        is_private ? REAI_BINARY_SCOPE_PRIVATE : REAI_BINARY_SCOPE_PUBLIC;
    request.create_analysis.file_name     = file_name;
    request.create_analysis.cmdline_args  = cmdline_args;
    request.create_analysis.priority      = 0;
    request.create_analysis.sha_256_hash  = sha_256_hash;
    request.create_analysis.debug_hash    = NULL;
    request.create_analysis.size_in_bytes = size_in_bytes;

    if ((response = reai_request (reai, &request, response))) {
        switch (response->type) {
            case REAI_RESPONSE_TYPE_CREATE_ANALYSIS : {
                return response->create_analysis.binary_id;
            }
            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                REAI_LOG_ERROR ("reveng.ai request returned validation error.");
                return 0;
            }
            default : {
                RETURN_VALUE_IF_REACHED (0, "Unexpected response type.");
            }
        }
    } else {
        REAI_LOG_ERROR ("Failed to make reveng.ai request");
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
 * @return @c NULL otherwise.
 * */
ReaiFnInfoVec*
    reai_get_basic_function_info (Reai* reai, ReaiResponse* response, ReaiBinaryId bin_id) {
    RETURN_VALUE_IF (!reai || !response || !bin_id, NULL, ERR_INVALID_ARGUMENTS);

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
                REAI_LOG_ERROR ("reveng.ai request returned validation error.");
                return NULL;
            }
            default : {
                RETURN_VALUE_IF_REACHED (NULL, "Unexpected response type.");
            }
        }
    } else {
        REAI_LOG_ERROR ("Failed to make reveng.ai request");
        return NULL;
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
 * @return @c NULL otherwise.
 * */
ReaiAnalysisInfoVec* reai_get_recent_analyses (
    Reai*              reai,
    ReaiResponse*      response,
    ReaiAnalysisStatus status,
    ReaiBinaryScope    scope,
    Size               count
) {
    RETURN_VALUE_IF (!reai || !response, NULL, ERR_INVALID_ARGUMENTS);

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
                REAI_LOG_ERROR ("reveng.ai request returned validation error.");
                return NULL;
            }
            default : {
                RETURN_VALUE_IF_REACHED (NULL, "Unexpected response type.");
            }
        }
    } else {
        REAI_LOG_ERROR ("Failed to make reveng.ai request");
        return NULL;
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
 * @return @c true if response type is same as request type.
 *         Note that this does not mean all functions are renamed correctly.
 * @return @c false otherwise.
 * */
Bool reai_batch_renames_functions (
    Reai*          reai,
    ReaiResponse*  response,
    ReaiFnInfoVec* new_name_mapping
) {
    RETURN_VALUE_IF (!reai || !response || !new_name_mapping, false, ERR_INVALID_ARGUMENTS);

    /* prepare new request to rename functions in batch */
    ReaiRequest request = {0};
    request.type        = REAI_REQUEST_TYPE_BATCH_RENAMES_FUNCTIONS;

    request.batch_renames_functions.new_name_mapping = new_name_mapping;

    if ((response = reai_request (reai, &request, response))) {
        switch (response->type) {
            case REAI_RESPONSE_TYPE_BATCH_RENAMES_FUNCTIONS : {
                return true;
            }
            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                REAI_LOG_ERROR ("reveng.ai request returned validation error.");
                return false;
            }
            default : {
                RETURN_VALUE_IF_REACHED (false, "Unexpected response type.");
            }
        }
    } else {
        REAI_LOG_ERROR ("Failed to make reveng.ai request");
        return false;
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
 * @return @c true on success.
 * @return @c false otherwise.
 * */
Bool reai_rename_function (
    Reai*          reai,
    ReaiResponse*  response,
    ReaiFunctionId fn_id,
    CString        new_name
) {
    RETURN_VALUE_IF (!reai || !response || !fn_id || !new_name, false, ERR_INVALID_ARGUMENTS);

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
                REAI_LOG_ERROR ("reveng.ai request returned validation error.");
                return false;
            }
            default : {
                RETURN_VALUE_IF_REACHED (false, "Unexpected response type.");
            }
        }
    } else {
        REAI_LOG_ERROR ("Failed to make reveng.ai request");
        return false;
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
 * @param collection Can be @c NULL.
 *
 * @return @c ReaiAnnFnMatch on success.
 * @return @c NULL otherwise.
 * */
ReaiAnnFnMatchVec* reai_batch_binary_symbol_ann (
    Reai*         reai,
    ReaiResponse* response,
    ReaiBinaryId  bin_id,
    Size          max_results_per_function,
    Float64       max_distance,
    CStrVec*      collection,
    Bool          debug_mode
) {
    RETURN_VALUE_IF (!reai || !response || !bin_id, NULL, ERR_INVALID_ARGUMENTS);

    /* prepare new request to rename functions in batch */
    ReaiRequest request = {0};
    request.type        = REAI_REQUEST_TYPE_BATCH_BINARY_SYMBOL_ANN;

    request.batch_binary_symbol_ann.binary_id            = bin_id;
    request.batch_binary_symbol_ann.collection           = collection ? collection->items : NULL;
    request.batch_binary_symbol_ann.collection_count     = collection ? collection->count : 0;
    request.batch_binary_symbol_ann.results_per_function = max_results_per_function;
    request.batch_binary_symbol_ann.distance             = max_distance;
    request.batch_binary_symbol_ann.debug_mode           = debug_mode;

    if ((response = reai_request (reai, &request, response))) {
        switch (response->type) {
            case REAI_RESPONSE_TYPE_BATCH_BINARY_SYMBOL_ANN : {
                return response->batch_binary_symbol_ann.success ?
                           response->batch_binary_symbol_ann.function_matches :
                           NULL;
            }

            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                REAI_LOG_ERROR ("reveng.ai request returned validation error.");
                return NULL;
            }

            default : {
                RETURN_VALUE_IF_REACHED (NULL, "Unexpected response type.");
            }
        }
    } else {
        REAI_LOG_ERROR ("Failed to make reveng.ai request");
        return NULL;
    }
}

ReaiAnnFnMatchVec* reai_batch_function_symbol_ann (
    Reai*          reai,
    ReaiResponse*  response,
    ReaiFunctionId fn_id,
    U64Vec*        speculative_fn_ids,
    Size           max_results_per_function,
    Float64        max_distance,
    CStrVec*       collection,
    Bool           debug_mode
) {
    RETURN_VALUE_IF (!reai || !response || !fn_id, NULL, ERR_INVALID_ARGUMENTS);

    /* prepare new request to rename functions in batch */
    ReaiRequest request = {0};
    request.type        = REAI_REQUEST_TYPE_BATCH_FUNCTION_SYMBOL_ANN;

    request.batch_function_symbol_ann.function_ids      = ((Uint64[]) {fn_id});
    request.batch_function_symbol_ann.function_id_count = 1;
    request.batch_function_symbol_ann.speculative_function_ids =
        speculative_fn_ids ? speculative_fn_ids->items : NULL;
    request.batch_function_symbol_ann.speculative_function_id_count =
        speculative_fn_ids ? speculative_fn_ids->count : 0;
    request.batch_function_symbol_ann.collection           = collection ? collection->items : NULL;
    request.batch_function_symbol_ann.collection_count     = collection ? collection->count : 0;
    request.batch_function_symbol_ann.results_per_function = max_results_per_function;
    request.batch_function_symbol_ann.distance             = max_distance;
    request.batch_function_symbol_ann.debug_mode           = debug_mode;

    if ((response = reai_request (reai, &request, response))) {
        switch (response->type) {
            case REAI_RESPONSE_TYPE_BATCH_FUNCTION_SYMBOL_ANN : {
                return response->batch_function_symbol_ann.success ?
                           response->batch_function_symbol_ann.function_matches :
                           NULL;
            }

            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                REAI_LOG_ERROR ("reveng.ai request returned validation error.");
                return NULL;
            }

            default : {
                RETURN_VALUE_IF_REACHED (NULL, "Unexpected response type.");
            }
        }
    } else {
        REAI_LOG_ERROR ("Failed to make reveng.ai request");
        return NULL;
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
                REAI_LOG_ERROR ("reveng.ai request returned validation error.");
                return REAI_ANALYSIS_STATUS_INVALID;
            }
            default : {
                RETURN_VALUE_IF_REACHED (REAI_ANALYSIS_STATUS_INVALID, "Unexpected response type.");
            }
        }
    } else {
        REAI_LOG_ERROR ("Failed to make reveng.ai request");
        return REAI_ANALYSIS_STATUS_INVALID;
    }
}


CStrVec* reai_get_available_models (Reai* reai, ReaiResponse* response) {
    RETURN_VALUE_IF (!reai || !response, NULL, ERR_INVALID_ARGUMENTS);

    ReaiRequest request = {0};
    request.type        = REAI_REQUEST_TYPE_GET_MODELS;

    if ((response = reai_request (reai, &request, response))) {
        switch (response->type) {
            case REAI_RESPONSE_TYPE_GET_MODELS : {
                return response->get_models.models;
            }
            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                REAI_LOG_ERROR ("reveng.ai request returned validation error.");
                return NULL;
            }
            default : {
                RETURN_VALUE_IF_REACHED (NULL, "Unexpected response type.");
            }
        }
    } else {
        REAI_LOG_ERROR ("Failed to make reveng.ai request");
        return NULL;
    }
}
