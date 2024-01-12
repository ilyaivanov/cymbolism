#include <windows.h>
#include <gl/gl.h>

float SYSTEM_SCALE = 1;
#define PX(val) ((val) * SYSTEM_SCALE)

#include "math.c"
#include "win32.c"

#include "input.c"
#include "item.c"

#define FILE_PATH "..\\data.txt"
#define TMP_FILE_PATH "..\\data_tmp.txt"
f32 appTime;
#include "ui.c"

int isRunning = 1;

V2i clientAreaSize;

UserInput userInput;
FileContent file;
Item root;



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
    else if (message == WM_CHAR)
    {
        if(wParam > 31)
            userInput.charEventsThisFrame[userInput.charEventsThisFrameCount++] = wParam;
    }

    return DefWindowProc(window, message, wParam, lParam);
}



int wWinMain(HINSTANCE instance, HINSTANCE prev, PWSTR cmdLine, int showCode)
{
    PreventWindowsDPIScaling();
    file = ReadMyFileImp(FILE_PATH);
    ParseFileContent(&root, file);

    HWND window = OpenAppWindowWithSize(instance, OnEvent, 500, 1500);
    HDC dc = GetDC(window);

    Win32InitOpenGL(window);
    OnSizeChange(clientAreaSize.x, clientAreaSize.y);

    Init(&root);

    // ToggleFullscreen(window, 1);
    
    while (isRunning)
    {
        userInput.zDeltaThisFrame = 0;
        memset(&userInput.keysPressedhisFrame, 0, sizeof(userInput.keysPressedhisFrame));
        memset(&userInput.charEventsThisFrame, 0, sizeof(userInput.charEventsThisFrame));
        userInput.charEventsThisFrameCount = 0;


        MSG msg;
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN)
            {
                userInput.keyboardState[msg.wParam] = 1;
                userInput.keysPressedhisFrame[msg.wParam] = 1;

                // prevent OS handling keys like ALT + J
                if (!(msg.wParam == VK_F4 && userInput.keyboardState[VK_MENU]) && !cursor.isEditing)
                    continue;
            }
            if (msg.message == WM_KEYUP || msg.message == WM_SYSKEYUP)
            {
                userInput.keyboardState[msg.wParam] = 0;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        Render();

        SwapBuffers(dc);

        // //TODO: use better timing
        appTime += 16.66666f;
    }
}