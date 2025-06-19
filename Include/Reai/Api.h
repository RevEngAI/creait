/**
 * @file Api.h
 * @date 18th May 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 *
 * @b Single header to include it all!
 * */

#ifndef REAI_API_H
#define REAI_API_H

#include <Reai/Api/Types.h>
#include <Reai/Types.h>
#include <Reai/Util/Str.h>

typedef enum FileOption {
    FILE_OPTION_AUTO = 0,
    FILE_OPTION_PE,
    FILE_OPTION_ELF,
    FILE_OPTION_MACHO,
    FILE_OPTION_RAW,
    FILE_OPTION_EXE,
    FILE_OPTION_DLL,
    FILE_OPTION_MAX
} FileOption;

typedef enum Workspace {
    WORKSPACE_PERSONAL = 0,
    WORKSPACE_TEAM,
    WORKSPACE_PUBLIC,
    WORKSPACE_MAX
} Workspace;

typedef enum OrderBy {
    ORDER_BY_CREATED = 0,
    ORDER_BY_NAME,
    ORDER_BY_MODEL,
    ORDER_BY_OWNER,
    ORDER_BY_SIZE,
    ORDER_BY_LAST_UPDATED,
    ORDER_BY_MAX
} OrderBy;

typedef struct Connection {
    Str user_agent;
    Str host;
    Str api_key;
} Connection;

#define ConnectionInit() {.host = StrInit(), .api_key = StrInit()}

typedef struct NewAnalysisRequest {
    Str           ai_model;          /**< @b BinNet model to be used */
    Str           platform_opt;      /**< @b Idk the possible values of this enum. */
    Str           isa_opt;           /**< @b Idk possible values of this one as well. */
    FileOption    file_opt;          /**< @b Info about file type. */
    Tags          tags;              /**< @b Some tags info to help searching later on. */
    bool          is_private;        /**< @b Scope of binary : public/private. */
    u64           base_addr;         /**< @b Base address where binary is loaded. */
    FunctionInfos functions;         /**< @b Vector of function information structures. */
    Str           file_name;         /**< @b Name of file. */
    Str           cmdline_args;      /**< @b Command like arguments if file takes any. */
    i32           priority;          /**< @b Priority level of this analysis. */
    Str           sha256;            /**< @b SHA256 hash returned when binary was uploaded. */
    Str           debug_hash;        /**< @b Idk what this really is */
    size          file_size;         /**< @b size of file in bytes. */
    bool          dynamic_execution; /**< @b Whether to perform dynamic execution or not */
    bool          skip_scraping;
    bool          skip_cves;
    bool          skip_sbom;
    bool          skip_capabilities;
    bool          ignore_cache;
    bool          advanced_analysis;
} NewAnalysisRequest;

typedef struct RecentAnalysisRequest {
    Str       search_term;
    Workspace workspace;
    Status    analysis_status;
    Str       model_name;
    Status    dyn_exec_status;
    Strs      usernames;
    u32       limit;
    u32       offset;
    OrderBy   order_by;
    bool      order_in_asc;
} RecentAnalysisRequest;

typedef struct BatchAnnSymbolRequest {
    AnalysisId analysis_id;
    size       limit;
    f64        distance;
    bool       debug_symbols_only;
    struct {
        AnalysisIds   analysis_ids;
        CollectionIds collection_ids;
        BinaryIds     binary_ids;
        FunctionIds   function_ids;
    } search;
} BatchAnnSymbolRequest;

typedef struct SearchBinaryRequest {
    size page;
    size page_size;
    Str  partial_name;
    Str  partial_sha256;
    Tags tags;
    Str  model_name;
} SearchBinaryRequest;

typedef struct SearchCollectionRequest {
    size    page;
    size    page_size;
    Str     partial_collection_name;
    Str     partial_binary_name;
    Str     partial_binary_sha256;
    Tags    tags;
    Str     model_name;
    bool    filter_official : 1;
    bool    filter_user     : 1;
    bool    filter_team     : 1;
    bool    filter_public   : 1;
    bool    hide_empty      : 1;
    OrderBy order_by;
    bool    order_in_asc;
} SearchCollectionRequest;

