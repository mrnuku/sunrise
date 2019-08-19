/*
==============================================================

COLLISION DETECTION

==============================================================
*/

// lower bits are stronger, and will eat weaker brushes completely
#define	CONTENTS_SOLID			1			// an eye is never valid in a solid
#define	CONTENTS_WINDOW			2			// translucent, but not watery
#define	CONTENTS_AUX			4
#define	CONTENTS_LAVA			8
#define	CONTENTS_SLIME			16
#define	CONTENTS_WATER			32
#define	CONTENTS_MIST			64
#define	LAST_VISIBLE_CONTENTS	64
#define	CONTENTS_AREAPORTAL		0x8000		// remaining contents are non-visible, and don't eat brushes
#define	CONTENTS_PLAYERCLIP		0x10000
#define	CONTENTS_MONSTERCLIP	0x20000
#define	CONTENTS_CURRENT_0		0x40000		// currents can be added to any other contents, and may be mixed
#define	CONTENTS_CURRENT_90		0x80000
#define	CONTENTS_CURRENT_180	0x100000
#define	CONTENTS_CURRENT_270	0x200000
#define	CONTENTS_CURRENT_UP		0x400000
#define	CONTENTS_CURRENT_DOWN	0x800000
#define	CONTENTS_ORIGIN			0x1000000	// removed before bsping an entity
#define	CONTENTS_MONSTER		0x2000000	// should never be on a brush, only in game
#define	CONTENTS_DEADMONSTER	0x4000000
#define	CONTENTS_DETAIL			0x8000000	// brushes to be added after vis leafs
#define	CONTENTS_TRANSLUCENT	0x10000000	// auto set if any surface has trans
#define	CONTENTS_LADDER			0x20000000

#define	SURF_LIGHT		0x1		// value will hold the light strength
#define	SURF_SLICK		0x2		// effects game physics
#define	SURF_SKY		0x4		// don't draw, but add to skybox
#define	SURF_WARP		0x8		// turbulent water warp
#define	SURF_TRANS33	0x10
#define	SURF_TRANS66	0x20
#define	SURF_FLOWING	0x40	// scroll towards angle
#define	SURF_NODRAW		0x80	// don't bother referencing the texture

// content masks
#define	MASK_ALL				( -1 )
#define	MASK_SOLID				( CONTENTS_SOLID|CONTENTS_WINDOW )
#define	MASK_PLAYERSOLID		( CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER )
#define	MASK_DEADSOLID			( CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW )
#define	MASK_MONSTERSOLID		( CONTENTS_SOLID|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER )
#define	MASK_WATER				( CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME )
#define	MASK_OPAQUE				( CONTENTS_SOLID|CONTENTS_SLIME|CONTENTS_LAVA )
#define	MASK_SHOT				( CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEADMONSTER )
#define MASK_CURRENT			( CONTENTS_CURRENT_0|CONTENTS_CURRENT_90|CONTENTS_CURRENT_180|CONTENTS_CURRENT_270|CONTENTS_CURRENT_UP|CONTENTS_CURRENT_DOWN )

// gi.BoxEdicts( ) can return a list of either solid or trigger entities
// FIXME: eliminate AREA_ distinction?
#define	AREA_SOLID		1
#define	AREA_TRIGGERS	2

// plane_t structure
typedef struct cplane_s {

	Vec3	normal;
	float	dist;
	byte	type;			// for fast side tests
	byte	signbits;		// signx +( signy<<1 ) +( signz<<1 )
	byte	pad[ 2 ];

} cplane_t;

typedef struct cmodel_s {

	Vec3		mins, maxs;
	Vec3		origin;		// for sounds or lights
	int			headnode;

} cmodel_t;

typedef struct csurface_s {

	char		name[ 16 ];
	int			flags;
	int			value;

} cPatch_t;

typedef struct mapsurface_s { // used internally due to name len probs //ZOID {

	cPatch_t	c;
	char		rname[ 32 ];

} mapsurface_t;

// a trace is returned when a box is swept through the world
typedef struct {

	bool				allsolid;	// if true, plane is not valid
	bool				startsolid;	// if true, the initial point was in a solid area
	float				fraction;	// time completed, 1.0 = didn't hit anything
	Vec3				endpos;		// final position
	cplane_t			plane;		// surface normal at impact
	cPatch_t *		surface;	// surface hit
	int					contents;	// contents on other side of surface hit
	struct edict_s *	ent;		// not set by CM_* ( ) functions

} trace_t;

typedef struct {

	cplane_t * plane;
	int children[ 2 ]; // negative numbers are leafs

} cnode_t;

