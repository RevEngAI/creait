#ifndef REAI_UTIL_JSON_H
#define REAI_UTIL_JSON_H

#include <Reai/Log.h>
#include <Reai/Types.h>
#include <Reai/Util/Str.h>

///
/// JSON
/// { .. } (object)
/// "key" : "string-value"
/// "key" : number-value
/// "key" : true/false
/// "key" : [array-value]
/// "key" : { object }
/// "key" : null
///

typedef Vec (Str) StrVec;
typedef Vec (i64) Si64Vec;
typedef Vec (i64) F64Vec;

typedef struct {
    char* data;
    i64   length;
    i64   pos;
    size  alignment;
} StrIter;

#define StrIterInit()           {.data = NULL, .length = 0, .pos = 0, .alignment = 1}
#define StrIterInitAligned(aln) {.data = NULL, .length = 0, .pos = 0, .alignment = (aln)}
#define StrIterInitFromStr(v)                                                                      \
    {.data = (v)->data, .length = (v)->length, .pos = 0, .alignment = (v)->alignment}

///
/// Get total length of this StrIter object
///
/// SUCCESS : If provided StrIter object is not NULL_ITER(mi) then returns size in bytes of memory region
///           this StrIter is iterating over.
/// FAILURE : If provided StrIter is NULL_ITER(mi) then returns 0
#define StrIterLength(mi)                                                                          \
    ((mi) ? ((mi)->length) : (LOG_ERROR ("StrIter: Invalid memory iter pointer"), 0))

///
/// Get remaining length left to read this memory iterator.
///
/// SUCCESS : If provided StrIter object is not NULL_ITER(mi) then remaining size left to read in
///           memory region is returned.
/// FAILURE : If provided StrIter is NULL_ITER(mi) then returns 0
///
#define StrIterRemainingLength(mi)                                                                 \
    ((mi) ?                                                                                        \
         (((mi)->pos >= 0 && (mi)->pos < StrIterLength (mi)) ? (StrIterLength (mi) - (mi)->pos) :  \
                                                               0) :                                \
         (LOG_ERROR ("StrIter: Invalid memory pointer"), 0))

///
/// If there's space left to read in memory region we're iterating over,
/// then return a pointer to current read position.
///
/// SUCCESS : If provided StrIter is not NULL_ITER_DATA(mi), and we have space left to read,
///           then return pointer to memory to start/resume reading from.
/// FAILURE : NULL_ITER_DATA(mi) othewise
///
#define StrIterPos(mi) (StrIterRemainingLength (mi) ? ((mi)->data + (mi)->pos) : 0)

///
/// Read object from memory iter, given that
/// - Provided StrIter object is not NULL_ITER(mi).
/// - There's space left to read.
/// - Length of object data is being read into is an integral multiple of size of data type
///   this memory iter is iterating over.
///
/// SUCCESS : Data is copied from current read position to provided `dst`, and `mi` is returned
/// FAILURE : NULL_ITER(mi) returned
///
#define StrIterRead(mi) (StrIterRemainingLength (mi) ? ((mi)->data[(mi)->pos++]) : 0)

///
/// Move current reading position of StrIterator.
///
/// SUCCESS : Data is copied from current read position to provided `dst`, and `mi` is returned
/// FAILURE : NULL_ITER(mi) returned
///
#define StrIterMove(mi, n)                                                                         \
    do {                                                                                           \
        if (((StrIterRemainingLength (mi) - (i64)(n) <= StrIterLength (mi)) &&                     \
             (StrIterRemainingLength (mi) - (i64)(n) >= 0)))                                       \
            (mi)->pos += (n);                                                                      \
        else                                                                                       \
            LOG_ERROR (                                                                            \
                "StrIter: iter move by %zu didn't take place {pos = %zu, length = %zu}",           \
                (n),                                                                               \
                (mi)->pos,                                                                         \
                (mi)->length                                                                       \
            );                                                                                     \
    } while (0)

#define StrIterNext(mi) StrIterMove (mi, 1)
#define StrIterPrev(mi) StrIterMove (mi, -1)

///
/// Peek (not read) object from memory iter, given that
/// - Provided StrIter object is not NULL_ITER(mi).
/// - There's space left to read.
/// - Length of object data is being read into is an integral multiple of size of data type
///   this memory iter is iterating over.
///
/// This is different from reading because it does not change current read position.
/// This is good for making some decisions over data without changing the read position.
///
/// SUCCESS : Data copied over to `dst` from current read position and `mi` is returned.
/// FAILURE : NULL_ITER(mi) returned.
///
#define StrIterPeek(mi) (StrIterRemainingLength (mi) ? ((mi)->data[(mi)->pos]) : 9)

