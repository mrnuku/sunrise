#include "precompiled.h"
#pragma hdrstop

int				Renderer::r_dlightframecount;
Vec3			Renderer::pointcolor;
Plane			Renderer::lightplane;		// used as shadow plane
Vec3			Renderer::lightspot;
float			Renderer::s_blocklights[ 34 * 34 * 3 ];

/*
=============================================================================

DYNAMIC LIGHTS BLEND RENDERING

=============================================================================
*/

void Renderer::RenderDlight( dlight_t * light ) {

	int		i, j;
	float	a;
	Vec3	v;
	float	rad;

	rad = light->intensity * 0.35f;

	v = light->origin - r_origin;

	glBegin( GL_TRIANGLE_FAN );
	glColor3f( light->color[ 0 ]* 0.2f, light->color[ 1 ]* 0.2f, light->color[ 2 ]* 0.2f );
	for( i = 0; i<3; i++ )
		v[ i ] = light->origin[ i ] - vpn[ i ]* rad;
	glVertex3fv( v.ToFloatPtr( ) );
	glColor3f( 0, 0, 0 );
	for( i = 16; i>= 0; i-- )
	{
		a = i/16.0f * M_PI* 2;
		for( j = 0; j<3; j++ )
			v[ j ] = light->origin[ j ] + vright[ j ]* cos( a )* rad
				+ vup[ j ]* sin( a )* rad;
		glVertex3fv( v.ToFloatPtr( ) );
	}
	glEnd( );
}

/*
=============
R_RenderDlights
=============
*/
void Renderer::RenderDlights( ) {

	dlight_t	* l;

	if( !gl_flashblend.GetBool( ) ) return;

	r_dlightframecount = r_framecount + 1;	// because the count hasn't
											//  advanced yet for this frame
	glDepthMask( 0 );
	//glDisable( GL_TEXTURE_2D );
	//glShadeModel( GL_SMOOTH );
	glEnable( GL_BLEND );
	glBlendFunc( GL_ONE, GL_ONE );

	l = r_newrefdef.dlights;
	for( int i = 0; i < r_newrefdef.num_dlights; i++, l++ ) RenderDlight( l );

	glColor3f( 1, 1, 1 );
	glDisable( GL_BLEND );
	//glEnable( GL_TEXTURE_2D );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glDepthMask( 1 );
}


/*
=============================================================================

DYNAMIC LIGHTS

=============================================================================
*/

/*
=============
R_MarkLights
=============
*/
void Renderer::MarkLights( dlight_t * light, int bit, mnode_t * node ) {

#if 0
	Plane	 splitplane;
	float		dist;
	msurface_t	* surf;
	
	if( node->contents != -1 )
		return;

	splitplane = node->plane;
	dist =( light->origin * splitplane.Normal( ) ) - splitplane.Dist( );
	
	if( dist > light->intensity-DLIGHT_CUTOFF ) {

		MarkLights( light, bit, node->children[ 0 ] );
		return;
	}
	if( dist < -light->intensity+DLIGHT_CUTOFF ) {

		MarkLights( light, bit, node->children[ 1 ] );
		return;
	}
		
// mark the polygons
	surf = r_worldmodel->surfaces + node->firstsurface;
	for( int i = 0; i < node->numsurfaces; i++, surf++ ) {

		if( surf->dlightframe != r_dlightframecount ) {

			surf->dlightbits = 0;
			surf->dlightframe = r_dlightframecount;
		}
		surf->dlightbits |= bit;
	}

	MarkLights( light, bit, node->children[ 0 ] );
	MarkLights( light, bit, node->children[ 1 ] );
#endif
}


/*
=============
R_PushDlights
=============
*/
void Renderer::PushDlights( ) {

#if 0
	dlight_t	* l;

	if( gl_flashblend.GetBool( ) ) return;

	r_dlightframecount = r_framecount + 1;	// because the count hasn't
											//  advanced yet for this frame
	l = r_newrefdef.dlights;
	for( int i  = 0; i < r_newrefdef.num_dlights; i++, l++ ) MarkLights( l, 1<<i, r_worldmodel->nodes );
#endif
}


/*
=============================================================================

LIGHT SAMPLING

=============================================================================
*/

