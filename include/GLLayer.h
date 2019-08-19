#ifndef GLLAYER_H
#define GLLAYER_H

// // //// // //// // //// //
/// GLType
// //// // //// // //// //

template< typename type > class GLType {
public:
	static GLenum						GetTypeEnum( );
};

// // //// // //// // //// //
/// GLBindingCheck
// //// // //// // //// //

class GLBindingCheck {
public:
	int									d_maxDrawBuffers;
	int									d_maxTextureUnits;

	glEnumName							d_drawBuffers[ 4 ];
	int									d_drawFrameBuffer;
	int									d_readFrameBuffer;
	int									d_renderBuffer;
	glEnumName							d_textureUnit;
	int									d_texture2D;
	int									d_texture2DMS;
	int									d_program;
	int									d_programUniforms;
	int									d_programSamplerBindings[ 4 ];
	int									d_programTextureBindings[ 4 ];
	float								d_viewport[ 4 ];
	int									d_vertexBuffer;
	int									d_indexBuffer;
	int									d_vertexAttribArray;

	bool								d_enable_BLEND;
	bool								d_enable_COLOR_LOGIC_OP;
	bool								d_enable_CULL_FACE;
	bool								d_enable_DEPTH_CLAMP;
	bool								d_enable_DEPTH_TEST;
	bool								d_enable_DITHER;
	bool								d_enable_FRAMEBUFFER_SRGB;
	bool								d_enable_LINE_SMOOTH;
	bool								d_enable_MULTISAMPLE;
	bool								d_enable_POLYGON_SMOOTH;
	bool								d_enable_POLYGON_OFFSET_FILL;
	bool								d_enable_POLYGON_OFFSET_LINE;
	bool								d_enable_POLYGON_OFFSET_POINT;
	bool								d_enable_PROGRAM_POINT_SIZE;
	bool								d_enable_PRIMITIVE_RESTART;
	bool								d_enable_SAMPLE_ALPHA_TO_COVERAGE;
	bool								d_enable_SAMPLE_ALPHA_TO_ONE;
	bool								d_enable_SAMPLE_COVERAGE;
	bool								d_enable_SAMPLE_MASK;
	bool								d_enable_SCISSOR_TEST;
	bool								d_enable_STENCIL_TEST;
	glEnumName							d_CLAMP_READ_COLOR;

										GLBindingCheck( );

	void								GetDrawBuffers( );
	void								GetFrameBuffers( );
	void								GetRenderBuffer( );
	void								GetTexture( );
	void								GetProgram( );
	void								GetViewport( );
	void								GetArrays( );
	void								GetStates( );

	void								AllCheck( );
};

// // //// // //// // //// //
/// VertexBufferFormat
// //// // //// // //// //

class VertexBufferFormat {
public:
	GLint											size;
 	GLenum											type;
 	GLboolean										normalized;
 	GLsizei											stride;
 	GLvoid *										pointer;

} ALIGN( 1 );

// // //// // //// // //// //
/// VertexUI
//
/// map & model vertex
// //// // //// // //// //

class Vertex3D {
public:
	VecT3f			xyz;
	VecT2hf			st;
	VecT4hf			normal;

	Vertex3D &		operator =( const DrawVert & vert ) { xyz = vert.xyz; st = vert.st; normal = vert.normal; return *this; }

	static VertexBufferFormat						g_vertexBufferFormat[ ];
};

// // //// // //// // //// //
/// VertexUI
//
/// 2D vertex representation
// //// // //// // //// //

class VertexUI {
public:
	VecT2hf											xy;
	VecT2hf											st;
	Color											color;

	static VertexBufferFormat						g_vertexBufferFormat[ ];
};

// // //// // //// // //// //
/// VertexColorOnlyUI
//
/// 2D vertex without texcoord for overlay color rects
// //// // //// // //// //

class VertexColorOnlyUI {
public:
	Vec2											xy;
	Color											color;

	static VertexBufferFormat						g_vertexBufferFormat[ ];
};

// // //// // //// // //// //
/// GLScopedLock
// //// // //// // //// //

class GLScopedLock {
private:
	GLsync											d_syncObject;

public:
													GLScopedLock( );
													~GLScopedLock( );
};

// // //// // //// // //// //
/// GLScopedQuery
// //// // //// // //// //

template< GLenum target > class GLScopedQuery {
private:
	GLuint											d_queryObject;

public:
													GLScopedQuery( );
													~GLScopedQuery( );

	void											Begin( );
	void											End( );

	int												GetResultInt( );
	uint											GetResultUint( );
};

class Image;

// // //// // //// // //// //
/// GLTexture
// //// // //// // //// //

class GLTexture {
protected:	
	GLuint											d_textureObject;
	GLenum											d_textureTarget;

public:
													GLTexture( );
													GLTexture( GLenum target );
													~GLTexture( );

	void											Bind( );
	void											UnBind( );

	GLenum											GetTarget( ) const;
	GLuint											GetObjectID( ) const;

	void											LoadFromImage( Image * src );
};

class GLFrameBuffer;

// // //// // //// // //// //
/// GLLogicalBuffer
// //// // //// // //// //

class GLLogicalBuffer : public NamedObject {
protected:
	GLenum											d_bufferID;		// logical buffer id		- COLOR_ATTACHMENTx, DEPTH_&|STENCIL_ATTACHMENT
	GLenum											d_bufferFormat;	// internal format id		- RGB[A][8|16F|32F], SRGB8[_ALPHA8], DEPTH24_STENCIL8, DEPTH_COMPONENT24

public:
	/// returns true if the buffer can be drawable by glDrawBuffers
	bool											IsDrawable( ) const;
	/// returns attachment id
	GLenum											GetID( ) const;
	/// gets data type		- UNSIGNED_BYTE, UNSIGNED_INT_24_8
	GLenum											GetDataType( ) const;
	/// returns tex format	- RGB[A], DEPTH_STENCIL, DEPTH_COMPONENT
	GLenum											GetDataFormat( ) const;
	/// virtual destructor called when the frame buffer destructor
	virtual											~GLLogicalBuffer( );
	/// called on resize/creation
	virtual void									Construct( const GLFrameBuffer * frame_buffer ) = 0;
	/// called on attachment sharing setup between frame buffers
	virtual void									Share( ) = 0;

