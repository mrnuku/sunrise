#include "precompiled.h"
#pragma hdrstop

clipMap_t	CollisionModel::cm;
int			CollisionModel::c_pointcontents;
int			CollisionModel::c_traces;
int			CollisionModel::c_brush_traces;
int			CollisionModel::c_patch_traces;

byte *		CollisionModel::cmod_base;
unsigned	CollisionModel::last_checksum;

cmodel_t	CollisionModel::box_model;
cplane_t *	CollisionModel::box_planes;
cbrush_t *	CollisionModel::box_brush;

/*
===============================================================================

					MAP LOADING

===============================================================================
*/

/*
=================
CMod_LoadShaders
=================
*/
void CollisionModel::CMod_LoadShaders( lump_t * l ) {

	dshader_t *		in;
	dshader_t *		out;

	in = ( dshader_t * )( cmod_base + l->fileofs );

	if( l->filelen % sizeof( dshader_t ) ) Common::Com_Error( ERR_DROP, "Unable to load map" );

	int count = l->filelen / sizeof( dshader_t );

	if( count < 1 ) Common::Com_Error( ERR_DROP, "Unable to load map" );

	cm.shaders = ( dshader_t * )Z_TagMalloc( count * sizeof( dshader_t ), TAG_CM );
	cm.numShaders = count;

	Common::Com_Memcpy( cm.shaders, in, count * sizeof( dshader_t ) );

	out = cm.shaders;
	for( int i = 0; i < count; i++, in++, out++ ) {

		out->contentFlags = LittleLong( out->contentFlags );
		out->surfaceFlags = LittleLong( out->surfaceFlags );
	}
}


/*
=================
CMod_LoadSubmodels
=================
*/
void CollisionModel::CMod_LoadSubmodels( lump_t * l ) {

	dmodel_t *	in;
	cmodel_t *	out;

	in = ( dmodel_t * )( cmod_base + l->fileofs );

	if( l->filelen % sizeof( dmodel_t ) ) Common::Com_Error( ERR_DROP, "Unable to load map" );

	int count = l->filelen / sizeof( dmodel_t );

	if (count < 1) Common::Com_Error( ERR_DROP, "Unable to load map" );

	cm.cmodels = ( cmodel_t * )Z_TagMalloc( count * sizeof( cmodel_t ), TAG_CM );
	cm.numSubModels = count;

	if( count > MAX_SUBMODELS ) Common::Com_Error( ERR_DROP, "MAX_SUBMODELS exceeded" );

	for( int i = 0; i < count; i++, in++, out++ ) {

		out = &cm.cmodels[ i ];

		for ( int j = 0; j < 3; j++ ) {	// spread the mins / maxs by a pixel

			out->mins[ j ] = LittleFloat( in->mins[ j ] ) - 1;
			out->maxs[ j ] = LittleFloat( in->maxs[ j ] ) + 1;
		}

		if ( i == 0 ) {

			continue;	// world model doesn't need other info
		}

		// make a "leaf" just to hold the model's brushes and surfaces
		out->leaf.numLeafBrushes = LittleLong( in->numBrushes );
		int * brushes = ( int * )Z_TagMalloc( out->leaf.numLeafBrushes * 4, TAG_CM );
		out->leaf.firstLeafBrush = brushes - cm.leafbrushes;

		for ( int j = 0; j < out->leaf.numLeafBrushes; j++ ) {

			brushes[ j ] = LittleLong( in->firstBrush ) + j;
		}

		out->leaf.numLeafSurfaces = LittleLong( in->numSurfaces );
		int * surfaces = ( int * )Z_TagMalloc( out->leaf.numLeafSurfaces * 4, TAG_CM );
		out->leaf.firstLeafSurface = surfaces - cm.leafsurfaces;

		for ( int j = 0; j < out->leaf.numLeafSurfaces; j++ ) {

			surfaces[ j ] = LittleLong( in->firstSurface ) + j;
		}
	}
}


