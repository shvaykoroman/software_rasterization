#include "math.cpp"
#include "graphics.cpp"
#include "texture.cpp"

#define IsButtonDown(Button) (Input->Controller[0].Button.IsDown)
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define Radians(X) (X * PI / 180.0f)
#define SCREEN_CENTER 0.5f*v3f((f32)WINDOW_WIDTH,(f32)WINDOW_HEIGHT,0.0f)
#define MAX_TRIANGLES 9064
#define MAX_MODELS 1024
#define MAX_POLYGONS_IN_MODEL 1024

#define CLIPCODE_X_GREATER (1 << 0) 
#define CLIPCODE_X_LOWER   (1 << 1)
#define CLIPCODE_X_INSIDE  (1 << 2)

#define CLIPCODE_Y_GREATER (1 << 3)
#define CLIPCODE_Y_LOWER   (1 << 4)
#define CLIPCODE_Y_INSIDE  (1 << 5)

#define CLIPCODE_Z_GREATER (1 << 6)
#define CLIPCODE_Z_LOWER   (1 << 7)
#define CLIPCODE_Z_INSIDE  (1 << 8)


#define ClippingBit_XGreater  0
#define ClippingBit_XLower    1 
#define ClippingBit_XInside   2 

#define ClippingBit_YGreater  3
#define ClippingBit_YLower    4
#define ClippingBit_YInside   5 

#define ClippingBit_ZGreater  6
#define ClippingBit_ZLower    7
#define ClippingBit_ZInside   8 

global bool gIsInit = false;
global v3 gCameraP;
global v3 gCameraTarget; 

enum
{
    Clipping_CheckX = 0,
    Clipping_CheckY,
    Clipping_CheckZ,
};

struct triangle
{
    v3 Vertex[3];
    v3 V0Color;
    v3 V1Color;
    v3 V2Color;
};

#pragma pack(push,1)
struct bitmap_format
{
    u16 FileType;
    u32 FileSize;
    u16 Reserved1;
    u16 Reserved2;
    u32 BitmapOffset;
    
    u32 Size;            /* Size of this header in bytes */
    s32 Width;           /* Image width in pixels */
    s32 Height;          /* Image height in pixels */
	u16 Planes;          /* Number of color planes */
	u16 BitsPerPixel;    /* Number of bits per pixel */
	u32 Compression;     /* Compression methods used */
	u32 SizeOfBitmap;    /* Size of bitmap in bytes */
    s32 HorzResolution;  /* Horizontal resolution in pixels per meter */
    s32 VertResolution;  /* Vertical resolution in pixels per meter */
	u32 ColorsUsed;      /* Number of colors in the image */
	u32 ColorsImportant; /* Minimum number of important colors */
    
    u32 DedMask;       /* Mask identifying bits of red component */
    u32 GreenMask;     /* Mask identifying bits of green component */
    u32 BlueMask;      /* Mask identifying bits of blue component */
    u32 AlphaMask;     /* Mask identifying bits of alpha component */
};
#pragma pack(pop)

inline void*
OpenFile(char *Filename)
{
    void *Result = 0;
    FILE *File = fopen(Filename,"rb");
    u32 FileSize = 0;
    if(File)
    {
        fseek(File,0,SEEK_END);
        FileSize = ftell(File);
        fseek(File,0,SEEK_SET);
        
        Result = malloc(FileSize);
        fread(Result,FileSize,1,File);
        fclose(File);
    }
    return Result;
}

inline texture 
LoadTexture(char *Filename)
{
    texture Result;
    
    Result.Memory = OpenFile(Filename);
    if(Result.Memory)
    {
        bitmap_format *Header = (bitmap_format*)Result.Memory;
        
        Result.Width  = Header->Width;
        Result.Height = Header->Height;
        Result.Stride = Header->Width * BBP;
        
        Result.Memory = (u32*)((u8*)Result.Memory + Header->BitmapOffset);
        
        u8 *Row = (u8*)Result.Memory;
        for(u32 Y = 0; Y < Result.Height; Y++)
        {
            u32 *Pixel = (u32*)Row;
            for(u32 X = 0; X < Result.Width; X++)
            {
                
            }
            Row += Result.Stride;
        }
        
    }
    
    return Result;
}

global u32 gTrianglesCount;
global triangle gTriangles[MAX_TRIANGLES];


struct cube_model
{
    v3 WorldP;
    
    u32 PolysCount;
    triangle Polys[MAX_POLYGONS_IN_MODEL];
};

