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

    ReaiConfig *cfg = reai_config_load (NULL);
    RETURN_VALUE_IF (!cfg, EXIT_FAILURE, "Configuration load failure.");

    Reai *reai = reai_create (cfg->host, cfg->apikey, cfg->model);

    Char db_path[64] = {0};
    snprintf (db_path, sizeof (db_path) - 1, "%s/reai.db", cfg->db_dir_path);
    ReaiDb *db = reai_db_create (db_path);
    RETURN_VALUE_IF (!db, EXIT_FAILURE, "Failed to create database.");

    reai_set_db (reai, db);

    U64Vec *analyses = reai_db_get_all_created_analyses (db);
    REAI_VEC_FOREACH (analyses, bin_id, {
        // NOTE: the strings returend must be freed, but IDC right noow.
        PRINT_ERR (
            "bin_id = %llu, name = %s, hash = %s, ai_model_name = %s, status = %s",
            *bin_id,
            reai_db_get_analysis_file_name (db, *bin_id),
            reai_db_get_analysis_binary_file_hash (db, *bin_id),
            reai_db_get_analysis_model_name (db, *bin_id),
            reai_analysis_status_to_cstr (reai_db_get_analysis_status (db, *bin_id))
        );
    });

    ReaiBinaryId latest_analysis = analyses->items[0];
    CString      analysis_name   = reai_db_get_analysis_file_name (db, latest_analysis);
    PRINT_ERR ("analysis name = %s", analysis_name);
    FREE (analysis_name);

    ReaiResponse response = {0};
    reai_response_init (&response);

    ReaiRequest request                       = {0};
    request.type                              = REAI_REQUEST_TYPE_BATCH_BINARY_SYMBOL_ANN;
    request.batch_binary_symbol_ann.binary_id = latest_analysis;
    request.batch_binary_symbol_ann.distance  = 0.25f;
    request.batch_binary_symbol_ann.results_per_function = 10;

    RETURN_VALUE_IF (
        !reai_request (reai, &request, &response),
        EXIT_FAILURE,
        ERR_INVALID_ARGUMENTS
    );

    ReaiAnnFnMatchVec *matches = response.batch_binary_symbol_ann.function_matches;
    REAI_VEC_FOREACH (matches, match, {
        PRINT_ERR (
            ".originalId = %llu, .nnId = %llu, .nnName = %s, .confidence = %lf",
            match->origin_function_id,
            match->nn_function_id,
            match->nn_function_name,
            match->confidence
        );
    });

    reai_u64_vec_destroy (analyses);
    reai_destroy (reai);
    reai_config_destroy (cfg);

    return EXIT_SUCCESS;
}
