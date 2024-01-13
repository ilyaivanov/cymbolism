#include <windows.h>
#include <gl/gl.h>
float SYSTEM_SCALE = 1;
#define PX(val) ((val) * SYSTEM_SCALE)

#include "..\math.c"
#include "..\win32.c"
#include "..\font.c"

#include "..\input.c"
#include "..\layout.c"
// #include "item.c"

#define FILE_PATH "..\\editor\\main.c"
// #define TMP_FILE_PATH "..\\data_tmp.txt"
// f32 appTime;
// #include "ui.c"

int isRunning = 1;

V2i clientAreaSize;
Layout layout;
UserInput userInput;
FileContent file;
i32 lines[256];
i32 linesCount;

i32 cursorPosition;
// Item root;


void SplitIntoLines()
{
    lines[linesCount++] = -1;
    for(int i = 0; i < file.size; i++){
        if(file.content[i] == '\n')
            lines[linesCount++] = i;
    }
}

inline i32 GetCurrentLineIndex()
{
    for(int i = 0; i < linesCount; i++)
    {
        i32 startIndex = lines[i];
        i32 length = ((i == linesCount - 1) ? file.size : lines[i + 1]) - startIndex;
        if((cursorPosition >= startIndex) && (cursorPosition < startIndex + length))
            return i;
    }
    return -1;
}

inline void Render()
{
    if (IsMouseOverCurrentLayout(&layout, &userInput) && layout.pageHeight > 0 && layout.pageHeight > layout.height)
    {
        // pageHeight is calculated on the previous frame, this might be limiting
        layout.offsetY = Clampf32(layout.offsetY + userInput.zDeltaThisFrame, -(layout.pageHeight - layout.height), 0);
    }

    if(userInput.keysPressedhisFrame['L'])
    {
        cursorPosition++;
        while (*(file.content + cursorPosition) < 32 && (cursorPosition < file.size))
            cursorPosition++;
    }

    if(userInput.keysPressedhisFrame['H'])
    {
        cursorPosition--;
        while (*(file.content + cursorPosition) < 32 && (cursorPosition >= 0))
            cursorPosition--;
    }

    if(userInput.keysPressedhisFrame['J'])
    {
        i32 currentLineIndex = GetCurrentLineIndex();
        if(currentLineIndex < linesCount){
            cursorPosition = lines[currentLineIndex + 1] + (cursorPosition - lines[currentLineIndex]);
        }
    }
    if(userInput.keysPressedhisFrame['K'])
    {
        i32 currentLineIndex = GetCurrentLineIndex();
        if(currentLineIndex > 0){
            cursorPosition = lines[currentLineIndex - 1] + (cursorPosition - lines[currentLineIndex]);
        }
    }

    glClearColor(0.1, 0.1, 0.1, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor4f(1, 0, 0, 1);
    DrawRectGLBottomLeft(20, clientAreaSize.y - 20 - 500, 500, 500);

    currentFont = &regularFont;

    float padding = PX(20);
    float x = padding;
    float y = clientAreaSize.y - padding - layout.offsetY;

    i32 currentLineIndex = GetCurrentLineIndex();

    // glColor3f(1, 1, 1);
    // float w = currentFont->textures['W'].width;
    // float h = currentFont->textMetric.tmHeight;
    // DrawRectGLBottomLeft(x + (cursorPosition - lines[currentLineIndex] - 1) * w, y - ((currentLineIndex + 1) * h), w, h);


    glEnable(GL_TEXTURE_2D);
    for(int i = 0; i < linesCount; i++){
        i32 startIndex = lines[i] + 1;
        char * start = file.content + startIndex;
        i32 length = ((i == linesCount - 1) ? file.size : lines[i + 1]) - startIndex;
        y -= currentFont->textMetric.tmHeight;
        

        float lineX = x;
        for(int charIndex = 0; charIndex < length; charIndex++)
        {
            u8 ch = *(start + charIndex);
            // skips new lines and other control symbols
            if(ch >= ' ')
            {
                if(i % 2 == 0)
                    glColor4f(0.8, 0.8, 0.8, 1);
                else
                    glColor4f(0, 0, 0, 1);
                    
                lineX += DrawCharBottomLeft(lineX, y, ch) + (GetKerningValue(ch, *(start + charIndex + 1)));
            }
        }
    }
    layout.pageHeight = linesCount * currentFont->textMetric.tmHeight + padding * 2;
    glDisable(GL_TEXTURE_2D);

    DrawScrollbar(&layout, PX(10));
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
        layout.width = clientAreaSize.x;
        layout.height = clientAreaSize.y;
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
        // if(wParam > 31)
        //     userInput.charEventsThisFrame[userInput.charEventsThisFrameCount++] = wParam;
    }

    return DefWindowProc(window, message, wParam, lParam);
}



int wWinMain(HINSTANCE instance, HINSTANCE prev, PWSTR cmdLine, int showCode)
{
    PreventWindowsDPIScaling();
    file = ReadMyFileImp(FILE_PATH);
    SplitIntoLines();
    // ParseFileContent(&root, file);

    HWND window = OpenAppWindowWithSize(instance, OnEvent, 1000, 1500);
    HDC dc = GetDC(window);

    Win32InitOpenGL(window);
    OnSizeChange(clientAreaSize.x, clientAreaSize.y);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    InitFonts();

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
                if ((!(msg.wParam == VK_F4 && userInput.keyboardState[VK_MENU])))
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
        // appTime += 16.66666f;
    }
}