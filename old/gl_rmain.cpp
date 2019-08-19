#include "precompiled.h"
#pragma hdrstop

int					Renderer::scrap_uploads	= 0;
bool				Renderer::scrap_dirty;
int					Renderer::scrap_allocated[ MAX_SCRAPS ][ SCRAP_BLOCK_WIDTH ];
byte				Renderer::scrap_texels[ MAX_SCRAPS ][ SCRAP_BLOCK_WIDTH * SCRAP_BLOCK_HEIGHT ];

TextureBase *		Renderer::draw_chars;

uint				Renderer::d_8to24table[ 256 ];
byte				Renderer::intensitytable[ 256 ];
unsigned char		Renderer::gammatable[ 256 ];

int					Renderer::gl_solid_format	= 3;
int					Renderer::gl_alpha_format	= 4;
int					Renderer::gl_tex_solid_format	= 3;
int					Renderer::gl_tex_alpha_format	= 4;
int					Renderer::gl_filter_min	= GL_LINEAR_MIPMAP_NEAREST;
int					Renderer::gl_filter_max		= GL_LINEAR;

int					Renderer::texEnvLastModes[ 2 ] = { -1, -1 };

model_t		* Renderer::r_worldmodel;

float		Renderer::gldepthmin, Renderer::gldepthmax;

glstate_t  Renderer::gl_state;

TextureBase	* Renderer::r_notexture;		// use for bad textures

entity_t	* Renderer::currententity;
model_t		* Renderer::currentmodel;

Plane	Renderer::frustum[ 4 ];

int			Renderer::r_visframecount;	// bumped when going to a new PVS
int			Renderer::r_framecount;		// used for dlight push checking

int			Renderer::c_brush_polys, Renderer::c_alias_polys;

float		Renderer::v_blend[ 4 ];			// final blending color

//
// view origin
//
Vec3	Renderer::vup;
Vec3	Renderer::vpn;
Vec3	Renderer::vright;
Vec3	Renderer::r_origin;

Frustum	Renderer::frustum2;

float	Renderer::r_world_matrix[ 16 ];
float	Renderer::r_base_world_matrix[ 16 ];

//
// screen size info
//
refdef_t	Renderer::r_newrefdef;

int		Renderer::r_viewcluster, Renderer::r_viewcluster2, Renderer::r_oldviewcluster, Renderer::r_oldviewcluster2;

/*
=================
R_CullBox

Returns true if the box is completely outside the frustom
=================
*/
bool Renderer::CullBox( Vec3 & mins, Vec3 & maxs ) {

	if( r_nocull.GetBool( ) ) return false;

	for( int i = 0; i < 4; i++ ) if( BOX_ON_PLANE_SIDE( mins, maxs, frustum[ i ] ) == 2 ) return true;
	return false;
}


void Renderer::RotateForEntity( entity_t * e ) {

    glTranslatef( e->origin[ 0 ],  e->origin[ 1 ],  e->origin[ 2 ] );

    glRotatef( e->angles[ 1 ],  0, 0, 1 );
    glRotatef( -e->angles[ 0 ],  0, 1, 0 );
    glRotatef( -e->angles[ 2 ],  1, 0, 0 );
}

/*
=============================================================

  SPRITE MODELS

=============================================================
*/


/*
=================
R_DrawSpriteModel

=================
*/
void Renderer::DrawSpriteModel( entity_t * e ) {

#if 0
	float alpha = 1.0f;
	Vec3	point;
	dsprframe_t	* frame;
	dsprite_t		* psprite;

	// don't even bother culling, because it's just a single
	// polygon without a surface cache

	psprite =( dsprite_t * )currentmodel->extradata;

	e->frame %= psprite->numframes;

	frame = &psprite->frames[ e->frame ];

	if( e->flags & RF_TRANSLUCENT )
		alpha = e->alpha;

	if( alpha != 1.0f )
		glEnable( GL_BLEND );

	glColor4f( 1, 1, 1, alpha );

	currentmodel->skins[ e->frame ]->Bind( );

	GL_TexEnv( GL_MODULATE );

	if( alpha == 1.0f )
		glEnable( GL_ALPHA_TEST );
	else
		glDisable( GL_ALPHA_TEST );

	glBegin( GL_QUADS );

	glTexCoord2f( 0, 1 );
	point = e->origin.Magnitude( ( float )-frame->origin_y, vup );
	point = point.Magnitude( ( float )-frame->origin_x, vright );
	glVertex3fv( point.ToFloatPtr( ) );

	glTexCoord2f( 0, 0 );
	point = e->origin.Magnitude( ( float )( frame->height - frame->origin_y ), vup );
	point = point.Magnitude( ( float )-frame->origin_x, vright );
	glVertex3fv( point.ToFloatPtr( ) );

	glTexCoord2f( 1, 0 );
	point = e->origin.Magnitude( ( float )( frame->height - frame->origin_y ), vup );
	point = point.Magnitude( ( float )( frame->width - frame->origin_x ), vright );
	glVertex3fv( point.ToFloatPtr( ) );

	glTexCoord2f( 1, 1 );
	point = e->origin.Magnitude( ( float )-frame->origin_y, vup );
	point = point.Magnitude( ( float )( frame->width - frame->origin_x ), vright );
	glVertex3fv( point.ToFloatPtr( ) );
	
	glEnd( );

	//glDisable( GL_ALPHA_TEST );
	GL_TexEnv( GL_REPLACE );

	if( alpha != 1.0f )
		glDisable( GL_BLEND );

	glColor4f( 1, 1, 1, 1 );

#endif
}

