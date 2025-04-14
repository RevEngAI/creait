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

PRIVATE CString log_level_to_cstr (ReaiLogLevel level);
PRIVATE CString generate_new_log_file_name();

static FILE* log_fd = NULL;

/**
 * @b Destroy given logger object and close attached file handle.
 * */
DESTRUCTOR void reai_log_destroy() {
    if (log_fd) {
        fclose (log_fd);
    }
}

/**
 * @b Create a new logger
 *
 * @return @c ReaiLog on success.
 * @return @c NULL otherwise.
 * */
CONSTRUCTOR void reai_log_create () {
    CString log_file_name = generate_new_log_file_name();
    if (log_file_name) {
        log_fd = fopen (log_file_name, "w");

        if (!log_fd) {
            PRINT_ERR (
                "Failed to create log file : %s. Sending all future logs to stderr",
                strerror (errno)
            );
            log_fd = stderr;
            FREE (log_file_name);
            return;
        }

        FREE (log_file_name);
    }

    setbuf (log_fd, NULL);
}

#ifdef _MSC_VER
// SOURCE: https://stackoverflow.com/a/20724874
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        reai_log_create();
    } else if (fdwReason == DLL_PROCESS_DETACH) {
        reai_log_destroy();
    }

    return TRUE;
}
#endif

/**
 * @b Write to log file with given log level.
 *
 * @param logger Logger object.
 * @param level Log level.
 * @param tag Tag attached with log entry. This is usally the log event creator.
 * @param fmtstr Format string.
 * @param ... Format string arguments.
 * */
void reai_log_printf (ReaiLogLevel level, CString tag, const char* fmtstr, ...) {
    if (!log_fd) {
        return;
    }

    tag = tag ? tag : "reai_log_printf";

    /* print time and log level */
    {
        time_t rawtime;
        time (&rawtime);
        struct tm* timeinfo;
        timeinfo = localtime (&rawtime);
        Char timebuf[10];
        strftime (timebuf, sizeof (timebuf), "%H:%M:%S", timeinfo);
        fprintf (log_fd, "[%-5s] : [%s] : [%s] : ", log_level_to_cstr (level), timebuf, tag);
    }

    /* pass on everything else to fprintf */
    if (fmtstr) {
        va_list args;
        va_start (args, fmtstr);
        vfprintf (log_fd, fmtstr, args);
        fputc ('\n', log_fd);
        va_end (args);
    } else {
        fprintf (log_fd, "invalid format string provided.");
    }
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
    tmp_dir = getenv ("TMP")    ? getenv ("TMP") :
              getenv ("TMPDIR") ? getenv ("TMPDIR") :
              getenv ("PWD")    ? getenv ("PWD") :
                                  NULL;
#endif

    RETURN_VALUE_IF (!tmp_dir, NULL, "Failed to get path to temporary directory.");

    /* generate filename with PID and time */
    Char filename[256] = {0};
    snprintf (filename, sizeof (filename), "%s/reai_%u_%s.log", tmp_dir, pid, time_str);

    return strdup (filename);
}
