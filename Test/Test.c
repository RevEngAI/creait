#include <Reai/Api/Api.h>
#include <Reai/Common.h>
#include <Reai/Types.h>

// defined in /Test/MockApi.c
ReaiResponse* reai_mock_request (
    Reai*         reai,
    ReaiRequest*  request,
    ReaiResponse* response,
    CString       endpoint_str,
    Uint32*       http_code
);

#define TEST(name, ok_condition)                                                                   \
    if (!(ok_condition)) {                                                                         \
        fprintf (stderr, "[XX] " name "\n");                                                       \
        success = false;                                                                           \
    } else {                                                                                       \
        fprintf (stderr, "[OK] " name "\n");                                                       \
    }

int main() {
    Reai* reai = reai_create ("https://mock.reveng.api", "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX");
    reai_set_mock_handler (reai, reai_mock_request);

    ReaiResponse response = {0};
    reai_response_init (&response);
    Bool success = true;

    ReaiRequest req = {0};
    req.type        = REAI_REQUEST_TYPE_HEALTH_CHECK;
    TEST (
        "Health check",
        reai_request (reai, &req, &response) && response.type == REAI_RESPONSE_TYPE_HEALTH_CHECK
    );

    TEST (
        "Auth check",
        reai_auth_check (
            reai,
            &response,
            "https://mock.reveng.api",
            "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX"
        )
    );

    TEST (
        "File upload",
        reai_upload_file (reai, &response, "/usr/bin/cmake") &&
            response.type == REAI_RESPONSE_TYPE_UPLOAD_FILE &&
            !(response.upload_file.success ^ !!response.upload_file.sha_256_hash)
    );

    TEST (
        "Get AI models",
        reai_get_available_models (reai, &response) &&
            response.type == REAI_RESPONSE_TYPE_GET_MODELS &&
            !(response.get_models.success ^ !!response.get_models.models)
    );

    TEST (
        "Create analysis",
        reai_create_analysis (
            reai,
            &response,
            "random-ai-model",
            0xf00dc01l, // base addr
            NULL,       // fn infos
            false,
            "sha256 hash",
            "filename",
            "cmdlineargs",
            0x4000
        ) && response.type == REAI_RESPONSE_TYPE_CREATE_ANALYSIS &&
            !(response.create_analysis.success ^ !!response.create_analysis.binary_id)
    );

    TEST (
        "Basic function info",
        reai_get_basic_function_info (reai, &response, 12345) &&
            response.type == REAI_RESPONSE_TYPE_BASIC_FUNCTION_INFO &&
            !(response.basic_function_info.success ^ !!response.basic_function_info.fn_infos)
    );

    TEST (
        "Recent analysis",
        reai_get_recent_analyses (
            reai,
            &response,
            REAI_ANALYSIS_STATUS_COMPLETE,
            REAI_BINARY_SCOPE_PUBLIC,
            5
        ) && response.type == REAI_RESPONSE_TYPE_RECENT_ANALYSIS &&
            !(response.recent_analysis.success ^ !!response.recent_analysis.analysis_infos)
    );

    TEST (
        "Analysis status",
        reai_get_analysis_status (reai, &response, 12345) &&
            response.type == REAI_RESPONSE_TYPE_ANALYSIS_STATUS &&
            (response.analysis_status.status > REAI_ANALYSIS_STATUS_INVALID) &&
            (response.analysis_status.status < REAI_ANALYSIS_STATUS_MAX)
    )

    // Dummy data
    ReaiFnInfoVec fn_infos = {0};
    fn_infos.items         = (ReaiFnInfo[]) {
        {.name = "name1",   .id = 1337},
        {.name = "name2", .id = 0xc0de}
    };
    fn_infos.count    = 2;
    fn_infos.capacity = 2;
    TEST (
        "Batch renames functions",
        reai_batch_renames_functions (reai, &response, &fn_infos) &&
            response.type == REAI_RESPONSE_TYPE_BATCH_RENAMES_FUNCTIONS
    );

    TEST (
        "Rename function",
        reai_rename_function (reai, &response, 534321, "trojan.rabbit") &&
            response.type == REAI_RESPONSE_TYPE_RENAME_FUNCTION
    );

    TEST (
        "Batch binary symbol ANN",
        reai_batch_binary_symbol_ann (reai, &response, 12345, 10, 0.1, NULL, true) &&
            response.type == REAI_RESPONSE_TYPE_BATCH_BINARY_SYMBOL_ANN &&
            !(response.batch_binary_symbol_ann.success ^
              !!response.batch_binary_symbol_ann.function_matches)
    )

  // TODO fix dis
    TEST (
        "Batch function symbol ANN",
        reai_batch_function_symbol_ann (reai, &response, 54321, NULL, 10, 0.1, NULL, true) &&
            response.type == REAI_RESPONSE_TYPE_BATCH_FUNCTION_SYMBOL_ANN &&
            !(response.batch_function_symbol_ann.success ^
              !!response.batch_function_symbol_ann.function_matches)
    )

    reai_destroy (reai);
    reai = NULL;
    reai_response_deinit (&response);

    return !success;
}