//==================================================================================

/*
=============
R_DrawNullModel
=============
*/
void Renderer::DrawNullModel( ) {

	Vec3	shadelight;
	int		i;

	if( currententity->flags & RF_FULLBRIGHT ) shadelight[ 0 ] = shadelight[ 1 ] = shadelight[ 2 ] = 1.0f;
	else LightPoint( currententity->origin, &shadelight );

    glPushMatrix( );
	RotateForEntity( currententity );

	//glDisable( GL_TEXTURE_2D );
	glColor3fv( shadelight.ToFloatPtr( ) );

	glBegin( GL_TRIANGLE_FAN );
	glVertex3f( 0, 0, -16 );
	for( i = 0; i<= 4; i++ )
		glVertex3f( 16* cos( i* M_PI/2 ), 16* sin( i* M_PI/2 ), 0 );
	glEnd( );

	glBegin( GL_TRIANGLE_FAN );
	glVertex3f( 0, 0, 16 );
	for( i = 4; i>= 0; i-- )
		glVertex3f( 16* cos( i* M_PI/2 ), 16* sin( i* M_PI/2 ), 0 );
	glEnd( );

	glColor3f( 1, 1, 1 );
	glPopMatrix( );
	//glEnable( GL_TEXTURE_2D );
}

/*
=============
R_DrawEntitiesOnList
=============
*/
void Renderer::DrawEntitiesOnList( ) {

	if( !r_drawentities.GetBool( ) ) return;

	// draw non-transparent first
	for( int i = 0; i < r_newrefdef.num_entities; i++ ) {

		currententity = &r_newrefdef.entities[ i ];
		if( currententity->flags & RF_TRANSLUCENT ) continue;	// solid

		if( currententity->flags & RF_BEAM ) DrawBeam( currententity );
		else {

			currentmodel = currententity->model;
			if( !currentmodel ) {

				DrawNullModel( );
				continue;
			}

			switch( currentmodel->type ) {

			case mod_alias:
				DrawAliasModel( currententity );
				break;

#if 0
			case mod_brush:
				DrawBrushModel( currententity );
				break;
#endif

			case mod_sprite:
				DrawSpriteModel( currententity );
				break;

			default:
				Common::Com_Error( ERR_DROP, "Bad modeltype" );
				break;
			}
		}
	}

	// draw transparent entities
	// we could sort these if it ever becomes a problem...
	glDepthMask( 0 );		// no z writes
	for( int i = 0; i < r_newrefdef.num_entities; i++ ) {

		currententity = &r_newrefdef.entities[ i ];
		if( !( currententity->flags & RF_TRANSLUCENT ) ) continue;	// solid

		if( currententity->flags & RF_BEAM ) DrawBeam( currententity );
		else {

			currentmodel = currententity->model;

			if( !currentmodel )
			{
				DrawNullModel( );
				continue;
			}

			switch( currentmodel->type ) {

			case mod_alias:
				DrawAliasModel( currententity );
				break;

#if 0
			case mod_brush:
				DrawBrushModel( currententity );
				break;
#endif

			case mod_sprite:
				DrawSpriteModel( currententity );
				break;

			default:
				Common::Com_Error( ERR_DROP, "Bad modeltype" );
				break;
			}
		}
	}

	glDepthMask( 1 );		// back to writing

}

/*
===============
R_DrawParticles
===============
*/
void Renderer::DrawParticles( ) {

	int i;
	unsigned char color[ 4 ];
	const particle_t * p;

	glDepthMask( GL_FALSE );
	glEnable( GL_BLEND );
	//glDisable( GL_TEXTURE_2D );

	glPointSize( gl_particle_size.GetFloat( ) );

	glBegin( GL_POINTS );
	for( i = 0, p = r_newrefdef.particles; i < r_newrefdef.num_particles; i++, p++ )
	{
		* ( int * )color = d_8to24table[ p->color ];
		color[ 3 ] = ( byte )( p->alpha * 255.0f );

		glColor4ubv( color );

		glVertex3fv( p->origin.ToFloatPtr( ) );
	}
	glEnd( );

	glDisable( GL_BLEND );
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	glDepthMask( GL_TRUE );
	//glEnable( GL_TEXTURE_2D );
}

