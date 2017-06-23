@echo off

set scriptdir=%cd%
set cpldirX=%scriptdir%\cpl\%date%
set cpldir=%cpldirX:/=.%
set incdir=%cd%\inc
set libdir=%cd%\lib
echo Compiling Asteroids Source
cd src
g++ -c -std=c++14 -Wall -Wextra -pedantic-errors -O3 *.cpp -I "%incdir%"
echo Building Asteroids (via *.o)

if not exist "%cpldir%" mkdir "%cpldir%"
for %%o in (.o) do move "*%%o" "%cpldir%"
cd "%cpldir%"

set lnkdir=%cpldir%\lnk

g++ -O3 -static-libgcc -static-libstdc++ -o asteroids.exe *.o "%scriptdir%\res\exe\asteroids.res" -L"%libdir%" -lOpenGL32 -lSDL2 -lSDL2_mixer -lSDL2main -lmdl -ltopazdll
move asteroids.exe %lnkdir%
cd "%scriptdir%\res\dep"
xcopy /s "%cd%" %lnkdir%
echo GUI Dynamic Link Library build attempted (asteroids.exe). Ensure that the process was successful!
pause