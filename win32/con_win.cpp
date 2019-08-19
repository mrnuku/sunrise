#include "precompiled.h"
#pragma hdrstop

/*#include <errno.h>
#include <float.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <conio.h>*/

using namespace WindowsNS;

const int MAX_EDIT_LINE = 256;

typedef struct autoComplete_s {
	bool			valid;
	int				length;
	char			completionString[MAX_EDIT_LINE];
	char			currentMatch[MAX_EDIT_LINE];
	int				matchCount;
	int				matchIndex;
	int				findMatchIndex;
} autoComplete_t;

class idEditField {
public:
					idEditField();
					~idEditField();

	void			Clear( void );
	void			SetWidthInChars( int w );
	void			SetCursor( int c );
	int				GetCursor( void ) const;
	void			ClearAutoComplete( void );
	int				GetAutoCompleteLength( void ) const;
	void			AutoComplete( void );
	void			CharEvent( int c );
	void			KeyDownEvent( int key );
	void			Paste( void );
	char *			GetBuffer( void );
	void			SetBuffer( const char *buffer );

private:
	int				cursor;
	int				scroll;
	int				widthInChars;
	char			buffer[MAX_EDIT_LINE];
	autoComplete_t	autoComplete;
};

static autoComplete_t	globalAutoComplete;

/*
===============
FindMatches
===============
*/
static void FindMatches( const char *s ) {
	int		i;
	if( Str::Icmpn( s, globalAutoComplete.completionString, ( int )strlen( globalAutoComplete.completionString ) ) != 0 )
		return;
	globalAutoComplete.matchCount++;
	if( globalAutoComplete.matchCount == 1 ) {
		Str::Copynz( globalAutoComplete.currentMatch, s, sizeof( globalAutoComplete.currentMatch ) );
		return;
	}
	// cut currentMatch to the amount common with s
	for ( i = 0; s[i]; i++ ) {
		if ( tolower( globalAutoComplete.currentMatch[i] ) != tolower( s[i] ) ) {
			globalAutoComplete.currentMatch[i] = 0;
			break;
		}
	}
	globalAutoComplete.currentMatch[i] = 0;
}

static void FindIndexMatch( const char *s ) {

	if( Str::Icmpn( s, globalAutoComplete.completionString, ( int )strlen( globalAutoComplete.completionString ) ) != 0 )
		return;
	if( globalAutoComplete.findMatchIndex == globalAutoComplete.matchIndex )
		Str::Copynz( globalAutoComplete.currentMatch, s, sizeof( globalAutoComplete.currentMatch ) );
	globalAutoComplete.findMatchIndex++;
}

static void PrintMatches( const char *s ) {
	if( Str::Icmpn( s, globalAutoComplete.currentMatch, ( int )strlen( globalAutoComplete.currentMatch ) ) == 0 )
		Common::Com_Printf( "    %s\n", s );
}

static void PrintCvarMatches( const char *s ) {
	if( Str::Icmpn( s, globalAutoComplete.currentMatch, ( int )strlen( globalAutoComplete.currentMatch ) ) == 0 )
		Common::Com_Printf( "    %s" S_COLOR_WHITE " = \"%s\"\n", s, CVarSystem::GetCVarStr( s ).c_str( ) );
}

idEditField::idEditField() {
	widthInChars = 0;
	Clear();
}

idEditField::~idEditField() {
}

void idEditField::Clear( void ) {
	buffer[0] = 0;
	cursor = 0;
	scroll = 0;
	autoComplete.length = 0;
	autoComplete.valid = false;
}

void idEditField::SetWidthInChars( int w ) {
	assert( w <= MAX_EDIT_LINE );
	widthInChars = w;
}

void idEditField::SetCursor( int c ) {
	assert( c <= MAX_EDIT_LINE );
	cursor = c;
}

int idEditField::GetCursor( void ) const {
	return cursor;
}

void idEditField::ClearAutoComplete( void ) {
	if ( autoComplete.length > 0 && autoComplete.length <= (int) strlen( buffer ) ) {
		buffer[autoComplete.length] = '\0';
		if ( cursor > autoComplete.length )
			cursor = autoComplete.length;
	}
	autoComplete.length = 0;
	autoComplete.valid = false;
}