///
/// JSON
/// { .. } (object)
/// "key" : "string-value"
/// "key" : number-value
/// "key" : true/false
/// "key" : [array-value]
/// "key" : { object }
/// "key" : null
///

/// TAGS: Number, Union, DataType, JSON, NumericType
typedef struct Number {
    bool is_float;
    union {
        f64 f;
        i64 i;
    };
} Number;

///
/// Skip whitespace from current reading position.
///
/// si[in] : Reading position to start looking for whitespace
///
/// SUCCESS : Returns `StrIter` advanced past all whitespace
/// FAILURE : Returns original `StrIter` if already at end
///
/// TAGS: JSON, Whitespace, Parsing, Utility
///
StrIter JSkipWhitespace (StrIter si);

///
/// Read a quoted string, handling escape sequences.
///
/// si[in]   : Current reading position in input string
/// str[out] : Output string to store parsed result
///
/// NOTE: Unicode escape sequences like `\uXXXX` are not supported.
///
/// SUCCESS : Returns `StrIter` advanced past closing quote
/// FAILURE : Returns original `StrIter` on error (invalid escape, missing quote, etc.)
///
/// TAGS: JSON, String, Parsing, EscapeSequences
///
StrIter JReadString (StrIter si, Str* str);

///
/// Read a JSON number (int or float) from input string.
///
/// si[in]   : Current reading position in input string
/// num[out] : Output number object to hold parsed result
///
/// SUCCESS : Returns `StrIter` advanced past number
/// FAILURE : Returns original `StrIter` on error (invalid format, empty number, etc.)
///
/// TAGS: JSON, Number, Parsing, Numeric
///
StrIter JReadNumber (StrIter si, Number* num);

///
/// Strictly read an integer from input string.
///
/// si[in]   : Current reading position in input string
/// val[out] : Pointer to i64 to store parsed integer
///
/// SUCCESS : Returns `StrIter` advanced past parsed integer
/// FAILURE : Returns original `StrIter` if float encountered or parsing fails
///
/// TAGS: JSON, Integer, Parsing, Strict
///
StrIter JReadInteger (StrIter si, i64* val);

///
/// Read a floating-point number from input string.
///
/// si[in]   : Current reading position in input string
/// val[out] : Pointer to f64 to store parsed value
///
/// SUCCESS : Returns `StrIter` advanced past parsed float
/// FAILURE : Returns original `StrIter` on error
///
/// TAGS: JSON, Float, Parsing
///
StrIter JReadFloat (StrIter si, f64* val);

///
/// Read a boolean value ("true" or "false") from input string.
///
/// si[in]   : Current reading position in input string
/// b[out]   : Pointer to bool to store parsed result
///
/// SUCCESS : Returns `StrIter` advanced past parsed boolean
/// FAILURE : Returns original `StrIter` if invalid or unrecognized value
///
/// TAGS: JSON, Boolean, Parsing
///
StrIter JReadBool (StrIter si, bool* b);

///
/// Read a "null" value from input string.
///
/// si[in]       : Current reading position in input string
/// is_null[out] : Pointer to bool set to true if "null" found
///
/// SUCCESS : Returns `StrIter` advanced past "null"
/// FAILURE : Returns original `StrIter` if "null" not found
///
StrIter JReadNull (StrIter si, bool* is_null);

///
/// Skip the current JSON value at reading position.
///
/// si[in] : Current position in string iterator to skip value from
///
/// SUCCESS : Returns updated `StrIter` after value is skipped
/// FAILURE : Returns same `StrIter` on error (e.g. invalid type)
///
/// TAGS: JSON, Parsing, Utility
///
StrIter JSkipValue (StrIter si);

///
/// Read a JSON string value from stream and assign to target.
/// The resulting string is dynamically allocated in `Str` format.
///
/// si[in,out] : JSON stream iterator to read from.
/// str[out]   : Destination `Str` to store the string.
///
/// USAGE:
///   JR_STR(si, name);
///
/// SUCCESS : `str` contains the read string
/// FAILURE : `si` updated to failure state on parse error
///
/// TAGS: JSON, Macro, Reader, String
///
#define JR_STR(si, str)                                                                            \
    do {                                                                                           \
        Str my_str = StrInit();                                                                    \
        si         = JReadString ((si), &my_str);                                                  \
        (str)      = my_str;                                                                       \
    } while (0)

