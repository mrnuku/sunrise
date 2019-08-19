#include "precompiled.h"
#pragma hdrstop

#if 0
int								Renderer::numgltextures;
int								Renderer::base_textureid;		// gltextures[ i ] = base_textureid+i
#endif

void Renderer::GL_EnableMultitexture( bool enable ) {

	if( enable ) {

		GL_SelectTexture( GL_TEXTURE1 );
		//glEnable( GL_TEXTURE_2D );
		GL_TexEnv( GL_REPLACE );
	} else {

		GL_SelectTexture( GL_TEXTURE1 );
		//glDisable( GL_TEXTURE_2D );
		GL_TexEnv( GL_REPLACE );
	}

	GL_SelectTexture( GL_TEXTURE0 );
	GL_TexEnv( GL_REPLACE );
}

void Renderer::GL_SelectTexture( uint texture ) {

	int tmu;

	if( texture == GL_TEXTURE0 ) tmu = 0;
	else tmu = 1;

	if( tmu == gl_state.currenttmu ) return;

	gl_state.currenttmu = tmu;

	GLImports::glActiveTexture( texture );
	GLImports::glClientActiveTextureARB( texture );
}

void Renderer::GL_TexEnv( uint mode ) {
	
	if( mode != texEnvLastModes[ gl_state.currenttmu ] ) {

		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode );
		texEnvLastModes[ gl_state.currenttmu ] = mode;
	}
}

void Renderer::GL_Bind( int texnum ) {

	if( gl_state.currenttextures[ gl_state.currenttmu ] == texnum ) return;

	gl_state.currenttextures[ gl_state.currenttmu ] = texnum;
	glBindTexture( GL_TEXTURE_2D, texnum );
}

void Renderer::GL_MBind( uint target, int texnum ) {

	GL_SelectTexture( target );

	if( target == GL_TEXTURE0 ) if( gl_state.currenttextures[ 0 ] == texnum ) return;
	else if( gl_state.currenttextures[ 1 ] == texnum ) return;

	GL_Bind( texnum );
}

/*
===============
GL_TextureMode
===============
*/
void Renderer::GL_TextureMode( const Str & string ) {

#if 0

	int		i;
	image_t	* glt;

	for( i = 0; modes[ i ].name; i++ ) {

		if( modes[ i ].name == string ) break;
	}

	if( !modes[ i ].name ) {

		Common::Com_Printf( "bad filter name\n" );
		return;
	}

	gl_filter_min = modes[ i ].minimize;
	gl_filter_max = modes[ i ].maximize;

	// change all the existing mipmap texture objects
	for( i = 0, glt = gltextures; i<numgltextures; i++, glt++ ) {

		if( glt->type != it_pic && glt->type != it_sky ) {

			GL_Bind( glt->texnum );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max );
		}
	}

#endif
}

/*
===============
GL_TextureAlphaMode
===============
*/
void Renderer::GL_TextureAlphaMode( const Str & string ) {

	int		i;

	for( i = 0; gl_alpha_modes[ i ].name; i++ ) {

		if( gl_alpha_modes[ i ].name == string ) break;
	}

	if( !gl_alpha_modes[ i ].name ) {

		Common::Com_Printf( "bad alpha texture mode name\n" );
		return;
	}

	gl_tex_alpha_format = gl_alpha_modes[ i ].mode;
}

/*
===============
GL_TextureSolidMode
===============
*/
void Renderer::GL_TextureSolidMode( const Str & string ) {

	int		i;

	for( i = 0; gl_solid_modes[ i ].name; i++ ) {

		if( gl_solid_modes[ i ].name == string ) break;
	}

	if( !gl_solid_modes[ i ].name ) {

		Common::Com_Printf( "bad solid texture mode name\n" );
		return;
	}

	gl_tex_solid_format = gl_solid_modes[ i ].mode;
}

/*
=============================================================================

  scrap allocation

  Allocate all the little status bar obejcts into a single texture
  to crutch up inefficient hardware / drivers

=============================================================================
*/

// returns a texture number and the position inside it
int Renderer::Scrap_AllocBlock( int w, int h, int * x, int * y ) {

	int		i, j;
	int		best, best2;
	int		texnum;

	for( texnum = 0; texnum < MAX_SCRAPS; texnum++ ) {

		best = SCRAP_BLOCK_HEIGHT;

		for( i = 0; i < SCRAP_BLOCK_WIDTH - w; i++ ) {

			best2 = 0;

			for( j = 0; j < w; j++ ) {

				if( scrap_allocated[ texnum ][ i + j ] >= best ) break;
				if( scrap_allocated[ texnum ][ i + j ] > best2 ) best2 = scrap_allocated[ texnum][i+j ];
			}
			if( j == w ) {	// this is a valid spot

				* x = i;
				* y = best = best2;
			}
		}

		if( best + h > SCRAP_BLOCK_HEIGHT ) continue;

		for( i = 0; i < w; i++ ) scrap_allocated[ texnum ][ * x + i ] = best + h;

		return texnum;
	}

	return -1;
//	Sys_Error( "Scrap_AllocBlock: full" );
}

void Renderer::Scrap_Upload( ) {

	scrap_uploads++;
	GL_Bind( TEXNUM_SCRAPS );
	GL_Upload8( scrap_texels[ 0 ], SCRAP_BLOCK_WIDTH, SCRAP_BLOCK_HEIGHT, false, false );
	scrap_dirty = false;
}

/*
=================================================================

PCX LOADING

=================================================================
*/


