#include <windows.h>
#include "types.h"
#include "win_utils.c"
#include "drawing.c"
#include "gdiFont.c"
#include "game.c"


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
        // UpdateAndDrawApp(&state);
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
    u64 baseCpuFrequency = 3000000000; // 3.0Gz, need to get this from an API
    u64 ticksPerFrame = baseCpuFrequency / 60;

    PreventWindowsDPIScaling();
    timeBeginPeriod(1);


    InitFontSystem(&state.fonts.regular, 100, "Segoe UI");
    // InitApp(&state);

    HWND window = OpenGameWindow(instance, OnEvent);
    if(isFullscreen)
        ToggleFullscreen(window, isFullscreen);

    HDC dc = GetDC(window);

    isRunning = 1;
    
    u64 startTicks = __rdtsc();

    while (isRunning)
    {

        memset(&input.keysPressed, 0, sizeof(input.keysPressed));

        //TODO: add proper FPS support. Be very carefull to collect input and react on the same frame when input hapenned
        Sleep(14);

        MSG msg;
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_KEYDOWN)
            {
                input.isPressed[msg.wParam] = 1;

                if (msg.wParam == VK_SPACE)
                {
                    isFullscreen = isFullscreen ? 0 : 1;
                    ToggleFullscreen(window, isFullscreen);
                }
                
                if(msg.wParam < 256)
                {
                    input.keysPressed[msg.wParam] = 1;
                }
            }
            else if (msg.message == WM_KEYUP)
            {
                input.isPressed[msg.wParam] = 0;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        u64 endTicks = __rdtsc();
        u64 ellapsedTicks = endTicks - startTicks;
        while(ellapsedTicks <= ticksPerFrame)
        {
            endTicks = __rdtsc();
            ellapsedTicks = endTicks - startTicks;
        }
        // reset pixels
        memset(state.canvas.pixels, BACKGROUND_COLOR_GREY, state.canvas.height * state.canvas.width * state.canvas.bytesPerPixel);

        // HandleInput(&input, &state);
        UpdateAndDrawApp(&state, &input);
        DrawBitmap(dc, &bitmapInfo, &state.canvas);

        startTicks = endTicks;
    }
}