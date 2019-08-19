#include "precompiled.h"
#pragma hdrstop

/*int		Keyboard::key_linepos;
int		Keyboard::shift_down = false;
int		Keyboard::anykeydown;

int		Keyboard::edit_line = 0;
int		Keyboard::history_line = 0;

int		Keyboard::key_waiting;
bool	Keyboard::consolekeys[ 256 ];
bool	Keyboard::menubound[ 256 ];
int		Keyboard::key_repeats[ 256 ];
bool	Keyboard::keydown[ 256 ];

bool	Keyboard::chat_team;*/

List< virtualCode_t >	KeyEvent::extFrameEvents;
List< virtualCode_t >	KeyEvent::extLastFrameEvents;
byte					KeyEvent::keyStateBuffer[ 256 ];
WindowsNS::HKL			KeyEvent::keyLayout;
List< virtualCode_t >	KeyEvent::csaFilter;

void KeyEvent::RefreshKeyStateBuffer( ) {

	WindowsNS::GetKeyboardState( keyStateBuffer );
}

virtualCode_t KeyEvent::LookupVirtualCode( dword win_wparam ) {

	for( int i = 0; keyTranslationTable[ i ].internalCode != vs_none; i++ ) {
		if( keyTranslationTable[ i ].externalCode == win_wparam )
			return keyTranslationTable[ i ].internalCode;
	}
	return vs_none;
}

bool KeyEvent::FilterOnly( virtualCode_t vc, const List< virtualCode_t > & filter_list ) {

	bool found = false;
	bool targetDown = false;
	for( int i = 0; i < filter_list.Num( ); i++ ) {
		if( IsDown( filter_list[ i ] ) ) {
			if( filter_list[ i ] == vc )
				targetDown = true;
			if( found )
				return false;
			found = true;
		}
	}
	return targetDown;
}

bool KeyEvent::IsDown( virtualCode_t vc ) {

	if( extFrameEvents.FindIndex( vc ) != -1 )
		return true;
	for( int i = 0; keyTranslationTable[ i ].internalCode != vs_none; i++ ) {
		if( keyTranslationTable[ i ].internalCode == vc ) {
			if( keyStateBuffer[ keyTranslationTable[ i ].externalCode ] & 0x80 )
				return true;
			return false;
		}
	}
	/// extended keys can only reaches here
	if( extLastFrameEvents.FindIndex( vc ) != -1 )
		return true;
	return false;
}

const Str & KeyEvent::LookupKeyName( virtualCode_t vc ) {

	for( int i = 0; keyTranslationTable[ i ].internalCode != vs_none; i++ ) {
		if( keyTranslationTable[ i ].internalCode == vc )
			return keyTranslationTable[ i ].name;
	}
	return null_string;
}

virtualCode_t KeyEvent::LookupKeyCode( const Str & name ) {

	for( int i = 0; keyTranslationTable[ i ].internalCode != vs_none; i++ ) {
		if( keyTranslationTable[ i ].name.IcmpFast( name ) )
			return keyTranslationTable[ i ].internalCode;
	}
	return vs_none;
}

void KeyEvent::Init( ) {

	char layoutName[ 256 ];
	keyLayout = WindowsNS::GetKeyboardLayout( 0 );
	WindowsNS::GetKeyboardLayoutNameA( layoutName );
	Common::Com_Printf( "Keyboard initialized. Layout: %s\n", layoutName );
	csaFilter.Append( key_ctrl );
	csaFilter.Append( key_shift );
	csaFilter.Append( key_alt );
	Frame( );
}

void KeyEvent::Shutdown( ) {

	WindowsNS::UnloadKeyboardLayout( keyLayout );
}

void KeyEvent::Frame( ) {

	RefreshKeyStateBuffer( );
	extLastFrameEvents = extFrameEvents;
	extFrameEvents.SetNum( 0, false );
}

KeyEvent::KeyEvent( virtualCode_t vc, bool down ) {

	this->virtualCode = vc;
	this->asciiCode = '\0';
	this->uniCode = 0;
	this->keyDown = down;
	this->keyRepeating = this->keyDown && ( extLastFrameEvents.FindIndex( this->virtualCode ) != -1 );
	if( this->keyDown )
		extFrameEvents.Append( this->virtualCode );
}

