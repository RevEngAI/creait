/**
 * @file Vec.h
 * @date 18th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 *
 * @brief A very minimal vector implementation just for use inside "creait" library.
 * To use, one just needs to define item type to store inside vector and define a
 * macro containing that item type name. Then define vec_tname macro to define
 * type name for vector of given item type and then define VEC_fn_infix as the name
 * to be used in reai_fn_infix_vec_op(...).
 * */

#ifndef REAI_UTIL_VEC_H
#define REAI_UTIL_VEC_H

#include <Reai/Common.h>
#include <Reai/Types.h>

/* libc */
#include <memory.h>

#ifndef REAI_VEC_INITIAL_ITEM_CAPACITY
#    define REAI_VEC_INITIAL_ITEM_CAPACITY 32
#endif

typedef void* (*ReaiGenericCloneInit) (void* dst, void* src);
typedef void* (*ReaiGenericCloneDeinit) (void* clone);

#define REAI_VEC_FOREACH(vec, iter, body)                                                          \
    do {                                                                                           \
        if (!vec) {                                                                                \
            break;                                                                                 \
        }                                                                                          \
                                                                                                   \
        typeof (vec->items) iter = Null;                                                           \
        for (Size ___idx = 0; ___idx < (vec)->count; ___idx++) {                                   \
            iter = vec->items + ___idx;                                                            \
            { body; };                                                                             \
        }                                                                                          \
    } while (0)

/**
 * @b Use to define new vector type.
 *
 * @param vec_tname Name of new vector type.
 * @param fn_infix Function name to place in between "reai" and "_vec_<op>"
 * @param vec_itype Vector item type.
 * @param vec_iclone_init Vector item clone method (can be Null)
 * @param vec_iclone_deinit Vector item clone deinit method (can be Null)
 * */
