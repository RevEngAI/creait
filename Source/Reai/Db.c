/**
 * @file Db.c
 * @date 30th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

/* reai */
#include <Reai/Common.h>
#include <Reai/Db.h>

/* libc */
#include <string.h>

/* sqlite3 */
#include <sqlite3.h>

#include "Reai/AnalysisInfo.h"
#include "Reai/Api/Request.h"

typedef struct ReaiDb {
    CString  db_name;
    sqlite3* db_conn;
} ReaiDb;

PRIVATE ReaiDb* init_tables (ReaiDb* db);

/**
 * @b Create a new Reai Toolking database.
 *
 * @param name Database name
 *
 * @return @c ReaiDb on success.
 * @return @c NULL otherwise.
 * */
ReaiDb* reai_db_create (CString name) {
    RETURN_VALUE_IF (!name, NULL, ERR_INVALID_ARGUMENTS);

    ReaiDb* db = NEW (ReaiDb);
    RETURN_VALUE_IF (!db, NULL, ERR_OUT_OF_MEMORY);

    db->db_name = strdup (name);
    GOTO_HANDLER_IF (!db->db_name, CREATE_NEW_DB_FAILED, ERR_OUT_OF_MEMORY);

    /* open database connection */
    GOTO_HANDLER_IF (
        (sqlite3_open (name, &db->db_conn) != SQLITE_OK) || !db->db_conn,
        CREATE_NEW_DB_FAILED,
        "Failed to create new Reai DB \"%s\" : %s",
        name,
        sqlite3_errmsg (db->db_conn)
    );

    GOTO_HANDLER_IF (
        !init_tables (db),
        CREATE_NEW_DB_FAILED,
        "Failed to initialize tables in database."
    );

    return db;

CREATE_NEW_DB_FAILED:
    reai_db_destroy (db);
    return NULL;
}

/**
 * @b Destroy given Reai Database.
 *
 * Closes connection with internal database and frees any acquired memory
 * and resources.
 *
 * @param db
 * */
void reai_db_destroy (ReaiDb* db) {
    RETURN_IF (!db, ERR_INVALID_ARGUMENTS);

    if (db->db_conn) {
        sqlite3_close (db->db_conn);
    }

    if (db->db_name) {
        FREE (db->db_name);
    }

    memset (db, 0, sizeof (ReaiDb));
    FREE (db);
}

#define PREPARE_SQL_QUERY(stmt, query_fmtstr, ...)                                                 \
    stmt = NULL;                                                                                   \
    do {                                                                                           \
        /* generate query */                                                                       \
        Size buf_size = snprintf (NULL, 0, query_fmtstr, __VA_ARGS__) + 1;                         \
        Char sql_query_buf[buf_size];                                                              \
        memset (sql_query_buf, 0, buf_size);                                                       \
        snprintf (sql_query_buf, buf_size, query_fmtstr, __VA_ARGS__);                             \
                                                                                                   \
        /* compile sql */                                                                          \
        /* REF : https://www.sqlite.org/c3ref/prepare.html */                                      \
        RETURN_VALUE_IF (                                                                          \
            sqlite3_prepare_v2 (db->db_conn, sql_query_buf, buf_size, &stmt, NULL) != SQLITE_OK,   \
            0,                                                                                     \
            SQL_ERROR ("Failed to prepare query for execution", sql_query_buf, db)                 \
        );                                                                                         \
    } while (0)

#define EXEC_SQL_QUERY(query_fmtstr, ...)                                                          \
    do {                                                                                           \
        Size buf_size = snprintf (NULL, 0, query_fmtstr, __VA_ARGS__) + 1;                         \
        Char sql_query_buf[buf_size];                                                              \
        memset (sql_query_buf, 0, buf_size);                                                       \
        snprintf (sql_query_buf, buf_size, query_fmtstr, __VA_ARGS__);                             \
                                                                                                   \
        RETURN_VALUE_IF (                                                                          \
            sqlite3_exec (db->db_conn, sql_query_buf, NULL, NULL, NULL) != SQLITE_OK,              \
            0,                                                                                     \
            SQL_ERROR ("Failed to exec query", sql_query_buf, db)                                  \
        );                                                                                         \
    } while (0)