KeyEvent::KeyEvent( dword win_wparam, dword win_lparam, bool down ) {

	WindowsNS::WORD toAsciiChar = 0;
	int toAsciiRet = WindowsNS::ToAsciiEx( win_wparam, win_lparam, keyStateBuffer, &toAsciiChar, 0, keyLayout );
	int toUnicodeRet = WindowsNS::ToUnicodeEx( win_wparam, win_lparam, keyStateBuffer, ( WindowsNS::LPWSTR )&uniCode, 2, 0, keyLayout );
	this->virtualCode = LookupVirtualCode( win_wparam );
	this->asciiCode = toAsciiRet ? toAsciiChar : '\0';
	if( !toUnicodeRet )
		uniCode = 0;
	else if( toUnicodeRet == 1 )
		uniCode &= 0x0000FFFF;
	this->keyDown = down;
	this->keyRepeating = this->keyDown && ( keyStateBuffer[ win_wparam ] & 0x80 );
	if( win_lparam & BIT( 30 ) )
		keyStateBuffer[ win_wparam ] &= 0x7F;
	else
		keyStateBuffer[ win_wparam ] |= 0x80;
	if( this->keyDown )
		extFrameEvents.Append( this->virtualCode );
}

#if 0

const Str & Keyboard::Key_GetEditLine( ) {

	return key_lines[ edit_line ];
}

int Keyboard::Key_GetEditLineCursorPos( ) {

	return key_linepos;
}

bool Keyboard::Key_AnyKeyDown( ) {

	return anykeydown > 0 ? true : false;
}

bool Keyboard::Key_ChatToTeam( ) {

	return chat_team;
}

const Str & Keyboard::Key_GetChatLine( ) {

	return chat_buffer;
}

const Str & Keyboard::Key_GetKeyBindByCommand( const Str & command ) {

	for( int j = 0; j < vs_endlist; j++ ) {

		if( keybindings[ j ].IsEmpty( ) ) continue;

		if( keybindings[ j ] == command ) return keybindings[ j ];
	}

	return null_string;
}

bool Keyboard::Key_ClearBindByCommand( const Str & command ) {

	bool found = false;

	for( int j = 0; j < vs_endlist; j++ ) {

		if( keybindings[ j ].IsEmpty( ) ) continue;

		if( keybindings[ j ] == command ) {
			
			keybindings[ j ].Clear( );
			found = true;
		}
	}

	return found;
}

bool Keyboard::Key_FindKeyBindingsByCommand( const Str & command, int * output, int count ) {

	bool found = false;
	int i = 0;

	for( int j = 0; j < vs_endlist && i < count; j++ ) {

		if( keybindings[ j ].IsEmpty( ) ) continue;

		if( keybindings[ j ] == command ) {
			
			output[ i ] = j;
			i++;
			found = true;
		}
	}

	return found;
}

bool Keyboard::Key_KeyIsDown( int key ) {

	if( key < 0 || key > 255 ) return false;

	return keydown[ key ];
}

void Keyboard::Key_SetTeamChat( bool tc ) {

	chat_team = tc;
}

void Keyboard::CompleteCommand( ) {

	Str cmd, s;

	s = key_lines[ edit_line ].Right( key_lines[ edit_line ].Length( ) - ( ( key_lines[ edit_line ][ 0 ] == '/' ) ? 1 : 0 ) );

	cmd = Command::Cmd_CompleteCommand( s );

	if( cmd.IsEmpty( ) ) cmd = CVarSystem::CompleteVariable( s );

	if( cmd.Length( ) ) {

		key_lines[ edit_line ].Clear( );
		key_lines[ edit_line ].Append( '/' );
		key_lines[ edit_line ].Append( cmd );
		key_lines[ edit_line ].Append( ' ' );
		key_linepos = key_lines[ edit_line ].Length( );
	}
}

