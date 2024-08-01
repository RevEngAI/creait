/* reai */
#include <Reai/Api/Api.h>
#include <Reai/Config.h>
#include <Reai/Db.h>

/* sqlite */
#include <sqlite3.h>

#include "Reai/Api/Request.h"
#include "Reai/Util/Vec.h"

Size file_size (CString file_name) {
    RETURN_VALUE_IF (!file_name, 0, ERR_INVALID_ARGUMENTS);

    FILE *f = fopen (file_name, "r");

    fseek (f, 0, SEEK_END);
    Size size = ftell (f);
    fseek (f, 0, SEEK_SET);

    fclose (f);

    return size;
}

int main (int argc, char **argv) {
    RETURN_VALUE_IF (!argc || !argv, EXIT_FAILURE, ERR_INVALID_ARGUMENTS);

    ReaiConfig *cfg = reai_config_load (Null);
    RETURN_VALUE_IF (!cfg, EXIT_FAILURE, "Configuration load failure.\n");

    Reai *reai = reai_create (cfg->host, cfg->apikey);

    Char db_path[64] = {0};
    snprintf (db_path, sizeof (db_path) - 1, "%s/reai.db", cfg->db_dir_path);
    ReaiDb *db = reai_db_create (db_path);
    RETURN_VALUE_IF (!db, EXIT_FAILURE, "Failed to create database.\n");

    ReaiResponse response;
    reai_response_init (&response);

    RETURN_VALUE_IF (
        !reai_upload_file (reai, &response, argv[0]),
        EXIT_FAILURE,
        "Failed to make request to upload file.\n"
    );
    RETURN_VALUE_IF (
        !reai_db_add_upload (db, argv[0], response.upload_file.sha_256_hash),
        EXIT_FAILURE,
        "Failed to add uploaded file to database.\n"
    );

    CStrVec *hashes = reai_db_get_hashes_for_file_path (db, argv[0]);
    RETURN_VALUE_IF (!hashes, EXIT_FAILURE, "No hashes found??\n");
    REAI_VEC_FOREACH (hashes, hash, {
        CString upload_time = reai_db_get_upload_time (db, *hash);
        PRINT_ERR ("%s @ %s\n", *hash, upload_time);
        FREE (upload_time);
    });
    reai_cstr_vec_destroy (hashes);

    CString latest_hash = reai_db_get_latest_hash_for_file_path (db, argv[0]);

    reai_create_analysis (
        reai,
        &response,
        REAI_MODEL_BINNET_0_3_X86_LINUX,
        Null,
        True,
        latest_hash,
        argv[0],
        Null,
        file_size (argv[0])
    );

    reai_db_add_analysis (
        db,
        response.create_analysis.binary_id,
        latest_hash,
        REAI_MODEL_BINNET_0_3_X86_LINUX,
        argv[0],
        Null
    );

    U64Vec *analyses = reai_db_get_analyses_created_for_binary (db, latest_hash);
    REAI_VEC_FOREACH (analyses, bin_id, {
        // NOTE: the strings returend must be freed, but IDC right noow.
        PRINT_ERR (
            "bin_id = %llu\n hash = %s\n ai_model_name = %s\n",
            *bin_id,
            reai_db_get_analysis_binary_file_hash (db, *bin_id),
            reai_db_get_analysis_model_name (db, *bin_id)
        );
    });
    reai_u64_vec_destroy (analyses);

    reai_db_destroy (db);
    reai_destroy (reai);
    reai_response_deinit (&response);
    reai_config_destroy (cfg);

    return EXIT_SUCCESS;
}
