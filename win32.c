#ifndef WIN32_C
#define WIN32_C

#include <windows.h>

// used to set dark mode
#include <dwmapi.h>
#include "math.c"
#include "memory.c"

#define EDITOR_DEFAULT_WINDOW_STYLE (WS_OVERLAPPEDWINDOW)

void Win32InitOpenGL(HWND Window)
{
    HDC WindowDC = GetDC(Window);

    PIXELFORMATDESCRIPTOR DesiredPixelFormat = {0};
    DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
    DesiredPixelFormat.nVersion = 1;
    DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
    DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
    DesiredPixelFormat.cColorBits = 32;
    DesiredPixelFormat.cAlphaBits = 8;
    DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

    int SuggestedPixelFormatIndex = ChoosePixelFormat(WindowDC, &DesiredPixelFormat);
    PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
    DescribePixelFormat(WindowDC, SuggestedPixelFormatIndex,
                        sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);
    SetPixelFormat(WindowDC, SuggestedPixelFormatIndex, &SuggestedPixelFormat);
    
    HGLRC OpenGLRC = wglCreateContext(WindowDC);
    if(!wglMakeCurrent(WindowDC, OpenGLRC))        
        Fail("Failed to initialize OpenGL");

    ReleaseDC(Window, WindowDC);
}

void OnSizeChange(i32 width, i32 height)
{
    glViewport(0, 0, width, height);

    // allows me to set vecrtex coords as 0..width/height, instead of -1..+1
    // 0,0 is bottom left, not top left
    // matrix in code != matrix in math notation, details at https://youtu.be/kBuaCqaCYwE?t=3084
    // in short: rows in math are columns in code
    float w = 2.0f / width;
    float h = 2.0f / height;
    GLfloat m[] = {
         w,  0,  0,  0,   
         0,  h,  0,  0,   
         0,  0,  1,  0,   
        -1, -1,  0,  1,   
    };
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m);
}

HWND OpenAppWindowWithSize(HINSTANCE instance, WNDPROC OnEvent, int windowWidth, int windowHeight)
{
    WNDCLASSW windowClass = {
        .hInstance = instance,
        .lpfnWndProc = OnEvent,
        .lpszClassName = L"MyWindow",
        .style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC,
        .hCursor = LoadCursor(0, IDC_ARROW),
        // not using COLOR_WINDOW + 1 because it doesn't fucking work
        // this line fixes a flash of a white background for 1-2 frames during start
        .hbrBackground = CreateSolidBrush(0x111111),
    };
    RegisterClassW(&windowClass);

    HDC dc = GetDC(0);
    int screenWidth = GetDeviceCaps(dc, HORZRES);
    int screenHeight = GetDeviceCaps(dc, VERTRES);

    HWND window = CreateWindowW(windowClass.lpszClassName, (wchar_t*)"Editor", EDITOR_DEFAULT_WINDOW_STYLE | WS_VISIBLE,
                         /* x */ screenWidth - windowWidth - 20,
                         /* y */ 0 + 20,
                         /* w */ windowWidth,
                         /* h */ windowHeight,
                         0, 0, instance, 0);

    BOOL USE_DARK_MODE = TRUE;
    BOOL SET_IMMERSIVE_DARK_MODE_SUCCESS = SUCCEEDED(DwmSetWindowAttribute(
        window, DWMWA_USE_IMMERSIVE_DARK_MODE, &USE_DARK_MODE, sizeof(USE_DARK_MODE)));

    DeleteObject(windowClass.hbrBackground);    
    return window;
}

void InitBitmapInfo(BITMAPINFO * bitmapInfo, u32 width, u32 height)
{
    bitmapInfo->bmiHeader.biSize = sizeof(bitmapInfo->bmiHeader);
    bitmapInfo->bmiHeader.biBitCount = 32;
    bitmapInfo->bmiHeader.biWidth = width;
    bitmapInfo->bmiHeader.biHeight = height; // makes rows go up, instead of going down by default
    bitmapInfo->bmiHeader.biPlanes = 1;
    bitmapInfo->bmiHeader.biCompression = BI_RGB;
}


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

//
// File IO
//

typedef struct FileContent
{
    char *content;
    i32 size;
} FileContent;


FileContent ReadMyFileImp(char* path)
{
    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    LARGE_INTEGER size;
    GetFileSizeEx(file, &size);

    u32 fileSize = (u32)size.QuadPart;

    void *buffer = VirtualAllocateMemory(fileSize);

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

    Assert(bytesWritten == size);
}


#endif