#include "precompiled.h"
#pragma hdrstop

// // //// // //// // //// //
// MapBrushSide
// //// // //// // //// //

MapBrushSide::MapBrushSide( ) {
	d_plane.Zero( );
	d_texMat[ 0 ].Zero( );
	d_texMat[ 1 ].Zero( );
	d_origin.Zero( );
}

void MapBrushSide::ComputeAxisBase( const Vec3 & normal, Vec3 & texS, Vec3 & texT ) {

	Vec3 n(	( fabs( normal[ 0 ] ) < 1e-6f ) ? 0.0f : normal[ 0 ],
			( fabs( normal[ 1 ] ) < 1e-6f ) ? 0.0f : normal[ 1 ],
			( fabs( normal[ 2 ] ) < 1e-6f ) ? 0.0f : normal[ 2 ] );

	float RotY = -atan2( n[ 2 ], sqrtf( n[ 1 ] * n[ 1 ] + n[ 0 ] * n[ 0 ]) );
	float RotZ = atan2( n[ 1 ], n[ 0 ] );
	// rotate (0,1,0) and (0,0,1) to compute texS and texT
	texS[ 0 ] = -sin( RotZ );
	texS[ 1 ] = cos( RotZ );
	texS[ 2 ] = 0;
	// the texT vector is along -Z ( T texture coorinates axis )
	texT[ 0 ] = -sin( RotY ) * cos( RotZ );
	texT[ 1 ] = -sin( RotY ) * sin( RotZ );
	texT[ 2 ] = -cos(RotY );
}

void MapBrushSide::GetTextureVectors( Vec4 v[ 2 ] ) const {
	Vec3 texX, texY;
	ComputeAxisBase( d_plane.Normal( ), texX, texY );
	for( int i = 0; i < 2; i++ ) {
		v[ i ][ 0 ] = texX[ 0 ] * d_texMat[ i ][ 0 ] + texY[ 0 ] * d_texMat[ i ][ 1 ];
		v[ i ][ 1 ] = texX[ 1 ] * d_texMat[ i ][ 0 ] + texY[ 1 ] * d_texMat[ i ][ 1 ];
		v[ i ][ 2 ] = texX[ 2 ] * d_texMat[ i ][ 0 ] + texY[ 2 ] * d_texMat[ i ][ 1 ];
		v[ i ][3] = d_texMat[ i ][ 2 ] + ( d_origin * v[ i ].ToVec3( ) );
	}
}

// // //// // //// // //// //
// MapPatch
// //// // //// // //// //

MapPatch::MapPatch( ) {
	d_horzSubdivisions		= 0;
	d_vertSubdivisions		= 0;
	d_explicitSubdivisions	= false;

	width					= 0;
	height					= 0;
	maxWidth				= 0;
	maxHeight				= 0;
	expanded				= false;
}

