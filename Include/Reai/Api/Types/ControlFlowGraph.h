/**
 * @file ControlFlowGraph.h
 * @date 25th May 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_CONTROL_FLOW_GRAPH_H
#define REAI_CONTROL_FLOW_GRAPH_H

#include <Reai/Api/Types/Common.h>
#include <Reai/Types.h>
#include <Reai/Util/Str.h>
#include <Reai/Util/Vec.h>

typedef struct Destination {
    u64 destination_block_id;
    Str flowtype;
    Str vaddr;
} Destination;

typedef Vec (Destination) Destinations;

typedef struct Block {
    Strs         asm_lines;    /**< Assembly instructions in this block */
    u64          id;           /**< Block ID */
    u64          min_addr;     /**< Minimum address of block */
    u64          max_addr;     /**< Maximum address of block */
    Destinations destinations; /**< Destination blocks and flow types */
    Str          comment;      /**< Comment for this block */
} Block;

typedef Vec (Block) Blocks;

// XXX: conflicts with definition in DataType.h
// Both are mergeable! Defining a separate one creates confusion
// and type-redundancy
typedef struct LocalVariable {
    Str address;
    Str d_type;
    u64 size;
    Str loc;
    Str name;
} LocalVariable;

typedef Vec (LocalVariable) LocalVariables;

typedef struct ControlFlowGraph {
    Blocks         blocks;
    LocalVariables local_variables;
    Str            overview_comment;
} ControlFlowGraph;

#ifdef __cplusplus
extern "C" {
#endif

    ///
    /// Deinit cloned Destination object. Provided pointer is not freed.
    /// That must be taken care of by the owner.
    ///
    /// dest[in,out] : Object to be destroyed.
    ///
    REAI_API void DestinationDeinit (Destination* dest);

    ///
    /// Clone a Destination object from `src` to `dst`
    ///
    /// dst[out] : Destination object.
    /// src[in]  : Source object.
    ///
    /// SUCCESS : True
    /// FAILURE : Does not return
    ///
    REAI_API bool DestinationInitClone (Destination* dst, Destination* src);

    ///
    /// Deinit cloned Block object. Provided pointer is not freed.
    /// That must be taken care of by the owner.
    ///
    /// block[in,out] : Object to be destroyed.
    ///
    REAI_API void BlockDeinit (Block* block);

    ///
    /// Clone a Block object from `src` to `dst`
    ///
    /// dst[out] : Destination object.
    /// src[in]  : Source object.
    ///
    /// SUCCESS : True
    /// FAILURE : Does not return
    ///
    REAI_API bool BlockInitClone (Block* dst, Block* src);

    ///
    /// Deinit cloned LocalVariable object. Provided pointer is not freed.
    /// That must be taken care of by the owner.
    ///
    /// var[in,out] : Object to be destroyed.
    ///
    REAI_API void LocalVariableDeinit (LocalVariable* var);

    ///
    /// Clone a LocalVariable object from `src` to `dst`
    ///
    /// dst[out] : Destination object.
    /// src[in]  : Source object.
    ///
    /// SUCCESS : True
    /// FAILURE : Does not return
    ///
    REAI_API bool LocalVariableInitClone (LocalVariable* dst, LocalVariable* src);

    ///
    /// Deinit cloned ControlFlowGraph object. Provided pointer is not freed.
    /// That must be taken care of by the owner.
    ///
    /// cfg[in,out] : Object to be destroyed.
    ///
    REAI_API void ControlFlowGraphDeinit (ControlFlowGraph* cfg);

    ///
    /// Clone a ControlFlowGraph object from `src` to `dst`
    ///
    /// dst[out] : Destination object.
    /// src[in]  : Source object.
    ///
    /// SUCCESS : True
    /// FAILURE : Does not return
    ///
    REAI_API bool ControlFlowGraphInitClone (ControlFlowGraph* dst, ControlFlowGraph* src);

#ifdef __cplusplus
}
#endif

#endif // REAI_CONTROL_FLOW_GRAPH_H
