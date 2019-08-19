#include "precompiled.h"
#pragma hdrstop

#define RCOLUMN_OFFSET  16
#define LCOLUMN_OFFSET -16

void Menu::Action_DoEnter( menuaction_s * a ) {

	if( a->generik.callback )
		a->generik.callback( a );
}

void Menu::Action_Draw( menuaction_s * a ) {

	if( a->generik.flags & QMF_LEFT_JUSTIFY )
	{
		if( a->generik.flags & QMF_GRAYED )
			Menu_DrawStringDark( a->generik.x + a->generik.parent->x + LCOLUMN_OFFSET, a->generik.y + a->generik.parent->y, a->generik.name );
		else
			Menu_DrawString( a->generik.x + a->generik.parent->x + LCOLUMN_OFFSET, a->generik.y + a->generik.parent->y, a->generik.name );
	}
	else
	{
		if( a->generik.flags & QMF_GRAYED )
			Menu_DrawStringR2LDark( a->generik.x + a->generik.parent->x + LCOLUMN_OFFSET, a->generik.y + a->generik.parent->y, a->generik.name );
		else
			Menu_DrawStringR2L( a->generik.x + a->generik.parent->x + LCOLUMN_OFFSET, a->generik.y + a->generik.parent->y, a->generik.name );
	}
	if( a->generik.ownerdraw )
		a->generik.ownerdraw( a );
}

bool Menu::Field_DoEnter( menufield_s * f ) {

	if( f->generik.callback )
	{
		f->generik.callback( f );
		return true;
	}
	return false;
}

void Menu::Field_Draw( menufield_s * f ) {

#if 0
	int i;
	char tempbuffer[ 128 ]="";

	if( f->generik.name )
		Menu_DrawStringR2LDark( f->generik.x + f->generik.parent->x + LCOLUMN_OFFSET, f->generik.y + f->generik.parent->y, f->generik.name );

	strncpy( tempbuffer, f->buffer + f->visible_offset, f->visible_length );

	Renderer::DrawChar( f->generik.x + f->generik.parent->x + 16, f->generik.y + f->generik.parent->y - 4, 18 );
	Renderer::DrawChar( f->generik.x + f->generik.parent->x + 16, f->generik.y + f->generik.parent->y + 4, 24 );

	Renderer::DrawChar( f->generik.x + f->generik.parent->x + 24 + f->visible_length * 8, f->generik.y + f->generik.parent->y - 4, 20 );
	Renderer::DrawChar( f->generik.x + f->generik.parent->x + 24 + f->visible_length * 8, f->generik.y + f->generik.parent->y + 4, 26 );

	for( i = 0; i < f->visible_length; i++ )
	{
		Renderer::DrawChar( f->generik.x + f->generik.parent->x + 24 + i * 8, f->generik.y + f->generik.parent->y - 4, 19 );
		Renderer::DrawChar( f->generik.x + f->generik.parent->x + 24 + i * 8, f->generik.y + f->generik.parent->y + 4, 25 );
	}

	Menu_DrawString( f->generik.x + f->generik.parent->x + 24, f->generik.y + f->generik.parent->y, tempbuffer );

	if( Menu_ItemAtCursor( f->generik.parent ) == f )
	{
		int offset;

		if( f->visible_offset )
			offset = f->visible_length;
		else
			offset = f->cursor;

		if( ( ( int )( System::Sys_Milliseconds( ) / 250 ) ) & 1 )
		{
			Renderer::DrawChar( f->generik.x + f->generik.parent->x +( offset + 2 ) * 8 + 8, f->generik.y + f->generik.parent->y, 11 );
		}
		else
		{
			Renderer::DrawChar( f->generik.x + f->generik.parent->x +( offset + 2 ) * 8 + 8, f->generik.y + f->generik.parent->y, ' ' );
		}
	}

#endif
}

