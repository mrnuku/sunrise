#include "precompiled.h"
#pragma hdrstop

List< CModel * >	CollisionModel::cmodels;
dWorldID			CollisionModel::odeWorldID;
dSpaceID			CollisionModel::odeBoxSpaceID;
dSpaceID			CollisionModel::odeRaySpaceID;
dGeomID				CollisionModel::odeBoxID;
dGeomID				CollisionModel::odeRayID;
List< ODEContact >	CollisionModel::odeContacts;
List< ODEContact >	CollisionModel::odeContactsPrev;
List< ODEContact >	CollisionModel::odeContactsFirst;

DynamicModule *							ODEImports::odeModule = NULL;
PFNDINITODEPROC							ODEImports::dInitODE;
PFNDINITODE2PROC						ODEImports::dInitODE2;
PFNDCLOSEODEPROC						ODEImports::dCloseODE;
PFNDWORLDCREATEPROC						ODEImports::dWorldCreate;
PFNDWORLDDESTROYPROC					ODEImports::dWorldDestroy;
PFNDSIMPLESPACECREATEPROC				ODEImports::dSimpleSpaceCreate;
PFNDHASHSPACECREATEPROC					ODEImports::dHashSpaceCreate;
PFNDQUADTREESPACECREATEPROC				ODEImports::dQuadTreeSpaceCreate;
PFNDCREATEPLANEPROC						ODEImports::dCreatePlane;
PFNDGEOMPLANESETPARAMSPROC				ODEImports::dGeomPlaneSetParams;
PFNDGEOMPLANEGETPARAMSPROC				ODEImports::dGeomPlaneGetParams;
PFNDGEOMPLANEPOINTDEPTHPROC				ODEImports::dGeomPlanePointDepth;
PFNDCREATEBOXPROC						ODEImports::dCreateBox;
PFNDGEOMBOXSETLENGTHSPROC				ODEImports::dGeomBoxSetLengths;
PFNDGEOMBOXGETLENGTHSPROC				ODEImports::dGeomBoxGetLengths;
PFNDGEOMBOXPOINTDEPTHPROC				ODEImports::dGeomBoxPointDepth;
PFNDCREATERAYPROC						ODEImports::dCreateRay;
PFNDGEOMRAYSETLENGTHPROC				ODEImports::dGeomRaySetLength;
PFNDGEOMRAYGETLENGTHPROC				ODEImports::dGeomRayGetLength;
PFNDGEOMRAYSETPROC						ODEImports::dGeomRaySet;
PFNDGEOMRAYGETPROC						ODEImports::dGeomRayGet;
PFNDGEOMRAYSETPARAMSPROC				ODEImports::dGeomRaySetParams;
PFNDGEOMRAYGETPARAMSPROC				ODEImports::dGeomRayGetParams;
PFNDGEOMRAYSETCLOSESTHITPROC			ODEImports::dGeomRaySetClosestHit;
PFNDGEOMRAYGETCLOSESTHITPROC			ODEImports::dGeomRayGetClosestHit;
PFNDGEOMTRIMESHDATACREATEPROC			ODEImports::dGeomTriMeshDataCreate;
PFNDGEOMTRIMESHDATADESTROYPROC			ODEImports::dGeomTriMeshDataDestroy;
PFNDGEOMTRIMESHDATASETPROC				ODEImports::dGeomTriMeshDataSet;
PFNDGEOMTRIMESHDATAGETPROC				ODEImports::dGeomTriMeshDataGet;
PFNDGEOMTRIMESHSETLASTTRANSFORMPROC		ODEImports::dGeomTriMeshSetLastTransform;
PFNDGEOMTRIMESHGETLASTTRANSFORMPROC		ODEImports::dGeomTriMeshGetLastTransform;
PFNDGEOMTRIMESHDATABUILDSINGLEPROC		ODEImports::dGeomTriMeshDataBuildSingle;
PFNDGEOMTRIMESHDATABUILDSINGLE1PROC		ODEImports::dGeomTriMeshDataBuildSingle1;
PFNDGEOMTRIMESHDATAPREPROCESSPROC		ODEImports::dGeomTriMeshDataPreprocess;
PFNDGEOMTRIMESHDATAGETBUFFERPROC		ODEImports::dGeomTriMeshDataGetBuffer;
PFNDGEOMTRIMESHDATASETBUFFERPROC		ODEImports::dGeomTriMeshDataSetBuffer;
PFNDCREATETRIMESHPROC					ODEImports::dCreateTriMesh;
PFNDGEOMTRIMESHENABLETCPROC				ODEImports::dGeomTriMeshEnableTC;
PFNDGEOMTRIMESHISTCENABLEDPROC			ODEImports::dGeomTriMeshIsTCEnabled;
PFNDGEOMTRIMESHCLEARTCCACHEPROC			ODEImports::dGeomTriMeshClearTCCache;
PFNDGEOMTRIMESHGETTRIANGLECOUNTPROC		ODEImports::dGeomTriMeshGetTriangleCount;
PFNDGEOMTRIMESHDATAUPDATEPROC			ODEImports::dGeomTriMeshDataUpdate;
PFNDGEOMDESTROYPROC						ODEImports::dGeomDestroy;
PFNDGEOMSETPOSITIONPROC					ODEImports::dGeomSetPosition;
PFNDSPACECOLLIDEPROC					ODEImports::dSpaceCollide;
PFNDSPACECOLLIDE2PROC					ODEImports::dSpaceCollide2;
PFNDCOLLIDEPROC							ODEImports::dCollide;