/*
============
R_PolyBlend
============
*/
void Renderer::PolyBlend( ) {

	if( !gl_polyblend.GetBool( ) )
		return;
	if( !v_blend[ 3 ] )
		return;

	//glDisable( GL_ALPHA_TEST );
	glEnable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );
	//glDisable( GL_TEXTURE_2D );

    glLoadIdentity( );

	// FIXME: get rid of these
    glRotatef( -90,  1, 0, 0 );	    // put Z going up
    glRotatef( 90,  0, 0, 1 );	    // put Z going up

	glColor4fv( v_blend );

	glBegin( GL_QUADS );

	glVertex3f( 10, 100, 100 );
	glVertex3f( 10, -100, 100 );
	glVertex3f( 10, -100, -100 );
	glVertex3f( 10, 100, -100 );
	glEnd( );

	glDisable( GL_BLEND );
	//glEnable( GL_TEXTURE_2D );
	//glEnable( GL_ALPHA_TEST );

	glColor4f( 1, 1, 1, 1 );
}

//=======================================================================

#if 0
int Renderer::SignbitsForPlane( Plane * out ) {

	int	bits, j;

	// for fast box on planeside test

	bits = 0;
	for( j = 0; j<3; j++ )
	{
		if( out->normal[ j ] < 0 )
			bits |= 1<<j;
	}
	return bits;
}
#endif

void Renderer::SetFrustum( ) {

	Vec3 normal;

	// rotate VPN right by FOV_X/2 degrees
	normal = RotatePointAroundVector( vup, vpn, -( 90.0f - r_newrefdef.fov_x / 2.0f ) );
	frustum[ 0 ] = Plane( normal, r_origin * normal );

	// rotate VPN left by FOV_X/2 degrees
	normal = RotatePointAroundVector( vup, vpn, 90.0f - r_newrefdef.fov_x / 2.0f );
	frustum[ 1 ] = Plane( normal, r_origin * normal );

	// rotate VPN up by FOV_X/2 degrees
	normal = RotatePointAroundVector( vright, vpn, 90.0f - r_newrefdef.fov_y / 2.0f );
	frustum[ 2 ] = Plane( normal, r_origin * normal );

	// rotate VPN down by FOV_X/2 degrees
	normal = RotatePointAroundVector( vright, vpn, -( 90.0f - r_newrefdef.fov_y / 2.0f ) );
	frustum[ 3 ] = Plane( normal, r_origin * normal );

#if 0
	// rotate VPN right by FOV_X/2 degrees
	frustum[ 0 ].normal = RotatePointAroundVector( vup, vpn, -( 90.0f - r_newrefdef.fov_x / 2.0f ) );
	// rotate VPN left by FOV_X/2 degrees
	frustum[ 1 ].normal = RotatePointAroundVector( vup, vpn, 90.0f - r_newrefdef.fov_x / 2.0f );
	// rotate VPN up by FOV_X/2 degrees
	frustum[ 2 ].normal = RotatePointAroundVector( vright, vpn, 90.0f - r_newrefdef.fov_y / 2.0f );
	// rotate VPN down by FOV_X/2 degrees
	frustum[ 3 ].normal = RotatePointAroundVector( vright, vpn, -( 90.0f - r_newrefdef.fov_y / 2.0f ) );

	for( int i = 0; i < 4; i++ ) {

		frustum[ i ].type = PLANE_ANYZ;
		frustum[ i ].dist = r_origin * frustum[ i ].normal;
		frustum[ i ].signbits = SignbitsForPlane( &frustum[ i ] );
	}
#endif
}

//=======================================================================

/*
===============
R_SetupFrame
===============
*/
void Renderer::SetupFrame( ) {

	int i;
	mleaf_t	* leaf;

	r_framecount++;

// build the transformation matrix for the given view angles
	r_origin = r_newrefdef.vieworg;

	r_newrefdef.viewangles.AngleVectors( &vpn, &vright, &vup );

#if 0
// current viewcluster
	if( !( r_newrefdef.rdflags & RDF_NOWORLDMODEL ) ) {

		r_oldviewcluster = r_viewcluster;
		r_oldviewcluster2 = r_viewcluster2;
		leaf = Mod_PointInLeaf( r_origin, r_worldmodel );
		r_viewcluster = r_viewcluster2 = leaf->cluster;

		// check above and below so crossing solid water doesn't draw wrong
		if( !leaf->contents ) { // look down a bit

			Vec3 temp = r_origin;
			temp[ 2 ] -= 16.0f;
			leaf = Mod_PointInLeaf( temp, r_worldmodel );
			if( !( leaf->contents & CONTENTS_SOLID ) &&( leaf->cluster != r_viewcluster2 ) ) r_viewcluster2 = leaf->cluster;

		} else { // look up a bit

			Vec3 temp = r_origin;
			temp[ 2 ] += 16.0f;
			leaf = Mod_PointInLeaf( temp, r_worldmodel );
			if( !( leaf->contents & CONTENTS_SOLID ) &&( leaf->cluster != r_viewcluster2 ) ) r_viewcluster2 = leaf->cluster;
		}
	}
#endif

	for( i = 0; i < 4; i++ ) v_blend[ i ] = r_newrefdef.blend[ i ];

	c_brush_polys = 0;
	c_alias_polys = 0;

	// clear out the portion of the screen that the NOWORLDMODEL defines
	if( r_newrefdef.rdflags & RDF_NOWORLDMODEL ) {

		glEnable( GL_SCISSOR_TEST );
		glClearColor( 0.3f, 0.3f, 0.3f, 1 );
		glScissor( r_newrefdef.x, Video::viddef.height - r_newrefdef.height - r_newrefdef.y, r_newrefdef.width, r_newrefdef.height );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glClearColor( 1, 0, 0.5f, 0.5f );
		glDisable( GL_SCISSOR_TEST );
	}
}