typedef struct SimilarFunctionsRequest {
    FunctionId    function_id;
    u32           limit;
    f32           distance;
    CollectionIds collection_ids;
    struct {
        bool user_symbols;
        bool system_symbols;
        bool external_symbols;
    } debug_include;
    BinaryIds binary_ids;
} SimilarFunctionsRequest;

#ifdef __cplusplus
extern "C" {
#endif

    ///
    /// Authenticates a connection using the provided API key and host.
    ///
    /// This function constructs the URL for the authentication endpoint (`/v1/authenticate`)
    /// using the provided host and sends a request via the `MakeRequest` function.
    /// If the authentication request is successful, the function returns `true`. If the request fails
    /// due to missing or invalid API key, host, or other errors, it returns `false` and logs an error message.
    ///
    /// conn[in] : Connection information
    ///
    /// SUCCESS : true
    /// FAILURE : false, error messages are logged.
    ///
    REAI_API bool Authenticate (Connection* conn);

    ///
    /// Sends a new analysis request to the server and parses the response.
    ///
    /// This function constructs a JSON request from the given `NewAnalysisRequest` object,
    /// performs a POST request to the analysis endpoint, and parses the resulting JSON response
    /// into a `NewAnalysisResponse` structure.
    ///
    /// conn[in]    : A valid connection object with host and API key set.
    /// request[in] : The request object to serialize and send to the server.
    ///
    /// SUCCESS : A non-zero binary ID corresponding to newly created analysis.
    /// FAILURE : 0, error messages logged.
    ///
    REAI_API BinaryId CreateNewAnalysis (Connection* conn, NewAnalysisRequest* request);

    ///
    /// Retrieves basic function information using the provided binary ID.
    ///
    /// This function constructs a GET request to the analysis server, using the given binary ID, to fetch
    /// a response containing basic function information. The response is parsed into a `BasicFunctionInfoResponse`
    /// structure, which includes the success flag and a list of functions associated with the binary ID.
    ///
    /// conn[in]      : A valid connection object with host and API key set for making the request.
    /// binary_id[in] : The binary ID used to identify the binary for which function information is to be fetched.
    ///
    /// SUCCESS : FunctionInfos vector filled with function symbol information retrieved from analysis.
    /// FAILURE : Empty FunctionInfos vector.
    ///
    REAI_API FunctionInfos GetBasicFunctionInfoUsingBinaryId (Connection* conn, BinaryId binary_id);

    ///
    /// Sends a request to retrieve recent analysis data based on the provided parameters.
    ///
    /// This function serializes the given `RecentAnalysisRequest` struct into a JSON string, sends it to the server
    /// through a GET request, and parses the returned response into a `RecentAnalysisResponse` struct.
    ///
    /// conn[in]    : A valid connection object with host and API key set.
    /// request[in] : The request object that contains the filters and parameters for retrieving recent analysis data.
    ///
    /// SUCCESS :  A populated `AnalysisInfos` struct on success.
    /// FAILURE :  An empty struct (all fields set to 0) on failure.
    ///
    REAI_API AnalysisInfos GetRecentAnalysis (Connection* conn, RecentAnalysisRequest* request);

    ///
    /// Search for binaries with given filters
    ///
    /// conn[in]    : Connection information.
    /// request[in] : Request info.
    ///
    /// SUCCESS : BinaryInfos struct filled with valid data on success.
    /// FAILURE : Empty struct otherwise.
    ///
    REAI_API BinaryInfos SearchBinary (Connection* conn, SearchBinaryRequest* request);

    ///
    /// Search for collection with given filters
    ///
    /// conn[in]    : Connection info.
    /// request[in] : Request info.
    ///
    /// SUCCESS : CollectionInfos struct filled with valid data on success.
    /// FAILURE : Empty struct otherwise.
    ///
    REAI_API CollectionInfos SearchCollection (Connection* conn, SearchCollectionRequest* request);

    ///
    /// Perform a batch function renaming operation for RevEngAI.
    ///
    /// conn[in]      : Connection
    /// functions[in] : Functions to be renamed. Only `id` and `symbol.name` field will be used.
    ///
    /// SUCCESS : true
    /// FAILURE : false
    ///
    REAI_API bool BatchRenameFunctions (Connection* conn, FunctionInfos functions);

    ///
    /// Rename a single function in RevEngAI.
    ///
    /// conn[in]     : Connection
    /// fn_id[in]    : Functions to be renamed. Only `id` and `symbol.name` field will be used.
    /// new_name[in] : Name of function after rename
    ///
    /// SUCCESS : true
    /// FAILURE : false
    ///
    REAI_API bool RenameFunction (Connection* conn, FunctionId fn_id, Str new_name);

    ///
    /// Perform a batch ann symbol request using analysis id.
    /// Get mapping of functions in analysis to corresponding similar functions.
    ///
    /// conn[in]    : Connection
    /// request[in] : Request data.
    ///
    /// SUCCESS: A populated `AnnSymbols` vector struct on success.
    /// FAILURE: An empty struct (all fields set to 0) on failure.
    ///
    REAI_API AnnSymbols GetBatchAnnSymbols (Connection* conn, BatchAnnSymbolRequest* request);

    /// Retrieves the status of an analysis job for a given binary ID.
    ///
    /// This function queries the analysis server to determine the current status
    /// of an analysis job associated with the specified binary ID.
    ///
    /// conn[in]       : Valid connection object with host and API key configured
    /// binary_id[in]  : Unique identifier for the binary being analyzed
    ///
    /// RETURNS:
    ///   - Status code indicating current analysis state (STATUS_SUCCESS, STATUS_PENDING, etc.)
    ///   - STATUS_INVALID if connection parameters are invalid or request fails
    ///
    REAI_API Status GetAnalysisStatus (Connection* conn, BinaryId binary_id);

    /// Retrieves information about available AI models.
    ///
    /// This function queries the server for a list of supported AI models that can be used
    /// for analysis tasks.
    ///
    /// conn[in] : Valid connection object with host and API key configured
    ///
    /// RETURNS:
    ///   - Vector of ModelInfo structures containing model IDs and names
    ///   - Empty vector if connection fails or no models available
    ///
    REAI_API ModelInfos GetAiModelInfos (Connection* conn);

    /// Initiates AI-powered decompilation for a specific function.
    ///
    /// This function sends a request to start the AI decompilation process
    /// for the specified function ID.
    ///
    /// conn[in]        : Valid connection object
    /// function_id[in] : ID of the function to decompile
    ///
    /// RETURNS:
    ///   - true if decompilation job started successfully
    ///   - false if invalid parameters or connection failure
    ///
    REAI_API bool BeginAiDecompilation (Connection* conn, FunctionId function_id);

    /// Checks the status of an AI decompilation job.
    ///
    /// This function polls the server to determine the current status
    /// of an ongoing AI decompilation job for a specific function.
    ///
    /// conn[in]        : Valid connection object
    /// function_id[in] : ID of the function being decompiled
    ///
    /// RETURNS:
    ///   - Status code (STATUS_PENDING, STATUS_SUCCESS, etc.)
    ///   - STATUS_INVALID if invalid parameters or connection failure
    ///
    REAI_API Status GetAiDecompilationStatus (Connection* conn, FunctionId function_id);

    /// Retrieves results of a completed AI decompilation job.
    ///
    /// This function fetches the decompilation results including generated code
    /// and symbol mappings after successful completion of an AI decompilation job.
    ///
    /// conn[in]          : Valid connection object
    /// function_id[in]   : ID of the decompiled function
    /// get_ai_summary[in]: Whether to include AI-generated summary
    ///
    /// RETURNS:
    ///   - AiDecompilation structure containing decompilation results
    ///   - Empty structure if job failed or results unavailable
    ///
    REAI_API AiDecompilation
        GetAiDecompilation (Connection* conn, FunctionId function_id, bool get_ai_summary);

    ///
    /// Retrieves the Control Flow Graph (CFG) with disassembly for a given function.
    ///
    /// This function fetches the control flow graph of a function including its basic blocks,
    /// assembly instructions, local variables, and flow control information.
    ///
    /// conn[in]       : A valid connection object with host and API key set.
    /// function_id[in]: The function ID for which to retrieve the CFG.
    ///
    /// SUCCESS : ControlFlowGraph structure populated with CFG data
    /// FAILURE : Empty ControlFlowGraph structure
    ///
    REAI_API ControlFlowGraph
        GetFunctionControlFlowGraph (Connection* conn, FunctionId function_id);

    /// Finds similar functions based on vector space analysis.
    ///
    /// This function queries the server to find functions with similar characteristics
    /// to the specified target function using machine learning embeddings.
    ///
    /// conn[in]    : Valid connection object
    /// request[in] : Parameters controlling search criteria and filters
    ///
    /// RETURNS:
    ///   - Vector of SimilarFunction structures containing match details
    ///   - Empty vector if no matches found or connection failure
    ///
    REAI_API SimilarFunctions
        GetSimilarFunctions (Connection* conn, SimilarFunctionsRequest* request);

    /// Maps a binary ID to its corresponding analysis ID.
    ///
    /// This function looks up the analysis job ID associated with
    /// a previously submitted binary file.
    ///
    /// conn[in]      : Valid connection object
    /// binary_id[in] : ID of the binary file to look up
    ///
    /// SUCCESS : Non-zero analysis ID if found
    /// FAILURE : 0 if invalid input or no matching analysis found.
    ///
    REAI_API AnalysisId AnalysisIdFromBinaryId (Connection* conn, BinaryId binary_id);

    /// Retrieves log data for a specific analysis job.
    ///
    /// This function fetches diagnostic logs generated during
    /// the processing of an analysis job.
    ///
    /// conn[in]        : Valid connection object
    /// analysis_id[in] : ID of the analysis job to query
    ///
    /// SUCCESS : String containing log text
    /// FAILURE : Empty string.
    ///
    REAI_API Str GetAnalysisLogs (Connection* conn, AnalysisId analysis_id);

    ///
    /// Upload a file for analysis.
    ///
    /// This function uploads a binary file to the analysis server using a multipart POST request.
    /// Along with the file, metadata and options provided in the `request` are serialized to JSON
    /// and included in the same request.
    ///
    /// conn[in]      : Connection information including host and API key.
    /// file_path[in] : File path of to-be-uploaded file
    ///
    /// SUCCESS : Str object containing SHA-256 Hash of uploaded file.
    /// FAILURE : Empty Str object.
    ///
    REAI_API Str UploadFile (Connection* conn, Str file_path);

    ///
    /// Add a URL query parameter to given URL string.
    ///
    /// str[in,out]  : Url to append query to.
    /// key[in]      : Name of query parameter.
    /// value[in]    : Value of query parameter. If NULL, then nothing is added and
    ///                return without any error message.
    /// is_first[in] : Optional pointer to boolean variable to keep track
    ///                of whether this is first query param. If not provided
    ///                it'll be treated as first query param.
    ///
    /// SUCCESS : Updated `url` string on success.
    /// FAILURE : `NULL` otherwise.
    ///
    REAI_API Str* UrlAddQueryStr (Str* url, const char* key, const char* value, bool* is_first);

    ///
    /// Add a URL query parameter to given URL string.
    ///
    /// str[in,out]  : Url to append query to.
    /// key[in]      : Name of query parameter.
    /// value[in]    : Value of query parameter.
    /// is_first[in] : Optional pointer to boolean variable to keep track
    ///                of whether this is first query param. If not provided
    ///                it'll be treated as first query param.
    ///
    /// SUCCESS : Updated `url` string on success.
    /// FAILURE : `NULL` otherwise.
    ///
    REAI_API Str* UrlAddQueryInt (Str* url, const char* key, i64 value, bool* is_first);

    ///
    /// Add a URL query parameter to given URL string.
    ///
    /// str[in,out]  : Url to append query to.
    /// key[in]      : Name of query parameter.
    /// value[in]    : Value of query parameter.
    /// is_first[in] : Optional pointer to boolean variable to keep track
    ///                of whether this is first query param. If not provided
    ///                it'll be treated as first query param.
    ///
    /// SUCCESS : Updated `url` string on success.
    /// FAILURE : `NULL` otherwise.
    ///
    REAI_API Str* UrlAddQueryFloat (Str* url, const char* key, f64 value, bool* is_first);

    ///
    /// Add a URL query parameter to given URL string.
    ///
    /// key[in]   : Name of query parameter
    /// value[in] : Value of query parameter.
    ///
    /// RETURNS:
    ///     - Updated `url` string on success.
    ///     - `NULL` otherwise.
    ///
    REAI_API Str* UrlAddQueryBool (Str* url, const char* key, bool value, bool* is_first);

    ///
    /// Make an HTTP request to the specified URL with the given method and JSON body.
    ///
    /// This function sends an HTTP request using the specified method (e.g., POST, GET), the provided URL,
    /// and an optional JSON body. The response is then stored in the response_json string object.
    ///
    /// request_url[in]    : The URL to which the request should be sent.
    /// request_json[in]   : JSON string to be sent as the body of the request. Can be NULL if no body is needed.
    /// response_json[out] : String object to store the response data in JSON format. Can be NULL if response is not required.
    /// request_method[in] : The HTTP method to use for the request (e.g., "POST", "GET").
    ///
    /// RETURNS:
    /// On success - true
    /// On failure - false, with log messages printed to log file or stderr.
    ///
    REAI_API bool MakeRequest (
        Str*        user_agent,
        Str*        api_key,
        Str*        request_url,
        Str*        request_json,
        Str*        response_json,
        const char* request_method
    );

    ///
    /// Make an HTTP request to the specified URL with the given method and JSON body (if provided)
    /// and a file mimepart data to upload a file at given path.
    ///
    /// This function sends an HTTP request using the specified method (e.g., POST, GET), the provided URL,
    /// and an optional JSON body. The response is then stored in the response_json string object.
    ///
    /// request_url[in]    : The URL to which the request should be sent.
    /// request_json[in]   : JSON string to be sent as the body of the request. Can be NULL if no body is needed.
    /// response_json[out] : String object to store the response data in JSON format. Can be NULL if response is not required.
    /// request_method[in] : The HTTP method to use for the request (e.g., "POST", "GET").
    /// file_path[in]      : File to be uploaded.
    ///
    /// RETURNS:
    /// On success - true
    /// On failure - false, with log messages printed to log file or stderr.
    ///
    REAI_API bool MakeUploadRequest (
        Str*        user_agent,
        Str*        api_key,
        Str*        request_url,
        Str*        request_json,
        Str*        response_json,
        const char* request_method,
        Str*        file_path
    );

#ifdef __cplusplus
}
#endif

