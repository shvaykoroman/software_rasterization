#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define SWAP(a,b,t) (((a) > (b)) ? {t = a; a = b; b = t;})

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

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
MulM4x4WithV3(m4x4 Matrix, v3f Vector)
{
    v3f Result = v3f(0.0f,0.0f,0.0f);
    
    for(s32 i = 0; i < 3; i++)
    {
        for(s32 j = 0; j < 3; j++)
        {
            Result.e[i] += Matrix.e[i][j]*Vector.e[j];
        }
    }
    
    return Result;
}

v3f 
addv3f(v3f A, v3f B)
{
    v3f Result = v3f(0.0f,0.0f,0.0f);
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    
    return Result;
}