bool Menu::Field_Key( menufield_s * f, int key ) {

	/*
	* * support pasting from the clipboard
	*/
	if( toupper( key ) == 'V' && Keyboard::Key_KeyIsDown( VK_CONTROL ) ) {

		f->buffer = System::Sys_GetClipboardData( );
		
		if( f->buffer.Length( ) ) {

			f->cursor = f->buffer.Length( );
			f->visible_offset = f->cursor - f->visible_length;
			if( f->visible_offset < 0 ) f->visible_offset = 0;

		}
		return true;
	}

	switch( key )
	{
	case VK_NUMPAD4:
	case VK_LEFT:
	case VK_BACK:
		if( f->cursor > 0 )
		{
			memmove( &f->buffer[ f->cursor-1 ], &f->buffer[ f->cursor ], strlen( &f->buffer[ f->cursor ] ) + 1 );
			f->cursor--;

			if( f->visible_offset )
			{
				f->visible_offset--;
			}
		}
		break;

	case VK_DECIMAL:
	case VK_DELETE:
		memmove( &f->buffer[ f->cursor ], &f->buffer[ f->cursor+1 ], strlen( &f->buffer[ f->cursor+1 ] ) + 1 );
		break;

	case VK_RETURN:
	case VK_ESCAPE:
	case VK_TAB:
		return false;

	case VK_SPACE:
	default:
		if( !isdigit( key ) &&( f->generik.flags & QMF_NUMBERSONLY ) )
			return false;

		if( f->cursor < f->length )
		{
			f->buffer[ f->cursor++ ] = key;
			f->buffer[ f->cursor ] = 0;

			if( f->cursor > f->visible_length )
			{
				f->visible_offset++;
			}
		}
	}

	return true;
}

void Menu::Menu_AddItem( menuframework_s * menu, void * item ) {

	if( menu->nitems == 0 )
		menu->nslots = 0;

	if( menu->nitems < MAXMENUITEMS )
	{
		menu->items[ menu->nitems ] = item;
		( ( menucommon_s * ) menu->items[ menu->nitems ] )->parent = menu;
		menu->nitems++;
	}

	menu->nslots = Menu_TallySlots( menu );
}

/*
* * Menu_AdjustCursor
* *
* * This function takes the given menu, the direction, and attempts
* * to adjust the menu's cursor so that it's at the next available
* * slot.
*/
void Menu::Menu_AdjustCursor( menuframework_s * m, int dir ) {

	menucommon_s * citem;

	/*
	* * see if it's in a valid spot
	*/
	if( m->cursor >= 0 && m->cursor < m->nitems )
	{
		if( ( citem =( menucommon_s * )Menu_ItemAtCursor( m ) ) != 0 )
		{
			if( citem->type != MTYPE_SEPARATOR )
				return;
		}
	}

	/*
	* * it's not in a valid spot, so crawl in the direction indicated until we
	* * find a valid spot
	*/
	if( dir == 1 )
	{
		while( 1 )
		{
			citem =( menucommon_s * )Menu_ItemAtCursor( m );
			if( citem )
				if( citem->type != MTYPE_SEPARATOR )
					break;
			m->cursor += dir;
			if( m->cursor >= m->nitems )
				m->cursor = 0;
		}
	}
	else
	{
		while( 1 )
		{
			citem =( menucommon_s * )Menu_ItemAtCursor( m );
			if( citem )
				if( citem->type != MTYPE_SEPARATOR )
					break;
			m->cursor += dir;
			if( m->cursor < 0 )
				m->cursor = m->nitems - 1;
		}
	}
}

void Menu::Menu_Center( menuframework_s * menu ) {

	int height;

	height =( ( menucommon_s * ) menu->items[ menu->nitems-1 ] )->y;
	height += 10;

	menu->y =( Video::viddef.height - height ) / 2;
}

void Menu::Menu_Draw( menuframework_s * menu ) {

	int i;
	menucommon_s * item;

	/*
	* * draw contents
	*/
	for( i = 0; i < menu->nitems; i++ )
	{
		switch( ( ( menucommon_s * ) menu->items[ i ] )->type )
		{
		case MTYPE_FIELD:
			Field_Draw( ( menufield_s * ) menu->items[ i ] );
			break;
		case MTYPE_SLIDER:
			Slider_Draw( ( menuslider_s * ) menu->items[ i ] );
			break;
		case MTYPE_LIST:
			MenuList_Draw( ( menulist_s * ) menu->items[ i ] );
			break;
		case MTYPE_SPINCONTROL:
			SpinControl_Draw( ( menulist_s * ) menu->items[ i ] );
			break;
		case MTYPE_ACTION:
			Action_Draw( ( menuaction_s * ) menu->items[ i ] );
			break;
		case MTYPE_SEPARATOR:
			Separator_Draw( ( menuseparator_s * ) menu->items[ i ] );
			break;
		}
	}

	item =( menucommon_s * )Menu_ItemAtCursor( menu );

	if( item && item->cursordraw )
	{
		item->cursordraw( item );
	}
	else if( menu->cursordraw )
	{
		menu->cursordraw( menu );
	}
	else if( item && item->type != MTYPE_FIELD )
	{
#if 0
		if( item->flags & QMF_LEFT_JUSTIFY )
		{
			Renderer::DrawChar( menu->x + item->x - 24 + item->cursor_offset, menu->y + item->y, 12 +( ( int )( System::Sys_Milliseconds( )/250 ) & 1 ) );
		}
		else
		{
			Renderer::DrawChar( menu->x + item->cursor_offset, menu->y + item->y, 12 +( ( int )( System::Sys_Milliseconds( )/250 ) & 1 ) );
		}
#endif
	}

	if( item )
	{
		if( item->statusbarfunc )
			item->statusbarfunc( ( void * ) item );
		else if( item->statusbar )
			Menu_DrawStatusBar( item->statusbar );
		else
			Menu_DrawStatusBar( menu->statusbar );

	}
	else
	{
		Menu_DrawStatusBar( menu->statusbar );
	}
}

