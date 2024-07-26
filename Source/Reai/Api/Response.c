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

PRIVATE Bool         json_response_get_bool (cJSON* json, CString name);
PRIVATE Uint64*      json_response_get_num (cJSON* json, CString name, Uint64* num);
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
 * @return @c Null otherwise.
 * */
PUBLIC ReaiResponse* reai_response_init (ReaiResponse* response) {
    RETURN_VALUE_IF (!response, Null, ERR_INVALID_ARGUMENTS);

    /* invalidate all fields */
    memset (response, 0, sizeof (ReaiResponse));

    /* NOTE: make sure to keep it to a large value from the beginning itself to avoid many reallocations */
#define RESPONSE_RAW_BUF_INITIAL_CAP 4000

    /* allocate a significantly large memory block for getting raw responses */
    response->raw.data = ALLOCATE (Char, RESPONSE_RAW_BUF_INITIAL_CAP);
    RETURN_VALUE_IF (!response->raw.data, Null, ERR_OUT_OF_MEMORY);
    response->raw.capacity = RESPONSE_RAW_BUF_INITIAL_CAP;

    return response;
}

/**
 * @b De-initialize given ReaiResponse structure. 
 *
 * @param reai_response
 *
 * @return @c reai_response on success.
 * @return @c Null otherwise.
 * */
PUBLIC ReaiResponse* reai_response_deinit (ReaiResponse* response) {
    RETURN_VALUE_IF (!response, Null, ERR_INVALID_ARGUMENTS);

    reai_response_reset (response);

    if (response->raw.data) {
        memset (response->raw.data, 0, response->raw.capacity);
        FREE (response->raw.data);
    }

    if (response->validation_error.locations) {
        reai_cstr_vec_destroy (response->validation_error.locations);
        response->validation_error.locations = Null;
    }

    memset (response, 0, sizeof (ReaiResponse));

    return response;
}

#define GET_JSON_BOOL(json, name, var) var = json_response_get_bool (json, name);

#define GET_JSON_NUM(json, name, var)                                                              \
    {                                                                                              \
        Uint64 num = 0;                                                                            \
        GOTO_HANDLER_IF (                                                                          \
            !(json_response_get_num (json, name, &num)),                                           \
            INIT_FAILED,                                                                           \
            "Failed to get number '%s' from response.\n",                                          \
            name                                                                                   \
        );                                                                                         \
                                                                                                   \
        var = num;                                                                                 \
    }

/* retrieved string must be freed after use */
#define GET_JSON_STRING(json, name, var)                                                           \
    {                                                                                              \
        CString str = Null;                                                                        \
        GOTO_HANDLER_IF (                                                                          \
            !(str = json_response_get_string (json, name)),                                        \
            INIT_FAILED,                                                                           \
            "Failed to get string '%s' from response.\n",                                          \
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
            "Failed to get '%s' string array in response\n",                                       \
            name                                                                                   \
        );                                                                                         \
    }

/* retrieved string must be freed after use */
#define GET_JSON_STRING_ON_SUCCESS(json, name, var, success)                                       \
    if (success) {                                                                                 \
        GET_JSON_STRING (json, name, var);                                                         \
    } else {                                                                                       \
        var = Null;                                                                                \
    }

#define GET_JSON_NUM_ON_SUCCESS(json, name, var, success)                                          \
    if (success) {                                                                                 \
        GET_JSON_NUM (json, name, var);                                                            \
    } else {                                                                                       \
        var = 0;                                                                                   \
    }

