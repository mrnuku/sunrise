#define	CONTENTS_SOLID			1		// an eye is never valid in a solid
#define	CONTENTS_LAVA			8
#define	CONTENTS_SLIME			16
#define	CONTENTS_WATER			32
#define	CONTENTS_FOG			64

#define CONTENTS_NOTTEAM1		0x0080
#define CONTENTS_NOTTEAM2		0x0100
#define CONTENTS_NOBOTCLIP		0x0200

#define	CONTENTS_AREAPORTAL		0x8000

#define	CONTENTS_PLAYERCLIP		0x10000
#define	CONTENTS_MONSTERCLIP	0x20000
//bot specific contents types
#define	CONTENTS_TELEPORTER		0x40000
#define	CONTENTS_JUMPPAD		0x80000
#define CONTENTS_CLUSTERPORTAL	0x100000
#define CONTENTS_DONOTENTER		0x200000
#define CONTENTS_BOTCLIP		0x400000
#define CONTENTS_MOVER			0x800000

#define	CONTENTS_ORIGIN			0x1000000	// removed before bsping an entity

#define	CONTENTS_BODY			0x2000000	// should never be on a brush, only in game
#define	CONTENTS_CORPSE			0x4000000
#define	CONTENTS_DETAIL			0x8000000	// brushes not used for the bsp
#define	CONTENTS_STRUCTURAL		0x10000000	// brushes used for the bsp
#define	CONTENTS_TRANSLUCENT	0x20000000	// don't consume surface fragments inside
#define	CONTENTS_TRIGGER		0x40000000
#define	CONTENTS_NODROP			0x80000000	// don't leave bodies or items (death fog, lava)

#define	SURF_NODAMAGE			0x1		// never give falling damage
#define	SURF_SLICK				0x2		// effects game physics
#define	SURF_SKY				0x4		// lighting from environment map
#define	SURF_LADDER				0x8
#define	SURF_NOIMPACT			0x10	// don't make missile explosions
#define	SURF_NOMARKS			0x20	// don't leave missile marks
#define	SURF_FLESH				0x40	// make flesh sounds and effects
#define	SURF_NODRAW				0x80	// don't generate a drawsurface at all
#define	SURF_HINT				0x100	// make a primary bsp splitter
#define	SURF_SKIP				0x200	// completely ignore, allowing non-closed brushes
#define	SURF_NOLIGHTMAP			0x400	// surface doesn't need a lightmap
#define	SURF_POINTLIGHT			0x800	// generate lighting info at vertexes
#define	SURF_METALSTEPS			0x1000	// clanking footsteps
#define	SURF_NOSTEPS			0x2000	// no footstep sounds
#define	SURF_NONSOLID			0x4000	// don't collide against curves with this set
#define	SURF_LIGHTFILTER		0x8000	// act as a light filter during q3map -light
#define	SURF_ALPHASHADOW		0x10000	// do per-pixel light shadow casting in q3map
#define	SURF_NODLIGHT			0x20000	// don't dlight even if solid (solid lava, skies)
#define SURF_DUST				0x40000 // leave a dust trail when walking on this surface

// content masks
#define	MASK_ALL				(-1)
#define	MASK_SOLID				(CONTENTS_SOLID)
#define	MASK_PLAYERSOLID		(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BODY)
#define	MASK_DEADSOLID			(CONTENTS_SOLID|CONTENTS_PLAYERCLIP)
#define	MASK_WATER				(CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME)
#define	MASK_OPAQUE				(CONTENTS_SOLID|CONTENTS_SLIME|CONTENTS_LAVA)
#define	MASK_SHOT				(CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_CORPSE)

#define	MAX_POINTS_ON_WINDING	64

#define	SIDE_FRONT	0
#define	SIDE_BACK	1
#define	SIDE_ON		2
#define	SIDE_CROSS	3

