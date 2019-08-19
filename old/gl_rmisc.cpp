#include "precompiled.h"
#pragma hdrstop

/*
==================
R_InitParticleTexture
==================
*/
byte Renderer::dottexture[ 8 ][ 8 ] = {

	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 1, 1, 0, 0, 0, 0},
	{0, 1, 1, 1, 1, 0, 0, 0},
	{0, 1, 1, 1, 1, 0, 0, 0},
	{0, 0, 1, 1, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
};

void Renderer::InitParticleTexture( ) {

#if 0

	int		x, y;
	byte	data[ 8 ][ 8 ][ 4 ];

	//
	// also use this for bad textures, but without alpha
	//
	for( x = 0; x<8; x++ ) {

		for( y = 0; y<8; y++ ) {

			data[ y][x ][ 0 ] = dottexture[ x&3][y&3 ]* 255;
			data[ y][x ][ 1 ] = 0; // dottexture[ x&3][y&3 ]* 255;
			data[ y][x ][ 2 ] = 0; //dottexture[ x&3][y&3 ]* 255;
			data[ y][x ][ 3 ] = 255;
		}
	}

	r_notexture = GL_LoadPic( Str( "***r_notexture***" ), ( byte * )data, 8, 8, it_wall, 32 );

#endif

}


/* 
============================================================================== 
 
						SCREEN SHOTS 
 
============================================================================== 
*/ 

typedef struct _TargaHeader {

	unsigned char 	_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;

} TargaHeader;

/*
* * GL_Strings_f
*/
void Renderer::GL_Strings_f( ) {

	Common::Com_Printf( "GL_VENDOR: %s\n", gl_config.vendor_string.c_str( ) );
	Common::Com_Printf( "GL_RENDERER: %s\n", gl_config.renderer_string.c_str( ) );
	Common::Com_Printf( "GL_VERSION: %s\n", gl_config.version_string.c_str( ) );
	Common::Com_Printf( "GL_EXTENSIONS: %s\n", gl_config.extensions_string.c_str( ) );
}

/*
* * GL_SetDefaultState
*/
void Renderer::GL_SetDefaultState( ) {

	glClearColor( 1.0f, 0.0f, 0.5f, 0.5f );
	glCullFace( GL_FRONT );
	//glEnable( GL_TEXTURE_2D );

	//glEnable( GL_ALPHA_TEST );
	//glAlphaFunc( GL_GREATER, 0.666f );

	glDisable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE );
	glDisable( GL_BLEND );

	glColor4f( 1, 1, 1, 1 );

	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	//glShadeModel( GL_FLAT );

	//GL_TextureMode( gl_texturemode.GetString( ) );
	//GL_TextureAlphaMode( gl_texturealphamode.GetString( ) );
	//GL_TextureSolidMode( gl_texturesolidmode.GetString( ) );

	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min );
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max );

	//glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	//glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	//glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	//GL_TexEnv( GL_REPLACE );

	float attenuations[ 3 ];

	attenuations[ 0 ] = gl_particle_att_a.GetFloat( );
	attenuations[ 1 ] = gl_particle_att_b.GetFloat( );
	attenuations[ 2 ] = gl_particle_att_c.GetFloat( );

	//glEnable( GL_POINT_SMOOTH );
	//GLImports::glPointParameterfEXT( GL_POINT_SIZE_MIN_EXT, gl_particle_min_size.GetFloat( ) );
	//GLImports::glPointParameterfEXT( GL_POINT_SIZE_MAX_EXT, gl_particle_max_size.GetFloat( ) );
	//GLImports::glPointParameterfvEXT( GL_DISTANCE_ATTENUATION_EXT, attenuations );

	GL_UpdateSwapInterval( );
}

void Renderer::GL_UpdateSwapInterval( ) {

	if( gl_swapinterval.IsModified( ) ) {

		gl_swapinterval.ClearModified( );

		if( !gl_state.stereo_enabled ) {
#ifdef _WIN32
			GLImports::wglSwapIntervalEXT( gl_swapinterval.GetInt( ) );
#endif
		}
	}
}