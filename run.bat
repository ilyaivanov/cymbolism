@echo off
if exist build rmdir /s /q build
mkdir build
pushd build


copy ..\res\resources.rc .\resources.rc >> NUL
rc /nologo .\resources.rc 

set CompilerOptions=/nologo /MD /Zi

set LinkerOptions=user32.lib gdi32.lib winmm.lib dwmapi.lib 

cl %CompilerOptions% ..\win.c .\resources.res %LinkerOptions% >> NUL

echo Compiled

set arg1=%1
IF NOT "%arg1%" == "b" (
    call .\win.exe
)

popd
