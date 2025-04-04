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

#include "Reai/Api/Request.h"
#include "Reai/Api/Response.h"

struct Reai {
    CURL*              curl;
    struct curl_slist* headers;

    CString host;
    CString api_key;

    ReaiResponse* (*mock_handler) (
        Reai*         reai,
        ReaiRequest*  req,
        ReaiResponse* response,
        CString       endpoint_str,
        Uint32*       http_code
    );
};

HIDDEN Size reai_response_write_callback (void* ptr, Size size, Size nmemb, ReaiResponse* response);
HIDDEN ReaiResponse* reai_response_init_for_type (ReaiResponse* response, ReaiResponseType type);
HIDDEN CString       reai_request_to_json_cstr (ReaiRequest* request);
HIDDEN ReaiResponse* reai_response_reset (ReaiResponse* request);
Reai*                reai_deinit_curl_headers (Reai* reai);
Reai*                reai_init_curl_headers (Reai* reai, CString api_key);
Reai*                reai_init_conn (Reai* reai);
Reai*                reai_deinit_conn (Reai* reai);

#define ENDPOINT_URL_STR_SIZE 1024

/**
 * @b Create a new @c Reai object to handle connection with RevEngAI servers.
 * 
 * @param host Base address of reai api in use
 * @param api_key API key provided for using RevEngAI.
 * @param model AI model to use to perform analysis.
 *
 * @return @c Reai.
 * */
