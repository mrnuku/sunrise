md client
md d3
md game
md map
md qcommon
md renderer
md server
md win32
md lib
g++ -msse -msse2 -O1 -D_WIN32_WINNT=0x0601 ..\include\precompiled.h -I..\dxsdk\Include -I..\magick -I..\zlib -I..\faac -I..\include -I..\cegui\include -I..\openal
cd client
g++ -msse -msse2 -O1 -c -D_WIN32_WINNT=0x0601 -I..\..\include ..\..\client\*.cpp
cd ..\d3
g++ -msse -msse2 -O1 -c -D_WIN32_WINNT=0x0601 -I..\..\include ..\..\d3\*.cpp
cd ..\game
g++ -msse -msse2 -O1 -c -D_WIN32_WINNT=0x0601 -I..\..\include ..\..\game\*.cpp
cd ..\map
g++ -msse -msse2 -O1 -c -D_WIN32_WINNT=0x0601 -I..\..\include ..\..\map\*.cpp
cd ..\qcommon
g++ -msse -msse2 -O1 -c -D_WIN32_WINNT=0x0601 -I..\..\include ..\..\qcommon\*.cpp
cd ..\renderer
g++ -msse -msse2 -O1 -c -D_WIN32_WINNT=0x0601 -I..\..\include ..\..\renderer\*.cpp
cd ..\server
g++ -msse -msse2 -O1 -c -D_WIN32_WINNT=0x0601 -I..\..\include ..\..\server\*.cpp
cd ..\win32
g++ -msse -msse2 -O1 -c -D_WIN32_WINNT=0x0601 -I..\..\include ..\..\win32\*.cpp
cd ..\lib
g++ -msse -msse2 -O1 -c -D_WIN32_WINNT=0x0601 -I..\..\include ..\..\expat\*.cpp
g++ -msse -msse2 -O1 -c -D_WIN32_WINNT=0x0601 -I..\..\include ..\..\lua\*.cpp
g++ -msse -msse2 -O1 -c -D_WIN32_WINNT=0x0601 -I..\..\include ..\..\zlib\*.cpp
cd ..
del ..\include\precompiled.h.gch
g++ -msse -msse2 -O1 -c -D_WIN32_WINNT=0x0601 -I..\dxsdk\Include -I..\magick -I..\zlib -I..\faac -I..\include -I..\cegui\include -I..\openal ..\typeinfo\*.cpp
windres -o q2.o ..\include\q2.rc
g++ -s -o chs.exe cegui\*.o client\*.o d3\*.o game\*.o lib\*.o map\*.o qcommon\*.o renderer\*.o server\*.o win32\*.o *.o -mwindows -lwinmm -lws2_32