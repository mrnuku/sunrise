#include "precompiled.h"
#pragma hdrstop

#define WORD				WindowsNS::WORD

intptr_t	System::findhandle;

//===============================================================================

int		hunkcount;


byte	* membase;
int		hunkmaxsize;
int		cursize;

#define	VIRTUAL_ALLOC

void * Hunk_Begin( int maxsize ) {

	// reserve a huge chunk of memory, but don't commit any yet
	cursize = 0;
	hunkmaxsize = maxsize;
#ifdef VIRTUAL_ALLOC
	membase =( byte * )WindowsNS::VirtualAlloc( NULL, maxsize, MEM_RESERVE, PAGE_NOACCESS );
#else
	membase = malloc( maxsize );
	Common::Com_Memset( membase, 0, maxsize );
#endif
	if( !membase ) System::Sys_Error( "VirtualAlloc reserve failed" );
	return( void * )membase;
}

void * Hunk_Alloc( int size ) {

	void	* buf;

	// round to cacheline
	size =( size+31 )&~31;

#ifdef VIRTUAL_ALLOC
	// commit pages as needed
	buf = WindowsNS::VirtualAlloc( membase, cursize+size, MEM_COMMIT, PAGE_READWRITE );

	if( !buf ) {

		WindowsNS::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WindowsNS::GetLastError( ), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( WindowsNS::LPTSTR ) &buf, 0, NULL );
		System::Sys_Error( "VirtualAlloc commit failed.\n%s", buf );
	}
#endif
	cursize += size;
	if( cursize > hunkmaxsize ) System::Sys_Error( "Hunk_Alloc overflow" );

	return( void * )( membase+cursize-size );
}

int Hunk_End( ) {


	// free the remaining unused virtual memory
#if 0
	void	* buf;

	// write protect it
	buf = VirtualAlloc( membase, cursize, MEM_COMMIT, PAGE_READONLY );
	if( !buf )
		Sys_Error( "VirtualAlloc commit failed" );
#endif

	hunkcount++;
//Com_Printf( "hunkcount: %i\n", hunkcount );
	return cursize;
}

void Hunk_Free( void * base ) {

	if( base )
#ifdef VIRTUAL_ALLOC
		WindowsNS::VirtualFree( base, 0, MEM_RELEASE );
#else
		free( base );
#endif

	hunkcount--;
}

//===============================================================================

void System::Sys_Mkdir( const Str & path ) {

	_mkdir( path );
}

//============================================

static bool CompareAttributes( unsigned found, unsigned musthave, unsigned canthave ) {

	if( ( found & _A_RDONLY ) &&( canthave & SFF_RDONLY ) )
		return false;
	if( ( found & _A_HIDDEN ) &&( canthave & SFF_HIDDEN ) )
		return false;
	if( ( found & _A_SYSTEM ) &&( canthave & SFF_SYSTEM ) )
		return false;
	if( ( found & _A_SUBDIR ) &&( canthave & SFF_SUBDIR ) )
		return false;
	if( ( found & _A_ARCH ) &&( canthave & SFF_ARCH ) )
		return false;

	if( ( musthave & SFF_RDONLY ) && !( found & _A_RDONLY ) )
		return false;
	if( ( musthave & SFF_HIDDEN ) && !( found & _A_HIDDEN ) )
		return false;
	if( ( musthave & SFF_SYSTEM ) && !( found & _A_SYSTEM ) )
		return false;
	if( ( musthave & SFF_SUBDIR ) && !( found & _A_SUBDIR ) )
		return false;
	if( ( musthave & SFF_ARCH ) && !( found & _A_ARCH ) )
		return false;

	return true;
}

const Str System::Sys_FindFirst( const Str & path, unsigned musthave, unsigned canthave ) {

	struct _finddata_t findinfo;

	if( findhandle ) System::Sys_Error( "Sys_BeginFind without close" );

	//findbase = path;
	//findbase.StripFilename( );
	findhandle = _findfirst( path, &findinfo );
	if( findhandle == -1 ) return NULL;
	if( !CompareAttributes( findinfo.attrib, musthave, canthave ) ) return NULL;
	//sprintf( findpath, "%s/%s", findbase.c_str( ), findinfo.name );

	return findinfo.name;
}

const Str System::Sys_FindNext( unsigned musthave, unsigned canthave ) {

	struct _finddata_t findinfo;

	if( findhandle == -1 ) return NULL;
	if( _findnext( findhandle, &findinfo ) == -1 ) return NULL;
	if( !CompareAttributes( findinfo.attrib, musthave, canthave ) ) return NULL;

	//sprintf( findpath, "%s/%s", findbase.c_str( ), findinfo.name );
	return findinfo.name;
}

void System::Sys_FindClose( ) {

	if( findhandle != -1 ) _findclose( findhandle );
	findhandle = 0;
}


//============================================