///
/// Read a string key-value pair if key matches.
///
/// si[in,out] : JSON stream iterator to read from.
/// k[in]      : Expected key name (C-string).
/// str[out]   : Destination `Str` to store the value.
///
/// USAGE:
///   JR_STR_KV(si, "username", user.name);
///
/// SUCCESS : `str` contains the value if key matched
/// FAILURE : No-op if key does not match
///
/// TAGS: JSON, Macro, Reader, String, KeyValue
///
#define JR_STR_KV(si, k, str)                                                                      \
    do {                                                                                           \
        if (!StrCmpZstr (&key, (k))) {                                                             \
            Str my_str = StrInit();                                                                \
            si         = JReadString ((si), &my_str);                                              \
            (str)      = my_str;                                                                   \
        }                                                                                          \
    } while (0)

///
/// Read a JSON integer value from stream and assign to target.
///
/// si[in,out] : JSON stream iterator to read from.
/// i[out]     : Integer variable to store the value.
///
/// USAGE:
///   JR_INT(si, count);
///
/// SUCCESS : `i` contains the parsed integer
/// FAILURE : `si` updated to failure state on parse error
///
/// TAGS: JSON, Macro, Reader, Integer
///
#define JR_INT(si, i)                                                                              \
    do {                                                                                           \
        i64 my_int = 0;                                                                            \
        si         = JReadInteger ((si), &my_int);                                                 \
        (i)        = my_int;                                                                       \
    } while (0)

///
/// Read an integer key-value pair if key matches.
///
/// si[in,out] : JSON stream iterator to read from.
/// k[in]      : Expected key name (C-string).
/// i[out]     : Integer variable to store the value.
///
/// USAGE:
///   JR_INT_KV(si, "ref", obj.ref);
///
/// SUCCESS : `i` contains value if key matched
/// FAILURE : No-op if key does not match
///
/// TAGS: JSON, Macro, Reader, Integer, KeyValue
///
#define JR_INT_KV(si, k, i)                                                                        \
    do {                                                                                           \
        if (!StrCmpZstr (&key, (k))) {                                                             \
            i64 my_int = 0;                                                                        \
            si         = JReadInteger ((si), &my_int);                                             \
            (i)        = my_int;                                                                   \
        }                                                                                          \
    } while (0)

///
/// Read a JSON float value from stream and assign to target.
///
/// si[in,out] : JSON stream iterator to read from.
/// f[out]     : Float variable to store the value.
///
/// USAGE:
///   JR_FLT(si, temperature);
///
/// SUCCESS : `f` contains the parsed float
/// FAILURE : `si` updated to failure state on parse error
///
/// TAGS: JSON, Macro, Reader, Float
///
#define JR_FLT(si, f)                                                                              \
    do {                                                                                           \
        f64 my_flt = 0;                                                                            \
        si         = JReadFloat ((si), &my_flt);                                                   \
        (f)        = my_flt;                                                                       \
    } while (0)

///
/// Read a float key-value pair if key matches.
///
/// si[in,out] : JSON stream iterator to read from.
/// k[in]      : Expected key name (C-string).
/// f[out]     : Float variable to store the value.
///
/// USAGE:
///   JR_FLT_KV(si, "x_axis_val", obj.data.x);
///
/// SUCCESS : `f` contains value if key matched
/// FAILURE : No-op if key does not match
///
/// TAGS: JSON, Macro, Reader, Float, KeyValue
///
#define JR_FLT_KV(si, k, f)                                                                        \
    do {                                                                                           \
        if (!StrCmpZstr (&key, (k))) {                                                             \
            f64 my_flt = 0;                                                                        \
            si         = JReadFloat ((si), &my_flt);                                               \
            (f)        = my_flt;                                                                   \
        }                                                                                          \
    } while (0)

///
/// Read a JSON boolean value from stream and assign to target.
///
/// si[in,out] : JSON stream iterator to read from.
/// b[out]     : Boolean variable to store the value.
///
/// USAGE:
///   JR_BOOL(si, is_active);
///
/// SUCCESS : `b` contains the parsed boolean
/// FAILURE : `si` updated to failure state on parse error
///
/// TAGS: JSON, Macro, Reader, Boolean
///
#define JR_BOOL(si, b)                                                                             \
    do {                                                                                           \
        bool my_b = 0;                                                                             \
        si        = JReadBool ((si), &my_b);                                                       \
        (b)       = my_b;                                                                          \
    } while (0)

