//
// console
//

#define	NUM_CON_TIMES 4

typedef struct {

	bool	initialized;

	Str		text;
	List< int >	lines;

	int		current;		// line where next message will be printed
	int		x;				// offset in current line for next print
	int		display;		// bottom of console displays this line

	int		ormask;			// high bit mask for colored characters

	int 	linewidth;		// characters across screen
	int		totallines;		// total lines in console scrollback

	float	cursorspeed;

	int		vislines;

	float	times[ NUM_CON_TIMES ];	// cls.realtime time the line was generated
								// for transparent notify lines
} console_t;

extern	console_t	con;

#define		MAXCMDLINE	256

class Console {

private:

	static CVar			rcon_client_password;
	static CVar			rcon_address;
	static CVar			con_notifytime;

public:

	static void			Con_DrawString( int x, int y, const Str & str );
	static void			Con_DrawAltString( int x, int y, const Str & str );	// toggle high bit
	static void			Con_DrawCharacter( int cx, int line, int num );
	static void			Con_CheckResize( );
	static void			Con_Init( );
	static void			Con_DrawConsole( float frac );
	static void			Con_Print( const Str & txt );
	static void			Con_CenteredPrint( const Str & text );
	static void			Con_Clear_f( );
	static void			Con_DrawNotify( );
	static void			Con_ClearNotify( );
	static void			Con_ToggleConsole_f( );
	static void			Con_ToggleChat_f( );
	static void			Con_Dump_f( );
	static void			Con_MessageMode_f( );
	static void			Con_MessageMode2_f( );
	static void			Con_DrawInput( );

	static void			Rcon_f( );
};
