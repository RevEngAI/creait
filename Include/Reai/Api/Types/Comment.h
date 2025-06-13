/**
 * @file Comment.h
 * @date 11th Jun 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef COMMENT_H
#define COMMENT_H

#include <Reai/Api/Types/Common.h>
#include <Reai/Util/Str.h>

typedef struct {
    Str        content;
    CommentId  id;
    UserId     user_id;
    Str        resource_type;
    ResourceId resource_id;
    struct {
        u32 start_line;
        u32 end_line;
    } context;
    Str created_at;
    Str updated_at;
} Comment;

typedef Vec (Comment) Comments;

#ifdef __cplusplus
extern "C" {
#endif

    ///
    /// Init clone of object
    /// Vectors in inited in clone will create new copies of data instead of sharing ownersip
    ///
    /// dst[out] : Destination of cloned data
    /// src[in]  : Cloning source
    ///
    /// SUCCESS : true
    /// FAILURE : Does not return
    ///
    REAI_API bool CommentInitClone (Comment* dst, Comment* src);


    ///
    /// Deinit object
    /// Must not be called manually if inside a vector that has deinit method set.
    ///
    /// dst[in] : Destination of cloned data
    ///
    REAI_API void CommentDeinit (Comment* c);

#ifdef __cplusplus
}
#endif

#endif // COMMENT_H
