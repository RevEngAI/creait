/**
 * @file Log.h
 * @date 29th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_LOG_H
#define REAI_LOG_H

#include <Reai/Types.h>

typedef struct ReaiLog ReaiLog;

typedef enum ReaiLogLevel {
    REAI_LOG_LEVEL_TRACE, ///< For traced logging
    REAI_LOG_LEVEL_INFO,  ///< For debugging only
    REAI_LOG_LEVEL_DEBUG, ///< For debugging only
    REAI_LOG_LEVEL_WARN,  ///< Recoverable within the function that generates this warning
    REAI_LOG_LEVEL_ERROR, ///< Might be recoverable so just return
    REAI_LOG_LEVEL_FATAL, ///< Qucik Abort!
    REAI_LOG_LEVEL_MAX,   ///< Number of log levels
} ReaiLogLevel;

ReaiLog* reai_log_create (CString file_name);
void     reai_log_destroy (ReaiLog* logger);
ReaiLog* reai_log_set_level (ReaiLog* log, ReaiLogLevel level);

void reai_log_printf (ReaiLog* logger, ReaiLogLevel level, CString tag, CString fmtstr, ...);

#define REAI_LOG_TRACE(logger, ...)                                                                \
    reai_log_printf (logger, REAI_LOG_LEVEL_TRACE, __FUNCTION__, __VA_ARGS__)
#define REAI_LOG_INFO(logger, ...)                                                                 \
    reai_log_printf (logger, REAI_LOG_LEVEL_INFO, __FUNCTION__, __VA_ARGS__)
#define REAI_LOG_DEBUG(logger, ...)                                                                \
    reai_log_printf (logger, REAI_LOG_LEVEL_DEBUG, __FUNCTION__, __VA_ARGS__)
#define REAI_LOG_WARN(logger, ...)                                                                 \
    reai_log_printf (logger, REAI_LOG_LEVEL_WARN, __FUNCTION__, __VA_ARGS__)
#define REAI_LOG_ERROR(logger, ...)                                                                \
    reai_log_printf (logger, REAI_LOG_LEVEL_ERROR, __FUNCTION__, __VA_ARGS__)
#define REAI_LOG_FATAL(logger, ...)                                                                \
    reai_log_printf (logger, REAI_LOG_LEVEL_FATAL, __FUNCTION__, __VA_ARGS__)

#endif // REAI_LOG_H