void ODEImports::Init( ) {

	dInitODE							= ::dInitODE;
	dInitODE2							= ::dInitODE2;
	dCloseODE							= ::dCloseODE;
	dWorldCreate						= ::dWorldCreate;
	dWorldDestroy						= ::dWorldDestroy;
	dSimpleSpaceCreate					= ::dSimpleSpaceCreate;
	dHashSpaceCreate					= ::dHashSpaceCreate;
	dQuadTreeSpaceCreate				= ::dQuadTreeSpaceCreate;
	dCreatePlane						= ::dCreatePlane;
	dGeomPlaneSetParams					= ::dGeomPlaneSetParams;
	dGeomPlaneGetParams					= ::dGeomPlaneGetParams;
	dGeomPlanePointDepth				= ::dGeomPlanePointDepth;
	dCreateBox							= ::dCreateBox;
	dGeomBoxSetLengths					= ::dGeomBoxSetLengths;
	dGeomBoxGetLengths					= ::dGeomBoxGetLengths;
	dGeomBoxPointDepth					= ::dGeomBoxPointDepth;
	dCreateRay							= ::dCreateRay;
	dGeomRaySetLength					= ::dGeomRaySetLength;
	dGeomRayGetLength					= ::dGeomRayGetLength;
	dGeomRaySet							= ::dGeomRaySet;
	dGeomRayGet							= ::dGeomRayGet;
	dGeomRaySetParams					= ::dGeomRaySetParams;
	dGeomRayGetParams					= ::dGeomRayGetParams;
	dGeomRaySetClosestHit				= ::dGeomRaySetClosestHit;
	dGeomRayGetClosestHit				= ::dGeomRayGetClosestHit;
	dGeomTriMeshDataCreate				= ::dGeomTriMeshDataCreate;
	dGeomTriMeshDataDestroy				= ::dGeomTriMeshDataDestroy;
	dGeomTriMeshDataSet					= ::dGeomTriMeshDataSet;
	dGeomTriMeshDataGet					= ::dGeomTriMeshDataGet;
	dGeomTriMeshSetLastTransform		= ::dGeomTriMeshSetLastTransform;
	dGeomTriMeshGetLastTransform		= ::dGeomTriMeshGetLastTransform;
	dGeomTriMeshDataBuildSingle			= ::dGeomTriMeshDataBuildSingle;
	dGeomTriMeshDataBuildSingle1		= ::dGeomTriMeshDataBuildSingle1;
	dGeomTriMeshDataPreprocess			= ::dGeomTriMeshDataPreprocess;
	dGeomTriMeshDataGetBuffer			= ::dGeomTriMeshDataGetBuffer;
	dGeomTriMeshDataSetBuffer			= ::dGeomTriMeshDataSetBuffer;
	dCreateTriMesh						= ::dCreateTriMesh;
	dGeomTriMeshEnableTC				= ::dGeomTriMeshEnableTC;
	dGeomTriMeshIsTCEnabled				= ::dGeomTriMeshIsTCEnabled;
	dGeomTriMeshClearTCCache			= ::dGeomTriMeshClearTCCache;
	dGeomTriMeshGetTriangleCount		= ::dGeomTriMeshGetTriangleCount;
	dGeomTriMeshDataUpdate				= ::dGeomTriMeshDataUpdate;
	dGeomDestroy						= ::dGeomDestroy;
	dGeomSetPosition					= ::dGeomSetPosition;
	dSpaceCollide						= ::dSpaceCollide;
	dSpaceCollide2						= ::dSpaceCollide2;
	dCollide							= ( PFNDCOLLIDEPROC )::dCollide;
#if 0
	odeModule							= new DynamicModule( "ode.dll" );

	dInitODE							= ( PFNDINITODEPROC )						odeModule->FindProc( "dInitODE" );
	dInitODE2							= ( PFNDINITODE2PROC )						odeModule->FindProc( "dInitODE2" );
	dCloseODE							= ( PFNDCLOSEODEPROC )						odeModule->FindProc( "dCloseODE" );
	dWorldCreate						= ( PFNDWORLDCREATEPROC )					odeModule->FindProc( "dWorldCreate" );
	dWorldDestroy						= ( PFNDWORLDDESTROYPROC )					odeModule->FindProc( "dWorldDestroy" );
	dSimpleSpaceCreate					= ( PFNDSIMPLESPACECREATEPROC )				odeModule->FindProc( "dSimpleSpaceCreate" );
	dHashSpaceCreate					= ( PFNDHASHSPACECREATEPROC )				odeModule->FindProc( "dHashSpaceCreate" );
	dQuadTreeSpaceCreate				= ( PFNDQUADTREESPACECREATEPROC )			odeModule->FindProc( "dQuadTreeSpaceCreate" );
	dCreatePlane						= ( PFNDCREATEPLANEPROC )					odeModule->FindProc( "dCreatePlane" );
	dGeomPlaneSetParams					= ( PFNDGEOMPLANESETPARAMSPROC )			odeModule->FindProc( "dGeomPlaneSetParams" );
	dGeomPlaneGetParams					= ( PFNDGEOMPLANEGETPARAMSPROC )			odeModule->FindProc( "dGeomPlaneGetParams" );
	dGeomPlanePointDepth				= ( PFNDGEOMPLANEPOINTDEPTHPROC )			odeModule->FindProc( "dGeomPlanePointDepth" );
	dCreateBox							= ( PFNDCREATEBOXPROC )						odeModule->FindProc( "dCreateBox" );
	dGeomBoxSetLengths					= ( PFNDGEOMBOXSETLENGTHSPROC )				odeModule->FindProc( "dGeomBoxSetLengths" );
	dGeomBoxGetLengths					= ( PFNDGEOMBOXGETLENGTHSPROC )				odeModule->FindProc( "dGeomBoxGetLengths" );
	dGeomBoxPointDepth					= ( PFNDGEOMBOXPOINTDEPTHPROC )				odeModule->FindProc( "dGeomBoxPointDepth" );
	dCreateRay							= ( PFNDCREATERAYPROC )						odeModule->FindProc( "dCreateRay" );
	dGeomRaySetLength					= ( PFNDGEOMRAYSETLENGTHPROC )				odeModule->FindProc( "dGeomRaySetLength" );
	dGeomRayGetLength					= ( PFNDGEOMRAYGETLENGTHPROC )				odeModule->FindProc( "dGeomRayGetLength" );
	dGeomRaySet							= ( PFNDGEOMRAYSETPROC )					odeModule->FindProc( "dGeomRaySet" );
	dGeomRayGet							= ( PFNDGEOMRAYGETPROC )					odeModule->FindProc( "dGeomRayGet" );
	dGeomRaySetParams					= ( PFNDGEOMRAYSETPARAMSPROC )				odeModule->FindProc( "dGeomRaySetParams" );
	dGeomRayGetParams					= ( PFNDGEOMRAYGETPARAMSPROC )				odeModule->FindProc( "dGeomRayGetParams" );
	dGeomRaySetClosestHit				= ( PFNDGEOMRAYSETCLOSESTHITPROC )			odeModule->FindProc( "dGeomRaySetClosestHit" );
	dGeomRayGetClosestHit				= ( PFNDGEOMRAYGETCLOSESTHITPROC )			odeModule->FindProc( "dGeomRayGetClosestHit" );
	dGeomTriMeshDataCreate				= ( PFNDGEOMTRIMESHDATACREATEPROC )			odeModule->FindProc( "dGeomTriMeshDataCreate" );
	dGeomTriMeshDataDestroy				= ( PFNDGEOMTRIMESHDATADESTROYPROC )		odeModule->FindProc( "dGeomTriMeshDataDestroy" );
	dGeomTriMeshDataSet					= ( PFNDGEOMTRIMESHDATASETPROC )			odeModule->FindProc( "dGeomTriMeshDataSet" );
	dGeomTriMeshDataGet					= ( PFNDGEOMTRIMESHDATAGETPROC )			odeModule->FindProc( "dGeomTriMeshDataGet" );
	dGeomTriMeshSetLastTransform		= ( PFNDGEOMTRIMESHSETLASTTRANSFORMPROC )	odeModule->FindProc( "dGeomTriMeshSetLastTransform" );
	dGeomTriMeshGetLastTransform		= ( PFNDGEOMTRIMESHGETLASTTRANSFORMPROC )	odeModule->FindProc( "dGeomTriMeshGetLastTransform" );
	dGeomTriMeshDataBuildSingle			= ( PFNDGEOMTRIMESHDATABUILDSINGLEPROC )	odeModule->FindProc( "dGeomTriMeshDataBuildSingle" );
	dGeomTriMeshDataBuildSingle1		= ( PFNDGEOMTRIMESHDATABUILDSINGLE1PROC )	odeModule->FindProc( "dGeomTriMeshDataBuildSingle1" );
	dGeomTriMeshDataPreprocess			= ( PFNDGEOMTRIMESHDATAPREPROCESSPROC )		odeModule->FindProc( "dGeomTriMeshDataPreprocess" );
	dGeomTriMeshDataGetBuffer			= ( PFNDGEOMTRIMESHDATAGETBUFFERPROC )		odeModule->FindProc( "dGeomTriMeshDataGetBuffer" );
	dGeomTriMeshDataSetBuffer			= ( PFNDGEOMTRIMESHDATASETBUFFERPROC )		odeModule->FindProc( "dGeomTriMeshDataSetBuffer" );
	dCreateTriMesh						= ( PFNDCREATETRIMESHPROC )					odeModule->FindProc( "dCreateTriMesh" );
	dGeomTriMeshEnableTC				= ( PFNDGEOMTRIMESHENABLETCPROC )			odeModule->FindProc( "dGeomTriMeshEnableTC" );
	dGeomTriMeshIsTCEnabled				= ( PFNDGEOMTRIMESHISTCENABLEDPROC )		odeModule->FindProc( "dGeomTriMeshIsTCEnabled" );
	dGeomTriMeshClearTCCache			= ( PFNDGEOMTRIMESHCLEARTCCACHEPROC )		odeModule->FindProc( "dGeomTriMeshClearTCCache" );
	dGeomTriMeshGetTriangleCount		= ( PFNDGEOMTRIMESHGETTRIANGLECOUNTPROC )	odeModule->FindProc( "dGeomTriMeshGetTriangleCount" );
	dGeomTriMeshDataUpdate				= ( PFNDGEOMTRIMESHDATAUPDATEPROC )			odeModule->FindProc( "dGeomTriMeshDataUpdate" );
	dGeomDestroy						= ( PFNDGEOMDESTROYPROC )					odeModule->FindProc( "dGeomDestroy" );
	dGeomSetPosition					= ( PFNDGEOMSETPOSITIONPROC )				odeModule->FindProc( "dGeomSetPosition" );
	dSpaceCollide						= ( PFNDSPACECOLLIDEPROC )					odeModule->FindProc( "dSpaceCollide" );
	dSpaceCollide2						= ( PFNDSPACECOLLIDE2PROC )					odeModule->FindProc( "dSpaceCollide2" );
	dCollide							= ( PFNDCOLLIDEPROC )						odeModule->FindProc( "dCollide" );
#endif
	ODEImports::dInitODE( );
}

