#ifndef MAPSYSTEM_H
#define MAPSYSTEM_H

#define MAPSYSTEM_EPSILON 0.5f
#define MAPSYSTEM_INTEGRALEPSILON 0.1f

// // //// // //// // //// //
/// MapGeometry
// //// // //// // //// //

class MapGeometry {
	friend class						MapData;
private:
	Bounds								d_geomBounds;
	DList< Vec3 >						d_geomVertices;
	DList< int >						d_geomIndices;

public:
	void								ComputeBounds( );
	const Bounds &						GetBounds( ) const;

	int									GetNumIndexes( ) const { return d_geomIndices.Num( ); }
	const int *							GetIndexes( ) const { return d_geomIndices.Ptr( ); }
	int									GetNumVertices( ) const { return d_geomVertices.Num( ); }
	const Vec3 *						GetVertices( ) const { return d_geomVertices.Ptr( ); }
};

// // //// // //// // //// //
/// MapData
// //// // //// // //// //

class MapData {
private:
	MapFile								d_mapFile;
	DList< MapGeometry >				d_mapGeoms;
	Bounds								d_mapBounds;

public:

	const MapGeometry &					operator[ ]( const int index ) const;

	MapFile *							GetMapFile( );
	void								BuildStaticGeometry( );
#ifndef DEDICATED_ONLY
	void								BuildRenderGeometry( List< RenderModelStatic * > & ren_models, BufferAllocatorMap * map_buffer );
#endif
	void								ComputeBounds( );
	int									GetNumGeoms( ) const;
	const Bounds &						GetBounds( ) const;
};

// // //// // //// // //// //
/// MapSystem
// //// // //// // //// //

class MapSystem {
private:
	static List< MapData * >			g_mapSections;
	static Bounds						g_worldBounds;

public:
	static MapData *					LoadMapFile( const Str & file_name );
	static unsigned int					ComputeCRC( );

	static int							GetNumSections( );
	static MapData *					GetSection( const int index );

	static const Bounds &				GetBounds( );
};

// // //// // //// // //// //
// MapGeometry
// //// // //// // //// //

INLINE const Bounds & MapGeometry::GetBounds( ) const {
	return d_geomBounds;
}

// // //// // //// // //// //
// MapData
// //// // //// // //// //

INLINE const MapGeometry & MapData::operator[ ]( const int index ) const {
	return d_mapGeoms[ index ];
}

INLINE MapFile * MapData::GetMapFile( ) {
	return &d_mapFile;
}

INLINE int MapData::GetNumGeoms( ) const {
	return d_mapGeoms.Num( );
}

INLINE const Bounds & MapData::GetBounds( ) const {
	return d_mapBounds;
}

// // //// // //// // //// //
// MapSystem
// //// // //// // //// //

INLINE int MapSystem::GetNumSections( ) {
	return g_mapSections.Num( );
}

INLINE MapData * MapSystem::GetSection( const int index ) {
	return g_mapSections[ index ];
}

INLINE const Bounds & MapSystem::GetBounds( ) {
	return g_worldBounds;
}

#endif
