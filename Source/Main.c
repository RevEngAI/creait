/* reai */
#include <Reai/Api/Api.h>
#include <Reai/Common.h>
#include <Reai/Config.h>

/* sqlite */
#include <sqlite3.h>

int main (int argc, char **argv) {
    RETURN_VALUE_IF (!argc || !argv, EXIT_FAILURE, ERR_INVALID_ARGUMENTS);

    ReaiConfig *cfg = reai_config_load (Null);
    PRINT_ERR ("HOST = %s\nAPIKEY = %s\n", cfg->host, cfg->apikey);

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

    ReaiRequest request = {0};

    memset (&request, 0, sizeof (request));
    request.type                   = REAI_REQUEST_TYPE_SEARCH;
    request.search.collection_name = "trojan";
    reai_request (reai, &request, &response);

    RETURN_VALUE_IF (
        response.type != REAI_RESPONSE_TYPE_SEARCH,
        EXIT_FAILURE,
        "Failed to perform search : %s.\n",
        response.raw.data
    );

    if (response.search.success) {
        PRINT_ERR ("Search result count = %zu\n", response.search.query_results->count);
        REAI_VEC_FOREACH (response.search.query_results, result, {
            PRINT_ERR ("%s\n", result->binary_name);
        });
    } else {
        PRINT_ERR ("Search failed.\n");
    }

    reai_destroy (reai);
    sqlite3_close (sql);
    reai_config_destroy (cfg);

    return EXIT_SUCCESS;
}
