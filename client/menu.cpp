#include "precompiled.h"
#pragma hdrstop

//=============================================================================
/* Support Routines */

m_drawfunc_f	Menu::m_drawfunc;
m_keyfunc_f		Menu::m_keyfunc;

int		Menu::m_main_cursor;
bool	Menu::m_entersound;		// play after drawing a frame, so caching won't disrupt the sound

int		Menu::keys_cursor;
int		Menu::bind_grab;
int		Menu::nummaps;

int		Menu::m_game_cursor;

char	Menu::m_savestrings[ MAX_SAVEGAMES][32 ];
bool	Menu::m_savevalid[ MAX_SAVEGAMES ];

int		Menu::m_num_servers;

int		Menu::s_numplayermodels;

const char * Menu::credits[ ] = {

	"+QUAKE II BY ID SOFTWARE", 0
};

int Menu::rate_tbl[ ] = {

	2500, 3200, 5000, 10000, 25000, 0

};

void Menu::M_Banner( const Str & name ) {

#if 0

	int w, h;

	Renderer::DrawGetPicSize( &w, &h, name );
	Renderer::DrawPic( Video::viddef.width / 2 - w / 2, Video::viddef.height / 2 - 110, name );

#endif

}

void Menu::M_PushMenu( m_drawfunc_f drawfunc, m_keyfunc_f keyfunc ) {

	int		i;

	if( Common::maxclients.GetInt( ) == 1 && Common::Com_ServerState( ) ) Common::paused.SetBool( true );

	// if this menu is already present, drop back to that level
	// to avoid stacking menus by hotkeys
	for( i = 0; i<m_menudepth; i++ )
		if( m_layers[ i ].draw == drawfunc &&
			m_layers[ i ].key == keyfunc )
		{
			m_menudepth = i;
		}

	if( i == m_menudepth )
	{
		if( m_menudepth >= MAX_MENU_DEPTH )
			Common::Com_Error( ERR_FATAL, "M_PushMenu: MAX_MENU_DEPTH" );
		m_layers[ m_menudepth ].draw = m_drawfunc;
		m_layers[ m_menudepth ].key = m_keyfunc;
		m_menudepth++;
	}

	m_drawfunc = drawfunc;
	m_keyfunc = keyfunc;

	m_entersound = true;

	Client::cls.key_dest = key_menu;
}

void Menu::M_ForceMenuOff( ) {

	m_drawfunc = 0;
	m_keyfunc = 0;
	Client::cls.key_dest = key_game;
	m_menudepth = 0;
	Keyboard::Key_ClearStates( );
	Common::paused.SetBool( false );
}

void Menu::M_PopMenu( ) {

	Sound::S_StartLocalSound( menu_out_sound );
	if( m_menudepth < 1 )
		Common::Com_Error( ERR_FATAL, "M_PopMenu: depth < 1" );
	m_menudepth--;

	m_drawfunc = m_layers[ m_menudepth ].draw;
	m_keyfunc = m_layers[ m_menudepth ].key;

	if( !m_menudepth )
		M_ForceMenuOff( );
}


const Str Menu::Default_MenuKey( menuframework_s * m, int key ) {

	const char * sound = NULL;
	menucommon_s * item;

	if( m )
	{
		if( ( item =( menucommon_s * )Menu_ItemAtCursor( m ) ) != 0 )
		{
			if( item->type == MTYPE_FIELD )
			{
				if( Field_Key( ( menufield_s * ) item, key ) )
					return NULL;
			}
		}
	}

	switch( key )
	{
	case VK_ESCAPE:
		M_PopMenu( );
		return menu_out_sound;
	case VK_NUMPAD8:
	case VK_UP:
		if( m )
		{
			m->cursor--;
			Menu_AdjustCursor( m, -1 );
			sound = menu_move_sound;
		}
		break;
	case VK_TAB:
		if( m )
		{
			m->cursor++;
			Menu_AdjustCursor( m, 1 );
			sound = menu_move_sound;
		}
		break;
	case VK_NUMPAD2:
	case VK_DOWN:
		if( m )
		{
			m->cursor++;
			Menu_AdjustCursor( m, 1 );
			sound = menu_move_sound;
		}
		break;
	case VK_NUMPAD4:
	case VK_LEFT:
		if( m )
		{
			Menu_SlideItem( m, -1 );
			sound = menu_move_sound;
		}
		break;
	case VK_NUMPAD6:
	case VK_RIGHT:
		if( m )
		{
			Menu_SlideItem( m, 1 );
			sound = menu_move_sound;
		}
		break;

	case VK_LBUTTON:
	case VK_RBUTTON:
	case VK_MBUTTON:
	case K_JOY1:
	case K_JOY2:
	case K_JOY3:
	case K_JOY4:
	case K_AUX1:
	case K_AUX2:
	case K_AUX3:
	case K_AUX4:
	case K_AUX5:
	case K_AUX6:
	case K_AUX7:
	case K_AUX8:
	case K_AUX9:
	case K_AUX10:
	case K_AUX11:
	case K_AUX12:
	case K_AUX13:
	case K_AUX14:
	case K_AUX15:
	case K_AUX16:
	case K_AUX17:
	case K_AUX18:
	case K_AUX19:
	case K_AUX20:
	case K_AUX21:
	case K_AUX22:
	case K_AUX23:
	case K_AUX24:
	case K_AUX25:
	case K_AUX26:
	case K_AUX27:
	case K_AUX28:
	case K_AUX29:
	case K_AUX30:
	case K_AUX31:
	case K_AUX32:
		
	case VK_RETURN:
		if( m )
			Menu_SelectItem( m );
		sound = menu_move_sound;
		break;
	}

	return sound;
}

//=============================================================================

/*
================
M_DrawCharacter

Draws one solid graphics character
cx and cy are in 320* 240 coordinates, and will be centered on
higher res screens.
================
*/
void Menu::M_DrawCharacter( int cx, int cy, int num ) {

#if 0

	Renderer::DrawChar( cx +( ( Video::viddef.width - 320 )>>1 ), cy +( ( Video::viddef.height - 240 )>>1 ), num );

#endif
}

void Menu::M_Print( int cx, int cy, const Str & str ) {

	for( int i = 0; str[ i ]; i++, cx += 8 ) M_DrawCharacter( cx, cy, str[ i ] + 128 );
}

void Menu::M_PrintWhite( int cx, int cy, const Str & str ) {

	for( int i = 0; str[ i ]; i++, cx += 8 ) M_DrawCharacter( cx, cy, str[ i ] );
}

void Menu::M_DrawPic( int x, int y, const Str & pic ) {

#if 0

	Renderer::DrawPic( x +( ( Video::viddef.width - 320 ) >> 1 ), y +( ( Video::viddef.height - 240 ) >> 1 ), pic );

#endif

}


/*
=============
M_DrawCursor

Draws an animating cursor with the point at
x, y.  The pic will extend to the left of x, and both above and below y.
=============
*/
void Menu::M_DrawCursor( int x, int y, int f ) {

#if 0

	Str	cursorname;
	static bool			cached;

	if( !cached ) {
		
		for( int i = 0; i < NUM_CURSOR_FRAMES; i++ ) {

			sprintf( cursorname, "pics/m_cursor%d.pcx", i );
			MaterialSystem::FindTexture( cursorname, "hud" );
		}

		cached = true;
	}

	sprintf( cursorname, "pics/m_cursor%d.pcx", f );
	Renderer::DrawPic( x, y, cursorname );	

#endif

}

void Menu::M_DrawTextBox( int x, int y, int width, int lines ) {

	int		cx, cy;

	// draw left side
	cx = x;
	cy = y;
	M_DrawCharacter( cx, cy, 1 );
	for( int n = 0; n < lines; n++ ) {

		cy += 8;
		M_DrawCharacter( cx, cy, 4 );
	}
	M_DrawCharacter( cx, cy + 8, 7 );

	// draw middle
	cx += 8;
	while( width > 0 ) {

		cy = y;
		M_DrawCharacter( cx, cy, 2 );
		for( int n = 0; n < lines; n++ ) {

			cy += 8;
			M_DrawCharacter( cx, cy, 5 );
		}
		M_DrawCharacter( cx, cy + 8, 8 );
		width -= 1;
		cx += 8;
	}

	// draw right side
	cy = y;
	M_DrawCharacter( cx, cy, 3 );
	for( int n = 0; n < lines; n++ ) {

		cy += 8;
		M_DrawCharacter( cx, cy, 6 );
	}
	M_DrawCharacter( cx, cy + 8, 9 );
}

		
/*
=======================================================================

MAIN MENU

=======================================================================
*/
#define	MAIN_ITEMS	5

void Menu::M_Main_Draw( ) {

#if 0

	int i;
	int w, h;
	int ystart;
	int	xoffset;
	int widest = -1;
	int totalheight = 0;
	Str litname;

	for( i = 0; m_main_names[ i ]; i++ ) {

		Renderer::DrawGetPicSize( &w, &h, m_main_names[ i ] );

		if( w > widest ) widest = w;
		totalheight +=( h + 12 );
	}

	ystart =( Video::viddef.height / 2 - 110 );
	xoffset =( Video::viddef.width - widest + 70 ) / 2;

	for( i = 0; m_main_names[ i ].Length( ); i++ )
		if( i != m_main_cursor ) Renderer::DrawPic( xoffset, ystart + i * 40 + 13, m_main_names[ i ] );

	litname = m_main_names[ m_main_cursor ];
	litname.StripFileExtension( );
	litname.Append( "_sel.pcx" );
	Renderer::DrawPic( xoffset, ystart + m_main_cursor * 40 + 13, litname );

	M_DrawCursor( xoffset - 25, ystart + m_main_cursor * 40 + 11, ( int )( Client::cls.realtime / 100 )%NUM_CURSOR_FRAMES );

	Renderer::DrawGetPicSize( &w, &h, Str( "pics/m_main_plaque.pcx" ) );
	Renderer::DrawPic( xoffset - 30 - w, ystart, Str( "pics/m_main_plaque.pcx" ) );

	Renderer::DrawPic( xoffset - 30 - w, ystart + h + 5, Str( "pics/m_main_logo.pcx" ) );
	
#endif

}

const Str Menu::M_Main_Key( int key ) {
	
	switch( key ) {

	case VK_ESCAPE:

		M_PopMenu( );
		break;

	case VK_NUMPAD2:
	case VK_DOWN:

		if( ++m_main_cursor >= MAIN_ITEMS ) m_main_cursor = 0;
		return menu_move_sound;

	case VK_NUMPAD8:
	case VK_UP:

		if( --m_main_cursor < 0 ) m_main_cursor = MAIN_ITEMS - 1;
		return menu_move_sound;

	case VK_RETURN:

		m_entersound = true;

		switch( m_main_cursor ) {

		case 0:

			M_Menu_Game_f( );
			break;

		case 1:

			M_Menu_Multiplayer_f( );
			break;

		case 2:

			M_Menu_Options_f( );
			break;

		case 3:

			M_Menu_Video_f( );
			break;

		case 4:

			M_Menu_Quit_f( );
			break;
		}
	}

	return NULL;
}

void Menu::M_Menu_Main_f( ) {

	M_PushMenu( M_Main_Draw, M_Main_Key );
}

/*
=======================================================================

MULTIPLAYER MENU

=======================================================================
*/

void Menu::Multiplayer_MenuDraw( ) {

	M_Banner( Str( "pics/m_banner_multiplayer.pcx" ) );

	Menu_AdjustCursor( &s_multiplayer_menu, 1 );
	Menu_Draw( &s_multiplayer_menu );
}

void Menu::PlayerSetupFunc( void * unused ) {

	M_Menu_PlayerConfig_f( );
}

void Menu::JoinNetworkServerFunc( void * unused ) {

	M_Menu_JoinServer_f( );
}

void Menu::StartNetworkServerFunc( void * unused ) {

	M_Menu_StartServer_f( );
}

void Menu::Multiplayer_MenuInit( ) {

	s_multiplayer_menu.x = ( int )( Video::viddef.width * 0.50f ) - 64;
	s_multiplayer_menu.nitems = 0;

	s_join_network_server_action.generik.type	= MTYPE_ACTION;
	s_join_network_server_action.generik.flags  = QMF_LEFT_JUSTIFY;
	s_join_network_server_action.generik.x		= 0;
	s_join_network_server_action.generik.y		= 0;
	s_join_network_server_action.generik.name	= " join network server";
	s_join_network_server_action.generik.callback = JoinNetworkServerFunc;

	s_start_network_server_action.generik.type	= MTYPE_ACTION;
	s_start_network_server_action.generik.flags  = QMF_LEFT_JUSTIFY;
	s_start_network_server_action.generik.x		= 0;
	s_start_network_server_action.generik.y		= 10;
	s_start_network_server_action.generik.name	= " start network server";
	s_start_network_server_action.generik.callback = StartNetworkServerFunc;

	s_player_setup_action.generik.type	= MTYPE_ACTION;
	s_player_setup_action.generik.flags  = QMF_LEFT_JUSTIFY;
	s_player_setup_action.generik.x		= 0;
	s_player_setup_action.generik.y		= 20;
	s_player_setup_action.generik.name	= " player setup";
	s_player_setup_action.generik.callback = PlayerSetupFunc;

	Menu_AddItem( &s_multiplayer_menu, ( void * ) &s_join_network_server_action );
	Menu_AddItem( &s_multiplayer_menu, ( void * ) &s_start_network_server_action );
	Menu_AddItem( &s_multiplayer_menu, ( void * ) &s_player_setup_action );

	Menu_SetStatusBar( &s_multiplayer_menu, Str( "" ) );

	Menu_Center( &s_multiplayer_menu );
}