/*
==============
LoadPCX
==============
*/
void Renderer::LoadPCX( const Str & filename, byte * * pic, byte * * palette, int * width, int * height ) {

	byte	* raw;
	pcx_t	* pcx;
	int		x, y;
	int		dataByte, runLength;
	byte	* out, * pix;

	* pic = NULL;
	* palette = NULL;

	//
	// load the file
	//
	File_Memory fm = FileSystem::ReadFile( filename );

	//
	// parse the PCX file
	//
	pcx = ( pcx_t * )fm.GetDataPtr( );

    pcx->xmin = LittleShort( pcx->xmin );
    pcx->ymin = LittleShort( pcx->ymin );
    pcx->xmax = LittleShort( pcx->xmax );
    pcx->ymax = LittleShort( pcx->ymax );
    pcx->hres = LittleShort( pcx->hres );
    pcx->vres = LittleShort( pcx->vres );
    pcx->bytes_per_line = LittleShort( pcx->bytes_per_line );
    pcx->palette_type = LittleShort( pcx->palette_type );

	raw = &pcx->data;

	if( pcx->manufacturer != 0x0A || pcx->version != 5 || pcx->encoding != 1 || pcx->bits_per_pixel != 8 || pcx->xmax >= 640 || pcx->ymax >= 480 ) {

		Common::Com_Printf( "Bad pcx file %s\n", filename.c_str( ) );
		return;
	}

	out =( byte * )malloc( ( pcx->ymax+1 ) * ( pcx->xmax+1 ) );

	*pic = out;

	pix = out;

	if( palette ) {

		*palette =( byte * )malloc( 768 );
		memcpy( *palette, ( byte * )pcx + fm.Length( ) - 768, 768 );
	}

	if( width ) * width = pcx->xmax+1;
	if( height ) * height = pcx->ymax+1;

	for( y = 0; y<= pcx->ymax; y++, pix += pcx->xmax+1 ) {

		for( x = 0; x<= pcx->xmax; ) {

			dataByte = * raw++;

			if( ( dataByte & 0xC0 ) == 0xC0 ) {

				runLength = dataByte & 0x3F;
				dataByte = * raw++;
			}
			else runLength = 1;

			while( runLength-- > 0 ) pix[ x++ ] = dataByte;
		}

	}

	if( raw -( byte * )pcx > fm.Length( ) ) {

		Common::Com_DPrintf( "PCX file %s was malformed", filename.c_str( ) );
		free( *pic );
		* pic = NULL;
	}

	FileSystem::FreeFile( fm );
}

/*
=========================================================

TARGA LOADING

=========================================================
*/

