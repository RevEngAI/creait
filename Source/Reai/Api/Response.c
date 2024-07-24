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

#include "Reai/Util/AnalysisInfo.h"

PRIVATE Bool    json_response_get_bool (cJSON* json, CString name);
PRIVATE Uint64* json_response_get_num (cJSON* json, CString name, Uint64* num);
PRIVATE CString json_response_get_string (cJSON* json, CString name);
PRIVATE Char*   json_response_get_string_array_as_comma_separated_list (
      cJSON*  json,
      CString name,
      Char*   buf,
      Size*   buf_cap
  );
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

    /* NOTE: Again, keep this a large value. */
#define RESPONSE_VALIDATION_ERR_LOCATIONS_BUF_INITIAL_CAP 512

    /* allocate large string where locations array will be converted to a comma separated list */
    response->validation_error.locations =
        ALLOCATE (Char, RESPONSE_VALIDATION_ERR_LOCATIONS_BUF_INITIAL_CAP);

    RETURN_VALUE_IF (!response->validation_error.locations, Null, ERR_OUT_OF_MEMORY);

    response->validation_error.locations_capacity =
        RESPONSE_VALIDATION_ERR_LOCATIONS_BUF_INITIAL_CAP;

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
        memset (
            response->validation_error.locations,
            0,
            response->validation_error.locations_capacity
        );

        FREE (response->validation_error.locations);
    }

    memset (response, 0, sizeof (ReaiResponse));

    return response;
}

/**************************************************************************************************/
/***************************************** HIDDEN METHODS *****************************************/
/**************************************************************************************************/

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

    CString analysis_status = Null;

    /* convert from string to json */
    cJSON* json = cJSON_ParseWithLength (response->raw.data, response->raw.length);
    GOTO_HANDLER_IF (!json, INIT_FAILED, "Failed to parse given response as JSON data\n");


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
#define GET_JSON_STRING_ARR(json, name, buf, bufszptr)                                             \
    {                                                                                              \
        Char* tmp = Null;                                                                          \
        GOTO_HANDLER_IF (                                                                          \
            !(tmp = json_response_get_string_array_as_comma_separated_list (                       \
                  detail_obj,                                                                      \
                  name,                                                                            \
                  buf,                                                                             \
                  bufszptr                                                                         \
              )),                                                                                  \
            INIT_FAILED,                                                                           \
            "Failed to get '%s' string array in response\n",                                       \
            name                                                                                   \
        );                                                                                         \
        buf = tmp;                                                                                 \
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

    /* each response type has a different structure */
    switch (type) {
        case REAI_RESPONSE_TYPE_DELETE_ANALYSIS :
        case REAI_RESPONSE_TYPE_HEALTH_CHECK :
        case REAI_RESPONSE_TYPE_AUTH_CHECK : {
            response->type = REAI_RESPONSE_TYPE_AUTH_CHECK;

            GET_JSON_BOOL (json, "success", response->auth_check.success);

            CString msg_keyname = response->auth_check.success ? "msg" : "error";
            GET_JSON_STRING (json, msg_keyname, response->auth_check.message);

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
                ReaiFnInfoVec* vec = Null;
                GOTO_HANDLER_IF (
                    !(vec = reai_fn_info_vec_create()),
                    INIT_FAILED,
                    "Failed to create FnInfo vector to store retrieved function information.\n"
                );

                /* go over each item in array and insert */
                cJSON*      function = Null;
                ReaiFnInfo* info     = vec->items;
                cJSON_ArrayForEach (function, functions) {
                    GET_JSON_NUM (function, "function_id", info->id);
                    GET_JSON_STRING (function, "function_name", info->name);
                    GET_JSON_NUM (function, "function_size", info->size);
                    GET_JSON_NUM (function, "function_vaddr", info->vaddr);

                    info++;
                }

                response->basic_function_info.fn_infos = vec;
            }

            break;
        }

#define GET_ANALYSIS_INFO(ainfo)                                                                   \
    do {                                                                                           \
        GET_JSON_NUM (analysis, "binary_id", ainfo.binary_id);                                     \
        GET_JSON_STRING (analysis, "binary_name", ainfo.binary_name);                              \
        GET_JSON_STRING (analysis, "creation", ainfo.creation);                                    \
        GET_JSON_NUM (analysis, "model_id", ainfo.model_id);                                       \
        GET_JSON_STRING (analysis, "model_name", ainfo.model_name);                                \
        GET_JSON_STRING (analysis, "sha_256_hash", ainfo.sha_256_hash);                            \
        ReaiAnalysisStatus estatus = 0;                                                            \
        GET_JSON_STRING (analysis, "status", analysis_status);                                     \
        GOTO_HANDLER_IF (                                                                          \
            !(estatus = reai_analysis_status_from_cstr (analysis_status)),                         \
            INIT_FAILED,                                                                           \
            "Failed to convert analysis status to enum\n"                                          \
        );                                                                                         \
        ainfo.status = estatus;                                                                    \
    } while (0)

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

                /* create array to store analysis info records */
                GOTO_HANDLER_IF (
                    !(response->recent_analysis.analysis_infos = reai_analysis_info_vec_create()),
                    INIT_FAILED,
                    "Failed to create analysis info array\n"
                );

                /* switch on whether analysis info is an array or an object */
                ReaiAnalysisInfo analysis_info;
                if (cJSON_IsObject (analysis)) {
                    GET_ANALYSIS_INFO (analysis_info);

                    reai_analysis_info_vec_append (
                        response->recent_analysis.analysis_infos,
                        &analysis_info
                    );
                } else if (cJSON_IsArray (analysis)) {
                    /* rename to make macro work */
                    cJSON* analysis_arr = analysis;

                    /* iterate over all elements, parse and insert */
                    cJSON_ArrayForEach (analysis, analysis_arr) {
                        GET_ANALYSIS_INFO (analysis_info);

                        reai_analysis_info_vec_append (
                            response->recent_analysis.analysis_infos,
                            &analysis_info
                        );
                    }
                }
            }

            break;
        }

