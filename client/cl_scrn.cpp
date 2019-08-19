#include "precompiled.h"
#pragma hdrstop

/*
full screen console
put up loading plaque
blanked background with loading plaque
blanked background with menu
cinematics
full screen image for quit and victory
end of unit intermissions
*/
float Screen::scr_con_current; // aproaches scr_conlines at scr_conspeed
float Screen::scr_conlines; // 0.0 to 1.0 lines of console to display

bool Screen::scr_initialized = false; // ready to draw

int Screen::scr_draw_loading;

vrect_t Screen::scr_vrect; // position of render window on screen

dirty_t Screen::scr_dirty;
dirty_t Screen::scr_old_dirty[ 2 ];

int Screen::crosshair_width;
int Screen::crosshair_height;

timeType Screen::scr_centertime_start; // for slow victory printing
timeTypeReal Screen::scr_centertime_off;
int Screen::scr_center_lines;
int Screen::scr_erase_center;

/*

===============================================================================



BAR GRAPHS



===============================================================================

*/
/*

==============

CL_AddNetgraph



A new packet was just parsed

==============

*/
void Client::CL_AddNetgraph( ) {
#if 0
	int i;
	int in;
	timeType ping;
	// if using the debuggraph for something else, don't
	// add the net lines
	if( CVarSystem::GetCVarBool( "scr_debuggraph" ) || CVarSystem::GetCVarBool( "scr_timegraph" ) )
		return;
	//for( i = 0; i < cls.d_clientChannel.; i++ ) Screen::SCR_DebugGraph( 30, 0x40 );
	//for( i = 0; i<Client::cl.surpressCount; i++ ) Screen::SCR_DebugGraph( 30, 0xDF );
	// see what the latency was on this packet
	//in = Client::cls.netchan.incoming_acknowledged &( CMD_BACKUP-1 );
	ping = Client::cls.realtime - Client::cl.cmd_time[ in ];
	ping /= 30;
	if( ping > 30 ) ping = 30;
	Screen::SCR_DebugGraph( ping, 0xD0 );
#endif
}


typedef struct {

	float value;
	int color;
} graphsamp_t;

static int current;
static graphsamp_t values[ 1024 ];

/*

==============

SCR_DebugGraph

==============

*/
void Screen::SCR_DebugGraph( float value, int color ) {

	values[ current&1023 ].value = value;
	values[ current&1023 ].color = color;
	current++;
}

/*

==============

SCR_DrawDebugGraph

==============

*/
void Screen::SCR_DrawDebugGraph( ) {

#if 0

	int a, x, y, w, i, h;
	float v;
	int color;

	//
	// draw the graph
	//
	w = scr_vrect.width;

	x = scr_vrect.x;
	y = scr_vrect.y+scr_vrect.height;
	Renderer::DrawFill( x, y-scr_graphheight.GetInt( ), w, scr_graphheight.GetInt( ), 8 );

	for( a = 0; a < w; a++ ) {

		i =( current-1-a+1024 ) & 1023;
		v = values[ i ].value;
		color = values[ i ].color;
		v = v * scr_graphscale.GetFloat( ) + scr_graphshift.GetFloat( );

		if( v < 0.0f ) v += scr_graphheight.GetFloat( ) * ( 1.0f + -v / scr_graphheight.GetFloat( ) );
		h = ( int )v % scr_graphheight.GetInt( );
		Renderer::DrawFill( x + w - 1 - a, y - h, 1, h, color );
	}

#endif
}

/*

===============================================================================



CENTER PRINTING



===============================================================================

*/

/*

==============

SCR_CenterPrint



Called for important messages that should stay in the center of the screen

for a few moments

==============

*/
void Screen::SCR_CenterPrint( const Str & str ) {

	scr_centerstring = str;
	scr_centertime_off = ( timeTypeReal )scr_centertime.GetFloat( );
	scr_centertime_start = Client::cl.time;

	// count the number of lines for centering
	scr_center_lines = 1;
	for( int i = 0; str[ i ]; i++ ) if( str[ i ] == '\n' ) scr_center_lines++;

	// echo it to the console
	Common::Com_Printf( "\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n" );

	Common::Com_Printf( "%s", str.c_str( ) );

	Common::Com_Printf( "\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n" );
	//Console::Con_ClearNotify( );
}