#define SQL_ERROR(myreason, sql_query_buf, db)                                                     \
    myreason " : %s. NEAR : \"%.30s\".\n", sqlite3_errmsg (db->db_conn),                           \
        sql_query_buf +                                                                            \
            MAX (sqlite3_error_offset (db->db_conn), sqlite3_error_offset (db->db_conn) - 10)

/**
 * @b Check whether or not given database requires to a analysis
 *    status update. If any of the records in "created_analysis"
 *    table has non "Complete" status then that requires a status
 *    update from RevEng.AI servers.
 *
 * @param db
 *
 * @return @c true if db requires an update.
 * @return @c false otherwise.
 * */
Bool reai_db_require_analysis_status_update (ReaiDb* db) {
    RETURN_VALUE_IF (!db, false, ERR_INVALID_ARGUMENTS);

    sqlite3_stmt* stmt = NULL;
    PREPARE_SQL_QUERY (
        stmt,
        "SELECT COUNT(*) "
        "FROM created_analysis "
        "WHERE status != 'Complete';%s",
        ""
    );

    /* only one result so we execute only once */
    switch (sqlite3_step (stmt)) {
        case SQLITE_ROW : {
            Size incomplete_count = sqlite3_column_int64 (stmt, 0);
            sqlite3_finalize (stmt);
            return incomplete_count != 0;
        }

        case SQLITE_DONE : {
            sqlite3_finalize (stmt);
            return false;
        }

        default : {
            sqlite3_finalize (stmt);

            PRINT_ERR (
                "Failed to execute query : %s @off=%d.",
                sqlite3_errmsg (db->db_conn),
                sqlite3_error_offset (db->db_conn)
            );
            return false;
        }
    }
    return false;
}

/**
 * @b Get SHA-256 hash value for given file if it's already uploaded
 *    to RevEng.AI Servers.
 *
 * The returned vector must be freed after use.
 * The returned vector contains entries in sorted (descending order) in order
 * of upload time. This means first entry is the hash of latest uploaded binary file,
 * that has the same file path.
 *
 * Multiple hashes are returned in case where multiple binaries belonged to
 * the same file path. This is usually in case of incremental builds.
 *
 * @param db
 * @param file_path
 *
 * @return @c CString containing SHA-256 hash on success and if the binary
 *         file was uploaded before (meaning present in db).
 * @return @c NULL on error or if file was not uploaded previously.
 * */
CStrVec* reai_db_get_hashes_for_file_path (ReaiDb* db, CString file_path) {
    RETURN_VALUE_IF (!db || !file_path, NULL, ERR_INVALID_ARGUMENTS);

    CStrVec* vec = reai_cstr_vec_create();
    RETURN_VALUE_IF (!vec, NULL, "Failed to create cstring vector.");

    sqlite3_stmt* stmt = NULL;
    PREPARE_SQL_QUERY (
        stmt,
        "SELECT sha_256_hash FROM uploaded_file WHERE file_path = ?"
        "ORDER BY upload_date_time DESC;%s",
        ""
    );

    sqlite3_bind_text (stmt, 1, file_path, -1, SQLITE_STATIC);

    /* only one result so we execute only once */
    while (true) {
        switch (sqlite3_step (stmt)) {
            case SQLITE_ROW : {
                CString sha_256_hash = (CString)sqlite3_column_text (stmt, 0);
                reai_cstr_vec_append (vec, &sha_256_hash);
                break;
            }

            case SQLITE_DONE : {
                sqlite3_finalize (stmt);
                return vec;
            }

            default : {
                sqlite3_finalize (stmt);
                reai_cstr_vec_destroy (vec);

                PRINT_ERR (
                    "Failed to execute query : %s @off=%d.",
                    sqlite3_errmsg (db->db_conn),
                    sqlite3_error_offset (db->db_conn)
                );
                return NULL;
            }
        }
    }
}

/**
 * @b Get SHA-256 hash value for given file if it's already uploaded
 *    to RevEng.AI Servers.
 *
 * The returned string must be freed after use.
 *
 * @param db
 * @param file_path
 *
 * @return @c CString containing SHA-256 hash on success and if the binary
 *         file was uploaded before (meaning present in db).
 * @return @c NULL on error or if file was not uploaded previously.
 * */
