#include "precompiled.h"
#pragma hdrstop

int								Renderer::g_brushMarkIndex = -1;
int								Renderer::g_pvsMarkIndex = -1;
Renderer *						SingleObject< Renderer >::g_objectPtr = NULL;

GUIThread *						SingleObject< GUIThread >::g_objectPtr = NULL;

// // //// // //// // //// //
// GUIThread
// //// // //// // //// //

GUIThread::GUIThread( ) {
	d_name = "GUIThread";
	WorkerThreadBase::Init( );
}

GUIThread::~GUIThread( ) {
	WorkerThreadBase::Shutdown( );
}

void GUIThread::Init( ) {
	GLContext::BindDefault( );
	new GUIThread( );
	g_objectPtr->d_signalEnd.Wait( );
	//g_objectPtr->d_signalEnd.Set( );
	GLOSLayer::GetContext( 0 ).Bind( );
}

void GUIThread::Shutdown( ) {
	delete g_objectPtr;
}

INLINE void GUIThread::DebugTextUpdate( ) {
	d_debugWindow->SetText( d_debugText );
	d_debugText.Clear( );
}

unsigned int GUIThread::Run( void * param ) {	
	GLScopedContext		threadContext;
	d_overlay			= new Overlay( );
	d_primitiveBuffer	= new BufferAllocatorUI( );
	d_screenRectTexture	= d_primitiveBuffer->TexturePrimitive( Video::GetResolution( ) );
	d_frameBuffer		= new GLFrameBuffer( "GUIThreadFrameBuffer", Video::GetResolution( ), true, 0 );
	d_frameBuffer->SingleSetup( "c0,rgba8", "color" );
	d_frameTexture		= g_objectPtr->d_frameBuffer->GetTextureBuffer( "color" );
	d_uiMatrix			= &GetShader( )->FindInput( "uiMatrix" );
	d_uiTexture			= &GetShader( )->FindInput( "uiTexture" );
	d_projectionMatrix	= Projection::Orthogonal( 0, ( float )Video::GetResolution( )[ 0 ], ( float )Video::GetResolution( )[ 1 ], 0, 0, 99 );
	GUISystem::Init( );

	d_debugWindow = new GUIWindow( "TaharezLook/StaticText", "RendererText", 0.65f, 0.05f, 0.3f, 0.15f );
	d_debugWindow->Register( );
	d_debugWindow->SetBind( key_f6 );
	d_debugWindow->SetText( "Loading..." );
	d_debugWindow->SetPos( 0.325f, 1.0f - 0.25f );

	d_signalEnd.Set( );

	while( !d_signalStart.Wait( 50 ) ) {
		if( GUISystem::NeedRedraw( ) ) {
			Clear( );
			GUISystem::DrawGUI( );
			Draw( );
			GLImports::GLErrorCheck( );
			threadContext.SwapBuffers( );
		}
	}

	GUISystem::GetWindow( 0 )->Hide( );
	d_debugWindow->SetPos( 0.65f, 0.05f );
	d_debugWindow->SetAlpha( 0.5f );
	d_signalStart.Set( );

	while( 1 ) {
		d_signalStart.Wait( );
		if( Terminating( ) )
			break;
		DebugTextUpdate( );
		{
			GLScopedLock glLock;
			Clear( );
			GUISystem::DrawGUI( );
		}
		d_signalEnd.Set( );
	}

	GUISystem::Shutdown( );
	delete d_frameBuffer;
	delete d_overlay;
	d_primitiveBuffer->FreePrimitive( d_screenRectTexture );
	delete d_primitiveBuffer;
	return 0;
}

void GUIThread::Clear( ) {
	g_objectPtr->d_frameBuffer->Bind( );
	g_objectPtr->d_frameBuffer->AdjustViewport( );
	GLImports::glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	GLImports::glClear( GL_COLOR_BUFFER_BIT );
	g_objectPtr->d_frameBuffer->UnBind( );
}

void GUIThread::Draw( ) {
	GLImports::glEnable( GL_BLEND );
	GLImports::glBlendFuncSeparate( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE );
	GLShader * uiShader = g_objectPtr->d_overlay->GetShader( );
	*g_objectPtr->d_uiMatrix = g_objectPtr->d_projectionMatrix;
	*g_objectPtr->d_uiTexture = g_objectPtr->d_frameTexture;
	uiShader->Bind( );
	uiShader->WriteSVars( );
	g_objectPtr->d_primitiveBuffer->BeginDraw( );
	g_objectPtr->d_screenRectTexture->Draw( 1 );
	g_objectPtr->d_primitiveBuffer->EndDraw( );
	uiShader->UnBind( );
	GLImports::glDisable( GL_BLEND );
}

void GUIThread::DebugPrint( const char * fmt, ... ) {
	va_list		argPtr;
	Str			outputStr;
	if( !g_objectPtr )
		return;
	ScopedLock< true > scopedLock( g_objectPtr->d_debugLock );
	va_start( argPtr, fmt );
	vsprintf( outputStr, fmt, argPtr );
	va_end( argPtr );
	g_objectPtr->d_debugText += outputStr;
}

// // //// // //// // //// //
// ScreenShotThread
// //// // //// // //// //

ScreenShotThread::ScreenShotThread( const Str & file_name, Image * image ) {
	d_name		= "ScreenShotThread";
	d_fileName	= file_name;
	d_image		= image;
	WorkerThreadBase::Init( );
}

unsigned int ScreenShotThread::Run( void * param ) {
	d_image->Flip( );
	d_image->Save( d_fileName );
	delete d_image;
	return 0;
}

// // //// // //// // //// //
// RendererThreadParams
// //// // //// // //// //

RendererThreadParams::RendererThreadParams( ) {
	d_indexUploadLimit				= 4096;
	d_indexUploadLimitCounter		= 0;
	d_pvsPlanes.SetNum( 6 );
	d_pvsPrimitiveStart				= -1;
	d_pvsLock						= false;
	d_mapBufferAllocator			= NULL;
	d_modelBufferAllocator			= NULL;
	d_uiBufferAllocator				= NULL;
	d_takeScreenShot				= false;
}

void RendererThreadParams::ClearCounters( ) {
	d_indexUploadLimitCounter = 0;
	d_timerFrame.Clear( );
	d_timerPVS.Clear( );
	d_timerRender.Clear( );
	d_primitiveCountBrush = 0;
}

// // //// // //// // //// //
// MainRendererThread
// //// // //// // //// //

