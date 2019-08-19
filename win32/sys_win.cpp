#include "precompiled.h"
#pragma hdrstop

#define DWORD				WindowsNS::DWORD
#define WORD				WindowsNS::WORD

bool			System::console_clearprev = false;
#ifdef MM_TIMER
timeType		System::baseTime;
#else
timeTypeSystem	System::baseTime = 0;
timeTypeReal	System::baseFreq = 0;
timeTypeSystem	System::prevCounter = 0;
timeTypeSystem	System::prevPerformance = 0;
int				System::cpuCount = 0;
#endif
timeType		System::lastTime;
bool			System::initialized = false;

dword			System::g_msgTimeBase = 0;
dword			System::g_msgLast = 0;

#define MINIMUM_WIN_MEMORY	0x0A00000
#define MAXIMUM_WIN_MEMORY	0x1000000

//#define DEMO

int			starttime;
bool		ActiveApp, Minimized;

static WindowsNS::HANDLE		hinput, houtput;

#define	MAX_NUM_ARGVS	128
int			argc;
char		* argv[ MAX_NUM_ARGVS ];

void System::Sys_Error( const char * error, ... ) {
	va_list		argptr;
	Str			text;
#ifndef DEDICATED_ONLY
	if( !Common::dedicated.GetBool( ) )
		Client::CL_Shutdown( );
#endif
	va_start( argptr, error );
	vsprintf( text, error, argptr );
	va_end( argptr );
	if( !Sys_HaveConsole( ) ) {
		Sys_CreateConsole( );
		Sys_ShowConsole( 1, false );
	}
	//WindowsNS::MessageBox( NULL, text.c_str( ), "Error", 0 /* MB_OK */ );
	Win_SetErrorText( text.c_str( ) );
	// shut down QHOST hooks if necessary
	//DeinitConProc( );
	Sys_DestroyConsole( );
	exit( 1 );
}

void System::Sys_Quit( ) {
#ifdef MM_TIMER
	WindowsNS::timeEndPeriod( 10 );
#endif
	//if( Common::dedicated.GetBool( ) )
	//	WindowsNS::FreeConsole( );
	// shut down QHOST hooks if necessary
	//DeinitConProc( );
	Sys_DestroyConsole( );
	exit( 0 );
}

void WinError( ) {
	WindowsNS::LPVOID lpMsgBuf;
	WindowsNS::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WindowsNS::GetLastError( ), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( WindowsNS::LPTSTR ) &lpMsgBuf, 0, NULL );
	// Display the string.
	WindowsNS::MessageBox( NULL, ( WindowsNS::LPCSTR )lpMsgBuf, "GetLastError", MB_OK | MB_ICONINFORMATION );
	// Free the buffer.
	WindowsNS::LocalFree( lpMsgBuf );
}

void System::Sys_Init( ) {
	WindowsNS::OSVERSIONINFO	vinfo;	
	if( !initialized ) {	// let base retain 16 bits of effectively random data
#ifdef MM_TIMER
		baseTime = ( timeType )WindowsNS::timeGetTime( );
#else
		__int64 longVal;
		WindowsNS::QueryPerformanceFrequency( ( WindowsNS::LARGE_INTEGER * )&longVal );
		baseFreq = ( timeTypeReal )( double( longVal ) / 1000.0 );
		WindowsNS::QueryPerformanceCounter( ( WindowsNS::LARGE_INTEGER * )&baseTime );
#endif
		initialized = true;
	}
#ifdef MM_TIMER
	WindowsNS::timeBeginPeriod( 10 );
#endif
	vinfo.dwOSVersionInfoSize = sizeof( vinfo );
	if( !WindowsNS::GetVersionEx( &vinfo ) )
		Sys_Error( "Couldn't get OS info" );
	if( Common::dedicated.GetBool( ) ) {
		//if( !WindowsNS::AllocConsole( ) )
		//	Sys_Error( "Couldn't create dedicated server console" );
		//hinput = WindowsNS::GetStdHandle( STD_INPUT_HANDLE );
		//houtput = WindowsNS::GetStdHandle( STD_OUTPUT_HANDLE );	
		// let QHOST hook in
		//InitConProc( argc, argv );
		Sys_CreateConsole( );
		Sys_ShowConsole( 1, false );
	}
	WindowsNS::SYSTEM_INFO sysinfo;
	WindowsNS::GetSystemInfo( &sysinfo );
	cpuCount = ( int )sysinfo.dwNumberOfProcessors;
	dword sample1 = WindowsNS::GetTickCount( );
	while( ( g_msgTimeBase = WindowsNS::GetTickCount( ) ) == sample1 );
}

