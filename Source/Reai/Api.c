/**
 * @file Api.c
 * @date 18th May 2025 
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#include <Reai/Api.h>
#include <Reai/Log.h>
#include <Reai/Util/Json.h>

bool Authenticate (Connection conn) {
    if (!conn.api_key.length || !conn.host.length) {
        LOG_ERROR ("Missing API key or host to connect to.");
        return false;
    }

    Str url = StrInit();
    StrPrintf (&url, "%s/v1/authenticate", conn.host.data);

    bool res = MakeRequest (&conn.api_key, &url, NULL, NULL, "GET");

    StrDeinit (&url);

    return res;
}

BinaryId CreateNewAnalysis (Connection conn, NewAnalysisRequest* request) {
    if (!conn.api_key.length || !conn.host.length) {
        LOG_ERROR ("Missing API key or host to connect to.");
        return 0;
    }

    if (!request) {
        LOG_ERROR ("Invalid request");
        return 0;
    }

    if (request->file_opt >= FILE_OPTION_MAX) {
        LOG_ERROR ("Invalid file option in create new analysis request. Using default value");
        request->file_opt = FILE_OPTION_AUTO;
    }

    Str url = StrInit();
    Str sj  = StrInit(); // send json

    StrPrintf (&url, "%s/v1/analyse/", conn.host.data);

    static const char* file_opt_to_str[] = {
        [FILE_OPTION_AUTO]  = "Auto",
        [FILE_OPTION_PE]    = "PE",
        [FILE_OPTION_ELF]   = "ELF",
        [FILE_OPTION_MACHO] = "MACHO",
        [FILE_OPTION_RAW]   = "RAW",
        [FILE_OPTION_EXE]   = "EXE",
        [FILE_OPTION_DLL]   = "DLL",
    };

    JW_OBJ (sj, {
        JW_STR_KV (sj, "model_name", request->ai_model);
        JW_STR_KV (sj, "platform_options", request->platform_opt);
        JW_STR_KV (sj, "isa_options", request->isa_opt);
        JW_ZSTR_KV (sj, "file_options", file_opt_to_str[request->file_opt]);
        JW_BOOL_KV (sj, "dynamic_execution", request->dynamic_execution);
        JW_ARR_KV (sj, "tags", request->tags, tag, { JW_STR (sj, tag); });
        JW_ZSTR_KV (sj, "binary_scope", request->is_private ? "PRIVATE" : "PUBLIC");
        JW_ARR_KV (sj, "symbols", request->functions, function, {
            if (!function.symbol.is_addr) {
                LOG_ERROR (
                    "Function \"%s\" symbol expected to be an address value.",
                    function.symbol.name
                );
                continue;
            }
            JW_OBJ (sj, {
                JW_STR_KV (sj, "name", function.symbol.name);
                JW_INT_KV (sj, "start_addr", function.symbol.value.addr);
                JW_INT_KV (sj, "end_ddr", function.symbol.value.addr + function.size);
            });
        });
        JW_STR_KV (sj, "file_name", request->file_name);
        JW_STR_KV (sj, "command_line_args", request->cmdline_args);
        JW_INT_KV (sj, "priority", request->priority);
        JW_STR_KV (sj, "sha_256_hash", request->sha256);
        JW_STR_KV (sj, "debug_hash", request->debug_hash);
        JW_INT_KV (sj, "size_in_bytes", request->file_size);
        JW_BOOL_KV (sj, "skip_scraping", request->skip_scraping);
        JW_BOOL_KV (sj, "skip_cves", request->skip_cves);
        JW_BOOL_KV (sj, "skip_sbom", request->skip_sbom);
        JW_BOOL_KV (sj, "skip_capabilities", request->skip_capabilities);
        JW_BOOL_KV (sj, "ignore_cache", request->ignore_cache);
        JW_BOOL_KV (sj, "advanced_analysis", request->advanced_analysis);
    });

    Str gj = StrInit(); // get json

    if (MakeRequest (&conn.api_key, &url, &sj, &gj, "POST")) {
        StrDeinit (&url);
        StrDeinit (&sj);

        StrIter j = StrIterInitFromStr (&gj);

        bool     success   = false;
        BinaryId binary_id = 0;
        JR_OBJ (j, {
            JR_BOOL_KV (j, "success", success);
            if (success) {
                JR_INT_KV (j, "binary_id", binary_id);
            }
        });

        StrDeinit (&gj);

        return binary_id;
    } else {
        return 0;
    }
}

FunctionInfos GetBasicFunctionInfoUsingBinaryId (Connection conn, BinaryId binary_id) {
    if (!conn.api_key.length || !conn.host.length) {
        LOG_ERROR ("Missing API key or host to connect to.");
        return (FunctionInfos) {0};
    }

    Str url = StrInit();
    Str gj  = StrInit();

    StrPrintf (&url, "%s/v1/analyse/functions/%llu", conn.host.data, binary_id);
    if (MakeRequest (&conn.api_key, &url, NULL, &gj, "GET")) {
        StrDeinit (&url);

        StrIter j = StrIterInitFromStr (&gj);

        bool          success   = false;
        FunctionInfos functions = VecInitWithDeepCopy (NULL, FunctionInfoDeinit);
        JR_OBJ (j, {
            JR_BOOL_KV (j, "success", success);
            if (success) {
                JR_ARR_KV (j, "functions", {
                    FunctionInfo function   = {0};
                    function.symbol.is_addr = true;
                    JR_OBJ (j, {
                        JR_INT_KV (j, "function_id", function.id);
                        JR_STR_KV (j, "function_name", function.symbol.name);
                        JR_INT_KV (j, "function_size", function.size);
                        JR_INT_KV (j, "function_vaddr", function.symbol.value.addr);
                    });
                    VecPushBack (&functions, function);
                });
            }
        });

        StrDeinit (&gj);

        return functions;
    } else {
        return (FunctionInfos) {0};
    }
}

// TODO: GetBasicFunctionInfoUsingAnalysisId

AnalysisInfos GetRecentAnalysis (Connection conn, RecentAnalysisRequest* request) {
    if (!conn.api_key.length || !conn.host.length) {
        LOG_ERROR ("Missing API key or host to connect to.");
        return (AnalysisInfos) {0};
    }

    if (!request) {
        LOG_ERROR ("Invalid request");
        return (AnalysisInfos) {0};
    }

    if (request->workspace >= WORKSPACE_MAX) {
        LOG_ERROR ("Invalid 'workspace'. Defaulting to 'personal'");
        request->workspace = WORKSPACE_PERSONAL;
    }

    if (request->order_by >= ORDER_BY_MAX) {
        LOG_ERROR ("Invalid 'order_by'. Defaulting to 'created'");
        request->order_by = ORDER_BY_CREATED;
    }

    Str url = StrInit();
    Str gj  = StrInit();

    StrPrintf (&url, "%s/v2/analyses/list", conn.host.data);

    bool        is_first          = true;
    const char* ws[WORKSPACE_MAX] = {"personal", "public", "team"};

    UrlAddQueryStr (&url, "search_term", request->search_term.data, &is_first);
    UrlAddQueryStr (&url, "model_name", request->model_name.data, &is_first);
    UrlAddQueryStr (&url, "workspace", ws[request->workspace], &is_first);
    VecForeach (&request->usernames, username, {
        UrlAddQueryStr (&url, "usernames", username.data, &is_first);
    });
    UrlAddQueryInt (&url, "limit", CLAMP (request->limit, 5, 50), &is_first);
    UrlAddQueryInt (&url, "offset", request->offset, &is_first);
    UrlAddQueryStr (&url, "order", request->order_in_asc ? "ASC" : "DESC", &is_first);

    switch (request->order_by) {
        case ORDER_BY_NAME :
            UrlAddQueryStr (&url, "order_by", "name", &is_first);
            break;

        case ORDER_BY_SIZE :
            UrlAddQueryStr (&url, "order_by", "size", &is_first);
            break;

        default :
            UrlAddQueryStr (&url, "order_by", "created", &is_first);
            break;
    }

    if (MakeRequest (&conn.api_key, &url, NULL, &gj, "GET")) {
        StrDeinit (&url);

        StrIter j = StrIterInitFromStr (&gj);

        AnalysisInfos infos   = VecInitWithDeepCopy (NULL, AnalysisInfoDeinit);
        bool          success = false;
        JR_OBJ (j, {
            JR_BOOL_KV (j, "status", success);
            if (success) {
                JR_OBJ_KV (j, "data", {
                    JR_ARR_KV (j, "results", {
                        AnalysisInfo info            = {0};
                        Str          scope           = StrInit();
                        Str          analysis_status = StrInit();
                        Str          dyn_exec_status = StrInit();
                        JR_OBJ (j, {
                            JR_INT_KV (j, "analysis_id", info.analysis_id);
                            JR_STR_KV (j, "analysis_scope", scope);
                            JR_INT_KV (j, "binary_id", info.binary_id);
                            JR_INT_KV (j, "model_id", info.model_id);
                            JR_STR_KV (j, "status", analysis_status);
                            JR_STR_KV (j, "creation", info.creation);
                            JR_BOOL_KV (j, "is_owner", info.is_owner);
                            JR_STR_KV (j, "binary_name", info.binary_name);
                            JR_STR_KV (j, "sha_256_hash", info.sha256);
                            JR_INT_KV (j, "binary_size", info.binary_size);
                            JR_STR_KV (j, "username", info.username);
                            JR_STR_KV (j, "dynamic_execution_status", dyn_exec_status);
                            JR_INT_KV (j, "dynamic_execution_task_id", info.dyn_exec_task_id);
                        });
                        info.is_private      = !StrCmpZstr (&scope, "PRIVATE");
                        info.status          = StatusFromStr (&analysis_status);
                        info.dyn_exec_status = StatusFromStr (&dyn_exec_status);
                        StrDeinit (&scope);
                        StrDeinit (&analysis_status);
                        StrDeinit (&dyn_exec_status);
                        VecPushBack (&infos, info);
                    });
                });
            }
        });

        StrDeinit (&gj);

        return infos;
    } else {
        return (AnalysisInfos) {0};
    }
}

BinaryInfos SearchBinary (Connection conn, SearchBinaryRequest* request) {
    if (!conn.api_key.length || !conn.host.length) {
        LOG_ERROR ("Missing API key or host to connect to.");
        return (BinaryInfos) {0};
    }

    if (!request) {
        LOG_ERROR ("Invalid request");
        return (BinaryInfos) {0};
    }

    Str url = StrInit();
    Str gj  = StrInit();

    StrPrintf (&url, "%s/v2/search/binaries", conn.host.data);

    bool is_first = true;

    UrlAddQueryInt (&url, "page", request->page, &is_first);
    UrlAddQueryInt (&url, "page_size", request->page_size, &is_first);
    UrlAddQueryStr (&url, "partial_name", request->partial_name.data, &is_first);
    UrlAddQueryStr (&url, "partial_sha256", request->partial_sha256.data, &is_first);
    UrlAddQueryStr (&url, "model_name", request->model_name.data, &is_first);
    VecForeach (&request->tags, tag, { UrlAddQueryStr (&url, "tags", tag.data, &is_first); });

    if (MakeRequest (&conn.api_key, &url, NULL, &gj, "GET")) {
        StrDeinit (&url);

        StrIter j = StrIterInitFromStr (&gj);

        bool        status = false;
        BinaryInfos infos  = VecInitWithDeepCopy (NULL, BinaryInfoDeinit);
        JR_OBJ (j, {
            JR_BOOL_KV (j, "status", status);
            if (status) {
                JR_OBJ_KV (j, "data", {
                    JR_ARR_KV (j, "results", {
                        BinaryInfo info = {0};
                        info.tags       = VecInitWithDeepCopy_T (&info.tags, NULL, StrDeinit);
                        JR_OBJ (j, {
                            JR_INT_KV (j, "binary_id", info.binary_id);
                            JR_STR_KV (j, "binary_name", info.binary_name);
                            JR_INT_KV (j, "analysis_id", info.analysis_id);
                            JR_STR_KV (j, "sha_256_hash", info.sha256);
                            JR_ARR_KV (j, "tags", {
                                Str tag = StrInit();
                                JR_STR (j, tag);
                                VecPushBack (&info.tags, tag);
                            });
                            JR_STR_KV (j, "created_at", info.created_at);
                            JR_INT_KV (j, "model_id", info.model_id);
                            JR_STR_KV (j, "model_name", info.model_name);
                            JR_STR_KV (j, "owned_by", info.owned_by);
                        });
                        VecPushBack (&infos, info);
                    });
                });
            }
        });

        StrDeinit (&gj);

        return infos;
    } else {
        return (BinaryInfos) {0};
    }
}

CollectionInfos SearchCollection (Connection conn, SearchCollectionRequest* request) {
    if (!conn.api_key.length || !conn.host.length) {
        LOG_ERROR ("Missing API key or host to connect to.");
        return (CollectionInfos) {0};
    }

    if (!request) {
        LOG_ERROR ("Invalid request");
        return (CollectionInfos) {0};
    }

    if (request->order_by >= ORDER_BY_MAX) {
        LOG_ERROR ("Invalid order_by search query parameter. Using default value.");
        request->order_by = ORDER_BY_CREATED;
    }

    static const char* order_by_to_str[] = {
        [ORDER_BY_CREATED]      = "created",
        [ORDER_BY_NAME]         = "collection",
        [ORDER_BY_MODEL]        = "model",
        [ORDER_BY_OWNER]        = "owner",
        [ORDER_BY_SIZE]         = "collection_size",
        [ORDER_BY_LAST_UPDATED] = "updated"
    };

    Str url = StrInit();
    Str gj  = StrInit();

    StrPrintf (&url, "%s/v2/search/collections", conn.host.data);

    bool is_first = true;

#define ADD_FILTER(f, v)                                                                           \
    if (request->f)                                                                                \
    UrlAddQueryStr (&url, "filters", v, &is_first)

    UrlAddQueryInt (&url, "page", request->page, &is_first);
    UrlAddQueryInt (&url, "page_size", request->page_size, &is_first);
    UrlAddQueryStr (
        &url,
        "partial_collection_name",
        request->partial_collection_name.data,
        &is_first
    );
    UrlAddQueryStr (&url, "partial_binary_name", request->partial_binary_name.data, &is_first);
    UrlAddQueryStr (&url, "partial_binary_sha256", request->partial_binary_sha256.data, &is_first);
    VecForeach (&request->tags, tag, { UrlAddQueryStr (&url, "tags", tag.data, &is_first); });
    UrlAddQueryStr (&url, "model_name", request->model_name.data, &is_first);
    UrlAddQueryStr (&url, "order_by", order_by_to_str[request->order_by], &is_first);
    UrlAddQueryStr (&url, "order_by_direction", request->order_in_asc ? "ASC" : "DESC", &is_first);
    ADD_FILTER (filter_official, "official_only");
    ADD_FILTER (filter_user, "user_only");
    ADD_FILTER (filter_team, "team_only");
    ADD_FILTER (filter_public, "public_only");
    ADD_FILTER (hide_empty, "hide_empty");

#undef ADD_FILTER

    if (MakeRequest (&conn.api_key, &url, NULL, &gj, "GET")) {
        StrDeinit (&url);

        StrIter j = StrIterInitFromStr (&gj);

        bool            status = false;
        CollectionInfos infos  = VecInitWithDeepCopy (NULL, CollectionInfoDeinit);
        JR_OBJ (j, {
            JR_BOOL_KV (j, "status", status);
            if (status) {
                JR_OBJ_KV (j, "data", {
                    JR_ARR_KV (j, "results", {
                        CollectionInfo info = {0};
                        info.tags           = VecInitWithDeepCopy_T (&info.tags, NULL, StrDeinit);
                        Str scope           = StrInit();
                        JR_OBJ (j, {
                            JR_INT_KV (j, "collection_id", info.id);
                            JR_STR_KV (j, "collection_name", info.name);
                            JR_STR_KV (j, "scope", scope);
                            JR_STR_KV (j, "last_updated_at", info.last_updated_at);
                            JR_STR_KV (j, "created_at", info.created_at);
                            JR_INT_KV (j, "model_id", info.model_id);
                            JR_STR_KV (j, "model_name", info.model_name);
                            JR_STR_KV (j, "owned_by", info.owned_by);
                            JR_ARR_KV (j, "tags", {
                                Str tag = StrInit();
                                JR_STR (j, tag);
                                VecPushBack (&info.tags, tag);
                            });
                            JR_INT_KV (j, "size", info.size);
                            JR_STR_KV (j, "description", info.description);
                            JR_INT_KV (j, "team_id", info.team_id);
                        });
                        info.is_private = !StrCmpZstr (&scope, "PRIVATE");
                        StrDeinit (&scope);
                        VecPushBack (&infos, info);
                    });
                });
            }
        });

        StrDeinit (&gj);

        return infos;
    } else {
        return (CollectionInfos) {0};
    }
}

bool BatchRenameFunctions (Connection conn, FunctionInfos functions) {
    if (!conn.api_key.length || !conn.host.length) {
        LOG_ERROR ("Missing API key or host to connect to.");
        return false;
    }

    Str url = StrInit();
    Str sj  = StrInit();

    StrPrintf (&url, "%s/v2/functions/rename/batch", conn.host.data);

    JW_OBJ (sj, {
        JW_ARR_KV (sj, "functions", functions, function, {
            JW_OBJ (sj, {
                JW_INT_KV (sj, "function_id", function.id);
                JW_STR_KV (sj, "new_name", function.symbol.name);
            });
        });
    });

    Str gj = StrInit();

    if (MakeRequest (&conn.api_key, &url, &sj, &gj, "POST")) {
        StrDeinit (&url);
        StrDeinit (&sj);

        StrIter j = StrIterInitFromStr (&gj);

        bool status = false;
        JR_OBJ (j, { JR_BOOL_KV (j, "status", status); });

        StrDeinit (&gj);

        return status;
    } else {
        return false;
    }
}

bool RenameFunction (Connection conn, FunctionId fn_id, Str new_name) {
    if (!conn.api_key.length || !conn.host.length) {
        LOG_ERROR ("Missing API key or host to connect to.");
        return false;
    }

    if (!fn_id) {
        LOG_ERROR ("Invalid function id.");
        return false;
    }

    if (!new_name.length) {
        LOG_ERROR ("New function name cannot have 0 length.");
        return false;
    }

    Str url = StrInit();
    Str sj  = StrInit();

    StrPrintf (&url, "%s/v2/functions/rename/%llu", conn.host.data, fn_id);
    StrPrintf (&sj, "{\"new_name\":\"%s\"}", new_name.data);

    Str gj = StrInit();

    if (MakeRequest (&conn.api_key, &url, &sj, &gj, "POST")) {
        StrDeinit (&url);
        StrDeinit (&sj);

        StrIter j = StrIterInitFromStr (&gj);

        bool status = false;
        JR_OBJ (j, { JR_BOOL_KV (j, "status", status); });

        StrDeinit (&gj);

        return status;
    } else {
        return false;
    }
}

AnnSymbols GetBatchAnnSymbols (Connection conn, BatchAnnSymbolRequest* request) {
    if (!conn.api_key.length || !conn.host.length) {
        LOG_ERROR ("Missing API key or host to connect to.");
        return (AnnSymbols) {0};
    }

    if (!request) {
        LOG_ERROR ("Invalid request");
        return (AnnSymbols) {0};
    }

    if (!request->analysis_id) {
        LOG_ERROR ("Invalid analysis id.");
        return (AnnSymbols) {0};
    }

    Str url = StrInit();
    Str sj  = StrInit();

    StrPrintf (
        &url,
        "%s/v2/analyses/%llu/similarity/functions",
        conn.host.data,
        request->analysis_id
    );

    JW_OBJ (sj, {
        JW_INT_KV (sj, "limit", request->limit);
        JW_FLT_KV (sj, "distance", request->distance);
        JW_ARR_KV (sj, "analysis_search_ids", request->search.analysis_ids, id, {
            JW_INT (sj, id);
        });
        JW_ARR_KV (sj, "collection_search_ids", request->search.collection_ids, id, {
            JW_INT (sj, id);
        });
        JW_ARR_KV (sj, "search_binary_ids", request->search.binary_ids, id, { JW_INT (sj, id); });
        JW_ARR_KV (sj, "search_function_ids", request->search.function_ids, id, {
            JW_INT (sj, id);
        });
    });

    Str gj = StrInit();

    if (MakeRequest (&conn.api_key, &url, &sj, &gj, "POST")) {
        StrDeinit (&url);
        StrDeinit (&sj);

        StrIter j = StrIterInitFromStr (&gj);

        bool       status = false;
        AnnSymbols syms   = VecInitWithDeepCopy (NULL, AnnSymbolDeinit);
        JR_OBJ (j, {
            JR_BOOL_KV (j, "status", status);
            if (status) {
                JR_OBJ_KV (j, "data", {
                    FunctionId source_function_id = strtoull (key.data, NULL, 10);
                    JR_OBJ (j, {
                        AnnSymbol sym          = {0};
                        sym.source_function_id = source_function_id;
                        sym.target_function_id = strtoull (key.data, NULL, 10);

                        JR_OBJ (j, {
                            JR_FLT_KV (j, "distance", sym.distance);
                            JR_INT_KV (j, "nearest_neighbor_analysis_id", sym.analysis_id);
                            JR_INT_KV (j, "nearest_neighbor_binary_id", sym.binary_id);
                            JR_STR_KV (j, "nearest_neighbor_analysis_name", sym.analysis_name);
                            JR_STR_KV (j, "nearest_neighbor_function_name", sym.function_name);
                            JR_STR_KV (j, "nearest_neighbor_sha_256_hash", sym.sha256);
                            JR_BOOL_KV (j, "nearest_neighbor_debug", sym.debug);
                            JR_STR_KV (
                                j,
                                "nearest_neighbor_function_name_mangled",
                                sym.function_mangled_name
                            );
                        });

                        LOG_INFO (
                            "Source (%zu) -> Target (%zu) [%s]",
                            sym.source_function_id,
                            sym.target_function_id,
                            sym.function_name.data
                        );

                        VecPushBack (&syms, sym);
                    });
                });
            }
        });

        StrDeinit (&gj);

        return syms;
    } else {
        return (AnnSymbols) {0};
    }
}

Status GetAnalysisStatus (Connection conn, BinaryId binary_id) {
    if (!conn.api_key.length || !conn.host.length) {
        LOG_ERROR ("Missing API key or host to connect to.");
        return STATUS_INVALID;
    }

    Str url = StrInit();
    Str gj  = StrInit();

    StrPrintf (&url, "%s/v1/analyse/status/%llu", conn.host.data, binary_id);

    if (MakeRequest (&conn.api_key, &url, NULL, &gj, "GET")) {
        StrDeinit (&url);

        StrIter j = StrIterInitFromStr (&gj);

        bool success = false;

        Str status = StrInit();
        JR_OBJ (j, {
            JR_BOOL_KV (j, "success", success);
            if (success) {
                JR_STR_KV (j, "status", status);
            }
        });

        StrDeinit (&gj);

        Status analysis_status = StatusFromStr (&status);
        StrDeinit (&status);
        return analysis_status;
    } else {
        return STATUS_INVALID;
    }
}


ModelInfos GetAiModelInfos (Connection conn) {
    if (!conn.api_key.length || !conn.host.length) {
        LOG_ERROR ("Missing API key or host to connect to.");
        return (ModelInfos) {0};
    }

    Str url = StrInit();
    Str gj  = StrInit();

    StrPrintf (&url, "%s/v1/models", conn.host.data);

    if (MakeRequest (&conn.api_key, &url, NULL, &gj, "GET")) {
        StrDeinit (&url);

        StrIter j = StrIterInitFromStr (&gj);

        bool       success = false;
        ModelInfos models  = VecInitWithDeepCopy (NULL, ModelInfoDeinit);

        JR_OBJ (j, {
            JR_BOOL_KV (j, "success", success);
            if (success) {
                JR_ARR_KV (j, "models", {
                    ModelInfo model = {0};
                    JR_OBJ (j, {
                        JR_INT_KV (j, "model_id", model.id);
                        JR_STR_KV (j, "model_name", model.name);
                    });
                    VecPushBack (&models, model);
                });
            }
        });

        StrDeinit (&gj);

        return models;
    } else {
        return (ModelInfos) {0};
    }
}

bool BeginAiDecompilation (Connection conn, FunctionId function_id) {
    if (!conn.api_key.length || !conn.host.length) {
        LOG_ERROR ("Missing API key or host to connect to.");
        return false;
    }

    if (!function_id) {
        LOG_ERROR ("Invalid function id.");
        return false;
    }

    Str url = StrInit();

    StrPrintf (&url, "%s/v2/functions/%llu/ai-decompilation", conn.host.data, function_id);

    Str gj = StrInit();

    if (MakeRequest (&conn.api_key, &url, NULL, &gj, "POST")) {
        StrDeinit (&url);

        StrIter j = StrIterInitFromStr (&gj);

        bool status = false;
        JR_OBJ (j, { JR_BOOL_KV (j, "status", status); });

        StrDeinit (&gj);

        return status;
    } else {
        return false;
    }
}

Status GetAiDecompilationStatus (Connection conn, FunctionId function_id) {
    if (!conn.api_key.length || !conn.host.length) {
        LOG_ERROR ("Missing API key or host to connect to.");
        return STATUS_INVALID;
    }

    if (!function_id) {
        LOG_ERROR ("Invalid function id.");
        return STATUS_INVALID;
    }

    Str url = StrInit();
    Str gj  = StrInit();

    StrPrintf (&url, "%s/v2/functions/%llu/ai-decompilation/status", conn.host.data, function_id);

    if (MakeRequest (&conn.api_key, &url, NULL, &gj, "GET")) {
        StrDeinit (&url);

        StrIter j = StrIterInitFromStr (&gj);

        bool status     = false;
        Str  status_str = StrInit();
        JR_OBJ (j, {
            JR_BOOL_KV (j, "status", status);
            if (status) {
                JR_OBJ_KV (j, "data", { JR_STR_KV (j, "status", status_str); });
            }
        });

        StrDeinit (&gj);

        Status decomp_status = StatusFromStr (&status_str);
        StrDeinit (&status_str);
        return decomp_status;
    } else {
        return STATUS_INVALID;
    }
}

AiDecompilation GetAiDecompilation (Connection conn, FunctionId function_id, bool get_ai_summary) {
    if (!conn.api_key.length || !conn.host.length) {
        LOG_ERROR ("Missing API key or host to connect to.");
        return (AiDecompilation) {0};
    }

    if (!function_id) {
        LOG_ERROR ("Invalid function id.");
        return (AiDecompilation) {0};
    }

    i32 MAX_RETRIES = 20;
    while (MAX_RETRIES--) {
        Status status = GetAiDecompilationStatus (conn, function_id);
        switch (status & STATUS_MASK) {
            case STATUS_UNINITIALIZED : {
                LOG_ERROR ("Ai decompilation not started yet.");
                return (AiDecompilation) {0};
            }

            case STATUS_PENDING : {
                break;
            }

            case STATUS_ERROR : {
                LOG_ERROR ("Last AI decompilation errored out. Restart.");
                return (AiDecompilation) {0};
            }

            case STATUS_SUCCESS : {
                MAX_RETRIES = 0;
                break;
            }

            default : {
                LOG_FATAL ("Unreachable code.");
            }
        }
    }

    Str url = StrInit();

    StrPrintf (&url, "%s/v2/functions/%llu/ai-decompilation", conn.host.data, function_id);
    UrlAddQueryBool (&url, "summarise", get_ai_summary, NULL);

    Str gj = StrInit();

    if (MakeRequest (&conn.api_key, &url, NULL, &gj, "GET")) {
        StrDeinit (&url);

        StrIter j = StrIterInitFromStr (&gj);

        bool            status = false;
        AiDecompilation decomp = {0};
        decomp.decompilation   = StrInit();
        decomp.summary         = StrInit();
        decomp.functions       = VecInitWithDeepCopy_T (&decomp.functions, NULL, SymbolInfoDeinit);
        decomp.strings         = VecInitWithDeepCopy_T (&decomp.strings, NULL, SymbolInfoDeinit);
        decomp.unmatched.strings =
            VecInitWithDeepCopy_T (&decomp.unmatched.strings, NULL, SymbolInfoDeinit);
        decomp.unmatched.functions =
            VecInitWithDeepCopy_T (&decomp.unmatched.functions, NULL, SymbolInfoDeinit);
        decomp.unmatched.vars =
            VecInitWithDeepCopy_T (&decomp.unmatched.vars, NULL, SymbolInfoDeinit);
        decomp.unmatched.external_vars =
            VecInitWithDeepCopy_T (&decomp.unmatched.external_vars, NULL, SymbolInfoDeinit);
        decomp.unmatched.custom_types =
            VecInitWithDeepCopy_T (&decomp.unmatched.custom_types, NULL, SymbolInfoDeinit);
        decomp.unmatched.go_to_labels =
            VecInitWithDeepCopy_T (&decomp.unmatched.go_to_labels, NULL, SymbolInfoDeinit);
        decomp.unmatched.custom_function_pointers = VecInitWithDeepCopy_T (
            &decomp.unmatched.custom_function_pointers,
            NULL,
            SymbolInfoDeinit
        );

        decomp.unmatched.variadic_lists =
            VecInitWithDeepCopy_T (&decomp.unmatched.variadic_lists, NULL, SymbolInfoDeinit);
        JR_OBJ (j, {
            JR_BOOL_KV (j, "status", status);
            if (status) {
                JR_OBJ_KV (j, "data", {
                    JR_STR_KV (j, "raw_decompilation", decomp.decompilation);
                    JR_STR_KV (j, "summary", decomp.summary);
                    JR_OBJ_KV (j, "function_mapping_full", {
                        JR_OBJ_KV (j, "inverse_string_map", {
                            SymbolInfo sym = {0};
                            sym.is_addr    = true;
                            JR_OBJ (j, {
                                JR_STR_KV (j, "string", sym.string);
                                JR_INT_KV (j, "addr", sym.value.addr);
                            });
                            VecPushBack (&decomp.strings, sym);
                        });

                        JR_OBJ_KV (j, "inverse_function_map", {
                            SymbolInfo sym = {0};
                            sym.is_addr    = true;
                            JR_OBJ (j, {
                                JR_STR_KV (j, "name", sym.name);
                                JR_INT_KV (j, "addr", sym.value.addr);
                                JR_BOOL_KV (j, "is_external", sym.is_external);
                            });
                            VecPushBack (&decomp.functions, sym);
                        });

                        JR_OBJ_KV (j, "unmatched_functions", {
                            SymbolInfo sym = {0};
                            sym.is_addr    = false;
                            StrInitCopy (&sym.name, &key);
                            JR_OBJ (j, { JR_STR_KV (j, "value", sym.value.str); });
                            VecPushBack (&decomp.unmatched.functions, sym);
                        });

                        JR_OBJ_KV (j, "unmatched_external_vars", {
                            SymbolInfo sym  = {0};
                            sym.is_addr     = false;
                            sym.is_external = true;
                            StrInitCopy (&sym.name, &key);
                            JR_OBJ (j, { JR_STR_KV (j, "value", sym.value.str); });
                            VecPushBack (&decomp.unmatched.external_vars, sym);
                        });

                        JR_OBJ_KV (j, "unmatched_custom_types", {
                            SymbolInfo sym = {0};
                            sym.is_addr    = false;
                            StrInitCopy (&sym.name, &key);
                            JR_OBJ (j, { JR_STR_KV (j, "value", sym.value.str); });
                            VecPushBack (&decomp.unmatched.custom_types, sym);
                        });

                        JR_OBJ_KV (j, "unmatched_strings", {
                            SymbolInfo sym = {0};
                            sym.is_addr    = false;
                            StrInitCopy (&sym.name, &key);
                            JR_OBJ (j, { JR_STR_KV (j, "value", sym.value.str); });
                            VecPushBack (&decomp.unmatched.strings, sym);
                        });

                        JR_OBJ_KV (j, "unmatched_vars", {
                            SymbolInfo sym = {0};
                            sym.is_addr    = false;
                            StrInitCopy (&sym.name, &key);
                            JR_OBJ (j, { JR_STR_KV (j, "value", sym.value.str); });
                            VecPushBack (&decomp.unmatched.vars, sym);
                        });

                        JR_OBJ_KV (j, "unmatched_go_to_labels", {
                            SymbolInfo sym = {0};
                            sym.is_addr    = false;
                            StrInitCopy (&sym.name, &key);
                            JR_OBJ (j, { JR_STR_KV (j, "value", sym.value.str); });
                            VecPushBack (&decomp.unmatched.go_to_labels, sym);
                        });

                        JR_OBJ_KV (j, "unmatched_custom_function_pointers", {
                            SymbolInfo sym = {0};
                            sym.is_addr    = false;
                            StrInitCopy (&sym.name, &key);
                            JR_OBJ (j, { JR_STR_KV (j, "value", sym.value.str); });
                            VecPushBack (&decomp.unmatched.custom_function_pointers, sym);
                        });

                        JR_OBJ_KV (j, "unmatched_variadic_lists", {
                            SymbolInfo sym = {0};
                            sym.is_addr    = false;
                            StrInitCopy (&sym.name, &key);
                            JR_OBJ (j, { JR_STR_KV (j, "value", sym.value.str); });
                            VecPushBack (&decomp.unmatched.variadic_lists, sym);
                        });
                    });
                    // NOTE: Fields skipped
                });
            }
        });

        StrDeinit (&gj);
        return decomp;
    } else {
        return (AiDecompilation) {0};
    }
}

SimilarFunctions GetSimilarFunctions (Connection conn, SimilarFunctionsRequest* request) {
    if (!conn.api_key.length || !conn.host.length) {
        LOG_ERROR ("Missing API key or host to connect to.");
        return (SimilarFunctions) {0};
    }

    if (!request) {
        LOG_ERROR ("Invalid request");
        return (SimilarFunctions) {0};
    }

    if (!request->function_id) {
        LOG_ERROR ("Invalid function id.");
        return (SimilarFunctions) {0};
    }

    Str url = StrInit();
    Str gj  = StrInit();

    StrPrintf (
        &url,
        "%s/v2/functions/%llu/similar-functions",
        conn.host.data,
        request->function_id
    );

    bool is_first = true;
    UrlAddQueryInt (&url, "limit", request->limit, &is_first);
    UrlAddQueryFloat (&url, "distance", request->distance, &is_first);
    VecForeach (&request->collection_ids, id, {
        UrlAddQueryInt (&url, "collection_ids", id, &is_first);
    });
    VecForeach (&request->binary_ids, id, { UrlAddQueryInt (&url, "binary_ids", id, &is_first); });
    UrlAddQueryBool (
        &url,
        "debug",
        (request->debug_include.external_symbols || request->debug_include.system_symbols ||
         request->debug_include.user_symbols),
        &is_first
    );
    if (request->debug_include.user_symbols) {
        UrlAddQueryStr (&url, "debug_types", "USER", &is_first);
    }
    if (request->debug_include.system_symbols) {
        UrlAddQueryStr (&url, "debug_types", "SYSTEM", &is_first);
    }
    if (request->debug_include.external_symbols) {
        UrlAddQueryStr (&url, "debug_types", "EXTERNAL", &is_first);
    }

    if (MakeRequest (&conn.api_key, &url, NULL, &gj, "GET")) {
        StrDeinit (&url);

        StrIter j = StrIterInitFromStr (&gj);

        bool             status    = false;
        SimilarFunctions functions = VecInitWithDeepCopy (NULL, SimilarFunctionDeinit);
        JR_OBJ (j, {
            JR_BOOL_KV (j, "status", status);
            if (status) {
                JR_ARR_KV (j, "data", {
                    SimilarFunction f = {0};
                    f.projection      = VecInit_T (&f.projection);
                    JR_OBJ (j, {
                        JR_INT_KV (j, "function_id", f.id);
                        JR_STR_KV (j, "function_name", f.name);
                        JR_INT_KV (j, "binary_id", f.binary_id);
                        JR_STR_KV (j, "binary_name", f.binary_name);
                        JR_FLT_KV (j, "distance", f.distance);
                        JR_ARR_KV (j, "projection", {
                            f64 p = 0;
                            JR_FLT (j, p);
                            VecPushBack (&f.projection, p);
                        });
                        JR_STR_KV (j, "sha_256_hash", f.sha256);
                    });

                    // XXX: This is a bug in API. API sends "distance" with value of "similarity"
                    // and below is a fix for that
                    f.distance = 1 - f.distance;
                    LOG_INFO ("Fixed distance = %f", f.distance);

                    VecPushBack (&functions, f);
                });
            }
        });

        StrDeinit (&gj);
        return functions;
    } else {
        return (SimilarFunctions) {0};
    }
}

AnalysisId AnalysisIdFromBinaryId (Connection conn, BinaryId binary_id) {
    if (!conn.api_key.length || !conn.host.length) {
        LOG_ERROR ("Missing API key or host to connect to.");
        return 0;
    }

    if (!binary_id) {
        LOG_ERROR ("Invalid binary id");
        return 0;
    }

    Str url = StrInit();
    Str gj  = StrInit();

    StrPrintf (&url, "%s/v2/analyses/lookup/%llu", conn.host.data, binary_id);

    if (MakeRequest (&conn.api_key, &url, NULL, &gj, "GET")) {
        StrDeinit (&url);

        StrIter j = StrIterInitFromStr (&gj);

        AnalysisId id = 0;
        JR_OBJ (j, { JR_INT_KV (j, "analysis_id", id); });
        LOG_INFO ("Analysis ID = %zu", id);

        StrDeinit (&gj);

        return id;
    } else {
        return 0;
    }
}

Str GetAnalysisLogs (Connection conn, AnalysisId analysis_id) {
    if (!conn.api_key.length || !conn.host.length) {
        LOG_ERROR ("Missing API key or host to connect to.");
        return (Str) {0};
    }

    if (!analysis_id) {
        LOG_ERROR ("Invalid analysis id");
        return (Str) {0};
    }

    Str url = StrInit();
    StrPrintf (&url, "%s/v2/analyses/%llu/logs", conn.host.data, analysis_id);

    Str gj = StrInit();

    if (MakeRequest (&conn.api_key, &url, NULL, &gj, "GET")) {
        StrDeinit (&url);

        StrIter j = StrIterInitFromStr (&gj);

        Str  logs   = StrInit();
        bool status = false;
        JR_OBJ (j, {
            JR_BOOL_KV (j, "status", status);
            if (status) {
                JR_OBJ_KV (j, "data", { JR_STR_KV (j, "logs", logs); });
            }
        });

        StrDeinit (&gj);

        return logs;
    } else {
        return (Str) {0};
    }
}

Str UploadFile (Connection conn, Str file_path) {
    if (!conn.api_key.length || !conn.host.length) {
        LOG_ERROR ("Missing API key or host to connect to.");
        return (Str) {0};
    }

    if (!file_path.length) {
        LOG_ERROR ("Invalid file path");
        return (Str) {0};
    }

    Str url = StrInit();
    StrPrintf (&url, "%s/v1/upload", conn.host.data);

    Str gj = StrInit();

    if (MakeUploadRequest (&conn.api_key, &url, NULL, &gj, "POST", &file_path)) {
        StrDeinit (&url);

        StrIter j = StrIterInitFromStr (&gj);

        bool success = false;
        Str  sha256  = StrInit();
        JR_OBJ (j, {
            JR_BOOL_KV (j, "success", success);
            JR_STR_KV (j, "sha_256_hash", sha256);
        });

        StrDeinit (&gj);

        return sha256;
    } else {
        return (Str) {0};
    }
}

Str* UrlAddQueryStr (Str* url, const char* key, const char* value, bool* is_first) {
    if (!url || !key) {
        LOG_ERROR ("Invalid arguments.");
        return NULL;
    }

    if (!value) {
        LOG_INFO ("Field \"%s\" is empty. Not adding to Url query.", key);
        return url;
    }

    char sep = !is_first ? '?' : *is_first ? '?' : '&';
    StrPushBack (url, sep);
    StrAppendf (url, "%s=%s", key, value);

    if (is_first) {
        *is_first = false;
    }

    return url;
}

Str* UrlAddQueryInt (Str* url, const char* key, i64 value, bool* is_first) {
    if (!url || !key || !value) {
        LOG_ERROR ("Invalid arguments.");
        return NULL;
    }

    char sep = !is_first ? '?' : *is_first ? '?' : '&';
    StrPushBack (url, sep);
    StrAppendf (url, "%s=%lld", key, value);

    if (is_first) {
        *is_first = false;
    }

    return url;
}

Str* UrlAddQueryFloat (Str* url, const char* key, f64 value, bool* is_first) {
    if (!url || !key || !value) {
        LOG_ERROR ("Invalid arguments.");
        return NULL;
    }

    char sep = !is_first ? '?' : *is_first ? '?' : '&';
    StrPushBack (url, sep);
    StrAppendf (url, "%s=%f", key, value);

    if (is_first) {
        *is_first = false;
    }

    return url;
}

Str* UrlAddQueryBool (Str* url, const char* key, bool value, bool* is_first) {
    return UrlAddQueryStr (url, key, value ? "true" : "false", is_first);
}

/* libCURL */
#include <curl/curl.h>