/*
=================
CMod_LoadNodes

=================
*/
void CollisionModel::CMod_LoadNodes( lump_t * l ) {

	dnode_t *	in;
	cNode_t *	out;
	
	in = ( dnode_t * )( cmod_base + l->fileofs );

	if( l->filelen % sizeof( dnode_t ) ) Common::Com_Error( ERR_DROP, "Unable to load map" );

	int count = l->filelen / sizeof( dnode_t );

	if( count < 1 ) Common::Com_Error( ERR_DROP, "Unable to load map" );

	cm.nodes = ( cNode_t * )Z_TagMalloc( count * sizeof( cNode_t ), TAG_CM );
	cm.numNodes = count;

	out = cm.nodes;

	for( int i = 0 ; i < count ; i++, out++, in++ ) {

		out->plane = cm.planes + LittleLong( in->planeNum );

		for( int j=0 ; j < 2 ; j++ ) {

			out->children[ j ] = LittleLong( in->children[ j ] );
		}
	}
}

/*
=================
CM_BoundBrush

=================
*/
void CollisionModel::CM_BoundBrush( cbrush_t * b ) {

	b->bounds[0][0] = -b->sides[0].plane->dist;
	b->bounds[1][0] = b->sides[1].plane->dist;

	b->bounds[0][1] = -b->sides[2].plane->dist;
	b->bounds[1][1] = b->sides[3].plane->dist;

	b->bounds[0][2] = -b->sides[4].plane->dist;
	b->bounds[1][2] = b->sides[5].plane->dist;
}


/*
=================
CMod_LoadBrushes

=================
*/
void CollisionModel::CMod_LoadBrushes( lump_t * l ) {

	dbrush_t *		in;
	cbrush_t *		out;

	in = ( dbrush_t * )( cmod_base + l->fileofs );

	if( l->filelen % sizeof( dbrush_t ) ) Common::Com_Error( ERR_DROP, "Unable to load map" );

	int count = l->filelen / sizeof( dbrush_t );

	cm.brushes = ( cbrush_t * )Z_TagMalloc( ( BOX_BRUSHES + count ) * sizeof( cbrush_t ), TAG_CM );
	cm.numBrushes = count;

	out = cm.brushes;

	for( int i = 0; i < count; i++, out++, in++ ) {

		out->sides = cm.brushsides + LittleLong( in->firstSide );
		out->numsides = LittleLong( in->numSides );
		out->shaderNum = LittleLong( in->shaderNum );

		if( out->shaderNum < 0 || out->shaderNum >= cm.numShaders ) {

			Common::Com_Error( ERR_DROP, "CMod_LoadBrushes: bad shaderNum: %i", out->shaderNum );
		}

		out->contents = cm.shaders[ out->shaderNum ].contentFlags;

		CM_BoundBrush( out );
	}

}

/*
=================
CMod_LoadLeafs
=================
*/
void CollisionModel::CMod_LoadLeafs( lump_t * l ) {

	cLeaf_t *	out;
	dleaf_t *	in;
	
	in = ( dleaf_t * )( cmod_base + l->fileofs );

	if( l->filelen % sizeof( dleaf_t ) ) Common::Com_Error( ERR_DROP, "Unable to load map" );

	int count = l->filelen / sizeof( dleaf_t );

	if( count < 1 ) Common::Com_Error( ERR_DROP, "Unable to load map" );

	cm.leafs = ( cLeaf_t * )Z_TagMalloc( ( BOX_LEAFS + count ) * sizeof( cLeaf_t ), TAG_CM );
	cm.numLeafs = count;

	out = cm.leafs;	
	for( int i = 0; i < count; i++, in++, out++ ) {

		out->cluster = LittleLong( in->cluster );
		out->area = LittleLong( in->area );
		out->firstLeafBrush = LittleLong( in->firstLeafBrush );
		out->numLeafBrushes = LittleLong( in->numLeafBrushes );
		out->firstLeafSurface = LittleLong( in->firstLeafSurface );
		out->numLeafSurfaces = LittleLong( in->numLeafSurfaces );

		if( out->cluster >= cm.numClusters ) cm.numClusters = out->cluster + 1;

		if( out->area >= cm.numAreas ) cm.numAreas = out->area + 1;
	}

	cm.areas = ( cArea_t * )Z_TagMalloc( cm.numAreas * sizeof( cArea_t ), TAG_CM );
	cm.areaPortals = ( int * )Z_TagMalloc( cm.numAreas * cm.numAreas * sizeof( int ), TAG_CM );
}