void Screen::SCR_DrawCenterString( ) {

	const char * start;
	int l;
	int x, y;
	int remaining;

	// the finale prints the characters one at a time
	remaining = 9999;

	scr_erase_center = 0;
	start = scr_centerstring;

	if( scr_center_lines <= 4 )
		y = ( int )( Video::viddef.height* 0.35f );
	else
		y = 48;

	do
	{
		// scan the width of the line
		for( l = 0; l<40; l++ )
			if( start[ l ] == '\n' || !start[ l ] )
				break;
		x =( Video::viddef.width - l* 8 )/2;
		SCR_AddDirtyPoint( x, y );
#if 0
		for( j = 0; j<l; j++, x+= 8 )
		{
			Renderer::DrawChar( x, y, start[ j ] );
			if( !remaining-- )
				return;
		}
#endif
		SCR_AddDirtyPoint( x, y+8 );

		y += 8;

		while( *start && * start != '\n' )
			start++;

		if( !* start )
			break;
		start++; // skip the \n
	} while( 1 );
}

void Screen::SCR_CheckDrawCenterString( ) {

	scr_centertime_off -= Client::cls.frametime;

	if( scr_centertime_off <= 0 ) return;

	SCR_DrawCenterString( );
}

//=============================================================================

/*

=================

SCR_CalcVrect



Sets scr_vrect, the coordinates of the rendered window

=================

*/
void Screen::SCR_CalcVrect( ) {

	int size;

	// bound viewsize
	if( scr_viewsize.GetInt( ) < 40 ) scr_viewsize.SetInt( 40 );
	if( scr_viewsize.GetInt( ) > 100 ) scr_viewsize.SetInt( 100 );

	size = scr_viewsize.GetInt( );

	scr_vrect.width = Video::viddef.width* size/100;
	scr_vrect.width &= ~7;

	scr_vrect.height = Video::viddef.height* size/100;
	scr_vrect.height &= ~1;

	scr_vrect.x =( Video::viddef.width - scr_vrect.width )/2;
	scr_vrect.y =( Video::viddef.height - scr_vrect.height )/2;
}


/*

=================

SCR_SizeUp_f



Keybinding command

=================

*/
void Screen::SCR_SizeUp_f( ) {

	scr_viewsize.SetInt( scr_viewsize.GetInt( ) + 10 );
}


/*

=================

SCR_SizeDown_f



Keybinding command

=================

*/
void Screen::SCR_SizeDown_f( ) {

	scr_viewsize.SetInt( scr_viewsize.GetInt( ) - 10 );
}

/*

=================

SCR_Sky_f



Set a specific sky and rotation speed

=================

*/
void Screen::SCR_Sky_f( ) {

	float rotate;
	Vec3 axis;

	if( Command::Cmd_Argc( ) < 2 )
	{
		Common::Com_Printf( "Usage: sky <basename> <rotate> <axis x y z>\n" );
		return;
	}
	if( Command::Cmd_Argc( ) > 2 )
		rotate = ( float )atof( Command::Cmd_Argv( 2 ) );
	else
		rotate = 0;
	if( Command::Cmd_Argc( ) == 6 )
	{
		axis[ 0 ] = ( float )atof( Command::Cmd_Argv( 3 ) );
		axis[ 1 ] = ( float )atof( Command::Cmd_Argv( 4 ) );
		axis[ 2 ] = ( float )atof( Command::Cmd_Argv( 5 ) );
	}
	else
	{
		axis[ 0 ] = 0;
		axis[ 1 ] = 0;
		axis[ 2 ] = 1;
	}

	Renderer::SetSky( Str( Command::Cmd_Argv( 1 ) ), rotate, axis );
}

//============================================================================

