#include <Reai/Diff.h>
#include <Reai/Log.h>

#include "Reai/Util/Str.h"
#include "Reai/Util/Vec.h"

bool DiffLineInitClone (DiffLine* dst, DiffLine* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid arguments.");
    }

    dst->type = src->type;
    switch (dst->type) {
        case DIFF_TYPE_ADD :
        case DIFF_TYPE_REM :
        case DIFF_TYPE_SAM : {
            dst->add.line    = src->add.line;
            dst->add.content = StrDup (&src->add.content);
            break;
        }

        case DIFF_TYPE_MOD :
        case DIFF_TYPE_MOV : {
            dst->mov.new_line    = src->mov.new_line;
            dst->mov.old_line    = src->mov.old_line;
            dst->mov.new_content = StrDup (&src->mov.new_content);
            dst->mov.old_content = StrDup (&src->mov.old_content);
            break;
        }

        default : {
            LOG_FATAL ("Unreachable code reached : Invalid diff line type.");
        }
    }

    return true;
}

void DiffLineDeinit (DiffLine* dl) {
    if (!dl) {
        LOG_FATAL ("Invalid argument");
    }

    switch (dl->type) {
        case DIFF_TYPE_ADD :
        case DIFF_TYPE_REM :
        case DIFF_TYPE_SAM : {
            StrDeinit (&dl->add.content);
            dl->add.line = 0;
            break;
        }

        case DIFF_TYPE_MOV :
        case DIFF_TYPE_MOD : {
            StrDeinit (&dl->mod.new_content);
            StrDeinit (&dl->mod.old_content);
            dl->mod.new_line = 0;
            dl->mod.old_line = 0;
            break;
        }

        default : {
            LOG_FATAL ("Unreachable code reached. Invalid diff line type.");
        }
    }

    dl->type = 0;
}

int DiffLineNumberCompare(const void* a, const void* b) {
    const DiffLine* line_a = (const DiffLine*)a;
    const DiffLine* line_b = (const DiffLine*)b;
    
    u64 line_a_num, line_b_num;
    
    // Get the primary line number for sorting (using old/source line numbers)
    switch (line_a->type) {
        case DIFF_TYPE_SAM:
        case DIFF_TYPE_REM:
            line_a_num = line_a->sam.line;
            break;
        case DIFF_TYPE_ADD:
            line_a_num = line_a->add.line + 1000000; // Sort additions after source lines
            break;
        case DIFF_TYPE_MOV:
        case DIFF_TYPE_MOD:
            line_a_num = line_a->mov.old_line;
            break;
        default:
            line_a_num = 0;
    }
    
    switch (line_b->type) {
        case DIFF_TYPE_SAM:
        case DIFF_TYPE_REM:
            line_b_num = line_b->sam.line;
            break;
        case DIFF_TYPE_ADD:
            line_b_num = line_b->add.line + 1000000; // Sort additions after source lines
            break;
        case DIFF_TYPE_MOV:
        case DIFF_TYPE_MOD:
            line_b_num = line_b->mov.old_line;
            break;
        default:
            line_b_num = 0;
    }
    
    if (line_a_num < line_b_num) return -1;
    if (line_a_num > line_b_num) return 1;
    return 0;
}

bool StrFuzzyCmp (Str* s1, Str* s2, u32 error_tolerance) {
    if (!s1 || !s2) {
        LOG_FATAL ("Invalid arguments");
    }

    u64         l1 = s1->length;
    u64         l2 = s2->length;
    const char* p1 = s1->data;
    const char* p2 = s2->data;

    u64 length_diff = MAX2 (l1, l2) - MIN2 (l1, l2);
    if (length_diff > error_tolerance) {
        return false;
    } else {
        error_tolerance -= length_diff;
    }

    u64 l = MIN2 (l1, l2);
    while (l-- && *p1 && *p2) {
        if (*p1 != *p2) {
            if (!error_tolerance) {
                return false;
            }
            error_tolerance--;
        }
    }

    return true;
}