int idEditField::GetAutoCompleteLength( void ) const {
	return autoComplete.length;
}

void idEditField::AutoComplete( void ) {
	char completionArgString[MAX_EDIT_LINE];
	CmdArgs args;
	if ( !autoComplete.valid ) {
		args.TokenizeString( buffer, false );
		Str::Copynz( autoComplete.completionString, args.Argv( 0 ), sizeof( autoComplete.completionString ) );
		Str::Copynz( completionArgString, args.Args(), sizeof( completionArgString ) );
		autoComplete.matchCount = 0;
		autoComplete.matchIndex = 0;
		autoComplete.currentMatch[0] = 0;
		if ( strlen( autoComplete.completionString ) == 0 )
			return;
		globalAutoComplete = autoComplete;
		//cmdSystem->CommandCompletion( FindMatches );
		//cvarSystem->CommandCompletion( FindMatches );
		autoComplete = globalAutoComplete;
		if ( autoComplete.matchCount == 0 )
			return;	// no matches
		// when there's only one match or there's an argument
		if ( autoComplete.matchCount == 1 || completionArgString[0] != '\0' ) {
			/// try completing arguments
			Str::Append( autoComplete.completionString, sizeof( autoComplete.completionString ), " " );
			Str::Append( autoComplete.completionString, sizeof( autoComplete.completionString ), completionArgString );
			autoComplete.matchCount = 0;
			globalAutoComplete = autoComplete;

			//cmdSystem->ArgCompletion( autoComplete.completionString, FindMatches );
			//cvarSystem->ArgCompletion( autoComplete.completionString, FindMatches );
			autoComplete = globalAutoComplete;
			Str::snPrintf( buffer, sizeof( buffer ), "%s", autoComplete.currentMatch );
			if ( autoComplete.matchCount == 0 ) {
				// no argument matches
				Str::Append( buffer, sizeof( buffer ), " " );
				Str::Append( buffer, sizeof( buffer ), completionArgString );
				SetCursor( ( int )strlen( buffer ) );
				return;
			}
		} else {
			// multiple matches, complete to shortest
			Str::snPrintf( buffer, sizeof( buffer ), "%s", autoComplete.currentMatch );
			if ( strlen( completionArgString ) ) {
				Str::Append( buffer, sizeof( buffer ), " " );
				Str::Append( buffer, sizeof( buffer ), completionArgString );
			}
		}
		autoComplete.length = ( int )strlen( buffer );
		autoComplete.valid = ( autoComplete.matchCount != 1 );
		SetCursor( autoComplete.length );
		Common::Com_Printf( "]%s\n", buffer );
		// run through again, printing matches
		globalAutoComplete = autoComplete;
		//cmdSystem->CommandCompletion( PrintMatches );
		//cmdSystem->ArgCompletion( autoComplete.completionString, PrintMatches );
		//cvarSystem->CommandCompletion( PrintCvarMatches );
		//cvarSystem->ArgCompletion( autoComplete.completionString, PrintMatches );
	} else if ( autoComplete.matchCount != 1 ) {
		// get the next match and show instead
		autoComplete.matchIndex++;
		if ( autoComplete.matchIndex == autoComplete.matchCount )
			autoComplete.matchIndex = 0;
		autoComplete.findMatchIndex = 0;
		globalAutoComplete = autoComplete;
		//cmdSystem->CommandCompletion( FindIndexMatch );
		//cmdSystem->ArgCompletion( autoComplete.completionString, FindIndexMatch );
		//cvarSystem->CommandCompletion( FindIndexMatch );
		//cvarSystem->ArgCompletion( autoComplete.completionString, FindIndexMatch );
		autoComplete = globalAutoComplete;
		// and print it
		Str::snPrintf( buffer, sizeof( buffer ), autoComplete.currentMatch );
		if ( autoComplete.length > (int)strlen( buffer ) )
			autoComplete.length = ( int )strlen( buffer );
		SetCursor( autoComplete.length );
	}
}