/*

==================

SCR_Init

==================

*/
void Screen::SCR_Init( ) {

	//
	// register our commands
	//
	Command::Cmd_AddCommand( "timerefresh", SCR_TimeRefresh_f );
	Command::Cmd_AddCommand( "loading", SCR_Loading_f );
	Command::Cmd_AddCommand( "sizeup", SCR_SizeUp_f );
	Command::Cmd_AddCommand( "sizedown", SCR_SizeDown_f );
	Command::Cmd_AddCommand( "sky", SCR_Sky_f );

	scr_initialized = true;
}


/*

==============

SCR_DrawNet

==============

*/
void Screen::SCR_DrawNet( ) {

#if 0

	if( Client::cls.netchan.outgoing_sequence - Client::cls.netchan.incoming_acknowledged
		< CMD_BACKUP-1 )
		return;

	Renderer::DrawPic( scr_vrect.x+64, scr_vrect.y, Str( "pics/net.pcx" ) );

#endif
}

/*

==============

SCR_DrawPause

==============

*/
void Screen::SCR_DrawPause( ) {

#if 0

	int w, h;

	if( !scr_showpause.GetBool( ) ) // turn off for screenshots
		return;

	if( !Common::paused.GetBool( ) ) return;

	Renderer::DrawGetPicSize( &w, &h, Str( "pics/pause.pcx" ) );
	Renderer::DrawPic( ( Video::viddef.width-w )/2, Video::viddef.height/2 + 8, Str( "pics/pause.pcx" ) );

#endif
}

/*

==============

SCR_DrawLoading

==============

*/
void Screen::SCR_DrawLoading( ) {

#if 0

	int w, h;

	if( !scr_draw_loading )
		return;

	scr_draw_loading = false;
	Renderer::DrawGetPicSize( &w, &h, Str( "pics/loading.pcx" ) );
	Renderer::DrawPic( ( Video::viddef.width-w )/2, ( Video::viddef.height-h )/2, Str( "pics/loading.pcx" ) );

#endif
}

//=============================================================================

/*

==================

SCR_RunConsole



Scroll it up or down

==================

*/
void Screen::SCR_RunConsole( ) {

	// decide on the height of the console
	if( Client::cls.key_dest == key_console )
		scr_conlines = 0.5f; // half screen
	else
		scr_conlines = 0; // none visible

	if( scr_conlines < scr_con_current ) {

		scr_con_current -= scr_conspeed.GetFloat( ) * Client::cls.frametime;

		if( scr_conlines > scr_con_current ) scr_con_current = scr_conlines;

	} else if( scr_conlines > scr_con_current ) {

		scr_con_current += scr_conspeed.GetFloat( ) * Client::cls.frametime;

		if( scr_conlines < scr_con_current ) scr_con_current = scr_conlines;
	}

}

/*

==================

SCR_DrawConsole

==================

*/
void Screen::SCR_DrawConsole( ) {
#if 0
	Console::Con_CheckResize( );

	if( Client::cls.state == ca_disconnected || Client::cls.state == ca_connecting )
	{ // forced full screen console
		Console::Con_DrawConsole( 1.0f );
		return;
	}

	if( Client::cls.state != ca_active || !Client::cl.refresh_prepped )
	{ // connected, but can't render
		Console::Con_DrawConsole( 0.5f );

		Renderer::DrawFill( 0, Video::viddef.height/2, Video::viddef.width, Video::viddef.height/2, 0 );

		return;
	}

	if( scr_con_current )
	{
		Console::Con_DrawConsole( scr_con_current );
	}
	else
	{
		if( Client::cls.key_dest == key_game || Client::cls.key_dest == key_message )
			Console::Con_DrawNotify( ); // only draw notify in game
	}
#endif
}

//=============================================================================

