#ifndef TYPES_H_
#define TYPES_H_

#include <sys/types.h>

#define nil ((void*)0)

#define KILOBYTE 1024llu
#define MEGABYTE KILOBYTE * 1024llu
#define GIGABYTE MEGABYTE * 1024llu

typedef signed char i8;
typedef signed short int i16;
typedef signed int i32;
typedef signed long long int i64;

typedef unsigned char byte;
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long int u64;

#if __STDC_VERSION >= 1999001L
#include <stdbool.h>
#else
typedef unsigned char bool;
#define true 1
#define false 0
#endif

#endif /* TYPES_H_ */
