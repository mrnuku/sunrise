#ifndef CM_H
#define CM_H

class ODEContact {

public:

	Vec3						pos;			///< contact position
	Vec3						normal;			///< normal vector
	dReal						depth;			///< penetration depth
	dGeomID						g1, g2;			///< the colliding geoms
	int							side1, side2;	///< (to be documented)
	class CModel *				cmod;
};

typedef void( ODE_API * PFNDINITODEPROC )( );
typedef int( ODE_API * PFNDINITODE2PROC )( unsigned int uiInitFlags/*=0*/ );
typedef void( ODE_API * PFNDCLOSEODEPROC )( );
typedef dWorldID( ODE_API * PFNDWORLDCREATEPROC )( );
typedef void( ODE_API * PFNDWORLDDESTROYPROC )( dWorldID world_id );
typedef dSpaceID( ODE_API * PFNDSIMPLESPACECREATEPROC )( dSpaceID space );
typedef dSpaceID( ODE_API * PFNDHASHSPACECREATEPROC )( dSpaceID space );
typedef dSpaceID( ODE_API * PFNDQUADTREESPACECREATEPROC )( dSpaceID space, const dVector3 Center, const dVector3 Extents, int Depth );
typedef dGeomID( ODE_API * PFNDCREATEPLANEPROC )( dSpaceID space, dReal a, dReal b, dReal c, dReal d );
typedef void( ODE_API * PFNDGEOMPLANESETPARAMSPROC )( dGeomID plane, dReal a, dReal b, dReal c, dReal d );
typedef void( ODE_API * PFNDGEOMPLANEGETPARAMSPROC )( dGeomID plane, dVector4 result );
typedef dReal( ODE_API * PFNDGEOMPLANEPOINTDEPTHPROC )( dGeomID plane, dReal x, dReal y, dReal z );
typedef dGeomID( ODE_API * PFNDCREATEBOXPROC )( dSpaceID space, dReal lx, dReal ly, dReal lz );
typedef void( ODE_API * PFNDGEOMBOXSETLENGTHSPROC )( dGeomID box, dReal lx, dReal ly, dReal lz );
typedef void( ODE_API * PFNDGEOMBOXGETLENGTHSPROC )( dGeomID box, dVector3 result );
typedef dReal( ODE_API * PFNDGEOMBOXPOINTDEPTHPROC )( dGeomID box, dReal x, dReal y, dReal z );
typedef dGeomID( ODE_API * PFNDCREATERAYPROC )( dSpaceID space, dReal length );
typedef void( ODE_API * PFNDGEOMRAYSETLENGTHPROC )( dGeomID ray, dReal length );
typedef dReal( ODE_API * PFNDGEOMRAYGETLENGTHPROC )( dGeomID ray );
typedef void( ODE_API * PFNDGEOMRAYSETPROC )( dGeomID ray, dReal px, dReal py, dReal pz, dReal dx, dReal dy, dReal dz );
typedef void( ODE_API * PFNDGEOMRAYGETPROC )( dGeomID ray, dVector3 start, dVector3 dir );
typedef void( ODE_API * PFNDGEOMRAYSETPARAMSPROC )( dGeomID g, int FirstContact, int BackfaceCull );
typedef void( ODE_API * PFNDGEOMRAYGETPARAMSPROC )( dGeomID g, int * FirstContact, int * BackfaceCull );
typedef void( ODE_API * PFNDGEOMRAYSETCLOSESTHITPROC )( dGeomID g, int closestHit );
typedef int( ODE_API * PFNDGEOMRAYGETCLOSESTHITPROC )( dGeomID g );
typedef dTriMeshDataID( ODE_API * PFNDGEOMTRIMESHDATACREATEPROC )( );
typedef void( ODE_API * PFNDGEOMTRIMESHDATADESTROYPROC )( dTriMeshDataID g );
typedef void( ODE_API * PFNDGEOMTRIMESHDATASETPROC )( dTriMeshDataID g, int data_id, void * in_data );
typedef void *( ODE_API * PFNDGEOMTRIMESHDATAGETPROC )( dTriMeshDataID g, int data_id );
typedef void( ODE_API * PFNDGEOMTRIMESHSETLASTTRANSFORMPROC )( dGeomID g, dMatrix4 last_trans );
typedef dReal *( ODE_API * PFNDGEOMTRIMESHGETLASTTRANSFORMPROC )( dGeomID g );
typedef void( ODE_API * PFNDGEOMTRIMESHDATABUILDSINGLEPROC )( dTriMeshDataID g, const void * Vertices, int VertexStride, int VertexCount, const void * Indices, int IndexCount, int TriStride );
typedef void( ODE_API * PFNDGEOMTRIMESHDATABUILDSINGLE1PROC )( dTriMeshDataID g, const void * Vertices, int VertexStride, int VertexCount, const void * Indices, int IndexCount, int TriStride, const void * Normals );
typedef void( ODE_API * PFNDGEOMTRIMESHDATAPREPROCESSPROC )( dTriMeshDataID g );
typedef void( ODE_API * PFNDGEOMTRIMESHDATAGETBUFFERPROC )( dTriMeshDataID g, unsigned char ** buf, int * bufLen );
typedef void( ODE_API * PFNDGEOMTRIMESHDATASETBUFFERPROC )( dTriMeshDataID g, unsigned char * buf );
typedef dGeomID( ODE_API * PFNDCREATETRIMESHPROC )( dSpaceID space, dTriMeshDataID Data, dTriCallback * Callback, dTriArrayCallback * ArrayCallback, dTriRayCallback * RayCallback );
typedef void( ODE_API * PFNDGEOMTRIMESHENABLETCPROC )( dGeomID g, int geomClass, int enable );
typedef int( ODE_API * PFNDGEOMTRIMESHISTCENABLEDPROC )( dGeomID g, int geomClass );
typedef void( ODE_API * PFNDGEOMTRIMESHCLEARTCCACHEPROC )( dGeomID g );
typedef int( ODE_API * PFNDGEOMTRIMESHGETTRIANGLECOUNTPROC )( dGeomID g );
typedef void( ODE_API * PFNDGEOMTRIMESHDATAUPDATEPROC )( dTriMeshDataID g );
typedef void( ODE_API * PFNDGEOMDESTROYPROC )( dGeomID geom );
typedef void( ODE_API * PFNDGEOMSETPOSITIONPROC )( dGeomID geom, dReal x, dReal y, dReal z );
typedef void( ODE_API * PFNDSPACECOLLIDEPROC )( dSpaceID space, void *data, dNearCallback * callback );
typedef void( ODE_API * PFNDSPACECOLLIDE2PROC )( dGeomID space1, dGeomID space2, void * data, dNearCallback * callback );
typedef int( ODE_API * PFNDCOLLIDEPROC )( dGeomID o1, dGeomID o2, int flags, ODEContact * contact, int skip );

