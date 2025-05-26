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

    void LogPrintf (LogLevel level, const char *tag, int line, const char *format, ...);

#define LOG_INFO(...)  LogPrintf (LOG_LEVEL_INFO, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) LogPrintf (LOG_LEVEL_ERROR, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) (LogPrintf (LOG_LEVEL_FATAL, __FUNCTION__, __LINE__, __VA_ARGS__), abort())

    void LogInit (bool redirect);

#ifdef __cplusplus
}
#endif

#endif // LOG_H
