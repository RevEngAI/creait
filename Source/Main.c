/* reai */
#include <Reai/Api/Api.h>
#include <Reai/Common.h>
#include <Reai/Config.h>

/* sqlite */
#include <sqlite3.h>

#include "Reai/Api/Request.h"

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

    ReaiRequest request = {0};

    request.type = REAI_REQUEST_TYPE_HEALTH_CHECK;
    reai_request (reai, &request, &response);

    PRINT_ERR ("%s", response.raw.data);

    reai_destroy (reai);
    sqlite3_close (sql);
    reai_config_destroy (cfg);

    return EXIT_SUCCESS;
}
