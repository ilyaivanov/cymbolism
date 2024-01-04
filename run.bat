@echo off
set arg1=%1

set CompilerOptions=/nologo /MD /Zi

set LinkerOptions=user32.lib gdi32.lib winmm.lib dwmapi.lib 


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


IF "%arg1%" == "u" (
    cl %CompilerOptions% ..\playground\unit.c %LinkerOptions%
    call .\unit.exe
    GOTO end
)

IF "%arg1%" == "gl" (
    cl %CompilerOptions% ..\playground\geometryWars.c %LinkerOptions% opengl32.lib
    call .\geometryWars.exe
    GOTO end
)


IF "%arg1%" == "p" (
    cl %CompilerOptions% ..\play.c %LinkerOptions%
    call .\play.exe
    GOTO end
)

copy ..\res\resources.rc .\resources.rc >> NUL
rc /nologo .\resources.rc 

cl %CompilerOptions% ..\win.c .\resources.res %LinkerOptions%

echo Compiled

IF NOT "%arg1%" == "b" (
    call .\win.exe
)

:end
popd
:terminate