#define	NORMAL_EPSILON	0.0001
#define	DIST_EPSILON	0.02
#define	CLIP_EPSILON	0.1f
#define	POINT_EPSILON	0.1f

#define MAX_MAP_BOUNDS			65535

#define	ON_EPSILON	0.1f

#define	MAX_SUBMODELS			256
#define	BOX_MODEL_HANDLE		255
#define CAPSULE_MODEL_HANDLE	254

// keep 1/8 unit away to keep the position valid before network snapping
// and to avoid various numeric issues
#define	SURFACE_CLIP_EPSILON	(0.125)

#define	MAX_FACETS			1024
#define	MAX_PATCH_PLANES	2048

#define	MAX_GRID_SIZE	129

#define	SUBDIVIDE_DISTANCE	16	//4	// never more than this units away from curve
#define	PLANE_TRI_EPSILON	0.1
#define	WRAP_POINT_EPSILON	0.1

// to allow boxes to be treated as brush models, we allocate
// some extra indexes along with those needed by the map
#define	BOX_BRUSHES		1
#define	BOX_SIDES		6
#define	BOX_LEAFS		2
#define	BOX_PLANES		12

typedef int		clipHandle_t;

// markfragments are returned by CM_MarkFragments()
typedef struct markFragment_s {

	int		firstPoint;
	int		numPoints;

} markFragment_t;

typedef struct orientation_s {

	Vec3		origin;
	Vec3		axis[ 3 ];

} orientation_t;

// plane_t structure
class cplane_t {

public:

	Vec3			normal;
	float			dist;
	planeType_e		type;			// for fast side tests: 0,1,2 = axial, 3 = nonaxial
	uint			signbits;		// signx + (signy<<1) + (signz<<2), used as lookup during collision

	void			SetPlaneSignbits( );
};

inline void cplane_t::SetPlaneSignbits( ) {

	// for fast box on planeside test
	int bits = 0;

	for( int j = 0; j < 3 ; j++ ) {

		if( normal[ j ] < 0 ) {

			bits |= 1<<j;
		}
	}

	signbits = bits;
}

// a trace is returned when a box is swept through the world
typedef struct trace_s {

	bool		allsolid;	// if true, plane is not valid
	bool		startsolid;	// if true, the initial point was in a solid area
	float		fraction;	// time completed, 1.0 = didn't hit anything
	Vec3		endpos;		// final position
	cplane_t	plane;		// surface normal at impact, transformed to world space
	int			surfaceFlags;	// surface hit
	int			contents;	// contents on other side of surface hit
	int			entityNum;	// entity the contacted sirface is a part of

} trace_t;

// this is only used for visualization tools in cm_ debug functions

typedef struct winding_s {

	int		numpoints;
	Vec3	p[ 4 ];		// variable sized

} winding_t;

typedef struct cNode_s {

	cplane_t	*plane;
	int			children[ 2 ];		// negative numbers are leafs

} cNode_t;

typedef struct cLeaf_s {

	int			cluster;
	int			area;

	int			firstLeafBrush;
	int			numLeafBrushes;

	int			firstLeafSurface;
	int			numLeafSurfaces;

} cLeaf_t;

typedef struct cmodel_s {

	Vec3		mins, maxs;
	cLeaf_t		leaf;			// submodels don't reference the main tree

} cmodel_t;

typedef struct cbrushside_s {

	cplane_t	*plane;
	int			surfaceFlags;
	int			shaderNum;

} cbrushside_t;

typedef struct cbrush_s {

	int				shaderNum;		// the shader that determined the contents
	int				contents;
	Vec3			bounds[ 2 ];
	int				numsides;
	cbrushside_t *	sides;
	int				checkcount;		// to avoid repeated testings

} cbrush_t;

typedef struct cPatch_s {

	int							checkcount;		// to avoid repeated testings
	int							surfaceFlags;
	int							contents;
	struct patchCollide_s *		pc;

} cPatch_t;

