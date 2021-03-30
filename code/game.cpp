#define BBP 4
#include "math.cpp"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define Radians(X) (X * PI / 180.0f)
#define CENTER_OF_PROJECTION 0.5f*v3f((f32)WINDOW_WIDTH,(f32)WINDOW_HEIGHT,0.0f)
#define MAX_TRIANGLES 9064

#define CLIPCODE_X_GREATER (1 << 0) 
#define CLIPCODE_X_LOWER   (1 << 1)
#define CLIPCODE_X_INSIDE  (1 << 2)

#define CLIPCODE_Y_GREATER (1 << 3)
#define CLIPCODE_Y_LOWER   (1 << 4)
#define CLIPCODE_Y_INSIDE  (1 << 5)

#define CLIPCODE_Z_GREATER (1 << 6)
#define CLIPCODE_Z_LOWER   (1 << 7)
#define CLIPCODE_Z_INSIDE  (1 << 8)

enum
{
    Clipping_CheckX = 0,
    Clipping_CheckY,
    Clipping_CheckZ,
};

struct triangle
{
    v3 Vertex[3];
    u32 Color;
};

global u32 gTrianglesCount;
global triangle gTriangles[MAX_TRIANGLES];

bool
IsBitSet(u32 Test,u32 Offset)
{
    bool Result = (((Test >> Offset) & 1) == 1);
    return Result;
}

bool
IsTrivialReject(u32 *ClipCode, u32 Dim)
{
    
    if(Dim == 0)
    {
        if((IsBitSet(ClipCode[0],0) &&
            IsBitSet(ClipCode[1],0) && 
            IsBitSet(ClipCode[2],0)) ||
           
           (IsBitSet(ClipCode[0],1) &&
            IsBitSet(ClipCode[1],1) &&
            IsBitSet(ClipCode[2],1)))
        {
            return true;
        }
        
    }
    else if(Dim == 1)
    {
        if((IsBitSet(ClipCode[0],3)  &&
            IsBitSet(ClipCode[1],3)  && 
            IsBitSet(ClipCode[2],3)) ||
           
           (IsBitSet(ClipCode[0],4) &&
            IsBitSet(ClipCode[1],4) &&
            IsBitSet(ClipCode[2],4)))
        {
            return true;
        }
    }
    else if(Dim == 2)
    {
        if((IsBitSet(ClipCode[0],6)  &&
            IsBitSet(ClipCode[1],6)  && 
            IsBitSet(ClipCode[2],6)) ||
           
           (IsBitSet(ClipCode[0],7) &&
            IsBitSet(ClipCode[1],7) &&
            IsBitSet(ClipCode[2],7)))
        {
            return true;
        }
    }
    else
    {
        assert(!"Unexpected Dim!");
    }
    return false;
}

void
TransformIntoClipSpace(f32 FOV,f32 n,f32 f,f32 AspectRatio, 
                       v3 RawV0, v3 RawV1, v3 RawV2,
                       v4 *ClipSpaceV0Out, v4 *ClipSpaceV1Out, v4 *ClipSpaceV2Out)
{
    
    f32 TanHalfFov = tanf(Radians(FOV * 0.5f));
    f32 XScale = 1.0f / (TanHalfFov * AspectRatio);
    f32 YScale = 1.0f / TanHalfFov;
    f32 a = f/(f-n);
    f32 b = -2*n*f / (f - n);
    
    ClipSpaceV0Out->x = RawV0.x * XScale;
    ClipSpaceV0Out->y = RawV0.y * YScale;
    ClipSpaceV0Out->z = RawV0.z * a + b;
    ClipSpaceV0Out->w = RawV0.z;
    
    ClipSpaceV1Out->x = RawV1.x * XScale;
    ClipSpaceV1Out->y = RawV1.y * YScale;
    ClipSpaceV1Out->z = RawV1.z * a + b;
    ClipSpaceV1Out->w = RawV1.z;
    
    ClipSpaceV2Out->x = RawV2.x * XScale;
    ClipSpaceV2Out->y = RawV2.y * YScale;
    ClipSpaceV2Out->z = RawV2.z * a + b;
    ClipSpaceV2Out->w = RawV2.z;
    
}

