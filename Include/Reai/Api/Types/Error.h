/**
 * @file ApiError.h
 * @date 5th Dec 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_API_ERROR_H
#define REAI_API_ERROR_H

#include <Reai/Api/Types/Common.h>
#include <Reai/Types.h>
#include <Reai/Util/Str.h>

typedef struct {
    Str code;
    Str message;
} Error;

#ifdef __cplusplus
extern "C" {
#endif

    REAI_API bool ErrorInitClone (Error* dst, Error* src);
    REAI_API void ErrorDeinit (Error* e);

#ifdef __cplusplus
}
#endif

#endif // REAI_API_ERROR_H
