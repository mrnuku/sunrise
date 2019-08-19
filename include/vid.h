// vid.h -- video driver defs

typedef struct vrect_s {

	int				x, y, width, height;

} vrect_t;

typedef struct {

	unsigned		width, height;			// coordinates from main game

} viddef_t;

extern	viddef_t	viddef;				// global video state

typedef struct vidmode_s {

	const Str	description;
	int         width, height;
	int         mode;

} vidmode_t;

class Video {

private:

	static VecT2i		g_screenResolution;

	/*static void			ScreenSizeCallback( void * s );
	static void			BrightnessCallback( void * s );
	static void			ResetDefaults( void * unused );
	static void			ApplyChanges( void * unused );
	static void			CancelChanges( void * unused );

	static const Str	resolutions[ ];

	static menuframework_s  s_software_menu;
	static menuframework_s	s_opengl_menu;
	static menuframework_s * s_current_menu;
	static int				s_current_menu_index;

	static menulist_s		s_mode_list[ 2 ];
	static menuslider_s		s_tq_slider;
	static menuslider_s		s_screensize_slider[ 2 ];
	static menuslider_s		s_brightness_slider[ 2 ];
	static menulist_s  		s_fs_box[ 2 ];
	static menulist_s  		s_stipple_box;
	static menulist_s  		s_finish_box;
	static menuaction_s		s_cancel_action[ 2 ];
	static menuaction_s		s_defaults_action[ 2 ];*/

public:
	static viddef_t		viddef;
// Video module initialisation etc
	static void			VID_Init( );
	static void			VID_Shutdown( );
	static void			VID_CheckChanges( );

	/*static void			VID_MenuInit(  );
	static void			VID_MenuDraw( );
	static const Str	VID_MenuKey( int );*/

	static void			AppActivate( bool fActive, bool minimize );
	static void			VID_Restart_f( );
	static void			VID_Front_f( );
	static void			VID_UpdateWindowPosAndSize( int x, int y );
	static void			VID_NewWindow( int width, int height );
	static bool			VID_LoadRefresh( );

	static const VecT2i &	GetResolution( );
};

INLINE const VecT2i & Video::GetResolution( ) {
	return g_screenResolution;
}