void System::Sys_SendKeyEvents( ) {
    WindowsNS::MSG        msg;
	while( WindowsNS::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ) {
		if( !WindowsNS::GetMessage( &msg, NULL, 0, 0 ) )
			Sys_Quit( );
		//if( !g_msgTimeBase )
		//	g_msgTimeBase = msg.time;
		g_msgLast = msg.time - g_msgTimeBase;
      	WindowsNS::TranslateMessage( &msg );
      	WindowsNS::DispatchMessage( &msg );
	}
}

const Str System::Sys_GetClipboardData( ) {
	Str ret;
	char * cliptext;
	if( WindowsNS::OpenClipboard( NULL ) != 0 ) {
		WindowsNS::HANDLE hClipboardData;
		if( ( hClipboardData = WindowsNS::GetClipboardData( CF_TEXT ) ) != 0 ) {
			if( ( cliptext = ( char * )WindowsNS::GlobalLock( hClipboardData ) ) != 0 ) {
				ret = cliptext;
				WindowsNS::GlobalUnlock( hClipboardData );
			}
		}
		WindowsNS::CloseClipboard( );
	}
	return ret;
}

void System::Sys_SetClipboardData( const Str & text ) {
	if( text.Length( ) && WindowsNS::OpenClipboard( NULL ) != 0 ) {
		WindowsNS::HGLOBAL hResult = WindowsNS::GlobalAlloc( GMEM_MOVEABLE, ( text.Length( ) + 1 ) * 2 );
		wchar_t * lptstrCopy = ( wchar_t * )WindowsNS::GlobalLock( hResult );
		//Common::Com_Memcpy( lptstrCopy, text.c_str( ), text.Length( ) + 1 );
		size_t ret = mbstowcs( lptstrCopy, text.c_str( ), text.Length( ) + 1 );
		WindowsNS::GlobalUnlock( hResult );
		if( WindowsNS::SetClipboardData( CF_UNICODETEXT, hResult ) == NULL ) {
			 WindowsNS::GlobalFree( hResult );
			 WindowsNS::CloseClipboard( );
			 return;
		}
		WindowsNS::CloseClipboard( );
	}
}

void System::Sys_AppActivate( ) {
#ifndef DEDICATED_ONLY
	WindowsNS::ShowWindow( cl_hwnd, SW_RESTORE );
	WindowsNS::SetForegroundWindow( cl_hwnd );
#endif
}