const Str Menu::Multiplayer_MenuKey( int key ) {

	return Default_MenuKey( &s_multiplayer_menu, key );
}

void Menu::M_Menu_Multiplayer_f( ) {

	Multiplayer_MenuInit( );
	M_PushMenu( Multiplayer_MenuDraw, Multiplayer_MenuKey );
}

/*
=======================================================================

KEYS MENU

=======================================================================
*/

void Menu::M_UnbindCommand( const Str & command ) {

	Keyboard::Key_ClearBindByCommand( command );
}

void Menu::M_FindKeysForCommand( const Str & command, int * twokeys ) {

	twokeys[ 0 ] = twokeys[ 1 ] = -1;
	Keyboard::Key_FindKeyBindingsByCommand( command, twokeys, 2 );
}

void Menu::KeyCursorDrawFunc( menuframework_s * menu ) {

#if 0

	if( bind_grab ) Renderer::DrawChar( menu->x, menu->y + menu->cursor * 9, '=' );
	else Renderer::DrawChar( menu->x, menu->y + menu->cursor * 9, 12 +( ( int )( System::Sys_Milliseconds( ) / 250 ) & 1 ) );

#endif

}

void Menu::DrawKeyBindingFunc( void * self ) {

	int keys[ 2 ];
	menuaction_s * a =( menuaction_s * ) self;

	M_FindKeysForCommand( bindnames[a->generik.localdata[ 0 ]][ 0 ], keys );
		
	if( keys[ 0 ] == -1 ) Menu_DrawString( a->generik.x + a->generik.parent->x + 16, a->generik.y + a->generik.parent->y, "???" );
	else {

		int x;
		const char * name;

		name = Keyboard::Key_KeynumToName( keys[ 0 ] );

		Menu_DrawString( a->generik.x + a->generik.parent->x + 16, a->generik.y + a->generik.parent->y, name );

		x = ( int )strlen( name ) * 8;

		if( keys[ 1 ] != -1 ) {

			Menu_DrawString( a->generik.x + a->generik.parent->x + 24 + x, a->generik.y + a->generik.parent->y, "or" );
			Menu_DrawString( a->generik.x + a->generik.parent->x + 48 + x, a->generik.y + a->generik.parent->y, Keyboard::Key_KeynumToName( keys[ 1 ] ) );
		}
	}
}

void Menu::KeyBindingFunc( void * self ) {

	menuaction_s * a =( menuaction_s * ) self;
	int keys[ 2 ];

	M_FindKeysForCommand( bindnames[a->generik.localdata[ 0 ]][ 0 ], keys );

	if( keys[ 1 ] != -1 )
		M_UnbindCommand( bindnames[a->generik.localdata[ 0 ]][ 0 ] );

	bind_grab = true;

	Menu_SetStatusBar( &s_keys_menu, Str( "press a key or button for this action" ) );
}

void Menu::Keys_MenuInit( ) {

	int y = 0;
	int i = 0;

	s_keys_menu.x = ( int )( Video::viddef.width * 0.50f );
	s_keys_menu.nitems = 0;
	s_keys_menu.cursordraw = KeyCursorDrawFunc;

	s_keys_attack_action.generik.type	= MTYPE_ACTION;
	s_keys_attack_action.generik.flags  = QMF_GRAYED;
	s_keys_attack_action.generik.x		= 0;
	s_keys_attack_action.generik.y		= y;
	s_keys_attack_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_attack_action.generik.localdata[ 0 ] = i;
	s_keys_attack_action.generik.name	= bindnames[s_keys_attack_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_change_weapon_action.generik.type	= MTYPE_ACTION;
	s_keys_change_weapon_action.generik.flags  = QMF_GRAYED;
	s_keys_change_weapon_action.generik.x		= 0;
	s_keys_change_weapon_action.generik.y		= y += 9;
	s_keys_change_weapon_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_change_weapon_action.generik.localdata[ 0 ] = ++i;
	s_keys_change_weapon_action.generik.name	= bindnames[s_keys_change_weapon_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_walk_forward_action.generik.type	= MTYPE_ACTION;
	s_keys_walk_forward_action.generik.flags  = QMF_GRAYED;
	s_keys_walk_forward_action.generik.x		= 0;
	s_keys_walk_forward_action.generik.y		= y += 9;
	s_keys_walk_forward_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_walk_forward_action.generik.localdata[ 0 ] = ++i;
	s_keys_walk_forward_action.generik.name	= bindnames[s_keys_walk_forward_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_backpedal_action.generik.type	= MTYPE_ACTION;
	s_keys_backpedal_action.generik.flags  = QMF_GRAYED;
	s_keys_backpedal_action.generik.x		= 0;
	s_keys_backpedal_action.generik.y		= y += 9;
	s_keys_backpedal_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_backpedal_action.generik.localdata[ 0 ] = ++i;
	s_keys_backpedal_action.generik.name	= bindnames[s_keys_backpedal_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_turn_left_action.generik.type	= MTYPE_ACTION;
	s_keys_turn_left_action.generik.flags  = QMF_GRAYED;
	s_keys_turn_left_action.generik.x		= 0;
	s_keys_turn_left_action.generik.y		= y += 9;
	s_keys_turn_left_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_turn_left_action.generik.localdata[ 0 ] = ++i;
	s_keys_turn_left_action.generik.name	= bindnames[s_keys_turn_left_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_turn_right_action.generik.type	= MTYPE_ACTION;
	s_keys_turn_right_action.generik.flags  = QMF_GRAYED;
	s_keys_turn_right_action.generik.x		= 0;
	s_keys_turn_right_action.generik.y		= y += 9;
	s_keys_turn_right_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_turn_right_action.generik.localdata[ 0 ] = ++i;
	s_keys_turn_right_action.generik.name	= bindnames[s_keys_turn_right_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_run_action.generik.type	= MTYPE_ACTION;
	s_keys_run_action.generik.flags  = QMF_GRAYED;
	s_keys_run_action.generik.x		= 0;
	s_keys_run_action.generik.y		= y += 9;
	s_keys_run_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_run_action.generik.localdata[ 0 ] = ++i;
	s_keys_run_action.generik.name	= bindnames[s_keys_run_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_step_left_action.generik.type	= MTYPE_ACTION;
	s_keys_step_left_action.generik.flags  = QMF_GRAYED;
	s_keys_step_left_action.generik.x		= 0;
	s_keys_step_left_action.generik.y		= y += 9;
	s_keys_step_left_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_step_left_action.generik.localdata[ 0 ] = ++i;
	s_keys_step_left_action.generik.name	= bindnames[s_keys_step_left_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_step_right_action.generik.type	= MTYPE_ACTION;
	s_keys_step_right_action.generik.flags  = QMF_GRAYED;
	s_keys_step_right_action.generik.x		= 0;
	s_keys_step_right_action.generik.y		= y += 9;
	s_keys_step_right_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_step_right_action.generik.localdata[ 0 ] = ++i;
	s_keys_step_right_action.generik.name	= bindnames[s_keys_step_right_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_sidestep_action.generik.type	= MTYPE_ACTION;
	s_keys_sidestep_action.generik.flags  = QMF_GRAYED;
	s_keys_sidestep_action.generik.x		= 0;
	s_keys_sidestep_action.generik.y		= y += 9;
	s_keys_sidestep_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_sidestep_action.generik.localdata[ 0 ] = ++i;
	s_keys_sidestep_action.generik.name	= bindnames[s_keys_sidestep_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_look_up_action.generik.type	= MTYPE_ACTION;
	s_keys_look_up_action.generik.flags  = QMF_GRAYED;
	s_keys_look_up_action.generik.x		= 0;
	s_keys_look_up_action.generik.y		= y += 9;
	s_keys_look_up_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_look_up_action.generik.localdata[ 0 ] = ++i;
	s_keys_look_up_action.generik.name	= bindnames[s_keys_look_up_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_look_down_action.generik.type	= MTYPE_ACTION;
	s_keys_look_down_action.generik.flags  = QMF_GRAYED;
	s_keys_look_down_action.generik.x		= 0;
	s_keys_look_down_action.generik.y		= y += 9;
	s_keys_look_down_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_look_down_action.generik.localdata[ 0 ] = ++i;
	s_keys_look_down_action.generik.name	= bindnames[s_keys_look_down_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_center_view_action.generik.type	= MTYPE_ACTION;
	s_keys_center_view_action.generik.flags  = QMF_GRAYED;
	s_keys_center_view_action.generik.x		= 0;
	s_keys_center_view_action.generik.y		= y += 9;
	s_keys_center_view_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_center_view_action.generik.localdata[ 0 ] = ++i;
	s_keys_center_view_action.generik.name	= bindnames[s_keys_center_view_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_mouse_look_action.generik.type	= MTYPE_ACTION;
	s_keys_mouse_look_action.generik.flags  = QMF_GRAYED;
	s_keys_mouse_look_action.generik.x		= 0;
	s_keys_mouse_look_action.generik.y		= y += 9;
	s_keys_mouse_look_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_mouse_look_action.generik.localdata[ 0 ] = ++i;
	s_keys_mouse_look_action.generik.name	= bindnames[s_keys_mouse_look_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_keyboard_look_action.generik.type	= MTYPE_ACTION;
	s_keys_keyboard_look_action.generik.flags  = QMF_GRAYED;
	s_keys_keyboard_look_action.generik.x		= 0;
	s_keys_keyboard_look_action.generik.y		= y += 9;
	s_keys_keyboard_look_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_keyboard_look_action.generik.localdata[ 0 ] = ++i;
	s_keys_keyboard_look_action.generik.name	= bindnames[s_keys_keyboard_look_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_move_up_action.generik.type	= MTYPE_ACTION;
	s_keys_move_up_action.generik.flags  = QMF_GRAYED;
	s_keys_move_up_action.generik.x		= 0;
	s_keys_move_up_action.generik.y		= y += 9;
	s_keys_move_up_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_move_up_action.generik.localdata[ 0 ] = ++i;
	s_keys_move_up_action.generik.name	= bindnames[s_keys_move_up_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_move_down_action.generik.type	= MTYPE_ACTION;
	s_keys_move_down_action.generik.flags  = QMF_GRAYED;
	s_keys_move_down_action.generik.x		= 0;
	s_keys_move_down_action.generik.y		= y += 9;
	s_keys_move_down_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_move_down_action.generik.localdata[ 0 ] = ++i;
	s_keys_move_down_action.generik.name	= bindnames[s_keys_move_down_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_inventory_action.generik.type	= MTYPE_ACTION;
	s_keys_inventory_action.generik.flags  = QMF_GRAYED;
	s_keys_inventory_action.generik.x		= 0;
	s_keys_inventory_action.generik.y		= y += 9;
	s_keys_inventory_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_inventory_action.generik.localdata[ 0 ] = ++i;
	s_keys_inventory_action.generik.name	= bindnames[s_keys_inventory_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_inv_use_action.generik.type	= MTYPE_ACTION;
	s_keys_inv_use_action.generik.flags  = QMF_GRAYED;
	s_keys_inv_use_action.generik.x		= 0;
	s_keys_inv_use_action.generik.y		= y += 9;
	s_keys_inv_use_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_inv_use_action.generik.localdata[ 0 ] = ++i;
	s_keys_inv_use_action.generik.name	= bindnames[s_keys_inv_use_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_inv_drop_action.generik.type	= MTYPE_ACTION;
	s_keys_inv_drop_action.generik.flags  = QMF_GRAYED;
	s_keys_inv_drop_action.generik.x		= 0;
	s_keys_inv_drop_action.generik.y		= y += 9;
	s_keys_inv_drop_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_inv_drop_action.generik.localdata[ 0 ] = ++i;
	s_keys_inv_drop_action.generik.name	= bindnames[s_keys_inv_drop_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_inv_prev_action.generik.type	= MTYPE_ACTION;
	s_keys_inv_prev_action.generik.flags  = QMF_GRAYED;
	s_keys_inv_prev_action.generik.x		= 0;
	s_keys_inv_prev_action.generik.y		= y += 9;
	s_keys_inv_prev_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_inv_prev_action.generik.localdata[ 0 ] = ++i;
	s_keys_inv_prev_action.generik.name	= bindnames[s_keys_inv_prev_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_inv_next_action.generik.type	= MTYPE_ACTION;
	s_keys_inv_next_action.generik.flags  = QMF_GRAYED;
	s_keys_inv_next_action.generik.x		= 0;
	s_keys_inv_next_action.generik.y		= y += 9;
	s_keys_inv_next_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_inv_next_action.generik.localdata[ 0 ] = ++i;
	s_keys_inv_next_action.generik.name	= bindnames[s_keys_inv_next_action.generik.localdata[ 0 ]][ 1 ];

	s_keys_help_computer_action.generik.type	= MTYPE_ACTION;
	s_keys_help_computer_action.generik.flags  = QMF_GRAYED;
	s_keys_help_computer_action.generik.x		= 0;
	s_keys_help_computer_action.generik.y		= y += 9;
	s_keys_help_computer_action.generik.ownerdraw = DrawKeyBindingFunc;
	s_keys_help_computer_action.generik.localdata[ 0 ] = ++i;
	s_keys_help_computer_action.generik.name	= bindnames[s_keys_help_computer_action.generik.localdata[ 0 ]][ 1 ];

	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_attack_action );
	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_change_weapon_action );
	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_walk_forward_action );
	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_backpedal_action );
	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_turn_left_action );
	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_turn_right_action );
	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_run_action );
	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_step_left_action );
	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_step_right_action );
	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_sidestep_action );
	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_look_up_action );
	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_look_down_action );
	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_center_view_action );
	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_mouse_look_action );
	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_keyboard_look_action );
	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_move_up_action );
	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_move_down_action );

	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_inventory_action );
	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_inv_use_action );
	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_inv_drop_action );
	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_inv_prev_action );
	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_inv_next_action );

	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_help_computer_action );
	
	Menu_SetStatusBar( &s_keys_menu, Str( "enter to change, backspace to clear" ) );
	Menu_Center( &s_keys_menu );
}