void idEditField::CharEvent( int ch ) {
	int		len;
	if ( ch == 'v' - 'a' + 1 ) {	// ctrl-v is paste
		Paste();
		return;
	}
	if ( ch == 'c' - 'a' + 1 ) {	// ctrl-c clears the field
		Clear();
		return;
	}
	len = ( int )strlen( buffer );
	if ( ch == 'h' - 'a' + 1 || ch == VK_BACK ) {	// ctrl-h is backspace
		if ( cursor > 0 ) {
			memmove( buffer + cursor - 1, buffer + cursor, len + 1 - cursor );
			cursor--;
			if ( cursor < scroll ) {
				scroll--;
			}
		}
		return;
	}
	if ( ch == 'a' - 'a' + 1 ) {	// ctrl-a is home
		cursor = 0;
		scroll = 0;
		return;
	}
	if ( ch == 'e' - 'a' + 1 ) {	// ctrl-e is end
		cursor = len;
		scroll = cursor - widthInChars;
		return;
	}
	// ignore any other non printable chars
	if ( ch < 32 )
		return;
	//if ( idKeyInput::GetOverstrikeMode() ) {	
		if ( cursor == MAX_EDIT_LINE - 1 )
			return;
		buffer[cursor] = ch;
		cursor++;
	/*} else {	// insert mode
		if ( len == MAX_EDIT_LINE - 1 )
			return; // all full
		memmove( buffer + cursor + 1, buffer + cursor, len + 1 - cursor );
		buffer[cursor] = ch;
		cursor++;
	}*/
	if ( cursor >= widthInChars )
		scroll++;
	if ( cursor == len + 1 )
		buffer[cursor] = 0;
}

/*
===============
idEditField::KeyDownEvent
===============
*/
void idEditField::KeyDownEvent( int key ) {
	//int		len;
	// shift-insert is paste
#if 0
	if ( ( ( key == K_INS ) || ( key == K_KP_INS ) ) && idKeyInput::IsDown( K_SHIFT ) ) {
		ClearAutoComplete();
		Paste();
		return;
	}
	len = strlen( buffer );
	if ( key == K_DEL ) {
		if ( autoComplete.length )
			ClearAutoComplete();
		else if ( cursor < len )
			memmove( buffer + cursor, buffer + cursor + 1, len - cursor );
		return;
	}

	if ( key == K_RIGHTARROW ) {
		if ( idKeyInput::IsDown( K_CTRL ) ) {
			// skip to next word
			while( ( cursor < len ) && ( buffer[ cursor ] != ' ' ) )
				cursor++;
			while( ( cursor < len ) && ( buffer[ cursor ] == ' ' ) )
				cursor++;
		} else
			cursor++;
		if ( cursor > len )
			cursor = len;
		if ( cursor >= scroll + widthInChars )
			scroll = cursor - widthInChars + 1;
		if ( autoComplete.length > 0 )
			autoComplete.length = cursor;
		return;
	}
	if( key == K_LEFTARROW ) {
		if ( idKeyInput::IsDown( K_CTRL ) ) {
			// skip to previous word
			while( ( cursor > 0 ) && ( buffer[ cursor - 1 ] == ' ' ) )
				cursor--;
			while( ( cursor > 0 ) && ( buffer[ cursor - 1 ] != ' ' ) )
				cursor--;
		} else
			cursor--;
		if ( cursor < 0 )
			cursor = 0;
		if ( cursor < scroll )
			scroll = cursor;
		if ( autoComplete.length )
			autoComplete.length = cursor;
		return;
	}
	if ( key == K_HOME || ( tolower( key ) == 'a' && idKeyInput::IsDown( K_CTRL ) ) ) {
		cursor = 0;
		scroll = 0;
		if ( autoComplete.length ) {
			autoComplete.length = cursor;
			autoComplete.valid = false;
		}
		return;
	}
	if ( key == K_END || ( tolower( key ) == 'e' && idKeyInput::IsDown( K_CTRL ) ) ) {
		cursor = len;
		if ( cursor >= scroll + widthInChars )
			scroll = cursor - widthInChars + 1;
		if ( autoComplete.length ) {
			autoComplete.length = cursor;
			autoComplete.valid = false;
		}
		return;
	}
	if ( key == K_INS ) {
		idKeyInput::SetOverstrikeMode( !idKeyInput::GetOverstrikeMode() );
		return;
	}
	// clear autocompletion buffer on normal key input
	if ( key != K_CAPSLOCK && key != K_ALT && key != K_CTRL && key != K_SHIFT )
		ClearAutoComplete();
#endif
}

