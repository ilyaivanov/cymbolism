#include <windows.h>
#include "types.h"
#include "performance.c"

#include "win_utils.c"
#include "drawing.c"

#include "gdiFont.c"
#include "editor.c"

MyInput input = {0};
BITMAPINFO bitmapInfo = {0};
AppState state = {0};


i32 isRunning = 0;
i32 isFullscreen = 0;

LRESULT OnEvent(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_DESTROY)
    {
        isRunning = 0;
    }
    else if (message == WM_SIZE)
    {
        OnResize(window, &bitmapInfo, &state.canvas);
        // This fails for my metrics for first frame, need to think how to resolve this
        // memset(state.canvas.pixels, BACKGROUND_COLOR_GREY, state.canvas.height * state.canvas.width * state.canvas.bytesPerPixel);
        // UpdateAndDrawApp(&state, &input);
    }
    else if (message == WM_PAINT)
    {
        PAINTSTRUCT paint = {0};
        HDC dc = BeginPaint(window, &paint);
        DrawBitmap(dc, &bitmapInfo, &state.canvas);
        EndPaint(window, &paint);
    }
    else if (message == WM_CHAR)
    {
        input.charEventsThisFrame[input.charEventsThisFrameCount++] = wParam;
    }

    return DefWindowProc(window, message, wParam, lParam);
}

int wWinMain(HINSTANCE instance, HINSTANCE prev, PWSTR cmdLine, int showCode)
{
    PreventWindowsDPIScaling();
    timeBeginPeriod(1);
    InitApp(&state);

    HWND window = OpenGameWindow(instance, OnEvent);
    HDC dc = GetDC(window);

    isRunning = 1;
    i32 framesPrinted = 0;
    #define FRAMES_TO_PRINT 10
    while (isRunning)
    {
        Start(FrameTotal);

        memset(&input.keysPressed, 0, sizeof(input.keysPressed));
        input.charEventsThisFrameCount = 0;
        input.wheelDelta = 0;

        MSG msg;
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_KEYDOWN)
            {
                input.isPressed[msg.wParam] = 1;
            }
            if (msg.message == WM_KEYUP)
            {
                input.isPressed[msg.wParam] = 0;
            }
            if (msg.message == WM_MOUSEWHEEL)
            {
                input.wheelDelta = GET_WHEEL_DELTA_WPARAM(msg.wParam);
            }
            if (msg.message == WM_KEYDOWN)
            {
                if (msg.wParam == VK_F11)
                {
                    isFullscreen = isFullscreen ? 0 : 1;
                    ToggleFullscreen(window, isFullscreen);
                }
                if(msg.wParam < 256)
                {
                    input.keysPressed[msg.wParam] = 1;
                }
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // reset pixels
        memset(state.canvas.pixels, BACKGROUND_COLOR_GREY, state.canvas.height * state.canvas.width * state.canvas.bytesPerPixel);

        UpdateAndDrawApp(&state, &input);
        DrawBitmap(dc, &bitmapInfo, &state.canvas);

        Stop(FrameTotal);

        // if (framesPrinted < FRAMES_TO_PRINT)
        {
            PrintFrameStats();
            framesPrinted++;
            
        }
        ResetMetrics();
        //TODO: add proper FPS support. Be very carefull to collect input and react on the same frame when input hapenned
        Sleep(14);
    }
}