///
/// Read a boolean key-value pair if key matches.
///
/// si[in,out] : JSON stream iterator to read from.
/// k[in]      : Expected key name (C-string).
/// b[out]     : Boolean variable to store the value.
///
/// USAGE:
///   JR_BOOL_KV(si, "enabled", flag);
///
/// SUCCESS : `b` contains value if key matched
/// FAILURE : No-op if key does not match
///
/// TAGS: JSON, Macro, Reader, Boolean, KeyValue
///
#define JR_BOOL_KV(si, k, b)                                                                       \
    do {                                                                                           \
        if (!StrCmpZstr (&key, (k))) {                                                             \
            bool my_b = 0;                                                                         \
            si        = JReadBool ((si), &my_b);                                                   \
            (b)       = my_b;                                                                      \
        }                                                                                          \
    } while (0)

///
/// Read a JSON array using a custom value reader expression.
///
/// The macro parses a JSON array and calls the user-provided code block for each element.
/// If the value can't be parsed or reader fails to advance the iterator, the value is skipped.
///
/// si[in,out] : Stream iterator to read from.
/// reader     : Code block to read each value.
///
/// USAGE:
///   JR_ARR(si, {
///       MyStruct tmp = {0};
///       JR_OBJ(si, {
///           JR_INT_KV(si, "x", tmp.x);
///           JR_FLT_KV(si, "y", tmp.y);
///       });
///       VecPush(&data.items, tmp);
///   });
///
/// SUCCESS : All values processed or skipped gracefully
/// FAILURE : Logs error and restores `si` on structural or read failure
///
/// TAGS: JSON, Macro, Reader, Array
///
#define JR_ARR(si, reader)                                                                         \
    do {                                                                                           \
        if (!StrIterRemainingLength (&si)) {                                                       \
            break;                                                                                 \
        }                                                                                          \
                                                                                                   \
        StrIter saved_si = si;                                                                     \
        si               = JSkipWhitespace (si);                                                   \
                                                                                                   \
        /* starting of an object */                                                                \
        if (StrIterPeek (&si) != '[') {                                                            \
            LOG_ERROR ("Invalid array start. Expected '['.");                                      \
            si = saved_si;                                                                         \
            break;                                                                                 \
        }                                                                                          \
        StrIterNext (&si);                                                                         \
        si = JSkipWhitespace (si);                                                                 \
                                                                                                   \
        bool expect_comma = false;                                                                 \
        bool failed       = false;                                                                 \
                                                                                                   \
        /* while not at the end of array. */                                                       \
        while (StrIterPeek (&si) && StrIterPeek (&si) != ']') {                                    \
            if (expect_comma) {                                                                    \
                if (StrIterPeek (&si) != ',') {                                                    \
                    LOG_ERROR ("Expected ',' between values in array. Invalid JSON array.");       \
                    failed = true;                                                                 \
                    si     = saved_si;                                                             \
                    break;                                                                         \
                }                                                                                  \
                StrIterNext (&si); /* skip comma */                                                \
                si = JSkipWhitespace (si);                                                         \
            }                                                                                      \
                                                                                                   \
            /* try reading using user provided reader */                                           \
            StrIter si_before_read = si;                                                           \
            { reader }                                                                             \
                                                                                                   \
            /* if no advancement in read position */                                               \
            if (si_before_read.pos == si.pos) {                                                    \
                /* skip the value */                                                               \
                StrIter read_si = JSkipValue (si);                                                 \
                                                                                                   \
                /* if still no advancement in read position */                                     \
                if (read_si.pos == si.pos) {                                                       \
                    LOG_ERROR ("Failed to parse value. Invalid JSON.");                            \
                    StrDeinit (&key);                                                              \
                    failed = true;                                                                 \
                    si     = saved_si;                                                             \
                    break;                                                                         \
                }                                                                                  \
                si = read_si;                                                                      \
            }                                                                                      \
            si = JSkipWhitespace (si);                                                             \
                                                                                                   \
            /* expect a comma after a successful value read in array */                            \
            expect_comma = true;                                                                   \
        }                                                                                          \
                                                                                                   \
        /* end of array */                                                                         \
        if (!failed) {                                                                             \
            if (StrIterPeek (&si) != ']') {                                                        \
                LOG_ERROR ("Invalid end of array. Expected ']'.");                                 \
                failed = true;                                                                     \
                si     = saved_si;                                                                 \
                break;                                                                             \
            }                                                                                      \
                                                                                                   \
            StrIterNext (&si);                                                                     \
        }                                                                                          \
    } while (0)

