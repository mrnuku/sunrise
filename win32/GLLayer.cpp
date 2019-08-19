#include "precompiled.h"
#pragma hdrstop

CANamedList< GLFrameBuffer *, 128 >			GLFrameBuffer::g_fameBufferList;
CANamedList< GLShader *, 128 >				GLShader::g_shaderList;

// // //// // //// // //// //
// GLScopedContext
// //// // //// // //// //

GLScopedContext::GLScopedContext( ) : GLContext( GLOSLayer::CreateContext( true ) ) {
}

GLScopedContext::~GLScopedContext( ) {
	GLOSLayer::DeleteContext( *this );
}

// // //// // //// // //// //
// GLBindingCheck
// //// // //// // //// //

GLBindingCheck::GLBindingCheck( ) {	
	Common::Com_Memset( this, 0, sizeof( GLBindingCheck ) );
	GLImports::glGetIntegerv( GL_MAX_DRAW_BUFFERS, &d_maxDrawBuffers );
	GLImports::glGetIntegerv( GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &d_maxTextureUnits );
	AllCheck( );
}

void GLBindingCheck::GetDrawBuffers( ) {	
	for( int i = 0; i < d_maxDrawBuffers; i++ ) {
		if( i == ( sizeof( d_drawBuffers ) / sizeof( int ) ) )
			break;		
		GLImports::glGetIntegerv( GL_DRAW_BUFFER0 + i, ( int * )&d_drawBuffers[ i ] );
	}
}

void GLBindingCheck::GetFrameBuffers( ) {	
	GLImports::glGetIntegerv( GL_DRAW_FRAMEBUFFER_BINDING, &d_drawFrameBuffer );
	GLImports::glGetIntegerv( GL_READ_FRAMEBUFFER_BINDING, &d_readFrameBuffer );
}

void GLBindingCheck::GetRenderBuffer( ) {	
	GLImports::glGetIntegerv( GL_RENDERBUFFER_BINDING, &d_renderBuffer );
}

void GLBindingCheck::GetTexture( ) {	
	GLImports::glGetIntegerv( GL_ACTIVE_TEXTURE, ( int * )&d_textureUnit );
	GLImports::glGetIntegerv( GL_TEXTURE_BINDING_2D, &d_texture2D );
	GLImports::glGetIntegerv( GL_TEXTURE_BINDING_2D_MULTISAMPLE, &d_texture2DMS );
}

void GLBindingCheck::GetProgram( ) {	
	GLint usize;
	GLenum utype, lastTexUnit = d_textureUnit;
	GLImports::glGetIntegerv( GL_CURRENT_PROGRAM, &d_program );
	if( d_program ) {
		GLImports::glGetProgramiv( d_program, GL_ACTIVE_UNIFORMS, &d_programUniforms );
	} else {		
		d_programUniforms = 0;
		Common::Com_Memset( d_programSamplerBindings, 0xFF, sizeof( d_programSamplerBindings ) );
		Common::Com_Memset( d_programTextureBindings, 0xFF, sizeof( d_programTextureBindings ) );
	}

	for( int i = 0; i < d_programUniforms; i++ ) {
		if( i == ( sizeof( d_programSamplerBindings ) / sizeof( int ) ) )
			break;
		GLImports::glGetActiveUniform( d_program, i, 0, NULL, &usize, &utype, NULL );
		if( utype == GL_SAMPLER_2D || utype == GL_SAMPLER_2D_SHADOW ) {			
			GLImports::glGetUniformiv( d_program, i, &d_programSamplerBindings[ i ] );
			lastTexUnit = GL_TEXTURE0 + d_programSamplerBindings[ i ];
			GLImports::glActiveTexture( lastTexUnit );
			GLImports::glGetIntegerv( GL_TEXTURE_BINDING_2D, &d_programTextureBindings[ i ] );
		} else
			d_programTextureBindings[ i ] = d_programSamplerBindings[ i ] = -1;
	}
	if( lastTexUnit != d_textureUnit )
		GLImports::glActiveTexture( d_textureUnit );
}

void GLBindingCheck::GetViewport( ) {	
	GLImports::glGetFloatv( GL_VIEWPORT, d_viewport );
}

void GLBindingCheck::GetArrays( ) {
	GLImports::glGetIntegerv( GL_ARRAY_BUFFER_BINDING, &d_vertexBuffer );
	GLImports::glGetIntegerv( GL_ELEMENT_ARRAY_BUFFER_BINDING, &d_indexBuffer );
	GLImports::glGetVertexAttribiv( 0, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &d_vertexAttribArray );
}

