/**
 * @file Response.c
 * @date 10th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

/* cjson */
#include <cJSON.h>

/* reai */
#include <Reai/Api/Response.h>

/* libc */
#include <memory.h>

#include "Reai/AnnFnMatch.h"
#include "Reai/Util/CStrVec.h"

PRIVATE Bool         json_response_get_bool (cJSON* json, CString name);
PRIVATE Uint64*      json_response_get_u64 (cJSON* json, CString name, Uint64* num);
PRIVATE Float64*     json_response_get_f64 (cJSON* json, CString name, Float64* num);
PRIVATE CString      json_response_get_string (cJSON* json, CString name);
PRIVATE CStrVec*     json_response_get_string_arr (cJSON* json, CString name);
HIDDEN ReaiResponse* reai_response_reset (ReaiResponse* response);

/**************************************************************************************************/
/***************************************** PUBLIC METHODS *****************************************/
/**************************************************************************************************/

/**
 * @b De-initialize given ReaiResponse structure. 
 *
 * @param reai_response
 *
 * @return @c reai_response on success.
 * @return @c NULL otherwise.
 * */
PUBLIC ReaiResponse* reai_response_init (ReaiResponse* response) {
    RETURN_VALUE_IF (!response, NULL, ERR_INVALID_ARGUMENTS);

    /* invalidate all fields */
    memset (response, 0, sizeof (ReaiResponse));

    /* NOTE: make sure to keep it to a large value from the beginning itself to avoid many reallocations */
#define RESPONSE_RAW_BUF_INITIAL_CAP 4000

    /* allocate a significantly large memory block for getting raw responses */
    response->raw.data = ALLOCATE (Char, RESPONSE_RAW_BUF_INITIAL_CAP);
    RETURN_VALUE_IF (!response->raw.data, NULL, ERR_OUT_OF_MEMORY);
    response->raw.capacity = RESPONSE_RAW_BUF_INITIAL_CAP;

    return response;
}

/**
 * @b De-initialize given ReaiResponse structure. 
 *
 * @param reai_response
 *
 * @return @c reai_response on success.
 * @return @c NULL otherwise.
 * */
PUBLIC ReaiResponse* reai_response_deinit (ReaiResponse* response) {
    RETURN_VALUE_IF (!response, NULL, ERR_INVALID_ARGUMENTS);

    reai_response_reset (response);

    if (response->raw.data) {
        memset (response->raw.data, 0, response->raw.capacity);
        FREE (response->raw.data);
    }

    if (response->validation_error.locations) {
        reai_cstr_vec_destroy (response->validation_error.locations);
        response->validation_error.locations = NULL;
    }

    memset (response, 0, sizeof (ReaiResponse));

    return response;
}

#define GET_JSON_BOOL(json, name, var) var = json_response_get_bool (json, name);

#define GET_JSON_U64(json, name, var)                                                              \
    {                                                                                              \
        Uint64 num = 0;                                                                            \
        GOTO_HANDLER_IF (                                                                          \
            !(json_response_get_u64 (json, name, &num)),                                           \
            INIT_FAILED,                                                                           \
            "Failed to get number '%s' from response.",                                            \
            name                                                                                   \
        );                                                                                         \
                                                                                                   \
        var = num;                                                                                 \
    }

#define GET_JSON_F64(json, name, var)                                                              \
    {                                                                                              \
        Float64 num = 0;                                                                           \
        GOTO_HANDLER_IF (                                                                          \
            !(json_response_get_f64 (json, name, &num)),                                           \
            INIT_FAILED,                                                                           \
            "Failed to get number '%s' from response.",                                            \
            name                                                                                   \
        );                                                                                         \
                                                                                                   \
        var = num;                                                                                 \
    }

