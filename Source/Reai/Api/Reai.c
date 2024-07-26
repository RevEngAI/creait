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

/* libc */
#include <memory.h>

#include "Reai/Api/Request.h"
#include "Reai/Api/Response.h"

struct Reai {
    CURL*              curl;
    struct curl_slist* headers;

    CString host;
    CString api_key;
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

    GOTO_HANDLER_IF (!reai_init_conn (reai), CREATE_FAILED, "Failed to create connection.\n");

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
    RETURN_VALUE_IF (!reai->curl, Null, "Failed to easy init curl\n");

    curl_easy_setopt (reai->curl, CURLOPT_WRITEFUNCTION, reai_response_write_callback);
    curl_easy_setopt (reai->curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt (reai->curl, CURLOPT_USERAGENT, "creait");
    curl_easy_setopt (reai->curl, CURLOPT_MAXREDIRS, 50);
    curl_easy_setopt (reai->curl, CURLOPT_SERVER_RESPONSE_TIMEOUT, 5);
    curl_easy_setopt (reai->curl, CURLOPT_TIMEOUT, 5);
    curl_easy_setopt (reai->curl, CURLOPT_CONNECTTIMEOUT, 5);

    /* curl_easy_setopt (reai->curl, CURLOPT_VERBOSE, 1); */

    Char auth_hdr_str[80];
    snprintf (auth_hdr_str, sizeof (auth_hdr_str), "Authorization: %s", reai->api_key);
    reai->headers = curl_slist_append (reai->headers, auth_hdr_str);

    /* reai->headers = curl_slist_append (reai->headers, "Expect:"); */
    RETURN_VALUE_IF (!reai->headers, Null, "Failed to prepare initial headers\n");

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
    } while (0)

#define SET_METHOD(method)                                                                         \
    do {                                                                                           \
        curl_easy_setopt (reai->curl, CURLOPT_CUSTOMREQUEST, method);                              \
    } while (0)

#define MAKE_REQUEST(expected_retcode, expected_response)                                          \
    do {                                                                                           \
        CURLcode retcode = curl_easy_perform (reai->curl);                                         \
        if (retcode == CURLE_OK) {                                                                 \
            Uint32 http_code = 0;                                                                  \
            curl_easy_getinfo (reai->curl, CURLINFO_RESPONSE_CODE, &http_code);                    \
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
        GOTO_HANDLER_IF (!json, REQUEST_FAILED, "Failed to convert request to JSON\n");            \
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
        GOTO_HANDLER_IF (!mime, REQUEST_FAILED, "Failed to create mime data\n");                   \
                                                                                                   \
        /* create mimepart for multipart data */                                                   \
        mimepart = curl_mime_addpart (mime);                                                       \
        GOTO_HANDLER_IF (!mimepart, REQUEST_FAILED, "Failed to add mime part to mime data\n");     \
                                                                                                   \
        /* set part info */                                                                        \
        curl_mime_name (mimepart, "file");                                                         \
        curl_mime_filedata (mimepart, request->upload_file.file_path);                             \
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

        default :
            PRINT_ERR ("Invalid request.\n");
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
                return response->upload_file.sha_256_hash;
            }
            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                return Null;
            }
            default : {
                RETURN_VALUE_IF_REACHED (Null, "Unexpected type.\n");
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
BinaryId reai_create_analysis (
    Reai*          reai,
    ReaiResponse*  response,
    ReaiModel      model,
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

    request.create_analysis.model        = REAI_MODEL_X86_LINUX;
    request.create_analysis.platform_opt = Null;
    request.create_analysis.isa_opt      = Null;
    request.create_analysis.file_opt     = REAI_FILE_OPTION_DEFAULT;
    request.create_analysis.dyn_exec     = False;
    request.create_analysis.tags         = Null;
    request.create_analysis.tags_count   = 0;
    request.create_analysis.functions    = fn_info_vec;
    request.create_analysis.bin_scope =
        is_private ? REAI_BINARY_SCOPE_PRIVATE : REAI_BINARY_SCOPE_PUBLIC;
    request.create_analysis.file_name     = file_name;
    request.create_analysis.cmdline_args  = Null;
    request.create_analysis.priority      = 0;
    request.create_analysis.sha_256_hash  = sha_256_hash;
    request.create_analysis.debug_hash    = cmdline_args;
    request.create_analysis.size_in_bytes = size_in_bytes;

    if ((response = reai_request (reai, &request, response))) {
        switch (response->type) {
            case REAI_RESPONSE_TYPE_CREATE_ANALYSIS : {
                return response->create_analysis.binary_id;
            }
            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                return 0;
            }
            default : {
                RETURN_VALUE_IF_REACHED (0, "Unexpected response type.\n");
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
ReaiFnInfoVec* reai_get_basic_function_info (Reai* reai, ReaiResponse* response, BinaryId bin_id) {
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
                RETURN_VALUE_IF_REACHED (Null, "Unexpected response type.\n");
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
                RETURN_VALUE_IF_REACHED (Null, "Unexpected response type.\n");
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
                RETURN_VALUE_IF_REACHED (False, "Unexpected response type.\n");
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
Bool reai_rename_function (Reai* reai, ReaiResponse* response, FunctionId fn_id, CString new_name) {
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
                RETURN_VALUE_IF_REACHED (False, "Unexpected response type.\n");
            }
        }
    } else {
        return False;
    }
}
