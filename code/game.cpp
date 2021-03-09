#define BBP 4
#include "math.cpp"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

v3
Project(f32 AspectRatio,f32 FocalLength, v3 P)
{
    v3 TransformedP;
    
    TransformedP.x = FocalLength*P.x / P.z;
    TransformedP.y = FocalLength*P.y*AspectRatio / P.z;
    
    return TransformedP;
}

void
ClearBackbuffer(game_backbuffer *Backbuffer)
{
    memset(Backbuffer->Memory,0, Backbuffer->Width * Backbuffer->Height * 4);
}

v3
Rotate(f32 Angle, v3 P)
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
RotateAroundPoint(f32 Angle, v3 P, v3 PToRotateAround)
{
    return Rotate(Angle, P - PToRotateAround) + PToRotateAround;
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



void
GameUpdateAndRender(game_backbuffer *Backbuffer, controller *Input)
{
    
    v3 FrontFaceV0 = v3f(-10.0f, 10.0f, 2.0f);
    v3 FrontFaceV1 = v3f( 10.0f, 10.0f, 2.0f);
    v3 FrontFaceV2 = v3f(-10.0f, -10.0f, 2.0f);
    
    v3 FrontFaceV00 = v3f(10.0f, 10.0f, 2.0f);
    v3 FrontFaceV01 = v3f(10.0f, -10.0f, 2.0f);
    v3 FrontFaceV02 = v3f(-10.0f, -10.0f, 2.0f);
    
    v3 Vertices[6] = 
    {
        FrontFaceV0, FrontFaceV1, FrontFaceV2,
        FrontFaceV00, FrontFaceV01, FrontFaceV02
    };
    
    persist f32 Angle = 0.001f;
    
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
    
    
    f32 AspectRatio = (f32)WINDOW_WIDTH / (f32)WINDOW_HEIGHT;
    f32 FocalLength = 1.0f;
    
    for(u32 VerticesIndex = 0;
        VerticesIndex < CountOf(Vertices);
        VerticesIndex++)
    {
        Vertices[VerticesIndex].z = ZOffset;
        Vertices[VerticesIndex] = Project(AspectRatio, FocalLength, Vertices[VerticesIndex]);
        
        Vertices[VerticesIndex] = Vertices[VerticesIndex] + v3f(400.0f,300.0f,0.0f);
    }
    
    DrawTriangle(Backbuffer, Vertices[0], Vertices[1], Vertices[2], 0x00FF00);
    DrawTriangle(Backbuffer, Vertices[3], Vertices[4], Vertices[5], 0x00FF00);
    
    Angle += 0.001f;
}