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

u64 getDiffLineNumber(const DiffLine* line) {
    switch (line->type) {
        case DIFF_TYPE_SAM:
        case DIFF_TYPE_REM:
        case DIFF_TYPE_ADD:
            return line->sam.line; // All use the same union field
        case DIFF_TYPE_MOV:
        case DIFF_TYPE_MOD:
            return line->mov.old_line;
        default:
            return 0;
    }
}

int DiffLineNumberCompare(const void* a, const void* b) {
    const DiffLine* line_a = (const DiffLine*)a;
    const DiffLine* line_b = (const DiffLine*)b;
    
    u64 line_a_num = getDiffLineNumber(line_a);
    u64 line_b_num = getDiffLineNumber(line_b);
    
    if (line_a_num < line_b_num) return -1;
    if (line_a_num > line_b_num) return 1;
    return 0;
}

u32 StrLevenshteinDistance (Str* s1, Str* s2) {
    if (!s1 || !s2) {
        LOG_FATAL ("Invalid arguments");
    }

    u64 len1 = s1->length;
    u64 len2 = s2->length;
    const char* str1 = s1->data;
    const char* str2 = s2->data;

    // Handle empty strings
    if (len1 == 0) return len2;
    if (len2 == 0) return len1;

    // Create matrix for dynamic programming
    // We only need two rows for space optimization
    u32* prev_row = (u32*)malloc((len2 + 1) * sizeof(u32));
    u32* curr_row = (u32*)malloc((len2 + 1) * sizeof(u32));
    
    if (!prev_row || !curr_row) {
        if (prev_row) free(prev_row);
        if (curr_row) free(curr_row);
        LOG_FATAL("Memory allocation failed for Levenshtein distance calculation");
    }

    // Initialize first row (distance from empty string)
    for (u64 j = 0; j <= len2; j++) {
        prev_row[j] = j;
    }

    // Fill the matrix row by row
    for (u64 i = 1; i <= len1; i++) {
        curr_row[0] = i; // Distance from empty string
        
        for (u64 j = 1; j <= len2; j++) {
            u32 cost = (str1[i-1] == str2[j-1]) ? 0 : 1;
            
            // Calculate minimum of three operations:
            // 1. Deletion: curr_row[j-1] + 1
            // 2. Insertion: prev_row[j] + 1  
            // 3. Substitution: prev_row[j-1] + cost
            u32 deletion = curr_row[j-1] + 1;
            u32 insertion = prev_row[j] + 1;
            u32 substitution = prev_row[j-1] + cost;
            
            curr_row[j] = MIN2(MIN2(deletion, insertion), substitution);
        }
        
        // Swap rows for next iteration
        u32* temp = prev_row;
        prev_row = curr_row;
        curr_row = temp;
    }

    u32 distance = prev_row[len2];
    
    free(prev_row);
    free(curr_row);
    
    return distance;
}

bool StrAreSimilar (Str* s1, Str* s2, u32 max_distance) {
    if (!s1 || !s2) {
        LOG_FATAL ("Invalid arguments");
    }

    // Quick check: if length difference is already > max_distance, they can't be similar
    u64 length_diff = (s1->length > s2->length) ? (s1->length - s2->length) : (s2->length - s1->length);
    if (length_diff > max_distance) {
        return false;
    }

    u32 distance = StrLevenshteinDistance(s1, s2);
    return distance <= max_distance;
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

    // Phase 1: Find exact matches in the same positions
    size_t min_length = og_lines.length < nw_lines.length ? og_lines.length : nw_lines.length;
    for (size_t i = 0; i < min_length; i++) {        
        if (StrCmp(VecPtrAt(&og_lines, i), VecPtrAt(&nw_lines, i)) == 0) {
            DiffLine same = DiffLineInitSam(i, VecPtrAt(&og_lines, i));
            VecPushBack(&diff, same);
            VecAt(&og_matched, i) = true;
            VecAt(&nw_matched, i) = true;
        }
    }

    // Phase 2: Find moves and modifications in a single pass
    VecForeachPtrIdx(&og_lines, og_line, og_idx, {
        if (VecAt(&og_matched, og_idx)) continue;
        
        bool found_match = false;
        VecForeachPtrIdx(&nw_lines, nw_line, nw_idx, {
            if (VecAt(&nw_matched, nw_idx)) continue;
            
            // First check for exact match (move)
            if (StrCmp(og_line, nw_line) == 0) {
                DiffLine move = DiffLineInitMov(og_idx, nw_idx, nw_line);
                VecPushBack(&diff, move);
                VecAt(&og_matched, og_idx) = true;
                VecAt(&nw_matched, nw_idx) = true;
                found_match = true;
                break;
            }
        });
        
        // If no exact match found, look for fuzzy match (modification)
        if (!found_match) {
            VecForeachPtrIdx(&nw_lines, nw_line, nw_idx, {
                if (VecAt(&nw_matched, nw_idx)) continue;
                
                // Calculate max distance based on string lengths
                // Use 20% of the average length, with minimum of 2 and maximum of 10
                u64 avg_length = (og_line->length + nw_line->length) / 2;
                u32 max_distance = MAX2(2, MIN2(10, avg_length / 5));
                
                if (StrAreSimilar(og_line, nw_line, max_distance)) {
                    DiffLine mod = DiffLineInitMod(og_idx, og_line, nw_idx, nw_line);
                    VecPushBack(&diff, mod);
                    VecAt(&og_matched, og_idx) = true;
                    VecAt(&nw_matched, nw_idx) = true;
                    break;
                }
            });
        }
    });

    // Phase 3: Handle remaining unmatched lines (removals and additions)
    VecForeachPtrIdx(&og_lines, og_line, og_idx, {
        if (!VecAt(&og_matched, og_idx)) {
            DiffLine removed = DiffLineInitRem(og_idx, og_line);
            VecPushBack(&diff, removed);
        }
    });
    
    VecForeachPtrIdx(&nw_lines, nw_line, nw_idx, {
        if (!VecAt(&nw_matched, nw_idx)) {
            DiffLine added = DiffLineInitAdd(nw_idx, nw_line);
            VecPushBack(&diff, added);
        }
    });

    // Phase 4: Sort the diff results by line number for proper output order
    VecSort(&diff, DiffLineNumberCompare);

    // Cleanup
    VecDeinit(&og_matched);
    VecDeinit(&nw_matched);
    VecDeinit(&og_lines);
    VecDeinit(&nw_lines);

    return diff;
}
