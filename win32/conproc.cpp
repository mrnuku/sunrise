#include "precompiled.h"
#pragma hdrstop

#define DWORD				WindowsNS::DWORD

// conproc.c -- support for qhost

#define CCOM_WRITE_TEXT		0x2
// Param1 : Text

#define CCOM_GET_TEXT		0x3
// Param1 : Begin line
// Param2 : End line

#define CCOM_GET_SCR_LINES	0x4
// No params

#define CCOM_SET_SCR_LINES	0x5
// Param1 : Number of lines


WindowsNS::HANDLE	heventDone;
WindowsNS::HANDLE	hfileBuffer;
WindowsNS::HANDLE	heventChildSend;
WindowsNS::HANDLE	heventParentSend;
WindowsNS::HANDLE	hStdout;
WindowsNS::HANDLE	hStdin;

unsigned _stdcall RequestProc( void * arg );
WindowsNS::LPVOID GetMappedBuffer( WindowsNS::HANDLE hfileBuffer );
void ReleaseMappedBuffer( WindowsNS::LPVOID pBuffer );
WindowsNS::BOOL GetScreenBufferLines( int * piLines );
WindowsNS::BOOL SetScreenBufferLines( int iLines );
WindowsNS::BOOL ReadText( WindowsNS::LPTSTR pszText, int iBeginLine, int iEndLine );
WindowsNS::BOOL WriteText( WindowsNS::LPCTSTR szText );
int CharToCode( char c );
WindowsNS::BOOL SetConsoleCXCY( WindowsNS::HANDLE hStdout, int cx, int cy );

int		ccom_argc;
char	* *ccom_argv;

/*
================
CCheckParm

Returns the position( 1 to argc-1 ) in the program's argument list
where the given parameter apears, or 0 if not present
================
*/
int CCheckParm( const char * parm ) {

	int             i;
	
	for( i = 1; i<ccom_argc; i++ )
	{
		if( !ccom_argv[ i ] )
			continue;
		if( !strcmp( parm, ccom_argv[ i ] ) ) 	return i;
	}
		
	return 0;
}


void InitConProc( int argc, char * *argv ) {

	unsigned	threadAddr;
	WindowsNS::HANDLE		hFile = NULL;
	WindowsNS::HANDLE		heventParent = NULL;
	WindowsNS::HANDLE		heventChild = NULL;
	int			t;

	ccom_argc = argc;
	ccom_argv = argv;

// give QHOST a chance to hook into the console
	if( ( t = CCheckParm( "-HFILE" ) ) > 0 )
	{
		if( t < argc )
			hFile =( WindowsNS::HANDLE )atoi( ccom_argv[ t+1 ] );
	}
		
	if( ( t = CCheckParm( "-HPARENT" ) ) > 0 )
	{
		if( t < argc )
			heventParent =( WindowsNS::HANDLE )atoi( ccom_argv[ t+1 ] );
	}
		
	if( ( t = CCheckParm( "-HCHILD" ) ) > 0 )
	{
		if( t < argc )
			heventChild =( WindowsNS::HANDLE )atoi( ccom_argv[ t+1 ] );
	}


// ignore if we don't have all the events.
	if( !hFile || !heventParent || !heventChild )
	{
		printf( "Qhost not present.\n" );
		return;
	}

	printf( "Initializing for qhost.\n" );

	hfileBuffer = hFile;
	heventParentSend = heventParent;
	heventChildSend = heventChild;

// so we'll know when to go away.
	heventDone = WindowsNS::CreateEvent( NULL, FALSE, FALSE, NULL );

	if( !heventDone )
	{
		printf( "Couldn't create heventDone\n" );
		return;
	}

	if( !_beginthreadex( NULL, 0, RequestProc, NULL, 0, &threadAddr ) ) {
		WindowsNS::CloseHandle( heventDone );
		printf( "Couldn't create QHOST thread\n" );
		return;
	}

// save off the input/output handles.
	hStdout = WindowsNS::GetStdHandle( STD_OUTPUT_HANDLE );
	hStdin = WindowsNS::GetStdHandle( STD_INPUT_HANDLE );

// force 80 character width, at least 25 character height
	SetConsoleCXCY( hStdout, 80, 25 );
}


