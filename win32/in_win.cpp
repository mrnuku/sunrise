#include "precompiled.h"
#pragma hdrstop

#define WORD				WindowsNS::WORD
#define DWORD				WindowsNS::DWORD

// joystick defines and variables
// where should defines be moved?
#define JOY_ABSOLUTE_AXIS	0x00000000		// control like a joystick
#define JOY_RELATIVE_AXIS	0x00000010		// control like a mouse, spinner, trackball
#define	JOY_MAX_AXES		6				// X, Y, Z, R, U, V
#define JOY_AXIS_X			0
#define JOY_AXIS_Y			1
#define JOY_AXIS_Z			2
#define JOY_AXIS_R			3
#define JOY_AXIS_U			4
#define JOY_AXIS_V			5

enum _ControlList {
	AxisNada = 0,
	AxisForward,
	AxisLook,
	AxisSide,
	AxisTurn,
	AxisUp
};

DWORD	dwAxisFlags[ JOY_MAX_AXES ] = {
	JOY_RETURNX,
	JOY_RETURNY,
	JOY_RETURNZ,
	JOY_RETURNR,
	JOY_RETURNU,
	JOY_RETURNV
};

DWORD	dwAxisMap[ JOY_MAX_AXES ];
DWORD	dwControlMap[ JOY_MAX_AXES ];
WindowsNS::PDWORD	pdwRawValue[ JOY_MAX_AXES ];

bool	joy_avail, joy_advancedinit, joy_haspov;
DWORD		joy_oldbuttonstate, joy_oldpovstate;

int			joy_id;
DWORD		joy_flags;
DWORD		joy_numbuttons;

static WindowsNS::JOYINFOEX	ji;

bool	in_appactive;

Vec2		Input::mouseMovement = vec2_origin;

void Input::ProcessRawMouseButtons( word button_flags, signed short button_data ) {
	if( button_flags & RI_MOUSE_LEFT_BUTTON_DOWN )
		Keyboard::Key_Event( KeyEvent( mouse_left, true ), System::Sys_MsgMilliseconds( ) );
	if( button_flags & RI_MOUSE_LEFT_BUTTON_UP )
		Keyboard::Key_Event( KeyEvent( mouse_left, false ), System::Sys_MsgMilliseconds( ) );	
	if( button_flags & RI_MOUSE_RIGHT_BUTTON_DOWN )
		Keyboard::Key_Event( KeyEvent( mouse_right, true ), System::Sys_MsgMilliseconds( ) );	
	if( button_flags & RI_MOUSE_RIGHT_BUTTON_UP )
		Keyboard::Key_Event( KeyEvent( mouse_right, false ), System::Sys_MsgMilliseconds( ) );
	if( button_flags & RI_MOUSE_MIDDLE_BUTTON_DOWN )
		Keyboard::Key_Event( KeyEvent( mouse_middle, true ), System::Sys_MsgMilliseconds( ) );	
	if( button_flags & RI_MOUSE_MIDDLE_BUTTON_UP )
		Keyboard::Key_Event( KeyEvent( mouse_middle, false ), System::Sys_MsgMilliseconds( ) );
	if( button_flags & RI_MOUSE_BUTTON_4_DOWN )
		Keyboard::Key_Event( KeyEvent( mouse_button_4, true ), System::Sys_MsgMilliseconds( ) );	
	if( button_flags & RI_MOUSE_BUTTON_4_UP )
		Keyboard::Key_Event( KeyEvent( mouse_button_4, false ), System::Sys_MsgMilliseconds( ) );
	if( button_flags & RI_MOUSE_BUTTON_5_DOWN )
		Keyboard::Key_Event( KeyEvent( mouse_button_5, true ), System::Sys_MsgMilliseconds( ) );	
	if( button_flags & RI_MOUSE_BUTTON_5_UP )
		Keyboard::Key_Event( KeyEvent( mouse_button_5, false ), System::Sys_MsgMilliseconds( ) );
	if( button_flags & RI_MOUSE_WHEEL ) {
		if( button_data > 0 ) {
			Keyboard::Key_Event( KeyEvent( mouse_wheelup, true ), System::Sys_MsgMilliseconds( ) );
			Keyboard::Key_Event( KeyEvent( mouse_wheelup, false ), System::Sys_MsgMilliseconds( ) );
		} else if( button_data < 0 ) {
			Keyboard::Key_Event( KeyEvent( mouse_wheeldown, true ), System::Sys_MsgMilliseconds( ) );
			Keyboard::Key_Event( KeyEvent( mouse_wheeldown, false ), System::Sys_MsgMilliseconds( ) );
		}
	}
	if( button_flags & 0x0800 ) {
		if( button_data > 0 ) {
			Keyboard::Key_Event( KeyEvent( mouse_wheelright, true ), System::Sys_MsgMilliseconds( ) );
			Keyboard::Key_Event( KeyEvent( mouse_wheelright, false ), System::Sys_MsgMilliseconds( ) );
		} else if( button_data < 0 ) {
			Keyboard::Key_Event( KeyEvent( mouse_wheelleft, true ), System::Sys_MsgMilliseconds( ) );
			Keyboard::Key_Event( KeyEvent( mouse_wheelleft, false ), System::Sys_MsgMilliseconds( ) );
		}
	}
}

