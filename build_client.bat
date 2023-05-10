@ECHO OFF
set command=g++ client.cpp -o build/client.exe -I C:\Users\smash\Downloads\boost_1_82_0 -l ws2_32 -l Gdiplus -l Shlwapi -municode
if "%1" == "release" (
    set command=%command% -O3
) else (
    set command=%command% -g
)
%command%