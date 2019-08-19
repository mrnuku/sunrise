#include "precompiled.h"
#pragma hdrstop

model_t	* Renderer::loadmodel;
int		Renderer::modfilelen;
byte	Renderer::mod_novis[ MAX_MAP_LEAFS/8 ];
int		Renderer::mod_numknown;
int		Renderer::registration_sequence;
byte *	Renderer::mod_base;

/*
===============
Mod_PointInLeaf
===============
*/
mleaf_t * Renderer::Mod_PointInLeaf( Vec3 & p, model_t * model ) {

	mnode_t		* node;
	float		d;
	Plane plane;
	
	if( !model || !model->nodes )
		Common::Com_Error( ERR_DROP, "Mod_PointInLeaf: bad model" );

	node = model->nodes;
	while( 1 )
	{
		if( node->contents != -1 )
			return( mleaf_t * )node;
		plane = node->plane;
		d =( p * plane.Normal( ) ) - plane.Dist( );
		if( d > 0 ) node = node->children[ 0 ];
		else node = node->children[ 1 ];
	}
	
	return NULL;	// never reached
}


/*
===================
Mod_DecompressVis
===================
*/
byte * Renderer::Mod_DecompressVis( byte * in, model_t * model ) {

	static byte		decompressed[ MAX_MAP_LEAFS / 8 ];
	int				c;
	byte			* out;
	int				row;

	row =( model->vis->numclusters+7 )>>3;	
	out = decompressed;

	if( !in ) {	// no vis info, so make all visible

		while( row ) {

			* out++ = 0xFF;
			row--;
		}
		return decompressed;		
	}

	do {

		if( *in ) {

			* out++ = * in++;
			continue;
		}
	
		c = in[ 1 ];
		in += 2;
		while( c ) {

			* out++ = 0;
			c--;
		}

	} while( out - decompressed < row );
	
	return decompressed;
}

/*
==============
Mod_ClusterPVS
==============
*/
byte * Renderer::Mod_ClusterPVS( int cluster, model_t * model ) {

	if( cluster == -1 || !model->vis ) return mod_novis;

	return Mod_DecompressVis( ( byte * )model->vis + model->vis->bitofs[ cluster ] [DVIS_PVS ], model );
}


//===============================================================================

/*
================
Mod_Modellist_f
================
*/
void Renderer::Mod_Modellist_f( ) {

	int		i;
	model_t	* mod;
	int		total;

	total = 0;
	Common::Com_Printf( "Loaded models:\n" );
	for( i = 0, mod = mod_known; i < mod_numknown; i++, mod++ )
	{
		if( !mod->name[ 0 ] )
			continue;
		Common::Com_Printf( "%8i : %s\n", mod->extradatasize, mod->name.c_str( ) );
		total += mod->extradatasize;
	}
	Common::Com_Printf( "Total resident: %i\n", total );
}



/*
===============
Mod_Init
===============
*/
void Renderer::Mod_Init( ) {

	memset( mod_novis, 0xFF, sizeof( mod_novis ) );
}

/*
==================
Mod_ForName

Loads in a model for the given name
==================
*/
model_t * Renderer::Mod_ForName( const Str & name, bool crash ) {

	model_t	* mod;
	int		i;
	
	if( !name[ 0 ] ) Common::Com_Error( ERR_DROP, "Mod_ForName: NULL name" );
		
	//
	// INLINE models are grabbed only from worldmodel
	//
	if( name[ 0 ] == '*' ) {

		Token token;
		token = name.Right( name.Length( ) - 1 );

		token.type = TT_NUMBER;
		token.subtype = TT_DECIMAL;
		i = token.GetIntValue( );
		if( i < 1 || !r_worldmodel || i >= r_worldmodel->numsubmodels )
			Common::Com_Error( ERR_DROP, "bad INLINE model number" );
		return &mod_inline[ i ];
	}

	//
	// search the currently loaded models
	//
	for( i = 0 , mod = mod_known; i < mod_numknown; i++, mod++ )
	{
		if( !mod->name )
			continue;
		if( mod->name == name )
			return mod;
	}
	
	//
	// find a free model slot spot
	//
	for( i = 0 , mod = mod_known; i<mod_numknown; i++, mod++ )
	{
		if( !mod->name) break;	// free spot
	}
	if( i == mod_numknown )
	{
		if( mod_numknown == MAX_MOD_KNOWN )
			Common::Com_Error( ERR_DROP, "mod_numknown == MAX_MOD_KNOWN" );
		mod_numknown++;
	}
	mod->name = name;
	
	//
	// load the file
	//
	File_Memory fm = FileSystem::ReadFile( mod->name );
	if( !crash && !fm.IsValid( ) ) return NULL;
	modfilelen = fm.Length( );
	
	loadmodel = mod;

	//
	// fill it in
	//


	// call the apropriate loader
	
	switch( LittleLong( *( unsigned * )fm.GetDataPtr( ) ) ) {

	case IDALIASHEADER:
		loadmodel->extradata = Hunk_Begin( 0x200000 );
		Mod_LoadAliasModel( mod, fm.GetDataPtr( ) );
		break;
		
	case IDSPRITEHEADER:
		loadmodel->extradata = Hunk_Begin( 0x10000 );
		Mod_LoadSpriteModel( mod, fm.GetDataPtr( ) );
		break;
	
	/*case IDBSPHEADER:
		loadmodel->extradata = Hunk_Begin( 0x1000000 );
		Mod_LoadBrushModel( mod, fm.GetDataPtr( ) );
		break;*/

	default:
		Common::Com_Error( ERR_DROP, "Mod_NumForName: unknown fileid for %s", mod->name.c_str( ) );
		break;
	}

	loadmodel->extradatasize = Hunk_End( );

	FileSystem::FreeFile( fm );

	return mod;
}