typedef struct _TargaHeader {
	unsigned char 	_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;


/*
=============
LoadTGA
=============
*/
void Renderer::LoadTGA( const Str & name, byte * * pic, int * width, int * height ) {

	int		columns, rows, numPixels;
	byte	* pixbuf;
	int		row, column;
	byte	* buf_p;
	TargaHeader		targa_header;
	byte			* targa_rgba;
	byte tmp[ 2 ];

	* pic = NULL;

	//
	// load the file
	//
	File_Memory fm = FileSystem::ReadFile( name );
	if( !fm.IsValid( ) ) {

		*pic = NULL;
		return;
	}

	buf_p = ( byte * )fm.GetDataPtr( );

	targa_header._length = * buf_p++;
	targa_header.colormap_type = * buf_p++;
	targa_header.image_type = * buf_p++;
	
	tmp[ 0 ] = buf_p[ 0 ];
	tmp[ 1 ] = buf_p[ 1 ];
	targa_header.colormap_index = LittleShort( *( ( short * )tmp ) );
	buf_p+= 2;
	tmp[ 0 ] = buf_p[ 0 ];
	tmp[ 1 ] = buf_p[ 1 ];
	targa_header.colormap_length = LittleShort( *( ( short * )tmp ) );
	buf_p+= 2;
	targa_header.colormap_size = * buf_p++;
	targa_header.x_origin = LittleShort( *( ( short * )buf_p ) );
	buf_p+= 2;
	targa_header.y_origin = LittleShort( *( ( short * )buf_p ) );
	buf_p+= 2;
	targa_header.width = LittleShort( *( ( short * )buf_p ) );
	buf_p+= 2;
	targa_header.height = LittleShort( *( ( short * )buf_p ) );
	buf_p+= 2;
	targa_header.pixel_size = * buf_p++;
	targa_header.attributes = * buf_p++;

	if( targa_header.image_type!= 2 && targa_header.image_type!= 10 )
		Common::Com_Error( ERR_DROP, "LoadTGA: Only type 2 and 10 targa RGB images supported\n" );

	if( targa_header.colormap_type != 0  ||( targa_header.pixel_size!= 32 && targa_header.pixel_size!= 24 ) ) Common::Com_Error( ERR_DROP, "LoadTGA: Only 32 or 24 bit images supported( no colormaps )\n" );

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if( width )
		* width = columns;
	if( height )
		* height = rows;

	targa_rgba =( byte * )malloc( numPixels* 4 );
	* pic = targa_rgba;

	if( targa_header._length != 0 )
		buf_p += targa_header._length;  // skip TARGA image comment
	
	if( targa_header.image_type == 2 ) {  // Uncompressed, RGB images
		for( row = rows-1; row>= 0; row-- ) {
			pixbuf = targa_rgba + row* columns* 4;
			for( column = 0; column<columns; column++ ) {
				unsigned char red, green, blue, alphabyte;
				switch( targa_header.pixel_size ) {
					case 24:
							
							blue = * buf_p++;
							green = * buf_p++;
							red = * buf_p++;
							* pixbuf++ = red;
							* pixbuf++ = green;
							* pixbuf++ = blue;
							* pixbuf++ = 255;
							break;
					case 32:
							blue = * buf_p++;
							green = * buf_p++;
							red = * buf_p++;
							alphabyte = * buf_p++;
							* pixbuf++ = red;
							* pixbuf++ = green;
							* pixbuf++ = blue;
							* pixbuf++ = alphabyte;
							break;
				}
			}
		}
	}
	else if( targa_header.image_type == 10 ) {   // Runlength encoded RGB images
		unsigned char red, green, blue, alphabyte, packetHeader, packetSize, j;
		for( row = rows-1; row>= 0; row-- ) {
			pixbuf = targa_rgba + row* columns* 4;
			for( column = 0; column<columns; ) {
				packetHeader = * buf_p++;
				packetSize = 1 +( packetHeader & 0x7F );
				if( packetHeader & 0x80 ) {        // run-length packet
					switch( targa_header.pixel_size ) {
						case 24:
								blue = * buf_p++;
								green = * buf_p++;
								red = * buf_p++;
								alphabyte = 255;
								break;
						case 32:
								blue = * buf_p++;
								green = * buf_p++;
								red = * buf_p++;
								alphabyte = * buf_p++;
								break;
					}
	
					for( j = 0;j<packetSize;j++ ) {
						* pixbuf++= red;
						* pixbuf++= green;
						* pixbuf++= blue;
						* pixbuf++= alphabyte;
						column++;
						if( column == columns ) { // run spans across rows
							column = 0;
							if( row>0 )
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row* columns* 4;
						}
					}
				}
				else {                            // non run-length packet
					for( j = 0;j<packetSize;j++ ) {
						switch( targa_header.pixel_size ) {
							case 24:
									blue = * buf_p++;
									green = * buf_p++;
									red = * buf_p++;
									* pixbuf++ = red;
									* pixbuf++ = green;
									* pixbuf++ = blue;
									* pixbuf++ = 255;
									break;
							case 32:
									blue = * buf_p++;
									green = * buf_p++;
									red = * buf_p++;
									alphabyte = * buf_p++;
									* pixbuf++ = red;
									* pixbuf++ = green;
									* pixbuf++ = blue;
									* pixbuf++ = alphabyte;
									break;
						}
						column++;
						if( column == columns ) { // pixel packet run spans across rows
							column = 0;
							if( row>0 )
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row* columns* 4;
						}						
					}
				}
			}
			breakOut:;
		}
	}

	FileSystem::FreeFile( fm );
}


/*
====================================================================

IMAGE FLOOD FILLING

====================================================================
*/


/*
=================
Mod_FloodFillSkin

Fill background pixels so mipmapping doesn't have haloes
=================
*/

typedef struct {

	short		x, y;
} floodfill_t;

// must be a power of 2
#define FLOODFILL_FIFO_SIZE 0x1000
#define FLOODFILL_FIFO_MASK ( FLOODFILL_FIFO_SIZE - 1 )

#define FLOODFILL_STEP( off, dx, dy ) { \
 \
	if( pos[ off ] == fillcolor ) \
	{ \
		pos[ off ] = 255; \
		fifo[ inpt ].x = x +( dx ), fifo[ inpt ].y = y +( dy ); \
		inpt =( inpt + 1 ) & FLOODFILL_FIFO_MASK; \
	} \
	else if( pos[ off ] != 255 ) fdc = pos[ off ]; \
}

void Renderer::FloodFillSkin( const byte * skin, int skinwidth, int skinheight ) {

#if 0
	byte				fillcolor = * skin; // assume this is the pixel to fill
	floodfill_t			fifo[ FLOODFILL_FIFO_SIZE ];
	int					inpt = 0, outpt = 0;
	int					filledcolor = -1;
	int					i;

	if( filledcolor == -1 )
	{
		filledcolor = 0;
		// attempt to find opaque black
		for( i = 0; i < 256; ++i )
			if( d_8to24table[ i ] ==( 255 << 0 ) ) // alpha 1.0f
			{
				filledcolor = i;
				break;
			}
	}

	// can't fill to filled color or to transparent color( used as visited marker )
	if( ( fillcolor == filledcolor ) ||( fillcolor == 255 ) ) {
		//printf( "not filling skin from %d to %d\n", fillcolor, filledcolor );
		return;
	}

	fifo[ inpt ].x = 0, fifo[ inpt ].y = 0;
	inpt =( inpt + 1 ) & FLOODFILL_FIFO_MASK;

	while( outpt != inpt )
	{
		int			x = fifo[ outpt ].x, y = fifo[ outpt ].y;
		int			fdc = filledcolor;
		byte		* pos = &skin[x + skinwidth * y];

		outpt =( outpt + 1 ) & FLOODFILL_FIFO_MASK;

		if( x > 0 )				FLOODFILL_STEP( -1, -1, 0 );
		if( x < skinwidth - 1 )	FLOODFILL_STEP( 1, 1, 0 );
		if( y > 0 )				FLOODFILL_STEP( -skinwidth, 0, -1 );
		if( y < skinheight - 1 )	FLOODFILL_STEP( skinwidth, 0, 1 );
		skin[x + skinwidth * y] = fdc;
	}

#endif
}

