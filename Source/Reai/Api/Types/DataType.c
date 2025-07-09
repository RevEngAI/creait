/**
 * @file DataType.c
 * @date 9th July 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#include <Reai/Api/Types/DataType.h>
#include <Reai/Log.h>

void NameValueDeinit (NameValue* nv) {
    if (!nv) {
        LOG_FATAL ("Invalid object provided. Cannot deinit. Aborting...");
    }

    StrDeinit (&nv->name);
    memset (nv, 0, sizeof (*nv));
}

bool NameValueInitClone (NameValue* dst, NameValue* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid objects provided for cloning. Cannot clone. Aborting...");
    }

    StrInitCopy (&dst->name, &src->name);
    dst->value = src->value;

    return true;
}

void EnumDeinit (Enum* e) {
    if (!e) {
        LOG_FATAL ("Invalid object provided. Cannot deinit. Aborting...");
    }

    StrDeinit (&e->name);
    VecDeinit (&e->members);

    memset (e, 0, sizeof (*e));
}

bool EnumInitClone (Enum* dst, Enum* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid objects provided for cloning. Cannot clone. Aborting...");
    }

    StrInitCopy (&dst->name, &src->name);
    VecInitClone (&dst->members, &src->members);

    return true;
}

void StructMemberDeinit (StructMember* sm) {
    if (!sm) {
        LOG_FATAL ("Invalid object provided. Cannot deinit. Aborting...");
    }

    StrDeinit (&sm->name);
    StrDeinit (&sm->type);

    memset (sm, 0, sizeof (*sm));
}

bool StructMemberInitClone (StructMember* dst, StructMember* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid objects provided for cloning. Cannot clone. Aborting...");
    }

    StrInitCopy (&dst->name, &src->name);
    StrInitCopy (&dst->type, &src->type);
    dst->offset = src->offset;
    dst->size   = src->size;

    return true;
}

void StructDeinit (Struct* s) {
    if (!s) {
        LOG_FATAL ("Invalid object provided. Cannot deinit. Aborting...");
    }

    StrDeinit (&s->last_change);
    StrDeinit (&s->name);
    StrDeinit (&s->type);
    StrDeinit (&s->artifact_type);
    VecDeinit (&s->members);

    memset (s, 0, sizeof (*s));
}

bool StructInitClone (Struct* dst, Struct* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid objects provided for cloning. Cannot clone. Aborting...");
    }

    StrInitCopy (&dst->last_change, &src->last_change);
    dst->offset = src->offset;
    dst->size   = src->size;
    StrInitCopy (&dst->name, &src->name);
    StrInitCopy (&dst->type, &src->type);
    StrInitCopy (&dst->artifact_type, &src->artifact_type);
    VecInitClone (&dst->members, &src->members);

    return true;
}

void LocalVariableDeinit (LocalVariable* lv) {
    if (!lv) {
        LOG_FATAL ("Invalid object provided. Cannot deinit. Aborting...");
    }

    StrDeinit (&lv->name);
    StrDeinit (&lv->type);

    memset (lv, 0, sizeof (*lv));
}

bool LocalVariableInitClone (LocalVariable* dst, LocalVariable* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid objects provided for cloning. Cannot clone. Aborting...");
    }

    dst->offset = src->offset;
    dst->size   = src->size;
    StrInitCopy (&dst->name, &src->name);
    StrInitCopy (&dst->type, &src->type);
    dst->addr = src->addr;

    return true;
}

void GlobalVariableDeinit (GlobalVariable* gv) {
    if (!gv) {
        LOG_FATAL ("Invalid object provided. Cannot deinit. Aborting...");
    }

    StrDeinit (&gv->name);
    StrDeinit (&gv->type);

    memset (gv, 0, sizeof (*gv));
}

bool GlobalVariableInitClone (GlobalVariable* dst, GlobalVariable* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid objects provided for cloning. Cannot clone. Aborting...");
    }

    dst->size = src->size;
    StrInitCopy (&dst->name, &src->name);
    StrInitCopy (&dst->type, &src->type);
    dst->addr = src->addr;

    return true;
}

void FunctionDeinit (Function* f) {
    if (!f) {
        LOG_FATAL ("Invalid object provided. Cannot deinit. Aborting...");
    }

    StrDeinit (&f->last_change);
    StrDeinit (&f->name);
    StrDeinit (&f->return_type);
    VecDeinit (&f->args);
    VecDeinit (&f->stack_vars);
    VecDeinit (&f->deps.enums);
    VecDeinit (&f->deps.structs);
    VecDeinit (&f->deps.global_vars);

    memset (f, 0, sizeof (*f));
}

bool FunctionInitClone (Function* dst, Function* src) {
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
    VecInitClone (&dst->deps.enums, &src->deps.enums);
    VecInitClone (&dst->deps.structs, &src->deps.structs);
    VecInitClone (&dst->deps.global_vars, &src->deps.global_vars);

    return true;
}

void TypedefDeinit (Typedef* td) {
    if (!td) {
        LOG_FATAL ("Invalid object provided. Cannot deinit. Aborting...");
    }

    StrDeinit (&td->name);
    StrDeinit (&td->type);

    memset (td, 0, sizeof (*td));
}

bool TypedefInitClone (Typedef* dst, Typedef* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid objects provided for cloning. Cannot clone. Aborting...");
    }

    StrInitCopy (&dst->name, &src->name);
    StrInitCopy (&dst->type, &src->type);

    return true;
}
