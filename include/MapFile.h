#ifndef MAPFILE_H
#define MAPFILE_H

/*===============================================================================
	Reads or writes the contents of .map files into a standard internal
	format, which can then be moved into private formats for collision
	detection, map processing, or editor use.

	No validation (duplicate planes, null area brushes, etc) is performed.
	There are no limits to the number of any of the elements in maps.
	The order of entities, brushes, and sides is maintained.
===============================================================================*/

#define OLD_MAP_VERSION				1
#define CURRENT_MAP_VERSION			2
#define DEFAULT_CURVE_SUBDIVISION	4
#define DEFAULT_CURVE_MAX_ERROR		4.0f
#define DEFAULT_CURVE_MAX_ERROR_CD	24.0f
#define DEFAULT_CURVE_MAX_LENGTH	-1.0f
#define DEFAULT_CURVE_MAX_LENGTH_CD	-1.0f

// // //// // //// // //// //
/// MapPrimitive
// //// // //// // //// //

class MapPrimitive {
protected:
	Dict							d_epairs;

public:
	const Dict &					GetEpairs( ) const;
};

// // //// // //// // //// //
/// MapBrushSide
// //// // //// // //// //

class MapBrushSide {
	friend class					MapBrush;
protected:
	Str								d_material;
	Plane							d_plane;
	CAList< int, 4 >				d_points;
	Vec3							d_texMat[ 2 ];
	Vec3							d_origin;
	bool							d_visible;

public:
									MapBrushSide( );

	const Str &						GetMaterial( ) const;
	void							SetMaterial( const Str & mat );
	const Plane &					GetPlane( ) const;
	void							SetPlane( const Plane & p );
	void							SetTextureMatrix( const Vec3 mat[ 2 ] );
	void							GetTextureMatrix( Vec3 & mat1, Vec3 & mat2 ) const;
	void							GetTextureVectors( Vec4 v[ 2 ] ) const;
	/// WARNING : special case behaviour of atan2(y,x) <-> atan(y/x) might not be the same everywhere when x == 0 rotation by (0,RotY,RotZ) assigns X to normal
	static void						ComputeAxisBase( const Vec3 & normal, Vec3 & texS, Vec3 & texT );

	int								GetNumPoints( ) const { return d_points.Num( ); }
	int								GetPoint( int i ) const { return d_points[ i ]; }

	bool							IsVisible( ) const { return d_visible; }
};

// // //// // //// // //// //
/// MapBrush
// //// // //// // //// //

class MapBrush : public MapPrimitive {
	friend class					MapFile;
protected:
	CAList< MapBrushSide, 6 >		d_sides;
	Bounds							d_bounds;
	CAList< Vec3, 8 >				d_verts;

	void							ComputeSideVerts( );

public:

	int								HideIncludedSides( const Bounds & bounds, const MapBrush & other );

	bool							Parse( Lexer & src, const Vec3 & origin, bool newFormat = true, float version = CURRENT_MAP_VERSION );
	bool							ParseQ3( Lexer & src, const Vec3 & origin );
	bool							Write( FileBase * fp, int primitiveNum, const Vec3 & origin ) const;

	const Bounds &					GetBounds( ) const { return d_bounds; }
	const Vec3 &					GetVertex( int i ) const { return d_verts[ i ]; }

	int								GetNumSides( ) const;
	const MapBrushSide &			GetSide( int i ) const;
	uint							GetGeometryCRC( ) const;
};

// // //// // //// // //// //
/// MapPatch
// //// // //// // //// //

class MapPatch : public MapPrimitive, public Patch {
protected:
	Str								d_material;
	int								d_horzSubdivisions;
	int								d_vertSubdivisions;
	bool							d_explicitSubdivisions;

public:
									MapPatch( );

	bool							Parse( Lexer & src, const Vec3 & origin, bool patchDef3 = true, float version = CURRENT_MAP_VERSION );
	bool							Write( FileBase * fp, int primitiveNum, const Vec3 & origin ) const;

	const Str &						GetMaterial( ) const;
	void							SetMaterial( const Str & mat );
	uint							GetGeometryCRC( ) const;
};

// // //// // //// // //// //
/// MapEntity
// //// // //// // //// //

class MapEntity {
	friend class					MapFile;
private:
	DList< MapBrush >				d_brushList;
	DList< MapPatch >				d_patchList;
	Dict							d_epairs;

	void							BrushSideVisibilityTest( );

public:

									MapEntity( );

	bool							Parse( Lexer & src, bool worldSpawn = false, float version = CURRENT_MAP_VERSION );
	bool							Write( FileBase * fp, int entityNum ) const;

