#include <Reai/File.h>
#include <Reai/Log.h>
#include <Reai/Sys.h>

// libc
#include <errno.h>
#include <stdio.h>

bool ReadCompleteFile (const char *filename, char **data, size *file_size, size *capacity) {
    if (!filename || !data || !file_size || !capacity) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    // get actual size of file
    i64 size = SysGetFileSize (filename);
    if (-1 == size) {
        LOG_ERROR ("failed to get file size");
        return false;
    }

    // allocate memory to hold the file contents if required
    char *buffer = *data;
    if (*capacity < (u64)size) {
        buffer = realloc (buffer, size + 1);
        if (!buffer) {
            Str syserr;
            StrInitStack (&syserr, SYS_ERROR_STR_MAX_LENGTH, {
                LOG_FATAL ("malloc() failed : %s.", SysStrError (errno, &syserr)->data);
            });
        }

        *capacity = size + 1;
    }

    // Open the file in binary mode
    FILE *file = NULL;
    int   e    = 0;
#ifdef _WIN32
    e = fopen_s (&file, filename, "rb");
#else
    file = fopen (filename, "rb");
    if (!file) {
        e = errno;
    }
#endif
    if (e || !file) {
        free (buffer);

        Str syserr;
        StrInitStack (&syserr, SYS_ERROR_STR_MAX_LENGTH, {
            LOG_ERROR ("fopen() failed : %s.", SysStrError (e, &syserr)->data);
        });
    }

    // Read the entire file into the buffer
    if (size != (i64)fread (buffer, 1, size, file)) {
        fclose (file);
        Str syserr;
        StrInitStack (&syserr, SYS_ERROR_STR_MAX_LENGTH, {
            LOG_ERROR ("failed to read complete file. : %s", SysStrError (errno, &syserr)->data);
        });
    }

    // Close the file and return the buffer
    fclose (file);

    ((char *)buffer)[size] = 0; // null-termination for just in case.
    *data                  = buffer;
    *file_size             = size;
    return true;
}