Reai* reai_create (CString host, CString api_key) {
    RETURN_VALUE_IF (!host || !api_key, NULL, ERR_INVALID_ARGUMENTS);

    Reai* reai = NEW (Reai);
    RETURN_VALUE_IF (!reai, NULL, ERR_INVALID_ARGUMENTS);

    GOTO_HANDLER_IF (
        !(reai->host = strdup (host)) || !(reai->api_key = strdup (api_key)),
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
 * If a mock handler is provided then the `reai_request` method
 * will automatically forward request to mock handler. No actual API calls
 * will be made.
 *
 * @param reai
 * @param mock_handler
 *
 * @return reai on success.
 * @return NULL otherwise.
 * */
Reai* reai_set_mock_handler (
    Reai* reai,
    ReaiResponse* (*mock_handler) (
        Reai*         reai,
        ReaiRequest*  req,
        ReaiResponse* response,
        CString       endpoint_str,
        Uint32*       http_code
    )
) {
    if (!reai || !mock_handler) {
        REAI_LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    reai->mock_handler = mock_handler;

    return reai;
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
    // curl_easy_setopt (reai->curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);
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
    }

    if (reai->api_key) {
        FREE (reai->api_key);
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
    Bool has_path_params = false;

    /* from data for uploading file */
    struct curl_mime*     mime     = NULL;
    struct curl_mimepart* mimepart = NULL;

#define SET_ENDPOINT(fmtstr, ...)                                                                  \
    do {                                                                                           \
        snprintf (endpoint_str, sizeof (endpoint_str), fmtstr, __VA_ARGS__);                       \
        curl_easy_setopt (reai->curl, CURLOPT_URL, endpoint_str);                                  \
        REAI_LOG_TRACE ("ENDPOINT : '%s'", endpoint_str);                                          \
    } while (0)

#define SET_PATH_QUERY_PARAM(fmtstr, ...)                                                          \
    do {                                                                                           \
        char tmpstr[ENDPOINT_URL_STR_SIZE] = {0};                                                  \
        snprintf (                                                                                 \
            tmpstr,                                                                                \
            sizeof (tmpstr),                                                                       \
            "%s%c" fmtstr,                                                                         \
            endpoint_str,                                                                          \
            has_path_params ? '&' : '?',                                                           \
            __VA_ARGS__                                                                            \
        );                                                                                         \
        strcpy (endpoint_str, tmpstr);                                                             \
        if (!has_path_params) {                                                                    \
            has_path_params = true;                                                                \
        };                                                                                         \
        curl_easy_setopt (reai->curl, CURLOPT_URL, endpoint_str);                                  \
        REAI_LOG_TRACE ("ADDED NEW PATH PARAM. NEW ENDPOINT : '%s'", endpoint_str);                \
    } while (0)

#define SET_METHOD(method)                                                                         \
    do {                                                                                           \
        curl_easy_setopt (reai->curl, CURLOPT_CUSTOMREQUEST, method);                              \
        REAI_LOG_TRACE ("METHOD : '%s'", method);                                                  \
    } while (0)

#define MAKE_REQUEST(expected_retcode, expected_response)                                          \
    do {                                                                                           \
        Uint32   http_code = 0;                                                                    \
        CURLcode retcode   = 0;                                                                    \
                                                                                                   \
        /* if mock handler is provided, then forward call to it. */                                \
        if (reai->mock_handler) {                                                                  \
            reai->mock_handler (reai, request, response, endpoint_str, &http_code);                \
            retcode = CURLE_OK;                                                                    \
        } else {                                                                                   \
            retcode = curl_easy_perform (reai->curl);                                              \
            curl_easy_getinfo (reai->curl, CURLINFO_RESPONSE_CODE, &http_code);                    \
        }                                                                                          \
                                                                                                   \
        if (retcode == CURLE_OK) {                                                                 \
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
                                                                                                   \
    } while (0)

#define MAKE_JSON_REQUEST(expected_retcode, expected_response)                                     \
    do {                                                                                           \
        /* add json header info */                                                                 \
        curl_slist_append (reai->headers, "Content-Type: application/json");                       \
                                                                                                   \
        /* convert request to json string */                                                       \
        CString json = reai_request_to_json_cstr (request);                                        \
        GOTO_HANDLER_IF (!json, REQUEST_FAILED, "Failed to convert request to JSON");              \
        REAI_LOG_TRACE ("REQUEST.JSON : '%s'", json);                                              \
                                                                                                   \
        /* set json data */                                                                        \
        /* HACK: it fails for some reason if we don't do this, seems to be a bug in curl */        \
        CString tmp_json_for_curl_setopt = json;                                                   \
        curl_easy_setopt (reai->curl, CURLOPT_POSTFIELDS, tmp_json_for_curl_setopt);               \
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
            SET_ENDPOINT ("%s/v1", reai->host);
            SET_METHOD ("GET");
            MAKE_REQUEST (200, REAI_RESPONSE_TYPE_HEALTH_CHECK);
            break;
        }

        /* GET : api.local/v1/authenticate */
        case REAI_REQUEST_TYPE_AUTH_CHECK : {
            reai_deinit_curl_headers (reai); // NOTE: Not the best solution here but it works...
            reai_init_curl_headers (reai, request->auth_check.api_key);
            SET_ENDPOINT ("%s/v1/authenticate", request->auth_check.host);
            SET_METHOD ("GET");
            MAKE_REQUEST (200, REAI_RESPONSE_TYPE_AUTH_CHECK);
            break;
        }

        /* POST : api.local/v1/upload */
        case REAI_REQUEST_TYPE_UPLOAD_FILE : {
            SET_ENDPOINT ("%s/v1/upload", reai->host);
            SET_METHOD ("POST");
            MAKE_UPLOAD_REQUEST (200, REAI_RESPONSE_TYPE_UPLOAD_FILE);
            break;
        }

        case REAI_REQUEST_TYPE_GET_MODELS : {
            SET_ENDPOINT ("%s/v1/models", reai->host);
            SET_METHOD ("GET");
            MAKE_REQUEST (200, REAI_RESPONSE_TYPE_GET_MODELS);
            break;
        }

        /* POST : api.local/v1/analyse */
        case REAI_REQUEST_TYPE_CREATE_ANALYSIS : {
            SET_ENDPOINT ("%s/v1/analyse/", reai->host);
            SET_METHOD ("POST");
            MAKE_JSON_REQUEST (201, REAI_RESPONSE_TYPE_CREATE_ANALYSIS);
            break;
        }

        /* DELETE : api.local/v1/analyse/binary_id */
        case REAI_REQUEST_TYPE_DELETE_ANALYSIS : {
            SET_ENDPOINT ("%s/v1/analyse/%llu", reai->host, request->delete_analysis.binary_id);
            SET_METHOD ("DELETE");
            MAKE_REQUEST (200, REAI_RESPONSE_TYPE_DELETE_ANALYSIS);
            break;
        }

        /* GET : api.local/v1/analyse/functions/binary_id */
        case REAI_REQUEST_TYPE_BASIC_FUNCTION_INFO : {
            SET_ENDPOINT (
                "%s/v1/analyse/functions/%llu",
                reai->host,
                request->basic_function_info.binary_id
            );
            SET_METHOD ("GET");
            MAKE_REQUEST (200, REAI_RESPONSE_TYPE_BASIC_FUNCTION_INFO);
            break;
        }

        /* GET : api.local/v1/analyse/recent */
        case REAI_REQUEST_TYPE_RECENT_ANALYSIS : {
            SET_ENDPOINT ("%s/v1/analyse/recent", reai->host);
            SET_METHOD ("GET");
            MAKE_JSON_REQUEST (200, REAI_RESPONSE_TYPE_RECENT_ANALYSIS);
            break;
        }

            /* GET : api.local/v1/analyse/status/{binary_id} */
        case REAI_REQUEST_TYPE_ANALYSIS_STATUS : {
            SET_ENDPOINT (
                "%s/v1/analyse/status/%llu",
                reai->host,
                request->analysis_status.binary_id
            );
            SET_METHOD ("GET");
            MAKE_REQUEST (200, REAI_RESPONSE_TYPE_ANALYSIS_STATUS);
            break;
        }

        case REAI_REQUEST_TYPE_SEARCH : {
            SET_ENDPOINT ("%s/v1/search", reai->host);
            SET_METHOD ("GET");
            MAKE_JSON_REQUEST (200, REAI_RESPONSE_TYPE_SEARCH);
            break;
        }

        case REAI_REQUEST_TYPE_BATCH_RENAMES_FUNCTIONS : {
            SET_ENDPOINT ("%s/v1/functions/batch/rename", reai->host);
            SET_METHOD ("POST");
            MAKE_JSON_REQUEST (200, REAI_RESPONSE_TYPE_BATCH_RENAMES_FUNCTIONS);
            break;
        }

        case REAI_REQUEST_TYPE_RENAME_FUNCTION : {
            SET_ENDPOINT (
                "%s/v1/functions/rename/%llu",
                reai->host,
                request->rename_function.function_id
            );
            SET_METHOD ("POST");
            MAKE_JSON_REQUEST (200, REAI_RESPONSE_TYPE_RENAME_FUNCTION);
            break;
        }

        case REAI_REQUEST_TYPE_BATCH_BINARY_SYMBOL_ANN : {
            SET_ENDPOINT (
                "%s/v1/ann/symbol/%llu",
                reai->host,
                request->batch_binary_symbol_ann.binary_id
            );
            SET_METHOD ("POST");
            MAKE_JSON_REQUEST (200, REAI_RESPONSE_TYPE_BATCH_BINARY_SYMBOL_ANN);
            break;
        }

        case REAI_REQUEST_TYPE_BATCH_FUNCTION_SYMBOL_ANN : {
            SET_ENDPOINT ("%s/v1/ann/symbol/batch", reai->host);
            SET_METHOD ("POST");
            MAKE_JSON_REQUEST (200, REAI_RESPONSE_TYPE_BATCH_FUNCTION_SYMBOL_ANN);
            break;
        }

        case REAI_REQUEST_TYPE_BEGIN_AI_DECOMPILATION : {
            SET_ENDPOINT ("%s/v2/ai-decompilation", reai->host);
            SET_METHOD ("POST");
            MAKE_JSON_REQUEST (201, REAI_RESPONSE_TYPE_BEGIN_AI_DECOMPILATION);
            break;
        }

        case REAI_REQUEST_TYPE_POLL_AI_DECOMPILATION : {
            SET_ENDPOINT (
                "%s/v2/ai-decompilation/%llu",
                reai->host,
                request->poll_ai_decompilation.function_id
            );
            SET_METHOD ("GET");
            MAKE_REQUEST (200, REAI_RESPONSE_TYPE_POLL_AI_DECOMPILATION);
            break;
        }

        case REAI_REQUEST_TYPE_ANALYSIS_ID_FROM_BINARY_ID : {
            SET_ENDPOINT ("%s/v2/analyses/lookup/%llu", reai->host, request->binary_id);
            SET_METHOD ("GET");
            MAKE_REQUEST (200, REAI_RESPONSE_TYPE_ANALYSIS_ID_FROM_BINARY_ID);
            break;
        }

        case REAI_REQUEST_TYPE_GET_SIMILAR_FUNCTIONS : {
            SET_ENDPOINT (
                "%s/v2/functions/%llu/similar-functions",
                reai->host,
                request->get_similar_functions.function_id
            );
            SET_METHOD ("GET");

            if (!request->get_similar_functions.function_id) {
                REAI_LOG_ERROR (
                    "Invalid function ID provided. Need a valid function ID to perform similar "
                    "function search."
                );
                goto REQUEST_FAILED;
            }

            SET_PATH_QUERY_PARAM ("function_id=%llu", request->get_similar_functions.function_id);

            if (request->get_similar_functions.limit) {
                SET_PATH_QUERY_PARAM ("limit=%u", request->get_similar_functions.limit);
            }

            SET_PATH_QUERY_PARAM (
                "distance=%f",
                request->get_similar_functions.distance < 0 ?
                    0 :
                request->get_similar_functions.distance > 1 ?
                    1 :
                    request->get_similar_functions.distance
            );

            if (request->get_similar_functions.collection_ids) {
                REAI_VEC_FOREACH (request->get_similar_functions.collection_ids, cid, {
                    SET_PATH_QUERY_PARAM ("collection_ids=%llu", *cid);
                });
            }

            SET_PATH_QUERY_PARAM (
                "debug=%s",
                request->get_similar_functions.debug ? "true" : "false"
            );

            if (request->get_similar_functions.binary_ids) {
                REAI_VEC_FOREACH (request->get_similar_functions.binary_ids, cid, {
                    SET_PATH_QUERY_PARAM ("binary_ids=%llu", *cid);
                });
            }

            MAKE_REQUEST (200, REAI_RESPONSE_TYPE_GET_SIMILAR_FUNCTIONS);
            break;
        }

        case REAI_REQUEST_TYPE_BASIC_COLLECTIONS_INFO : {
            SET_ENDPOINT ("%s/v2/collections", reai->host);
            SET_METHOD ("GET");

            if (request->basic_collections_info.search_term) {
                SET_PATH_QUERY_PARAM (
                    "search_term=%s",
                    request->basic_collections_info.search_term
                );
            }

            for (Size f = 0; (1 << f) < REAI_COLLECTION_BASIC_INFO_FILTER_MAX; f++) {
                ReaiCollectionBasicInfoFilterFlags flag = 1 << f;
                if ((request->basic_collections_info.filters & flag) == flag) {
                    SET_PATH_QUERY_PARAM (
                        "filters=%s",
                        ((CString[]) {NULL,
                                      "official_only",
                                      "user_only",
                                      "team_only",
                                      "public_only",
                                      "hide_empty_only",
                                      NULL})[f]
                    );
                }
            }

            if (request->basic_collections_info.limit) {
                SET_PATH_QUERY_PARAM (
                    "limit=%zu",
                    request->basic_collections_info.limit > 50 ?
                        50 :
                    request->basic_collections_info.limit < 5 ?
                        5 :
                        request->basic_collections_info.limit
                );
            }

            if (request->basic_collections_info.offset) {
                SET_PATH_QUERY_PARAM ("offset=%zu", request->basic_collections_info.offset);
            }

            if (request->basic_collections_info.order_by &&
                request->basic_collections_info.order_by <
                    REAI_COLLECTION_BASIC_INFO_ORDER_BY_MAX) {
                SET_PATH_QUERY_PARAM (
                    "order_by=%s",
                    ((CString[]
                    ) {NULL, "created", "collection", "model", "owner", "collection_size", NULL}
                    )[request->basic_collections_info.order_by]
                );
            }

            SET_PATH_QUERY_PARAM (
                "order=%s",
                request->basic_collections_info.order_in ==
                        REAI_COLLECTION_BASIC_INFO_ORDER_IN_DESC ?
                    "DESC" :
                    "ASC"
            );

            MAKE_REQUEST (200, REAI_RESPONSE_TYPE_BASIC_COLLECTIONS_INFO);
            break;
        }

        case REAI_REQUEST_TYPE_COLLECTION_SEARCH : {
            SET_ENDPOINT ("%s/v2/search/collections", reai->host);
            SET_METHOD ("GET");

            if (request->collection_search.page) {
                SET_PATH_QUERY_PARAM ("page=%zu", request->collection_search.page);
            }

            if (request->collection_search.page_size) {
                SET_PATH_QUERY_PARAM ("page_size=%zu", request->collection_search.page_size);
            }

            if (request->collection_search.partial_collection_name) {
                SET_PATH_QUERY_PARAM (
                    "partial_collection_name=%s",
                    request->collection_search.partial_collection_name
                );
            }

            if (request->collection_search.partial_binary_name) {
                SET_PATH_QUERY_PARAM (
                    "partial_binary_name=%s",
                    request->collection_search.partial_binary_name
                );
            }

            if (request->collection_search.partial_binary_sha256) {
                SET_PATH_QUERY_PARAM (
                    "partial_binary_sha256=%s",
                    request->collection_search.partial_binary_sha256
                );
            }

            if (request->collection_search.model_name) {
                SET_PATH_QUERY_PARAM ("model_name=%s", request->collection_search.model_name);
            }

            if (request->collection_search.tags) {
                REAI_VEC_FOREACH (request->collection_search.tags, tag, {
                    SET_PATH_QUERY_PARAM ("tags=%s", *tag);
                });
            }

            MAKE_REQUEST (200, REAI_RESPONSE_TYPE_COLLECTION_SEARCH);
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
#undef SET_PATH_QUERY_PARAM

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
                REAI_LOG_ERROR ("Unexpected respose type.");
                return false;
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
                REAI_LOG_ERROR ("Unexpected respose type.");
                return NULL;
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
                REAI_LOG_ERROR ("Unexpected respose type.");
                return 0;
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
                REAI_LOG_ERROR ("Unexpected respose type.");
                return NULL;
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
                REAI_LOG_ERROR ("Unexpected respose type.");
                return NULL;
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
                REAI_LOG_ERROR ("Unexpected respose type.");
                return false;
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
                REAI_LOG_ERROR ("Unexpected respose type.");
                return false;
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
    CStrVec*      collections,
    Bool          debug_mode
) {
    RETURN_VALUE_IF (!reai || !response || !bin_id, NULL, ERR_INVALID_ARGUMENTS);

    /* prepare new request to rename functions in batch */
    ReaiRequest request = {0};
    request.type        = REAI_REQUEST_TYPE_BATCH_BINARY_SYMBOL_ANN;

    request.batch_binary_symbol_ann.binary_id            = bin_id;
    request.batch_binary_symbol_ann.collections          = collections;
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
                REAI_LOG_ERROR ("Unexpected respose type.");
                return NULL;
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
    CStrVec*       collections,
    Bool           debug_mode
) {
    RETURN_VALUE_IF (!reai || !response || !fn_id, NULL, ERR_INVALID_ARGUMENTS);

    /* prepare new request to rename functions in batch */
    ReaiRequest request = {0};
    request.type        = REAI_REQUEST_TYPE_BATCH_FUNCTION_SYMBOL_ANN;

    U64Vec fn_ids = {.items = ((Uint64[]) {fn_id}), .count = 1, .capacity = 1};

    request.batch_function_symbol_ann.function_ids             = &fn_ids;
    request.batch_function_symbol_ann.speculative_function_ids = speculative_fn_ids;
    request.batch_function_symbol_ann.collections              = collections;
    request.batch_function_symbol_ann.results_per_function     = max_results_per_function;
    request.batch_function_symbol_ann.distance                 = max_distance;
    request.batch_function_symbol_ann.debug_mode               = debug_mode;

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
                REAI_LOG_ERROR ("Unexpected respose type.");
                return NULL;
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
                REAI_LOG_ERROR ("Unexpected respose type.");
                return REAI_ANALYSIS_STATUS_INVALID;
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
                REAI_LOG_ERROR ("Unexpected respose type.");
                return NULL;
            }
        }
    } else {
        REAI_LOG_ERROR ("Failed to make reveng.ai request");
        return NULL;
    }
}

Reai* reai_begin_ai_decompilation (Reai* reai, ReaiResponse* response, ReaiFunctionId fn_id) {
    RETURN_VALUE_IF (!reai || !response || !fn_id, NULL, ERR_INVALID_ARGUMENTS);

    ReaiRequest request                        = {0};
    request.type                               = REAI_REQUEST_TYPE_BEGIN_AI_DECOMPILATION;
    request.begin_ai_decompilation.function_id = fn_id;

    if ((response = reai_request (reai, &request, response))) {
        switch (response->type) {
            case REAI_RESPONSE_TYPE_BEGIN_AI_DECOMPILATION : {
                return response->begin_ai_decompilation.status ? reai : NULL;
            }
            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                REAI_LOG_ERROR ("reveng.ai request returned validation error.");
                return NULL;
            }
            default : {
                REAI_LOG_ERROR ("Unexpected respose type.");
                return NULL;
            }
        }
    } else {
        REAI_LOG_ERROR ("Failed to make reveng.ai request");
        return NULL;
    }
}

ReaiAiDecompilationStatus
    reai_poll_ai_decompilation (Reai* reai, ReaiResponse* response, ReaiFunctionId fn_id) {
    RETURN_VALUE_IF (
        !reai || !response || !fn_id,
        REAI_AI_DECOMPILATION_STATUS_ERROR,
        ERR_INVALID_ARGUMENTS
    );

    ReaiRequest request                       = {0};
    request.type                              = REAI_REQUEST_TYPE_POLL_AI_DECOMPILATION;
    request.poll_ai_decompilation.function_id = fn_id;

    if ((response = reai_request (reai, &request, response))) {
        switch (response->type) {
            case REAI_RESPONSE_TYPE_POLL_AI_DECOMPILATION : {
                REAI_LOG_TRACE (
                    "AI Decompilation Status = \"%s\"",
                    reai_ai_decompilation_status_to_cstr (
                        response->poll_ai_decompilation.data.status
                    )
                );
                return response->poll_ai_decompilation.data.status;
            }
            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                REAI_LOG_ERROR ("reveng.ai request returned validation error.");
                return REAI_AI_DECOMPILATION_STATUS_ERROR;
            }
            default : {
                REAI_LOG_ERROR ("Unexpected respose type.");
                return REAI_AI_DECOMPILATION_STATUS_ERROR;
            }
        }
    } else {
        REAI_LOG_ERROR ("Failed to make reveng.ai request");
        return REAI_AI_DECOMPILATION_STATUS_ERROR;
    }
}

