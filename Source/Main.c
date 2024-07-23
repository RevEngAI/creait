/* reai */
#include <Reai/Api/Api.h>
#include <Reai/Common.h>
#include <Reai/Config.h>

/* sqlite */
#include <sqlite3.h>

/* libc */
#include <threads.h>
#include <time.h>

#include "Reai/Api/Request.h"
#include "Reai/Util/AnalysisInfo.h"

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

    request.type = REAI_REQUEST_TYPE_RECENT_ANALYSIS;
    reai_request (reai, &request, &response);
    RETURN_VALUE_IF (
        response.type != REAI_RESPONSE_TYPE_RECENT_ANALYSIS,
        EXIT_FAILURE,
        "Failed to get recent analysis info from RevEngAI servers : %s.\n",
        response.raw.data
    );

    ReaiAnalysisInfoVec *recent_analyses =
        reai_analysis_info_vec_clone_create (response.recent_analysis.analysis_infos);
    RETURN_VALUE_IF (!recent_analyses, EXIT_FAILURE, "Failed to clone recent analysis vec\n");

    for (Size s = 0; s < recent_analyses->count; s++) {
        ReaiAnalysisInfo *info = recent_analyses->items + s;
        PRINT_ERR ("%llu:%s\n", info->binary_id, info->binary_name);
    }

    reai_analysis_info_vec_destroy (recent_analyses);

    reai_destroy (reai);
    sqlite3_close (sql);
    reai_config_destroy (cfg);

    return EXIT_SUCCESS;
}
