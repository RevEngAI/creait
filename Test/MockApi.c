#include <Reai/Api/Reai.h>
#include <Reai/Api/Response.h>

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

#define HANDLER_NAME(handler_name) reai_mock_##handler_name##_handler
#define HANDLER(handler_name, body)                                                                \
    ReaiResponse* HANDLER_NAME (handler_name) (                                                    \
        Reai * reai,                                                                               \
        ReaiRequest * request,                                                                     \
        ReaiResponse * response,                                                                   \
        CString endpoint_str,                                                                      \
        Uint32 * http_code                                                                         \
    ) {                                                                                            \
        CString host     = reai->host;                                                             \
        Size    host_len = strlen (host);                                                          \
        if (!reai || !request || !response || !endpoint_str) {                                     \
            REAI_LOG_ERROR (ERR_INVALID_ARGUMENTS);                                                \
            return NULL;                                                                           \
        }                                                                                          \
                                                                                                   \
        if (reai->host && strlen (reai->host) && strncmp (endpoint_str, host, host_len) == 0) {    \
            endpoint_str += host_len;                                                              \
            { body }                                                                               \
        } else {                                                                                   \
            REAI_LOG_ERROR ("invalid host or endpoint");                                           \
            *http_code = 404;                                                                      \
            return NULL;                                                                           \
        }                                                                                          \
                                                                                                   \
        return response;                                                                           \
    }


#define RESPONSE_SET(raw_data)                                                                     \
    do {                                                                                           \
        response->raw.data     = strdup (raw_data);                                                \
        response->raw.length   = strlen (response->raw.data);                                      \
        response->raw.capacity = response->raw.length;                                             \
    } while (0)


HANDLER (todo, {
    REAI_LOG_ERROR ("todo!");
    RESPONSE_SET ("{\"detail\": \"Not Implemented\"}");
    *http_code = 404;
});


HANDLER (health_check, {
    if (!strcmp (endpoint_str, "/v1")) {
        RESPONSE_SET ("{\"success\":true,\"message\":\"Welcome to RevEng.AI's API!\"}");
        *http_code = 200;
    } else {
        REAI_LOG_ERROR ("invalid health check endpoint");
        RESPONSE_SET ("{\"detail\":\"Not Found\"}");
        *http_code = 404;
    }
});


HANDLER (auth_check, {
    if (!strcmp (endpoint_str, "/v1/authenticate")) {
        RESPONSE_SET ("{\"message\":\"User has been authenticated!\"}");
        *http_code = 200;
    } else {
        REAI_LOG_ERROR ("invalid auth check endpoint");
        RESPONSE_SET ("{\"detail\":\"Not Found\"}");
        *http_code = 404;
    }
});


HANDLER (upload_file, {
    if (!strcmp (endpoint_str, "/v1/upload")) {
        // file is required and is provided in headers as multiplart/form-data
        if (request->upload_file.file_path && strlen (request->upload_file.file_path)) {
            RESPONSE_SET (
                "{\"success\": true, \"message\": \"File successfully uploaded!\", "
                "\"sha_256_hash\": "
                "\"a1b2c3d4e5f6g7h8i9j0a1b2c3d4e5f6g7h8i9j0a1b2c3d4e5f6g7h8i9j0\"}"
            );
        } else {
            REAI_LOG_ERROR ("file parameter in header is required");
            RESPONSE_SET ("{\"detail\":\"Method Not Allowed\"}");
        }
        *http_code = 200;
    } else {
        REAI_LOG_ERROR ("invalid upload endpoint");
        RESPONSE_SET ("{\"detail\":\"Not Found\"}");
        *http_code = 404;
    }
});


