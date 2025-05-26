/**
 * @file AnnSymbol.h
 * @date 31st March 2025
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) RevEngAI. All Rights Reserved.
 * */


#ifndef REAI_ANN_SYMBOL_H
#define REAI_ANN_SYMBOL_H

#include <Reai/Api/Types/Common.h>
#include <Reai/Types.h>
#include <Reai/Util/Str.h>

typedef struct AnnSymbol {
    FunctionId source_function_id;
    FunctionId target_function_id;
    f64        distance;
    AnalysisId analysis_id;
    Str        analysis_name;
    Str        function_name;
    Str        function_mangled_name;
    BinaryId   binary_id;
    Str        sha256;
    bool       debug;
} AnnSymbol;

typedef Vec (AnnSymbol) AnnSymbols;

#ifdef __cplusplus
extern "C" {
#endif


    ///
    /// Deinit cloned AnnSymbol object. Provided pointer is not freed.
    /// That must be taken care of by the owner.
    ///
    /// fi[in,out] : Object to be destroyed.
    ///
    void AnnSymbolDeinit (AnnSymbol* clone);

    ///
    /// Clone a AnnSymbol object from `src` to `dst`
    ///
    /// dst[out] : Destination object.
    /// src[in]  : Source object.
    ///
    /// SUCCESS : True
    /// FAILURE : Does not return
    ///
    bool AnnSymbolInitClone (AnnSymbol* dst, AnnSymbol* src);

#ifdef __cplusplus
}
#endif

#endif // REAI_ANN_SYMBOL_H
