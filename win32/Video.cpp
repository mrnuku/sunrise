#include "precompiled.h"
#pragma hdrstop

#define DWORD				WindowsNS::DWORD
#define WORD				WindowsNS::WORD
#define DWORD_PTR			WindowsNS::DWORD_PTR

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL ( WM_MOUSELAST+1 )  // message that will be supported by the OS 
#endif

static WindowsNS::UINT MSH_MOUSEWHEEL;

// Console variables that we need to access from this module

// Global variables used internally by this module
VecT2i		Video::g_screenResolution( 0, 0 );
viddef_t	Video::viddef;				// global video state; used by other modules
WindowsNS::HINSTANCE	reflib_library;		// Handle to refresh DLL 
bool	reflib_active = 0;

WindowsNS::HWND        cl_hwnd;            // Main window handle for life of program

static bool s_alttab_disabled;

/*
==========================================================================

DLL GLUE

==========================================================================
*/

//==========================================================================

void Video::AppActivate( bool fActive, bool minimize ) {

	Minimized = minimize;
	Keyboard::Key_ClearStates( );
	if( fActive && !Minimized ) // we don't want to act like we're active if we're minimized
		ActiveApp = true;
	else
		ActiveApp = false;
	if( !ActiveApp ) { // minimize/restore mouse-capture on demand
		Input::IN_Activate( false );
		//Sound::S_Activate( false );
		SoundSystem::Deactivate( );
	} else {
		if( Client::cls.key_dest != key_console )
			Input::IN_Activate( true );
		//Sound::S_Activate( true );
		SoundSystem::Activate( );
	}
}

/*
====================
MainWndProc

main window procedure
====================
*/
WindowsNS::LRESULT WINAPI System::MainWndProc( WindowsNS::HWND hWnd, WindowsNS::UINT uMsg, WindowsNS::WPARAM wParam, WindowsNS::LPARAM lParam ) {

	byte lpb[ 48 ];
	WindowsNS::RAWINPUT * raw;
	WindowsNS::UINT dwSize, dwRet;

	switch( uMsg ) {

		case WM_INPUT:
			dwSize = 48;    
			dwRet = WindowsNS::GetRawInputData( ( WindowsNS::HRAWINPUT )lParam, RID_INPUT, lpb, &dwSize, sizeof( WindowsNS::RAWINPUTHEADER ) );
			raw = ( WindowsNS::RAWINPUT * )lpb;
			if( raw->header.dwType == RIM_TYPEMOUSE && !raw->data.mouse.usFlags ) {
				Input::AddMouseMovement( Vec2( ( float )raw->data.mouse.lLastX, ( float )raw->data.mouse.lLastY ) );
				Input::ProcessRawMouseButtons( raw->data.mouse.usButtonFlags, *( short * )&raw->data.mouse.usButtonData );
			}
			break;

		case WM_CREATE:
			cl_hwnd = hWnd;
			//MSH_MOUSEWHEEL = WindowsNS::RegisterWindowMessage( "MSWHEEL_ROLLMSG" );
			break;

		case WM_PAINT:
			Screen::SCR_DirtyScreen( );	// force entire screen to update next frame
			break;

		case WM_DESTROY:
			// let sound and input know about this?
			cl_hwnd = NULL;
			break;

		case WM_ACTIVATE:
			int	fActive, fMinimized;
			// KJB: Watch this for problems in fullscreen modes with Alt-tabbing.
			fActive = LOWORD( wParam );
			fMinimized =( WindowsNS::BOOL ) HIWORD( wParam );
			Video::AppActivate( fActive != WA_INACTIVE, fMinimized != 0 );
			if( reflib_active )
				GLOSLayer::AppActivate( !( fActive == WA_INACTIVE ) );
			break;

		case WM_MOVE:
			int		xPos, yPos;
			WindowsNS::RECT	r;
			int		style;

			if( !Common::vid_fullscreen.GetBool( ) ) {
				xPos = ( short )LOWORD( lParam );    // horizontal position 
				yPos = ( short )HIWORD( lParam );    // vertical position 
				r.left   = 0;
				r.top    = 0;
				r.right  = 1;
				r.bottom = 1;
				style = WindowsNS::GetWindowLong( hWnd, GWL_STYLE );
				WindowsNS::AdjustWindowRect( &r, style, FALSE );
				Common::vid_xpos.SetInt( xPos + r.left );
				Common::vid_ypos.SetInt( yPos + r.top );
				Common::vid_xpos.ClearModified( );
				Common::vid_ypos.ClearModified( );
				//if( ActiveApp ) Input::IN_Activate( true );
			}
			break;

		case WM_MOUSEMOVE:
			GUISystem::SendMouseMovement( Vec2( ( float )LOWORD( lParam ), ( float )HIWORD(lParam) ) );
			break;

		case WM_SYSCOMMAND:
			if( wParam == SC_SCREENSAVE )
				return 0;
			break;

		case WM_SYSKEYDOWN:
			if( wParam == 13 )
				Common::vid_fullscreen.SetBool( !Common::vid_fullscreen.GetBool( ) );
			// fall through

		case WM_KEYDOWN:
			Keyboard::Key_Event( KeyEvent( ( dword )wParam, ( dword )lParam, true ), System::Sys_MsgMilliseconds( ) );
			break;

		case WM_SYSKEYUP:
		case WM_KEYUP:
			Keyboard::Key_Event( KeyEvent( ( dword )wParam, ( dword )lParam, false ), System::Sys_MsgMilliseconds( ) );
			break;
	}

	/* return 0 if handled message, 1 if not */
	return WindowsNS::DefWindowProc( hWnd, uMsg, wParam, lParam );
}