bool MapPatch::Parse( Lexer & src, const Vec3 & origin, bool patchDef3, float version ) {
	float		info[ 7 ];
	Token		token;
	if( !src.ExpectTokenString( "{" ) )
		return false;
	// read the material (we had an implicit 'textures/' in the old format...)
	if( !src.ReadToken( &token ) ) {
		src.Error( "MapPatch::Parse: unexpected EOF" );
		return false;
	}
	// Parse it
	if( patchDef3 ) {
		if( !src.Parse1DMatrix( 7, info ) ) {
			src.Error( "MapPatch::Parse: unable to Parse patchDef3 info" );
			return false;
		}
	} else {
		if( !src.Parse1DMatrix( 5, info ) ) {
			src.Error( "MapPatch::Parse: unable to parse patchDef2 info" );
			return false;
		}
	}
	maxWidth = ( int )info[ 0 ];
	maxHeight = ( int )info[ 1 ];
	verts.SetNum( ( int )info[ 0 ] * ( int )info[ 1 ] );
	SetSize( ( int )info[ 0 ], ( int )info[ 1 ] );
	if( version < 2.0f )
		SetMaterial( "textures/" + token );
	else
		SetMaterial( token );
	if( patchDef3 ) {
		d_horzSubdivisions = ( int )info[ 2 ];
		d_vertSubdivisions = ( int )info[ 3 ];
		d_explicitSubdivisions = true;
	}
	if( GetWidth( ) < 0 || GetHeight( ) < 0 ) {
		src.Error( "MapPatch::Parse: bad size" );
		return false;
	}
	// these were written out in the wrong order, IMHO
	if( !src.ExpectTokenString( "(" ) ) {
		src.Error( "MapPatch::Parse: bad patch vertex data" );
		return false;
	}
	for( int j = 0; j < GetWidth( ); j++ ) {
		if( !src.ExpectTokenString( "(" ) ) {
			src.Error( "MapPatch::Parse: bad vertex row data" );
			return false;
		}
		for( int i = 0; i < GetHeight( ); i++ ) {
			float v[ 5 ];
			if( !src.Parse1DMatrix( 5, v ) ) {
				src.Error( "MapPatch::Parse: bad vertex column data" );
				return false;
			}
			DrawVert & vert = ( *this )[ i * GetWidth( ) + j ];
			vert.xyz[ 0 ] = v[ 0 ] - origin[ 0 ];
			vert.xyz[ 1 ] = v[ 1 ] - origin[ 1 ];
			vert.xyz[ 2 ] = v[ 2 ] - origin[ 2 ];
			vert.st[ 0 ] = v[ 3 ];
			vert.st[ 1 ] = v[ 4 ];
		}
		if( !src.ExpectTokenChar( ')' ) ) {
			src.Error( "MapPatch::Parse: unable to parse patch control points" );
			return false;
		}
	}
	if( !src.ExpectTokenChar( ')' ) ) {
		src.Error( "MapPatch::Parse: unable to parse patch control points, no closure" );
		return false;
	}
	// read any key/value pairs
	while( src.ReadToken( &token ) ) {
		if( token == '}' ) {
			src.ExpectTokenChar( '}' );
			break;
		}
		if( token.type == TT_STRING ) {
			Str key = token;
			src.ExpectTokenType( TT_STRING, 0, &token );
			d_epairs.Set( key, token );
		}
	}
	return true;
}

bool MapPatch::Write( FileBase * fp, int primitiveNum, const Vec3 & origin ) const {
	if( d_explicitSubdivisions ) {
		fp->WriteFloatString( "// primitive %d\n{\n patchDef3\n {\n", primitiveNum );
		fp->WriteFloatString( "  \"%s\"\n  ( %d %d %d %d 0 0 0 )\n", GetMaterial( ), GetWidth( ), GetHeight( ), d_horzSubdivisions, d_horzSubdivisions );
	} else {
		fp->WriteFloatString( "// primitive %d\n{\n patchDef2\n {\n", primitiveNum );
		fp->WriteFloatString( "  \"%s\"\n  ( %d %d 0 0 0 )\n", GetMaterial( ), GetWidth( ), GetHeight( ));
	}
	fp->WriteFloatString( "  (\n" );
	for( int i = 0; i < GetWidth( ); i++ ) {
		fp->WriteFloatString( "   ( " );
		for( int j = 0; j < GetHeight( ); j++ ) {
			const DrawVert * v = &verts[ j * GetWidth( ) + i ];
			fp->WriteFloatString( " ( %f %f %f %f %f )", v->xyz[ 0 ] + origin[ 0 ], v->xyz[ 1 ] + origin[ 1 ], v->xyz[ 2 ] + origin[ 2 ], v->st[ 0 ], v->st[ 1 ] );
		}
		fp->WriteFloatString( " )\n" );
	}
	fp->WriteFloatString( "  )\n }\n}\n" );
	return true;
}

uint MapPatch::GetGeometryCRC( ) const {
	uint crc = d_horzSubdivisions ^ d_horzSubdivisions;
	for( int i = 0; i < GetWidth( ); i++ ) {
		for( int j = 0; j < GetHeight( ); j++ ) {
			crc ^= FloatCRC( verts[j * GetWidth( ) + i].xyz.x );
			crc ^= FloatCRC( verts[j * GetWidth( ) + i].xyz.y );
			crc ^= FloatCRC( verts[j * GetWidth( ) + i].xyz.z );
		}
	}
	crc ^= StringCRC( GetMaterial( ) );
	return crc;
}

// // //// // //// // //// //
// MapBrush
// //// // //// // //// //

