#include "precompiled.h"
#pragma hdrstop

/*

=====================

CL_Rcon_f



Send the rest of the command line over as

an unconnected command.

=====================

*/
void Console::Rcon_f( ) {

	char message[ 1024 ];
	int i;
	netadr_t to;

	if( !rcon_client_password.GetString( ).IsEmpty( ) )
	{
		Common::Com_Printf( "You must set 'rcon_password' before\n"
			"issuing an rcon command.\n" );
		return;
	}

	message[ 0 ] =( char )255;
	message[ 1 ] =( char )255;
	message[ 2 ] =( char )255;
	message[ 3 ] =( char )255;
	message[ 4 ] = 0;

	Network::NET_Config( true ); // allow remote

	strcat( message, "rcon " );

	strcat( message, rcon_client_password.GetString( ) );
	strcat( message, " " );

	for( i = 1; i<Command::Cmd_Argc( ); i++ )
	{
		strcat( message, Command::Cmd_Argv( i ) );
		strcat( message, " " );
	}

	if( Client::cls.state >= ca_connected )
		to = Client::cls.netchan.remote_address;
	else
	{
		if( rcon_address.GetString( ).IsEmpty( ) ) {

			Common::Com_Printf( "You must either be connected, or set the 'rcon_address' cvar to issue rcon commands\n" );

			return;
		}
		Network::NET_StringToAdr( rcon_address.GetString( ), &to );
		if( to.port == 0 ) to.port = BigShort( PORT_SERVER );
	}

	Network::NET_SendPacket( NS_CLIENT, strlen( message )+1, message, to );
}

void Console::Con_DrawString( int x, int y, const Str & str ) {

#if 0
	for( int i = 0; str[ i ]; i++ ) {

		Renderer::DrawChar( x, y, str[ i ] );
		x += 8;
	}
#endif

}

void Console::Con_DrawAltString( int x, int y, const Str & str ) {

#if 0

	for( int i = 0; str[ i ]; i++ ) {

		Renderer::DrawChar( x, y, str[ i ] ^ 0x80 );
		x += 8;
	}

#endif
}


void Keyboard::Key_ClearTyping( ) {

	key_lines[ edit_line ].Clear( );
	key_linepos = 0;
}

/*
================
Con_ToggleConsole_f
================
*/
void Console::Con_ToggleConsole_f( ) {

	Screen::SCR_EndLoadingPlaque( );	// get rid of loading plaque

	if( Client::cl.attractloop ) {

		CBuffer::Cbuf_AddText( Str( "killserver\n" ) );
		return;
	}

	if( Client::cls.state == ca_disconnected ) {	// start the demo loop again

		CBuffer::Cbuf_AddText( Str( "d1\n" ) );
		return;
	}

	Keyboard::Key_ClearTyping( );
	Con_ClearNotify( );

	if( Client::cls.key_dest == key_console ) {

		Input::IN_Activate( true );
		Menu::M_ForceMenuOff( );
		Common::paused.SetBool( false );

	} else {

		Input::IN_Activate( false );
		Menu::M_ForceMenuOff( );
		Client::cls.key_dest = key_console;	

		if( Common::maxclients.GetInt( ) == 1 && Common::Com_ServerState( ) ) CVarSystem::SetCVarBool( "paused", true );
	}
}

/*
================
Con_ToggleChat_f
================
*/
void Console::Con_ToggleChat_f( ) {

	Keyboard::Key_ClearTyping( );

	if( Client::cls.key_dest == key_console ) {

		if( Client::cls.state == ca_active ) {

			Menu::M_ForceMenuOff( );
			Client::cls.key_dest = key_game;
		}

	} else Client::cls.key_dest = key_console;
	
	Con_ClearNotify( );
}

/*
================
Con_Clear_f
================
*/
void Console::Con_Clear_f( ) {

	con.text.Clear( );
}

						
/*
================
Con_Dump_f

Save the console contents out to a file
================
*/
void Console::Con_Dump_f( ) {

	if( Command::Cmd_Argc( ) != 2 ) {

		Common::Com_Printf( "usage: condump <filename>\n" );
		return;
	}

	FileBase * f = FileSystem::OpenFileByMode( Command::Cmd_Argv( 1 ), FS_WRITE );

	if( !f ) return;

	f->Printf( "%s\n", con.text.c_str( ) );

	FileSystem::CloseFile( f );
}

						
/*
================
Con_ClearNotify
================
*/
void Console::Con_ClearNotify( ) {

	for( int i = 0; i < NUM_CON_TIMES; i++ ) con.times[ i ] = 0;
}

						
/*
================
Con_MessageMode_f
================
*/
void Console::Con_MessageMode_f( ) {

	Keyboard::Key_SetTeamChat( false );
	Client::cls.key_dest = key_message;
}