void
TransformHomogeneousToNDC(v4 HomoV0, v4 HomoV1,v4 HomoV2,
                          v3 *NdcV0Out,v3 *NdcV1Out,v3 *NdcV2Out)
{
    NdcV0Out->x = HomoV0.x / HomoV0.w;
    NdcV0Out->y = HomoV0.y / HomoV0.w;
    NdcV0Out->z = HomoV0.z / HomoV0.w;
    
    NdcV1Out->x = HomoV1.x / HomoV1.w;
    NdcV1Out->y = HomoV1.y / HomoV1.w;
    NdcV1Out->z = HomoV1.z / HomoV1.w;
    
    NdcV2Out->x = HomoV2.x / HomoV2.w;
    NdcV2Out->y = HomoV2.y / HomoV2.w;
    NdcV2Out->z = HomoV2.z / HomoV2.w;
}

void
Viewport(v3 NdcV0,v3 NdcV1,v3 NdcV2,
         v3 *WinPV0Out,v3 *WinPV1Out,v3 *WinPV2Out,
         f32 ClipV0Z,f32 ClipV1Z,f32 ClipV2Z)
{
    
    WinPV0Out->x = (NdcV0.x + 1.0f)*WINDOW_WIDTH*0.5f;
    WinPV0Out->y = (NdcV0.y + 1.0f)*WINDOW_HEIGHT*0.5f;
    WinPV0Out->z = ClipV0Z;
    
    WinPV1Out->x = (NdcV1.x + 1.0f)*WINDOW_WIDTH*0.5f;
    WinPV1Out->y = (NdcV1.y + 1.0f)*WINDOW_HEIGHT*0.5f;
    WinPV1Out->z = ClipV1Z;
    
    WinPV2Out->x = (NdcV2.x + 1.0f)*WINDOW_WIDTH*0.5f;
    WinPV2Out->y = (NdcV2.y + 1.0f)*WINDOW_HEIGHT*0.5f;
    WinPV2Out->z = ClipV2Z;
    
}

void
MoveTriangle(u32 TriangleIndex, f32 X,f32 Y,f32 Z)
{
    triangle *Triangle = &gTriangles[TriangleIndex];
    
    Triangle->Vertex[0] = Triangle->Vertex[0]+v3f(X,Y,Z);
    Triangle->Vertex[1] = Triangle->Vertex[1]+v3f(X,Y,Z);
    Triangle->Vertex[2] = Triangle->Vertex[2]+v3f(X,Y,Z);
}


internal void
AddTriangle(v3 V0, v3 V1, v3 V2, u32 Color)
{
    //assert(gTrianglesCount < MAX_TRIANGLES);
    triangle *Triangle = &gTriangles[gTrianglesCount++];
    
    Triangle->Vertex[0] = V0;
    Triangle->Vertex[1] = V1;
    Triangle->Vertex[2] = V2;
    Triangle->Color = Color;
}

f32
Lerp(f32 A,f32 B, f32 Alpha)
{
    f32 Result = 0;
    
    Result = (1.0f - Alpha)*A+Alpha*B;
    
    return Result;
}

void
ClearBackbuffer(game_backbuffer *Backbuffer)
{
    memset(Backbuffer->Memory,0, Backbuffer->Width * Backbuffer->Height * 4);
}


v3
RotateAroundX(f32 Angle, v3 P)
{
    v3 Result;
    
    f32 CosAngle = cosf(Angle);
    f32 SinAngle = sinf(Angle);
    
    Result.x = P.x;
    Result.y = P.y * CosAngle - P.z * SinAngle;
    Result.z = P.y * SinAngle + P.z * CosAngle;
    
    return Result;
}