void MapBrush::ComputeSideVerts( ) {
	FixedWinding winding;
	for( int i = 0; i < d_sides.Num( ); i++ ) {
		d_sides[ i ].d_visible = true;
		winding.BaseForPlane( d_sides[ i ].d_plane, MAX_WORLD_SIZE );
		for( int j = 0; j < d_sides.Num( ); j++ ) {
			if( j == i )
				continue;
			if( !winding.ClipInPlace( -d_sides[ j ].d_plane, ON_EPSILON, true ) )
				break;
		}
		for( int j = 0; j < winding.GetNumPoints( ); j++ ) {
			int k;
			for( k = 0; k < d_verts.Num( ); k++ ) {
				if( d_verts[ k ].Compare( winding[ j ].ToVec3( ), 0.5f ) )
					break;
			}
			if( k >= d_verts.Num( ) )
				k = d_verts.Append( winding[ j ].ToVec3( ) );
			d_sides[ i ].d_points.Append( k );
		}
	}
	d_bounds.Clear( );
	for( int i = 0; i < d_verts.Num( ); i++ ) {
		d_bounds.AddPoint( d_verts[ i ] );
		//for( int j = 0; j < d_sides.Num( ); j++ ) {
		//	assert( d_sides[ j ].d_plane.Side( d_verts[ i ], 0.1f ) != PLANESIDE_FRONT );
		//}
	}
}

int MapBrush::HideIncludedSides( const Bounds & bounds, const MapBrush & other ) {
	int pointsNS = 0;
	//if( !bounds.IntersectsBounds( d_bounds ) )
	//	return 0;
	for( int i = 0; i < d_sides.Num( ); i++ ) {
		if( !d_sides[ i ].d_visible )
			continue;
		bool visible = false;
		for( int j = 0; j < d_sides[ i ].d_points.Num( ) && !visible; j++ ) {
			for( int k = 0; k < other.d_sides.Num( ) && !visible; k++ )
				visible = other.d_sides[ k ].d_plane.Side( d_verts[ d_sides[ i ].d_points[ j ] ], 0.1f ) == PLANESIDE_FRONT;
			//visible = !bounds.ContainsPoint( d_verts[ d_sides[ i ].d_points[ j ] ] );
		}
		if( visible )
			continue;
		d_sides[ i ].d_visible = visible;
		pointsNS++;
	}
	return pointsNS;
}

bool MapBrush::Parse( Lexer & src, const Vec3 & origin, bool newFormat, float version ) {
	Vec3 planepts[ 3 ];
	Token token;
	DList< MapBrushSide > sides;
	Dict epairs;
	if( !src.ExpectTokenChar( '{' ) )
		return false;
	do {
		if( !src.ReadToken( &token ) ) {
			src.Error( "MapBrush::Parse: unexpected EOF" );
			return false;
		}
		if( token == '}' )
			break;
		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is a brace
			if( token == '(' )
				break;
			// the token should be a key string for a key/value pair
			if( token.type != TT_STRING ) {
				src.Error( "MapBrush::Parse: unexpected %s, expected ( or epair key string", token.c_str( ) );
				return false;
			}
			Str key = token;
			if( !src.ReadTokenOnLine( &token ) || token.type != TT_STRING ) {
				src.Error( "MapBrush::Parse: expected epair value string not found" );
				return false;
			}
			epairs.Set( key, token );
			// try to read the next key
			if( !src.ReadToken( &token ) ) {
				src.Error( "MapBrush::Parse: unexpected EOF" );
				return false;
			}
		} while( 1 );
		src.UnreadToken( &token );
		MapBrushSide & side = sides.Alloc( );
		if( newFormat ) {
			if( !src.Parse1DMatrix( 4, side.d_plane.ToFloatPtr( ) ) ) {
				src.Error( "MapBrush::Parse: unable to read brush side plane definition" );
				return false;
			}
		} else {
			// read the three point plane definition
			if(!src.Parse1DMatrix( 3, planepts[ 0 ].ToFloatPtr( ) ) ||
				!src.Parse1DMatrix( 3, planepts[ 1 ].ToFloatPtr( ) ) ||
				!src.Parse1DMatrix( 3, planepts[ 2 ].ToFloatPtr( ) ) ) {
				src.Error( "MapBrush::Parse: unable to read brush side plane definition" );
				return false;
			}
			planepts[ 0 ] -= origin;
			planepts[ 1 ] -= origin;
			planepts[ 2 ] -= origin;
			side.d_plane.FromPoints( planepts[ 0 ], planepts[ 1 ], planepts[ 2 ] );
		}
		// read the texture matrix, this is odd, because the texmat is 2D relative to default planar texture axis
		if( !src.ExpectTokenChar( '(' ) ) {
			src.Error( "MapBrush::Parse: wrong syntax" );
			return false;
		}
		if( !src.Parse1DMatrix( 3, side.d_texMat[ 0 ].ToFloatPtr( ) ) ) {
			src.Error( "MapBrush::Parse: unable to read brush side texture matrix 1" );
			return false;
		}
		if( !src.Parse1DMatrix( 3, side.d_texMat[ 1 ].ToFloatPtr( ) ) ) {
			src.Error( "MapBrush::Parse: unable to read brush side texture matrix 2" );
			return false;
		}
		if( !src.ExpectTokenChar( ')' ) ) {
			src.Error( "MapBrush::Parse: wrong syntax" );
			return false;
		}
		/*if( !src.Parse2DMatrix( 2, 3, side->texMat[ 0 ].ToFloatPtr( ) ) ) {
			src.Error( "MapBrush::Parse: unable to read brush side texture matrix" );
			return NULL;
		}*/
		side.d_origin = origin;		
		// read the material
		if( !src.ReadTokenOnLine( &token ) ) {
			src.Error( "MapBrush::Parse: unable to read brush side material" );
			return false;
		}
		// we had an implicit 'textures/' in the old format...
		if( version < 2.0f )
			side.d_material = "textures/" + token;
		else
			side.d_material = token;
		// Q2 allowed override of default flags and values, but we don't any more
		if( src.ReadTokenOnLine( &token ) ) {
			if( src.ReadTokenOnLine( &token ) ) {
				if( src.ReadTokenOnLine( &token ) ) {
				}
			}
		}
	} while( 1 );
	if( !src.ExpectTokenChar( '}' ) )
		return false;
	d_sides = sides;
	d_epairs = epairs;
	ComputeSideVerts( );
	return true;
}