///
/// Read a JSON object using a custom field reader expression.
///
/// The macro parses the object and invokes the provided code block for each key-value pair.
/// If the key is not recognized or parsing fails, the value is skipped.
///
/// si[in,out] : Stream iterator to read from.
/// reader     : Code block to handle each key-value pair. Can include JR_*_KV macros.
///
/// USAGE:
///   JR_OBJ(si, {
///       JR_STR_KV(si, "name", obj.name);
///       JR_INT_KV(si, "id", obj.id);
///   });
///
/// SUCCESS : Entire object read or skipped successfully
/// FAILURE : Logs error and restores `si` on structural or read failure
///
/// TAGS: JSON, Macro, Reader, Object
///
#define JR_OBJ(si, reader)                                                                         \
    do {                                                                                           \
        if (!StrIterRemainingLength (&si)) {                                                       \
            break;                                                                                 \
        }                                                                                          \
                                                                                                   \
        StrIter saved_si = si;                                                                     \
        si               = JSkipWhitespace (si);                                                   \
                                                                                                   \
        /* starting of an object */                                                                \
        if (StrIterPeek (&si) != '{') {                                                            \
            LOG_ERROR ("Invalid object start. Expected '{'.");                                     \
            si = saved_si;                                                                         \
            break;                                                                                 \
        }                                                                                          \
        StrIterNext (&si);                                                                         \
        si = JSkipWhitespace (si);                                                                 \
                                                                                                   \
        StrIter read_si;                                                                           \
        bool    expect_comma = false;                                                              \
        bool    failed       = false;                                                              \
                                                                                                   \
        /* while not at the end of object. */                                                      \
        while (!failed && StrIterPeek (&si) && StrIterPeek (&si) != '}') {                         \
            if (expect_comma) {                                                                    \
                if (StrIterPeek (&si) != ',') {                                                    \
                    LOG_ERROR (                                                                    \
                        "Expected ',' after key/value pairs in object. Invalid JSON object."       \
                    );                                                                             \
                    failed = true;                                                                 \
                    si     = saved_si;                                                             \
                    break;                                                                         \
                }                                                                                  \
                StrIterNext (&si); /* skip comma */                                                \
                si = JSkipWhitespace (si);                                                         \
            }                                                                                      \
                                                                                                   \
                                                                                                   \
            Str key = StrInit();                                                                   \
                                                                                                   \
            /* key start */                                                                        \
            read_si = JReadString (si, &key);                                                      \
            if (read_si.pos == si.pos) {                                                           \
                LOG_ERROR ("Failed to read string key in object. Invalid JSON");                   \
                StrDeinit (&key);                                                                  \
                failed = true;                                                                     \
                si     = saved_si;                                                                 \
                break;                                                                             \
            }                                                                                      \
                                                                                                   \
            si = read_si;                                                                          \
            si = JSkipWhitespace (si);                                                             \
                                                                                                   \
                                                                                                   \
            if (StrIterPeek (&si) != ':') {                                                        \
                LOG_ERROR ("Expected ':' after key string. Failed to read JSON");                  \
                StrDeinit (&key);                                                                  \
                failed = true;                                                                     \
                si     = saved_si;                                                                 \
                break;                                                                             \
            }                                                                                      \
            StrIterNext (&si);                                                                     \
            si = JSkipWhitespace (si);                                                             \
                                                                                                   \
                                                                                                   \
            /* try reading using user provided reader */                                           \
            StrIter si_before_read = si;                                                           \
            { reader }                                                                             \
                                                                                                   \
            /* if no advancement in read position */                                               \
            if (si_before_read.pos == si.pos) {                                                    \
                /* skip the value */                                                               \
                StrIter read_si = JSkipValue (si);                                                 \
                                                                                                   \
                                                                                                   \
                /* if still no advancement in read position */                                     \
                if (read_si.pos == si.pos) {                                                       \
                    LOG_ERROR ("Failed to parse value. Invalid JSON.");                            \
                    StrDeinit (&key);                                                              \
                    failed = true;                                                                 \
                    si     = saved_si;                                                             \
                    break;                                                                         \
                }                                                                                  \
                                                                                                   \
                LOG_INFO ("User skipped reading of '%s' field in JSON object.", key.data);         \
                si = read_si;                                                                      \
            }                                                                                      \
            StrDeinit (&key);                                                                      \
            si = JSkipWhitespace (si);                                                             \
                                                                                                   \
                                                                                                   \
            /* expect a comma after a successful key-value pair read */                            \
            expect_comma = true;                                                                   \
        }                                                                                          \
                                                                                                   \
        if (!failed) {                                                                             \
            if (StrIterPeek (&si) != '}') {                                                        \
                LOG_ERROR ("Expected end of object '}' but found '%c'", StrIterPeek (&si));        \
                failed = true;                                                                     \
                si     = saved_si;                                                                 \
                break;                                                                             \
            }                                                                                      \
                                                                                                   \
            StrIterNext (&si);                                                                     \
        }                                                                                          \
    } while (0)