/* retrieved string must be freed after use */
#define GET_JSON_STRING(json, name, var)                                                           \
    {                                                                                              \
        CString str = NULL;                                                                        \
        GOTO_HANDLER_IF (                                                                          \
            !(str = json_response_get_string (json, name)),                                        \
            INIT_FAILED,                                                                           \
            "Failed to get string '%s' from response.",                                            \
            name                                                                                   \
        );                                                                                         \
                                                                                                   \
        var = str;                                                                                 \
    }

/* retrieved string must be freed after use */
#define GET_JSON_STRING_ARR(json, name, arr)                                                       \
    {                                                                                              \
        GOTO_HANDLER_IF (                                                                          \
            !(arr = json_response_get_string_arr (json, name)),                                    \
            INIT_FAILED,                                                                           \
            "Failed to get '%s' string array in response.",                                        \
            name                                                                                   \
        );                                                                                         \
    }

/* retrieved string must be freed after use */
#define GET_JSON_STRING_ON_SUCCESS(json, name, var, success)                                       \
    if (success) {                                                                                 \
        GET_JSON_STRING (json, name, var);                                                         \
    } else {                                                                                       \
        var = NULL;                                                                                \
    }

#define GET_JSON_U64_ON_SUCCESS(json, name, var, success)                                          \
    if (success) {                                                                                 \
        GET_JSON_U64 (json, name, var);                                                            \
    } else {                                                                                       \
        var = 0;                                                                                   \
    }

#define GET_JSON_F64_ON_SUCCESS(json, name, var, success)                                          \
    if (success) {                                                                                 \
        GET_JSON_F64 (json, name, var);                                                            \
    } else {                                                                                       \
        var = 0;                                                                                   \
    }

#define GET_JSON_FN_INFO(json, fn)                                                                 \
    do {                                                                                           \
        GET_JSON_U64 (json, "function_id", fn.id);                                                 \
        GET_JSON_STRING (json, "function_name", fn.name);                                          \
        GET_JSON_U64 (json, "function_size", fn.size);                                             \
        GET_JSON_U64 (json, "function_vaddr", fn.vaddr);                                           \
    } while (0)

#define GET_JSON_ANALYSIS_INFO(json, ainfo)                                                        \
    do {                                                                                           \
        GET_JSON_U64 (json, "binary_id", ainfo.binary_id);                                         \
        GET_JSON_STRING (json, "binary_name", ainfo.binary_name);                                  \
        GET_JSON_STRING (json, "creation", ainfo.creation);                                        \
        GET_JSON_U64 (json, "model_id", ainfo.model_id);                                           \
        GET_JSON_STRING (json, "model_name", ainfo.model_name);                                    \
        GET_JSON_STRING (json, "sha_256_hash", ainfo.sha_256_hash);                                \
                                                                                                   \
        CString            status  = NULL;                                                         \
        ReaiAnalysisStatus estatus = 0;                                                            \
        GET_JSON_STRING (json, "status", status);                                                  \
        if (!(estatus = reai_analysis_status_from_cstr (status))) {                                \
            PRINT_ERR ("Failed to convert analysis status to enum");                               \
            if (status) {                                                                          \
                FREE (status);                                                                     \
            }                                                                                      \
            goto INIT_FAILED;                                                                      \
        }                                                                                          \
        if (status) {                                                                              \
            FREE (status);                                                                         \
        }                                                                                          \
        ainfo.status = estatus;                                                                    \
    } while (0)