//=======================================================


/*
================
GL_ResampleTexture
================
*/
void Renderer::GL_ResampleTexture( unsigned * in, int inwidth, int inheight, unsigned * out, int outwidth, int outheight ) {

	int		i, j;
	unsigned	* inrow, * inrow2;
	unsigned	frac, fracstep;
	unsigned	p1[ 1024 ], p2[ 1024 ];
	byte		* pix1, * pix2, * pix3, * pix4;

	fracstep = inwidth* 0x10000/outwidth;

	frac = fracstep>>2;
	for( i = 0; i<outwidth; i++ )
	{
		p1[ i ] = 4* ( frac>>16 );
		frac += fracstep;
	}
	frac = 3* ( fracstep>>2 );
	for( i = 0; i<outwidth; i++ )
	{
		p2[ i ] = 4* ( frac>>16 );
		frac += fracstep;
	}

	for( i = 0; i<outheight; i++, out += outwidth )
	{
		inrow = in + inwidth* ( int )( ( i+0.25f )* inheight/outheight );
		inrow2 = in + inwidth* ( int )( ( i+0.75f )* inheight/outheight );
		frac = fracstep >> 1;
		for( j = 0; j<outwidth; j++ )
		{
			pix1 =( byte * )inrow + p1[ j ];
			pix2 =( byte * )inrow + p2[ j ];
			pix3 =( byte * )inrow2 + p1[ j ];
			pix4 =( byte * )inrow2 + p2[ j ];
			( ( byte * )( out+j ) )[ 0 ] =( pix1[ 0 ] + pix2[ 0 ] + pix3[ 0 ] + pix4[ 0 ] )>>2;
			( ( byte * )( out+j ) )[ 1 ] =( pix1[ 1 ] + pix2[ 1 ] + pix3[ 1 ] + pix4[ 1 ] )>>2;
			( ( byte * )( out+j ) )[ 2 ] =( pix1[ 2 ] + pix2[ 2 ] + pix3[ 2 ] + pix4[ 2 ] )>>2;
			( ( byte * )( out+j ) )[ 3 ] =( pix1[ 3 ] + pix2[ 3 ] + pix3[ 3 ] + pix4[ 3 ] )>>2;
		}
	}
}

/*
================
GL_LightScaleTexture

Scale up the pixel values in a texture to increase the
lighting range
================
*/
void Renderer::GL_LightScaleTexture( unsigned * in, int inwidth, int inheight, bool only_gamma ) {

	if( only_gamma )
	{
		int		i, c;
		byte	* p;

		p =( byte * )in;

		c = inwidth* inheight;
		for( i = 0; i<c; i++, p+= 4 )
		{
			p[ 0 ] = gammatable[p[ 0 ]];
			p[ 1 ] = gammatable[p[ 1 ]];
			p[ 2 ] = gammatable[p[ 2 ]];
		}
	}
	else
	{
		int		i, c;
		byte	* p;

		p =( byte * )in;

		c = inwidth* inheight;
		for( i = 0; i<c; i++, p+= 4 )
		{
			p[ 0 ] = gammatable[intensitytable[p[ 0 ]]];
			p[ 1 ] = gammatable[intensitytable[p[ 1 ]]];
			p[ 2 ] = gammatable[intensitytable[p[ 2 ]]];
		}
	}
}

/*
================
GL_MipMap

Operates in place, quartering the size of the texture
================
*/
void Renderer::GL_MipMap( byte * in, int width, int height ) {

	int		i, j;
	byte	* out;

	width <<= 2;
	height >>= 1;
	out = in;
	for( i = 0; i<height; i++, in+= width ) {

		for( j = 0; j<width; j+= 8, out+= 4, in+= 8 ) {

			out[ 0 ] =( in[ 0 ] + in[ 4 ] + in[ width+0 ] + in[ width+4 ] )>>2;
			out[ 1 ] =( in[ 1 ] + in[ 5 ] + in[ width+1 ] + in[ width+5 ] )>>2;
			out[ 2 ] =( in[ 2 ] + in[ 6 ] + in[ width+2 ] + in[ width+6 ] )>>2;
			out[ 3 ] =( in[ 3 ] + in[ 7 ] + in[ width+3 ] + in[ width+7 ] )>>2;
		}
	}
}

/*
===============
GL_Upload32

Returns has_alpha
===============
*/
void Renderer::GL_BuildPalettedTexture( unsigned char * paletted_texture, unsigned char * scaled, int scaled_width, int scaled_height ) {

	int i;

	for( i = 0; i < scaled_width * scaled_height; i++ ) {

		unsigned int r, g, b, c;

		r =( scaled[ 0 ] >> 3 ) & 31;
		g =( scaled[ 1 ] >> 2 ) & 63;
		b =( scaled[ 2 ] >> 3 ) & 31;

		c = r |( g << 5 ) |( b << 11 );

		paletted_texture[ i ] = gl_state.d_16to8table[ c ];

		scaled += 4;
	}
}

int		upload_width, upload_height;
bool uploaded_paletted;

