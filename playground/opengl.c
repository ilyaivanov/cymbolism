#define USE_OPEN_GL

#include <windows.h>

#ifdef USE_OPEN_GL
#include <gl/gl.h>
#include "..\win_opengl.c"
#endif

#include <stdio.h>
#include <math.h>
#include "..\types.h"
#include "..\memory.c"
#include "..\performance.c"
#include "..\win_utils.c"
#include "..\drawing.c"

#include "..\gdiFont.c"

i32 isRunning = 1;

BITMAPINFO bitmapInfo;
MyBitmap bitmap;

AppState app;




LRESULT OnEvent(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_DESTROY)
    {
        isRunning = 0;
    }
    else if (message == WM_SIZE)
    {
        RECT rect;
        GetClientRect(window, &rect);
        int windowWidth = rect.right - rect.left;
        int windowHeight = rect.bottom - rect.top;
        OnResize(&bitmapInfo, &bitmap, windowWidth, windowHeight);
    }
    else if (message == WM_PAINT)
    {
        PAINTSTRUCT paint = {0};
        HDC dc = BeginPaint(window, &paint);
        EndPaint(window, &paint);
    }

    return DefWindowProc(window, message, wParam, lParam);
}

float time = 0;
float width = 0.03;

int wWinMain(HINSTANCE instance, HINSTANCE prev, PWSTR cmdLine, int showCode)
{
    PreventWindowsDPIScaling();

    InitFontSystem(&app.fonts.regular, 100, "Segoe UI");

    HWND window = OpenGameWindowDim(instance, OnEvent, 2200, 1500);
    // ToggleFullscreen(window, 1);
    HDC dc = GetDC(window);

    u32 frequency = 3000000000;



    MyBitmap * icon = &app.fonts.regular.checkmark;

#ifdef USE_OPEN_GL
    GLuint texHandle = 0;
    glGenTextures(1, &texHandle);
    Win32InitOpenGL(window);
#endif
    while (isRunning)
    {
        ResetMetrics();
        Start(FrameTotal);

        MSG msg;
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // [0 .. 2 - width]
        float offset = (sinf(time / 500) + 1) / (2 / (2 - width));
#ifdef USE_OPEN_GL
        glViewport(0, 0, bitmap.width, bitmap.height);

        glBindTexture(GL_TEXTURE_2D, texHandle);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, icon->width, icon->height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, icon->pixels);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        glEnable(GL_TEXTURE_2D);

        glClearColor(0, 0, 0, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();

        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();


        glMatrixMode(GL_PROJECTION);


        // allows to specify pixel coordinates for vertices
        GLfloat m[] = {
            2.0f / bitmap.width, 0, 0, 0,   
            0, 2.0f / bitmap.height, 0, 0,   
            0, 0, 1, 0,   
            -1, -1, 0, 1,   
        };
        glLoadMatrixf(m);
        // glLoadIdentity();



        
        float p = (float)icon->width / bitmap.width;
        float h = (float)icon->height / bitmap.height;
        
        glBegin(GL_TRIANGLE_STRIP);

        glTexCoord2f(0, 1);
        glVertex2f(bitmap.width / 2 - icon->width / 2, bitmap.height / 2 + icon->height / 2);

        glTexCoord2f(0, 0);
        glVertex2f(bitmap.width / 2 - icon->width / 2, bitmap.height / 2 - icon->height / 2);

        glTexCoord2f(1, 1);
        glVertex2f(bitmap.width / 2 + icon->width / 2, bitmap.height / 2 + icon->height / 2);

        glTexCoord2f(1, 0);
        glVertex2f(bitmap.width / 2 + icon->width / 2, bitmap.height / 2 - icon->height / 2);        
        glEnd();

        SwapBuffers(dc);
#else 
        memset(bitmap.pixels, 0x11, bitmap.height * bitmap.width * bitmap.bytesPerPixel);

       
        DrawRect(&bitmap, bitmap.width / 2 - icon->width / 2, bitmap.height / 2 - icon->height / 2, icon->width, icon->height, 0x333333);
        DrawTextureCentered(&bitmap, icon, bitmap.width /2 , bitmap.height / 2, 0xffffff);
        // DrawRect(&bitmap, offset / 2 * bitmap.width , 0, bitmap.width * width / 2, bitmap.height, 0xffffff);
        DrawBitmap(dc, &bitmapInfo, &bitmap);
#endif

        Stop(FrameTotal);
        time += (float)metrics.perf[FrameTotal] * 1000 / frequency;
        PrintFrameStats();
    }
}