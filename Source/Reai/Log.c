/**
 * @file Log.h
 * @date 29th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

/* reai */
#include <Reai/Common.h>
#include <Reai/Log.h>

/* libc */
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

/* platform specific */
#ifdef _WIN32
#    include <windows.h>
#    define getpid GetCurrentProcessId
#else
#    include <unistd.h>
#endif

typedef struct ReaiLog {
    CString      log_file;
    FILE*        log_fd;
    ReaiLogLevel min_log_level;
} ReaiLog;

PRIVATE CString log_level_to_cstr (ReaiLogLevel level);
PRIVATE CString generate_new_log_file_name();

/**
 * @b Destroy given logger object and close attached file handle.
 *
 * @param logger
 * */
void reai_log_destroy (ReaiLog* logger) {
    RETURN_IF (!logger, ERR_INVALID_ARGUMENTS);

    if (logger->log_file) {
        FREE (logger->log_file);
    }

    if (logger->log_fd) {
        fclose (logger->log_fd);
    }

    memset (logger, 0, sizeof (ReaiLog));
    FREE (logger);
}

/**
 * @b Create a new logger
 *
 * @param file_name Name/Path of file to log to. If NULL is provided, then
 *        a new file name will automatically be generated and be stored in
 *        platform's temp directory.
 *
 * @return @c ReaiLog on success.
 * @return @c NULL otherwise.
 * */
ReaiLog* reai_log_create (CString file_name) {
    ReaiLog* log = NEW (ReaiLog);
    RETURN_VALUE_IF (!log, NULL, ERR_OUT_OF_MEMORY);

    log->log_file = file_name ? strdup (file_name) : generate_new_log_file_name();
    GOTO_HANDLER_IF (!log->log_file, CREATE_FAILED, ERR_OUT_OF_MEMORY);

    log->log_fd = fopen (log->log_file, "w");
    GOTO_HANDLER_IF (
        !log->log_fd,
        CREATE_FAILED,
        "Failed to create new log file \"%s\" : %s",
        log->log_file,
        strerror (errno)
    );

    return log;

CREATE_FAILED:
    reai_log_destroy (log);
    return NULL;
}

/**
 * @b Set minimum log level for given logger.
 *
 * @param log
 * @parma level
 *
 * @return @c log on success.
 * @return @c NULL otherwise.
 * */
ReaiLog* reai_log_set_level (ReaiLog* log, ReaiLogLevel level) {
    RETURN_VALUE_IF (!log || (level >= REAI_LOG_LEVEL_MAX), NULL, ERR_INVALID_ARGUMENTS);

    log->min_log_level = level;

    return log;
}

/**
 * @b Write to log file with given log level.
 *
 * @param logger Logger object.
 * @param level Log level.
 * @param tag Tag attached with log entry. This is usally the log event creator.
 * @param fmtstr Format string.
 * @param ... Format string arguments.
 * */
void reai_log_printf (ReaiLog* logger, ReaiLogLevel level, CString tag, const char* fmtstr, ...) {
    RETURN_IF (!logger || !tag || !fmtstr, ERR_INVALID_ARGUMENTS);

    /* if given log level is less than min log level then skip */
    if (level < logger->min_log_level) {
        return;
    }

    /* print time and log level */
    {
        time_t rawtime;
        time (&rawtime);
        struct tm* timeinfo;
        timeinfo = localtime (&rawtime);
        Char timebuf[10];
        strftime (timebuf, sizeof (timebuf), "%H:%M:%S", timeinfo);
        fprintf (
            logger->log_fd,
            "[%-5s] : [%s] : [%s] : ",
            log_level_to_cstr (level),
            timebuf,
            tag
        );
    }

    /* pass on everything else to fprintf */
    va_list args;
    va_start (args, fmtstr);
    vfprintf (logger->log_fd, fmtstr, args);
    fprintf (logger->log_fd, "\n");
    va_end (args);
}

PRIVATE CString log_level_to_cstr (ReaiLogLevel level) {
    switch (level) {
        case REAI_LOG_LEVEL_TRACE : {
            return "TRACE";
        }
        case REAI_LOG_LEVEL_INFO : {
            return "INFO";
        }
        case REAI_LOG_LEVEL_DEBUG : {
            return "DEBUG";
        }
        case REAI_LOG_LEVEL_WARN : {
            return "WARN";
        }
        case REAI_LOG_LEVEL_ERROR : {
            return "ERROR";
        }
        case REAI_LOG_LEVEL_FATAL : {
            return "FATAL";
        }
        default : {
            RETURN_VALUE_IF_REACHED (
                "INVALID_LOG_LEVEL",
                "Unreachable core reached. Invalid log level.\n"
            );
        }
    }
}

PRIVATE CString generate_new_log_file_name() {
    // Get current process ID
    Uint32 pid = getpid();

    // Get current time
    time_t     rawtime;
    struct tm* timeinfo;
    time (&rawtime);
    timeinfo = localtime (&rawtime);

    // Format time as YYYYMMDD_HHMMSS
    char time_str[20];
    strftime (time_str, sizeof (time_str), "%Y%m%d_%H%M%S", timeinfo);

    // Get temporary directory path
    const char* tmp_dir = NULL;
#ifdef _WIN32
    tmp_dir = getenv ("TMP") ? getenv ("TMP") : getenv ("TEMP") ? getenv ("TEMP") : NULL;
#else
    tmp_dir = getenv ("TMPDIR") ? getenv ("TMPDIR") : NULL;
#endif

    RETURN_VALUE_IF (!tmp_dir, NULL, "Failed to get path to temporary directory.");

    /* generate filename with PID and time */
    Char filename[64] = {0};
    snprintf (filename, sizeof (filename), "%s/reai_%u_%s.log", tmp_dir, pid, time_str);

    return strdup (filename);
}