CString reai_db_get_latest_hash_for_file_path (ReaiDb* db, CString file_path) {
    RETURN_VALUE_IF (!db || !file_path, NULL, ERR_INVALID_ARGUMENTS);

    sqlite3_stmt* stmt = NULL;
    PREPARE_SQL_QUERY (
        stmt,
        "SELECT sha_256_hash FROM uploaded_file WHERE file_path = '%s' "
        "ORDER BY upload_date_time DESC "
        "LIMIT 1;",
        file_path
    );

    /* only one result so we execute only once */
    switch (sqlite3_step (stmt)) {
        case SQLITE_ROW : {
            CString latest_hash = strdup ((CString)sqlite3_column_text (stmt, 0));
            sqlite3_finalize (stmt);
            return latest_hash;
        }

        case SQLITE_DONE : {
            sqlite3_finalize (stmt);
            return NULL;
        }

        default : {
            sqlite3_finalize (stmt);

            PRINT_ERR (
                "Failed to execute query : %s @off=%d.",
                sqlite3_errmsg (db->db_conn),
                sqlite3_error_offset (db->db_conn)
            );
            return NULL;
        }
    }
}

/**
 * @b Get upload time for binary with given hash.
 *
 * Returned string must be freed after use.
 *
 * @param db
 * @param sha_256_hash
 *
 * @return @c CString containing upload time on success.
 * @return @c NULL otherwise.
 * */
CString reai_db_get_upload_time (ReaiDb* db, CString sha_256_hash) {
    RETURN_VALUE_IF (!db || !sha_256_hash, NULL, ERR_INVALID_ARGUMENTS);

    sqlite3_stmt* stmt = NULL;
    PREPARE_SQL_QUERY (
        stmt,
        "SELECT upload_date_time FROM uploaded_file WHERE sha_256_hash = '%s'",
        sha_256_hash
    );

    /* only one result so we execute only once */
    switch (sqlite3_step (stmt)) {
        case SQLITE_ROW : {
            CString upload_time = strdup ((CString)sqlite3_column_text (stmt, 0));
            sqlite3_finalize (stmt);
            return upload_time;
        }

        case SQLITE_DONE : {
            sqlite3_finalize (stmt);
            return NULL;
        }

        default : {
            sqlite3_finalize (stmt);

            PRINT_ERR (
                "Failed to execute query : %s @off=%d.",
                sqlite3_errmsg (db->db_conn),
                sqlite3_error_offset (db->db_conn)
            );
            return NULL;
        }
    }
}

/**
 * @b Get latest record of analysis created for binary with given file path,
 *    that exists in the database.
 *
 * @param db
 * @param file_path
 *
 * @return @c ReaiBinaryId (non-zero) if found successfully.
 * @return @c 0 otherwise.
 * */
ReaiBinaryId reai_db_get_latest_analysis_for_file (ReaiDb* db, CString file_path) {
    RETURN_VALUE_IF (!db || !file_path, 0, ERR_INVALID_ARGUMENTS);

    CString latest_hash = reai_db_get_latest_hash_for_file_path (db, file_path);
    if (!latest_hash) {
        return 0;
    }

    sqlite3_stmt* stmt = NULL;
    PREPARE_SQL_QUERY (
        stmt,
        "SELECT binary_id FROM created_analysis WHERE sha_256_hash = '%s' "
        "ORDER BY creation_date_time DESC "
        "LIMIT 1;",
        latest_hash
    );

    // TODO: if above macro makes an early exit then this won't be freed
    FREE (latest_hash);

    switch (sqlite3_step (stmt)) {
        case SQLITE_ROW : {
            ReaiBinaryId binary_id = sqlite3_column_int64 (stmt, 0);
            sqlite3_finalize (stmt);
            return binary_id;
        }

        case SQLITE_DONE : {
            sqlite3_finalize (stmt);
            return 0;
        }

        default : {
            sqlite3_finalize (stmt);

            PRINT_ERR (
                "Failed to execute query : %s @off=%d.",
                sqlite3_errmsg (db->db_conn),
                sqlite3_error_offset (db->db_conn)
            );
            return 0;
        }
    }
}

/**
 * @b Check if analysis is already created for given binary id.
 *
 * @parma db
 * @param id
 *
 * @return @c true if analysis already exists on success.
 * @return @c false otherwise.
 * */