typedef struct cArea_s {

	int			floodnum;
	int			floodvalid;

} cArea_t;

typedef struct clipMap_s {

	Str				name;

	int				numShaders;
	dshader_t *		shaders;

	int				numBrushSides;
	cbrushside_t *	brushsides;

	int				numPlanes;
	cplane_t *		planes;

	int				numNodes;
	cNode_t *		nodes;

	int				numLeafs;
	cLeaf_t *		leafs;

	int				numLeafBrushes;
	int *			leafbrushes;

	int				numLeafSurfaces;
	int *			leafsurfaces;

	int				numSubModels;
	cmodel_t *		cmodels;

	int				numBrushes;
	cbrush_t *		brushes;

	int				numClusters;
	int				clusterBytes;
	byte *			visibility;
	bool			vised;			// if false, visibility is just a single cluster of ffs

	Str				entityString;

	int				numAreas;
	cArea_t *		areas;
	int *			areaPortals;	// [ numAreas*numAreas ] reference counts

	int				numSurfaces;
	cPatch_t **		surfaces;		// non-patches will be NULL

	int				floodvalid;
	int				checkcount;		// incremented on each trace

} clipMap_t;

// Used for oriented capsule collision detection
typedef struct sphere_s {

	bool		use;
	float		radius;
	float		halfheight;
	Vec3		offset;

} sphere_t;

typedef struct traceWork_s {

	Vec3		start;
	Vec3		end;
	Vec3		size[ 2 ];		// size of the box being swept through the model
	Vec3		offsets[ 8 ];	// [signbits][x] = either size[0][x] or size[1][x]
	float		maxOffset;		// longest corner length from origin
	Vec3		extents;		// greatest of abs(size[0]) and abs(size[1])
	Vec3		bounds[ 2 ];	// enclosing box of start and end surrounding by size
	Vec3		modelOrigin;	// origin of the model tracing through
	int			contents;		// ored contents of the model tracing through
	bool		isPoint;		// optimized case
	trace_t		trace;			// returned from trace call
	sphere_t	sphere;			// sphere for oriendted capsule collision

} traceWork_t;

typedef struct leafList_s {

	int		count;
	int		maxcount;
	bool	overflowed;
	int *	list;
	Vec3	bounds[ 2 ];
	int		lastLeaf;		// for overflows where each leaf can't be stored individually

	void	( * storeLeafs )( struct leafList_s * ll, int nodenum );

} leafList_t;

typedef struct patchPlane_s {

	Plane	plane;
	int		signbits;		// signx + (signy<<1) + (signz<<2), used as lookup during collision

} patchPlane_t;

typedef struct facet_s {

	int			surfacePlane;
	int			numBorders;		// 3 or four + 6 axial bevels + 4 or 3 * 4 edge bevels
	int			borderPlanes[ 4 + 6 + 16 ];
	int			borderInward[ 4 + 6 + 16 ];
	bool		borderNoAdjust[ 4 + 6 + 16 ];

} facet_t;

typedef struct patchCollide_s {

	Vec3			bounds[ 2 ];
	int				numPlanes;			// surface planes plus edge planes
	patchPlane_t *	planes;
	int				numFacets;
	facet_t *		facets;

} patchCollide_t;


typedef struct cGrid_s {

	int			width;
	int			height;
	bool		wrapWidth;
	bool		wrapHeight;
	Vec3		points[ MAX_GRID_SIZE ][ MAX_GRID_SIZE ];	// [width][height]

} cGrid_t;


class CollisionModel {

private:

	static clipMap_t				cm;
	static int						c_pointcontents;
	static int						c_traces, c_brush_traces, c_patch_traces;

	static byte *					cmod_base;
	static unsigned					last_checksum;

	static CVar						cm_noAreas;
	static CVar						cm_noCurves;
	static CVar						cm_playerCurveClip;