void idEditField::Paste( void ) {

	Str cbd = System::Sys_GetClipboardData( );
	// send as if typed, so insert / overstrike works properly
	for( int i = 0; i < cbd.Length( ); i++ )
		CharEvent( cbd[ i ] );
}

char *idEditField::GetBuffer( void ) {
	return buffer;
}

void idEditField::SetBuffer( const char *buf ) {
	Clear();
	Str::Copynz( buffer, buf, sizeof( buffer ) );
	SetCursor( ( int )strlen( buffer ) );
}

#define COPY_ID			1
#define QUIT_ID			2
#define CLEAR_ID		3

#define ERRORBOX_ID		10
#define ERRORTEXT_ID	11

#define EDIT_ID			100
#define INPUT_ID		101

#define	COMMAND_HISTORY	64

class WinConData {
public:
	HWND		hWnd;
	HWND		hwndBuffer;

	//HWND		hwndButtonClear;
	//HWND		hwndButtonCopy;
	//HWND		hwndButtonQuit;

	HWND		hwndErrorBox;

	//HBITMAP		hbmLogo;

	HBRUSH		hbrEditBackground;
	HBRUSH		hbrErrorBackground;
	HBRUSH		hbrEditLineBackground;

	HFONT		hfBufferFont;

	HWND		hwndInputLine;

	char		errorString[80];

	Lock		d_consoleLock;
	Str			consoleText, returnedText;
	bool		quitOnClose;
	int			windowWidth, windowHeight;
	 
	WNDPROC		SysInputLineWndProc;

	idEditField	historyEditLines[COMMAND_HISTORY];

	int			nextHistoryLine;// the last line in the history buffer, not masked
	int			historyLine;	// the line being displayed from history buffer
								// will be <= nextHistoryLine

	idEditField	consoleField;
};

WinConData *		System::g_conData = NULL;

