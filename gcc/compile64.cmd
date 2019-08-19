x86_64-w64-mingw32-g++ -fpermissive -O1 ..\include\precompiled.h -I..\dxsdk\Include -I..\magick
x86_64-w64-mingw32-g++ -fpermissive -O1 -c -I..\include ..\client\*.cpp
x86_64-w64-mingw32-g++ -fpermissive -O1 -c -I..\include ..\d3\*.cpp
x86_64-w64-mingw32-g++ -fpermissive -O1 -c -I..\include ..\game\*.cpp
x86_64-w64-mingw32-g++ -fpermissive -O1 -c -I..\include ..\map\*.cpp
x86_64-w64-mingw32-g++ -fpermissive -O1 -c -I..\include ..\qcommon\*.cpp
x86_64-w64-mingw32-g++ -fpermissive -O1 -c -I..\include ..\renderer\*.cpp
x86_64-w64-mingw32-g++ -fpermissive -O1 -c -I..\include ..\server\*.cpp
x86_64-w64-mingw32-g++ -fpermissive -O1 -c -I..\include ..\win32\*.cpp
del ..\include\precompiled.h.gch
x86_64-w64-mingw32-g++ -fpermissive -O1 -c -I..\include -I..\dxsdk\Include -I..\magick ..\typeinfo\*.cpp
x86_64-w64-mingw32-windres -o q2.o ..\include\q2.rc
x86_64-w64-mingw32-g++ -s -o chs_gcc64.exe *.o -mwindows -lwinmm -lws2_32 -L..\magick\lib64 -lCORE_RL_magick_