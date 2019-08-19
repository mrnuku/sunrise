#include "precompiled.h"
#pragma hdrstop

/*
====================================================================

MENU INTERACTION

====================================================================
*/
#define SOFTWARE_MENU 0 // FIXME LATER destroy this
#define OPENGL_MENU   1

menuframework_s * Video::s_current_menu;
int				Video::s_current_menu_index;


void Video::ScreenSizeCallback( void * s ) {

	menuslider_s * slider =( menuslider_s * ) s;

	CVarSystem::SetCVarInt( "viewsize", ( int )slider->curvalue * 10 );
}

void Video::BrightnessCallback( void * s ) {

	menuslider_s * slider =( menuslider_s * ) s;

	if( s_current_menu_index == SOFTWARE_MENU )
		s_brightness_slider[ 1 ].curvalue = s_brightness_slider[ 0 ].curvalue;
	else
		s_brightness_slider[ 0 ].curvalue = s_brightness_slider[ 1 ].curvalue;

}

void Video::ResetDefaults( void * unused ) {

	VID_MenuInit( );
}

void Video::ApplyChanges( void * unused ) {

	float gamma;

	/*
	* * make values consistent
	*/
	s_fs_box[ !s_current_menu_index ].curvalue = s_fs_box[ s_current_menu_index ].curvalue;
	s_brightness_slider[ !s_current_menu_index ].curvalue = s_brightness_slider[ s_current_menu_index ].curvalue;

	/*
	* * invert sense so greater = brighter, and scale to a range of 0.5 to 1.3
	*/
	gamma =( 0.8f -( s_brightness_slider[ s_current_menu_index ].curvalue/10.0f - 0.5f ) ) + 0.5f;

	Common::vid_gamma.SetFloat( gamma );
	CVarSystem::SetCVarInt( "gl_picmip", 3 - ( int )s_tq_slider.curvalue );
	Common::vid_fullscreen.SetInt( s_fs_box[ s_current_menu_index ].curvalue );
	CVarSystem::SetCVarInt( "gl_finish", s_finish_box.curvalue );
	CVarSystem::SetCVarInt( "gl_mode", s_mode_list[ OPENGL_MENU ].curvalue );

	Menu::M_ForceMenuOff( );
}

void Video::CancelChanges( void * unused ) {

	Menu::M_PopMenu( );
}

