#include <Reai/Api/Types/Comment.h>
#include <Reai/Log.h>

#include "Reai/Util/Str.h"

bool CommentInitClone (Comment *dst, Comment *src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid comments object provided.");
    }

    dst->content            = StrDup (&src->content);
    dst->id                 = src->id;
    dst->user_id            = src->user_id;
    dst->resource_type      = StrDup (&src->resource_type);
    dst->resource_id        = src->resource_id;
    dst->context.start_line = src->context.start_line;
    dst->context.end_line   = src->context.end_line;
    dst->created_at         = StrDup (&src->created_at);
    dst->updated_at         = StrDup (&src->updated_at);

    return true;
}

void CommentDeinit (Comment *c) {
    if (!c) {
        LOG_FATAL ("Invalid comment object provided");
    }

    StrDeinit (&c->content);
    StrDeinit (&c->resource_type);
    StrDeinit (&c->updated_at);
    StrDeinit (&c->updated_at);

    memset (c, 0, sizeof (Comment));
}
