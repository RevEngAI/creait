#include <Reai/Config.h>

/* tomlc99 */
#include <toml.h>

/* libc  */
#include <errno.h>
#include <string.h>

/**
 * @b Get default path where .reait.toml is supposed to be present.
 *
 * @param buf Buffer to get full path into
 * @param buf_cap Buffer capacity
 *
 * @return @c buf on success
 * @return @c Null otherwise.
 * */
CString reai_config_get_default_path() {
    static Char buf[1024]        = {0};
    static Bool default_path_set = False;

    if (default_path_set) {
        return buf;
    }

    snprintf (buf, sizeof (buf), "%s/%s", REAI_CONFIG_DIR_PATH, REAI_CONFIG_FILE_NAME);
    default_path_set = True;

    return buf;
}

/**
 * @b Load TOML config from given path.
 *
 * @param path If @c Null then default path is used.
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
    RETURN_VALUE_IF (!fp, Null, ERR_FILE_OPEN_FAILED " : %s", strerror (errno));

    toml_table_t *reai_conf = toml_parse_file (fp, errbuf, sizeof (errbuf));
    fclose (fp);
    RETURN_VALUE_IF (!reai_conf, Null, "Failed to parse toml config file.");

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

    toml_datum_t db_dir_path = toml_string_in (reai_conf, "db_dir_path");
    GOTO_HANDLER_IF (
        !db_dir_path.ok,
        LOAD_FAILED,
        "Cannot find 'db_dir_path' (required) in RevEngAI config."
    );
    cfg->db_dir_path = db_dir_path.u.s;

    toml_datum_t log_dir_path = toml_string_in (reai_conf, "log_dir_path");
    GOTO_HANDLER_IF (
        !log_dir_path.ok,
        LOAD_FAILED,
        "Cannot find 'log_dir_path' (required) in RevEngAI config."
    );
    cfg->log_dir_path = log_dir_path.u.s;

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
    return Null;
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
    if (cfg->db_dir_path) {
        memset ((Char *)cfg->db_dir_path, 0, strlen (cfg->db_dir_path));
        FREE (cfg->db_dir_path);
    }
    if (cfg->log_dir_path) {
        memset ((Char *)cfg->log_dir_path, 0, strlen (cfg->log_dir_path));
        FREE (cfg->log_dir_path);
    }

    memset (cfg, 0, sizeof (ReaiConfig));
    FREE (cfg);
}

/**
 * @b Check whether or not the API key is in correct format.
 *
 * @param apikey
 *
 * @return @c True if given API key is correct.
 * @return @c False otherwise.
 * */
Bool reai_config_check_api_key (CString apikey) {
    CString iter = Null;

    if (!(iter = strchr (apikey, '-')) || ((iter - apikey) != 8)) {
        return False;
    }
    apikey = iter + 1;

    if (!(iter = strchr (apikey, '-')) || ((iter - apikey) != 4)) {
        return False;
    }
    apikey = iter + 1;

    if (!(iter = strchr (apikey, '-')) || ((iter - apikey) != 4)) {
        return False;
    }
    apikey = iter + 1;

    if (!(iter = strchr (apikey, '-')) || ((iter - apikey) != 4)) {
        return False;
    }
    apikey = iter + 1;

    if (!(iter = strchr (apikey, 0)) || ((iter - apikey) != 12)) {
        return False;
    }

    return True;
}
