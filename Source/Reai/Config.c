#include <Reai/Config.h>

#include "Reai/Common.h"

/* tomlc99 */
#include <toml.h>

/* libc  */
#include <ctype.h>
#include <errno.h>
#include <string.h>

/**
 * @b Get default file path where .reait.toml is supposed to be present.
 *
 * @param buf Buffer to get full path into
 * @param buf_cap Buffer capacity
 *
 * @return @c buf on success
 * @return @c NULL otherwise.
 * */
CString reai_config_get_default_path() {
    static Char buf[1024]        = {0};
    static Bool default_path_set = false;

    if (default_path_set) {
        return buf;
    }

    snprintf (buf, sizeof (buf), "%s/%s", REAI_CONFIG_DIR_PATH, REAI_CONFIG_FILE_NAME);
    default_path_set = true;

    return buf;
}

/**
 * @b Get default directory path where .reait.toml is supposed to be present.
 *
 * @param buf Buffer to get full path into
 * @param buf_cap Buffer capacity
 *
 * @return @c buf on success
 * @return @c NULL otherwise.
 * */
CString reai_config_get_default_dir_path() {
    return REAI_CONFIG_DIR_PATH;
}

/**
 * @b Load TOML config from given path.
 *
 * @param path If @c NULL then default path is used.
 *
 * @return ReaiConfig
 * */
PUBLIC ReaiConfig *reai_config_load (CString path) {
    if (!path) {
        path = reai_config_get_default_path();
    }

    ReaiConfig *cfg = NEW (ReaiConfig);

    FILE *fp;
    char  errbuf[200];

    fp = fopen (path, "r");
    RETURN_VALUE_IF (!fp, NULL, ERR_FILE_OPEN_FAILED " : %s", strerror (errno));

    toml_table_t *reai_conf = toml_parse_file (fp, errbuf, sizeof (errbuf));
    fclose (fp);
    RETURN_VALUE_IF (!reai_conf, NULL, "Failed to parse toml config file.");

    toml_datum_t apikey = toml_string_in (reai_conf, "apikey");
    GOTO_HANDLER_IF (
        !apikey.ok,
        LOAD_FAILED,
        "Cannot find 'apikey' (required) in RevEngAI config."
    );
    cfg->apikey = apikey.u.s;

    toml_datum_t host = toml_string_in (reai_conf, "host");
    GOTO_HANDLER_IF (!host.ok, LOAD_FAILED, "Cannot find 'host' (required) in RevEngAI config.");
    cfg->host = host.u.s;

    toml_datum_t model = toml_string_in (reai_conf, "model");
    if (model.ok) {
        cfg->model = model.u.s;
    }

    if (reai_conf) {
        toml_free (reai_conf);
    }
    return cfg;

LOAD_FAILED:
    if (reai_conf) {
        toml_free (reai_conf);
    }

    reai_config_destroy (cfg);
    return NULL;
}

/**
 * @b Destroy given TOML config object.
 *
 * @param cfg
 * @return ReaiConfig
 * */
PUBLIC void reai_config_destroy (ReaiConfig *cfg) {
    RETURN_IF (!cfg, ERR_INVALID_ARGUMENTS);

    if (cfg->apikey) {
        memset ((Char *)cfg->apikey, 0, strlen (cfg->apikey));
        FREE (cfg->apikey);
    }
    if (cfg->host) {
        memset ((Char *)cfg->host, 0, strlen (cfg->host));
        FREE (cfg->host);
    }
    if (cfg->model) {
        memset ((Char *)cfg->model, 0, strlen (cfg->model));
        FREE (cfg->model);
    }

    memset (cfg, 0, sizeof (ReaiConfig));
    FREE (cfg);
}

/**
 * @b Check whether or not the API key is in correct format.
 *
 * @param apikey
 *
 * @return @c true if given API key is correct.
 * @return @c false otherwise.
 * */
Bool reai_config_check_api_key (CString apikey) {
    RETURN_VALUE_IF (!apikey, false, ERR_INVALID_ARGUMENTS);

    // Check the length of the string
    if (strlen (apikey) != 36) {
        return 0; // Invalid length
    }

    // Check the format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    for (int i = 0; i < 36; i++) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            // Check for dashes at the correct positions
            if (apikey[i] != '-') {
                return 0; // Incorrect character at dash position
            }
        } else {
            // Check for hexadecimal characters
            if (!isxdigit ((Uint8)apikey[i])) {
                return 0; // Non-hexadecimal character
            }
        }
    }

    return 1; // Valid UUID
}