Bool reai_db_check_analysis_exists (ReaiDb* db, ReaiBinaryId id) {
    RETURN_VALUE_IF (!db || !id, false, ERR_INVALID_ARGUMENTS);

    sqlite3_stmt* stmt = NULL;
    PREPARE_SQL_QUERY (
        stmt,
        "SELECT COUNT(binary_id) FROM created_analysis WHERE binary_id = %llu",
        id
    );

    /* only one result so we execute only once */
    switch (sqlite3_step (stmt)) {
        case SQLITE_ROW : {
            Bool has_analysis = sqlite3_column_int (stmt, 0) == 1;
            sqlite3_finalize (stmt);
            return has_analysis;
        }

        case SQLITE_DONE : {
            sqlite3_finalize (stmt);
            return false;
        }

        default : {
            sqlite3_finalize (stmt);

            PRINT_ERR (
                "Failed to execute query : %s @off=%d.",
                sqlite3_errmsg (db->db_conn),
                sqlite3_error_offset (db->db_conn)
            );
            return false;
        }
    }
}

/**
 * @b Get a list of all created analysis info for given binary.
 *
 * The returned array must be freed after use.
 *
 * @param db
 * @param sha_256_hash
 *
 * @return @c ReaiBinaryId array on success and if analysis were previously
 *         created and added.
 * @return @c NULL otherwise on failure or if no analysis is present.
 * */
U64Vec* reai_db_get_analyses_created_for_binary (ReaiDb* db, CString binary_sha_256_hash) {
    RETURN_VALUE_IF (!db || !binary_sha_256_hash, NULL, ERR_INVALID_ARGUMENTS);

    U64Vec* vec = reai_u64_vec_create();
    RETURN_VALUE_IF (!vec, NULL, "Failed to create uint64 vector.");

    sqlite3_stmt* stmt = NULL;
    PREPARE_SQL_QUERY (
        stmt,
        "SELECT binary_id FROM created_analysis WHERE sha_256_hash = '%s';",
        binary_sha_256_hash
    );

    while (true) {
        switch (sqlite3_step (stmt)) {
            case SQLITE_ROW : {
                Uint64 binary_id = sqlite3_column_int64 (stmt, 0);
                reai_u64_vec_append (vec, &binary_id);
                break;
            }

            case SQLITE_DONE : {
                sqlite3_finalize (stmt);
                return vec;
            }

            default : {
                sqlite3_finalize (stmt);
                reai_u64_vec_destroy (vec);

                PRINT_ERR (
                    "Failed to execute query : %s @off=%d.",
                    sqlite3_errmsg (db->db_conn),
                    sqlite3_error_offset (db->db_conn)
                );
                return NULL;
            }
        }
    }
}

/**
 * @b Get binary ID for all created analyses.
 *
 * @param db
 *
 * @return @c U64Vec containing binary ids for all created analyses on success.
 * @return @c NULL otherwise.
 * */
U64Vec* reai_db_get_all_created_analyses (ReaiDb* db) {
    RETURN_VALUE_IF (!db, NULL, ERR_INVALID_ARGUMENTS);

    U64Vec* vec = reai_u64_vec_create();
    RETURN_VALUE_IF (!vec, NULL, "Failed to create uint64 vector.");

    sqlite3_stmt* stmt = NULL;
    PREPARE_SQL_QUERY (
        stmt,
        "SELECT binary_id FROM created_analysis "
        "ORDER BY creation_date_time DESC;%s",
        ""
    );

    while (true) {
        switch (sqlite3_step (stmt)) {
            case SQLITE_ROW : {
                Uint64 binary_id = sqlite3_column_int64 (stmt, 0);
                reai_u64_vec_append (vec, &binary_id);
                break;
            }

            case SQLITE_DONE : {
                sqlite3_finalize (stmt);
                return vec;
            }

            default : {
                sqlite3_finalize (stmt);
                reai_u64_vec_destroy (vec);

                PRINT_ERR (
                    "Failed to execute query : %s @off=%d.",
                    sqlite3_errmsg (db->db_conn),
                    sqlite3_error_offset (db->db_conn)
                );
                return NULL;
            }
        }
    }
}

/**
 * @b Get all binaries with given analysis status in given database.
 *
 * @param db
 * @param status
 *
 * @return @c U64Vec containing binary id of all analyses with given status on success.
 * @return @c NULL otherwise.
 * */
