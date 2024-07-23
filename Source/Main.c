/* reai */
#include <Reai/Api/Api.h>
#include <Reai/Common.h>

/* libc */
#include <errno.h>
#include <string.h>
#include <threads.h>

/* linux kernel */
#include <sys/stat.h>

/* toml parsing */
#include <toml.h>

Size get_file_size (CString file_path) {
    RETURN_VALUE_IF (!file_path, 0, ERR_INVALID_ARGUMENTS);

    struct stat st;
    if (!stat (file_path, &st))
        return st.st_size;
    else {
        return 0;
    }
}

Int64 upload_and_create_analysis (Reai *reai, ReaiResponse *response, CString file_path) {
    RETURN_VALUE_IF (!reai || !response || !file_path, -1, ERR_INVALID_ARGUMENTS);
    PRINT_ERR (
        "------------------------------------- %s --------------------------------------\n\n\n\n",
        file_path
    );

    ReaiRequest request = {0};

    request.type                  = REAI_REQUEST_TYPE_UPLOAD_FILE;
    request.upload_file.file_path = file_path;
    reai_request (reai, &request, response);
    PRINT_ERR ("%s\n", response->raw.data);

    CString sha_256_hash = response->upload_file.sha_256_hash;
    PRINT_ERR ("HASH = %s\n", sha_256_hash);

    request.type                          = REAI_REQUEST_TYPE_CREATE_ANALYSIS;
    request.create_analysis.model         = REAI_MODEL_X86_LINUX;
    request.create_analysis.platform_opt  = Null;
    request.create_analysis.isa_opt       = Null;
    request.create_analysis.file_opt      = REAI_FILE_OPTION_ELF;
    request.create_analysis.dyn_exec      = False;
    request.create_analysis.tags          = Null;
    request.create_analysis.tags_count    = 0;
    request.create_analysis.bin_scope     = REAI_BINARY_SCOPE_PRIVATE;
    request.create_analysis.base_addr     = 0;
    request.create_analysis.functions     = Null;
    request.create_analysis.file_name     = "main";
    request.create_analysis.cmdline_args  = Null;
    request.create_analysis.priority      = 0;
    request.create_analysis.sha_256_hash  = sha_256_hash;
    request.create_analysis.debug_hash    = "1234";
    request.create_analysis.size_in_bytes = get_file_size (file_path);
    reai_request (reai, &request, response);

    Int64 binary_id = (Int64)response->create_analysis.binary_id;
    PRINT_ERR ("%s\n", response->raw.data);

    PRINT_ERR (
        "------------------------------------- %s --------------------------------------\n\n\n\n",
        file_path
    );

    return binary_id;
}


int main (int argc, char **argv) {
    RETURN_VALUE_IF (!argc, EXIT_FAILURE, ERR_INVALID_ARGUMENTS);

    FILE *fp;
    char  errbuf[200];

    // 1. Read and parse toml file
    fp = fopen ("sample.toml", "r");
    RETURN_VALUE_IF (!fp, EXIT_FAILURE, ERR_FILE_OPEN_FAILED " : %s", strerror (errno));

    toml_table_t *reai_conf = toml_parse_file (fp, errbuf, sizeof (errbuf));
    fclose (fp);
    RETURN_VALUE_IF (!reai_conf, EXIT_FAILURE, "Failed to parse toml config file.\n");

    toml_datum_t toml_api_key = toml_string_in (reai_conf, "apikey");
    RETURN_VALUE_IF (
        !toml_api_key.ok,
        EXIT_FAILURE,
        "Cannot find 'apikey' (required) in RevEngAI config.\n"
    );

    toml_datum_t toml_host = toml_string_in (reai_conf, "host");
    RETURN_VALUE_IF (
        !toml_host.ok,
        EXIT_FAILURE,
        "Cannot find 'host' (required) in RevEngAI config.\n"
    );

    Reai *reai = reai_create (toml_host.u.s, toml_api_key.u.s);
    FREE (toml_host.u.s);
    FREE (toml_api_key.u.s);

    ReaiResponse response;
    reai_response_init (&response);

    CString file_path = argv[0];
    upload_and_create_analysis (reai, &response, file_path);
    upload_and_create_analysis (reai, &response, "/usr/bin/rizin");
    Int64 binary_id = upload_and_create_analysis (
        reai,
        &response,
        "/home/misra/Desktop/RevEngAI/plugins/native/Build/Source/Rizin/libreai_rizin.so"
    );

    RETURN_VALUE_IF (binary_id == -1, EXIT_FAILURE, "Failed to upload and create binary analysis");

    PRINT_ERR (
        "------------------------------------- %s --------------------------------------\n\n\n\n",
        __FILE__
    );

    ReaiRequest request                   = {};
    request.type                          = REAI_REQUEST_TYPE_BASIC_FUNCTION_INFO;
    request.basic_function_info.binary_id = binary_id;
    reai_request (reai, &request, &response);

    if (response.type == REAI_RESPONSE_TYPE_BASIC_FUNCTION_INFO) {
        ReaiFnInfoVec *vec = response.basic_function_info.fn_infos;
        for (Size s = 0; s < vec->count; s++) {
            ReaiFnInfo *info = vec->items + s;
            PRINT_ERR (
                "\tFN_ID = %llu\n"
                "\tFN_NAME = %s\n"
                "\tFN_VADDR = %llu\n"
                "\tFN_SIZE = %llu\n\n",
                info->id,
                info->name,
                info->vaddr,
                info->size
            );
        }
    }

    PRINT_ERR ("%s\n", response.raw.data);

    PRINT_ERR (
        "------------------------------------- %s --------------------------------------\n\n\n\n",
        __FILE__
    );

    request.type = REAI_REQUEST_TYPE_RECENT_ANALYSIS;
    reai_request (reai, &request, &response);
    PRINT_ERR ("%s\n", response.raw.data);

    /* create clone if want to keep vector */
    ReaiAnalysisInfoVec *analysis_infos = Null;
    RETURN_VALUE_IF (
        !(analysis_infos =
              reai_analysis_info_vec_clone_create (response.recent_analysis.analysis_infos)),
        EXIT_FAILURE,
        "Failed to create clone of analysis info array\n"
    );

    PRINT_ERR (
        "------------------------------------- %s --------------------------------------\n\n\n\n",
        __FILE__
    );

    if (response.type == REAI_RESPONSE_TYPE_RECENT_ANALYSIS) {
        ReaiAnalysisInfoVec *vec = analysis_infos;

        /* all upcoming are delete requests*/
        request.type = REAI_REQUEST_TYPE_DELETE_ANALYSIS;

        for (Size s = 0; s < vec->count; s++) {
            ReaiAnalysisInfo *info = vec->items + s;
            PRINT_ERR ("deleting analysis for binary_id = %llu\n", info->binary_id);

            request.delete_analysis.binary_id = info->binary_id;
            reai_request (reai, &request, &response);

            PRINT_ERR ("status = %s\n", response.delete_analysis.success ? "success" : "fail");
            PRINT_ERR ("%s\n\n", response.raw.data);
        }
    }

    PRINT_ERR (
        "------------------------------------- %s --------------------------------------\n\n\n\n",
        __FILE__
    );

    reai_analysis_info_vec_destroy (analysis_infos);
    reai_response_deinit (&response);
    reai_destroy (reai);
    toml_free (reai_conf);

    return EXIT_SUCCESS;
}
