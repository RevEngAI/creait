#ifndef REAI_API_TYPES_COMMON_H
#define REAI_API_TYPES_COMMON_H

#include <Reai/Util/Str.h>
#include <Reai/Util/Vec.h>

typedef Vec (Str) Tags;
typedef Vec (Str) Collections;

typedef u64 BinaryId;
typedef u64 AnalysisId;
typedef u64 FunctionId;
typedef u64 CollectionId;
typedef u64 ModelId;
typedef u64 TeamId;

typedef Vec (BinaryId) BinaryIds;
typedef Vec (AnalysisId) AnalysisIds;
typedef Vec (FunctionId) FunctionIds;
typedef Vec (CollectionId) CollectionIds;
typedef Vec (ModelId) ModelIds;
typedef Vec (TeamId) TeamIds;

#endif // REAI_API_TYPES_COMMON_H
