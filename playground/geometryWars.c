#include <windows.h>
#include <stdio.h>
#include <gl/gl.h>
#include <stdio.h>
#include <math.h>
#include "..\primitives.h"
#include "..\memory.c"
#include "..\number.c"
#include "..\win_utils.c"


#define ONE_OVER_SQUARE_ROOT_OF_TWO 0.70710678118

i32 isRunning = 1;
i32 isPaused = 0;
V2i screen = {0};

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
    else if (message == WM_SIZE)
    {
        screen.x = LOWORD(lParam);
        screen.y = HIWORD(lParam);
    }
    else if (message == WM_KILLFOCUS)
    {
        isPaused = 1;
    }
    else if (message == WM_SETFOCUS)
    {
        isPaused = 0;
    }
    else if (message == WM_PAINT)
    {
        PAINTSTRUCT paint = {0};
        HDC dc = BeginPaint(window, &paint);
        EndPaint(window, &paint);
    }

    return DefWindowProc(window, message, wParam, lParam);
}

V2f size = {40.0f, 40.0f};

V2f pos = {50.0f, 50.0f};

V3f color = {1.0f, 1.0f, 1.0f};

V2f shift = {0};
f32 speed = 10;

u8 keyboardState[256];
u32 counter = 0;

int wWinMain(HINSTANCE instance, HINSTANCE prev, PWSTR cmdLine, int showCode)
{
    PreventWindowsDPIScaling();

    HWND window = OpenGameWindowDim(instance, OnEvent, 1000, 1500);
    // ToggleFullscreen(window, 1);
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

        GetKeyboardState(keyboardState);

        if(keyboardState['W'] & 0b10000000)
            shift.y = 1;
        else if(keyboardState['S'] & 0b10000000)
            shift.y = -1;
        else 
            shift.y = 0;

        if(keyboardState['A'] & 0b10000000)
            shift.x = -1;
        else if(keyboardState['D'] & 0b10000000)
            shift.x = 1;
        else 
            shift.x = 0;

        if(shift.x != 0 && shift.y != 0)
        {
            shift.x *= ONE_OVER_SQUARE_ROOT_OF_TWO;
            shift.y *= ONE_OVER_SQUARE_ROOT_OF_TWO;
        }


        pos.x = Clampf32(pos.x + shift.x * speed, 0, screen.x - size.x);
        pos.y = Clampf32(pos.y + shift.y * speed, 0, screen.y - size.y);



        glClearColor(0, 0, 0, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glViewport(0, 0, screen.x, screen.y);

        glMatrixMode(GL_PROJECTION);

        // allows to specify pixel coordinates for vertices
        GLfloat m[] = {
            2.0f / screen.x, 0, 0, 0,   
            0, 2.0f / screen.y, 0, 0,   
            0, 0, 1, 0,   
            -1, -1, 0, 1,   
        };
        glLoadMatrixf(m);

        glBegin(GL_TRIANGLE_STRIP);

        glTexCoord2f(0, 1);
        glColor3f(color.x, color.y, color.z);
        glVertex2f(pos.x, pos.y + size.y);

        glTexCoord2f(0, 0);
        glVertex2f(pos.x, pos.y);

        glTexCoord2f(1, 1);
        glVertex2f(pos.x + size.y, pos.y + size.y);

        glTexCoord2f(1, 0);
        glVertex2f(pos.x + size.y, pos.y);
        glEnd();

        SwapBuffers(dc);
    }
}