@ECHO OFF
set command=g++ video_server.cpp -o build/video_server.exe -I C:\Users\smash\Downloads\boost_1_82_0 -l ws2_32
if "%1" == "release" (
    set command=%command% -O3
) else (
    set command=%command% -g
)
%command%