LRESULT WINAPI System::ConWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
	//char *cmdString;
	static bool s_timePolarity;

	switch (uMsg) {
		case WM_ACTIVATE:
			if ( LOWORD( wParam ) != WA_INACTIVE ) {
				SetFocus( g_conData->hwndInputLine );
			}
		break;
		case WM_CLOSE:
			/*if ( cvarSystem->IsInitialized() ) {
				cmdString = Mem_CopyString( "quit" );
				Sys_QueEvent( 0, SE_CONSOLE, 0, 0, strlen( cmdString ) + 1, cmdString );
			} else*/ if ( g_conData->quitOnClose ) {
				PostQuitMessage( 0 );
			} else {
				Common::Com_Quit( );
				//Sys_ShowConsole( 0, false );
				//win32.win_viewlog.SetBool( false );
			}
			return 0;
		case WM_CTLCOLOREDIT:
			SetBkColor( ( HDC )wParam, RGB( 0x40, 0x40, 0x40 ) );
			SetTextColor( ( HDC )wParam, RGB( 0xFF, 0x80, 0x00 ) );
			return ( LRESULT )g_conData->hbrEditLineBackground;

		case WM_CTLCOLORSTATIC:
			if ( ( HWND )lParam == g_conData->hwndBuffer ) {
				//SetBkColor( ( HDC )wParam, RGB( 0x00, 0x00, 0x80 ) );
				//SetTextColor( ( HDC )wParam, RGB( 0xff, 0xff, 0x00 ) );
				SetBkColor( ( HDC ) wParam, RGB( 0x23, 0x23, 0x23 ) );
				SetTextColor( ( HDC ) wParam, RGB( 0xC0, 0xC0, 0xC0 ) );
				return ( LRESULT )g_conData->hbrEditBackground;
			} else if ( ( HWND )lParam == g_conData->hwndErrorBox ) {
				if ( s_timePolarity & 1 ) {
					SetBkColor( ( HDC )wParam, RGB( 0x80, 0x80, 0x80 ) );
					SetTextColor( ( HDC )wParam, RGB( 0xff, 0x0, 0x00 ) );
				} else {
					SetBkColor( ( HDC )wParam, RGB( 0x80, 0x80, 0x80 ) );
					SetTextColor( ( HDC )wParam, RGB( 0x00, 0x0, 0x00 ) );
				}
				return ( LRESULT )g_conData->hbrErrorBackground;
			}
			break;
		case WM_SYSCOMMAND:
			if ( wParam == SC_CLOSE ) {
				PostQuitMessage( 0 );
			}
			break;
		case WM_COMMAND:
			/*if ( wParam == COPY_ID ) {
				SendMessage( g_conData->hwndBuffer, EM_SETSEL, 0, -1 );
				SendMessage( g_conData->hwndBuffer, WM_COPY, 0, 0 );
			} else if ( wParam == QUIT_ID ) {
				if ( g_conData->quitOnClose ) {
					PostQuitMessage( 0 );
				} else {
					Common::Com_Quit( );
					//cmdString = Mem_CopyString( "quit" );
					//Sys_QueEvent( 0, SE_CONSOLE, 0, 0, strlen( cmdString ) + 1, cmdString );
				}
			} else if ( wParam == CLEAR_ID ) {
				SendMessage( g_conData->hwndBuffer, EM_SETSEL, 0, -1 );
				SendMessage( g_conData->hwndBuffer, EM_REPLACESEL, FALSE, ( LPARAM ) "" );
				UpdateWindow( g_conData->hwndBuffer );
			}*/
			break;
		case WM_CREATE:
			//g_conData->hbrEditBackground = CreateSolidBrush( RGB( 0x00, 0x00, 0x80 ) );
			//g_conData->hbrErrorBackground = CreateSolidBrush( RGB( 0x80, 0x80, 0x80 ) );
			g_conData->hbrEditLineBackground = CreateSolidBrush( RGB( 0x40, 0x40, 0x40 ) );
			g_conData->hbrEditBackground = CreateSolidBrush( RGB( 0x23, 0x23, 0x23 ) );
			g_conData->hbrErrorBackground = CreateSolidBrush( RGB( 0xC0, 0xC0, 0xC0 ) );
			SetTimer( hWnd, 1, 1000, NULL );
			break;
		case WM_TIMER:
			if ( wParam == 1 ) {
				s_timePolarity = (bool)!s_timePolarity;
				if ( g_conData->hwndErrorBox ) {
					InvalidateRect( g_conData->hwndErrorBox, NULL, FALSE );
				}
			}
			break;
    }

    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