int Renderer::RecursiveLightPoint( struct mnode_s * node, Vec3 & start, Vec3 & end ) {

	float		front, back, frac;
	int			side;
	Plane	 plane;
	Vec3		mid;
	msurface_t	* surf;
	int			s, t, ds, dt;
	int			i;
	mtexinfo_t	* tex;
	byte		* lightmap;
	int			maps;
	int			r;

	if( node->contents != -1 )
		return -1;		// didn't hit anything
	
// calculate mid point

// FIXME: optimize for axial
	plane = node->plane;
	front = start * plane.Normal( ) - plane.Dist( );
	back = end * plane.Normal( ) - plane.Dist( );
	side = front < 0;
	
	if( ( back < 0 ) == side ) return RecursiveLightPoint( node->children[ side ], start, end );

	frac = front /( front-back );
	mid[ 0 ] = start[ 0 ] +( end[ 0 ] - start[ 0 ] ) * frac;
	mid[ 1 ] = start[ 1 ] +( end[ 1 ] - start[ 1 ] ) * frac;
	mid[ 2 ] = start[ 2 ] +( end[ 2 ] - start[ 2 ] ) * frac;
	
// go down front side	
	r = RecursiveLightPoint( node->children[ side ], start, mid );
	if( r >= 0 ) return r;		// hit something
		
	if( ( back < 0 ) == side ) return -1;		// didn't hit anuthing
		
// check for impact on this node
	lightspot = mid;
	lightplane = plane;

	surf = r_worldmodel->surfaces + node->firstsurface;
	for( i = 0; i < node->numsurfaces; i++, surf++ ) {

		if( surf->flags &( SURF_DRAWTURB | SURF_DRAWSKY ) ) continue;	// no lightmaps

		tex = surf->texinfo;
		
		s = ( int )( mid * tex->vecs[ 0 ] ) + tex->vecs[ 0 ][ 3 ];
		t = ( int )( mid * tex->vecs[ 1 ] ) + tex->vecs[ 1 ][ 3 ];

		if( s < surf->texturemins[ 0 ] ||
		t < surf->texturemins[ 1 ] )
			continue;
		
		ds = s - surf->texturemins[ 0 ];
		dt = t - surf->texturemins[ 1 ];
		
		if( ds > surf->extents[ 0 ] || dt > surf->extents[ 1 ] )
			continue;

		if( !surf->samples )
			return 0;

		ds >>= 4;
		dt >>= 4;

		lightmap = surf->samples;
		pointcolor = vec3_origin;
		if( lightmap )
		{
			Vec3 scale;

			lightmap += 3* ( dt * ( ( surf->extents[ 0 ]>>4 )+1 ) + ds );

			for( maps = 0; maps < MAXLIGHTMAPS && surf->styles[ maps ] != 255;
					maps++ )
			{
				for( i = 0; i<3; i++ )
					scale[ i ] = gl_modulate.GetFloat( )* r_newrefdef.lightstyles[ surf->styles[maps] ].rgb[ i ];

				pointcolor[ 0 ] += lightmap[ 0 ] * scale[ 0 ] * ( 1.0f/255 );
				pointcolor[ 1 ] += lightmap[ 1 ] * scale[ 1 ] * ( 1.0f/255 );
				pointcolor[ 2 ] += lightmap[ 2 ] * scale[ 2 ] * ( 1.0f/255 );
				lightmap += 3* ( ( surf->extents[ 0 ]>>4 )+1 ) *
						( ( surf->extents[ 1 ]>>4 )+1 );
			}
		}
		
		return 1;
	}

// go down back side
	return RecursiveLightPoint( node->children[ !side ], mid, end );
}

/*
===============
R_LightPoint
===============
*/
void Renderer::LightPoint( Vec3 & p, Vec3 * color ) {

#if 0
	Vec3		end;
	int			lnum;
	dlight_t	* dl;
	float		light;
	Vec3		dist;
	float		add;
	
	if( !r_worldmodel->lightdata ) {

		*color = Vec3( 1.0f, 1.0f, 1.0f );
		return;
	}
	
	end = p;
	end[ 2 ] -= 2048.0f;
	
	if( RecursiveLightPoint( r_worldmodel->nodes, p, end ) == -1 ) *color = vec3_origin;
	else *color = pointcolor;

	//
	// add dynamic lights
	//
	light = 0;
	dl = r_newrefdef.dlights;
	for( lnum = 0; lnum<r_newrefdef.num_dlights; lnum++, dl++ )
	{
		dist = currententity->origin - dl->origin;
		add = dl->intensity - dist.Length( );
		add *=( 1.0f / 256.0f );
		if( add > 0.0f ) * color = color->Magnitude( add, dl->color );
	}

	* color *= gl_modulate.GetFloat( );

#endif
}