ReaiSimilarFnVec* reai_get_similar_functions (
    Reai*          reai,
    ReaiResponse*  response,
    ReaiFunctionId fn_id,
    Uint64         limit,
    Float64        distance,
    U64Vec*        collection_ids,
    Bool           debug,
    U64Vec*        binary_ids
) {
    RETURN_VALUE_IF (!reai || !response || !fn_id, NULL, ERR_INVALID_ARGUMENTS);

    ReaiRequest request                          = {0};
    request.type                                 = REAI_REQUEST_TYPE_GET_SIMILAR_FUNCTIONS;
    request.get_similar_functions.function_id    = fn_id;
    request.get_similar_functions.limit          = limit;
    request.get_similar_functions.distance       = distance;
    request.get_similar_functions.collection_ids = collection_ids;
    request.get_similar_functions.debug          = debug;
    request.get_similar_functions.binary_ids     = binary_ids;

    if ((response = reai_request (reai, &request, response))) {
        switch (response->type) {
            case REAI_RESPONSE_TYPE_GET_SIMILAR_FUNCTIONS : {
                REAI_LOG_TRACE (
                    "Found %llu similar functions for function id %llu",
                    response->get_similar_functions.data->count,
                    fn_id
                );
                return response->get_similar_functions.data;
            }
            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                REAI_LOG_ERROR (
                    "Failed to find similar functions for function id %llu. Validation Error.",
                    fn_id
                );
                return NULL;
            }
            default : {
                REAI_LOG_ERROR ("Unexpected respose type.");
                return NULL;
            }
        }
    } else {
        REAI_LOG_ERROR ("Failed to make reveng.ai request");
        return NULL;
    }
}

