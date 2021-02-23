#include "stdlib.h"
#include "stdio.h"
#include "stdint.h"

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

#define internal static
#define persist  static
#define global   static


char*
ParseOneLine(char *Buffer, u32 Length, FILE *File)
{
    char *Result;
    for(;;)
    {
        if(!gets(Buffer, Length, File))
        {
            Result = 0;
        }
        else
        {
            for(u32 Length = 0;)
            {
                
            }
        }
    }
    return Result;
}

s32 
main(s32 argv, char *argv[])
{
    
    
    return 0;
}