void GLBindingCheck::GetStates( ) {
	GLImports::glGetBooleanv( GL_BLEND, ( GLboolean * )&d_enable_BLEND );
	GLImports::glGetBooleanv( GL_COLOR_LOGIC_OP, ( GLboolean * )&d_enable_COLOR_LOGIC_OP );
	GLImports::glGetBooleanv( GL_CULL_FACE, ( GLboolean * )&d_enable_CULL_FACE );
	GLImports::glGetBooleanv( GL_DEPTH_CLAMP, ( GLboolean * )&d_enable_DEPTH_CLAMP );
	GLImports::glGetBooleanv( GL_DEPTH_TEST, ( GLboolean * )&d_enable_DEPTH_TEST );
	GLImports::glGetBooleanv( GL_DITHER, ( GLboolean * )&d_enable_DITHER );
	GLImports::glGetBooleanv( GL_FRAMEBUFFER_SRGB, ( GLboolean * )&d_enable_FRAMEBUFFER_SRGB );
	GLImports::glGetBooleanv( GL_LINE_SMOOTH, ( GLboolean * )&d_enable_LINE_SMOOTH );
	GLImports::glGetBooleanv( GL_MULTISAMPLE, ( GLboolean * )&d_enable_MULTISAMPLE );
	GLImports::glGetBooleanv( GL_POLYGON_SMOOTH, ( GLboolean * )&d_enable_POLYGON_SMOOTH );
	GLImports::glGetBooleanv( GL_POLYGON_OFFSET_FILL, ( GLboolean * )&d_enable_POLYGON_OFFSET_FILL );
	GLImports::glGetBooleanv( GL_POLYGON_OFFSET_LINE, ( GLboolean * )&d_enable_POLYGON_OFFSET_LINE );
	GLImports::glGetBooleanv( GL_POLYGON_OFFSET_POINT, ( GLboolean * )&d_enable_POLYGON_OFFSET_POINT );
	GLImports::glGetBooleanv( GL_PROGRAM_POINT_SIZE, ( GLboolean * )&d_enable_PROGRAM_POINT_SIZE );
	GLImports::glGetBooleanv( GL_PRIMITIVE_RESTART, ( GLboolean * )&d_enable_PRIMITIVE_RESTART );
	GLImports::glGetBooleanv( GL_SAMPLE_ALPHA_TO_COVERAGE, ( GLboolean * )&d_enable_SAMPLE_ALPHA_TO_COVERAGE );
	GLImports::glGetBooleanv( GL_SAMPLE_ALPHA_TO_ONE, ( GLboolean * )&d_enable_SAMPLE_ALPHA_TO_ONE );
	GLImports::glGetBooleanv( GL_SAMPLE_COVERAGE, ( GLboolean * )&d_enable_SAMPLE_COVERAGE );
	GLImports::glGetBooleanv( GL_SAMPLE_MASK, ( GLboolean * )&d_enable_SAMPLE_MASK );
	GLImports::glGetBooleanv( GL_SCISSOR_TEST, ( GLboolean * )&d_enable_SCISSOR_TEST );
	GLImports::glGetBooleanv( GL_STENCIL_TEST, ( GLboolean * )&d_enable_STENCIL_TEST );
	GLImports::glGetIntegerv( GL_CLAMP_READ_COLOR, ( GLint * )&d_CLAMP_READ_COLOR );
}

void GLBindingCheck::AllCheck( ) {	
	GetDrawBuffers( );
	GetFrameBuffers( );
	GetRenderBuffer( );
	GetTexture( );
	GetProgram( );
	GetViewport( );
	GetArrays( );
	GetStates( );
}

// // //// // //// // //// //
// GLScopedLock
// //// // //// // //// //

GLScopedLock::GLScopedLock( ) {
	//GLImports::glFinish( );
	d_syncObject = GLImports::glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 );
	GLImports::glFlush( );
	GLImports::glWaitSync( d_syncObject, 0, GL_TIMEOUT_IGNORED );
	GLImports::glDeleteSync( d_syncObject );
}

GLScopedLock::~GLScopedLock( ) {
	//GLImports::glClientWaitSync( d_syncObject, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED );
	d_syncObject = GLImports::glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 );
	GLImports::glWaitSync( d_syncObject, 0, GL_TIMEOUT_IGNORED );
	//GLImports::glClientWaitSync( d_syncObject, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED );
	GLImports::glDeleteSync( d_syncObject );
}

// // //// // //// // //// //
// GLTexture
// //// // //// // //// //

GLTexture::GLTexture( ) {
	d_textureTarget = 0;
	d_textureObject = 0;
}

GLTexture::GLTexture( GLenum target ) : d_textureTarget( target ) {
	GLImports::glGenTextures( 1, &d_textureObject );
}

GLTexture::~GLTexture( ) {
	if( d_textureObject )
		GLImports::glDeleteTextures( 1, &d_textureObject );
}

void GLTexture::LoadFromImage( Image * src ) {
	if( !src->GetSize( )[ 0 ] || !src->GetSize( )[ 1 ] ) {
		Common::Com_Printf( "GLTexture::UpLoad: empty image.\n" );
		return;
	}
	GLScopedLock glLock;
	GLImports::glActiveTexture( GL_TEXTURE31 );
	Bind( );
	//GLImports::glTexImage2D( d_textureTarget, 0, src->Alpha( ) ? GL_RGBA8 : GL_RGB8, src->GetSize( )[ 0 ],
	//	src->GetSize( )[ 1 ], 0, src->Alpha( ) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, src->GetPtr( ) );
	src->GLTexUpLoad( d_textureTarget );
	UnBind( );
	GLImports::glActiveTexture( GL_TEXTURE0 );
}

// // //// // //// // //// //
// GLLogicalBuffer
// //// // //// // //// //

GLLogicalBuffer::~GLLogicalBuffer( ) {
}

bool GLLogicalBuffer::IsDrawable( ) const {
	switch( d_bufferID ) {
		case GL_DEPTH_STENCIL_ATTACHMENT:
		case GL_DEPTH_ATTACHMENT:
		case GL_STENCIL_ATTACHMENT:
			return false;		
	}
	return true;
}