class ODEImports {

private:

	static DynamicModule *							odeModule;

public:

	static PFNDINITODEPROC							dInitODE;
	static PFNDINITODE2PROC							dInitODE2;
	static PFNDCLOSEODEPROC							dCloseODE;
	static PFNDWORLDCREATEPROC						dWorldCreate;
	static PFNDWORLDDESTROYPROC						dWorldDestroy;
	static PFNDSIMPLESPACECREATEPROC				dSimpleSpaceCreate;
	static PFNDHASHSPACECREATEPROC					dHashSpaceCreate;
	static PFNDQUADTREESPACECREATEPROC				dQuadTreeSpaceCreate;
	static PFNDCREATEPLANEPROC						dCreatePlane;
	static PFNDGEOMPLANESETPARAMSPROC				dGeomPlaneSetParams;
	static PFNDGEOMPLANEGETPARAMSPROC				dGeomPlaneGetParams;
	static PFNDGEOMPLANEPOINTDEPTHPROC				dGeomPlanePointDepth;
	static PFNDCREATEBOXPROC						dCreateBox;
	static PFNDGEOMBOXSETLENGTHSPROC				dGeomBoxSetLengths;
	static PFNDGEOMBOXGETLENGTHSPROC				dGeomBoxGetLengths;
	static PFNDGEOMBOXPOINTDEPTHPROC				dGeomBoxPointDepth;
	static PFNDCREATERAYPROC						dCreateRay;
	static PFNDGEOMRAYSETLENGTHPROC					dGeomRaySetLength;
	static PFNDGEOMRAYGETLENGTHPROC					dGeomRayGetLength;
	static PFNDGEOMRAYSETPROC						dGeomRaySet;
	static PFNDGEOMRAYGETPROC						dGeomRayGet;
	static PFNDGEOMRAYSETPARAMSPROC					dGeomRaySetParams;
	static PFNDGEOMRAYGETPARAMSPROC					dGeomRayGetParams;
	static PFNDGEOMRAYSETCLOSESTHITPROC				dGeomRaySetClosestHit;
	static PFNDGEOMRAYGETCLOSESTHITPROC				dGeomRayGetClosestHit;
	static PFNDGEOMTRIMESHDATACREATEPROC			dGeomTriMeshDataCreate;
	static PFNDGEOMTRIMESHDATADESTROYPROC			dGeomTriMeshDataDestroy;
	static PFNDGEOMTRIMESHDATASETPROC				dGeomTriMeshDataSet;
	static PFNDGEOMTRIMESHDATAGETPROC				dGeomTriMeshDataGet;
	static PFNDGEOMTRIMESHSETLASTTRANSFORMPROC		dGeomTriMeshSetLastTransform;
	static PFNDGEOMTRIMESHGETLASTTRANSFORMPROC		dGeomTriMeshGetLastTransform;
	static PFNDGEOMTRIMESHDATABUILDSINGLEPROC		dGeomTriMeshDataBuildSingle;
	static PFNDGEOMTRIMESHDATABUILDSINGLE1PROC		dGeomTriMeshDataBuildSingle1;
	static PFNDGEOMTRIMESHDATAPREPROCESSPROC		dGeomTriMeshDataPreprocess;
	static PFNDGEOMTRIMESHDATAGETBUFFERPROC			dGeomTriMeshDataGetBuffer;
	static PFNDGEOMTRIMESHDATASETBUFFERPROC			dGeomTriMeshDataSetBuffer;
	static PFNDCREATETRIMESHPROC					dCreateTriMesh;
	static PFNDGEOMTRIMESHENABLETCPROC				dGeomTriMeshEnableTC;
	static PFNDGEOMTRIMESHISTCENABLEDPROC			dGeomTriMeshIsTCEnabled;
	static PFNDGEOMTRIMESHCLEARTCCACHEPROC			dGeomTriMeshClearTCCache;
	static PFNDGEOMTRIMESHGETTRIANGLECOUNTPROC		dGeomTriMeshGetTriangleCount;
	static PFNDGEOMTRIMESHDATAUPDATEPROC			dGeomTriMeshDataUpdate;
	static PFNDGEOMDESTROYPROC						dGeomDestroy;
	static PFNDGEOMSETPOSITIONPROC					dGeomSetPosition;
	static PFNDSPACECOLLIDEPROC						dSpaceCollide;
	static PFNDSPACECOLLIDE2PROC					dSpaceCollide2;
	static PFNDCOLLIDEPROC							dCollide;

