#include <Reai/Api/Types/AiDecompilation.h>
#include <Reai/Log.h>

void AiDecompilationDeinit (AiDecompilation* clone) {
    if (!clone) {
        LOG_FATAL ("Invalid argument");
    }

    StrDeinit (&clone->decompilation);
    StrDeinit (&clone->raw_decompilation);
    StrDeinit (&clone->ai_summary);
    StrDeinit (&clone->raw_ai_summary);
    VecDeinit (&clone->functions);
    VecDeinit (&clone->strings);
    VecDeinit (&clone->unmatched.strings);
    VecDeinit (&clone->unmatched.functions);
    VecDeinit (&clone->unmatched.vars);
    VecDeinit (&clone->unmatched.external_vars);
    VecDeinit (&clone->unmatched.custom_types);
    VecDeinit (&clone->unmatched.go_to_labels);
    VecDeinit (&clone->unmatched.custom_function_pointers);
    VecDeinit (&clone->unmatched.variadic_lists);

    memset (clone, 0, sizeof (AiDecompilation));
}

bool AiDecompilationInitClone (AiDecompilation* dst, AiDecompilation* src) {
    if (!dst || !src) {
        LOG_FATAL ("Invalid argument");
    }

    dst->decompilation     = StrInit();
    dst->raw_decompilation = StrInit();
    dst->ai_summary        = StrInit();
    dst->raw_ai_summary    = StrInit();
    dst->functions = VecInitWithDeepCopy_T (&dst->functions, SymbolInfoInitClone, SymbolInfoDeinit);
    dst->strings   = VecInitWithDeepCopy_T (&dst->strings, SymbolInfoInitClone, SymbolInfoDeinit);
    dst->unmatched.strings =
        VecInitWithDeepCopy_T (&dst->unmatched.strings, SymbolInfoInitClone, SymbolInfoDeinit);
    dst->unmatched.functions =
        VecInitWithDeepCopy_T (&dst->unmatched.functions, SymbolInfoInitClone, SymbolInfoDeinit);
    dst->unmatched.vars =
        VecInitWithDeepCopy_T (&dst->unmatched.vars, SymbolInfoInitClone, SymbolInfoDeinit);
    dst->unmatched.external_vars = VecInitWithDeepCopy_T (
        &dst->unmatched.external_vars,
        SymbolInfoInitClone,
        SymbolInfoDeinit
    );
    dst->unmatched.custom_types =
        VecInitWithDeepCopy_T (&dst->unmatched.custom_types, SymbolInfoInitClone, SymbolInfoDeinit);
    dst->unmatched.go_to_labels =
        VecInitWithDeepCopy_T (&dst->unmatched.go_to_labels, SymbolInfoInitClone, SymbolInfoDeinit);
    dst->unmatched.custom_function_pointers = VecInitWithDeepCopy_T (
        &dst->unmatched.custom_function_pointers,
        SymbolInfoInitClone,
        SymbolInfoDeinit
    );

    StrInitCopy (&dst->decompilation, &src->decompilation);
    StrInitCopy (&dst->raw_decompilation, &src->raw_decompilation);
    StrInitCopy (&dst->ai_summary, &src->ai_summary);
    StrInitCopy (&dst->raw_ai_summary, &src->raw_ai_summary);
    VecInitClone (&dst->strings, &src->strings);
    VecInitClone (&dst->functions, &src->functions);
    VecInitClone (&dst->unmatched.strings, &src->unmatched.strings);
    VecInitClone (&dst->unmatched.functions, &src->unmatched.functions);
    VecInitClone (&dst->unmatched.vars, &src->unmatched.vars);
    VecInitClone (&dst->unmatched.external_vars, &src->unmatched.external_vars);
    VecInitClone (&dst->unmatched.custom_types, &src->unmatched.custom_types);
    VecInitClone (&dst->unmatched.go_to_labels, &src->unmatched.go_to_labels);
    VecInitClone (
        &dst->unmatched.custom_function_pointers,
        &src->unmatched.custom_function_pointers
    );
    VecInitClone (&dst->unmatched.variadic_lists, &src->unmatched.variadic_lists);

    return true;
}