	static cmodel_t					box_model;
	static cplane_t	*				box_planes;
	static cbrush_t	*				box_brush;

	static int						c_totalPatchBlocks;
	static int						c_totalPatchSurfaces;
	static int						c_totalPatchEdges;

	static const patchCollide_t	*	debugPatchCollide;
	static const facet_t *			debugFacet;
	static bool						debugBlock;
	static Vec3						debugBlockPoints[ 4 ];

	static	int						numPlanes;
	static	patchPlane_t			planes[ MAX_PATCH_PLANES ];

	static	int						numFacets;
	static	facet_t					facets[ MAX_PATCH_PLANES ]; //maybe MAX_FACETS ??

	static winding_t *				AllocWinding( int points );
	static vec_t					WindingArea( winding_t * w );
	static void						WindingCenter( winding_t * w, Vec3 & center );
	static void						ClipWindingEpsilon( winding_t * in, Vec3 & normal, vec_t dist, vec_t epsilon, winding_t ** front, winding_t ** back );
	static winding_t *				ChopWinding( winding_t * in, Vec3 & normal, vec_t dist );
	static winding_t *				CopyWinding( winding_t * w );
	static winding_t *				ReverseWinding( winding_t * w );
	static winding_t *				BaseWindingForPlane( Vec3 & normal, vec_t dist );
	static void						CheckWinding( winding_t * w );
	static void						WindingPlane( winding_t * w, Vec3 & normal, vec_t * dist );
	static void						RemoveColinearPoints( winding_t * w );
	static int						WindingOnPlaneSide( winding_t * w, Vec3 & normal, vec_t dist );
	static void						FreeWinding( winding_t * w );
	static void						WindingBounds( winding_t * w, Vec3 & mins, Vec3 & maxs );

	static void						AddWindingToConvexHull( winding_t * w, winding_t ** hull, Vec3 & normal );

	static void						ChopWindingInPlace( winding_t ** w, Vec3 & normal, vec_t dist, vec_t epsilon );

									// frees the original if clipped
	static void						pw( winding_t * w );

	static int						CM_BoxBrushes( const Vec3 & mins, const Vec3 & maxs, cbrush_t ** list, int listsize );

	static void						CM_StoreLeafs( leafList_t * ll, int nodenum );
	static void						CM_StoreBrushes( leafList_t * ll, int nodenum );

	static void						CM_BoxLeafnums_r( leafList_t * ll, int nodenum );

	static cmodel_t	*				CM_ClipHandleToModel( clipHandle_t handle );

	// cm_patch.c

	static struct patchCollide_s *	CM_GeneratePatchCollide( int width, int height, Vec3 * points );
	static void						CM_TraceThroughPatchCollide( traceWork_t * tw, const struct patchCollide_s * pc );
	static bool						CM_PositionTestInPatchCollide( traceWork_t * tw, const struct patchCollide_s * pc );
	static void						CM_ClearLevelPatches( );

	// cm_load.cpp
	static void						CMod_LoadShaders( lump_t * l );
	static void						CMod_LoadSubmodels( lump_t * l );
	static void						CMod_LoadNodes( lump_t * l );
	static void						CM_BoundBrush( cbrush_t * b );
	static void						CMod_LoadBrushes( lump_t * l );
	static void						CMod_LoadLeafs( lump_t * l );
	static void						CMod_LoadPlanes( lump_t * l );
	static void						CMod_LoadLeafBrushes( lump_t * l );
	static void						CMod_LoadLeafSurfaces( lump_t * l );
	static void						CMod_LoadBrushSides( lump_t * l );
	static void						CMod_LoadEntityString( lump_t * l );
	static void						CMod_LoadVisibility( lump_t * l );
	static void						CMod_LoadPatches( lump_t * surfs, lump_t * verts );
	static unsigned					CM_LumpChecksum( lump_t * lump );
	static unsigned					CM_Checksum( dheader_t * header );
	static void						CM_InitBoxHull( );