DiffLines GetDiff (Str* og, Str* nw) {
    if (!og || !nw) {
        LOG_FATAL ("Invalid arguments");
    }

    DiffLines diff = VecInitWithDeepCopy (NULL, DiffLineDeinit);

    Strs og_lines = StrSplit (og, "\n");
    Strs nw_lines = StrSplit (nw, "\n");
    
    // Create tracking vectors to mark which lines have been matched
    Vec(bool) og_matched = VecInit();
    Vec(bool) nw_matched = VecInit();
    
    // Resize vectors to match the number of lines and initialize to false
    VecResize(&og_matched, og_lines.length);
    VecResize(&nw_matched, nw_lines.length);
    
    // Initialize all elements to false (VecResize should zero-initialize)
    for (size_t i = 0; i < og_lines.length; i++) {
        VecAt(&og_matched, i) = false;
    }
    for (size_t i = 0; i < nw_lines.length; i++) {
        VecAt(&nw_matched, i) = false;
    }

    // Phase 1: Find exact matches in the same positions
    for (size_t i = 0; i < og_lines.length && i < nw_lines.length; i++) {
        if (VecAt(&og_matched, i) || VecAt(&nw_matched, i)) continue;
        
        if (StrCmp(&og_lines.data[i], &nw_lines.data[i]) == 0) {
            DiffLine same = DiffLineInitSam(i, &og_lines.data[i]);
            VecPushBack(&diff, same);
            VecAt(&og_matched, i) = true;
            VecAt(&nw_matched, i) = true;
        }
    }

    // Phase 2: Find moves (exact content in different positions)
    for (size_t og_idx = 0; og_idx < og_lines.length; og_idx++) {
        if (VecAt(&og_matched, og_idx)) continue;
        
        for (size_t nw_idx = 0; nw_idx < nw_lines.length; nw_idx++) {
            if (VecAt(&nw_matched, nw_idx)) continue;
            
            if (StrCmp(&og_lines.data[og_idx], &nw_lines.data[nw_idx]) == 0) {
                DiffLine move = DiffLineInitMov(og_idx, &og_lines.data[og_idx], nw_idx, &nw_lines.data[nw_idx]);
                VecPushBack(&diff, move);
                VecAt(&og_matched, og_idx) = true;
                VecAt(&nw_matched, nw_idx) = true;
                break;
            }
        }
    }

    // Phase 3: Find modifications using fuzzy matching
    for (size_t og_idx = 0; og_idx < og_lines.length; og_idx++) {
        if (VecAt(&og_matched, og_idx)) continue;
        
        for (size_t nw_idx = 0; nw_idx < nw_lines.length; nw_idx++) {
            if (VecAt(&nw_matched, nw_idx)) continue;
            
            // Calculate fuzzy match tolerance (75% of the longer string)
            u32 tolerance = (MAX2(og_lines.data[og_idx].length, nw_lines.data[nw_idx].length) * 3) / 4;
            
            if (StrFuzzyCmp(&og_lines.data[og_idx], &nw_lines.data[nw_idx], tolerance)) {
                DiffLine mod = DiffLineInitMod(og_idx, &og_lines.data[og_idx], nw_idx, &nw_lines.data[nw_idx]);
                VecPushBack(&diff, mod);
                VecAt(&og_matched, og_idx) = true;
                VecAt(&nw_matched, nw_idx) = true;
                break;
            }
        }
    }

    // Phase 4: Handle remaining unmatched lines (removals and additions)
    for (size_t og_idx = 0; og_idx < og_lines.length; og_idx++) {
        if (!VecAt(&og_matched, og_idx)) {
            DiffLine removed = DiffLineInitRem(og_idx, &og_lines.data[og_idx]);
            VecPushBack(&diff, removed);
        }
    }
    
    for (size_t nw_idx = 0; nw_idx < nw_lines.length; nw_idx++) {
        if (!VecAt(&nw_matched, nw_idx)) {
            DiffLine added = DiffLineInitAdd(nw_idx, &nw_lines.data[nw_idx]);
            VecPushBack(&diff, added);
        }
    }

    // Phase 5: Sort the diff results by line number for proper output order
    VecSort(&diff, DiffLineNumberCompare);

    // Cleanup
    VecDeinit(&og_matched);
    VecDeinit(&nw_matched);
    VecDeinit(&og_lines);
    VecDeinit(&nw_lines);

    return diff;
}
