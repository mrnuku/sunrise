/// joystick buttons
#define	K_JOY1			203
#define	K_JOY2			204
#define	K_JOY3			205
#define	K_JOY4			206

/// aux keys are for multi-buttoned joysticks to generate so they can use
/// the normal binding process
#define	K_AUX1			207
#define	K_AUX2			208
#define	K_AUX3			209
#define	K_AUX4			210
#define	K_AUX5			211
#define	K_AUX6			212
#define	K_AUX7			213
#define	K_AUX8			214
#define	K_AUX9			215
#define	K_AUX10			216
#define	K_AUX11			217
#define	K_AUX12			218
#define	K_AUX13			219
#define	K_AUX14			220
#define	K_AUX15			221
#define	K_AUX16			222
#define	K_AUX17			223
#define	K_AUX18			224
#define	K_AUX19			225
#define	K_AUX20			226
#define	K_AUX21			227
#define	K_AUX22			228
#define	K_AUX23			229
#define	K_AUX24			230
#define	K_AUX25			231
#define	K_AUX26			232
#define	K_AUX27			233
#define	K_AUX28			234
#define	K_AUX29			235
#define	K_AUX30			236
#define	K_AUX31			237
#define	K_AUX32			238

#define M_WHEELDOWN		5
#define M_WHEELUP		6

typedef enum {

	vs_none = -1,

	key_esc,

	key_f1,
	key_f2,
	key_f3,
	key_f4,
	key_f5,
	key_f6,
	key_f7,
	key_f8,
	key_f9,
	key_f10,
	key_f11,
	key_f12,

	key_space,
	key_tab,
	key_backspace,
	key_return,

	key_caps_lock,
	key_shift,
	key_ctrl,
	key_alt,
	key_left_shift,
	key_left_ctrl,
	key_left_alt,
	key_right_shift,
	key_right_ctrl,
	key_right_alt,

	key_comma,
	key_dot,
	key_minus,
	key_plus,

	key_oem_1,
	key_oem_2,
	key_oem_3,
	key_oem_4,
	key_oem_5,
	key_oem_6,
	key_oem_7,
	key_oem_8,
	key_oem_ax,
	key_oem_102,

	key_0,
	key_1,
	key_2,
	key_3,
	key_4,
	key_5,
	key_6,
	key_7,
	key_8,
	key_9,

	key_a,
	key_b,
	key_c,
	key_d,
	key_e,
	key_f,
	key_g,
	key_h,
	key_i,
	key_j,
	key_k,
	key_l,
	key_m,
	key_n,
	key_o,
	key_p,
	key_q,
	key_r,
	key_s,
	key_t,
	key_u,
	key_v,
	key_w,
	key_x,
	key_y,
	key_z,

	key_printscreen,
	key_scroll_lock,
	key_pause,

	key_insert,
	key_delete,
	key_home,
	key_end,
	key_pageup,
	key_pagedown,

	key_left,
	key_up,
	key_down,
	key_right,

	key_num_lock,
	key_num_0,
	key_num_1,
	key_num_2,
	key_num_3,
	key_num_4,
	key_num_5,
	key_num_6,
	key_num_7,
	key_num_8,
	key_num_9,
	key_num_slash,
	key_num_star,
	key_num_minus,
	key_num_plus,
	key_num_return,
	key_num_comma,

	mouse_left,
	mouse_right,
	mouse_middle,
	mouse_button_4,
	mouse_button_5,
	mouse_wheelup,
	mouse_wheeldown,
	mouse_wheelleft,
	mouse_wheelright,

	joystick_1,
	joystick_2,
	joystick_3,
	joystick_4,

	vs_aux_1,
	vs_aux_2,
	vs_aux_3,
	vs_aux_4,
	vs_aux_5,
	vs_aux_6,
	vs_aux_7,
	vs_aux_8,
	vs_aux_9,
	vs_aux_10,
	vs_aux_11,
	vs_aux_12,
	vs_aux_13,
	vs_aux_14,
	vs_aux_15,
	vs_aux_16,
	vs_aux_17,
	vs_aux_18,
	vs_aux_19,
	vs_aux_20,
	vs_aux_21,
	vs_aux_22,
	vs_aux_23,
	vs_aux_24,
	vs_aux_25,
	vs_aux_26,
	vs_aux_27,
	vs_aux_28,
	vs_aux_29,
	vs_aux_30,
	vs_aux_31,
	vs_aux_32,

	vs_endlist,

} virtualCode_t;

typedef struct {

	const Str								name;
	virtualCode_t							internalCode;
	dword									externalCode;

} keyTranslation_t;

class KeyEvent {

private:

	static List< virtualCode_t >			extFrameEvents;
	static List< virtualCode_t >			extLastFrameEvents;

/// Windows specific stuffs...
	static keyTranslation_t					keyTranslationTable[ ];
	static byte								keyStateBuffer[ 256 ];
	static WindowsNS::HKL					keyLayout;
	static void								RefreshKeyStateBuffer( );
	static virtualCode_t					LookupVirtualCode( dword win_wparam );

public:

	/// control - shift - alt filter list
	static List< virtualCode_t >			csaFilter;

	/// checks the given key states and returns true if only one is down
	static bool								FilterOnly( virtualCode_t vc, const List< virtualCode_t > & filter_list = KeyEvent::csaFilter );

	static bool								IsDown( virtualCode_t vc );
	static const Str &						LookupKeyName( virtualCode_t vc );
	static virtualCode_t					LookupKeyCode( const Str & name );
	static void								Init( );
	static void								Shutdown( );
	static void								Frame( );

private:

	virtualCode_t							virtualCode;
	char									asciiCode;
	uint									uniCode;

	bool									keyDown;
	bool									keyRepeating;

public:

											/// default constructor
											KeyEvent( ) : virtualCode( vs_none ), asciiCode( '\0' ), uniCode( 0 ), keyDown( false ), keyRepeating( false ) { }

											/// for non-keyboard events
											KeyEvent( virtualCode_t vc, bool down );

											/// for keyboard events, calls Windows keyboard translator
											KeyEvent( dword win_wparam, dword win_lparam, bool down );
											
	virtualCode_t							GetVirtual( ) const { return this->virtualCode; }
	operator								virtualCode_t( ) const { return this->virtualCode; }
	char									GetAscii( ) const { return this->asciiCode; }
	uint									GetUnicode( ) const { return uniCode; }

	/// returns current key state
	bool									IsDown( ) const { return this->keyDown; }

	/// false when down message first comes in
	bool									IsRepeating( ) const { return this->keyRepeating; }

	const Str &								GetName( ) const { return LookupKeyName( this->virtualCode ); }
};

/*typedef struct {

	const Str	name;
	int			keynum;

} keyname_t;*/

#ifndef DEDICATED_ONLY

class Keyboard {

private:

	/*static const keyname_t	keynames[ ];
	static List< Str >		key_lines;
	static int				key_linepos;
	static int				shift_down;
	static int				anykeydown;

	static int				edit_line;
	static int				history_line;

	static int				key_waiting;
	static Str				keybindings[ 256 ];

	/// if true, can't be rebound while in console
	static bool				consolekeys[ 256 ];

	/// if true, can't be rebound while in menu
	static bool				menubound[ 256 ];

	/// if > 1, it is autorepeating
	static int				key_repeats[ 256 ];
	static bool				keydown[ 256 ];

	static bool				chat_team;
	static Str				chat_buffer;*/

	static Str				g_bindings[ 256 ];

	static int				lm_Bind( LuaState & state );
	static int				lm_Get( LuaState & state );
	static int				lm___index( LuaState & state );
	static int				lg_registerInput( LuaState & state );
	
public:

	/// Called by the system between frames for both key up and key down events. Should NOT be called during an interrupt!
	static void			Key_Event( const KeyEvent & key_event, unsigned time );

	static void			Key_Init( );

	/// Writes lines containing "bind key value"
	static void			Key_WriteBindings( FileBase * f );
	static void			Key_SetBinding( virtualCode_t keynum, const Str & binding );
	static const Str &	Key_GetBinding( virtualCode_t keynum );
	static void			Key_ClearStates( );
	//static int			Key_GetKey( );
	//static void			Key_ClearTyping( );

	/// Interactive line editing and console scrollback
	//static void			Key_Console( const KeyEvent & key_event );
	//static void			Key_Message( int key );

	/// Returns a key number to be used to index keybindings[] by looking at the given string. Single ascii characters return themselves, while the K_* names are matched up.
	//static int			Key_StringToKeynum( const Str & str );

	/// Returns a string( either a single ascii char, or a K_* name ) for the given keynum. FIXME: handle quote special ( general escape sequence? )
	//static Str			Key_KeynumToString( int keynum );
	//static Str			Key_KeynumToName( int keynum );

	/*static const Str &	Key_GetEditLine( );
	static int			Key_GetEditLineCursorPos( );
	static bool			Key_AnyKeyDown( );
	static bool			Key_ChatToTeam( );
	static const Str &	Key_GetChatLine( );

	static const Str &	Key_GetKeyBindByCommand( const Str & command );
	static bool			Key_ClearBindByCommand( const Str & command );
	static bool			Key_FindKeyBindingsByCommand( const Str & command, int * output, int count );
	static bool			Key_KeyIsDown( int key );
	static void			Key_SetTeamChat( bool tc );*/

	static void			Key_Unbind_f( );
	static void			Key_Unbindall_f( );
	static void			Key_Bind_f( );
	static void			Key_Bindlist_f( );
	static void			Con_ToggleConsole_f( );

	//static void			CompleteCommand( );
};

#endif