U64Vec* reai_db_get_analyses_with_status (ReaiDb* db, ReaiAnalysisStatus status) {
    RETURN_VALUE_IF (!db, NULL, ERR_INVALID_ARGUMENTS);

    U64Vec* vec = reai_u64_vec_create();
    RETURN_VALUE_IF (!vec, NULL, "Failed to create uint64 vector.");

    sqlite3_stmt* stmt = NULL;
    PREPARE_SQL_QUERY (
        stmt,
        "SELECT binary_id FROM created_analysis WHERE status = '%s';",
        reai_analysis_status_to_cstr (status)
    );

    while (true) {
        switch (sqlite3_step (stmt)) {
            case SQLITE_ROW : {
                Uint64 binary_id = sqlite3_column_int64 (stmt, 0);
                reai_u64_vec_append (vec, &binary_id);
                break;
            }

            case SQLITE_DONE : {
                sqlite3_finalize (stmt);
                return vec;
            }

            default : {
                sqlite3_finalize (stmt);
                reai_u64_vec_destroy (vec);

                PRINT_ERR (
                    "Failed to execute query : %s @off=%d.",
                    sqlite3_errmsg (db->db_conn),
                    sqlite3_error_offset (db->db_conn)
                );
                return NULL;
            }
        }
    }
}

/**
 * @b Get value from given column for given analysis.
 *
 * The returned string must be freed after use.
 *
 * @param db Database to search in.
 * @param id Binary Id provided after creating analysis.
 *
 * @return @c CString on success.
 * @return @c NULL otherwise
 * */
#define GEN_TEXT_GETTER_FROM_ANALYSIS_TABLE(fn_suffix, field_name)                                 \
    CString reai_db_get_analysis_##fn_suffix (ReaiDb* db, ReaiBinaryId bin_id) {                   \
        RETURN_VALUE_IF (!db || !bin_id, NULL, ERR_INVALID_ARGUMENTS);                             \
                                                                                                   \
        sqlite3_stmt* stmt = NULL;                                                                 \
        PREPARE_SQL_QUERY (                                                                        \
            stmt,                                                                                  \
            "SELECT " #field_name " FROM created_analysis WHERE binary_id = %llu;",                \
            bin_id                                                                                 \
        );                                                                                         \
                                                                                                   \
        /* only one result so we execute only once */                                              \
        CString val = NULL;                                                                        \
        switch (sqlite3_step (stmt)) {                                                             \
            case SQLITE_ROW : {                                                                    \
                val = (CString)sqlite3_column_text (stmt, 0);                                      \
                sqlite3_finalize (stmt);                                                           \
                return val ? strdup (val) : NULL;                                                  \
            }                                                                                      \
                                                                                                   \
            case SQLITE_DONE : {                                                                   \
                sqlite3_finalize (stmt);                                                           \
                return NULL;                                                                       \
            }                                                                                      \
                                                                                                   \
            default : {                                                                            \
                sqlite3_finalize (stmt);                                                           \
                PRINT_ERR (                                                                        \
                    "Failed to execute query : %s @off=%d.",                                       \
                    sqlite3_errmsg (db->db_conn),                                                  \
                    sqlite3_error_offset (db->db_conn)                                             \
                );                                                                                 \
                return NULL;                                                                       \
            }                                                                                      \
        }                                                                                          \
                                                                                                   \
        return val;                                                                                \
    }

GEN_TEXT_GETTER_FROM_ANALYSIS_TABLE (binary_file_hash, sha_256_hash);
GEN_TEXT_GETTER_FROM_ANALYSIS_TABLE (file_name, file_name);
GEN_TEXT_GETTER_FROM_ANALYSIS_TABLE (cmdline_args, cmdline_args);

/**
 * @b Get analysis status for given binary id.
 *
 * @param db
 * @param bin_id
 *
 * @return @c ReaiAnalysisStatus on success.
 @ @return @c REAI_ANALYSIS_STATUS_INVALID otherwise
 * */