void Keyboard::Key_Console( const KeyEvent & key_event ) {

	if( key_event == key_v && KeyEvent::FilterOnly( key_ctrl ) ) {

		Str cbd = System::Sys_GetClipboardData( );

		if( cbd.Length( ) ) {

			Lexer lexer( cbd, cbd.Length( ), "console_input", LEXFL_NOERRORS | LEXFL_NOWARNINGS | LEXFL_NOSTRINGCONCAT | LEXFL_ALLOWPATHNAMES | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWIPADDRESSES );
			Token token;
			int lastTokenType = TT_PUNCTUATION;

			while( lexer.ReadToken( &token ) ) {

				if( lastTokenType != TT_PUNCTUATION && token.type != TT_PUNCTUATION ) {

					key_lines[ edit_line ].Append( ' ' );
				}

				lastTokenType = token.type;
				key_lines[ edit_line ].Append( token );
			}

			key_linepos = key_lines[ edit_line ].Length( );
		}

		return;
	}

	if( key_event == key_c && KeyEvent::FilterOnly( key_ctrl ) ) {

		System::Sys_SetClipboardData( con.text );
		return;
	}

	/// backslash text are commands, else chat
	if( key_event == key_return ) {

		Str cmd;

		if( key_lines[ edit_line ][ 0 ] != '/' ) {

			cmd = "say \"";
			cmd.Append( key_lines[ edit_line ] );
			cmd.Append( '"' );

		} else {
			
			cmd = key_lines[ edit_line ].Right( key_lines[ edit_line ].Length( ) - 1 );
		}

		CBuffer::Cbuf_AddText( cmd );
		CBuffer::Cbuf_AddText( Str( "\n" ) );

		Common::Com_Printf( "%s\n", cmd.c_str( ) );

		if( history_line == edit_line || ( history_line == ( edit_line - 1 ) && key_lines[ history_line ] != key_lines[ edit_line ] ) ) {

			key_lines.Append( Str( ) );
			edit_line++;

		} else {

			key_lines[ edit_line ].Clear( );
		}

		history_line = edit_line;
		key_linepos = 0;

		/// force an update, because the command
		if( Client::cls.state == ca_disconnected ) {
			
			Screen::SCR_UpdateScreen( );
		}

		/// may take some time
		return;
	}

	/// command completion
	if( key_event == key_tab ) {

		CompleteCommand( );
		return;
	}

	if( key_event == key_backspace ) {

		if( key_linepos < 1 ) {
			
			return;
		}

		Str newEditLine = key_lines[ edit_line ].Left( key_linepos - 1 );

		if( key_linepos < key_lines[ edit_line ].Length( ) ) {

			newEditLine.Append( key_lines[ edit_line ].Right( key_lines[ edit_line ].Length( ) - key_linepos ) );
		}

		key_lines[ edit_line ] = newEditLine;
		key_linepos--;
		return;
	}

	if( key_event == key_delete ) {

		if( key_linepos == key_lines[ edit_line ].Length( ) ) {
			
			return;
		}

		Str newEditLine = key_lines[ edit_line ].Left( key_linepos );

		if( key_linepos < key_lines[ edit_line ].Length( ) ) {

			newEditLine.Append( key_lines[ edit_line ].Right( key_lines[ edit_line ].Length( ) - ( key_linepos + 1 ) ) );
		}

		key_lines[ edit_line ] = newEditLine;
		return;
	}
	
	if( key_event == key_left ) {

		if( key_linepos > 0 ) {

			key_linepos--;
		}

		return;
	}

	if( key_event == key_right ) {

		key_linepos = Min( key_linepos + 1, key_lines[ edit_line ].Length( ) );
		return;
	}

	if( key_event == key_home ) {

		key_linepos = 0;
		return;
	}

	if( key_event == key_end ) {

		key_linepos = key_lines[ edit_line ].Length( );
		return;
	}

	if( key_event == key_up ) {

		history_line--;

		if( history_line < 0 ) {

			history_line = 0;
			return;
		}

		key_lines[ edit_line ] = key_lines[ history_line ];
		key_linepos = key_lines[ edit_line ].Length( );
		return;
	}

	if( key_event == key_down ) {

		history_line++;

		if( history_line >= edit_line ) {
			
			history_line = edit_line;
			key_lines[ edit_line ].Clear( );
			key_linepos = 0;
			return;
		}

		if( history_line != edit_line ) {

			key_lines[ edit_line ] = key_lines[ history_line ];
			key_linepos = key_lines[ edit_line ].Length( );
		}

		return;
	}

	if( key_event == key_pageup ) {

		if( KeyEvent::FilterOnly( key_ctrl ) ) {

			con.display = con.vislines;

		} else {

			con.display -= 16;

			if( con.display < con.vislines ) {

				con.display = con.vislines;
			}
		}

		return;
	}

	if( key_event == key_pagedown ) {

		if( KeyEvent::FilterOnly( key_ctrl ) ) {

			con.display = con.current;

		} else {

			con.display += 16;

			if( con.display > con.current ) {
				
				con.display = con.current;
			}
		}

		return;
	}

	if( key_event == mouse_wheelup ) {

		con.display--;

		if( con.display < con.vislines ) {

			con.display = con.vislines;
		}

		return;
	}

	if( key_event == mouse_wheeldown ) {

		con.display++;

		if( con.display > con.current ) {
			
			con.display = con.current;
		}

		return;
	}

	/// non printable
	if( !key_event.GetAscii( ) ) return;
		
	key_lines[ edit_line ].Insert( key_event.GetAscii( ), key_linepos );
	key_linepos++;
}

