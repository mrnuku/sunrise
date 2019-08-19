typedef struct {

	byte * data;
	int count;

} cblock_t;

typedef struct {

	int x1, y1, x2, y2;

} dirty_t;

class Screen {

	friend class	View;

private:

	static CVar		scr_viewsize;
	static CVar		scr_conspeed;
	static CVar		scr_showpause;
	static CVar		scr_centertime;
	static CVar		scr_netgraph;
	static CVar		scr_timegraph;
	static CVar		scr_debuggraph;
	static CVar		scr_graphheight;
	static CVar		scr_graphscale;
	static CVar		scr_graphshift;

	static float		scr_con_current;
	static float		scr_conlines;		// lines of console to display

	static struct vrect_s		scr_vrect;		// position of render window

	static int			crosshair_width;
	static int			crosshair_height;

	static bool			scr_initialized; // ready to draw
	static int			scr_draw_loading;

	static dirty_t		scr_dirty;
	static dirty_t		scr_old_dirty[ 2 ];

	static Str			scr_centerstring;
	static timeType		scr_centertime_start; // for slow victory printing
	static timeTypeReal	scr_centertime_off;
	static int			scr_center_lines;
	static int			scr_erase_center;

public:

	static void		SCR_Init( );

	static void		SCR_UpdateScreen( );

	static void		SCR_SizeUp( );
	static void		SCR_SizeDown( );
	static void		SCR_CenterPrint( const Str & str );
	static void		SCR_BeginLoadingPlaque( );
	static void		SCR_EndLoadingPlaque( );

	static void		SCR_DebugGraph( float value, int color );
	static void		SCR_DrawDebugGraph( );

	static void		SCR_AddDirtyPoint( int x, int y );
	static void		SCR_DirtyScreen( );

	// cl_scrn.cpp
	static void		SCR_DrawCenterString( );
	static void		SCR_CheckDrawCenterString( );
	static void		SCR_CalcVrect( );
	static void		SCR_DrawNet( );
	static void		SCR_DrawPause( );
	static void		SCR_DrawLoading( );
	static void		SCR_RunConsole( );
	static void		SCR_DrawConsole( );
	static void		SCR_TileClear( );
	static void		SCR_DrawField( int x, int y, int color, int width, int value );
	static void		SCR_TouchPics( );
	static void		SCR_DrawLayout( );
	static void		SCR_ExecuteLayoutString( const Str & s );
	static void		SCR_DrawStats( );

	static void		SCR_TimeRefresh_f( );
	static void		SCR_Loading_f( );
	static void		SCR_SizeUp_f( );
	static void		SCR_SizeDown_f( );
	static void		SCR_Sky_f( );
};