v3
RotateAroundY(f32 Angle, v3 P)
{
    v3 Result;
    
    f32 CosAngle = cosf(Angle);
    f32 SinAngle = sinf(Angle);
    
    Result.x = P.x * CosAngle + P.z * SinAngle;
    Result.z = -P.x * SinAngle + P.z * CosAngle;
    
    return Result;
}


v3
RotateAroundZ(f32 Angle, v3 P)
{
    v3 Result;
    
    f32 CosAngle = cosf(Angle);
    f32 SinAngle = sinf(Angle);
    
    Result.x = P.x * CosAngle - P.y * SinAngle;
    Result.y = P.x * SinAngle + P.y * CosAngle;
    Result.z = P.z;
    
    return Result;
}



v3
RotateZAroundPoint(f32 Angle, v3 P, v3 PToRotateAround)
{
    return RotateAroundZ(Angle, P - PToRotateAround) + PToRotateAround;
}


v3
RotateXAroundPoint(f32 Angle, v3 P, v3 PToRotateAround)
{
    return RotateAroundX(Angle, P - PToRotateAround) + PToRotateAround;
}


v3
RotateYAroundPoint(f32 Angle, v3 P, v3 PToRotateAround)
{
    return RotateAroundY(Angle, P - PToRotateAround) + PToRotateAround;
}


void
PlotPixel(game_backbuffer *Backbuffer, u32 X, u32 Y, u32 Color)
{
    u32 *Pixel = (u32*)((u8*)(Backbuffer->Memory) + X * BBP + Y * Backbuffer->Stride); 
    *Pixel = Color;
}

void
DrawFlatTopTriangle(game_backbuffer *Backbuffer,v3 Vertex0, v3 Vertex1, v3 Vertex2, u32 Color)
{
    // NOTE(shvayko): change in x per y
    f32 SlopeLeftSide = (Vertex1.x - Vertex2.x) / (Vertex1.y - Vertex2.y);
    f32 SlopeRightSide = (Vertex1.x-Vertex0.x) / (Vertex1.y - Vertex0.y);
    
    // NOTE(shvayko): Top left rasterization rule
    s32 yStart = (s32)(ceil(Vertex0.y-0.5f));
    s32 yEnd = (s32)(ceil(Vertex1.y-0.5f));
    
    
    if(yStart < 0)
    {
        yStart = 0;
    }
    if(yEnd > WINDOW_HEIGHT)
    {
        yEnd = WINDOW_HEIGHT;
    }
    
    for(s32 YIndex = yStart; 
        YIndex < yEnd;
        YIndex++)
    {
        
        f32 X0 = (SlopeLeftSide  * (YIndex  + 0.5f - Vertex2.y)) + Vertex2.x;
        f32 X1 = (SlopeRightSide * (YIndex  + 0.5f - Vertex2.y)) + Vertex0.x;
        
        s32 xStart = (s32)ceil(X0 - 0.5f);
        s32 xEnd   = (s32)ceil(X1 - 0.5f);
        
        if(xStart < 0)
        {
            xStart = 0;
        }
        if(xStart > WINDOW_WIDTH)
        {
            xStart = WINDOW_WIDTH;
        }
        
        if(xEnd < 0)
        {
            xEnd = 0;
        }
        if(xEnd > WINDOW_WIDTH)
        {
            xEnd = WINDOW_WIDTH;
        }
        
        
        
        for(s32 XIndex = xStart;
            XIndex < xEnd;
            XIndex++)
        {
            PlotPixel(Backbuffer, XIndex, YIndex, Color);
        }
    }
    
}

