#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

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

#define IS_DOWN(Name,IsDown)  (Buttons->Name.IsDown = IsDown)
#define WAS_DOWN(Name,WasDown) (Buttons->Name.WasDown = WasDown)

internal void
KeyboardMessagesProccessing(buttons *Buttons)  
{
    MSG Message;
    while(PeekMessageA(&Message,0,0,0,PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_KEYUP:
            case WM_KEYDOWN:
            {
                bool IsDown  =  (Message.lParam & (1 << 31)) == 0;
                bool WasDown =  (Message.lParam & (1 << 30)) != 0;
                if(IsDown != WasDown)
                {
                    switch(Message.wParam)
                    {
                        case 'W':
                        {
                            IS_DOWN(ButtonUp,IsDown);
                            WAS_DOWN(ButtonUp,WasDown);
                        }break;
                        case 'A':
                        {
                            IS_DOWN(ButtonLeft,IsDown);
                            WAS_DOWN(ButtonLeft,WasDown);
                        }break;
                        case 'S':
                        {
                            IS_DOWN(ButtonDown,IsDown);
                            WAS_DOWN(ButtonDown,WasDown);
                        }break;
                        case 'D':
                        {
                            IS_DOWN(ButtonRight,IsDown);
                            WAS_DOWN(ButtonRight,WasDown);
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
    
    
    controller  Keyboard[2] = {};
    controller *OldInput = &Keyboard[0];
    controller *NewInput = &Keyboard[1];
    
    while(gGameIsRunning)
    {
        
        buttons *OldControllerInput = &OldInput->Controller[0];
        buttons *NewControllerInput = &NewInput->Controller[0];
        buttons  clearControllerInput = {};
        *NewControllerInput = clearControllerInput;
        
        for(u32 ButtonIndex = 0;
            ButtonIndex < CountOf(NewControllerInput->Buttons);
            ButtonIndex++)
        {
            NewControllerInput->Buttons[ButtonIndex].IsDown = OldControllerInput->Buttons[ButtonIndex].IsDown;
        }
        
        game_backbuffer GameBackbuffer = {};
        
        GameBackbuffer.Width  = gBackbuffer.Width;
        GameBackbuffer.Height = gBackbuffer.Height; 
        GameBackbuffer.Stride = gBackbuffer.Stride;
        GameBackbuffer.Memory = gBackbuffer.Memory;
        
        KeyboardMessagesProccessing(NewControllerInput);
        GameUpdateAndRender(&GameBackbuffer, Keyboard);
        window_dim Dim = GetWindowDim(WindowHandle);
        BlitToScreen(DeviceContext, &gBackbuffer, Dim.Width, Dim.Height);
        
        controller *Temp = NewInput;
        NewInput = OldInput;
        OldInput = Temp;
    }
    return 0;
}