#undef GET_ANALYSIS_INFO

        case REAI_RESPONSE_TYPE_ANALYSIS_STATUS : {
            response->type = REAI_RESPONSE_TYPE_ANALYSIS_STATUS;

            GET_JSON_BOOL (json, "success", response->analysis_status.success);
            GET_JSON_STRING_ON_SUCCESS (
                json,
                "status",
                analysis_status,
                response->analysis_status.success
            );
            response->analysis_status.status = reai_analysis_status_from_cstr (analysis_status);

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

            /* loc */
            GET_JSON_STRING_ARR (
                detail_obj,
                "loc",
                response->validation_error.locations,
                &response->validation_error.locations_capacity
            );

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

#undef GET_JSON_BOOL
#undef GET_JSON_STRING
#undef GET_JSON_STRING_ARR
#undef GET_JSON_STRING_ON_SUCCESS

DEFAULT_RETURN:
    if (json) {
        cJSON_Delete (json);
    }

    if (analysis_status) {
        FREE (analysis_status);
    }

    return response;

INIT_FAILED:
    response = Null;
    goto DEFAULT_RETURN;
}

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
 * @b Helper method to extract a array of string form given JSON
 * into a comma separated string list. This means the whole array
 * will be converted into a single string.
 *
 * NOTE: The pointer returned may or may not be same as buf. This depends on
 * the response size and whether a reallocation is required or not. Caller
 * must keep the buffer and returned size value in sync to avoid UB.
 *
 * NOTE: If provided buffer is Null or if given buffer size pointer contains
 * zero value, then the buffer will be reallocated for sure and returned pointer
 * will definitely be different (given no errors occur).
 *
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
PRIVATE Char* json_response_get_string_array_as_comma_separated_list (
    cJSON*  json,
    CString name,
    Char*   buf,
    Size*   buf_cap
) {
    RETURN_VALUE_IF (!json || !name || !buf_cap, Null, ERR_INVALID_ARGUMENTS);

    /* get message */
    cJSON* arr_value = cJSON_GetObjectItemCaseSensitive (json, name);
    RETURN_VALUE_IF (
        !arr_value || !cJSON_IsArray (arr_value),
        Null,
        "Failed to get string value for key '%s'\n",
        name
    );

    Size  tmpbuf_length = 0;
    Size  tmpbuf_cap    = *buf_cap;
    Char* tmpbuf        = buf;
    Bool  allocated     = False;

    /* allocate buffer if not provided */
    if (!tmpbuf || !tmpbuf_cap) {
        tmpbuf = ALLOCATE (Char, RESPONSE_VALIDATION_ERR_LOCATIONS_BUF_INITIAL_CAP);
        RETURN_VALUE_IF (!tmpbuf, Null, ERR_OUT_OF_MEMORY);
        tmpbuf_cap = RESPONSE_VALIDATION_ERR_LOCATIONS_BUF_INITIAL_CAP;
        allocated  = True;
    }

    /* iterate over array and keep appending entries to a comma separated string list */
    cJSON* location = Null;
    Char*  iter     = tmpbuf;
    cJSON_ArrayForEach (location, arr_value) {
        /* if an entry is not string then just deallocated (if allocated) and exit */
        GOTO_HANDLER_IF (
            !cJSON_IsString (location),
            PARSE_FAILED,
            "Locations array expects entries to be in String."
        );

        /* append string */
        /* NOTE: the value returned by sprintf does not count null terminal character */
        tmpbuf_length += sprintf (iter, "%s, ", cJSON_GetStringValue (location));
        iter          += tmpbuf_length;

        /* reallocated if required */
        if (tmpbuf_length + 1 >= tmpbuf_cap) {
            Size off = iter - tmpbuf; /* distance from starting position of array */

            Char* tmp = REALLOCATE (tmpbuf, Char, tmpbuf_length * 2);
            GOTO_HANDLER_IF (!tmp, PARSE_FAILED, ERR_OUT_OF_MEMORY);

            /* readjust iter */
            tmpbuf = tmp;
            iter   = tmpbuf + off;
        }
    }

    *buf_cap = tmpbuf_cap;
    return tmpbuf;

PARSE_FAILED:
    if (allocated) {
        FREE (tmpbuf);
    }
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

        default :
            break;
    }

    return Null;
}
