#include "precompiled.h"
#pragma hdrstop

List< MapData * >			MapSystem::g_mapSections;
Bounds						MapSystem::g_worldBounds;

// // //// // //// // //// //
// MapGeometry
// //// // //// // //// //

void MapGeometry::ComputeBounds( ) {
	d_geomBounds.Clear( );	
	for( int i = 0; i < this->GetNumVertices( ); i++ )
		d_geomBounds.AddPoint( d_geomVertices[ i ] );
}

// // //// // //// // //// //
// MapData
// //// // //// // //// //

void MapData::BuildStaticGeometry( ) {
	Patch				expPatch;
	int					vertRelocationBase;
	int					vertLookup[ MAX_POINTS_ON_WINDING ];

	d_mapBounds.Clear( );
	
	for( int i = 0; i < d_mapFile.GetNumEntities( ); i++ ) {
		const MapEntity & mapEntity = d_mapFile.GetEntity( i );
		if( !mapEntity.GetNumPrimitives( ) )
			continue;
		MapGeometry & mapGeometry = d_mapGeoms.Alloc( );
		for( int j = 0; j < mapEntity.GetNumBrushes( ); j++ ) {
			const MapBrush & mapBrush = mapEntity.GetBrush( j );
			Common::Com_Memset( vertLookup, -1, NumSizeOf( int, MAX_POINTS_ON_WINDING ) );
			for( int k = 0; k < mapBrush.GetNumSides( ); k++ ) {
				const MapBrushSide & mapSide = mapBrush.GetSide( k );
				if( !mapSide.IsVisible( ) )
					continue;
				for( int l = 0; l < mapSide.GetNumPoints( ); l++ ) {
					int point = mapSide.GetPoint( l );
					int m = vertLookup[ point ];
					if( m == -1 ) {
						const Vec3 & mapVert = mapBrush.GetVertex( point );
						for( m = 0; m < mapGeometry.d_geomVertices.Num( ); m++ ) {
							if( mapGeometry.d_geomVertices[ m ].Compare( mapVert, 0.5f ) )
								break;
						}
						if( m >= mapGeometry.d_geomVertices.Num( ) )
							m = mapGeometry.d_geomVertices.Append( mapVert );
						vertLookup[ point ] = m;
					}
				}
				for( int l = 2; l < mapSide.GetNumPoints( ); l++ ) {
					mapGeometry.d_geomIndices.Append( vertLookup[ mapSide.GetPoint( 0 ) ] );
					mapGeometry.d_geomIndices.Append( vertLookup[ mapSide.GetPoint( l - 1 ) ] );
					mapGeometry.d_geomIndices.Append( vertLookup[ mapSide.GetPoint( l ) ] );
				}
			}
		}
		for( int j = 0; j < mapEntity.GetNumPatches( ); j++ ) {
			const MapPatch & mapPatch = mapEntity.GetPatch( j );
			expPatch = mapPatch;
			//expPatch.Subdivide( 0.5f, 0.5f, 0, false );
			expPatch.SubdivideExplicit( 4, 4, true, false );
			vertRelocationBase = mapGeometry.d_geomVertices.Num( );
			for( int k = 0; k < expPatch.GetNumVertices( ); k++ )
				mapGeometry.d_geomVertices.Append( expPatch[ k ].xyz );
			for( int k = 0; k < expPatch.GetNumIndexes( ); k++ )
				mapGeometry.d_geomIndices.Append( expPatch.GetIndexes( )[ k ] + vertRelocationBase );
		}
		mapGeometry.ComputeBounds( );
		d_mapBounds.AddBounds( mapGeometry.GetBounds( ) );
	}
}

