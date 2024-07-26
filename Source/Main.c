/* reai */
#include <Reai/Api/Api.h>
#include <Reai/Config.h>

/* sqlite */
#include <sqlite3.h>

int main (int argc, char **argv) {
    RETURN_VALUE_IF (!argc || !argv, EXIT_FAILURE, ERR_INVALID_ARGUMENTS);

    ReaiConfig *cfg = reai_config_load (Null);
    RETURN_VALUE_IF (!cfg, EXIT_FAILURE, "Configuration load failure.\n");

    sqlite3 *sql = Null;
    RETURN_VALUE_IF (
        (sqlite3_open (".reai.db", &sql) != SQLITE_OK) || !sql,
        EXIT_FAILURE,
        "Failed to load sqlite database : %s\n",
        sqlite3_errmsg (sql)
    );

    Reai *reai = reai_create (cfg->host, cfg->apikey);

    ReaiResponse response;
    reai_response_init (&response);

    /* get function infos and print info abt them */
    ReaiFnInfoVec *fn_infos = reai_get_basic_function_info (reai, &response, 18434);
    REAI_VEC_FOREACH (fn_infos, fn, { PRINT_ERR ("%llu:%s\n", fn->id, fn->name); });

    /* since we need to reuse the fn_infos vec, we clone */
    fn_infos = reai_fn_info_vec_clone_create (fn_infos);
    REAI_VEC_FOREACH (fn_infos, fn, {
        Char new_name[40] = {0};
        snprintf (new_name, sizeof (new_name), "renamed_%s", fn->name);
        if (!reai_rename_function (reai, &response, fn->id, new_name)) {
            PRINT_ERR ("Failed to rename function.\n");
        } else {
            PRINT_ERR ("Renamed function '%s' -> '%s'\n", fn->name, new_name);
        }
    });

    reai_response_deinit (&response);
    reai_destroy (reai);
    sqlite3_close (sql);
    reai_config_destroy (cfg);

    return EXIT_SUCCESS;
}