void Renderer::SetProjection( GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar ) {

   GLdouble xmin, xmax, ymin, ymax;

   ymax = zNear * tan( fovy * M_PI / 360.0f );
   ymin = -ymax;

   xmin = ymin * aspect;
   xmax = ymax * aspect;

   xmin += -( 2.0f * gl_state.camera_separation ) / zNear;
   xmax += -( 2.0f * gl_state.camera_separation ) / zNear;

   glFrustum( xmin, xmax, ymin, ymax, zNear, zFar );
}


/*
=============
R_SetupGL
=============
*/
void Renderer::SetupGL( ) {

	float	screenaspect;
//	float	yfov;
	int		x, x2, y2, y, w, h;

	//
	// set up viewport
	// well done ;)
	x = ( int )floor( ( double )r_newrefdef.x * ( double )Video::viddef.width / ( double )Video::viddef.width );
	x2 = ( int )ceil( ( ( double )r_newrefdef.x +( double )r_newrefdef.width ) * ( double )Video::viddef.width / ( double )Video::viddef.width );
	y = ( int )floor( ( double )Video::viddef.height - ( double )r_newrefdef.y * ( double )Video::viddef.height / ( double )Video::viddef.height );
	y2 = ( int )ceil( ( double )Video::viddef.height - ( ( double )r_newrefdef.y + ( double )r_newrefdef.height ) * ( double )Video::viddef.height / ( double )Video::viddef.height );

	w = x2 - x;
	h = y - y2;

	glViewport( x, y2, w, h );

	//
	// set up projection matrix
	//
    screenaspect =( float )r_newrefdef.width/r_newrefdef.height;
//	yfov = 2* atan( ( float )r_newrefdef.height/r_newrefdef.width )* 180/M_PI;
	glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
    SetProjection( r_newrefdef.fov_y, screenaspect, 4.0f, 80000.0f );

	glCullFace( GL_FRONT );

	glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );

    glRotatef( -90.0f, 1.0f, 0.0f, 0.0f );	    // put Z going up
    glRotatef(  90.0f, 0.0f, 0.0f, 1.0f );	    // put Z going up
    glRotatef( -r_newrefdef.viewangles[ 2 ],  1, 0, 0 );
    glRotatef( -r_newrefdef.viewangles[ 0 ],  0, 1, 0 );
    glRotatef( -r_newrefdef.viewangles[ 1 ],  0, 0, 1 );
    glTranslatef( -r_newrefdef.vieworg[ 0 ],  -r_newrefdef.vieworg[ 1 ],  -r_newrefdef.vieworg[ 2 ] );

//	if( gl_state.camera_separation != 0 && gl_state.stereo_enabled )
//		glTranslatef( gl_state.camera_separation, 0, 0 );

	glGetFloatv( GL_MODELVIEW_MATRIX, r_world_matrix );

	//
	// set drawing parms
	//
	if( gl_cull.GetBool( ) ) glEnable( GL_CULL_FACE );
	else glDisable( GL_CULL_FACE );

	glDisable( GL_BLEND );
	//glDisable( GL_ALPHA_TEST );
	glEnable( GL_DEPTH_TEST );
}

/*
=============
R_Clear
=============
*/
void Renderer::Clear( ) {

	if( gl_ztrick.GetBool( ) ) {

		static int			trickframe;

		if( gl_clear.GetBool( ) )
			glClear( GL_COLOR_BUFFER_BIT );

		trickframe++;

		if( trickframe & 1 ) {

			gldepthmin = 0;
			gldepthmax = 0.49999f;
			glDepthFunc( GL_LEQUAL );
		} else {

			gldepthmin = 1;
			gldepthmax = 0.5f;
			glDepthFunc( GL_GEQUAL );
		}
	} else {

		if( gl_clear.GetFloat( ) ) glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		else glClear( GL_DEPTH_BUFFER_BIT );

		gldepthmin = 0;
		gldepthmax = 1;
		glDepthFunc( GL_LEQUAL );
	}

	glDepthRange( gldepthmin, gldepthmax );

}

void Renderer::Flash( ) {

	PolyBlend( );
}