void Menu::Menu_DrawStatusBar( const Str & str ) {

#if 0

	if( str.Length( ) ) {

		int l = str.Length( );
		int maxrow = Video::viddef.height / 8;
		int maxcol = Video::viddef.width / 8;
		int col = maxcol / 2 - l / 2;

		Renderer::DrawFill( 0, Video::viddef.height-8, Video::viddef.width, 8, 4 );
		Menu_DrawString( col* 8, Video::viddef.height - 8, str );

	} else Renderer::DrawFill( 0, Video::viddef.height-8, Video::viddef.width, 8, 0 );

#endif

}

void Menu::Menu_DrawString( int x, int y, const Str & str ) {

#if 0


	int len = str.Length( );
	for( int i = 0; i < len; i++ ) Renderer::DrawChar( ( x + i * 8 ), y, str[ i ] );

#endif

}

void Menu::Menu_DrawStringDark( int x, int y, const Str & str ) {
	
#if 0


	int len = str.Length( );
	for( int i = 0; i < len; i++ ) Renderer::DrawChar( ( x + i * 8 ), y, str[ i ] + 128 );

#endif

}

void Menu::Menu_DrawStringR2L( int x, int y, const Str & str ) {
	
#if 0


	int len = str.Length( );
	for( int i = 0; i < len; i++ ) Renderer::DrawChar( ( x - i * 8 ), y, str[ len - i - 1 ] );

#endif

}

void Menu::Menu_DrawStringR2LDark( int x, int y, const Str & str ) {
	
#if 0


	int len = str.Length( );
	for( int i = 0; i < len; i++ ) Renderer::DrawChar( ( x - i * 8 ), y, str[ len - i - 1 ] + 128 );

#endif

}

void * Menu::Menu_ItemAtCursor( menuframework_s * m ) {

	if( m->cursor < 0 || m->cursor >= m->nitems ) return 0;

	return m->items[ m->cursor ];
}

bool Menu::Menu_SelectItem( menuframework_s * s ) {

	menucommon_s * item =( menucommon_s * ) Menu_ItemAtCursor( s );

	if( item ) {

		switch( item->type )
		{
		case MTYPE_FIELD:
			return Field_DoEnter( ( menufield_s * ) item );
		case MTYPE_ACTION:
			Action_DoEnter( ( menuaction_s * ) item );
			return true;
		case MTYPE_LIST:
//			Menulist_DoEnter( ( menulist_s * ) item );
			return false;
		case MTYPE_SPINCONTROL:
//			SpinControl_DoEnter( ( menulist_s * ) item );
			return false;
		}
	}
	return false;
}

void Menu::Menu_SetStatusBar( menuframework_s * m, const Str & string ) {

	m->statusbar = string;
}

void Menu::Menu_SlideItem( menuframework_s * s, int dir ) {

	menucommon_s * item =( menucommon_s * ) Menu_ItemAtCursor( s );

	if( item ) {

		switch( item->type ) {

		case MTYPE_SLIDER:
			Slider_DoSlide( ( menuslider_s * ) item, dir );
			break;

		case MTYPE_SPINCONTROL:
			SpinControl_DoSlide( ( menulist_s * ) item, dir );
			break;
		}
	}
}

int Menu::Menu_TallySlots( menuframework_s * menu ) {

	int i;
	int total = 0;

	for( i = 0; i < menu->nitems; i++ ) {

		if( ( ( menucommon_s * ) menu->items[ i ] )->type == MTYPE_LIST ) {

			int nitems = 0;
			const Str * n =( ( menulist_s * ) menu->items[ i ] )->itemnames;

			while( *n ) nitems++, n++;

			total += nitems;

		} else total++;
	}

	return total;
}