void Menu::Keys_MenuDraw( ) {

	Menu_AdjustCursor( &s_keys_menu, 1 );
	Menu_Draw( &s_keys_menu );
}

const Str Menu::Keys_MenuKey( int key ) {

	menuaction_s * item =( menuaction_s * ) Menu_ItemAtCursor( &s_keys_menu );

	if( bind_grab ) {	

		if( key != VK_ESCAPE && key != VK_SCROLL ) {

			Str cmd;

			sprintf( cmd, "bind \"%s\" \"%s\"\n", Keyboard::Key_KeynumToString( key ).c_str( ), bindnames[item->generik.localdata[ 0 ]][ 0 ].c_str( ) );
			CBuffer::Cbuf_InsertText( cmd );
		}
		
		Menu_SetStatusBar( &s_keys_menu, Str( "enter to change, backspace to clear" ) );
		bind_grab = false;
		return menu_out_sound;
	}

	switch( key ) {

	case VK_RETURN:

		KeyBindingFunc( item );
		return menu_in_sound;

	case VK_BACK:		// delete bindings
	case VK_DELETE:				// delete bindings
	case VK_DECIMAL:

		M_UnbindCommand( bindnames[item->generik.localdata[ 0 ]][ 0 ] );
		return menu_out_sound;

	default:

		return Default_MenuKey( &s_keys_menu, key );
	}
}

void Menu::M_Menu_Keys_f( ) {

	Keys_MenuInit( );
	M_PushMenu( Keys_MenuDraw, Keys_MenuKey );
}


/*
=======================================================================

CONTROLS MENU

=======================================================================
*/

void Menu::CrosshairFunc( void * unused ) {

	CVarSystem::SetCVarInt( "crosshair", s_options_crosshair_box.curvalue );
}

void Menu::JoystickFunc( void * unused ) {

	CVarSystem::SetCVarInt( "in_joystick", s_options_joystick_box.curvalue );
}

void Menu::CustomizeControlsFunc( void * unused ) {

	M_Menu_Keys_f( );
}

void Menu::AlwaysRunFunc( void * unused ) {

	CVarSystem::SetCVarInt( "cl_run", s_options_alwaysrun_box.curvalue );
}

void Menu::FreeLookFunc( void * unused ) {

	CVarSystem::SetCVarInt( "freelook", s_options_freelook_box.curvalue );
}

void Menu::MouseSpeedFunc( void  * unused ) {

	CVarSystem::SetCVarFloat( "sensitivity", s_options_sensitivity_slider.curvalue / 2.0f );
}

void Menu::ControlsSetMenuItemValues( ) {

	s_options_sfxvolume_slider.curvalue		= ( float )CVarSystem::GetCVarInt( "s_volume" ) * 10;

	s_options_cdvolume_box.curvalue 		= !CVarSystem::GetCVarInt( "cd_nocd" );

	s_options_quality_list.curvalue			= !CVarSystem::GetCVarInt( "s_loadas8bit" );

	s_options_sensitivity_slider.curvalue	= ( float )CVarSystem::GetCVarInt( "sensitivity" ) * 2;

	s_options_alwaysrun_box.curvalue		= CVarSystem::GetCVarInt( "cl_run" );

	s_options_invertmouse_box.curvalue		= CVarSystem::GetCVarInt( "m_pitch" ) < 0;

	s_options_lookspring_box.curvalue		= CVarSystem::GetCVarInt( "lookspring" );

	s_options_lookstrafe_box.curvalue		= CVarSystem::GetCVarInt( "lookstrafe" );

	s_options_freelook_box.curvalue			= CVarSystem::GetCVarInt( "freelook" );

	s_options_crosshair_box.curvalue		= CVarSystem::GetCVarInt( "crosshair" );

	s_options_joystick_box.curvalue			= CVarSystem::GetCVarInt( "in_joystick" );
}

void Menu::ControlsResetDefaultsFunc( void * unused ) {

	CBuffer::Cbuf_AddText( Str( "exec default.cfg\n" ) );
	CBuffer::Cbuf_Execute( );

	ControlsSetMenuItemValues( );
}

void Menu::InvertMouseFunc( void * unused ) {

	CVarSystem::SetCVarFloat( "m_pitch", -CVarSystem::GetCVarFloat( "m_pitch" ) );
}

void Menu::LookspringFunc( void * unused ) {

	CVarSystem::SetCVarBool( "lookspring", !CVarSystem::GetCVarBool( "lookspring" ) );
}

void Menu::LookstrafeFunc( void * unused ) {

	CVarSystem::SetCVarBool( "lookstrafe", !CVarSystem::GetCVarBool( "lookstrafe" ) );
}

void Menu::UpdateVolumeFunc( void * unused ) {

	CVarSystem::SetCVarFloat( "s_volume", ( float )s_options_sfxvolume_slider.curvalue / 10.0f );
}

void Menu::UpdateCDVolumeFunc( void * unused ) {

	CVarSystem::SetCVarBool( "cd_nocd", !s_options_cdvolume_box.curvalue );
}

void Menu::ConsoleFunc( void * unused ) {

	/*
	* * the proper way to do this is probably to have ToggleConsole_f accept a parameter
	*/

	if( Client::cl.attractloop ) {

		CBuffer::Cbuf_AddText( Str( "killserver\n" ) );
		return;
	}

	Keyboard::Key_ClearTyping( );
	Console::Con_ClearNotify( );

	M_ForceMenuOff( );
	Client::cls.key_dest = key_console;
}

void Menu::UpdateSoundQualityFunc( void * unused ) {
	
	CVarSystem::SetCVarInt( "s_primary", s_options_compatibility_list.curvalue );

	M_DrawTextBox( 8, 120 - 48, 36, 3 );
	M_Print( 16 + 16, 120 - 48 + 8,  Str( "Restarting the sound system. This" ) );
	M_Print( 16 + 16, 120 - 48 + 16, Str( "could take up to a minute, so" ) );
	M_Print( 16 + 16, 120 - 48 + 24, Str( "please be patient." ) );

	// the text box won't show up unless we do a buffer swap
	Renderer::EndFrame( );

	Client::CL_Snd_Restart_f( );
}

void Menu::Options_MenuInit( ) {

	/*
	* * configure controls menu and menu items
	*/
	s_options_menu.x = Video::viddef.width / 2;
	s_options_menu.y = Video::viddef.height / 2 - 58;
	s_options_menu.nitems = 0;

	s_options_sfxvolume_slider.generik.type	= MTYPE_SLIDER;
	s_options_sfxvolume_slider.generik.x	= 0;
	s_options_sfxvolume_slider.generik.y	= 0;
	s_options_sfxvolume_slider.generik.name	= "effects volume";
	s_options_sfxvolume_slider.generik.callback	= UpdateVolumeFunc;
	s_options_sfxvolume_slider.minvalue		= 0;
	s_options_sfxvolume_slider.maxvalue		= 10;
	s_options_sfxvolume_slider.curvalue		= CVarSystem::GetCVarFloat( "s_volume" ) * 10.0f;

	s_options_cdvolume_box.generik.type	= MTYPE_SPINCONTROL;
	s_options_cdvolume_box.generik.x		= 0;
	s_options_cdvolume_box.generik.y		= 10;
	s_options_cdvolume_box.generik.name	= "CD music";
	s_options_cdvolume_box.generik.callback	= UpdateCDVolumeFunc;
	s_options_cdvolume_box.itemnames		= cd_music_items;
	s_options_cdvolume_box.curvalue 		= !CVarSystem::GetCVarInt( "cd_nocd" );

	s_options_quality_list.generik.type	= MTYPE_SPINCONTROL;
	s_options_quality_list.generik.x		= 0;
	s_options_quality_list.generik.y		= 20;;
	s_options_quality_list.generik.name		= "sound quality";
	s_options_quality_list.generik.callback = UpdateSoundQualityFunc;
	s_options_quality_list.itemnames		= quality_items;
	s_options_quality_list.curvalue			= !CVarSystem::GetCVarInt( "s_loadas8bit" );

	s_options_compatibility_list.generik.type	= MTYPE_SPINCONTROL;
	s_options_compatibility_list.generik.x		= 0;
	s_options_compatibility_list.generik.y		= 30;
	s_options_compatibility_list.generik.name	= "sound compatibility";
	s_options_compatibility_list.generik.callback = UpdateSoundQualityFunc;
	s_options_compatibility_list.itemnames		= compatibility_items;
	s_options_compatibility_list.curvalue		= CVarSystem::GetCVarInt( "s_primary" );

	s_options_sensitivity_slider.generik.type	= MTYPE_SLIDER;
	s_options_sensitivity_slider.generik.x		= 0;
	s_options_sensitivity_slider.generik.y		= 50;
	s_options_sensitivity_slider.generik.name	= "mouse speed";
	s_options_sensitivity_slider.generik.callback = MouseSpeedFunc;
	s_options_sensitivity_slider.minvalue		= 2;
	s_options_sensitivity_slider.maxvalue		= 22;

	s_options_alwaysrun_box.generik.type = MTYPE_SPINCONTROL;
	s_options_alwaysrun_box.generik.x	= 0;
	s_options_alwaysrun_box.generik.y	= 60;
	s_options_alwaysrun_box.generik.name	= "always run";
	s_options_alwaysrun_box.generik.callback = AlwaysRunFunc;
	s_options_alwaysrun_box.itemnames = yesno_names;

	s_options_invertmouse_box.generik.type = MTYPE_SPINCONTROL;
	s_options_invertmouse_box.generik.x	= 0;
	s_options_invertmouse_box.generik.y	= 70;
	s_options_invertmouse_box.generik.name	= "invert mouse";
	s_options_invertmouse_box.generik.callback = InvertMouseFunc;
	s_options_invertmouse_box.itemnames = yesno_names;

	s_options_lookspring_box.generik.type = MTYPE_SPINCONTROL;
	s_options_lookspring_box.generik.x	= 0;
	s_options_lookspring_box.generik.y	= 80;
	s_options_lookspring_box.generik.name	= "lookspring";
	s_options_lookspring_box.generik.callback = LookspringFunc;
	s_options_lookspring_box.itemnames = yesno_names;

	s_options_lookstrafe_box.generik.type = MTYPE_SPINCONTROL;
	s_options_lookstrafe_box.generik.x	= 0;
	s_options_lookstrafe_box.generik.y	= 90;
	s_options_lookstrafe_box.generik.name	= "lookstrafe";
	s_options_lookstrafe_box.generik.callback = LookstrafeFunc;
	s_options_lookstrafe_box.itemnames = yesno_names;

	s_options_freelook_box.generik.type = MTYPE_SPINCONTROL;
	s_options_freelook_box.generik.x	= 0;
	s_options_freelook_box.generik.y	= 100;
	s_options_freelook_box.generik.name	= "free look";
	s_options_freelook_box.generik.callback = FreeLookFunc;
	s_options_freelook_box.itemnames = yesno_names;

	s_options_crosshair_box.generik.type = MTYPE_SPINCONTROL;
	s_options_crosshair_box.generik.x	= 0;
	s_options_crosshair_box.generik.y	= 110;
	s_options_crosshair_box.generik.name	= "crosshair";
	s_options_crosshair_box.generik.callback = CrosshairFunc;
	s_options_crosshair_box.itemnames = crosshair_names;
/*
	s_options_noalttab_box.generik.type = MTYPE_SPINCONTROL;
	s_options_noalttab_box.generik.x	= 0;
	s_options_noalttab_box.generik.y	= 110;
	s_options_noalttab_box.generik.name	= "disable alt-tab";
	s_options_noalttab_box.generik.callback = NoAltTabFunc;
	s_options_noalttab_box.itemnames = yesno_names;
*/
	s_options_joystick_box.generik.type = MTYPE_SPINCONTROL;
	s_options_joystick_box.generik.x	= 0;
	s_options_joystick_box.generik.y	= 120;
	s_options_joystick_box.generik.name	= "use joystick";
	s_options_joystick_box.generik.callback = JoystickFunc;
	s_options_joystick_box.itemnames = yesno_names;

	s_options_customize_options_action.generik.type	= MTYPE_ACTION;
	s_options_customize_options_action.generik.x		= 0;
	s_options_customize_options_action.generik.y		= 140;
	s_options_customize_options_action.generik.name	= "customize controls";
	s_options_customize_options_action.generik.callback = CustomizeControlsFunc;

	s_options_defaults_action.generik.type	= MTYPE_ACTION;
	s_options_defaults_action.generik.x		= 0;
	s_options_defaults_action.generik.y		= 150;
	s_options_defaults_action.generik.name	= "reset defaults";
	s_options_defaults_action.generik.callback = ControlsResetDefaultsFunc;

	s_options_console_action.generik.type	= MTYPE_ACTION;
	s_options_console_action.generik.x		= 0;
	s_options_console_action.generik.y		= 160;
	s_options_console_action.generik.name	= "go to console";
	s_options_console_action.generik.callback = ConsoleFunc;

	ControlsSetMenuItemValues( );

	Menu_AddItem( &s_options_menu, ( void * ) &s_options_sfxvolume_slider );
	Menu_AddItem( &s_options_menu, ( void * ) &s_options_cdvolume_box );
	Menu_AddItem( &s_options_menu, ( void * ) &s_options_quality_list );
	Menu_AddItem( &s_options_menu, ( void * ) &s_options_compatibility_list );
	Menu_AddItem( &s_options_menu, ( void * ) &s_options_sensitivity_slider );
	Menu_AddItem( &s_options_menu, ( void * ) &s_options_alwaysrun_box );
	Menu_AddItem( &s_options_menu, ( void * ) &s_options_invertmouse_box );
	Menu_AddItem( &s_options_menu, ( void * ) &s_options_lookspring_box );
	Menu_AddItem( &s_options_menu, ( void * ) &s_options_lookstrafe_box );
	Menu_AddItem( &s_options_menu, ( void * ) &s_options_freelook_box );
	Menu_AddItem( &s_options_menu, ( void * ) &s_options_crosshair_box );
	Menu_AddItem( &s_options_menu, ( void * ) &s_options_joystick_box );
	Menu_AddItem( &s_options_menu, ( void * ) &s_options_customize_options_action );
	Menu_AddItem( &s_options_menu, ( void * ) &s_options_defaults_action );
	Menu_AddItem( &s_options_menu, ( void * ) &s_options_console_action );
}