LRESULT WINAPI System::InputLineWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
	int cursor;
	KeyEvent key;
	switch ( uMsg ) {
	case WM_KILLFOCUS:
		if ( ( HWND ) wParam == g_conData->hWnd || ( HWND ) wParam == g_conData->hwndErrorBox ) {
			SetFocus( hWnd );
			return 0;
		}
		break;

	case WM_KEYDOWN:
		//key = MapKey( lParam );
		key = KeyEvent( ( dword )wParam, ( dword )lParam, true );
		// command history
		if ( key == key_up ) {
			if ( g_conData->nextHistoryLine - g_conData->historyLine < COMMAND_HISTORY && g_conData->historyLine > 0 )
				g_conData->historyLine--;
			g_conData->consoleField = g_conData->historyEditLines[ g_conData->historyLine % COMMAND_HISTORY ];
			SetWindowText( g_conData->hwndInputLine, g_conData->consoleField.GetBuffer() );
			SendMessage( g_conData->hwndInputLine, EM_SETSEL, g_conData->consoleField.GetCursor(), g_conData->consoleField.GetCursor() );
			return 0;
		}
		if ( key == key_down ) {
			if ( g_conData->historyLine == g_conData->nextHistoryLine )
				return 0;
			g_conData->historyLine++;
			g_conData->consoleField = g_conData->historyEditLines[ g_conData->historyLine % COMMAND_HISTORY ];
			SetWindowText( g_conData->hwndInputLine, g_conData->consoleField.GetBuffer() );
			SendMessage( g_conData->hwndInputLine, EM_SETSEL, g_conData->consoleField.GetCursor(), g_conData->consoleField.GetCursor() );
			return 0;
		}
		//break;

	//case WM_CHAR:
		//key = MapKey( lParam );
		GetWindowText( g_conData->hwndInputLine, g_conData->consoleField.GetBuffer(), MAX_EDIT_LINE );
		SendMessage( g_conData->hwndInputLine, EM_GETSEL, (WPARAM) NULL, (LPARAM) &cursor );
		g_conData->consoleField.SetCursor( cursor );
		// enter the line
		if( key == key_return ) {
			ScopedLock< true >( g_conData->d_consoleLock );
			g_conData->consoleText = g_conData->consoleField.GetBuffer();
			g_conData->consoleText += '\n';
			SetWindowText( g_conData->hwndInputLine, "" );
			//Sys_Printf( "]%s\n", g_conData->consoleField.GetBuffer() );
			// copy line to history buffer
			g_conData->historyEditLines[g_conData->nextHistoryLine % COMMAND_HISTORY] = g_conData->consoleField;
			g_conData->nextHistoryLine++;
			g_conData->historyLine = g_conData->nextHistoryLine;
			g_conData->consoleField.Clear();
			return 0;
		}

		// command completion
		if( key == key_tab ) {
			g_conData->consoleField.AutoComplete();
			SetWindowText( g_conData->hwndInputLine, g_conData->consoleField.GetBuffer() );
			//g_conData->consoleField.SetWidthInChars( strlen( g_conData->consoleField.GetBuffer() ) );
			SendMessage( g_conData->hwndInputLine, EM_SETSEL, g_conData->consoleField.GetCursor(), g_conData->consoleField.GetCursor() );
			return 0;
		}
		// clear autocompletion buffer on normal key input
		if( key.GetAscii( ) )
			g_conData->consoleField.ClearAutoComplete();
		break;
	}

	return CallWindowProc( g_conData->SysInputLineWndProc, hWnd, uMsg, wParam, lParam );
}

