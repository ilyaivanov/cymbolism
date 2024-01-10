#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <gl/gl.h>

float appTime;

#include "..\win_opengl.c"

#include "..\types.h"
#include "..\vec.c"
#include "..\number.c"
#include "..\memory.c"
// #include "..\performance.c"
#include "..\textReflow.c"
#include "..\win_utils.c"
#include "..\drawing.c"
#include "..\gdiFont.c"

#include "..\text.c"
#include "..\item.c"
#include "..\serialization.c"

#include ".\ui\ui.c"


i32 isRunning = 1;

V2i screenSize = {0};
AppState app;

u8 keyboardState[256];
V2i mouse;

#define FILE_PATH "..\\data.txt"

LRESULT OnEvent(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_DESTROY)
    {
        isRunning = 0;
    }
    else if (message == WM_SIZE)
    {
        screenSize.x = LOWORD(lParam);
        screenSize.y = HIWORD(lParam);
        OnSizeChange(&screenSize);
    }
    else if (message == WM_MOUSEMOVE)
    {
        mouse.x = LOWORD(lParam);
        mouse.y = screenSize.y - HIWORD(lParam);
    }
    else if (message == WM_PAINT)
    {
        PAINTSTRUCT paint = {0};
        HDC dc = BeginPaint(window, &paint);
        EndPaint(window, &paint);
    }

    return DefWindowProc(window, message, wParam, lParam);
}


Item root;

int wWinMain(HINSTANCE instance, HINSTANCE prev, PWSTR cmdLine, int showCode)
{
    PreventWindowsDPIScaling();

    HWND window = OpenGameWindowDim(instance, OnEvent, 1000, 1000);
    // ToggleFullscreen(window, 1);
    HDC dc = GetDC(window);
    Win32InitOpenGL(window);
    OnSizeChange(&screenSize);

    
    InitUI(&app, &screenSize);

    FileContent file = ReadMyFileImp("..\\data.txt");
    ParseFileContent(&app, &root, file);

    while (isRunning)
    {
        i32 zDelta = 0;
        MSG msg;
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            if(msg.message == WM_MOUSEWHEEL)
            {
                zDelta = GET_WHEEL_DELTA_WPARAM(msg.wParam);
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        GetKeyboardState(keyboardState);

        DrawUI(&app, &screenSize, keyboardState, &mouse, &zDelta);

        SwapBuffers(dc);

        //TODO: use better timing
        appTime += 16.66666f;
    }
}