void Keyboard::Key_Message( int key ) {

	if( key == VK_RETURN ) {

		if( chat_team ) {
			
			CBuffer::Cbuf_AddText( Str( "say_team \"" ) );

		} else {
			
			CBuffer::Cbuf_AddText( Str( "say \"" ) );
		}

		CBuffer::Cbuf_AddText( chat_buffer );
		CBuffer::Cbuf_AddText( Str( " \"\n" ) );

		Client::cls.key_dest = key_game;
		chat_buffer.Clear( );
		return;
	}

	if( key == VK_ESCAPE ) {

		Client::cls.key_dest = key_game;
		chat_buffer.Clear( );
		return;
	}

	/// non printable
	if( key < 32 || key > 127 ) {
		
		return;
	}

	if( key == VK_BACK ) {

		chat_buffer = chat_buffer.Left( chat_buffer.Length( ) - 1 );
		return;
	}

	chat_buffer.Append( key );
}

int Keyboard::Key_StringToKeynum( const Str & str ) {

	const keyname_t	* kn;
	
	if( str.IsEmpty( ) ) return -1;
	if( !str[ 1 ] ) return str[ 0 ];

	for( kn = keynames; kn->name.Length( ); kn++ ) {

		if( str == kn->name ) 	return kn->keynum;
	}
	return -1;
}

Str Keyboard::Key_KeynumToString( int keynum ) {

	const keyname_t	* kn;	
	char	tinystr[ 2 ];
	
	if( keynum == -1 ) return "key_error";

	/// printable ascii
	if( keynum > 32 && keynum < 127 ) {

		tinystr[ 0 ] = keynum;
		tinystr[ 1 ] = 0;
		return tinystr;
	}
	
	for( kn = keynames; kn->name; kn++ ) {

		if( keynum == kn->keynum ) {

			return kn->name;
		}
	}

	return "key_error";
}

Str Keyboard::Key_KeynumToName( int keynum ) {

	char	str[ 32 ], str2[ 32 ];
	
	WindowsNS::LONG c = WindowsNS::MapVirtualKey( keynum, 0 ); // 0 = MAPVK_VK_TO_VSC
	WindowsNS::GetKeyNameText( c, str, sizeof( str ) );
	WindowsNS::GetKeyNameText( keynum, str2, sizeof( str2 ) );

	Str xd;
	sprintf( xd, "%c,%c,%s,%s,%s\n", keynum, c, str, str2, Key_KeynumToString( keynum ).c_str( ) );

	return xd;
}
#endif

#ifndef DEDICATED_ONLY

void Keyboard::Key_SetBinding( virtualCode_t keynum, const Str & binding ) {

	if( keynum == vs_none )
		return;
	g_bindings[ keynum ] = binding;
}

const Str &	Keyboard::Key_GetBinding( virtualCode_t keynum ) {

	return g_bindings[ keynum ];
}