GLenum GLLogicalBuffer::GetDataType( ) const {
	if( d_bufferID == GL_DEPTH_STENCIL_ATTACHMENT )
		return GL_UNSIGNED_INT_24_8;
	return GL_UNSIGNED_BYTE;
}

GLenum GLLogicalBuffer::GetDataFormat( ) const {
	switch( d_bufferFormat ) {
		case GL_RGB8:
		case GL_SRGB8:
		case GL_RGB32F:
		case GL_RGB16F:
			return GL_RGB;
		case GL_RGBA8:
		case GL_SRGB8_ALPHA8:
		case GL_RGBA32F:
		case GL_RGBA16F:
			return GL_RGBA;
		case GL_DEPTH_COMPONENT24:
			return GL_DEPTH_COMPONENT;
		case GL_DEPTH24_STENCIL8:
			return GL_DEPTH_STENCIL;
	}
	assert( 0 );
	return 0;
}

GLLogicalBuffer * GLLogicalBuffer::ConstructBuffer( const GLFrameBuffer * frame_buffer, const Str & descriptor, const Str & name ) {

	Lexer lexer( descriptor.c_str( ), descriptor.Length( ), "GLLogicalBuffer descriptor", LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS );
	Token token;
	GLenum bufferID = 0, bufferFormat = 0;
	while( 1 ) {
		if( lexer.EndOfFile( ) )
			break;
		if( !lexer.ReadToken( &token ) )
			break;
		if( token.subtype == P_COMMA )
			continue;
		else if( token.IcmpnFast( "c", 1 ) ) {
			bufferID = GL_COLOR_ATTACHMENT0 + token.GetIntValueAfter( "c" );
			assert( ( bufferID >= GL_COLOR_ATTACHMENT0 ) && ( bufferID <= GL_COLOR_ATTACHMENT15 ) );
		} else if( token.IcmpFast( "d" ) ) {
			bufferID = GL_DEPTH_ATTACHMENT;
			bufferFormat = GL_DEPTH_COMPONENT24;
		} else if( token.IcmpFast( "s" ) ) {
			bufferID = GL_STENCIL_ATTACHMENT;
			assert( 0 );
		} else if( token.IcmpFast( "ds" ) ) {
			bufferID = GL_DEPTH_STENCIL_ATTACHMENT;
			bufferFormat = GL_DEPTH24_STENCIL8;
		} else if( token.IcmpnFast( "rgba", 4 ) ) {
			switch( token.GetIntValueAfter( "rgba" ) ) {
				case 8:
					bufferFormat = GL_RGBA8;
					break;
				case 16:
					bufferFormat = GL_RGBA16F;
					break;
				case 32:
					bufferFormat = GL_RGBA32F;
					break;
				default:
					assert( 0 );
			}
		} else if( token.IcmpnFast( "rgb", 3 ) ) {
			switch( token.GetIntValueAfter( "rgb" ) ) {
				case 8:
					bufferFormat = GL_RGB8;
					break;
				case 16:
					bufferFormat = GL_RGB16F;
					break;
				case 32:
					bufferFormat = GL_RGB32F;
					break;
				default:
					assert( 0 );
			}
		} else if( token.IcmpFast( "srgb" ) )
			bufferFormat = GL_SRGB8;
		else if( token.IcmpFast( "srgba" ) )
			bufferFormat = GL_SRGB8_ALPHA8;
		else
			assert( 0 );
	}
	assert( bufferID && bufferFormat );

	GLLogicalBuffer * logicalBuffer = frame_buffer->GetTextureMode( ) ?
		( GLLogicalBuffer * )new GLTextureBuffer( frame_buffer->GetSampleCount( ) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D ) :
		( GLLogicalBuffer * )new GLRenderBuffer( );

	logicalBuffer->d_name			= name;
	logicalBuffer->d_bufferID		= bufferID;
	logicalBuffer->d_bufferFormat	= bufferFormat;
	logicalBuffer->Construct( frame_buffer );
	return logicalBuffer;
}

// // //// // //// // //// //
// GLRenderBuffer
// //// // //// // //// //

GLRenderBuffer::GLRenderBuffer( ) {
	GLImports::glGenRenderbuffers( 1, &d_bufferObject );
}

GLRenderBuffer::~GLRenderBuffer( ) {
	GLImports::glDeleteRenderbuffers( 1, &d_bufferObject );
}

void GLRenderBuffer::Construct( const GLFrameBuffer * frame_buffer ) {
	GLImports::glBindRenderbuffer( GL_RENDERBUFFER, d_bufferObject ); // bind
	if( frame_buffer->GetSampleCount( ) ) // set storage
		GLImports::glRenderbufferStorageMultisample( GL_RENDERBUFFER, frame_buffer->GetSampleCount( ), d_bufferFormat, frame_buffer->GetSize( )[ 0 ], frame_buffer->GetSize( )[ 1 ] );
	else
		GLImports::glRenderbufferStorage( GL_RENDERBUFFER, d_bufferFormat, frame_buffer->GetSize( )[ 0 ], frame_buffer->GetSize( )[ 1 ] );
	GLImports::glFramebufferRenderbuffer( GL_FRAMEBUFFER, d_bufferID, GL_RENDERBUFFER, d_bufferObject ); // bind to frame buffer
	GLImports::glBindRenderbuffer( GL_RENDERBUFFER, 0 ); // un-bind
}