void ODEImports::Shutdown( ) {
	
	dCloseODE( );
#if 0
	delete odeModule;
	odeModule = NULL;
#endif
}

#if 0
int PlaneReference::PlaneDistCmp( PlaneReference * const * a, PlaneReference * const * b ) {	
	if( ( *a )->Dist( ) > ( *b )->Dist( ) )
		return -1;
	if( ( *a )->Dist( ) < ( *b )->Dist( ) )
		return 1;
	return 0;
}

int PlaneReference::ParalellSetIndexCmp( PlaneReference * const * a, PlaneReference * const * b ) {
	int delta = ( *a )->paralellSetIndex - ( *b )->paralellSetIndex;
	if( delta )
		return delta;
	if( ( *a )->Dist( ) > ( *b )->Dist( ) )
		return -1;
	if( ( *a )->Dist( ) < ( *b )->Dist( ) )
		return 1;
	return 0;
}

int PlaneSet::ParalellNumCmp( const PlaneSet * a, const PlaneSet * b ) {	
	int delta = b->Num( ) - a->Num( );	
	return delta ? delta : b->SumReferenceCount( ) - a->SumReferenceCount( );
}
#endif

CModel * CollisionModel::CM_LoadMap( const Str & name, bool clientload, unsigned int & checksum ) {

	if( MapSystem::GetNumSections( ) && MapSystem::GetSection( 0 )->GetMapFile( )->GetName( ) == Str( name ).StripFileExtension( ) && ( clientload || !CVarSystem::GetCVarBool( "flushmap" ) ) ) {
		MapData * mapData = MapSystem::LoadMapFile( name );
		checksum = mapData->GetMapFile( )->GetGeometryCRC( );
		if( !clientload ) {
			//Common::Com_Memset( portalopen, 0, sizeof( portalopen ) );
			//FloodAreaConnections( );
		}
		return cmodels[ 0 ];
	}
	if( name.IsEmpty( ) )
		return NULL;
	if( !ODEImports::IsValid( ) ) {
		ODEImports::Init( );
		odeWorldID = ODEImports::dWorldCreate( );
		odeBoxSpaceID = ODEImports::dHashSpaceCreate( 0 );
		odeRaySpaceID = ODEImports::dHashSpaceCreate( 0 );
		odeBoxID = ODEImports::dCreateBox( odeBoxSpaceID, 1.0f, 1.0f, 1.0f );
		odeRayID = ODEImports::dCreateRay( odeRaySpaceID, 1.0f );
	}

	MapData * mapData = MapSystem::LoadMapFile( name );

	for( int i = 0; i < mapData->GetNumGeoms( ); i++ ) {
		const MapGeometry & mapGeom = ( *mapData )[ i ];
		CModel * loadModel = new CModel( 1 );
		loadModel->surface = &mapGeom;
		loadModel->odeSpaceID = ODEImports::dHashSpaceCreate( 0 );
		loadModel->odeMeshID = ODEImports::dGeomTriMeshDataCreate( );
		ODEImports::dGeomTriMeshDataBuildSingle( loadModel->odeMeshID, mapGeom.GetVertices( ), sizeof( Vec3 ), mapGeom.GetNumVertices( ),
			mapGeom.GetIndexes( ), mapGeom.GetNumIndexes( ), sizeof( int ) * 3 );
		ODEImports::dGeomTriMeshDataPreprocess( loadModel->odeMeshID );
		loadModel->odeGeomIDs.Append( ODEImports::dCreateTriMesh( loadModel->odeSpaceID, loadModel->odeMeshID, NULL, NULL, NULL ) );
		cmodels.Append( loadModel );
	}
	checksum = mapData->GetMapFile( )->GetGeometryCRC( );
	return cmodels[ 0 ];
}