/*
* * VID_MenuInit
*/
void Video::VID_MenuInit( ) {

	int i;

	s_mode_list[ OPENGL_MENU ].curvalue = CVarSystem::GetCVarInt( "gl_mode" );

	s_screensize_slider[ OPENGL_MENU ].curvalue =  CVarSystem::GetCVarFloat( "scr_viewsize" ) / 10.0f;

	s_software_menu.x = Video::viddef.width / 2;
	s_software_menu.nitems = 0;
	s_opengl_menu.x = Video::viddef.width / 2;
	s_opengl_menu.nitems = 0;

	for( i = 0; i < 2; i++ )
	{
		s_mode_list[ i ].generik.type = MTYPE_SPINCONTROL;
		s_mode_list[ i ].generik.name = "video mode";
		s_mode_list[ i ].generik.x = 0;
		s_mode_list[ i ].generik.y = 10;
		s_mode_list[ i ].itemnames = resolutions;

		s_screensize_slider[ i ].generik.type	= MTYPE_SLIDER;
		s_screensize_slider[ i ].generik.x		= 0;
		s_screensize_slider[ i ].generik.y		= 20;
		s_screensize_slider[ i ].generik.name	= "screen size";
		s_screensize_slider[ i ].minvalue = 3;
		s_screensize_slider[ i ].maxvalue = 12;
		s_screensize_slider[ i ].generik.callback = ScreenSizeCallback;

		s_brightness_slider[ i ].generik.type	= MTYPE_SLIDER;
		s_brightness_slider[ i ].generik.x	= 0;
		s_brightness_slider[ i ].generik.y	= 30;
		s_brightness_slider[ i ].generik.name	= "brightness";
		s_brightness_slider[ i ].generik.callback = BrightnessCallback;
		s_brightness_slider[ i ].minvalue = 5;
		s_brightness_slider[ i ].maxvalue = 13;
		s_brightness_slider[ i ].curvalue =( 1.3f - Common::vid_gamma.GetFloat( ) + 0.5f ) * 10;

		s_fs_box[ i ].generik.type = MTYPE_SPINCONTROL;
		s_fs_box[ i ].generik.x	= 0;
		s_fs_box[ i ].generik.y	= 40;
		s_fs_box[ i ].generik.name	= "fullscreen";
		s_fs_box[ i ].itemnames = Menu::yesno_names;
		s_fs_box[ i ].curvalue = Common::vid_fullscreen.GetInt( );

		s_defaults_action[ i ].generik.type = MTYPE_ACTION;
		s_defaults_action[ i ].generik.name = "reset to defaults";
		s_defaults_action[ i ].generik.x    = 0;
		s_defaults_action[ i ].generik.y    = 90;
		s_defaults_action[ i ].generik.callback = ResetDefaults;

		s_cancel_action[ i ].generik.type = MTYPE_ACTION;
		s_cancel_action[ i ].generik.name = "cancel";
		s_cancel_action[ i ].generik.x    = 0;
		s_cancel_action[ i ].generik.y    = 100;
		s_cancel_action[ i ].generik.callback = CancelChanges;
	}

	s_tq_slider.generik.type	= MTYPE_SLIDER;
	s_tq_slider.generik.x		= 0;
	s_tq_slider.generik.y		= 60;
	s_tq_slider.generik.name	= "texture quality";
	s_tq_slider.minvalue = 0;
	s_tq_slider.maxvalue = 3;
	s_tq_slider.curvalue = 3.0f - CVarSystem::GetCVarFloat( "gl_picmip" );

	s_finish_box.generik.type = MTYPE_SPINCONTROL;
	s_finish_box.generik.x	= 0;
	s_finish_box.generik.y	= 80;
	s_finish_box.generik.name	= "sync every frame";
	s_finish_box.curvalue = CVarSystem::GetCVarInt( "gl_finish" );
	s_finish_box.itemnames = Menu::yesno_names;

	Menu::Menu_AddItem( &s_software_menu, ( void * ) &s_mode_list[ SOFTWARE_MENU ] );
	Menu::Menu_AddItem( &s_software_menu, ( void * ) &s_screensize_slider[ SOFTWARE_MENU ] );
	Menu::Menu_AddItem( &s_software_menu, ( void * ) &s_brightness_slider[ SOFTWARE_MENU ] );
	Menu::Menu_AddItem( &s_software_menu, ( void * ) &s_fs_box[ SOFTWARE_MENU ] );
	Menu::Menu_AddItem( &s_software_menu, ( void * ) &s_stipple_box );

	Menu::Menu_AddItem( &s_opengl_menu, ( void * ) &s_mode_list[ OPENGL_MENU ] );
	Menu::Menu_AddItem( &s_opengl_menu, ( void * ) &s_screensize_slider[ OPENGL_MENU ] );
	Menu::Menu_AddItem( &s_opengl_menu, ( void * ) &s_brightness_slider[ OPENGL_MENU ] );
	Menu::Menu_AddItem( &s_opengl_menu, ( void * ) &s_fs_box[ OPENGL_MENU ] );
	Menu::Menu_AddItem( &s_opengl_menu, ( void * ) &s_tq_slider );
	Menu::Menu_AddItem( &s_opengl_menu, ( void * ) &s_finish_box );

	Menu::Menu_AddItem( &s_software_menu, ( void * ) &s_defaults_action[ SOFTWARE_MENU ] );
	Menu::Menu_AddItem( &s_software_menu, ( void * ) &s_cancel_action[ SOFTWARE_MENU ] );
	Menu::Menu_AddItem( &s_opengl_menu, ( void * ) &s_defaults_action[ OPENGL_MENU ] );
	Menu::Menu_AddItem( &s_opengl_menu, ( void * ) &s_cancel_action[ OPENGL_MENU ] );

	Menu::Menu_Center( &s_software_menu );
	Menu::Menu_Center( &s_opengl_menu );
	s_opengl_menu.x -= 8;
	s_software_menu.x -= 8;
}

/*
================
VID_MenuDraw
================
*/
void Video::VID_MenuDraw( ) {

	if( s_current_menu_index == 0 )
		s_current_menu = &s_software_menu;
	else
		s_current_menu = &s_opengl_menu;

	/*
	* * draw the banner
	*/
	//Renderer::DrawGetPicSize( &w, &h, Str( "pics/m_banner_video.pcx" ) );
	//Renderer::DrawPic( Video::viddef.width / 2 - w / 2, Video::viddef.height /2 - 110, Str( "pics/m_banner_video.pcx" ) );

	/*
	* * move cursor to a reasonable starting position
	*/
	Menu::Menu_AdjustCursor( s_current_menu, 1 );

	/*
	* * draw the menu
	*/
	Menu::Menu_Draw( s_current_menu );
}

/*
================
VID_MenuKey
================
*/
const Str Video::VID_MenuKey( int key ) {

	menuframework_s * m = s_current_menu;

	switch( key ) {

	case VK_ESCAPE:

		ApplyChanges( 0 );
		return NULL;

	case VK_NUMPAD8:
	case VK_UP:

		m->cursor--;
		Menu::Menu_AdjustCursor( m, -1 );
		break;

	case VK_NUMPAD2:
	case VK_DOWN:

		m->cursor++;
		Menu::Menu_AdjustCursor( m, 1 );
		break;

	case VK_NUMPAD4:
	case VK_LEFT:

		Menu::Menu_SlideItem( m, -1 );
		break;

	case VK_NUMPAD6:
	case VK_RIGHT:

		Menu::Menu_SlideItem( m, 1 );
		break;

	case VK_RETURN:

		if( !Menu::Menu_SelectItem( m ) ) ApplyChanges( NULL );
		break;

	}

	return "misc/menu1.wav";
}