	static GLLogicalBuffer *						ConstructBuffer( const GLFrameBuffer * frame_buffer, const Str & descriptor, const Str & name );
};

// // //// // //// // //// //
/// GLRenderBuffer
// //// // //// // //// //

class GLRenderBuffer : public GLLogicalBuffer {
private:
	GLuint											d_bufferObject;			// object handle

public:
													GLRenderBuffer( );
	virtual											~GLRenderBuffer( );

	virtual void									Construct( const GLFrameBuffer * frame_buffer );
	virtual void									Share( );

	GLuint											GetRenderBufferID( );
};

// // //// // //// // //// //
/// GLTextureBuffer
// //// // //// // //// //

class GLTextureBuffer : public GLLogicalBuffer {
private:
	GLTexture										d_texture;

public:
													GLTextureBuffer( GLenum target );
	virtual											~GLTextureBuffer( );

	virtual void									Construct( const GLFrameBuffer * frame_buffer );
	virtual void									Share( );

	GLTexture *										GetTexture( );
};

// // //// // //// // //// //
/// GLFrameBuffer
// //// // //// // //// //

class GLFrameBuffer : public NamedObject {
private:
	GLuint											d_frameBufferObject;
	VecT2i											d_frameSize;
	int												d_frameSamples;
	bool											d_frameTextureMode;
	CANamedList< GLLogicalBuffer *, 16 >			d_logicalBuffers;
	bool											d_isSharedContext;

	void											SetDrawBuffers( );

	static CANamedList< GLFrameBuffer *, 128 >		g_fameBufferList;

public:
													GLFrameBuffer( const Str & name, const VecT2i & size, bool use_textures, int ms_count );
													~GLFrameBuffer( );

	GLFrameBuffer *									ContextCreate( );

	void											ReSize( const VecT2i & size );
	void											ReSize( const VecT2i & size, int ms_count );
	void											ReSize( int ms_count );

	const VecT2i &									GetSize( ) const;
	int												GetSampleCount( ) const;
	bool											GetTextureMode( ) const;
	float											GetAspectRatio( ) const;

	GLLogicalBuffer *								GetLogicalBuffer( const Str & name );
	GLuint											GetRenderBuffer( const Str & name );
	GLTexture *										GetTextureBuffer( const Str & name );

	void											CustomSetup( const Str targets[ ][ 2 ] );
	void											SingleSetup( const Str & descriptor, const Str & name );

	void											RenderSetup( );
	void											RenderBlit( GLFrameBuffer * dest );
	void											Blit( );
	void											DrawAttachment( int index, float space_frac, float size_frac, int pos );
	void											DrawAttachmentDebug( int index );

	void											Bind( );
	void											UnBind( );
	void											ReadBind( );
	void											DrawBind( );

	void											AdjustViewport( );
};

// // //// // //// // //// //
/// GLSampler
// //// // //// // //// //

class GLSampler {
private:
	GLenum											d_samplerType;
	GLuint											d_samplerObject;
	int												d_textureUnit;
	int												d_uniformLocation;
	int *											d_numUsers;

	void											SetParam( GLenum param, int value );
	void											SetParam( GLenum param, float value );
	void											SetParam( GLenum param, float * values );

public:
													GLSampler( GLenum sampler_type );
													GLSampler( const GLSampler & other );
													~GLSampler( );

	bool											operator ==( GLenum with ) const;
	void											SetTextureUnit( int texture_unit );
	void											SetUniformLocation( int uniform_location );
	int												GetUniformLocation( ) const;

	void											ActivateTextureUnit( );
	void											Bind( );
	void											UnBind( );
};

class SVar;

// // //// // //// // //// //
/// GLShader
// //// // //// // //// //

class GLShader : public NamedObject {
private:
	Str												d_path;
	GLhandleARB										d_program;

	CAList< GLhandleARB, 32 >						d_compilerShaderList;
	CAList< GLhandleARB, 32 >						d_internalShaderList;
	CAList< GLenum, 32 >							d_loadedShaderTypeList;
	CANamedList< SVar *, 32 >						d_inputSVars;
	CAList< GLSampler *, 32 >						d_inputSamplers;

	bool											d_sharedProgram;
	bool											d_sharedShaders;

	GLSampler *										FindSampler( GLenum sampler_type );
	GLSampler *										CreateSampler( GLenum sampler_type );
	
	void											FreeProgram( bool compiler_list );
	void											ReuseProgram( );
	void											FreeSVars( );

	bool											Load( const Str & filename );
	bool											Compile( );
	bool											Link( );

	static CANamedList< GLShader *, 128 >			g_shaderList;

public:
													GLShader( );
													GLShader( const Str & filename );
													~GLShader( );

	GLShader *										ContextCreate( );

	void											Bind( );
	void											UnBind( );

	void											WriteSVars( );

	void											Reload( );
	void											Print( );
	void											GetBufferLetters( Str & letters );

	SVar &											FindInput( const Str & input_name );

	static GLShader *								Find( const Str & shader_name );
	static GLShader *								FindByPath( const Str & shader_path );
	static int										GetNumShaders( );
	static GLShader *								GetShader( const int index );
};

// // //// // //// // //// //
/// GLVertexArrayObject
// //// // //// // //// //

template< typename type > class GLVertexArrayObject {
private:
	GLuint											d_vertexArrayID;

public:
													GLVertexArrayObject( );
	GLVertexArrayObject &							operator =( const GLVertexArrayObject & other );

	void											Bind( );
	void											UnBind( )	{ GLImports::glBindVertexArray( 0 ); }
	void											SetupBuffer( );
};