CModel * CollisionModel::CM_InlineModel( const Str & name ) {
	if( name.IsEmpty( ) || name[ 0 ] != '*' )
		Common::Com_Error( ERR_DROP, "CollisionModel::CM_InlineModel: bad name" );
	Token token;
	int num = token.StrIntValue( name.Right( name.Length( ) - 1 ) );
	if( num < 1 || num >= cmodels.Num( ) )
		Common::Com_Error( ERR_DROP, "CollisionModel::CM_InlineModel: bad number" );
	return cmodels[ num ];
}

CModel * CollisionModel::CM_GetInlineModel( int indx ) {
	if( indx < 1 || indx >= cmodels.Num( ) )
		Common::Com_Error( ERR_DROP, "CollisionModel::CM_InlineModel: bad number" );
	return cmodels[ indx ];
}

int CollisionModel::CM_NumInlineModels( ) {
	return cmodels.Num( );
}

Str CollisionModel::CM_EntityString( ) {
	return Str( "" );
}


int CollisionModel::CM_HeadnodeForBox( Vec3 & mins, Vec3 & maxs ) {
	return 0;
}

void CollisionModel::ODENearCallback( void * data, dGeomID o1, dGeomID o2 ) {
	const int N = 64;
	ODEContact contact[ N ];
	int n = ODEImports::dCollide( o1, o2, N, contact, sizeof( ODEContact ) );
	for( int i = 0; i < n; i++ ) {
		contact[ i ].cmod = ( CModel * )data;
		odeContacts.Append( contact[ i ] );
	}
}

