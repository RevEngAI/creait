/**
 * @file Types.h
 * @date Mon, 8th January 2024
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) 2024 RevEngAI. All Rights Reserved.
 * */

#ifndef REAI_TYPE_H
#define REAI_TYPE_H

// clang-format off

typedef unsigned long long int Uint64;
typedef unsigned int           Uint32;
typedef unsigned short int     Uint16;
typedef unsigned char          Uint8; 
typedef unsigned long int      Size;  

typedef signed long long int Int64;
typedef signed int           Int32;
typedef signed short int     Int16;
typedef signed char          Int8;

typedef char Char;

typedef float       Float32;
typedef double      Float64;
typedef long double Fload80;

typedef unsigned char Bool;

typedef const Char *CString;

typedef Uint64 ReaiBinaryId;
typedef Uint64 ReaiAnalysisId;
typedef Uint64 ReaiFunctionId;
typedef Uint64 ReaiCollectionId;
typedef Uint64 ReaiTeamId;

// clang-format on

#include <stdbool.h>

#ifndef SIZE_MAX
#    define SIZE_MAX ((Size)1 << (sizeof (Size) * 8 - 1))
#endif

#ifndef UINT32_MAX
#    define UINT32_MAX ((Uint32)1 << (sizeof (Uint32) * 8 - 1))
#endif

#endif // REAI_TYPE_H
