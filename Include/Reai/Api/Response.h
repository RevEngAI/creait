/**
 * @file Response.h
 * @date 9th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_API_RESPONSE_H
#define REAI_API_RESPONSE_H

#include <Reai/Api/Request.h>
#include <Reai/Util/AnalysisInfo.h>
#include <Reai/Util/FnInfo.h>

C_SOURCE_BEGIN

/* fwd declarations */
typedef struct ReaiResponseBuf ReaiResponseBuf;

typedef enum ReaiResponseType {
    REAI_RESPONSE_TYPE_UNKNOWN_ERR         = 0,
    REAI_RESPONSE_TYPE_VALIDATION_ERR      = -1,
    REAI_RESPONSE_TYPE_UPLOAD_FILE         = REAI_REQUEST_TYPE_UPLOAD_FILE,
    REAI_RESPONSE_TYPE_AUTH_CHECK          = REAI_REQUEST_TYPE_AUTH_CHECK,
    REAI_RESPONSE_TYPE_HEALTH_CHECK        = REAI_REQUEST_TYPE_HEALTH_CHECK,
    REAI_RESPONSE_TYPE_CREATE_ANALYSIS     = REAI_REQUEST_TYPE_CREATE_ANALYSIS,
    REAI_RESPONSE_TYPE_DELETE_ANALYSIS     = REAI_REQUEST_TYPE_DELETE_ANALYSIS,
    REAI_RESPONSE_TYPE_BASIC_FUNCTION_INFO = REAI_REQUEST_TYPE_BASIC_FUNCTION_INFO,
    REAI_RESPONSE_TYPE_RECENT_ANALYSIS     = REAI_REQUEST_TYPE_RECENT_ANALYSIS,
    REAI_RESPONSE_TYPE_ANALYSIS_STATUS     = REAI_REQUEST_TYPE_ANALYSIS_STATUS,
    REAI_RESPONSE_TYPE_MAX /* enum value less than this is valid */
} ReaiResponseType;


/**
 * @b Structure returned and taken by reai_request calls that get
 * a response from an API endpoint.
 *
 * User must initialize a `ReaiResponse` structure and must pass it
 * when making a call to `reai_request`. When a response is returned,
 * the `reai_request` method will parse the response data and store
 * all the returned details into this structure with proper response type
 * information in `type` member.
 * */
typedef struct ReaiResponse {
    ReaiResponseType type; /**< @b Received response type. */

    /**
     * This is where write callback writes raw response data.
     * Usually this is JSON data when interacting with Reai API,
     * but this is treated as raw data.
     *
     * Internally code uses `reai_response_init_for_type(response, type)`
     * to parse this raw data as JSON data based on given type.
     *
     * This is initialized when `reai_response_init` is called with a
     * preallocated response object. The idea is to initialize a response
     * object only once and keep reusing it throughout the lifetime of application,
     * and de-initialize it only when you're done with it finally.
     * */
    struct {
        Char* data;
        Size  length;
        Size  capacity;
    } raw;

    /**
     * Like raw data this is initialized/allocated when init is called and
     * de-initialized/deallocated when deinit is called. This is again to reduce
     * the total number of allocations/reallocations throught program lifetime.
     *
     * Ideally this should've been part of the union below, but it is because
     * of the above reason that we place it here.
     * */
    struct {
        Char*   locations;          /**< @b Error locations (comma separated). */
        Size    locations_capacity; /**< @b Total capacity of locations string. */
        CString message;            /**< @b Error message. */
        CString type;               /**< @b Error type. */
    } validation_error;

    union {
        struct {
            Bool    success; /**< @b Is true when request was successful */
            CString message; /**< @b Message returned by request */
        } health_check, auth_check, delete_analysis;

        struct {
            Bool    success;      /**< @b Is true when request was successful */
            CString message;      /**< @b Message returned by request */
            CString sha_256_hash; /**< @b Contains sha256 hash value if upload was successful */
        } upload_file;

        struct {
            Bool   success;   /**< @b True when analysis created successfully */
            Uint64 binary_id; /**< @b Contains binary id if analysis creation was successful. */
        } create_analysis;

        struct {
            Bool           success;  /**< @b True on success */
            ReaiFnInfoVec* fn_infos; /**< @b Contains array of (id, name, vaddr, size) records */
        } basic_function_info;

        struct {
            Bool                 success;
            ReaiAnalysisInfoVec* analysis_infos;
        } recent_analysis;

        struct {
            Bool               success;
            ReaiAnalysisStatus status;
        } analysis_status;
    };
} ReaiResponse;

ReaiResponse* reai_response_init (ReaiResponse* response);
ReaiResponse* reai_response_deinit (ReaiResponse* response);

C_SOURCE_END

#endif // REAI_API_RESPONSE_H