void GLRenderBuffer::Share( ) {
	GLImports::glBindRenderbuffer( GL_RENDERBUFFER, d_bufferObject ); // bind
	GLImports::glFramebufferRenderbuffer( GL_FRAMEBUFFER, d_bufferID, GL_RENDERBUFFER, d_bufferObject ); // bind to frame buffer
	GLImports::glBindRenderbuffer( GL_RENDERBUFFER, 0 ); // un-bind
}

// // //// // //// // //// //
// GLTextureBuffer
// //// // //// // //// //

GLTextureBuffer::GLTextureBuffer( GLenum target ) : d_texture( target ) {
}

GLTextureBuffer::~GLTextureBuffer( ) {
}

void GLTextureBuffer::Construct( const GLFrameBuffer * frame_buffer ) {
	d_texture.Bind( ); // bind
	if( frame_buffer->GetSampleCount( ) ) {
		assert( d_texture.GetTarget( ) == GL_TEXTURE_2D_MULTISAMPLE );
		GLImports::glTexImage2DMultisample( d_texture.GetTarget( ), frame_buffer->GetSampleCount( ), d_bufferFormat, frame_buffer->GetSize( )[ 0 ], frame_buffer->GetSize( )[ 1 ], GL_FALSE );
	} else {
		assert( d_texture.GetTarget( ) == GL_TEXTURE_2D );
		GLImports::glTexImage2D( d_texture.GetTarget( ), 0, d_bufferFormat, frame_buffer->GetSize( )[ 0 ], frame_buffer->GetSize( )[ 1 ], 0, GetDataFormat( ), GetDataType( ), NULL );
	}
	GLImports::glFramebufferTexture2D( GL_FRAMEBUFFER, d_bufferID, d_texture.GetTarget( ), d_texture.GetObjectID( ), 0 ); // bind to frame buffer
	d_texture.UnBind( ); // un-bind
}

void GLTextureBuffer::Share( ) {
	d_texture.Bind( ); // bind
	GLImports::glFramebufferTexture2D( GL_FRAMEBUFFER, d_bufferID, d_texture.GetTarget( ), d_texture.GetObjectID( ), 0 ); // bind to frame buffer
	d_texture.UnBind( ); // un-bind
}

// // //// // //// // //// //
// GLFrameBuffer
// //// // //// // //// //

void GLFrameBuffer::SetDrawBuffers( ) {
	CAList< GLenum, 16 > bufferList;
	for( int i = 0; i < d_logicalBuffers.Num( ); i++ ) {
		if( d_logicalBuffers[ i ]->IsDrawable( ) )
			bufferList.Append( d_logicalBuffers[ i ]->GetID( ) );
	}
	GLImports::glDrawBuffers( bufferList.Num( ), bufferList.Ptr( ) );
}

GLFrameBuffer::GLFrameBuffer( const Str & name, const VecT2i & size, bool use_textures, int ms_count ) {
	GLImports::glGenFramebuffers( 1, &d_frameBufferObject );
	d_frameSize			= size;
	d_name				= name;
	d_frameSamples		= ms_count;
	d_frameTextureMode	= use_textures;
	d_isSharedContext	= false;
	g_fameBufferList.Append( this );
}

GLFrameBuffer::~GLFrameBuffer( ) {
	if( !d_isSharedContext )
		d_logicalBuffers.DeleteContents( true );
	GLImports::glDeleteFramebuffers( 1, &d_frameBufferObject );
	g_fameBufferList.Remove( this );
}

GLFrameBuffer * GLFrameBuffer::ContextCreate( ) {
	GLFrameBuffer * frameBuffer = new GLFrameBuffer( d_name, d_frameSize, d_frameTextureMode, d_frameSamples );
	frameBuffer->d_isSharedContext = true;
	frameBuffer->Bind( );
	for( int i = 0; i < d_logicalBuffers.Num( ); i++ )
		d_logicalBuffers[ i ]->Share( );
	frameBuffer->d_logicalBuffers = d_logicalBuffers;
	frameBuffer->SetDrawBuffers( );
	frameBuffer->UnBind( );
	g_fameBufferList.Append( this );
	return frameBuffer;
}