global cube_model CubeModels[MAX_MODELS];
global u32 gCubeModelsCount;

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
        if((IsBitSet(ClipCode[0],ClippingBit_XGreater) &&
            IsBitSet(ClipCode[1],ClippingBit_XGreater) && 
            IsBitSet(ClipCode[2],ClippingBit_XGreater)) ||
           
           (IsBitSet(ClipCode[0],ClippingBit_XLower) &&
            IsBitSet(ClipCode[1],ClippingBit_XLower) &&
            IsBitSet(ClipCode[2],ClippingBit_XLower)))
        {
            return true;
        }
        
    }
    else if(Dim == 1)
    {
        if((IsBitSet(ClipCode[0],ClippingBit_YGreater)  &&
            IsBitSet(ClipCode[1],ClippingBit_YGreater)  && 
            IsBitSet(ClipCode[2],ClippingBit_YGreater)) ||
           
           (IsBitSet(ClipCode[0],ClippingBit_YLower) &&
            IsBitSet(ClipCode[1],ClippingBit_YLower) &&
            IsBitSet(ClipCode[2],ClippingBit_YLower)))
        {
            return true;
        }
    }
    else if(Dim == 2)
    {
        if((IsBitSet(ClipCode[0],ClippingBit_ZGreater)  &&
            IsBitSet(ClipCode[1],ClippingBit_ZGreater)  && 
            IsBitSet(ClipCode[2],ClippingBit_ZGreater)) ||
           
           (IsBitSet(ClipCode[0],ClippingBit_ZLower) &&
            IsBitSet(ClipCode[1],ClippingBit_ZLower) &&
            IsBitSet(ClipCode[2],ClippingBit_ZLower)))
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

global f32 gDepthBuffer[WINDOW_HEIGHT][WINDOW_WIDTH];

internal void
ClearDepthBuffer(f32 InitialDepthValue)
{
    for(u32 Y = 0;
        Y < WINDOW_HEIGHT;
        Y++)
    {
        for(u32 X = 0;
            X < WINDOW_WIDTH;
            X++)
        {
            gDepthBuffer[Y][X] = InitialDepthValue;
        }
    }
}

internal f32
InterpolateDepth(f32 Depth0, f32 Depth1, f32 Depth2,
                 f32 W0, f32 W1, f32 W2)
{
    f32 Result = 0;
    
    f32 D0 = Depth0 * W0;
    f32 D1 = Depth1 * W1;
    f32 D2 = Depth2 * W2;
    
    Result = D0 + D1 + D2;
    
    return Result;
}

internal f32
GetValueFromDepthBufferAt(s32 X,s32 Y)
{
    f32 Result = gDepthBuffer[Y][X];
    return Result;
}

internal void
SetValueToDepthBufferAt(s32 X, s32 Y, f32 Value)
{
    gDepthBuffer[Y][X] = Value;
}


internal void
AddTriangle(v3 V0, v3 V1, v3 V2, v3 V0Color,v3 V1Color,v3 V2Color)
{
    //assert(gTrianglesCount < MAX_TRIANGLES);
    triangle *Triangle = &gTriangles[gTrianglesCount++];
    
    Triangle->Vertex[0] = V0;
    Triangle->Vertex[1] = V1;
    Triangle->Vertex[2] = V2;
    Triangle->V0Color = V0Color;
    Triangle->V1Color = V1Color;
    Triangle->V2Color = V2Color;
}


internal void
CameraTransform(v3 *V0,v3 *V1,v3 *V2, v3 WorldP)
{
    v3 TemporaryUp = v3f(0.0f,1.0f,0.0f);
    
    v3 CameraD = gCameraTarget - gCameraP;
    CameraD = Normalize(CameraD);
    
    v3 CameraRight = CrossProduct(CameraD,TemporaryUp);
    CameraRight = Normalize(CameraRight);
    
    v3 CameraUp = CrossProduct(CameraRight,CameraD);
    CameraUp = Normalize(CameraUp);
    
    f32 DP0 = DotProduct(CameraRight, gCameraP);
    f32 DP1 = DotProduct(CameraUp, gCameraP);
    f32 DP2 = DotProduct(CameraD, gCameraP);
    
    /*
x_axis.x  x_axis.y  x_axis.z  -dot(x_axis,eye)   |x|
  y_axis.x  y_axis.y  y_axis.z  -dot(y_axis,eye)   |y|
  z_axis.x  z_axis.y  z_axis.z  -dot(z_axis,eye)   |z|
                      
=>

x = x_axis.x*x + x_axis.y*y + x_axis.z * z - dot
 y = y_axis.x*x + y_axis.y*y + y_axis.z * z - dot
 z = z_axis.x*x + z_axis.y*y + z_axis.z * z - dot

*/
    
    f32 CameraSpaceXV0 = -CameraRight.x*V0->x+CameraRight.y*V0->y+CameraRight.z*V0->z - DP0;
    f32 CameraSpaceYV0 = CameraUp.x*V0->x+CameraUp.y*V0->y+CameraUp.z*V0->z - DP1;
    f32 CameraSpaceZV0 = CameraD.x*V0->x+CameraD.y*V0->y+CameraD.z*V0->z - DP2;
    
    f32 CameraSpaceXV1 = -CameraRight.x*V1->x+CameraRight.y*V1->y+CameraRight.z*V1->z - DP0;
    f32 CameraSpaceYV1 = CameraUp.x*V1->x+CameraUp.y*V1->y+CameraUp.z*V1->z - DP1;
    f32 CameraSpaceZV1 = CameraD.x*V1->x+CameraD.y*V1->y+CameraD.z*V1->z - DP2;
    
    f32 CameraSpaceXV2 = -CameraRight.x*V2->x+CameraRight.y*V2->y+CameraRight.z*V2->z - DP0;
    f32 CameraSpaceYV2 = CameraUp.x*V2->x+CameraUp.y*V2->y+CameraUp.z*V2->z - DP1;
    f32 CameraSpaceZV2 = CameraD.x*V2->x+CameraD.y*V2->y+CameraD.z*V2->z - DP2;
    
    *V0 = v3f(CameraSpaceXV0,CameraSpaceYV0,CameraSpaceZV0);
    *V1 = v3f(CameraSpaceXV1,CameraSpaceYV1,CameraSpaceZV1);
    *V2 = v3f(CameraSpaceXV2,CameraSpaceYV2,CameraSpaceZV2);
}


internal void
AddPolygonIntoModel(cube_model *Model, v3 V0, v3 V1, v3 V2,
                    v3 C0, v3 C1, v3 C2)
{
    AddTriangle(V0,V1,V2, C0, C1, C2);
    Model->Polys[Model->PolysCount] = gTriangles[gTrianglesCount - 1];
    Model->PolysCount++;
}

internal cube_model*
CreateCube(v3 InitWorldP)
{
    cube_model *Result = &CubeModels[gCubeModelsCount];
    Result->WorldP = InitWorldP;
    gCubeModelsCount += 1;
    
    v3 FrontFaceV0 = v3f(-1.0f, 1.0f, 1.0f);
    v3 FrontFaceV1 = v3f( 1.0f, 1.0f, 1.0f);
    v3 FrontFaceV2 = v3f(-1.0f, -1.0f,1.0f);
    AddPolygonIntoModel(Result, FrontFaceV0,FrontFaceV1,FrontFaceV2, v3f(1.0f,0,0),v3f(0,1.0f,0),v3f(0,0,1.0f));
    
    v3 FrontFaceV00 = v3f(1.0f, 1.0f,  1.0f);
    v3 FrontFaceV01 = v3f(1.0f, -1.0f, 1.0f);
    v3 FrontFaceV02 = v3f(-1.0f, -1.0f,1.0f);
    AddPolygonIntoModel(Result,FrontFaceV00,FrontFaceV01,FrontFaceV02,v3f(1.0f,0,0),v3f(0,1.0f,0),v3f(0,0,1.0f));
    
    v3 RightFaceV0 = v3f(1.0f, -1.0f, 1.0f);
    v3 RightFaceV1 = v3f(1.0f, 1.0f,  1.0f);
    v3 RightFaceV2 = v3f(1.0f, -1.0f, 2.0f);
    AddPolygonIntoModel(Result,RightFaceV0,RightFaceV1,RightFaceV2,v3f(1.0f,0,0),v3f(1.0f,0.0f,0),v3f(1.0,0,0.0f));
    
    v3 RightFaceV00 = v3f(1.0f, 1.0f, 1.0f);
    v3 RightFaceV01 = v3f(1.0f, 1.0f, 2.0f);
    v3 RightFaceV02 = v3f(1.0f, -1.0f,2.0f);
    AddPolygonIntoModel(Result,RightFaceV00,RightFaceV01,RightFaceV02,v3f(1.0f,0,0),v3f(1.0f,0.0f,0),v3f(1.0f,0,0.0f));
    
    v3 BackFaceV0 = v3f(1.0f, 1.0f,  2.0f);
    v3 BackFaceV1 = v3f(1.0f, -1.0f, 2.0f);
    v3 BackFaceV2 = v3f(-1.0f, 1.0f, 2.0f);
    AddPolygonIntoModel(Result,BackFaceV0,BackFaceV1,BackFaceV2,v3f(0.0f,1.0f,0),v3f(0,1.0f,0),v3f(0,1.0f,0.0f));
    
    v3 BackFaceV00 = v3f(1.0f, -1.0f, 2.0f);
    v3 BackFaceV01 = v3f(-1.0f, -1.0f,2.0f);
    v3 BackFaceV02 = v3f(-1.0f, 1.0f, 2.0f);
    AddPolygonIntoModel(Result,BackFaceV00,BackFaceV01,BackFaceV02,v3f(0.0f,1.0f,0),v3f(0,1.0f,0),v3f(0,1.0f,0.0f));
    
    v3 LeftFaceV0 = v3f(-1.0f, 1.0f, 1.0f);
    v3 LeftFaceV1 = v3f(-1.0f, -1.0f,1.0f);
    v3 LeftFaceV2 = v3f(-1.0f, 1.0f, 2.0f);
    AddPolygonIntoModel(Result,LeftFaceV0,LeftFaceV1,LeftFaceV2,v3f(0.0f,0,1.0f),v3f(0,0.0f,1.0f),v3f(0,0,1.0f));
    
    v3 LeftFaceV00 = v3f(-1.0f, -1.0f, 1.0f);
    v3 LeftFaceV01 = v3f(-1.0f, -1.0f, 2.0f);
    v3 LeftFaceV02 = v3f(-1.0f, 1.0f,  2.0f);
    AddPolygonIntoModel(Result,LeftFaceV00,LeftFaceV01,LeftFaceV02,v3f(0.0f,0,1.0f),v3f(0,0.0f,1.0f),v3f(0,0,1.0f));
    
    v3 UpFaceV0 = v3f(-1.0f,  -1.0f,  1.0f);
    v3 UpFaceV1 = v3f(-1.0f,  -1.0f,  2.0f);
    v3 UpFaceV2 = v3f(1.0f,  -1.0f,   2.0f);
    AddPolygonIntoModel(Result,UpFaceV0,UpFaceV1,UpFaceV2,v3f(0.0f,0,0),v3f(0,1.0f,0),v3f(0,0,0.0f));
    
    v3 UpFaceV00 = v3f(1.0f, -1.0f,  1.0f);
    v3 UpFaceV01 = v3f(1.0f, -1.0f,  2.0f);
    v3 UpFaceV02 = v3f(-1.0f, -1.0f, 1.0f);
    AddPolygonIntoModel(Result,UpFaceV00,UpFaceV01,UpFaceV02,v3f(0.0f,0,0),v3f(0,1.0f,0),v3f(0,0,0.0f));
    
    v3 BottomFaceV0 = v3f(-1.0f,  1.0f,  1.0f);
    v3 BottomFaceV1 = v3f(-1.0f,  1.0f,  2.0f);
    v3 BottomFaceV2 = v3f(1.0f,  1.0f,   2.0f);
    AddPolygonIntoModel(Result,BottomFaceV0,BottomFaceV1,BottomFaceV2,v3f(0.0f,0,0),v3f(0,1.0f,0),v3f(0,0,0.0f));
    
    v3 BottomFaceV00 = v3f(1.0f, 1.0f,  1.0f);
    v3 BottomFaceV01 = v3f(1.0f, 1.0f,  2.0f);
    v3 BottomFaceV02 = v3f(-1.0f,1.0f,  1.0f);
    AddPolygonIntoModel(Result,BottomFaceV00,BottomFaceV01,BottomFaceV02,v3f(0,0,0),v3f(0,1,0),v3f(0,0,0));
    
    return Result;
}

internal void
TranslateCube(cube_model *Cube, v3 P)
{
    for(u32 PolysIndex = 0;
        PolysIndex < Cube->PolysCount;
        PolysIndex++)
    {
        for(u32 VertexIndex = 0;
            VertexIndex < 3;
            VertexIndex++)
        {
            triangle *Poly = Cube->Polys + PolysIndex;
            
            Poly->Vertex[VertexIndex] = Poly->Vertex[VertexIndex] + P;
        }
    }
}

void
TransformIntoClipSpace(f32 FOV,f32 n,f32 f,f32 AspectRatio, 
                       v3 RawV0, v3 RawV1, v3 RawV2,
                       v4 *ClipSpaceV0Out, v4 *ClipSpaceV1Out, v4 *ClipSpaceV2Out)
{
    
    f32 TanHalfFov = tanf(Radians(FOV * 0.5f));
    f32 XScale = 1.0f / (TanHalfFov * AspectRatio);
    f32 YScale = 1.0f / TanHalfFov;
    
    f32 a = (n+f)/(f-n);
    f32 b = (-2*n*f) / (f - n);
    
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

internal void
RotateCube(cube_model *Cube,f32 Angle)
{
    for(u32 Index = 0;
        Index < Cube->PolysCount;
        Index++)
    {
        for(u32 VertexIndex = 0;
            VertexIndex < 3;
            VertexIndex++)
        {
            Cube->Polys[Index].Vertex[VertexIndex] = RotateZAroundPoint(Angle,Cube->Polys[Index].Vertex[VertexIndex], v3f(1,1,0));
            
            Cube->Polys[Index].Vertex[VertexIndex] = RotateXAroundPoint(Angle,Cube->Polys[Index].Vertex[VertexIndex], v3f(0,1,1));
        }
    }
}

internal u32
GetColorForPixel(barycentric_results BColor,v3 V0Color,v3 V1Color,v3 V2Color)
{
    u32 Result = 0;
    u32 ColorV0 = ((u32)((V0Color.r*255.0f)*BColor.W0) << 16) |
        ((u32)((V0Color.g*255.0f)*BColor.W1) << 8) | 
        ((u32)((V0Color.b*255.0f)*BColor.W2) << 0);
    
    u32 ColorV1 = ((u32)((V1Color.r*255.0f)*BColor.W0) << 16) |
        ((u32)((V1Color.g*255.0f)*BColor.W1) << 8) | 
        ((u32)((V1Color.b*255.0f)*BColor.W2) << 0);
    
    u32 ColorV2 = ((u32)((V2Color.r*255.0f)*BColor.W0) << 16) |
        ((u32)((V2Color.g*255.0f)*BColor.W1) << 8) | 
        ((u32)((V2Color.b*255.0f)*BColor.W2) << 0);
    
    
    Result = ColorV0+ColorV1+ColorV2;
    return Result;
}

void
DrawFlatTopTriangle(game_backbuffer *Backbuffer, v3 Vertex0, v3 Vertex1, v3 Vertex2, v3 V0Color,v3 V1Color, v3 V2Color, f32 DepthV0,f32 DepthV1,f32 DepthV2)
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
            f32 DepthValue = GetValueFromDepthBufferAt(XIndex,YIndex);
            
            barycentric_results BarycentricWeights = Barycentric(XIndex,YIndex, Vertex0, Vertex1, Vertex2);
            f32 InterpolatedZ = InterpolateDepth(DepthV0,DepthV1,DepthV2,BarycentricWeights.W0,BarycentricWeights.W1,BarycentricWeights.W2);
            if(InterpolatedZ < DepthValue)
            {
                SetValueToDepthBufferAt(XIndex,YIndex, InterpolatedZ);
                
                u32 Color = GetColorForPixel(BarycentricWeights,V0Color,V1Color,V2Color);
                PlotPixel(Backbuffer, XIndex, YIndex, Color);
            }
        }
    }
    
}