HANDLER (get_models, {
    if (!strcmp (endpoint_str, "/v1/models")) {
        RESPONSE_SET (
            "{\"success\":true,\"models\":[{\"model_id\":4,\"model_name\":\"binnet-0.3-x86-"
            "windows\"},{\"model_id\":5,\"model_name\":\"binnet-0.3-x86-linux\"},{\"model_id\":6,"
            "\"model_name\":\"binnet-0.3-x86-macos\"},{\"model_id\":7,\"model_name\":\"binnet-0.3-"
            "x86-android\"},{\"model_id\":8,\"model_name\":\"binnet-0.4-x86-windows\"},{\"model_"
            "id\":9,\"model_name\":\"binnet-0.4-x86-linux\"},{\"model_id\":10,\"model_name\":"
            "\"binnet-0.4-x86-macos\"},{\"model_id\":11,\"model_name\":\"binnet-0.4-x86-android\"}]"
            "}"
        );
        *http_code = 200;
    } else {
        REAI_LOG_ERROR ("invalid get models endpoint");
        RESPONSE_SET ("{\"detail\":\"Not Found\"}");
        *http_code = 404;
    }
});


HANDLER (create_analysis, {
    if (!strcmp (endpoint_str, "/v1/analyse/")) {
        if (request->create_analysis.ai_model && request->create_analysis.sha_256_hash &&
            strlen (request->create_analysis.ai_model) &&
            strlen (request->create_analysis.sha_256_hash) &&
            request->create_analysis.size_in_bytes) {
            RESPONSE_SET ("{\"success\":true,\"binary_id\":12345}");
            *http_code = 201;
        } else {
            REAI_LOG_ERROR ("missing required json parameters");
            RESPONSE_SET ("{\"detail\":\"Method Not Allowed\"}");
            *http_code = 422;
        }
    } else {
        REAI_LOG_ERROR ("invalid create analysis endpoint");
        RESPONSE_SET ("{\"detail\":\"Not Found\"}");
        *http_code = 404;
    }
});


HANDLER (delete_analysis, {
    CString base     = "/v1/analyse/";
    Size    base_len = strlen (base);
    if (!strncmp (endpoint_str, base, base_len)) {
        // skip to where binary id is present as path parameter
        endpoint_str += base_len;

        // get binary id path parameter
        char*        end    = NULL;
        ReaiBinaryId bin_id = strtoull (endpoint_str, &end, 10);

        // check binary id is valid and that there's no extra information available
        if (!bin_id || strlen (end)) {
            REAI_LOG_ERROR ("invalid delete analysis request : failed to parse binary id");
            RESPONSE_SET ("{\"detail\":\"Method Not Allowed\"}");
            *http_code = 405;
        } else {
            RESPONSE_SET ("{\"success\":true,\"msg\":\"Binary Deleted!\"}");
            *http_code = 200;
        }
    } else {
        REAI_LOG_ERROR ("invalid delete analysis endpoint");
        RESPONSE_SET ("{\"detail\":\"Not Found\"}");
        *http_code = 404;
    }
});


HANDLER (basic_function_info, {
    CString base     = "/v1/analyse/functions/";
    Size    base_len = strlen (base);
    if (!strncmp (endpoint_str, base, base_len)) {
        // skip to where binary id is present as path parameter
        endpoint_str += base_len;

        // get binary id path parameter
        char*        end    = NULL;
        ReaiBinaryId bin_id = strtoull (endpoint_str, &end, 10);

        // check binary id is valid and that there's no extra information available
        if (!bin_id || strlen (end)) {
            REAI_LOG_ERROR ("invalid basic function info request : failed to parse binary id");
            RESPONSE_SET ("{\"detail\":\"Method Not Allowed\"}");
            *http_code = 405;
        } else {
            RESPONSE_SET (
                "{\"success\":true,\"functions\":[{\"function_id\":3506902,\"function_name\":"
                "\"init\",\"function_size\":771,\"function_vaddr\":40640},{\"function_id\":3506903,"
                "\"function_name\":\"factory\",\"function_size\":27,\"function_vaddr\":41472},{"
                "\"function_id\":3506904,\"function_name\":\"qt_plugin_instance\",\"function_"
                "size\":186,\"function_vaddr\":41504}]}"
            );
            *http_code = 200;
        }
    } else {
        REAI_LOG_ERROR ("invalid basic function info endpoint");
        RESPONSE_SET ("{\"detail\":\"Not Found\"}");
        *http_code = 404;
    }
});


