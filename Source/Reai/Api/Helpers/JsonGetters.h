/**
 * @file JsonGetters.h
 * @date 25th Sept 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef CREAIT_API_HELPERS_JSON_GETTERS_H
#define CREAIT_API_HELPERS_JSON_GETTERS_H

#define IF_JSON_OBJECT_EXISTS(json_obj, name)                                                      \
    if (cJSON_GetObjectItemCaseSensitive ((json_obj), (name)))

#define GET_OPTIONAL_JSON_BOOL(json_bool, name, var)                                               \
    IF_JSON_OBJECT_EXISTS (json_bool, name) {                                                      \
        (var) = json_response_get_bool ((json_bool), (name));                                      \
    }                                                                                              \
    else {                                                                                         \
        (var) = 0;                                                                                 \
    }

#define GET_OPTIONAL_JSON_U64(json_u64, name, var)                                                 \
    IF_JSON_OBJECT_EXISTS (json_u64, name) {                                                       \
        json_response_get_u64 ((json_u64), (name), &(var));                                        \
    }                                                                                              \
    else {                                                                                         \
        (var) = 0;                                                                                 \
    }

#define GET_OPTIONAL_JSON_F64(json_f64, name, var)                                                 \
    IF_JSON_OBJECT_EXISTS (json_f64, name) {                                                       \
        json_response_get_f64 ((json_f64), (name), &(var));                                        \
    }                                                                                              \
    else {                                                                                         \
        (var) = 0;                                                                                 \
    }


#define GET_OPTIONAL_JSON_STRING(json_str, name, var)                                              \
    IF_JSON_OBJECT_EXISTS (json_str, name) {                                                       \
        (var) = json_response_get_string ((json_str), (name));                                     \
    }                                                                                              \
    else {                                                                                         \
        (var) = 0;                                                                                 \
    }


#define GET_OPTIONAL_JSON_STRING_ARR(json_str_arr, name, var)                                      \
    IF_JSON_OBJECT_EXISTS (json_str_arr, name) {                                                   \
        (var) = json_response_get_string_arr ((json_str_arr), (name));                             \
    }                                                                                              \
    else {                                                                                         \
        (var) = 0;                                                                                 \
    }




#define GET_JSON_BOOL(json_bool, name, var) (var) = json_response_get_bool ((json_bool), name);

#define GET_JSON_U64(json_u64, name, var)                                                          \
    {                                                                                              \
        Uint64 num = 0;                                                                            \
        GOTO_HANDLER_IF (                                                                          \
            !(json_response_get_u64 ((json_u64), (name), &num)),                                   \
            INIT_FAILED,                                                                           \
            "Failed to get number '%s' from response.",                                            \
            (name)                                                                                 \
        );                                                                                         \
                                                                                                   \
        (var) = num;                                                                               \
    }

#define GET_JSON_F64(json_f64, name, var)                                                          \
    {                                                                                              \
        Float64 num = 0;                                                                           \
        GOTO_HANDLER_IF (                                                                          \
            !(json_response_get_f64 ((json_f64), (name), &num)),                                   \
            INIT_FAILED,                                                                           \
            "Failed to get number '%s' from response.",                                            \
            (name)                                                                                 \
        );                                                                                         \
                                                                                                   \
        (var) = num;                                                                               \
    }

/* retrieved string must be freed after use */
#define GET_JSON_STRING(json_str, name, var)                                                       \
    {                                                                                              \
        CString str = NULL;                                                                        \
        GOTO_HANDLER_IF (                                                                          \
            !(str = json_response_get_string ((json_str), (name))),                                \
            INIT_FAILED,                                                                           \
            "Failed to get string '%s' from response.",                                            \
            (name)                                                                                 \
        );                                                                                         \
                                                                                                   \
        (var) = str;                                                                               \
    }

