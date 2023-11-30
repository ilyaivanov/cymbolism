#include <windows.h>
#include "types.h"
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
        UpdateAndDrawApp(&state);
    }
    else if (message == WM_PAINT)
    {
        PAINTSTRUCT paint = {0};
        HDC dc = BeginPaint(window, &paint);
        DrawBitmap(dc, &bitmapInfo, &state.canvas);
        EndPaint(window, &paint);
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
    while (isRunning)
    {
        memset(&input, 0, sizeof(input));

        //TODO: add proper FPS support. Be very carefull to collect input and react on the same frame when input hapenned
        Sleep(14);

        MSG msg;
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_KEYDOWN)
            {
                if (msg.wParam == VK_SPACE)
                {
                    isFullscreen = isFullscreen ? 0 : 1;
                    ToggleFullscreen(window, isFullscreen);
                }
                else if (msg.wParam == VK_DOWN || msg.wParam == 'J')
                    input.downPressed = 1;
             
                else if (msg.wParam == VK_UP || msg.wParam == 'K')
                    input.upPressed = 1;

                else if (msg.wParam == VK_LEFT || msg.wParam == 'H')
                    input.leftPressed = 1;

                else if (msg.wParam == VK_RIGHT || msg.wParam == 'L')
                    input.rightPressed = 1;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // reset pixels
        memset(state.canvas.pixels, BACKGROUND_COLOR_GREY, state.canvas.height * state.canvas.width * state.canvas.bytesPerPixel);

        HandleInput(&input, &state);
        UpdateAndDrawApp(&state);
        DrawBitmap(dc, &bitmapInfo, &state.canvas);
    }
}