/*
================
R_RenderView

r_newrefdef must be set before the first call
================
*/
void Renderer::RenderView( refdef_t * fd ) {

	if( r_norefresh.GetBool( ) ) return;

	r_newrefdef = * fd;

	//if( !r_worldmodel && !( r_newrefdef.rdflags & RDF_NOWORLDMODEL ) ) Common::Com_Error( ERR_DROP, "R_RenderView: NULL worldmodel" );

	if( r_speeds.GetBool( ) )  c_brush_polys = c_alias_polys = 0;

	PushDlights( );
	GLImports::GLErrorCheck( );

	if( gl_finish.GetBool( ) ) glFinish( );
	GLImports::GLErrorCheck( );

	SetupFrame( );
	GLImports::GLErrorCheck( );

	SetFrustum( );
	GLImports::GLErrorCheck( );

	SetupGL( );
	GLImports::GLErrorCheck( );

	//MarkLeaves( );	// done here so we know if we're in water

	DrawWorld( );
	GLImports::GLErrorCheck( );

	DrawEntitiesOnList( );
	GLImports::GLErrorCheck( );

	RenderDlights( );
	GLImports::GLErrorCheck( );

	DrawParticles( );
	GLImports::GLErrorCheck( );

	//DrawAlphaSurfaces( );

	Flash( );
	GLImports::GLErrorCheck( );

	//if( r_speeds.GetBool( ) ) Common::Com_Printf( "%4i wpoly %4i epoly %i tex %i lmaps\n", c_brush_polys, c_alias_polys, c_visible_textures, c_visible_lightmaps );
}


void Renderer::SetGL2D( ) {

	// set 2D virtual screen size
	glViewport( 0, 0, Video::viddef.width, Video::viddef.height );
	glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
	glOrtho ( 0, Video::viddef.width, Video::viddef.height, 0, -99999, 99999 );
	glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE );
	glDisable( GL_BLEND );
	//glEnable( GL_ALPHA_TEST );
	glColor4f( 1, 1, 1, 1 );
}

void Renderer::GL_DrawColoredStereoLinePair( float r, float g, float b, float y ) {

	glColor3f( r, g, b );
	glVertex2f( 0, y );
	glVertex2f( ( float )Video::viddef.width, y );
	glColor3f( 0, 0, 0 );
	glVertex2f( 0, y + 1 );
	glVertex2f( ( float )Video::viddef.width, y + 1 );
}

void Renderer::GL_DrawStereoPattern( ) {

	if( !gl_state.stereo_enabled ) return;

	SetGL2D( );

	glDrawBuffer( GL_BACK_LEFT );

	for( int i = 0; i < 20; i++ ) {

		glBegin( GL_LINES );
			GL_DrawColoredStereoLinePair( 1, 0, 0, 0 );
			GL_DrawColoredStereoLinePair( 1, 0, 0, 2 );
			GL_DrawColoredStereoLinePair( 1, 0, 0, 4 );
			GL_DrawColoredStereoLinePair( 1, 0, 0, 6 );
			GL_DrawColoredStereoLinePair( 0, 1, 0, 8 );
			GL_DrawColoredStereoLinePair( 1, 1, 0, 10 );
			GL_DrawColoredStereoLinePair( 1, 1, 0, 12 );
			GL_DrawColoredStereoLinePair( 0, 1, 0, 14 );
		glEnd( );
		
		EndFrame( );
	}
}


/*
====================
R_SetLightLevel

====================
*/
void Renderer::SetLightLevel( ) {

	Vec3		shadelight;

	if( r_newrefdef.rdflags & RDF_NOWORLDMODEL ) return;

	// save off light value for server to look at( BIG HACK! )

	LightPoint( r_newrefdef.vieworg, &shadelight );

	// pick the greatest component, which should be the same
	// as the mono value returned by software
}

/*
@@@@@@@@@@@@@@@@@@@@@
R_RenderFrame

@@@@@@@@@@@@@@@@@@@@@
*/
void Renderer::RenderFrame( refdef_t * fd ) {

	RenderView( fd );
	SetLightLevel( );
	SetGL2D( );
}

/*
===============
GL_ImageList_f
===============
*/
void Renderer::GL_ImageList_f( ) {

#if 0
	int		i;
	image_t	* image;
	int		texels;

	Common::Com_Printf( "------------------\n" );
	texels = 0;

	for( i = 0, image = gltextures; i<numgltextures; i++, image++ ) {

		if( image->texnum <= 0 )
			continue;
		texels += image->upload_width* image->upload_height;
		switch( image->type ) {

		case it_skin:

			Common::Com_Printf( "M" );
			break;

		case it_sprite:

			Common::Com_Printf( "S" );
			break;

		case it_wall:

			Common::Com_Printf( "W" );
			break;

		case it_pic:

			Common::Com_Printf( "P" );
			break;

		default:

			Common::Com_Printf( " " );
			break;

		}

		Common::Com_Printf( " %3i %3i %s: %s\n", image->upload_width, image->upload_height, image->paletted ? "PAL" : "RGB", image->name.c_str( ) );
	}
	Common::Com_Printf( "Total texel count( not counting mipmaps ): %i\n", texels );

#endif
}