HANDLER (recent_analysis, {
    if (!strcmp (endpoint_str, "/v1/analyse/recent")) {
        RESPONSE_SET (
            "{\"success\": true, \"analysis\": {\"binary_id\": 864, \"binary_name\": "
            "\"8d05425e8e4be4c17de915a85bc1642cf2177ed6a35ffa5ad0655bdd565853bc.elf\", "
            "\"creation\": \"Sat, 19 Aug 2023 22:08:45 GMT\", \"model_id\": 1, \"model_name\": "
            "\"binnet-0.2-x86-linux\", \"sha_256_hash\": "
            "\"8d05425e8e4be4c17de915a85bc1642cf2177ed6a35ffa5ad0655bdd565853bc\", \"status\": "
            "\"Complete\"}}"
        );
        *http_code = 200;
    } else {
        REAI_LOG_ERROR ("invalid recent analysis endpoint");
        RESPONSE_SET ("{\"detail\":\"Not Found\"}");
        *http_code = 404;
    }
});


HANDLER (analysis_status, {
    CString base     = "/v1/analyse/status/";
    Size    base_len = strlen (base);
    if (!strncmp (endpoint_str, base, base_len)) {
        // skip to where binary id is present as path parameter
        endpoint_str += base_len;

        // get binary id path parameter
        char*        end    = NULL;
        ReaiBinaryId bin_id = strtoull (endpoint_str, &end, 10);

        // check binary id is valid and that there's no extra information available
        if (!bin_id || strlen (end)) {
            REAI_LOG_ERROR ("invalid analysis status request : failed to parse binary id");
            RESPONSE_SET ("{\"detail\":\"Method Not Allowed\"}");
            *http_code = 405;
        } else {
            RESPONSE_SET ("{\"success\": true, \"status\": \"Complete\"}");
            *http_code = 200;
        }
    } else {
        REAI_LOG_ERROR ("invalid analysis status endpoint");
        RESPONSE_SET ("{\"detail\":\"Not Found\"}");
        *http_code = 404;
    }
});


HANDLER (search, {
    if (!strcmp (endpoint_str, "/v1/search")) {
        RESPONSE_SET (
            "{\"success\": true, \"query_results\": {\"binary_id\": 18031, \"binary_name\": "
            "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\", "
            "\"collections\": [\"trojan\", \"ransomware\", \"clop\", \"chapak\", \"hydracrypt\", "
            "\"Clop\", \"exe\"], \"creation\": \"2024-05-14T16:03:31.009311\", \"model_id\": 4, "
            "\"model_name\": \"binnet-0.3-x86-windows\", \"sha_256_hash\": "
            "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\", \"status\": "
            "\"Complete\", \"tags\": [\"trojan\", \"ransomware\", \"clop\", \"chapak\", "
            "\"hydracrypt\", \"Clop\", \"exe\"]}}"
        );
        *http_code = 200;
    } else {
        REAI_LOG_ERROR ("invalid search endpoint");
        RESPONSE_SET ("{\"detail\":\"Not Found\"}");
        *http_code = 404;
    }
});


HANDLER (batch_renames_functions, {
    if (!strcmp (endpoint_str, "/v1/functions/batch/rename")) {
        if (request->batch_renames_functions.new_name_mapping &&
            request->batch_renames_functions.new_name_mapping->count) {
            RESPONSE_SET ("");
            *http_code = 200;
        } else {
            REAI_LOG_ERROR ("required field missing");
            RESPONSE_SET ("{\"success\": false, \"detail\":\"Required field missing\"}");
            *http_code = 422;
        }
    } else {
        REAI_LOG_ERROR ("invalid batch renames functions endpoint");
        RESPONSE_SET ("{\"detail\":\"Not Found\"}");
        *http_code = 404;
    }
});


