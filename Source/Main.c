/* reai */
#include <Reai/Api/Api.h>
#include <Reai/Common.h>
#include <Reai/Config.h>

int main (int argc, char **argv) {
    RETURN_VALUE_IF (!argc || !argv, EXIT_FAILURE, ERR_INVALID_ARGUMENTS);

    ReaiConfig *cfg  = reai_config_load (Null);
    Reai       *reai = reai_create (cfg->host, cfg->apikey);

    reai_destroy (reai);
    reai_config_destroy (cfg);

    return EXIT_SUCCESS;
}