ReaiAnalysisStatus reai_db_get_analysis_status (ReaiDb* db, ReaiBinaryId bin_id) {
    RETURN_VALUE_IF (!db || !bin_id, REAI_ANALYSIS_STATUS_INVALID, ERR_INVALID_ARGUMENTS);

    sqlite3_stmt* stmt = NULL;
    PREPARE_SQL_QUERY (stmt, "SELECT status FROM created_analysis WHERE binary_id = %llu;", bin_id);

    /* only one result so we execute only once */
    switch (sqlite3_step (stmt)) {
        case SQLITE_ROW : {
            ReaiAnalysisStatus val =
                reai_analysis_status_from_cstr ((CString)sqlite3_column_text (stmt, 0));
            sqlite3_finalize (stmt);
            return val;
        }

        case SQLITE_DONE : {
            sqlite3_finalize (stmt);
            return REAI_ANALYSIS_STATUS_INVALID;
        }

        default : {
            sqlite3_finalize (stmt);
            PRINT_ERR (
                "Failed to execute query : %s @off=%d.",
                sqlite3_errmsg (db->db_conn),
                sqlite3_error_offset (db->db_conn)
            );
            return REAI_ANALYSIS_STATUS_INVALID;
        }
    }
}

/**
 * @b Get model name used to create given analysis.
 *
 * Returned string must be freed after use.
 *
 * @param db
 * @param bin_id
 *
 * @return @c CString containing model name on success.
 * @return @c NULL otherwise.
 * */
CString reai_db_get_analysis_model_name (ReaiDb* db, ReaiBinaryId bin_id) {
    RETURN_VALUE_IF (!db, NULL, ERR_INVALID_ARGUMENTS);

    sqlite3_stmt* stmt = NULL;
    PREPARE_SQL_QUERY (
        stmt,
        "SELECT model_id FROM created_analysis WHERE binary_id = %llu",
        bin_id
    );

    /* only one result so we execute only once */
    switch (sqlite3_step (stmt)) {
        case SQLITE_ROW : {
            Uint32 model_id = sqlite3_column_int (stmt, 0);
            sqlite3_finalize (stmt);
            return reai_db_get_model_name_for_model_id (db, model_id);
        }

        case SQLITE_DONE : {
            sqlite3_finalize (stmt);
            return NULL;
        }

        default : {
            sqlite3_finalize (stmt);

            PRINT_ERR (
                "Failed to execute query : %s @off=%d.",
                sqlite3_errmsg (db->db_conn),
                sqlite3_error_offset (db->db_conn)
            );
            return NULL;
        }
    }
}

/**
 * @b Get model id for given model name.
 *
 * @param db
 * @param model_name
 *
 * @return @c Uint32 model id on success.
 * @return @c 0 otherwise.
 * */
Uint32 reai_db_get_model_id_for_model_name (ReaiDb* db, CString model_name) {
    RETURN_VALUE_IF (!db || !model_name, 0, ERR_INVALID_ARGUMENTS);

    static const CString query_fmtstr = "SELECT model_id FROM ai_model WHERE model_name = '%s';";

    /* generate query */
    Size buf_size = snprintf (NULL, 0, query_fmtstr, model_name);
    Char query_buf[buf_size + 1];
    memset (query_buf, 0, buf_size + 1);
    snprintf (query_buf, buf_size, query_fmtstr, model_name);

    /* compile sql */
    /* REF : https://www.sqlite.org/c3ref/prepare.html */
    sqlite3_stmt* stmt = NULL;
    RETURN_VALUE_IF (
        sqlite3_prepare_v2 (db->db_conn, query_buf, buf_size, &stmt, NULL) != SQLITE_OK,
        0,
        "Failed to prepare query for execution."
    );

    /* execute once because we expect only one result */
    switch (sqlite3_step (stmt)) {
        case SQLITE_ROW : {
            Uint32 model_id = sqlite3_column_int (stmt, 0);
            sqlite3_finalize (stmt);
            return model_id;
        }

        case SQLITE_DONE : {
            sqlite3_finalize (stmt);
            return 0;
        }

        default : {
            sqlite3_finalize (stmt);
            PRINT_ERR (
                "Failed to execute query : %s @off=%d.",
                sqlite3_errmsg (db->db_conn),
                sqlite3_error_offset (db->db_conn)
            );
            return 0;
        }
    }
}

/**
 * @b Get AI model name from given model id.
 *
 * The returned string must be freed after use.
 *
 * @param db
 * @param model_id
 *
 * @return @c CString on success.
 * @return @c NULL otherwise.
 * */
