g++ -ggdb -c -I..\magick -DSTATIC_MAGICK -D_MAGICK_CONFIG_H "..\magick\Magick++\lib\*.cpp"
g++ -ggdb ..\include\precompiled.h -I..\dxsdk\Include -I..\magick
g++ -ggdb -c -I..\include ..\client\*.cpp
g++ -ggdb -c -I..\include ..\d3\*.cpp
g++ -ggdb -c -I..\include ..\game\*.cpp
g++ -ggdb -c -I..\include ..\map\*.cpp
g++ -ggdb -c -I..\include ..\qcommon\*.cpp
g++ -ggdb -c -I..\include ..\renderer\*.cpp
g++ -ggdb -c -I..\include ..\server\*.cpp
g++ -ggdb -c -I..\include ..\win32\*.cpp
del ..\include\precompiled.h.gch
g++ -ggdb -c -I..\include -I..\dxsdk\Include -I..\magick ..\typeinfo\*.cpp
windres -o q2.o ..\include\q2.rc
g++ -o chs_gcc_deb.exe *.o -mwindows -lwinmm -lws2_32 -L..\magick\lib32 -lCORE_RL_magick_ -lCORE_RL_wand_