static size CURLResponseWriteCallback (void* ptr, size sz, size nmemb, Str* raw_response) {
    if (!ptr || !raw_response) {
        LOG_ERROR ("Invalid arguments.");
        return 0;
    }

    /* compute and resize buffer to required size */
    size received_size = sz * nmemb;
    size newlen        = raw_response->length + received_size;
    StrPushBackCstr (raw_response, ptr, received_size);
    return received_size;
}

bool MakeRequest (
    Str*        api_key,
    Str*        request_url,
    Str*        request_json,
    Str*        response_json,
    const char* request_method
) {
    if (!api_key || !api_key->length) {
        LOG_ERROR ("Invalid API key");
        return false;
    }

    if (!request_url || !request_url->length) {
        LOG_ERROR ("Invalid request url");
        return false;
    }

    if (!request_method) {
        LOG_ERROR ("Invalid request method.");
        return false;
    }

    LOG_INFO ("REQUEST.URL : %s", request_url->data);

    CURL* curl = curl_easy_init();
    if (!curl) {
        LOG_ERROR ("Failed to create a CURL handle. Cannot make requests.");
        return false;
    }

    // use our own Str if none provided
    Str my_json = StrInit();
    if (!response_json) {
        my_json       = StrInit();
        response_json = &my_json;
    }

    // Authorization header
    Str auth = StrInit();
    StrPrintf (&auth, "Authorization: %s", api_key->data);

    struct curl_slist* headers = NULL;
    headers                    = curl_slist_append (headers, auth.data);
    StrDeinit (&auth);

    if (request_json && request_json->length) {
        headers = curl_slist_append (headers, "Content-Type: application/json");
        curl_easy_setopt (curl, CURLOPT_POSTFIELDS, request_json->data);
        LOG_INFO ("request->JSON: '%s'", request_json->data);
    }

    curl_easy_setopt (curl, CURLOPT_URL, request_url->data);
    curl_easy_setopt (curl, CURLOPT_CUSTOMREQUEST, request_method);
    curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt (curl, CURLOPT_USERAGENT, "creait");
    curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, CURLResponseWriteCallback);
    curl_easy_setopt (curl, CURLOPT_WRITEDATA, response_json);
    curl_easy_setopt (curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt (curl, CURLOPT_CONNECTTIMEOUT, 10L);

    // make request
    CURLcode retcode = curl_easy_perform (curl);
    curl_slist_free_all (headers);
    curl_easy_cleanup (curl);

    // log response always!
    LOG_INFO ("RESPONSE.JSON: '%s'", response_json->data);

    // if we used our json, then deinit that
    if (my_json.length) {
        StrDeinit (&my_json);
    }

    if (retcode != CURLE_OK) {
        LOG_ERROR ("curl_easy_perform() failed: %s", curl_easy_strerror (retcode));
        StrDeinit (response_json);
        return false;
    }

    return true;
}