/*
=================
CMod_LoadPlanes
=================
*/
void CollisionModel::CMod_LoadPlanes( lump_t * l ) {

	cplane_t *	out;
	dplane_t *	in;
	
	in = ( dplane_t * )( cmod_base + l->fileofs );

	if( l->filelen % sizeof( dplane_t ) ) Common::Com_Error( ERR_DROP, "Unable to load map" );

	int count = l->filelen / sizeof( dplane_t );

	if( count < 1 ) Common::Com_Error( ERR_DROP, "Unable to load map" );

	cm.planes = ( cplane_t * )Z_TagMalloc( ( BOX_PLANES + count ) * sizeof( cplane_t ), TAG_CM );
	cm.numPlanes = count;

	out = cm.planes;	

	for( int i = 0 ; i < count; i++, in++, out++ ) {

		uint bits = 0;
		for( int j = 0; j < 3; j++ ) {

			out->normal[ j ] = LittleFloat( in->normal[ j ] );
			if( out->normal[ j ] < 0 ) bits |= 1 << j;
		}

		out->dist = LittleFloat (in->dist);
		out->type = out->normal.GetPlaneType( );
		out->signbits = bits;
	}
}

/*
=================
CMod_LoadLeafBrushes
=================
*/
void CollisionModel::CMod_LoadLeafBrushes( lump_t * l ) {

	int *		out;
	int *		in;
	
	in = ( int * )( cmod_base + l->fileofs );

	if( l->filelen % sizeof( int ) ) Common::Com_Error( ERR_DROP, "Unable to load map" );

	int count = l->filelen / sizeof( int );

	cm.leafbrushes = ( int * )Z_TagMalloc( ( count + BOX_BRUSHES ) * sizeof( int ), TAG_CM );
	cm.numLeafBrushes = count;

	out = cm.leafbrushes;

	for ( int i = 0; i < count; i++, in++, out++ ) {

		*out = LittleLong( *in );
	}
}

/*
=================
CMod_LoadLeafSurfaces
=================
*/
void CollisionModel::CMod_LoadLeafSurfaces( lump_t * l ) {

	int *		out;
	int *		in;
	
	in = ( int * )( cmod_base + l->fileofs );

	if( l->filelen % sizeof( int ) ) Common::Com_Error( ERR_DROP, "Unable to load map" );

	int count = l->filelen / sizeof( int );

	cm.leafsurfaces = ( int * )Z_TagMalloc( count * sizeof( int ), TAG_CM );
	cm.numLeafSurfaces = count;

	out = cm.leafsurfaces;

	for( int i = 0 ; i<count ; i++, in++, out++ ) {

		*out = LittleLong( *in );
	}
}

/*
=================
CMod_LoadBrushSides
=================
*/
void CollisionModel::CMod_LoadBrushSides( lump_t * l ) {

	cbrushside_t *	out;
	dbrushside_t *	in;

	in = ( dbrushside_t * )( cmod_base + l->fileofs );

	if ( l->filelen % sizeof( dbrushside_t ) ) Common::Com_Error( ERR_DROP, "Unable to load map" );

	int count = l->filelen / sizeof( dbrushside_t );

	cm.brushsides = ( cbrushside_t * )Z_TagMalloc( ( BOX_SIDES + count ) * sizeof( cbrushside_t ), TAG_CM );
	cm.numBrushSides = count;

	out = cm.brushsides;	

	for( int i = 0 ; i < count; i++, in++, out++ ) {

		int num = LittleLong( in->planeNum );
		out->plane = &cm.planes[ num ];
		out->shaderNum = LittleLong( in->shaderNum );
		if ( out->shaderNum < 0 || out->shaderNum >= cm.numShaders ) Common::Com_Error( ERR_DROP, "Unable to load map" );
		out->surfaceFlags = cm.shaders[ out->shaderNum ].surfaceFlags;
	}
}


/*
=================
CMod_LoadEntityString
=================
*/
void CollisionModel::CMod_LoadEntityString( lump_t * l ) {

	cm.entityString = Str( ( char * )( cmod_base + l->fileofs ), 0, l->filelen );
}

/*
=================
CMod_LoadVisibility
=================
*/
#define	VIS_HEADER	8
void CollisionModel::CMod_LoadVisibility( lump_t * l ) {

	byte *	buf;

	int len = l->filelen;

	if ( !len ) {

		cm.clusterBytes = ( cm.numClusters + 31 ) & ~31;
		cm.visibility = ( byte * )Z_TagMalloc( cm.clusterBytes, TAG_CM );
		Common::Com_Memset( cm.visibility, 255, cm.clusterBytes );
		return;
	}

	buf = cmod_base + l->fileofs;

	cm.vised = true;
	cm.visibility = ( byte * )Z_TagMalloc( len, TAG_CM );
	cm.numClusters = LittleLong( ( ( int * )buf )[ 0 ] );
	cm.clusterBytes = LittleLong( ( ( int * )buf )[ 1 ] );
	Common::Com_Memcpy( cm.visibility, buf + VIS_HEADER, len - VIS_HEADER );
}