void Keyboard::Key_Unbind_f( ) {

	if( Command::Cmd_Argc( ) != 2 ) {
		Common::Com_Printf( "unbind <key> : remove commands from a key\n" );
		return;
	}	
	virtualCode_t b = KeyEvent::LookupKeyCode( Command::Cmd_Argv( 1 ) );
	if( b == vs_none ) {
		Common::Com_Printf( "\"%s\" isn't a valid key\n", Command::Cmd_Argv( 1 ).c_str( ) );
		return;
	}
	Key_SetBinding( b, null_string );
}

void Keyboard::Key_Unbindall_f( ) {
	
	for( int i = 0; i < vs_endlist; i++ ) {		
		if( g_bindings[ i ].Length( ) )			
			Key_SetBinding( ( virtualCode_t )i, null_string );
	}
}

void Keyboard::Key_Bind_f( ) {
	
	int c = Command::Cmd_Argc( );
	if( c < 2 ) {
		Common::Com_Printf( "bind <key> [ command ] : attach a command to a key\n" );
		return;
	}
	virtualCode_t b = KeyEvent::LookupKeyCode( Command::Cmd_Argv( 1 ) );
	if( b == vs_none ) {
		Common::Com_Printf( "\"%s\" isn't a valid key\n", Command::Cmd_Argv( 1 ).c_str( ) );
		return;
	}
	if( c == 2 ) {
		if( g_bindings[ b ].Length( ) )
			Common::Com_Printf( "\"%s\" = \"%s\"\n", Command::Cmd_Argv( 1 ).c_str( ), g_bindings[ b ].c_str( ) );
		else
			Common::Com_Printf( "\"%s\" is not bound\n", Command::Cmd_Argv( 1 ).c_str( ) );
		return;
	}	
	/// copy the rest of the command line
	Str cmd;
	for( int i = 2; i < c; i++ ) {
		cmd.Append( Command::Cmd_Argv( i ) );
		if( i != ( c - 1 ) )			
			cmd.Append( " " );
	}
	Key_SetBinding( b, cmd );
}

void Keyboard::Key_WriteBindings( FileBase * f ) {

	Str bVal;
	for( int i = 0; i < vs_endlist; i++ ) {
		if( g_bindings[ i ].IsEmpty( ) )
			continue;
		bVal = "Input.";
		bVal += KeyEvent::LookupKeyName( ( virtualCode_t )i );
		bVal += ":Bind( '";
		bVal += g_bindings[ i ];
		bVal += "' )\n";			
		f->Write( bVal.c_str( ), bVal.Length( ) );
	}
}

void Keyboard::Key_Bindlist_f( ) {

	for( int i = 0; i < vs_endlist; i++ ) {
		if( g_bindings[ i ] && g_bindings[ i ][ 0 ] )
			Common::Com_Printf( "%s \"%s\"\n", KeyEvent::LookupKeyName( ( virtualCode_t )i ).c_str( ), g_bindings[ i ].c_str( ) );
	}
}

#define INP_ENTRY_CHECK()
#define INP_LINE_CHECK()
#define	INP_RETURN_CHECK(x)	return(x);

int Keyboard::lm_Bind( LuaState & state ) {

	INP_ENTRY_CHECK();
	if( !state.IsUserdata( 1 ) ) INP_RETURN_CHECK(0);
	virtualCode_t vCode = ( virtualCode_t )( ( int )state.GetPtr( 1 ) );
	if( vCode == vs_none ) INP_RETURN_CHECK(0);
	Key_SetBinding( vCode, state.GetString( 2 ) );
	INP_RETURN_CHECK(0);
}

int Keyboard::lm_Get( LuaState & state ) {

	INP_ENTRY_CHECK();
	if( !state.IsUserdata( 1 ) ) INP_RETURN_CHECK(0);
	virtualCode_t vCode = ( virtualCode_t )( ( int )state.GetPtr( 1 ) );
	if( vCode == vs_none ) { Lua::lua_pushnil( &state ); INP_RETURN_CHECK(1); }
	Lua::lua_pushstring( &state, Key_GetBinding( vCode ) );
	INP_RETURN_CHECK(1);
}