CTrace CollisionModel::CM_PointTrace( const Vec3 & start, const Vec3 & end, int headnode, int brushmask ) {

	Vec3 traceDir = end - start;
	float traceLen = traceDir.Length( );
	ODEImports::dGeomRaySetLength( odeRayID, traceLen );
	if( traceLen > 0.0f ) {
		traceDir.Normalize( );
		ODEImports::dGeomRaySet( odeRayID, start[ 0 ], start[ 1 ], start[ 2 ], traceDir[ 0 ], traceDir[ 1 ], traceDir[ 2 ] );
	}
	odeContacts.SetNum( 0, false );
	for( int i = 0; i < cmodels.Num( ); i++ ) {
		ODEImports::dSpaceCollide2( ( dGeomID )cmodels[ i ]->odeSpaceID, ( dGeomID )odeRaySpaceID, cmodels[ i ], ODENearCallback );
	}
	float traceSmLen = INFINITY;
	int traceSmIdx = -1;
	for( int cd = 0; cd < odeContacts.Num( ); cd++ ) {
		if( traceSmLen > odeContacts[ cd ].depth ) {
			traceSmIdx = cd;
			traceSmLen = odeContacts[ cd ].depth;
		}
	}
	CTrace trace;
	if( odeContacts.Num( ) && odeContacts[ traceSmIdx ].depth < traceLen ) {
		trace.endpos = odeContacts[ traceSmIdx ].pos;
		trace.plane = odeContacts[ traceSmIdx ].normal;
		trace.fraction = odeContacts[ traceSmIdx ].depth / traceLen;
		//trace.surface = odeContacts[ traceSmIdx ].cmod->surface;
		trace.surface++;
		trace.contents = odeContacts[ traceSmIdx ].cmod->contents;
	} else {		
		trace.endpos = end;
	}
	return trace;
}

