/**
 * @file JsonGetters.h
 * @date 25th Sept 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef CREAIT_API_HELPERS_JSON_GETTERS_H
#define CREAIT_API_HELPERS_JSON_GETTERS_H

#define IF_JSON_OBJECT_EXISTS(json, name) if (cJSON_GetObjectItemCaseSensitive (json, name))

#define GET_OPTIONAL_JSON_BOOL(json, name, var)                                                    \
    IF_JSON_OBJECT_EXISTS (json, name) {                                                           \
        GET_JSON_BOOL (json, name, var);                                                           \
    }                                                                                              \
    else {                                                                                         \
        var = 0;                                                                                   \
    }

#define GET_OPTIONAL_JSON_U64(json, name, var)                                                     \
    IF_JSON_OBJECT_EXISTS (json, name) {                                                           \
        GET_JSON_U64 (json, name, var);                                                            \
    }                                                                                              \
    else {                                                                                         \
        var = 0;                                                                                   \
    }

#define GET_OPTIONAL_JSON_F64(json, name, var)                                                     \
    IF_JSON_OBJECT_EXISTS (json, name) {                                                           \
        GET_JSON_F64 (json, name, var);                                                            \
    }                                                                                              \
    else {                                                                                         \
        var = 0;                                                                                   \
    }


#define GET_OPTIONAL_JSON_STRING(json, name, var)                                                  \
    IF_JSON_OBJECT_EXISTS (json, name) {                                                           \
        GET_JSON_STRING (json, name, var);                                                         \
    }                                                                                              \
    else {                                                                                         \
        var = 0;                                                                                   \
    }


#define GET_OPTIONAL_JSON_STRING_ARR(json, name, var)                                              \
    IF_JSON_OBJECT_EXISTS (json, name) {                                                           \
        GET_JSON_STRING_ARR (json, name, var);                                                     \
    }                                                                                              \
    else {                                                                                         \
        var = 0;                                                                                   \
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

/* retrieved string  array must be freed after use */
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
        GET_OPTIONAL_JSON_F64 (match, "confidence", fn_match.confidence);                          \
        GET_OPTIONAL_JSON_U64 (match, "nearest_neighbor_binary_id", fn_match.nn_binary_id);        \
        GET_OPTIONAL_JSON_STRING (match, "nearest_neighbor_binary_name", fn_match.nn_binary_name); \
        GET_OPTIONAL_JSON_BOOL (match, "nearest_neighbor_debug", fn_match.nn_debug);               \
        GET_OPTIONAL_JSON_STRING (                                                                 \
            match,                                                                                 \
            "nearest_neighbor_function_name",                                                      \
            fn_match.nn_function_name                                                              \
        );                                                                                         \
        GET_OPTIONAL_JSON_U64 (match, "nearest_neighbor_id", fn_match.nn_function_id);             \
        GET_OPTIONAL_JSON_STRING (                                                                 \
            match,                                                                                 \
            "nearest_neighbor_sha_256_hash",                                                       \
            fn_match.nn_sha_256_hash                                                               \
        );                                                                                         \
        GET_OPTIONAL_JSON_U64 (match, "origin_function_id", fn_match.origin_function_id);          \
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

#endif // CREAIT_API_HELPERS_JSON_GETTERS_H
