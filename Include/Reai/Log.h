/**
 * @file Log.h
 * @date 29th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_LOG_H
#define REAI_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Reai/Types.h>

    typedef enum ReaiLogLevel {
        REAI_LOG_LEVEL_TRACE, ///< For traced logging
        REAI_LOG_LEVEL_INFO,  ///< For debugging only
        REAI_LOG_LEVEL_DEBUG, ///< For debugging only
        REAI_LOG_LEVEL_WARN,  ///< Recoverable within the function that generates this
                              ///< warning
        REAI_LOG_LEVEL_ERROR, ///< Might be recoverable so just return
        REAI_LOG_LEVEL_FATAL, ///< Qucik Abort!
        REAI_LOG_LEVEL_MAX,   ///< Number of log levels
    } ReaiLogLevel;

    void reai_log_printf (ReaiLogLevel level, CString tag, CString fmtstr, ...);

#define REAI_LOG_TRACE(...) reai_log_printf (REAI_LOG_LEVEL_TRACE, __FUNCTION__, __VA_ARGS__)
#define REAI_LOG_INFO(...)  reai_log_printf (REAI_LOG_LEVEL_INFO, __FUNCTION__, __VA_ARGS__)
#define REAI_LOG_DEBUG(...) reai_log_printf (REAI_LOG_LEVEL_DEBUG, __FUNCTION__, __VA_ARGS__)
#define REAI_LOG_WARN(...)  reai_log_printf (REAI_LOG_LEVEL_WARN, __FUNCTION__, __VA_ARGS__)
#define REAI_LOG_ERROR(...) reai_log_printf (REAI_LOG_LEVEL_ERROR, __FUNCTION__, __VA_ARGS__)
#define REAI_LOG_FATAL(...) reai_log_printf (REAI_LOG_LEVEL_FATAL, __FUNCTION__, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // REAI_LOG_H