/* retrieved string array must be freed after use */
#define GET_JSON_STRING_ARR(json_str_arr, name, arr)                                               \
    {                                                                                              \
        GOTO_HANDLER_IF (                                                                          \
            !(arr = json_response_get_string_arr ((json_str_arr), (name))),                        \
            INIT_FAILED,                                                                           \
            "Failed to get '%s' string array in response.",                                        \
            (name)                                                                                 \
        );                                                                                         \
    }

/* retrieved string must be freed after use */
#define GET_JSON_STRING_ON_SUCCESS(json_str, name, var, success)                                   \
    if (success) {                                                                                 \
        GET_JSON_STRING (json_str, name, var);                                                     \
    } else {                                                                                       \
        (var) = NULL;                                                                              \
    }

#define GET_JSON_U64_ON_SUCCESS(json_u64, name, var, success)                                      \
    if (success) {                                                                                 \
        GET_JSON_U64 (json_u64, name, var);                                                        \
    } else {                                                                                       \
        (var) = 0;                                                                                 \
    }

#define GET_JSON_F64_ON_SUCCESS(json_f64, name, var, success)                                      \
    if (success) {                                                                                 \
        GET_JSON_F64 (json_f64, name, var);                                                        \
    } else {                                                                                       \
        (var) = 0;                                                                                 \
    }

#define GET_JSON_FN_INFO(json_fn_info, fn)                                                         \
    do {                                                                                           \
        GET_JSON_U64 (json_fn_info, "function_id", (fn).id);                                       \
        GET_JSON_STRING (json_fn_info, "function_name", (fn).name);                                \
        GET_JSON_U64 (json_fn_info, "function_size", (fn).size);                                   \
        GET_JSON_U64 (json_fn_info, "function_vaddr", (fn).vaddr);                                 \
    } while (0)

#define GET_JSON_ANALYSIS_INFO(json_analysis_info, ainfo)                                          \
    do {                                                                                           \
        GET_JSON_U64 (json_analysis_info, "binary_id", (ainfo).binary_id);                         \
        GET_JSON_STRING (json_analysis_info, "binary_name", (ainfo).binary_name);                  \
        GET_JSON_STRING (json_analysis_info, "creation", (ainfo).creation);                        \
        GET_JSON_U64 (json_analysis_info, "model_id", (ainfo).model_id);                           \
        GET_JSON_STRING (json_analysis_info, "model_name", (ainfo).model_name);                    \
        GET_JSON_STRING (json_analysis_info, "sha_256_hash", (ainfo).sha_256_hash);                \
                                                                                                   \
        CString            status  = NULL;                                                         \
        ReaiAnalysisStatus estatus = 0;                                                            \
        GET_JSON_STRING (json_analysis_info, "status", status);                                    \
        if (!(estatus = reai_analysis_status_from_cstr (status))) {                                \
            PRINT_ERR ("Failed to convert analysis status to enum");                               \
            goto INIT_FAILED;                                                                      \
        }                                                                                          \
        (ainfo).status = estatus;                                                                  \
    } while (0)

#define GET_JSON_QUERY_RESULT(json_query_res, qres)                                                \
    do {                                                                                           \
        GET_JSON_U64 (json_query_res, "binary_id", (qres).binary_id);                              \
        GET_JSON_STRING (json_query_res, "binary_name", (qres).binary_name);                       \
        GET_JSON_STRING_ARR (json_query_res, "collections", (qres).collections);                   \
        GET_JSON_STRING (json_query_res, "creation", (qres).creation);                             \
        GET_JSON_U64 (json_query_res, "model_id", (qres).model_id);                                \
        GET_JSON_STRING (json_query_res, "model_name", (qres).model_name);                         \
        GET_JSON_STRING (json_query_res, "sha_256_hash", (qres).sha_256_hash);                     \
        GET_JSON_STRING_ARR (json_query_res, "tags", (qres).tags);                                 \
                                                                                                   \
        CString            status  = NULL;                                                         \
        ReaiAnalysisStatus estatus = 0;                                                            \
        GET_JSON_STRING (json_query_res, "status", status);                                        \
        if (!((qres).status = reai_analysis_status_from_cstr (status))) {                          \
            PRINT_ERR ("Failed to convert analysis status to enum");                               \
            goto INIT_FAILED;                                                                      \
        }                                                                                          \
        (qres).status = estatus;                                                                   \
    } while (0)

