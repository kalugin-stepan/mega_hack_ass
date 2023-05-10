@ECHO OFF
set command=g++ installer.cpp -o build/installer.exe
if "%1" == "release" (
    set command=%command% -O3
) else (
    set command=%command% -g
)
%command%