/*

================

SCR_BeginLoadingPlaque

================

*/
void Screen::SCR_BeginLoadingPlaque( ) {

	//Sound::S_StopAllSounds( );
	SoundSystem::StopAllSounds( );
	Client::cl.sound_prepped = false; // don't play ambients
	if( Client::cls.disable_screen ) return;
	if( Common::developer.GetBool( ) ) return;
	if( Client::cls.state == ca_disconnected ) return; // if at console, don't bring up the plaque
	if( Client::cls.key_dest == key_console ) return;
#if 0
	if( Client::cl.cinematictime > 0 ) scr_draw_loading = 2; // clear to balack first
#endif
	else scr_draw_loading = 1;
	SCR_UpdateScreen( );
	Client::cls.disable_screen = System::Sys_Milliseconds( );
	Client::cls.disable_servercount = Client::cl.servercount;
}

/*

================

SCR_EndLoadingPlaque

================

*/
void Screen::SCR_EndLoadingPlaque( ) {

	Client::cls.disable_screen = 0;
	//Console::Con_ClearNotify( );
}

/*

================

SCR_Loading_f

================

*/
void Screen::SCR_Loading_f( ) {

	SCR_BeginLoadingPlaque( );
}

/*
================
SCR_TimeRefresh_f
================
*/

void Screen::SCR_TimeRefresh_f( ) {

	if( Client::cls.state != ca_active )
		return;
	timeType start = System::Sys_Milliseconds( );
	if( Command::Cmd_Argc( ) == 2 ) { // run without page flipping
		Renderer::BeginFrame( );
		for( int  i = 0; i < 128; i++ ) {
			Client::cl.refdef.viewangles[ 1 ] = i / 128.0f * 360.0f;
			Renderer::RenderFrame( Client::cl.refdef );
		}
		Renderer::EndFrame( );
	} else {
		for( int i = 0; i<128; i++ ) {
			Client::cl.refdef.viewangles[ 1 ] = i / 128.0f * 360.0f;
			Renderer::BeginFrame( );
			Renderer::RenderFrame( Client::cl.refdef );
			Renderer::EndFrame( );
		}
	}
	timeType stop = System::Sys_Milliseconds( );
	timeType time = ( stop - start ) / 1000;
	Common::Com_Printf( "%f seconds( %f fps )\n", time, 128.0/time );
}

/*
=================
SCR_AddDirtyPoint
=================
*/
void Screen::SCR_AddDirtyPoint( int x, int y ) {

	if( x < scr_dirty.x1 ) scr_dirty.x1 = x;
	if( x > scr_dirty.x2 ) scr_dirty.x2 = x;
	if( y < scr_dirty.y1 ) scr_dirty.y1 = y;
	if( y > scr_dirty.y2 ) scr_dirty.y2 = y;
}

void Screen::SCR_DirtyScreen( ) {

	SCR_AddDirtyPoint( 0, 0 );
	SCR_AddDirtyPoint( Video::viddef.width-1, Video::viddef.height-1 );
}