bool MapBrush::ParseQ3( Lexer & src, const Vec3 & origin ) {
	int shift[ 2 ], rotate;
	float scale[ 2 ];
	Vec3 planepts[ 3 ];
	Token token;
	DList< MapBrushSide > sides;
	Dict epairs;
	do {
		if( src.ExpectTokenChar( '}' ) )
			break;
		MapBrushSide & side = sides.Alloc( );
		// read the three point plane definition
		if(!src.Parse1DMatrix( 3, planepts[ 0 ].ToFloatPtr( ) ) ||
			!src.Parse1DMatrix( 3, planepts[ 1 ].ToFloatPtr( ) ) ||
			!src.Parse1DMatrix( 3, planepts[ 2 ].ToFloatPtr( ) ) ) {
			src.Error( "MapBrush::ParseQ3: unable to read brush side plane definition" );
			return false;
		}
		planepts[ 0 ] -= origin;
		planepts[ 1 ] -= origin;
		planepts[ 2 ] -= origin;
		side.d_plane.FromPoints( planepts[ 0 ], planepts[ 1 ], planepts[ 2 ] );
		// read the material
		if( !src.ReadTokenOnLine( &token ) ) {
			src.Error( "MapBrush::ParseQ3: unable to read brush side material" );
			return false;
		}
		// we have an implicit 'textures/' in the old format
		side.d_material = "textures/" + token;
		// read the texture shift, rotate and scale
		shift[ 0 ] = src.ParseInt( );
		shift[ 1 ] = src.ParseInt( );
		rotate = src.ParseInt( );
		scale[ 0 ] = src.ParseFloat( );
		scale[ 1 ] = src.ParseFloat( );
		side.d_texMat[ 0 ] = Vec3( 0.03125f, 0.0f, 0.0f );
		side.d_texMat[ 1 ] = Vec3( 0.0f, 0.03125f, 0.0f );
		side.d_origin = origin;		
		// Q2 allowed override of default flags and values, but we don't any more
		if( src.ReadTokenOnLine( &token ) ) {
			if( src.ReadTokenOnLine( &token ) ) {
				if( src.ReadTokenOnLine( &token ) ) {
				}
			}
		}
	} while( 1 );
	d_sides = sides;
	d_epairs = epairs;
	ComputeSideVerts( );
	return true;
}