///
/// Conditionally parse a JSON object if key matches expected name.
///
/// si[in,out] : Stream iterator to read from.
/// k[in]      : Expected key name (C-string).
/// reader     : Code block to handle key-value pairs in object.
///
/// USAGE:
///   JR_OBJ_KV(si, "config", {
///       JR_BOOL_KV(si, "debug", flags.debug_mode);
///   });
///
/// SUCCESS : Object parsed if key matched
/// FAILURE : No-op if key does not match
///
/// TAGS: JSON, Macro, Reader, Object, KeyValue
///
#define JR_OBJ_KV(si, k, reader)                                                                   \
    do {                                                                                           \
        if (!StrCmpZstr (&key, (k))) {                                                             \
            JR_OBJ (si, reader);                                                                   \
        }                                                                                          \
    } while (0)

///
/// Conditionally parse a JSON array if key matches expected name.
///
/// si[in,out] : Stream iterator to read from.
/// k[in]      : Expected key name (C-string).
/// reader     : Code block to handle array element parsing.
///
/// USAGE:
///   JR_ARR_KV(si, "list", {
///       JR_INT(si, val);
///       VecPush(&arr, val);
///   });
///
/// SUCCESS : Array parsed if key matched
/// FAILURE : No-op if key does not match
///
/// TAGS: JSON, Macro, Reader, Array, KeyValue
///
#define JR_ARR_KV(si, k, reader)                                                                   \
    do {                                                                                           \
        if (!StrCmpZstr (&key, (k))) {                                                             \
            JR_ARR (si, reader);                                                                   \
        }                                                                                          \
    } while (0)

///
/// Begin a JSON object and write key-value entries using JW_*_KV macros.
/// This macro must be used as a wrapper for other `JW_*_KV` macros to generate a JSON object.
/// Tracks whether commas are needed between entries using an internal flag.
///
/// writer[in] : A block of code containing `JW_*_KV` calls for populating the object.
///
/// USAGE:
///   JW_OBJ(json, {
///       JW_STR_KV(json, "name", obj.name);
///       JW_INT_KV(json, "ref", obj.ref);
///   });
///
/// SUCCESS : Appends a valid JSON object to `json`
/// FAILURE : Does not return on failure (relies on internal string operations)
///
/// TAGS: JSON, Macro, Writer, Object, Structure
///
#define JW_OBJ(j, writer)                                                                          \
    do {                                                                                           \
        bool ___is_first___ = true;                                                                \
        StrPushBack (&(j), '{');                                                                   \
        {writer};                                                                                  \
        StrPushBack (&(j), '}');                                                                   \
    } while (0)

///
/// Write a key and nested object inside an existing JSON object.
/// Should be called inside `JW_OBJ`. Adds commas appropriately based on insertion order.
///
/// j[in,out] : The target string to append to.
/// k[in]     : The key name to use in the JSON object.
/// writer[in]: A block of code using `JW_*_KV` to populate the inner object.
///
/// USAGE:
///   JW_OBJ_KV(json, "config", {
///       JW_INT_KV(json, "timeout", 30);
///   });
///
/// SUCCESS : Appends a nested JSON object under the given key
/// FAILURE : Does not return on failure
///
/// TAGS: JSON, Macro, Writer, Object, KeyValue, Nested
///
#define JW_OBJ_KV(j, k, writer)                                                                    \
    do {                                                                                           \
        if (___is_first___) {                                                                      \
            ___is_first___ = false;                                                                \
        } else {                                                                                   \
            StrPushBack (&(j), ',');                                                               \
        }                                                                                          \
        StrAppendf (&(j), "\"%s\":", k);                                                           \
        JW_OBJ (j, writer);                                                                        \
    } while (0)

