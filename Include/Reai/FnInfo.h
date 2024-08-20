/**
 * @file FnInfo.h
 * @date 17th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_FN_INFO_H
#define REAI_FN_INFO_H

#include <Reai/Common.h>
#include <Reai/Types.h>

/* libc */
#include <memory.h>

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @b Represents a single function symbol information entry in array of symbol
     * info. This same struct is used in both `ReaiRequest` and `ReaiResponse` structures
     * to retrieve and send information about functions from and to RevEngAI servers.
     * */
    typedef struct ReaiFnInfo {
        /**
         * @b Binary function ID, unique to corresponding function.
         *
         * NOTE : This field is ignored when passed into `ReaiRequest`
         * for making request to create analysis `/analyse` endpoint,
         * but contains a valid and a very important value for
         * in the `ReaiResponse` structure returned by request to
         * get basic function info `/analyse/functions/binary_id` endpoint.
         * */
        ReaiFunctionId id;

        /**
         * @b Must specify name of function.
         *
         * NOTE : This is valid and must be specified in both `ReaiRequest` and
         * `ReaiResponse` structure.
         * */
        CString name;

        /**
         * @b Starting virtual address of corresponding function.
         *
         * NOTE : This is valid and must be specified in both `ReaiRequest` and
         * `ReaiResponse` structure.
         * */
        Uint64 vaddr;

        /**
         * @b Size of function.
         *
         * NOTE: In `ReaiRequest`, this is used to compute function boundaries.
         * In `ReaiResponse`, this is the value returned in response to `/analyse/functions`
         * endpoint.
         * */
        Uint64 size;
    } ReaiFnInfo;

    ReaiFnInfo* reai_fn_info_clone_init (ReaiFnInfo* dst, ReaiFnInfo* src);
    ReaiFnInfo* reai_fn_info_clone_deinit (ReaiFnInfo* clone);

#include <Reai/Util/Vec.h>

    REAI_MAKE_VEC (
        ReaiFnInfoVec,
        fn_info,
        ReaiFnInfo,
        reai_fn_info_clone_init,
        reai_fn_info_clone_deinit
    );

#ifdef __cplusplus
}
#endif

#endif // REAI_FN_INFO_H
