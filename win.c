#include <windows.h>

int isRunning = 0;

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
        EndPaint(window, &paint);
    }

    return DefWindowProc(window, message, wParam, lParam);
}

HWND OpenGameWindow(HINSTANCE instance)
{
    WNDCLASS windowClass = {
        .hInstance = instance,
        .lpfnWndProc = OnEvent,
        .lpszClassName = "MyWindow",
        .style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC,
        .hCursor = LoadCursor(0, IDC_ARROW),
    };
    RegisterClassA(&windowClass);

    HDC dc = GetDC(0);
    int screenWidth = GetDeviceCaps(dc, HORZRES);

    int windowWidth = 800;
    int windowHeight = 600;
    return CreateWindowA(windowClass.lpszClassName, "Cymbolism", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                         /* x */ screenWidth - windowWidth - 25,
                         /* y */ 25,
                         /* w */ windowWidth,
                         /* h */ windowHeight,
                         0, 0, instance, 0);
}

int wWinMain(HINSTANCE instance, HINSTANCE prev, PWSTR cmdLine, int showCode)
{
    HWND window = OpenGameWindow(instance);
    isRunning = 1;
    while (isRunning)
    {
        MSG msg;

        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}