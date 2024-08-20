#include <Reai/FnInfo.h>


/**
 * @b Method to clone function info items.
 *
 * @param dst Memory pointer where cloned data must be placed
 * @param src Memory pointer where source data is stored.
 *
 * @return @c dst on success.
 * @return @c Null otherwise.
 * */
ReaiFnInfo* reai_fn_info_clone_init (ReaiFnInfo* dst, ReaiFnInfo* src) {
    RETURN_VALUE_IF (!dst || !src, (ReaiFnInfo*)Null, ERR_INVALID_ARGUMENTS);

    dst->name = strdup (src->name);
    RETURN_VALUE_IF (!dst->name, (ReaiFnInfo*)Null, ERR_OUT_OF_MEMORY);
    dst->id    = src->id;
    dst->vaddr = src->vaddr;
    dst->size  = src->size;

    return dst;
}

/**
 * @b Method to destroy cloned items.
 *
 * @param clone.
 *
 * @return @c clone on success.
 * @return @c Null otherwise.
 * */
ReaiFnInfo* reai_fn_info_clone_deinit (ReaiFnInfo* clone) {
    RETURN_VALUE_IF (!clone, (ReaiFnInfo*)Null, ERR_INVALID_ARGUMENTS);

    if (clone->name) {
        memset ((Char*)clone->name, 0, strlen (clone->name));
        FREE (clone->name);
    }

    memset (clone, 0, sizeof (*clone));
    return clone;
}