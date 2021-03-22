#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define SWAP(a,b,t) (((a) > (b)) ? {t = a; a = b; b = t;})

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

#define PI 3.14159f

extern f32 gCosTable[361];
extern f32 gSinTable[361];

struct m4x4
{
    union
    {
        f32 e[4][4];
    };
};


v3f 
addv3f(v3f A, v3f B)
{
    v3f Result = v3f(0.0f,0.0f,0.0f);
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    
    return Result;
}

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