HANDLER (rename_function, {
    CString base     = "/v1/functions/rename/";
    Size    base_len = strlen (base);
    if (!strncmp (endpoint_str, base, base_len)) {
        // skip to where function id is present as path parameter
        endpoint_str += base_len;

        // get function id path parameter
        char*          end    = NULL;
        ReaiFunctionId fun_id = strtoull (endpoint_str, &end, 10);

        // check function id is valid and that there's no extra information available
        if (!fun_id || strlen (end)) {
            REAI_LOG_ERROR ("invalid function rename request : failed to parse function id");
            RESPONSE_SET ("{\"detail\":\"Method Not Allowed\"}");
            *http_code = 405;
        } else {
            RESPONSE_SET ("{\"success\": true, \"msg\": \"Done!\"}");
            *http_code = 200;
        }
    } else {
        REAI_LOG_ERROR ("invalid rename function endpoint");
        RESPONSE_SET ("{\"detail\":\"Not Found\"}");
        *http_code = 404;
    }
});


HANDLER (batch_binary_symbol_ann, {
    CString base     = "/v1/ann/symbol/";
    Size    base_len = strlen (base);
    if (!strncmp (endpoint_str, base, base_len)) {
        // skip to where binary id is present as path parameter
        endpoint_str += base_len;

        // get binary id path parameter
        char*        end    = NULL;
        ReaiBinaryId bin_id = strtoull (endpoint_str, &end, 10);

        // check binary id is valid and that there's no extra information available
        if (!bin_id || strlen (end)) {
            REAI_LOG_ERROR ("invalid batch binary symbol ann request : failed to parse binary id");
            RESPONSE_SET ("{\"detail\":\"Method Not Allowed\"}");
            *http_code = 405;
        } else {
            RESPONSE_SET (
                "{\"success\": true, \"settings\": {\"collection\": [], \"debug_mode\": false, "
                "\"distance\": 0.1, \"result_per_function\": 5}, \"function_matches\": "
                "{\"confidence\": 0.809873463561612, \"nearest_neighbor_binary_id\": 17882, "
                "\"nearest_neighbor_binary_name\": "
                "\"x86-64_libsodium_1.0.19_libsodium.so.26.1.0_O3_"
                "15786002d4e406781b138cae220192ecbd9ef6e3e9f795e3ed3b0011c087d86b\", "
                "\"nearest_neighbor_debug\": true, \"nearest_neighbor_function_name\": "
                "\"randombytes_stir\", \"nearest_neighbor_id\": 3517104, "
                "\"nearest_neighbor_sha_256_hash\": "
                "\"15786002d4e406781b138cae220192ecbd9ef6e3e9f795e3ed3b0011c087d86b\", "
                "\"origin_function_id\": 3506881}}"
            );
            *http_code = 200;
        }
    } else {
        REAI_LOG_ERROR ("invalid rename function endpoint");
        RESPONSE_SET ("{\"detail\":\"Not Found\"}");
        *http_code = 404;
    }
});