/* 
================== 
GL_ScreenShot_f
================== 
*/  
void Renderer::GL_ScreenShot_f( ) {

	Str			picname; 
	Str			checkname;

	//
	// find a file name to save it to
	//
	for( int i = 0; ; i++ ) {

		if( i > 999 ) return;

		sprintf( picname, "sh_%0.3i.%s", i, screenshotFormat.GetString( ).c_str( ) );

		sprintf( checkname, "screenshot/%s", picname.c_str( ) );
		if( !FileSystem::TouchFile( checkname ) ) break;	// file doesn't exist
	}

	byte * buffer = ( byte * )Mem_Alloc( Video::viddef.width* Video::viddef.height * 3 );

	glReadPixels( 0, 0, Video::viddef.width, Video::viddef.height, GL_RGB, GL_UNSIGNED_BYTE, buffer );

	Magick::Image sh( Video::viddef.width, Video::viddef.height, "RGB", Magick::CharPixel, buffer );
	sh.flip( );
	Magick::Blob blob;
	sh.magick( screenshotFormat.GetString( ).c_str( ) );
	sh.write( &blob );

	File f = FileSystem::OpenFileByMode( checkname, FS_WRITE );
	f.Write( blob.data( ), blob.length( ) );
	FileSystem::CloseFile( f );

	Mem_Free( buffer );
	Common::Com_Printf( "Screen saved to %s in %.2f KiB\n", picname.c_str( ), ( float )blob.length( ) / 1024.0f );
}

void Renderer::GL_MagickInfo_f( ) {

	std::list< Magick::CoderInfo > coderList; 

	Magick::coderInfoList( &coderList,           // Reference to output list 
		Magick::CoderInfo::TrueMatch, // Match readable formats 
		Magick::CoderInfo::AnyMatch,  // Don't care about writable formats 
		Magick::CoderInfo::AnyMatch); // Don't care about multi-frame support 

	Common::Com_Printf( "Magick supported file formats: ( [R]eadable, [W]ritable, [M]ultiFrame, Extension string, description )\n\n" );

	for( std::list< Magick::CoderInfo >::iterator entry = coderList.begin( ); entry != coderList.end( ); entry++ ) {

		Common::Com_Printf( "%c%c%c %8s - \"%s\"\n", entry->isReadable( ) ? 'R' : ' ', entry->isWritable( ) ? 'W' : ' ',
			entry->isMultiFrame( ) ? 'M' : ' ', entry->name( ).c_str( ), entry->description( ).c_str( ) );
	}
}

void Renderer::Register( ) {

	Command::Cmd_AddCommand( "magickinfo", GL_MagickInfo_f );
	Command::Cmd_AddCommand( "imagelist", GL_ImageList_f );
	Command::Cmd_AddCommand( "screenshot", GL_ScreenShot_f );
	Command::Cmd_AddCommand( "modellist", Mod_Modellist_f );
	Command::Cmd_AddCommand( "gl_strings", GL_Strings_f );
	//Command::Cmd_AddCommand( "saveTextures", GL_SaveTextures_f );
}

/*
==================
R_SetMode
==================
*/
bool Renderer::SetMode( ) {

	uint err;
	bool fullscreen;

	if( Common::vid_fullscreen.IsModified( ) )
	{
		Common::Com_Printf( "R_SetMode( ) - CDS not allowed with this driver\n" );
		Common::vid_fullscreen.SetBool( !Common::vid_fullscreen.GetBool( ) );
		Common::vid_fullscreen.ClearModified( );
	}

	fullscreen = Common::vid_fullscreen.GetBool( );

	Common::vid_fullscreen.ClearModified( );
	gl_mode.ClearModified( );

	if( ( err = GLimp_SetMode( &Video::viddef.width, &Video::viddef.height, gl_mode.GetInt( ), fullscreen ) ) == rserr_ok )
	{
		gl_state.prev_mode = gl_mode.GetInt( );
	}
	else
	{
		if( err == rserr_invalid_fullscreen )
		{
			Common::vid_fullscreen.SetBool( false );
			Common::vid_fullscreen.ClearModified( );
			Common::Com_Printf( "ref_gl::R_SetMode( ) - fullscreen unavailable in this mode\n" );
			if( ( err = GLimp_SetMode( &Video::viddef.width, &Video::viddef.height, gl_mode.GetInt( ), false ) ) == rserr_ok )
				return true;
		}
		else if( err == rserr_invalid_mode )
		{
			gl_mode.SetInt( gl_state.prev_mode );
			gl_mode.ClearModified( );
			Common::Com_Printf( "ref_gl::R_SetMode( ) - invalid mode\n" );
		}

		// try setting it back to something safe
		if( ( err = GLimp_SetMode( &Video::viddef.width, &Video::viddef.height, gl_state.prev_mode, false ) ) != rserr_ok )
		{
			Common::Com_Printf( "ref_gl::R_SetMode( ) - could not revert to safe mode\n" );
			return false;
		}
	}
	return true;
}

