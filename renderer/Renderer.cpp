#include "precompiled.h"
#pragma hdrstop

List< RenderModelStatic * >				Renderer::g_mapBrushes;
BSPTree									Renderer::g_pvsTree;

RenderModel *							Renderer::g_missingModel;
CANamedList< RenderModel *, 256 >		Renderer::g_renderModels;

BufferAllocatorMap *					Renderer::g_mapBufferAllocator;
BufferAllocatorModel *					Renderer::g_modelBufferAllocator;
Mat4									Renderer::g_localCoordSystemRotation = Angles( 0.0f, -90.0f, 90.0f ).ToMat4( );
Vec3									Renderer::g_sunPosition;
Angles									Renderer::g_sunNormal;
bool									Renderer::g_sunRedraw;

// // //// // //// // //// //
// Renderer
// //// // //// // //// //

float Renderer::GetAspectRatio( ) {
	return ( float )Video::viddef.width / ( float )Video::viddef.height;
}

void Renderer::TakeScreenshot( ) {
	Str			picname; 
	Str			screenshotFormat = "png";
	for( int i = 0; ; i++ ) {
		if( i > 999 )
			return;
		sprintf( picname, "sh_%0.3i.%s", i, screenshotFormat.c_str( ) );
		if( !FileSystem::TouchFile( picname ) )
			break;
	}
	//MainRendererThread::TakeScreenShot( picname );
	g_objectPtr->d_rendererThreadParams.d_nameScreenShot = picname;
	g_objectPtr->d_rendererThreadParams.d_takeScreenShot = true;
}

void Renderer::SunPosition( ) {

	if( Command::Cmd_Argc( ) != 5 ) {
		Scene & rScene = g_objectPtr->d_rendererThreadParams.d_scene;
		Rotation rRot = Angles( rScene.viewangles[ 1 ], -rScene.viewangles[ 2 ], -rScene.viewangles[ 0 ] ).ToRotation( );
		g_sunNormal.Set( rScene.viewangles[ 1 ], -rScene.viewangles[ 2 ], -rScene.viewangles[ 0 ] );
		g_sunPosition = rScene.vieworg;
		g_sunRedraw = true;
		SVar::Find( "sunPlane" ) = Plane( rRot.GetVec( ), rScene.vieworg.Length( ) );
		return;
	}

	Token token;
	float x, y, z, d;

	token.type = TT_NUMBER;
	token.subtype = TT_DECIMAL | TT_FLOAT;
	token = Command::Cmd_Argv( 1 );
	x = token.StripLeadingOnce( "-" ) ? token.GetFloatValue( ) * -1.0f : token.GetFloatValue( );

	token.type = TT_NUMBER;
	token.subtype = TT_DECIMAL | TT_FLOAT;
	token = Command::Cmd_Argv( 2 );
	y = token.StripLeadingOnce( "-" ) ? token.GetFloatValue( ) * -1.0f : token.GetFloatValue( );

	token.type = TT_NUMBER;
	token.subtype = TT_DECIMAL | TT_FLOAT;
	token = Command::Cmd_Argv( 3 );
	z = token.StripLeadingOnce( "-" ) ? token.GetFloatValue( ) * -1.0f : token.GetFloatValue( );

	token.type = TT_NUMBER;
	token.subtype = TT_DECIMAL | TT_FLOAT;
	token = Command::Cmd_Argv( 4 );
	d = token.StripLeadingOnce( "-" ) ? token.GetFloatValue( ) * -1.0f : token.GetFloatValue( );

	Plane newSunPlane( x, y, z, d );

	if( newSunPlane.Normalize( ) != 0.0f )
		SVar::Find( "sunPlane" ) = newSunPlane;
	else
		Common::Com_Printf( "sun: wrong parameters\n" );
}

bool Renderer::Init( void * hinstance, void * hWnd ) {

	Command::Cmd_AddCommand( "ss", TakeScreenshot );
	Command::Cmd_AddCommand( "sun", SunPosition );

	GLImports::PreInit( );

	if( !GLOSLayer::Init( hinstance, hWnd ) )
		return false;

	if( !SetMode( ) ) {
        Common::Com_Printf( "ref_gl::R_Init( ) - could not R_SetMode( )\n" );
		return false;
	}

	//Video::VID_MenuInit( );
	GLImports::Init( );
	GLContext::InitParent( );
	DWImports::Init( );
	GUIThread::Init( );

	g_mapBufferAllocator		= new BufferAllocatorMap( );
	g_modelBufferAllocator		= new BufferAllocatorModel( );

	GLint fxaaSupport;
	GLImports::glGetIntegerv( GL_FRAMEBUFFER_SRGB_CAPABLE_EXT, &fxaaSupport );

	//RendererThreadParams & defaultRenderingParams = MainRendererThread::GetThreadParams( );

	g_missingModel = NULL;
	g_missingModel = RegisterModel( "models/wrench.md5mesh" );

	g_sunPlane = Plane( 0.62f, -0.74f, -0.27f, 2000 );
	g_sunPlane.GetDataRef( ).Normalize( );

	g_sunPosition.Set( 3500, -3500, 1750 );
	g_sunNormal.Set( 135, 0, -36 );
	g_sunRedraw = true;

	GLImports::wglSwapInterval( 0 );
	GLImports::GLErrorCheck( );

	//MainRendererThread::Init( );
	GLContext::BindDefault( );
	new Renderer( );
	g_objectPtr->d_signalEnd.Wait( );
	g_objectPtr->d_signalEnd.Set( );
	GLOSLayer::GetContext( 0 ).Bind( );
	MaterialSystem::Init( );
	//MovieExport::Init( );

	return true;
}

