/**
 * @file Log.h
 * @date 29th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

/* reai */
#include <Reai/Log.h>
#include <Reai/Sys.h>
#include <Reai/Types.h>

/* libc */
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static FILE     *stderror  = NULL;
static SysMutex *log_mutex = NULL;

void LogInit (bool redirect) {
    if (redirect) {
        // Get the current time
        time_t    raw_time;
        struct tm time_info;
        char      time_buffer[20] = {0};

        time (&raw_time);
#ifdef _WIN32
        // Order is inverted in Windows
        if (localtime_s (&time_info, &raw_time)) {
#else
        if (!localtime_r (&raw_time, &time_info)) {
#endif
            Str syserr;
            StrInitStack (&syserr, SYS_ERROR_STR_MAX_LENGTH, {
                LOG_ERROR ("Failed to get localtime : %s", SysStrError (errno, &syserr)->data);
            });
            goto LOG_STREAM_FALLBACK;
        }
        strftime (time_buffer, sizeof (time_buffer), "%Y-%m-%d-%H-%M-%S", &time_info);

        // Get path to temp directory
        Str log_dir = StrInit();
        if (!SysGetEnv ("TMP", &log_dir) && !SysGetEnv ("TEMP", &log_dir) &&
            !SysGetEnv ("TMPDIR", &log_dir) && !SysGetEnv ("TEMPDIR", &log_dir) &&
            !SysGetEnv ("PWD", &log_dir)) {
            Str syserr;
            StrInitStack (&syserr, SYS_ERROR_STR_MAX_LENGTH, {
                fprintf (
                    stderr,
                    "error opening logfile : %s\n"
                    "All logs will now be redirected to stderr.\n",
                    SysStrError (errno, &syserr)->data
                );
            });
            stderror = stderr;
            goto LOG_STREAM_FALLBACK;
        }

        // generate log file name
        Str file_name = StrInit();
        StrPrintf (
            &file_name,
            "%s/misra-%lu-%s",
            log_dir.data,
            SysGetCurrentProcessId(),
            time_buffer
        );
        fprintf (stderr, "storing logs in %s\n", file_name.data);

        // Open the file for writing (create if it doesn't exist, overwrite if it does)
        i32 e = 0;
#ifdef _WIN32
        e = fopen_s (&stderror, file_name.data, "w");
#else
        stderror = fopen (file_name.data, "w");
        if (!stderror) {
            e = errno;
        }
#endif

        // Free resources
        StrDeinit (&file_name);
        StrDeinit (&log_dir);

        if (e || !stderror) {
            Str syserr;
            StrInitStack (&syserr, SYS_ERROR_STR_MAX_LENGTH, {
                LOG_ERROR ("Failed to open log file : %s", SysStrError (e, &syserr)->data);
            });
            goto LOG_STREAM_FALLBACK;
        }

        // Flush buffer instantly!
        setvbuf (stderror, NULL, _IONBF, 0);
        return;

LOG_STREAM_FALLBACK: {
    Str syserr;
    StrInitStack (&syserr, SYS_ERROR_STR_MAX_LENGTH, {
        fprintf (stderr, "Error opening log file, will write logs to stderr\n");
    });
    stderror = stderr;
}
    } else {
        stderror = stderr;
    }
}

void LogPrintf (LogLevel level, const char *tag, int line, const char *format, ...) {
    if (!format) {
        LOG_ERROR ("Invalid format string provided. Cannot write to logs.");
        return;
    }

    // By default we have a "decompiler" tag in all logs
    tag = tag ? tag : "log_write";

    // Initialize log if not already
    if (!stderror) {
        LogInit (false);
    }

    // Initialize the mutex if not already
    if (!log_mutex) {
        log_mutex = SysMutexCreate();
    }

    // Get the current time
    time_t    raw_time;
    struct tm time_info;
    char      time_buffer[20];

    time (&raw_time);
#ifdef _WIN32
    // Order is inverted in Windows
    if (localtime_s (&time_info, &raw_time)) {
#else
    if (!localtime_r (&raw_time, &time_info)) {
#endif
        Str syserr;
        StrInitStack (&syserr, SYS_ERROR_STR_MAX_LENGTH, {
            LOG_ERROR ("Failed to get localtime : %s", SysStrError (errno, &syserr)->data);
        });
        return;
    }
    strftime (time_buffer, sizeof (time_buffer), "%Y-%m-%d-%H-%M-%S", &time_info);

    const char *msg_type = NULL;
    switch (level) {
        case LOG_LEVEL_INFO :
            msg_type = "INFO";
            break;
        case LOG_LEVEL_ERROR :
            msg_type = "ERROR";
            break;
        case LOG_LEVEL_FATAL :
            msg_type = "FATAL";
            break;
        default :
            msg_type = "UNKNOWN_MESSAGE_TYPE";
            break;
    }

    SysMutexLock (log_mutex);

    // Print the log prefix to stderr
    fprintf (stderror, "[%s] [%s] [%s:%d] ", msg_type, time_buffer, tag, line);

    // Process the variadic arguments and print the log message
    va_list args;
    va_start (args, format);
    vfprintf (stderror, format, args);
    va_end (args);

    // Print a newline for better readability
    fprintf (stderror, "\n");

    SysMutexUnlock (log_mutex);
}
