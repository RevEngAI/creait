/**
 * @file DataType.h
 * @date 9th July 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */


#ifndef REAI_DATA_TYPE_H
#define REAI_DATA_TYPE_H

#include <Reai/Api/Types/Common.h>
#include <Reai/Types.h>
#include <Reai/Util/Str.h>

typedef struct Typedef {
    Str name;
    Str type;
} Typedef;

typedef Vec (Typedef) Typedefs;

typedef struct NameValue {
    Str name;
    i64 value;
} NameValue;

typedef Vec (NameValue) NameValues;

typedef struct Enum {
    Str        name;
    NameValues members;
} Enum;

typedef Vec (Enum) Enums;

typedef struct StructMember {
    Str name;
    Str type;
    u64 offset;
    u64 size;
} StructMember, FunctionArgument;

typedef Vec (StructMember) StructMembers;
typedef Vec (FunctionArgument) FunctionArguments;

typedef struct Struct {
    u64           offset;
    u64           size;
    Str           name;
    Str           type;
    StructMembers members;
} Struct;

typedef Vec (Struct) Structs;

/// XXX: Conflicts with definition in ControlFlowGraph.h
/// Both types are mergeable!
/// Same goes with some other types like FunctionArgument
typedef struct LocalVariable {
    u64 offset;
    u64 size;
    Str name;
    Str type;
    u64 addr;
} LocalVariable;

typedef Vec (LocalVariable) LocalVariables;

typedef struct GlobalVariable {
    u64 size;
    Str name;
    Str type;
    u64 addr;
} GlobalVariable;

typedef Vec (GlobalVariable) GlobalVariables;

typedef struct Function {
    u64               addr;
    u64               size;
    Str               name;
    Str               return_type;
    FunctionArguments args;
    LocalVariables    stack_vars;
    struct {
        Enums           enums;
        Structs         structs;
        GlobalVariables global_vars;
        Typedefs        typedefs;
    } deps;
} Function;

typedef Vec (Function) Functions;