/*
===============
R_Init
===============
*/
bool Renderer::Init( void * hinstance, void * hWnd ) {

	int		err;
	int		j;

	for( j = 0; j < 256; j++ )
	{
		r_turbsin[ j ] *= 0.5f;
	}

	Common::Com_Printf( "ref_gl version: "REF_VERSION"\n" );

	//Draw_GetPalette( );

	Register( );

	// initialize OS-specific parts of OpenGL
	if( !GLimp_Init( hinstance, hWnd ) ) return false;

	// set our "safe" modes
	gl_state.prev_mode = 3;

	// create the window and set up the context
	if( !SetMode( ) ) {

        Common::Com_Printf( "ref_gl::R_Init( ) - could not R_SetMode( )\n" );
		return false;
	}

	Video::VID_MenuInit( );

	/*
	* * get our various GL strings
	*/
	gl_config.vendor_string = Str( ( char * )glGetString( GL_VENDOR ) );
	//Common::Com_Printf( "GL_VENDOR: %s\n", gl_config.vendor_string.c_str( ) );
	gl_config.renderer_string = Str( ( char * )glGetString( GL_RENDERER ) );
	//Common::Com_Printf( "GL_RENDERER: %s\n", gl_config.renderer_string.c_str( ) );
	gl_config.version_string = Str( ( char * )glGetString( GL_VERSION ) );
	//Common::Com_Printf( "GL_VERSION: %s\n", gl_config.version_string.c_str( ) );
	gl_config.extensions_string = Str( ( char * )glGetString( GL_EXTENSIONS ) );
	//Common::Com_Printf( "GL_EXTENSIONS: %s\n", gl_config.extensions_string.c_str( ) );

	GLImports::Init( );

	MaterialSystem::Init( );

	GL_SetDefaultState( );

	/*
	* * draw our stereo patterns
	*/
#if 0 // commented out until H3D pays us the money they owe us
	GL_DrawStereoPattern( );
#endif

	//GL_InitImages( );
	Mod_Init( );
	InitParticleTexture( );
	//Draw_InitLocal( );

	err = glGetError( );
	if( err != GL_NO_ERROR ) Common::Com_Printf( "glGetError( ) = 0x%x\n", err );

	return true;
}

/*
===============
R_Shutdown
===============
*/
void Renderer::Shutdown( ) {

	Command::Cmd_RemoveCommand( "magickinfo" );
	Command::Cmd_RemoveCommand( "modellist" );
	Command::Cmd_RemoveCommand( "screenshot" );
	Command::Cmd_RemoveCommand( "imagelist" );
	Command::Cmd_RemoveCommand( "gl_strings" );
	Command::Cmd_RemoveCommand( "saveTextures" );

	Mod_FreeAll( );

	MaterialSystem::Shutdown( );

	//GL_ShutdownImages( );

	/*
	* * shut down OS specific OpenGL stuff like contexts, etc.
	*/
	GLimp_Shutdown( );
}



/*
@@@@@@@@@@@@@@@@@@@@@
R_BeginFrame
@@@@@@@@@@@@@@@@@@@@@
*/
void Renderer::BeginFrame( float camera_separation ) {

	gl_state.camera_separation = camera_separation;

	if( Common::vid_gamma.IsModified( ) ) Common::vid_gamma.ClearModified( );

#if 0
	GLimp_BeginFrame( camera_separation );

	/*
	* * go into 2D mode
	*/
	SetGL2D( );
#endif

	/*
	* * draw buffer stuff
	*/
#if 0
	if( gl_drawbuffer.IsModified( ) )
	{
		gl_drawbuffer.ClearModified( );

		if( gl_state.camera_separation == 0 || !gl_state.stereo_enabled )
		{
			if( Q_stricmp( gl_drawbuffer.GetString( ), "GL_FRONT" ) == 0 )
				glDrawBuffer( GL_FRONT );
			else
				glDrawBuffer( GL_BACK );
		}
	}
#endif

	/*
	* * texturemode stuff
	*/
#if 0
	if( gl_texturemode.IsModified( ) )
	{
		GL_TextureMode( gl_texturemode.GetString( ) );
		gl_texturemode.ClearModified( );
	}

	if( gl_texturealphamode.IsModified( ) )
	{
		GL_TextureAlphaMode( gl_texturealphamode.GetString( ) );
		gl_texturealphamode.ClearModified( );
	}

	if( gl_texturesolidmode.IsModified( ) )
	{
		GL_TextureSolidMode( gl_texturesolidmode.GetString( ) );
		gl_texturesolidmode.ClearModified( );
	}
#endif

	/*
	* * swapinterval stuff
	*/
	GL_UpdateSwapInterval( );

	//
	// clear screen if desired
	//
	Clear( );
}