CString reai_db_get_model_name_for_model_id (ReaiDb* db, Uint32 model_id) {
    RETURN_VALUE_IF (!db, NULL, ERR_INVALID_ARGUMENTS);

    sqlite3_stmt* stmt = NULL;
    PREPARE_SQL_QUERY (stmt, "SELECT model_name FROM ai_model WHERE model_id = %u;", model_id);

    /* execute once because we expect only one result */
    switch (sqlite3_step (stmt)) {
        case SQLITE_ROW : {
            CString model_name = strdup ((CString)sqlite3_column_text (stmt, 0));
            sqlite3_finalize (stmt);
            return model_name;
        }

        case SQLITE_DONE : {
            sqlite3_finalize (stmt);
            return NULL;
        }

        default : {
            sqlite3_finalize (stmt);
            PRINT_ERR (
                "Failed to execute query : %s @off=%d.",
                sqlite3_errmsg (db->db_conn),
                sqlite3_error_offset (db->db_conn)
            );
            return NULL;
        }
    }
}

/**
 * @b Add binary file path and SHA-256 hash value retrieved from server
 *    response after uploading file.
 *
 * @param db
 * @param file_path
 * @param sha_256_hash
 * */
Bool reai_db_add_upload (ReaiDb* db, CString file_path, CString sha_256_hash) {
    RETURN_VALUE_IF (!db || !file_path || !sha_256_hash, false, ERR_INVALID_ARGUMENTS);

    sqlite3_stmt* stmt = NULL;
    PREPARE_SQL_QUERY (
        stmt,
        "INSERT  INTO uploaded_file "
        "(file_path, sha_256_hash) VALUES "
        "(?,      '%s');",
        sha_256_hash
    );

    sqlite3_bind_text (stmt, 1, file_path, -1, SQLITE_STATIC);

    switch (sqlite3_step (stmt)) {
        case SQLITE_ROW :
        case SQLITE_DONE : {
            sqlite3_finalize (stmt);
            return true;
        }
        default : {
            sqlite3_finalize (stmt);
            RETURN_VALUE_IF_REACHED (
                false,
                "Failed to execute sql query. : %s",
                sqlite3_errmsg (db->db_conn)
            );
        }
    }
}

/**
 * @b Add analysis info for given file. The analysis info is sent to server
 *    in order to create a new analysis.
 *
 * @param db
 * @param sha_256_hash Hash value returned in response after uploading binary file.
 * @param binary_id Binary Id returned in response after creating analysis.
 *
 * @return @c true on successful insertion to database.
 * @return @c false otherwise.
 * */
Bool reai_db_add_analysis (
    ReaiDb*      db,
    ReaiBinaryId binary_id,
    CString      sha_256_hash,
    Uint32       model_id,
    CString      file_name,
    CString      cmdline_args
) {
    RETURN_VALUE_IF (!db || !sha_256_hash || !file_name, false, ERR_INVALID_ARGUMENTS);

    sqlite3_stmt* stmt = NULL;
    PREPARE_SQL_QUERY (
        stmt,
        "INSERT INTO created_analysis "
        "(binary_id, sha_256_hash, model_id, file_name, cmdline_args) VALUES "
        "(%llu,      '%s',         %u,       ?,        '%s');",
        binary_id,
        sha_256_hash,
        model_id,
        cmdline_args ? cmdline_args : "NULL"
    );

    sqlite3_bind_text (stmt, 1, file_name, -1, SQLITE_STATIC);

    switch (sqlite3_step (stmt)) {
        case SQLITE_ROW :
        case SQLITE_DONE : {
            sqlite3_finalize (stmt);
            return true;
        }
        default : {
            sqlite3_finalize (stmt);
            RETURN_VALUE_IF_REACHED (false, "Failed to execute sql query.");
        }
    }
}

/**
 * @b Set/Update analysis status for given binary.
 *
 * @param db
 * @param binary_id Unique binary id assigned to each analysis.
 * @param new_status
 *
 * @return @c true on success.
 * @return @c false otherwise.
 * */