ReaiCollectionBasicInfoVec* reai_get_basic_collection_info (
    Reai*                              reai,
    ReaiResponse*                      response,
    CString                            search_term,
    ReaiCollectionBasicInfoFilterFlags filters,
    Uint64                             limit,
    Uint64                             offset,
    ReaiCollectionBasicInfoOrderBy     order_by,
    ReaiCollectionBasicInfoOrderIn     order_in
) {
    RETURN_VALUE_IF (!reai || !response || !search_term, NULL, ERR_INVALID_ARGUMENTS);

    ReaiRequest request                        = {0};
    request.type                               = REAI_REQUEST_TYPE_BASIC_COLLECTIONS_INFO;
    request.basic_collections_info.search_term = search_term;
    request.basic_collections_info.filters     = filters;
    request.basic_collections_info.limit       = limit;
    request.basic_collections_info.offset      = offset;
    request.basic_collections_info.order_by    = order_by;
    request.basic_collections_info.order_in    = order_in;

    if ((response = reai_request (reai, &request, response))) {
        switch (response->type) {
            case REAI_RESPONSE_TYPE_BASIC_COLLECTIONS_INFO : {
                REAI_LOG_TRACE (
                    "Found %llu collections for search term \"%s\" with provided filters",
                    response->basic_collection_info.data.results->count,
                    search_term
                );
                return response->basic_collection_info.data.results;
            }
            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                REAI_LOG_ERROR (
                    "Failed to find collections for search term \"%s\" with provided filters. "
                    "Validation Error.",
                    search_term
                );
                return NULL;
            }
            default : {
                REAI_LOG_ERROR ("Unexpected respose type.");
                return NULL;
            }
        }
    } else {
        REAI_LOG_ERROR ("Failed to make reveng.ai request");
        return NULL;
    }
}