float System::CpuUsage( ) {
	PerfData perfData;
	PerfCounter counterProcessID, counterProcessorTime;
	DWORD procIdReq = WindowsNS::GetCurrentProcessId( );
	//perfData.PrintHelp( 230 );
	/*perfData.PrintHelp( 236 );
	perfData.PrintHelp( 4 );
	perfData.PrintHelp( 6 );
	perfData.PrintHelp( 24 );*/
	for( PerfObject perfObject = perfData.Query( 230 ); perfObject.IsValid( ); perfObject++ ) {
		for( PerfCounter perfCounter = perfObject.FirstCounter( ); perfCounter.IsValid( ); perfCounter++ ) {
			int perfCounterTitleIndex = perfCounter.GetNameIndex( );
			if( perfCounterTitleIndex == 784 )
				counterProcessID = perfCounter;
			if( perfCounterTitleIndex == 6 )
				counterProcessorTime = perfCounter;
			if( counterProcessID.IsValid( ) && counterProcessorTime.IsValid( ) ) {
				for( PerfInstance perfInstance = perfObject.FirstInstance( ); perfInstance.IsValid( ); perfInstance++ ) {
					PerfCounterBlock perfCounterBlock = perfInstance.GetBlock( );
					if( *( DWORD * )perfCounterBlock.GetCounterDword( counterProcessID ) == procIdReq ) {
						__int64	perfDelta = *perfCounterBlock.GetCounter( counterProcessorTime ) - prevCounter;
						__int64	perfDeltaTime = *perfData.GetTime100ns( ) - prevPerformance;
						prevCounter = *perfCounterBlock.GetCounter( counterProcessorTime );
						prevPerformance = *perfData.GetTime100ns( );
						float usage = ( float )( ( ( double )perfDelta / ( double )perfDeltaTime ) * ( 100.0 / cpuCount ) );
						return ( usage < 0.0f ) ? 0.0f : usage;
					}
				}
			}
		}
	}
	return -1.0f;
}

void ParseCommandLine( WindowsNS::LPSTR lpCmdLine ) {
	argc = 1;
	argv[ 0 ] = "exe";
	while( *lpCmdLine &&( argc < MAX_NUM_ARGVS ) ) {
		while( *lpCmdLine &&( ( *lpCmdLine <= 32 ) ||( *lpCmdLine > 126 ) ) )
			lpCmdLine++;
		if( *lpCmdLine ) {
			argv[ argc ] = lpCmdLine;
			argc++;
			while( *lpCmdLine &&( ( *lpCmdLine > 32 ) &&( *lpCmdLine <= 126 ) ) )
				lpCmdLine++;
			if( *lpCmdLine ) {
				* lpCmdLine = 0;
				lpCmdLine++;
			}			
		}
	}
}

WindowsNS::HINSTANCE	global_hInstance;

extern "C" {

int WINAPI WinMain( WindowsNS::HINSTANCE hInstance, WindowsNS::HINSTANCE hPrevInstance, WindowsNS::LPSTR lpCmdLine, int nCmdShow ) {
	timeType					time;
#ifdef MM_TIMER
	DWORD						oldtime, newtime;
#else
	__int64						oldtime, newtime;
	//timeType					freq;
#endif
#if defined _MSC_VER && defined _DEBUG
	//_CrtSetDbgFlag( 0 );
	//int flags = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
#endif
	global_hInstance = hInstance;
	ParseCommandLine( lpCmdLine );
	Common::Init( argc, argv );
#ifdef MM_TIMER
	oldtime = WindowsNS::timeGetTime( );
#else
	//WindowsNS::QueryPerformanceFrequency( ( WindowsNS::LARGE_INTEGER * )&oldtime );
	//freq = ( timeType )( double( oldtime ) / 1000.0 );
	//WindowsNS::QueryPerformanceCounter( ( WindowsNS::LARGE_INTEGER * )&oldtime );
	oldtime = System::Sys_Milliseconds( );
#endif
    /* main window message loop */
	while( 1 ) {
		// if at a full screen console, don't update unless needed
		if( Minimized || Common::dedicated.GetBool( ) )
			WindowsNS::Sleep( 10 );
		newtime = System::Sys_Milliseconds( );
		time = newtime - oldtime;
		//while( 1 ) {			
#ifdef MM_TIMER
			newtime = WindowsNS::timeGetTime( );
			if( ( newtime - oldtime ) > 0 ) {
				time = ( timeType )( newtime - oldtime );
				break;
			}
#else
			//WindowsNS::QueryPerformanceCounter( ( WindowsNS::LARGE_INTEGER * )&newtime );
			//if( ( newtime - oldtime ) > 0 ) {
			//	time = ( timeType )( newtime - oldtime ) / freq;
			//	break;
			//}
#endif
		//}
		Common::Frame( time );
		//WindowsNS::Sleep( 1 );
		oldtime = newtime;
	}
	// never gets here
    return TRUE;
}

}