bool Renderer::GL_Upload32( const unsigned * data, int width, int height,  bool mipmap ) {

	int			samples;
	unsigned	scaled[ 256 * 256 ];
	int			scaled_width, scaled_height;
	int			i, c;
	byte *		scan;
	int			comp;

	uploaded_paletted = false;

	for( scaled_width = 1; scaled_width < width; scaled_width<<= 1 );
	if( gl_round_down.GetBool( ) && scaled_width > width && mipmap ) scaled_width >>= 1;
	for( scaled_height = 1; scaled_height < height; scaled_height<<= 1 );
	if( gl_round_down.GetBool( ) && scaled_height > height && mipmap ) scaled_height >>= 1;

	// let people sample down the world textures for speed
	if( mipmap ) {

		scaled_width >>= gl_picmip.GetInt( );
		scaled_height >>= gl_picmip.GetInt( );
	}

	// don't ever bother with >256 textures
	if( scaled_width > 256 )	scaled_width = 256;
	if( scaled_height > 256 )	scaled_height = 256;

	if( scaled_width < 1 )		scaled_width = 1;
	if( scaled_height < 1 )		scaled_height = 1;

	upload_width = scaled_width;
	upload_height = scaled_height;

	if( scaled_width * scaled_height > sizeof( scaled ) / 4 ) Common::Com_Error( ERR_DROP, "GL_Upload32: too big" );

	// scan the texture for any non-255 alpha
	c = width * height;
	scan =( ( byte * )data ) + 3;
	samples = gl_solid_format;
	for( i = 0; i < c; i ++, scan += 4 ) {

		if( *scan != 255 ) {

			samples = gl_alpha_format;
			break;
		}
	}

	if( samples == gl_solid_format ) comp = gl_tex_solid_format;
	else if( samples == gl_alpha_format ) comp = gl_tex_alpha_format;
	else {

	    Common::Com_Printf( "Unknown number of texture components %i\n", samples );
	    comp = samples;
	}

	if( scaled_width == width && scaled_height == height ) {

		if( !mipmap ) {

			glTexImage2D( GL_TEXTURE_2D, 0, comp, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
			goto done;
		}
		memcpy( scaled, data, width * height * 4 );
	}
	else GL_ResampleTexture( const_cast< unsigned int * >( data ), width, height, scaled, scaled_width, scaled_height );

	GL_LightScaleTexture( scaled, scaled_width, scaled_height, !mipmap );

	glTexImage2D( GL_TEXTURE_2D, 0, comp, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled );

	if( mipmap ) {

		int		miplevel;

		miplevel = 0;
		while( scaled_width > 1 || scaled_height > 1 )
		{
			GL_MipMap( ( byte * )scaled, scaled_width, scaled_height );
			scaled_width >>= 1;
			scaled_height >>= 1;
			if( scaled_width < 1 ) scaled_width = 1;
			if( scaled_height < 1 ) scaled_height = 1;
			miplevel++;
			
			glTexImage2D( GL_TEXTURE_2D, miplevel, comp, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled );
		}
	}
done:;

	if( mipmap ) {

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max );
	} else {

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max );
	}

	return( samples == gl_alpha_format );
}

bool Renderer::GL_Upload8( const byte * data, int width, int height,  bool mipmap, bool is_sky ) {

	unsigned	trans[512* 256];
	int			i, s;
	int			p;

	s = width* height;

	if( s > sizeof( trans ) / 4 ) Common::Com_Error( ERR_DROP, "GL_Upload8: too large" );

	for( i = 0; i < s;i++ ) {

		p = data[ i ];
		trans[ i ] = d_8to24table[ p ];

		if( p == 255 )
		{	// transparent, so scan around for another color
			// to avoid alpha fringes
			// FIXME: do a full flood fill so mips work...
			if( i > width && data[ i-width ] != 255 )
				p = data[ i-width ];
			else if( i < s-width && data[ i+width ] != 255 )
				p = data[ i+width ];
			else if( i > 0 && data[ i-1 ] != 255 )
				p = data[ i-1 ];
			else if( i < s-1 && data[ i+1 ] != 255 )
				p = data[ i+1 ];
			else
				p = 0;
			// copy rgb components
			( ( byte * )&trans[ i ] )[ 0 ] =( ( byte * )&d_8to24table[ p ] )[ 0 ];
			( ( byte * )&trans[ i ] )[ 1 ] =( ( byte * )&d_8to24table[ p ] )[ 1 ];
			( ( byte * )&trans[ i ] )[ 2 ] =( ( byte * )&d_8to24table[ p ] )[ 2 ];
		}
	}

	return GL_Upload32( trans, width, height, mipmap );
}