bool MapBrush::Write( FileBase * fp, int primitiveNum, const Vec3 & origin ) const {
	fp->WriteFloatString( "// primitive %d\n{\n brushDef3\n {\n", primitiveNum );
	// write brush epairs
	for( int i = 0; i < d_epairs.GetNumKeyVals( ); i++)
		fp->WriteFloatString( "  \"%s\" \"%s\"\n", d_epairs.GetKeyVal(i)->GetKey( ).c_str( ), d_epairs.GetKeyVal(i)->GetValue( ).c_str( ));
	// write brush sides
	for( int i = 0; i < GetNumSides( ); i++ ) {
		const MapBrushSide & side = GetSide( i );
		fp->WriteFloatString( "  ( %f %f %f %f ) ", side.d_plane[ 0 ], side.d_plane[ 1 ], side.d_plane[ 2 ], side.d_plane[3] );
		fp->WriteFloatString( "( ( %f %f %f ) ( %f %f %f ) ) \"%s\" 0 0 0\n",
			side.d_texMat[ 0 ][ 0 ], side.d_texMat[ 0 ][ 1 ], side.d_texMat[ 0 ][ 2 ],
			side.d_texMat[ 1 ][ 0 ], side.d_texMat[ 1 ][ 1 ], side.d_texMat[ 1 ][ 2 ],
			side.d_material.c_str( ) );
	}
	fp->WriteFloatString( " }\n}\n" );
	return true;
}

uint MapBrush::GetGeometryCRC( ) const {
	uint crc = 0;
	for( int i = 0; i < GetNumSides( ); i++ ) {
		const MapBrushSide & mapSide = GetSide( i );
		for( int j = 0; j < 4; j++ )
			crc ^= FloatCRC( mapSide.GetPlane( )[ j ] );
		crc ^= StringCRC( mapSide.GetMaterial( ) );
	}
	return crc;
}

// // //// // //// // //// //
// MapEntity
// //// // //// // //// //

void MapEntity::BrushSideVisibilityTest( ) {
	int pointsNS = 0;
	int pointAll = 0;
	if( !d_brushList.Num( ) )
		return;
	for( int i = 0; i < d_brushList.Num( ); i++ ) {
		MapBrush & brush1 = d_brushList[ i ];
		Bounds bounds1 = brush1.GetBounds( ).Expand( 0.1f );
		for( int j = 0; j < d_brushList.Num( ); j++ ) {
			if( i == j )
				continue;
			MapBrush & brush2 = d_brushList[ j ];
			pointsNS += brush2.HideIncludedSides( bounds1, brush1 );
			pointAll += brush2.GetNumSides( );
		}
	}
	Common::Com_DPrintf( "%i (%.2f%%%%) sides eliminated\n", pointsNS, ( ( float )pointsNS / ( float )pointAll ) * 1000.0f );
}

MapEntity::MapEntity( ) {
	d_epairs.SetHashSize( 64 );
}

bool MapEntity::Parse( Lexer & src, bool worldSpawn, float version ) {
	Token	token;
	bool worldent;
	Vec3 origin;
	double v1, v2, v3;
	if( !src.ReadToken( &token ) )
		return false;
	if( token != '{' ) {
		src.Error( "MapEntity::Parse: { not found, found %s", token.c_str( ) );
		return false;
	}
	origin.Zero( );
	worldent = false;
	do {
		if( !src.ReadToken( &token ) ) {
			src.Error( "MapEntity::Parse: EOF without closing brace" );
			return false;
		}
		if( token == '}' )
			break;
		if( token == '{' ) {
			// parse a brush or patch
			if( !src.ReadToken( &token ) ) {
				src.Error( "MapEntity::Parse: unexpected EOF" );
				return false;
			}
			if( worldent )
				origin.Zero( );
			// if is it a brush: brush, brushDef, brushDef2, brushDef3
			if( token.IcmpnFast( "brush", 5 ) ) {
				if( !d_brushList.Alloc( ).Parse( src, origin, ( token.IcmpFast( "brushDef2" ) || token.IcmpFast( "brushDef3" ) ), version ) ) {
					d_brushList.RemoveLast( );
					return false;
				}
			}
			// if is it a patch: patchDef2, patchDef3
			else if( token.IcmpnFast( "patch", 5 ) ) {
				if( !d_patchList.Alloc( ).Parse( src, origin, token.IcmpFast( "patchDef3" ), version ) ) {
					d_patchList.RemoveLast( );
					return false;
				}
			}
			// assume it's a brush in Q3 or older style
			else {
				src.UnreadToken( &token );
				if( !d_brushList.Alloc( ).ParseQ3( src, origin ) ) {
					d_brushList.RemoveLast( );
					return false;
				}
			}
		} else {
			Str key, value;
			// parse a key / value pair
			key = token;
			src.ReadTokenOnLine( &token );
			value = token;
			// strip trailing spaces that sometimes get accidentally added in the editor
			value.StripTrailingWhitespace( );
			key.StripTrailingWhitespace( );
			d_epairs.Set( key, value );
			if( key.IcmpFast( "origin" ) ) {
				// scanf into doubles, then assign, so it is idVec size independent
				v1 = v2 = v3 = 0;
				sscanf( value, "%lf %lf %lf", &v1, &v2, &v3 );
				origin.x = ( float )v1;
				origin.y = ( float )v2;
				origin.z = ( float )v3;
			}
			else if( key.IcmpFast( "classname" ) && value.IcmpFast( "worldspawn" ) )
				worldent = true;
		}
	} while( 1 );
	//BrushSideVisibilityTest( );
	return true;
}

