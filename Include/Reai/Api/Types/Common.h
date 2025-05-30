#ifndef REAI_API_TYPES_COMMON_H
#define REAI_API_TYPES_COMMON_H

#include <Reai/Util/Str.h>
#include <Reai/Util/Vec.h>

typedef Strs Tags;
typedef Strs Collections;

typedef u64 BinaryId;
typedef u64 AnalysisId;
typedef u64 FunctionId;
typedef u64 CollectionId;
typedef u64 ModelId;
typedef u64 TeamId;

typedef Vec (u64) IdsVec;

typedef IdsVec BinaryIds;
typedef IdsVec AnalysisIds;
typedef IdsVec FunctionIds;
typedef IdsVec CollectionIds;
typedef IdsVec ModelIds;
typedef IdsVec TeamIds;

#endif // REAI_API_TYPES_COMMON_H