#define GET_JSON_ANN_FN_MATCH(json_fn_match, fn_match)                                             \
    do {                                                                                           \
        GET_OPTIONAL_JSON_F64 (json_fn_match, "confidence", (fn_match).confidence);                \
        GET_OPTIONAL_JSON_U64 (                                                                    \
            json_fn_match,                                                                         \
            "nearest_neighbor_binary_id",                                                          \
            (fn_match).nn_binary_id                                                                \
        );                                                                                         \
        GET_OPTIONAL_JSON_STRING (                                                                 \
            json_fn_match,                                                                         \
            "nearest_neighbor_binary_name",                                                        \
            (fn_match).nn_binary_name                                                              \
        );                                                                                         \
        GET_OPTIONAL_JSON_BOOL (json_fn_match, "nearest_neighbor_debug", (fn_match).nn_debug);     \
        GET_OPTIONAL_JSON_STRING (                                                                 \
            json_fn_match,                                                                         \
            "nearest_neighbor_function_name",                                                      \
            (fn_match).nn_function_name                                                            \
        );                                                                                         \
        GET_OPTIONAL_JSON_U64 (json_fn_match, "nearest_neighbor_id", (fn_match).nn_function_id);   \
        GET_OPTIONAL_JSON_STRING (                                                                 \
            json_fn_match,                                                                         \
            "nearest_neighbor_sha_256_hash",                                                       \
            (fn_match).nn_sha_256_hash                                                             \
        );                                                                                         \
        GET_OPTIONAL_JSON_U64 (                                                                    \
            json_fn_match,                                                                         \
            "origin_function_id",                                                                  \
            (fn_match).origin_function_id                                                          \
        );                                                                                         \
    } while (0)

#define GET_JSON_AI_MODEL(json_ai_model, model_name)                                               \
    do {                                                                                           \
        GET_JSON_STRING (json_ai_model, "model_name", model_name);                                 \
    } while (0)

#define GET_JSON_API_ERROR(json_api_error, api_error)                                              \
    do {                                                                                           \
        GET_OPTIONAL_JSON_STRING (json_api_error, "code", (api_error).code);                       \
        GET_OPTIONAL_JSON_STRING (json_api_error, "message", (api_error).message);                 \
    } while (0)

#define GET_JSON_CUSTOM_ARR(json_arr, type_name, type_infix, reader, vec)                          \
    do {                                                                                           \
        (vec) = reai_##type_infix##_vec_create();                                                  \
        GOTO_HANDLER_IF (!(vec), INIT_FAILED, "Failed to create " #type_name " vector.");          \
                                                                                                   \
        if (cJSON_IsObject (json_arr)) {                                                           \
            type_name item = {0};                                                                  \
            reader (json_arr, item);                                                               \
                                                                                                   \
            if (!reai_##type_infix##_vec_append ((vec), &item)) {                                  \
                PRINT_ERR ("Failed to insert " #type_name " object into vector.");                 \
                reai_##type_infix##_vec_destroy (vec);                                             \
                goto INIT_FAILED;                                                                  \
            }                                                                                      \
        } else {                                                                                   \
            cJSON* arr_item = NULL;                                                                \
            cJSON_ArrayForEach (arr_item, json_arr) {                                              \
                type_name item = {0};                                                              \
                reader (arr_item, item);                                                           \
                                                                                                   \
                if (!reai_##type_infix##_vec_append ((vec), &item)) {                              \
                    PRINT_ERR ("Failed to insert " #type_name " object into vector.");             \
                    reai_##type_infix##_vec_destroy (vec);                                         \
                    goto INIT_FAILED;                                                              \
                }                                                                                  \
            }                                                                                      \
        }                                                                                          \
    } while (0)

#endif // CREAIT_API_HELPERS_JSON_GETTERS_H