/*
================
GL_LoadPic

This is also used as an entry point for the generated r_notexture
================
*/
image_t * Renderer::GL_LoadPic( const Str & name, const byte * pic, int width, int height, uint type, int bits ) {

	image_t		* image;

	// find a free image_t
	int i;
	for( i = 0, image = gltextures; i < numgltextures; i++, image++ ) {

		if( !image->texnum ) break;
	}
	if( i == numgltextures ) {

		if( numgltextures == MAX_GLTEXTURES ) Common::Com_Error( ERR_DROP, "MAX_GLTEXTURES" );
		numgltextures++;
	}
	image = &gltextures[ i ];

	//if( strlen( name ) >= sizeof( image->name ) ) Common::Com_Error( ERR_DROP, "Draw_LoadPic: \"%s\" is too long", name.c_str( ) );
	image->name = name;
	image->registration_sequence = registration_sequence;

	image->width = width;
	image->height = height;
	image->type =( imagetype_t )type;

	//if( type == it_skin && bits == 8 ) FloodFillSkin( pic, width, height );

	// load little pics into the scrap
	if( image->type == it_pic && bits == 8 && image->width < 64 && image->height < 64 ) {

		int		x, y;
		int		i, j, k;
		int		texnum;

		texnum = Scrap_AllocBlock( image->width, image->height, &x, &y );
		if( texnum == -1 ) goto nonscrap;
		scrap_dirty = true;

		// copy the texels into the scrap block
		k = 0;

		for( i = 0; i < image->height; i++ ) {

			for( j = 0; j < image->width; j++, k++ ) {

				scrap_texels[ texnum ][( y + i ) * SCRAP_BLOCK_WIDTH + x + j ] = pic[ k ];
			}
		}

		image->texnum = TEXNUM_SCRAPS + texnum;
		image->scrap = true;
		image->has_alpha = true;
		image->sl =( x + 0.01f ) /( float )SCRAP_BLOCK_WIDTH;
		image->sh =( x + image->width - 0.01f ) /( float )SCRAP_BLOCK_WIDTH;
		image->tl =( y + 0.01f ) /( float )SCRAP_BLOCK_WIDTH;
		image->th =( y + image->height - 0.01f ) /( float )SCRAP_BLOCK_WIDTH;
	} else {
nonscrap:
		image->scrap = false;
		image->texnum = TEXNUM_IMAGES +( image - gltextures );
		GL_Bind( image->texnum );

		if( bits == 8 ) image->has_alpha = GL_Upload8( pic, width, height, ( image->type != it_pic && image->type != it_sky ), image->type == it_sky );
		else image->has_alpha = GL_Upload32( ( unsigned * )pic, width, height, ( image->type != it_pic && image->type != it_sky ) );

		image->upload_width = upload_width;		// after power of 2 and scales
		image->upload_height = upload_height;
		image->paletted = uploaded_paletted;
		image->sl = 0;
		image->sh = 1;
		image->tl = 0;
		image->th = 1;
	}

	return image;
}

/*
================
GL_LoadWal
================
*/
image_t * Renderer::GL_LoadWal( const Str & name ) {

	int			width, height, ofs;
	image_t		* image;

	File_Memory fm = FileSystem::ReadFile( name );
	miptex_t * mt = ( miptex_t * )fm.GetDataPtr( );

	width = LittleLong( mt->width );
	height = LittleLong( mt->height );
	ofs = LittleLong( mt->offsets[ 0 ] );

	image = GL_LoadPic( name, ( byte * )mt + ofs, width, height, it_wall, 8 );

	FileSystem::FreeFile( fm );

	return image;
}

/*
===============
GL_FindImage

Finds or loads the given image
===============
*/

void Renderer::GL_SaveTextures_f( ) {

#if 0

	Magick::PixelPacket * inputPacket, * outputPacket;
	uint count;
	Vec2 freeSpaceSize;

	for( BTreeNode< Memory2D, Rectangular > * node = textureAllocator.GetRoot( ); node; node = textureAllocator.GetNextLeaf( node ) ) {

		if( node->object ) {

			freeSpaceSize = node->object->GetBounds( )[ 1 ] - node->object->GetBounds( )[ 0 ];
			if( freeSpaceSize[ 0 ] <= 0 || freeSpaceSize[ 1 ] <= 0 )
				continue;
			Magick::Image freeSpace;
			freeSpace.size( Magick::Geometry( freeSpaceSize[ 0 ], freeSpaceSize[ 1 ] ) );

			freeSpace.strokeColor( "white" ); // Outline color
			freeSpace.fillColor( "gray" );
			freeSpace.strokeWidth( 1 );
			freeSpace.draw( Magick::DrawableRectangular( 1, 1, freeSpaceSize[ 0 ] - 2, freeSpaceSize[ 1 ] - 2 ) );

			freeSpace.font( "consola.ttf" );
			freeSpace.fontPointsize( 12 );
			freeSpace.boxColor( "transparent" );
			freeSpace.draw( Magick::DrawableText( 4, 16, ( node->key.GetExtents( ) * 2 ).ToString( ).c_str( ) ) );

			inputPacket = freeSpace.getPixels( 0, 0, freeSpaceSize[ 0 ], freeSpaceSize[ 1 ] );
			outputPacket = bigTexture->getPixels( node->object->GetBounds( )[ 0 ][ 0 ], node->object->GetBounds( )[ 0 ][ 1 ], freeSpaceSize[ 0 ], freeSpaceSize[ 1 ] );

			count = freeSpaceSize[ 0 ] * freeSpaceSize[ 1 ];

			for( uint i = 0; i < count; i++ ) {

				*outputPacket = *inputPacket;

				outputPacket++;
				inputPacket++;
			}

			bigTexture->syncPixels( );
		}
	}

	bigTexture->gaussianBlur( 1, 0.5 );
	bigTexture->write( "bigTexture.png" );

#endif
}

