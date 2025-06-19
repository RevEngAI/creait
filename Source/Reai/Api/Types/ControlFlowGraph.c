/**
 * @file ControlFlowGraph.c
 * @date 25th May 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#include <Reai/Api/Types/ControlFlowGraph.h>
#include <Reai/Log.h>

/* libc */
#include <string.h>

void DestinationDeinit (Destination* dest) {
    if (!dest) {
        LOG_FATAL ("Invalid object provided. Cannot deinit. Aborting...");
    }

    StrDeinit (&dest->flowtype);
    StrDeinit (&dest->vaddr);

    memset (dest, 0, sizeof (Destination));
}

bool DestinationInitClone (Destination* dst, Destination* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid objects provided. Cannot init clone. Aborting...");
    }

    dst->destination_block_id = src->destination_block_id;
    StrInitCopy (&dst->flowtype, &src->flowtype);
    StrInitCopy (&dst->vaddr, &src->vaddr);

    return true;
}

void BlockDeinit (Block* block) {
    if (!block) {
        LOG_FATAL ("Invalid object provided. Cannot deinit. Aborting...");
    }

    VecDeinit (&block->asm_lines);
    VecDeinit (&block->destinations);
    StrDeinit (&block->comment);

    memset (block, 0, sizeof (Block));
}

bool BlockInitClone (Block* dst, Block* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid objects provided. Cannot init clone. Aborting...");
    }

    VecInitClone (&dst->asm_lines, &src->asm_lines);
    dst->id       = src->id;
    dst->min_addr = src->min_addr;
    dst->max_addr = src->max_addr;
    VecInitClone (&dst->destinations, &src->destinations);
    StrInitCopy (&dst->comment, &src->comment);

    return true;
}

void LocalVariableDeinit (LocalVariable* var) {
    if (!var) {
        LOG_FATAL ("Invalid object provided. Cannot deinit. Aborting...");
    }

    StrDeinit (&var->address);
    StrDeinit (&var->d_type);
    StrDeinit (&var->loc);
    StrDeinit (&var->name);

    memset (var, 0, sizeof (LocalVariable));
}

bool LocalVariableInitClone (LocalVariable* dst, LocalVariable* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid objects provided. Cannot init clone. Aborting...");
    }

    StrInitCopy (&dst->address, &src->address);
    StrInitCopy (&dst->d_type, &src->d_type);
    dst->size = src->size;
    StrInitCopy (&dst->loc, &src->loc);
    StrInitCopy (&dst->name, &src->name);

    return true;
}

void ControlFlowGraphDeinit (ControlFlowGraph* cfg) {
    if (!cfg) {
        LOG_FATAL ("Invalid object provided. Cannot deinit. Aborting...");
    }

    VecDeinit (&cfg->blocks);
    VecDeinit (&cfg->local_variables);
    StrDeinit (&cfg->overview_comment);

    memset (cfg, 0, sizeof (ControlFlowGraph));
}

bool ControlFlowGraphInitClone (ControlFlowGraph* dst, ControlFlowGraph* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid objects provided. Cannot init clone. Aborting...");
    }

    VecInitClone (&dst->blocks, &src->blocks);
    VecInitClone (&dst->local_variables, &src->local_variables);
    StrInitCopy (&dst->overview_comment, &src->overview_comment);

    return true;
} 