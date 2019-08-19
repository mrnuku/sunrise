// input.h -- external( non-keyboard ) input devices

typedef struct {

	int			down[ 2 ];		// key nums holding it down
	unsigned	downtime;		// msec timestamp
	unsigned	msec;			// msec down this frame
	int			state;
} kbutton_t;

extern	kbutton_t	in_mlook, in_klook;
extern 	kbutton_t 	in_strafe;
extern 	kbutton_t 	in_speed;

class Input {

private:
	
	static CVar		freelook;
	static CVar		lookspring;
	static CVar		lookstrafe;
	static CVar		sensitivity;

	static CVar		m_pitch;
	static CVar		m_yaw;
	static CVar		m_forward;
	static CVar		m_side;

	static CVar		m_filter;
	static CVar		in_mouse;

	// joystick variables
	static CVar		in_joystick;
	static CVar		joy_name;
	static CVar		joy_advanced;
	static CVar		joy_advaxisx;
	static CVar		joy_advaxisy;
	static CVar		joy_advaxisz;
	static CVar		joy_advaxisr;
	static CVar		joy_advaxisu;
	static CVar		joy_advaxisv;
	static CVar		joy_forwardthreshold;
	static CVar		joy_sidethreshold;
	static CVar		joy_upthreshold;
	static CVar		joy_pitchthreshold;
	static CVar		joy_yawthreshold;
	static CVar		joy_forwardsensitivity;
	static CVar		joy_sidesensitivity;
	static CVar		joy_upsensitivity;
	static CVar		joy_pitchsensitivity;
	static CVar		joy_yawsensitivity;

	static Vec2		mouseMovement;

public:

	static void			ProcessRawMouseButtons( word button_flags, signed short button_data );
	static void			AddMouseMovement( const Vec2 & coords ) { mouseMovement += coords; }

	static void			IN_Init( );
	static void			IN_Shutdown( );
	static void			IN_Commands( ); // oportunity for devices to stick commands on the script buffer
	static void			IN_Frame( );
	static void			IN_Move( usercmd_t * cmd ); // add additional movement on top of the keyboard move cmd
	static void			IN_Activate( bool active );
	static void			IN_MouseEvent( int mstate );
	static void			IN_MLookDown( );
	static void			IN_MLookUp( );
	static void			IN_ActivateMouse( );
	static void			IN_DeactivateMouse( );
	static void			IN_StartupMouse( );
	static void			IN_MouseMove( usercmd_t * cmd );
	static void			IN_ClearStates( );
	static void			IN_StartupJoystick( );
	static void			IN_JoyMove( usercmd_t * cmd );
	static void			Joy_AdvancedUpdate_f( );
	static bool			IN_ReadJoystick( );

	// cl_input.cpp
	static void			IN_KeyDown( kbutton_t * b );
	static void			IN_KeyUp( kbutton_t * b );
	static void			IN_KLookDown( );
	static void			IN_KLookUp( );
	static void			IN_UpDown( );
	static void			IN_UpUp( );
	static void			IN_DownDown( );
	static void			IN_DownUp( );
	static void			IN_LeftDown( );
	static void			IN_LeftUp( );
	static void			IN_RightDown( );
	static void			IN_RightUp( );
	static void			IN_ForwardDown( );
	static void			IN_ForwardUp( );
	static void			IN_BackDown( );
	static void			IN_BackUp( );
	static void			IN_LookupDown( );
	static void			IN_LookupUp( );
	static void			IN_LookdownDown( );
	static void			IN_LookdownUp( );
	static void			IN_MoveleftDown( );
	static void			IN_MoveleftUp( );
	static void			IN_MoverightDown( );
	static void			IN_MoverightUp( );
	static void			IN_SpeedDown( );
	static void			IN_SpeedUp( );
	static void			IN_StrafeDown( );
	static void			IN_StrafeUp( );
	static void			IN_AttackDown( );
	static void			IN_AttackUp( );
	static void			IN_UseDown( );
	static void			IN_UseUp( );
	static void			IN_Impulse( );

	static void			IN_CenterView( );
};
