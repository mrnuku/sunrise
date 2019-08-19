#include "precompiled.h"
#pragma hdrstop

/*
===============
Draw_InitLocal
===============
*/
void Renderer::Draw_InitLocal( ) {

	// load console characters( don't bilerp characters )
	draw_chars = MaterialSystem::FindTexture( "pics/conchars.pcx", TextureBase::modelTexture ); // FIXME
	draw_chars->Bind( );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
}



/*
================
Draw_Char

Draws one 8* 8 graphics character with 0 being transparent.
It can be clipped to the top of the screen to allow the console to be
smoothly scrolled off.
================
*/
void Renderer::DrawChar( int x, int y, int num ) {

	int				row, col;
	float			frow, fcol, size;

	num &= 255;
	
	if( ( num&127 ) == 32 )
		return;		// space

	if( y <= -8 )
		return;			// totally off screen

	row = num>>4;
	col = num&15;

	frow = row* 0.0625f;
	fcol = col* 0.0625f;
	size = 0.0625f;

	draw_chars->Bind( );

	glBegin( GL_QUADS );
	glTexCoord2f( fcol, frow );
	glVertex2i( x, y );
	glTexCoord2f( fcol + size, frow );
	glVertex2i( x + 8, y );
	glTexCoord2f( fcol + size, frow + size );
	glVertex2i( x + 8, y + 8 );
	glTexCoord2f( fcol, frow + size );
	glVertex2i( x, y + 8 );
	glEnd( );
}

/*
=============
Draw_FindPic
=============
*/
image_t	* Renderer::RegisterPic( const Str & name ) {

	return GL_FindImage( name, it_pic );
}

/*
=============
Draw_GetPicSize
=============
*/
void Renderer::DrawGetPicSize( int * w, int * h, const Str & pic ) {

	TextureBase * gl = MaterialSystem::FindTexture( pic, TextureBase::uiTexture );

	if( !gl ) {

		*w = *h = -1;
		return;
	}

	Bounds2D size = gl->GetImage( )->Bounds( );

	*w = size[ 1 ][ 0 ];
	*h = size[ 1 ][ 1 ];
}

/*
=============
Draw_StretchPic
=============
*/
void Renderer::DrawStretchPic( int x, int y, int w, int h, const Str & pic ) {

	TextureBase * gl = MaterialSystem::FindTexture( pic, TextureBase::uiTexture );

	if( !gl ) {

		Common::Com_Printf( "Can't find pic: %s\n", pic.c_str( ) );
		return;
	}

	//if( scrap_dirty ) Scrap_Upload( );

	gl->Bind( );
	glBegin( GL_QUADS );
	/*glTexCoord2f( gl->sl, gl->tl );
	glVertex2i( x, y );
	glTexCoord2f( gl->sh, gl->tl );
	glVertex2i( x + w, y );
	glTexCoord2f( gl->sh, gl->th );
	glVertex2i( x + w, y+h );
	glTexCoord2f( gl->sl, gl->th );
	glVertex2i( x, y + h );*/ // FIXME
	glEnd( );
}


/*
=============
Draw_Pic
=============
*/
void Renderer::DrawPic( int x, int y, const Str & pic ) {

	TextureBase * gl = MaterialSystem::FindTexture( pic, TextureBase::uiTexture );

	if( !gl ) {

		Common::Com_Printf( "Can't find pic: %s\n", pic.c_str( ) );
		return;
	}

	//if( scrap_dirty ) Scrap_Upload( );

	gl->Bind( );
	glBegin( GL_QUADS );
	/*glTexCoord2f( gl->sl, gl->tl );
	glVertex2i( x, y );
	glTexCoord2f( gl->sh, gl->tl );
	glVertex2i( x + gl->width, y );
	glTexCoord2f( gl->sh, gl->th );
	glVertex2i( x + gl->width, y + gl->height );
	glTexCoord2f( gl->sl, gl->th );
	glVertex2i( x, y + gl->height );*/ // FIXME
	glEnd( );
}