/*
============================================================

  MOUSE CONTROL

============================================================
*/

// mouse variables

bool	mlooking;

void Input::IN_MLookDown( ) {
	mlooking = true;
}

void Input::IN_MLookUp( ) {
	mlooking = false;
	if( !freelook.GetBool( ) && lookspring.GetBool( ) )
		IN_CenterView( );
}

int			mouse_buttons;
int			mouse_oldbuttonstate;
WindowsNS::POINT		current_pos;
//int			mouse_x, mouse_y, old_mouse_x, old_mouse_y, mx_accum, my_accum;

int			old_x, old_y;

bool	mouseactive;	// false when not focus app

bool	restore_spi;
bool	mouseinitialized;
int		originalmouseparms[ 3 ], newmouseparms[ 3 ] = {0, 0, 1};
bool	mouseparmsvalid;

int			window_center_x, window_center_y;
WindowsNS::RECT		window_rect;


/*
===========
IN_ActivateMouse

Called when the window gains focus or changes in some way
===========
*/
void Input::IN_ActivateMouse( ) {

	int		width, height;

	if( !mouseinitialized ) return;

	if( !in_mouse.GetBool( ) ) {

		mouseactive = false;
		return;
	}

	if( mouseactive ) return;

	mouseactive = true;

	if( mouseparmsvalid ) restore_spi = WindowsNS::SystemParametersInfo( SPI_SETMOUSE, 0, newmouseparms, 0 ) ? true : false;

	width = WindowsNS::GetSystemMetrics( SM_CXSCREEN );
	height = WindowsNS::GetSystemMetrics( SM_CYSCREEN );

	GetWindowRect( cl_hwnd, &window_rect );
	if( window_rect.left < 0 )
		window_rect.left = 0;
	if( window_rect.top < 0 )
		window_rect.top = 0;
	if( window_rect.right >= width )
		window_rect.right = width-1;
	if( window_rect.bottom >= height-1 )
		window_rect.bottom = height-1;

	window_center_x =( window_rect.right + window_rect.left )/2;
	window_center_y =( window_rect.top + window_rect.bottom )/2;

	WindowsNS::SetCursorPos( window_center_x, window_center_y );

	old_x = window_center_x;
	old_y = window_center_y;

	SetCapture( cl_hwnd );
	ClipCursor( &window_rect );
	while( WindowsNS::ShowCursor( FALSE ) >= 0 );
}


/*
===========
IN_DeactivateMouse

Called when the window loses focus
===========
*/
void Input::IN_DeactivateMouse( ) {

	if( !mouseinitialized )
		return;
	if( !mouseactive )
		return;

	if( restore_spi )
		WindowsNS::SystemParametersInfo( SPI_SETMOUSE, 0, originalmouseparms, 0 );

	mouseactive = false;

	WindowsNS::ClipCursor( NULL );
	WindowsNS::ReleaseCapture( );
	while( WindowsNS::ShowCursor( TRUE ) < 0 );
}

/*
===========
IN_StartupMouse
===========
*/
void Input::IN_StartupMouse( ) {

	if( !in_mouse.GetBool( ) )  return; 

	mouseinitialized = true;
	mouseparmsvalid = WindowsNS::SystemParametersInfo( SPI_GETMOUSE, 0, originalmouseparms, 0 ) ? true : false;
	mouse_buttons = 3;
}