#define REAI_MAKE_VEC(vec_tname, fn_infix, vec_itype, vec_iclone_init, vec_iclone_deinit)          \
    /* separate typedefs make sure different vector types don't mix */                             \
    typedef struct vec_tname {                                                                     \
        vec_itype* items;                                                                          \
        Size       count;                                                                          \
        Size       capacity;                                                                       \
    } vec_tname;                                                                                   \
                                                                                                   \
    /* forward declarations */                                                                     \
    PRIVATE vec_tname* reai_##fn_infix##_vec_create();                                             \
    PRIVATE void       reai_##fn_infix##_vec_destroy (vec_tname* vec);                             \
    PRIVATE vec_tname* reai_##fn_infix##_vec_init (vec_tname* vec);                                \
    PRIVATE vec_tname* reai_##fn_infix##_vec_deinit (vec_tname* vec);                              \
    PRIVATE vec_tname* reai_##fn_infix##_vec_append (vec_tname* vec, vec_itype* item);             \
                                                                                                   \
    PRIVATE vec_tname* reai_##fn_infix##_vec_create() {                                            \
        vec_tname* vec = Null;                                                                     \
        if (!(vec = reai_##fn_infix##_vec_init (NEW (vec_tname)))) {                               \
            PRINT_ERR (ERR_OBJECT_INITIALIZATION_FAILED);                                          \
            reai_##fn_infix##_vec_destroy (vec);                                                   \
        }                                                                                          \
        return vec;                                                                                \
    }                                                                                              \
                                                                                                   \
    PRIVATE void reai_##fn_infix##_vec_destroy (vec_tname* vec) {                                  \
        RETURN_IF (!vec, ERR_INVALID_ARGUMENTS);                                                   \
        reai_##fn_infix##_vec_deinit (vec);                                                        \
        FREE (vec);                                                                                \
    }                                                                                              \
                                                                                                   \
    PRIVATE vec_tname* reai_##fn_infix##_vec_init (vec_tname* vec) {                               \
        RETURN_VALUE_IF (!vec, Null, ERR_INVALID_ARGUMENTS);                                       \
                                                                                                   \
        Size cap   = REAI_VEC_INITIAL_ITEM_CAPACITY;                                               \
        vec->items = ALLOCATE (vec_itype, cap);                                                    \
        RETURN_VALUE_IF (!vec->items, Null, ERR_OUT_OF_MEMORY);                                    \
        vec->count    = 0;                                                                         \
        vec->capacity = cap;                                                                       \
                                                                                                   \
        return vec;                                                                                \
    }                                                                                              \
                                                                                                   \
    PRIVATE vec_tname* reai_##fn_infix##_vec_deinit (vec_tname* vec) {                             \
        RETURN_VALUE_IF (!vec, Null, ERR_INVALID_ARGUMENTS);                                       \
                                                                                                   \
        ReaiGenericCloneDeinit deiniter = (ReaiGenericCloneDeinit)vec_iclone_deinit;               \
        if (deiniter != Null) {                                                                    \
            for (Size s = 0; s < vec->count; s++) {                                                \
                deiniter (vec->items + s);                                                         \
            }                                                                                      \
        } else {                                                                                   \
            memset (vec->items, 0, sizeof (vec_itype) * vec->count);                               \
        }                                                                                          \
                                                                                                   \
        FREE (vec->items);                                                                         \
        memset (vec, 0, sizeof (vec_tname));                                                       \
                                                                                                   \
        return vec;                                                                                \
    }                                                                                              \
                                                                                                   \
    PRIVATE vec_tname* reai_##fn_infix##_vec_append (vec_tname* vec, vec_itype* item) {            \
        RETURN_VALUE_IF (!vec || !item, Null, ERR_INVALID_ARGUMENTS);                              \
                                                                                                   \
        if (vec->count >= vec->capacity) {                                                         \
            Size       newcap = vec->count ? vec->count * 2 : REAI_VEC_INITIAL_ITEM_CAPACITY;      \
            vec_itype* temp   = REALLOCATE (vec->items, vec_itype, newcap);                        \
            RETURN_VALUE_IF (!temp, Null, ERR_OUT_OF_MEMORY);                                      \
                                                                                                   \
            vec->items    = temp;                                                                  \
            vec->capacity = newcap;                                                                \
        }                                                                                          \
                                                                                                   \
        ReaiGenericCloneInit initer = (ReaiGenericCloneInit)vec_iclone_init;                       \
        if (initer != Null) {                                                                      \
            RETURN_VALUE_IF (                                                                      \
                !initer (vec->items + vec->count++, item),                                         \
                Null,                                                                              \
                "Failed to make item clone"                                                        \
            );                                                                                     \
        } else {                                                                                   \
            vec->items[vec->count++] = *item;                                                      \
        }                                                                                          \
                                                                                                   \
        return vec;                                                                                \
    }                                                                                              \
                                                                                                   \
    PRIVATE vec_tname* reai_##fn_infix##_vec_clone_create (vec_tname* vec) {                       \
        RETURN_VALUE_IF (!vec, Null, ERR_INVALID_ARGUMENTS);                                       \
                                                                                                   \
        vec_tname* clone_vec = reai_##fn_infix##_vec_create();                                     \
        RETURN_VALUE_IF (                                                                          \
            !clone_vec,                                                                            \
            Null,                                                                                  \
            "Failed to create new '%s' vector to create clone",                                    \
            #vec_tname                                                                             \
        );                                                                                         \
                                                                                                   \
        if (vec->capacity > clone_vec->capacity) {                                                 \
            vec_itype* temp = REALLOCATE (clone_vec->items, vec_itype, vec->capacity);             \
            RETURN_VALUE_IF (!temp, Null, ERR_OUT_OF_MEMORY);                                      \
                                                                                                   \
            clone_vec->items    = temp;                                                            \
            clone_vec->capacity = vec->capacity;                                                   \
        }                                                                                          \
                                                                                                   \
        ReaiGenericCloneInit initer = (ReaiGenericCloneInit)vec_iclone_init;                       \
        if (initer) {                                                                              \
            for (Size s = 0; s < vec->count; s++) {                                                \
                vec_itype* clone_item = clone_vec->items + clone_vec->count++;                     \
                vec_itype* item       = vec->items + s;                                            \
                GOTO_HANDLER_IF (                                                                  \
                    !initer (clone_item, item),                                                    \
                    CLONE_FAILED,                                                                  \
                    "Failed to create item clone"                                                  \
                );                                                                                 \
            }                                                                                      \
        } else {                                                                                   \
            memcpy (clone_vec->items, vec->items, sizeof (vec_itype) * vec->count);                \
            clone_vec->count = vec->count;                                                         \
        }                                                                                          \
        return clone_vec;                                                                          \
                                                                                                   \
CLONE_FAILED:                                                                                      \
        reai_##fn_infix##_vec_destroy (clone_vec);                                                 \
        return Null;                                                                               \
    }

#endif