/*
=============
Draw_TileClear

This repeats a 64* 64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void Renderer::DrawTileClear( int x, int y, int w, int h, const Str & pic ) {

	TextureBase * image = MaterialSystem::FindTexture( pic, TextureBase::uiTexture );

	if( !image ) {

		Common::Com_Printf( "Can't find pic: %s\n", pic.c_str( ) );
		return;
	}

	image->Bind( );
	glBegin( GL_QUADS );
	glTexCoord2f( x/64.0f, y/64.0f );
	glVertex2i( x, y );
	glTexCoord2f( ( x+w )/64.0f, y/64.0f );
	glVertex2i( x + w, y );
	glTexCoord2f( ( x+w )/64.0f, ( y+h )/64.0f );
	glVertex2i( x + w, y + h );
	glTexCoord2f( x/64.0f, ( y+h )/64.0f );
	glVertex2i( x, y + h );
	glEnd( );
}


/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Renderer::DrawFill( int x, int y, int w, int h, int c ) {

	union {

		unsigned	c;
		byte		v[ 4 ];

	} color;

	if( ( unsigned )c > 255 ) Common::Com_Error( ERR_FATAL, "Draw_Fill: bad color" );

	//glDisable( GL_TEXTURE_2D );

	color.c = d_8to24table[ c ];
	glColor3f( color.v[ 0 ] / 255.0f, color.v[ 1 ] / 255.0f, color.v[ 2 ] / 255.0f );

	glBegin( GL_QUADS );

	glVertex2i( x, 		y );
	glVertex2i( x + w, 	y );
	glVertex2i( x+  w, 	y + h );
	glVertex2i( x, 		y + h );

	glEnd( );
	glColor3f( 1.0f, 1.0f, 1.0f );
	//glEnable( GL_TEXTURE_2D );
}

//=============================================================================

/*
================
Draw_FadeScreen
================
*/
void Renderer::DrawFadeScreen( ) {

	glEnable( GL_BLEND );
	//glDisable( GL_TEXTURE_2D );
	glColor4f( 0.0f, 0.0f, 0.0f, 0.8f );
	glBegin( GL_QUADS );

	glVertex2i( 0, 0 );
	glVertex2i( Video::viddef.width, 0 );
	glVertex2i( Video::viddef.width, Video::viddef.height );
	glVertex2i( 0, Video::viddef.height );

	glEnd( );
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	//glEnable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );
}


//====================================================================


/*
=============
Draw_StretchRaw
=============
*/
void Renderer::DrawStretchRaw( int x, int y, int w, int h, int cols, int rows, byte * data ) {

	unsigned	image32[256* 256];
	int			i, j, trows;
	byte		* source;
	int			frac, fracstep;
	float		hscale;
	int			row;
	float		t;

	GL_Bind( 0 );

	if( rows<= 256 )
	{
		hscale = 1;
		trows = rows;
	}
	else
	{
		hscale = rows/256.0f;
		trows = 256;
	}
	t = rows* hscale / 256;

	unsigned * dest;

	for( i = 0; i<trows; i++ )
	{
		row =( int )( i* hscale );
		if( row > rows )
			break;
		source = data + cols* row;
		dest = &image32[i* 256];
		fracstep = cols* 0x10000/256;
		frac = fracstep >> 1;
		for( j = 0; j<256; j++ )
		{
			dest[ j ] = source[ frac >> 16 ];
			frac += fracstep;
		}
	}

	glTexImage2D( GL_TEXTURE_2D, 0, gl_tex_solid_format, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, image32 );

	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glBegin( GL_QUADS );
	glTexCoord2f( 0, 0 );
	glVertex2i( x, y );
	glTexCoord2f( 1, 0 );
	glVertex2i( x + w, y );
	glTexCoord2f( 1, t );
	glVertex2i( x + w, y+h );
	glTexCoord2f( 0, t );
	glVertex2i( x, y + h );
	glEnd( );
}

