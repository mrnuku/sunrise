
#define	VERSION		"$Rev: 27 $"

#ifdef NDEBUG
#define BUILDSTRING "Release"
#else
#define BUILDSTRING "Debug"
#endif

#ifdef _WIN64

#ifdef _M_IA64
#define	CPUSTRING	"IA64"
#elif defined _M_AMD64
#define	CPUSTRING	"AMD64"
#else
#define CPUSTRING	"x64"
#endif

#elif defined _WIN32

#ifdef _M_IX86
#define	CPUSTRING	"x86"
#endif

#elif defined __linux__

#ifdef __i386__
#define CPUSTRING "i386"
#else
#define CPUSTRING "Unknown"
#endif

#else	// !WIN32

#define	CPUSTRING	"Unknow architecture"

#endif