	static void										Init( );
	static void										Shutdown( );
	static bool										IsValid( )		{ return odeModule != NULL; }
};

// contents flags, NOTE: make sure to keep the defines in doom_defs.script up to date with these!
typedef enum {

	CONTENTS_SOLID				= BIT(0),	// an eye is never valid in a solid
	CONTENTS_OPAQUE				= BIT(1),	// blocks visibility (for ai)
	CONTENTS_WATER				= BIT(2),	// used for water
	CONTENTS_PLAYERCLIP			= BIT(3),	// solid to players
	CONTENTS_MONSTERCLIP		= BIT(4),	// solid to monsters
	CONTENTS_MOVEABLECLIP		= BIT(5),	// solid to moveable entities
	CONTENTS_IKCLIP				= BIT(6),	// solid to IK
	CONTENTS_BLOOD				= BIT(7),	// used to detect blood decals
	CONTENTS_BODY				= BIT(8),	// used for actors
	CONTENTS_PROJECTILE			= BIT(9),	// used for projectiles
	CONTENTS_CORPSE				= BIT(10),	// used for dead bodies
	CONTENTS_RENDERMODEL		= BIT(11),	// used for render models for collision detection
	CONTENTS_TRIGGER			= BIT(12),	// used for triggers
	CONTENTS_AAS_SOLID			= BIT(13),	// solid for AAS
	CONTENTS_AAS_OBSTACLE		= BIT(14),	// used to compile an obstacle into AAS that can be enabled/disabled
	CONTENTS_FLASHLIGHT_TRIGGER	= BIT(15),	// used for triggers that are activated by the flashlight
	CONTENTS_LADDER				= BIT(16),
	CONTENTS_SLIME				= BIT(17),
	CONTENTS_LAVA				= BIT(18),

	// contents used by utils
	CONTENTS_AREAPORTAL			= BIT(20),	// portal separating renderer areas
	CONTENTS_NOCSG				= BIT(21),	// don't cut this brush with CSG operations in the editor

	CONTENTS_REMOVE_UTIL		= ~(CONTENTS_AREAPORTAL|CONTENTS_NOCSG)

} contentsFlags_t;

// content masks
#define	MASK_ALL					(-1)
#define	MASK_SOLID					(CONTENTS_SOLID)
#define	MASK_MONSTERSOLID			(CONTENTS_SOLID|CONTENTS_MONSTERCLIP|CONTENTS_BODY)
#define	MASK_PLAYERSOLID			(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BODY)
#define	MASK_DEADSOLID				(CONTENTS_SOLID|CONTENTS_PLAYERCLIP)
#define	MASK_WATER					(CONTENTS_WATER)
#define	MASK_OPAQUE					(CONTENTS_OPAQUE)
#define	MASK_SHOT					(CONTENTS_SOLID|CONTENTS_RENDERMODEL)

