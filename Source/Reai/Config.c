#include <Reai/Config.h>
#include <Reai/Log.h>

/* libc  */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
#    define CONFIG_DIR_PATH getenv ("USERPROFILE")
#elif defined(__linux__) || defined(__APPLE__)
#    define CONFIG_DIR_PATH getenv ("HOME")
#else
#    error "Unsupported OS"
#endif

#define CONFIG_FILE_NAME ".creait"

/**
 * @b Get default file path where .reait.toml is supposed to be present.
 *
 * @param buf Buffer to get full path into
 * @param buf_cap Buffer capacity
 *
 * @return @c buf on success
 * @return @c NULL otherwise.
 * */
const char *get_default_config_path() {
    static char buf[1024]        = {0};
    static bool default_path_set = false;

    if (default_path_set) {
        return buf;
    }

    snprintf (buf, sizeof (buf), "%s/%s", CONFIG_DIR_PATH, CONFIG_FILE_NAME);
    default_path_set = true;

    return buf;
}

// Helper function to trim leading and trailing whitespace from a string
char *trim (char *str) {
    char *end;

    // Trim leading space
    while (isspace ((unsigned char)*str))
        str++;

    if (*str == 0) // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen (str) - 1;
    while (end > str && isspace ((unsigned char)*end))
        end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

Config ConfigRead (const char *path) {
    if (!path) {
        LOG_INFO ("Config file path not provided. Using default path.");
        path = get_default_config_path();
    }


    FILE *file = fopen (path, "r");
    if (file) {
        char   *line = NULL;
        size_t  len  = 0;
        ssize_t read;

        Config cfg = VecInit();

        while ((read = getline (&line, &len, file)) != -1) {
            // Remove trailing newline character if present
            if (line[read - 1] == '\n') {
                line[read - 1] = '\0';
            }

            char *eq_pos = strchr (line, '=');
            if (eq_pos != NULL) {
                *eq_pos = '\0'; // Split the string at the first '='

                char *key_str   = trim (line);
                char *value_str = trim (eq_pos + 1);

                if (strlen (key_str) > 0) {
                    KvPair pair;
                    pair.key   = StrInitFromZstr (key_str);
                    pair.value = StrInitFromZstr (value_str);
                    VecPushBack (&cfg, pair);
                }
            }
            // Ignore lines without '=' or empty lines after trimming
        }

        free (line);
        fclose (file);
        return cfg;
    } else {
        LOG_ERROR ("Failed to open config file at : %s", path);
        return (Config) {0};
    }
}

Str *ConfigFind (Config *cfg, const char *key) {
    VecForeachPtr (cfg, kv, {
        if (!StrCmpZstr (&kv->key, key)) {
            return &kv->value;
        }
    });
    return NULL; // Key not found
}