#define GET_JSON_QUERY_RESULT(json, qres)                                                          \
    do {                                                                                           \
        GET_JSON_U64 (json, "binary_id", qres.binary_id);                                          \
        GET_JSON_STRING (json, "binary_name", qres.binary_name);                                   \
        GET_JSON_STRING_ARR (json, "collections", qres.collections);                               \
        GET_JSON_STRING (json, "creation", qres.creation);                                         \
        GET_JSON_U64 (json, "model_id", qres.model_id);                                            \
        GET_JSON_STRING (json, "model_name", qres.model_name);                                     \
        GET_JSON_STRING (json, "sha_256_hash", qres.sha_256_hash);                                 \
        GET_JSON_STRING_ARR (json, "tags", qres.tags);                                             \
                                                                                                   \
        CString            status  = NULL;                                                         \
        ReaiAnalysisStatus estatus = 0;                                                            \
        GET_JSON_STRING (json, "status", status);                                                  \
        if (!(qres.status = reai_analysis_status_from_cstr (status))) {                            \
            PRINT_ERR ("Failed to convert analysis status to enum");                               \
            if (status) {                                                                          \
                FREE (status);                                                                     \
            }                                                                                      \
            goto INIT_FAILED;                                                                      \
        }                                                                                          \
        if (status) {                                                                              \
            FREE (status);                                                                         \
        }                                                                                          \
        qres.status = estatus;                                                                     \
    } while (0)

#define GET_JSON_ANN_FN_MATCH(match, fn_match)                                                     \
    do {                                                                                           \
        GET_JSON_F64 (match, "confidence", fn_match.confidence);                                   \
        GET_JSON_U64 (match, "nearest_neighbor_binary_id", fn_match.nn_binary_id);                 \
        GET_JSON_STRING (match, "nearest_neighbor_binary_name", fn_match.nn_binary_name);          \
        GET_JSON_BOOL (match, "nearest_neighbor_debug", fn_match.nn_debug);                        \
        GET_JSON_STRING (match, "nearest_neighbor_function_name", fn_match.nn_function_name);      \
        GET_JSON_U64 (match, "nearest_neighbor_id", fn_match.nn_function_id);                      \
        GET_JSON_STRING (match, "nearest_neighbor_sha_256_hash", fn_match.nn_sha_256_hash);        \
        GET_JSON_U64 (match, "origin_function_id", fn_match.origin_function_id);                   \
    } while (0)

