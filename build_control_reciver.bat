@ECHO OFF
set command=g++ control_reciver.cpp -o build/control_reciver.exe -I C:\Users\smash\Downloads\boost_1_82_0 -l ws2_32
if "%1" == "release" (
    set command=%command% -O3 -mwindows
) else (
    set command=%command% -g
)
%command%