#ifndef DEDICATED_ONLY
void MapData::BuildRenderGeometry( List< RenderModelStatic * > & ren_models, BufferAllocatorMap * map_buffer ) {
	Patch				expPatch;
	RenderModelStatic *	renderModel;
	RenderSurfaceMap *	renderSurface;
	Str					materialName;
	Material *			surfaceMaterial;
	List< RenderSurfaceMap * > renderSurfaceList;
	
	for( int i = 0; i < d_mapFile.GetNumEntities( ); i++ ) {
		const MapEntity & mapEntity = d_mapFile.GetEntity( i );
		for( int j = 0; j < mapEntity.GetNumBrushes( ); j++ ) {
			const MapBrush & mapBrush = mapEntity.GetBrush( j );
			renderModel = new RenderModelStatic( );
			ren_models.Append( renderModel );
			for( int k = 0; k < mapBrush.GetNumSides( ); k++ ) {
				const MapBrushSide & mapSide = mapBrush.GetSide( k );
				if( !mapSide.IsVisible( ) )
					continue;
				materialName = mapSide.GetMaterial( );
				surfaceMaterial = MaterialSystem::FindMaterial( materialName );
				renderSurface = renderModel->LoadBrushSide( surfaceMaterial->GetShader( ), map_buffer, mapBrush, mapSide );
				surfaceMaterial->ConfigRenderSurface( renderSurface );
				renderSurfaceList.Append( renderSurface );
			}
			renderModel->SetBounds( mapBrush.GetBounds( ) );
		}
		for( int j = 0; j < mapEntity.GetNumPatches( ); j++ ) {
			const MapPatch & mapPatch = mapEntity.GetPatch( j );
			renderModel = new RenderModelStatic( );
			ren_models.Append( renderModel );
			expPatch = mapPatch;
			expPatch.Subdivide( 0.5f, 0.5f, DEFAULT_CURVE_MAX_LENGTH, true );
			//expPatch.SubdivideExplicit( 4, 4, true, false );						
			materialName = mapPatch.GetMaterial( );
			surfaceMaterial = MaterialSystem::FindMaterial( materialName );
			renderSurface = renderModel->LoadSurface( surfaceMaterial->GetShader( ), map_buffer, &expPatch );
			surfaceMaterial->ConfigRenderSurface( renderSurface );
			renderSurfaceList.Append( renderSurface );
			renderModel->SetBounds( mapPatch.ComputeBounds( ) );
		}
	}
	renderSurfaceList.Sort( RenderSurfaceMap::ShaderAndSVarAndIndexCmp );
	RenderSurfaceMap * currentSurface;
	RenderSurfaceMap * lastSurface = NULL;
	int currentRenderClass = 0;
	for( int i = 0; i < renderSurfaceList.Num( ); i++ ) {
		currentSurface = renderSurfaceList[ i ];
		if( lastSurface && RenderSurfaceMap::ShaderAndSVarCmp( &currentSurface, &lastSurface ) )
			currentRenderClass++;
		currentSurface->SetRenderClass( currentRenderClass );
		currentSurface->ComputeIndexes( );
		lastSurface = currentSurface;
	}
	Common::Com_Printf( "MapData::BuildRenderGeometry: %i surface class created.\n", currentRenderClass + 1 );
}
#endif

void MapData::ComputeBounds( ) {
	d_mapBounds.Clear( );
	for( int i = 0; i < d_mapGeoms.Num( ); i++ ) {
		d_mapGeoms[ i ].ComputeBounds( );
		d_mapBounds.AddBounds( d_mapGeoms[ i ].GetBounds( ) );
	}
}

// // //// // //// // //// //
// MapSystem
// //// // //// // //// //

MapData * MapSystem::LoadMapFile( const Str & file_name ) {
	for( int i = 0; i < g_mapSections.Num( ); i++ ) {
		if( g_mapSections[ i ]->GetMapFile( )->GetName( ) == Str( file_name ).StripFileExtension( ) )
			return g_mapSections[ i ];
	}
	if( !FileSystem::TouchFile( file_name ) ) {
		Common::Com_Printf( "MapSystem::LoadMapFile: unable to load \"%s\"\n", file_name.c_str( ) );
		return NULL;
	}
	if( !g_mapSections.Num( ) )
		g_worldBounds.Clear( );
	MapData * mapData = new MapData;
	mapData->GetMapFile( )->Parse( file_name );
	mapData->BuildStaticGeometry( );
	g_worldBounds.AddBounds( mapData->GetBounds( ) );
	g_mapSections.Append( mapData );
	return mapData;
}

unsigned int MapSystem::ComputeCRC( ) {
	unsigned int crcSum = 0;
	for( int i = 0; i < g_mapSections.Num( ); i++ )
		crcSum ^= g_mapSections[ i ]->GetMapFile( )->GetGeometryCRC( );
	return crcSum;
}