///
/// Write a JSON array from a vector. Each item is rendered using the provided `writer`.
/// Handles inserting commas between elements.
///
/// j[in,out]  : The target string to append to.
/// arr[in]    : A vector to iterate over.
/// item[out]  : Iterator variable for the current item.
/// writer[in] : Code block that appends JSON for each `item`.
///
/// USAGE:
///   JW_ARR(json, some_vec, s, {
///       JW_STR(json, s);
///   });
///
/// SUCCESS : Appends a JSON array to `json`
/// FAILURE : Does not return on failure
///
/// TAGS: JSON, Macro, Writer, Array, Structure
///
#define JW_ARR(j, arr, item, writer)                                                               \
    do {                                                                                           \
        bool ___is_first___ = true;                                                                \
        StrPushBack (&(j), '[');                                                                   \
        VecForeach (&(arr), item, {                                                                \
            if (___is_first___) {                                                                  \
                ___is_first___ = false;                                                            \
            } else {                                                                               \
                StrPushBack (&(j), ',');                                                           \
            }                                                                                      \
            { writer }                                                                             \
        });                                                                                        \
        StrPushBack (&(j), ']');                                                                   \
    } while (0)

///
/// Write a key and an array value into a JSON object.
/// Intended for use within a `JW_OBJ`. Adds commas automatically.
///
/// j[in,out]  : The target string to append to.
/// k[in]      : Key name for the array.
/// arr[in]    : A vector to iterate over.
/// item[out]  : Iterator variable for the current item.
/// writer[in] : Code block that appends JSON for each `item`.
///
/// USAGE:
///   JW_ARR_KV(json, "tags", tag_vec, tag, {
///       JW_STR(json, tag);
///   });
///
/// SUCCESS : Appends a key-value array pair to `json`
/// FAILURE : Does not return on failure
///
/// TAGS: JSON, Macro, Writer, Array, KeyValue
///
#define JW_ARR_KV(j, k, arr, item, writer)                                                         \
    do {                                                                                           \
        if (___is_first___) {                                                                      \
            ___is_first___ = false;                                                                \
        } else {                                                                                   \
            StrPushBack (&(j), ',');                                                               \
        }                                                                                          \
        StrAppendf (&(j), "\"%s\":", k);                                                           \
        JW_ARR (j, arr, item, writer);                                                             \
    } while (0)

///
/// Append an integer value to a JSON string.
///
/// j[in,out] : The target string to append to.
/// i[in]     : Integer value.
///
/// USAGE:
///   JW_INT(json, 42);
///
/// SUCCESS : Appends a numeric value to `json`
/// FAILURE : Does not return on failure
///
/// TAGS: JSON, Macro, Writer, Integer, Numeric
///
#define JW_INT(j, i)                                                                               \
    do {                                                                                           \
        i64 my_int = (i);                                                                          \
        StrAppendf (&(j), "%lld", my_int);                                                         \
    } while (0)

///
/// Write a key and integer value to a JSON object.
///
/// j[in,out] : The target string to append to.
/// k[in]     : Key name.
/// i[in]     : Integer value.
///
/// USAGE:
///   JW_INT_KV(json, "count", 5);
///
/// SUCCESS : Appends a key-value integer pair
/// FAILURE : Does not return on failure
///
/// TAGS: JSON, Macro, Writer, Integer, KeyValue
///
#define JW_INT_KV(j, k, i)                                                                         \
    do {                                                                                           \
        if (___is_first___) {                                                                      \
            ___is_first___ = false;                                                                \
        } else {                                                                                   \
            StrPushBack (&(j), ',');                                                               \
        }                                                                                          \
        StrAppendf (&(j), "\"%s\":", k);                                                           \
        JW_INT (j, i);                                                                             \
    } while (0)

///
/// Append a floating-point value to a JSON string.
///
/// j[in,out] : The target string to append to.
/// f[in]     : Floating-point value.
///
/// USAGE:
///   JW_FLT(json, 3.14);
///
/// SUCCESS : Appends a float value to `json`
/// FAILURE : Does not return on failure
///
/// TAGS: JSON, Macro, Writer, Float, Numeric
///
#define JW_FLT(j, f)                                                                               \
    do {                                                                                           \
        f64 my_flt = (f);                                                                          \
        StrAppendf (&(j), "%f", my_flt);                                                           \
    } while (0)

///
/// Write a key and float value to a JSON object.
///
/// j[in,out] : The target string to append to.
/// k[in]     : Key name.
/// f[in]     : Floating-point value.
///
/// USAGE:
///   JW_FLT_KV(json, "pi", 3.14159);
///
/// SUCCESS : Appends a key-value float pair
/// FAILURE : Does not return on failure
///
/// TAGS: JSON, Macro, Writer, Float, KeyValue
///
#define JW_FLT_KV(j, k, f)                                                                         \
    do {                                                                                           \
        if (___is_first___) {                                                                      \
            ___is_first___ = false;                                                                \
        } else {                                                                                   \
            StrPushBack (&(j), ',');                                                               \
        }                                                                                          \
        StrAppendf (&(j), "\"%s\":", k);                                                           \
        JW_FLT (j, f);                                                                             \
    } while (0)