void DeinitConProc( ) {

	if( heventDone )
		WindowsNS::SetEvent( heventDone );
}


unsigned _stdcall RequestProc( void * arg ) {

	int		* pBuffer;
	DWORD	dwRet;
	WindowsNS::HANDLE	heventWait[ 2 ];
	int		iBeginLine, iEndLine;
	
	heventWait[ 0 ] = heventParentSend;
	heventWait[ 1 ] = heventDone;

	while( 1 )
	{
		dwRet = WindowsNS::WaitForMultipleObjects( 2, heventWait, FALSE, INFINITE );

	// heventDone fired, so we're exiting.
		if( dwRet == WAIT_OBJECT_0 + 1 )	 	break;

		pBuffer =( int * ) GetMappedBuffer( hfileBuffer );
		
	// hfileBuffer is invalid.  Just leave.
		if( !pBuffer )
		{
			printf( "Invalid hfileBuffer\n" );
			break;
		}

		switch( pBuffer[ 0 ] )
		{
			case CCOM_WRITE_TEXT:
			// Param1 : Text
				pBuffer[ 0 ] = WriteText( ( WindowsNS::LPCTSTR )( pBuffer + 1 ) );
				break;

			case CCOM_GET_TEXT:
			// Param1 : Begin line
			// Param2 : End line
				iBeginLine = pBuffer[ 1 ];
				iEndLine = pBuffer[ 2 ];
				pBuffer[ 0 ] = ReadText( ( WindowsNS::LPTSTR )( pBuffer + 1 ), iBeginLine, iEndLine );
				break;

			case CCOM_GET_SCR_LINES:
			// No params
				pBuffer[ 0 ] = GetScreenBufferLines( &pBuffer[ 1 ] );
				break;

			case CCOM_SET_SCR_LINES:
			// Param1 : Number of lines
				pBuffer[ 0 ] = SetScreenBufferLines( pBuffer[ 1 ] );
				break;
		}

		ReleaseMappedBuffer( pBuffer );
		WindowsNS::SetEvent( heventChildSend );
	}

	_endthreadex( 0 );
	return 0;
}


WindowsNS::LPVOID GetMappedBuffer( WindowsNS::HANDLE hfileBuffer ) {

	WindowsNS::LPVOID pBuffer;

	pBuffer = WindowsNS::MapViewOfFile( hfileBuffer, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0 );

	return pBuffer;
}


void ReleaseMappedBuffer( WindowsNS::LPVOID pBuffer ) {

	WindowsNS::UnmapViewOfFile( pBuffer );
}


WindowsNS::BOOL GetScreenBufferLines( int * piLines ) {

	WindowsNS::CONSOLE_SCREEN_BUFFER_INFO	info;							  
	WindowsNS::BOOL						bRet;

	bRet = WindowsNS::GetConsoleScreenBufferInfo( hStdout, &info );
		
	if( bRet )
		* piLines = info.dwSize.Y;

	return bRet;
}


WindowsNS::BOOL SetScreenBufferLines( int iLines ) {


	return SetConsoleCXCY( hStdout, 80, iLines );
}


WindowsNS::BOOL ReadText( WindowsNS::LPTSTR pszText, int iBeginLine, int iEndLine ) {

	WindowsNS::COORD	coord;
	DWORD	dwRead;
	WindowsNS::BOOL	bRet;

	coord.X = 0;
	coord.Y = iBeginLine;

	bRet = WindowsNS::ReadConsoleOutputCharacter(
		hStdout, pszText, 80 * ( iEndLine - iBeginLine + 1 ), coord, &dwRead );

	// Make sure it's null terminated.
	if( bRet )
		pszText[ dwRead ] = '\0';

	return bRet;
}