void Menu::Menulist_DoEnter( menulist_s * l ) {

	int start;

	start = l->generik.y / 10 + 1;

	l->curvalue = l->generik.parent->cursor - start;

	if( l->generik.callback ) l->generik.callback( l );
}

void Menu::MenuList_Draw( menulist_s * l ) {

	const Str * n;
	int y = 0;

	Menu_DrawStringR2LDark( l->generik.x + l->generik.parent->x + LCOLUMN_OFFSET, l->generik.y + l->generik.parent->y, l->generik.name );

	n = l->itemnames;

#if 0

  	Renderer::DrawFill( l->generik.x - 112 + l->generik.parent->x, l->generik.parent->y + l->generik.y + l->curvalue* 10 + 10, 128, 10, 16 );

#endif


	while( *n ) {

		Menu_DrawStringR2LDark( l->generik.x + l->generik.parent->x + LCOLUMN_OFFSET, l->generik.y + l->generik.parent->y + y + 10, * n );

		n++;
		y += 10;
	}
}

void Menu::Separator_Draw( menuseparator_s * s ) {

	if( s->generik.name.Length( ) ) Menu_DrawStringR2LDark( s->generik.x + s->generik.parent->x, s->generik.y + s->generik.parent->y, s->generik.name );
}

void Menu::Slider_DoSlide( menuslider_s * s, int dir ) {

	s->curvalue += dir;

	if( s->curvalue > s->maxvalue ) s->curvalue = s->maxvalue;
	else if( s->curvalue < s->minvalue ) s->curvalue = s->minvalue;

	if( s->generik.callback ) s->generik.callback( s );
}

#define SLIDER_RANGE 10

void Menu::Slider_Draw( menuslider_s * s ) {

#if 0

	Renderer::DrawChar( s->generik.x + s->generik.parent->x + RCOLUMN_OFFSET, s->generik.y + s->generik.parent->y, 128 );

	int i;
	for( i = 0; i < SLIDER_RANGE; i++ ) Renderer::DrawChar( RCOLUMN_OFFSET + s->generik.x + i* 8 + s->generik.parent->x + 8, s->generik.y + s->generik.parent->y, 129 );

	Renderer::DrawChar( RCOLUMN_OFFSET + s->generik.x + i* 8 + s->generik.parent->x + 8, s->generik.y + s->generik.parent->y, 130 );
	Renderer::DrawChar( ( int )( 8 + RCOLUMN_OFFSET + s->generik.parent->x + s->generik.x +( SLIDER_RANGE - 1 ) * 8 * s->range ), s->generik.y + s->generik.parent->y, 131 );

#endif

}

void Menu::SpinControl_DoEnter( menulist_s * s ) {

	s->curvalue++;
	if( s->itemnames[ s->curvalue ].IsEmpty( ) ) s->curvalue = 0;

	if( s->generik.callback ) s->generik.callback( s );
}

void Menu::SpinControl_DoSlide( menulist_s * s, int dir ) {

	s->curvalue += dir;

	if( s->curvalue < 0 ) s->curvalue = 0;
	else if( s->itemnames[ s->curvalue ].IsEmpty( ) ) s->curvalue--;

	if( s->generik.callback ) s->generik.callback( s );
}

void Menu::SpinControl_Draw( menulist_s * s ) {

	if( s->generik.name.Length( ) ) {

		Menu_DrawStringR2LDark( s->generik.x + s->generik.parent->x + LCOLUMN_OFFSET, s->generik.y + s->generik.parent->y, s->generik.name );
	}

	if( s->itemnames[ s->curvalue ].Find( '\n' ) ) {

		Str str = s->itemnames[ s->curvalue ].Left( s->itemnames[ s->curvalue ].Find( '\n' ) );
		Menu_DrawString( RCOLUMN_OFFSET + s->generik.x + s->generik.parent->x, s->generik.y + s->generik.parent->y, str );
		str = s->itemnames[ s->curvalue ].Right( s->itemnames[ s->curvalue ].Length( ) - s->itemnames[ s->curvalue ].Find( '\n' ) );
		Menu_DrawString( RCOLUMN_OFFSET + s->generik.x + s->generik.parent->x, s->generik.y + s->generik.parent->y + 10, str );

	} else Menu_DrawString( RCOLUMN_OFFSET + s->generik.x + s->generik.parent->x, s->generik.y + s->generik.parent->y, s->itemnames[ s->curvalue ] );
}