// // //// // //// // //// //
/// GLDynamicBufferBlock
// //// // //// // //// //

template< typename type > class GLDynamicBufferBlock {
public:
	type *											d_data;
	int												d_offset;
	int												d_size;

	type &											operator[ ]( int index ) { assert( index < d_size ); return d_data[ index ]; }
	int												ObjectNum( ) const { return d_offset / sizeof( type ); }
	size_t											GetByteSize( ) const { return NumSizeOf( type, d_size ); }

	static int										OffsetCompare( GLDynamicBufferBlock * const * a,  GLDynamicBufferBlock * const * b ) { return ( *a )->d_offset - ( *b )->d_offset; }
};

// // //// // //// // //// //
/// GLBufferPrimitive
// //// // //// // //// //

template< typename vertex_type, typename element_type > class GLBufferPrimitive {
public:
	GLDynamicBufferBlock< vertex_type > *			d_vertexBlock;
	GLDynamicBufferBlock< element_type > *			d_indexBlock;
	GLenum											d_primitiveType;
	GLenum											d_indexType;

	void											Draw( GLsizei primitive_count );
};

// // //// // //// // //// //
/// GLDynamicBufferState
// //// // //// // //// //

template< typename buffer_type, GLenum buffer_target, GLenum buffer_usage > class GLDynamicBufferState {
private:
	typedef GLDynamicBufferBlock< buffer_type >		t_block;
	typedef BTreeNode< int, int >					t_freeAreaNode;

	GLuint											d_bufferID;
	int												d_bufferNum;
	int												d_bufferAllocSize;
	int												d_bufferCurrentSize;
	int												d_bufferGranularity;
	BlockAlloc< t_block, 128 >						d_blockAllocator;
	BlockAlloc< int, 128 >							d_keyAllocator;
	BTree< int, int, 4 >							d_freeTree;

	void											ResizeBuffer( );

public:
													GLDynamicBufferState( int granularity = 32 );
													~GLDynamicBufferState( );

	void											Bind( );
	void											UnBind( );
	t_block *										Alloc( int num );
	void											Remap( t_block * block );
	void											Free( t_block * block );
	int												Num( ) const;
};

// // //// // //// // //// //
/// GLDynamicBufferAllocator
// //// // //// // //// //

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage > class GLDynamicBufferAllocator {
public:
	typedef GLBufferPrimitive< vertex_type, element_type >									t_primitive;
	typedef vertex_type																		t_vertexType;
	typedef GLDynamicBufferBlock< vertex_type >												t_vertexBlock;
	typedef GLDynamicBufferState< vertex_type, GL_ARRAY_BUFFER,vertex_usage >				t_vertexState;
	typedef element_type																	t_indexType;
	typedef GLDynamicBufferBlock< element_type >											t_indexBlock;
	typedef GLDynamicBufferState< element_type, GL_ELEMENT_ARRAY_BUFFER, element_usage >	t_indexState;

private:

	GLVertexArrayObject< vertex_type >				d_vertexArrayObject;

	t_indexState *									d_stateIndex;
	t_vertexState *									d_stateVertex;
	BlockAlloc< t_primitive, 128 > *				d_primitiveAllocator;

	bool											d_isOwner;

													GLDynamicBufferAllocator( GLDynamicBufferAllocator * other );

	void											InitVAO( );

public:
													GLDynamicBufferAllocator( );
													~GLDynamicBufferAllocator( );

	GLDynamicBufferAllocator *						ContextCreate( );

	void											BeginDraw( );
	void											EndDraw( );

	t_vertexBlock *									AllocVertex( int num );
	void											RemapVertex( t_vertexBlock* block );
	void											FreeVertex( t_vertexBlock * block );
	void											UploadVertex( t_vertexBlock * block );
	int												NumVertex( ) const;

	t_indexBlock *									AllocIndex( int num );
	void											RemapIndex( t_indexBlock * block );
	void											FreeIndex( t_indexBlock * block );
	void											UploadIndex( t_indexBlock * block );
	int												NumIndex( ) const;

	t_primitive *									TexturePrimitive( const Bounds2D & bounds, bool rotate );
	t_primitive *									TexturePrimitive( const RectTi & rect, bool rotate );
	t_primitive *									TexturePrimitive( const VecT2i & size );
	t_primitive *									ColorRectPrimitive( const Bounds2D & bounds, const Color & color );
	t_primitive *									SurfacePrimitive( const Surface & surface );
	void											FreePrimitive( t_primitive * primitive );
};

typedef GLDynamicBufferAllocator< Vertex3D, GL_STATIC_DRAW, GLuint, GL_STATIC_DRAW >			BufferAllocatorMap;
typedef GLDynamicBufferAllocator< Vertex3D, GL_DYNAMIC_DRAW, GLuint, GL_STATIC_DRAW >			BufferAllocatorModel;
typedef GLBufferPrimitive< Vertex3D, GLuint >													BufferPrimitive3D;
typedef GLDynamicBufferBlock< Vertex3D >														BufferVertex3D;

typedef GLDynamicBufferAllocator< VertexUI, GL_STREAM_DRAW, GLushort, GL_STREAM_DRAW >			BufferAllocatorUI;
typedef GLBufferPrimitive< VertexUI, GLushort >													BufferPrimitiveUI;

typedef GLDynamicBufferAllocator< VertexColorOnlyUI, GL_STATIC_DRAW, GLushort, GL_STATIC_DRAW >	BufferAllocatorCR;
typedef GLBufferPrimitive< VertexColorOnlyUI, GLushort >										BufferPrimitiveCR;

// // //// // //// // //// //
// GLType
// //// // //// // //// //

template< > INLINE GLenum GLType< GLbyte >::GetTypeEnum( ) {
	return GL_BYTE;
}

template< > INLINE GLenum GLType< GLubyte >::GetTypeEnum( ) {
	return GL_UNSIGNED_BYTE;
}