/*

==============

SCR_TileClear



Clear any parts of the tiled background that were drawn on last frame

==============

*/
void Screen::SCR_TileClear( ) {

	int i;
	int top, bottom, left, right;
	dirty_t clear;

	if( scr_con_current == 1.0f ) return; // full screen console
	if( scr_viewsize.GetInt( ) == 100 ) return; // full screen rendering
#if 0
	if( Client::cl.cinematictime > 0 ) return; // full screen cinematic
#endif

	// erase rect will be the union of the past three frames
	// so tripple buffering works properly
	clear = scr_dirty;
	for( i = 0; i<2; i++ )
	{
		if( scr_old_dirty[ i ].x1 < clear.x1 )
			clear.x1 = scr_old_dirty[ i ].x1;
		if( scr_old_dirty[ i ].x2 > clear.x2 )
			clear.x2 = scr_old_dirty[ i ].x2;
		if( scr_old_dirty[ i ].y1 < clear.y1 )
			clear.y1 = scr_old_dirty[ i ].y1;
		if( scr_old_dirty[ i ].y2 > clear.y2 )
			clear.y2 = scr_old_dirty[ i ].y2;
	}

	scr_old_dirty[ 1 ] = scr_old_dirty[ 0 ];
	scr_old_dirty[ 0 ] = scr_dirty;

	scr_dirty.x1 = 9999;
	scr_dirty.x2 = -9999;
	scr_dirty.y1 = 9999;
	scr_dirty.y2 = -9999;

	// don't bother with anything convered by the console )
	top = ( int )( scr_con_current* Video::viddef.height );
	if( top >= clear.y1 )
		clear.y1 = top;

	if( clear.y2 <= clear.y1 )
		return; // nothing disturbed

	top = scr_vrect.y;
	bottom = top + scr_vrect.height-1;
	left = scr_vrect.x;
	right = left + scr_vrect.width-1;

#if 0

	if( clear.y1 < top )
	{ // clear above view screen
		i = clear.y2 < top-1 ? clear.y2 : top-1;
		Renderer::DrawTileClear( clear.x1 , clear.y1, clear.x2 - clear.x1 + 1, i - clear.y1+1, Str( "pics/backtile.pcx" ) );
		clear.y1 = top;
	}
	if( clear.y2 > bottom )
	{ // clear below view screen
		i = clear.y1 > bottom+1 ? clear.y1 : bottom+1;
		Renderer::DrawTileClear( clear.x1, i, clear.x2-clear.x1+1, clear.y2-i+1, Str( "pics/backtile.pcx" ) );
		clear.y2 = bottom;
	}
	if( clear.x1 < left )
	{ // clear left of view screen
		i = clear.x2 < left-1 ? clear.x2 : left-1;
		Renderer::DrawTileClear( clear.x1, clear.y1, i-clear.x1+1, clear.y2 - clear.y1 + 1, Str( "pics/backtile.pcx" ) );
		clear.x1 = left;
	}
	if( clear.x2 > right )
	{ // clear left of view screen
		i = clear.x1 > right+1 ? clear.x1 : right+1;
		Renderer::DrawTileClear( i, clear.y1, clear.x2-i+1, clear.y2 - clear.y1 + 1, Str( "pics/backtile.pcx" ) );
		clear.x2 = right;
	}

#endif
}


//===============================================================

/*

================

SizeHUDString



Allow embedded \n in the string

================

*/
void SizeHUDString( char * string, int * w, int * h ) {

	int lines, width, current;

	lines = 1;
	width = 0;

	current = 0;
	while( *string )
	{
		if( *string == '\n' )
		{
			lines++;
			current = 0;
		}
		else
		{
			current++;
			if( current > width )
				width = current;
		}
		string++;
	}

	* w = width * 8;
	* h = lines * 8;
}

void DrawHUDString( char * string, int x, int y, int centerwidth, int xorvalue ) {

#if 0

	int margin;
	char line[ 1024 ];
	int width;
	int i;

	margin = x;

	while( *string )
	{
		// scan out one line of text from the string
		width = 0;
		while( *string && * string != '\n' )
			line[ width++ ] = * string++;
		line[ width ] = 0;

		if( centerwidth )
			x = margin +( centerwidth - width* 8 )/2;
		else
			x = margin;
		for( i = 0; i<width; i++ )
		{
			Renderer::DrawChar( x, y, line[ i ]^xorvalue );
			x += 8;
		}
		if( *string )
		{
			string++; // skip the \n
			x = margin;
			y += 8;
		}
	}

#endif
}


/*

==============

SCR_DrawField

==============

*/
void Screen::SCR_DrawField( int x, int y, int color, int width, int value ) {

#if 0

	Str num;
	const char * ptr;
	int l;
	int frame;

	if( width < 1 )
		return;

	// draw number string
	if( width > 5 )
		width = 5;

	SCR_AddDirtyPoint( x, y );
	SCR_AddDirtyPoint( x+width* 16 +2, y+23 );

	sprintf( num, "%i", value );
	l = strlen( num );
	if( l > width )
		l = width;
	x += 2 + 16* ( width - l );

	ptr = num;
	while( *ptr && l )
	{
		if( *ptr == '-' )
			frame = 10;
		else
			frame = * ptr -'0';

		Renderer::DrawPic( x, y, color ? sb_nums2[ frame ] : sb_nums1[ frame ] );
		x += 16;
		ptr++;
		l--;
	}

#endif
}


