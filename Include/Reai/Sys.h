#ifndef REAI_SYS_H
#define REAI_SYS_H

#include <Reai/Util/Str.h>

#ifndef SYS_ERROR_STR_MAX_LENGTH
#    define SYS_ERROR_STR_MAX_LENGTH 128
#endif

typedef unsigned long   SysProcessId;
typedef struct SysMutex SysMutex;

REAI_API Str *SysGetLocalTime (Str *timebuf);

///
/// Get size of file without opening it.
///
/// filename[in] : Name/path of file.
///
/// SUCCESS : Non-negative value representing size of file in bytes.
/// FAILURE : -1
///
REAI_API i64 SysGetFileSize (const char *filename);

///
/// Get environment value value in a `Str` object.
/// Object must be destroyed after use.
///
/// name[in]   : Name of environment variable.
/// value[out] : Value of environment variable.
///
/// SUCCESS : `Str` object containing value of environment variable.
/// FAILURE : `NULL`
///
REAI_API Str *SysGetEnv (const char *name, Str *value);

///
/// Platform independent method to get current process Id
///
REAI_API SysProcessId SysGetCurrentProcessId();

///
/// Create a platform-independent mutex object.
///
/// SUCCESS : A valid SysMutex object
/// FAILURE : `NULL`
///
REAI_API SysMutex *SysMutexCreate();

///
/// Destroy the provided mutex object.
/// Once a mutex is destroyed, all resources held by it will be freed.
/// Using it after this cal is UB.
///
/// m[in] : Mutex object to be destroyed.
///
REAI_API void SysMutexDestroy (SysMutex *m);

///
/// Acquire lock on provided mutex object.
///
/// m[in,out] : Mutex to lock.
///
/// SUCCESS : `m`
/// FAILURE : `NULL`
///
REAI_API SysMutex *SysMutexLock (SysMutex *m);

///
/// Release lock on provided mutex object.
///
/// m[in,out] : Mutex to unlock.
///
/// SUCCESS : `m`
/// FAILURE : `NULL`
///
REAI_API SysMutex *SysMutexUnlock (SysMutex *m);

///
/// Get last error using an error number.
///
/// eno[in]      : Unique error number descriptor.
/// err_str[out] : Error string will be stored in this.
///
/// SUCCESS : Error string describing last error.
/// FAILURE : NULL only if `err_str` is NULL
///
REAI_API Str *SysStrError (i32 eno, Str *err_str);

#endif