void Menu::Options_MenuDraw( ) {

	M_Banner( Str( "pics/m_banner_options.pcx" ) );
	Menu_AdjustCursor( &s_options_menu, 1 );
	Menu_Draw( &s_options_menu );
}

const Str Menu::Options_MenuKey( int key ) {

	return Default_MenuKey( &s_options_menu, key );
}

void Menu::M_Menu_Options_f( ) {

	Options_MenuInit( );
	M_PushMenu( Options_MenuDraw, Options_MenuKey );
}

/*
=======================================================================

VIDEO MENU

=======================================================================
*/

void Menu::M_Menu_Video_f( ) {

	Video::VID_MenuInit( );
	M_PushMenu( Video::VID_MenuDraw, Video::VID_MenuKey );
}

/*
=============================================================================

END GAME MENU

=============================================================================
*/

void Menu::M_Credits_MenuDraw( ) {

#if 0

	int i, y;

	/*
	* * draw the credits
	*/
	for( i = 0, y = Video::viddef.height -( ( Client::cls.realtime - credits_start_time ) / 40.0f ); credits[ i ] && y < Video::viddef.height; y += 10, i++ )
	{
		int j, stringoffset = 0;
		int bold = false;

		if( y <= -8 )
			continue;

		if( credits[ i ][ 0 ] == '+' )
		{
			bold = true;
			stringoffset = 1;
		}
		else
		{
			bold = false;
			stringoffset = 0;
		}

		for( j = 0; credits[ i ][ j+stringoffset ]; j++ )
		{
			int x;

			x =( Video::viddef.width - strlen( credits[ i ] ) * 8 - stringoffset * 8 ) / 2 +( j + stringoffset ) * 8;

			if( bold )
				Renderer::DrawChar( x, y, credits[ i ][ j+stringoffset ] + 128 );
			else
				Renderer::DrawChar( x, y, credits[ i ][ j+stringoffset ] );
		}
	}

	if( y < 0 )
		credits_start_time = Client::cls.realtime;

#endif

}

const Str Menu::M_Credits_Key( int key ) {

	switch( key )
	{
	case VK_ESCAPE:
		if( creditsBuffer.IsValid( ) ) FileSystem::FreeFile( creditsBuffer );
		M_PopMenu( );
		break;
	}

	return menu_out_sound;

}

void Menu::M_Menu_Credits_f( ) {

	int		n;
	int		count = 0;
	const char	* p;

	creditsBuffer = FileSystem::ReadFile( "credits" );
	if( count != -1 )
	{
		p = ( char * )creditsBuffer.GetDataPtr( );
		for( n = 0; n < 255; n++ )
		{
			creditsIndex[ n ] = p;
			while( *p != '\r' && * p != '\n' )
			{
				p++;
				if( --count == 0 )
					break;
			}
			if( *p == '\r' )
			{
				//* p++ = 0;
				if( --count == 0 )
					break;
			}
			//* p++ = 0;
			if( --count == 0 )
				break;
		}
		creditsIndex[ ++n ] = 0;
	}

	credits_start_time = ( int )Client::cls.realtime;
	M_PushMenu( M_Credits_MenuDraw, M_Credits_Key );
}

/*
=============================================================================

GAME MENU

=============================================================================
*/

void Menu::StartGame( ) {

	// disable updates and start the cinematic going
	Client::cl.servercount = -1;
	M_ForceMenuOff( );

	CVarSystem::SetCVarInt( "deathmatch", 0 );
	CVarSystem::SetCVarInt( "coop", 0 );

	CVarSystem::SetCVarInt( "gamerules", 0 );		//PGM

	CBuffer::Cbuf_AddText( Str( "loading; killserver; wait; newgame\n" ) );
	Client::cls.key_dest = key_game;
}

void Menu::EasyGameFunc( void * data ) {

	CVarSystem::SetCVarInt( "skill", 0 );
	StartGame( );
}

void Menu::MediumGameFunc( void * data ) {

	CVarSystem::SetCVarInt( "skill", 1 );
	StartGame( );
}

void Menu::HardGameFunc( void * data ) {

	CVarSystem::SetCVarInt( "skill", 2 );
	StartGame( );
}

void Menu::LoadGameFunc( void * unused ) {

	M_Menu_LoadGame_f( );
}

void Menu::SaveGameFunc( void * unused ) {

	M_Menu_SaveGame_f( );
}

void Menu::CreditsFunc( void * unused ) {

	M_Menu_Credits_f( );
}

void Menu::Game_MenuInit( ) {

	s_game_menu.x = ( int )( Video::viddef.width * 0.50f );
	s_game_menu.nitems = 0;

	s_easy_game_action.generik.type	= MTYPE_ACTION;
	s_easy_game_action.generik.flags  = QMF_LEFT_JUSTIFY;
	s_easy_game_action.generik.x		= 0;
	s_easy_game_action.generik.y		= 0;
	s_easy_game_action.generik.name	= "easy";
	s_easy_game_action.generik.callback = EasyGameFunc;

	s_medium_game_action.generik.type	= MTYPE_ACTION;
	s_medium_game_action.generik.flags  = QMF_LEFT_JUSTIFY;
	s_medium_game_action.generik.x		= 0;
	s_medium_game_action.generik.y		= 10;
	s_medium_game_action.generik.name	= "medium";
	s_medium_game_action.generik.callback = MediumGameFunc;

	s_hard_game_action.generik.type	= MTYPE_ACTION;
	s_hard_game_action.generik.flags  = QMF_LEFT_JUSTIFY;
	s_hard_game_action.generik.x		= 0;
	s_hard_game_action.generik.y		= 20;
	s_hard_game_action.generik.name	= "hard";
	s_hard_game_action.generik.callback = HardGameFunc;

	s_blankline.generik.type = MTYPE_SEPARATOR;

	s_load_game_action.generik.type	= MTYPE_ACTION;
	s_load_game_action.generik.flags  = QMF_LEFT_JUSTIFY;
	s_load_game_action.generik.x		= 0;
	s_load_game_action.generik.y		= 40;
	s_load_game_action.generik.name	= "load game";
	s_load_game_action.generik.callback = LoadGameFunc;

	s_save_game_action.generik.type	= MTYPE_ACTION;
	s_save_game_action.generik.flags  = QMF_LEFT_JUSTIFY;
	s_save_game_action.generik.x		= 0;
	s_save_game_action.generik.y		= 50;
	s_save_game_action.generik.name	= "save game";
	s_save_game_action.generik.callback = SaveGameFunc;

	s_credits_action.generik.type	= MTYPE_ACTION;
	s_credits_action.generik.flags  = QMF_LEFT_JUSTIFY;
	s_credits_action.generik.x		= 0;
	s_credits_action.generik.y		= 60;
	s_credits_action.generik.name	= "credits";
	s_credits_action.generik.callback = CreditsFunc;

	Menu_AddItem( &s_game_menu, ( void * ) &s_easy_game_action );
	Menu_AddItem( &s_game_menu, ( void * ) &s_medium_game_action );
	Menu_AddItem( &s_game_menu, ( void * ) &s_hard_game_action );
	Menu_AddItem( &s_game_menu, ( void * ) &s_blankline );
	Menu_AddItem( &s_game_menu, ( void * ) &s_load_game_action );
	Menu_AddItem( &s_game_menu, ( void * ) &s_save_game_action );
	Menu_AddItem( &s_game_menu, ( void * ) &s_blankline );
	Menu_AddItem( &s_game_menu, ( void * ) &s_credits_action );

	Menu_Center( &s_game_menu );
}

void Menu::Game_MenuDraw( ) {

	M_Banner( Str( "pics/m_banner_game.pcx" ) );
	Menu_AdjustCursor( &s_game_menu, 1 );
	Menu_Draw( &s_game_menu );
}

const Str Menu::Game_MenuKey( int key ) {

	return Default_MenuKey( &s_game_menu, key );
}

void Menu::M_Menu_Game_f( ) {

	Game_MenuInit( );
	M_PushMenu( Game_MenuDraw, Game_MenuKey );
	m_game_cursor = 1;
}

/*
=============================================================================

LOADGAME MENU

=============================================================================
*/

void Menu::Create_Savestrings( ) {

	int		i;
	Str	name;

	for( i = 0; i<MAX_SAVEGAMES; i++ )
	{
		sprintf( name, "save/save%i/server.ssv", i );
		FileBase * f = FileSystem::OpenFileRead( name );
		if( !f ) return;
		f->Read( m_savestrings[ i ], sizeof( m_savestrings[ i ] ) );
		FileSystem::CloseFile( f );
		m_savevalid[ i ] = true;
	}
}

void Menu::LoadGameCallback( void * self ) {

	menuaction_s * a =( menuaction_s * ) self;

	if( m_savevalid[ a->generik.localdata[ 0 ] ] ) {

		Str str;
		sprintf( str, "load save%i\n",  a->generik.localdata[ 0 ] );
		CBuffer::Cbuf_AddText( str );
	}

	M_ForceMenuOff( );
}

void Menu::LoadGame_MenuInit( ) {

	int i;

	s_loadgame_menu.x = Video::viddef.width / 2 - 120;
	s_loadgame_menu.y = Video::viddef.height / 2 - 58;
	s_loadgame_menu.nitems = 0;

	Create_Savestrings( );

	for( i = 0; i < MAX_SAVEGAMES; i++ )
	{
		s_loadgame_actions[ i ].generik.name			= m_savestrings[ i ];
		s_loadgame_actions[ i ].generik.flags			= QMF_LEFT_JUSTIFY;
		s_loadgame_actions[ i ].generik.localdata[ 0 ]	= i;
		s_loadgame_actions[ i ].generik.callback		= LoadGameCallback;

		s_loadgame_actions[ i ].generik.x = 0;
		s_loadgame_actions[ i ].generik.y =( i ) * 10;
		if( i>0 )	// separate from autosave
			s_loadgame_actions[ i ].generik.y += 10;

		s_loadgame_actions[ i ].generik.type = MTYPE_ACTION;

		Menu_AddItem( &s_loadgame_menu, &s_loadgame_actions[ i ] );
	}
}

void Menu::LoadGame_MenuDraw( ) {

	M_Banner( Str( "pics/m_banner_load_game.pcx" ) );
//	Menu_AdjustCursor( &s_loadgame_menu, 1 );
	Menu_Draw( &s_loadgame_menu );
}

const Str Menu::LoadGame_MenuKey( int key ) {

	if( key == VK_ESCAPE || key == VK_RETURN )
	{
		s_savegame_menu.cursor = s_loadgame_menu.cursor - 1;
		if( s_savegame_menu.cursor < 0 )
			s_savegame_menu.cursor = 0;
	}
	return Default_MenuKey( &s_loadgame_menu, key );
}

void Menu::M_Menu_LoadGame_f( ) {

	LoadGame_MenuInit( );
	M_PushMenu( LoadGame_MenuDraw, LoadGame_MenuKey );
}


/*
=============================================================================

SAVEGAME MENU

=============================================================================
*/

void Menu::SaveGameCallback( void * self ) {

	menuaction_s * a =( menuaction_s * ) self;

	Str str;
	sprintf( str, "save save%i\n", a->generik.localdata[ 0 ] );

	CBuffer::Cbuf_AddText( str );
	M_ForceMenuOff( );
}

void Menu::SaveGame_MenuDraw( ) {

	M_Banner( Str( "pics/m_banner_save_game.pcx" ) );
	Menu_AdjustCursor( &s_savegame_menu, 1 );
	Menu_Draw( &s_savegame_menu );
}

