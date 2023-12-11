@echo off
if exist build rmdir /s /q build
mkdir build
pushd build

set CompilerOptions=/nologo /MD /Zi

set LinkerOptions=user32.lib gdi32.lib winmm.lib dwmapi.lib shell32.lib

cl %CompilerOptions% ..\win.c %LinkerOptions%

set arg1=%1
IF NOT "%arg1%" == "b" (
    call .\win.exe
)

popd