image_t	* Renderer::GL_FindImage( const Str & name, uint type ) {

#if 0

	image_t	* image;
	int		i, len;
	//byte	* pic, * palette;
	int		width, height;

	int dot = name.Find( '.' );

	if( dot == -1 ) {

		Common::Com_Printf( "Wrong image name: \"%s\"\n", name.c_str( ) );
		return NULL;
	}

	len = name.Length( );

	Str ext = name.Right( len - ( dot + 1 ) );

	// look for it
	for( i = 0, image = gltextures; i < numgltextures; i++, image++ ) {

		if( image->name.Length( ) && name == image->name ) {

			image->registration_sequence = registration_sequence;
			return image;
		}
	}

	File_Memory fm = FileSystem::ReadFile( name );

	if( !fm.IsValid( ) ) {

		Common::Com_Printf( "Unable to load image: \"%s\"\n", name.c_str( ) );
		return NULL;
	}

	Magick::Blob blob( fm.GetDataPtr( ), fm.Length( ) );

	FileSystem::FreeFile( fm );

	Magick::Image imageData;
	Magick::PixelPacket * inputPacket, * outputPacket;
	uint count;
	Vec2 freeSpaceSize;

	imageData.magick( ext.c_str( ) );
	imageData.read( blob );

	Magick::Blob rgbaBlob;
	imageData.magick( "rgba" );
	imageData.write( &rgbaBlob );

	Bounds2D imageBounds( Vec2( 0, 0 ), Vec2( imageData.columns( ), imageData.rows( ) ) );
	Rectangular imageBox( imageBounds );

	BTreeNode< Memory2D, Rectangular > * node = textureAllocator.GetRoot( );
	node = textureAllocator.GetNextLeaf( node );

	while( !node->key.CanFit( imageBox ) ) {

		node = textureAllocator.GetNextLeaf( node );
		if( !node ) break;
	}

	if( !node ) {

		GL_SaveTextures_f( );
		return NULL;
	}

	Memory2D freeMemory2D = *node->object;

	Memory2D::alignType_e freeAlign = freeMemory2D.GetAlign( );
	freeMemory2D.UpdateFlipped( imageBounds );

	if( freeMemory2D.IsFlipped( ) ) {

		imageData.rotate( 90 );
		Swap( imageBounds[ 1 ][ 0 ], imageBounds[ 1 ][ 1 ] );
	}

	imageData.fillColor( "transparent" );
	imageData.strokeColor( "white" );
	imageData.strokeWidth( 1 );
	imageData.draw( Magick::DrawableRectangular( 1, 1, imageBounds[ 1 ][ 0 ] - 2, imageBounds[ 1 ][ 1 ] - 2 ) );

//	Common::Com_DPrintf( "Bounds start: ( %s ) ( %s ) for image: ( %s ) ( %s )\n", freeMemory2D.GetBounds( )[ 0 ].ToString( ).c_str( ),
//		( freeAlign == Memory2D::align_ident ) ? "ident" : ( ( freeAlign == Memory2D::align_vertical ) ? "vertical" : "horizontal" ),
//		imageBounds[ 1 ].ToString( ).c_str( ), freeMemory2D.IsFlipped( ) ? "flipped" : "not flipped" );

	inputPacket = imageData.getPixels( 0, 0, imageBounds[ 1 ][ 0 ], imageBounds[ 1 ][ 1 ] );
	outputPacket = bigTexture->getPixels( freeMemory2D.GetBounds( )[ 0 ][ 0 ], freeMemory2D.GetBounds( )[ 0 ][ 1 ], imageBounds[ 1 ][ 0 ], imageBounds[ 1 ][ 1 ] );

	count = imageBounds[ 1 ][ 0 ] * imageBounds[ 1 ][ 1 ];

	for( uint i = 0; i < count; i++ ) {

		*outputPacket = *inputPacket;

		outputPacket++;
		inputPacket++;
	}

	bigTexture->syncPixels( );

	Bounds2D newFreeBounds;
	Memory2D * freeMem2D;

	switch( freeAlign ) {

		case Memory2D::align_ident:

			for( int index = 0; index < Memory2D::align_maxnum; index++ ) {

				newFreeBounds = freeMemory2D.CutBoundsBy( imageBounds, ( Memory2D::alignType_e )index );
				freeSpaceSize = newFreeBounds[ 1 ] - newFreeBounds[ 0 ];
				if( freeSpaceSize[ 0 ] > 0 && freeSpaceSize[ 1 ] > 0 ) {

					freeMem2D = new Memory2D( newFreeBounds, false, ( Memory2D::alignType_e )index );
					freeMem2D->node = textureAllocator.Add( freeMem2D, Rectangular( newFreeBounds ) );
				}
			}
			break;

		case Memory2D::align_vertical:

			newFreeBounds = freeMemory2D.CutBoundsBy( imageBounds, Memory2D::align_horizontal );
			freeSpaceSize = newFreeBounds[ 1 ] - newFreeBounds[ 0 ];
			if( freeSpaceSize[ 0 ] > 0 && freeSpaceSize[ 1 ] > 0 ) {

				freeMem2D = new Memory2D( newFreeBounds, false, Memory2D::align_horizontal );
				freeMem2D->node = textureAllocator.Add( freeMem2D, Rectangular( newFreeBounds ) );
			}

			newFreeBounds = freeMemory2D.CutBoundsBy( imageBounds, Memory2D::align_vertical );
			freeSpaceSize = newFreeBounds[ 1 ] - newFreeBounds[ 0 ];
			if( freeSpaceSize[ 0 ] > 0 && freeSpaceSize[ 1 ] > 0 ) {

				freeMem2D = new Memory2D( newFreeBounds, false, Memory2D::align_vertical );
				freeMem2D->node = textureAllocator.Add( freeMem2D, Rectangular( newFreeBounds ) );
			}

			break;

		case Memory2D::align_horizontal:

			newFreeBounds = freeMemory2D.CutBoundsBy( imageBounds, Memory2D::align_vertical );
			freeSpaceSize = newFreeBounds[ 1 ] - newFreeBounds[ 0 ];
			if( freeSpaceSize[ 0 ] > 0 && freeSpaceSize[ 1 ] > 0 ) {

				freeMem2D = new Memory2D( newFreeBounds, false, Memory2D::align_vertical );
				freeMem2D->node = textureAllocator.Add( freeMem2D, Rectangular( newFreeBounds ) );
			}

			newFreeBounds = freeMemory2D.CutBoundsBy( imageBounds, Memory2D::align_horizontal );
			freeSpaceSize = newFreeBounds[ 1 ] - newFreeBounds[ 0 ];
			if( freeSpaceSize[ 0 ] > 0 && freeSpaceSize[ 1 ] > 0 ) {

				freeMem2D = new Memory2D( newFreeBounds, false, Memory2D::align_horizontal );
				freeMem2D->node = textureAllocator.Add( freeMem2D, Rectangular( newFreeBounds ) );
			}

			break;
	}

	textureAllocator.Remove( node );
	delete node->object;

	image = GL_LoadPic( name, ( const byte * )rgbaBlob.data( ), imageData.columns( ), imageData.rows( ), type, 32 );

	return image;

#endif

	//
	// load the pic from disk
	//
	/*pic = NULL;
	palette = NULL;
	if( ext == "pcx" ) {

		LoadPCX( name, &pic, &palette, &width, &height );
		if( !pic ) return NULL; // Common::Com_Error( ERR_DROP, "GL_FindImage: can't load %s", name.c_str( ) );
		image = GL_LoadPic( name, pic, width, height, type, 8 );

	} else if( ext == "wal" ) {

		image = GL_LoadWal( name );

	} else if( ext == "tga" ) {

		LoadTGA( name, &pic, &width, &height );
		if( !pic ) return NULL; // Common::Com_Error( ERR_DROP, "GL_FindImage: can't load %s", name.c_str( ) );
		image = GL_LoadPic( name, pic, width, height, type, 32 );

	} else return NULL;	//	Common::Com_Error( ERR_DROP, "GL_FindImage: bad extension on: %s", name.c_str( ) );


	if( pic ) free( pic );
	if( palette ) free( palette );

	return image;*/

	return NULL;
}