Bool reai_db_set_analysis_status (
    ReaiDb*            db,
    ReaiBinaryId       binary_id,
    ReaiAnalysisStatus new_status
) {
    RETURN_VALUE_IF (
        !db || !binary_id || !new_status || (new_status >= REAI_ANALYSIS_STATUS_MAX),
        false,
        ERR_INVALID_ARGUMENTS
    );

    EXEC_SQL_QUERY (
        "UPDATE created_analysis SET status = '%s' WHERE binary_id = %llu",
        reai_analysis_status_to_cstr (new_status),
        binary_id
    );

    return true;
}

/**
 * @b Add a new model id,name pair in database.
 *
 * @param db
 * @param model_name
 * @param model_id
 *
 * @return @c true on success.
 * @return @c false otherwise.
 * */
Bool reai_db_add_ai_model (ReaiDb* db, CString model_name, Uint32 model_id) {
    RETURN_VALUE_IF (!db || !model_name, false, ERR_INVALID_ARGUMENTS);

    EXEC_SQL_QUERY (
        "INSERT INTO ai_model "
        "(model_id, model_name) VALUES "
        "(%u,       '%s');",
        model_id,
        model_name
    );

    return true;
}

PRIVATE ReaiDb* init_tables (ReaiDb* db) {
    RETURN_VALUE_IF (!db, NULL, ERR_INVALID_ARGUMENTS);

    // Schema diagram : https://dbdiagram.io/d/RevEng-AI-Local-DB-66a8ba118b4bb5230ebb81a0
    // Many analysis can be created from same uploaded binary file
    // For each created analysis, there is only one analysis info record
    // Many analysis can be created using same AI model
    // Single analysis might have many tags

    EXEC_SQL_QUERY (
        " CREATE TABLE IF NOT EXISTS uploaded_file ( "
        "   file_path        VARCHAR(255) NOT NULL, "
        "   sha_256_hash     VARCHAR(64)  PRIMARY KEY, "
        "   upload_date_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP "
        " );"

        " CREATE TABLE IF NOT EXISTS ai_model ( "
        "   model_id   UNSIGNED INTEGER UNIQUE NOT NULL, "
        "   model_name VARCHAR(255)     PRIMARY KEY "
        " );"

        " CREATE TABLE IF NOT EXISTS created_analysis ( "
        "   binary_id          UNSIGNED BIGINT  PRIMARY KEY, "
        "   sha_256_hash       VARCHAR(64)      NOT NULL, "
        "   file_name          VARCHAR(64)      NOT NULL, "
        "   cmdline_args       VARCHAR(64), "
        "   model_id           UNSIGNED INTEGER NOT NULL, "
        "   creation_date_time DATETIME         NOT NULL DEFAULT CURRENT_TIMESTAMP, "
        "   status             VARCHAR(12)      NOT NULL DEFAULT 'Queued', "

        "   FOREIGN KEY (model_id)     REFERENCES ai_model (model_id), "
        "   FOREIGN KEY (sha_256_hash) REFERENCES uploaded_file (sha_256_hash), "
        "   CHECK       (status IN ('Queued', 'Processing', 'Complete', 'Error')) "
        " ); "

        // Maps a model name to a unique id
        " CREATE TABLE IF NOT EXISTS tags ( "
        "   tag_id   UNSIGNED INTEGER UNIQUE NOT NULL, "
        "   tag_name VARCHAR(255)     PRIMARY KEY "
        " );"

        // Relation between analysis tags and analysis
        " CREATE TABLE IF NOT EXISTS analysis_tags ( "
        "   binary_id UNSIGNED BIGINT NOT NULL, "
        "   tag_id    INTEGER         NOT NULL, "

        "   FOREIGN KEY (binary_id) REFERENCES created_analysis (binary_id), "
        "   FOREIGN KEY (tag_id)    REFERENCES tags (tag_id) "
        " );%s",
        ""
    );

    /* insert AI models with ids */
    EXEC_SQL_QUERY (
        "INSERT OR IGNORE INTO ai_model(model_id, model_name) VALUES "
        "(%u, '%s'), (%u, '%s'), (%u, '%s'), (%u, '%s');",
        REAI_MODEL_X86_WINDOWS,
        "binnet-0.3-x86-windows",
        REAI_MODEL_X86_LINUX,
        "binnet-0.3-x86-linux",
        REAI_MODEL_X86_MACOS,
        "binnet-0.3-x86-macos",
        REAI_MODEL_X86_ANDROID,
        "binnet-0.3-x86-android"
    );

    return db;
}