/*
=============
R_SetPalette
=============
*/
#if 0
void Renderer::CinematicSetPalette( const unsigned char * palette ) {
}
#endif
/*
* * R_DrawBeam
*/
void Renderer::DrawBeam( entity_t * e ) {

#define NUM_BEAM_SEGS 6

	int	i;
	float r, g, b;

	Vec3 perpvec;
	Vec3 direction, normalized_direction;
	Vec3	start_points[ NUM_BEAM_SEGS ], end_points[ NUM_BEAM_SEGS ];
	Vec3 oldorigin, origin;

	oldorigin[ 0 ] = e->oldorigin[ 0 ];
	oldorigin[ 1 ] = e->oldorigin[ 1 ];
	oldorigin[ 2 ] = e->oldorigin[ 2 ];

	origin[ 0 ] = e->origin[ 0 ];
	origin[ 1 ] = e->origin[ 1 ];
	origin[ 2 ] = e->origin[ 2 ];

	normalized_direction[ 0 ] = direction[ 0 ] = oldorigin[ 0 ] - origin[ 0 ];
	normalized_direction[ 1 ] = direction[ 1 ] = oldorigin[ 1 ] - origin[ 1 ];
	normalized_direction[ 2 ] = direction[ 2 ] = oldorigin[ 2 ] - origin[ 2 ];

	if( normalized_direction.Normalize( ) == 0.0f ) return;

	perpvec = PerpendicularVector( normalized_direction );
	perpvec *= e->frame / 2.0f;

	for( i = 0; i < 6; i++ )
	{
		start_points[ i ] = RotatePointAroundVector( normalized_direction, perpvec, ( 360.0f/NUM_BEAM_SEGS )* i );
		start_points[ i ] = start_points[ i ] + origin;
		end_points[ i ] = start_points[ i ] + direction;
	}

	//glDisable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	glDepthMask( GL_FALSE );

	r = ( float )( ( d_8to24table[e->skinnum & 0xFF] ) & 0xFF );
	g = ( float )( ( d_8to24table[e->skinnum & 0xFF] >> 8 ) & 0xFF );
	b = ( float )( ( d_8to24table[e->skinnum & 0xFF] >> 16 ) & 0xFF );

	r *= 1/255.0f;
	g *= 1/255.0f;
	b *= 1/255.0f;

	glColor4f( r, g, b, e->alpha );

	glBegin( GL_TRIANGLE_STRIP );
	for( i = 0; i < NUM_BEAM_SEGS; i++ )
	{
		glVertex3fv( start_points[ i ].ToFloatPtr( ) );
		glVertex3fv( end_points[ i ].ToFloatPtr( ) );
		glVertex3fv( start_points[( i + 1 ) % NUM_BEAM_SEGS ].ToFloatPtr( ) );
		glVertex3fv( end_points[( i + 1 ) % NUM_BEAM_SEGS ].ToFloatPtr( ) );
	}
	glEnd( );

	//glEnable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );
	glDepthMask( GL_TRUE );
}

/*
=============
R_DrawWorld
=============
*/
void Renderer::DrawWorld( ) {

	entity_t	ent;

	if( !r_drawworld.GetBool( ) ) return;
	if( r_newrefdef.rdflags & RDF_NOWORLDMODEL ) return;

	currentmodel = r_worldmodel;

	//modelorg = r_newrefdef.vieworg;

	// auto cycle the world frame for texture animation
	memset( &ent, 0, sizeof( entity_t ) );
	ent.frame =( int )( r_newrefdef.time * 2 );
	currententity = &ent;

	gl_state.currenttextures[ 0 ] = gl_state.currenttextures[ 1 ] = -1;

	for( int i = 1; i < CollisionModel::CM_NumInlineModels( ); i++ ) {

		CModel * cModel = CollisionModel::CM_GetInlineModel( i );

		for( int j = 0; j < cModel->surfaces.Num( ); j++ ) {

			Surface * surface = cModel->surfaces[ j ];
			const int * indexes = surface->GetIndexes( );

			if( cModel->contents & CONTENTS_SOLID )
				cModel->materialIndexes[ j ]->Bind( );
			else {
				//glDisable( GL_TEXTURE_2D );
				glPolygonMode( GL_BACK, GL_LINE );
			}

			glBegin( GL_TRIANGLES );
			for( int k = 0; k < surface->GetNumIndexes( ); k += 3 ) {

				DrawVert vert0 = surface->operator[]( indexes[ k + 0 ] );
				DrawVert vert1 = surface->operator[]( indexes[ k + 1 ] );
				DrawVert vert2 = surface->operator[]( indexes[ k + 2 ] );

				//glColor4f( 1.0f, 0.0f, 0.0f, 0.5f );
				if( cModel->contents & CONTENTS_SOLID )
					glTexCoord2fv( vert0.st.ToFloatPtr( ) );

				glVertex3fv( vert0.xyz.ToFloatPtr( ) );

				//glColor4f( 0.0f, 1.0f, 0.0f, 0.5f );
				if( cModel->contents & CONTENTS_SOLID )
					glTexCoord2fv( vert1.st.ToFloatPtr( ) );

				glVertex3fv( vert1.xyz.ToFloatPtr( ) );

				//glColor4f( 0.0f, 0.0f, 1.0f, 0.5f );
				if( cModel->contents & CONTENTS_SOLID )
					glTexCoord2fv( vert2.st.ToFloatPtr( ) );

				glVertex3fv( vert2.xyz.ToFloatPtr( ) );
			}
			glEnd( );

			if( !( cModel->contents & CONTENTS_SOLID ) ) {
				glPolygonMode( GL_BACK, GL_FILL );
				//glEnable( GL_TEXTURE_2D );
			}
		}
	}
}