// gi.BoxEdicts( ) can return a list of either solid or trigger entities
// FIXME: eliminate AREA_ distinction?
#define	AREA_SOLID		1
#define	AREA_TRIGGERS	2

class CModel {

public:

							CModel( int conts = 0 );
							~CModel( );

	Bounds					bounds;
	Vec3					origin;
	int						headnode;
	int						contents;

	const MapGeometry *		surface;

	dSpaceID				odeSpaceID;
	dTriMeshDataID			odeMeshID;
	List< dGeomID >			odeGeomIDs;

	int						BrushIndex( );
};

/// trace result
class CTrace {

public:

							CTrace( );

	void					Clear( );

	/// fraction of movement completed, 1.0 = didn't hit anything
	float					fraction;

	/// final position of trace model
	Vec3					endpos;

	/// if true, plane is not valid
	bool					allsolid;

	/// if true, the initial point was in a solid area
	bool					startsolid;

	/// surface normal at impact
	Plane					plane;

	/// surface hit
	Surface	*				surface;

	CModel *				brush;
	int						brushCheckCount;

	/// contents on other side of surface hit
	int						contents;

	/// not set by CM_* ( ) functions
	class Entity *			ent;
};

class CollisionModel {

	friend class				CModel;

private:

	static dWorldID				odeWorldID;
	static dSpaceID				odeBoxSpaceID;
	static dSpaceID				odeRaySpaceID;
	static dGeomID				odeBoxID;
	static dGeomID				odeRayID;

	static List< ODEContact >	odeContacts;
	static List< ODEContact >	odeContactsPrev;
	static List< ODEContact >	odeContactsFirst;

public:

	static List< CModel * >		cmodels;

	static CModel	*	CM_LoadMap( const Str & name, bool clientload, unsigned int & checksum );
	static CModel	*	CM_InlineModel( const Str & name );	// * 1, * 2, etc
	static CModel	*	CM_GetInlineModel( int indx );	// * 1, * 2, etc
	static int			CM_NumInlineModels( );
	static Str			CM_EntityString( );

	// creates a clipping hull for an arbitrary box
	static int			CM_HeadnodeForBox( Vec3 & mins, Vec3 & maxs );

	static void			ODENearCallback( void * data, dGeomID o1, dGeomID o2 );

	static CTrace		CM_PointTrace( const Vec3 & start, const Vec3 & end, int headnode, int brushmask );
	static CTrace		CM_BoundTrace( const Vec3 & start, const Vec3 & end, const Bounds & bounds, int headnode, int brushmask );
	static CTrace		CM_BoxTrace( Vec3 & start, Vec3 & end, Vec3 & mins, Vec3 & maxs, int headnode, int brushmask );
	static CTrace		CM_TransformedBoxTrace( Vec3 & start, Vec3 & end, Vec3 & mins, Vec3 & maxs, int headnode, int brushmask, Vec3 & origin, Vec3 & angles );

	// returns an ORed contents mask
	static int			CM_PointContents( const Vec3 & p, int headnode );
	static int			CM_TransformedPointContents( const Vec3 & p, int headnode, const Vec3 & origin, const Vec3 & angles );

	static void			CM_SetAreaPortalState( int portalnum, bool open );
	static bool			CM_AreasConnected( int area1, int area2 );

	static void			CM_WritePortalState( FileBase * f );
	static void			CM_ReadPortalState( FileBase * f );

	// call with topnode set to the headnode, returns with topnode
	// set to the first node that splits the box
	static int			CM_BoxLeafnums( Vec3 & mins, Vec3 & maxs, int * list, int listsize, int * topnode );
	static int			CM_NumClusters( );
	static int			CM_LeafCluster( int leafnum );
	static byte *		CM_ClusterPVS( int cluster );
	static byte *		CM_ClusterPHS( int cluster );
	static int			CM_PointLeafnum( Vec3 & p );
	static int			CM_LeafArea( int leafnum );
	static int			CM_WriteAreaBits( byte * buffer, int area );
	static bool			CM_HeadnodeVisible( int headnode, byte * visbits );
};

INLINE void CTrace::Clear( ) {

	fraction = 1.0f;
	endpos.Zero( );
	allsolid = false;
	startsolid = false;
	plane.Zero( );
	surface = NULL;
	brush = NULL;
	brushCheckCount = 0;
	contents = 0;
	ent = NULL;
}

INLINE CTrace::CTrace( ) {

	Clear( );
}

INLINE CModel::CModel( int conts ) {

	bounds.Clear( );
	origin.Zero( );
	headnode = 0;
	contents = conts;
}

INLINE CModel::~CModel( ) {
}

INLINE int CModel::BrushIndex( ) {
	
	return CollisionModel::cmodels.FindIndex( this );
}

#endif