#if 0
/*
===============================================================================

					BRUSHMODEL LOADING

===============================================================================
*/

/*
=================
Mod_LoadLighting
=================
*/
void Renderer::Mod_LoadLighting( lump_t * l ) {

	if( !l->filelen ) {

		loadmodel->lightdata = NULL;
		return;
	}
	loadmodel->lightdata =( byte * )Hunk_Alloc( l->filelen );	
	memcpy( loadmodel->lightdata, mod_base + l->fileofs, l->filelen );
}


/*
=================
Mod_LoadVisibility
=================
*/
void Renderer::Mod_LoadVisibility( lump_t * l ) {

	if( !l->filelen )
	{
		loadmodel->vis = NULL;
		return;
	}
	loadmodel->vis =( dvis_t * )Hunk_Alloc( l->filelen );	
	memcpy( loadmodel->vis, mod_base + l->fileofs, l->filelen );

	loadmodel->vis->numclusters = LittleLong( loadmodel->vis->numclusters );
	for( int i = 0; i < loadmodel->vis->numclusters; i++ ) {

		loadmodel->vis->bitofs[ i ][ 0 ] = LittleLong( loadmodel->vis->bitofs[ i ][ 0 ] );
		loadmodel->vis->bitofs[ i ][ 1 ] = LittleLong( loadmodel->vis->bitofs[ i ][ 1 ] );
	}
}