/*
** Sys_CreateConsole
*/
void System::Sys_CreateConsole( ) {
	HDC hDC;
	WNDCLASS wc;
	RECT rect, rectC;
	const char *DEDCLASS = "SRC";
	int nHeight;
	int swidth, sheight;
	int DEDSTYLE = WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX;
	int i;

	Common::Com_Memset( &wc, 0, sizeof( wc ) );

	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC) ConWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = global_hInstance;
	wc.hIcon         = LoadIcon( global_hInstance, MAKEINTRESOURCE(2));
	wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
	wc.hbrBackground = (struct HBRUSH__ *)COLOR_WINDOW;
	wc.lpszMenuName  = 0;
	wc.lpszClassName = DEDCLASS;

	if ( !RegisterClass (&wc) )
		return;

	rect.left = 0;
	rect.right = 600;
	rect.top = 0;
	rect.bottom = 600;
	AdjustWindowRect( &rect, DEDSTYLE, FALSE );

	hDC = GetDC( GetDesktopWindow() );
	swidth = GetDeviceCaps( hDC, HORZRES );
	sheight = GetDeviceCaps( hDC, VERTRES );
	ReleaseDC( GetDesktopWindow(), hDC );

	g_conData = new WinConData( );

	g_conData->windowWidth = rect.right - rect.left + 1;
	g_conData->windowHeight = rect.bottom - rect.top + 1;

	//g_conData->hbmLogo = LoadBitmap( global_hInstance, MAKEINTRESOURCE( IDB_BITMAP_LOGO) );

	g_conData->hWnd = CreateWindowExA( 0, DEDCLASS, "SunRise Console "VERSION, DEDSTYLE, swidth / 2 - rect.right / 2, sheight / 2 - rect.bottom / 2, rect.right - rect.left + 1,
		rect.bottom - rect.top + 1, NULL, NULL, global_hInstance,  NULL );
	if ( g_conData->hWnd == NULL )
		return;
	// create fonts
	hDC = GetDC( g_conData->hWnd );
	nHeight = -MulDiv( 8, GetDeviceCaps( hDC, LOGPIXELSY ), 72 );
	g_conData->hfBufferFont = CreateFontA( nHeight, 0, 0, 0, FW_EXTRABOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_MODERN | FIXED_PITCH, "Consolas Bold" );
	ReleaseDC( g_conData->hWnd, hDC );
	// create the input line
	DEDSTYLE = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL;
	rectC.left = rect.left + 8;
	rectC.right = rect.right - 10;
	rectC.top = rect.bottom - 40;
	rectC.bottom = 25;
	AdjustWindowRect( &rectC, DEDSTYLE, FALSE );
	g_conData->hwndInputLine = CreateWindowExA( 0, "EDIT", NULL, DEDSTYLE, rectC.left, rectC.top, rectC.right, rectC.bottom, g_conData->hWnd, ( HMENU ) INPUT_ID, global_hInstance, NULL );
	// create the buttons
	//g_conData->hwndButtonCopy = CreateWindow( "button", NULL, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 5, 425, 72, 24, g_conData->hWnd, ( HMENU ) COPY_ID, global_hInstance, NULL );
	//SendMessage( g_conData->hwndButtonCopy, WM_SETTEXT, 0, ( LPARAM ) "copy" );
	//g_conData->hwndButtonClear = CreateWindow( "button", NULL, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 82, 425, 72, 24, g_conData->hWnd, ( HMENU ) CLEAR_ID, global_hInstance, NULL );
	//SendMessage( g_conData->hwndButtonClear, WM_SETTEXT, 0, ( LPARAM ) "clear" );
	//g_conData->hwndButtonQuit = CreateWindow( "button", NULL, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 462, 425, 72, 24, g_conData->hWnd, ( HMENU ) QUIT_ID, global_hInstance, NULL );
	//SendMessage( g_conData->hwndButtonQuit, WM_SETTEXT, 0, ( LPARAM ) "quit" );
	// create the scrollbuffer
	DEDSTYLE = WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY;
	rectC.left = rect.left + 8;
	rectC.right = rect.right - 10;
	rectC.top = rect.top + 30;
	rectC.bottom = rect.bottom - 50;
	AdjustWindowRect( &rectC, DEDSTYLE, FALSE );
	g_conData->hwndBuffer = CreateWindowExA( 0, "EDIT", NULL, DEDSTYLE, rectC.left, rectC.top, rectC.right, rectC.bottom, g_conData->hWnd, ( HMENU ) EDIT_ID, global_hInstance, NULL );
	SendMessage( g_conData->hwndBuffer, WM_SETFONT, ( WPARAM ) g_conData->hfBufferFont, 0 );
	g_conData->SysInputLineWndProc = ( WNDPROC )SetWindowLongPtr( g_conData->hwndInputLine, ( -4 ), ( LONG_PTR )InputLineWndProc );
	SendMessage( g_conData->hwndInputLine, WM_SETFONT, ( WPARAM ) g_conData->hfBufferFont, 0 );
	// don't show it now that we have a splash screen up
	/*if ( win32.win_viewlog.GetBool() ) {
		ShowWindow( g_conData->hWnd, SW_SHOWDEFAULT);
		UpdateWindow( g_conData->hWnd );
		SetForegroundWindow( g_conData->hWnd );
		SetFocus( g_conData->hwndInputLine );
	}*/
	g_conData->consoleField.Clear();
	for ( i = 0 ; i < COMMAND_HISTORY ; i++ )
		g_conData->historyEditLines[i].Clear();
}

