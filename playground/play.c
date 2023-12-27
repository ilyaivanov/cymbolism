#include <windows.h>
#include <stdio.h>
#include "types.h"
#include "memory.c"
#include "performance.c"
#include "win_utils.c"

typedef void UpdateAndDraw(MyBitmap *canvas, AppState *state);

inline u64 Win32GetLastWriteTime(char *Filename)
{
    FILETIME LastWriteTime = {0};

    WIN32_FIND_DATA FindData;
    HANDLE FindHandle = FindFirstFileA(Filename, &FindData);
    if (FindHandle != INVALID_HANDLE_VALUE)
    {
        LastWriteTime = FindData.ftLastWriteTime;
        FindClose(FindHandle);
    }

    return ((u64)LastWriteTime.dwHighDateTime << 32) + (u64)LastWriteTime.dwLowDateTime;
}


char * dllPath = "..\\play2.dll";
char * dllPathCopy = "..\\play2_copy.dll";
MyBitmap canvas = {0};
BITMAPINFO bitmapInfo = {0};

i32 isRunning = 0;
i32 isFullscreen = 0;

u64 lastDLLBuildTime = 0;
HMODULE play2;
UpdateAndDraw *updateAndDraw;

AppState state = {0};

LRESULT OnEvent(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_DESTROY)
    {
        isRunning = 0;
    }
    else if (message == WM_SIZE)
    {
        if(wParam != SIZE_MINIMIZED)
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            OnResize(&bitmapInfo, &canvas, width, height);
        }
        return 0;
    }
    else if (message == WM_PAINT)
    {
        PAINTSTRUCT paint = {0};
        HDC dc = BeginPaint(window, &paint);
        DrawBitmap(dc, &bitmapInfo, &canvas);
        EndPaint(window, &paint);
    }
    return DefWindowProc(window, message, wParam, lParam);
}


void LoadLib()
{
    play2 = LoadLibraryA(dllPathCopy);
    updateAndDraw = (UpdateAndDraw *)GetProcAddress(play2, "UpdateAndDraw");
}


typedef enum ReloadingStatus { Hidden, Reloading, Done } ReloadingStatus;

ReloadingStatus reloadingStatus;
double reloadedTimeMs;
double timeToDisappearMs;
FontData font;

// #define SHOW_RELOAD_MESSAGE

int wWinMain(HINSTANCE instance, HINSTANCE prev, PWSTR cmdLine, int showCode)
{
    u64 procSpeed = 3000000000;
    PreventWindowsDPIScaling();
    timeBeginPeriod(1);

    HWND window = OpenGameWindow(instance, OnEvent);
    HDC dc = GetDC(window);
    InitBitmapInfo(&bitmapInfo, canvas.width, canvas.height);
    InitFontSystem(&font, 16, "Segoe UI");
    isRunning = 1;

    u64 start = __rdtsc();
    OutputDebugStringA("Starting\n");
    while (isRunning)
    {
        MSG msg;
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        u64 dllBuildTime = Win32GetLastWriteTime(dllPath);

        if(dllBuildTime > lastDLLBuildTime)
        {
            if(reloadingStatus != Reloading)
            {
                start = __rdtsc();
                reloadingStatus = Reloading;
            }

            if(play2)
                FreeLibrary(play2);
                
            if (CopyFileA(dllPath, dllPathCopy, 0) != 0)
            {
                lastDLLBuildTime = dllBuildTime;
                timeToDisappearMs = 2000;
                reloadingStatus = Done;
                u64 end = __rdtsc();
                reloadedTimeMs = (double)(end - start) * 1000 / procSpeed;
            }
            
            LoadLib();
        }

        memset(canvas.pixels, 0, canvas.bytesPerPixel * canvas.width * canvas.height);



        if(reloadingStatus == Done)
        {
            timeToDisappearMs -= 16.666666f;
            if(timeToDisappearMs < 0)
            {
                reloadingStatus = Hidden;
            }
        }
        
#ifdef SHOW_RELOAD_MESSAGE
        if(reloadingStatus != Hidden)
        {
            char buff[256];
            i32 len = 0;
            if (reloadingStatus == Done)
                len = sprintf(buff, "Done: %5.2fms", reloadedTimeMs);
            else
                len = sprintf(buff, "Reloading...");

            u32 bgColor = reloadingStatus == Done ? 0x226622 : 0x666622;

            i32 hPadding = 2;
            i32 width = 200; // Should be based on font size, just making it easy
            i32 height = font.textMetric.tmHeight + hPadding * 2;
            DrawRect(&canvas, canvas.width / 2 - width / 2, 0, width, height, bgColor);
            DrawTextCenterBottom(&canvas, &font, canvas.width / 2, height - hPadding - 2, buff, len, 0xffffff);
        }
#endif

        updateAndDraw(&canvas, &state);
        DrawBitmap(dc, &bitmapInfo, &canvas);
        //TODO: add proper FPS support. Be very carefull to collect input and react on the same frame when input hapenned
        Sleep(14);
    }
}