HANDLER (batch_function_symbol_ann, {
    if (!strcmp (endpoint_str, "/v1/ann/symbol/batch")) {
        if (request->batch_function_symbol_ann.function_ids &&
            request->batch_function_symbol_ann.function_id_count) {
            RESPONSE_SET (
                "{\"success\":true,\"settings\":{\"collection\":[],\"debug_mode\":false,"
                "\"distance\":0."
                "1,\"result_per_function\":5},\"function_matches\":{\"function_matches\":[{"
                "\"confidence\":1,\"nearest_neighbor_binary_id\":18160,\"nearest_neighbor_binary_"
                "name\":\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\","
                "\"nearest_"
                "neighbor_debug\":true,\"nearest_neighbor_function_name\":\"__realloc_base\","
                "\"nearest_"
                "neighbor_id\":3641806,\"nearest_neighbor_sha_256_hash\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"origin_"
                "function_id\":3614690},{\"confidence\":1,\"nearest_neighbor_binary_id\":18160,"
                "\"nearest_neighbor_binary_name\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"nearest_"
                "neighbor_debug\":true,\"nearest_neighbor_function_name\":\"operator()<class_<"
                "lambda_"
                "2866be3712abc81a800a822484c830d8>,class_<lambda_39ca0ed439415581b5b15c265174cece>&"
                ","
                "class_<lambda_2b24c74d71094a6cd0cb82e44167d71b>_>\",\"nearest_neighbor_id\":"
                "3641808,"
                "\"nearest_neighbor_sha_256_hash\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"origin_"
                "function_id\":3614692},{\"confidence\":1,\"nearest_neighbor_binary_id\":18160,"
                "\"nearest_neighbor_binary_name\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"nearest_"
                "neighbor_debug\":false,\"nearest_neighbor_function_name\":\"FUN_0003c1f5\","
                "\"nearest_"
                "neighbor_id\":3641809,\"nearest_neighbor_sha_256_hash\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"origin_"
                "function_id\":3614693},{\"confidence\":1,\"nearest_neighbor_binary_id\":18160,"
                "\"nearest_neighbor_binary_name\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"nearest_"
                "neighbor_debug\":false,\"nearest_neighbor_function_name\":\"FUN_0003c288\","
                "\"nearest_"
                "neighbor_id\":3641810,\"nearest_neighbor_sha_256_hash\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"origin_"
                "function_id\":3614694},{\"confidence\":1,\"nearest_neighbor_binary_id\":18160,"
                "\"nearest_neighbor_binary_name\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"nearest_"
                "neighbor_debug\":true,\"nearest_neighbor_function_name\":\"__acrt_lowio_lock_fh_"
                "and_"
                "call<class_<lambda_6978c1fb23f02e42e1d9e99668cc68aa>_>\",\"nearest_neighbor_id\":"
                "3641820,\"nearest_neighbor_sha_256_hash\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"origin_"
                "function_id\":3614695},{\"confidence\":1,\"nearest_neighbor_binary_id\":18160,"
                "\"nearest_neighbor_binary_name\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"nearest_"
                "neighbor_debug\":false,\"nearest_neighbor_function_name\":\"FUN_0003c2d8\","
                "\"nearest_"
                "neighbor_id\":3641812,\"nearest_neighbor_sha_256_hash\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"origin_"
                "function_id\":3614696},{\"confidence\":1,\"nearest_neighbor_binary_id\":18160,"
                "\"nearest_neighbor_binary_name\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"nearest_"
                "neighbor_debug\":false,\"nearest_neighbor_function_name\":\"FUN_0003c317\","
                "\"nearest_"
                "neighbor_id\":3641813,\"nearest_neighbor_sha_256_hash\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"origin_"
                "function_id\":3614697},{\"confidence\":1,\"nearest_neighbor_binary_id\":18160,"
                "\"nearest_neighbor_binary_name\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"nearest_"
                "neighbor_debug\":false,\"nearest_neighbor_function_name\":\"FUN_0003c349\","
                "\"nearest_"
                "neighbor_id\":3641814,\"nearest_neighbor_sha_256_hash\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"origin_"
                "function_id\":3614698},{\"confidence\":1,\"nearest_neighbor_binary_id\":18160,"
                "\"nearest_neighbor_binary_name\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"nearest_"
                "neighbor_debug\":true,\"nearest_neighbor_function_name\":\"___acrt_stdio_flush_"
                "nolock\",\"nearest_neighbor_id\":3641815,\"nearest_neighbor_sha_256_hash\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"origin_"
                "function_id\":3614699},{\"confidence\":1,\"nearest_neighbor_binary_id\":18160,"
                "\"nearest_neighbor_binary_name\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"nearest_"
                "neighbor_debug\":true,\"nearest_neighbor_function_name\":\"__fflush_nolock\","
                "\"nearest_neighbor_id\":3641816,\"nearest_neighbor_sha_256_hash\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"origin_"
                "function_id\":3614700},{\"confidence\":1,\"nearest_neighbor_binary_id\":18160,"
                "\"nearest_neighbor_binary_name\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"nearest_"
                "neighbor_debug\":false,\"nearest_neighbor_function_name\":\"FUN_0003c41f\","
                "\"nearest_"
                "neighbor_id\":3641817,\"nearest_neighbor_sha_256_hash\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"origin_"
                "function_id\":3614701},{\"confidence\":1,\"nearest_neighbor_binary_id\":18160,"
                "\"nearest_neighbor_binary_name\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"nearest_"
                "neighbor_debug\":true,\"nearest_neighbor_function_name\":\"___acrt_stdio_free_"
                "buffer_"
                "nolock\",\"nearest_neighbor_id\":3641818,\"nearest_neighbor_sha_256_hash\":"
                "\"f55e4553ed04f085bc5b93cf7e5386c3d938abcb13a95ef1d2c3f8b21c454db8\",\"origin_"
                "function_id\":3614702}}]}"
            );
            *http_code = 200;
        } else {
            REAI_LOG_ERROR ("required field missing");
            RESPONSE_SET ("{\"success\": false, \"detail\":\"Required field missing\"}");
            *http_code = 422;
        }
    } else {
        REAI_LOG_ERROR ("invalid search endpoint");
        RESPONSE_SET ("{\"detail\":\"Not Found\"}");
        *http_code = 404;
    }
});


