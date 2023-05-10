@ECHO OFF
set command=g++ control_server.cpp -o build/control_server.exe -I C:\Users\smash\Downloads\boost_1_82_0 -l ws2_32
if "%1" == "release" (
    set command=%command% -O3
) else (
    set command=%command% -g
)
%command%