Renderer::Renderer( ) {
	d_name = "MainRendererThread";
	g_renSleep.DRegister(				"renSleep",		0 );
	g_pvsLock.DRegister(				"pvsLock",		false );
	g_brushMark.DRegister(				"brushMark",	-1 );
	g_pvsMark.DRegister(				"pvsMark",		null_string );
	g_shadowEnable.DRegister(			"shadow",		1 );
	g_ssaoEnable.DRegister(				"ssao",			1 );
	g_fsaaEnable.DRegister(				"fsaa",			1 );
	g_utilDraw.DRegister(				"utilb",		0 );
	g_shadowMapStaticSize.DRegister(	"smss",			0,		CVAR_ARCHIVE );
	g_shadowMapDynamicSize.DRegister(	"smds",			0,		CVAR_ARCHIVE );
	WorkerThreadBase::Init( );
}

Renderer::~Renderer( ) {
	WorkerThreadBase::Shutdown( );
}

void Renderer::DebugDrawPrimitive( GLShader * debug_shader, BufferPrimitive3D * primitive, SVar & debug_color ) {
	GLImports::glEnable( GL_DEPTH_TEST );
	GLImports::glEnable( GL_BLEND );
	GLImports::glEnable( GL_CULL_FACE );
	GLImports::glCullFace( GL_FRONT );
	debug_color = Vec4( 0.5f, 0.5f, 0.5f, 0.5f );
	debug_shader->WriteSVars( );
	d_rendererThreadParams.d_mapBufferAllocator->BeginDraw( );
	primitive->Draw( 1 );
	d_rendererThreadParams.d_mapBufferAllocator->EndDraw( );
	GLImports::glDisable( GL_CULL_FACE );
	GLImports::glDisable( GL_BLEND );
	GLImports::glDisable( GL_DEPTH_TEST );
	GLImports::glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	debug_color = Vec4( 1.0f, 1.0f, 1.0f, 1.0f );
	debug_shader->WriteSVars( );
	d_rendererThreadParams.d_mapBufferAllocator->BeginDraw( );
	primitive->Draw( 1 );
	d_rendererThreadParams.d_mapBufferAllocator->EndDraw( );
	GLImports::glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
	GLImports::glPointSize( 1.0f );
	debug_color = Vec4( 1.0f, 0.0f, 0.0f, 1.0f );
	debug_shader->WriteSVars( );
	d_rendererThreadParams.d_mapBufferAllocator->BeginDraw( );
	primitive->Draw( 1 );
	d_rendererThreadParams.d_mapBufferAllocator->EndDraw( );
	GLImports::glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}

void Renderer::UpdateBrushMark( ) {
	g_brushMark.ClearModified( );
	if( g_brushMarkIndex != -1 ) {
		for( int i = g_brushMarkIndex + 5; i >= g_brushMarkIndex; i-- ) {
			d_rendererThreadParams.d_mapBufferAllocator->FreePrimitive( d_rendererThreadParams.d_debugPrimitives[ i ] );
			d_rendererThreadParams.d_debugPrimitives.RemoveIndex( i );
		}
	}
	if( g_brushMark.GetInt( ) < 0 ) {
		g_brushMark.SetInt( -1 );
		g_brushMarkIndex = -1;
	} else {
		if( g_brushMark.GetInt( ) > ( g_mapBrushes.Num( ) - 1 ) )
			g_brushMark.SetInt( g_mapBrushes.Num( ) - 1 );
		CAList< Plane, 6 > brushBoundPlanes;
		brushBoundPlanes.SetNum( 6 );
		g_mapBrushes[ g_brushMark.GetInt( ) ]->GetBounds( ).ToPlanes( brushBoundPlanes.Ptr( ) );
		g_brushMarkIndex = d_rendererThreadParams.d_debugPrimitives.Append( d_rendererThreadParams.d_mapBufferAllocator->SurfacePrimitive( PolygonBuilder( brushBoundPlanes, 0 ) ) );
		d_rendererThreadParams.d_debugPrimitives.Append( d_rendererThreadParams.d_mapBufferAllocator->SurfacePrimitive( PolygonBuilder( brushBoundPlanes, 1 ) ) );
		d_rendererThreadParams.d_debugPrimitives.Append( d_rendererThreadParams.d_mapBufferAllocator->SurfacePrimitive( PolygonBuilder( brushBoundPlanes, 2 ) ) );
		d_rendererThreadParams.d_debugPrimitives.Append( d_rendererThreadParams.d_mapBufferAllocator->SurfacePrimitive( PolygonBuilder( brushBoundPlanes, 3 ) ) );
		d_rendererThreadParams.d_debugPrimitives.Append( d_rendererThreadParams.d_mapBufferAllocator->SurfacePrimitive( PolygonBuilder( brushBoundPlanes, 4 ) ) );
		d_rendererThreadParams.d_debugPrimitives.Append( d_rendererThreadParams.d_mapBufferAllocator->SurfacePrimitive( PolygonBuilder( brushBoundPlanes, 5 ) ) );
	}
}

void Renderer::UpdatePvsMark( ) {

	g_pvsMark.ClearModified( );
	if( g_pvsMarkIndex != -1 ) {
		for( int i = g_pvsMarkIndex + 5; i >= g_pvsMarkIndex; i-- ) {
			d_rendererThreadParams.d_mapBufferAllocator->FreePrimitive( d_rendererThreadParams.d_debugPrimitives[ i ] );
			d_rendererThreadParams.d_debugPrimitives.RemoveIndex( i );
		}
		g_pvsMarkIndex = -1;
	}
	BSPNode * node = g_pvsTree.GetRoot( );
	const Str & route = g_pvsMark.GetString( );
	for( int level = 0; level < route.Length( ); level++ ) {
		if( route[ level ] == '-' ) {
			if( !node->d_childs[ PLANESIDE_BACK ] ) {
				Common::Com_DPrintf( "PvsMark: no back child present at level %i\n", level );
				g_pvsMark.SetString( route.Left( level ) );
				break;
			}
			node = node->d_childs[ PLANESIDE_BACK ];
		} else if( route[ level ] == '+' ) {
			if( !node->d_childs[ PLANESIDE_FRONT ] ) {
				Common::Com_DPrintf( "PvsMark: no front child present at level %i\n", level );
				g_pvsMark.SetString( route.Left( level ) );
				break;
			}
			node = node->d_childs[ PLANESIDE_FRONT ];
		} else {			
			Common::Com_DPrintf( "PvsMark: wrong route symbol '%c' at level %i\n", route[ level ], level );
			g_pvsMark.SetString( route.Left( level ) );
			break;
		}
	}
	CAList< Plane, 6 > nodeBoundPlanes;
	nodeBoundPlanes.SetNum( 6 );
	node->d_key->d_bounds.ToPlanes( nodeBoundPlanes.Ptr( ) );
	g_pvsMarkIndex = d_rendererThreadParams.d_debugPrimitives.Append( d_rendererThreadParams.d_mapBufferAllocator->SurfacePrimitive( PolygonBuilder( nodeBoundPlanes, 0 ) ) );
	d_rendererThreadParams.d_debugPrimitives.Append( d_rendererThreadParams.d_mapBufferAllocator->SurfacePrimitive( PolygonBuilder( nodeBoundPlanes, 1 ) ) );
	d_rendererThreadParams.d_debugPrimitives.Append( d_rendererThreadParams.d_mapBufferAllocator->SurfacePrimitive( PolygonBuilder( nodeBoundPlanes, 2 ) ) );
	d_rendererThreadParams.d_debugPrimitives.Append( d_rendererThreadParams.d_mapBufferAllocator->SurfacePrimitive( PolygonBuilder( nodeBoundPlanes, 3 ) ) );
	d_rendererThreadParams.d_debugPrimitives.Append( d_rendererThreadParams.d_mapBufferAllocator->SurfacePrimitive( PolygonBuilder( nodeBoundPlanes, 4 ) ) );
	d_rendererThreadParams.d_debugPrimitives.Append( d_rendererThreadParams.d_mapBufferAllocator->SurfacePrimitive( PolygonBuilder( nodeBoundPlanes, 5 ) ) );
}

