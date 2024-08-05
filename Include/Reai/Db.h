/**
 * @file Db.h
 * @date 30th July 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_DB_H
#define REAI_DB_H

#include <Reai/AnalysisInfo.h>
#include <Reai/Types.h>
#include <Reai/Util/CStrVec.h>
#include <Reai/Util/IntVec.h>

typedef struct ReaiDb ReaiDb;
typedef struct Reai   Reai;

ReaiDb* reai_db_create (CString name);
void    reai_db_destroy (ReaiDb* db);

Bool reai_db_require_analysis_status_update (ReaiDb* db);

/* interaction with "uploaded_file" table */
CStrVec* reai_db_get_hashes_for_file_path (ReaiDb* db, CString file_path);
CString  reai_db_get_latest_hash_for_file_path (ReaiDb* db, CString file_path);
CString  reai_db_get_upload_time (ReaiDb* db, CString sha_256_hash);

/* with "created_analysis" table */
ReaiBinaryId reai_db_get_latest_analysis_for_file (ReaiDb* db, CString file_path);
Bool         reai_db_check_analysis_exists (ReaiDb* db, ReaiBinaryId id);
U64Vec*      reai_db_get_analyses_created_for_binary (ReaiDb* db, CString binary_sha_256_hash);
U64Vec*      reai_db_get_all_created_analyses (ReaiDb* db);
U64Vec*      reai_db_get_analyses_with_status (ReaiDb* db, ReaiAnalysisStatus);
CString      reai_db_get_analysis_creation_time (ReaiDb* db, ReaiBinaryId id);
CString      reai_db_get_analysis_binary_file_hash (ReaiDb* db, ReaiBinaryId id);
CString      reai_db_get_analysis_file_name (ReaiDb* db, ReaiBinaryId id);
CString      reai_db_get_analysis_cmdline_args (ReaiDb* db, ReaiBinaryId id);
CString      reai_db_get_analysis_model_name (ReaiDb* db, ReaiBinaryId id);
ReaiAnalysisStatus reai_db_get_analysis_status (ReaiDb* db, ReaiBinaryId id);

/* with "ai_model" table */
Uint32  reai_db_get_model_id_for_model_name (ReaiDb* db, CString model_name);
CString reai_db_get_model_name_for_model_id (ReaiDb* db, Uint32 model_id);

/* with "functions" table */
/* CString reai_db_get_function_name (ReaiDb* db, ReaiFunctionId fn_id); */

Bool reai_db_set_analysis_status (ReaiDb* db, ReaiBinaryId id, ReaiAnalysisStatus new_status);

Bool reai_db_add_upload (ReaiDb* db, CString file_path, CString sha_256_hash);
Bool reai_db_add_analysis (
    ReaiDb*      db,
    ReaiBinaryId binary_id,
    CString      sha_256_hash,
    Uint32       model_id,
    CString      file_name,
    CString      cmdline_args
);
Bool reai_db_add_ai_model (ReaiDb* db, CString model_name, Uint32 model_id);
/* Bool reai_db_add_function (ReaiDb* db, ReaiBinaryId bin_id, ReaiFunctionId fn_id, CString fn_name); */

#endif // REAI_DB_H
