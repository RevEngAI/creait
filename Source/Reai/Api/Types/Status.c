#include <Reai/Api/Types/Status.h>
#include <Reai/Log.h>

void StatusToStr (Status status, Str* ss) {
    if (!ss) {
        LOG_FATAL (
            "Invalid Str object provided to store status string into. Cannot convert to status "
            "string. Aborting..."
        );
    }

    // validation
    const u8 SOURCE_MASK = ANALYSIS_STATUS | DYN_EXEC_STATUS | AI_DECOMP_STATUS;
    if ((status & SOURCE_MASK) == SOURCE_MASK) {
        LOG_FATAL (
            "Invalid status value. Source seems to be both analysis status and dynamic execution "
            "status. Cannot convert. Aborting..."
        );
    }

    if (status & ANALYSIS_STATUS) {
        switch (status & STATUS_MASK) {
            case STATUS_QUEUED : {
                StrPrintf (ss, "Queued");
                break;
            }
            case STATUS_PROCESSING : {
                StrPrintf (ss, "Processing");
                break;
            }
            case STATUS_COMPLETE : {
                StrPrintf (ss, "Complete");
                break;
            }
            case STATUS_UPLOADED : {
                StrPrintf (ss, "Uploaded");
                break;
            }
            case STATUS_ERROR : {
                StrPrintf (ss, "Error");
                break;
            }
            case STATUS_ALL : {
                StrPrintf (ss, "All");
                break;
            }
            default : {
                LOG_ERROR ("Invalid status provided.");
                StrPrintf (ss, "InvalidAnalysisStatus");
            }
        }
    } else if (status & DYN_EXEC_STATUS) {
        // NOTE: this branch handles some cases from AI_DECOMP_STATUS branch as well
        switch (status & STATUS_MASK) {
            case STATUS_PENDING :
                StrPrintf (ss, "PENDING");
                break;
            case STATUS_ERROR :
                StrPrintf (ss, "ERROR");
                break;
            case STATUS_SUCCESS :
                StrPrintf (ss, "SUCCESS");
                break;
            case STATUS_ALL :
                StrPrintf (ss, "ALL");
                break;
            default :
                LOG_ERROR ("Invalid status provided.");
                StrPrintf (ss, "InvalidDynExecStatus");
        }
    } else if (status & AI_DECOMP_STATUS) {
        switch (status & STATUS_MASK) {
            case STATUS_UNINITIALIZED :
                StrPrintf (ss, "UNINITIALIZED");
                break;
            case STATUS_RUNNING :
                StrPrintf (ss, "RUNNING");
                break;
                // NOTE: cases of error and success are common with DYN_EXEC_STATUS
                // whenever these enums are provided, translation will happen from that branch
            default :
                LOG_ERROR ("Invalid status provided.");
                StrPrintf (ss, "InvalidDynExecStatus");
        }
    } else {
        LOG_FATAL ("Invalid status value. Cannot stringify. Aborting...");
    }
}

Status StatusFromStr (Str* ss) {
    if (!ss) {
        LOG_FATAL (
            "Invalid status string provided to read analysis status from. Cannot convert to "
            "analysis status. Aborting..."
        );
    }

    if (!ss->data || !ss->length) {
        return STATUS_INVALID;
    }

    if (!StrCmpZstr (ss, "Queued")) {
        return STATUS_QUEUED | ANALYSIS_STATUS;
    }
    if (!StrCmpZstr (ss, "Processing")) {
        return STATUS_PROCESSING | ANALYSIS_STATUS;
    }
    if (!StrCmpZstr (ss, "Complete")) {
        return STATUS_COMPLETE | ANALYSIS_STATUS;
    }
    if (!StrCmpZstr (ss, "Uploaded")) {
        return STATUS_UPLOADED | ANALYSIS_STATUS;
    }
    if (!StrCmpZstr (ss, "Error")) {
        return STATUS_ERROR | ANALYSIS_STATUS;
    }
    if (!StrCmpZstr (ss, "All")) {
        return STATUS_ALL | ANALYSIS_STATUS;
    }
    if (!StrCmpZstr (ss, "PENDING")) {
        return STATUS_PROCESSING | DYN_EXEC_STATUS ;
    }
    if (!StrCmpZstr (ss, "ERROR")) {
        return STATUS_ERROR | DYN_EXEC_STATUS | AI_DECOMP_STATUS;
    }
    if (!StrCmpZstr (ss, "SUCCESS")) {
        return STATUS_COMPLETE | DYN_EXEC_STATUS;
    }
    if (!StrCmpZstr (ss, "COMPLETED")) {
        return STATUS_COMPLETE | AI_DECOMP_STATUS;
    }
    if (!StrCmpZstr (ss, "RUNNING")) {
        return STATUS_RUNNING | DYN_EXEC_STATUS ;
    }
    if (!StrCmpZstr (ss, "UNINITIALISED")) {
        return STATUS_UNINITIALIZED | AI_DECOMP_STATUS;
    }
    if (!StrCmpZstr (ss, "ALL")) {
        return STATUS_ALL | DYN_EXEC_STATUS;
    }

    return STATUS_INVALID;
}