void Renderer::UpdatePvsLock( ) {

	g_pvsLock.ClearModified( );
	if( g_pvsLock.GetBool( ) ) {
		for( int i = 0; i < d_rendererThreadParams.d_pvsPlanes.Num( ); i++ ) {
			int idx = d_rendererThreadParams.d_debugPrimitives.Append( d_rendererThreadParams.d_mapBufferAllocator->SurfacePrimitive( PolygonBuilder( d_rendererThreadParams.d_pvsPlanes, i ) ) );
			if( !i )
				d_rendererThreadParams.d_pvsPrimitiveStart = idx;
		}
		d_rendererThreadParams.d_pvsLock = true;
	} else {
		for( int i = d_rendererThreadParams.d_pvsPrimitiveStart + ( d_rendererThreadParams.d_pvsPlanes.Num( ) - 1 ); i >= d_rendererThreadParams.d_pvsPrimitiveStart; i-- ) {
			d_rendererThreadParams.d_mapBufferAllocator->FreePrimitive( d_rendererThreadParams.d_debugPrimitives[ i ] );
			d_rendererThreadParams.d_debugPrimitives.RemoveIndex( i );
		}
		d_rendererThreadParams.d_pvsPrimitiveStart = -1;
		d_rendererThreadParams.d_pvsLock = false;
	}
}

#if 0
// Experimental UI system
// // //// // //// // //// //
/// CompactList
// //// // //// // //// //
template< class type > class CompactList {
private:
	int								d_num;
	type *							d_list;

	void							Free( );

public:
									CompactList( );
									~CompactList( );

	int								Alloc( int num );
	int								Num( ) const;
	const type &					operator[ ]( int index ) const;
	type &							operator[ ]( int index );
	CompactList< type > &			operator =( const CompactList< type > & other );
	type *							GetPtr( );
};

// // //// // //// // //// //
// CompactList
// //// // //// // //// //
template< class type > CompactList< type >::CompactList( ) {
	d_num = 0;
	d_list = NULL;
}

template< class type > CompactList< type >::~CompactList( ) {
	Free( );
}

template< class type > void CompactList< type >::Free( ) {
	if( d_num ) {
		delete[] d_list;
		d_num = 0;
		d_list = NULL;
	}
	assert( !d_list );
}

template< class type > int CompactList< type >::Alloc( int num ) {
	int oldNum = d_num;
	Free( );
	d_num = num;
	d_list = new type[ num ];
	return oldNum;
}

template< class type > INLINE int CompactList< type >::Num( ) const {
	return d_num;
}

template< class type > INLINE const type & CompactList< type >::operator[ ]( int index ) const {
	assert( ( index >= 0 ) && ( index <= d_num ) );
	return d_list[ index ];
}

template< class type > INLINE type & CompactList< type >::operator[ ]( int index ) {
	assert( ( index >= 0 ) && ( index <= d_num ) );
	return d_list[ index ];
}

template< class type > INLINE CompactList< type > & CompactList< type >::operator =( const CompactList< type > & other ) {
	Alloc( other.d_num );
	for( int i = 0; i < d_num; i++ )
		d_list[ i ] = other.d_list[ i ];
}

template< class type > INLINE type * CompactList< type >::GetPtr( ) {
	return d_list;
}

// // //// // //// // //// //
/// WindowPose
// //// // //// // //// //
class WindowPose {
private:
	Mat2									d_rotation;
	Vec2									d_translation;
	float									d_duration;
	float									d_alpha;

public:
											WindowPose( );

	void									SetRotation( float degrees, float zoom = 1.0f );
	void									SetTranslation( const Vec2 & trans );
	void									SetDuration( float duration );
	void									SetAlpha( float val );

	const Mat2 &							GetRotation( ) const;
	const Vec2 &							GetTranslation( ) const;
	float									GetDuration( ) const;
	float									GetAlpha( ) const;
};

// // //// // //// // //// //
// WindowPose
// //// // //// // //// //
WindowPose::WindowPose( ) {
	d_rotation.Identity( );
	d_translation.Zero( );
	d_alpha = 0;
}

void WindowPose::SetRotation( float degrees, float zoom ) {
	Vec2 dsincos;
	SinCos( DEG2RAD( degrees ), dsincos[ 0 ], dsincos[ 1 ] );
	dsincos *= 1.0f / Max( zoom, 0.0f ); // clamp 0-x and get frac
	d_rotation = Mat2( dsincos[ 1 ], dsincos[ 1 ], dsincos[ 0 ], dsincos[ 0 ] );
}

INLINE void WindowPose::SetTranslation( const Vec2 & trans ) {
	d_translation = trans;
}

INLINE void WindowPose::SetDuration( float duration ) {
	d_duration = duration;
}

INLINE void WindowPose::SetAlpha( float val ) {
	d_alpha = val;
}

INLINE const Mat2 & WindowPose::GetRotation( ) const {
	return d_rotation;
}

INLINE const Vec2 & WindowPose::GetTranslation( ) const {
	return d_translation;
}

INLINE float WindowPose::GetDuration( ) const {
	return d_duration;
}

INLINE float WindowPose::GetAlpha( ) const {
	return d_alpha;
}

#define MAX_WINDOW_VERTS 32

// // //// // //// // //// //
/// WindowVertexCache
// //// // //// // //// //
class WindowVertexCache {
private:
	CAList< Vec2, MAX_WINDOW_VERTS >		d_coords;
	CAList< Vec2, MAX_WINDOW_VERTS >		d_texCoors;

public:
	void									SetNum( int num );
	int										Num( ) const;
	Vec2 *									GetCoords( );
	const Vec2 *							GetCoords( ) const;
	Vec2 *									GetTexCoords( );
	const Vec2 *							GetTexCoords( ) const;