//==================================================================


/*
=================
CMod_LoadPatches
=================
*/
#define	MAX_PATCH_VERTS		1024
void CollisionModel::CMod_LoadPatches( lump_t * surfs, lump_t * verts ) {

	drawVert_t	*dv, *dv_p;
	dsurface_t	*in;
	int			count;
	int			i, j;
	int			c;
	cPatch_t	*patch;
	Vec3		points[ MAX_PATCH_VERTS ];
	int			width, height;
	int			shaderNum;

	in = ( dsurface_t * )( cmod_base + surfs->fileofs );

	if( surfs->filelen % sizeof( dsurface_t ) ) Common::Com_Error( ERR_DROP, "Unable to load map" );

	cm.numSurfaces = count = surfs->filelen / sizeof( dsurface_t );
	cm.surfaces = ( cPatch_t ** )Z_TagMalloc( cm.numSurfaces * sizeof( cPatch_t ), TAG_CM );

	dv = ( drawVert_t * )( cmod_base + verts->fileofs );

	if( verts->filelen % sizeof( drawVert_t ) ) Common::Com_Error( ERR_DROP, "Unable to load map" );

	// scan through all the surfaces, but only load patches,
	// not planar faces
	for ( i = 0 ; i < count ; i++, in++ ) {

		if ( LittleLong( in->surfaceType ) != MST_PATCH ) {
			continue;		// ignore other surfaces
		}
		// FIXME: check for non-colliding patches

		cm.surfaces[ i ] = patch = ( cPatch_t * )Z_TagMalloc( sizeof( cPatch_t ), TAG_CM );

		// load the full drawverts onto the stack
		width = LittleLong( in->patchWidth );
		height = LittleLong( in->patchHeight );
		c = width * height;
		if ( c > MAX_PATCH_VERTS ) Common::Com_Error( ERR_DROP, "Unable to load map" );

		dv_p = dv + LittleLong( in->firstVert );

		for ( j = 0 ; j < c ; j++, dv_p++ ) {

			points[j][0] = LittleFloat( dv_p->xyz[0] );
			points[j][1] = LittleFloat( dv_p->xyz[1] );
			points[j][2] = LittleFloat( dv_p->xyz[2] );
		}

		shaderNum = LittleLong( in->shaderNum );
		patch->contents = cm.shaders[shaderNum].contentFlags;
		patch->surfaceFlags = cm.shaders[shaderNum].surfaceFlags;

		// create the internal facet structure
		patch->pc = CM_GeneratePatchCollide( width, height, points );
	}
}

//==================================================================

unsigned CollisionModel::CM_LumpChecksum( lump_t * lump ) {

	return LittleLong( Common::Com_BlockChecksum( cmod_base + lump->fileofs, lump->filelen ) );
}

unsigned CollisionModel::CM_Checksum( dheader_t * header ) {

	unsigned checksums[16];

	checksums[0] = CM_LumpChecksum(&header->lumps[LUMP_SHADERS]);
	checksums[1] = CM_LumpChecksum(&header->lumps[LUMP_LEAFS]);
	checksums[2] = CM_LumpChecksum(&header->lumps[LUMP_LEAFBRUSHES]);
	checksums[3] = CM_LumpChecksum(&header->lumps[LUMP_LEAFSURFACES]);
	checksums[4] = CM_LumpChecksum(&header->lumps[LUMP_PLANES]);
	checksums[5] = CM_LumpChecksum(&header->lumps[LUMP_BRUSHSIDES]);
	checksums[6] = CM_LumpChecksum(&header->lumps[LUMP_BRUSHES]);
	checksums[7] = CM_LumpChecksum(&header->lumps[LUMP_MODELS]);
	checksums[8] = CM_LumpChecksum(&header->lumps[LUMP_NODES]);
	checksums[9] = CM_LumpChecksum(&header->lumps[LUMP_SURFACES]);
	checksums[10] = CM_LumpChecksum(&header->lumps[LUMP_DRAWVERTS]);

	return LittleLong( Common::Com_BlockChecksum( checksums, 11 * 4 ) );
}

