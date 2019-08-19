#include "precompiled.h"
#pragma hdrstop

/*

================

CL_ParseInventory

================

*/
void Client::CL_ParseInventory( MessageBuffer & msg_buffer ) {
	for( int i = 0; i < MAX_ITEMS; i++ )
		cl.inventory[ i ] = msg_buffer.ReadShort( );
}


/*

================

Inv_DrawString

================

*/
void Inv_DrawString( int x, int y, const Str & str ) {

#if 0

	const char * string = str;
	while( *string ) {

		Renderer::DrawChar( x, y, * string );
		x+= 8;
		string++;
	}

#endif
}

/*

================

CL_DrawInventory

================

*/
void Client::CL_DrawInventory( ) {

#if 0

	int i, j;
	int num, selected_num, item;
	int index[ MAX_ITEMS ];
	Str string;
	int x, y;
	Str binding;
	Str bind;
	int selected;
	int top;

	selected = cl.frame.playerstate.stats[ STAT_SELECTED_ITEM ];

	num = 0;
	selected_num = 0;
	for( i = 0; i<MAX_ITEMS; i++ )
	{
		if( i == selected )
			selected_num = num;
		if( cl.inventory[ i ] )
		{
			index[ num ] = i;
			num++;
		}
	}

	// determine scroll point
	top = selected_num - 17/2;
	if( num - top < 17 )
		top = num - 17;
	if( top < 0 )
		top = 0;

	x =( Video::viddef.width-256 )/2;
	y =( Video::viddef.height-240 )/2;

	// repaint everything next frame
	Screen::SCR_DirtyScreen( );

	Renderer::DrawPic( x, y+8, Str( "pics/inventory.pcx" ) );

	y += 24;
	x += 24;
	Inv_DrawString( x, y, Str( "hotkey ### item" ) );
	Inv_DrawString( x, y+8, Str( "------ --- ----" ) );
	y += 16;
	for( i = top; i < num && i < top + 17; i++ ) {

		item = index[ i ];
		// search for a binding
		sprintf( binding, "use %s", cl.configstrings[ CS_ITEMS+item ].c_str( ) );
		bind = Keyboard::Key_GetKeyBindByCommand( binding );

		sprintf( string, "%s %3i %s", bind.c_str( ), cl.inventory[ item ], cl.configstrings[ CS_ITEMS+item ].c_str( ) );
		//if( item != selected ) SetStringHighBit( &string ); // FIXME
		//else // draw a blinky cursor by the selected item
		{
			if( ( int )( cls.realtime* 10 ) & 1 )
				Renderer::DrawChar( x-8, y, 15 );
		}
		Inv_DrawString( x, y, string );
		y += 8;
	}

#endif
}