#define NewAnalysisRequestInit()                                                                   \
    {                                                                                              \
        .ai_model          = StrInit(),                                                            \
        .platform_opt      = StrInit(),                                                            \
        .isa_opt           = StrInit(),                                                            \
        .file_opt          = FILE_OPTION_AUTO,                                                     \
        .tags              = VecInitWithDeepCopy (NULL, StrDeinit),                                \
        .is_private        = false,                                                                \
        .base_addr         = 0,                                                                    \
        .functions         = VecInitWithDeepCopy (NULL, FunctionInfoDeinit),                       \
        .file_name         = StrInit(),                                                            \
        .cmdline_args      = StrInit(),                                                            \
        .priority          = 0,                                                                    \
        .sha256            = StrInit(),                                                            \
        .debug_hash        = StrInit(),                                                            \
        .file_size         = 0,                                                                    \
        .dynamic_execution = false,                                                                \
        .skip_scraping     = true,                                                                 \
        .skip_cves         = true,                                                                 \
        .skip_sbom         = true,                                                                 \
        .skip_capabilities = true,                                                                 \
        .ignore_cache      = false,                                                                \
        .advanced_analysis = false,                                                                \
    }

#define NewAnalysisRequestDeinit(r)                                                                \
    do {                                                                                           \
        StrDeinit (&(r)->ai_model);                                                                \
        StrDeinit (&(r)->platform_opt);                                                            \
        StrDeinit (&(r)->isa_opt);                                                                 \
        VecDeinit (&(r)->tags);                                                                    \
        VecDeinit (&(r)->functions);                                                               \
        StrDeinit (&(r)->file_name);                                                               \
        StrDeinit (&(r)->cmdline_args);                                                            \
        StrDeinit (&(r)->sha256);                                                                  \
        StrDeinit (&(r)->debug_hash);                                                              \
        memset (r, 0, sizeof (NewAnalysisRequest));                                                \
    } while (0)

