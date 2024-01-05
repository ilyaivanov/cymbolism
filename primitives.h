#ifndef PRMITIVES_H
#define PRMITIVES_H

#include <stdint.h>


#define ArrayLength(array) (sizeof(array) / sizeof(array[0]))
#define MyAssert(cond) if (!(cond)) { *(u32*)0 = 0; }
#define InvalidCodePath MyAssert(!"InvalidCodePath")

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;


#endif