void
DrawFlatBottomTriangle(game_backbuffer *Backbuffer,v3 Vertex0,v3 Vertex1,v3 Vertex2, v3 V0Color,v3 V1Color, v3 V2Color,f32 DepthV0,f32 DepthV1,f32 DepthV2)
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
            f32 DepthValue = GetValueFromDepthBufferAt(XIndex,YIndex);
            
            barycentric_results BarycentricWeights = Barycentric(XIndex,YIndex, Vertex0, Vertex1, Vertex2);
            
            f32 InterpolatedZ = InterpolateDepth(DepthV0,DepthV1,DepthV2,BarycentricWeights.W0,BarycentricWeights.W1,BarycentricWeights.W2);
            
            if(InterpolatedZ < DepthValue)
            {
                SetValueToDepthBufferAt(XIndex,YIndex, InterpolatedZ);
                u32 Color = GetColorForPixel(BarycentricWeights,V0Color,V1Color,V2Color);
                
                PlotPixel(Backbuffer, XIndex, YIndex, Color);
            }
        }
    }
    
}

internal void
DrawTriangle(game_backbuffer *Backbuffer, v3 Vertex0, v3 Vertex1, v3 Vertex2, 
             v3 V0Color,v3 V1Color, v3 V2Color, f32 ZDepthV0, f32 ZDepthV1, f32 ZDepthV2)
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
        
        DrawFlatTopTriangle(Backbuffer,Vertex0,Vertex2,Vertex1,V0Color,V1Color,V2Color,
                            ZDepthV0, ZDepthV1,ZDepthV2);
    }
    else if(Vertex1.y == Vertex2.y)  // NOTE(shvayko): Flat bottom triangle
    {
        
        if(Vertex2.x > Vertex1.x)
        {
            v3 Temp = Vertex2;
            Vertex2 = Vertex1;
            Vertex1 = Temp;
        }
        
        
        DrawFlatBottomTriangle(Backbuffer, Vertex0,Vertex1,Vertex2, V0Color,V1Color, V2Color,ZDepthV0, ZDepthV1,ZDepthV2);
    }
    else  // NOTE(shvayko): General triangle
    {
        // NOTE(shvayko):General triangle will split up into 2 triangles: Flat bottom and flat top.
        
        f32 Alpha  = (Vertex1.y - Vertex0.y) / (Vertex2.y - Vertex0.y);
        
        // NOTE(shvaykop): Find new vertex through interpolating
        v3 Vertex = LerpV3(Vertex0,Vertex2,Alpha);
        // NOTE(shvayko): Decide what major the triangle is
        
        if(Vertex.x < Vertex1.x) // NOTE(shvayko): Left major
        {
            DrawFlatBottomTriangle(Backbuffer,Vertex0, Vertex1, Vertex, V0Color,V1Color, V2Color,ZDepthV0, ZDepthV1,ZDepthV2);
            
            DrawFlatTopTriangle(Backbuffer, Vertex1, Vertex2, Vertex, V0Color,V1Color, V2Color, ZDepthV0,ZDepthV1,ZDepthV2);
        }
        else // NOTE(shvayko): Right major
        {
            DrawFlatBottomTriangle(Backbuffer,Vertex0, Vertex, Vertex1, V0Color,V1Color, V2Color,ZDepthV0, ZDepthV1,ZDepthV2);
            
            DrawFlatTopTriangle(Backbuffer, Vertex, Vertex2, Vertex1, V0Color,V1Color, V2Color,ZDepthV0,ZDepthV1,ZDepthV2);
        }
    }
}