/*
===============
R_RegisterSkin
===============
*/
struct image_s * Renderer::RegisterSkin( const Str & name ) {

	return GL_FindImage( name, it_skin );
}

/*
================
GL_FreeUnusedImages

Any image that was not touched on this registration sequence
will be freed.
================
*/
void Renderer::GL_FreeUnusedImages( ) {

	int		i;
	image_t	* image;

	// never free r_notexture or particle texture
	//r_notexture->registration_sequence = registration_sequence;
	//r_particletexture->registration_sequence = registration_sequence;

	for( i = 0, image = gltextures; i<numgltextures; i++, image++ )
	{
		if( image->registration_sequence == registration_sequence )
			continue;		// used this sequence
		if( !image->registration_sequence )
			continue;		// free image_t slot
		if( image->type == it_pic )
			continue;		// don't free pics
		// free it
		glDeleteTextures( 1, &image->texnum );
		memset( image, 0, sizeof( *image ) );
	}
}

/*
===============
Draw_GetPalette
===============
*/
void Renderer::Draw_GetPalette( ) {

	byte	* pic, * pal;
	int		width, height;

	// get the palette

	LoadPCX( Str( "pics/colormap.pcx" ), &pic, &pal, &width, &height );

	if( !pal ) Common::Com_Error( ERR_FATAL, "Couldn't load pics/colormap.pcx" );
	if( width != 256 ) Common::Com_Error( ERR_FATAL, "width of pics/colormap.pcx is not exactly 256" );

	for( int i = 0; i < 256; i++ ) {

		int r = pal[ i * 3 + 0 ];
		int g = pal[ i * 3 + 1 ];
		int b = pal[ i * 3 + 2 ];
		
		unsigned v =( 255 << 24 ) + ( r << 0 ) +( g << 8 ) +( b << 16 );

		d_8to24table[ i ] = LittleLong( v );
	}

	d_8to24table[ 255 ] &= LittleLong( 0xFFFFFF );	// 255 is transparent

	free( pic );
	free( pal );
}

/*
===============
GL_InitImages
===============
*/
void Renderer::GL_InitImages( ) {

	int		i, j;
	float	g = Common::vid_gamma.GetFloat( );

	registration_sequence = 1;

	if( r_intensity.GetFloat( ) <= 1.0f ) r_intensity.SetFloat( 1.0f );

	gl_state.inverse_intensity = 1.0f / r_intensity.GetFloat( );

	Draw_GetPalette( );

	for( i = 0; i < 256; i++ ) {

		if( g == 1 ) {

			gammatable[ i ] = i;

		} else {

			float inf = 255.0f * pow( ( i + 0.5f ) / 255.5f, g ) + 0.5f;
			if( inf < 0.0f ) inf = 0.0f;
			if( inf > 255 ) inf = 255.0f;
			gammatable[ i ] = ( unsigned char )inf;
		}
	}

	for( i = 0; i < 256; i++ ) {

		j = ( int )( ( float )i * r_intensity.GetFloat( ) );
		if( j > 255 ) j = 255;
		intensitytable[ i ] = j;
	}

}

/*
===============
GL_ShutdownImages
===============
*/
void Renderer::GL_ShutdownImages( ) {

	int		i;
	image_t	* image;

	for( i = 0, image = gltextures; i<numgltextures; i++, image++ )
	{
		if( !image->registration_sequence )
			continue;		// free image_t slot
		// free it
		glDeleteTextures( 1, &image->texnum );
		memset( image, 0, sizeof( *image ) );
	}
}