/*
=================
Mod_LoadVertexes
=================
*/
void Renderer::Mod_LoadVertexes( lump_t * l ) {

	dvertex_t	* in;
	mvertex_t	* out;
	int			i, count;

	in =( dvertex_t * )( mod_base + l->fileofs );
	if( l->filelen % sizeof( dvertex_t ) ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size in %s", loadmodel->name.c_str( ) );
	count = l->filelen / sizeof( dvertex_t );
	out =( mvertex_t * )Hunk_Alloc( count * sizeof( mvertex_t ) );	

	loadmodel->vertexes = out;
	loadmodel->numvertexes = count;

	for( i = 0; i<count; i++, in++, out++ )
	{
		out->position[ 0 ] = LittleFloat( in->point[ 0 ] );
		out->position[ 1 ] = LittleFloat( in->point[ 1 ] );
		out->position[ 2 ] = LittleFloat( in->point[ 2 ] );
	}
}

/*
=================
RadiusFromBounds
=================
*/
float Renderer::RadiusFromBounds( Vec3 & mins, Vec3 & maxs ) {

	Vec3	corner;

	for( int i = 0; i < 3; i++ ) corner[ i ] = fabs( mins[ i ] ) > fabs( maxs[ i ] ) ? fabs( mins[ i ] ) : fabs( maxs[ i ] );

	return corner.Length( );
}


/*
=================
Mod_LoadSubmodels
=================
*/
void Renderer::Mod_LoadSubmodels( lump_t * l ) {

	dmodel_t	* in;
	mmodel_t	* out;
	int			i, j, count;

	in =( dmodel_t * )( mod_base + l->fileofs );
	if( l->filelen % sizeof( dmodel_t ) ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size in %s", loadmodel->name.c_str( ) );
	count = l->filelen / sizeof( dmodel_t );
	out =( mmodel_t * )Hunk_Alloc( count * sizeof( mmodel_t ) );	

	loadmodel->submodels = out;
	loadmodel->numsubmodels = count;

	for( i = 0; i<count; i++, in++, out++ )
	{
		for( j = 0; j<3; j++ )
		{	// spread the mins / maxs by a pixel
			out->mins[ j ] = LittleFloat( in->mins[ j ] ) - 1;
			out->maxs[ j ] = LittleFloat( in->maxs[ j ] ) + 1;
			out->origin[ j ] = LittleFloat( in->origin[ j ] );
		}
		out->radius = RadiusFromBounds( out->mins, out->maxs );
		out->headnode = LittleLong( in->headnode );
		out->firstface = LittleLong( in->firstface );
		out->numfaces = LittleLong( in->numfaces );
	}
}

/*
=================
Mod_LoadEdges
=================
*/
void Renderer::Mod_LoadEdges( lump_t * l ) {

	dedge_t * in;
	medge_t * out;
	int 	i, count;

	in =( dedge_t * )( mod_base + l->fileofs );
	if( l->filelen % sizeof( dedge_t ) ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size in %s", loadmodel->name.c_str( ) );
	count = l->filelen / sizeof( dedge_t );
	out =( medge_t * )Hunk_Alloc( ( count + 1 ) * sizeof( medge_t ) );	

	loadmodel->edges = out;
	loadmodel->numedges = count;

	for( i = 0; i<count; i++, in++, out++ )
	{
		out->v[ 0 ] =( unsigned short )LittleShort( in->v[ 0 ] );
		out->v[ 1 ] =( unsigned short )LittleShort( in->v[ 1 ] );
	}
}

/*
=================
Mod_LoadTexinfo
=================
*/
void Renderer::Mod_LoadTexinfo( lump_t * l ) {

	texinfo_t * in;
	mtexinfo_t * out, * step;
	int 	i, j, count;
	Str	name;
	int		next;

	in =( texinfo_t * )( mod_base + l->fileofs );
	if( l->filelen % sizeof( texinfo_t ) ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size in %s", loadmodel->name.c_str( ) );
	count = l->filelen / sizeof( texinfo_t );
	out =( mtexinfo_t * )Hunk_Alloc( count * sizeof( mtexinfo_t ) );	

	loadmodel->texinfo = out;
	loadmodel->numtexinfo = count;

	for( i = 0; i<count; i++, in++, out++ ) {

		for( j = 0; j<8; j++ ) out->vecs[ 0 ][ j ] = LittleFloat( in->vecs[ 0 ][ j ] );

		out->flags = LittleLong( in->flags );
		next = LittleLong( in->nexttexinfo );

		if( next > 0 ) out->next = loadmodel->texinfo + next;
		else out->next = NULL;

		sprintf( name, "textures/%s.wal", in->texture );

		out->image = GL_FindImage( name, it_wall );

		if( !out->image ) {

			Common::Com_Printf( "Couldn't load %s\n", name.c_str( ) );
			out->image = r_notexture;
		}
	}

	// count animation frames
	for( i = 0; i < count; i++ ) {

		out = &loadmodel->texinfo[ i ];
		out->numframes = 1;
		for( step = out->next; step && step != out; step = step->next ) out->numframes++;
	}
}

/*
================
CalcSurfaceExtents

Fills in s->texturemins[] and s->extents[]
================
*/
void Renderer::CalcSurfaceExtents( msurface_t * s ) {

	float	mins[ 2 ], maxs[ 2 ], val;
	int		i, j, e;
	mvertex_t	* v;
	mtexinfo_t	* tex;
	int		bmins[ 2 ], bmaxs[ 2 ];

	mins[ 0 ] = mins[ 1 ] = 999999;
	maxs[ 0 ] = maxs[ 1 ] = -99999;

	tex = s->texinfo;
	
	for( i = 0; i < s->numedges; i++ ) {

		e = loadmodel->surfedges[ s->firstedge+i ];
		if( e >= 0 )
			v = &loadmodel->vertexes[ loadmodel->edges[e ].v[ 0 ]];
		else
			v = &loadmodel->vertexes[ loadmodel->edges[-e ].v[ 1 ]];
		
		for( j = 0; j<2; j++ )
		{
			val = v->position[ 0 ] * tex->vecs[ j ][ 0 ] + 
				v->position[ 1 ] * tex->vecs[ j ][ 1 ] +
				v->position[ 2 ] * tex->vecs[ j ][ 2 ] +
				tex->vecs[ j ][ 3 ];
			if( val < mins[ j ] )
				mins[ j ] = val;
			if( val > maxs[ j ] )
				maxs[ j ] = val;
		}
	}

	for( i = 0; i<2; i++ )
	{	
		bmins[ i ] = ( int )floor( mins[ i ] / 16.0f );
		bmaxs[ i ] = ( int )ceil( maxs[ i ] / 16.0f );

		s->texturemins[ i ] = bmins[ i ] * 16;
		s->extents[ i ] =( bmaxs[ i ] - bmins[ i ] ) * 16;

//		if( !( tex->flags & TEX_SPECIAL ) && s->extents[ i ] > 512 /* 256 */ )
//			Common::Com_Error( ERR_DROP, "Bad surface extents" );
	}
}

/*
=================
Mod_LoadFaces
=================
*/
void Renderer::Mod_LoadFaces( lump_t * l ) {

	dface_t			* in;
	msurface_t		* out;
	int				count, surfnum;
	int				planenum, side;
	int				ti;

	in =( dface_t * )( mod_base + l->fileofs );
	if( l->filelen % sizeof( dface_t ) ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size in %s", loadmodel->name.c_str( ) );
	count = l->filelen / sizeof( dface_t );
	out =( msurface_t * )Hunk_Alloc( count * sizeof( msurface_t ) );	

	loadmodel->surfaces = out;
	loadmodel->numsurfaces = count;

	currentmodel = loadmodel;

	GL_BeginBuildingLightmaps( loadmodel );

	for( surfnum = 0; surfnum < count; surfnum++, in++, out++ ) {

		out->firstedge = LittleLong( in->firstedge );
		out->numedges = LittleShort( in->numedges );		
		out->flags = 0;
		out->polys = NULL;

		planenum = LittleShort( in->planenum );
		side = LittleShort( in->side );
		if( side ) out->flags |= SURF_PLANEBACK;			

		out->plane = loadmodel->planes + planenum;

		ti = LittleShort( in->texinfo );
		if( ti < 0 || ti >= loadmodel->numtexinfo ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: bad texinfo number" );
		out->texinfo = loadmodel->texinfo + ti;

		CalcSurfaceExtents( out );
				
		int i;
		// lighting info
		for( i = 0; i < MAXLIGHTMAPS; i++ ) out->styles[ i ] = in->styles[ i ];
		i = LittleLong( in->lightofs );
		if( i == -1 ) out->samples = NULL;
		else out->samples = loadmodel->lightdata + i;
		
		// set the drawing flags		
		if( out->texinfo->flags & SURF_WARP ) {
			out->flags |= SURF_DRAWTURB;
			for( i = 0; i < 2; i++ ) {

				out->extents[ i ] = 16384;
				out->texturemins[ i ] = -8192;
			}
			GL_SubdivideSurface( out );	// cut up polygon for warps
		}

		// create lightmaps and polygons
		if( !( out->texinfo->flags &( SURF_SKY | SURF_TRANS33 | SURF_TRANS66 | SURF_WARP ) ) ) GL_CreateSurfaceLightmap( out );
		if( !( out->texinfo->flags & SURF_WARP ) ) GL_BuildPolygonFromSurface( out );

	}

	GL_EndBuildingLightmaps( );
}


/*
=================
Mod_SetParent
=================
*/
void Renderer::Mod_SetParent( mnode_t * node, mnode_t * parent ) {

	node->parent = parent;
	if( node->contents != -1 ) return;
	Mod_SetParent( node->children[ 0 ], node );
	Mod_SetParent( node->children[ 1 ], node );
}

/*
=================
Mod_LoadNodes
=================
*/
void Renderer::Mod_LoadNodes( lump_t * l ) {

	int			i, j, count, p;
	dnode_t		* in;
	mnode_t 	* out;

	in =( dnode_t * )( mod_base + l->fileofs );
	if( l->filelen % sizeof( dnode_t ) ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size in %s", loadmodel->name.c_str( ) );
	count = l->filelen / sizeof( dnode_t );
	out =( mnode_t * )Hunk_Alloc( count * sizeof( mnode_t ) );	

	loadmodel->nodes = out;
	loadmodel->numnodes = count;

	for( i = 0; i<count; i++, in++, out++ )
	{
		for( j = 0; j<3; j++ )
		{
			out->mins[ j ] = LittleShort( in->mins[ j ] );
			out->maxs[ j ] = LittleShort( in->maxs[ j ] );
		}
	
		p = LittleLong( in->planenum );
		out->plane = loadmodel->planes + p;

		out->firstsurface = LittleShort( in->firstface );
		out->numsurfaces = LittleShort( in->numfaces );
		out->contents = -1;	// differentiate from leafs

		for( j = 0; j<2; j++ )
		{
			p = LittleLong( in->children[ j ] );
			if( p >= 0 )
				out->children[ j ] = loadmodel->nodes + p;
			else
				out->children[ j ] =( mnode_t * )( loadmodel->leafs +( -1 - p ) );
		}
	}
	
	Mod_SetParent( loadmodel->nodes, NULL );	// sets nodes and leafs
}

/*
=================
Mod_LoadLeafs
=================
*/
void Renderer::Mod_LoadLeafs( lump_t * l ) {

	dleaf_t 	* in;
	mleaf_t 	* out;
	int			i, j, count, p;
//	glpoly_t	* poly;

	in =( dleaf_t * )( mod_base + l->fileofs );
	if( l->filelen % sizeof( dleaf_t ) ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size in %s", loadmodel->name.c_str( ) );
	count = l->filelen / sizeof( dleaf_t );
	out =( mleaf_t * )Hunk_Alloc( count * sizeof( mleaf_t ) );

	loadmodel->leafs = out;
	loadmodel->numleafs = count;

	for( i = 0; i<count; i++, in++, out++ )
	{
		for( j = 0; j<3; j++ )
		{
			out->mins[ j ] = LittleShort( in->mins[ j ] );
			out->maxs[ j ] = LittleShort( in->maxs[ j ] );
		}

		p = LittleLong( in->contents );
		out->contents = p;

		out->cluster = LittleShort( in->cluster );
		out->area = LittleShort( in->area );

		out->firstmarksurface = loadmodel->marksurfaces +
			LittleShort( in->firstleafface );
		out->nummarksurfaces = LittleShort( in->numleaffaces );
	}	
}

/*
=================
Mod_LoadMarksurfaces
=================
*/
void Renderer::Mod_LoadMarksurfaces( lump_t * l ) {

	int		i, j, count;
	short		* in;
	msurface_t * *out;
	
	in =( short * )( mod_base + l->fileofs );
	if( l->filelen % sizeof( short ) ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size in %s", loadmodel->name.c_str( ) );
	count = l->filelen / sizeof( short );
	out =( msurface_t * * )Hunk_Alloc( count * sizeof( msurface_t * ) );

	loadmodel->marksurfaces = out;
	loadmodel->nummarksurfaces = count;

	for( i = 0; i<count; i++ )
	{
		j = LittleShort( in[ i ] );
		if( j < 0 ||  j >= loadmodel->numsurfaces )
			Common::Com_Error( ERR_DROP, "Mod_ParseMarksurfaces: bad surface number" );
		out[ i ] = loadmodel->surfaces + j;
	}
}

/*
=================
Mod_LoadSurfedges
=================
*/
void Renderer::Mod_LoadSurfedges( lump_t * l ) {

	int		i, count;
	int		* in, * out;
	
	in =( int * )( mod_base + l->fileofs );
	if( l->filelen % sizeof( int ) ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size in %s", loadmodel->name.c_str( ) );
	count = l->filelen / sizeof( int );
	if( count < 1 || count >= MAX_MAP_SURFEDGES ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: bad surfedges count in %s: %i", loadmodel->name.c_str( ), count );
	out =( int * )Hunk_Alloc( count * sizeof( int ) );

	loadmodel->surfedges = out;
	loadmodel->numsurfedges = count;

	for( i = 0; i<count; i++ )
		out[ i ] = LittleLong( in[ i ] );
}


/*
=================
Mod_LoadPlanes
=================
*/
void Renderer::Mod_LoadPlanes( lump_t * l ) {

	int			i, j;
	Plane	* out;
	dplane_t 	* in;
	int			count;
	int			bits;
	
	in =( dplane_t * )( mod_base + l->fileofs );
	if( l->filelen % sizeof( dplane_t ) ) Common::Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size in %s", loadmodel->name.c_str( ) );
	count = l->filelen / sizeof( dplane_t );
	out =( Plane * )Hunk_Alloc( count * 2 * sizeof( Plane ) );	
	
	loadmodel->planes = out;
	loadmodel->numplanes = count;

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
Mod_LoadBrushModel
=================
*/
void Renderer::Mod_LoadBrushModel( model_t * mod, const void * buffer ) {

	int			i;
	dheader_t	* header;
	mmodel_t 	* bm;
	
	loadmodel->type = mod_brush;
	if( loadmodel != mod_known )
		Common::Com_Error( ERR_DROP, "Loaded a brush model after the world" );

	header =( dheader_t * )buffer;

	i = LittleLong( header->version );
	if( i != BSPVERSION )
		Common::Com_Error( ERR_DROP, "Mod_LoadBrushModel: %s has wrong version number( %i should be %i )", mod->name, i, BSPVERSION );

// swap all the lumps
	mod_base =( byte * )header;

	for( i = 0; i<sizeof( dheader_t )/4; i++ )
		( ( int * )header )[ i ] = LittleLong( ( ( int * )header )[ i ] );

// load into heap
	
	Mod_LoadVertexes( &header->lumps[ LUMP_VERTEXES ] );
	Mod_LoadEdges( &header->lumps[ LUMP_EDGES ] );
	Mod_LoadSurfedges( &header->lumps[ LUMP_SURFEDGES ] );
	Mod_LoadLighting( &header->lumps[ LUMP_LIGHTING ] );
	Mod_LoadPlanes( &header->lumps[ LUMP_PLANES ] );
	Mod_LoadTexinfo( &header->lumps[ LUMP_TEXINFO ] );
	Mod_LoadFaces( &header->lumps[ LUMP_FACES ] );
	Mod_LoadMarksurfaces( &header->lumps[ LUMP_LEAFFACES ] );
	Mod_LoadVisibility( &header->lumps[ LUMP_VISIBILITY ] );
	Mod_LoadLeafs( &header->lumps[ LUMP_LEAFS ] );
	Mod_LoadNodes( &header->lumps[ LUMP_NODES ] );
	Mod_LoadSubmodels( &header->lumps[ LUMP_MODELS ] );
	mod->numframes = 2;		// regular and alternate animation
	
//
// set up the submodels
//
	for( i = 0; i<mod->numsubmodels; i++ )
	{
		model_t	* starmod;

		bm = &mod->submodels[ i ];
		starmod = &mod_inline[ i ];

		* starmod = * loadmodel;
		
		starmod->firstmodelsurface = bm->firstface;
		starmod->nummodelsurfaces = bm->numfaces;
		starmod->firstnode = bm->headnode;
		if( starmod->firstnode >= loadmodel->numnodes )
			Common::Com_Error( ERR_DROP, "INLINE model %i has bad firstnode", i );

		starmod->maxs = bm->maxs;
		starmod->mins = bm->mins;
		starmod->radius = bm->radius;
	
		if( i == 0 )
			* loadmodel = * starmod;

		starmod->numleafs = bm->visleafs;
	}
}
#endif

/*
==============================================================================

ALIAS MODELS

==============================================================================
*/

/*
=================
Mod_LoadAliasModel
=================
*/
void Renderer::Mod_LoadAliasModel( model_t * mod, const void * buffer ) {

	dmdl_t				* pinmodel, * pheader;
	dstvert_t			* pinst, * poutst;
	dtriangle_t			* pintri, * pouttri;
	daliasframe_t		* pinframe, * poutframe;
	int					* pincmd, * poutcmd;
	int					version;

	pinmodel =( dmdl_t * )buffer;

	version = LittleLong( pinmodel->version );
	if( version != ALIAS_VERSION ) Common::Com_Error( ERR_DROP, "%s has wrong version number( %i should be %i )", mod->name, version, ALIAS_VERSION );

	pheader =( dmdl_t * )Hunk_Alloc( LittleLong( pinmodel->ofs_end ) );
	
	// byte swap the header fields and sanity check
	for( int i = 0; i < sizeof( dmdl_t ) / 4; i++ )( ( int * )pheader )[ i ] = LittleLong( ( ( int * )buffer )[ i ] );

	if( pheader->skinheight > MAX_LBM_HEIGHT ) Common::Com_Error( ERR_DROP, "model %s has a skin taller than %d", mod->name.c_str( ), MAX_LBM_HEIGHT );

	if( pheader->num_xyz <= 0 ) Common::Com_Error( ERR_DROP, "model %s has no vertices", mod->name.c_str( ) );

	if( pheader->num_xyz > MAX_VERTS ) Common::Com_Error( ERR_DROP, "model %s has too many vertices", mod->name.c_str( ) );

	if( pheader->num_st <= 0 ) Common::Com_Error( ERR_DROP, "model %s has no st vertices", mod->name.c_str( ) );

	if( pheader->num_tris <= 0 ) Common::Com_Error( ERR_DROP, "model %s has no triangles", mod->name.c_str( ) );

	if( pheader->num_frames <= 0 ) Common::Com_Error( ERR_DROP, "model %s has no frames", mod->name.c_str( ) );

//
// load base s and t vertices( not used in gl version )
//
	pinst =( dstvert_t * )( ( byte * )pinmodel + pheader->ofs_st );
	poutst =( dstvert_t * )( ( byte * )pheader + pheader->ofs_st );

	for( int i = 0; i < pheader->num_st; i++ ) {

		poutst[ i ].s = LittleShort( pinst[ i ].s );
		poutst[ i ].t = LittleShort( pinst[ i ].t );
	}

//
// load triangle lists
//
	pintri =( dtriangle_t * )( ( byte * )pinmodel + pheader->ofs_tris );
	pouttri =( dtriangle_t * )( ( byte * )pheader + pheader->ofs_tris );

	for( int i = 0; i<pheader->num_tris; i++ ) {

		for( int j = 0; j<3; j++ ) {

			pouttri[ i ].index_xyz[ j ]	= LittleShort( pintri[ i ].index_xyz[ j ] );
			pouttri[ i ].index_st[ j ]	= LittleShort( pintri[ i ].index_st[ j ] );
		}
	}

//
// load the frames
//
	for( int i = 0; i < pheader->num_frames; i++ ) {

		pinframe =( daliasframe_t * )( ( byte * )pinmodel + pheader->ofs_frames + i * pheader->framesize );
		poutframe =( daliasframe_t * )( ( byte * )pheader + pheader->ofs_frames + i * pheader->framesize );

		memcpy( poutframe->name, pinframe->name, sizeof( poutframe->name ) );
		for( int j = 0; j < 3; j++ ) {

			poutframe->scale[ j ]		= LittleFloat( pinframe->scale[ j ] );
			poutframe->translate[ j ]	= LittleFloat( pinframe->translate[ j ] );
		}
		// verts are all 8 bit, so no swapping needed
		memcpy( poutframe->verts, pinframe->verts, pheader->num_xyz * sizeof( dtrivertx_t ) );

	}

	mod->type = mod_alias;

	//
	// load the glcmds
	//
	pincmd =( int * )( ( byte * )pinmodel + pheader->ofs_glcmds );
	poutcmd =( int * )( ( byte * )pheader + pheader->ofs_glcmds );
	for( int i = 0; i < pheader->num_glcmds;i++ ) poutcmd[ i ] = LittleLong( pincmd[ i ] );


	// register all skins
	memcpy( ( char * )pheader + pheader->ofs_skins, ( char * )pinmodel + pheader->ofs_skins, pheader->num_skins * MAX_SKINNAME );
	//for( int i = 0; i < pheader->num_skins; i++ ) mod->skins[ i ] = GL_FindImage( Str( ( char * )pheader + pheader->ofs_skins + i * MAX_SKINNAME ), it_skin );

	mod->mins[ 0 ] = -32.0f;
	mod->mins[ 1 ] = -32.0f;
	mod->mins[ 2 ] = -32.0f;
	mod->maxs[ 0 ] = 32.0f;
	mod->maxs[ 1 ] = 32.0f;
	mod->maxs[ 2 ] = 32.0f;
}

/*
==============================================================================

SPRITE MODELS

==============================================================================
*/

/*
=================
Mod_LoadSpriteModel
=================
*/
void Renderer::Mod_LoadSpriteModel( model_t * mod, const void * buffer ) {

	dsprite_t	* sprin, * sprout;
	int			i;

	sprin =( dsprite_t * )buffer;
	sprout =( dsprite_t * )Hunk_Alloc( modfilelen );

	sprout->ent = LittleLong( sprin->ent );
	sprout->version = LittleLong( sprin->version );
	sprout->numframes = LittleLong( sprin->numframes );

	if( sprout->version != SPRITE_VERSION )
		Common::Com_Error( ERR_DROP, "%s has wrong version number( %i should be %i )", mod->name, sprout->version, SPRITE_VERSION );

	if( sprout->numframes > MAX_MD2SKINS )
		Common::Com_Error( ERR_DROP, "%s has too many frames( %i > %i )", mod->name, sprout->numframes, MAX_MD2SKINS );

	// byte swap everything
	for( i = 0; i<sprout->numframes; i++ ) {

		sprout->frames[ i ].width = LittleLong( sprin->frames[ i ].width );
		sprout->frames[ i ].height = LittleLong( sprin->frames[ i ].height );
		sprout->frames[ i ].origin_x = LittleLong( sprin->frames[ i ].origin_x );
		sprout->frames[ i ].origin_y = LittleLong( sprin->frames[ i ].origin_y );
		memcpy( sprout->frames[ i ].name, sprin->frames[ i ].name, MAX_SKINNAME );
		//mod->skins[ i ] = GL_FindImage( Str( sprout->frames[ i ].name ), it_sprite );
	}

	mod->type = mod_sprite;
}

//=============================================================================

/*
@@@@@@@@@@@@@@@@@@@@@
R_BeginRegistration

Specifies the model that will be used as the world
@@@@@@@@@@@@@@@@@@@@@
*/
void Renderer::BeginRegistration( const Str & model ) {

	Str	fullname;

	registration_sequence++;
	r_oldviewcluster = -1;		// force markleafs

	sprintf( fullname, "maps/%s.map", model.c_str( ) );

	// explicitly free the old map if different
	// this guarantees that mod_known[ 0 ] is the world map

	//if( mod_known[ 0 ].name != fullname || flushmap.GetBool( ) ) Mod_Free( &mod_known[ 0 ] );

	//r_worldmodel = Mod_ForName( fullname, true );

	r_viewcluster = -1;

	r_notexture = MaterialSystem::FindTexture( "textures/common/shadernotex.tga", TextureBase::mapTexture );
	if( !r_notexture ) Common::Com_Error( ERR_DROP, "Error texture not found" );

	for( int i = 1; i < CollisionModel::CM_NumInlineModels( ); i++ ) {

		CModel * cModel = CollisionModel::CM_GetInlineModel( i );

		for( int j = 0; j < cModel->materials.Num( ); j++ ) {

			Str fileName = *cModel->materials[ j ];
			fileName.Append( ".tga" );
			TextureBase * texture = MaterialSystem::FindTexture( fileName, TextureBase::mapTexture );
			cModel->materialIndexes.Append( texture ? texture : r_notexture );
		}

		cModel->materialIndexes.Condense( );
	}
}


/*
@@@@@@@@@@@@@@@@@@@@@
R_RegisterModel

@@@@@@@@@@@@@@@@@@@@@
*/
struct model_s * Renderer::RegisterModel( const Str & name ) {

	model_t	* mod;
	int		i;
	dsprite_t	* sprout;
	dmdl_t		* pheader;

	mod = Mod_ForName( name, false );
	if( mod ) {

		mod->registration_sequence = registration_sequence;

		// register any images used by the models
		if( mod->type == mod_sprite ) {

			sprout =( dsprite_t * )mod->extradata;
			//for( i = 0; i<sprout->numframes; i++ ) mod->skins[ i ] = GL_FindImage( Str( sprout->frames[ i ].name ), it_sprite );

		} else if( mod->type == mod_alias ) {

			pheader =( dmdl_t * )mod->extradata;
			//for( i = 0; i<pheader->num_skins; i++ ) mod->skins[ i ] = GL_FindImage( Str( ( char * )pheader + pheader->ofs_skins + i * MAX_SKINNAME ), it_skin );
//PGM
			mod->numframes = pheader->num_frames;
//PGM
		} else if( mod->type == mod_brush ) {

			//for( i = 0; i<mod->numtexinfo; i++ )
				//mod->texinfo[ i ].image->registration_sequence = registration_sequence;
		}
	}

	return mod;
}


/*
@@@@@@@@@@@@@@@@@@@@@
R_EndRegistration

@@@@@@@@@@@@@@@@@@@@@
*/
void Renderer::EndRegistration( ) {

	int		i;
	model_t	* mod;

	for( i = 0, mod = mod_known; i<mod_numknown; i++, mod++ )
	{
		if( !mod->name ) continue;
		if( mod->registration_sequence != registration_sequence ) {	// don't need this model

			Mod_Free( mod );
		}
	}

	//GL_FreeUnusedImages( );
}


//=============================================================================


/*
================
Mod_Free
================
*/
void Renderer::Mod_Free( model_t * mod ) {

	Hunk_Free( mod->extradata );
	memset( mod, 0, sizeof( model_t ) );
}

/*
================
Mod_FreeAll
================
*/
void Renderer::Mod_FreeAll( ) {

	for( int i = 0; i < mod_numknown; i++ ) {

		if( mod_known[ i ].extradatasize ) Mod_Free( &mod_known[ i ] );
	}
}