/*
================
Con_MessageMode2_f
================
*/
void Console::Con_MessageMode2_f( ) {

	Keyboard::Key_SetTeamChat( true );
	Client::cls.key_dest = key_message;
}

/*
================
Con_CheckResize

If the line width has changed, reformat the buffer.
================
*/
void Console::Con_CheckResize( ) {

	int		width, oldwidth, oldtotallines, numlines, numchars;

	width = ( Video::viddef.width >> 3 ) - 2;

	if( width == con.linewidth ) return;

	if( width < 1 )			// video hasn't been initialized yet
	{
		width = 96;
		con.linewidth = width;

	} else {

		oldwidth = con.linewidth;
		con.linewidth = width;
		oldtotallines = con.totallines;
		numlines = oldtotallines;

		numchars = oldwidth;
	
		if( con.linewidth < numchars )
			numchars = con.linewidth;

		Con_ClearNotify( );
	}
}


/*
================
Con_Init
================
*/
void Console::Con_Init( ) {

	con.text.ReAllocate( 256*1024, true );
	con.linewidth = -1;
	con.display = con.current = 0;

	con.lines.SetGranularity( 1024 );
	con.lines.Append( 0 );
	Con_CheckResize( );
	
	Common::Com_Printf( "Console initialized.\n" );

//
// register our commands
//
	Command::Cmd_AddCommand( Str( "toggleconsole" ), Con_ToggleConsole_f );
	Command::Cmd_AddCommand( Str( "togglechat" ), Con_ToggleChat_f );
	Command::Cmd_AddCommand( Str( "messagemode" ), Con_MessageMode_f );
	Command::Cmd_AddCommand( Str( "messagemode2" ), Con_MessageMode2_f );
	Command::Cmd_AddCommand( Str( "clear" ), Con_Clear_f );
	Command::Cmd_AddCommand( Str( "condump" ), Con_Dump_f );
	Command::Cmd_AddCommand( Str( "rcon" ), Rcon_f );
	con.initialized = true;
}

/*
================
Con_Print

Handles cursor positioning, line wrapping, etc
All console printing must go through this in order to be logged to disk
If no console is visible, the text will appear at the top of the game window
================
*/
void Console::Con_Print( const Str & txt ) {

	int oldLen = con.text.Length( );
	con.text.Append( txt );

	int newLineCount = 0;
	for( int j = 0; txt[ j ]; j++ ) if( txt[ j ] == '\n' ) {
		
		con.lines.Append( oldLen + ( j + 1 ) );
		newLineCount++;
	}

	if( con.display == con.current ) {

		con.display = con.current = con.lines.Num( );

	} else con.current += newLineCount;
}


/*
==============
Con_CenteredPrint
==============
*/
void Console::Con_CenteredPrint( const Str & text ) {

	Str	buffer;

	int l = text.Length( );
	l =( con.linewidth - l ) / 2;
	if( l < 0 ) l = 0;

	buffer.Fill( ' ', l );
	buffer += text;
	buffer += '\n';

	Con_Print( buffer );
}

/*
==============================================================================

DRAWING

==============================================================================
*/


/*
================
Con_DrawInput

The input line scrolls horizontally if typing goes beyond the right edge
================
*/
void Console::Con_DrawInput( ) {

	int		y;
	Str	text;

	if( Client::cls.key_dest == key_menu ) return;
	if( Client::cls.key_dest != key_console && Client::cls.state == ca_active ) return;		// don't draw anything( always draw if not active )

	text = Keyboard::Key_GetEditLine( );
	
	// add the cursor frame
	//text[ key_linepos ] = 10+( ( int )( Client::cls.realtime>>8 )&1 );
	
	// draw it
	y = con.vislines - 16;

#if 0
	for( i = 0; i < con.linewidth && text[ i ]; i++ )
		Renderer::DrawChar( ( i+1 ) << 3, con.vislines - 22, text[ i ] );

	Renderer::DrawChar( ( Keyboard::Key_GetEditLineCursorPos( ) ) << 3, con.vislines - 22, 11 );
#endif

	// remove cursor
	//key_lines[ edit_line ][ key_linepos ] = 0;
}


