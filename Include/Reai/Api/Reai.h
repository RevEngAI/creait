/**
 * @file Reai.h
 * @date 9th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_API_REAI_H
#define REAI_API_REAI_H

#include <Reai/Common.h>
#include <Reai/Types.h>

C_SOURCE_BEGIN

/* fwd declarations */
typedef void                CURL;
typedef struct Reai         Reai; /* opaque object */
typedef struct ReaiResponse ReaiResponse;
typedef struct ReaiRequest  ReaiRequest;

PUBLIC Reai*         reai_create (CString host, CString api_key);
PUBLIC void          reai_destroy (Reai* reai);
PUBLIC ReaiResponse* reai_request (Reai* reai, ReaiRequest* req, ReaiResponse* response);

C_SOURCE_END

#endif // REAI_API_REAI_H