//===================================================================

/*
===============
R_AddDynamicLights
===============
*/
void Renderer::AddDynamicLights( msurface_t * surf ) {

	int			sd, td;
	float		fdist, frad, fminlight;
	Vec3		impact, local;
	int			s, t;
	int			i;
	int			smax, tmax;
	mtexinfo_t	* tex;
	dlight_t	* dl;
	float		* pfBL;
	float		fsacc, ftacc;

	smax =( surf->extents[ 0 ] >> 4 ) + 1;
	tmax =( surf->extents[ 1 ] >> 4 ) + 1;
	tex = surf->texinfo;

	for( int lnum = 0; lnum < r_newrefdef.num_dlights; lnum++ ) {

		if( !( surf->dlightbits &( 1 << lnum ) ) )
			continue;		// not lit by this light

		dl = &r_newrefdef.dlights[ lnum ];
		frad = dl->intensity;
		fdist =( dl->origin * surf->plane.Normal( ) ) - surf->plane.Dist( );
		frad -= fabs( fdist );
		// rad is now the highest intensity on the plane

		fminlight = DLIGHT_CUTOFF;	// FIXME: make configurable?
		if( frad < fminlight ) continue;
		fminlight = frad - fminlight;

		for( i = 0; i < 3; i++ ) impact[ i ] =( dl->origin[ i ] - surf->plane.Normal( )[ i ] ) * fdist;

		local[ 0 ] =( impact * tex->vecs[ 0 ] ) + tex->vecs[ 0 ][ 3 ] - surf->texturemins[ 0 ];
		local[ 1 ] =( impact * tex->vecs[ 1 ] ) + tex->vecs[ 1 ][ 3 ] - surf->texturemins[ 1 ];

		pfBL = s_blocklights;
		for( t = 0, ftacc = 0; t<tmax; t++, ftacc += 16.0f ) {

			td = ( int )abs( local[ 1 ] - ftacc );

			for( s = 0, fsacc = 0; s < smax; s++, fsacc += 16.0f, pfBL += 3 ) {

				sd = ( int )abs( local[ 0 ] - fsacc );

				fdist = ( sd > td ) ? ( float )( sd + ( td >> 1 ) ) : ( float )( td + ( sd >> 1 ) );

				if( fdist < fminlight ) {

					pfBL[ 0 ] +=( frad - fdist ) * dl->color[ 0 ];
					pfBL[ 1 ] +=( frad - fdist ) * dl->color[ 1 ];
					pfBL[ 2 ] +=( frad - fdist ) * dl->color[ 2 ];
				}
			}
		}
	}
}


/*
* * R_SetCacheState
*/
void Renderer::SetCacheState( msurface_t * surf ) {

	for( int maps = 0; maps < MAXLIGHTMAPS && surf->styles[ maps ] != 255; maps++ )
		surf->cached_light[ maps ] = r_newrefdef.lightstyles[ surf->styles[ maps ] ].white;
}