#define RecentAnalysisRequestInit()                                                                \
    {                                                                                              \
        .search_term     = StrInit(),                                                              \
        .workspace       = WORKSPACE_PERSONAL,                                                     \
        .analysis_status = STATUS_COMPLETE,                                                        \
        .model_name      = StrInit(),                                                              \
        .dyn_exec_status = STATUS_COMPLETE,                                                        \
        .usernames       = VecInitWithDeepCopy (NULL, StrDeinit),                                  \
        .limit           = 50,                                                                     \
        .offset          = 0,                                                                      \
        .order_by        = ORDER_BY_CREATED,                                                       \
        .order_in_asc    = false,                                                                  \
    }

#define RecentAnalysisRequestDeinit(r)                                                             \
    do {                                                                                           \
        StrDeinit (&(r)->search_term);                                                             \
        StrDeinit (&(r)->model_name);                                                              \
        VecDeinit (&(r)->usernames);                                                               \
        memset (r, 0, sizeof (RecentAnalysisRequest));                                             \
    } while (0)

#define BatchAnnSymbolRequestInit()                                                                \
    {                                                                                              \
        .analysis_id = 0, .limit = 50, .distance = 0.1, .debug_symbols_only = true, .search = {    \
            .analysis_ids   = VecInit(),                                                           \
            .collection_ids = VecInit(),                                                           \
            .binary_ids     = VecInit(),                                                           \
            .function_ids   = VecInit(),                                                           \
        }                                                                                          \
    }

