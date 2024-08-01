/**
 * @file Request.c
 * @date 10th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

/* reai */
#include <Reai/Api/Request.h>

/* cjson */
#include <cJSON.h>

/* libc */
#include <memory.h>

#include "Reai/Util/Vec.h"

static CString reai_model_to_name[] = {
    [REAI_MODEL_BINNET_0_3_X86_WINDOWS] = "binnet-0.3-x86-windows",
    [REAI_MODEL_BINNET_0_3_X86_LINUX]   = "binnet-0.3-x86-linux",
    [REAI_MODEL_BINNET_0_3_X86_MACOS]   = "binnet-0.3-x86-macos",
    [REAI_MODEL_BINNET_0_3_X86_ANDROID] = "binnet-0.3-x86-android",
};

static CString reai_file_opt_to_str[] = {
    [REAI_FILE_OPTION_PE]    = "PE",
    [REAI_FILE_OPTION_ELF]   = "ELF",
    [REAI_FILE_OPTION_MACHO] = "MACHO",
    [REAI_FILE_OPTION_RAW]   = "RAW",
    [REAI_FILE_OPTION_EXE]   = "EXE",
    [REAI_FILE_OPTION_DLL]   = "DLL",
};

#define JSON_ADD_STRING(cj, item_name, value)                                                      \
    {                                                                                              \
        cJSON* e = cJSON_CreateString (value);                                                     \
        GOTO_HANDLER_IF (!e, CONVERSION_FAILED, "Failed to convert " item_name " to JSON\n");      \
        cJSON_AddItemToObject (cj, item_name, e);                                                  \
    }

#define JSON_ADD_STRING_ARR(cj, item_name, values, value_count)                                    \
    {                                                                                              \
        cJSON* earr = cJSON_CreateArray();                                                         \
        GOTO_HANDLER_IF (                                                                          \
            !earr,                                                                                 \
            CONVERSION_FAILED,                                                                     \
            "Failed to create JSON array for strings array\n"                                      \
        );                                                                                         \
        cJSON_AddItemToObject (cj, item_name, earr);                                               \
                                                                                                   \
        for (Size s = 0; s < value_count; s++) {                                                   \
            cJSON* e = cJSON_CreateString (values[s]);                                             \
            GOTO_HANDLER_IF (!e, CONVERSION_FAILED, "Failed to convert " item_name " to JSON\n");  \
            cJSON_AddItemToArray (earr, e);                                                        \
        }                                                                                          \
    }

#define JSON_ADD_BOOL(cj, item_name, value)                                                        \
    {                                                                                              \
        cJSON* e = cJSON_CreateBool (!!value);                                                     \
        GOTO_HANDLER_IF (!e, CONVERSION_FAILED, "Failed to convert " item_name " to JSON\n");      \
        cJSON_AddItemToObject (cj, item_name, e);                                                  \
    }

#define JSON_ADD_NUMBER(cj, item_name, value)                                                      \
    {                                                                                              \
        cJSON* e = cJSON_CreateNumber (value);                                                     \
        GOTO_HANDLER_IF (!e, CONVERSION_FAILED, "Failed to convert " item_name " to JSON\n");      \
        cJSON_AddItemToObject (cj, item_name, e);                                                  \
    }

/**
 * @b Convert given request to json.
 *
 * The returned string is completely owned by the caller and must be freed
 * after use. Don't use this method for requests that are not convertible
 * to json (like health-check, auth-check, upload etc...).
 *
 * @param[in] request
 *
 * @return @c CString containing request data in json format.
 * @return @c Null if json is empty or on failure.
 * */