void
DrawFlatBottomTriangle(game_backbuffer *Backbuffer,v3 Vertex0, v3 Vertex1, v3 Vertex2, u32 Color)
{
    // NOTE(shvayko): change in x per y
    f32 SlopeLeftSide = (Vertex2.x  - Vertex0.x) / (Vertex1.y - Vertex0.y);
    f32 SlopeRightSide = (Vertex1.x - Vertex0.x) / (Vertex2.y - Vertex0.y);
    
    // NOTE(shvayko): Top left rasterization rule
    s32 yStart = (s32)(ceil(Vertex0.y-0.5f));
    s32 yEnd = (s32)(ceil(Vertex1.y-0.5f));
    
    if(yStart < 0)
    {
        yStart = 0;
    }
    
    if(yEnd > WINDOW_HEIGHT)
    {
        yEnd = WINDOW_HEIGHT;
    }
    
    for(s32 YIndex = yStart; 
        YIndex < yEnd;
        YIndex++)
    {
        
        f32 X0 = (SlopeLeftSide  * ((f32)YIndex + 0.5f - Vertex2.y)) + Vertex2.x;
        f32 X1 = (SlopeRightSide * (YIndex + 0.5f - Vertex2.y)) + Vertex1.x;
        
        s32 xStart = (s32)ceil(X0 - 0.5f);
        s32 xEnd   = (s32)ceil(X1 - 0.5f);
        
        if(xStart < 0)
        {
            xStart = 0;
        }
        if(xStart > WINDOW_WIDTH)
        {
            xStart = WINDOW_WIDTH;
        }
        
        if(xEnd < 0)
        {
            xEnd = 0;
        }
        if(xEnd > WINDOW_WIDTH)
        {
            xEnd = WINDOW_WIDTH;
        }
        
        for(s32 XIndex = xStart;
            XIndex < xEnd;
            XIndex++)
        {
            PlotPixel(Backbuffer, XIndex, YIndex, Color);
        }
    }
    
}

void
DrawTriangle(game_backbuffer *Backbuffer, v3 Vertex0, v3 Vertex1, v3 Vertex2, u32 Color)
{
    // NOTE(shvayko): Sort vertices from smallest to the biggest
    // V0 Smallest Y - V2 Biggest Y. Top-down
    
    if(Vertex0.y > Vertex1.y)
    {
        v3 Temp = Vertex0;
        Vertex0 = Vertex1;
        Vertex1 = Temp;
    } 
    // V1 > V0
    if(Vertex1.y > Vertex2.y)
    {
        v3 Temp = Vertex2;
        Vertex2 = Vertex1;
        Vertex1 = Temp;
    }
    // V2 > V1
    if(Vertex0.y > Vertex1.y)
    {
        v3 Temp = Vertex0;
        Vertex0 = Vertex1;
        Vertex1 = Temp;
    }
    
    if(Vertex0.y == Vertex1.y) // NOTE(shvayko): Flat top triangle
    {
        
        if(Vertex0.x < Vertex1.x)
        {
            v3 Temp = Vertex0;
            Vertex0 = Vertex1;
            Vertex1 = Temp;
        }
        
        DrawFlatTopTriangle(Backbuffer,Vertex0,Vertex2,Vertex1,Color);
    }
    else if(Vertex1.y == Vertex2.y)  // NOTE(shvayko): Flat bottom triangle
    {
        
        if(Vertex2.x > Vertex1.x)
        {
            v3 Temp = Vertex2;
            Vertex2 = Vertex1;
            Vertex1 = Temp;
        }
        
        
        DrawFlatBottomTriangle(Backbuffer, Vertex0,Vertex1,Vertex2, Color);
    }
    else  // NOTE(shvayko): General triangle
    {
        // NOTE(shvayko):General triangle will split up into 2 triangles: Flat bottom and flat top.
        
        f32 Alpha  = (Vertex1.y - Vertex0.y) / (Vertex2.y - Vertex0.y);
        //f32 V = (1.0f - Alpha)*Vertex0.x + Vertex2.x*Alpha;
        
        // TODO(shvayko):Replace that with Lerp function!
        f32 VX = Vertex0.x + (Vertex2.x - Vertex0.x)*Alpha;
        f32 VY = Vertex0.y + (Vertex2.y - Vertex0.y)*Alpha;
        
        v3 Vertex = v3f(VX,VY,0.0f); 
        // NOTE(shvayko): Decide what major the triangle is
        
        if(VX < Vertex1.x) // NOTE(shvayko): Left major
        {
            DrawFlatBottomTriangle(Backbuffer,Vertex0, Vertex1, Vertex, Color);
            
            DrawFlatTopTriangle(Backbuffer, Vertex1, Vertex2, Vertex, Color);
        }
        else // NOTE(shvayko): Right major
        {
            DrawFlatBottomTriangle(Backbuffer,Vertex0, Vertex, Vertex1, Color);
            
            DrawFlatTopTriangle(Backbuffer, Vertex, Vertex2, Vertex1, Color);
        }
    }
}

