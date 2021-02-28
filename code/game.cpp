#define BBP 4

#include "math.cpp"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

void
ClearBackbuffer(game_backbuffer *Backbuffer)
{
    memset(Backbuffer->Memory,0, Backbuffer->Width * Backbuffer->Height * 4);
}

v2
Rotate(f32 Angle, v2 P)
{
    v2 Result;
    
    f32 CosAngle = cosf(Angle);
    f32 SinAngle = sinf(Angle);
    
    Result.x = P.x * CosAngle - P.y * SinAngle;
    Result.y = P.x * SinAngle + P.y * CosAngle;
    
    return Result;
}

v2
RotateAroundPoint(f32 Angle, v2 P, v2 PToRotateAround)
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
DrawFlatTopTriangle(game_backbuffer *Backbuffer,v2 Vertex0, v2 Vertex1, v2 Vertex2, u32 Color)
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
DrawFlatBottomTriangle(game_backbuffer *Backbuffer,v2 Vertex0, v2 Vertex1, v2 Vertex2, u32 Color)
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
DrawTriangle(game_backbuffer *Backbuffer, v2 Vertex0, v2 Vertex1, v2 Vertex2, u32 Color)
{
    // NOTE(shvayko): Sort vertices from smallest to the biggest
    // V0 Smallest Y - V2 Biggest Y. Top-down
    
    if(Vertex0.y > Vertex1.y)
    {
        v2 Temp = Vertex0;
        Vertex0 = Vertex1;
        Vertex1 = Temp;
    } 
    // V1 > V0
    if(Vertex1.y > Vertex2.y)
    {
        v2 Temp = Vertex2;
        Vertex2 = Vertex1;
        Vertex1 = Temp;
    }
    // V2 > V1
    if(Vertex0.y > Vertex1.y)
    {
        v2 Temp = Vertex0;
        Vertex0 = Vertex1;
        Vertex1 = Temp;
    }
    
    if(Vertex0.y == Vertex1.y) // NOTE(shvayko): Flat top triangle
    {
        
        if(Vertex0.x < Vertex1.x)
        {
            v2 Temp = Vertex0;
            Vertex0 = Vertex1;
            Vertex1 = Temp;
        }
        
        DrawFlatTopTriangle(Backbuffer,Vertex0,Vertex2,Vertex1,Color);
    }
    else if(Vertex1.y == Vertex2.y)  // NOTE(shvayko): Flat bottom triangle
    {
        
        if(Vertex2.x > Vertex1.x)
        {
            v2 Temp = Vertex2;
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
        v2 Vertex = v2f(VX,VY); 
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
GameUpdateAndRender(game_backbuffer *Backbuffer)
{
    
    v2 FrontFaceV0 = v2f(400.0f, 200.0f);
    v2 FrontFaceV1 = v2f(400.0f, 250.0f);
    v2 FrontFaceV2 = v2f(550.0f, 250.0f);
    
    v2 Vertices[3] = 
    {
        FrontFaceV0, FrontFaceV1, FrontFaceV2
    };
    
    static f32 Angle = 0.001f;
    
    ClearBackbuffer(Backbuffer);
    
    for(u32 VerticesIndex = 0;
        VerticesIndex < 3;
        VerticesIndex++)
    {
        Vertices[VerticesIndex] = RotateAroundPoint(Angle, Vertices[VerticesIndex], v2f(250.0f,250.0f));
    }
    
    DrawTriangle(Backbuffer, Vertices[0], Vertices[1], Vertices[2], 0x00FF00);
    Angle += 0.001f;
}