global texture Texture;

void
GameUpdateAndRender(game_backbuffer *Backbuffer, controller *Input)
{
    if(!gIsInit)
    {
        gIsInit = true;
        gCameraP = v3f(0.0f,0.0f,0.0f);
        gCameraTarget = v3f(0.0f,0.0f,1.0f);
        
        Texture = LoadTexture("../test.bmp");
        
        s32 FooBar = 5;
    }
    
    gTrianglesCount = 0; // NOTE(shvayko): Reset
    gCubeModelsCount = 0;
    memset(CubeModels,0,sizeof(cube_model)*1024);
    
    cube_model *Cube = CreateCube(v3f(1,0,13));
    cube_model *CubeSecond = CreateCube(v3f(5.8f,0,7));
    
    ClearDepthBuffer(1.0f);
    ClearBackbuffer(Backbuffer);
    
    textured_vertex TV0 = {v3f(200.0f,  200.0f, 1.0f), v2f(0.0f,0.0f)};
    textured_vertex TV1 = {v3f(200.0f,  50.0f,  1.0f), v2f(0.0f,1.0f)};
    textured_vertex TV2 = {v3f(250.0f,  50.0f,  1.0f), v2f(1.0f,1.0f)};
    v3 TVC0 = v3f(1.0f,0,0);
    v3 TVC1 = v3f(1.0f,0,0);
    v3 TVC2 = v3f(1.0f,0,0);
    
    textured_vertex TV10 = {v3f(250.0f,  200.0f,  1.0f), v2f(1.0f,0.0f)};
    textured_vertex TV20 = {v3f(250.0f,  50.0f,  1.0f), v2f(1.0f,1.0f)};
    textured_vertex TV00 = {v3f(200.0f,  200.0f, 1.0f), v2f(0.0f,0.0f)};
    
    DrawTriangleTextured(Backbuffer,TV0, TV1, TV2, TVC0, TVC1, TVC2, 0,0,0, Texture);
    DrawTriangleTextured(Backbuffer,TV00, TV10, TV20, TVC0, TVC1, TVC2, 0,0,0, Texture);
    
    
    // NOTE(shvayko): Test Cube 
    
    
    
    persist v3 P = Cube->WorldP;
    persist v3 P0 = CubeSecond->WorldP;
    
    if(IsButtonDown(ButtonUp))
    {
        P.z += 0.1f;
    }
    if(IsButtonDown(ButtonDown))
    {
        P.z -= 0.1f;
    }
    if(IsButtonDown(ButtonLeft))
    {
        P.x -= 0.1f;
    }
    if(IsButtonDown(ButtonRight))
    {
        P.x += 0.1f;
    }
    
    
    TranslateCube(Cube,P);
    TranslateCube(CubeSecond,P0);
    
    
#if 1
    {
        // NOTE(shvayko): Testing rotations
        persist f32 Angle = 0.001f;
        RotateCube(CubeSecond,Angle);
        Angle += 0.01f;
    }
#endif
    
    f32 FOV = 90.0f; // NOTE(shvayko): Field of View
    f32 n = 1.0f; // NOTE: Near plane
    f32 f = 15.0f; // NOTE: Far plane
    f32 AspectRatio = (f32)WINDOW_WIDTH / (f32)WINDOW_HEIGHT;
    for(u32 ModelIndex  = 0;
        ModelIndex < gCubeModelsCount;
        ModelIndex++)
    {
        cube_model *Model = CubeModels + ModelIndex;
        for(u32 ModelPolysIndex = 0;
            ModelPolysIndex < Model->PolysCount;
            ModelPolysIndex++)
        {
            triangle *Triangle = &Model->Polys[ModelPolysIndex];
            v3 V0 = Triangle->Vertex[0];
            v3 V1 = Triangle->Vertex[1];
            v3 V2 = Triangle->Vertex[2];
            
            CameraTransform(&V0,&V1,&V2, Model->WorldP);
            
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
            
            for(u32 VertexIndex = 0;
                VertexIndex < 3;
                VertexIndex++)
            {
                v4 CurrentTestVertex = TestVertices[VertexIndex];
                if(CurrentTestVertex.x >= CurrentTestVertex.w)
                {
                    ClipCode[VertexIndex] |= CLIPCODE_X_GREATER;
                }
                else if(CurrentTestVertex.x <= -CurrentTestVertex.w)
                {
                    ClipCode[VertexIndex] |= CLIPCODE_X_LOWER;
                }
                else
                {
                    ClipCode[VertexIndex] |= CLIPCODE_X_INSIDE;
                }
                
                if(CurrentTestVertex.y >= CurrentTestVertex.w)
                {
                    ClipCode[VertexIndex] |= CLIPCODE_Y_GREATER;
                }
                else if(CurrentTestVertex.y <= -CurrentTestVertex.w)
                {
                    ClipCode[VertexIndex] |= CLIPCODE_Y_LOWER;
                }
                else
                {
                    ClipCode[VertexIndex] |= CLIPCODE_Y_INSIDE;
                }
                
                if(CurrentTestVertex.z >= CurrentTestVertex.w)
                {
                    ClipCode[VertexIndex] |= CLIPCODE_Z_GREATER;
                }
                else if(CurrentTestVertex.z <= -CurrentTestVertex.w)
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
            if(IsBitSet(ClipCode[0],ClippingBit_ZLower) ||
               IsBitSet(ClipCode[1],ClippingBit_ZLower) ||
               IsBitSet(ClipCode[2],ClippingBit_ZLower))
            {
                if(VerticesInsideZRange == 1)
                {
                    // NOTE(shvayko): The simplest case where only one interior vertex. 
                    // Just interpolate each exterior vertex with interior vertex and that 
                    // will produce new vertices which will  represent one new triangle.
                    
                    // NOTE(shvayko): TmpV0 - Interior vertex; TmpV1 - Exterior vertex; 
                    // TmpV2 - Exterior vertex
                    v3 TmpV0,TmpV1,TmpV2;
                    if(IsBitSet(ClipCode[0],ClippingBit_ZInside))
                    {
                        TmpV0 = V0;
                        TmpV1 = V1;
                        TmpV2 = V2;
                    }
                    else if(IsBitSet(ClipCode[1],ClippingBit_ZInside))
                    {
                        TmpV0 = V1;
                        TmpV1 = V0;
                        TmpV2 = V2;
                    }
                    else
                    {
                        TmpV0 = V2;
                        TmpV1 = V0;
                        TmpV2 = V1;
                    }
                    
                    // NOTE(shvayko): Solve for t when z component is equal to near z
                    // Pi(x,y,z) = P0(x,y,z) + (P1(x,y,z) - P0(x,y,z))*t
                    
                    // NOTE(shvayko): 1.0f is near plane Z
                    
                    // NOTE(shvayko): TmpV0 and TmpV2
                    f32 t = (1.0f-TmpV1.z) / (TmpV0.z - TmpV1.z); 
                    
                    f32 x = TmpV1.x + (TmpV0.x - TmpV1.x)*t;
                    f32 y = TmpV1.y + (TmpV0.y - TmpV1.y)*t;
                    f32 z = TmpV1.z + (TmpV0.z - TmpV1.z)*t;
                    
                    TmpV1 = v3f(x,y,1.0f);
                    // NOTE(shvayko): TmpV0 and TmpV2
                    
                    f32 t1 = (1.0f - TmpV2.z) / (TmpV0.z - TmpV2.z); 
                    
                    x = TmpV2.x + (TmpV0.x - TmpV2.x)*t1;
                    y = TmpV2.y + (TmpV0.y - TmpV2.y)*t1;
                    z = TmpV2.z + (TmpV0.z - TmpV2.z)*t1;
                    
                    TmpV2 = v3f(x,y,1.0f);
                    
                    // NOTE(shvayko): New triangle
                    v3 NT0 = TmpV0;
                    v3 NT1 = TmpV1;
                    v3 NT2 = TmpV2;
                    
                    AddPolygonIntoModel(Model, NT0,NT1,NT2, Triangle->V0Color,Triangle->V1Color,Triangle->V2Color);
                    
                }
                else if(VerticesInsideZRange == 2)
                {
                    
                    // NOTE(shvayko): The  case where two interior vertex. 
                    // That case will produce 2 triangles
                    
                    // NOTE(shvayko): TmpV0 - Interior vertex; TmpV1 - Interior vertex; 
                    // TmpV2 - Exterior vertex
                    
                    v3 TmpV0,TmpV1,TmpV2;
                    if(IsBitSet(ClipCode[0],ClippingBit_ZInside))
                    {
                        TmpV0 = V0;
                        if(IsBitSet(ClipCode[1],ClippingBit_ZInside))
                        {
                            TmpV1 = V1;
                            TmpV2 = V2;
                        }
                        else
                        {
                            TmpV1 = V2;
                            TmpV2 = V1;
                        }
                    } 
                    else if(IsBitSet(ClipCode[1],ClippingBit_ZInside))
                    {
                        TmpV0 = V1;
                        TmpV1 = V2;
                        TmpV2 = V0;
                    }
                    
                    // NOTE(shvayko): Solve for t when z component is equal to near z
                    
                    // NOTE(shvayko): first created new vertex
                    
                    // NOTE(shvayko): 1.0f is near plane Z
                    f32 t = (1.0f-TmpV2.z) / (TmpV0.z - TmpV2.z); 
                    
                    f32 X0i = TmpV2.x + (TmpV0.x - TmpV2.x)*t;
                    f32 Y0i = TmpV2.y + (TmpV0.y - TmpV2.y)*t;
                    f32 Z0i = TmpV2.z + (TmpV0.z - TmpV2.z)*t;
                    
                    // NOTE(shvayko): second created new vertex
                    
                    f32 t1 = (1.0f-TmpV2.z) / (TmpV1.z - TmpV2.z);
                    
                    f32 X1i = TmpV2.x + (TmpV1.x - TmpV2.x)*t1;
                    f32 Y1i = TmpV2.y + (TmpV1.y - TmpV2.y)*t1;
                    f32 Z1i = TmpV2.z + (TmpV1.z - TmpV2.z)*t1;
                    
                    // NOTE(shvayko): Split into 2 triangles
                    AddPolygonIntoModel(Model, TmpV0,v3f(X1i,Y1i,1.0f),TmpV1, Triangle->V0Color,Triangle->V1Color,Triangle->V2Color);
                    AddPolygonIntoModel(Model, TmpV0,v3f(X0i,Y0i,1.0f),v3f(X1i,Y1i,1.0f), Triangle->V0Color,Triangle->V1Color,Triangle->V2Color);
                    
                }
                else
                {
                    assert(!"lol");
                }
            }
            else
            {
                // NOTE(shvayko): All vertices is in Z range. Process without any clipping
                
                // NOTE(shvayko): Pespective divide. (Transforming from Clip Space into NDC space(Z from 0 to 1))
                
                v3 NdcV0,NdcV1,NdcV2;
                TransformHomogeneousToNDC(ClipV0,ClipV1,ClipV2,&NdcV0,&NdcV1,&NdcV2);
                
                // NOTE(shvayko):Viewport tranformation(Transforming from NDC space(-1.0f - 1.0f))
                // to screen space(0 - Width, 0 - Height)
                // NOTE(shvayko): Depth testing is done in screen space
                v3 WinPV0,WinPV1,WinPV2;
                Viewport(NdcV0,NdcV1,NdcV2,&WinPV0,&WinPV1,&WinPV2,ClipV0.z,ClipV1.z,ClipV2.z);
                
                // NOTE(shvayko): Rasterization Stage
                DrawTriangle(Backbuffer,WinPV0,WinPV1,WinPV2,
                             Triangle->V0Color,Triangle->V1Color,Triangle->V2Color,
                             NdcV0.z,NdcV1.z,NdcV2.z);
            }
        }
    }
}