void Renderer::Shutdown( ) {	
	Command::Cmd_RemoveCommand( "ss" );
	Command::Cmd_RemoveCommand( "sun" );

	//MainRendererThread::Shutdown( );
	delete g_objectPtr;

	g_mapBrushes.DeleteContents( true );
	delete g_mapBufferAllocator;
	delete g_modelBufferAllocator;

	MaterialSystem::Shutdown( );
	GUIThread::Shutdown( );
	DWImports::Shutdown( );
	//MovieExport::Shutdown( );
	GLOSLayer::Shutdown( );
	GLImports::Shutdown( );
}

bool Renderer::SetMode( ) {
	bool fullscreen = Common::vid_fullscreen.GetBool( );
	if( !GLOSLayer::SetMode( Vec2( ( float )Common::vid_width.GetInt( ), ( float )Common::vid_height.GetInt( ) ), fullscreen ) )
		return false;
	return true;
}

void Renderer::BeginRegistration( const Str & model ) {
	Str fullname;
	sprintf( fullname, "maps/%s.map", model.c_str( ) );
	g_mapBrushes.DeleteContents( true );
	MapData * mapData = MapSystem::LoadMapFile( fullname );
	mapData->BuildRenderGeometry( g_mapBrushes, g_mapBufferAllocator );
	g_pvsTree.Clear( );
	g_pvsTree.AddRenderModelStaticList( g_mapBrushes, mapData->GetBounds( ) );
	Common::Com_Printf( "Pvs btree ready, depth: %i, nodes: %i, leafs: %i\n", g_pvsTree.GetDepth( ), g_pvsTree.GetNodeCount( ), g_pvsTree.GetLeafCount( ) );
	InitPVS( );
}

void Renderer::EndRegistration( ) {
}

RenderModel * Renderer::RegisterModel( const Str & name ) {
	RenderModel * renderModel = g_renderModels.FindByName( name );
	if( renderModel )
		return renderModel;
	renderModel = RenderModel::Parse( name );
	if( !renderModel ) {
		Common::Com_Printf( "Renderer::RegisterModel: No such model \"%s\".\n", name.c_str( ) );
		return g_missingModel;
	}
	renderModel->Setup( g_modelBufferAllocator );
	g_renderModels.Append( renderModel );
	return renderModel;
}

void Renderer::SetSky( const Str & name, float rotate, Vec3 & axis ) {
}

void Renderer::BeginFrame( ) {
}

void SceneEntity::Clear( ) {
	model		= NULL;
	angles.Zero( );
	origin.Zero( );
	frame		= 0;	
	oldorigin.Zero( );
	oldframe	= 0;
	backlerp	= 0;
	skinnum		= 0;
	lightstyle	= 0;
	alpha		= 0;
	skin		= NULL;
	flags		= 0;
}

void Scene::Clear( ) {
	x			= 0;
	y			= 0;
	width		= 0;
	height		= 0;
	fov_x		= 0;
	fov_y		= 0;
	vieworg.Zero( );
	viewangles.Zero( );
	time		= 0;
	rdflags		= 0;
	d_zFar		= 0;
	d_zNear		= 0;
	d_viewMatrix.Zero( );
	d_viewProjectionMatrix.Zero( );	
	entities.Clear( );
	dlights.Clear( );
	for( int i = 0; i < 4; i++ )
		blend[ i ]	= 0;
}

void Scene::CalculateProjection( ) {
	d_viewProjectionMatrix.PerspectiveSelf( d_zNear, d_zFar, fov_y, Renderer::GetAspectRatio( ) );
	d_viewMatrix = Angles( viewangles[ 1 ], -viewangles[ 2 ], -viewangles[ 0 ] ).ToMat4( );
	d_viewMatrix *= Renderer::GetCoordSystemRotation( );
	d_viewMatrix.TranslateSelf( -vieworg );
	d_viewProjectionMatrix *= d_viewMatrix;
}

void Renderer::RenderFrame( const Scene & scene ) {
	//MainRendererThread::RenderFrame( scene );
	g_objectPtr->Wait( );
	g_objectPtr->d_rendererThreadParams.d_scene = scene;
	g_objectPtr->DoWork( );
}

void Renderer::EndFrame( ) {
	GLOSLayer::EndFrame( );
	if( g_objectPtr->g_renSleep.GetInt( ) )
		WindowsNS::Sleep( g_objectPtr->g_renSleep.GetInt( ) );
	//MovieExport::Frame( );
}