WindowsNS::BOOL WriteText( WindowsNS::LPCTSTR szText ) {

	DWORD			dwWritten;
	WindowsNS::INPUT_RECORD	rec;
	char			upper, * sz;

	sz =( WindowsNS::LPTSTR ) szText;

	while( *sz )
	{
	// 13 is the code for a carriage return( \n ) instead of 10.
		if( *sz == 10 )
			* sz = 13;

		upper = toupper( *sz );

		rec.EventType = KEY_EVENT;
		rec.Event.KeyEvent.bKeyDown = TRUE;
		rec.Event.KeyEvent.wRepeatCount = 1;
		rec.Event.KeyEvent.wVirtualKeyCode = upper;
		rec.Event.KeyEvent.wVirtualScanCode = CharToCode( *sz );
		rec.Event.KeyEvent.uChar.AsciiChar = * sz;
		rec.Event.KeyEvent.uChar.UnicodeChar = * sz;
		rec.Event.KeyEvent.dwControlKeyState = isupper( *sz ) ? 0x80 : 0x0; 

		WindowsNS::WriteConsoleInput(
			hStdin, &rec, 1, &dwWritten );

		rec.Event.KeyEvent.bKeyDown = FALSE;

		WindowsNS::WriteConsoleInput(
			hStdin, &rec, 1, &dwWritten );

		sz++;
	}

	return TRUE;
}


int CharToCode( char c ) {

	char upper;
		
	upper = toupper( c );

	switch( c )
	{
		case 13:
			return 28;

		default:
			break;
	}

	if( isalpha( c ) ) return( 30 + upper - 65 ); 

	if( isdigit( c ) ) return( 1 + upper - 47 );

	return c;
}


WindowsNS::BOOL SetConsoleCXCY( WindowsNS::HANDLE hStdout, int cx, int cy ) {

	WindowsNS::CONSOLE_SCREEN_BUFFER_INFO	info;
	WindowsNS::COORD						coordMax;
 
	coordMax = WindowsNS::GetLargestConsoleWindowSize( hStdout );

	if( cy > coordMax.Y )
		cy = coordMax.Y;

	if( cx > coordMax.X )
		cx = coordMax.X;
 
	if( !WindowsNS::GetConsoleScreenBufferInfo( hStdout, &info ) ) return FALSE;
 
// height
    info.srWindow.Left = 0;         
    info.srWindow.Right = info.dwSize.X - 1;                
    info.srWindow.Top = 0;
    info.srWindow.Bottom = cy - 1;          
 
	if( cy < info.dwSize.Y )
	{
		if( !WindowsNS::SetConsoleWindowInfo( hStdout, TRUE, &info.srWindow ) ) 	return FALSE;
 
		info.dwSize.Y = cy;
 
		if( !WindowsNS::SetConsoleScreenBufferSize( hStdout, info.dwSize ) ) 	return FALSE;
    }
    else if( cy > info.dwSize.Y )
    {
		info.dwSize.Y = cy;
 
		if( !WindowsNS::SetConsoleScreenBufferSize( hStdout, info.dwSize ) ) 	return FALSE;
 
		if( !WindowsNS::SetConsoleWindowInfo( hStdout, TRUE, &info.srWindow ) ) 	return FALSE;
    }
 
	if( !WindowsNS::GetConsoleScreenBufferInfo( hStdout, &info ) ) return FALSE;
 
// width
	info.srWindow.Left = 0;         
	info.srWindow.Right = cx - 1;
	info.srWindow.Top = 0;
	info.srWindow.Bottom = info.dwSize.Y - 1;               
 
	if( cx < info.dwSize.X )
	{
		if( !WindowsNS::SetConsoleWindowInfo( hStdout, TRUE, &info.srWindow ) ) 	return FALSE;
 
		info.dwSize.X = cx;
    
		if( !WindowsNS::SetConsoleScreenBufferSize( hStdout, info.dwSize ) ) 	return FALSE;
	}
	else if( cx > info.dwSize.X )
	{
		info.dwSize.X = cx;
 
		if( !WindowsNS::SetConsoleScreenBufferSize( hStdout, info.dwSize ) ) 	return FALSE;
 
		if( !WindowsNS::SetConsoleWindowInfo( hStdout, TRUE, &info.srWindow ) ) 	return FALSE;
	}
 
	return TRUE;
}
     