CTrace CollisionModel::CM_BoundTrace( const Vec3 & start, const Vec3 & end, const Bounds & bounds, int headnode, int brushmask ) {

	CTrace trace;
	Vec3 traceDir = end - start, tracePoint = start + bounds.GetCenter( );
	float traceLen = traceDir.Length( );
	int step, numSteps = 1;

	if( traceLen > 4.0f ) {
		traceDir.Normalize( );
		numSteps = ( int )traceLen + 1;
	} else if( traceLen > 0.0f ) {
		traceDir /= 4.0f;
		numSteps = 4;
	}
	odeContacts.SetNum( 0, false );
	ODEImports::dGeomBoxSetLengths( odeBoxID, fabs( bounds[ 1 ][ 0 ] ) + fabs( bounds[ 0 ][ 0 ] ), fabs( bounds[ 1 ][ 1 ] ) + fabs( bounds[ 0 ][ 1 ] ), fabs( bounds[ 1 ][ 2 ] ) + fabs( bounds[ 0 ][ 2 ] ) );
	for( step = 0; step < numSteps; step++, tracePoint += traceDir ) {
		ODEImports::dGeomSetPosition( odeBoxID, tracePoint[ 0 ], tracePoint[ 1 ], tracePoint[ 2 ] );
		for( int i = 0; i < cmodels.Num( ); i++ ) {
			ODEImports::dSpaceCollide2( ( dGeomID )odeBoxSpaceID, ( dGeomID )cmodels[ i ]->odeSpaceID, cmodels[ i ], ODENearCallback );
		}
		if( odeContacts.Num( ) ) {
			if( !step ) {				
				trace.startsolid = true;
				trace.allsolid = true;
				odeContactsFirst = odeContacts;
			}
			if( !trace.startsolid )
				break;
			odeContactsPrev = odeContacts;
			odeContacts.SetNum( 0, false );			
		} else if( trace.startsolid ) {
			trace.allsolid = false;
			odeContacts = odeContactsPrev;
			break;
		}
	}
	if( trace.allsolid ) {
		odeContacts = odeContactsFirst;
		tracePoint = start + bounds.GetCenter( );
	}
	float traceSmLen = INFINITY;
	int traceSmIdx = -1;
	for( int cd = 0; cd < odeContacts.Num( ); cd++ ) {
		float traceDot = odeContacts[ cd ].normal * traceDir;
		if( traceSmLen > traceDot ) {
			traceSmIdx = cd;
			traceSmLen = traceDot;
		}
	}
	if( odeContacts.Num( ) ) {
		trace.endpos = tracePoint - bounds.GetCenter( );
		if( step != numSteps )
			trace.endpos -= traceDir;
		trace.plane = odeContacts[ traceSmIdx ].normal;
		trace.fraction = ( traceLen > 0.0f ) ? ( trace.endpos - start ).Length( ) / traceLen : 1.0f;
		//trace.surface = odeContacts[ traceSmIdx ].cmod->surface;
		trace.surface++;
		trace.contents = odeContacts[ traceSmIdx ].cmod->contents;
	} else {		
		trace.endpos = end;
	}
	return trace;
}

