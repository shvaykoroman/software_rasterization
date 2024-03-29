#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define SWAP(a,b,t) (((a) > (b)) ? {t = a; a = b; b = t;})

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

#define PI 3.14159f

struct m4x4
{
    union
    {
        f32 e[4][4];
    };
};


inline v2
operator+(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    
    return Result;
}

inline v2
operator-(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    
    return Result;
}


inline v3
operator+(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    
    return Result;
}

inline v3
operator-(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z + B.z;
    
    return Result;
}


inline v3
operator*(f32 A, v3 B)
{
    v3 Result;
    
    Result.x = A * B.x;
    Result.y = A * B.y;
    Result.z = A * B.z;
    
    return Result;
}


inline v2
operator*(f32 A, v2 B)
{
    v2 Result;
    
    Result.x = A * B.x;
    Result.y = A * B.y;
    
    return Result;
}


inline f32
Square(f32 X)
{
    f32 Result = X*X;
    return Result;
}

inline v3 
Normalize(v3 A)
{
    v3 Result;
    
    f32 Length = sqrtf(A.x*A.x + A.y*A.y+ A.z*A.z);
    
    Result.x = A.x / Length;
    Result.y = A.y / Length;
    Result.z = A.z / Length;
    
    return Result;
}

inline v3 
CrossProduct(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.y*B.z - A.z * B.y;
    Result.y = A.z*B.x - A.x * B.z;
    Result.z = A.x*B.y - A.y * B.x;
    
    return Result;
}

inline f32
DotProduct(v3 A, v3 B)
{
    f32 Result = 0;
    
    Result = A.x*B.x+A.y*B.y+A.z*B.z;
    
    return Result;
}

inline v2
LerpV2(v2 Source0,v2 Source1,f32 Alpha)
{
    v2 Result;
    
    Result = (1.0f - Alpha)*Source0 + Alpha*Source1;
    
    return Result;
}


inline v3
LerpV3(v3 Source0,v3 Source1,f32 Alpha)
{
    v3 Result;
    
    Result = (1.0f - Alpha)*Source0 + Alpha*Source1;
    
    return Result;
}