/*
===========
IN_MouseEvent
===========
*/
void Input::IN_MouseEvent( int mstate ) {

	int		i;

	if( !mouseinitialized )
		return;

// perform button actions
	for( i = 0; i<mouse_buttons; i++ )
	{
		if( ( mstate & ( 1<<i ) ) &&
			!( mouse_oldbuttonstate &( 1<<i ) ) )
		{
			Keyboard::Key_Event( KeyEvent( virtualCode_t( mouse_left + i ), true ), System::Sys_MsgMilliseconds( ) );
		}

		if( !( mstate &( 1<<i ) ) &&
			( mouse_oldbuttonstate &( 1<<i ) ) )
		{
				Keyboard::Key_Event( KeyEvent( virtualCode_t( mouse_left + i ), false ), System::Sys_MsgMilliseconds( ) );
		}
	}	
		
	mouse_oldbuttonstate = mstate;
}


/*
===========
IN_MouseMove
===========
*/
void Input::IN_MouseMove( usercmd_t * cmd ) {

	//int		mx, my;

	if( !mouseactive )
		return;

	// find mouse movement
	/*if( !WindowsNS::GetCursorPos( &current_pos ) ) return;

	mx = current_pos.x - window_center_x;
	my = current_pos.y - window_center_y;

	if( m_filter.GetBool( ) ) {

		mouse_x = ( mx + old_mouse_x ) / 2;
		mouse_y = ( my + old_mouse_y ) / 2;

	} else {

		mouse_x = mx;
		mouse_y = my;
	}

	old_mouse_x = mx;
	old_mouse_y = my;

	mouse_x = ( int )( ( float )mouse_x * sensitivity.GetFloat( ) );
	mouse_y = ( int )( ( float )mouse_y * sensitivity.GetFloat( ) );*/

	mouseMovement *= sensitivity.GetFloat( );

	//Str renderStatText;
	/*sprintf( renderStatText, "WM_MOUSE: ( %i %i )", mouse_x, mouse_y );
	Font::Find( "consoleFont" )->SetColor( color_yellow );
	Font::Find( "consoleFont" )->PushText( Vec2( 5.0f, 64.0f ), renderStatText );*/

	/*sprintf( renderStatText, "WM_INPUT: %i ( %.2f %.2f )", m_filter.GetBool( ), mouseMovement[ 0 ], mouseMovement[ 1 ] );
	Font::Find( "consoleFont" )->SetColor( color_red );
	Font::Find( "consoleFont" )->PushText( Vec2( 5.0f, 32.0f ), renderStatText );*/

	// add mouse X/Y movement to cmd
	if( ( in_strafe.state & 1 ) || ( lookstrafe.GetBool( ) && mlooking ) ) {
		
		cmd->sidemove += ( short )( m_side.GetInt( ) * mouseMovement[ 0 ] );

	} else {
		
		Client::cl.viewangles[ YAW ] -= m_yaw.GetFloat( ) * mouseMovement[ 0 ];
	}

	if( ( mlooking || freelook.GetBool( ) ) && !( in_strafe.state & 1 ) ) {

		Client::cl.viewangles[ PITCH ] += m_pitch.GetFloat( ) * mouseMovement[ 1 ];

	} else {

		cmd->forwardmove -= ( short )( m_forward.GetInt( ) * mouseMovement[ 1 ] );
	}

	//if( mouseMovement.LengthSqr( ) ) WindowsNS::SetCursorPos( window_center_x, window_center_y );

	mouseMovement.Zero( );

// add mouse X/Y movement to cmd
	/*if( ( in_strafe.state & 1 ) || ( lookstrafe.GetBool( ) && mlooking ) ) cmd->sidemove += m_side.GetInt( ) * mouse_x;
	else Client::cl.viewangles[ YAW ] -= m_yaw.GetFloat( ) * ( float )mouse_x;

	if( ( mlooking || freelook.GetBool( ) ) && !( in_strafe.state & 1 ) ) {

		Client::cl.viewangles[ PITCH ] += m_pitch.GetFloat( ) * ( float )mouse_y;

	} else {

		cmd->forwardmove -= m_forward.GetInt( ) * mouse_y;
	}*/

	// force the mouse to the center, so there's room to move
	//if( mx || my ) WindowsNS::SetCursorPos( window_center_x, window_center_y );
}


