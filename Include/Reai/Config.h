/**
 * @file Common.h
 * @date Tue, 23rd July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) 2024 RevEngAI. All Rights Reserved.
 *
 * @brief Loads the RevEngAI Toolit config file and returns an easy to use
 * object representing the config.
 * */

#ifndef REAI_CONFIG_H
#define REAI_CONFIG_H

#include <Reai/Types.h>
#include <Reai/Util/Str.h>

#ifdef __cplusplus
extern "C" {
#endif

    /* Config file is just a set of key-value pairs in
     * "\s*key\s*=\s*value\n" format
     * in new line each. */

    typedef struct {
        Str key;
        Str value;
    } KvPair;
    typedef Vec (KvPair) KvPairs;
    typedef KvPairs Config;

    void KvPairDeinit (KvPair* c);
    bool KvPairInitClone (KvPair* d, KvPair* s);

#define ConfigInit()    VecInitWithDeepCopy (NULL, KvPairDeinit)
#define ConfigInit_T(c) VecInitWithDeepCopy_T (c, NULL, KvPairDeinit)

    ///
    /// Load config from given path
    ///
    /// path[in] : Path to config file. If `NULL` then default path will be used.
    ///
    /// SUCCESS : Contents of config file loaded into Config object.
    /// FAILURE : Empty object.
    ///
    Config ConfigRead (const char* path);

    ///
    /// Write config to a file.
    ///
    /// c[in]    : Config to be serialized
    /// path[in] : Path to config save file.
    ///
    /// SUCCESS : Config file written to file at given path.
    /// FAILURE : Error message logged.
    ///
    void ConfigWrite (Config* c, const char* path);

    ///
    /// Add a new key-value pair to config.
    ///
    /// c[out]    : Config to add new key-value pair to.
    /// key[in]   : Key
    /// value[in] : Corresponding value.
    ///
    /// SUCCESS : Add new kv-pair to given config. If key already exists then it's overwritten.
    /// FAILURE : Error message logged.
    ///
    void ConfigAdd (Config* c, const char* key, const char* value);

    ///
    /// Look for a certain key in config
    ///
    /// c[in]   : Config
    /// key[in] : Key to search for.
    ///
    /// SUCCESS : Str object with value corresponding to key. Don't ever deinit!
    /// FAILURE : NULL.
    ///
    Str* ConfigFind (Config* c, const char* key);

#ifdef __cplusplus
}
#endif

#endif // REAI_CONFIG_H