void Menu::SaveGame_MenuInit( ) {

	int i;

	s_savegame_menu.x = Video::viddef.width / 2 - 120;
	s_savegame_menu.y = Video::viddef.height / 2 - 58;
	s_savegame_menu.nitems = 0;

	Create_Savestrings( );

	// don't include the autosave slot
	for( i = 0; i < MAX_SAVEGAMES-1; i++ )
	{
		s_savegame_actions[ i ].generik.name = m_savestrings[ i+1 ];
		s_savegame_actions[ i ].generik.localdata[ 0 ] = i+1;
		s_savegame_actions[ i ].generik.flags = QMF_LEFT_JUSTIFY;
		s_savegame_actions[ i ].generik.callback = SaveGameCallback;

		s_savegame_actions[ i ].generik.x = 0;
		s_savegame_actions[ i ].generik.y =( i ) * 10;

		s_savegame_actions[ i ].generik.type = MTYPE_ACTION;

		Menu_AddItem( &s_savegame_menu, &s_savegame_actions[ i ] );
	}
}

const Str Menu::SaveGame_MenuKey( int key ) {

	if( key == VK_RETURN || key == VK_ESCAPE )
	{
		s_loadgame_menu.cursor = s_savegame_menu.cursor - 1;
		if( s_loadgame_menu.cursor < 0 )
			s_loadgame_menu.cursor = 0;
	}
	return Default_MenuKey( &s_savegame_menu, key );
}

void Menu::M_Menu_SaveGame_f( ) {

	if( !Common::Com_ServerState( ) )
		return;		// not playing a game

	SaveGame_MenuInit( );
	M_PushMenu( SaveGame_MenuDraw, SaveGame_MenuKey );
	Create_Savestrings( );
}


/*
=============================================================================

JOIN SERVER MENU

=============================================================================
*/

void Menu::M_AddToServerList( netadr_t adr, const Str & inf ) {

	int		i;
	const char * info = inf;

	if( m_num_servers == MAX_LOCAL_SERVERS )
		return;
	while( *info == ' ' )
		info++;

	// ignore if duplicated
	for( i = 0; i<m_num_servers; i++ )
		if( !strcmp( info, local_server_names[ i ] ) ) 	return;

	local_server_netadr[ m_num_servers ] = adr;
	strncpy( local_server_names[ m_num_servers ], info, sizeof( local_server_names[ 0 ] )-1 );
	m_num_servers++;
}


void Menu::JoinServerFunc( void * self ) {

	Str	buffer;
	int		index;

	index = ( int )( ( menuaction_s * )self - s_joinserver_server_actions );

	if( Q_stricmp( local_server_names[ index ], NO_SERVER_STRING ) == 0 )
		return;

	if( index >= m_num_servers )
		return;

	sprintf( buffer, "connect %s\n", Network::NET_AdrToString( local_server_netadr[ index ] ).c_str( ) );
	CBuffer::Cbuf_AddText( buffer );
	M_ForceMenuOff( );
}

void Menu::AddressBookFunc( void * self ) {

	M_Menu_AddressBook_f( );
}

void Menu::NullCursorDraw( void * self ) {
}

void Menu::SearchLocalGames( ) {

	int		i;

	m_num_servers = 0;
	for( i = 0; i<MAX_LOCAL_SERVERS; i++ )
		strcpy( local_server_names[ i ], NO_SERVER_STRING );

	M_DrawTextBox( 8, 120 - 48, 36, 3 );
	M_Print( 16 + 16, 120 - 48 + 8,  Str( "Searching for local servers, this" ) );
	M_Print( 16 + 16, 120 - 48 + 16, Str( "could take up to a minute, so" ) );
	M_Print( 16 + 16, 120 - 48 + 24, Str( "please be patient." ) );

	// the text box won't show up unless we do a buffer swap
	Renderer::EndFrame( );

	// send out info packets
	Client::CL_PingServers_f( );
}

void Menu::SearchLocalGamesFunc( void * self ) {

	SearchLocalGames( );
}

void Menu::JoinServer_MenuInit( ) {

	int i;

	s_joinserver_menu.x = ( int )( Video::viddef.width * 0.50f ) - 120;
	s_joinserver_menu.nitems = 0;

	s_joinserver_address_book_action.generik.type	= MTYPE_ACTION;
	s_joinserver_address_book_action.generik.name	= "address book";
	s_joinserver_address_book_action.generik.flags	= QMF_LEFT_JUSTIFY;
	s_joinserver_address_book_action.generik.x		= 0;
	s_joinserver_address_book_action.generik.y		= 0;
	s_joinserver_address_book_action.generik.callback = AddressBookFunc;

	s_joinserver_search_action.generik.type = MTYPE_ACTION;
	s_joinserver_search_action.generik.name	= "refresh server list";
	s_joinserver_search_action.generik.flags	= QMF_LEFT_JUSTIFY;
	s_joinserver_search_action.generik.x	= 0;
	s_joinserver_search_action.generik.y	= 10;
	s_joinserver_search_action.generik.callback = SearchLocalGamesFunc;
	s_joinserver_search_action.generik.statusbar = "search for servers";

	s_joinserver_server_title.generik.type = MTYPE_SEPARATOR;
	s_joinserver_server_title.generik.name = "connect to...";
	s_joinserver_server_title.generik.x    = 80;
	s_joinserver_server_title.generik.y	   = 30;

	for( i = 0; i < MAX_LOCAL_SERVERS; i++ )
	{
		s_joinserver_server_actions[ i ].generik.type	= MTYPE_ACTION;
		strcpy( local_server_names[ i ], NO_SERVER_STRING );
		s_joinserver_server_actions[ i ].generik.name	= local_server_names[ i ];
		s_joinserver_server_actions[ i ].generik.flags	= QMF_LEFT_JUSTIFY;
		s_joinserver_server_actions[ i ].generik.x		= 0;
		s_joinserver_server_actions[ i ].generik.y		= 40 + i* 10;
		s_joinserver_server_actions[ i ].generik.callback = JoinServerFunc;
		s_joinserver_server_actions[ i ].generik.statusbar = "press ENTER to connect";
	}

	Menu_AddItem( &s_joinserver_menu, &s_joinserver_address_book_action );
	Menu_AddItem( &s_joinserver_menu, &s_joinserver_server_title );
	Menu_AddItem( &s_joinserver_menu, &s_joinserver_search_action );

	for( i = 0; i < 8; i++ )
		Menu_AddItem( &s_joinserver_menu, &s_joinserver_server_actions[ i ] );

	Menu_Center( &s_joinserver_menu );

	SearchLocalGames( );
}

void Menu::JoinServer_MenuDraw( ) {

	M_Banner( Str( "pics/m_banner_join_server.pcx" ) );
	Menu_Draw( &s_joinserver_menu );
}


const Str Menu::JoinServer_MenuKey( int key ) {

	return Default_MenuKey( &s_joinserver_menu, key );
}

void Menu::M_Menu_JoinServer_f( ) {

	JoinServer_MenuInit( );
	M_PushMenu( JoinServer_MenuDraw, JoinServer_MenuKey );
}


/*
=============================================================================

START SERVER MENU

=============================================================================
*/

void Menu::DMOptionsFunc( void * self ) {

	if( s_rules_box.curvalue == 1 ) return;
	M_Menu_DMOptions_f( );
}

void Menu::RulesChangeFunc( void * self ) {

	// DM
	if( s_rules_box.curvalue == 0 )
	{
		s_maxclients_field.generik.statusbar = NULL;
		s_startserver_dmoptions_action.generik.statusbar = NULL;
	}
	else if( s_rules_box.curvalue == 1 )		// coop				// PGM
	{
		s_maxclients_field.generik.statusbar = "4 maximum for cooperative";
		if( atoi( s_maxclients_field.buffer ) > 4 ) s_maxclients_field.buffer = "4";
		s_startserver_dmoptions_action.generik.statusbar = "N/A for cooperative";
	}
}

void Menu::StartServerActionFunc( void * self ) {

	char	startmap[ 1024 ];
	int		timelimit;
	int		fraglimit;
	int		maxclients;
	const char	* spot;

	strcpy( startmap, strchr( mapnames[ s_startmap_list.curvalue ], '\n' ) + 1 );

	maxclients  = atoi( s_maxclients_field.buffer );
	timelimit	= atoi( s_timelimit_field.buffer );
	fraglimit	= atoi( s_fraglimit_field.buffer );

	CVarSystem::SetCVarInt( "maxclients", maxclients );
	CVarSystem::SetCVarInt( "timelimit", timelimit );
	CVarSystem::SetCVarInt( "fraglimit", fraglimit );
	CVarSystem::SetCVarStr( "hostname", s_hostname_field.buffer );
	CVarSystem::SetCVarInt( "deathmatch", !s_rules_box.curvalue );
	CVarSystem::SetCVarInt( "coop", s_rules_box.curvalue );
	CVarSystem::SetCVarInt( "gamerules", 0 );

	spot = NULL;
	if( s_rules_box.curvalue == 1 )		// PGM
	{
 		if( Q_stricmp( startmap, "bunk1" ) == 0 )
  			spot = "start";
 		else if( Q_stricmp( startmap, "mintro" ) == 0 )
  			spot = "start";
 		else if( Q_stricmp( startmap, "fact1" ) == 0 )
  			spot = "start";
 		else if( Q_stricmp( startmap, "power1" ) == 0 )
  			spot = "pstart";
 		else if( Q_stricmp( startmap, "biggun" ) == 0 )
  			spot = "bstart";
 		else if( Q_stricmp( startmap, "hangar1" ) == 0 )
  			spot = "unitstart";
 		else if( Q_stricmp( startmap, "city1" ) == 0 )
  			spot = "unitstart";
 		else if( Q_stricmp( startmap, "boss1" ) == 0 )
			spot = "bosstart";
	}

	Str text;

	if( spot ) {

		if( Common::Com_ServerState( ) ) CBuffer::Cbuf_AddText( Str( "disconnect\n" ) );
		sprintf( text, "gamemap \"*%s$%s\"\n", startmap, spot );
		CBuffer::Cbuf_AddText( text );

	} else {

		sprintf( text, "map %s\n", startmap );
		CBuffer::Cbuf_AddText( text );
	}

	M_ForceMenuOff( );
}

void Menu::StartServer_MenuInit( )  {

	Str  mapsname;
	const char * s;
	int i;

	/*
	* * load the list of map names
	*/
	File_Memory fm = FileSystem::ReadFile( "maps.lst" );

	s = ( char * )fm.GetDataPtr( );

	i = 0;
	while( i < fm.Length( ) )
	{
		if( s[ i ] == '\r' )
			nummaps++;
		i++;
	}

	if( nummaps == 0 ) Common::Com_Error( ERR_DROP, "no maps in maps.lst\n" );

	mapnames = new Str [ nummaps + 1 ];

	s = ( char * )fm.GetDataPtr( );

	for( i = 0; i < nummaps; i++ ) {

		Str  shortname;
		Str  longname;
		Str  scratch;
		int		j, l;

		//shortname = COM_Parse( &s );
		l = ( int )strlen( shortname );
		for( j = 0; j<l; j++ )
			shortname[ j ] = toupper( shortname[ j ] );
		//longname = COM_Parse( &s );
		sprintf( scratch, "%s\n%s", longname.c_str( ), shortname.c_str( ) );

		mapnames[ i ] = scratch;
	}
	mapnames[ nummaps ] = 0;

	FileSystem::FreeFile( fm );

	/*
	* * initialize the menu stuff
	*/
	s_startserver_menu.x = ( int )( Video::viddef.width * 0.50f );
	s_startserver_menu.nitems = 0;

	s_startmap_list.generik.type = MTYPE_SPINCONTROL;
	s_startmap_list.generik.x	= 0;
	s_startmap_list.generik.y	= 0;
	s_startmap_list.generik.name	= "initial map";
	s_startmap_list.itemnames = mapnames;

	s_rules_box.generik.type = MTYPE_SPINCONTROL;
	s_rules_box.generik.x	= 0;
	s_rules_box.generik.y	= 20;
	s_rules_box.generik.name	= "rules";
	
	s_rules_box.itemnames = dm_coop_names;

	if( CVarSystem::GetCVarBool( "coop" ) ) s_rules_box.curvalue = 1;
	else s_rules_box.curvalue = 0;
	s_rules_box.generik.callback = RulesChangeFunc;

	s_timelimit_field.generik.type = MTYPE_FIELD;
	s_timelimit_field.generik.name = "time limit";
	s_timelimit_field.generik.flags = QMF_NUMBERSONLY;
	s_timelimit_field.generik.x	= 0;
	s_timelimit_field.generik.y	= 36;
	s_timelimit_field.generik.statusbar = "0 = no limit";
	s_timelimit_field.length = 3;
	s_timelimit_field.visible_length = 3;
	s_timelimit_field.buffer = CVarSystem::GetCVarStr( "timelimit" );

	s_fraglimit_field.generik.type = MTYPE_FIELD;
	s_fraglimit_field.generik.name = "frag limit";
	s_fraglimit_field.generik.flags = QMF_NUMBERSONLY;
	s_fraglimit_field.generik.x	= 0;
	s_fraglimit_field.generik.y	= 54;
	s_fraglimit_field.generik.statusbar = "0 = no limit";
	s_fraglimit_field.length = 3;
	s_fraglimit_field.visible_length = 3;
	s_fraglimit_field.buffer = CVarSystem::GetCVarStr( "fraglimit" );

	/*
	* * maxclients determines the maximum number of players that can join
	* * the game.  If maxclients is only "1" then we should default the menu
	* * option to 8 players, otherwise use whatever its current value is. 
	* * Clamping will be done when the server is actually started.
	*/
	s_maxclients_field.generik.type = MTYPE_FIELD;
	s_maxclients_field.generik.name = "max players";
	s_maxclients_field.generik.flags = QMF_NUMBERSONLY;
	s_maxclients_field.generik.x	= 0;
	s_maxclients_field.generik.y	= 72;
	s_maxclients_field.generik.statusbar = NULL;
	s_maxclients_field.length = 3;
	s_maxclients_field.visible_length = 3;
	if( Common::maxclients.GetInt( ) == 1 ) s_maxclients_field.buffer = "8";
	else s_maxclients_field.buffer = CVarSystem::GetCVarStr( "maxclients" );

	s_hostname_field.generik.type = MTYPE_FIELD;
	s_hostname_field.generik.name = "hostname";
	s_hostname_field.generik.flags = 0;
	s_hostname_field.generik.x	= 0;
	s_hostname_field.generik.y	= 90;
	s_hostname_field.generik.statusbar = NULL;
	s_hostname_field.length = 12;
	s_hostname_field.visible_length = 12;
	s_hostname_field.buffer = CVarSystem::GetCVarStr( "hostname" );

	s_startserver_dmoptions_action.generik.type = MTYPE_ACTION;
	s_startserver_dmoptions_action.generik.name	= " deathmatch flags";
	s_startserver_dmoptions_action.generik.flags = QMF_LEFT_JUSTIFY;
	s_startserver_dmoptions_action.generik.x	= 24;
	s_startserver_dmoptions_action.generik.y	= 108;
	s_startserver_dmoptions_action.generik.statusbar = NULL;
	s_startserver_dmoptions_action.generik.callback = DMOptionsFunc;

	s_startserver_start_action.generik.type = MTYPE_ACTION;
	s_startserver_start_action.generik.name	= " begin";
	s_startserver_start_action.generik.flags = QMF_LEFT_JUSTIFY;
	s_startserver_start_action.generik.x	= 24;
	s_startserver_start_action.generik.y	= 128;
	s_startserver_start_action.generik.callback = StartServerActionFunc;

	Menu_AddItem( &s_startserver_menu, &s_startmap_list );
	Menu_AddItem( &s_startserver_menu, &s_rules_box );
	Menu_AddItem( &s_startserver_menu, &s_timelimit_field );
	Menu_AddItem( &s_startserver_menu, &s_fraglimit_field );
	Menu_AddItem( &s_startserver_menu, &s_maxclients_field );
	Menu_AddItem( &s_startserver_menu, &s_hostname_field );
	Menu_AddItem( &s_startserver_menu, &s_startserver_dmoptions_action );
	Menu_AddItem( &s_startserver_menu, &s_startserver_start_action );

	Menu_Center( &s_startserver_menu );

	// call this now to set proper inital state
	RulesChangeFunc( NULL );
}

