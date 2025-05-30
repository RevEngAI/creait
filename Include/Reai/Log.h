/**
 * @file Log.h
 * @date 29th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef LOG_H
#define LOG_H

#include <Reai/Types.h>

typedef enum LogLevel {
    LOG_LEVEL_INVALID = 0,
    LOG_LEVEL_INFO,  ///< For general information
    LOG_LEVEL_ERROR, ///< Might be recoverable so just return
    LOG_LEVEL_FATAL, ///< Qucik Abort!
    LOG_LEVEL_MAX,   ///< Number of log levels
} LogLevel;

#ifdef __cplusplus
extern "C" {
#endif

    void LogWrite (LogLevel level, const char *tag, int line, const char *msg);

#define LOG_INFO(...)                                                                              \
    do {                                                                                           \
        Str msg = StrInit();                                                                       \
        StrPrintf (&msg, __VA_ARGS__);                                                             \
        LogWrite (LOG_LEVEL_INFO, __func__, __LINE__, msg.data);                          \
        StrDeinit (&msg);                                                                          \
    } while (0)

#define LOG_ERROR(...)                                                                             \
    do {                                                                                           \
        Str msg = StrInit();                                                                       \
        StrPrintf (&msg, __VA_ARGS__);                                                             \
        LogWrite (LOG_LEVEL_ERROR, __func__, __LINE__, msg.data);                         \
        StrDeinit (&msg);                                                                          \
    } while (0)

#define LOG_FATAL(...)                                                                             \
    do {                                                                                           \
        Str msg = StrInit();                                                                       \
        StrPrintf (&msg, __VA_ARGS__);                                                             \
        LogWrite (LOG_LEVEL_FATAL, __func__, __LINE__, msg.data);                         \
        StrDeinit (&msg);                                                                          \
        abort();                                                                                   \
    } while (0)

    void LogInit (bool redirect);

#ifdef __cplusplus
}
#endif

#endif // LOG_H
