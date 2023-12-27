@echo off
if exist build rmdir /s /q build
mkdir build
pushd build

set arg1=%1

set CompilerOptions=/nologo /MD /Zi

set LinkerOptions=user32.lib gdi32.lib winmm.lib dwmapi.lib 


IF "%arg1%" == "u" (
    cl %CompilerOptions% ..\unit.c %LinkerOptions%
    call .\unit.exe
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