void Menu::StartServer_MenuDraw( ) {

	Menu_Draw( &s_startserver_menu );
}

const Str Menu::StartServer_MenuKey( int key ) {

	if( key == VK_ESCAPE ) {
		if( mapnames ) {

			delete[] mapnames;
		}
		mapnames = 0;
		nummaps = 0;
	}

	return Default_MenuKey( &s_startserver_menu, key );
}

void Menu::M_Menu_StartServer_f( ) {

	StartServer_MenuInit( );
	M_PushMenu( StartServer_MenuDraw, StartServer_MenuKey );
}

/*
=============================================================================

DMOPTIONS BOOK MENU

=============================================================================
*/

void Menu::DMFlagCallback( void * self ) {

	menulist_s * f =( menulist_s * ) self;
	int flags;
	int bit = 0;

	flags = CVarSystem::GetCVarInt( "dmflags" );

	if( f == &s_friendlyfire_box )
	{
		if( f->curvalue )
			flags &= ~DF_NO_FRIENDLY_FIRE;
		else
			flags |= DF_NO_FRIENDLY_FIRE;
		goto setvalue;
	}
	else if( f == &s_falls_box )
	{
		if( f->curvalue )
			flags &= ~DF_NO_FALLING;
		else
			flags |= DF_NO_FALLING;
		goto setvalue;
	}
	else if( f == &s_weapons_stay_box )  {
		bit = DF_WEAPONS_STAY;
	}
	else if( f == &s_instant_powerups_box )
	{
		bit = DF_INSTANT_ITEMS;
	}
	else if( f == &s_allow_exit_box )
	{
		bit = DF_ALLOW_EXIT;
	}
	else if( f == &s_powerups_box )
	{
		if( f->curvalue )
			flags &= ~DF_NO_ITEMS;
		else
			flags |= DF_NO_ITEMS;
		goto setvalue;
	}
	else if( f == &s_health_box )
	{
		if( f->curvalue )
			flags &= ~DF_NO_HEALTH;
		else
			flags |= DF_NO_HEALTH;
		goto setvalue;
	}
	else if( f == &s_spawn_farthest_box )
	{
		bit = DF_SPAWN_FARTHEST;
	}
	else if( f == &s_teamplay_box )
	{
		if( f->curvalue == 1 )
		{
			flags |=  DF_SKINTEAMS;
			flags &= ~DF_MODELTEAMS;
		}
		else if( f->curvalue == 2 )
		{
			flags |=  DF_MODELTEAMS;
			flags &= ~DF_SKINTEAMS;
		}
		else
		{
			flags &= ~( DF_MODELTEAMS | DF_SKINTEAMS );
		}

		goto setvalue;
	}
	else if( f == &s_samelevel_box )
	{
		bit = DF_SAME_LEVEL;
	}
	else if( f == &s_force_respawn_box )
	{
		bit = DF_FORCE_RESPAWN;
	}
	else if( f == &s_armor_box )
	{
		if( f->curvalue )
			flags &= ~DF_NO_ARMOR;
		else
			flags |= DF_NO_ARMOR;
		goto setvalue;
	}
	else if( f == &s_infinite_ammo_box )
	{
		bit = DF_INFINITE_AMMO;
	}
	else if( f == &s_fixed_fov_box )
	{
		bit = DF_FIXED_FOV;
	}
	else if( f == &s_quad_drop_box )
	{
		bit = DF_QUAD_DROP;
	}

	if( f )
	{
		if( f->curvalue == 0 )
			flags &= ~bit;
		else
			flags |= bit;
	}

setvalue:
	CVarSystem::SetCVarInt( "dmflags", flags );

	sprintf( dmoptions_statusbar, "dmflags = %d", flags );

}

void Menu::DMOptions_MenuInit( ) {

	int dmflags = CVarSystem::GetCVarInt( "dmflags" );
	int y = 0;

	s_dmoptions_menu.x = ( int )( Video::viddef.width * 0.50f );
	s_dmoptions_menu.nitems = 0;

	s_falls_box.generik.type = MTYPE_SPINCONTROL;
	s_falls_box.generik.x	= 0;
	s_falls_box.generik.y	= y;
	s_falls_box.generik.name	= "falling damage";
	s_falls_box.generik.callback = DMFlagCallback;
	s_falls_box.itemnames = yesno_names;
	s_falls_box.curvalue =( dmflags & DF_NO_FALLING ) == 0;

	s_weapons_stay_box.generik.type = MTYPE_SPINCONTROL;
	s_weapons_stay_box.generik.x	= 0;
	s_weapons_stay_box.generik.y	= y += 10;
	s_weapons_stay_box.generik.name	= "weapons stay";
	s_weapons_stay_box.generik.callback = DMFlagCallback;
	s_weapons_stay_box.itemnames = yesno_names;
	s_weapons_stay_box.curvalue =( dmflags & DF_WEAPONS_STAY ) != 0;

	s_instant_powerups_box.generik.type = MTYPE_SPINCONTROL;
	s_instant_powerups_box.generik.x	= 0;
	s_instant_powerups_box.generik.y	= y += 10;
	s_instant_powerups_box.generik.name	= "instant powerups";
	s_instant_powerups_box.generik.callback = DMFlagCallback;
	s_instant_powerups_box.itemnames = yesno_names;
	s_instant_powerups_box.curvalue =( dmflags & DF_INSTANT_ITEMS ) != 0;

	s_powerups_box.generik.type = MTYPE_SPINCONTROL;
	s_powerups_box.generik.x	= 0;
	s_powerups_box.generik.y	= y += 10;
	s_powerups_box.generik.name	= "allow powerups";
	s_powerups_box.generik.callback = DMFlagCallback;
	s_powerups_box.itemnames = yesno_names;
	s_powerups_box.curvalue =( dmflags & DF_NO_ITEMS ) == 0;

	s_health_box.generik.type = MTYPE_SPINCONTROL;
	s_health_box.generik.x	= 0;
	s_health_box.generik.y	= y += 10;
	s_health_box.generik.callback = DMFlagCallback;
	s_health_box.generik.name	= "allow health";
	s_health_box.itemnames = yesno_names;
	s_health_box.curvalue =( dmflags & DF_NO_HEALTH ) == 0;

	s_armor_box.generik.type = MTYPE_SPINCONTROL;
	s_armor_box.generik.x	= 0;
	s_armor_box.generik.y	= y += 10;
	s_armor_box.generik.name	= "allow armor";
	s_armor_box.generik.callback = DMFlagCallback;
	s_armor_box.itemnames = yesno_names;
	s_armor_box.curvalue =( dmflags & DF_NO_ARMOR ) == 0;

	s_spawn_farthest_box.generik.type = MTYPE_SPINCONTROL;
	s_spawn_farthest_box.generik.x	= 0;
	s_spawn_farthest_box.generik.y	= y += 10;
	s_spawn_farthest_box.generik.name	= "spawn farthest";
	s_spawn_farthest_box.generik.callback = DMFlagCallback;
	s_spawn_farthest_box.itemnames = yesno_names;
	s_spawn_farthest_box.curvalue =( dmflags & DF_SPAWN_FARTHEST ) != 0;

	s_samelevel_box.generik.type = MTYPE_SPINCONTROL;
	s_samelevel_box.generik.x	= 0;
	s_samelevel_box.generik.y	= y += 10;
	s_samelevel_box.generik.name	= "same map";
	s_samelevel_box.generik.callback = DMFlagCallback;
	s_samelevel_box.itemnames = yesno_names;
	s_samelevel_box.curvalue =( dmflags & DF_SAME_LEVEL ) != 0;

	s_force_respawn_box.generik.type = MTYPE_SPINCONTROL;
	s_force_respawn_box.generik.x	= 0;
	s_force_respawn_box.generik.y	= y += 10;
	s_force_respawn_box.generik.name	= "force respawn";
	s_force_respawn_box.generik.callback = DMFlagCallback;
	s_force_respawn_box.itemnames = yesno_names;
	s_force_respawn_box.curvalue =( dmflags & DF_FORCE_RESPAWN ) != 0;

	s_teamplay_box.generik.type = MTYPE_SPINCONTROL;
	s_teamplay_box.generik.x	= 0;
	s_teamplay_box.generik.y	= y += 10;
	s_teamplay_box.generik.name	= "teamplay";
	s_teamplay_box.generik.callback = DMFlagCallback;
	s_teamplay_box.itemnames = teamplay_names;

	s_allow_exit_box.generik.type = MTYPE_SPINCONTROL;
	s_allow_exit_box.generik.x	= 0;
	s_allow_exit_box.generik.y	= y += 10;
	s_allow_exit_box.generik.name	= "allow exit";
	s_allow_exit_box.generik.callback = DMFlagCallback;
	s_allow_exit_box.itemnames = yesno_names;
	s_allow_exit_box.curvalue =( dmflags & DF_ALLOW_EXIT ) != 0;

	s_infinite_ammo_box.generik.type = MTYPE_SPINCONTROL;
	s_infinite_ammo_box.generik.x	= 0;
	s_infinite_ammo_box.generik.y	= y += 10;
	s_infinite_ammo_box.generik.name	= "infinite ammo";
	s_infinite_ammo_box.generik.callback = DMFlagCallback;
	s_infinite_ammo_box.itemnames = yesno_names;
	s_infinite_ammo_box.curvalue =( dmflags & DF_INFINITE_AMMO ) != 0;

	s_fixed_fov_box.generik.type = MTYPE_SPINCONTROL;
	s_fixed_fov_box.generik.x	= 0;
	s_fixed_fov_box.generik.y	= y += 10;
	s_fixed_fov_box.generik.name	= "fixed FOV";
	s_fixed_fov_box.generik.callback = DMFlagCallback;
	s_fixed_fov_box.itemnames = yesno_names;
	s_fixed_fov_box.curvalue =( dmflags & DF_FIXED_FOV ) != 0;

	s_quad_drop_box.generik.type = MTYPE_SPINCONTROL;
	s_quad_drop_box.generik.x	= 0;
	s_quad_drop_box.generik.y	= y += 10;
	s_quad_drop_box.generik.name	= "quad drop";
	s_quad_drop_box.generik.callback = DMFlagCallback;
	s_quad_drop_box.itemnames = yesno_names;
	s_quad_drop_box.curvalue =( dmflags & DF_QUAD_DROP ) != 0;

	s_friendlyfire_box.generik.type = MTYPE_SPINCONTROL;
	s_friendlyfire_box.generik.x	= 0;
	s_friendlyfire_box.generik.y	= y += 10;
	s_friendlyfire_box.generik.name	= "friendly fire";
	s_friendlyfire_box.generik.callback = DMFlagCallback;
	s_friendlyfire_box.itemnames = yesno_names;
	s_friendlyfire_box.curvalue =( dmflags & DF_NO_FRIENDLY_FIRE ) == 0;

	Menu_AddItem( &s_dmoptions_menu, &s_falls_box );
	Menu_AddItem( &s_dmoptions_menu, &s_weapons_stay_box );
	Menu_AddItem( &s_dmoptions_menu, &s_instant_powerups_box );
	Menu_AddItem( &s_dmoptions_menu, &s_powerups_box );
	Menu_AddItem( &s_dmoptions_menu, &s_health_box );
	Menu_AddItem( &s_dmoptions_menu, &s_armor_box );
	Menu_AddItem( &s_dmoptions_menu, &s_spawn_farthest_box );
	Menu_AddItem( &s_dmoptions_menu, &s_samelevel_box );
	Menu_AddItem( &s_dmoptions_menu, &s_force_respawn_box );
	Menu_AddItem( &s_dmoptions_menu, &s_teamplay_box );
	Menu_AddItem( &s_dmoptions_menu, &s_allow_exit_box );
	Menu_AddItem( &s_dmoptions_menu, &s_infinite_ammo_box );
	Menu_AddItem( &s_dmoptions_menu, &s_fixed_fov_box );
	Menu_AddItem( &s_dmoptions_menu, &s_quad_drop_box );
	Menu_AddItem( &s_dmoptions_menu, &s_friendlyfire_box );

	Menu_Center( &s_dmoptions_menu );

	// set the original dmflags statusbar
	DMFlagCallback( 0 );
	Menu_SetStatusBar( &s_dmoptions_menu, dmoptions_statusbar );
}

