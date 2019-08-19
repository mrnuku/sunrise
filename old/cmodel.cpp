#include "precompiled.h"
#pragma hdrstop

int checkcount;

int numbrushsides;
cbrushside_t map_brushsides[ MAX_MAP_BRUSHSIDES ];

int numtexinfo;
mapsurface_t map_surfaces[ MAX_MAP_TEXINFO ];

int numplanes;
cplane_t map_planes[ MAX_MAP_PLANES+6 ]; // extra for box hull

int numnodes;
cnode_t map_nodes[ MAX_MAP_NODES+6 ]; // extra for box hull

int numleafs = 1; // allow leaf funcs to be called without a map
cleaf_t map_leafs[ MAX_MAP_LEAFS ];
int emptyleaf, solidleaf;

int numleafbrushes;
unsigned short map_leafbrushes[ MAX_MAP_LEAFBRUSHES ];

int numcmodels;
cmodel_t map_cmodels[ MAX_MAP_MODELS ];

int numbrushes;
cbrush_t map_brushes[ MAX_MAP_BRUSHES ];

int numvisibility;
byte map_visibility[ MAX_MAP_VISIBILITY ];
dvis_t * map_vis =( dvis_t * )map_visibility;

int numareas = 1;
carea_t map_areas[ MAX_MAP_AREAS ];

int numareaportals;
dareaportal_t map_areaportals[ MAX_MAP_AREAPORTALS ];

int numclusters = 1;

mapsurface_t nullsurface;

int floodvalid;

bool portalopen[ MAX_MAP_AREAPORTALS ];

int c_pointcontents;
int c_traces, c_brush_traces;


/*
===============================================================================
MAP LOADING
===============================================================================
*/
byte * cmod_base;