#define GET_JSON_CUSTOM_ARR(json, type_name, type_infix, reader, vec)                              \
    do {                                                                                           \
        vec = reai_##type_infix##_vec_create();                                                    \
        GOTO_HANDLER_IF (!vec, INIT_FAILED, "Failed to create " #type_name " vector.");            \
                                                                                                   \
        if (cJSON_IsObject (json)) {                                                               \
            type_name item = {0};                                                                  \
            reader (json, item);                                                                   \
                                                                                                   \
            if (!reai_##type_infix##_vec_append (vec, &item)) {                                    \
                PRINT_ERR ("Failed to insert " #type_name " object into vector.");                 \
                reai_##type_infix##_vec_destroy (vec);                                             \
                goto INIT_FAILED;                                                                  \
            }                                                                                      \
        } else {                                                                                   \
            cJSON* arr_item = NULL;                                                                \
            cJSON_ArrayForEach (arr_item, json) {                                                  \
                type_name item = {0};                                                              \
                reader (arr_item, item);                                                           \
                                                                                                   \
                if (!reai_##type_infix##_vec_append (vec, &item)) {                                \
                    PRINT_ERR ("Failed to insert " #type_name " object into vector.");             \
                    reai_##type_infix##_vec_destroy (vec);                                         \
                    goto INIT_FAILED;                                                              \
                }                                                                                  \
            }                                                                                      \
        }                                                                                          \
    } while (0)

/**
 * @b Initialize given ReaiResponse structure for given response type by
 * reading contents from given response buffer.
 *
 * Calling this with different @c ReaiResponseType but same raw data
 * will cause issues.
 *
 * @param reai_response
 * @param response_type
 * @param buf
 *
 * @return @c reai_response on success.
 * @return @c NULL otherwise.
 * */
HIDDEN ReaiResponse* reai_response_init_for_type (ReaiResponse* response, ReaiResponseType type) {
    RETURN_VALUE_IF (!response, NULL, ERR_INVALID_ARGUMENTS);

    /* convert from string to json */
    cJSON* json = cJSON_ParseWithLength (response->raw.data, response->raw.length);
    GOTO_HANDLER_IF (!json, INIT_FAILED, "Failed to parse given response as JSON data");

    /* each response type has a different structure */
    switch (type) {
        case REAI_RESPONSE_TYPE_HEALTH_CHECK : {
            response->type = REAI_RESPONSE_TYPE_HEALTH_CHECK;

            GET_JSON_BOOL (json, "success", response->auth_check.success);

            CString msg_keyname = response->auth_check.success ? "message" : "error";
            GET_JSON_STRING (json, msg_keyname, response->auth_check.message);

            break;
        }

        case REAI_RESPONSE_TYPE_AUTH_CHECK : {
            response->type = REAI_RESPONSE_TYPE_AUTH_CHECK;

            GET_JSON_BOOL (json, "success", response->auth_check.success);

            CString msg_keyname = response->auth_check.success ? "message" : "error";
            GET_JSON_STRING (json, msg_keyname, response->auth_check.message);

            break;
        }

        case REAI_RESPONSE_TYPE_DELETE_ANALYSIS : {
            response->type = REAI_RESPONSE_TYPE_AUTH_CHECK;

            GET_JSON_BOOL (json, "success", response->delete_analysis.success);

            CString msg_keyname = response->delete_analysis.success ? "msg" : "error";
            GET_JSON_STRING (json, msg_keyname, response->delete_analysis.message);

            break;
        }

        case REAI_RESPONSE_TYPE_UPLOAD_FILE : {
            response->type = REAI_RESPONSE_TYPE_UPLOAD_FILE;

            GET_JSON_BOOL (json, "success", response->create_analysis.success);

            CString msg_keyname = response->auth_check.success ? "message" : "error";
            GET_JSON_STRING (json, msg_keyname, response->auth_check.message);

            /* sha256 hash provided only on success */
            GET_JSON_STRING_ON_SUCCESS (
                json,
                "sha_256_hash",
                response->upload_file.sha_256_hash,
                response->upload_file.success
            );

            break;
        }

        case REAI_RESPONSE_TYPE_CREATE_ANALYSIS : {
            response->type = REAI_RESPONSE_TYPE_CREATE_ANALYSIS;

            GET_JSON_BOOL (json, "success", response->create_analysis.success);

            /* binary id provided only on success */
            GET_JSON_U64_ON_SUCCESS (
                json,
                "binary_id",
                response->create_analysis.binary_id,
                response->create_analysis.success
            );

            break;
        }

        case REAI_RESPONSE_TYPE_BASIC_FUNCTION_INFO : {
            response->type = REAI_RESPONSE_TYPE_BASIC_FUNCTION_INFO;

            GET_JSON_BOOL (json, "success", response->basic_function_info.success);
            if (!response->basic_function_info.success) {
                break;
            }

            cJSON* functions = cJSON_GetObjectItem (json, "functions");
            GOTO_HANDLER_IF (
                !functions || !cJSON_IsArray (functions),
                INIT_FAILED,
                "Given JSON response does not contain 'functions' array"
            );

            GET_JSON_CUSTOM_ARR (
                functions,
                ReaiFnInfo,
                fn_info,
                GET_JSON_FN_INFO,
                response->basic_function_info.fn_infos
            );

            break;
        }

        case REAI_RESPONSE_TYPE_RECENT_ANALYSIS : {
            response->type = REAI_RESPONSE_TYPE_RECENT_ANALYSIS;

            GET_JSON_BOOL (json, "success", response->recent_analysis.success);
            if (!response->recent_analysis.success) {
                break;
            }

            cJSON* analysis = cJSON_GetObjectItem (json, "analysis");
            GOTO_HANDLER_IF (
                !cJSON_IsObject (analysis) && !cJSON_IsArray (analysis),
                INIT_FAILED,
                "Expected array or object for analysis."
            );

            GET_JSON_CUSTOM_ARR (
                analysis,
                ReaiAnalysisInfo,
                analysis_info,
                GET_JSON_ANALYSIS_INFO,
                response->recent_analysis.analysis_infos
            );

            break;
        }


        case REAI_RESPONSE_TYPE_ANALYSIS_STATUS : {
            response->type = REAI_RESPONSE_TYPE_ANALYSIS_STATUS;

            GET_JSON_BOOL (json, "success", response->analysis_status.success);

            CString status = NULL;
            GET_JSON_STRING_ON_SUCCESS (json, "status", status, response->analysis_status.success);

            if (!(response->analysis_status.status = reai_analysis_status_from_cstr (status))) {
                PRINT_ERR ("Failed to convert analysis status from string to enum.");
                if (status) {
                    FREE (status);
                }
                goto INIT_FAILED;
            }

            break;
        }


        case REAI_RESPONSE_TYPE_SEARCH : {
            response->type = REAI_RESPONSE_TYPE_SEARCH;

            GET_JSON_BOOL (json, "success", response->search.success);
            if (!response->search.success) {
                break;
            }

            cJSON* query_results = cJSON_GetObjectItem (json, "query_results");
            GOTO_HANDLER_IF (
                !cJSON_IsObject (query_results) && !cJSON_IsArray (query_results),
                INIT_FAILED,
                "Expected array or object for analysis."
            );

            GET_JSON_CUSTOM_ARR (
                query_results,
                ReaiQueryResult,
                query_result,
                GET_JSON_QUERY_RESULT,
                response->search.query_results
            );

            break;
        }

        case REAI_RESPONSE_TYPE_BATCH_RENAMES_FUNCTIONS : {
            response->type = REAI_RESPONSE_TYPE_BATCH_RENAMES_FUNCTIONS;
            break;
        }

        case REAI_RESPONSE_TYPE_RENAME_FUNCTION : {
            response->type = REAI_RESPONSE_TYPE_RENAME_FUNCTION;

            GET_JSON_BOOL (json, "success", response->rename_function.success);

            /* this string is optional, so we won't throw error if failed */
            response->rename_function.msg = json_response_get_string (json, "msg");

            break;
        }

        case REAI_RESPONSE_TYPE_BATCH_BINARY_SYMBOL_ANN :
        case REAI_RESPONSE_TYPE_BATCH_FUNCTION_SYMBOL_ANN : {
            response->type = (ReaiResponseType)type;

            GET_JSON_BOOL (json, "success", response->batch_binary_symbol_ann.success);
            if (!response->batch_binary_symbol_ann.success) {
                break;
            }

            cJSON* settings = cJSON_GetObjectItem (json, "settings");
            GOTO_HANDLER_IF (
                !settings,
                INIT_FAILED,
                "Failed to get 'settings' from returned response"
            );

            GET_JSON_STRING_ARR (
                settings,
                "collection",
                response->batch_binary_symbol_ann.settings.collections
            );

            GET_JSON_BOOL (
                settings,
                "debug_mode",
                response->batch_binary_symbol_ann.settings.debug_mode
            );

            GET_JSON_F64 (
                settings,
                "distance",
                response->batch_binary_symbol_ann.settings.distance
            );

            GET_JSON_U64 (
                settings,
                "result_per_function",
                response->batch_binary_symbol_ann.settings.result_per_function
            );

            cJSON* function_matches = cJSON_GetObjectItem (json, "function_matches");
            GOTO_HANDLER_IF (
                !function_matches,
                INIT_FAILED,
                "Failed to get 'function_matches' array from returned response"
            );

            GET_JSON_CUSTOM_ARR (
                function_matches,
                ReaiAnnFnMatch,
                ann_fn_match,
                GET_JSON_ANN_FN_MATCH,
                response->batch_binary_symbol_ann.function_matches
            );

            break;
        }

        case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
            response->type = REAI_RESPONSE_TYPE_VALIDATION_ERR;

            cJSON* detail_arr = cJSON_GetObjectItem (json, "detail");
            GOTO_HANDLER_IF (
                !detail_arr,
                INIT_FAILED,
                "Failed to get 'detail' array from returned response"
            );

            cJSON* detail_obj = cJSON_GetArrayItem (detail_arr, 0);
            GOTO_HANDLER_IF (
                !detail_obj,
                INIT_FAILED,
                "Failed to get 'detail' object from returned response"
            );

            // BUG:TODO: fix this, go over all entries in detail_obj
            GET_JSON_STRING_ARR (detail_obj, "loc", response->validation_error.locations);
            GET_JSON_STRING (detail_obj, "msg", response->validation_error.message);
            GET_JSON_STRING (detail_obj, "type", response->validation_error.type);

            break;
        }

        default :
        case REAI_RESPONSE_TYPE_UNKNOWN_ERR : {
            response->type = REAI_RESPONSE_TYPE_UNKNOWN_ERR;
            break;
        }
    }

DEFAULT_RETURN:
    if (json) {
        cJSON_Delete (json);
    }

    return response;

INIT_FAILED:
    response = NULL;
    goto DEFAULT_RETURN;
}

#undef GET_JSON_BOOL
#undef GET_JSON_STRING
#undef GET_JSON_STRING_ARR
#undef GET_JSON_STRING_ON_SUCCESS
#undef GET_JSON_ANALYSIS_INFO
#undef GET_JSON_QUERY_RESULT

/**
 * @b This callback is set as `CURLOPT_WRITEFUNCTION` to receive response data,
 * whenever `curl_easy_perform` is called. It's here because having this
 * inside ReaiResponse doesn't really make sense.
 *
 * NOTE: This method is not for direct use except by reai methods.
 *
 * @param[in] ptr Data pointer provided by curl where incoming data is temporarily stored.
 * @param[in] size Size of each item in ptr (if array).
 * @param[in] nmemb Number of members in ptr (if array).
 * @param[out] response Where raw response data will be stored.
 *
 * @return Number of bytes we read.
 * */
HIDDEN Size
    reai_response_write_callback (void* ptr, Size size, Size nmemb, ReaiResponse* response) {
    RETURN_VALUE_IF (!ptr || !response, 0, ERR_INVALID_ARGUMENTS);

    /* compute and resize buffer to required size */
    Size received_size = size * nmemb;

    Size newlen = response->raw.length + received_size;

    /* resize only if required */
    if (newlen >= response->raw.capacity) {
        Char* data = REALLOCATE (response->raw.data, Char, newlen + 1);
        RETURN_VALUE_IF (!data, 0, ERR_OUT_OF_MEMORY);

        response->raw.data                         = data;
        response->raw.capacity                     = newlen;
        response->raw.data[response->raw.capacity] = 0;
    }

    /* copy over retrieved data to given buffer */
    memcpy (response->raw.data + response->raw.length, ptr, received_size);
    response->raw.length = newlen;

    return received_size;
}

/**************************************************************************************************/
/**************************************** PRIVATE METHODS *****************************************/
/**************************************************************************************************/

/**
 * @b Helper method to extract a Boolean field form given JSON.
 * 
 * @param json[in] JSON Object containing the Boolean field.
 * @param name[in] Name of Boolean field.
 * 
 * @return @c true if Boolean is present and is true.
 * @return @c false otherwise.
 * */
PRIVATE Bool json_response_get_bool (cJSON* json, CString name) {
    RETURN_VALUE_IF (!json || !name, false, ERR_INVALID_ARGUMENTS);

    /* get Boolean value */
    cJSON* Bool_value = cJSON_GetObjectItemCaseSensitive (json, name);
    RETURN_VALUE_IF (!Bool_value, false, "Failed to get '%s' result", name);
    RETURN_VALUE_IF (
        !cJSON_IsBool (Bool_value),
        false,
        "Given success value in response is not a Boolean value\n"
    );

    return cJSON_IsTrue (Bool_value);
}

/**
 * @b Helper method to extract a String field form given JSON.
 * 
 * @param json[in] JSON Object containing the string field.
 * @param name[in] Name of string field.
 * @param num[in] Pointer to memory where value will be read.
 * 
 * @return @c num if field is present and loaded successful.
 * @return @c NULL otherwise.
 * */
PRIVATE Uint64* json_response_get_u64 (cJSON* json, CString name, Uint64* num) {
    RETURN_VALUE_IF (!json || !name || !num, NULL, ERR_INVALID_ARGUMENTS);

    /* get message */
    cJSON* flt_value = cJSON_GetObjectItemCaseSensitive (json, name);
    RETURN_VALUE_IF (
        !flt_value || !cJSON_IsNumber (flt_value),
        NULL,
        "Failed to get string value for key '%s'\n",
        name
    );

    /* copy value */
    *num = cJSON_GetNumberValue (flt_value);
    return num;
}

/**
 * @b Helper method to extract a Float64 field form given JSON.
 *
 * @param json[in] JSON Object containing the string field.
 * @param name[in] Name of string field.
 * @param num[in]  Pointer to memory where value will be read.
 *
 * @return @c num if field is present and loaded successfull.
 * @return @c NULL otherwise.
 * */
PRIVATE Float64* json_response_get_f64 (cJSON* json, CString name, Float64* num) {
    RETURN_VALUE_IF (!json || !name || !num, NULL, ERR_INVALID_ARGUMENTS);

    /* get message */
    cJSON* int_value = cJSON_GetObjectItemCaseSensitive (json, name);
    RETURN_VALUE_IF (
        !int_value || !cJSON_IsNumber (int_value),
        NULL,
        "Failed to get string value for key '%s'\n",
        name
    );

    /* copy value */
    *num = cJSON_GetNumberValue (int_value);
    return num;
}

/**
 * @b Helper method to extract a String field form given JSON.
 * 
 * The returned pointer (if any) is created by `strdup` and hence is
 * completely owned by the caller. After use, the string must be freed
 * using `FREE`
 *
 * @param json[in] JSON Object containing the string field.
 * @param name[in] Name of string field.
 * 
 * @return @c CString if field is present.
 * @return @c NULL otherwise.
 * */
PRIVATE CString json_response_get_string (cJSON* json, CString name) {
    RETURN_VALUE_IF (!json || !name, NULL, ERR_INVALID_ARGUMENTS);

    /* get message */
    cJSON* str_value = cJSON_GetObjectItemCaseSensitive (json, name);
    RETURN_VALUE_IF (
        !str_value || !cJSON_IsString (str_value),
        NULL,
        "Failed to get string value for key '%s'\n",
        name
    );

    /* copy value */
    CString str = NULL;
    RETURN_VALUE_IF (!(str = strdup (cJSON_GetStringValue (str_value))), NULL, ERR_OUT_OF_MEMORY);

    return str;
}

/**
 * @param json[in] JSON Object containing the string field.
 * @param name[in] Name of string field.
 * @param buf[out] Buffer where string list will be written to.
 * @param buf_cap[in,out] Pointer to variable that must contain the current
 * capacity of given buffer. If buffer is resized, new capacity will be
 * stored in this variable through this pointer.
 * 
 * @return @c buf if field is present.
 * @return @c NULL otherwise.
 * */
PRIVATE CStrVec* json_response_get_string_arr (cJSON* json, CString name) {
    RETURN_VALUE_IF (!json || !name, NULL, ERR_INVALID_ARGUMENTS);

    /* get array object*/
    cJSON* arr_value = cJSON_GetObjectItemCaseSensitive (json, name);
    RETURN_VALUE_IF (
        !arr_value || !cJSON_IsArray (arr_value),
        NULL,
        "Failed to get string value for key '%s'\n",
        name
    );

    CStrVec* vec = reai_cstr_vec_create();
    RETURN_VALUE_IF (!vec, NULL, "Failed to create new CStrVec object.");

    /* iterate over array and keep appending entries to a comma separated string list */
    cJSON* location = NULL;
    cJSON_ArrayForEach (location, arr_value) {
        /* if an entry is not string then just deallocated (if allocated) and exit */
        GOTO_HANDLER_IF (
            !cJSON_IsString (location),
            PARSE_FAILED,
            "Locations array expects entries to be in String."
        );

        CString loc = cJSON_GetStringValue (location);
        if (!reai_cstr_vec_append (vec, &loc)) {
            PRINT_ERR ("Failed to append value to list.");
            reai_cstr_vec_destroy (vec);
            return NULL;
        }
    }

    return vec;

PARSE_FAILED:
    return NULL;
}

/**
 * @b Reset response. This is different from de-initializing because
 * this does not free response buffer.
 *
 * @param[out] response
 *
 * @return @c response on success.
 * @return @c NULL otherwise.
 * */
HIDDEN ReaiResponse* reai_response_reset (ReaiResponse* response) {
    RETURN_VALUE_IF (!response, NULL, ERR_INVALID_ARGUMENTS);

    memset (response->raw.data, 0, response->raw.length);
    response->raw.length = 0;

    if (response->validation_error.locations) {
        reai_cstr_vec_destroy (response->validation_error.locations);
        response->validation_error.locations = NULL;
    }

    switch (response->type) {
        case REAI_RESPONSE_TYPE_AUTH_CHECK :
        case REAI_RESPONSE_TYPE_HEALTH_CHECK : {
            if (response->health_check.message) {
                FREE (response->health_check.message);
                response->health_check.message = NULL;
            }
            break;
        }

        case REAI_RESPONSE_TYPE_UPLOAD_FILE : {
            if (response->upload_file.message) {
                FREE (response->upload_file.message);
                response->upload_file.message = NULL;
            }
            if (response->upload_file.sha_256_hash) {
                FREE (response->upload_file.sha_256_hash);
                response->upload_file.sha_256_hash = NULL;
            }
            break;
        }

        case REAI_RESPONSE_TYPE_BASIC_FUNCTION_INFO : {
            if (response->basic_function_info.fn_infos) {
                reai_fn_info_vec_destroy (response->basic_function_info.fn_infos);
                response->basic_function_info.fn_infos = NULL;
            }
            break;
        }

        case REAI_RESPONSE_TYPE_RECENT_ANALYSIS : {
            if (response->recent_analysis.analysis_infos) {
                reai_analysis_info_vec_destroy (response->recent_analysis.analysis_infos);
                response->recent_analysis.analysis_infos = NULL;
            }
            break;
        }

        case REAI_RESPONSE_TYPE_SEARCH : {
            if (response->search.query_results) {
                reai_query_result_vec_destroy (response->search.query_results);
                response->search.query_results = NULL;
            }
            break;
        }

        case REAI_RESPONSE_TYPE_BATCH_BINARY_SYMBOL_ANN :
        case REAI_RESPONSE_TYPE_BATCH_FUNCTION_SYMBOL_ANN : {
            if (response->batch_binary_symbol_ann.function_matches) {
                reai_ann_fn_match_vec_destroy (response->batch_binary_symbol_ann.function_matches);
                response->batch_binary_symbol_ann.function_matches = NULL;
            }

            if (response->batch_binary_symbol_ann.settings.collections) {
                reai_cstr_vec_destroy (response->batch_binary_symbol_ann.settings.collections);
                response->batch_binary_symbol_ann.settings.collections = NULL;
            }
        }

        default :
            break;
    }

    return NULL;
}