template< > INLINE GLenum GLType< GLshort >::GetTypeEnum( ) {
	return GL_SHORT;
}

template< > INLINE GLenum GLType< GLushort >::GetTypeEnum( ) {
	return GL_UNSIGNED_SHORT;
}

template< > INLINE GLenum GLType< GLint >::GetTypeEnum( ) {
	return GL_INT;
}

template< > INLINE GLenum GLType< GLuint >::GetTypeEnum( ) {
	return GL_UNSIGNED_INT;
}

template< > INLINE GLenum GLType< GLfloat >::GetTypeEnum( ) {
	return GL_FLOAT;
}

template< > INLINE GLenum GLType< GLdouble >::GetTypeEnum( ) {
	return GL_DOUBLE;
}

// // //// // //// // //// //
// GLScopedQuery
// //// // //// // //// //

template< GLenum target >
GLScopedQuery< target >::GLScopedQuery( ) {
	GLImports::glGenQueries( 1, &d_queryObject );
}

template< GLenum target >
GLScopedQuery< target >::~GLScopedQuery( ) {
	GLImports::glDeleteQueries( 1, &d_queryObject );
}

template< GLenum target >
INLINE void GLScopedQuery< target >::Begin( ) {
	GLImports::glBeginQuery( target, d_queryObject );
}

template< GLenum target >
INLINE void GLScopedQuery< target >::End( ) {
	GLImports::glEndQuery( target );
}

template< GLenum target >
int GLScopedQuery< target >::GetResultInt( ) {
	int val;
	GLImports::glGetQueryObjectiv( d_queryObject, GL_QUERY_RESULT, &val );
	return val;
}

template< GLenum target >
uint GLScopedQuery< target >::GetResultUint( ) {
	uint val;
	GLImports::glGetQueryObjectuiv( d_queryObject, GL_QUERY_RESULT, &val );
	return val;
}

// // //// // //// // //// //
// GLTexture
// //// // //// // //// //

INLINE void GLTexture::Bind( ) {
	GLImports::glBindTexture( d_textureTarget, d_textureObject );
}

INLINE void GLTexture::UnBind( ) {
	GLImports::glBindTexture( d_textureTarget, 0 );
}

INLINE GLenum GLTexture::GetTarget( ) const {
	return d_textureTarget;
}

INLINE GLuint GLTexture::GetObjectID( ) const {
	return d_textureObject;
}

// // //// // //// // //// //
// GLLogicalBuffer, GLRenderBuffer, GLTextureBuffer
// //// // //// // //// //

INLINE GLenum GLLogicalBuffer::GetID( ) const {
	return d_bufferID;
}

INLINE GLuint GLRenderBuffer::GetRenderBufferID( ) {
	return d_bufferObject;
}

INLINE GLTexture * GLTextureBuffer::GetTexture( ) {
	return &d_texture;
}

// // //// // //// // //// //
// GLFrameBuffer
// //// // //// // //// //

INLINE void GLFrameBuffer::AdjustViewport( ) {
	GLImports::glViewport( 0, 0, d_frameSize[ 0 ], d_frameSize[ 1 ] );
}

INLINE const VecT2i & GLFrameBuffer::GetSize( ) const {
	return d_frameSize;
}

INLINE int GLFrameBuffer::GetSampleCount( ) const {
	return d_frameSamples;
}

INLINE bool GLFrameBuffer::GetTextureMode( ) const {
	return d_frameTextureMode;
}

INLINE float GLFrameBuffer::GetAspectRatio( ) const {
	return ( float )d_frameSize[ 0 ] / ( float )d_frameSize[ 1 ];
}

INLINE GLLogicalBuffer * GLFrameBuffer::GetLogicalBuffer( const Str & name ) {
	return d_logicalBuffers.FindByName( name );
}

INLINE GLuint GLFrameBuffer::GetRenderBuffer( const Str & name ) {
	GLRenderBuffer * renderBuffer = reinterpret_cast< GLRenderBuffer * >( d_logicalBuffers.FindByName( name ) );
	assert( renderBuffer && !d_frameTextureMode );
	return renderBuffer->GetRenderBufferID( );
}

INLINE GLTexture * GLFrameBuffer::GetTextureBuffer( const Str & name ) {
	GLTextureBuffer * textureBuffer = reinterpret_cast< GLTextureBuffer * >( d_logicalBuffers.FindByName( name ) );
	assert( textureBuffer && d_frameTextureMode );
	return textureBuffer->GetTexture( );
}

INLINE void GLFrameBuffer::Bind( ) {
	GLImports::glBindFramebuffer( GL_FRAMEBUFFER, d_frameBufferObject );
}