/*
=================
CMod_LoadSubmodels
=================
*/
void CollisionModel::CMod_LoadSubmodels( lump_t * l ) {

	dmodel_t * in;
	cmodel_t * out;
	int i, j, count;

	in =( dmodel_t * )( cmod_base + l->fileofs );
	if( l->filelen % sizeof( *in ) ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	count = l->filelen / sizeof( *in );

	if( count < 1 )
		Common::Com_Error( ERR_DROP, "Map with no models" );
	if( count > MAX_MAP_MODELS )
		Common::Com_Error( ERR_DROP, "Map has too many models" );

	numcmodels = count;

	for( i = 0; i<count; i++, in++, out++ )
	{
		out = &map_cmodels[ i ];

		for( j = 0; j<3; j++ )
		{ // spread the mins / maxs by a pixel
			out->mins[ j ] = LittleFloat( in->mins[ j ] ) - 1;
			out->maxs[ j ] = LittleFloat( in->maxs[ j ] ) + 1;
			out->origin[ j ] = LittleFloat( in->origin[ j ] );
		}
		out->headnode = LittleLong( in->headnode );
	}
}

/*
=================
CMod_LoadSurfaces
=================
*/
void CollisionModel::CMod_LoadSurfaces( lump_t * l ) {

	texinfo_t * in;
	mapsurface_t * out;
	int i, count;

	in =( texinfo_t * )( cmod_base + l->fileofs );
	if( l->filelen % sizeof( *in ) ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	count = l->filelen / sizeof( *in );
	if( count < 1 )
		Common::Com_Error( ERR_DROP, "Map with no surfaces" );
	if( count > MAX_MAP_TEXINFO )
		Common::Com_Error( ERR_DROP, "Map has too many surfaces" );

	numtexinfo = count;
	out = map_surfaces;

	for( i = 0; i<count; i++, in++, out++ ) {

		strncpy( out->c.name, in->texture, sizeof( out->c.name )-1 );
		strncpy( out->rname, in->texture, sizeof( out->rname )-1 );
		out->c.flags = LittleLong( in->flags );
		out->c.value = LittleLong( in->value );
	}
}

/*
=================
CMod_LoadNodes
=================
*/
void CollisionModel::CMod_LoadNodes( lump_t * l ) {

	dnode_t * in;
	int child;
	cnode_t * out;
	int i, j, count;

	in =( dnode_t * )( cmod_base + l->fileofs );
	if( l->filelen % sizeof( *in ) ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	count = l->filelen / sizeof( *in );

	if( count < 1 )
		Common::Com_Error( ERR_DROP, "Map has no nodes" );
	if( count > MAX_MAP_NODES )
		Common::Com_Error( ERR_DROP, "Map has too many nodes" );

	out = map_nodes;

	numnodes = count;

	for( i = 0; i<count; i++, out++, in++ )
	{
		out->plane = map_planes + LittleLong( in->planenum );
		for( j = 0; j<2; j++ )
		{
			child = LittleLong( in->children[ j ] );
			out->children[ j ] = child;
		}
	}

}

/*
=================
CMod_LoadBrushes
=================
*/
void CollisionModel::CMod_LoadBrushes( lump_t * l ) {

	dbrush_t * in;
	cbrush_t * out;
	int i, count;

	in =( dbrush_t * )( cmod_base + l->fileofs );
	if( l->filelen % sizeof( *in ) ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	count = l->filelen / sizeof( *in );

	if( count > MAX_MAP_BRUSHES )
		Common::Com_Error( ERR_DROP, "Map has too many brushes" );

	out = map_brushes;

	numbrushes = count;

	for( i = 0; i<count; i++, out++, in++ )
	{
		out->firstbrushside = LittleLong( in->firstside );
		out->numsides = LittleLong( in->numsides );
		out->contents = LittleLong( in->contents );
	}

}

/*
=================
CMod_LoadLeafs
=================
*/
void CollisionModel::CMod_LoadLeafs( lump_t * l ) {

	int i;
	cleaf_t * out;
	dleaf_t * in;
	int count;

	in =( dleaf_t * )( cmod_base + l->fileofs );
	if( l->filelen % sizeof( *in ) ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	count = l->filelen / sizeof( *in );

	if( count < 1 )
		Common::Com_Error( ERR_DROP, "Map with no leafs" );
	// need to save space for box planes
	if( count > MAX_MAP_PLANES )
		Common::Com_Error( ERR_DROP, "Map has too many planes" );

	out = map_leafs;
	numleafs = count;
	numclusters = 0;

	for( i = 0; i<count; i++, in++, out++ )
	{
		out->contents = LittleLong( in->contents );
		out->cluster = LittleShort( in->cluster );
		out->area = LittleShort( in->area );
		out->firstleafbrush = LittleShort( in->firstleafbrush );
		out->numleafbrushes = LittleShort( in->numleafbrushes );

		if( out->cluster >= numclusters )
			numclusters = out->cluster + 1;
	}

	if( map_leafs[ 0 ].contents != CONTENTS_SOLID )
		Common::Com_Error( ERR_DROP, "Map leaf 0 is not CONTENTS_SOLID" );
	solidleaf = 0;
	emptyleaf = -1;
	for( i = 1; i<numleafs; i++ )
	{
		if( !map_leafs[ i ].contents )
		{
			emptyleaf = i;
			break;
		}
	}
	if( emptyleaf == -1 )
		Common::Com_Error( ERR_DROP, "Map does not have an empty leaf" );
}

/*
=================
CMod_LoadPlanes
=================
*/
void CollisionModel::CMod_LoadPlanes( lump_t * l ) {

	int i, j;
	cplane_t * out;
	dplane_t * in;
	int count;
	int bits;

	in =( dplane_t * )( cmod_base + l->fileofs );
	if( l->filelen % sizeof( *in ) ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	count = l->filelen / sizeof( *in );

	if( count < 1 )
		Common::Com_Error( ERR_DROP, "Map with no planes" );
	// need to save space for box planes
	if( count > MAX_MAP_PLANES )
		Common::Com_Error( ERR_DROP, "Map has too many planes" );

	out = map_planes;
	numplanes = count;

	for( i = 0; i<count; i++, in++, out++ )
	{
		bits = 0;
		for( j = 0; j<3; j++ )
		{
			out->normal[ j ] = LittleFloat( in->normal[ j ] );
			if( out->normal[ j ] < 0 )
				bits |= 1<<j;
		}

		out->dist = LittleFloat( in->dist );
		out->type = LittleLong( in->type );
		out->signbits = bits;
	}
}

/*
=================
CMod_LoadLeafBrushes
=================
*/
void CollisionModel::CMod_LoadLeafBrushes( lump_t * l ) {

	int i;
	unsigned short * out;
	unsigned short * in;
	int count;

	in =( unsigned short * )( cmod_base + l->fileofs );
	if( l->filelen % sizeof( *in ) ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	count = l->filelen / sizeof( *in );

	if( count < 1 )
		Common::Com_Error( ERR_DROP, "Map with no planes" );
	// need to save space for box planes
	if( count > MAX_MAP_LEAFBRUSHES )
		Common::Com_Error( ERR_DROP, "Map has too many leafbrushes" );

	out = map_leafbrushes;
	numleafbrushes = count;

	for( i = 0; i<count; i++, in++, out++ )
		* out = LittleShort( *in );
}

/*
=================
CMod_LoadBrushSides
=================
*/
void CollisionModel::CMod_LoadBrushSides( lump_t * l ) {

	int i, j;
	cbrushside_t * out;
	dbrushside_t * in;
	int count;
	int num;

	in =( dbrushside_t * )( cmod_base + l->fileofs );
	if( l->filelen % sizeof( *in ) ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	count = l->filelen / sizeof( *in );

	// need to save space for box planes
	if( count > MAX_MAP_BRUSHSIDES )
		Common::Com_Error( ERR_DROP, "Map has too many planes" );

	out = map_brushsides;
	numbrushsides = count;

	for( i = 0; i<count; i++, in++, out++ )
	{
		num = LittleShort( in->planenum );
		out->plane = &map_planes[ num ];
		j = LittleShort( in->texinfo );
		if( j >= numtexinfo )
			Common::Com_Error( ERR_DROP, "Bad brushside texinfo" );
		out->surface = &map_surfaces[ j ];
	}
}

/*
=================
CMod_LoadAreas
=================
*/
void CollisionModel::CMod_LoadAreas( lump_t * l ) {

	int i;
	carea_t * out;
	darea_t * in;
	int count;

	in =( darea_t * )( cmod_base + l->fileofs );
	if( l->filelen % sizeof( *in ) ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	count = l->filelen / sizeof( *in );

	if( count > MAX_MAP_AREAS )
		Common::Com_Error( ERR_DROP, "Map has too many areas" );

	out = map_areas;
	numareas = count;

	for( i = 0; i<count; i++, in++, out++ )
	{
		out->numareaportals = LittleLong( in->numareaportals );
		out->firstareaportal = LittleLong( in->firstareaportal );
		out->floodvalid = 0;
		out->floodnum = 0;
	}
}

/*
=================
CMod_LoadAreaPortals
=================
*/
void CollisionModel::CMod_LoadAreaPortals( lump_t * l ) {

	int i;
	dareaportal_t * out;
	dareaportal_t * in;
	int count;

	in =( dareaportal_t * )( cmod_base + l->fileofs );
	if( l->filelen % sizeof( *in ) ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	count = l->filelen / sizeof( *in );

	if( count > MAX_MAP_AREAS )
		Common::Com_Error( ERR_DROP, "Map has too many areas" );

	out = map_areaportals;
	numareaportals = count;

	for( i = 0; i<count; i++, in++, out++ )
	{
		out->portalnum = LittleLong( in->portalnum );
		out->otherarea = LittleLong( in->otherarea );
	}
}

/*
=================
CMod_LoadVisibility
=================
*/
void CollisionModel::CMod_LoadVisibility( lump_t * l ) {

	int i;

	numvisibility = l->filelen;
	if( l->filelen > MAX_MAP_VISIBILITY )
		Common::Com_Error( ERR_DROP, "Map has too large visibility lump" );

	memcpy( map_visibility, cmod_base + l->fileofs, l->filelen );

	map_vis->numclusters = LittleLong( map_vis->numclusters );
	for( i = 0; i<map_vis->numclusters; i++ )
	{
		map_vis->bitofs[ i ][ 0 ] = LittleLong( map_vis->bitofs[ i ][ 0 ] );
		map_vis->bitofs[ i ][ 1 ] = LittleLong( map_vis->bitofs[ i ][ 1 ] );
	}
}


/*
=================
CMod_LoadEntityString
=================
*/
void CollisionModel::CMod_LoadEntityString( lump_t * l ) {

	if( l->filelen > MAX_MAP_ENTSTRING ) Common::Com_Error( ERR_DROP, "Map has too large entity lump" );

	map_entitystring = Str( ( const char * )cmod_base + l->fileofs, 0, l->filelen - 1 );
}



/*
==================
CM_LoadMap

Loads in the map and all submodels
==================
*/
cmodel_t * CollisionModel::CM_LoadMap( const Str & name, bool clientload, unsigned * checksum ) {

	int i;
	dheader_t header;
	static unsigned			last_checksum;
	
	if( map_name == name &&( clientload || !CVarSystem::GetCVarBool( "flushmap" ) ) ) {

		* checksum = last_checksum;
		if( !clientload ) {

			memset( portalopen, 0, sizeof( portalopen ) );
			FloodAreaConnections( );
		}
		return &map_cmodels[ 0 ]; // still have the right version
	}

	// free old stuff
	numplanes = 0;
	numnodes = 0;
	numleafs = 0;
	numcmodels = 0;
	numvisibility = 0;
	map_entitystring.Clear( );
	map_name.Clear( );

	if( name.IsEmpty( ) ) {

		numleafs = 1;
		numclusters = 1;
		numareas = 1;
		* checksum = 0;
		return &map_cmodels[ 0 ]; // cinematic servers won't have anything at all
	}

	//
	// load the file
	//
	File_Memory fm = FileSystem::ReadFile( name );

	last_checksum = LittleLong( Common::Com_BlockChecksum( fm.GetDataPtr( ), fm.Length( ) ) );
	*checksum = last_checksum;

	header = * ( dheader_t * )fm.GetDataPtr( );
	for( i = 0; i < sizeof( dheader_t ) / 4; i++ )( ( int * )&header )[ i ] = LittleLong( ( ( int * ) &header )[ i ] );

	if( header.version != BSPVERSION ) Common::Com_Error( ERR_DROP, "CMod_LoadBrushModel: %s has wrong version number( %i should be %i )", name.c_str( ), header.version, BSPVERSION );

	cmod_base =( byte * )fm.GetDataPtr( );

	// load into heap
	CMod_LoadSurfaces( &header.lumps[ LUMP_TEXINFO ] );
	CMod_LoadLeafs( &header.lumps[ LUMP_LEAFS ] );
	CMod_LoadLeafBrushes( &header.lumps[ LUMP_LEAFBRUSHES ] );
	CMod_LoadPlanes( &header.lumps[ LUMP_PLANES ] );
	CMod_LoadBrushes( &header.lumps[ LUMP_BRUSHES ] );
	CMod_LoadBrushSides( &header.lumps[ LUMP_BRUSHSIDES ] );
	CMod_LoadSubmodels( &header.lumps[ LUMP_MODELS ] );
	CMod_LoadNodes( &header.lumps[ LUMP_NODES ] );
	CMod_LoadAreas( &header.lumps[ LUMP_AREAS ] );
	CMod_LoadAreaPortals( &header.lumps[ LUMP_AREAPORTALS ] );
	CMod_LoadVisibility( &header.lumps[ LUMP_VISIBILITY ] );
	CMod_LoadEntityString( &header.lumps[ LUMP_ENTITIES ] );

	FileSystem::FreeFile( fm );

	CM_InitBoxHull( );

	memset( portalopen, 0, sizeof( portalopen ) );
	FloodAreaConnections( );

	map_name = name;

	return &map_cmodels[ 0 ];
}

/*
==================
CM_InlineModel
==================
*/
cmodel_t * CollisionModel::CM_InlineModel( const Str & name ) {

	int num;

	if( name.IsEmpty( ) || name[ 0 ] != '*' ) Common::Com_Error( ERR_DROP, "CM_InlineModel: bad name" );

	Token token;
	token = name.Right( name.Length( ) - 1 );
	token.type = TT_NUMBER;
	token.subtype = TT_DECIMAL;

	num = token.GetIntValue( );

	if( num < 1 || num >= numcmodels ) Common::Com_Error( ERR_DROP, "CM_InlineModel: bad number" );

	return &map_cmodels[ num ];
}

int CollisionModel::CM_NumClusters( ) {

	return numclusters;
}

int CollisionModel::CM_NumInlineModels( ) {

	return numcmodels;
}

Str CollisionModel::CM_EntityString( ) {

	return map_entitystring;
}

int CollisionModel::CM_LeafContents( int leafnum ) {

	if( leafnum < 0 || leafnum >= numleafs )
		Common::Com_Error( ERR_DROP, "CM_LeafContents: bad number" );
	return map_leafs[ leafnum ].contents;
}

int CollisionModel::CM_LeafCluster( int leafnum ) {

	if( leafnum < 0 || leafnum >= numleafs )
		Common::Com_Error( ERR_DROP, "CM_LeafCluster: bad number" );
	return map_leafs[ leafnum ].cluster;
}

int CollisionModel::CM_LeafArea( int leafnum ) {

	if( leafnum < 0 || leafnum >= numleafs )
		Common::Com_Error( ERR_DROP, "CM_LeafArea: bad number" );
	return map_leafs[ leafnum ].area;
}

//=======================================================================


cplane_t * box_planes;
int box_headnode;
cbrush_t * box_brush;
cleaf_t * box_leaf;

/*
===================
CM_InitBoxHull

Set up the planes and nodes so that the six floats of a bounding box
can just be stored out and get a proper clipping hull structutpRenderer::
===================
*/
void CollisionModel::CM_InitBoxHull( ) {

	int i;
	int side;
	cnode_t * c;
	cplane_t * p;
	cbrushside_t * s;

	box_headnode = numnodes;
	box_planes = &map_planes[ numplanes ];
	if( numnodes+6 > MAX_MAP_NODES
		|| numbrushes+1 > MAX_MAP_BRUSHES
		|| numleafbrushes+1 > MAX_MAP_LEAFBRUSHES
		|| numbrushsides+6 > MAX_MAP_BRUSHSIDES
		|| numplanes+12 > MAX_MAP_PLANES )
		Common::Com_Error( ERR_DROP, "Not enough room for box tree" );

	box_brush = &map_brushes[ numbrushes ];
	box_brush->numsides = 6;
	box_brush->firstbrushside = numbrushsides;
	box_brush->contents = CONTENTS_MONSTER;

	box_leaf = &map_leafs[ numleafs ];
	box_leaf->contents = CONTENTS_MONSTER;
	box_leaf->firstleafbrush = numleafbrushes;
	box_leaf->numleafbrushes = 1;

	map_leafbrushes[ numleafbrushes ] = numbrushes;

	for( i = 0; i<6; i++ )
	{
		side = i&1;

		// brush sides
		s = &map_brushsides[ numbrushsides+i ];
		s->plane = map_planes +( numplanes+i* 2+side );
		s->surface = &nullsurface;

		// nodes
		c = &map_nodes[ box_headnode+i ];
		c->plane = map_planes +( numplanes+i* 2 );
		c->children[ side ] = -1 - emptyleaf;
		if( i != 5 )
			c->children[ side^1 ] = box_headnode+i + 1;
		else
			c->children[ side^1 ] = -1 - numleafs;

		// planes
		p = &box_planes[i* 2];
		p->type = i>>1;
		p->signbits = 0;
		p->normal = vec3_origin;
		p->normal[ i>>1 ] = 1;

		p = &box_planes[i* 2+1];
		p->type = 3 +( i>>1 );
		p->signbits = 0;
		p->normal = vec3_origin;
		p->normal[ i>>1 ] = -1;
	}
}

/*
===================
CM_HeadnodeForBox

To keep everything totally uniform, bounding boxes are turned into small
BSP trees instead of being compared directly.
===================
*/
int CollisionModel::CM_HeadnodeForBox( Vec3 & mins, Vec3 & maxs ) {

	box_planes[ 0 ].dist = maxs[ 0 ];
	box_planes[ 1 ].dist = -maxs[ 0 ];
	box_planes[ 2 ].dist = mins[ 0 ];
	box_planes[ 3 ].dist = -mins[ 0 ];
	box_planes[ 4 ].dist = maxs[ 1 ];
	box_planes[ 5 ].dist = -maxs[ 1 ];
	box_planes[ 6 ].dist = mins[ 1 ];
	box_planes[ 7 ].dist = -mins[ 1 ];
	box_planes[ 8 ].dist = maxs[ 2 ];
	box_planes[ 9 ].dist = -maxs[ 2 ];
	box_planes[ 10 ].dist = mins[ 2 ];
	box_planes[ 11 ].dist = -mins[ 2 ];

	return box_headnode;
}


/*
==================
CM_PointLeafnum_r
==================
*/
int CollisionModel::CM_PointLeafnum_r( Vec3 & p, int num ) {

	float d;
	cnode_t * node;
	cplane_t * plane;

	while( num >= 0 )
	{
		node = map_nodes + num;
		plane = node->plane;

		if( plane->type < 3 )
			d = p[ plane->type ] - plane->dist;
		else
			d = plane->normal * p - plane->dist;
		if( d < 0 )
			num = node->children[ 1 ];
		else
			num = node->children[ 0 ];
	}

	c_pointcontents++; // optimize counter

	return -1 - num;
}

int CollisionModel::CM_PointLeafnum( Vec3 & p ) {

	if( !numplanes )
		return 0; // sound may call this without map loaded
	return CM_PointLeafnum_r( p, 0 );
}

/*
=============
CM_BoxLeafnums

Fills in a list of all the leafs touched
=============
*/
int leaf_count, leaf_maxcount;
int * leaf_list;
Vec3 leaf_mins, leaf_maxs;
int leaf_topnode;

void CollisionModel::CM_BoxLeafnums_r( int nodenum ) {

	cplane_t * plane;
	cnode_t * node;
	int s;

	while( 1 )
	{
		if( nodenum < 0 )
		{
			if( leaf_count >= leaf_maxcount )
			{
				//				Com_Printf( "CM_BoxLeafnums_r: overflow\n" );
				return;
			}
			leaf_list[ leaf_count++ ] = -1 - nodenum;
			return;
		}

		node = &map_nodes[ nodenum ];
		plane = node->plane;
		//		s = BoxOnPlaneSide( leaf_mins, leaf_maxs, plane );
		s = BOX_ON_PLANE_SIDE( leaf_mins, leaf_maxs, plane );
		if( s == 1 )
			nodenum = node->children[ 0 ];
		else if( s == 2 )
			nodenum = node->children[ 1 ];
		else
		{ // go down both
			if( leaf_topnode == -1 )
				leaf_topnode = nodenum;
			CM_BoxLeafnums_r( node->children[ 0 ] );
			nodenum = node->children[ 1 ];
		}

	}
}

int CollisionModel::CM_BoxLeafnums_headnode( Vec3 & mins, Vec3 & maxs, int * list, int listsize, int headnode, int * topnode ) {

	leaf_list = list;
	leaf_count = 0;
	leaf_maxcount = listsize;
	leaf_mins = mins;
	leaf_maxs = maxs;

	leaf_topnode = -1;

	CM_BoxLeafnums_r( headnode );

	if( topnode )
		* topnode = leaf_topnode;

	return leaf_count;
}

int CollisionModel::CM_BoxLeafnums( Vec3 & mins, Vec3 & maxs, int * list, int listsize, int * topnode ) {

	return CM_BoxLeafnums_headnode( mins, maxs, list, listsize, map_cmodels[ 0 ].headnode, topnode );
}

/*
==================
CM_PointContents
==================
*/
int CollisionModel::CM_PointContents( Vec3 & p, int headnode ) {

	int l;

	if( !numnodes ) // map not loaded
		return 0;

	l = CM_PointLeafnum_r( p, headnode );

	return map_leafs[ l ].contents;
}

/*
==================
CM_TransformedPointContents

Handles offseting and rotation of the end points for moving and
rotating entities
==================
*/
int CollisionModel::CM_TransformedPointContents( Vec3 & p, int headnode, Vec3 & origin, Vec3 & angles ) {

	Vec3 p_l = p - origin; // subtract origin offset
	Vec3 temp;
	Vec3 forward, right, up;
	int l;

	// rotate start and end into the models frame of reference
	if( headnode != box_headnode &&
		( angles[ 0 ] || angles[ 1 ] || angles[ 2 ] ) )
	{
		angles.AngleVectors( &forward, &right, &up );

		temp = p_l;
		p_l[ 0 ] = temp * forward;
		p_l[ 1 ] = -( temp * right );
		p_l[ 2 ] = temp * up;
	}

	l = CM_PointLeafnum_r( p_l, headnode );

	return map_leafs[ l ].contents;
}

/*
===============================================================================
BOX TRACING
===============================================================================

*/
// 1/32 epsilon to keep floating point happy


Vec3 trace_start, trace_end;
Vec3 trace_mins, trace_maxs;
Vec3 trace_extents;

trace_t trace_trace;
int trace_contents;
bool trace_ispoint; // optimized case

/*
================
CM_ClipBoxToBrush
================
*/
void CollisionModel::CM_ClipBoxToBrush( Vec3 & mins, Vec3 & maxs, Vec3 & p1, Vec3 & p2, trace_t * trace, cbrush_t * brush ) {

	int i, j;
	cplane_t * plane, * clipplane;
	float dist;
	float enterfrac, leavefrac;
	Vec3 ofs;
	float d1, d2;
	bool getout, startout;
	float f;
	cbrushside_t * side, * leadside;

	enterfrac = -1;
	leavefrac = 1;
	clipplane = NULL;

	if( !brush->numsides )
		return;

	c_brush_traces++;

	getout = false;
	startout = false;
	leadside = NULL;

	for( i = 0; i<brush->numsides; i++ )
	{
		side = &map_brushsides[ brush->firstbrushside+i ];
		plane = side->plane;

		// FIXME: special case for axial

		if( !trace_ispoint )
		{ // general box case

			// push the plane out apropriately for mins/maxs

			// FIXME: use signbits into 8 way lookup for each mins/maxs
			for( j = 0; j<3; j++ )
			{
				if( plane->normal[ j ] < 0 )
					ofs[ j ] = maxs[ j ];
				else
					ofs[ j ] = mins[ j ];
			}
			dist = ofs * plane->normal;
			dist = plane->dist - dist;
		}
		else
		{ // special point case
			dist = plane->dist;
		}

		d1 = p1 * plane->normal - dist;
		d2 = p2 * plane->normal - dist;

		if( d2 > 0 )
			getout = true; // endpoint is not in solid
		if( d1 > 0 )
			startout = true;

		// if completely in front of face, no intersection
		if( d1 > 0 && d2 >= d1 )
			return;

		if( d1 <= 0 && d2 <= 0 )
			continue;

		// crosses face
		if( d1 > d2 )
		{ // enter
			f =( d1-( 0.03125f ) ) /( d1-d2 );
			if( f > enterfrac )
			{
				enterfrac = f;
				clipplane = plane;
				leadside = side;
			}
		}
		else
		{ // leave
			f =( d1+( 0.03125f ) ) /( d1-d2 );
			if( f < leavefrac )
				leavefrac = f;
		}
	}

	if( !startout )
	{ // original point was inside brush
		trace->startsolid = true;
		if( !getout )
			trace->allsolid = true;
		return;
	}
	if( enterfrac < leavefrac )
	{
		if( enterfrac > -1 && enterfrac < trace->fraction )
		{
			if( enterfrac < 0 )
				enterfrac = 0;
			trace->fraction = enterfrac;
			trace->plane = * clipplane;
			trace->surface = &( leadside->surface->c );
			trace->contents = brush->contents;
		}
	}
}

/*
================
CM_TestBoxInBrush
================
*/
void CollisionModel::CM_TestBoxInBrush( Vec3 & mins, Vec3 & maxs, Vec3 & p1, trace_t * trace, cbrush_t * brush ) {

	int i, j;
	cplane_t * plane;
	float dist;
	Vec3 ofs;
	float d1;
	cbrushside_t * side;

	if( !brush->numsides )
		return;

	for( i = 0; i<brush->numsides; i++ )
	{
		side = &map_brushsides[ brush->firstbrushside+i ];
		plane = side->plane;

		// FIXME: special case for axial

		// general box case

		// push the plane out apropriately for mins/maxs

		// FIXME: use signbits into 8 way lookup for each mins/maxs
		for( j = 0; j<3; j++ )
		{
			if( plane->normal[ j ] < 0 )
				ofs[ j ] = maxs[ j ];
			else
				ofs[ j ] = mins[ j ];
		}
		dist = ofs * plane->normal;
		dist = plane->dist - dist;

		d1 = p1 * plane->normal - dist;

		// if completely in front of face, no intersection
		if( d1 > 0 )
			return;

	}

	// inside this brush
	trace->startsolid = trace->allsolid = true;
	trace->fraction = 0;
	trace->contents = brush->contents;
}

/*
================
CM_TraceToLeaf
================
*/
void CollisionModel::CM_TraceToLeaf( int leafnum ) {

	int k;
	int brushnum;
	cleaf_t * leaf;
	cbrush_t * b;

	leaf = &map_leafs[ leafnum ];
	if( !( leaf->contents & trace_contents ) ) return;
	// trace line against all brushes in the leaf
	for( k = 0; k<leaf->numleafbrushes; k++ )
	{
		brushnum = map_leafbrushes[ leaf->firstleafbrush+k ];
		b = &map_brushes[ brushnum ];
		if( b->checkcount == checkcount )
			continue; // already checked this brush in another leaf
		b->checkcount = checkcount;

		if( !( b->contents & trace_contents ) ) 	continue;
		CM_ClipBoxToBrush( trace_mins, trace_maxs, trace_start, trace_end, &trace_trace, b );
		if( !trace_trace.fraction )
			return;
	}

}

/*
================
CM_TestInLeaf
================
*/
void CollisionModel::CM_TestInLeaf( int leafnum ) {

	int k;
	int brushnum;
	cleaf_t * leaf;
	cbrush_t * b;

	leaf = &map_leafs[ leafnum ];
	if( !( leaf->contents & trace_contents ) ) return;
	// trace line against all brushes in the leaf
	for( k = 0; k<leaf->numleafbrushes; k++ )
	{
		brushnum = map_leafbrushes[ leaf->firstleafbrush+k ];
		b = &map_brushes[ brushnum ];
		if( b->checkcount == checkcount )
			continue; // already checked this brush in another leaf
		b->checkcount = checkcount;

		if( !( b->contents & trace_contents ) ) 	continue;
		CM_TestBoxInBrush( trace_mins, trace_maxs, trace_start, &trace_trace, b );
		if( !trace_trace.fraction )
			return;
	}

}

/*
==================
CM_RecursiveHullCheck
==================
*/
void CollisionModel::CM_RecursiveHullCheck( int num, float p1f, float p2f, Vec3 & p1, Vec3 & p2 ) {

	cnode_t * node;
	cplane_t * plane;
	float t1, t2, offset;
	float frac, frac2;
	float ist;
	int i;
	Vec3 mid;
	int side;
	float midf;

	if( trace_trace.fraction <= p1f )
		return; // already hit something nearer

	// if < 0, we are in a leaf node
	if( num < 0 )
	{
		CM_TraceToLeaf( -1-num );
		return;
	}

	//
	// find the point distances to the seperating plane
	// and the offset for the size of the box
	//
	node = map_nodes + num;
	plane = node->plane;

	if( plane->type < 3 )
	{
		t1 = p1[ plane->type ] - plane->dist;
		t2 = p2[ plane->type ] - plane->dist;
		offset = trace_extents[ plane->type ];
	}
	else
	{
		t1 = plane->normal * p1 - plane->dist;
		t2 = plane->normal * p2 - plane->dist;
		if( trace_ispoint )
			offset = 0;
		else
			offset = fabs( trace_extents[ 0 ]* plane->normal[ 0 ] ) +
			fabs( trace_extents[ 1 ]* plane->normal[ 1 ] ) +
			fabs( trace_extents[ 2 ]* plane->normal[ 2 ] );
	}
	// see which sides we need to consider
	if( t1 >= offset && t2 >= offset )
	{
		CM_RecursiveHullCheck( node->children[ 0 ], p1f, p2f, p1, p2 );
		return;
	}
	if( t1 < -offset && t2 < -offset )
	{
		CM_RecursiveHullCheck( node->children[ 1 ], p1f, p2f, p1, p2 );
		return;
	}

	// put the crosspoint DIST_EPSILON pixels on the near side
	if( t1 < t2 )
	{
		ist = 1.0f/( t1-t2 );
		side = 1;
		frac2 =( t1 + offset +( 0.03125f ) )* ist;
		frac =( t1 - offset +( 0.03125f ) )* ist;
	}
	else if( t1 > t2 )
	{
		ist = 1.0f/( t1-t2 );
		side = 0;
		frac2 =( t1 - offset -( 0.03125f ) )* ist;
		frac =( t1 + offset +( 0.03125f ) )* ist;
	}
	else
	{
		side = 0;
		frac = 1;
		frac2 = 0;
	}

	// move up to the node
	if( frac < 0 )
		frac = 0;
	if( frac > 1 )
		frac = 1;

	midf = p1f +( p2f - p1f )* frac;
	for( i = 0; i<3; i++ )
		mid[ i ] = p1[ i ] + frac* ( p2[ i ] - p1[ i ] );

	CM_RecursiveHullCheck( node->children[ side ], p1f, midf, p1, mid );


	// go past the node
	if( frac2 < 0 )
		frac2 = 0;
	if( frac2 > 1 )
		frac2 = 1;

	midf = p1f +( p2f - p1f )* frac2;
	for( i = 0; i<3; i++ )
		mid[ i ] = p1[ i ] + frac2* ( p2[ i ] - p1[ i ] );

	CM_RecursiveHullCheck( node->children[ side^1 ], midf, p2f, mid, p2 );
}



//======================================================================

/*
==================
CM_BoxTrace
==================
*/
trace_t CollisionModel::CM_BoxTrace( Vec3 & start, Vec3 & end, Vec3 & mins, Vec3 & maxs, int headnode, int brushmask ) {

	int i;

	checkcount++; // for multi-check avoidance

	c_traces++; // for statistics, may be zeroed

	// fill in a default trace
	memset( &trace_trace, 0, sizeof( trace_trace ) );
	trace_trace.fraction = 1;
	trace_trace.surface = &( nullsurface.c );

	if( !numnodes ) // map not loaded
		return trace_trace;

	trace_contents = brushmask;
	trace_start = start;
	trace_end = end;
	trace_mins = mins;
	trace_maxs = maxs;

	//
	// check for position test special case
	//
	if( start[ 0 ] == end[ 0 ] && start[ 1 ] == end[ 1 ] && start[ 2 ] == end[ 2 ] )
	{
		int leafs[ 1024 ];
		int i, numleafs;
		Vec3 c1, c2;
		int topnode;

		c1 = start + mins;
		c2 = start + maxs;
		for( i = 0; i<3; i++ )
		{
			c1[ i ] -= 1;
			c2[ i ] += 1;
		}

		numleafs = CM_BoxLeafnums_headnode( c1, c2, leafs, 1024, headnode, &topnode );
		for( i = 0; i<numleafs; i++ )
		{
			CM_TestInLeaf( leafs[ i ] );
			if( trace_trace.allsolid )
				break;
		}
		trace_trace.endpos = start;
		return trace_trace;
	}

	//
	// check for point special case
	//
	if( mins[ 0 ] == 0 && mins[ 1 ] == 0 && mins[ 2 ] == 0
		&& maxs[ 0 ] == 0 && maxs[ 1 ] == 0 && maxs[ 2 ] == 0 )
	{
		trace_ispoint = true;
		trace_extents = vec3_origin;
	}
	else
	{
		trace_ispoint = false;
		trace_extents[ 0 ] = -mins[ 0 ] > maxs[ 0 ] ? -mins[ 0 ] : maxs[ 0 ];
		trace_extents[ 1 ] = -mins[ 1 ] > maxs[ 1 ] ? -mins[ 1 ] : maxs[ 1 ];
		trace_extents[ 2 ] = -mins[ 2 ] > maxs[ 2 ] ? -mins[ 2 ] : maxs[ 2 ];
	}

	//
	// general sweeping through world
	//
	CM_RecursiveHullCheck( headnode, 0, 1, start, end );

	if( trace_trace.fraction == 1 )
	{
		trace_trace.endpos = end;
	}
	else
	{
		for( i = 0; i<3; i++ )
			trace_trace.endpos[ i ] = start[ i ] + trace_trace.fraction * ( end[ i ] - start[ i ] );
	}
	return trace_trace;
}


/*
==================
CM_TransformedBoxTrace

Handles offseting and rotation of the end points for moving and
rotating entities
==================
*/
trace_t CollisionModel::CM_TransformedBoxTrace( Vec3 & start, Vec3 & end, Vec3 & mins, Vec3 & maxs, int headnode, int brushmask, Vec3 & origin, Vec3 & angles ) {

	trace_t trace;
	Vec3 start_l = start - origin; // subtract origin offset
	Vec3 end_l = end - origin;
	Vec3 a;
	Vec3 forward, right, up;
	Vec3 temp;
	bool rotated;

	// rotate start and end into the models frame of reference
	if( headnode != box_headnode &&( angles[ 0 ] || angles[ 1 ] || angles[ 2 ] ) ) rotated = true;
	else rotated = false;

	if( rotated ) {

		angles.AngleVectors( &forward, &right, &up );

		temp = start_l;
		start_l[ 0 ] = temp * forward;
		start_l[ 1 ] = -( temp * right );
		start_l[ 2 ] = temp * up;

		temp = end_l;
		end_l[ 0 ] = temp * forward;
		end_l[ 1 ] = -( temp * right );
		end_l[ 2 ] = temp * up;
	}

	// sweep the box through the model
	trace = CM_BoxTrace( start_l, end_l, mins, maxs, headnode, brushmask );

	if( rotated && trace.fraction != 1.0f )
	{
		// FIXME: figure out how to do this with existing angles
		a = angles * -1.0f;
		a.AngleVectors( &forward, &right, &up );

		temp = trace.plane.normal;
		trace.plane.normal[ 0 ] = temp * forward;
		trace.plane.normal[ 1 ] = -( temp * right );
		trace.plane.normal[ 2 ] = temp * up;
	}

	trace.endpos[ 0 ] = start[ 0 ] + trace.fraction * ( end[ 0 ] - start[ 0 ] );
	trace.endpos[ 1 ] = start[ 1 ] + trace.fraction * ( end[ 1 ] - start[ 1 ] );
	trace.endpos[ 2 ] = start[ 2 ] + trace.fraction * ( end[ 2 ] - start[ 2 ] );

	return trace;
}

/*
===============================================================================
PVS / PHS
===============================================================================
*/
/*
===================
CM_DecompressVis
===================
*/
void CollisionModel::CM_DecompressVis( byte * in, byte * out ) {

	int c;
	byte * out_p;
	int row;

	row =( numclusters+7 )>>3;
	out_p = out;

	if( !in || !numvisibility )
	{ // no vis info, so make all visible
		while( row )
		{
			* out_p++ = 0xFF;
			row--;
		}
		return;
	}

	do
	{
		if( *in )
		{
			* out_p++ = * in++;
			continue;
		}

		c = in[ 1 ];
		in += 2;
		if( ( out_p - out ) + c > row )
		{
			c = row -( out_p - out );
			Common::Com_DPrintf( "warning: Vis decompression overrun\n" );
		}
		while( c )
		{
			* out_p++ = 0;
			c--;
		}
	} while( out_p - out < row );
}

byte pvsrow[ MAX_MAP_LEAFS/8 ];
byte phsrow[ MAX_MAP_LEAFS/8 ];

byte * CollisionModel::CM_ClusterPVS( int cluster ) {

	if( cluster == -1 ) memset( pvsrow, 0, ( numclusters+7 )>>3 );
	else CM_DecompressVis( map_visibility + map_vis->bitofs[ cluster][DVIS_PVS ], pvsrow );
	return pvsrow;
}

byte * CollisionModel::CM_ClusterPHS( int cluster ) {

	if( cluster == -1 ) memset( phsrow, 0, ( numclusters+7 )>>3 );
	else CM_DecompressVis( map_visibility + map_vis->bitofs[ cluster][DVIS_PHS ], phsrow );
	return phsrow;
}

/*
===============================================================================
AREAPORTALS
===============================================================================
*/
void CollisionModel::FloodArea_r( carea_t * area, int floodnum ) {

	if( area->floodvalid == floodvalid ) {

		if( area->floodnum == floodnum ) return;

		Common::Com_Error( ERR_DROP, "FloodArea_r: reflooded" );
	}

	area->floodnum = floodnum;
	area->floodvalid = floodvalid;

	dareaportal_t * p = &map_areaportals[ area->firstareaportal ];

	for( int i = 0; i < area->numareaportals; i++, p++ ) {

		if( portalopen[ p->portalnum ] ) FloodArea_r( &map_areas[ p->otherarea ], floodnum );
	}
}

/*
====================
FloodAreaConnections
====================
*/
void CollisionModel::FloodAreaConnections( ) {

	carea_t * area;
	int floodnum = 0;

	// all current floods are now invalid
	floodvalid++;

	// area 0 is not used
	for( int i = 1; i < numareas; i++ ) {

		area = &map_areas[ i ];

		if( area->floodvalid == floodvalid ) continue; // already flooded into

		floodnum++;
		FloodArea_r( area, floodnum );
	}
}

void CollisionModel::CM_SetAreaPortalState( int portalnum, bool open ) {

	if( portalnum > numareaportals )
		Common::Com_Error( ERR_DROP, "areaportal > numareaportals" );

	portalopen[ portalnum ] = open;
	FloodAreaConnections( );
}

bool CollisionModel::CM_AreasConnected( int area1, int area2 ) {

	if( map_noareas.GetBool( ) ) return true;

	if( area1 > numareas || area2 > numareas ) Common::Com_Error( ERR_DROP, "area > numareas" );

	if( map_areas[ area1 ].floodnum == map_areas[ area2 ].floodnum ) return true;

	return false;
}

/*
=================
CM_WriteAreaBits

Writes a length byte followed by a bit vector of all the areas
that area in the same flood as the area parameter

This is used by the client refreshes to cull visibility
=================
*/
int CollisionModel::CM_WriteAreaBits( byte * buffer, int area ) {

	int i;
	int floodnum;
	int bytes;

	bytes =( numareas + 7 ) >> 3;

	if( map_noareas.GetBool( ) ) { // for debugging, send everything

		memset( buffer, 255, bytes );

	} else {

		memset( buffer, 0, bytes );

		floodnum = map_areas[ area ].floodnum;
		for( i = 0; i<numareas; i++ ) {

			if( map_areas[ i ].floodnum == floodnum || !area ) buffer[ i >> 3 ] |= 1 << ( i & 7 );
		}
	}

	return bytes;
}

/*

===================
CM_WritePortalState

Writes the portal state to a savegame file
===================
*/
void CollisionModel::CM_WritePortalState( File & f ) {

	f.Write( portalopen, sizeof( portalopen ) );
}

/*
===================
CM_ReadPortalState

Reads the portal state from a savegame file
and recalculates the area connections
===================
*/
void CollisionModel::CM_ReadPortalState( File & f ) {

	f.Read( portalopen, sizeof( portalopen ) );
	FloodAreaConnections( );
}

/*
=============
CM_HeadnodeVisible

Returns true if any leaf under headnode has a cluster that
is potentially visible
=============
*/
bool CollisionModel::CM_HeadnodeVisible( int nodenum, byte * visbits ) {

	int leafnum;
	int cluster;
	cnode_t * node;

	if( nodenum < 0 )
	{
		leafnum = -1-nodenum;
		cluster = map_leafs[ leafnum ].cluster;
		if( cluster == -1 )
			return false;
		if( visbits[ cluster>>3 ] &( 1<<( cluster&7 ) ) )
			return true;
		return false;
	}

	node = &map_nodes[ nodenum ];
	if( CM_HeadnodeVisible( node->children[ 0 ], visbits ) ) return true;
	return CM_HeadnodeVisible( node->children[ 1 ], visbits );
}