/*
================
Con_DrawNotify

Draws the last few lines of output transparently over the game top
================
*/
void Console::Con_DrawNotify( ) {

	int			v;
	const char	* text;
	int			i;
	timeType	time;
	int			skip;

	v = 0;
	for( i = con.current-NUM_CON_TIMES+1; i<= con.current; i++ )
	{
		if( i < 0 )
			continue;
		time = con.times[i % NUM_CON_TIMES];
		if( time == 0 )
			continue;
		time = Client::cls.realtime - time;
		if( time > con_notifytime.GetFloat( ) * 1000.0 ) continue;
		text = con.text.c_str( ) +( i % con.totallines )* con.linewidth;
		
#if 0
		for( x = 0; x < con.linewidth; x++ )
			Renderer::DrawChar( ( x+1 )<<3, v, text[ x ] );
#endif

		v += 8;
	}


	if( Client::cls.key_dest == key_message )
	{
		if( Keyboard::Key_ChatToTeam( ) )
		{
			Con_DrawString( 8, v, Str( "say_team:" ) );
			skip = 11;
		}
		else
		{
			Con_DrawString( 8, v, Str( "say:" ) );
			skip = 5;
		}

#if 0
		x = 0;
		Str chatBuffer = Keyboard::Key_GetChatLine( );
		while( chatBuffer[ x ] ) {

			Renderer::DrawChar( ( x + skip ) << 3, v, chatBuffer[ x ] );
			x++;
		}
		Renderer::DrawChar( ( x+skip )<<3, v, 10+( ( ( int )Client::cls.realtime>>8 )&1 ) );
		v += 8;
#endif
	}
	
	if( v )
	{
		Screen::SCR_AddDirtyPoint( 0, 0 );
		Screen::SCR_AddDirtyPoint( Video::viddef.width-1, v );
	}
}

/*
================
Con_DrawConsole

Draws the console with the solid background
================
*/
void Console::Con_DrawConsole( float frac ) {

#ifdef ENABLE_MATSYSTEM_UI
	Font * consoleFont = Font::Find( "consoleFont" );
	Font * consoleFont2 = Font::Find( "consoleFont2" );

	//consoleFont->SetScale( 0.8f );
	Bounds2D conBounds( 0, 0, ( float )Video::viddef.width, Video::viddef.height * frac );
	MaterialSystem::PushColorRect( conBounds, Color( 0.5f, 0.5f, 0.5f, 0.25f, false ) );
	int conColumns, conRows;
	consoleFont2->BoundsColumnsRowsNum( conBounds, conColumns, conRows );
	//conRows--; // console input's row
	con.vislines = conRows;
	//conBounds = consoleFont->MakeTextBoard( conColumns, conRows );
	//const char * conText = con.text.c_str( );
	//Bounds2D conCharSize = consoleFont2->MakeTextBoard( 1, 1 );
	//Vec2 yTrans = Vec2( 0.0f, conCharSize[ 1 ][ 1 ] );
	/*for( int i = ( con.display - 1 ); i >= 0; i-- ) {

		int lineRowUsage = consoleFont2->TextRowUsage( conBounds, con.text, con.lines[ i ] );
		if( lineRowUsage >= conRows || !i ) {

			consoleFont2->SetColor( color_orange );
			consoleFont2->PushTextBox( conBounds, con.text, con.lines[ lineRowUsage == conRows || !i ? i : i + 1 ] );
			break;
		}
	}*/

	consoleFont2->SetColor( color_orange );
	consoleFont2->PushTextBox( conBounds, con.text, con.lines[ con.display - 1 ] );

	conBounds.TranslateSelf( Vec2( 0.0f, conBounds[ 1 ][ 1 ] ) );

	//consoleFont->SetScale( 1.0f );
	consoleFont->SetColor( color_white );
	consoleFont->PushTextBox( conBounds, Keyboard::Key_GetEditLine( ) );
	Bounds2D conEditSize = consoleFont->MakeTextBoard( 1, 1 );
	if( fmod( Client::cls.realtime, 750.0f ) < 500.0f ) {

		float cursorPos = Keyboard::Key_GetEditLineCursorPos( );
		Bounds2D cursorBounds = conBounds.Translate( Vec2( conEditSize[ 1 ][ 0 ] * cursorPos, conEditSize[ 1 ][ 1 ] / 8.0f ) );
		consoleFont->PushTextBox( cursorBounds, "_" );
	}

	//consoleFont->SetScale( 0.8f );
	/*for( int i = ( con.display - 1 ); i >= 0; i-- ) {

		Str lineData = &conText[ con.lines[ i ] ];
		int lineRowUsage = consoleFont2->TextRowUsage( conBounds, lineData );
		conRows -= lineRowUsage;
		if( conRows < 0 ) break;

		conBounds.TranslateSelf( -yTrans );
		consoleFont2->SetColor( color_orange );
		consoleFont2->PushTextBox( conBounds, lineData, true );
	}*/
	//consoleFont->SetScale( 1.0f );
#endif
}

# if 0

	int				i, j, x, y, n;
	int				rows;
	int				row;
	int				lines;
	Str				version;

	lines = Video::viddef.height * frac;
	if( lines <= 0 )
		return;

	if( lines > Video::viddef.height )
		lines = Video::viddef.height;

