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

#include <Reai/Common.h>
#include <Reai/Types.h>

/**
 * Get directory path where config file must be stored, depending
 * on operating system.
 * */

#if defined(_WIN32) || defined(_WIN64)
#    define REAI_CONFIG_DIR_PATH getenv ("USERPROFILE")
#elif defined(__linux__) || defined(__APPLE__)
#    define REAI_CONFIG_DIR_PATH getenv ("HOME")
#else
#    error "Unsupported OS"
#endif

#define REAI_CONFIG_FILE_NAME ".creait.toml"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct ReaiConfig {
        CString host;
        CString apikey;
    } ReaiConfig;

    ReaiConfig *reai_config_load (CString path);
    void        reai_config_destroy (ReaiConfig *cfg);
    CString     reai_config_get_default_path();
    CString     reai_config_get_default_dir_path();
    Bool        reai_config_check_api_key (CString apikey);

#ifdef __cplusplus
}
#endif

#endif // REAI_CONFIG_H
