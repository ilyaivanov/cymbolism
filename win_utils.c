#include <windows.h>
#include "types.h"
#include <dwmapi.h>

#define EDITOR_DEFAULT_WINDOW_STYLE (WS_OVERLAPPEDWINDOW)

//
// File API
//

FileContent ReadMyFileImp(char* path)
{
    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    LARGE_INTEGER size;
    GetFileSizeEx(file, &size);

    u32 fileSize = (u32)size.QuadPart;

    void *buffer = VirtualAlloc(0, fileSize, MEM_COMMIT, PAGE_READWRITE);

    DWORD bytesRead;
    ReadFile(file, buffer, fileSize, &bytesRead, 0);
    CloseHandle(file);

    return (FileContent){.content = buffer, .size = bytesRead};
}


void WriteMyFile(char *path, char* content, int size)
{
    HANDLE file = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

    DWORD bytesWritten;
    int res = WriteFile(file, content, size, &bytesWritten, 0);
    CloseHandle(file);

    MyAssert(bytesWritten == size);
}


//
// BITMAP
//

void InitBitmapInfo(BITMAPINFO * bitmapInfo, u32 width, u32 height)
{
    bitmapInfo->bmiHeader.biSize = sizeof(bitmapInfo->bmiHeader);
    bitmapInfo->bmiHeader.biBitCount = 32;
    bitmapInfo->bmiHeader.biWidth = width;
    bitmapInfo->bmiHeader.biHeight = -height; // makes rows go up, instead of going down by default
    bitmapInfo->bmiHeader.biPlanes = 1;
    bitmapInfo->bmiHeader.biCompression = BI_RGB;
}

void OnResize(HWND window, BITMAPINFO *bitmapInfo, MyBitmap *bitmap)
{
    RECT rect;
    GetClientRect(window, &rect);

    if (bitmap->pixels)
    {
        VirtualFree(bitmap->pixels, 0, MEM_RELEASE);
    }

    bitmap->width = rect.right - rect.left;
    bitmap->height = rect.bottom - rect.top;
    bitmap->bytesPerPixel = 4;

    InitBitmapInfo(bitmapInfo, bitmap->width, bitmap->height);

    i32 size = bitmap->width * bitmap->height * bitmap->bytesPerPixel;
    bitmap->pixels = VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);
}


inline void DrawBitmap(HDC dc, BITMAPINFO *bitmapInfo, MyBitmap *bitmap){
    StretchDIBits(dc,
                  0, 0, bitmap->width, bitmap->height,
                  0, 0, bitmap->width, bitmap->height,
                  bitmap->pixels, bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}


//
// WINDOW
//

HWND OpenGameWindow(HINSTANCE instance, WNDPROC OnEvent)
{
    WNDCLASSW windowClass = {
        .hInstance = instance,
        .lpfnWndProc = OnEvent,
        .lpszClassName = L"MyWindow",
        .style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC,
        .hCursor = LoadCursor(0, IDC_ARROW),
    };
    RegisterClassW(&windowClass);

    HDC dc = GetDC(0);
    int screenWidth = GetDeviceCaps(dc, HORZRES);

    int windowWidth = 1000;
    int windowHeight = 1200;
    HWND window = CreateWindowW(windowClass.lpszClassName, L"Cymbolism", EDITOR_DEFAULT_WINDOW_STYLE | WS_VISIBLE,
                         /* x */ screenWidth - windowWidth - 10,
                         /* y */ 10,
                         /* w */ windowWidth,
                         /* h */ windowHeight,
                         0, 0, instance, 0);

    BOOL USE_DARK_MODE = TRUE;
    BOOL SET_IMMERSIVE_DARK_MODE_SUCCESS = SUCCEEDED(DwmSetWindowAttribute(
        window, DWMWA_USE_IMMERSIVE_DARK_MODE, &USE_DARK_MODE, sizeof(USE_DARK_MODE)));

    return window;
}

// taken from https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
WINDOWPLACEMENT prevWindowDimensions = {sizeof(prevWindowDimensions)};
void ToggleFullscreen(HWND window, i32 isFullscreen)
{
    DWORD style = GetWindowLong(window, GWL_STYLE);
    if (style & EDITOR_DEFAULT_WINDOW_STYLE)
    {
        MONITORINFO monitorInfo = {sizeof(monitorInfo)};
        if (GetWindowPlacement(window, &prevWindowDimensions) &&
            GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
        {
            SetWindowLong(window, GWL_STYLE, style & ~EDITOR_DEFAULT_WINDOW_STYLE);

            SetWindowPos(window, HWND_TOP,
                         monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                         monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                         monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(window, GWL_STYLE, style | EDITOR_DEFAULT_WINDOW_STYLE);
        SetWindowPlacement(window, &prevWindowDimensions);
        SetWindowPos(window, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

//
// DPI Scaling
// user32.dll is linked statically, so dynamic linking won't load that dll again
// taken from https://github.com/cmuratori/refterm/blob/main/refterm.c#L80
//

typedef BOOL WINAPI set_process_dpi_aware(void);
typedef BOOL WINAPI set_process_dpi_awareness_context(DPI_AWARENESS_CONTEXT);
static void PreventWindowsDPIScaling()
{
    HMODULE WinUser = LoadLibraryW(L"user32.dll");
    set_process_dpi_awareness_context *SetProcessDPIAwarenessContext = (set_process_dpi_awareness_context *)GetProcAddress(WinUser, "SetProcessDPIAwarenessContext");
    if (SetProcessDPIAwarenessContext)
    {
        SetProcessDPIAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
    }
    else
    {
        set_process_dpi_aware *SetProcessDPIAware = (set_process_dpi_aware *)GetProcAddress(WinUser, "SetProcessDPIAware");
        if (SetProcessDPIAware)
        {
            SetProcessDPIAware();
        }
    }
}