///
/// Append a string value (quoted) to the JSON.
///
/// j[in,out] : The target string to append to.
/// s[in]     : A `Str` object containing the string.
///
/// USAGE:
///   JW_STR(json, name);
///
/// SUCCESS : Appends a quoted string to `json`
/// FAILURE : Does not return on failure
///
/// TAGS: JSON, Macro, Writer, String
///
#define JW_STR(j, s)                                                                               \
    do {                                                                                           \
        StrAppendf (&(j), "\"%s\"", (s).data);                                                     \
    } while (0)

///
/// Write a key and string value into a JSON object.
///
/// j[in,out] : The target string to append to.
/// k[in]     : Key name.
/// s[in]     : A `Str` object containing the value.
///
/// USAGE:
///   JW_STR_KV(json, "username", user.name);
///
/// SUCCESS : Appends a key-value string pair
/// FAILURE : Does not return on failure
///
/// TAGS: JSON, Macro, Writer, String, KeyValue
///
#define JW_STR_KV(j, k, s)                                                                         \
    do {                                                                                           \
        if (___is_first___) {                                                                      \
            ___is_first___ = false;                                                                \
        } else {                                                                                   \
            StrPushBack (&(j), ',');                                                               \
        }                                                                                          \
        StrAppendf (&(j), "\"%s\":", k);                                                           \
        JW_STR (j, s);                                                                             \
    } while (0)

///
/// Append a string value (quoted) to the JSON.
///
/// j[in,out] : The target string to append to.
/// s[in]     : A `Str` object containing the string.
///
/// USAGE:
///   JW_STR(json, name);
///
/// SUCCESS : Appends a quoted string to `json`
/// FAILURE : Does not return on failure
///
/// TAGS: JSON, Macro, Writer, String
///
#define JW_ZSTR(j, s)                                                                              \
    do {                                                                                           \
        StrAppendf (&(j), "\"%s\"", (s));                                                          \
    } while (0)

///
/// Write a key and string value into a JSON object.
///
/// j[in,out] : The target string to append to.
/// k[in]     : Key name.
/// s[in]     : A `Str` object containing the value.
///
/// USAGE:
///   JW_STR_KV(json, "username", user.name);
///
/// SUCCESS : Appends a key-value string pair
/// FAILURE : Does not return on failure
///
/// TAGS: JSON, Macro, Writer, String, KeyValue
///
#define JW_ZSTR_KV(j, k, s)                                                                        \
    do {                                                                                           \
        if (___is_first___) {                                                                      \
            ___is_first___ = false;                                                                \
        } else {                                                                                   \
            StrPushBack (&(j), ',');                                                               \
        }                                                                                          \
        StrAppendf (&(j), "\"%s\":", k);                                                           \
        JW_ZSTR (j, s);                                                                            \
    } while (0)

///
/// Append a boolean value to the JSON as a string "true"/"false".
///
/// j[in,out] : The target string to append to.
/// b[in]     : Boolean value.
///
/// USAGE:
///   JW_BOOL(json, true);
///
/// SUCCESS : Appends "true" or "false" as a string
/// FAILURE : Does not return on failure
///
/// TAGS: JSON, Macro, Writer, Boolean
///
#define JW_BOOL(j, b)                                                                              \
    do {                                                                                           \
        StrAppendf (&(j), "\"%b\"", b);                                                            \
    } while (0)

///
/// Write a key and boolean value into a JSON object.
///
/// j[in,out] : The target string to append to.
/// k[in]     : Key name.
/// b[in]     : Boolean value.
///
/// USAGE:
///   JW_BOOL_KV(json, "is_active", user.active);
///
/// SUCCESS : Appends a key-value boolean pair
/// FAILURE : Does not return on failure
///
/// TAGS: JSON, Macro, Writer, Boolean, KeyValue
///
#define JW_BOOL_KV(j, k, b)                                                                        \
    do {                                                                                           \
        if (___is_first___) {                                                                      \
            ___is_first___ = false;                                                                \
        } else {                                                                                   \
            StrPushBack (&(j), ',');                                                               \
        }                                                                                          \
        StrAppendf (&(j), "\"%s\":", k);                                                           \
        JW_BOOL (j, b);                                                                            \
    } while (0)

#endif // REAI_UTIL_JSON_H