typedef struct {

	cplane_t * plane;
	mapsurface_t * surface;

} cbrushside_t;

typedef struct {

	int contents;
	int cluster;
	int area;
	unsigned short firstleafbrush;
	unsigned short numleafbrushes;

} cleaf_t;

typedef struct {

	int contents;
	int numsides;
	int firstbrushside;
	int checkcount; // to avoid repeated testings

} cbrush_t;

typedef struct {

	int numareaportals;
	int firstareaportal;
	int floodnum; // if two areas have equal floodnums, they are connected
	int floodvalid;

} carea_t;

class CollisionModel {

private:

	static CVar			map_noareas;

	static Str			map_entitystring;
	static Str			map_name;

public:

	static cmodel_t	*	CM_LoadMap( const Str & name, bool clientload, unsigned * checksum );
	static cmodel_t	*	CM_InlineModel( const Str & name );	// * 1, * 2, etc

	static int			CM_NumClusters( );
	static int			CM_NumInlineModels( );
	static Str			CM_EntityString( );

	// creates a clipping hull for an arbitrary box
	static int			CM_HeadnodeForBox( Vec3 & mins, Vec3 & maxs );


	// returns an ORed contents mask
	static int			CM_PointContents( Vec3 & p, int headnode );
	static int			CM_TransformedPointContents( Vec3 & p, int headnode, Vec3 & origin, Vec3 & angles );

	static trace_t		CM_BoxTrace( Vec3 & start, Vec3 & end, Vec3 & mins, Vec3 & maxs, int headnode, int brushmask );
	static trace_t		CM_TransformedBoxTrace( Vec3 & start, Vec3 & end, Vec3 & mins, Vec3 & maxs, int headnode, int brushmask, Vec3 & origin, Vec3 & angles );

	static byte *		CM_ClusterPVS( int cluster );
	static byte *		CM_ClusterPHS( int cluster );

	static int			CM_PointLeafnum( Vec3 & p );

	// call with topnode set to the headnode, returns with topnode
	// set to the first node that splits the box
	static int			CM_BoxLeafnums( Vec3 & mins, Vec3 & maxs, int * list, int listsize, int * topnode );

	static int			CM_LeafContents( int leafnum );
	static int			CM_LeafCluster( int leafnum );
	static int			CM_LeafArea( int leafnum );

	static void			CM_SetAreaPortalState( int portalnum, bool open );
	static bool			CM_AreasConnected( int area1, int area2 );

	static int			CM_WriteAreaBits( byte * buffer, int area );
	static bool			CM_HeadnodeVisible( int headnode, byte * visbits );

	static void			CM_WritePortalState( File & f );
	static void			CM_ReadPortalState( File & f );

	static void			CMod_LoadSubmodels( lump_t * l );
	static void			CMod_LoadSurfaces( lump_t * l );
	static void			CMod_LoadNodes( lump_t * l );
	static void			CMod_LoadBrushes( lump_t * l );
	static void			CMod_LoadLeafs( lump_t * l );
	static void			CMod_LoadPlanes( lump_t * l );
	static void			CMod_LoadLeafBrushes( lump_t * l );
	static void			CMod_LoadBrushSides( lump_t * l );
	static void			CMod_LoadAreas( lump_t * l );
	static void			CMod_LoadAreaPortals( lump_t * l );
	static void			CMod_LoadVisibility( lump_t * l );
	static void			CMod_LoadEntityString( lump_t * l );
	
	static void			CM_InitBoxHull( );
	static int			CM_PointLeafnum_r( Vec3 & p, int num );
	static void			CM_BoxLeafnums_r( int nodenum );
	static int			CM_BoxLeafnums_headnode( Vec3 & mins, Vec3 & maxs, int * list, int listsize, int headnode, int * topnode );
	static void			CM_ClipBoxToBrush( Vec3 & mins, Vec3 & maxs, Vec3 & p1, Vec3 & p2, trace_t * trace, cbrush_t * brush );
	static void			CM_TestBoxInBrush( Vec3 & mins, Vec3 & maxs, Vec3 & p1, trace_t * trace, cbrush_t * brush );
	static void			CM_TraceToLeaf( int leafnum );
	static void			CM_TestInLeaf( int leafnum );
	static void			CM_RecursiveHullCheck( int num, float p1f, float p2f, Vec3 & p1, Vec3 & p2 );
	static void			CM_DecompressVis( byte * in, byte * out );
	static void			FloodArea_r( carea_t * area, int floodnum );
	static void			FloodAreaConnections( );
};
