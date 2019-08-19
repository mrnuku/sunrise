#ifndef SYSTEM_OS_H
#define SYSTEM_OS_H

#ifndef _WIN32
#include <pthread.h>
#endif

#if defined _MSC_VER && defined _DEBUG
#include <crtdbg.h>
#endif

namespace WindowsNS {
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winperf.h>
#include <winsock2.h>
#include <Vfw.h>
#ifdef LoadString
#undef LoadString
#endif
}

#endif