global bool gIsInit = false;

void
GameUpdateAndRender(game_backbuffer *Backbuffer, controller *Input)
{
    if(!gIsInit)
    {
        gIsInit = true;
    }
    
    gTrianglesCount = 0;
#if 1
    v3 FrontFaceV0 = v3f(-10.0f, 10.0f, 20.0f);
    v3 FrontFaceV1 = v3f( 10.0f, 10.0f, 20.0f);
    v3 FrontFaceV2 = v3f(-10.0f, -10.0f,20.0f);
    AddTriangle(FrontFaceV0,FrontFaceV1,FrontFaceV2, 0x0000FF);
    
    v3 FrontFaceV00 = v3f(10.0f, 10.0f, 20.0f);
    v3 FrontFaceV01 = v3f(10.0f, -10.0f, 20.0f);
    v3 FrontFaceV02 = v3f(-10.0f, -10.0f,20.0f);
    AddTriangle(FrontFaceV00,FrontFaceV01,FrontFaceV02,0x000FF);
    
    
    v3 RightFaceV0 = v3f(10.0f, -10.0f, 20.0f);
    v3 RightFaceV1 = v3f(10.0f, 10.0f,  20.0f);
    v3 RightFaceV2 = v3f(10.0f, -10.0f, 25.0f);
    AddTriangle(RightFaceV0,RightFaceV1,RightFaceV2,0x00FF00);
    
    v3 RightFaceV00 = v3f(10.0f, 10.0f, 20.0f);
    v3 RightFaceV01 = v3f(10.0f, 10.0f,  25.0f);
    v3 RightFaceV02 = v3f(10.0f, -10.0f, 25.0f);
    AddTriangle(RightFaceV00,RightFaceV01,RightFaceV02,0xFF0000);
    
    
    v3 BackFaceV0 = v3f(10.0f, 10.0f,  25.0f);
    v3 BackFaceV1 = v3f(10.0f, -10.0f, 25.0f);
    v3 BackFaceV2 = v3f(-10.0f, 10.0f, 25.0f);
    AddTriangle(BackFaceV0,BackFaceV1,BackFaceV2,0xFFFF00);
    
    v3 BackFaceV00 = v3f(10.0f, -10.0f, 25.0f);
    v3 BackFaceV01 = v3f(-10.0f, -10.0f,  25.0f);
    v3 BackFaceV02 = v3f(-10.0f, 10.0f, 25.0f);
    AddTriangle(BackFaceV00,BackFaceV01,BackFaceV02,0xFF0000);
    
    v3 LeftFaceV0 = v3f(-10.0f, 10.0f, 20.0f);
    v3 LeftFaceV1 = v3f(-10.0f, -10.0f,20.0f);
    v3 LeftFaceV2 = v3f(-10.0f, 10.0f, 25.0f);
    AddTriangle(LeftFaceV0,LeftFaceV1,LeftFaceV2,0xF0000F);
    
    v3 LeftFaceV00 = v3f(-10.0f, -10.0f, 20.0f);
    v3 LeftFaceV01 = v3f(-10.0f, -10.0f,  25.0f);
    v3 LeftFaceV02 = v3f(-10.0f, 10.0f, 25.0f);
    AddTriangle(LeftFaceV00,LeftFaceV01,LeftFaceV02,0x00FFFF);
#else
    v3 LeftFaceV00 = v3f(5.0f, -10.0f, 20.0f);
    v3 LeftFaceV01 = v3f(10.0f, -10.0f, 20.0f);
    v3 LeftFaceV02 = v3f(5.0f, 10.0f,  -1.0f);
    AddTriangle(LeftFaceV00,LeftFaceV01,LeftFaceV02,0x00FFFF);
#endif
    ClearBackbuffer(Backbuffer);
    
    persist f32 ZOffset = 1.0f;
    persist f32 XOffset = 1.0f;
    if(Input->Controller[0].ButtonUp.IsDown)
    {
        ZOffset += 0.01f;
    }
    if(Input->Controller[0].ButtonDown.IsDown)
    {
        ZOffset -= 0.01f;
    }
    if(Input->Controller[0].ButtonLeft.IsDown)
    {
        XOffset -= 0.01f;
    }
    if(Input->Controller[0].ButtonRight.IsDown)
    {
        XOffset += 0.01f;
    }
    
    static f32 Angle = 0.001f;
    Angle += 0.001f;
#if 1
    
    //gTriangles[0].Vertex[1].z = gTriangles[0].Vertex[1].z + ZOffset;
    //gTriangles[0].Vertex[2].z = gTriangles[0].Vertex[2].z + ZOffset;
    
    MoveTriangle(0,XOffset,0,ZOffset);
    MoveTriangle(1,XOffset,0,ZOffset);
    MoveTriangle(2,XOffset,0,ZOffset);
    MoveTriangle(3,XOffset,0,ZOffset);
    MoveTriangle(4,XOffset,0,ZOffset);
    MoveTriangle(5,XOffset,0,ZOffset);
    MoveTriangle(6,XOffset,0,ZOffset);
    MoveTriangle(7,XOffset,0,ZOffset);
    MoveTriangle(8,XOffset,0,ZOffset);
    
    
    
    for(u32 Index = 0;
        Index < 8;
        Index++)
    {
        for(u32 VertexIndex = 0;
            VertexIndex < 3;
            VertexIndex++)
        {
            gTriangles[Index].Vertex[VertexIndex] = RotateZAroundPoint(Angle, gTriangles[Index].Vertex[VertexIndex], v3f(1,1,0));
        }
    }
    
#endif
    f32 FOV = 90.0f; // NOTE(shvayko): 
    f32 n = 1.0f; // NOTE: Near plane
    f32 f = 15.0f; // NOTE: Far plane
    f32 AspectRatio = (f32)WINDOW_WIDTH / (f32)WINDOW_HEIGHT;
    for(u32 TriangleIndex = 0;
        TriangleIndex < gTrianglesCount;
        TriangleIndex++)
    {
        triangle *Triangle = &gTriangles[TriangleIndex];
        v3 V0 = Triangle->Vertex[0];
        v3 V1 = Triangle->Vertex[1];
        v3 V2 = Triangle->Vertex[2];
        
        // NOTE(shvayko): Transform to clip space
        v4 ClipV0,ClipV1,ClipV2;
        TransformIntoClipSpace(FOV,n,f,AspectRatio,V0,V1,V2,
                               &ClipV0, &ClipV1, &ClipV2);
        
        // NOTE(shvayko): Clipping  -1 <= x/w <= 1 ==> -w <= x <= w
        
        // NOTE(shvayko): Clipcode for every vertex
        u32 ClipCode[3];
        memset(ClipCode,0,4*3);
        // NOTE(shvayko): Count how much vertices have Z > NearPlane
        u32 VerticesInsideZRange = 0;
        v4 TestVertices[3] = {ClipV0,ClipV1,ClipV2};
        
        // NOTE(shvayko): Check X coordinates
        for(u32 VertexIndex = 0;
            VertexIndex < 3;
            VertexIndex++)
        {
            v4 CurrentTestVertex = TestVertices[VertexIndex];
            if(CurrentTestVertex.x > CurrentTestVertex.w)
            {
                ClipCode[VertexIndex] |= CLIPCODE_X_GREATER;
            }
            else if(CurrentTestVertex.x < -CurrentTestVertex.w)
            {
                ClipCode[VertexIndex] |= CLIPCODE_X_LOWER;
            }
            else
            {
                ClipCode[VertexIndex] |= CLIPCODE_X_INSIDE;
            }
            
            if(CurrentTestVertex.y > CurrentTestVertex.w)
            {
                ClipCode[VertexIndex] |= CLIPCODE_Y_GREATER;
            }
            else if(CurrentTestVertex.y < -CurrentTestVertex.w)
            {
                ClipCode[VertexIndex] |= CLIPCODE_Y_LOWER;
            }
            else
            {
                ClipCode[VertexIndex] |= CLIPCODE_Y_INSIDE;
            }
            
            if(CurrentTestVertex.z > CurrentTestVertex.w)
            {
                ClipCode[VertexIndex] |= CLIPCODE_Z_GREATER;
            }
            else if(CurrentTestVertex.z < -CurrentTestVertex.w)
            {
                ClipCode[VertexIndex] |= CLIPCODE_Z_LOWER;
            }
            else
            {
                ClipCode[VertexIndex] |= CLIPCODE_Z_INSIDE;
                VerticesInsideZRange++;
            }
            
        }
        // NOTE(shvayko):Check if all 3 points outside canonical volume
        if(IsTrivialReject(ClipCode,Clipping_CheckX) ||
           IsTrivialReject(ClipCode,Clipping_CheckY) ||  
           IsTrivialReject(ClipCode,Clipping_CheckZ))
        {
            // NOTE(shvayko): Go to the next polygon
            continue;
        }
        
        
        // NOTE(shvayko):Check if any vertex lying beyond near plane
        
        if(IsBitSet(ClipCode[0],7) ||
           IsBitSet(ClipCode[1],7) ||
           IsBitSet(ClipCode[2],7))
        {
            if(VerticesInsideZRange == 1)
            {
                // NOTE(shvayko): The simplest case where only one interior vertex. 
                // Just interpolate each exterior vertex with interior vertex and that 
                // will produce new vertices which will  represent one new triangle.
                
                // NOTE(shvayko): TmpV0 - Interior vertex; TmpV1 - Exterior vertex; 
                // TmpV2 - Exterior vertex
                v4 TmpV0,TmpV1,TmpV2;
                if(ClipCode[0] & CLIPCODE_Z_INSIDE)
                {
                    TmpV0 = ClipV0;
                    TmpV1 = ClipV1;
                    TmpV2 = ClipV2;
                }
                else if(ClipCode[1] & CLIPCODE_Z_INSIDE)
                {
                    TmpV0 = ClipV1;
                    TmpV1 = ClipV0;
                    TmpV2 = ClipV2;
                }
                else
                {
                    TmpV0 = ClipV2;
                    TmpV1 = ClipV0;
                    TmpV2 = ClipV1;
                }
                
                // NOTE(shvayko): solving line equations
                // Pi(x,y,z) = P0(x,y,z) + (P1(x,y,z) - P0(x,y,z))*t
                // NOTE(shvayko): TmpV0 and TmpV1
                // 1.0f is Near plane
                f32 t = -TmpV1.z / (TmpV0.z - TmpV1.z); 
                
                f32 x = TmpV1.x + (TmpV0.x - TmpV1.x)*t;
                f32 y = TmpV1.y + (TmpV0.y - TmpV1.y)*t;
                f32 z = TmpV1.w + (TmpV0.w - TmpV1.w)*t;
                TmpV1 = v4f(x,y,z,z);
                // NOTE(shvayko): TmpV0 and TmpV2
                // 1.0f is Near plane
                f32 t1 = -TmpV2.z / (TmpV0.z - TmpV2.z); 
                
                x = TmpV2.x + (TmpV0.x - TmpV2.x)*t1;
                y = TmpV2.y + (TmpV0.y - TmpV2.y)*t1;
                z = TmpV2.w + (TmpV0.w - TmpV2.w)*t1;
                TmpV2 = v4f(x,y,z,z);
                
                // NOTE(shvayko): New triangle
                ClipV0 = TmpV0;
                ClipV1 = TmpV1;
                ClipV2 = TmpV2;
                
                int foo = 5;
            }
            else if(VerticesInsideZRange == 2)
            {
                
                // NOTE(shvayko): The  case where two interior vertex. 
                // That case will produce 2 triangles
                
                // NOTE(shvayko): TmpV0 - Interior vertex; TmpV1 - Interior vertex; 
                // TmpV2 - Exterior vertex
                
                v4 TmpV0,TmpV1,TmpV2;
                if(ClipCode[0] & CLIPCODE_Z_INSIDE)
                {
                    TmpV0 = ClipV0;
                    if(ClipCode[1] & CLIPCODE_Z_INSIDE)
                    {
                        TmpV1 = ClipV1;
                        TmpV2 = ClipV2;
                    }
                    else
                    {
                        TmpV1 = ClipV2;
                        TmpV2 = ClipV1;
                    }
                } 
                else if(ClipCode[1] & CLIPCODE_Z_INSIDE)
                {
                    TmpV0 = ClipV1;
                    TmpV1 = ClipV2;
                    TmpV2 = ClipV0;
                }
                else
                {
                    assert(!"May be I need one more if statement for clipcode[2]");
                }
                
                // NOTE(shvayko): Solve for t when z component is equal to near z
                // NOTE(shvayko): first created new vertex
                f32 t = -TmpV2.z / (TmpV0.z - TmpV2.z); 
                
                f32 X0i = TmpV2.x + (TmpV0.x - TmpV2.x)*t;
                f32 Y0i = TmpV2.y + (TmpV0.y - TmpV2.y)*t;
                f32 Z0i = TmpV2.z + (TmpV0.z - TmpV2.z)*t;
                
                // NOTE(shvayko): second created new vertex
                
                f32 t1 = -TmpV2.z / (TmpV1.z - TmpV2.z);
                
                f32 X1i = TmpV2.x + (TmpV1.x - TmpV2.x)*t1;
                f32 Y1i = TmpV2.y + (TmpV1.y - TmpV2.y)*t1;
                f32 Z1i = TmpV2.z + (TmpV1.z - TmpV2.z)*t1;
                
                // NOTE(shvayko): Split in 2 triangles
                
                AddTriangle(TmpV0.xyz,v3f(X0i,Y0i,Z0i),TmpV1.xyz,0xFF0000);
                AddTriangle(TmpV0.xyz,TmpV1.xyz,v3f(X1i,Y1i,Z1i),0xFF0000);
                
            }
            else
            {
                assert(!"lol");
            }
        }
        else
        {
            // NOTE(shvayko): All vertices is in Z range. Process without any clipping
            
            // NOTE(shvayko): Pespective divide. (Transforming from Clip Space into NDC space)
            
            v3 NdcV0,NdcV1,NdcV2;
            TransformHomogeneousToNDC(ClipV0,ClipV1,ClipV2,&NdcV0,&NdcV1,&NdcV2);
            
            // NOTE(shvayko):Viewport tranformation(Transforming from NDC space(-1.0f - 1.0f))
            // to screen space(0 - Width, 0 - Height)
            
            v3 WinPV0,WinPV1,WinPV2;
            Viewport(NdcV0,NdcV1,NdcV2,&WinPV0,&WinPV1,&WinPV2,ClipV0.z,ClipV1.z,ClipV2.z);
            
            // NOTE(shvayko): Rasterization Stage
            DrawTriangle(Backbuffer,WinPV0,WinPV1,WinPV2, Triangle->Color);
        }
    }
}