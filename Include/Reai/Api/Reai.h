/**
 * @file Reai.h
 * @date 9th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_API_REAI_H
#define REAI_API_REAI_H

#include <Reai/Api/Request.h>
#include <Reai/Common.h>
#include <Reai/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

    /* fwd declarations */
    typedef void                CURL;
    typedef struct Reai         Reai; /* opaque object */
    typedef struct ReaiResponse ReaiResponse;
    typedef struct ReaiRequest  ReaiRequest;

    Reai*         reai_create (CString host, CString api_key);
    void          reai_destroy (Reai* reai);
    ReaiResponse* reai_request (Reai* reai, ReaiRequest* req, ReaiResponse* response);
    CString       reai_upload_file (Reai* reai, ReaiResponse* response, CString file_path);
    BinaryId      reai_create_analysis (
             Reai*          reai,
             ReaiResponse*  response,
             ReaiModel      model,
             ReaiFnInfoVec* fn_info_vec,
             Bool           is_private,
             CString        sha_256_hash,
             CString        file_name,
             CString        cmdline_args,
             Size           size_in_bytes
         );


#ifdef __cplusplus
}
#endif

#endif // REAI_API_REAI_H
