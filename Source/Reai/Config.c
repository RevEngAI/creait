#include <Reai/Config.h>
#include <Reai/Log.h>
#include <Reai/Sys.h>

/* libc  */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#    define CONFIG_DIR_PATH getenv ("USERPROFILE")
#elif defined(__linux__) || defined(__APPLE__)
#    define CONFIG_DIR_PATH getenv ("HOME")
#else
#    error "Unsupported OS"
#endif

#define CONFIG_FILE_NAME ".creait"

i64 getline_compat(char **restrict lineptr, size *restrict n, FILE *restrict stream) {
    if (!lineptr || !n || !stream) return -1;

    size sz = 0;
    int c = 0;

    while (((c = getc(stream)) != EOF) && (c != '\n')) {
        if (sz + 1 >= *n) {
            size new_sz = *n ? (*n * 2) : 128;
            char *new_buf = realloc(*lineptr, new_sz);
            if (!new_buf)
                return -1;
            *lineptr = new_buf;
            *n = new_sz;
        }
        (*lineptr)[sz++] = (char)c;
    }

    if (size == 0 && c == EOF)
        return -1;

    if(sz) {
        (*lineptr)[sz] = '\0';
    }

    return sz;
}


const char *defaultConfigPath() {
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
        path = defaultConfigPath();
    }


    FILE *file = fopen (path, "r");
    if (file) {
        char   *line = NULL;
        size  len    = 0;
        i64 read     = 0;

        Config cfg = ConfigInit();

        while ((read = getline_compat (&line, &len, file)) > 0) {
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

void ConfigWrite (Config *c, const char *path) {
    if (!c) {
        LOG_FATAL ("Invalid arguments");
    }

    if (!path) {
        path = defaultConfigPath();
    }

    FILE *outfile = fopen (path, "w");
    if (!outfile) {
        Str syserr;
        StrInitStack (&syserr, 128, {
            LOG_ERROR ("Error opening file for writing %s", SysStrError (errno, &syserr)->data);
        });
    }

    VecForeach (c, kv, { fprintf (outfile, "%s = %s\n", kv.key.data, kv.value.data); });

    fclose (outfile);
}

Str *ConfigGet (Config *cfg, const char *key) {
    VecForeachPtr (cfg, kv, {
        if (!StrCmpZstr (&kv->key, key)) {
            return &kv->value;
        }
    });
    return NULL; // Key not found
}

void KvPairDeinit (KvPair *c) {
    if (!c) {
        LOG_FATAL ("Invalid argument");
    }
    StrDeinit (&c->key);
    StrDeinit (&c->value);
}

bool KvPairInitClone (KvPair *d, KvPair *s) {
    if (!d || !s) {
        LOG_FATAL ("Invalid arguments");
    }
    StrInitCopy (&d->key, &s->key);
    StrInitCopy (&d->value, &s->value);
    return true;
}

void ConfigAdd (Config *c, const char *key, const char *value) {
    if (!c || !key || !value) {
        LOG_FATAL ("Invalid arguments");
    }

    KvPair kv = {.key = StrInitFromZstr (key), .value = StrInitFromZstr (value)};
    VecPushBack (c, kv);
}