/*

===============

SCR_TouchPics



Allows rendering code to cache all needed sbar graphics

===============

*/
void Screen::SCR_TouchPics( ) {

#if 0

	CVar * crosshair = CVarSystem::Find( "crosshair" );

	if( crosshair->GetBool( ) ) {

		if( crosshair->GetInt( ) > 3 || crosshair->GetInt( ) < 0 ) crosshair->SetInt( 3 );

		sprintf( crosshair_pic, "pics/ch%i.pcx", crosshair->GetInt( ) );
		Renderer::DrawGetPicSize( &crosshair_width, &crosshair_height, crosshair_pic );
		if( !crosshair_width ) crosshair_pic.Clear( );
	}

#endif
}

/*

================

SCR_ExecuteLayoutString 



================

*/
void Screen::SCR_ExecuteLayoutString( const Str & str ) {

	int x, y;
	int value;
	char * token;
	int width;
	int index;
	clientinfo_t * ci;

	if( Client::cls.state != ca_active || !Client::cl.refresh_prepped ) return;

	if( !str.Length( ) ) return;
	char * s =( char * )Mem_Alloc( str.Length( ) + 1 );
	sprintf( s, str.c_str( ) );
	char * origs = s;

	x = 0;
	y = 0;
	width = 3;

	while( s )
	{
		token = COM_Parse( &s );
		if( !strcmp( token, "xl" ) )
		{
			token = COM_Parse( &s );
			x = atoi( token );
			continue;
		}
		if( !strcmp( token, "xr" ) )
		{
			token = COM_Parse( &s );
			x = Video::viddef.width + atoi( token );
			continue;
		}
		if( !strcmp( token, "xv" ) )
		{
			token = COM_Parse( &s );
			x = Video::viddef.width/2 - 160 + atoi( token );
			continue;
		}

		if( !strcmp( token, "yt" ) )
		{
			token = COM_Parse( &s );
			y = atoi( token );
			continue;
		}
		if( !strcmp( token, "yb" ) )
		{
			token = COM_Parse( &s );
			y = Video::viddef.height + atoi( token );
			continue;
		}
		if( !strcmp( token, "yv" ) )
		{
			token = COM_Parse( &s );
			y = Video::viddef.height/2 - 120 + atoi( token );
			continue;
		}

		if( !strcmp( token, "pic" ) )
		{ // draw a pic from a stat number
			token = COM_Parse( &s );
			value = Client::cl.frame.playerstate.stats[atoi( token )];
			if( value >= MAX_IMAGES )
				Common::Com_Error( ERR_DROP, "Pic >= MAX_IMAGES" );
			if( Client::cl.configstrings[ CS_IMAGES+value ].Length( ) )
			{
				SCR_AddDirtyPoint( x, y );
				SCR_AddDirtyPoint( x+23, y+23 );
#if 0
				Renderer::DrawPic( x, y, Str( Client::cl.configstrings[ CS_IMAGES + value ] ) );
#endif
			}
			continue;
		}

		if( !strcmp( token, "client" ) )
		{ // draw a deathmatch client block
			int score, ping, time;

			token = COM_Parse( &s );
			x = Video::viddef.width/2 - 160 + atoi( token );
			token = COM_Parse( &s );
			y = Video::viddef.height/2 - 120 + atoi( token );
			SCR_AddDirtyPoint( x, y );
			SCR_AddDirtyPoint( x+159, y+31 );

			token = COM_Parse( &s );
			value = atoi( token );
			if( value >= MAX_CLIENTS || value < 0 )
				Common::Com_Error( ERR_DROP, "client >= MAX_CLIENTS" );
			ci = &Client::cl.clientinfo[ value ];

			token = COM_Parse( &s );
			score = atoi( token );

			token = COM_Parse( &s );
			ping = atoi( token );

			token = COM_Parse( &s );
			time = atoi( token );

			/*Console::Con_DrawAltString( x+32, y, ci->name );
			Console::Con_DrawString( x+32, y+8, Str( "Score: " ) );
			Console::Con_DrawAltString( x+32+7* 8, y+8, Str( va( "%i", score ) ) );
			Console::Con_DrawString( x+32, y+16, Str( va( "Ping:  %i", ping ) ) );
			Console::Con_DrawString( x+32, y+24, Str( va( "Time:  %i", time ) ) );*/

			if( !ci->icon )
				ci = &Client::cl.baseclientinfo;
#if 0
			Renderer::DrawPic( x, y, Str( ci->iconname ) );
#endif
			continue;
		}

		if( !strcmp( token, "ctf" ) )
		{ // draw a ctf client block
			int score, ping;
			char block[ 80 ];

			token = COM_Parse( &s );
			x = Video::viddef.width/2 - 160 + atoi( token );
			token = COM_Parse( &s );
			y = Video::viddef.height/2 - 120 + atoi( token );
			SCR_AddDirtyPoint( x, y );
			SCR_AddDirtyPoint( x+159, y+31 );

			token = COM_Parse( &s );
			value = atoi( token );
			if( value >= MAX_CLIENTS || value < 0 )
				Common::Com_Error( ERR_DROP, "client >= MAX_CLIENTS" );
			ci = &Client::cl.clientinfo[ value ];

			token = COM_Parse( &s );
			score = atoi( token );

			token = COM_Parse( &s );
			ping = atoi( token );
			if( ping > 999 )
				ping = 999;

			sprintf( block, "%3d %3d %-12.12s", score, ping, ci->name.c_str( ) );

			/*if( value == Client::cl.playernum )
				Console::Con_DrawAltString( x, y, Str( block ) );
			else
				Console::Con_DrawString( x, y, Str( block ) );*/
			continue;
		}

		if( !strcmp( token, "picn" ) )
		{ // draw a pic from a name
			token = COM_Parse( &s );
			SCR_AddDirtyPoint( x, y );
			SCR_AddDirtyPoint( x+23, y+23 );

#if 0
			Renderer::DrawPic( x, y, Str( token ) );
#endif

			continue;
		}

		if( !strcmp( token, "num" ) )
		{ // draw a number
			token = COM_Parse( &s );
			width = atoi( token );
			token = COM_Parse( &s );
			value = Client::cl.frame.playerstate.stats[atoi( token )];
			SCR_DrawField( x, y, 0, width, value );
			continue;
		}

		if( !strcmp( token, "hnum" ) )
		{ // health number
			int color;

			width = 3;
			value = Client::cl.frame.playerstate.stats[ STAT_HEALTH ];
			if( value > 25 )
				color = 0; // green
			else if( value > 0 )
				color =( Client::cl.frame.serverframe>>2 ) & 1; // flash
			else
				color = 1;


#if 0
			if( Client::cl.frame.playerstate.stats[ STAT_FLASHES ] & 1 )
				Renderer::DrawPic( x, y, Str( "pics/field_3.pcx" ) );
#endif


			SCR_DrawField( x, y, color, width, value );
			continue;
		}

		if( !strcmp( token, "anum" ) )
		{ // ammo number
			int color;

			width = 3;
			value = Client::cl.frame.playerstate.stats[ STAT_AMMO ];
			if( value > 5 )
				color = 0; // green
			else if( value >= 0 )
				color =( Client::cl.frame.serverframe>>2 ) & 1; // flash
			else
				continue; // negative number = don't show

#if 0
			if( Client::cl.frame.playerstate.stats[ STAT_FLASHES ] & 4 )
				Renderer::DrawPic( x, y, Str( "pics/field_3.pcx" ) );
#endif

			SCR_DrawField( x, y, color, width, value );
			continue;
		}

		if( !strcmp( token, "rnum" ) )
		{ // armor number
			int color;

			width = 3;
			value = Client::cl.frame.playerstate.stats[ STAT_ARMOR ];
			if( value < 1 )
				continue;

			color = 0; // green

#if 0
			if( Client::cl.frame.playerstate.stats[ STAT_FLASHES ] & 2 )
				Renderer::DrawPic( x, y, Str( "pics/field_3.pcx" ) );
#endif

			SCR_DrawField( x, y, color, width, value );
			continue;
		}


		if( !strcmp( token, "stat_string" ) )
		{
			token = COM_Parse( &s );
			index = atoi( token );
			if( index < 0 || index >= MAX_CONFIGSTRINGS )
				Common::Com_Error( ERR_DROP, "Bad stat_string index" );
			index = Client::cl.frame.playerstate.stats[ index ];
			if( index < 0 || index >= MAX_CONFIGSTRINGS )
				Common::Com_Error( ERR_DROP, "Bad stat_string index" );
			//Console::Con_DrawString( x, y, Client::cl.configstrings[ index ] );
			continue;
		}

		if( !strcmp( token, "cstring" ) )
		{
			token = COM_Parse( &s );
			DrawHUDString( token, x, y, 320, 0 );
			continue;
		}

		if( !strcmp( token, "string" ) )
		{
			token = COM_Parse( &s );
			//Console::Con_DrawString( x, y, Str( token ) );
			continue;
		}

		if( !strcmp( token, "cstring2" ) )
		{
			token = COM_Parse( &s );
			DrawHUDString( token, x, y, 320, 0x80 );
			continue;
		}

		if( !strcmp( token, "string2" ) )
		{
			token = COM_Parse( &s );
			//Console::Con_DrawAltString( x, y, Str( token ) );
			continue;
		}

		if( !strcmp( token, "if" ) )
		{ // draw a number
			token = COM_Parse( &s );
			value = Client::cl.frame.playerstate.stats[atoi( token )];
			if( !value )
			{ // skip to endif
				while( s && strcmp( token, "endif" ) )
				{
					token = COM_Parse( &s );
				}
			}

			continue;
		}


	}

	Mem_Free( origs );
}