/*
** Sys_DestroyConsole
*/
void System::Sys_DestroyConsole( ) {
	if ( g_conData ) {
		ShowWindow( g_conData->hWnd, SW_HIDE );
		CloseWindow( g_conData->hWnd );
		DestroyWindow( g_conData->hWnd );
		delete g_conData;
		g_conData = NULL;
	}
}

/*
** Sys_ShowConsole
*/
void System::Sys_ShowConsole( int visLevel, bool quitOnClose ) {

	g_conData->quitOnClose = quitOnClose;
	if ( !g_conData )
		return;
	switch ( visLevel ) {
		case 0:
			ShowWindow( g_conData->hWnd, SW_HIDE );
		break;
		case 1:
			ShowWindow( g_conData->hWnd, SW_SHOWNORMAL );
			SendMessage( g_conData->hwndBuffer, EM_LINESCROLL, 0, 0xffff );
		break;
		case 2:
			ShowWindow( g_conData->hWnd, SW_MINIMIZE );
		break;
		default:
			System::Sys_Error( "Invalid visLevel %d sent to Sys_ShowConsole\n", visLevel );
		break;
	}
}

/*
** Sys_ConsoleInput
*/
const Str & System::Sys_InputConsole( ) {	
	ScopedLock< true >( g_conData->d_consoleLock );	
	g_conData->returnedText = g_conData->consoleText;
	g_conData->consoleText.Clear( );
	return g_conData->returnedText;
}

/*
** Conbuf_AppendText
*/
void System::Sys_OutputConsole( const char * pMsg ) {
#define CONSOLE_BUFFER_SIZE		16384

	char buffer[CONSOLE_BUFFER_SIZE*2];
	char *b = buffer;
	const char *msg;
	int bufLen;
	int i = 0;
	static unsigned long s_totalChars;

	//
	// if the message is REALLY long, use just the last portion of it
	//
	if ( strlen( pMsg ) > CONSOLE_BUFFER_SIZE - 1 )	{
		msg = pMsg + strlen( pMsg ) - CONSOLE_BUFFER_SIZE + 1;
	} else {
		msg = pMsg;
	}

	//
	// copy into an intermediate buffer
	//
	while ( msg[i] && ( ( b - buffer ) < sizeof( buffer ) - 1 ) ) {
		if ( msg[i] == '\n' && msg[i+1] == '\r' ) {
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
			i++;
		} else if ( msg[i] == '\r' ) {
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
		} else if ( msg[i] == '\n' ) {
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
		} else if ( Str::IsColor( &msg[i] ) ) {
			i++;
		} else {
			*b= msg[i];
			b++;
		}
		i++;
	}
	*b = 0;
	bufLen = ( int )( b - buffer );

	s_totalChars += bufLen;

	//
	// replace selection instead of appending if we're overflowing
	//
	if ( s_totalChars > 0x7000 ) {
		SendMessage( g_conData->hwndBuffer, EM_SETSEL, 0, -1 );
		s_totalChars = bufLen;
	}

	//
	// put this text into the windows console
	//
	SendMessage( g_conData->hwndBuffer, EM_LINESCROLL, 0, 0xffff );
	SendMessage( g_conData->hwndBuffer, EM_SCROLLCARET, 0, 0 );
	SendMessage( g_conData->hwndBuffer, EM_REPLACESEL, 0, (LPARAM) buffer );
}

/*
** Win_SetErrorText
*/
void System::Win_SetErrorText( const char * buf ) {
	Str::Copynz( g_conData->errorString, buf, sizeof( g_conData->errorString ) );
	if ( !g_conData->hwndErrorBox ) {
		g_conData->hwndErrorBox = CreateWindow( "static", NULL, WS_CHILD | WS_VISIBLE | SS_SUNKEN, 6, 5, 526, 30, g_conData->hWnd, ( HMENU ) ERRORBOX_ID, global_hInstance, NULL );
		SendMessage( g_conData->hwndErrorBox, WM_SETFONT, ( WPARAM ) g_conData->hfBufferFont, 0 );
		SetWindowText( g_conData->hwndErrorBox, g_conData->errorString );
		DestroyWindow( g_conData->hwndInputLine );
		g_conData->hwndInputLine = NULL;
	}
}