int Keyboard::lm___index( LuaState & state ) {

	INP_ENTRY_CHECK();
	virtualCode_t vCode;

	switch( state.GetType( 1 ) ) {
		case LUA_TTABLE:
			vCode = KeyEvent::LookupKeyCode( state.GetString( 2 ) );
			lua_pushlightuserdata( &state, ( void * )vCode ); INP_LINE_CHECK();
			Lua::lua_getmetatable( &state, 1 ); INP_LINE_CHECK();
			Lua::lua_setmetatable( &state, -2 ); INP_LINE_CHECK();
			INP_RETURN_CHECK(1);
		
		case LUA_TLIGHTUSERDATA:
			Lua::lua_getmetatable( &state, -2 ); INP_LINE_CHECK();
			Lua::lua_pushvalue( &state, -2 ); INP_LINE_CHECK();
			Lua::lua_rawget( &state, -2 ); INP_LINE_CHECK();
			Lua::lua_replace( &state, -2 ); INP_LINE_CHECK();
			INP_RETURN_CHECK(1);
	}

	INP_RETURN_CHECK(0);
}

int Keyboard::lg_registerInput( LuaState & state ) {

	INP_ENTRY_CHECK();
	//state.PushGlobal( );

	Lua::lua_newtable( &state );
	state.PushMetatable( "Input" );

	state.AddMethod( "__index", lm___index );
	state.AddMethod( "Bind", lm_Bind );
	state.AddMethod( "Get", lm_Get );

	//state.PushTable( "Input" );
	//Lua::lua_pushvalue( &state, 2 );
	Lua::lua_setmetatable( &state, -2 );
	//Lua::lua_rawset( &state, 1 );

	//state.Pop( 2 );
	//INP_RETURN_CHECK(0);
	INP_RETURN_CHECK(1);
}

void Keyboard::Key_Init( ) {

	/*int		i;
	
	key_lines.Append( Str( ) );
	
	/// init ascii characters in console mode
	for( i = 32; i < 128; i++ ) consolekeys[ i ] = true;
	consolekeys[ VK_RETURN ] = true;
	consolekeys[ VK_TAB ] = true;
	consolekeys[ VK_LEFT ] = true;
	consolekeys[ VK_RIGHT ] = true;
	consolekeys[ VK_UP ] = true;
	consolekeys[ VK_DOWN ] = true;
	consolekeys[ VK_BACK ] = true;
	consolekeys[ VK_HOME ] = true;
	consolekeys[ VK_END ] = true;
	consolekeys[ VK_PRIOR ] = true;
	consolekeys[ VK_NEXT ] = true;
	consolekeys[ VK_INSERT ] = true;
	consolekeys[ VK_DECIMAL ] = true;

	consolekeys[ VK_SCROLL ] = false;

	menubound[ key_esc ] = true;
	for( i = 0; i < 12; i++ ) menubound[ key_f1 + i ] = true;*/

	/// register our functions
	Command::Cmd_AddCommand( "bind", Key_Bind_f );
	Command::Cmd_AddCommand( "unbind", Key_Unbind_f );
	Command::Cmd_AddCommand( "unbindall", Key_Unbindall_f );
	Command::Cmd_AddCommand( "bindlist", Key_Bindlist_f );

	LuaScript::RegisterModule( "Input", lg_registerInput );
	//LuaScript::ExecuteInlineProt( "print(CVar.sv_maxvelocity:Desc())" );
}

void Keyboard::Con_ToggleConsole_f( ) {

	Screen::SCR_EndLoadingPlaque( );	// get rid of loading plaque
	if( Client::cl.attractloop ) {
		CBuffer::Cbuf_AddText( "killserver\n" );
		return;
	}
	if( Client::cls.state == ca_disconnected ) {	// start the demo loop again
		CBuffer::Cbuf_AddText( "d1\n" );
		return;
	}
	//Key_ClearTyping( );
	//Con_ClearNotify( );
	if( Client::cls.key_dest == key_console ) {
		Input::IN_Activate( true );
		//Menu::M_ForceMenuOff( );
		Common::paused.SetBool( false );
		Client::cls.key_dest = key_game;	
	} else {
		Input::IN_Activate( false );
		//Menu::M_ForceMenuOff( );
		Client::cls.key_dest = key_console;	
		if( Common::maxclients.GetInt( ) == 1 && Common::Com_ServerState( ) )
			CVarSystem::SetCVarBool( "paused", true );
	}
}

