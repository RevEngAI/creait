/**
 * @file AnalysisInfo.c
 * @date 23rd July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

/* reai */
#include <Reai/Api/Types/AnalysisInfo.h>
#include <Reai/Log.h>

/**
 * @b Method to destroy cloned items.
 *
 * @param clone.
 *
 * @return @c clone on success.
 * @return @c NULL otherwise.
 * */
void AnalysisInfoDeinit (AnalysisInfo* clone) {
    if (!clone) {
        LOG_FATAL ("Invalid arguments. Cannot deinit.");
    }

    StrDeinit (&clone->creation);
    StrDeinit (&clone->binary_name);
    StrDeinit (&clone->sha256);
    StrDeinit (&clone->username);

    memset (clone, 0, sizeof (*clone));
}

/**
 * @b Method to clone function info items.
 *
 * @param dst Memory pointer where cloned data must be placed
 * @param src Memory pointer where source data is stored.
 *
 * @return @c dst on success.
 * @return @c NULL otherwise.
 * */
bool AnalysisInfoInitClone (AnalysisInfo* dst, AnalysisInfo* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid source or destination object. Cannot create clone.");
    }

    StrInitCopy (&dst->creation, &src->creation);
    StrInitCopy (&dst->binary_name, &src->binary_name);
    StrInitCopy (&dst->sha256, &src->sha256);
    StrInitCopy (&dst->username, &src->username);

    dst->binary_id        = src->binary_id;
    dst->analysis_id      = src->analysis_id;
    dst->is_private       = src->is_private;
    dst->model_id         = src->model_id;
    dst->status           = src->status;
    dst->is_owner         = src->is_owner;
    dst->binary_size      = src->binary_size;
    dst->dyn_exec_status  = src->dyn_exec_status;
    dst->dyn_exec_task_id = src->dyn_exec_task_id;

    return true;
}