void Menu::DMOptions_MenuDraw( ) {

	Menu_Draw( &s_dmoptions_menu );
}

const Str Menu::DMOptions_MenuKey( int key ) {

	return Default_MenuKey( &s_dmoptions_menu, key );
}

void Menu::M_Menu_DMOptions_f( ) {

	DMOptions_MenuInit( );
	M_PushMenu( DMOptions_MenuDraw, DMOptions_MenuKey );
}

/*
=============================================================================

DOWNLOADOPTIONS BOOK MENU

=============================================================================
*/

void Menu::DownloadCallback( void * self ) {

	menulist_s * f =( menulist_s * ) self;

	if( f == &s_allow_download_box )				CVarSystem::SetCVarInt( "allow_download", f->curvalue );
	else if( f == &s_allow_download_maps_box )		CVarSystem::SetCVarInt( "allow_download_maps", f->curvalue );
	else if( f == &s_allow_download_models_box )	CVarSystem::SetCVarInt( "allow_download_models", f->curvalue );
	else if( f == &s_allow_download_players_box )	CVarSystem::SetCVarInt( "allow_download_players", f->curvalue );
	else if( f == &s_allow_download_sounds_box )	CVarSystem::SetCVarInt( "allow_download_sounds", f->curvalue );
}

void Menu::DownloadOptions_MenuInit( ) {

	int y = 0;

	s_downloadoptions_menu.x = ( int )( Video::viddef.width * 0.50f );
	s_downloadoptions_menu.nitems = 0;

	s_download_title.generik.type = MTYPE_SEPARATOR;
	s_download_title.generik.name = "Download Options";
	s_download_title.generik.x    = 48;
	s_download_title.generik.y	 = y;

	s_allow_download_box.generik.type = MTYPE_SPINCONTROL;
	s_allow_download_box.generik.x	= 0;
	s_allow_download_box.generik.y	= y += 20;
	s_allow_download_box.generik.name	= "allow downloading";
	s_allow_download_box.generik.callback = DownloadCallback;
	s_allow_download_box.itemnames = yesno_names;
	s_allow_download_box.curvalue = !CVarSystem::GetCVarInt( "allow_download" );

	s_allow_download_maps_box.generik.type = MTYPE_SPINCONTROL;
	s_allow_download_maps_box.generik.x	= 0;
	s_allow_download_maps_box.generik.y	= y += 20;
	s_allow_download_maps_box.generik.name	= "maps";
	s_allow_download_maps_box.generik.callback = DownloadCallback;
	s_allow_download_maps_box.itemnames = yesno_names;
	s_allow_download_maps_box.curvalue = !CVarSystem::GetCVarInt( "allow_download_maps" );

	s_allow_download_players_box.generik.type = MTYPE_SPINCONTROL;
	s_allow_download_players_box.generik.x	= 0;
	s_allow_download_players_box.generik.y	= y += 10;
	s_allow_download_players_box.generik.name	= "player models/skins";
	s_allow_download_players_box.generik.callback = DownloadCallback;
	s_allow_download_players_box.itemnames = yesno_names;
	s_allow_download_players_box.curvalue = !CVarSystem::GetCVarInt( "allow_download_players" );

	s_allow_download_models_box.generik.type = MTYPE_SPINCONTROL;
	s_allow_download_models_box.generik.x	= 0;
	s_allow_download_models_box.generik.y	= y += 10;
	s_allow_download_models_box.generik.name	= "models";
	s_allow_download_models_box.generik.callback = DownloadCallback;
	s_allow_download_models_box.itemnames = yesno_names;
	s_allow_download_models_box.curvalue = !CVarSystem::GetCVarInt( "allow_download_models" );

	s_allow_download_sounds_box.generik.type = MTYPE_SPINCONTROL;
	s_allow_download_sounds_box.generik.x	= 0;
	s_allow_download_sounds_box.generik.y	= y += 10;
	s_allow_download_sounds_box.generik.name	= "sounds";
	s_allow_download_sounds_box.generik.callback = DownloadCallback;
	s_allow_download_sounds_box.itemnames = yesno_names;
	s_allow_download_sounds_box.curvalue = !CVarSystem::GetCVarInt( "allow_download_sounds" );

	Menu_AddItem( &s_downloadoptions_menu, &s_download_title );
	Menu_AddItem( &s_downloadoptions_menu, &s_allow_download_box );
	Menu_AddItem( &s_downloadoptions_menu, &s_allow_download_maps_box );
	Menu_AddItem( &s_downloadoptions_menu, &s_allow_download_players_box );
	Menu_AddItem( &s_downloadoptions_menu, &s_allow_download_models_box );
	Menu_AddItem( &s_downloadoptions_menu, &s_allow_download_sounds_box );

	Menu_Center( &s_downloadoptions_menu );

	// skip over title
	if( s_downloadoptions_menu.cursor == 0 )
		s_downloadoptions_menu.cursor = 1;
}

void Menu::DownloadOptions_MenuDraw( ) {

	Menu_Draw( &s_downloadoptions_menu );
}

const Str Menu::DownloadOptions_MenuKey( int key ) {

	return Default_MenuKey( &s_downloadoptions_menu, key );
}

void Menu::M_Menu_DownloadOptions_f( ) {

	DownloadOptions_MenuInit( );
	M_PushMenu( DownloadOptions_MenuDraw, DownloadOptions_MenuKey );
}
/*
=============================================================================

ADDRESS BOOK MENU

=============================================================================
*/

void Menu::AddressBook_MenuInit( ) {

	int i;

	s_addressbook_menu.x = Video::viddef.width / 2 - 142;
	s_addressbook_menu.y = Video::viddef.height / 2 - 58;
	s_addressbook_menu.nitems = 0;

	for( i = 0; i < NUM_ADDRESSBOOK_ENTRIES; i++ )
	{
		CVar * adr;
		Str buffer;

		sprintf( buffer, "adr%d", i );

		adr = CVarSystem::Find( buffer );

		s_addressbook_fields[ i ].generik.type = MTYPE_FIELD;
		s_addressbook_fields[ i ].generik.name = 0;
		s_addressbook_fields[ i ].generik.callback = 0;
		s_addressbook_fields[ i ].generik.x		= 0;
		s_addressbook_fields[ i ].generik.y		= i * 18 + 0;
		s_addressbook_fields[ i ].generik.localdata[ 0 ] = i;
		s_addressbook_fields[ i ].cursor			= 0;
		s_addressbook_fields[ i ].length			= 60;
		s_addressbook_fields[ i ].visible_length	= 30;

		s_addressbook_fields[ i ].buffer = adr->GetString( );

		Menu_AddItem( &s_addressbook_menu, &s_addressbook_fields[ i ] );
	}
}

const Str Menu::AddressBook_MenuKey( int key ) {

	if( key == VK_ESCAPE )
	{
		int index;
		Str buffer;

		for( index = 0; index < NUM_ADDRESSBOOK_ENTRIES; index++ )
		{
			sprintf( buffer, "adr%d", index );
			CVarSystem::SetCVarStr( buffer, s_addressbook_fields[ index ].buffer );
		}
	}

	return Default_MenuKey( &s_addressbook_menu, key );
}

void Menu::AddressBook_MenuDraw( ) {

	M_Banner( Str( "pics/m_banner_addressbook.pcx" ) );
	Menu_Draw( &s_addressbook_menu );
}

void Menu::M_Menu_AddressBook_f( ) {

	AddressBook_MenuInit( );
	M_PushMenu( AddressBook_MenuDraw, AddressBook_MenuKey );
}

/*
=============================================================================

PLAYER CONFIG MENU

=============================================================================
*/

void Menu::DownloadOptionsFunc( void * self ) {

	M_Menu_DownloadOptions_f( );
}

void Menu::HandednessCallback( void * unused ) {

	CVarSystem::SetCVarInt( "hand", s_player_handedness_box.curvalue );
}

void Menu::RateCallback( void * unused ) {

	if( s_player_rate_box.curvalue != sizeof( rate_tbl ) / sizeof( *rate_tbl ) - 1 ) CVarSystem::SetCVarInt( "rate", rate_tbl[ s_player_rate_box.curvalue ] );
}

void Menu::ModelCallback( void * unused ) {

	s_player_skin_box.itemnames = s_pmi[ s_player_model_box.curvalue ].skindisplaynames;
	s_player_skin_box.curvalue = 0;
}

void Menu::FreeFileList( const Str * list, int n ) {	
}

bool Menu::IconOfSkinExists( const Str & skin, List< Str > & pcxfiles ) {

	Str scratch;

	scratch = skin;
	scratch = scratch.Left( scratch.Find( '.' ) );
	scratch.Append( "_i.pcx" );

	for( int i = 0; i <( pcxfiles.Num( ) - 1 ); i++ ) {

		if( pcxfiles[ i ] == scratch ) return true;
	}

	return false;
}

bool Menu::PlayerConfig_ScanDirectories( ) {

	Str findname;
	Str scratch;
	int npms = 0;
	List< Str > dirnames;
	Str path;
	int i;

	s_numplayermodels = 0;

	/*
	* * get a list of directories
	*/
	do {

		//path = FileSystem::NextPath( path );
		sprintf( findname, "%s/players/* .* ", path.c_str( ) );

		dirnames = FileSystem::ListFiles( findname, SFF_SUBDIR, 0 );

		if( dirnames.Num( ) ) break;

	} while( path.Length( ) );

	if( !dirnames.Num( ) ) return false;

	/*
	* * go through the subdirectories
	*/
	npms = dirnames.Num( );
	if( npms > MAX_PLAYERMODELS ) npms = MAX_PLAYERMODELS;

	for( i = 0; i < npms; i++ ) {

		int k;
		List< Str > pcxnames;
		List< Str > skinnames;

		if( dirnames[ i ] == 0 )
			continue;

		// verify the existence of tris.md2
		scratch = dirnames[ i ];
		scratch.Append( "/tris.md2" );
		if( !System::Sys_FindFirst( scratch, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM ) ) {

			dirnames.Remove( dirnames[ i ] );
			System::Sys_FindClose( );
			continue;
		}
		System::Sys_FindClose( );

		// verify the existence of at least one pcx skin
		scratch = dirnames[ i ];
		scratch.Append( "/* .pcx" );
		pcxnames = FileSystem::ListFiles( scratch, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM );

		if( !pcxnames.Num( ) ) {

			dirnames.Remove( dirnames[ i ] );
			continue;
		}

		// count valid skins, which consist of a skin with a matching "_i" icon
		for( k = 0; k < pcxnames.Num( ) - 1; k++ ) {

			if( pcxnames[ k ] != "_i.pcx" ) {

				if( IconOfSkinExists( pcxnames[ k ], pcxnames ) ) {

					scratch = pcxnames[ k ].Right( pcxnames[ k ].Length( ) -( 1 + Max( pcxnames[ k ].Find( '/' ), pcxnames[ k ].Find( '\\' ) ) ) );

					scratch = scratch.Left( scratch.Find( '.' ) );

					skinnames.Insert( scratch );
				}
			}
		}
		if( !skinnames.Num( ) ) continue;

		// at this point we have a valid player model
		s_pmi[ s_numplayermodels ].skindisplaynames = new Str[ skinnames.Num( ) + 1 ];
		for( int i = 0; i <( skinnames.Num( ) - 1 ); i++ ) s_pmi[ s_numplayermodels ].skindisplaynames[ i ] = skinnames[ i ];

		s_pmi[ s_numplayermodels ].displayname = dirnames[ k ].Right( dirnames[ k ].Length( ) -( 1 + Max( dirnames[ k ].Find( '/' ), dirnames[ k ].Find( '\\' ) ) ) );

		s_pmi[ s_numplayermodels ].directory = dirnames[ k ].Left( Max( dirnames[ k ].Find( '/' ), dirnames[ k ].Find( '\\' ) ) );

		pcxnames.Clear( );

		s_numplayermodels++;
	}

	if( dirnames.Num( ) ) dirnames.Clear( );

	return true;
}