void Keyboard::Key_Event( const KeyEvent & key_event, unsigned time ) {

	Str	kb;
	Str	cmd;

	if( key_event == vs_none || key_event != key_backspace && key_event != key_pause && key_event != key_pageup && key_event != key_pagedown &&
	key_event != mouse_wheelup && key_event != mouse_wheeldown && key_event.IsRepeating( ) )
		return;
	GUISystem::Key_Event( key_event );
	/// console key is hardcoded, so the user can never unbind it
	if( key_event == mouse_middle ) {
		if( !key_event.IsDown( ) )			
			return;
		Con_ToggleConsole_f( );
		return;
	}
	/// menu key is hardcoded, so the user can never unbind it
	if( key_event == key_esc || Client::cl.attractloop && Client::cls.key_dest != key_menu ) {
		if( !key_event.IsDown( ) )			
			return;
		/// put away help computer / inventory
		if( Client::cl.frame.playerstate.stats[ STAT_LAYOUTS ] && Client::cls.key_dest == key_game ) {
			CBuffer::Cbuf_AddText( "cmd putaway\n" );
			return;
		}
		switch( Client::cls.key_dest ) {
			case key_message:
				//Key_Message( key_event.GetAscii( ) );
				break;

			case key_menu:
				//Menu::M_Keydown( key_event.GetAscii( ) );
				break;

			case key_game:
			case key_console:
				//Menu::M_Menu_Main_f( );
				break;

			default:
				Common::Com_Error( ERR_FATAL, "Bad Client::cls.key_dest" );
		}
		return;
	}
	/// key up events only generate commands if the game key binding is
	/// a button command( leading + sign ).  These will occur even in console mode, // to keep the character from continuing an action started before a console
	/// switch.  Button commands include the kenum as a parameter, so multiple downs can be matched with ups
	if( !key_event.IsDown( ) ) {
		kb = g_bindings[ key_event ];
		if( kb.Length( ) && kb[ 0 ] == '+' ) {
			sprintf( cmd, "-%s %i %i\n", kb.Right( kb.Length( ) - 1 ).c_str( ), key_event.GetVirtual( ), time );
			CBuffer::Cbuf_AddText( cmd );
		}
		return;
	}
	/// if not a consolekey, send to the interpreter no matter what mode is
	if( //( Client::cls.key_dest == key_menu && menubound[ key_event ] ) ||
//		( Client::cls.key_dest == key_console && !consolekeys[ key_event ] ) ||
		( Client::cls.key_dest == key_game && ( Client::cls.state == ca_active /*|| !consolekeys[ key_event ]*/ ) ) ) {

		kb = g_bindings[ key_event ];
		if( kb.Length( ) ) {
			/// button commands add keynum and time as a parm
			if( kb[ 0 ] == '+' ) {
				sprintf( cmd, "%s %i %i\n", kb.c_str( ), key_event.GetVirtual( ), time );
				CBuffer::Cbuf_AddText( cmd );
			} else {
				CBuffer::Cbuf_AddText( kb );
				CBuffer::Cbuf_AddText( "\n" );
			}
		}
		return;
	}

	/// other systems only care about key down events
	if( !key_event.IsDown( ) )		
		return;

	switch( Client::cls.key_dest ) {
		case key_message:
			//Key_Message( key_event.GetAscii( ) );
			break;

		case key_menu:
			//Menu::M_Keydown( key_event.GetAscii( ) );
			break;

		case key_game:
		case key_console:
			//Key_Console( key_event );
			break;

		default:
			Common::Com_Error( ERR_FATAL, "Bad Client::cls.key_dest" );
	}
}

void Keyboard::Key_ClearStates( ) {

	KeyEvent::Frame( );

	/*anykeydown = false;
	for( int i = 0; i < 256; i++ ) {
		if( keydown[ i ] || key_repeats[ i ] ) {			
			Key_Event( i, false, 0 );
		}
		keydown[ i ] = 0;
		key_repeats[ i ] = 0;
	}*/
}

/*int Keyboard::Key_GetKey( ) {

	key_waiting = -1;
	while( key_waiting == -1 )		
		System::Sys_SendKeyEvents( );
	return key_waiting;
}*/

#endif