bool MapEntity::Write( FileBase * fp, int entityNum ) const {
	fp->WriteFloatString( "// entity %d\n{\n", entityNum );
	// write entity epairs
	for( int i = 0; i < d_epairs.GetNumKeyVals( ); i++ )
		fp->WriteFloatString( "\"%s\" \"%s\"\n", d_epairs.GetKeyVal(i)->GetKey( ).c_str( ), d_epairs.GetKeyVal(i)->GetValue( ).c_str( ));
	Vec3 origin;
	d_epairs.GetVector( "origin", "0 0 0", origin );
	// write pritimives
	for( int i = 0; i < GetNumBrushes( ); i++ )
		GetBrush( i ).Write( fp, i, origin );
	for( int i = 0; i < GetNumPatches( ); i++ )
		GetPatch( i ).Write( fp, i, origin );
	fp->WriteFloatString( "}\n" );
	return true;
}

void MapEntity::RemovePrimitiveData( ) {
	d_brushList.Clear( );
	d_patchList.Clear( );
}

void MapEntity::AddPrimitiveData( const MapEntity & other ) {
	d_brushList.Append( other.d_brushList );
	d_patchList.Append( other.d_patchList );
}

uint MapEntity::GetGeometryCRC( ) const {
	uint crc = 0;
	for( int i = 0; i < GetNumBrushes( ); i++ )
		crc ^= GetBrush( i ).GetGeometryCRC( );
	for( int i = 0; i < GetNumPatches( ); i++ )
		crc ^= GetPatch( i ).GetGeometryCRC( );
	return crc;
}

// // //// // //// // //// //
// MapFile
// //// // //// // //// //

MapFile::MapFile( ) {
	d_version			= CURRENT_MAP_VERSION;
	d_fileTime			= 0;
	d_geometryCRC		= 0;
	d_entities.SetGranularity( 256 );
	d_hasPrimitiveData	= false;
}

