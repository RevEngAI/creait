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

typedef struct ReaiConfig {
    CString host;
    CString apikey;
    CString model;
} ReaiConfig;

ReaiConfig *reai_config_load (CString path);
void        reai_config_destroy (ReaiConfig *cfg);

#endif // REAI_CONFIG_H
