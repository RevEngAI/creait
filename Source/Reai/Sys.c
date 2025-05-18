/**
 * @file Sys.c
 * @date 5th May 2025 
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#include <Reai/Log.h>
#include <Reai/Sys.h>
#include <Reai/Types.h>

// cstd
#include <errno.h>
#include <stdlib.h>

#ifdef _WIN32
#    include <windows.h>
#else
#    include <dirent.h>
#    include <pthread.h>
#    include <sys/stat.h>
#    include <unistd.h>
#endif

struct SysMutex {
#ifdef _WIN32
    CRITICAL_SECTION lock;
#else
    pthread_mutex_t lock;
#endif
};

// Cross platform get current time
Str* SysGetLocalTime (Str* timebuf) {
    // Get the current time
    time_t    raw_time;
    struct tm time_info;

    // Init time buffer with some initial memory
    *timebuf = StrInit();
    StrReserve (timebuf, 32);

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
        return NULL;
    }
    strftime (timebuf->data, timebuf->length, "%Y-%m-%d-%H-%M-%S", &time_info);

    return timebuf;
}

// Cross-platform function to get file size
i64 SysGetFileSize (const char* filename) {
    if (!filename) {
        LOG_ERROR ("invalid arguments.\n");
        return -1;
    }

#ifdef _WIN32
    // Windows-specific code using GetFileSizeEx
    HANDLE file =
        CreateFileA (filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        Str err;
        StrInitStack (&err, SYS_ERROR_STR_MAX_LENGTH, {
            LOG_ERROR ("failed to open file: %s\n", SysStrError (errno, &err)->data);
        });
        return -1;
    }

    LARGE_INTEGER file_size;
    if (!GetFileSizeEx (file, &file_size)) {
        Str err;
        StrInitStack (&err, SYS_ERROR_STR_MAX_LENGTH, {
            LOG_ERROR ("failed to get file size: %s\n", SysStrError (errno, &err)->data);
        });
        CloseHandle (file);
        return -1;
    }

    CloseHandle (file);
    return (i64)file_size.QuadPart;
#else
    // Unix-like systems (Linux/macOS) code using stat
    struct stat file_stat;
    if (stat (filename, &file_stat) == 0) {
        return (i64)file_stat.st_size;
    } else {
        Str err;
        StrInitStack (&err, SYS_ERROR_STR_MAX_LENGTH, {
            LOG_ERROR ("failed to get file size: %s\n", SysStrError (errno, &err)->data);
        });
        return -1;
    }
#endif
}

Str* SysGetEnv (const char* name, Str* value) {
    if (!name || !value) {
        return NULL;
    }
#ifdef _WIN32
    char*  env_var;
    size_t requiredSize;

    getenv_s (&requiredSize, NULL, 0, name);
    if (requiredSize == 0) {
        return NULL;
    }

    env_var = (char*)malloc (requiredSize);
    if (!env_var) {
        return NULL;
    }

    // Get the value of the LIB environment variable.
    getenv_s (&requiredSize, env_var, requiredSize, name);

    *value          = StrInit();
    value->data     = env_var;
    value->length   = requiredSize;
    value->capacity = requiredSize;
    return value;
#else
    char* env_var = getenv (name);
    if (env_var) {
        *value = StrInitFromZstr (env_var);
        return value;
    }
    return NULL;
#endif
}

unsigned long SysGetCurrentProcessId() {
#ifdef _WIN32
    return (unsigned long)GetCurrentProcessId(); // Windows API
#else
    return (unsigned long)getpid(); // POSIX API (Linux/macOS)
#endif
}

SysMutex* SysMutexCreate() {
    SysMutex* m = NEW (SysMutex);
#ifdef _WIN32
    InitializeCriticalSection (&m->lock);
#else
    memset (&m->lock, 0, sizeof (m->lock));
#endif
    return m;
}

void SysMutexDestroy (SysMutex* m) {
    if (!m) {
        return;
    }
#ifdef _WIN32
    DeleteCriticalSection (&m->lock);
#else
    pthread_mutex_destroy (&m->lock);
#endif
    memset (m, 0, sizeof (SysMutex));
    FREE (m);
}

SysMutex* SysMutexLock (SysMutex* m) {
    if (!m) {
        return NULL;
    }
#ifdef _WIN32
    EnterCriticalSection (&m->lock);
#else
    pthread_mutex_lock (&m->lock);
#endif
    return m;
}

SysMutex* SysMutexUnlock (SysMutex* m) {
    if (!m) {
        return NULL;
    }
#ifdef _WIN32
    LeaveCriticalSection (&m->lock);
#else
    pthread_mutex_unlock (&m->lock);
#endif
    return m;
}

Str* SysStrError (i32 eno, Str* err_str) {
    if (!err_str) {
        LOG_ERROR ("Invalid arguments");
    }

    err_str->length = err_str->capacity = 128; // I hope it's enough on all platforms
    err_str->data                       = (char*)calloc (err_str->length, 1);
#if _WIN32
    strerror_s (err_str->data, err_str->length, eno);
#else
    strerror_r (eno, err_str->data, err_str->length);
#endif

    if (!strlen (err_str->data)) {
        return NULL;
    }

    return err_str;
}