ReaiResponse* reai_mock_request (
    Reai*         reai,
    ReaiRequest*  request,
    ReaiResponse* response,
    CString       endpoint_str,
    Uint32*       http_code
) {
    if (!reai || !request || !response || !endpoint_str || !http_code) {
        REAI_LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    if (response->raw.data) {
        FREE (response->raw.data);
        response->raw.data     = NULL;
        response->raw.length   = 0;
        response->raw.capacity = 0;
    }

#define FORWARD(handler)                                                                           \
    return HANDLER_NAME (handler) (reai, request, response, endpoint_str, http_code)

    switch (request->type) {
        case REAI_REQUEST_TYPE_HEALTH_CHECK :
            FORWARD (health_check);

        case REAI_REQUEST_TYPE_AUTH_CHECK :
            FORWARD (auth_check);

        case REAI_REQUEST_TYPE_UPLOAD_FILE :
            FORWARD (upload_file);

        case REAI_REQUEST_TYPE_GET_MODELS :
            FORWARD (get_models);

        case REAI_REQUEST_TYPE_CREATE_ANALYSIS :
            FORWARD (create_analysis);

        case REAI_REQUEST_TYPE_DELETE_ANALYSIS :
            FORWARD (delete_analysis);

        case REAI_REQUEST_TYPE_BASIC_FUNCTION_INFO :
            FORWARD (basic_function_info);

        case REAI_REQUEST_TYPE_RECENT_ANALYSIS :
            FORWARD (recent_analysis);

        case REAI_REQUEST_TYPE_ANALYSIS_STATUS :
            FORWARD (analysis_status);

        case REAI_REQUEST_TYPE_SEARCH :
            FORWARD (search);

        case REAI_REQUEST_TYPE_BATCH_RENAMES_FUNCTIONS :
            FORWARD (batch_renames_functions);

        case REAI_REQUEST_TYPE_RENAME_FUNCTION :
            FORWARD (rename_function);

        case REAI_REQUEST_TYPE_BATCH_BINARY_SYMBOL_ANN :
            FORWARD (batch_binary_symbol_ann);

        case REAI_REQUEST_TYPE_BATCH_FUNCTION_SYMBOL_ANN :
            FORWARD (batch_function_symbol_ann);

        case REAI_REQUEST_TYPE_BEGIN_AI_DECOMPILATION :
            FORWARD (todo);

        case REAI_REQUEST_TYPE_POLL_AI_DECOMPILATION :
            FORWARD (todo);

        default :
            PRINT_ERR ("Invalid request.");
            RESPONSE_SET ("{\"detail\":\"Not Found\"}");
            *http_code = 404;
            return response;
    }
}
