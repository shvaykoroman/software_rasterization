#define BBP 4

typedef struct test_struct
{
    s32 A;
    s32 B;
}test_struct;

test_struct
TestFunction(s32 A, s32 B)
{
    test_struct Result = {0};
    
    return Result;
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
    u32 yStart = (u32)(ceil(Vertex0.y-0.5f));
    u32 yEnd = (u32)(ceil(Vertex1.y-0.5f));
    
    for(u32 YIndex = yStart; 
        YIndex < yEnd;
        YIndex++)
    {
        
        f32 X0 = (SlopeLeftSide  * (YIndex  + 0.5f - Vertex2.y)) + Vertex2.x;
        f32 X1 = (SlopeRightSide * (YIndex  + 0.5f - Vertex2.y)) + Vertex0.x;
        
        u32 xStart = (u32)ceil(X0 - 0.5f);
        u32 xEnd   = (u32)ceil(X1 - 0.5f);
        
        for(u32 XIndex = xStart;
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
    u32 yStart = (u32)(ceil(Vertex0.y-0.5f));
    u32 yEnd = (u32)(ceil(Vertex1.y-0.5f));
    
    for(u32 YIndex = yStart; 
        YIndex < yEnd;
        YIndex++)
    {
        
        f32 X0 = (SlopeLeftSide  * ((f32)YIndex + 0.5f - Vertex2.y)) + Vertex2.x;
        f32 X1 = (SlopeRightSide * (YIndex + 0.5f - Vertex2.y)) + Vertex1.x;
        
        u32 xStart = (u32)ceil(X0 - 0.5f);
        u32 xEnd   = (u32)ceil(X1 - 0.5f);
        
        for(u32 XIndex = xStart;
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
            DrawFlatBottomTriangle(Backbuffer,Vertex0, Vertex, Vertex1, 0x00FF00);
            
            DrawFlatTopTriangle(Backbuffer, Vertex, Vertex2, Vertex1, 0xFF0000);
        }
    }
}

void
GameUpdateAndRender(game_backbuffer *Backbuffer)
{
    // NOTE(shvayko): Clockwise FLAT TOP TRIANGLE TEST FIRST
    
    v2 Vertex, Vertex1,Vertex2;
    
    Vertex = v2f(600.0f,100.0f);
    Vertex2 = v2f(500.0f,100.0f);
    Vertex1  = v2f(550.0f,300.0f);
    
    DrawFlatTopTriangle(Backbuffer,Vertex,Vertex1,Vertex2, 0xFFFF0000);
    
    
    v2 Vertex00 = v2f(250.0f,100.0f);
    v2 Vertex01 = v2f(300.0f,250.0f);
    v2 Vertex02  = v2f(200.0f,250.0f);
    
    DrawFlatBottomTriangle(Backbuffer, Vertex00, Vertex01, Vertex02 ,0xFFFF00);
    
    v2 Vertex000 = v2f(200.0f,350.0f);
    v2 Vertex001 = v2f(300.0f,350.0f);
    v2 Vertex002  = v2f(350.0f,450.0f);
    
    DrawTriangle(Backbuffer, Vertex000, Vertex001, Vertex002 ,0x0000FF);
    
    // NOTE(shvayko): Right major
    v2 GeneralTriangleV0 = v2f(600.0f, 450.0f); 
    v2 GeneralTriangleV1 = v2f(500.0f, 500.0f);
    v2 GeneralTriangleV2 = v2f(700.0f, 550.0f);
    
    DrawTriangle(Backbuffer, GeneralTriangleV0,GeneralTriangleV1,GeneralTriangleV2, 
                 0x00FFFF);
    
    // NOTE(shvayko): Left major
    v2 GeneralTriangleV00 = v2f(500.0f, 450.0f); 
    v2 GeneralTriangleV01 = v2f(400.0f, 500.0f);
    v2 GeneralTriangleV02 = v2f(380.0f, 550.0f);
    
    DrawTriangle(Backbuffer, GeneralTriangleV00,GeneralTriangleV01,GeneralTriangleV02, 
                 0x00FFFF);
    
    
    DrawTriangle(Backbuffer, v2f(50.0f,100.0f),v2f(70.0f,100.0f),v2f(90.0f,50.0f), 
                 0xFFFFFF);
}