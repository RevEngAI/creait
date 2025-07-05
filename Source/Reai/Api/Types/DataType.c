/**
 * @file DataType.c
 * @date 1st April 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#include <Reai/Api/Types/DataType.h>
#include <Reai/Log.h>

/* libc */
#include <string.h>

#include "Reai/Util/Vec.h"

void DataTypeDeinit (DataType** dt) {
    if (!dt || !*dt) {
        LOG_FATAL ("Invalid object provided. Cannot deinit. Aborting...");
    }

    DataType* d = *dt;

    StrDeinit (&d->last_change);
    StrDeinit (&d->name);
    StrDeinit (&d->type);
    StrDeinit (&d->artifact_type);
    VecDeinit (&d->members);

    memset (d, 0, sizeof (*d));
    *dt = NULL;
}

bool DataTypeInitClone (DataType** dst, DataType** src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid objects provided for cloning. Cannot clone. Aborting...");
    }

    DataType* d = *dst;
    DataType* s = *src;

    StrInitCopy (&d->last_change, &s->last_change);
    d->offset = s->offset;
    d->size   = s->size;
    StrInitCopy (&d->name, &s->name);
    StrInitCopy (&d->type, &s->type);
    StrInitCopy (&d->artifact_type, &s->artifact_type);

    d->members = VecInitWithDeepCopy_T (&d->members, DataTypeInitClone, DataTypeDeinit);
    VecInitClone (&d->members, &s->members);
    d->members.copy_init = NULL;

    return true;
}

void FunctionTypeDeinit (FunctionType* ft) {
    if (!ft) {
        LOG_FATAL ("Invalid object provided. Cannot deinit. Aborting...");
    }

    StrDeinit (&ft->last_change);
    StrDeinit (&ft->name);
    StrDeinit (&ft->return_type);
    VecDeinit (&ft->args);
    VecDeinit (&ft->stack_vars);
    VecDeinit (&ft->deps);

    memset (ft, 0, sizeof (*ft));
}

bool FunctionTypeInitClone (FunctionType* dst, FunctionType* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid objects provided for cloning. Cannot clone. Aborting...");
    }

    StrInitCopy (&dst->last_change, &src->last_change);
    dst->addr = src->addr;
    dst->size = src->size;
    StrInitCopy (&dst->name, &src->name);
    StrInitCopy (&dst->return_type, &src->return_type);
    VecInitClone (&dst->args, &src->args);
    VecInitClone (&dst->stack_vars, &src->stack_vars);
    VecInitClone (&dst->deps, &src->deps);

    return true;
}