// draw the background
	//Renderer::DrawStretchPic( 0, -( int )Video::viddef.height + lines, Video::viddef.width, Video::viddef.height, Str( "pics/conback.pcx" ) );
	//MaterialSystem::PushTextureStrech( Bounds2D( 0, 0, 320, 240 ), MaterialSystem::FindTexture( "pics/conback.pcx", "hud" ) );
	//MaterialSystem::PushTextureStrech( Bounds2D( 320, 240, 320+256, 240+192 ), MaterialSystem::FindTexture( "pics/help.pcx", "hud" ) );
	Screen::SCR_AddDirtyPoint( 0, 0 );
	Screen::SCR_AddDirtyPoint( Video::viddef.width-1, lines-1 );

	version = "ver "VERSION;
	//version = Str::stringDataAllocator.GetAllocInfoString( );
	//Con_DrawAltString( Video::viddef.width - 8 * version.Length( ), lines - 12, version );
	//for( x = 0; version[ x ]; x++ ) Renderer::DrawChar( Video::viddef.width - 44 + x * 8, lines-12, version[ x ] );

	Font * consoleFont = Font::Find( "consoleFont" );
	consoleFont->PushTextBox( Bounds2D( 0, 0, Video::viddef.width, Video::viddef.height - 40 ), con.text );
	consoleFont->PushText( Vec2( 0, Video::viddef.height - 20 ), Keyboard::Key_GetEditLine( ) );


// draw the text
	con.vislines = lines;
	rows =( lines-22 )>>3;		// rows of text to draw
	y = lines - 30;

// draw from the bottom up
	if( con.display != con.current )
	{
	// draw arrows to show the buffer is backscrolled

		for( x = 0; x<con.linewidth; x+= 4 )
			Renderer::DrawChar( ( x+1 )<<3, y, '^' );

	
		y -= 8;
		rows--;
	}

	n = 0;
	for( int j = 0; con.text[ j ]; j++ ) if( con.text[ j ] == '\n' ) n++;

	i = 0;
	row = con.display;
	for( int j = 0; con.text[ j ]; j++ ) {

		if( row <= rows ) break; // skip text pointer to con.display - con.vislines

		if( con.text[ j ] != '\n' ) continue;
		
		i = j + 1;
		row--;
	}

	y = 0;
	while( row-- ) {

		if( !con.text[ i ] ) break;


		for( int x = 0; con.text[ i ] != '\n'; x++, i++ ) {

			Renderer::DrawChar( ( x + 1 ) << 3, y, con.text[ i ] );
		}


		i++; // skip newline
		y += 8;
	}
	
	/*row = con.display;
	for( i = 0; i<rows; i++, y-= 8, row-- )
	{
		if( row < 0 )
			break;
		if( con.current - row >= con.totallines )
			break;		// past scrollback wrap point
			
		text = con.text.c_str( ) +( row % con.totallines )* con.linewidth;

		for( x = 0; x<con.linewidth; x++ )
			Renderer::DrawChar( ( x+1 )<<3, y, text[ x ] );
	}*/

//ZOID
	// draw the download bar
	// figure out width
	/*if( Client::cls.download.IsValid( ) ) {
		if( ( text = strrchr( Client::cls.downloadname, '/' ) ) != NULL )
			text++;
		else
			text = Client::cls.downloadname;

		x = con.linewidth -( ( con.linewidth * 7 ) / 40 );
		y = x - strlen( text ) - 8;
		i = con.linewidth/3;
		if( strlen( text ) > i ) {
			y = x - i - 11;
			strncpy( dlbar, text, i );
			dlbar[ i ] = 0;
			strcat( dlbar, "..." );
		} else
			strcpy( dlbar, text );
		strcat( dlbar, ": " );
		i = strlen( dlbar );
		dlbar[ i++ ] = '\x80';
		// where's the dot go?
		if( Client::cls.downloadpercent == 0 )
			n = 0;
		else
			n = y * Client::cls.downloadpercent / 100;
			
		for( j = 0; j < y; j++ )
			if( j == n )
				dlbar[ i++ ] = '\x83';
			else
				dlbar[ i++ ] = '\x81';
		dlbar[ i++ ] = '\x82';
		dlbar[ i ] = 0;

		sprintf( dlbar + strlen( dlbar ), " %02d%%", Client::cls.downloadpercent );

		// draw it
		y = con.vislines-12;
		for( i = 0; i < strlen( dlbar ); i++ )
			Renderer::DrawChar( ( i+1 )<<3, y, dlbar[ i ] );
	}*/
//ZOID

// draw the input prompt, user text, and cursor if desired
	//Con_DrawInput( );
#endif