/*
=========================================================================

VIEW CENTERING

=========================================================================
*/

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         (0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        (0x02)
#endif

/*
===========
IN_Init
===========
*/
void Input::IN_Init( ) {

	Command::Cmd_AddCommand( "+mlook", IN_MLookDown );
	Command::Cmd_AddCommand( "-mlook", IN_MLookUp );

	Command::Cmd_AddCommand( "joy_advancedupdate", Joy_AdvancedUpdate_f );

	IN_StartupMouse( );
	IN_StartupJoystick( );

	DynamicModule * user32Module = new DynamicModule( "user32.dll" );

	typedef struct {

		WindowsNS::USHORT usUsagePage; // Toplevel collection UsagePage
		WindowsNS::USHORT usUsage;     // Toplevel collection Usage
		DWORD dwFlags;
		WindowsNS::HWND hwndTarget;    // Target hwnd. NULL = follows keyboard focus

	} locRAWINPUTDEVICE;

	typedef bool ( WINAPI * RegisterRawInputDevicesProc )( locRAWINPUTDEVICE * pRawInputDevices, uint uiNumDevices, uint cbSize );
	RegisterRawInputDevicesProc rridFunc = ( RegisterRawInputDevicesProc )user32Module->FindProc( "RegisterRawInputDevices" );

	locRAWINPUTDEVICE Rid;
	Rid.usUsagePage = HID_USAGE_PAGE_GENERIC; 
	Rid.usUsage = HID_USAGE_GENERIC_MOUSE; 
	Rid.dwFlags = RIDEV_INPUTSINK;   
	Rid.hwndTarget = cl_hwnd;
	rridFunc( &Rid, 1, sizeof( locRAWINPUTDEVICE ) );
}

/*
===========
IN_Shutdown
===========
*/
void Input::IN_Shutdown( ) {

	IN_DeactivateMouse( );
}


/*
===========
IN_Activate

Called when the main window gains or loses focus.
The window may have been destroyed and recreated
between a deactivate and an activate.
===========
*/
void Input::IN_Activate( bool active ) {

	in_appactive = active;
	mouseactive = !active;		// force a new window check or turn off
	mouseMovement.Zero( );
	if( Client::cls.key_dest != key_console ) WindowsNS::SetCursorPos( window_center_x, window_center_y );
}


/*
==================
IN_Frame

Called every frame, even if not generating commands
==================
*/
void Input::IN_Frame( ) {

	if( !mouseinitialized ) return;

	if( !in_mouse.GetBool( ) || !in_appactive ) {

		IN_DeactivateMouse( );
		return;
	}

	if( !Client::cl.refresh_prepped || Client::cls.key_dest == key_console || Client::cls.key_dest == key_menu ) {

		// temporarily deactivate if in fullscreen
		if( CVarSystem::GetCVarBool( "vid_fullscreen" ) ) {

			IN_DeactivateMouse( );
			return;
		}
	}

	IN_ActivateMouse( );
}

/*
===========
IN_Move
===========
*/
void Input::IN_Move( usercmd_t * cmd ) {

	IN_MouseMove( cmd );

	if( ActiveApp ) IN_JoyMove( cmd );
}


/*
===================
IN_ClearStates
===================
*/
void Input::IN_ClearStates( ) {

	//mx_accum = 0;
	//my_accum = 0;
	mouse_oldbuttonstate = 0;
}


/*
=========================================================================

JOYSTICK

=========================================================================
*/

/* 
=============== 
IN_StartupJoystick 
=============== 
*/  
void Input::IN_StartupJoystick( )  {
 
	int			numdevs;
	WindowsNS::JOYCAPS		jc;
	WindowsNS::MMRESULT	mmr;

 	// assume no joystick
	joy_avail = false; 

	// abort startup if user requests no joystick
	if( !in_joystick.GetBool( ) )  return; 
 
	// verify joystick driver is present
	if( ( numdevs = WindowsNS::joyGetNumDevs( ) ) == 0 )
	{
//		Common::Com_Printf( "\njoystick not found -- driver not present\n\n" );
		return;
	}

	// cycle through the joystick s for the first valid one
	for( joy_id = 0; joy_id<numdevs; joy_id++ )
	{
		Common::Com_Memset( &ji, 0, sizeof( ji ) );
		ji.dwSize = sizeof( ji );
		ji.dwFlags = JOY_RETURNCENTERED;

		if( ( mmr = joyGetPosEx( joy_id, &ji ) ) == JOYERR_NOERROR )
			break;
	} 

	// abort startup if we didn't find a valid joystick
	if( mmr != JOYERR_NOERROR )
	{
		Common::Com_Printf( "\njoystick not found -- no valid joysticks( %x )\n\n", mmr );
		return;
	}

	// get the capabilities of the selected joystick
	// abort startup if command fails
	Common::Com_Memset( &jc, 0, sizeof( jc ) );
	if( ( mmr = joyGetDevCaps( joy_id, &jc, sizeof( jc ) ) ) != JOYERR_NOERROR )
	{
		Common::Com_Printf( "\njoystick not found -- invalid joystick capabilities( %x )\n\n", mmr ); 
		return;
	}

	// save the joystick's number of buttons and POV status
	joy_numbuttons = jc.wNumButtons;
	joy_haspov = ( jc.wCaps & JOYCAPS_HASPOV ) ? true : false;

	// old button and POV states default to no buttons pressed
	joy_oldbuttonstate = joy_oldpovstate = 0;

	// mark the joystick as available and advanced initialization not completed
	// this is needed as cvars are not available during initialization

	joy_avail = true; 
	joy_advancedinit = false;

	Common::Com_Printf( "\njoystick detected\n\n" ); 
}


/*
===========
RawValuePointer
===========
*/
WindowsNS::PDWORD RawValuePointer( int axis ) {

	switch( axis )
	{
	case JOY_AXIS_X:
		return &ji.dwXpos;
	case JOY_AXIS_Y:
		return &ji.dwYpos;
	case JOY_AXIS_Z:
		return &ji.dwZpos;
	case JOY_AXIS_R:
		return &ji.dwRpos;
	case JOY_AXIS_U:
		return &ji.dwUpos;
	case JOY_AXIS_V:
		return &ji.dwVpos;
	}

	return NULL;
}


/*
===========
Joy_AdvancedUpdate_f
===========
*/
void Input::Joy_AdvancedUpdate_f( ) {

	// called once by IN_ReadJoystick and by user whenever an update is needed
	// cvars are now available
	int	i;
	DWORD dwTemp;

	// initialize all the maps
	for( i = 0; i < JOY_MAX_AXES; i++ )
	{
		dwAxisMap[ i ] = AxisNada;
		dwControlMap[ i ] = JOY_ABSOLUTE_AXIS;
		pdwRawValue[ i ] = RawValuePointer( i );
	}

	if( !joy_advanced.GetBool( ) )
	{
		// default joystick initialization
		// 2 axes only with joystick control
		dwAxisMap[ JOY_AXIS_X ] = AxisTurn;
		// dwControlMap[ JOY_AXIS_X ] = JOY_ABSOLUTE_AXIS;
		dwAxisMap[ JOY_AXIS_Y ] = AxisForward;
		// dwControlMap[ JOY_AXIS_Y ] = JOY_ABSOLUTE_AXIS;

	} else {

		if( joy_name.GetString( ) != "joystick" ) {

			// notify user of advanced controller
			Common::Com_Printf( "\n%s configured\n\n", joy_name.GetString( ).c_str( ) );
		}

		// advanced initialization here
		// data supplied by user via joy_axisn cvars
		dwTemp = ( DWORD )joy_advaxisx.GetInt( );
		dwAxisMap[ JOY_AXIS_X ] = dwTemp & 0x0000000F;
		dwControlMap[ JOY_AXIS_X ] = dwTemp & JOY_RELATIVE_AXIS;

		dwTemp = ( DWORD )joy_advaxisy.GetInt( );
		dwAxisMap[ JOY_AXIS_Y ] = dwTemp & 0x0000000F;
		dwControlMap[ JOY_AXIS_Y ] = dwTemp & JOY_RELATIVE_AXIS;

		dwTemp = ( DWORD )joy_advaxisz.GetInt( );
		dwAxisMap[ JOY_AXIS_Z ] = dwTemp & 0x0000000F;
		dwControlMap[ JOY_AXIS_Z ] = dwTemp & JOY_RELATIVE_AXIS;

		dwTemp = ( DWORD )joy_advaxisr.GetInt( );
		dwAxisMap[ JOY_AXIS_R ] = dwTemp & 0x0000000F;
		dwControlMap[ JOY_AXIS_R ] = dwTemp & JOY_RELATIVE_AXIS;

		dwTemp = ( DWORD )joy_advaxisu.GetInt( );
		dwAxisMap[ JOY_AXIS_U ] = dwTemp & 0x0000000F;
		dwControlMap[ JOY_AXIS_U ] = dwTemp & JOY_RELATIVE_AXIS;

		dwTemp = ( DWORD )joy_advaxisv.GetInt( );
		dwAxisMap[ JOY_AXIS_V ] = dwTemp & 0x0000000F;
		dwControlMap[ JOY_AXIS_V ] = dwTemp & JOY_RELATIVE_AXIS;
	}

	// compute the axes to collect from DirectInput
	joy_flags = JOY_RETURNCENTERED | JOY_RETURNBUTTONS | JOY_RETURNPOV;
	for( i = 0; i < JOY_MAX_AXES; i++ )
	{
		if( dwAxisMap[ i ] != AxisNada )
		{
			joy_flags |= dwAxisFlags[ i ];
		}
	}
}


/*
===========
IN_Commands
===========
*/
void Input::IN_Commands( ) {

	int		key_index;
	DWORD	buttonstate, povstate;

	if( !joy_avail )
	{
		return;
	}

	
	// loop through the joystick buttons
	// key a joystick event or auxillary event for higher number buttons for each state change
	buttonstate = ji.dwButtons;
	for( DWORD i = 0; i < joy_numbuttons; i++ )
	{
		if( ( buttonstate &( 1<<i ) ) && !( joy_oldbuttonstate &( 1<<i ) ) )
		{
			key_index =( i < 4 ) ? joystick_1 : vs_aux_1;
			Keyboard::Key_Event( KeyEvent( virtualCode_t( key_index + i ), true ), 0 );
		}

		if( !( buttonstate &( 1<<i ) ) &&( joy_oldbuttonstate &( 1<<i ) ) )
		{
			key_index =( i < 4 ) ? joystick_1 : vs_aux_1;
			Keyboard::Key_Event( KeyEvent( virtualCode_t( key_index + i ), false ), 0 );
		}
	}
	joy_oldbuttonstate = buttonstate;

	if( joy_haspov )
	{
		// convert POV information into 4 bits of state information
		// this avoids any potential problems related to moving from one
		// direction to another without going through the center position
		povstate = 0;
		if( ji.dwPOV != JOY_POVCENTERED )
		{
			if( ji.dwPOV == JOY_POVFORWARD )
				povstate |= 0x01;
			if( ji.dwPOV == JOY_POVRIGHT )
				povstate |= 0x02;
			if( ji.dwPOV == JOY_POVBACKWARD )
				povstate |= 0x04;
			if( ji.dwPOV == JOY_POVLEFT )
				povstate |= 0x08;
		}
		// determine which bits have changed and key an auxillary event for each change
		for( int i = 0; i < 4; i++ )
		{
			if( ( povstate &( 1<<i ) ) && !( joy_oldpovstate &( 1<<i ) ) )
			{
				Keyboard::Key_Event( KeyEvent( virtualCode_t( vs_aux_29 + i ), true ), 0 );
			}

			if( !( povstate &( 1<<i ) ) &&( joy_oldpovstate &( 1<<i ) ) )
			{
				Keyboard::Key_Event( KeyEvent( virtualCode_t( vs_aux_29 + i ), false ), 0 );
			}
		}
		joy_oldpovstate = povstate;
	}
}


/* 
=============== 
IN_ReadJoystick
=============== 
*/  
bool Input::IN_ReadJoystick( ) {


	Common::Com_Memset( &ji, 0, sizeof( ji ) );
	ji.dwSize = sizeof( ji );
	ji.dwFlags = joy_flags;

	if( joyGetPosEx( joy_id, &ji ) == JOYERR_NOERROR )
	{
		return true;
	}
	else
	{
		// read error occurred
		// turning off the joystick seems too harsh for 1 read error, \
		// but what should be done?
		// Common::Com_Printf( "IN_ReadJoystick: no response\n" );
		// joy_avail = false;
		return false;
	}
}


/*
===========
IN_JoyMove
===========
*/
void Input::IN_JoyMove( usercmd_t * cmd ) {

	float	speed, aspeed;
	float	fAxisValue;
	int		i;

	// complete initialization if first time in
	// this is needed as cvars are not available at initialization time
	if( joy_advancedinit != true )
	{
		Joy_AdvancedUpdate_f( );
		joy_advancedinit = true;
	}

	// verify joystick is available and that the user wants to use it
	if( !joy_avail || !in_joystick.GetBool( ) ) return;
 
	// collect the joystick data, if possible
	if( IN_ReadJoystick( ) != true )
	{
		return;
	}


	if ( (in_speed.state & 1) ^ CVarSystem::GetCVarInt( "cl_run" ) ) speed = 2;
	else speed = 1;
	aspeed = speed * ( float )Client::cls.frametime;

	// loop through the axes
	for( i = 0; i < JOY_MAX_AXES; i++ )
	{
		// get the floating point zero-centered, potentially-inverted data for the current axis
		fAxisValue =( float ) * pdwRawValue[ i ];
		// move centerpoint to zero
		fAxisValue -= 32768.0f;

		// convert range from -32768..32767 to -1..1 
		fAxisValue /= 32768.0f;

		switch( dwAxisMap[ i ] ) {

		case AxisForward:

			if( !joy_advanced.GetBool( ) && mlooking ) {

				// user wants forward control to become look control
				if( fabs( fAxisValue ) > joy_pitchthreshold.GetFloat( ) )
				{		
					// if mouse invert is on, invert the joystick pitch value
					// only absolute control support here( joy_advanced is false )
					if( m_pitch.GetFloat( ) < 0.0f ) Client::cl.viewangles[ PITCH ] -=( fAxisValue * joy_pitchsensitivity.GetFloat( ) ) * aspeed * CVarSystem::GetCVarFloat( "cl_pitchspeed" );
					else Client::cl.viewangles[ PITCH ] += ( fAxisValue * joy_pitchsensitivity.GetFloat( ) ) * aspeed * CVarSystem::GetCVarFloat( "cl_pitchspeed" );
				}

			} else {

				// user wants forward control to be forward control
				if( fabs( fAxisValue ) > joy_forwardthreshold.GetFloat( ) ) cmd->forwardmove += ( short )( ( fAxisValue * joy_forwardsensitivity.GetFloat( ) ) * speed * CVarSystem::GetCVarFloat( "cl_forwardspeed" ) );
			}
			break;

		case AxisSide:

			if( fabs( fAxisValue ) > joy_sidethreshold.GetFloat( ) ) cmd->sidemove += ( short )( ( fAxisValue * joy_sidesensitivity.GetFloat( ) ) * speed * CVarSystem::GetCVarFloat( "cl_sidespeed" ) );
			break;

		case AxisUp:

			if( fabs( fAxisValue ) > joy_upthreshold.GetFloat( ) ) cmd->upmove += ( short )( ( fAxisValue * joy_upsensitivity.GetFloat( ) ) * speed * CVarSystem::GetCVarFloat( "cl_upspeed" ) );
			break;

		case AxisTurn:

			if( ( in_strafe.state & 1 ) || ( lookstrafe.GetBool( ) && mlooking ) ) {

				// user wants turn control to become side control
				if( fabs( fAxisValue ) > joy_sidethreshold.GetFloat( ) ) cmd->sidemove -= ( short )( ( fAxisValue * joy_sidesensitivity.GetFloat( ) ) * speed * CVarSystem::GetCVarFloat( "cl_sidespeed" ) );
			
			} else {

				// user wants turn control to be turn control
				if( fabs( fAxisValue ) > joy_yawthreshold.GetFloat( ) ) {

					if( dwControlMap[ i ] == JOY_ABSOLUTE_AXIS ) Client::cl.viewangles[ YAW ] +=( fAxisValue * joy_yawsensitivity.GetFloat( ) ) * aspeed * CVarSystem::GetCVarFloat( "cl_yawspeed" );
					else Client::cl.viewangles[ YAW ] +=( fAxisValue * joy_yawsensitivity.GetFloat( ) ) * speed * 180.0f;
				}
			}
			break;

		case AxisLook:

			if( mlooking ) {

				if( fabs( fAxisValue ) > joy_pitchthreshold.GetFloat( ) ) {

					// pitch movement detected and pitch movement desired by user
					if( dwControlMap[ i ] == JOY_ABSOLUTE_AXIS ) Client::cl.viewangles[ PITCH ] +=( fAxisValue * joy_pitchsensitivity.GetFloat( ) ) * aspeed * CVarSystem::GetCVarFloat( "cl_pitchspeed" );
					else Client::cl.viewangles[ PITCH ] +=( fAxisValue * joy_pitchsensitivity.GetFloat( ) ) * speed * 180.0f;
				}
			}
			break;

		default:
			break;
		}
	}
}

