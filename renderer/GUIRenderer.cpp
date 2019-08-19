#include "precompiled.h"
#pragma hdrstop

namespace CEGUI {

// // //// // //// // //// //
// String
// //// // //// // //// //

const String::size_type String::npos = ( String::size_type )( -1 );

String::~String( ) {
	if( d_reserve > STR_QUICKBUFF_SIZE )
		g_stringDataAllocator.Free( ( char * )d_buffer );
	if( d_encodedbufflen > 0 )
		g_stringDataAllocator.Free( ( char * )d_encodedbuff );
}

bool String::grow( size_type new_size ) {
	// check for too big
	if( max_size( ) <= new_size )
		std::length_error( "Resulting CEGUI::String would be too big" );
	new_size++; // increase, as we always null-terminate the buffer.
	if( new_size > d_reserve ) {
		utf32 * temp = ( utf32 * )g_stringDataAllocator.Alloc( ( int )( new_size << 2 ) );
		if( d_reserve > STR_QUICKBUFF_SIZE ) {
			Common::Com_Memcpy(temp, d_buffer, ( d_cplength + 1 ) * sizeof( utf32 ) );
			g_stringDataAllocator.Free( ( char * )d_buffer );
		} else
			Common::Com_Memcpy( temp, d_quickbuff, ( d_cplength + 1 ) * sizeof( utf32 ) );
		d_buffer = temp;
		d_reserve = new_size;
		return true;
	}
	return false;
}

void String::trim( ) {
	size_type min_size = d_cplength + 1;
	if( ( d_reserve > STR_QUICKBUFF_SIZE ) && ( d_reserve > min_size ) ) { // only re-allocate when not using quick-buffer, and when size can be trimmed
		if( min_size <= STR_QUICKBUFF_SIZE ) { // see if we can trim to quick-buffer
			Common::Com_Memcpy( d_quickbuff, d_buffer, min_size * sizeof( utf32 ) );
			g_stringDataAllocator.Free( ( char * )d_buffer );
			d_reserve = STR_QUICKBUFF_SIZE;
		} else { // re-allocate buffer
			utf32 * temp = ( utf32 * )g_stringDataAllocator.Alloc( ( int )( min_size << 2 ) );
			Common::Com_Memcpy( temp, d_buffer, min_size * sizeof( utf32 ) );
			g_stringDataAllocator.Free( ( char * )d_buffer );
			d_buffer = temp;
			d_reserve = min_size;
		}
	}
}

utf8 * String::build_utf8_buff( ) const {
	size_type buffsize = encoded_size( ptr( ), d_cplength ) + 1;
	if( buffsize > d_encodedbufflen ) {
		if( d_encodedbufflen > 0 )
			g_stringDataAllocator.Free( ( char * )d_encodedbuff );
		d_encodedbuff = ( utf8 * )g_stringDataAllocator.Alloc( ( int )buffsize );
		d_encodedbufflen = buffsize;
	}
	encode( ptr( ), d_encodedbuff, buffsize, d_cplength );
	d_encodedbuff[ buffsize - 1 ] = ( utf8 )0; // always add a null at end
	d_encodeddatlen = buffsize;
	return d_encodedbuff;
}

// // //// // //// // //// //
// GUITexture
// //// // //// // //// //

GUITexture::GUITexture( ) {
}

GUITexture::~GUITexture( ) {
}

const Size & GUITexture::getSize( ) const {
	return d_size;
}

const Size & GUITexture::getOriginalDataSize( ) const {
	return d_dataSize;
}

const Vector2 & GUITexture::getTexelScaling( ) const {
	return d_texelScaling;
}

void GUITexture::loadFromFile( const String & filename, const String & resourceGroup ) {
	Str fName = resourceGroup.c_str( );
	fName += filename.c_str( );
	*( DynamicTexture * )this = *GUIThread::GetOverlay( )->LoadFromFile( fName );	
	d_size.d_width = ( float )GetSize( )[ 0 ];
	d_size.d_height = ( float )GetSize( )[ 1 ];
	d_dataSize = d_size;
	d_texelScaling = Vector2( 1.0f / d_dataSize.d_width, 1.0f / d_dataSize.d_height );
}

void GUITexture::loadFromMemory( const void * buffer, const Size & buffer_size, PixelFormat pixel_format ) {
	::Image * loadImage = new ::Image( );
	size_t pixel_size = pixel_format == PF_RGBA ? 4 : 3;
	loadImage->LoadRaw( buffer, ( int )( buffer_size.d_width * buffer_size.d_height ) * pixel_size, pixel_format == PF_RGBA ? "rgba" : "rgb", VecT2i( ( int )buffer_size.d_width, ( int )buffer_size.d_height ), false );
	*( DynamicTexture * )this = *GUIThread::GetOverlay( )->LoadFromImage( loadImage );
	d_size.d_width = ( float )GetSize( )[ 0 ];
	d_size.d_height = ( float )GetSize( )[ 1 ];
	d_dataSize = buffer_size;
	d_texelScaling = Vector2( 1.0f / d_dataSize.d_width, 1.0f / d_dataSize.d_height );
	delete loadImage;
}

void GUITexture::saveToMemory( void * buffer ) {
	return;
}

// // //// // //// // //// //
// GUIGeometryBuffer
// //// // //// // //// //

GUIGeometryBuffer::GUIGeometryBuffer( ) : d_activeTexture(NULL), d_clipRect(0,0,0,0), d_translation(0,0,0), d_rotation(0,0,0), d_pivot(0,0,0), d_effect(NULL) {	
	d_matrixRecompute = true;
}

GUIGeometryBuffer::~GUIGeometryBuffer( ) {
}

void GUIGeometryBuffer::draw( ) const {
	if( !d_activeTexture || !d_indexRangeList.Num( ) )
		return; // Initial state
	int pass_count = d_effect ? d_effect->getPassCount( ) : 1;
	for( int pass = 0; pass < pass_count; pass++ ) {
		// set up RenderEffect
		if( d_effect )
			d_effect->performPreRenderFunctions( pass );
	}
	GLShader * uiShader = GUIThread::GetShader( );
	if( d_matrixRecompute )
		recomputeMatrix( );
	uiShader->FindInput( "uiMatrix" ) = d_matrix;
	uiShader->FindInput( "uiTexture" ) = d_activeTexture;
	uiShader->Bind( );
	uiShader->WriteSVars( );
	for( int i = 0, drawOffset = d_indexRangeList[ 0 ]->d_offset, drawSize = d_indexRangeList[ 0 ]->d_size; ; i++ ) {
		bool lastBatch = ( i == ( d_indexRangeList.Num( ) - 1 ) );
		if( !i && !lastBatch )
			continue;
		else if( d_indexRangeList[ i ]->d_offset == ( drawOffset + NumSizeOf( BufferAllocatorUI::t_indexType, drawSize ) ) ) {
			drawSize += d_indexRangeList[ i ]->d_size;
			if( !lastBatch )
				continue;
		}
		if( drawSize ) {
			GUIThread::GetPrimitiveBuffer( )->BeginDraw( );
			GLImports::glDrawElementsInstanced( GL_TRIANGLES, drawSize, GLType< BufferAllocatorUI::t_indexType >::GetTypeEnum( ), ( GLvoid * )drawOffset, 1 );
			GUIThread::GetPrimitiveBuffer( )->EndDraw( );
		}
		if( lastBatch )
			break;
		drawOffset = d_indexRangeList[ i ]->d_offset;
		drawSize = d_indexRangeList[ i ]->d_size;
	}
	uiShader->UnBind( );
	// clean up RenderEffect
	if( d_effect ) d_effect->performPostRenderFunctions( );
}

void GUIGeometryBuffer::recomputeMatrix( ) const {
	d_matrix = Projection::Orthogonal( 0, ( float )Video::viddef.width, ( float )Video::viddef.height, 0, 0, 99 );
	d_matrix.TranslateSelf( d_translation + d_pivot );
	d_matrix *= Angles( d_rotation ).ToMat4( );
	d_matrix.TranslateSelf( -d_pivot );
	d_matrixRecompute = false;
}

void GUIGeometryBuffer::setTranslation( const Vector3 & v ) {	
	d_translation.Set( v.d_x, v.d_y, v.d_z );
	d_matrixRecompute = true;
}

void GUIGeometryBuffer::setRotation( const Vector3 & r ) {	
	d_rotation.Set( r.d_x, r.d_y, r.d_z );
	d_matrixRecompute = true;
}

void GUIGeometryBuffer::setPivot( const Vector3 & p ) {	
	d_pivot.Set( p.d_x, p.d_y, p.d_z );
	d_matrixRecompute = true;
}

void GUIGeometryBuffer::setClippingRegion( const Rect & region ) {	
	d_clipRect = region;
}

void GUIGeometryBuffer::appendVertex( const Vertex & vertex ) {	
	appendGeometry( &vertex, 1 );
}

void GUIGeometryBuffer::appendGeometry( const Vertex * const vbuff, uint vertex_count ) {
	bool vertOptimize = vertex_count == 6 && !memcmp( &vbuff[ 0 ], &vbuff[ 4 ], sizeof( Vertex ) ) && !memcmp( &vbuff[ 2 ], &vbuff[ 5 ], sizeof( Vertex ) );
	uint uploadCount = vertOptimize ? 4 : vertex_count;
	BufferAllocatorUI::t_vertexBlock * vertexBlock = GUIThread::GetPrimitiveBuffer( )->AllocVertex( uploadCount );
	d_vertexRangeList.Append( vertexBlock );	
	for( uint i = 0; i < uploadCount; i++ ) {
		( *vertexBlock )[ i ].xy.Set( vbuff[ i ].position.d_x, vbuff[ i ].position.d_y );
		( *vertexBlock )[ i ].st = d_activeTexture->RemapTexCoord( Vec2( vbuff[ i ].tex_coords.d_x, vbuff[ i ].tex_coords.d_y ) );
		( *vertexBlock )[ i ].color = Color( vbuff[ i ].colour_val.getARGB( ) );
	}
	GUIThread::GetPrimitiveBuffer( )->UploadVertex( vertexBlock );
	BufferAllocatorUI::t_indexBlock * indexBlock = GUIThread::GetPrimitiveBuffer( )->AllocIndex( vertOptimize ? 6 : uploadCount );
	d_indexRangeList.Append( indexBlock );
	uint startIndex = vertexBlock->d_offset / sizeof( VertexUI );
	for( uint i = 0; i < uploadCount; i++ )
		( *indexBlock )[ i ] = i + startIndex;
	if( vertOptimize ) {
		( *indexBlock )[ 4 ] = 0 + startIndex;
		( *indexBlock )[ 5 ] = 2 + startIndex;
	}
	GUIThread::GetPrimitiveBuffer( )->UploadIndex( indexBlock );
}

void GUIGeometryBuffer::setActiveTexture( Texture * texture ) {	
	d_activeTexture = ( GUITexture * )texture;
}

void GUIGeometryBuffer::reset( ) {
	for( int i = 0; i < d_vertexRangeList.Num( ); i++ )
		GUIThread::GetPrimitiveBuffer( )->FreeVertex( d_vertexRangeList[ i ] );
	for( int i = 0; i < d_indexRangeList.Num( ); i++ )
		GUIThread::GetPrimitiveBuffer( )->FreeIndex( d_indexRangeList[ i ] );
	d_vertexRangeList.Clear( );
	d_indexRangeList.Clear( );
}

Texture * GUIGeometryBuffer::getActiveTexture( ) const {
	return d_activeTexture;
}

uint GUIGeometryBuffer::getVertexCount( ) const {
	return 1;
}

uint GUIGeometryBuffer::getBatchCount( ) const {
	return 1;
}

void GUIGeometryBuffer::setRenderEffect( RenderEffect * effect ) {
	d_effect = effect;
}

RenderEffect * GUIGeometryBuffer::getRenderEffect( ) {
	return d_effect;
}

// // //// // //// // //// //
// GUIRenderTarget
// //// // //// // //// //

GUIRenderTarget::GUIRenderTarget( ) : d_area( 0, 0, ( float )Video::GetResolution( )[ 0 ], ( float )Video::GetResolution( )[ 1 ] ) {
	d_frameBuffer = new GLFrameBuffer( "GUIThreadFrameBuffer", Video::GetResolution( ), false, 0 );
	d_frameBuffer->SingleSetup( "c0,rgba8", "color" );
	d_ownCreation = true;
}

GUIRenderTarget::GUIRenderTarget( GLFrameBuffer * frame_buffer ) {
	d_area = Rect( 0, 0, ( float )frame_buffer->GetSize( )[ 0 ], ( float )frame_buffer->GetSize( )[ 1 ] );
	d_frameBuffer = frame_buffer;
	d_ownCreation = false;
}

GUIRenderTarget::~GUIRenderTarget( ) {
	if( d_ownCreation )
		delete d_frameBuffer;
}

void GUIRenderTarget::draw( const GeometryBuffer & buffer ) {	
	buffer.draw( );
}

void GUIRenderTarget::draw( const RenderQueue & queue ) {	
	queue.draw( );
}

void GUIRenderTarget::setArea( const Rect & area ) {
	assert( d_ownCreation );
	d_area = area;
	d_frameBuffer->ReSize( VecT2i( ( int )d_area.d_right, ( int )d_area.d_bottom ) );
}

const Rect & GUIRenderTarget::getArea( ) const {	
	return d_area;
}

void GUIRenderTarget::activate( ) {
	d_frameBuffer->Bind( );
	d_frameBuffer->AdjustViewport( );
}

void GUIRenderTarget::deactivate( ) {
	d_frameBuffer->UnBind( );
}

void GUIRenderTarget::unprojectPoint( const GeometryBuffer & buff, const Vector2 & p_in, Vector2 & p_out ) const {
	p_out = p_in;
}

bool GUIRenderTarget::isImageryCache( ) const {	
	return d_frameBuffer->GetTextureMode( );
}

// // //// // //// // //// //
// GUITextureTarget
// //// // //// // //// //

GUITextureTarget::GUITextureTarget( ) : d_CEGUITexture( NULL ) {
	GUIRenderer * renderer = ( GUIRenderer * )System::getSingletonPtr( )->getRenderer( );
	d_CEGUITexture = ( GUITexture * )&renderer->createTexture( Size( ( float )Video::viddef.width, ( float )Video::viddef.height ) );
	d_frameBuffer = new GLFrameBuffer( "guiTextureTargetFrameBuffer", Video::GetResolution( ), true, 0 );
	d_frameBuffer->SingleSetup( "c0,rgba8", "color" );
	*( ( GLTexture * )d_CEGUITexture ) = *d_frameBuffer->GetTextureBuffer( "color" );
	d_doClear = true;
}

GUITextureTarget::~GUITextureTarget( ) {
	GUIRenderer * renderer = ( GUIRenderer * )System::getSingletonPtr( )->getRenderer( );
	renderer->destroyTexture( *d_CEGUITexture );
	delete d_frameBuffer;
}

void GUITextureTarget::draw( const GeometryBuffer & buffer ) {	
	if( d_doClear ) {
		GLImports::glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
		GLImports::glClear( GL_COLOR_BUFFER_BIT );
		d_doClear = false;
	}
	GUIRenderTarget::draw( buffer );
}

void GUITextureTarget::draw( const RenderQueue & queue ) {	
	if( d_doClear ) {
		GLImports::glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
		GLImports::glClear( GL_COLOR_BUFFER_BIT );
		d_doClear = false;
	}
	GUIRenderTarget::draw( queue );
}

void GUITextureTarget::setArea( const Rect & area ) {	
	GUIRenderTarget::setArea( area );
}

const Rect & GUITextureTarget::getArea( ) const {	
	return GUIRenderTarget::getArea( );
}

void GUITextureTarget::activate( ) {	
	d_frameBuffer->Bind( );
}

void GUITextureTarget::deactivate( ) {	
	d_frameBuffer->UnBind( );
}

void GUITextureTarget::unprojectPoint( const GeometryBuffer & buff, const Vector2 & p_in, Vector2 & p_out ) const {	
	GUIRenderTarget::unprojectPoint( buff, p_in, p_out );
}

bool GUITextureTarget::isImageryCache( ) const {	
	return true;
}

void GUITextureTarget::clear( ) {
	d_doClear = true;
}

Texture & GUITextureTarget::getTexture( ) const {	
	return *d_CEGUITexture;
}

void GUITextureTarget::declareRenderSize( const Size & sz ) {	
	Rect r;
	r.setSize( sz );
	r.setPosition( Point( 0, 0 ) );
	GUIRenderTarget::setArea( r );
}

bool GUITextureTarget::isRenderingInverted( ) const {	
	return true;
}

// // //// // //// // //// //
// GUIFont
// //// // //// // //// //

GUIFont::GUIFont( const String & font_name, const float point_size ) :
	Font( font_name, Font_xmlHandler::FontTypeFreeType, font_name, "", false, ( float )Video::viddef.width, ( float )Video::viddef.height ) {

	d_renderer = new DWFontRenderer( font_name.c_str( ), point_size );
	setMaxCodepoint( d_renderer->GetCodepointMap( d_cp_map ) );
	d_ascender = ceil( d_renderer->GetAscender( ) );
	d_descender = ceil( d_renderer->GetDescender( ) );
	d_height = ceil( d_renderer->GetHeight( ) );
}

GUIFont::~GUIFont( ) {
	d_cp_map.clear( );
	delete d_renderer;
}

void GUIFont::rasterise( utf32 start_codepoint, utf32 end_codepoint ) const {
	d_renderer->Render( d_name, d_cp_map, start_codepoint, end_codepoint );
}

void GUIFont::updateFont( ) {
	return;
}

void GUIFont::writeXMLToStream_impl( XMLSerializer & xml_stream ) const {
	return;
}

// // //// // //// // //// //
// GUIRenderer
// //// // //// // //// //

GUIRenderer::GUIRenderer( ) {
	d_displaySize = Size( ( float )Video::viddef.width, ( float )Video::viddef.height );
	d_displayDPI = Vector2( 96, 96 );
	d_maxTextureSize = 2048;
	d_rendererID = "GUIRenderer";
	d_defaultTarget = new GUIRenderTarget( GUIThread::GetFrameBuffer( ) );
	//d_defaultTarget->setArea( Rect( Point( 0.0f, 0.0f ), d_displaySize ) );
	d_defaultRoot = new RenderingRoot( *d_defaultTarget );
}

GUIRenderer::~GUIRenderer( ) {
	delete d_defaultTarget;
	delete d_defaultRoot;
	d_textureTargets.DeleteContents( true );
	d_geometryBuffers.DeleteContents( true );
	d_textures.DeleteContents( true );
}

RenderingRoot & GUIRenderer::getDefaultRenderingRoot( ) {
	return *d_defaultRoot;
}

GeometryBuffer & GUIRenderer::createGeometryBuffer( ) {
	GUIGeometryBuffer * gb = new GUIGeometryBuffer( );
	d_geometryBuffers.Append( gb );
	return *gb;
}

void GUIRenderer::destroyGeometryBuffer( const GeometryBuffer & buffer ) {
	GUIGeometryBuffer * gb = ( GUIGeometryBuffer * )const_cast< GeometryBuffer * >( &buffer );
	d_geometryBuffers.Remove( gb );
	delete gb;
}

void GUIRenderer::destroyAllGeometryBuffers( ) {
	d_geometryBuffers.DeleteContents( true );
}

TextureTarget * GUIRenderer::createTextureTarget( ) {
	GUITextureTarget * tt = new GUITextureTarget( );
	d_textureTargets.Append( tt );
    return tt;
}

void GUIRenderer::destroyTextureTarget( TextureTarget * target ) {
	GUITextureTarget * tt = ( GUITextureTarget * )target;
	d_textureTargets.Remove( tt );
	delete tt;
}

void GUIRenderer::destroyAllTextureTargets( ) {
	d_textureTargets.DeleteContents( true );
}

Texture & GUIRenderer::createTexture( ) {
	GUITexture * t = new GUITexture( );
	d_textures.Append( t );
	return *t;
}

Texture & GUIRenderer::createTexture( const String & filename,  const String & resourceGroup ) {
	Texture * t = &createTexture( );
	t->loadFromFile( filename, resourceGroup );
	return *t;
}

Texture & GUIRenderer::createTexture( const Size & size ) {
	GUITexture * t = ( GUITexture * )&createTexture( );
	t->setAllSize( size );
	return *t;
}

void GUIRenderer::destroyTexture( Texture & texture ) {
	GUITexture * t = ( GUITexture * )const_cast< Texture * >( &texture );
	d_textures.Remove( t );
	delete t;
}

void GUIRenderer::destroyAllTextures( ) {
	d_textures.DeleteContents( true );
}

void GUIRenderer::beginRendering( ) {
	GLImports::glEnable( GL_BLEND );
	setupRenderingBlendMode( BM_NORMAL );
}

void GUIRenderer::endRendering( ) {
	GLImports::glDisable( GL_BLEND );
}

const Size & GUIRenderer::getDisplaySize( ) const {
    return d_displaySize;
}

const Vector2 & GUIRenderer::getDisplayDPI( ) const {
	return d_displayDPI;
}

uint GUIRenderer::getMaxTextureSize( ) const {	
	return d_maxTextureSize;
}

const String & GUIRenderer::getIdentifierString( ) const {	
	return d_rendererID;
}

void GUIRenderer::setDisplaySize( const Size & sz ) {
	if( sz != d_displaySize ) {
		d_displaySize = sz;
		// FIXME: This is probably not the right thing to do in all cases.
		Rect area( d_defaultTarget->getArea( ) );
		area.setSize( sz );
		d_defaultTarget->setArea( area );
	}
}

void GUIRenderer::setupRenderingBlendMode( const BlendMode mode ) {
	if( mode == BM_RTT_PREMULTIPLIED ) {
		GLImports::glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
	} else {
		GLImports::glBlendFuncSeparate( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE );
	}
}

}