bool Menu::PlayerConfig_MenuInit( ) {

	char currentdirectory[ 1024 ];
	char currentskin[ 1024 ];
	int i = 0;

	int currentdirectoryindex = 0;
	int currentskinindex = 0;

	PlayerConfig_ScanDirectories( );

	if( s_numplayermodels == 0 ) return false;

	strcpy( currentdirectory, CVarSystem::GetCVarStr( "skin" ) );

	if( strchr( currentdirectory, '/' ) )
	{
		strcpy( currentskin, strchr( currentdirectory, '/' ) + 1 );
		* strchr( currentdirectory, '/' ) = 0;
	}
	else if( strchr( currentdirectory, '\\' ) )
	{
		strcpy( currentskin, strchr( currentdirectory, '\\' ) + 1 );
		* strchr( currentdirectory, '\\' ) = 0;
	}
	else
	{
		strcpy( currentdirectory, "male" );
		strcpy( currentskin, "grunt" );
	}

	Common::Com_Memset( s_pmnames, 0, sizeof( s_pmnames ) );
	for( i = 0; i < s_numplayermodels; i++ )
	{
		s_pmnames[ i ] = s_pmi[ i ].displayname;
		if( Q_stricmp( s_pmi[ i ].directory, currentdirectory ) == 0 )
		{
			int j;

			currentdirectoryindex = i;

			for( j = 0; s_pmi[ i ].skindisplaynames[ j ]; j++ )
			{
				if( Q_stricmp( s_pmi[ i ].skindisplaynames[ j ], currentskin ) == 0 )
				{
					currentskinindex = j;
					break;
				}
			}
		}
	}

	s_player_config_menu.x = Video::viddef.width / 2 - 95; 
	s_player_config_menu.y = Video::viddef.height / 2 - 97;
	s_player_config_menu.nitems = 0;

	s_player_name_field.generik.type = MTYPE_FIELD;
	s_player_name_field.generik.name = "name";
	s_player_name_field.generik.callback = 0;
	s_player_name_field.generik.x		= 0;
	s_player_name_field.generik.y		= 0;
	s_player_name_field.length	= 20;
	s_player_name_field.visible_length = 20;
	s_player_name_field.buffer = CVarSystem::GetCVarStr( "name" );
	s_player_name_field.cursor = CVarSystem::GetCVarStr( "name" ).Length( );;

	s_player_model_title.generik.type = MTYPE_SEPARATOR;
	s_player_model_title.generik.name = "model";
	s_player_model_title.generik.x    = -8;
	s_player_model_title.generik.y	 = 60;

	s_player_model_box.generik.type = MTYPE_SPINCONTROL;
	s_player_model_box.generik.x	= -56;
	s_player_model_box.generik.y	= 70;
	s_player_model_box.generik.callback = ModelCallback;
	s_player_model_box.generik.cursor_offset = -48;
	s_player_model_box.curvalue = currentdirectoryindex;
	s_player_model_box.itemnames = s_pmnames;

	s_player_skin_title.generik.type = MTYPE_SEPARATOR;
	s_player_skin_title.generik.name = "skin";
	s_player_skin_title.generik.x    = -16;
	s_player_skin_title.generik.y	 = 84;

	s_player_skin_box.generik.type = MTYPE_SPINCONTROL;
	s_player_skin_box.generik.x	= -56;
	s_player_skin_box.generik.y	= 94;
	s_player_skin_box.generik.name	= 0;
	s_player_skin_box.generik.callback = 0;
	s_player_skin_box.generik.cursor_offset = -48;
	s_player_skin_box.curvalue = currentskinindex;
	s_player_skin_box.itemnames = s_pmi[ currentdirectoryindex ].skindisplaynames;

	s_player_hand_title.generik.type = MTYPE_SEPARATOR;
	s_player_hand_title.generik.name = "handedness";
	s_player_hand_title.generik.x    = 32;
	s_player_hand_title.generik.y	 = 108;

	s_player_handedness_box.generik.type = MTYPE_SPINCONTROL;
	s_player_handedness_box.generik.x	= -56;
	s_player_handedness_box.generik.y	= 118;
	s_player_handedness_box.generik.name	= 0;
	s_player_handedness_box.generik.cursor_offset = -48;
	s_player_handedness_box.generik.callback = HandednessCallback;
	s_player_handedness_box.curvalue = CVarSystem::GetCVarInt( "hand" );
	s_player_handedness_box.itemnames = handedness;

	for( i = 0; i < sizeof( rate_tbl ) / sizeof( *rate_tbl ) - 1; i++ ) {

		if( CVarSystem::GetCVarInt( "rate" ) == rate_tbl[ i ] ) break;
	}

	s_player_rate_title.generik.type = MTYPE_SEPARATOR;
	s_player_rate_title.generik.name = "connect speed";
	s_player_rate_title.generik.x    = 56;
	s_player_rate_title.generik.y	 = 156;

	s_player_rate_box.generik.type = MTYPE_SPINCONTROL;
	s_player_rate_box.generik.x	= -56;
	s_player_rate_box.generik.y	= 166;
	s_player_rate_box.generik.name	= 0;
	s_player_rate_box.generik.cursor_offset = -48;
	s_player_rate_box.generik.callback = RateCallback;
	s_player_rate_box.curvalue = i;
	s_player_rate_box.itemnames = rate_names;

	s_player_download_action.generik.type = MTYPE_ACTION;
	s_player_download_action.generik.name	= "download options";
	s_player_download_action.generik.flags = QMF_LEFT_JUSTIFY;
	s_player_download_action.generik.x	= -24;
	s_player_download_action.generik.y	= 186;
	s_player_download_action.generik.statusbar = NULL;
	s_player_download_action.generik.callback = DownloadOptionsFunc;

	Menu_AddItem( &s_player_config_menu, &s_player_name_field );
	Menu_AddItem( &s_player_config_menu, &s_player_model_title );
	Menu_AddItem( &s_player_config_menu, &s_player_model_box );
	if( s_player_skin_box.itemnames )
	{
		Menu_AddItem( &s_player_config_menu, &s_player_skin_title );
		Menu_AddItem( &s_player_config_menu, &s_player_skin_box );
	}
	Menu_AddItem( &s_player_config_menu, &s_player_hand_title );
	Menu_AddItem( &s_player_config_menu, &s_player_handedness_box );
	Menu_AddItem( &s_player_config_menu, &s_player_rate_title );
	Menu_AddItem( &s_player_config_menu, &s_player_rate_box );
	Menu_AddItem( &s_player_config_menu, &s_player_download_action );

	return true;
}

void Menu::PlayerConfig_MenuDraw( ) {

#if 0

	refdef_t refdef;
	Str scratch;

	Common::Com_Memset( &refdef, 0, sizeof( refdef ) );

	refdef.x = Video::viddef.width / 2;
	refdef.y = Video::viddef.height / 2 - 72;
	refdef.width = 144;
	refdef.height = 168;
	refdef.fov_x = 40;
	refdef.fov_y = View::CalcFov( refdef.fov_x, refdef.width, refdef.height );
	refdef.time = Client::cls.realtime* 0.001f;

	if( s_pmi[ s_player_model_box.curvalue ].skindisplaynames )
	{
		static int			yaw;
		int maxframe = 29;
		SceneEntity entity;

		Common::Com_Memset( &entity, 0, sizeof( entity ) );

		sprintf( scratch, "players/%s/tris.md2", s_pmi[ s_player_model_box.curvalue ].directory.c_str( ) );
		entity.model = Renderer::RegisterModel( scratch );
		sprintf( scratch, "players/%s/%s.pcx", s_pmi[ s_player_model_box.curvalue ].directory.c_str( ), s_pmi[ s_player_model_box.curvalue].skindisplaynames[s_player_skin_box.curvalue ].c_str( ) );
		entity.skin = MaterialSystem::FindTexture( scratch, "model" );
		entity.flags = RF_FULLBRIGHT;
		entity.origin = Vec3( 80.0f, 0.0f, 0.0f );
		entity.oldorigin = entity.origin;
		entity.frame = 0;
		entity.oldframe = 0;
		entity.backlerp = 0.0f;
		entity.angles[ 1 ] = yaw++;
		if( ++yaw > 360.0f ) yaw -= 360.0f;

		refdef.areabits = 0;
		refdef.num_entities = 1;
		refdef.entities = &entity;
		refdef.lightstyles = 0;
		refdef.rdflags = RDF_NOWORLDMODEL;

		Menu_Draw( &s_player_config_menu );

		M_DrawTextBox( ( refdef.x ) * ( 320.0f / Video::viddef.width ) - 8, ( Video::viddef.height / 2 ) * ( 240.0f / Video::viddef.height ) - 77, refdef.width / 8, refdef.height / 8 );
		refdef.height += 4;

		Renderer::RenderFrame( &refdef );

		sprintf( scratch, "/players/%s/%s_i.pcx", s_pmi[ s_player_model_box.curvalue ].directory, s_pmi[ s_player_model_box.curvalue].skindisplaynames[s_player_skin_box.curvalue ] );
		Renderer::DrawPic( s_player_config_menu.x - 40, refdef.y, scratch );
	}

	#endif
}

const Str Menu::PlayerConfig_MenuKey( int key ) {

	int i;

	if( key == VK_ESCAPE )
	{
		Str scratch;

		CVarSystem::SetCVarStr( "name", s_player_name_field.buffer );

		sprintf( scratch, "%s/%s", s_pmi[ s_player_model_box.curvalue ].directory.c_str( ), s_pmi[ s_player_model_box.curvalue].skindisplaynames[ s_player_skin_box.curvalue ].c_str( ) );

		CVarSystem::SetCVarStr( "skin", scratch );

		for( i = 0; i < s_numplayermodels; i++ )
		{
			int j;

			for( j = 0; s_pmi[ i ].skindisplaynames[ j ]; j++ )
			{
				if( s_pmi[ i ].skindisplaynames[ j ] ) s_pmi[ i ].skindisplaynames[ j ].Clear( );
			}
		}
	}

	return Default_MenuKey( &s_player_config_menu, key );
}


void Menu::M_Menu_PlayerConfig_f( ) {

	if( !PlayerConfig_MenuInit( ) ) {

		Menu_SetStatusBar( &s_multiplayer_menu, Str( "No valid player models found" ) );
		return;
	}

	Menu_SetStatusBar( &s_multiplayer_menu, Str( "" ) );
	M_PushMenu( PlayerConfig_MenuDraw, PlayerConfig_MenuKey );
}

/*
=======================================================================

QUIT MENU

=======================================================================
*/

const Str Menu::M_Quit_Key( int key ) {

	switch( key ) {

	case VK_ESCAPE:
	case 'n':
	case 'N':

		M_PopMenu( );
		break;

	case 'Y':
	case 'y':

		Client::cls.key_dest = key_console;
		Client::CL_Quit_f( );
		break;

	default:

		break;
	}

	return NULL;

}

void Menu::M_Quit_Draw( ) {

}

void Menu::M_Menu_Quit_f( ) {

	M_PushMenu( M_Quit_Draw, M_Quit_Key );
}



//=============================================================================
/* Menu Subsystem */


/*
=================
M_Init
=================
*/
void Menu::M_Init( ) {

	Command::Cmd_AddCommand( Str( "menu_main" ), M_Menu_Main_f );
	Command::Cmd_AddCommand( Str( "menu_game" ), M_Menu_Game_f );
	Command::Cmd_AddCommand( Str( "menu_loadgame" ), M_Menu_LoadGame_f );
	Command::Cmd_AddCommand( Str( "menu_savegame" ), M_Menu_SaveGame_f );
	Command::Cmd_AddCommand( Str( "menu_joinserver" ), M_Menu_JoinServer_f );
	Command::Cmd_AddCommand( Str( "menu_addressbook" ), M_Menu_AddressBook_f );
	Command::Cmd_AddCommand( Str( "menu_startserver" ), M_Menu_StartServer_f );
	Command::Cmd_AddCommand( Str( "menu_dmoptions" ), M_Menu_DMOptions_f );
	Command::Cmd_AddCommand( Str( "menu_playerconfig" ), M_Menu_PlayerConfig_f );
	Command::Cmd_AddCommand( Str( "menu_downloadoptions" ), M_Menu_DownloadOptions_f );
	Command::Cmd_AddCommand( Str( "menu_credits" ), M_Menu_Credits_f );
	Command::Cmd_AddCommand( Str( "menu_multiplayer" ), M_Menu_Multiplayer_f );
	Command::Cmd_AddCommand( Str( "menu_video" ), M_Menu_Video_f );
	Command::Cmd_AddCommand( Str( "menu_options" ), M_Menu_Options_f );
	Command::Cmd_AddCommand( Str( "menu_keys" ), M_Menu_Keys_f );
	Command::Cmd_AddCommand( Str( "menu_quit" ), M_Menu_Quit_f );
}


/*
=================
M_Draw
=================
*/
void Menu::M_Draw( ) {

	if( Client::cls.key_dest != key_menu ) return;

	// repaint everything next frame
	Screen::SCR_DirtyScreen( );

	// dim everything behind it down
#if 0

	if( Client::cl.cinematictime > 0 ) Renderer::DrawFill( 0, 0, Video::viddef.width, Video::viddef.height, 0 );
	else Renderer::DrawFadeScreen( );

#endif


	m_drawfunc( );

	// delay playing the enter sound until after the
	// menu has been drawn, to avoid delay while
	// caching images
	if( m_entersound ) {

		Sound::S_StartLocalSound( menu_in_sound );
		m_entersound = false;
	}
}


/*
=================
M_Keydown
=================
*/
void Menu::M_Keydown( int key ) {

	Str s;

	if( m_keyfunc ) {

		s = m_keyfunc( key );
		if( s.Length( ) ) Sound::S_StartLocalSound( s );
	}
}