/*
==================
CM_LoadMap

Loads in the map and all submodels
==================
*/
void CollisionModel::CM_LoadMap( const Str & name, bool clientload, int * checksum ) {

	dheader_t		header;

	if ( name.IsEmpty( ) ) Common::Com_Error( ERR_DROP, "Unable to load map" );

	Common::Com_DPrintf( "CM_LoadMap( %s, %i )\n", name, clientload );

	if ( cm.name == name && clientload ) {

		*checksum = last_checksum;
		return;
	}

	// free old stuff
	Common::Com_Memset( &cm, 0, sizeof( cm ) );
	CM_ClearLevelPatches();

	if ( !name[0] ) {
		cm.numLeafs = 1;
		cm.numClusters = 1;
		cm.numAreas = 1;
		cm.cmodels = ( cmodel_t * )Z_TagMalloc( sizeof( cmodel_t ), TAG_CM );
		*checksum = 0;
		return;
	}

	//
	// load the file
	//
	
	File_Memory fm = FileSystem::ReadFile( name );

	if ( !fm.IsValid( ) ) Common::Com_Error( ERR_DROP, "Unable to load map" );
	const char * buf = fm.GetDataPtr( );

	last_checksum = LittleLong (Common::Com_BlockChecksum( buf, fm.Length( ) ) );
	*checksum = last_checksum;

	header = *(dheader_t *)buf;

	for( int i = 0; i < sizeof( dheader_t ) / 4 ; i++ ) {

		( ( int * )&header )[ i ] = LittleLong( ( ( int * ) &header)[ i ] );
	}

	if( header.version != BSP_VERSION ) Common::Com_Error( ERR_DROP, "Unable to load map" );

	cmod_base = ( byte * )buf;

	// load into heap
	CMod_LoadShaders( &header.lumps[LUMP_SHADERS] );
	CMod_LoadLeafs (&header.lumps[LUMP_LEAFS]);
	CMod_LoadLeafBrushes (&header.lumps[LUMP_LEAFBRUSHES]);
	CMod_LoadLeafSurfaces (&header.lumps[LUMP_LEAFSURFACES]);
	CMod_LoadPlanes (&header.lumps[LUMP_PLANES]);
	CMod_LoadBrushSides (&header.lumps[LUMP_BRUSHSIDES]);
	CMod_LoadBrushes (&header.lumps[LUMP_BRUSHES]);
	CMod_LoadSubmodels (&header.lumps[LUMP_MODELS]);
	CMod_LoadNodes (&header.lumps[LUMP_NODES]);
	CMod_LoadEntityString (&header.lumps[LUMP_ENTITIES]);
	CMod_LoadVisibility( &header.lumps[LUMP_VISIBILITY] );
	CMod_LoadPatches( &header.lumps[LUMP_SURFACES], &header.lumps[LUMP_DRAWVERTS] );

	// we are NOT freeing the file, because it is cached for the ref
	FileSystem::FreeFile( fm );

	CM_InitBoxHull( );

	CM_FloodAreaConnections( );

	// allow this to be cached if it is loaded by the server
	if ( !clientload ) {

		cm.name = name;
	}
}

/*
==================
CM_ClearMap
==================
*/
void CollisionModel::CM_ClearMap( void ) {

	Common::Com_Memset( &cm, 0, sizeof( cm ) );
	CM_ClearLevelPatches();
}

/*
==================
CM_ClipHandleToModel
==================
*/
cmodel_t * CollisionModel::CM_ClipHandleToModel( clipHandle_t handle ) {

	if ( handle < 0 ) {

		Common::Com_Error( ERR_DROP, "CM_ClipHandleToModel: bad handle %i", handle );
	}

	if ( handle < cm.numSubModels ) {
		return &cm.cmodels[handle];
	}

	if ( handle == BOX_MODEL_HANDLE ) {
		return &box_model;
	}

	if ( handle < MAX_SUBMODELS ) {

		Common::Com_Error( ERR_DROP, "CM_ClipHandleToModel: bad handle %i < %i < %i", cm.numSubModels, handle, MAX_SUBMODELS );
	}

	Common::Com_Error( ERR_DROP, "CM_ClipHandleToModel: bad handle %i", handle + MAX_SUBMODELS );

	return NULL;

}