HIDDEN CString reai_request_to_json_cstr (ReaiRequest* request) {
    RETURN_VALUE_IF (!request, Null, ERR_INVALID_ARGUMENTS);

    cJSON* json = cJSON_CreateObject();
    GOTO_HANDLER_IF (!json, CONVERSION_FAILED, "Failed to create JSON\n");

    switch (request->type) {
        case REAI_REQUEST_TYPE_CREATE_ANALYSIS : {
            GOTO_HANDLER_IF (
                (request->create_analysis.model == REAI_MODEL_BINNET_0_3_UNKNOWN ||
                 request->create_analysis.model >= REAI_MODEL_BINNET_0_3_MAX),
                CONVERSION_FAILED,
                "Required field model is invalid\n"
            );

            /* required field */
            CString model_name = reai_model_to_name[request->create_analysis.model];
            JSON_ADD_STRING (json, "model_name", model_name);

            /* optional field */
            if (request->create_analysis.platform_opt) {
                JSON_ADD_STRING (json, "platform_options", request->create_analysis.platform_opt);
            }

            /* optional field */
            if (request->create_analysis.isa_opt) {
                JSON_ADD_STRING (json, "isa_options", request->create_analysis.isa_opt);
            }

            /* optional field */
            if (request->create_analysis.file_opt) {
                GOTO_HANDLER_IF (
                    request->create_analysis.file_opt >= REAI_FILE_OPTION_MAX,
                    CONVERSION_FAILED,
                    "Invalid file option\n"
                );

                CString file_opt = reai_file_opt_to_str[request->create_analysis.file_opt];
                JSON_ADD_STRING (json, "file_options", file_opt);
            }

            JSON_ADD_BOOL (json, "dynamic_execution", request->create_analysis.dyn_exec);

            /* optional */
            if (request->create_analysis.tags) {
                JSON_ADD_STRING_ARR (
                    json,
                    "tags",
                    request->create_analysis.tags,
                    request->create_analysis.tags_count
                );
            }

            /* optional */
            if (request->create_analysis.bin_scope) {
                GOTO_HANDLER_IF (
                    request->create_analysis.bin_scope >= REAI_BINARY_SCOPE_MAX,
                    CONVERSION_FAILED,
                    "Invalid binary scope\n"
                );

                CString bin_scope =
                    request->create_analysis.bin_scope == REAI_BINARY_SCOPE_PRIVATE ? "PRIVATE" :
                                                                                      "PUBLIC";

                JSON_ADD_STRING (json, "binary_scope", bin_scope);
            }

            /* optional */
            if (request->create_analysis.base_addr) {
                /* create new object for symbol info */
                cJSON* symbols = cJSON_CreateObject();
                GOTO_HANDLER_IF (!symbols, CONVERSION_FAILED, "Failed to create JSON object\n");
                cJSON_AddItemToObject (json, "symbols", symbols);

                /* add base addr info to symbols obect */
                JSON_ADD_NUMBER (symbols, "base_addr", request->create_analysis.base_addr);

                /* optionally if function infos are provided  */
                if (request->create_analysis.functions) {
                    /* create array for function infos and add to symbols object */
                    cJSON* functions = cJSON_CreateArray();
                    GOTO_HANDLER_IF (
                        !functions,
                        CONVERSION_FAILED,
                        "Failed to create JSON array\n"
                    );
                    cJSON_AddItemToObject (symbols, "functions", functions);

                    /* add all function info to functions array */
                    for (Size s = 0; s < request->create_analysis.functions->count; s++) {
                        cJSON* fn = cJSON_CreateObject();

                        ReaiFnInfo* fninfo = request->create_analysis.functions->items + s;

                        /* function ID ignored here */
                        JSON_ADD_STRING (fn, "name", fninfo->name)
                        JSON_ADD_NUMBER (fn, "start_addr", fninfo->vaddr)
                        JSON_ADD_NUMBER (fn, "end_addr", fninfo->vaddr + fninfo->size)

                        cJSON_AddItemToArray (functions, fn);
                    }
                }
            }


            /* optional */
            if (request->create_analysis.file_name) {
                JSON_ADD_STRING (json, "file_name", request->create_analysis.file_name);
            }

            /* optional */
            if (request->create_analysis.cmdline_args) {
                JSON_ADD_STRING (json, "command_line_args", request->create_analysis.cmdline_args);
            }

            JSON_ADD_NUMBER (json, "priority", request->create_analysis.priority);

            /* optional */
            if (request->create_analysis.sha_256_hash) {
                JSON_ADD_STRING (json, "sha_256_hash", request->create_analysis.sha_256_hash);
            }

            /* optional */
            if (request->create_analysis.debug_hash) {
                JSON_ADD_STRING (json, "debug_hash", request->create_analysis.debug_hash);
            }

            /* required */
            GOTO_HANDLER_IF (
                !request->create_analysis.size_in_bytes,
                CONVERSION_FAILED,
                "File size is a required field. Cannot be zero.\n"
            );

            JSON_ADD_NUMBER (json, "size_in_bytes", request->create_analysis.size_in_bytes);

            break;
        }

        case REAI_REQUEST_TYPE_RECENT_ANALYSIS : {
            if (request->recent_analysis.status) {
                RETURN_VALUE_IF (
                    request->recent_analysis.status >= REAI_ANALYSIS_STATUS_MAX,
                    Null,
                    "Invalid analysis status\n"
                );

                JSON_ADD_STRING (
                    json,
                    "status",
                    reai_analysis_status_to_cstr (request->recent_analysis.status)
                );
            }

            if (request->recent_analysis.scope) {
                CString bin_scope =
                    request->create_analysis.bin_scope == REAI_BINARY_SCOPE_PRIVATE ? "PRIVATE" :
                                                                                      "PUBLIC";
                JSON_ADD_STRING (json, "scope", bin_scope);
            }

            if (request->recent_analysis.count) {
                JSON_ADD_NUMBER (json, "n", request->recent_analysis.count);
            }

            break;
        }

        case REAI_REQUEST_TYPE_SEARCH : {
            if (request->search.sha_256_hash) {
                JSON_ADD_STRING (json, "sha_256_hash", request->search.sha_256_hash);
            }
            if (request->search.binary_name) {
                JSON_ADD_STRING (json, "binary_name", request->search.binary_name);
            }
            if (request->search.collection_name) {
                JSON_ADD_STRING (json, "collection_name", request->search.collection_name);
            }
            if (request->search.state) {
                JSON_ADD_STRING (json, "state", request->search.state);
            }
            break;
        }

        case REAI_REQUEST_TYPE_BATCH_RENAMES_FUNCTIONS : {
            if (request->batch_renames_functions.new_name_mapping) {
                /* create new json array to store mappings */
                cJSON* mapping_arr = cJSON_CreateObject();
                GOTO_HANDLER_IF (
                    !mapping_arr,
                    CONVERSION_FAILED,
                    "Failed to create JSON object.\n"
                );
                cJSON_AddItemToObject (json, "new_name_mapping", mapping_arr);

                /* create new id-name pair for each rename in batch rename */
                REAI_VEC_FOREACH (request->batch_renames_functions.new_name_mapping, function, {
                    /* create a new object for each function to store id-name pairs */
                    cJSON* new_map = cJSON_CreateObject();
                    GOTO_HANDLER_IF (
                        !new_map,
                        CONVERSION_FAILED,
                        "Failed to create JSON object.\n"
                    );
                    cJSON_AddItemToArray (mapping_arr, new_map);

                    /* create map */
                    JSON_ADD_NUMBER (new_map, "function_id", function->id);
                    if (function->name) {
                        JSON_ADD_STRING (new_map, "function_name", function->name);
                    } else {
                        PRINT_ERR (
                            "Function name is required for each rename : id = %llu.\n",
                            function->id
                        );
                    }
                });
            } else {
                PRINT_ERR ("'new_name_mapping' is a required field for batch rename.\n");
            }
            break;
        }

        case REAI_REQUEST_TYPE_RENAME_FUNCTION : {
            if (request->rename_function.new_name) {
                JSON_ADD_STRING (json, "new_name", request->rename_function.new_name);
            } else {
                PRINT_ERR (
                    "'new_name' is a requred field for function name : id = %llu.\n",
                    request->rename_function.function_id
                );
            }

            break;
        }

        default : {
            GOTO_HANDLER_IF_REACHED (
                CONVERSION_FAILED,
                "Cannot generate json with given request type.\n"
            );
        }
    }

    /* get json as string and destroy json object */
    CString json_str = cJSON_Print (json);
    cJSON_Delete (json);

    return json_str;

CONVERSION_FAILED:
    if (json) {
        cJSON_Delete (json);
    }
    return Null;
}
