#pragma once
#ifdef _WIN64
typedef unsigned __int64 size_t;
typedef __int64          ptrdiff_t;
typedef __int64          intptr_t;
#else
typedef unsigned int     size_t;
typedef int              ptrdiff_t;
typedef int              intptr_t;
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

enum class byte_t : unsigned char {};
typedef short int16;
typedef int   int32;
typedef long long int64;
typedef unsigned int uint;
typedef unsigned short u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;
typedef unsigned char u8;