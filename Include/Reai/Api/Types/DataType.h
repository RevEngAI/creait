#ifndef REAI_DATA_TYPE_H
#define REAI_DATA_TYPE_H

#include <Reai/Api/Types/Common.h>
#include <Reai/Types.h>
#include <Reai/Util/Str.h>

typedef struct DataType DataType;
typedef Vec (DataType*) DataTypes;

struct DataType {
    Str       last_change;
    u64       offset;
    u64       size;
    Str       name;
    Str       type;
    Str       artifact_type;
    DataTypes members;
};

#ifdef __cplusplus
#    define DataTypeInit()                                                                         \
        (DataType {                                                                                \
            .last_change   = StrInit(),                                                            \
            .offset        = 0,                                                                    \
            .size          = 0,                                                                    \
            .name          = StrInit(),                                                            \
            .type          = StrInit(),                                                            \
            .artifact_type = StrInit(),                                                            \
            .members       = VecInitWithDeepCopy (NULL, DataTypeDeinit)                            \
        })
#else
#    define DataTypeInit()                                                                         \
        ((DataType) {.last_change   = StrInit(),                                                   \
                     .offset        = 0,                                                           \
                     .size          = 0,                                                           \
                     .name          = StrInit(),                                                   \
                     .type          = StrInit(),                                                   \
                     .artifact_type = StrInit(),                                                   \
                     .members       = VecInitWithDeepCopy (NULL, DataTypeDeinit)})
#endif

typedef struct FunctionType {
    Str       last_change;
    u64       addr;
    u64       size;
    Str       name;
    Str       return_type;
    DataTypes args;
    DataTypes stack_vars;
    DataTypes deps;
} FunctionType;

#ifdef __cplusplus
#    define FunctionTypeInit()                                                                     \
        (FunctionType {                                                                            \
            .last_change = StrInit(),                                                              \
            .addr        = 0,                                                                      \
            .size        = 0,                                                                      \
            .name        = StrInit(),                                                              \
            .return_type = StrInit(),                                                              \
            .args        = VecInitWithDeepCopy (NULL, DataTypeDeinit),                             \
            .stack_vars  = VecInitWithDeepCopy (NULL, DataTypeDeinit),                             \
            .deps        = VecInitWithDeepCopy (NULL, DataTypeDeinit)                              \
        })
#else
#    define FunctionTypeInit()                                                                     \
        ((FunctionType) {.last_change = StrInit(),                                                 \
                         .addr        = 0,                                                         \
                         .size        = 0,                                                         \
                         .name        = StrInit(),                                                 \
                         .return_type = StrInit(),                                                 \
                         .args        = VecInitWithDeepCopy (NULL, DataTypeDeinit),                \
                         .stack_vars  = VecInitWithDeepCopy (NULL, DataTypeDeinit),                \
                         .deps        = VecInitWithDeepCopy (NULL, DataTypeDeinit)})
#endif

typedef Vec (FunctionType) FunctionTypes;

#ifdef __cplusplus
extern "C" {
#endif

    ///
    /// Deinit DataType object clone. This won't free provided pointer.
    /// That must be done by owner.
    ///
    /// dt[in] : DataType object.
    ///
    /// RETURN : Does not return on failure
    ///
    REAI_API void DataTypeDeinit (DataType** dt);

    ///
    /// Create clone of given `src` object into `dst` object
    ///
    /// dst[out] : Destination DataType object.
    /// src[in]  : Source DataType object.
    ///
    /// SUCCESS : true
    /// FAILURE : Does not return on failure
    ///
    REAI_API bool DataTypeInitClone (DataType** dst, DataType** src);

    ///
    /// Deinit FunctionType object clone. This won't free provided pointer.
    /// That must be done by owner.
    ///
    /// ft[in] : FunctionType object.
    ///
    /// RETURN : Does not return on failure
    ///
    REAI_API void FunctionTypeDeinit (FunctionType* ft);

    ///
    /// Create clone of given `src` object into `dst` object
    ///
    /// dst[out] : Destination FunctionType object.
    /// src[in]  : Source FunctionType object.
    ///
    /// SUCCESS : true
    /// FAILURE : Does not return on failure
    ///
    REAI_API bool FunctionTypeInitClone (FunctionType* dst, FunctionType* src);

#ifdef __cplusplus
}
#endif

#endif // REAI_DATA_TYPE_H