/*

================

SCR_DrawStats



The status bar is a small layout program that

is based on the stats array

================

*/
void Screen::SCR_DrawStats( ) {

	SCR_ExecuteLayoutString( Client::cl.configstrings[ CS_STATUSBAR ] );
}


/*

================

SCR_DrawLayout



================

*/
void Screen::SCR_DrawLayout( ) {

	if( !Client::cl.frame.playerstate.stats[ 13 ] ) return;
	SCR_ExecuteLayoutString( Client::cl.layout );
}

//=======================================================

/*

==================

SCR_UpdateScreen



This is called every frame, and can also be called explicitly to flush

text to the screen.

==================

*/
void Screen::SCR_UpdateScreen( ) {
	
	// if the screen is disabled( loading plaque is up, or vid mode changing )
	// do nothing at all
	if( Client::cls.disable_screen ) {
		if( System::Sys_Milliseconds( ) - Client::cls.disable_screen > 120000.0 ) {
			Client::cls.disable_screen = 0;
			Common::Com_Printf( "Loading plaque timed out.\n" );
		}
		return;
	}
	if( !scr_initialized )
		return; // not initialized yet

	Renderer::BeginFrame( );

	if( scr_draw_loading == 2 ) { //  loading plaque over black screen

	} else {
		// do 3D refresh drawing, and then update the screen
		SCR_CalcVrect( );
		// clear any dirty part of the background
		SCR_TileClear( );
		View::V_RenderView( );

		SCR_DrawStats( );
		if( Client::cl.frame.playerstate.stats[ 13 ] & 1 )
			SCR_DrawLayout( );
		if( Client::cl.frame.playerstate.stats[ 13 ] & 2 )
			Client::CL_DrawInventory( );

		SCR_DrawNet( );
		SCR_CheckDrawCenterString( );

		if( scr_timegraph.GetBool( ) )
			SCR_DebugGraph( Client::cls.frametime * 300.0f, 0 );

		if( scr_debuggraph.GetBool( ) || scr_timegraph.GetBool( ) || scr_netgraph.GetBool( ) )
			SCR_DrawDebugGraph( );

		SCR_DrawPause( );
		SCR_DrawConsole( );
		//Menu::M_Draw( );
		SCR_DrawLoading( );
	}

	Renderer::EndFrame( );
}
