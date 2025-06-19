#ifndef REAI_DIFF_H
#define REAI_DIFF_H

#include <Reai/Util/Str.h>

typedef enum {
    DIFF_TYPE_SAM = 's',
    DIFF_TYPE_ADD = '+',
    DIFF_TYPE_REM = '-',
    DIFF_TYPE_MOD = 'a',
    DIFF_TYPE_MOV = 'm'
} DiffType;

typedef struct {
    DiffType type;
    union {
        struct {
            u64 line;
            Str content;
        } sam, add, rem;
        struct {
            u64 old_line;
            u64 new_line;
            Str old_content;
            Str new_content;
        } mov, mod;
    };
} DiffLine;

typedef Vec (DiffLine) DiffLines;

#ifdef __cplusplus
#    define DiffLineInitSam(l, s)                                                                  \
        (DiffLine {.type = DIFF_TYPE_SAM, .sam.line = (l), .sam.content = StrDup (s)})
#    define DiffLineInitAdd(l, s)                                                                  \
        (DiffLine {.type = DIFF_TYPE_ADD, .add.line = (l), .add.content = StrDup (s)})
#    define DiffLineInitRem(l, s)                                                                  \
        (DiffLine {.type = DIFF_TYPE_REM, .rem.line = (l), .rem.content = StrDup (s)})
#    define DiffLineInitMov(lo, ln, s)                                                             \
        (DiffLine {                                                                                \
            .type            = DIFF_TYPE_MOV,                                                      \
            .mov.old_line    = (lo),                                                               \
            .mov.new_line    = (ln),                                                               \
            .mov.old_content = StrDup (s),                                                         \
            .mov.new_content = StrDup (s)                                                          \
        })
#    define DiffLineInitMod(lo, so, ln, sn)                                                        \
        (DiffLine {                                                                                \
            .type            = DIFF_TYPE_MOD,                                                      \
            .mov.old_line    = (lo),                                                               \
            .mov.new_line    = (ln),                                                               \
            .mov.old_content = StrDup (so),                                                        \
            .mov.new_content = StrDup (sn)                                                         \
        })
#else
#    define DiffLineInitSam(l, s)                                                                  \
        ((DiffLine) {.type = DIFF_TYPE_SAM, .sam.line = (l), .sam.content = StrDup (s)})
#    define DiffLineInitAdd(l, s)                                                                  \
        ((DiffLine) {.type = DIFF_TYPE_ADD, .add.line = (l), .add.content = StrDup (s)})
#    define DiffLineInitRem(l, s)                                                                  \
        ((DiffLine) {.type = DIFF_TYPE_REM, .rem.line = (l), .rem.content = StrDup (s)})
#    define DiffLineInitMov(lo, ln, s)                                                             \
        ((DiffLine) {.type            = DIFF_TYPE_MOV,                                             \
                     .mov.old_line    = (lo),                                                      \
                     .mov.new_line    = (ln),                                                      \
                     .mov.old_content = StrDup (s),                                                \
                     .mov.new_content = StrDup (s)})
#    define DiffLineInitMod(lo, so, ln, sn)                                                        \
        ((DiffLine) {.type            = DIFF_TYPE_MOD,                                             \
                     .mov.old_line    = (lo),                                                      \
                     .mov.new_line    = (ln),                                                      \
                     .mov.old_content = StrDup (so),                                               \
                     .mov.new_content = StrDup (sn)})
#endif

#ifdef __cplusplus
extern "C" {
#endif

    REAI_API bool DiffLineInitClone (DiffLine* dst, DiffLine* src);
    REAI_API void DiffLineDeinit (DiffLine* dl);

    REAI_API u32  StrLevenshteinDistance (Str* s1, Str* s2);
    REAI_API bool StrAreSimilar (Str* s1, Str* s2, u32 max_distance);

    REAI_API DiffLines GetDiff (Str* og, Str* nw);

#ifdef __cplusplus
}
#endif

#endif // REAI_DIFF_H
