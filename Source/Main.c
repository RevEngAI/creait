/* reai */
#include <Reai/Api/Api.h>
#include <Reai/Config.h>
#include <Reai/Db.h>
#include <Reai/Log.h>

/* sqlite */
#include <sqlite3.h>

#include "Reai/Api/Request.h"
#include "Reai/Common.h"

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

    ReaiConfig *cfg = reai_config_load ("/Users/misra/.reai-rz.toml");
    RETURN_VALUE_IF (!cfg, EXIT_FAILURE, "Configuration load failure.");

    Reai *reai = reai_create (cfg->host, cfg->apikey, cfg->model);

    Char db_path[64] = {0};
    snprintf (db_path, sizeof (db_path) - 1, "%s/reai.db", cfg->db_dir_path);
    ReaiDb *db = reai_db_create (db_path);
    RETURN_VALUE_IF (!db, EXIT_FAILURE, "Failed to create database.");

    reai_set_db (reai, db);

    Char log_filename[256] = {0};
    snprintf (log_filename, ARRAY_SIZE (log_filename), "%s/creait.log", cfg->log_dir_path);
    ReaiLog *log = reai_log_create (log_filename);

    reai_set_logger (reai, log);

    ReaiResponse response = {0};
    reai_response_init (&response);

    ReaiFunctionId fn_ids[] = {43250783};

    ReaiRequest request                            = {0};
    request.type                                   = REAI_REQUEST_TYPE_BATCH_FUNCTION_SYMBOL_ANN;
    request.batch_function_symbol_ann.function_ids = fn_ids;
    request.batch_function_symbol_ann.function_id_count    = ARRAY_SIZE (fn_ids);
    request.batch_function_symbol_ann.distance             = 0.25f;
    request.batch_function_symbol_ann.results_per_function = 10;
    request.batch_function_symbol_ann.debug_mode           = true;

    RETURN_VALUE_IF (
        !reai_request (reai, &request, &response) || !response.batch_function_symbol_ann.success,
        EXIT_FAILURE,
        "Request failed : JSON RESPONSE : '%s'\n",
        response.raw.data
    );

    ReaiAnnFnMatchVec *matches = response.batch_function_symbol_ann.function_matches;
    REAI_VEC_FOREACH (matches, match, {
        PRINT_ERR (
            ".originalId = %llu, .nnId = %llu, .nnName = %s, .confidence = %lf",
            match->origin_function_id,
            match->nn_function_id,
            match->nn_function_name,
            match->confidence
        );
    });

    reai_destroy (reai);
    reai_config_destroy (cfg);

    return EXIT_SUCCESS;
}