	int								GetNumPrimitives( ) const;
	int								GetNumBrushes( ) const;
	const MapBrush &				GetBrush( int i ) const;
	int								GetNumPatches( ) const;
	const MapPatch &				GetPatch( int i ) const;

	void							RemovePrimitiveData( );
	void							AddPrimitiveData( const MapEntity & other );

	uint							GetGeometryCRC( ) const;
	const Dict &					GetEpairs( ) const;
};

// // //// // //// // //// //
/// MapFile
// //// // //// // //// //

class MapFile : public NamedObject {
private:
	float							d_version;
	timeStamp_t						d_fileTime;
	uint							d_geometryCRC;
	DList< MapEntity >				d_entities;
	bool							d_hasPrimitiveData;

	void							SetGeometryCRC( );

public:
									MapFile( );

	/// filename does not require an extension, normally this will use a .reg file instead of a .map file if it exists,
	/// which is what the game and dmap want, but the editor will want to always load a .map file
	bool							Parse( const Str & filename, bool ignoreRegion = true );
	bool							Write( const Str & fileName, const Str & ext, bool fromBasePath = true );

	/// get the number of entities in the map
	int								GetNumEntities( ) const;
	/// get the specified entity
	const MapEntity &				GetEntity( int i ) const;
	/// get the file time
	timeStamp_t						GetFileTime( ) const;
	/// get CRC for the map geometry, Texture coordinates and entity key/value pairs are not taken into account
	uint							GetGeometryCRC( ) const;
	/// returns true if the file on disk changed
	bool							NeedsReload( );

	int								AddEntity( const MapEntity & mapentity );
	const MapEntity &				FindEntity( const Str & name ) const;
	void							RemoveEntities( const Str & classname );
	void							RemoveAllEntities( );
	void							RemovePrimitiveData( );
	bool							HasPrimitiveData( );
};

// // //// // //// // //// //
// MapPrimitive
// //// // //// // //// //

INLINE const Dict & MapPrimitive::GetEpairs( ) const {
	return d_epairs;
}

// // //// // //// // //// //
// MapBrushSide
// //// // //// // //// //

INLINE const Str & MapBrushSide::GetMaterial( ) const {
	return d_material;
}

INLINE void MapBrushSide::SetMaterial( const Str & mat ) {
	d_material = mat;
}

INLINE const Plane & MapBrushSide::GetPlane( ) const {
	return d_plane;
}

INLINE void MapBrushSide::SetPlane( const Plane & p ) {
	d_plane = p;
}

INLINE void MapBrushSide::SetTextureMatrix( const Vec3 mat[ 2 ] ) {
	d_texMat[ 0 ] = mat[ 0 ];
	d_texMat[ 1 ] = mat[ 1 ];
}

INLINE void MapBrushSide::GetTextureMatrix( Vec3 & mat1, Vec3 & mat2 ) const {
	mat1 = d_texMat[ 0 ];
	mat2 = d_texMat[ 1 ];
}

// // //// // //// // //// //
// MapBrush
// //// // //// // //// //

INLINE int MapBrush::GetNumSides( ) const {
	return d_sides.Num( );
}

INLINE const MapBrushSide & MapBrush::GetSide( int i ) const {
	return d_sides[ i ];
}

// // //// // //// // //// //
// MapPatch
// //// // //// // //// //

INLINE const Str & MapPatch::GetMaterial( ) const {
	return d_material;
}

INLINE void MapPatch::SetMaterial( const Str & mat ) {
	d_material = mat;
}

// // //// // //// // //// //
// MapEntity
// //// // //// // //// //

INLINE int MapEntity::GetNumPrimitives( ) const {
	return d_brushList.Num( ) + d_patchList.Num( );
}

INLINE int MapEntity::GetNumBrushes( ) const {
	return d_brushList.Num( );
}

INLINE const MapBrush & MapEntity::GetBrush( int i ) const {
	return d_brushList[ i ];
}

INLINE int MapEntity::GetNumPatches( ) const {
	return d_patchList.Num( );
}

INLINE const MapPatch & MapEntity::GetPatch( int i ) const {
	return d_patchList[ i ];
}

INLINE const Dict & MapEntity::GetEpairs( ) const {
	return d_epairs;
}

// // //// // //// // //// //
// MapFile
// //// // //// // //// //

INLINE int MapFile::GetNumEntities( ) const {
	return d_entities.Num( );
}

INLINE const MapEntity & MapFile::GetEntity( int i ) const {
	return d_entities[ i ];
}

INLINE timeStamp_t MapFile::GetFileTime( ) const {
	return d_fileTime;
}

INLINE uint MapFile::GetGeometryCRC( ) const {
	return d_geometryCRC;
}

INLINE bool MapFile::HasPrimitiveData( ) {
	return d_hasPrimitiveData;
}

#endif
