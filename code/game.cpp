#define BBP 4
#include "math.cpp"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define Radians(X) (X * PI / 180.0f)
#define CENTER_OF_PROJECTION 0.5f*v3f((f32)WINDOW_WIDTH,(f32)WINDOW_HEIGHT,0.0f)
#define MAX_TRIANGLES 9064

struct triangle
{
    v3 Vertex[3];
    u32 Color;
};

global u32 gTrianglesCount;
global triangle gTriangles[MAX_TRIANGLES];


internal void
NewTriangle(v3 V0, v3 V1, v3 V2, u32 Color)
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
ClipTriangleIfOneVertexOutside(v4 V0, v4 V1, v4 V2)
{
    // NOTE(shvayko): We are assume what V0 is in wrong side
    
    f32 A = -V0.z / (V1.z - V0.z);
    f32 B = -V0.z / (V2.z - V0.z);
    
    v3 NewVertexV0;
    v3 NewVertexV1;
    NewVertexV0.x = Lerp(V0.x,V1.x, A);
    NewVertexV0.y = Lerp(V0.y,V1.y, A);
    NewVertexV0.z = Lerp(V0.z,V1.z, A);
    
    NewVertexV1.x = Lerp(V0.z,V2.z, B);
    NewVertexV1.y = Lerp(V0.z,V2.z, B);
    NewVertexV1.z = Lerp(V0.z,V2.z, B);
    
    //NewTriangle(NewVertexV0, V1.xyz, V2.xyz,0xFFFF00);
    //NewTriangle(NewVertexV1, V1.xyz, V2.xyz,0xFFFF00);
}

