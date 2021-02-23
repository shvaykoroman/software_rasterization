#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/*Well, if you’re using the convention that one unit in local coordinates is 1 meter, and you
want to have a universe that’s 100×100×100 km (a good size for a tank or racing sim), that
means that your game grid has the following dimensions, given that there are 1,000
meters in a kilometer:*/

#include "game.h"
#include <math.h>
#include "game.cpp"

bool gGameIsRunning;

global backbuffer gBackbuffer;

internal window_dim
GetWindowDim(HWND Window)
{
    window_dim Result = {};
    RECT Rect;
    GetClientRect(Window,&Rect);
    
    Result.Width  = Rect.right - Rect.left;
    Result.Height = Rect.bottom - Rect.top;
    
    return Result;
}

internal void
CreateBackbuffer(backbuffer *Backbuffer, s32 Width, s32 Height)
{
    if(Backbuffer->Memory)
    {
        VirtualFree(Backbuffer->Memory,0,MEM_RELEASE);
    }
    
    Backbuffer->Width  = Width;
    Backbuffer->Height = Height;
    
    Backbuffer->BitmapInfo.bmiHeader = {};
    
    Backbuffer->BitmapInfo.bmiHeader.biSize     = sizeof(Backbuffer->BitmapInfo.bmiHeader);
    Backbuffer->BitmapInfo.bmiHeader.biWidth    = Backbuffer->Width;
    Backbuffer->BitmapInfo.bmiHeader.biHeight   = -Backbuffer->Height;
    Backbuffer->BitmapInfo.bmiHeader.biPlanes   = 1;
    Backbuffer->BitmapInfo.bmiHeader.biBitCount = 32;
    Backbuffer->BitmapInfo.bmiHeader.biCompression = BI_RGB;
    
    s32 BackbufferSize = Backbuffer->Width * Backbuffer->Height * 4;
    Backbuffer->Stride = Backbuffer->Width * 4;  
    
    Backbuffer->Memory = VirtualAlloc(0, (size_t)BackbufferSize,MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);  
    
}

internal void
BlitToScreen(HDC DeviceContext, backbuffer *Backbuffer, s32 WindowWidth, s32 WindowHeight)
{
    
    StretchDIBits(DeviceContext,
                  0,0, WindowWidth, WindowHeight,
                  0,0, Backbuffer->Width, Backbuffer->Height,
                  Backbuffer->Memory,
                  &Backbuffer->BitmapInfo,
                  DIB_RGB_COLORS,
                  SRCCOPY);
}

internal void
KeyboardMessagesProccessing()  
{
    MSG Message;
    while(PeekMessageA(&Message,0,0,0,PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_KEYUP:
            case WM_KEYDOWN:
            {
                bool isDown  =  (Message.lParam & (1 << 31)) == 0;
                bool wasDown =  (Message.lParam & (1 << 30)) != 0;
                if(isDown != wasDown)
                {
                    switch(Message.wParam)
                    {
                        case 'W':
                        {
                            
                        }break;
                        case 'A':
                        {
                            
                        }break;
                        case 'S':
                        {
                            
                        }break;
                        case 'D':
                        {
                            
                        }break;
                        default:
                        {
                        }break;
                    }
                }
            }break;
            default:
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            }break;
        }
    }
}

LRESULT CALLBACK 
WindowProc(HWND Window, UINT Message, WPARAM WParam,LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch(Message)
    {
        case WM_ACTIVATE:
        {
        }break;
        case WM_CREATE:
        {
        }break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC deviceContext = BeginPaint(Window, &ps);
            window_dim Dim = GetWindowDim(Window);
            BlitToScreen(deviceContext, &gBackbuffer, Dim.Width, Dim.Height);
            EndPaint(Window, &ps);
        }break;
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            gGameIsRunning = false;
        }break;
        case WM_QUIT:
        {
        }break;
        default:
        {
            return DefWindowProc(Window,Message,WParam,LParam);
        }break;
    }
    return 0;
}

int 
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance, LPSTR CmdLine, int ShowCmd)
{
    u32 WindowWidth = 800;
    u32 WindowHeight = 600;
    CreateBackbuffer(&gBackbuffer,WindowWidth,WindowHeight);
    
    WNDCLASS WindowClass = {};
    
    char *WindowClassName = "MyWindowClassName";
    
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = WindowClassName;
    
    RegisterClassA(&WindowClass);
    
    HWND WindowHandle = CreateWindowExA(0, WindowClassName, "MyWindowName",
                                        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                        CW_USEDEFAULT, CW_USEDEFAULT,
                                        WindowWidth, WindowHeight, 0,0,Instance, 0);
    
    gGameIsRunning = true;
    HDC DeviceContext = GetDC(WindowHandle);
    while(gGameIsRunning)
    {
        game_backbuffer GameBackbuffer = {};
        
        GameBackbuffer.Width  = gBackbuffer.Width;
        GameBackbuffer.Height = gBackbuffer.Height; 
        GameBackbuffer.Stride = gBackbuffer.Stride;
        GameBackbuffer.Memory = gBackbuffer.Memory;
        
        KeyboardMessagesProccessing();
        GameUpdateAndRender(&GameBackbuffer);
        window_dim Dim = GetWindowDim(WindowHandle);
        BlitToScreen(DeviceContext, &gBackbuffer, Dim.Width, Dim.Height);
    }
    return 0;
}