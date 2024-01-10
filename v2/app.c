#include <windows.h>
#include <gl/gl.h>

float SYSTEM_SCALE = 1;
#define PX(val) ((val) * SYSTEM_SCALE)

#include "math.c"
#include "win32.c"

#include "input.c"
#include "item.c"

f32 appTime;
Item *selectedItem;
#include "ui.c"

int isRunning = 1;

V2i clientAreaSize;

UserInput userInput;
FileContent file;
Item root;


#define FILE_PATH "..\\data.txt"

inline void Render()
{
    DrawUI(clientAreaSize, &root, &userInput);
}

LRESULT OnEvent(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_DESTROY)
    {
        isRunning = 0;
    }
    else if (message == WM_SIZE)
    {
        clientAreaSize.x = LOWORD(lParam);
        clientAreaSize.y = HIWORD(lParam);
        OnSizeChange(clientAreaSize.x, clientAreaSize.y);

        HDC dc = GetDC(window);
        SYSTEM_SCALE = (float)GetDeviceCaps(dc, LOGPIXELSY) / (float)USER_DEFAULT_SCREEN_DPI;

        InvalidateRect(window, NULL, TRUE);
    }
    else if (message == WM_MOUSEMOVE)
    {
        userInput.mouseX = (float)LOWORD(lParam);
        userInput.mouseY = (float)(clientAreaSize.y - HIWORD(lParam));
    }
    else if (message == WM_KEYDOWN)
    {
        userInput.keysPressedhisFrame[wParam] = 1;
    }
    else if (message == WM_MOUSEWHEEL)
    {
        userInput.zDeltaThisFrame += (float)GET_WHEEL_DELTA_WPARAM(wParam);
    }
    else if (message == WM_PAINT)
    {
        PAINTSTRUCT paint = {0};
        HDC dc = BeginPaint(window, &paint);
        EndPaint(window, &paint);

        Render();
        SwapBuffers(dc);
    }

    return DefWindowProc(window, message, wParam, lParam);
}



int wWinMain(HINSTANCE instance, HINSTANCE prev, PWSTR cmdLine, int showCode)
{
    PreventWindowsDPIScaling();
    file = ReadMyFileImp("..\\data.txt");
    ParseFileContent(&root, file);
    selectedItem = GetChildAt(&root, 0);

    HWND window = OpenAppWindowWithSize(instance, OnEvent, 1500, 1500);
    HDC dc = GetDC(window);

    Win32InitOpenGL(window);
    OnSizeChange(clientAreaSize.x, clientAreaSize.y);

    Init();

    // ToggleFullscreen(window, 1);
    
    while (isRunning)
    {
        userInput.zDeltaThisFrame = 0;
        memset(&userInput.keysPressedhisFrame, 0, sizeof(userInput.keysPressedhisFrame));

        MSG msg;
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        GetKeyboardState(userInput.keyboardState);

        

        Render();

        SwapBuffers(dc);

        // //TODO: use better timing
        appTime += 16.66666f;
    }
}