/*
===============
R_BuildLightMap

Combine and scale multiple lightmaps into the floating format in blocklights
===============
*/
void Renderer::BuildLightMap( msurface_t * surf, byte * dest, int stride ) {

	int			smax, tmax;
	int			r, g, b, a, max;
	int			i, j, size;
	byte		* lightmap;
	float		scale[ 4 ];
	int			nummaps;
	float		* bl;
	lightstyle_t	* style;

	if( surf->texinfo->flags &( SURF_SKY | SURF_TRANS33 | SURF_TRANS66 | SURF_WARP ) )
		Common::Com_Error( ERR_DROP, "R_BuildLightMap called for non-lit surface" );

	smax =( surf->extents[ 0 ] >> 4 ) + 1;
	tmax =( surf->extents[ 1 ] >> 4 ) + 1;
	size = smax* tmax;
	if( size >( sizeof( s_blocklights ) >> 4 ) ) Common::Com_Error( ERR_DROP, "Bad s_blocklights size" );

// set to full bright if no light data
	if( !surf->samples ) {

		for( i = 0; i < size * 3; i++ ) s_blocklights[ i ] = 255;

		for( int maps = 0; maps < MAXLIGHTMAPS && surf->styles[ maps ] != 255; maps++ )
			style = &r_newrefdef.lightstyles[ surf->styles[ maps ] ];

	} else {

		// count the # of maps
		for( nummaps = 0; nummaps < MAXLIGHTMAPS && surf->styles[ nummaps ] != 255; nummaps++ );

		lightmap = surf->samples;

		// add all the lightmaps
		if( nummaps == 1 ) {

			for( int maps = 0; maps < MAXLIGHTMAPS && surf->styles[ maps ] != 255; maps++ ) {

				bl = s_blocklights;

				for( i = 0; i < 3; i ++ )
					scale[ i ] = gl_modulate.GetFloat( ) * r_newrefdef.lightstyles[ surf->styles[ maps ] ].rgb[ i ];

				if( scale[ 0 ] == 1.0f && scale[ 1 ] == 1.0f && scale[ 2 ] == 1.0f ) {

					for( i = 0; i < size; i++, bl += 3 ) {

						bl[ 0 ] = lightmap[ i * 3 + 0 ];
						bl[ 1 ] = lightmap[ i * 3 + 1 ];
						bl[ 2 ] = lightmap[ i * 3 + 2 ];
					}
				} else {

					for( i = 0; i < size; i++, bl += 3 ) {

						bl[ 0 ] = lightmap[ i * 3 + 0 ] * scale[ 0 ];
						bl[ 1 ] = lightmap[ i * 3 + 1 ] * scale[ 1 ];
						bl[ 2 ] = lightmap[ i * 3 + 2 ] * scale[ 2 ];
					}
				}
				lightmap += size * 3;		// skip to next lightmap
			}
		} else {

			memset( s_blocklights, 0, sizeof( s_blocklights[ 0 ] ) * size * 3 );

			for( int maps = 0; maps < MAXLIGHTMAPS && surf->styles[ maps ] != 255; maps++ ) {

				bl = s_blocklights;

				for( i = 0; i < 3; i ++ )
					scale[ i ] = gl_modulate.GetFloat( ) * r_newrefdef.lightstyles[ surf->styles[ maps ] ].rgb[ i ];

				if( scale[ 0 ] == 1.0f && scale[ 1 ] == 1.0f && scale[ 2 ] == 1.0f ) {

					for( i = 0; i < size; i++, bl += 3 ) {

						bl[ 0 ] += lightmap[ i * 3 + 0 ];
						bl[ 1 ] += lightmap[ i * 3 + 1 ];
						bl[ 2 ] += lightmap[ i * 3 + 2 ];
					}
				} else {

					for( i = 0; i < size; i++, bl += 3 ) {

						bl[ 0 ] += lightmap[ i * 3 + 0 ] * scale[ 0 ];
						bl[ 1 ] += lightmap[ i * 3 + 1 ] * scale[ 1 ];
						bl[ 2 ] += lightmap[ i * 3 + 2 ] * scale[ 2 ];
					}
				}
				lightmap += size* 3;		// skip to next lightmap
			}
		}

	// add all the dynamic lights
		if( surf->dlightframe == r_framecount ) AddDynamicLights( surf );
	}

// put into texture format
	stride -=( smax << 2 );
	bl = s_blocklights;

	for( i = 0; i<tmax; i++, dest += stride ) {

		for( j = 0; j < smax; j++ ) {
				
			r =( int )( bl[ 0 ] );
			g =( int )( bl[ 1 ] );
			b =( int )( bl[ 2 ] );

			// catch negative lights
			if( r < 0 ) r = 0;
			if( g < 0 ) g = 0;
			if( b < 0 ) b = 0;

			/*
			* * determine the brightest of the three color components
			*/
			if( r > g ) max = r;
			else max = g;
			if( b > max ) max = b;

			/*
			* * alpha is ONLY used for the mono lightmap case.  For this reason
			* * we set it to the brightest of the color components so that 
			* * things don't get too dim.
			*/
			a = max;

			/*
			* * rescale all the color components if the intensity of the greatest
			* * channel exceeds 1.0f
			*/
			if( max > 255 ) {

				float t = 255.0f / max;

				r *= t;
				g *= t;
				b *= t;
				a *= t;
			}

			dest[ 0 ] = r;
			dest[ 1 ] = g;
			dest[ 2 ] = b;
			dest[ 3 ] = a;

			bl += 3;
			dest += 4;
		}
	}
}