bool MapFile::Parse( const Str & filename, bool ignoreRegion ) {
	// no string concatenation for epairs and allow path names for materials
	Lexer src( LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );
	Token token;
	Str fullName;
	d_name = filename;
	d_name.StripFileExtension( );
	fullName = d_name;
	d_hasPrimitiveData = false;
	if( !ignoreRegion ) {
		// try loading a .reg file first
		fullName.SetFileExtension( "reg" );
		src.LoadFile( fullName );
	}
	if( !src.IsLoaded( ) ) {
		// now try a .map file
		fullName.SetFileExtension( "map" );
		src.LoadFile( fullName );
		if( !src.IsLoaded( ) )
			return false;
	}
	d_version = OLD_MAP_VERSION;
	d_fileTime = src.GetFileTime( );
	d_entities.SetNum( 0, false );
	if( src.CheckTokenString( "Version" ) ) {
		src.ReadTokenOnLine( &token );
		d_version = token.GetFloatValue( );
	}
	while( 1 ) {
		if( !d_entities.Alloc( ).Parse( src, ( d_entities.Num( ) == 0 ), d_version ) ) {
			d_entities.RemoveLast( );
			break;
		}
	}
	SetGeometryCRC( );
	// if the map has a worldspawn
	if( d_entities.Num( ) ) {
		// "removeEntities" "classname" can be set in the worldspawn to remove all entities with the given classname
		const KeyValue * removeEntities = d_entities[ 0 ].d_epairs.MatchPrefix( "removeEntities", NULL );
		while ( removeEntities ) {
			RemoveEntities( removeEntities->GetValue( ) );
			removeEntities = d_entities[ 0 ].d_epairs.MatchPrefix( "removeEntities", removeEntities );
		}
		// "overrideMaterial" "material" can be set in the worldspawn to reset all materials
		Str material;
		if( d_entities[ 0 ].d_epairs.GetString( "overrideMaterial", "", material ) ) {
			for( int i = 0; i < d_entities.Num( ); i++ ) {
				MapEntity & mapEnt = d_entities[ i ];
				for( int i = 0; i < mapEnt.GetNumBrushes( ); i++ ) {
					MapBrush & mapBrush = mapEnt.d_brushList[ i ];
					for( int k = 0; k < mapBrush.GetNumSides( ); k++ )
							mapBrush.d_sides[ k ].SetMaterial( material );
				}
				for( int i = 0; i < mapEnt.GetNumPatches( ); i++ )
					mapEnt.d_patchList[ i ].SetMaterial( material );
			}
		}
		// force all entities to have a name key/value pair
		if( d_entities[ 0 ].d_epairs.GetBool( "forceEntityNames" ) ) {
			for( int i = 1; i < d_entities.Num( ); i++ ) {
				MapEntity & mapEnt = d_entities[ i ];
				if( !mapEnt.d_epairs.FindKey( "name" ) )
					mapEnt.d_epairs.Set( "name", va( "%s%d", mapEnt.d_epairs.GetString( "classname", "forcedName" ), i ) );
			}
		}
		// move the primitives of any func_group entities to the worldspawn
		if( d_entities[ 0 ].d_epairs.GetBool( "moveFuncGroups" ) ) {
			for( int i = 1; i < d_entities.Num( ); i++ ) {
				MapEntity & mapEnt = d_entities[ i ];
				if( Str::IcmpFast( mapEnt.d_epairs.GetString( "classname" ), "func_group" ) ) {
					d_entities[ 0 ].AddPrimitiveData( mapEnt );
					mapEnt.RemovePrimitiveData( );
				}
			}
		}
	}
	d_hasPrimitiveData = true;
	return true;
}

bool MapFile::Write( const Str & fileName, const Str & ext, bool fromBasePath ) {
	Str qpath = fileName;
	qpath.SetFileExtension( ext );
	Common::Com_Printf( "writing %s...\n", qpath.c_str( ) );
	FileBase * fp = FileSystem::OpenFileByMode( qpath, FS_WRITE );
	if( !fp ) {
		Common::Com_Printf( "Couldn't open %s\n", qpath.c_str( ) );
		return false;
	}
	fp->WriteFloatString( "Version %f\n", (float) CURRENT_MAP_VERSION );
	for( int i = 0; i < d_entities.Num( ); i++ )
		d_entities[ i ].Write( fp, i );
	FileSystem::CloseFile( fp );
	return true;
}

void MapFile::SetGeometryCRC( ) {
	d_geometryCRC = 0;
	for( int i = 0; i < d_entities.Num( ); i++ )
		d_geometryCRC ^= d_entities[ i ].GetGeometryCRC( );
}

int MapFile::AddEntity( const MapEntity & mapEnt ) {
	int ret = d_entities.Append( mapEnt );
	return ret;
}

const MapEntity & MapFile::FindEntity( const Str & name ) const {
	for( int i = 0; i < d_entities.Num( ); i++ ) {
		const MapEntity & ent = d_entities[ i ];
		if( Str::IcmpFast( ent.d_epairs.GetString( "name" ), name ) )
			return ent;
	}
	return d_entities[ 0 ];
}

void MapFile::RemoveEntities( const Str & classname ) {
	for( int i = 0; i < d_entities.Num( ); i++ ) {
		MapEntity & ent = d_entities[ i ];
		if( Str::IcmpFast( ent.d_epairs.GetString( "classname" ), classname ) ) {
			d_entities.RemoveIndex( i );
			i--;
		}
	}
}

void MapFile::RemoveAllEntities( ) {
	d_entities.Clear( );
	d_hasPrimitiveData = false;
}

void MapFile::RemovePrimitiveData( ) {
	for( int i = 0; i < d_entities.Num( ); i++ )
		d_entities[ i ].RemovePrimitiveData( );
	d_hasPrimitiveData = false;
}

bool MapFile::NeedsReload( ) {
	if( d_name.Length( ) ) {
		uint time = ( uint )-1;
		if( FileSystem::TouchFile( d_name ) ) {
			return ( time > d_fileTime );
		}
	}
	return true;
}