INLINE void GLFrameBuffer::UnBind( ) {
	GLImports::glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

INLINE void GLFrameBuffer::ReadBind( ) {
	GLImports::glBindFramebuffer( GL_READ_FRAMEBUFFER, d_frameBufferObject );
}

INLINE void GLFrameBuffer::DrawBind( ) {
	GLImports::glBindFramebuffer( GL_DRAW_FRAMEBUFFER, d_frameBufferObject );
}

INLINE void GLFrameBuffer::ReSize( const VecT2i & size ) {
	if( d_frameSize == size )
		return;
	d_frameSize = size;
	for( int i = 0; i < d_logicalBuffers.Num( ); i++ )
		d_logicalBuffers[ i ]->Construct( this );
}

INLINE void GLFrameBuffer::ReSize( const VecT2i & size, int ms_count ) {
	if( d_frameSamples == ms_count && d_frameSize == size )
		return;
	d_frameSize = size;
	d_frameSamples = ms_count;
	for( int i = 0; i < d_logicalBuffers.Num( ); i++ )
		d_logicalBuffers[ i ]->Construct( this );
}

INLINE void GLFrameBuffer::ReSize( int ms_count ) {
	if( d_frameSamples == ms_count )
		return;
	d_frameSamples = ms_count;
	for( int i = 0; i < d_logicalBuffers.Num( ); i++ )
		d_logicalBuffers[ i ]->Construct( this );
}

// // //// // //// // //// //
// GLSampler
// //// // //// // //// //

INLINE void GLSampler::SetParam( GLenum param, int value ) {
	GLImports::glSamplerParameteri( d_samplerObject, param, value );
}

INLINE void GLSampler::SetParam( GLenum param, float value ) {
	GLImports::glSamplerParameterf( d_samplerObject, param, value );
}

INLINE void GLSampler::SetParam( GLenum param, float * values ) {
	GLImports::glSamplerParameterfv( d_samplerObject, param, values );
}

INLINE bool GLSampler::operator ==( GLenum with ) const {
	return d_samplerType == with;
}

INLINE void GLSampler::SetTextureUnit( int texture_unit ) {
	d_textureUnit = texture_unit;
}

INLINE void GLSampler::SetUniformLocation( int uniform_location ) {
	d_uniformLocation = uniform_location;
}

INLINE int GLSampler::GetUniformLocation( ) const {
	return d_uniformLocation;
}

INLINE void GLSampler::ActivateTextureUnit( ) {
	GLImports::glActiveTexture( GL_TEXTURE0 + d_textureUnit );
}

INLINE void GLSampler::Bind( ) {
	GLImports::glUniform1i( d_uniformLocation, d_textureUnit );
	GLImports::glBindSampler( d_textureUnit, d_samplerObject );
}

INLINE void GLSampler::UnBind( ) {
	GLImports::glBindSampler( d_textureUnit, 0 );
}

// // //// // //// // //// //
// GLVertexArrayObject
// //// // //// // //// //

template< typename type >
GLVertexArrayObject< type >::GLVertexArrayObject( ) {
	GLImports::glGenVertexArrays( 1, &d_vertexArrayID );
}

template< typename type >
GLVertexArrayObject< type > &	GLVertexArrayObject< type >::operator =( const GLVertexArrayObject & other ) {
	GLImports::glGenVertexArrays( 1, &d_vertexArrayID );
	return *this;
}

template< typename type >
INLINE void GLVertexArrayObject< type >::Bind( ) {	
	GLImports::glBindVertexArray( d_vertexArrayID );
}

template< typename type >
void GLVertexArrayObject< type >::SetupBuffer( ) {
	for( GLuint i = 0; type::g_vertexBufferFormat[ i ].size; i++ ) {
		GLImports::glVertexAttribPointer( i, type::g_vertexBufferFormat[ i ].size, type::g_vertexBufferFormat[ i ].type,
			type::g_vertexBufferFormat[ i ].normalized, type::g_vertexBufferFormat[ i ].stride, type::g_vertexBufferFormat[ i ].pointer );
	}
	GLImports::glBindBuffer( GL_ARRAY_BUFFER, 0 );
	for( GLuint i = 0; type::g_vertexBufferFormat[ i ].size; i++ )
		GLImports::glEnableVertexAttribArray( i );
}

// // //// // //// // //// //
// GLShader
// //// // //// // //// //

INLINE void GLShader::Bind( ) {
#ifdef _DEBUG
	GLImports::GLErrorCheck( );
#endif
	GLImports::glUseProgram( d_program );
	for( int i = 0; i < d_inputSamplers.Num( ); i++ )
		d_inputSamplers[ i ]->Bind( );
}

INLINE void GLShader::UnBind( ) {
	for( int i = ( d_inputSamplers.Num( ) - 1 ); i >= 0; i-- )
		d_inputSamplers[ i ]->UnBind( );
	GLImports::glUseProgram( 0 );
#ifdef _DEBUG
	GLImports::GLErrorCheck( );
#endif
}

INLINE SVar & GLShader::FindInput( const Str & input_name ) {
	return *d_inputSVars.FindByName( input_name );
}

INLINE GLShader * GLShader::Find( const Str & shader_name ) {
	return g_shaderList.FindByName( shader_name );
}

INLINE int GLShader::GetNumShaders( ) {
	return g_shaderList.Num( );
}

INLINE GLShader * GLShader::GetShader( const int index ) {
	return g_shaderList[ index ];
}

// // //// // //// // //// //
// GLBufferPrimitive
// //// // //// // //// //

template< typename vertex_type, typename element_type >
INLINE void GLBufferPrimitive< vertex_type, element_type >::Draw( GLsizei primitive_count ) {
	GLImports::glDrawElementsInstanced( d_primitiveType, d_indexBlock->d_size, d_indexType, ( GLvoid * )d_indexBlock->d_offset, primitive_count );
}

// // //// // //// // //// //
// GLDynamicBufferState
// //// // //// // //// //

template< typename buffer_type, GLenum buffer_target, GLenum buffer_usage >
GLDynamicBufferState< buffer_type, buffer_target, buffer_usage >::GLDynamicBufferState( int granularity ) {
	d_bufferNum			= 0;
	d_bufferAllocSize	= 0;
	d_bufferCurrentSize	= 0;
	d_bufferGranularity	= granularity;
	GLImports::glGenBuffers( 1, &d_bufferID );
}

template< typename buffer_type, GLenum buffer_target, GLenum buffer_usage >
GLDynamicBufferState< buffer_type, buffer_target, buffer_usage >::~GLDynamicBufferState( ) {
	GLImports::glDeleteBuffers( 1, &d_bufferID );
}

template< typename buffer_type, GLenum buffer_target, GLenum buffer_usage >
void GLDynamicBufferState< buffer_type, buffer_target, buffer_usage >::ResizeBuffer( ) {
	GLvoid * tempMem = NULL;
	//GLScopedLock glLock;
	if( d_bufferCurrentSize ) {
		tempMem = Mem_Alloc( NumSizeOf( buffer_type, d_bufferAllocSize ) );
		GLvoid * bufferMap = GLImports::glMapBuffer( buffer_target, GL_READ_ONLY );
		Common::Com_Memcpy( tempMem, bufferMap, NumSizeOf( buffer_type, d_bufferCurrentSize ) );
		GLImports::glUnmapBuffer( buffer_target );
	}
	GLImports::glBufferData( buffer_target, NumSizeOf( buffer_type, d_bufferAllocSize ), tempMem, GL_STATIC_DRAW );
	d_bufferCurrentSize = d_bufferAllocSize;
	Mem_Free( tempMem );
}

template< typename buffer_type, GLenum buffer_target, GLenum buffer_usage >
INLINE void GLDynamicBufferState< buffer_type, buffer_target, buffer_usage >::Bind( ) {
	GLImports::glBindBuffer( buffer_target, d_bufferID );
}

template< typename buffer_type, GLenum buffer_target, GLenum buffer_usage >
INLINE void GLDynamicBufferState< buffer_type, buffer_target, buffer_usage >::UnBind( ) {
	GLImports::glBindBuffer( buffer_target, 0 );
}

template< typename buffer_type, GLenum buffer_target, GLenum buffer_usage >
GLDynamicBufferBlock< buffer_type > * GLDynamicBufferState< buffer_type, buffer_target, buffer_usage >::Alloc( int num ) {
	t_block * block = d_blockAllocator.Alloc( );
	t_freeAreaNode * freeArea = NULL;
	t_freeAreaNode * lastArea = d_freeTree.Begin( );
	for( t_freeAreaNode * treeItr = lastArea; treeItr; treeItr = d_freeTree.GetNextLeaf( treeItr ) ) {
		if( treeItr->d_key > lastArea->d_key )
			lastArea = treeItr;
		if( *treeItr->d_object < num )
			continue;
		if( !freeArea || *freeArea->d_object > *treeItr->d_object )
			freeArea = treeItr;
	}
	if( freeArea ) {
		int blockOffset = freeArea->d_key;
		int remainingArea = *freeArea->d_object - num;
		if( remainingArea ) {
			*freeArea->d_object = remainingArea;
			d_freeTree.Add( freeArea->d_object, blockOffset + NumSizeOf( buffer_type, num ) );
		} else
			d_keyAllocator.Free( freeArea->d_object );
		d_freeTree.Remove( freeArea );
		block->d_offset = blockOffset;
	} else {
		int newBlockOffset = d_bufferAllocSize;
		if( lastArea ) {
			if( lastArea->d_key == NumSizeOf( buffer_type, d_bufferAllocSize - *lastArea->d_object ) ) {
				newBlockOffset -= *lastArea->d_object;
				d_bufferAllocSize -= *lastArea->d_object;
				d_freeTree.Remove( lastArea );
			} else
				lastArea = NULL;
		}
		d_bufferAllocSize += num;
		d_bufferAllocSize += d_bufferGranularity - 1;
		d_bufferAllocSize -= d_bufferAllocSize % d_bufferGranularity;
		int remainingArea = d_bufferAllocSize - ( newBlockOffset + num );
		if( remainingArea ) {
			int * offsetSize = lastArea ? lastArea->d_object : d_keyAllocator.Alloc( );
			*offsetSize = remainingArea;
			d_freeTree.Add( offsetSize, NumSizeOf( buffer_type, newBlockOffset + num ) );
		} else if( lastArea )
			d_keyAllocator.Free( lastArea->d_object );
		ResizeBuffer( );
		block->d_offset = NumSizeOf( buffer_type, newBlockOffset );
	}
	block->d_data = ( buffer_type * )GLImports::glMapBufferRange( buffer_target, block->d_offset, NumSizeOf( buffer_type, num ), GL_MAP_WRITE_BIT );
	block->d_size = num;
	d_bufferNum += num;
	return block;
}

template< typename buffer_type, GLenum buffer_target, GLenum buffer_usage >
INLINE void GLDynamicBufferState< buffer_type, buffer_target, buffer_usage >::Remap( t_block * block ) {
	block->d_data = ( buffer_type * )GLImports::glMapBufferRange( buffer_target, block->d_offset, NumSizeOf( buffer_type, block->d_size ), GL_MAP_WRITE_BIT );
}

template< typename buffer_type, GLenum buffer_target, GLenum buffer_usage >
void GLDynamicBufferState< buffer_type, buffer_target, buffer_usage >::Free( t_block * block ) {
	t_freeAreaNode * prevArea = NULL;
	t_freeAreaNode * nextArea = NULL;
	for( t_freeAreaNode * treeItr = d_freeTree.Begin( ); treeItr; treeItr = d_freeTree.GetNextLeaf( treeItr ) ) {
		if( block->d_offset == ( treeItr->d_key + NumSizeOf( buffer_type, *treeItr->d_object ) ) )
			prevArea = treeItr;
		if( treeItr->d_key == ( block->d_offset + NumSizeOf( buffer_type, block->d_size ) ) )
			nextArea = treeItr;
		if( prevArea && nextArea )
			break;
	}
	if( prevArea ) {
		int freeSizeMerged = *prevArea->d_object + block->d_size;
		if( nextArea ) {
			freeSizeMerged += *nextArea->d_object;
			d_freeTree.Remove( nextArea );
			d_keyAllocator.Free( nextArea->d_object );
		}
		*prevArea->d_object = freeSizeMerged;
		d_freeTree.Add( prevArea->d_object, prevArea->d_key );
		d_freeTree.Remove( prevArea );
	} else if( nextArea ) {
		*nextArea->d_object += block->d_size;
		d_freeTree.Add( nextArea->d_object, block->d_offset );
		d_freeTree.Remove( nextArea );
	} else {
		int * offsetSize = d_keyAllocator.Alloc( );
		*offsetSize = block->d_size;
		d_freeTree.Add( offsetSize, block->d_offset );
	}
	d_blockAllocator.Free( block );
	d_bufferNum -= block->d_size;
}

template< typename buffer_type, GLenum buffer_target, GLenum buffer_usage >
INLINE int GLDynamicBufferState< buffer_type, buffer_target, buffer_usage >::Num( ) const {
	return d_bufferNum;
}

// // //// // //// // //// //
// GLDynamicBufferAllocator
// //// // //// // //// //

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
void GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::InitVAO( ) {
	d_vertexArrayObject.Bind( );
	d_stateVertex->Bind( );
	d_vertexArrayObject.SetupBuffer( );
	d_vertexArrayObject.UnBind( );
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::GLDynamicBufferAllocator( GLDynamicBufferAllocator * other ) {
	d_stateVertex = other->d_stateVertex;
	d_stateIndex = other->d_stateIndex;
	d_primitiveAllocator = other->d_primitiveAllocator;
	InitVAO( );
	d_isOwner = false;
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::GLDynamicBufferAllocator( ) {
	d_stateVertex			= new t_vertexState( );
	d_stateIndex			= new t_indexState( );
	InitVAO( );
	d_primitiveAllocator	= new BlockAlloc< t_primitive, 128 >( );
	d_isOwner = true;
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::~GLDynamicBufferAllocator( ) {
	if( d_isOwner ) {
		delete d_stateVertex;
		delete d_stateIndex;
		delete d_primitiveAllocator;
	}
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
INLINE GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage > * GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::ContextCreate( ) {
	return new GLDynamicBufferAllocator( this );
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
INLINE void GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::BeginDraw( ) {
	d_vertexArrayObject.Bind( );
	d_stateIndex->Bind( );
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
INLINE void GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::EndDraw( ) {
	d_vertexArrayObject.UnBind( );
	d_stateIndex->UnBind( );
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
INLINE GLDynamicBufferBlock< vertex_type > * GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::AllocVertex( int num ) {
	d_stateVertex->Bind( );
	return d_stateVertex->Alloc( num );
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
INLINE void GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::RemapVertex( t_vertexBlock * block ) {
	d_stateVertex->Bind( );
	d_stateVertex->Remap( block );
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
INLINE void GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::FreeVertex( t_vertexBlock * block ) {
	if( block )
		d_stateVertex->Free( block );
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
INLINE void GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::UploadVertex( t_vertexBlock * block ) {
	GLImports::glUnmapBuffer( GL_ARRAY_BUFFER );
	d_stateVertex->UnBind( );
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
INLINE int GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::NumVertex( ) const {
	return d_stateVertex->Num( );
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
INLINE GLDynamicBufferBlock< element_type > * GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::AllocIndex( int num ) {
	d_stateIndex->Bind( );
	return d_stateIndex->Alloc( num );
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
INLINE void GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::RemapIndex( t_indexBlock * block ) {
	d_stateIndex->Bind( );
	d_stateIndex->Remap( block );
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
INLINE void GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::FreeIndex( t_indexBlock * block ) {
	if( block )
		d_stateIndex->Free( block );
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
INLINE void GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::UploadIndex( t_indexBlock * block ) {
	GLImports::glUnmapBuffer( GL_ELEMENT_ARRAY_BUFFER );
	d_stateIndex->UnBind( );
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
INLINE int GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::NumIndex( ) const {
	return d_stateIndex->Num( );
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
GLBufferPrimitive< vertex_type, element_type > * GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::TexturePrimitive( const Bounds2D & bounds, bool rotate ) {
	Vec2 oneNull( 1.0f, 0.0f );
	Vec2 nullOne( 0.0f, 1.0f );
	Vec2 oneOne( 1.0f, 1.0f );
	VertexUI quadTexCoords[ ] = {
		bounds[ 0 ],								rotate ? vec2_origin : nullOne,	color_white,
		Vec2( bounds[ 1 ][ 0 ], bounds[ 0 ][ 1 ] ),	rotate ? nullOne : oneOne,		color_white,
		bounds[ 1 ],								rotate ? oneOne : oneNull,		color_white,
		Vec2( bounds[ 0 ][ 0 ], bounds[ 1 ][ 1 ] ),	rotate ? oneNull : vec2_origin,	color_white
	};
	t_primitive * primitive = d_primitiveAllocator->Alloc( );
	primitive->d_vertexBlock = AllocVertex( 4 );
	Common::Com_Memcpy( primitive->d_vertexBlock->d_data, quadTexCoords, NumSizeOf( vertex_type, 4 ) );
	UploadVertex( primitive->d_vertexBlock );
	element_type vertexStart = ( element_type )primitive->d_vertexBlock->ObjectNum( );
	element_type quadElements[ ] = { 0, 1, 2, 2, 3, 0 };
	primitive->d_indexBlock = AllocIndex( 6 );
	for( int i = 0; i < 6; i++ )
		( *primitive->d_indexBlock )[ i ] = quadElements[ i ] + vertexStart;
	UploadIndex( primitive->d_indexBlock );
	primitive->d_primitiveType = GL_TRIANGLES;
	primitive->d_indexType = GLType< element_type >::GetTypeEnum( );
	return primitive;
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
GLBufferPrimitive< vertex_type, element_type > * GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::TexturePrimitive( const RectTi & rect, bool rotate ) {
	const Vec2 oneNull( 1.0f, 0.0f );
	const Vec2 nullOne( 0.0f, 1.0f );
	const Vec2 oneOne( 1.0f, 1.0f );
	float minX = (float)( rect.GetCenter( )[0]-rect.GetExtents( )[0] );
	float minY = (float)( rect.GetCenter( )[1]-rect.GetExtents( )[1] );
	float maxX = (float)( rect.GetCenter( )[0]+rect.GetExtents( )[0] );
	float maxY = (float)( rect.GetCenter( )[1]+rect.GetExtents( )[1] );
	VertexUI quadTexCoords[ ] = {
		Vec2( minX, minY ),		rotate ? vec2_origin : nullOne,	color_white,
		Vec2( maxX, minY ),		rotate ? nullOne : oneOne,		color_white,
		Vec2( maxX, maxY ),		rotate ? oneOne : oneNull,		color_white,
		Vec2( minX, maxY ),		rotate ? oneNull : vec2_origin,	color_white
	};
	t_primitive * primitive = d_primitiveAllocator->Alloc( );
	primitive->d_vertexBlock = AllocVertex( 4 );
	Common::Com_Memcpy( primitive->d_vertexBlock->d_data, quadTexCoords, NumSizeOf( vertex_type, 4 ) );
	UploadVertex( primitive->d_vertexBlock );
	element_type vertexStart = ( element_type )primitive->d_vertexBlock->ObjectNum( );
	element_type quadElements[ ] = { 0, 1, 2, 2, 3, 0 };
	primitive->d_indexBlock = AllocIndex( 6 );
	for( int i = 0; i < 6; i++ )
		( *primitive->d_indexBlock )[ i ] = quadElements[ i ] + vertexStart;
	UploadIndex( primitive->d_indexBlock );
	primitive->d_primitiveType = GL_TRIANGLES;
	primitive->d_indexType = GLType< element_type >::GetTypeEnum( );
	return primitive;
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
GLBufferPrimitive< vertex_type, element_type > * GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::TexturePrimitive( const VecT2i & size ) {
	const Vec2 oneNull( 1.0f, 0.0f );
	const Vec2 nullOne( 0.0f, 1.0f );
	const Vec2 oneOne( 1.0f, 1.0f );
	float maxX = (float)size[0];
	float maxY = (float)size[1];
	VertexUI quadTexCoords[ ] = {
		Vec2( 0, 0 ),		nullOne,		color_white,
		Vec2( maxX, 0 ),	oneOne,			color_white,
		Vec2( maxX, maxY ),	oneNull,		color_white,
		Vec2( 0, maxY ),	vec2_origin,	color_white
	};
	t_primitive * primitive = d_primitiveAllocator->Alloc( );
	primitive->d_vertexBlock = AllocVertex( 4 );
	Common::Com_Memcpy( primitive->d_vertexBlock->d_data, quadTexCoords, NumSizeOf( vertex_type, 4 ) );
	UploadVertex( primitive->d_vertexBlock );
	element_type vertexStart = ( element_type )primitive->d_vertexBlock->ObjectNum( );
	element_type quadElements[ ] = { 0, 1, 2, 2, 3, 0 };
	primitive->d_indexBlock = AllocIndex( 6 );
	for( int i = 0; i < 6; i++ )
		( *primitive->d_indexBlock )[ i ] = quadElements[ i ] + vertexStart;
	UploadIndex( primitive->d_indexBlock );
	primitive->d_primitiveType = GL_TRIANGLES;
	primitive->d_indexType = GLType< element_type >::GetTypeEnum( );
	return primitive;
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
GLBufferPrimitive< vertex_type, element_type > * GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::ColorRectPrimitive( const Bounds2D & bounds, const Color & color ) {
	VertexColorOnlyUI quadTexCoords[ ] = {
		bounds[ 0 ], color, Vec2( bounds[ 1 ][ 0 ], bounds[ 0 ][ 1 ] ), color,
		bounds[ 1 ], color, Vec2( bounds[ 0 ][ 0 ], bounds[ 1 ][ 1 ] ), color
	};
	t_primitive * primitive = d_primitiveAllocator->Alloc( );
	primitive->d_vertexBlock = AllocVertex( 4 );
	Common::Com_Memcpy( primitive->d_vertexBlock->d_data, quadTexCoords, NumSizeOf( vertex_type, 4 ) );
	UploadVertex( primitive->d_vertexBlock );
	element_type vertexStart = ( element_type )primitive->d_vertexBlock->ObjectNum( );
	element_type quadElements[ ] = { 0, 1, 2, 2, 3, 0 };
	primitive->d_indexBlock = AllocIndex( 6 );
	for( int i = 0; i < 6; i++ )
		( *primitive->d_indexBlock )[ i ] = quadElements[ i ] + vertexStart;
	UploadIndex( primitive->d_indexBlock );
	primitive->d_primitiveType = GL_TRIANGLES;
	primitive->d_indexType = GLType< element_type >::GetTypeEnum( );
	return primitive;
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
GLBufferPrimitive< vertex_type, element_type > * GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::SurfacePrimitive( const Surface & surface ) {
	t_primitive * primitive = d_primitiveAllocator->Alloc( );
	primitive->d_vertexBlock = AllocVertex( surface.GetNumVertices( ) );
	Common::Com_Memcpy( primitive->d_vertexBlock->d_data, surface.GetVertices( ), NumSizeOf( vertex_type,  surface.GetNumVertices( ) ) );
	UploadVertex( primitive->d_vertexBlock );
	element_type vertexStart = ( element_type )primitive->d_vertexBlock->ObjectNum( );
	primitive->d_indexBlock = AllocIndex( surface.GetNumIndexes( ) );
	for( int i = 0; i < surface.GetNumIndexes( ); i++ )
		( *primitive->d_indexBlock )[ i ] = surface.GetIndexes( )[ i ] + vertexStart;
	UploadIndex( primitive->d_indexBlock );
	primitive->d_primitiveType = GL_TRIANGLES;
	primitive->d_indexType = GLType< element_type >::GetTypeEnum( );
	return primitive;
}

template< typename vertex_type, GLenum vertex_usage, typename element_type, GLenum element_usage >
void GLDynamicBufferAllocator< vertex_type, vertex_usage, element_type, element_usage >::FreePrimitive( t_primitive * primitive ) {
	if( primitive ) {
		d_stateVertex->Free( primitive->d_vertexBlock );
		d_stateIndex->Free( primitive->d_indexBlock );
		d_primitiveAllocator->Free( primitive );
	}
}

#endif