	void									Lerp( const WindowVertexCache & current, const WindowVertexCache & next, float frac );
};

// // //// // //// // //// //
// WindowVertexCache
// //// // //// // //// //
INLINE void WindowVertexCache::SetNum( int num ) {
	d_coords.SetNum( num );
	d_texCoors.SetNum( num );
}

INLINE int WindowVertexCache::Num( ) const {
	return d_coords.Num( );
}

INLINE Vec2 * WindowVertexCache::GetCoords( ) {
	return d_coords.Ptr( );
}

INLINE const Vec2 * WindowVertexCache::GetCoords( ) const {
	return d_coords.Ptr( );
}

INLINE Vec2 * WindowVertexCache::GetTexCoords( ) {
	return d_texCoors.Ptr( );
}

INLINE const Vec2 * WindowVertexCache::GetTexCoords( ) const {
	return d_texCoors.Ptr( );
}

INLINE void WindowVertexCache::Lerp( const WindowVertexCache & current, const WindowVertexCache & next, float frac ) {
	assert( current.Num( ) == next.Num( ) );
	for( int i = 0; i < current.Num( ); i++ ) {
		d_coords[ i ].Lerp( current.d_coords[ i ], next.d_coords[ i ], frac );
		d_texCoors[ i ] = current.d_texCoors[ i ];
	}
}

// // //// // //// // //// //
/// WindowFrame
// //// // //// // //// //
class WindowFrame {
private:
	Bounds2D								d_bounds;
	CompactList< CAList< Plane2D, MAX_WINDOW_VERTS > >	d_segments;
	WindowVertexCache						d_vertexBase;

	void									GetSegment( int segment_num, Winding2D & winding ) const;
	void									GetPoints( Vec2 * coord_buff, int num ) const;
	void									InitVertexBase( );

public:
	int										NumPoints( ) const;
	void									GetTexCoords( Vec2 * coord_buff, int num ) const;
	void									GetVertices( const WindowPose & pose, Vec2 * coord_buff, int num ) const;

	void									SetupRect( const Bounds2D & bounds );

	bool									PointInside( const Vec2 & point ) const;
};

// // //// // //// // //// //
// WindowFrame
// //// // //// // //// //
INLINE void WindowFrame::GetSegment( int segment_num, Winding2D & winding ) const {
	assert( ( segment_num >= 0 ) && ( segment_num <= d_segments.Num( ) ) );
	winding.Clear( );
	winding.AddPoint( vec2_origin );
	winding.AddPoint( Vec2( 0.0f, 1.0f ) );
	winding.AddPoint( Vec2( 1.0f, 1.0f ) );
	winding.AddPoint( Vec2( 1.0f, 0.0f ) );
	for( int i = 0; i < d_segments[ segment_num ].Num( ); i++ )
		winding.ClipInPlace( d_segments[ segment_num ][ i ], 0.0f );
}

int WindowFrame::NumPoints( ) const {
	int sum = 0;
	Winding2D winding;
	for( int i = 0; i < d_segments.Num( ); i++ ) {
		GetSegment( i, winding );
		sum += winding.GetNumPoints( );
	}
	return sum;
}

void WindowFrame::GetPoints( Vec2 * coord_buff, int num ) const {
	int numLeft = num;
	Winding2D winding;
	for( int i = 0; i < d_segments.Num( ); i++ ) {
		GetSegment( i, winding );
		for( int j = 0; j < winding.GetNumPoints( ); j++ ) {
			assert( numLeft );
			if( numLeft ) {
				coord_buff[ num - numLeft ] = winding[ j ];
				numLeft--;
			} else return;
		}
	}
}

void WindowFrame::InitVertexBase( ) {
	int sumPoints = 0;
	Winding2D winding;
	for( int i = 0; i < d_segments.Num( ); i++ ) {
		GetSegment( i, winding );
		sumPoints += winding.GetNumPoints( );
	}
	d_vertexBase.SetNum( sumPoints );
	Vec2 * coord_buff = d_vertexBase.GetCoords( );
	Vec2 * texcoord_buff = d_vertexBase.GetTexCoords( );
	int numLeft = sumPoints;
	for( int i = 0; i < d_segments.Num( ); i++ ) {
		GetSegment( i, winding );
		for( int j = 0; j < winding.GetNumPoints( ); j++ ) {
			coord_buff[ sumPoints - numLeft ] = winding[ j ];
			texcoord_buff[ i ] = coord_buff[ i ] - d_bounds[ 0 ];
			texcoord_buff[ i ] /= ( d_bounds[ 1 ] - d_bounds[ 0 ] );
			numLeft--;
		}
	}
}

/*INLINE void WindowFrame::GetTexCoords( Vec2 * coord_buff, Vec2 * texcoord_buff, int num ) const {
	for( int i = 0; i < num; i++ ) {
		texcoord_buff[ i ] = coord_buff[ i ] - d_bounds[ 0 ];
		texcoord_buff[ i ] /= ( d_bounds[ 1 ] - d_bounds[ 0 ] );
	}
}

void WindowFrame::GetVertices( const WindowPose & pose, Vec2 * coord_buff, Vec2 * texcoord_buff, int num ) const {
	int numLeft = num;
	Winding2D winding;
	for( int i = 0; i < d_segments.Num( ); i++ ) {
		GetSegment( i, winding );
		for( int j = 0; j < winding.GetNumPoints( ); j++ ) {
			assert( numLeft );
			if( numLeft ) {
				coord_buff[ num - numLeft ] = winding[ j ];
				numLeft--;
			} else return;
		}
	}
}*/

void WindowFrame::SetupRect( const Bounds2D & bounds ) {
	d_segments.Alloc( 1 );
	d_segments[ 0 ].SetNum( 4 );
	Vec2 upperRight = bounds.GetUpperRight( );
	Vec2 lowerLeft = bounds.GetLowerLeft( );
	d_segments[ 0 ][ 0 ].FromPoints( upperRight, bounds[ 0 ], true );
	d_segments[ 0 ][ 1 ].FromPoints( bounds[ 0 ], lowerLeft, true );
	d_segments[ 0 ][ 2 ].FromPoints( bounds[ 1 ], upperRight, true );
	d_segments[ 0 ][ 3 ].FromPoints( lowerLeft, bounds[ 1 ], true );
	d_bounds = bounds;
}

INLINE bool WindowFrame::PointInside( const Vec2 & point ) const {
	if( d_bounds.ContainsPoint( point ) ) {
		for( int i = 0, j; i < d_segments.Num( ); i++ ) {
			for( j = 0; j < d_segments[ i ].Num( ); j++ )
				if( d_segments[ i ][ j ].Side( point ) == PLANESIDE_BACK )
					break;
			if( d_segments[ i ].Num( ) == j )
				return true;
		}
	}
	return false;
}

