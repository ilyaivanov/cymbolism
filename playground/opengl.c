#include <windows.h>
#include <gl/gl.h>
#include <stdio.h>
#include "..\performance.c"
#include "..\memory.c"
#include "..\win_utils.c"

i32 isRunning = 1;

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#define InvalidCodePath Assert(!"InvalidCodePath")

void Win32InitOpenGL(HWND Window)
{
    HDC WindowDC = GetDC(Window);

    PIXELFORMATDESCRIPTOR DesiredPixelFormat = {0};
    DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
    DesiredPixelFormat.nVersion = 1;
    DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
    DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
    DesiredPixelFormat.cColorBits = 32;
    DesiredPixelFormat.cAlphaBits = 8;
    DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

    int SuggestedPixelFormatIndex = ChoosePixelFormat(WindowDC, &DesiredPixelFormat);
    PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
    DescribePixelFormat(WindowDC, SuggestedPixelFormatIndex,
                        sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);
    SetPixelFormat(WindowDC, SuggestedPixelFormatIndex, &SuggestedPixelFormat);
    
    HGLRC OpenGLRC = wglCreateContext(WindowDC);
    if(wglMakeCurrent(WindowDC, OpenGLRC))        
    {
        // NOTE(casey): Success!!!
    }
    else
    {
        InvalidCodePath;
        // TODO(casey): Diagnostic
    }
    ReleaseDC(Window, WindowDC);
}

LRESULT OnEvent(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_DESTROY)
    {
        isRunning = 0;
    }
    else if (message == WM_PAINT)
    {
        PAINTSTRUCT paint = {0};
        HDC dc = BeginPaint(window, &paint);

        // glViewport(0, 0, 200, 200);
        glClearColor(1.0f, 0.5f, 1.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        SwapBuffers(dc);

        EndPaint(window, &paint);
    }

    return DefWindowProc(window, message, wParam, lParam);
}
i32 goingUp = 1;
float color = 0;
int wWinMain(HINSTANCE instance, HINSTANCE prev, PWSTR cmdLine, int showCode)
{
    PreventWindowsDPIScaling();

    HWND window = OpenGameWindow(instance, OnEvent);
    HDC dc = GetDC(window);

    Win32InitOpenGL(window);

    while (isRunning)
    {
        MSG msg;
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if(goingUp)
            color += 0.01f;
        else 
            color -= 0.01f;
        
        if(color >= 1)
            goingUp = 0;
        else if (color <= 0)
            goingUp = 1;

        Start(FrameTotal);
        glClearColor(1.0f, color, 1.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        Stop(FrameTotal);
        SwapBuffers(dc);

        PrintFrameStats();
        ResetMetrics();
    }
}