ReaiCollectionSearchResultVec* reai_collection_search (
    Reai*         reai,
    ReaiResponse* response,
    CString       partial_collection_name,
    CString       partial_binary_name,
    CString       partial_binary_sha256,
    CStrVec*      tags,
    CString       model_name
) {
    RETURN_VALUE_IF (!reai || !response, NULL, ERR_INVALID_ARGUMENTS);

    ReaiRequest request                               = {0};
    request.type                                      = REAI_REQUEST_TYPE_COLLECTION_SEARCH;
    request.collection_search.partial_collection_name = partial_collection_name;
    request.collection_search.partial_binary_name     = partial_binary_name;
    request.collection_search.partial_binary_sha256   = partial_binary_sha256;
    request.collection_search.tags                    = tags;
    request.collection_search.model_name              = model_name;

    if ((response = reai_request (reai, &request, response))) {
        switch (response->type) {
            case REAI_RESPONSE_TYPE_COLLECTION_SEARCH : {
                REAI_LOG_TRACE (
                    "Found %llu collections",
                    response->collection_search.data.results->count
                );
                return response->collection_search.data.results;
            }
            case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
                REAI_LOG_ERROR ("Failed to find collections. Validation Error.");
                return NULL;
            }
            default : {
                REAI_LOG_ERROR ("Unexpected respose type.");
                return NULL;
            }
        }
    } else {
        REAI_LOG_ERROR ("Failed to make reveng.ai request");
        return NULL;
    }
}
