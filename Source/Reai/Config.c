#include <Reai/Config.h>
#include <Reai/File.h>
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

Config ConfigRead (const char *path) {
    if (!path) {
        LOG_INFO ("Config file path not provided. Using default path.");
        path = defaultConfigPath();
    }

    Str cfg = StrInit();
    if (ReadCompleteFile (path, &cfg.data, &cfg.length, &cfg.capacity)) {
        Strs   lines  = StrSplit (&cfg, "\n");
        Config config = ConfigInit();

        VecForeachPtr (&lines, line, {
            Strs kvsplit = StrSplit (line, "=");
            if (kvsplit.length != 2) {
                LOG_ERROR ("Config file is invalid. Each line must be in form 'key = value'");
                StrDeinit (&cfg);
                VecDeinit (&lines);
                VecDeinit (&kvsplit);
                return (Config) {0};
            }

            KvPair kv = KvPairInit();
            kv.key    = StrStrip (VecPtrAt (&kvsplit, 0), " \r\t");
            kv.value  = StrStrip (VecPtrAt (&kvsplit, 1), " \r\t");
            VecDeinit (&kvsplit);

            VecPushBack (&config, kv);
        });

        return config;
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
        Str syserr = StrInit();
        LOG_ERROR ("Error opening file for writing %s", SysStrError (errno, &syserr)->data);
        StrDeinit (&syserr);
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