bool MakeUploadRequest (
    Str*        api_key,
    Str*        request_url,
    Str*        request_json,
    Str*        response_json,
    const char* request_method,
    Str*        file_path
) {
    if (!api_key || !api_key->length) {
        LOG_ERROR ("Invalid API key");
        return false;
    }

    if (!request_url || !request_url->length) {
        LOG_ERROR ("Invalid request url");
        return false;
    }

    if (!request_method) {
        LOG_ERROR ("Invalid request method.");
        return false;
    }

    if (!file_path || !file_path->length) {
        LOG_ERROR (
            "Invalid file path. If uploading a file is not intended, then use MakeRequest method "
            "instead."
        );
        return false;
    }

    LOG_INFO ("REQUEST.URL : %s", request_url->data);

    CURL* curl = curl_easy_init();
    if (!curl) {
        LOG_ERROR ("Failed to create a CURL handle. Cannot make requests.");
        return false;
    }

    /* create a new mime */
    curl_mime* mime = curl_mime_init (curl);
    if (!mime) {
        LOG_ERROR ("CURL failed to create mime.");
        curl_easy_cleanup (curl);
        return false;
    }

    /* create mimepart for multipart data */
    curl_mimepart* mimepart = curl_mime_addpart (mime);
    if (!mimepart) {
        LOG_ERROR ("CURL failed to add mime part.");
        curl_mime_free (mime);
        curl_easy_cleanup (curl);
        return false;
    }

    /* set part info */
    curl_mime_name (mimepart, "file");
    curl_mime_filedata (mimepart, file_path->data);

    LOG_INFO ("UPLOAD FILE : '%s'", file_path->data);

    // use our own Str if none provided
    Str my_json = StrInit();
    if (!response_json) {
        my_json       = StrInit();
        response_json = &my_json;
    }

    Str auth = StrInit();
    StrPrintf (&auth, "Authorization: %s", api_key->data);

    struct curl_slist* headers = NULL;
    headers                    = curl_slist_append (headers, auth.data);
    StrDeinit (&auth);

    if (request_json && request_json->length) {
        headers = curl_slist_append (headers, "Content-Type: application/json");
        curl_easy_setopt (curl, CURLOPT_POSTFIELDS, request_json->data);
        LOG_INFO ("request->JSON: '%s'", request_json->data);
    }

    curl_easy_setopt (curl, CURLOPT_MIMEPOST, mime);
    curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt (curl, CURLOPT_URL, request_url->data);
    curl_easy_setopt (curl, CURLOPT_CUSTOMREQUEST, request_method);
    curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt (curl, CURLOPT_USERAGENT, "creait");
    curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, CURLResponseWriteCallback);
    curl_easy_setopt (curl, CURLOPT_WRITEDATA, response_json);
    curl_easy_setopt (curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt (curl, CURLOPT_CONNECTTIMEOUT, 10L);

    CURLcode retcode = curl_easy_perform (curl);
    curl_slist_free_all (headers);
    curl_mime_free (mime);
    curl_easy_cleanup (curl);

    LOG_INFO ("RESPONSE.JSON: '%s'", response_json->data);

    // if we used our json, then deinit that
    if (my_json.length) {
        StrDeinit (&my_json);
    }

    if (retcode != CURLE_OK) {
        LOG_ERROR ("curl_easy_perform() failed: %s", curl_easy_strerror (retcode));
        StrDeinit (response_json);
        return false;
    }

    return true;
}