/*
==================
CM_InlineModel
==================
*/
clipHandle_t CollisionModel::CM_InlineModel( int index ) {

	if ( index < 0 || index >= cm.numSubModels ) {

		Common::Com_Error( ERR_DROP, "CM_InlineModel: bad number" );
	}

	return index;
}

int CollisionModel::CM_NumClusters( void ) {

	return cm.numClusters;
}

int CollisionModel::CM_NumInlineModels( void ) {

	return cm.numSubModels;
}

const Str CollisionModel::CM_EntityString( void ) {

	return cm.entityString;
}

int CollisionModel::CM_LeafCluster( int leafnum ) {

	if( leafnum < 0 || leafnum >= cm.numLeafs ) {

		Common::Com_Error( ERR_DROP, "CM_LeafCluster: bad number" );
	}

	return cm.leafs[leafnum].cluster;
}

int CollisionModel::CM_LeafArea( int leafnum ) {

	if ( leafnum < 0 || leafnum >= cm.numLeafs ) {

		Common::Com_Error( ERR_DROP, "CM_LeafArea: bad number" );
	}

	return cm.leafs[ leafnum ].area;
}

//=======================================================================


/*
===================
CM_InitBoxHull

Set up the planes and nodes so that the six floats of a bounding box
can just be stored out and get a proper clipping hull structure.
===================
*/
void CollisionModel::CM_InitBoxHull( ) {

	int				side;
	cplane_t *		p;
	cbrushside_t *	s;

	box_planes = &cm.planes[ cm.numPlanes ];

	box_brush = &cm.brushes[ cm.numBrushes ];
	box_brush->numsides = 6;
	box_brush->sides = cm.brushsides + cm.numBrushSides;
	box_brush->contents = CONTENTS_BODY;

	box_model.leaf.numLeafBrushes = 1;
	box_model.leaf.firstLeafBrush = cm.numLeafBrushes;
	cm.leafbrushes[ cm.numLeafBrushes ] = cm.numBrushes;

	for( int i = 0; i < 6; i++ ) {

		side = i&1;

		// brush sides
		s = &cm.brushsides[ cm.numBrushSides + i ];
		s->plane = &cm.planes[ cm.numPlanes + i * 2 + side ];
		s->surfaceFlags = 0;

		// planes
		p = &box_planes[ i * 2 ];
		p->type = ( planeType_e )( i >> 1 );
		p->signbits = 0;
		p->normal = vec3_origin;
		p->normal[ i >> 1 ] = 1;

		p = &box_planes[i*2+1];
		p->type = ( planeType_e )( 3 + ( i >> 1 ) );
		p->signbits = 0;
		p->normal = vec3_origin;
		p->normal[ i >> 1 ] = -1;

		p->SetPlaneSignbits( );
	}	
}

/*
===================
CM_TempBoxModel

To keep everything totally uniform, bounding boxes are turned into small
BSP trees instead of being compared directly.
Capsules are handled differently though.
===================
*/
clipHandle_t CollisionModel::CM_TempBoxModel( const Vec3 & mins, const Vec3 & maxs, int capsule ) {

	box_model.mins = mins;
	box_model.maxs = maxs;

	if ( capsule ) {

		return CAPSULE_MODEL_HANDLE;
	}

	box_planes[0].dist = maxs[0];
	box_planes[1].dist = -maxs[0];
	box_planes[2].dist = mins[0];
	box_planes[3].dist = -mins[0];
	box_planes[4].dist = maxs[1];
	box_planes[5].dist = -maxs[1];
	box_planes[6].dist = mins[1];
	box_planes[7].dist = -mins[1];
	box_planes[8].dist = maxs[2];
	box_planes[9].dist = -maxs[2];
	box_planes[10].dist = mins[2];
	box_planes[11].dist = -mins[2];

	box_brush->bounds[ 0 ] = mins;
	box_brush->bounds[ 1 ] = maxs;

	return BOX_MODEL_HANDLE;
}

/*
===================
CM_ModelBounds
===================
*/
void CollisionModel::CM_ModelBounds( clipHandle_t model, Vec3 & mins, Vec3 & maxs ) {

	cmodel_t	*cmod;

	cmod = CM_ClipHandleToModel( model );
	mins = cmod->mins;
	maxs = cmod->maxs;
}
