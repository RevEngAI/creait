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
typedef u64 CommentId;
typedef u64 UserId;
typedef u64 ResourceId;

typedef Vec (u64) IdsVec;

typedef IdsVec BinaryIds;
typedef IdsVec AnalysisIds;
typedef IdsVec FunctionIds;
typedef IdsVec CollectionIds;
typedef IdsVec ModelIds;
typedef IdsVec TeamIds;
typedef IdsVec CommentIds;
typedef IdsVec UserIds;
typedef IdsVec ResourceIds;

#endif // REAI_API_TYPES_COMMON_H
