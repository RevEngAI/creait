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

// clang-format on

#define True  ((Bool)1)
#define False ((Bool)0)
#define Null  ((void *)0)

#ifndef SIZE_MAX
#    define SIZE_MAX ((Size)1 << (sizeof (Size) * 8 - 1))
#endif

#endif // REAI_TYPE_H
