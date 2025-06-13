#include <Reai/Diff.h>
#include <Reai/File.h>
#include <Reai/Log.h>
#include <Reai/Types.h>
#include <Reai/Util/Str.h>
#include <Reai/Util/Vec.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ANSI color codes for terminal output
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

// Function to read a file into a Str object
bool ReadFileToStr (const char* filename, Str* str) {
    char* data      = NULL;
    size  file_size = 0;
    size  capacity  = 0;

    if (!ReadCompleteFile (filename, &data, &file_size, &capacity)) {
        fprintf (stderr, "Error: Failed to read file '%s'\n", filename);
        return false;
    }

    // Initialize Str with the file content
    *str = StrInitFromCstr (data, file_size);
    // Free the original data since StrInitFromCstr makes a copy
    free (data);

    return true;
}

// Function to print a line with proper formatting and type info
void PrintLine (
    const char* prefix,
    const char* color,
    u64         line_num,
    const Str*  content,
    const char* type_info
) {
    if (type_info && strlen (type_info) > 0) {
        printf (
            "%s%s%3llu: %.*s%s %s[%s]%s\n",
            color,
            prefix,
            line_num + 1,
            (int)content->length,
            content->data,
            RESET,
            BLUE,
            type_info,
            RESET
        );
    } else {
        printf (
            "%s%s%3llu: %.*s%s\n",
            color,
            prefix,
            line_num + 1,
            (int)content->length,
            content->data,
            RESET
        );
    }
}

// Function to print diff results with colors
void PrintDiff (const char* file1, const char* file2, const DiffLines* diff) {
    printf ("%s--- %s%s\n", BOLD, file1, RESET);
    printf ("%s+++ %s%s\n", BOLD, file2, RESET);

    if (diff->length == 0) {
        printf ("%sFiles are identical%s\n", GREEN, RESET);
        return;
    }

    VecForeachPtr (diff, line, {
        switch (line->type) {
            case DIFF_TYPE_SAM : {
                PrintLine (" ", "", line->sam.line, &line->sam.content, "");
                break;
            }

            case DIFF_TYPE_ADD : {
                PrintLine ("+", GREEN, line->add.line, &line->add.content, "ADDED");
                break;
            }

            case DIFF_TYPE_REM : {
                PrintLine ("-", RED, line->rem.line, &line->rem.content, "REMOVED");
                break;
            }

            case DIFF_TYPE_MOD : {
                char mod_info[64];
                snprintf (
                    mod_info,
                    sizeof (mod_info),
                    "MODIFIED: was line %llu",
                    line->mod.old_line + 1
                );
                PrintLine ("-", RED, line->mod.old_line, &line->mod.old_content, "MODIFIED: old");
                PrintLine ("+", GREEN, line->mod.new_line, &line->mod.new_content, mod_info);
                break;
            }

            case DIFF_TYPE_MOV : {
                // Show moved lines with yellow color and clear info
                printf (
                    "%s~ %3llu: %.*s%s %s[MOVED: from line %llu]%s\n",
                    YELLOW,
                    line->mov.new_line + 1,
                    (int)line->mov.new_content.length,
                    line->mov.new_content.data,
                    RESET,
                    BLUE,
                    line->mov.old_line + 1,
                    RESET
                );
                break;
            }

            default : {
                fprintf (stderr, "Warning: Unknown diff type '%c'\n", line->type);
                break;
            }
        }
    });
}

// Function to print usage information
void PrintUsage (const char* program_name) {
    printf ("Usage: %s <file1> <file2>\n", program_name);
    printf ("\nCompare two files line by line and show differences.\n");
    printf ("\nColor codes:\n");
    printf ("  %s- Red:%s    Lines removed from file1\n", RED, RESET);
    printf ("  %s+ Green:%s  Lines added in file2\n", GREEN, RESET);
    printf ("  %s~ Yellow:%s Lines moved between files\n", YELLOW, RESET);
    printf ("    Normal:  Lines that are the same\n");
    printf ("\nModifications are shown as a removal followed by an addition.\n");
}

int main (int argc, char* argv[]) {
    // Check command line arguments
    if (argc != 3) {
        PrintUsage (argv[0]);
        return 1;
    }

    const char* file1 = argv[1];
    const char* file2 = argv[2];

    // Initialize strings for file contents
    Str  str1, str2;
    bool success = true;

    // Read first file
    if (!ReadFileToStr (file1, &str1)) {
        return 1;
    }

    // Read second file
    if (!ReadFileToStr (file2, &str2)) {
        StrDeinit (&str1);
        return 1;
    }

    // Get the diff
    DiffLines diff = GetDiff (&str1, &str2);

    // Print the diff results
    PrintDiff (file1, file2, &diff);

    // Count different types of changes for summary
    size same_count = 0, add_count = 0, rem_count = 0, mod_count = 0, mov_count = 0;
    VecForeachPtr (&diff, line, {
        switch (line->type) {
            case DIFF_TYPE_SAM :
                same_count++;
                break;
            case DIFF_TYPE_ADD :
                add_count++;
                break;
            case DIFF_TYPE_REM :
                rem_count++;
                break;
            case DIFF_TYPE_MOD :
                mod_count++;
                break;
            case DIFF_TYPE_MOV :
                mov_count++;
                break;
        }
    });

    // Print summary
    printf ("\n%sSummary:%s\n", BOLD, RESET);
    printf ("  Same lines:     %zu\n", same_count);
    printf ("  Added lines:    %s%zu%s\n", GREEN, add_count, RESET);
    printf ("  Removed lines:  %s%zu%s\n", RED, rem_count, RESET);
    printf ("  Modified lines: %s%zu%s\n", YELLOW, mod_count, RESET);
    printf ("  Moved lines:    %s%zu%s\n", CYAN, mov_count, RESET);
    printf ("  Total changes:  %zu\n", add_count + rem_count + mod_count + mov_count);

    // Cleanup
    VecDeinit (&diff);
    StrDeinit (&str1);
    StrDeinit (&str2);

    // Return non-zero if files differ
    return (add_count + rem_count + mod_count + mov_count) > 0 ? 1 : 0;
}