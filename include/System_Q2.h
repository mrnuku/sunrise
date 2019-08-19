#ifndef SYSTEM_CHAOS_H
#define SYSTEM_CHAOS_H

#include "predefines.h"

// these used by the Common class
class FileBase;
class CVar;
class LuaState;
template< class type > class List;
template< class type, int baseBlockSize, int minBlockSize > class DynamicBlockAlloc;
class Plane;

#include "BuildInfo.h"
#include "EndianUtil.h"
#include "ThreadingDefs.h"
#include "SysLock.h"
#include "SysSignal.h"
#include "SysThread.h"
#include "Lock.h"
#include "Signal.h"
#include "ThreadProcess.h"
#include "SSE_Data.h"
#include "BTree.h"
#include "Allocator.h"
#include "Vec3.h"

#include "q_shared.h"
#include "qcommon.h"

#include "vector.h"
#include "Matrix.h"
#include "Quat.h"
#include "Angles.h"
#include "Rotation.h"
#include "Plane.h"
#include "Sphere.h"
#include "Bounds.h"
#include "Projection.h"
#include "Box.h"
//#include "Frustum.h"
#include "Pluecker.h"
#include "Winding.h"

#include "BitField.h"
#include "CRC32.h"
#include "LinkList.h"
#include "StaticList.h"
#include "List.h"
#include "CAList.h"
#include "CmdArgs.h"
#define USE_STRING_DATA_ALLOCATOR
#include "Str.h"
#include "WStr.h"
#include "HashIndex.h"
#include "NamedList.h"
#include "PerfMonitor.h"
#include "ThreadBase.h"
#include "StrPool.h"
#include "File.h"
#include "StrList.h"
#include "FileSystem.h"
#include "CmdSystem.h"
#include "Token.h"
#include "Lexer.h"
#include "Parser.h"
#include "Random.h"
#include "Dict.h"
#include "CVarSystem.h"
#include "Hierarchy.h"
#include "BinaryTree.h"
#include "Heap.h"
#include "Timer.h"
#include "MemoryUtil.h"
//#include "VectorWeld.h"

#include "Color.h"
#include "DrawVert.h"
#include "Surface.h"
#include "Surface_Patch.h"
#include "Surface_Polytope.h"
#include "Surface_Polygon.h"

#include "MiscImports.h"

#ifndef DEDICATED_ONLY
#include "GLImports.h"
#include "GLLayer.h"
#include "MovieExport.h"
#include "SVar.h"
#include "MaterialSystem.h"
#include "Renderer.h"
#include "GUIRenderer.h"
#endif

#include "PlaneSet.h"
#include "MapFile.h"
#include "MapSystem.h"
#include "cm.h"
#include "pmove.h"

#ifndef DEDICATED_ONLY
#include "Blip_Buffer.h"
#include "SoundSystem.h"
#endif

#include "LuaStateExtraData.h"
#include "LuaScript.h"

#include "keys.h"

#ifndef DEDICATED_ONLY
#include "screen.h"
#include "input.h"
#include "vid.h"
#include "GUISystem.h"
#endif

#include "g_local.h"
#include "game.h"
#include "server.h"

#ifndef DEDICATED_ONLY
#include "client.h"
#endif

#include "winquake.h"

#endif