#define BatchAnnSymbolRequestDeinit(r)                                                             \
    do {                                                                                           \
        VecDeinit (&(r)->search.analysis_ids);                                                     \
        VecDeinit (&(r)->search.collection_ids);                                                   \
        VecDeinit (&(r)->search.binary_ids);                                                       \
        VecDeinit (&(r)->search.function_ids);                                                     \
        memset (r, 0, sizeof (BatchAnnSymbolRequest));                                             \
    } while (0)

#define SearchBinaryRequestInit()                                                                  \
    {.page           = 0,                                                                          \
     .page_size      = 50,                                                                         \
     .partial_name   = StrInit(),                                                                  \
     .partial_sha256 = StrInit(),                                                                  \
     .tags           = VecInitWithDeepCopy (NULL, StrDeinit),                                      \
     .model_name     = StrInit()}

#define SearchBinaryRequestDeinit(r)                                                               \
    do {                                                                                           \
        StrDeinit (&(r)->partial_name);                                                            \
        StrDeinit (&(r)->partial_sha256);                                                          \
        VecDeinit (&(r)->tags);                                                                    \
        StrDeinit (&(r)->model_name);                                                              \
        memset (r, 0, sizeof (SearchBinaryRequest));                                               \
    } while (0)

#define SearchCollectionRequestInit()                                                              \
    {                                                                                              \
        .page                    = 0,                                                              \
        .page_size               = 50,                                                             \
        .partial_collection_name = StrInit(),                                                      \
        .partial_binary_name     = StrInit(),                                                      \
        .partial_binary_sha256   = StrInit(),                                                      \
        .tags                    = VecInitWithDeepCopy (NULL, StrDeinit),                          \
        .model_name              = StrInit(),                                                      \
        .filter_official         = false,                                                          \
        .filter_user             = false,                                                          \
        .filter_team             = false,                                                          \
        .filter_public           = false,                                                          \
        .hide_empty              = true,                                                           \
        .order_by                = ORDER_BY_SIZE,                                                  \
        .order_in_asc            = false,                                                          \
    }

#define SearchCollectionRequestDeinit(r)                                                           \
    do {                                                                                           \
        StrDeinit (&(r)->partial_collection_name);                                                 \
        StrDeinit (&(r)->partial_binary_name);                                                     \
        StrDeinit (&(r)->partial_binary_sha256);                                                   \
        VecDeinit (&(r)->tags);                                                                    \
        StrDeinit (&(r)->model_name);                                                              \
        memset (r, 0, sizeof (SearchCollectionRequest));                                           \
    } while (0)

#define SimilarFunctionsRequestInit()                                                               \
    {                                                                                               \
        .function_id    = 0,                                                                        \
        .limit          = 50,                                                                       \
        .distance       = 0.1,                                                                      \
        .collection_ids = VecInit(),                                                                \
        .debug_include  = {.user_symbols = true, .system_symbols = true, .external_symbols = true}, \
        .binary_ids     = VecInit()                                                                 \
}

#define SimilarFunctionsRequestDeinit(r)                                                           \
    do {                                                                                           \
        VecDeinit (&(r)->collection_ids);                                                          \
        VecDeinit (&(r)->binary_ids);                                                              \
    } while (0)

#endif // REAI_API_H