#define GET_JSON_ANALYSIS_INFO(json, ainfo)                                                        \
    do {                                                                                           \
        GET_JSON_NUM (json, "binary_id", ainfo.binary_id);                                         \
        GET_JSON_STRING (json, "binary_name", ainfo.binary_name);                                  \
        GET_JSON_STRING (json, "creation", ainfo.creation);                                        \
        GET_JSON_NUM (json, "model_id", ainfo.model_id);                                           \
        GET_JSON_STRING (json, "model_name", ainfo.model_name);                                    \
        GET_JSON_STRING (json, "sha_256_hash", ainfo.sha_256_hash);                                \
                                                                                                   \
        CString            status  = Null;                                                         \
        ReaiAnalysisStatus estatus = 0;                                                            \
        GET_JSON_STRING (json, "status", status);                                                  \
        if (!(estatus = reai_analysis_status_from_cstr (status))) {                                \
            PRINT_ERR ("Failed to convert analysis status to enum\n");                             \
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
        GET_JSON_NUM (json, "binary_id", qres.binary_id);                                          \
        GET_JSON_STRING (json, "binary_name", qres.binary_name);                                   \
        GET_JSON_STRING_ARR (json, "collections", qres.collections);                               \
        GET_JSON_STRING (json, "creation", qres.creation);                                         \
        GET_JSON_NUM (json, "model_id", qres.model_id);                                            \
        GET_JSON_STRING (json, "model_name", qres.model_name);                                     \
        GET_JSON_STRING (json, "sha_256_hash", qres.sha_256_hash);                                 \
        GET_JSON_STRING_ARR (json, "tags", qres.tags);                                             \
                                                                                                   \
        CString            status  = Null;                                                         \
        ReaiAnalysisStatus estatus = 0;                                                            \
        GET_JSON_STRING (json, "status", status);                                                  \
        if (!(qres.status = reai_analysis_status_from_cstr (status))) {                            \
            PRINT_ERR ("Failed to convert analysis status to enum\n");                             \
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
 * @return @c Null otherwise.
 * */
HIDDEN ReaiResponse* reai_response_init_for_type (ReaiResponse* response, ReaiResponseType type) {
    RETURN_VALUE_IF (!response, Null, ERR_INVALID_ARGUMENTS);

    /* convert from string to json */
    cJSON* json = cJSON_ParseWithLength (response->raw.data, response->raw.length);
    GOTO_HANDLER_IF (!json, INIT_FAILED, "Failed to parse given response as JSON data\n");

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
            GET_JSON_NUM_ON_SUCCESS (
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

            if (response->basic_function_info.success) {
                /* get functions array */
                cJSON* functions = cJSON_GetObjectItem (json, "functions");
                GOTO_HANDLER_IF (
                    !functions || !cJSON_IsArray (functions),
                    INIT_FAILED,
                    "Given JSON response does not contain 'functions' array\n"
                );

                /* create vector to store information */
                ReaiFnInfoVec* fn_infos = Null;
                GOTO_HANDLER_IF (
                    !(fn_infos = reai_fn_info_vec_create()),
                    INIT_FAILED,
                    "Failed to create FnInfo vector to store retrieved function information.\n"
                );

                /* go over each item in array and insert */
                cJSON* function = Null;
                cJSON_ArrayForEach (function, functions) {
                    ReaiFnInfo fn = {0};
                    GET_JSON_NUM (function, "function_id", fn.id);
                    GET_JSON_STRING (function, "function_name", fn.name);
                    GET_JSON_NUM (function, "function_size", fn.size);
                    GET_JSON_NUM (function, "function_vaddr", fn.vaddr);

                    if (!reai_fn_info_vec_append (fn_infos, &fn)) {
                        PRINT_ERR ("Failed to insert a new function info to vector.\n");
                        reai_fn_info_vec_destroy (fn_infos);
                        goto INIT_FAILED;
                    }
                }

                response->basic_function_info.fn_infos = fn_infos;
            }

            break;
        }

        case REAI_RESPONSE_TYPE_RECENT_ANALYSIS : {
            response->type = REAI_RESPONSE_TYPE_RECENT_ANALYSIS;

            GET_JSON_BOOL (json, "success", response->recent_analysis.success);

            if (response->recent_analysis.success) {
                cJSON* analysis = cJSON_GetObjectItem (json, "analysis");
                GOTO_HANDLER_IF (
                    !cJSON_IsObject (analysis) && !cJSON_IsArray (analysis),
                    INIT_FAILED,
                    "Expected array or object for analysis.\n"
                );

                GOTO_HANDLER_IF (
                    !(response->recent_analysis.analysis_infos = reai_analysis_info_vec_create()),
                    INIT_FAILED,
                    "Failed to create analysis info array\n"
                );

                ReaiAnalysisInfo analysis_info;
                if (cJSON_IsObject (analysis)) {
                    GET_JSON_ANALYSIS_INFO (analysis, analysis_info);

                    reai_analysis_info_vec_append (
                        response->recent_analysis.analysis_infos,
                        &analysis_info
                    );
                } else {
                    /* iterate over all elements, parse and insert */
                    cJSON* info = Null;

                    cJSON_ArrayForEach (info, analysis) {
                        GET_JSON_ANALYSIS_INFO (info, analysis_info);

                        reai_analysis_info_vec_append (
                            response->recent_analysis.analysis_infos,
                            &analysis_info
                        );
                    }
                }
            }

            break;
        }


        case REAI_RESPONSE_TYPE_ANALYSIS_STATUS : {
            response->type = REAI_RESPONSE_TYPE_ANALYSIS_STATUS;

            GET_JSON_BOOL (json, "success", response->analysis_status.success);

            CString status = Null;
            GET_JSON_STRING_ON_SUCCESS (json, "status", status, response->analysis_status.success);

            if (!(response->analysis_status.status = reai_analysis_status_from_cstr (status))) {
                PRINT_ERR ("Failed to convert analysis status from string to enum.\n");
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

            if (response->search.success) {
                cJSON* query_results = cJSON_GetObjectItem (json, "query_results");
                GOTO_HANDLER_IF (
                    !cJSON_IsObject (query_results) && !cJSON_IsArray (query_results),
                    INIT_FAILED,
                    "Expected array or object for analysis.\n"
                );

                /* create array to store analysis info records */
                GOTO_HANDLER_IF (
                    !(response->search.query_results = reai_query_result_vec_create()),
                    INIT_FAILED,
                    "Failed to create query results array\n"
                );

                ReaiQueryResult query_result;

                if (cJSON_IsObject (query_results)) {
                    /* insert just one item */
                    GET_JSON_QUERY_RESULT (query_results, query_result);
                    reai_query_result_vec_append (response->search.query_results, &query_result);
                } else {
                    /* iterate over all elements, parse and insert */
                    cJSON* result = Null;
                    cJSON_ArrayForEach (result, query_results) {
                        GET_JSON_QUERY_RESULT (result, query_result);
                        reai_query_result_vec_append (
                            response->search.query_results,
                            &query_result
                        );
                    }
                }
            }

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

        case REAI_RESPONSE_TYPE_VALIDATION_ERR : {
            response->type = REAI_RESPONSE_TYPE_VALIDATION_ERR;

            cJSON* detail_arr = cJSON_GetObjectItem (json, "detail");
            GOTO_HANDLER_IF (
                !detail_arr,
                INIT_FAILED,
                "Failed to get 'detail' array from returned response\n"
            );

            cJSON* detail_obj = cJSON_GetArrayItem (detail_arr, 0);
            GOTO_HANDLER_IF (
                !detail_obj,
                INIT_FAILED,
                "Failed to get 'detail' object from returned response\n"
            );

            // BUG:TODO: fix this, go over all entries in detail_obj

            /* loc */
            GET_JSON_STRING_ARR (detail_obj, "loc", response->validation_error.locations);

            /* msg */
            GET_JSON_STRING (detail_obj, "msg", response->validation_error.message);

            /* type */
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
    response = Null;
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
 * @b Helper method to extract a boolean field form given JSON.
 * 
 * @param json[in] JSON Object containing the boolean field.
 * @param name[in] Name of boolean field.
 * 
 * @return @c True if boolean is present and is true.
 * @return @c False otherwise.
 * */
PRIVATE Bool json_response_get_bool (cJSON* json, CString name) {
    RETURN_VALUE_IF (!json || !name, False, ERR_INVALID_ARGUMENTS);

    /* get boolean value */
    cJSON* bool_value = cJSON_GetObjectItemCaseSensitive (json, name);
    RETURN_VALUE_IF (!bool_value, False, "Failed to get '%s' result\n", name);
    RETURN_VALUE_IF (
        !cJSON_IsBool (bool_value),
        False,
        "Given success value in response is not a boolean value\n"
    );

    return cJSON_IsTrue (bool_value);
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
 * @return @c num if field is present and loaded successfull.
 * @return @c Null otherwise.
 * */
PRIVATE Uint64* json_response_get_num (cJSON* json, CString name, Uint64* num) {
    RETURN_VALUE_IF (!json || !name || !num, Null, ERR_INVALID_ARGUMENTS);

    /* get message */
    cJSON* int_value = cJSON_GetObjectItemCaseSensitive (json, name);
    RETURN_VALUE_IF (
        !int_value || !cJSON_IsNumber (int_value),
        Null,
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
 * @return @c Null otherwise.
 * */
PRIVATE CString json_response_get_string (cJSON* json, CString name) {
    RETURN_VALUE_IF (!json || !name, Null, ERR_INVALID_ARGUMENTS);

    /* get message */
    cJSON* str_value = cJSON_GetObjectItemCaseSensitive (json, name);
    RETURN_VALUE_IF (
        !str_value || !cJSON_IsString (str_value),
        Null,
        "Failed to get string value for key '%s'\n",
        name
    );

    /* copy value */
    CString str = Null;
    RETURN_VALUE_IF (!(str = strdup (cJSON_GetStringValue (str_value))), Null, ERR_OUT_OF_MEMORY);

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
 * @return @c Null otherwise.
 * */
PRIVATE CStrVec* json_response_get_string_arr (cJSON* json, CString name) {
    RETURN_VALUE_IF (!json || !name, Null, ERR_INVALID_ARGUMENTS);

    /* get array object*/
    cJSON* arr_value = cJSON_GetObjectItemCaseSensitive (json, name);
    RETURN_VALUE_IF (
        !arr_value || !cJSON_IsArray (arr_value),
        Null,
        "Failed to get string value for key '%s'\n",
        name
    );

    CStrVec* vec = reai_cstr_vec_create();
    RETURN_VALUE_IF (!vec, Null, "Failed to create new CStrVec object.\n");

    /* iterate over array and keep appending entries to a comma separated string list */
    cJSON* location = Null;
    cJSON_ArrayForEach (location, arr_value) {
        /* if an entry is not string then just deallocated (if allocated) and exit */
        GOTO_HANDLER_IF (
            !cJSON_IsString (location),
            PARSE_FAILED,
            "Locations array expects entries to be in String."
        );

        CString loc = cJSON_GetStringValue (location);
        if (!reai_cstr_vec_append (vec, &loc)) {
            PRINT_ERR ("Failed to append value to list.\n");
            reai_cstr_vec_destroy (vec);
            return Null;
        }
    }

    return vec;

PARSE_FAILED:
    return Null;
}

/**
 * @b Reset response. This is different from de-initializing because
 * this does not free response buffer.
 *
 * @param[out] response
 *
 * @return @c response on success.
 * @return @c Null otherwise.
 * */
HIDDEN ReaiResponse* reai_response_reset (ReaiResponse* response) {
    RETURN_VALUE_IF (!response, Null, ERR_INVALID_ARGUMENTS);

    memset (response->raw.data, 0, response->raw.length);
    response->raw.length = 0;

    if (response->validation_error.locations) {
        reai_cstr_vec_destroy (response->validation_error.locations);
        response->validation_error.locations = Null;
    }

    switch (response->type) {
        case REAI_RESPONSE_TYPE_AUTH_CHECK :
        case REAI_RESPONSE_TYPE_HEALTH_CHECK : {
            if (response->health_check.message) {
                FREE (response->health_check.message);
                response->health_check.message = Null;
            }
            break;
        }

        case REAI_RESPONSE_TYPE_UPLOAD_FILE : {
            if (response->upload_file.message) {
                FREE (response->upload_file.message);
                response->upload_file.message = Null;
            }
            if (response->upload_file.sha_256_hash) {
                FREE (response->upload_file.sha_256_hash);
                response->upload_file.sha_256_hash = Null;
            }
            break;
        }

        case REAI_RESPONSE_TYPE_BASIC_FUNCTION_INFO : {
            if (response->basic_function_info.fn_infos) {
                reai_fn_info_vec_destroy (response->basic_function_info.fn_infos);
                response->basic_function_info.fn_infos = Null;
            }
            break;
        }

        case REAI_RESPONSE_TYPE_RECENT_ANALYSIS : {
            if (response->recent_analysis.analysis_infos) {
                reai_analysis_info_vec_destroy (response->recent_analysis.analysis_infos);
                response->recent_analysis.analysis_infos = Null;
            }
            break;
        }

        case REAI_RESPONSE_TYPE_SEARCH : {
            if (response->search.query_results) {
                reai_query_result_vec_destroy (response->search.query_results);
                response->search.query_results = Null;
            }
            break;
        }

        default :
            break;
    }

    return Null;
}