CTrace CollisionModel::CM_BoxTrace( Vec3 & start, Vec3 & end, Vec3 & mins, Vec3 & maxs, int headnode, int brushmask ) {
	return CM_BoundTrace( start, end, Bounds( mins, maxs ), headnode, brushmask );
}

CTrace CollisionModel::CM_TransformedBoxTrace( Vec3 & start, Vec3 & end, Vec3 & mins, Vec3 & maxs, int headnode, int brushmask, Vec3 & origin, Vec3 & angles ) {
	return CM_BoxTrace( start, end, mins, maxs, headnode, brushmask );
}


int CollisionModel::CM_PointContents( const Vec3 & p, int headnode ) {
	return 0;
}

int CollisionModel::CM_TransformedPointContents( const Vec3 & p, int headnode, const Vec3 & origin, const Vec3 & angles ) {
	return 0;
}

void CollisionModel::CM_SetAreaPortalState( int portalnum, bool open ) {
}

bool CollisionModel::CM_AreasConnected( int area1, int area2 ) {
	return false;
}

void CollisionModel::CM_WritePortalState( FileBase * f ) {
}

void CollisionModel::CM_ReadPortalState( FileBase * f ) {
}

int	 CollisionModel::CM_BoxLeafnums( Vec3 & mins, Vec3 & maxs, int * list, int listsize, int * topnode ) {
	return 0;
}

int CollisionModel::CM_NumClusters( ) {
	return 0;
}

int CollisionModel::CM_LeafCluster( int leafnum ) {
	return 0;
}

byte * CollisionModel::CM_ClusterPVS( int cluster ) {
	return NULL;
}

byte * CollisionModel::CM_ClusterPHS( int cluster ) {
	return NULL;
}

int CollisionModel::CM_PointLeafnum( Vec3 & p ) {
	return 0;
}

int CollisionModel::CM_LeafArea( int leafnum ) {
	return 0;
}

int CollisionModel::CM_WriteAreaBits( byte * buffer, int area ) {
	return 0;
}

bool CollisionModel::CM_HeadnodeVisible( int headnode, byte * visbits ) {
	return true;
}
