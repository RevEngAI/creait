/**
 * @file Request.h
 * @date 9th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_API_REQUEST_H
#define REAI_API_REQUEST_H

#include <Reai/AnalysisInfo.h>
#include <Reai/Common.h>
#include <Reai/FnInfo.h>
#include <Reai/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef Uint64 BinaryId;

    typedef enum ReaiFileOption {
        REAI_FILE_OPTION_DEFAULT,
        REAI_FILE_OPTION_PE,
        REAI_FILE_OPTION_ELF,
        REAI_FILE_OPTION_MACHO,
        REAI_FILE_OPTION_RAW,
        REAI_FILE_OPTION_EXE,
        REAI_FILE_OPTION_DLL,
        REAI_FILE_OPTION_MAX
    } ReaiFileOption;

    typedef enum ReaiBinaryScope {
        REAI_BINARY_SCOPE_DEFAULT,
        REAI_BINARY_SCOPE_PRIVATE,
        REAI_BINARY_SCOPE_PUBLIC,
        REAI_BINARY_SCOPE_MAX
    } ReaiBinaryScope;

    /**
     * @b Each request types represents a unique API endpoint it'll communicate
     * with.
     * */
    typedef enum ReaiRequestType {
        REAI_REQUEST_TYPE_INVALID = 0,

        /* health api */
        REAI_REQUEST_TYPE_HEALTH_CHECK,

        /* authentication api */
        REAI_REQUEST_TYPE_AUTH_CHECK,

        /* utility api */
        REAI_REQUEST_TYPE_UPLOAD_FILE,
        /* REAI_REQUEST_TYPE_GET_CONFIG, */
        REAI_REQUEST_TYPE_SEARCH,
        /* REAI_REQUEST_TYPE_GET_MODELS, */

        /* analysis api */
        REAI_REQUEST_TYPE_CREATE_ANALYSIS,
        REAI_REQUEST_TYPE_DELETE_ANALYSIS,
        REAI_REQUEST_TYPE_BASIC_FUNCTION_INFO,
        REAI_REQUEST_TYPE_RECENT_ANALYSIS,
        REAI_REQUEST_TYPE_ANALYSIS_STATUS,


        REAI_REQUEST_TYPE_MAX /**< Total number of request types */
    } ReaiRequestType;

    /**
     * @b Represents the BinNet model to use in RevEngAI created analysis.
     * This is used in create analysis request type.
     * */
    typedef enum ReaiModel {
        REAI_MODEL_UNKNOWN = 0,
        REAI_MODEL_X86_WINDOWS,
        REAI_MODEL_X86_LINUX,
        REAI_MODEL_X86_MACOS,
        REAI_MODEL_X86_ANDROID,
        REAI_MODEL_MAX
    } ReaiModel;

    /**
     * @b Structure to be prepared with valid field values before making request to API
     * endpoint.
     * */
    typedef struct ReaiRequest {
        ReaiRequestType type; /**< @b Type of request. */

        union {
            struct {
                CString file_path; /**< @b Complete file path to be uploaded */
            } upload_file;

            struct {
                ReaiModel       model;        /**< @b BinNet model to be used */
                CString         platform_opt; /**< @b Idk the possible values of this enum. */
                CString         isa_opt;      /**< @b Idk possible values of this one as well. */
                ReaiFileOption  file_opt;     /**< @b Info about file type. */
                Bool            dyn_exec;     /**< @b Whether to perform dynamic execution or not */
                CString*        tags;         /**< @b Some tags info to help searching later on. */
                Size            tags_count;   /**< @b Number of tags in the tags array. */
                ReaiBinaryScope bin_scope;    /**< @b Scope of binary : public/private. */

                /* NOTE: both function info vector and base addr must be provided to upload symbol info */
                Uint64         base_addr; /**< @b Base address where binary is loaded. */
                ReaiFnInfoVec* functions; /**< @b Vector of function information structures. */

                CString file_name;        /**< @b Name of file. */
                CString cmdline_args;     /**< @b Command like arguments if file takes any. */
                Int32   priority;         /**< @b Priority level of this analysis. */
                CString sha_256_hash;     /**< @b SHA256 hash returned when binary was uploaded. */
                CString debug_hash;       /**< @b Idk what this really is */
                Size    size_in_bytes;    /**< @b Size of file in bytes. */
            } create_analysis;

            // TODO: request waits for too long when calling endpoint GET `/analyse/functions/{binary_id}`

            struct {
                BinaryId binary_id;
            } delete_analysis, basic_function_info, analysis_status;

            struct {
                ReaiAnalysisStatus status;
                ReaiBinaryScope    scope;
                Size               count;
            } recent_analysis;

            struct {
                CString sha_256_hash;
                CString binary_name;
                CString collection_name;
                CString state;
            } search;
        };
    } ReaiRequest;

#ifdef __cplusplus
}
#endif

#endif // REAI_API_REQUEST_H
