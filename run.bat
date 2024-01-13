@echo off
set arg1=%1

set CompilerOptions=/nologo /MD /Zi /FC

set LinkerOptions=user32.lib gdi32.lib winmm.lib dwmapi.lib opengl32.lib


IF "%arg1%" == "l" (
    if exist playground\build rmdir /s /q playground\build
    mkdir playground\build
    pushd playground\build
    cl %CompilerOptions% ..\..\playground\listener.c user32.lib advapi32.lib
    popd
    call .\playground\build\listener.exe
    GOTO terminate
)



if exist build rmdir /s /q build
mkdir build
pushd build

copy ..\res\resources.rc .\resources.rc >> NUL
rc /nologo .\resources.rc 

cl %CompilerOptions% ..\editor\main.c .\resources.res %LinkerOptions%

echo Compiled

IF NOT "%arg1%" == "b" (
    call .\main.exe
)

:end
popd
:terminate
