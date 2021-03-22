/* date = January 19th 2021 11:17 pm */

#ifndef GAME_H
#define GAME_H

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

#define assert(D) if(!(D)) {*(u32*)0 = 0;}

#define CountOf(X) (sizeof(X) / sizeof(X[0]))

typedef struct v2
{
    union
    {
        f32 e[2];
        struct
        {
            f32 x,y;
        };
    };
}v2;


typedef struct v3f
{
    union
    {
        f32 e[3];
        struct
        {
            f32 x,y,z;
        };
    };
}v3;



typedef struct v4f
{
    union
    {
        f32 e[4];
        struct
        {
            f32 x,y,z,w;
        };
        struct
        {
            v3 xyz;
            f32 Ignored;
        };
        struct
        {
            f32 Ignored;
            v3 yzw;
        };
    };
}v4;


#define v4f(x,y,z,w) v4f_create(x,y,z,w)

inline v4
v4f_create(f32 x,f32 y, f32 z, f32 w)
{
    v4 Result = {};
    Result = {x,y,z,w};
    return Result;
}



#define v3f(x,y,z) v3f_create(x,y,z)

inline v3
v3f_create(f32 x,f32 y, f32 z)
{
    v3 Result = {};
    Result = {x,y,z};
    return Result;
}


#define v2f(x,y) v2f_create(x,y)

inline v2
v2f_create(f32 x,f32 y)
{
    v2 Result = {};
    Result = {x,y};
    return Result;
}

struct backbuffer
{
    BITMAPINFO BitmapInfo;
    s32 Width;
    s32 Height;
    s32 Stride;
    
    void *Memory;
};

struct game_backbuffer
{
    s32 Width;
    s32 Height;
    s32 Stride;
    
    void *Memory;
};

struct window_dim
{
    s32 Width;
    s32 Height;
};

struct button_state
{
    bool IsDown;
    bool WasDown;
};

struct buttons
{
    union
    {
        button_state Buttons[4];
        
        struct
        {
            button_state ButtonUp;
            button_state ButtonDown;
            button_state ButtonLeft;
            button_state ButtonRight;
        };
    };
};

struct controller
{
    buttons Controller[2];
};

#endif //GAME_H