void GLFrameBuffer::CustomSetup( const Str targets[ ][ 2 ] ) {
	Bind( );
	for( int i = 0; targets[ i ][ 0 ].Length( ); i++ )
		d_logicalBuffers.Append( GLLogicalBuffer::ConstructBuffer( this, targets[ i ][ 1 ], targets[ i ][ 0 ] ) );
	SetDrawBuffers( );
	assert( GLImports::glCheckFramebufferStatus( GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE );
	UnBind( );
}

void GLFrameBuffer::SingleSetup( const Str & descriptor, const Str & name ) {
	Bind( );
	d_logicalBuffers.Append( GLLogicalBuffer::ConstructBuffer( this, descriptor, name ) );
	SetDrawBuffers( );
	assert( GLImports::glCheckFramebufferStatus( GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE );
	UnBind( );
}

void GLFrameBuffer::RenderSetup( ) {
	const Str deferredSetup[ ][ 2 ] = {
		"diffuse",		"c0,rgba8",
		"normal",		"c1,rgb16",
		"coord",		"c2,rgb32",
		"depth",		"ds",
		null_string,	null_string
	};
	CustomSetup( deferredSetup );
}

void GLFrameBuffer::RenderBlit( GLFrameBuffer * dest ) {
	GLenum outBuffer;

	GLImports::glBindFramebuffer( GL_READ_FRAMEBUFFER, d_frameBufferObject );
	GLImports::glBindFramebuffer( GL_DRAW_FRAMEBUFFER, dest->d_frameBufferObject );

	outBuffer = GL_COLOR_ATTACHMENT0;
	GLImports::glReadBuffer( outBuffer );
	GLImports::glDrawBuffers( 1, &outBuffer );
	GLImports::glBlitFramebuffer( 0, 0, d_frameSize[0], d_frameSize[1], 0, 0, d_frameSize[0], d_frameSize[1], GL_COLOR_BUFFER_BIT, GL_NEAREST );

	outBuffer = GL_COLOR_ATTACHMENT1;
	GLImports::glReadBuffer( outBuffer );
	GLImports::glDrawBuffers( 1, &outBuffer );
	GLImports::glBlitFramebuffer( 0, 0, d_frameSize[0], d_frameSize[1], 0, 0, d_frameSize[0], d_frameSize[1], GL_COLOR_BUFFER_BIT, GL_NEAREST );

	outBuffer = GL_COLOR_ATTACHMENT2;
	GLImports::glReadBuffer( outBuffer );
	GLImports::glDrawBuffers( 1, &outBuffer );
	GLImports::glBlitFramebuffer( 0, 0, d_frameSize[0], d_frameSize[1], 0, 0, d_frameSize[0], d_frameSize[1], GL_COLOR_BUFFER_BIT, GL_NEAREST );

	GLImports::glBlitFramebuffer( 0, 0, d_frameSize[0], d_frameSize[1], 0, 0, d_frameSize[0], d_frameSize[1], GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST );
}

void GLFrameBuffer::Blit( ) {
	GLImports::glBindFramebuffer( GL_READ_FRAMEBUFFER, d_frameBufferObject );
	GLImports::glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
	GLImports::glBlitFramebuffer( 0, 0, d_frameSize[0], d_frameSize[1], 0, 0, d_frameSize[0], d_frameSize[1], GL_COLOR_BUFFER_BIT, GL_LINEAR );
}

void GLFrameBuffer::DrawAttachment( int index, float space_frac, float size_frac, int pos ) {

	int xSpace = ( int )( d_frameSize[0] * space_frac );
	int ySpace = ( int )( d_frameSize[1] * space_frac );
	int xSize = ( int )( d_frameSize[0] * size_frac );
	int ySize = ( int )( d_frameSize[1] * size_frac );

	GLImports::glReadBuffer( d_logicalBuffers[ index ]->GetID( ) );

	GLImports::glBlitFramebuffer(	0, 0, d_frameSize[0], d_frameSize[1], 
									( xSpace * ( pos+1 ) ) + ( xSize * pos ), d_frameSize[1] - ( ySpace + ySize ),
									( xSpace * ( pos+1 ) ) + ( xSize * ( pos+1 ) ), d_frameSize[1] - ySpace,
									GL_COLOR_BUFFER_BIT, GL_NEAREST );
}

void GLFrameBuffer::DrawAttachmentDebug( int index ) {
	float parts = Max( ceilf((float)d_frameSize[0]/(float)Video::GetResolution( )[0]), ceilf((float)d_frameSize[1]/(float)Video::GetResolution( )[1]) );
	int xSize = (int)( (float)d_frameSize[0] * parts );
	int ySize = (int)((float) d_frameSize[1] * parts );

	GLImports::glBindFramebuffer( GL_READ_FRAMEBUFFER, d_frameBufferObject );
	GLImports::glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
	GLImports::glReadBuffer( d_logicalBuffers[ index ]->GetID( ) );
	GLImports::glBlitFramebuffer( 0, 0, d_frameSize[0], d_frameSize[1], 0, 0, xSize, ySize, GL_COLOR_BUFFER_BIT, GL_NEAREST );
	UnBind( );
}

// // //// // //// // //// //
// GLSampler
// //// // //// // //// //

GLSampler::GLSampler( GLenum sampler_type ) {
	
	d_textureUnit		= 0;
	d_uniformLocation	= 0;
	d_samplerType		= sampler_type;
	d_numUsers			= new int;
	*d_numUsers			= 1;

	GLImports::glGenSamplers( 1, &d_samplerObject );

	switch( d_samplerType ) {
		case GL_SAMPLER_2D_SHADOW:
			SetParam( GL_TEXTURE_WRAP_S,			GL_CLAMP_TO_EDGE );
			SetParam( GL_TEXTURE_WRAP_T,			GL_CLAMP_TO_EDGE );
			SetParam( GL_TEXTURE_MIN_FILTER,		GL_NEAREST );
			SetParam( GL_TEXTURE_MAG_FILTER,		GL_NEAREST );
			//SetParam( GL_TEXTURE_COMPARE_MODE,	GL_COMPARE_REF_TO_TEXTURE );
			//SetParam( GL_TEXTURE_COMPARE_FUNC,	GL_LEQUAL );
			break;
		case GL_SAMPLER_2D:
			SetParam( GL_TEXTURE_WRAP_S,			GL_REPEAT );
			SetParam( GL_TEXTURE_WRAP_T,			GL_REPEAT );
			SetParam( GL_TEXTURE_MIN_FILTER,		GL_NEAREST );
			SetParam( GL_TEXTURE_MAG_FILTER,		GL_NEAREST );
			break;
		default:
			assert( d_samplerType != GL_SAMPLER_2D_SHADOW && d_samplerType != GL_SAMPLER_2D );
	}
}

GLSampler::GLSampler( const GLSampler & other ) {
	d_textureUnit		= other.d_textureUnit;
	d_uniformLocation	= other.d_uniformLocation;
	d_samplerType		= other.d_samplerType;
	d_samplerObject		= other.d_samplerObject;
	d_numUsers			= other.d_numUsers;
	( *d_numUsers )++;
}

GLSampler::~GLSampler( ) {
	assert( d_samplerObject );
	( *d_numUsers )--;
	if( !( *d_numUsers ) ) {
		GLImports::glDeleteSamplers( 1, &d_samplerObject );
		delete d_numUsers;
	}
}

// // //// // //// // //// //
// GLShader
// //// // //// // //// //

GLShader::GLShader( ) {
	d_program = 0;
	d_sharedProgram = false;
	d_sharedShaders = false;
}

GLShader * GLShader::ContextCreate( ) {
	GLShader * conShader = new GLShader( );
	conShader->d_name = d_name;
	conShader->d_path = d_path;
	for( int i = 0; i < d_inputSVars.Num( ); i++ )		
		conShader->d_inputSVars.Append( ( *d_inputSVars[ i ] == SVar_Sampler2D ) ? d_inputSVars[ i ]->CreateParam( ) : d_inputSVars[ i ] );
	for( int i = 0; i < d_inputSamplers.Num( ); i++ )
		conShader->d_inputSamplers.Append( new GLSampler( *d_inputSamplers[ i ] ) );
	conShader->d_program = d_program;
	conShader->d_sharedProgram = true;
	conShader->d_sharedShaders = true;
	g_shaderList.Append( this );
	return conShader;
}

GLShader::GLShader( const Str & filename ) {
	if( !Load( filename ) )
		Common::Com_Error( ERR_DROP, "Unable to load shader from file \"%s\"", filename.c_str( ) );
	d_sharedShaders = false;
	g_shaderList.Append( this );
}

GLShader::~GLShader( ) {
	if( d_sharedShaders )
		FreeSVars( );
	if( !d_sharedProgram ) {		
		FreeSVars( );
		FreeProgram( false );
	}
	d_inputSamplers.DeleteContents( true );
	g_shaderList.Remove( this );
}

void GLShader::Reload( ) {
	if( !Compile( ) ) {
		FreeProgram( true );
		return;
	}
	Link( );
}

void GLShader::Print( ) {

	char		variableName[ 256 ];
	GLsizei		variableLength;
	GLint		variableSize;
	GLenum		variableType;
	int			uniformNum;
	int			attribNum;

	if( !d_program )
		return;

	Common::Com_Printf( "Shader %s \"%s\" {\n\n", GetName( ).c_str( ), d_path.c_str( ) );
	GLImports::glGetProgramiv( d_program, GL_ACTIVE_UNIFORMS, &uniformNum );
	GLImports::glGetProgramiv( d_program, GL_ACTIVE_ATTRIBUTES, &attribNum );
	for( int i = 0; i < uniformNum; i++ ) {
		GLImports::glGetActiveUniform( d_program, i, sizeof( variableName ), &variableLength, &variableSize, &variableType, variableName );
		Common::Com_Printf( "\tUn\t%s\t%s\n", GLImports::GetEnumString( variableType ).c_str( ), variableName );
	}
	if( uniformNum && attribNum ) Common::Com_Printf( "\n" );
	for( int i = 0; i < attribNum; i++ ) {
		GLImports::glGetActiveAttrib( d_program, i, sizeof( variableName ), &variableLength, &variableSize, &variableType, variableName );
		Common::Com_Printf( "\tAt\t%s\t%s\n", GLImports::GetEnumString( variableType ).c_str( ), variableName );
	}
	Common::Com_Printf( "}\n" );
}

void GLShader::GetBufferLetters( Str & letters ) {
	letters.Clear( );
	for( int i = 0; i < d_loadedShaderTypeList.Num( ); i++ ) {
		switch( d_loadedShaderTypeList[ i ] ) {
			case GL_VERTEX_SHADER:		letters.Append( 'V' ); break;
			case GL_FRAGMENT_SHADER:	letters.Append( 'F' ); break;
			case GL_GEOMETRY_SHADER:	letters.Append( 'G' ); break;
		}
	}
}

bool GLShader::Load( const Str & filename ) {
	d_path = filename;
	ReuseProgram( );
	if( d_sharedProgram )
		return true;
	if( !Compile( ) ) {
		FreeProgram( true );
		return false;
	}
	if( !Link( ) ) {
		FreeProgram( false );
		return false;
	}
	return true;
}

bool GLShader::Compile( ) {

	Lexer		lexer( LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS );
	Token		token;
	Str			GLSL_version = "330";
	Dict		preprocessorDict;

	lexer.LoadFile( d_path );
	if( !lexer.IsLoaded( ) ) return false;
	while( 1 ) {
		if( lexer.EndOfFile( ) )
			break;
		if( !lexer.ReadToken( &token ) )
			return false;
		if( token.IcmpFast( "name" ) ) {
			if( !lexer.ReadToken( &token ) )
				return false;
			d_name = token;
		} else if( token.IcmpFast( "version" ) ) {
			if( !lexer.ReadToken( &token ) )
				return false;
			GLSL_version = token;
		} else if( token.IcmpFast( "define" ) ) {
			Str label;
			while( 1 ) {
				if( !lexer.ReadToken( &token ) ) {
					Common::Com_Printf( "Shader::Compile: out of data in \"%s\"\n", d_path.c_str( ) );
					return false;
				}
				label = token;
				lexer.ExpectTokenType( TT_PUNCTUATION, P_ASSIGN, &token );
				if( !lexer.ReadToken( &token ) ) {
					Common::Com_Printf( "Shader::Compile: out of data in \"%s\"\n", d_path.c_str( ) );
					return false;
				}
				preprocessorDict.Set( label, token );
				if( !lexer.ReadToken( &token ) ) {
					Common::Com_DPrintf( "Shader::Compile: out of data in \"%s\"\n", d_path.c_str( ) );
					return false;
				}
				if( token == ',' )
					continue;
				if( token == ';' )
					break;
				Common::Com_Printf( "Shader::Compile: wrong token \"%s\" in \"%s\"\n", token.c_str( ), d_path.c_str( ) );
				return false;
			}
		} else if( token.IcmpFast( "shader" ) ) {
			GLenum				shaderType;
			Str					source, define, blockName;
			int					braceDepth		= 0;
			const KeyValue *	keyValue;
			bool				blockDisable = false;

			if( !lexer.ReadToken( &token ) )
				return false;
			blockName = token;
			if( blockName.IcmpFast( "vertex" ) )
				shaderType = GL_VERTEX_SHADER;
			else if( blockName.IcmpFast( "fragment" ) )
				shaderType = GL_FRAGMENT_SHADER;
			else if( blockName.IcmpFast( "geometry" ) )
				shaderType = GL_GEOMETRY_SHADER;
			else
				blockDisable = true;
			sprintf( source, "#version %s\n", GLSL_version.c_str( ) );
			while( 1 ) {
				if( !lexer.ReadToken( &token ) ) {					
					if( !braceDepth ) {
						Common::Com_Printf( "Shader::Compile: Missing closing brace from \"%s\"\n", d_path.c_str( ) );
						break;
					}
					return false;
				}
				if( token == '{' ) {					
					braceDepth++;
					if( braceDepth == 1 )
						continue;
				} else if( token == '}' ) {					
					braceDepth--;
					if( !braceDepth )
						break;
				}
				if( blockDisable )
					continue;
				if( source.Length( ) )
					source.Append( ' ' );
				keyValue = preprocessorDict.FindKey( token );
				if( keyValue )
					source.Append( keyValue->GetValue( ) );
				else
					source.Append( token );
				if( token == ';' || token == '{' || token == '}' )
					source.Append( '\n' );
			}
			if( blockDisable ) {
				Common::Com_Printf( "Shader::Compile: disabled shader block: \"%s\" in \"%s\"\n", blockName.c_str( ), d_path.c_str( ) );
				continue;
			}
			if( source.IsEmpty( ) ) {
				Common::Com_Printf( "Shader::Compile: Shader block contains no data in \"%s\"\n", d_path.c_str( ) );
				break;
			}
			GLhandleARB			shaderHandle	= GLImports::glCreateShader( shaderType );
			const GLcharARB *	sptr			= ( GLcharARB * )source.c_str( );
			int					status;
			char				errorText[ 256 ];

			d_loadedShaderTypeList.Append( shaderType );
			GLImports::glShaderSource( shaderHandle, 1, &sptr, NULL );
			GLImports::glCompileShader( shaderHandle );
			GLImports::glGetShaderiv( shaderHandle, GL_COMPILE_STATUS, &status );

			if( !status ) {
				int errorLen;
				Str srcLine;
				GLImports::glGetShaderInfoLog( shaderHandle, sizeof( errorText ), &errorLen, errorText );
				lexer.FreeSource( );
				lexer.LoadMemory( errorText, errorLen, "GLSL error" );
				int errorParsable = lexer.IsLoaded( );
				if( errorParsable )
					errorParsable = lexer.CheckTokenType( TT_NUMBER, TT_INTEGER, &token );
				if( errorParsable )
					errorParsable = lexer.CheckTokenType( TT_PUNCTUATION, P_PARENTHESESOPEN, &token );
				if( errorParsable )
					errorParsable = lexer.CheckTokenType( TT_NUMBER, TT_INTEGER, &token );
				if( errorParsable ) {
					int errorLine = token.GetIntValue( );
					lexer.FreeSource( );
					lexer.LoadMemory( source.c_str( ), source.Length( ), "GLSL source" );
					for( int i = 1; i < errorLine; i++ )
						lexer.SkipRestOfLine( );
					lexer.ReadRestOfLine( srcLine );
				}
				if( srcLine.IsEmpty( ) )
					srcLine = "[GLSL source unknow]";
				Common::Com_Printf( "Shader compile error \"%s\" {\n\t%s\n\t%s\n}\n", d_path.c_str( ), srcLine.c_str( ), errorText );
				return false;
			}
			d_compilerShaderList.Append( shaderHandle );
		}
	}
	return true;
}

GLSampler * GLShader::FindSampler( GLenum sampler_type ) {
	for( int i = 0; i < d_inputSamplers.Num( ); i++ ) {
		if( *d_inputSamplers[ i ] == sampler_type )
			return d_inputSamplers[ i ];
	}
	return NULL;
}

GLSampler * GLShader::CreateSampler( GLenum sampler_type ) {
	GLSampler * sampler = FindSampler( sampler_type );
	sampler = sampler ? new GLSampler( *sampler ) : new GLSampler( sampler_type );
	sampler->SetTextureUnit( d_inputSamplers.Num( ) );
	d_inputSamplers.Append( sampler );
	return sampler;
}

bool GLShader::Link( ) {
	
	if( !d_compilerShaderList.Num( ) ) {
		Common::Com_Printf( "Shader::Link: Missing/faulty compiler input: \"%s\"\n", GetName( ).c_str( ) );
		return false;
	}
	char		uniformName[ 256 ];
	GLsizei		ulength;
	GLint		usize;
	GLenum		utype;
	int			status;

	GLhandleARB inputProgram = GLImports::glCreateProgram( );
	for( int i = 0; i < d_compilerShaderList.Num( ); i++ ) {
		GLImports::glAttachShader( inputProgram, d_compilerShaderList[ i ] );
	}
	GLImports::glLinkProgram( inputProgram );
	GLImports::glGetProgramiv( inputProgram, GL_LINK_STATUS, &status );
	if( status == GL_FALSE ) {
		Common::Com_Printf( "Shader::Link: Linker error: \"%s\"\n", GetName( ).c_str( ) );
		GLImports::glDeleteProgram( inputProgram );
		return false;
	}
	GLImports::glGetProgramiv( inputProgram, GL_ACTIVE_UNIFORMS, &status );
	for( int i = 0; i < status; i++ ) {
		GLImports::glGetActiveUniform( inputProgram, i, sizeof( uniformName ), &ulength, &usize, &utype, uniformName );
		if( !SVar::CheckGlobal( uniformName ) ) {
			GLSampler * sampler = CreateSampler( utype );
			sampler->SetUniformLocation( i );
			d_inputSVars.Append( new SVarSampler2D( uniformName ) );
		} else {
			d_inputSVars.Append( &SVar::Find( uniformName ) );
		}
	}
	/*GLImports::glGetProgramiv( inputProgram, GL_ACTIVE_ATTRIBUTES, &status );
	for( int i = 0; i < status; i++ ) {
		GLImports::glGetActiveAttrib( inputProgram, i, sizeof( uniformName ), &ulength, &usize, &utype, uniformName );
		GLint loc = GLImports::glGetAttribLocation( inputProgram, uniformName );
		Common::Com_Printf( "Shader::Link: Attrib \"%s\" (ai:%i aloc:%i atype:%x)\n", uniformName, i, loc, utype );
	}
	Common::Com_Printf( "Shader::Link: Shader \"%s\" loaded.\n", GetName( ).c_str( ) );*/
	if( !d_sharedProgram && d_program )
		GLImports::glDeleteProgram( d_program );
	d_program = inputProgram;
	d_internalShaderList = d_compilerShaderList;
	d_compilerShaderList.Clear( );
	return true;
}

void GLShader::FreeProgram( bool compiler_list ) {
	if( compiler_list ) {
		for( int i = 0; i < d_compilerShaderList.Num( ); i++ )
			GLImports::glDeleteShader( d_compilerShaderList[ i ] );
		d_compilerShaderList.Clear( );
		return;
	}
	if( d_program ) {
		GLImports::glDeleteProgram( d_program );
		d_program = 0;
	}
	if( !d_sharedShaders ) {
		for( int i = 0; i < d_internalShaderList.Num( ); i++ )
			GLImports::glDeleteShader( d_internalShaderList[ i ] );
	}
	d_internalShaderList.Clear( );
	d_loadedShaderTypeList.Clear( );
}

void GLShader::ReuseProgram( ) {
	GLShader * identShader = FindByPath( d_path );
	if( identShader ) {
		d_program = identShader->d_program;
		for( int i = 0; i < identShader->d_inputSVars.Num( ); i++ )
			d_inputSVars.Append( ( *identShader->d_inputSVars[ i ] == SVar_Sampler2D ) ? identShader->d_inputSVars[ i ]->CreateParam( ) : identShader->d_inputSVars[ i ] );
		for( int i = 0; i < identShader->d_inputSamplers.Num( ); i++ )
			d_inputSamplers.Append( new GLSampler( *identShader->d_inputSamplers[ i ] ) );
		d_name = identShader->d_name;
		d_sharedProgram = true;
	} else {
		d_program = 0;
		d_sharedProgram = false;
	}
}

void GLShader::FreeSVars( ) {
	for( int i = 0; i < d_inputSVars.Num( ); i++ ) {
		if( *d_inputSVars[ i ] == SVar_Sampler2D ) // free only texture inputs
			delete d_inputSVars[ i ];
	}
}

GLShader * GLShader::FindByPath( const Str & shader_path ) {
	for( int i = 0; i < g_shaderList.Num( ); i++ ) {
		if( g_shaderList[ i ]->d_path == shader_path )
			return g_shaderList[ i ];
	}
	return NULL;
}