// // //// // //// // //// //
/// WindowBase
// //// // //// // //// //
class WindowBase : public NamedObject {
protected:
	bool								d_visible;
	WindowBase *						d_parent;
	List< WindowBase * >				d_childs;

	WindowFrame *						d_frame;
	WindowPose *						d_poseCurrent;
	WindowPose *						d_poseFurther;
	float								d_poseTimer;
};
#endif

unsigned int Renderer::Run( void * param ) {
	VisTreeNode *						visNode;
	VisTreeNode *						visNodeNext;
	GLShader *							lastShader;
	RenderSurfaceMap *					currentSurface, * lastSurface = NULL;
	bool								lastIndex;
	GLuint								startIndex;
	GLuint								numPrimitives;
	Timer								fpsZeroTimer;
	CAList< Timer, 256 >				d_fpsPerfTable;
	CAList< Timer, 256 >				d_fpsPerfTableMax;
	CAList< float, 256 >				d_cpuPerfTable;
	float								d_fpsDisplayValue = 0;
	float								d_fpsDisplayMaxValue = 0;
	float								d_cpuDisplayValue = 0;
	bool								d_fpsPerfTableIsFull = false;
	int									d_fpsPerfTableIndex;

	//WindowFrame aFrame;
	//Vec2 aFramePoints[ 4 ];
	//Vec2 aFrameTexCoords[ 4 ];
	//aFrame.SetupRect( Bounds2D( Vec2( 0.1f, 0.1f ), Vec2( 0.3f, 0.4f ) ) );
	//bool ret = aFrame.PointInside( vec2_origin );
	//ret = aFrame.PointInside( Vec2( 0.3f, 0.4f ) );
	//ret = aFrame.PointInside( Vec2( 0.4f, 0.4f ) );
	//aFrame.GetPoints( aFramePoints, 4 );
	//aFrame.GetTexCoords( aFramePoints, aFrameTexCoords, 4 );

	GLScopedContext		threadContext;
	d_rendererThreadParams.d_context = threadContext;
	//GLBindingCheck bCheck;	
	//bCheck.AllCheck( );

	d_signalEnd.Set( );
	d_signalStart.Wait( );

	d_rendererThreadParams.d_mapBufferAllocator		= g_mapBufferAllocator->ContextCreate( );
	d_rendererThreadParams.d_modelBufferAllocator	= g_modelBufferAllocator->ContextCreate( );
	d_rendererThreadParams.d_uiBufferAllocator		= new BufferAllocatorUI( );
	BufferPrimitiveUI * deferredTextPrimitive		= d_rendererThreadParams.d_uiBufferAllocator->TexturePrimitive( Bounds2D( 0, 0, ( float )Video::viddef.width, ( float )Video::viddef.height ), false );

	GLFrameBuffer * d_frameBufferSimple = new GLFrameBuffer( "Deferred", Video::GetResolution( ), true, 0 );
	d_frameBufferSimple->RenderSetup( );
	GLFrameBuffer * d_deferredUtil = new GLFrameBuffer( "DeferredUtil", Video::GetResolution( ), true, 0 );
	d_deferredUtil->SingleSetup( "c0,rgba8", "extra" );
	GLFrameBuffer * d_shadowMapStatic = new GLFrameBuffer( "StaticShadowMap", GetSMStaticSize( ), true, 0 );
	d_shadowMapStatic->SingleSetup( "d", "depth" );
	GLFrameBuffer * d_shadowMapDynamic = new GLFrameBuffer( "DynamicShadowMap", GetSMDynamicSize( ), true, 0 );
	d_shadowMapDynamic->SingleSetup( "d", "depth" );
	GLShader * shaderLighting				= new GLShader( "shaders/lighting.csp" );
	GLShader * shaderSSAO					= new GLShader( "shaders/ssao.csp" );
	GLShader * shaderFSAA					= new GLShader( "shaders/fsaa.csp" );
	GLShader * shaderFast					= new GLShader( "shaders/fast.csp" );
	GLShader * shaderShadowBlend			= new GLShader( "shaders/shadow_blend.csp" );
	GLShader * shaderDepthDraw				= new GLShader( "shaders/depth_draw.csp" );
	GLShader * shaderUI						= new GLShader( "shaders/ui.csp" );
	GLShader * shaderDebug					= new GLShader( "shaders/debug.csp" );
	BufferPrimitiveUI * sunDepthPrimitive	= d_rendererThreadParams.d_uiBufferAllocator->TexturePrimitive( Bounds2D( 448, 12, 448+128, 12+128 ), false );
	Projection sunDepthProjection;

	GLTexture perlinTexture( GL_TEXTURE_2D );
	PerlinNoise< 1024, float > noiseGenerator;
	noiseGenerator.FillTexture( &perlinTexture );
	
	//SVar & sbsExtraTexture	= shaderShadowBlend->FindInput( "extraTexture" );
	shaderLighting->FindInput( "diffuseTexture" )	= d_frameBufferSimple->GetTextureBuffer( "diffuse" );
	shaderLighting->FindInput( "normalTexture" )	= d_frameBufferSimple->GetTextureBuffer( "normal" );
	shaderLighting->FindInput( "coordTexture" )		= d_frameBufferSimple->GetTextureBuffer( "coord" );
	//shaderLighting->FindInput( "depthTexture" )		= d_shadowMapStatic->GetTextureBuffer( "depth" );
	//shaderLighting->FindInput( "depthTexture2" )	= d_shadowMapDynamic->GetTextureBuffer( "depth" );
	//shaderLighting->FindInput( "perlinTexture" )	= &perlinTexture;
	shaderLighting->FindInput( "utilTexture" )		= d_deferredUtil->GetTextureBuffer( "extra" );
	shaderSSAO->FindInput( "normalTexture" )		= d_frameBufferSimple->GetTextureBuffer( "normal" );
	shaderSSAO->FindInput( "coordTexture" )			= d_frameBufferSimple->GetTextureBuffer( "coord" );
	shaderFSAA->FindInput( "normalTexture" )		= d_frameBufferSimple->GetTextureBuffer( "normal" );
	shaderFSAA->FindInput( "coordTexture" )			= d_frameBufferSimple->GetTextureBuffer( "coord" );
	//shaderSSAO->FindInput( "perlinTexture" )		= &perlinTexture;
	shaderShadowBlend->FindInput( "depthTexture" )	= d_shadowMapStatic->GetTextureBuffer( "depth" );
	shaderShadowBlend->FindInput( "depthTexture2" )	= d_shadowMapDynamic->GetTextureBuffer( "depth" );
	shaderShadowBlend->FindInput( "coordTexture" )	= d_frameBufferSimple->GetTextureBuffer( "coord" );

	//d_signalStart.Wait( );
	d_signalStart.Set( );

	while( 1 ) {
		d_signalStart.Wait( );
		if( Terminating( ) )
			break;

		d_fpsPerfTableIndex = d_fpsPerfTable.Num( );
		if( d_fpsPerfTableIndex )
			d_fpsPerfTable[ d_fpsPerfTableIndex - 1 ].Stop( );
		if( d_fpsPerfTableIndex == 256 ) {
			d_fpsPerfTableMax.RemoveIndexMemcpy( 0 );
			d_fpsPerfTable.RemoveIndexMemcpy( 0 );
			d_fpsPerfTableIndex--;
			d_fpsPerfTableIsFull = true;
		}
		d_fpsPerfTableMax.Append( fpsZeroTimer );
		d_fpsPerfTable.Append( fpsZeroTimer );
		d_fpsPerfTableMax[ d_fpsPerfTableIndex ].Start( );
		d_fpsPerfTable[ d_fpsPerfTableIndex ].Start( );

		d_rendererThreadParams.d_scene.CalculateProjection( );
		g_mvpMatrix		= d_rendererThreadParams.d_scene.d_viewProjectionMatrix;
		g_normalMatrix	= d_rendererThreadParams.d_scene.d_viewMatrix.Inverse( ).Transpose( );
		g_viewMatrix	= d_rendererThreadParams.d_scene.d_viewMatrix;

		GUIThread::SingleDoWork( );

		d_rendererThreadParams.ClearCounters( );
		if( g_pvsLock.IsModified( ) )
			UpdatePvsLock( );
		if( !d_rendererThreadParams.d_pvsLock ) {
			d_rendererThreadParams.d_scene.d_viewProjectionMatrix.ToInvPlanes( Bounds2D( -1.0f, -1.0f, 1.0f, 1.0f ), d_rendererThreadParams.d_pvsPlanes.Ptr( ) );
			ClearBrushPVSRef( );
			d_rendererThreadParams.d_visTree.Clear( );	
			d_rendererThreadParams.d_timerPVS.Start( );
			//// ->
			g_pvsTree.GetPVS( d_rendererThreadParams.d_pvsPlanes, d_rendererThreadParams.d_visTree, 0 );
			//// <-
			d_rendererThreadParams.d_timerPVS.Stop( );
		}

		GUIThread::SingleWait( );
		//bCheck.AllCheck( );
		g_uiMatrix = GUIThread::GetProjection( );
		{
			GLScopedLock glLock;
			//bCheck.AllCheck( );
			d_rendererThreadParams.d_timerFrame.Start( );
			//////// ->
			d_frameBufferSimple->Bind( );
			d_frameBufferSimple->AdjustViewport( );
			//GLImports::glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
			GLImports::glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
			GLImports::glEnable( GL_DEPTH_TEST );
			GLImports::glEnable( GL_CULL_FACE );
			GLImports::glCullFace( GL_FRONT );
			visNode = d_rendererThreadParams.d_visTree.Begin( );
			GLScopedQuery< GL_PRIMITIVES_GENERATED > primQuery;
			primQuery.Begin( );
			d_rendererThreadParams.d_timerRender.Start( );
			//// ->
			if( visNode ) {
				lastShader = d_rendererThreadParams.TranslatorLookup( visNode->d_object->GetShader( ) );
				lastSurface = NULL;
				startIndex = visNode->d_object->BaseIndex( );
				numPrimitives = 0;
				//bCheck.AllCheck( );
				d_rendererThreadParams.d_mapBufferAllocator->BeginDraw( );
				while( visNode ) {
					visNodeNext = d_rendererThreadParams.d_visTree.GetNextLeaf( visNode );
					lastIndex = !visNodeNext;
					currentSurface = visNode->d_object;
					if( lastSurface && ( lastIndex || !( currentSurface->RenderableWith( lastSurface ) && ( ( lastSurface->BaseIndex( ) + lastSurface->IndexNum( ) ) == currentSurface->BaseIndex( ) )) ) ) {

						lastShader = d_rendererThreadParams.TranslatorLookup( lastSurface->GetShader( ) );
						lastSurface->UpdateShaderParams( lastShader );
						lastShader->Bind( );
						lastShader->WriteSVars( );					
						GLImports::glDrawElementsInstanced( GL_TRIANGLES, numPrimitives, GL_UNSIGNED_INT, ( GLvoid * )NumSizeOf( GLuint, startIndex ), 1 );
						lastShader->UnBind( );
						startIndex = currentSurface->BaseIndex( );
						numPrimitives = 0;
						d_rendererThreadParams.d_indexUploadLimitCounter++;
					}
					numPrimitives += currentSurface->IndexNum( );
					lastSurface = currentSurface;
					visNode = visNodeNext;
				}
				d_rendererThreadParams.d_mapBufferAllocator->EndDraw( );
			}
			d_rendererThreadParams.d_modelBufferAllocator->BeginDraw( );
			for( int i = 0; i < d_rendererThreadParams.d_scene.entities.Num( ); i++ ) {
				SceneEntity & ent = d_rendererThreadParams.d_scene.entities[ i ];
				if( !ent.model )
					continue;
				RenderSurfaceModel * modelSurface = ent.model->GetSurface( );
				g_mvpMatrix		= d_rendererThreadParams.d_scene.d_viewProjectionMatrix.Translate( ent.origin ) * Angles( ent.angles ).ToMat4( ).Transpose( );
				g_normalMatrix	= ( d_rendererThreadParams.d_scene.d_viewMatrix.Translate( ent.origin ) * Angles( ent.angles ).ToMat4( ).Transpose( ) ).Inverse( ).Transpose( );
				GLShader * modelShader = d_rendererThreadParams.TranslatorLookup( modelSurface->GetShader( ) );
				modelSurface->UpdateShaderParams( modelShader );
				modelShader->Bind( );
				modelShader->WriteSVars( );				
				GLImports::glDrawElementsInstanced( GL_TRIANGLES, modelSurface->IndexNum( ), GL_UNSIGNED_INT, ( GLvoid * )NumSizeOf( GLuint, modelSurface->BaseIndex( ) ), 1 );
				modelShader->UnBind( );
			}
			d_rendererThreadParams.d_modelBufferAllocator->EndDraw( );
			//// <-
			d_rendererThreadParams.d_timerRender.Stop( );
			primQuery.End( );
			d_rendererThreadParams.d_primitiveCountBrush = primQuery.GetResultInt( );
			GLImports::glDisable( GL_DEPTH_TEST );
			GLImports::glDisable( GL_CULL_FACE );
			d_frameBufferSimple->UnBind( );
			//GLImports::GLErrorCheck( );
			//////// <-
			d_rendererThreadParams.d_timerFrame.Stop( );
			//Str iRate, oRate;
			//iRate.BestUnit( Client::CL_GetMessageChannel( ).GetIncomingRate( ), MEASURE_BANDWIDTH );
			//oRate.BestUnit( Client::CL_GetMessageChannel( ).GetOutgoingRate( ), MEASURE_BANDWIDTH );
			//GUIThread::DebugPrint( "NET      input    output\n" );
			//GUIThread::DebugPrint( "loss %9.0f\n", Client::CL_GetMessageChannel( ).GetIncomingPacketLoss( ) );
			//GUIThread::DebugPrint( "rate %9s %9s\n", iRate.c_str( ), oRate.c_str( ) );
			//GUIThread::DebugPrint( "comp %7.2f %% %7.2f %%\n", Client::CL_GetMessageChannel( ).GetIncomingCompression( ), Client::CL_GetMessageChannel( ).GetOutgoingCompression( ) );
			//GUIThread::DebugPrint( "seq  %9i %9i\n", Client::CL_GetMessageChannel( ).GetIncomingSequence( ), Client::CL_GetMessageChannel( ).GetOutgoingSequence( ) );
			GUIThread::DebugPrint( "FPS: %.02f PMax: %.02f\nCPU: %.02f Ents: %i\n", d_fpsDisplayValue, d_fpsDisplayMaxValue, d_cpuDisplayValue, d_rendererThreadParams.d_scene.entities.Num( ) );	
			GUIThread::DebugPrint( "PVS:%.2f F:%.2f\nI:%i R:%i\n", d_rendererThreadParams.d_timerPVS.ClockTicks( ), d_rendererThreadParams.d_timerFrame.ClockTicks( ), d_rendererThreadParams.d_indexUploadLimitCounter, d_rendererThreadParams.d_primitiveCountBrush );
			if( g_pvsMark.GetString( ).Length( ) )
				GUIThread::DebugPrint( "%s\n", g_pvsMark.GetString( ).c_str( ) );


			if( g_brushMark.IsModified( ) )
				UpdateBrushMark( );
			if( g_pvsMark.IsModified( ) )
				UpdatePvsMark( );
			if( d_rendererThreadParams.d_debugPrimitives.Num( ) ) {
				shaderDebug->Bind( );
				for( int j = 0; j < d_rendererThreadParams.d_debugPrimitives.Num( ); j++ )
					DebugDrawPrimitive( shaderDebug, d_rendererThreadParams.d_debugPrimitives[ j ], g_debugColor );
				shaderDebug->UnBind( );
			}
			if( g_shadowEnable.GetBool( ) ) {
				if( g_shadowMapStaticSize.IsModified( ) ) {
					d_shadowMapStatic->ReSize( GetSMStaticSize( ) );
					g_shadowMapStaticSize.ClearModified( );
					g_sunRedraw = true;
				}		
				GLImports::glEnable( GL_DEPTH_TEST );
				GLImports::glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
				shaderFast->Bind( );
				if( g_sunRedraw ) {
					d_shadowMapStatic->Bind( );
					d_shadowMapStatic->AdjustViewport( );
					g_sunRedraw = false;
					GLImports::glClear( GL_DEPTH_BUFFER_BIT );
					sunDepthProjection.OrthoLightSelf( 5000 );
					sunDepthProjection *= g_sunNormal.ToMat4( );
					sunDepthProjection *= GetCoordSystemRotation( );
					sunDepthProjection.TranslateSelf( -g_sunPosition );
					g_mvpMatrix = sunDepthProjection;
					shaderFast->WriteSVars( );			
					d_rendererThreadParams.d_mapBufferAllocator->BeginDraw( );
					GLImports::glDrawElementsInstanced( GL_TRIANGLES, d_rendererThreadParams.d_mapBufferAllocator->NumIndex( ), GL_UNSIGNED_INT, NULL, 1 );
					d_rendererThreadParams.d_mapBufferAllocator->EndDraw( );
					d_shadowMapStatic->UnBind( );
				}
				if( g_shadowMapDynamicSize.IsModified( ) ) {
					d_shadowMapDynamic->ReSize( GetSMDynamicSize( ) );
					g_shadowMapDynamicSize.ClearModified( );
				}
				d_shadowMapDynamic->Bind( );
				d_shadowMapDynamic->AdjustViewport( );
				GLImports::glClear( GL_DEPTH_BUFFER_BIT );
				d_rendererThreadParams.d_modelBufferAllocator->BeginDraw( );
				for( int i = 0; i < d_rendererThreadParams.d_scene.entities.Num( ); i++ ) {
					SceneEntity & ent = d_rendererThreadParams.d_scene.entities[ i ];
					if( !ent.model || ent.flags & RF_WEAPONMODEL )
						continue;
					RenderSurfaceModel * modelSurface = ent.model->GetSurface( );
					g_mvpMatrix = sunDepthProjection.Translate( ent.origin ) * Angles( ent.angles ).ToMat4( ).Transpose( );
					shaderFast->WriteSVars( );					
					GLImports::glDrawElementsInstanced( GL_TRIANGLES, modelSurface->IndexNum( ), GL_UNSIGNED_INT, ( GLvoid * )NumSizeOf( GLuint, modelSurface->BaseIndex( ) ), 1 );
				}
				d_rendererThreadParams.d_modelBufferAllocator->EndDraw( );

				shaderFast->UnBind( );
				GLImports::glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
				GLImports::glDisable( GL_DEPTH_TEST );
				d_shadowMapDynamic->UnBind( );
			}
			//d_frameBufferSimple->AdjustViewport( );

			//d_deferredUtil->Bind( );			
			d_deferredUtil->AdjustViewport( );
			//shaderShadowCombine->Bind( );
			//g_mvpMatrix			= mat4_bias * sunDepthProjection;
			//shaderShadowCombine->WriteSVars( );
			//d_uiBufferAllocator->BeginDraw( );
			//deferredTextPrimitive->Draw( 1 );
			//d_uiBufferAllocator->EndDraw( );
			//shaderShadowCombine->UnBind( );
			//d_deferredUtil->UnBind( );

			d_deferredUtil->Bind( );
			GLImports::glClear( GL_COLOR_BUFFER_BIT );
			if( g_shadowEnable.GetBool( ) ) {
				GLImports::glColorMask( GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE );
				shaderShadowBlend->Bind( );
				//GLImports::glEnable( GL_BLEND );
				//GLImports::glBlendFuncSeparate( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE );
				g_mvpMatrix			= mat4_bias * sunDepthProjection;
				shaderShadowBlend->WriteSVars( );
				d_rendererThreadParams.d_uiBufferAllocator->BeginDraw( );
				deferredTextPrimitive->Draw( 1 );
				d_rendererThreadParams.d_uiBufferAllocator->EndDraw( );
				//GLImports::glDisable( GL_BLEND );
				shaderShadowBlend->UnBind( );
			}
			g_cameraOrigin	= d_rendererThreadParams.d_scene.vieworg;
			g_normalMatrix	= d_rendererThreadParams.d_scene.d_viewMatrix.Inverse( ).Transpose( );
			g_viewMatrix	= d_rendererThreadParams.d_scene.d_viewMatrix;
			if( g_ssaoEnable.GetBool( ) ) {
				GLImports::glColorMask( GL_FALSE, GL_TRUE, GL_FALSE, GL_FALSE );
				shaderSSAO->Bind( );
				shaderSSAO->WriteSVars( );
				d_rendererThreadParams.d_uiBufferAllocator->BeginDraw( );
				deferredTextPrimitive->Draw( 1 );
				d_rendererThreadParams.d_uiBufferAllocator->EndDraw( );
				shaderSSAO->UnBind( );				
			}
			if( g_fsaaEnable.GetBool( ) ) {
				GLImports::glColorMask( GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE );
				shaderFSAA->Bind( );
				shaderFSAA->WriteSVars( );
				d_rendererThreadParams.d_uiBufferAllocator->BeginDraw( );
				deferredTextPrimitive->Draw( 1 );
				d_rendererThreadParams.d_uiBufferAllocator->EndDraw( );
				shaderFSAA->UnBind( );				
			}
			GLImports::glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
			d_deferredUtil->UnBind( );
			
			g_lightOrigin	= g_sunPosition;
			shaderLighting->Bind( );
			shaderLighting->WriteSVars( );
			d_rendererThreadParams.d_uiBufferAllocator->BeginDraw( );
			deferredTextPrimitive->Draw( 1 );
			d_rendererThreadParams.d_uiBufferAllocator->EndDraw( );
			shaderLighting->UnBind( );

			switch( g_utilDraw.GetInt( ) ) {
				case 1: d_deferredUtil->ReadBind( ); d_deferredUtil->DrawAttachment( 0, 1.0f / 64.0f, 1.0f / 8.0f, 0 ); d_deferredUtil->UnBind( ); break;
				case 2: d_deferredUtil->Blit( ); break;
			}

			if( 0 ) {
				shaderDepthDraw->Bind( );
				shaderDepthDraw->FindInput( "depthTexture" ) = d_shadowMapStatic->GetTextureBuffer( "depth" );
				shaderDepthDraw->WriteSVars( );
				d_rendererThreadParams.d_uiBufferAllocator->BeginDraw( );
				sunDepthPrimitive->Draw( 1 );
				d_rendererThreadParams.d_uiBufferAllocator->EndDraw( );
				shaderDepthDraw->UnBind( );
			}
			//GUIThread::SingleDoWork( );
			//GUIThread::SingleWait( );
			GLImports::glEnable( GL_BLEND );
			GLImports::glBlendFuncSeparate( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE );
			shaderUI->FindInput( "uiTexture" ) = GUIThread::GetTexture( );
			shaderUI->Bind( );
			shaderUI->WriteSVars( );
			d_rendererThreadParams.d_uiBufferAllocator->BeginDraw( );
			deferredTextPrimitive->Draw( 1 );
			d_rendererThreadParams.d_uiBufferAllocator->EndDraw( );
			shaderUI->UnBind( );
			GLImports::glDisable( GL_BLEND );
			if( g_utilDraw.GetInt( ) == 3 ) {
				d_frameBufferSimple->ReadBind( );
				//d_frameBufferSimple->DrawAttachment( 0, 1.0f / 64.0f, 1.0f / 8.0f, 2 );
				d_frameBufferSimple->DrawAttachment( 0, 1.0f / 64.0f, 1.0f / 8.0f, 0 );
				d_frameBufferSimple->DrawAttachment( 1, 1.0f / 64.0f, 1.0f / 8.0f, 1 );
				d_frameBufferSimple->DrawAttachment( 2, 1.0f / 64.0f, 1.0f / 8.0f, 2 );
				d_frameBufferSimple->UnBind( );
			}
			//d_overlay->GetFrameBuffer( )->DrawAttachmentDebug( 0 );
		} //GLScopedLock

		GLImports::GLErrorCheck( );
		d_rendererThreadParams.d_context.SwapBuffers( );

		if( d_rendererThreadParams.d_takeScreenShot ) {
			d_rendererThreadParams.d_takeScreenShot = false;
			Image * screenShot = new Image( );
			GLImports::glReadPixels( 0, 0, d_rendererThreadParams.d_scene.width, d_rendererThreadParams.d_scene.height, GL_RGB, GL_UNSIGNED_BYTE, screenShot->Alloc( d_rendererThreadParams.d_scene.width, d_rendererThreadParams.d_scene.height, false ) );
			GLImports::glFinish( );
			new ScreenShotThread( d_rendererThreadParams.d_nameScreenShot, screenShot );
		}

		d_cpuDisplayValue = d_fpsDisplayMaxValue = d_fpsDisplayValue = 0;
		d_fpsPerfTableMax[ d_fpsPerfTableIndex ].Stop( );
		if( d_fpsPerfTableIsFull )
			d_fpsPerfTableIndex++;
		for( int i = 0; i < d_fpsPerfTableIndex; i++ ) {
			d_fpsDisplayMaxValue += d_fpsPerfTableMax[ i ].Milliseconds( );
			if( d_fpsPerfTableIsFull && i == 255 )
				break;
			d_fpsDisplayValue += d_fpsPerfTable[ i ].Milliseconds( );
		}
		if( d_fpsPerfTableIndex ) {
			d_fpsDisplayMaxValue /= d_fpsPerfTableIndex; d_fpsDisplayMaxValue = 1000.0f / d_fpsDisplayMaxValue;
			d_fpsDisplayValue /= d_fpsPerfTableIndex; d_fpsDisplayValue = 1000.0f / d_fpsDisplayValue;
		}
		if( d_cpuPerfTable.Num( ) == 256 )
			d_cpuPerfTable.RemoveIndexMemcpy( 0 );
		d_cpuPerfTable.Append( System::CpuUsage( ) );
		for( int i = 0; i < d_cpuPerfTable.Num( ); i++ )
			d_cpuDisplayValue += d_cpuPerfTable[ i ];
		d_cpuDisplayValue /= d_cpuPerfTable.Num( );

		d_signalEnd.Set( );
	}

	delete d_deferredUtil;
	delete d_shadowMapStatic;
	delete d_shadowMapDynamic;
	delete shaderLighting;
	delete shaderSSAO;
	delete shaderFSAA;
	delete shaderFast;
	delete shaderShadowBlend;
	delete shaderDepthDraw;
	delete shaderUI;
	delete shaderDebug;
	delete d_frameBufferSimple;
	delete d_rendererThreadParams.d_uiBufferAllocator;
	for( int i = 0; i < d_rendererThreadParams.d_shaderTranslator.Num( ); i++ )
		delete d_rendererThreadParams.d_shaderTranslator[ i ].val2;

	return 0;
}