#ifdef __cplusplus
extern "C" {
#endif

    ///
    /// Deinit NameValue object clone. This won't free provided pointer.
    /// That must be done by owner.
    ///
    /// nv[in] : NameValue object.
    ///
    /// RETURN : Does not return on failure
    ///
    REAI_API void NameValueDeinit (NameValue* nv);

    ///
    /// Create clone of given `src` object into `dst` object
    ///
    /// dst[out] : Destination NameValue object.
    /// src[in]  : Source NameValue object.
    ///
    /// SUCCESS : true
    /// FAILURE : Does not return on failure
    ///
    REAI_API bool NameValueInitClone (NameValue* dst, NameValue* src);

    ///
    /// Deinit Enum object clone. This won't free provided pointer.
    /// That must be done by owner.
    ///
    /// e[in] : Enum object.
    ///
    /// RETURN : Does not return on failure
    ///
    REAI_API void EnumDeinit (Enum* e);

    ///
    /// Create clone of given `src` object into `dst` object
    ///
    /// dst[out] : Destination Enum object.
    /// src[in]  : Source Enum object.
    ///
    /// SUCCESS : true
    /// FAILURE : Does not return on failure
    ///
    REAI_API bool EnumInitClone (Enum* dst, Enum* src);

    ///
    /// Deinit StructMember object clone. This won't free provided pointer.
    /// That must be done by owner.
    ///
    /// nv[in] : StructMember object.
    ///
    /// RETURN : Does not return on failure
    ///
    REAI_API void StructMemberDeinit (StructMember* nv);

    ///
    /// Create clone of given `src` object into `dst` object
    ///
    /// dst[out] : Destination StructMember object.
    /// src[in]  : Source StructMember object.
    ///
    /// SUCCESS : true
    /// FAILURE : Does not return on failure
    ///
    REAI_API bool StructMemberInitClone (StructMember* dst, StructMember* src);

    ///
    /// Deinit Struct object clone. This won't free provided pointer.
    /// That must be done by owner.
    ///
    /// s[in] : Struct object.
    ///
    /// RETURN : Does not return on failure
    ///
    REAI_API void StructDeinit (Struct* s);

    ///
    /// Create clone of given `src` object into `dst` object
    ///
    /// dst[out] : Destination Struct object.
    /// src[in]  : Source Struct object.
    ///
    /// SUCCESS : true
    /// FAILURE : Does not return on failure
    ///
    REAI_API bool StructInitClone (Struct* dst, Struct* src);

    ///
    /// Deinit LocalVariable object clone. This won't free provided pointer.
    /// That must be done by owner.
    ///
    /// lv[in] : LocalVariable object.
    ///
    /// RETURN : Does not return on failure
    ///
    REAI_API void LocalVariableDeinit (LocalVariable* lv);

    ///
    /// Create clone of given `src` object into `dst` object
    ///
    /// dst[out] : Destination LocalVariable object.
    /// src[in]  : Source LocalVariable object.
    ///
    /// SUCCESS : true
    /// FAILURE : Does not return on failure
    ///
    REAI_API bool LocalVariableInitClone (LocalVariable* dst, LocalVariable* src);

    ///
    /// Deinit GlobalVariable object clone. This won't free provided pointer.
    /// That must be done by owner.
    ///
    /// lv[in] : GlobalVariable object.
    ///
    /// RETURN : Does not return on failure
    ///
    REAI_API void GlobalVariableDeinit (GlobalVariable* lv);

    ///
    /// Create clone of given `src` object into `dst` object
    ///
    /// dst[out] : Destination GlobalVariable object.
    /// src[in]  : Source GlobalVariable object.
    ///
    /// SUCCESS : true
    /// FAILURE : Does not return on failure
    ///
    REAI_API bool GlobalVariableInitClone (GlobalVariable* dst, GlobalVariable* src);

    ///
    /// Deinit Function object clone. This won't free provided pointer.
    /// That must be done by owner.
    ///
    /// ft[in] : Function object.
    ///
    /// RETURN : Does not return on failure
    ///
    REAI_API void FunctionDeinit (Function* ft);

    ///
    /// Create clone of given `src` object into `dst` object
    ///
    /// dst[out] : Destination Function object.
    /// src[in]  : Source Function object.
    ///
    /// SUCCESS : true
    /// FAILURE : Does not return on failure
    ///
    REAI_API bool FunctionInitClone (Function* dst, Function* src);

    ///
    /// Deinit FunctionArgument object clone. This won't free provided pointer.
    /// That must be done by owner.
    ///
    /// fa[in] : FunctionArgument object.
    ///
    /// RETURN : Does not return on failure
    ///
    static inline void FunctionArgumentDeinit (FunctionArgument* fa) {
        StructMemberDeinit (fa);
    }

    ///
    /// Create clone of given `src` object into `dst` object
    ///
    /// dst[out] : Destination FunctionArgument object.
    /// src[in]  : Source FunctionArgument object.
    ///
    /// SUCCESS : true
    /// FAILURE : Does not return on failure
    ///
    static inline bool FunctionArgumentInitClone (FunctionArgument* dst, FunctionArgument* src) {
        return StructMemberInitClone (dst, src);
    }

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#    define FunctionInit()                                                                         \
        (Function {                                                                                \
            .last_change = StrInit(),                                                              \
            .addr        = 0,                                                                      \
            .size        = 0,                                                                      \
            .name        = StrInit(),                                                              \
            .return_type = StrInit(),                                                              \
            .args        = VecInitWithDeepCopy (NULL, FunctionArgumentDeinit),                     \
            .stack_vars  = VecInitWithDeepCopy (NULL, LocalVariableDeinit),                        \
            .deps        = {                                                                       \
                            .enums       = VecInitWithDeepCopy (NULL, EnumDeinit),                      \
                            .structs     = VecInitWithDeepCopy (NULL, StructDeinit),                    \
                            .global_vars = VecInitWithDeepCopy (NULL, GlobalVariableDeinit),            \
                            .typedefs    = VecInitWithDeepCopy (NULL, TypedefDeinit)                    \
            }                                                                               \
        })

#    define LocalVariableInit()                                                                    \
        (LocalVariable {.offset = 0, .size = 0, .name = StrInit(), .type = StrInit(), .addr = 0})

#    define GlobalVariableInit()                                                                   \
        (GlobalVariable {.size = 0, .name = StrInit(), .type = StrInit(), .addr = 0})

#    define StructInit()                                                                           \
        (Struct {                                                                                  \
            .last_change   = StrInit(),                                                            \
            .offset        = 0,                                                                    \
            .size          = 0,                                                                    \
            .name          = StrInit(),                                                            \
            .type          = StrInit(),                                                            \
            .artifact_type = StrInit(),                                                            \
            .members       = VecInitWithDeepCopy (NULL, StructDeinit)                              \
        })

#    define StructMemberInit()                                                                     \
        (StructMember {.name = StrInit(), .type = StrInit(), .offset = 0, .size = 0})
#else

///
/// Initialize a Function object with default values.
///
/// RETURN : A fully initialized Function object:
///          - last_change, name, return_type: empty strings
///          - addr, size: zero
///          - args, stack_vars: empty vectors with deep copy support
///          - deps.enums, deps.structs, deps.global_vars, deps.typedefs: empty vectors
///
#    define FunctionInit()                                                                         \
        ((Function) {                                                                              \
            .last_change = StrInit(),                                                              \
            .addr        = 0,                                                                      \
            .size        = 0,                                                                      \
            .name        = StrInit(),                                                              \
            .return_type = StrInit(),                                                              \
            .args        = VecInitWithDeepCopy (NULL, FunctionArgumentDeinit),                     \
            .stack_vars  = VecInitWithDeepCopy (NULL, LocalVariableDeinit),                        \
            .deps        = {                                                                       \
                            .enums       = VecInitWithDeepCopy (NULL, EnumDeinit),                      \
                            .structs     = VecInitWithDeepCopy (NULL, StructDeinit),                    \
                            .global_vars = VecInitWithDeepCopy (NULL, GlobalVariableDeinit),            \
                            .typedefs    = VecInitWithDeepCopy (NULL, TypedefDeinit)                    \
            }                                                                               \
        })

///
/// Initialize a LocalVariable object with default values.
///
/// RETURN : A fully initialized LocalVariable object:
///          - name, type: empty strings
///          - offset, size, addr: zero
///
#    define LocalVariableInit()                                                                    \
        ((LocalVariable) {.offset = 0, .size = 0, .name = StrInit(), .type = StrInit(), .addr = 0})

///
/// Initialize a GlobalVariable object with default values.
///
/// RETURN : A fully initialized GlobalVariable object:
///          - name, type: empty strings
///          - size, addr: zero
///
#    define GlobalVariableInit()                                                                   \
        ((GlobalVariable) {.size = 0, .name = StrInit(), .type = StrInit(), .addr = 0})

///
/// Initialize a Struct object with default values.
///
/// RETURN : A fully initialized Struct object:
///          - last_change, name, type, artifact_type: empty strings
///          - offset, size: zero
///          - members: empty vector with deep copy support
///
#    define StructInit()                                                                           \
        ((Struct) {.last_change   = StrInit(),                                                     \
                   .offset        = 0,                                                             \
                   .size          = 0,                                                             \
                   .name          = StrInit(),                                                     \
                   .type          = StrInit(),                                                     \
                   .artifact_type = StrInit(),                                                     \
                   .members       = VecInitWithDeepCopy (NULL, StructDeinit)})

///
/// Initialize a StructMember object with default values.
///
/// RETURN : A fully initialized StructMember object:
///          - name, type: empty strings
///          - offset, size: zero
///
#    define StructMemberInit()                                                                     \
        ((StructMember) {.name = StrInit(), .type = StrInit(), .offset = 0, .size = 0})
#endif // __cplusplus

#endif // REAI_DATA_TYPE_H