	// cm_patch.cpp
	static bool						CM_PlaneFromPoints( Plane & plane, Vec3 & a, Vec3 & b, Vec3 & c );
	static bool						CM_NeedsSubdivision( Vec3 & a, Vec3 & b, Vec3 & c );
	static void						CM_Subdivide( Vec3 & a, Vec3 & b, Vec3 & c, Vec3 & out1, Vec3 & out2, Vec3 & out3 );
	static void						CM_TransposeGrid( cGrid_t * grid );
	static void						CM_SetGridWrapWidth( cGrid_t * grid );
	static void						CM_SubdivideGridColumns( cGrid_t * grid );
	static void						CM_RemoveDegenerateColumns( cGrid_t * grid );
	static int						CM_PlaneEqual( patchPlane_t * p, Plane & plane, int * flipped );
	static void						CM_SnapVector( Vec3 & normal );
	static int						CM_FindPlane2( Plane & plane, int * flipped );
	static int						CM_FindPlane( Vec3 & p1, Vec3 & p2, Vec3 & p3 );
	static int						CM_PointOnPlaneSide( Vec3 & p, int planeNum );
	static int						CM_GrPlane( int grPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2], int i, int j, int tri );
	static int						CM_EdgePlaneNum( cGrid_t *grid, int grPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2], int i, int j, int k );

public:

	static void						CM_LoadMap( const Str & name, bool clientload, int * checksum );
	static void						CM_ClearMap( );
	static clipHandle_t				CM_InlineModel( int index );		// 0 = world, 1 + are bmodels
	static clipHandle_t				CM_TempBoxModel( const Vec3 & mins, const Vec3 & maxs, int capsule );

	static void						CM_ModelBounds( clipHandle_t model, Vec3 & mins, Vec3 & maxs );

	static int						CM_NumClusters( );
	static int						CM_NumInlineModels( );
	static const Str				CM_EntityString( );

									// returns an ORed contents mask
	static int						CM_PointContents( const Vec3 & p, clipHandle_t model );
	static int						CM_TransformedPointContents( const Vec3 & p, clipHandle_t model, const Vec3 & origin, const Vec3 & angles );

	static void						CM_BoxTrace( trace_t * results, const Vec3 & start, const Vec3 & end, vec3_t mins, Vec3 & maxs, clipHandle_t model, int brushmask, int capsule );
	static void						CM_TransformedBoxTrace( trace_t * results, const Vec3 & start, const Vec3 & end, Vec3 & mins, Vec3 & maxs, clipHandle_t model, int brushmask, const Vec3 & origin, const Vec3 & angles, int capsule );

	static byte *					CM_ClusterPVS( int cluster );

	static int						CM_PointLeafnum( const Vec3 & p );

									// only returns non-solid leafs
									// overflow if return listsize and if *lastLeaf != list[listsize-1]
	static int						CM_BoxLeafnums( const Vec3 & mins, const Vec3 & maxs, int * list, int listsize, int * lastLeaf );

	static int						CM_LeafCluster( int leafnum );
	static int						CM_LeafArea( int leafnum );

	static void						CM_AdjustAreaPortalState( int area1, int area2, bool open );
	static bool						CM_AreasConnected( int area1, int area2 );

	static int						CM_WriteAreaBits( byte * buffer, int area );

	// cm_tag.c
	static int						CM_LerpTag( orientation_t * tag,  clipHandle_t model, int startFrame, int endFrame, float frac, const Str & tagName );

	// cm_marks.c
	static int						CM_MarkFragments( int numPoints, const Vec3 * points, const Vec3 & projection, int maxPoints, Vec3 & pointBuffer, int maxFragments, markFragment_t * fragmentBuffer );

	// cm_patch.c
	static void						CM_DrawDebugSurface( void ( * drawPoly )( int color, int numPoints, float * points ) );
};
