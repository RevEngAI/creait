/**
 * @file QueryResult.h
 * @date 24th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_QUERY_RESULT_H
#define REAI_QUERY_RESULT_H

#include <Reai/AnalysisInfo.h>
#include <Reai/Api/Request.h>
#include <Reai/Types.h>
#include <Reai/Util/CStrVec.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct ReaiQueryResult {
        Uint64             binary_id;
        CString            binary_name;
        CStrVec*           collections;
        CString            creation;
        Uint64             model_id;
        CString            model_name;
        CString            sha_256_hash;
        ReaiAnalysisStatus status;
        CStrVec*           tags;
    } ReaiQueryResult;

    ReaiQueryResult* reai_query_result_clone_init (ReaiQueryResult* dst, ReaiQueryResult* src);
    ReaiQueryResult* reai_query_result_clone_deinit (ReaiQueryResult* clone);

    REAI_MAKE_VEC (
        ReaiQueryResultVec,
        query_result,
        ReaiQueryResult,
        reai_query_result_clone_init,
        reai_query_result_clone_deinit
    );

#ifdef __cplusplus
}
#endif

#endif // REAI_QUERY_RESULT_H