void
ClipTriangleIfTwoVertexOutside(v4 V0, v4 V1, v4 V2)
{
    // NOTE(shvayko): We are assume what V0 and V1 in the wrong side
    
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
    
    // NOTE(shvayko): Always clear to zero for inserting new triangles
    gTrianglesCount = 0;
    memset(gTriangles,0,MAX_TRIANGLES);
    v3 FrontFaceV0 = v3f(-10.0f, 10.0f, 20.0f);
    v3 FrontFaceV1 = v3f( 10.0f, 10.0f, 20.0f);
    v3 FrontFaceV2 = v3f(-10.0f, -10.0f,20.0f);
    NewTriangle(FrontFaceV0,FrontFaceV1,FrontFaceV2, 0x0000FF);
    
    v3 FrontFaceV00 = v3f(10.0f, 10.0f, 20.0f);
    v3 FrontFaceV01 = v3f(10.0f, -10.0f, 20.0f);
    v3 FrontFaceV02 = v3f(-10.0f, -10.0f,20.0f);
    NewTriangle(FrontFaceV00,FrontFaceV01,FrontFaceV02,0x000FF);
    
    
    v3 RightFaceV0 = v3f(10.0f, -10.0f, 20.0f);
    v3 RightFaceV1 = v3f(10.0f, 10.0f,  20.0f);
    v3 RightFaceV2 = v3f(10.0f, -10.0f, 25.0f);
    NewTriangle(RightFaceV0,RightFaceV1,RightFaceV2,0x00FF00);
    
    v3 RightFaceV00 = v3f(10.0f, 10.0f, 20.0f);
    v3 RightFaceV01 = v3f(10.0f, 10.0f,  25.0f);
    v3 RightFaceV02 = v3f(10.0f, -10.0f, 25.0f);
    NewTriangle(RightFaceV00,RightFaceV01,RightFaceV02,0xFF0000);
    
    
    v3 BackFaceV0 = v3f(10.0f, 10.0f,  25.0f);
    v3 BackFaceV1 = v3f(10.0f, -10.0f, 25.0f);
    v3 BackFaceV2 = v3f(-10.0f, 10.0f, 25.0f);
    NewTriangle(BackFaceV0,BackFaceV1,BackFaceV2,0xFFFF00);
    
    v3 BackFaceV00 = v3f(10.0f, -10.0f, 25.0f);
    v3 BackFaceV01 = v3f(-10.0f, -10.0f,  25.0f);
    v3 BackFaceV02 = v3f(-10.0f, 10.0f, 25.0f);
    NewTriangle(BackFaceV00,BackFaceV01,BackFaceV02,0xFF0000);
    
    
    v3 LeftFaceV0 = v3f(-10.0f, 10.0f, 20.0f);
    v3 LeftFaceV1 = v3f(-10.0f, -10.0f,20.0f);
    v3 LeftFaceV2 = v3f(-10.0f, 10.0f, 25.0f);
    NewTriangle(LeftFaceV0,LeftFaceV1,LeftFaceV2,0xF0000F);
    
    v3 LeftFaceV00 = v3f(-10.0f, -10.0f, 20.0f);
    v3 LeftFaceV01 = v3f(-10.0f, -10.0f,  25.0f);
    v3 LeftFaceV02 = v3f(-10.0f, 10.0f, 25.0f);
    NewTriangle(LeftFaceV00,LeftFaceV01,LeftFaceV02,0x00FFFF);
    
    ClearBackbuffer(Backbuffer);
    
    persist f32 ZOffset = 1.0f;
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
        ZOffset += 0.01f;
    }
    if(Input->Controller[0].ButtonRight.IsDown)
    {
        ZOffset -= 0.01f;
    }
    
    f32 AspectRatio = (f32)WINDOW_WIDTH / (f32)WINDOW_HEIGHT;
    f32 FocalLength = 1.0f;
    
    
    static f32 Angle = 0.0001f;
    Angle += 0.0001f;
    
    for(u32 TriangleIndex = 0;
        TriangleIndex < gTrianglesCount;
        TriangleIndex++)
    {
        triangle *Triangle = &gTriangles[TriangleIndex];
        v3 V0 = Triangle->Vertex[0];
        v3 V1 = Triangle->Vertex[1];
        v3 V2 = Triangle->Vertex[2];
        
        // NOTE(shvayko): Transform to clip space
        f32 n = 1.0f; // NOTE: Near plane
        f32 f = 10.0f; // NOTE: Far plane
        
        f32 TanHalfFov = tanf(Radians(90.0f * 0.5f));
        f32 xScale = 1.0f / (TanHalfFov * AspectRatio);
        f32 yScale = 1.0f / TanHalfFov;
        f32 a = f/(f-n);
        f32 b = -2*n*f / (f - n);
        
        v4 ClipV0;
        v4 ClipV1;
        v4 ClipV2;
        
        ClipV0.x = V0.x * xScale;
        ClipV0.y = V0.y * yScale;
        ClipV0.z = V0.z * a + b;
        ClipV0.w = V0.z + ZOffset;
        
        ClipV1.x = V1.x * xScale;
        ClipV1.y = V1.y * yScale;
        ClipV1.z = V1.z * a + b;
        ClipV1.w = V1.z + ZOffset;
        
        ClipV2.x = V2.x * xScale;
        ClipV2.y = V2.y * yScale;
        ClipV2.z = V2.z * a + b;
        ClipV2.w = V2.z + ZOffset;
        
        // TODO(shvayko):Clipping  -1 <= x/w <= 1 ==> -w <= x <= w
        // NOTE(shvayko): Test if there are no vertices inside clip shape.
        bool DiscardTriangle = false;
        
        if((ClipV0.x > ClipV0.w) &&
           (ClipV1.x > ClipV1.w) &&
           (ClipV2.x > ClipV2.w))
        {
            DiscardTriangle = true;
        }
        if((ClipV0.x < -ClipV0.w) &&
           (ClipV1.x < -ClipV1.w) &&
           (ClipV2.x < -ClipV2.w))
        {
            DiscardTriangle = true;
        }
        if((ClipV0.y > ClipV0.w) &&
           (ClipV1.y > ClipV1.w) &&
           (ClipV2.y > ClipV2.w))
        {
            DiscardTriangle = true;
        }
        if((ClipV0.y < -ClipV0.w) &&
           (ClipV1.y < -ClipV1.w) &&
           (ClipV2.y < -ClipV2.w))
        {
            DiscardTriangle = true;
        }
        if((ClipV0.z > ClipV0.w) &&
           (ClipV1.z > ClipV1.w) &&
           (ClipV2.z > ClipV2.w))
        {
            DiscardTriangle = true;
        }
        if((ClipV0.z < -ClipV0.w) &&
           (ClipV1.z < -ClipV1.w) &&
           (ClipV2.z < -ClipV2.w))
        {
            DiscardTriangle = true;
        }
        
        
        // NOTE(shvayko):Near clipping test
        // We will not test far plane
        if(!DiscardTriangle)
        {
            if(ClipV0.z < 0.0f)
            {
                
                if(V1.z < 0.0f)
                {
                    //ClipTriangleIfTwoVertexOutside(ClipV0,ClipV1,ClipV2);
                }
                else if(V2.z < 0.0f)
                {
                    //ClipTriangleIfTwoVertexOutside(ClipV0,ClipV2,ClipV1);
                }
                else
                {
                    //ClipTriangleIfOneVertexOutside(ClipV0,ClipV1,ClipV2);
                }
            }
            else if(ClipV1.z < 0.0f)
            {
                if(ClipV0.z < 0.0f)
                {
                    //ClipTriangleIfTwoVertexOutside(ClipV1,ClipV0,ClipV2);
                }
                else if(ClipV2.z < 0.0f)
                {
                    //ClipTriangleIfTwoVertexOutside(ClipV1,ClipV2,ClipV0);
                }
                else
                {
                    //ClipTriangleIfOneVertexOutside(ClipV1,ClipV0,ClipV2);
                }
            }
            else if(ClipV2.z < 0.0f)
            {
                //ClipTriangleIfOneVertexOutside(ClipV2,ClipV0,ClipV1);
            }
            else
            {
                // NOTE(shvayko): All vertices on the right side
                // NOTE(shvayko): Pespective divide. 
                v3 NdcV0;
                v3 NdcV1;
                v3 NdcV2;
                NdcV0.x = ClipV0.x / ClipV0.w;
                NdcV0.y = ClipV0.y / ClipV0.w;
                NdcV0.z = ClipV0.z / ClipV0.w;
                
                NdcV1.x = ClipV1.x / ClipV1.w;
                NdcV1.y = ClipV1.y / ClipV1.w;
                NdcV1.z = ClipV1.z / ClipV1.w;
                
                NdcV2.x = ClipV2.x / ClipV2.w;
                NdcV2.y = ClipV2.y / ClipV2.w;
                NdcV2.z = ClipV2.z / ClipV2.w;
#if 1
                assert((-1.0f <= NdcV0.x) && (NdcV0.x <= 1.0f));
                assert((-1.0f <= NdcV0.y) && (NdcV0.y  <= 1.0f));
                assert((-1.0f <= NdcV0.z) && (NdcV0.z  <= 1.0f));
                
                assert((-1.0f <= NdcV1.x) && (NdcV1.x <= 1.0f));
                assert((-1.0f <= NdcV1.y) && (NdcV1.y  <= 1.0f));
                assert((-1.0f <= NdcV1.z) && (NdcV1.z  <= 1.0f));
                
                assert((-1.0f <= NdcV2.x) && (NdcV2.x <= 1.0f));
                assert((-1.0f <= NdcV2.y) && (NdcV2.y  <= 1.0f));
                assert((-1.0f <= NdcV2.z) && (NdcV2.z  <= 1.0f));
#endif
                
                // NOTE(shvayko):Transforming from NDC space to windows coordinates
                v3 WinPV0;
                v3 WinPV1;
                v3 WinPV2;
                WinPV0.x = (NdcV0.x + 1.0f)*WINDOW_WIDTH*0.5f;
                WinPV0.y = (NdcV0.y + 1.0f)*WINDOW_HEIGHT*0.5f;
                WinPV0.z = ClipV0.z;
                
                WinPV1.x = (NdcV1.x + 1.0f)*WINDOW_WIDTH*0.5f;
                WinPV1.y = (NdcV1.y + 1.0f)*WINDOW_HEIGHT*0.5f;
                WinPV1.z = ClipV1.z;
                
                WinPV2.x = (NdcV2.x + 1.0f)*WINDOW_WIDTH*0.5f;
                WinPV2.y = (NdcV2.y + 1.0f)*WINDOW_HEIGHT*0.5f;
                WinPV2.z = ClipV2.z;
                
                // NOTE(shvayko): Rasterization Stage
                DrawTriangle(Backbuffer,WinPV0,WinPV1,WinPV2, Triangle->Color);
            }
        }
    }
}