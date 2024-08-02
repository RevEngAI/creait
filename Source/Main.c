/* reai */
#include <Reai/Api/Api.h>
#include <Reai/Config.h>
#include <Reai/Db.h>
#include <Reai/Log.h>

/* sqlite */
#include <sqlite3.h>

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

    reai_set_db (reai, db);

    U64Vec *analyses = reai_db_get_all_created_analyses (db);
    REAI_VEC_FOREACH (analyses, bin_id, {
        // NOTE: the strings returend must be freed, but IDC right noow.
        PRINT_ERR (
            "bin_id = %llu\n hash = %s\n ai_model_name = %s\n status = %s\n",
            *bin_id,
            reai_db_get_analysis_binary_file_hash (db, *bin_id),
            reai_db_get_analysis_model_name (db, *bin_id),
            reai_db_get_analysis_status (db, *bin_id)
        );
    });
    reai_u64_vec_destroy (analyses);

    reai_destroy (reai);
    reai_config_destroy (cfg);

    /* create logger */
    ReaiLog *log = reai_log_create (Null);
    RETURN_VALUE_IF (!log, False, "Failed to create Reai logger.\n");

    REAI_LOG_ERROR (log, "Log Level %s\n", "Debug");
    REAI_LOG_TRACE (log, "Log Level %s\n", "TRACE");
    REAI_LOG_INFO (log, "Log Level %s %d\n", "INFO", rand());

    reai_log_destroy (log);

    return EXIT_SUCCESS;
}
