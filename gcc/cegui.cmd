md cegui
cd cegui
g++ -msse -msse2 -O1 -c -DCEGUI_STATIC -DHAVE_CONFIG_H -DCEGUIBASE_EXPORTS -DTOLUA_STATIC -I..\..\stlport\stlport -I..\..\cegui\include ..\..\cegui\src\*.cpp
g++ -msse -msse2 -O1 -c -DCEGUI_STATIC -DHAVE_CONFIG_H -DCEGUIBASE_EXPORTS -DTOLUA_STATIC -I..\..\stlport\stlport -I..\..\cegui\include ..\..\cegui\src\elements\*.cpp
g++ -msse -msse2 -O1 -c -DCEGUI_STATIC -DHAVE_CONFIG_H -DCEGUIBASE_EXPORTS -DTOLUA_STATIC -I..\..\stlport\stlport -I..\..\cegui\include ..\..\cegui\src\falagard\*.cpp
g++ -msse -msse2 -O1 -c -DCEGUI_STATIC -DHAVE_CONFIG_H -DCEGUIBASE_EXPORTS -DTOLUA_STATIC -I..\..\stlport\stlport -I..\..\cegui\include -I..\..\cegui\include\WindowRendererSets\Falagard ..\..\cegui\src\WindowRendererSets\Falagard\*.cpp
cd ..