/*
============
VID_Restart_f

Console command to re-start the video mode and refresh DLL. We do this
simply by setting the modified flag for the vid_ref variable, which will
cause the entire video mode and refresh DLL to be reset on the next frame.
============
*/

void Video::VID_Restart_f( ) {

	// FIXME
}

void Video::VID_Front_f( ) {

	WindowsNS::SetWindowLong( cl_hwnd, GWL_EXSTYLE, WS_EX_TOPMOST );
	WindowsNS::SetForegroundWindow( cl_hwnd );
}

/*
* * VID_UpdateWindowPosAndSize
*/
void Video::VID_UpdateWindowPosAndSize( int x, int y ) {

	WindowsNS::RECT r;
	int		style, w, h;
	r.left   = 0;
	r.top    = 0;
	r.right  = Video::viddef.width;
	r.bottom = Video::viddef.height;
	style = WindowsNS::GetWindowLong( cl_hwnd, GWL_STYLE );
	WindowsNS::AdjustWindowRect( &r, style, FALSE );
	w = r.right - r.left;
	h = r.bottom - r.top;
	WindowsNS::MoveWindow( cl_hwnd, Common::vid_xpos.GetInt( ), Common::vid_ypos.GetInt( ), w, h, TRUE );
}

/*
* * VID_NewWindow
*/
void Video::VID_NewWindow( int width, int height ) {

	g_screenResolution.Set( width, height );
	Video::viddef.width  = width;
	Video::viddef.height = height;
	Client::cl.force_refdef = true;		// can't use a paused refdef
}

/*
==============
VID_LoadRefresh
==============
*/
bool Video::VID_LoadRefresh( ) {
	
	if( reflib_active ) {
		System::Sys_SendKeyEvents( );
		Renderer::Shutdown( );
	}
	Common::Com_Printf( ">>> init renderer >>>\n" );
	if( !Renderer::Init( global_hInstance, ( void * )System::MainWndProc ) ) {
		System::Sys_SendKeyEvents( );
		Renderer::Shutdown( );
		return false;
	}
	System::Sys_SendKeyEvents( );
	Common::Com_Printf( "<<< int renderer <<<\n" );
	reflib_active = true;
	return true;
}

/*
============
VID_CheckChanges

This function gets called once just before drawing each frame, and it's sole purpose in life
is to check to see if any of the video mode parameters have changed, and if they have to 
update the rendering DLL and/or video mode to match.
============
*/
void Video::VID_CheckChanges( ) {
	
	if( reflib_active )
		return;
	VID_LoadRefresh( );
	if( Common::vid_xpos.IsModified( ) || Common::vid_ypos.IsModified( ) ) {
		if( !Common::vid_fullscreen.GetBool( ) )
			VID_UpdateWindowPosAndSize( Common::vid_xpos.GetInt( ), Common::vid_ypos.GetInt( ) );
		Common::vid_xpos.ClearModified( );
		Common::vid_ypos.ClearModified( );
	}
}

/*
============
VID_Init
============
*/
void Video::VID_Init( ) {

	/* Create the video variables so we know how to start the graphics drivers */
	/* Add some console commands that we want to handle */
	Command::Cmd_AddCommand( "vid_restart", VID_Restart_f );
	Command::Cmd_AddCommand( "vid_front", VID_Front_f );		
	/* Start the graphics mode and load refresh DLL */
	VID_CheckChanges( );
}

/*
============
VID_Shutdown
============
*/
void Video::VID_Shutdown( ) {

	Common::Com_Printf( ">>> shuting down renderer >>>\n" );
	if( reflib_active )
		Renderer::Shutdown( );
	Common::Com_Printf( "<<< shuting down renderer <<<\n" );
}


