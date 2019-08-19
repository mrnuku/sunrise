#ifndef GUIRENDERER_H
#define GUIRENDERER_H

namespace CEGUI {

// // //// // //// // //// //
/// GUITexture
// //// // //// // //// //

class GUITexture : public Texture, public DynamicTexture {
private:
	Size											d_size;			// Size of the texture.
	Size											d_dataSize;		// original pixel of size data loaded into texture
	Vector2											d_texelScaling;	// cached pixel to texel mapping scale values.

	void											updateCachedScaleValues( );

public:
	void											setAllSize( const Size & size );
	void											setSize( const Size & size );

													GUITexture( );
	virtual											~GUITexture( );

	// implement CEGUI::Texture interface
	virtual const Size &							getSize( ) const;
	virtual const Size &							getOriginalDataSize( ) const;
	virtual const Vector2 &							getTexelScaling( ) const;
	virtual void									loadFromFile( const String & filename, const String & resourceGroup );
	virtual void									loadFromMemory( const void * buffer, const Size & buffer_size, PixelFormat pixel_format );
	virtual void									saveToMemory( void * buffer );
};

// // //// // //// // //// //
/// GUIGeometryBuffer
// //// // //// // //// //

class GUIGeometryBuffer : public GeometryBuffer {
private:
	GUITexture *									d_activeTexture;	// Texture that is set as active
	Rect											d_clipRect;			// rectangular clip region
	Vec3											d_translation;		// translation vector
	Vec3											d_rotation;			// rotation vector
	Vec3											d_pivot;			// pivot point for rotation
	RenderEffect *									d_effect;			// RenderEffect that will be used by the GeometryBuffer
	mutable CAList< BufferAllocatorUI::t_vertexBlock *, 256 >	d_vertexRangeList;
	mutable CAList< BufferAllocatorUI::t_indexBlock *, 256 >		d_indexRangeList;
	mutable bool									d_matrixRecompute;
	mutable Projection								d_matrix;

	void											recomputeMatrix( ) const;

public:
													GUIGeometryBuffer( );
	virtual											~GUIGeometryBuffer( );

	// implement CEGUI::GeometryBuffer interface.
	virtual void									draw( ) const;
	virtual void									setTranslation( const Vector3 & v );
	virtual void									setRotation( const Vector3 & r );
	virtual void									setPivot( const Vector3 & p );
	virtual void									setClippingRegion( const Rect & region );
	virtual void									appendVertex( const Vertex & vertex );
	virtual void									appendGeometry( const Vertex * const vbuff, uint vertex_count );
	virtual void									setActiveTexture( Texture * texture );
	virtual void									reset( );
	virtual Texture *								getActiveTexture( ) const;
	virtual uint									getVertexCount( ) const;
	virtual uint									getBatchCount( ) const;
	virtual void									setRenderEffect( RenderEffect * effect );
	virtual RenderEffect *							getRenderEffect( );
};

// // //// // //// // //// //
/// GUIRenderTarget
// //// // //// // //// //

class GUIRenderTarget : public RenderTarget {
protected:
	Rect											d_area;			// holds defined area for the RenderTarget
	GLFrameBuffer *									d_frameBuffer;
	bool											d_ownCreation;

public:
													GUIRenderTarget( );
													GUIRenderTarget( GLFrameBuffer * frame_buffer );
	virtual											~GUIRenderTarget( );

	// implement parts of CEGUI::RenderTarget interface
	virtual void									draw( const GeometryBuffer & buffer );
	virtual void									draw( const RenderQueue & queue );
	virtual void									setArea( const Rect & area );
	virtual const Rect &							getArea( ) const;
	virtual void									activate( );
	virtual void									deactivate( );
	virtual void									unprojectPoint( const GeometryBuffer & buff, const Vector2 & p_in, Vector2 & p_out ) const;
	virtual bool									isImageryCache( ) const;
};

// // //// // //// // //// //
/// GUITextureTarget
// //// // //// // //// //

class GUITextureTarget : public GUIRenderTarget, public TextureTarget {
private:
	GUITexture *									d_CEGUITexture;		// This wraps d_texture so it can be used by the core CEGUI lib.
	bool											d_doClear;

public:
													GUITextureTarget( );
	virtual											~GUITextureTarget( );

	// implementation of RenderTarget interface
	virtual void									draw( const GeometryBuffer & buffer );
	virtual void									draw( const RenderQueue & queue );
	virtual void									setArea( const Rect & area );
	virtual const Rect &							getArea( ) const;
	virtual void									activate( );
	virtual void									deactivate( );
	virtual void									unprojectPoint( const GeometryBuffer & buff, const Vector2 & p_in, Vector2 & p_out ) const;
	virtual bool									isImageryCache( ) const;

	// implement CEGUI::TextureTarget interface.
	virtual void									clear( );
	virtual Texture &								getTexture( ) const;
	virtual void									declareRenderSize( const Size & sz );
	virtual bool									isRenderingInverted( ) const;
};

// // //// // //// // //// //
/// GUIFont
// //// // //// // //// //

class GUIFont : public Font {
private:
	DWFontRenderer *								d_renderer;
	float											d_pointSize;

protected:
	virtual void									rasterise( utf32 start_codepoint, utf32 end_codepoint ) const;
	virtual void									updateFont( );
	virtual void									writeXMLToStream_impl( XMLSerializer & xml_stream ) const;

public:
													GUIFont( const String & font_name, const float point_size );
	virtual											~GUIFont( );

	float											getPointSize( ) const;
	void											setPointSize( const float point_size );
	bool											isAntiAliased( ) const;
};

// // //// // //// // //// //
/// GUIRenderer
// //// // //// // //// //

class GUIRenderer : public Renderer {
private:
	String											d_rendererID;		// String holding the renderer identification text.
	Size											d_displaySize;		// What the renderer considers to be the current display size.
	Vector2											d_displayDPI;		// What the renderer considers to be the current display DPI resolution.
	RenderingRoot *									d_defaultRoot;		// The default rendering root object
	GUIRenderTarget *								d_defaultTarget;	// The default RenderTarget (used by d_defaultRoot)
	List< GUITextureTarget * >						d_textureTargets;	// Container used to track texture targets.
	List< GUIGeometryBuffer * >						d_geometryBuffers;	// Container used to track geometry buffers.
	List< GUITexture * >							d_textures;			// Container used to track textures.
	uint											d_maxTextureSize;	// What the renderer thinks the max texture size is.

public:
													GUIRenderer( );
	virtual											~GUIRenderer( );

	// implement CEGUI::Renderer interface
	virtual RenderingRoot &							getDefaultRenderingRoot( );
	virtual GeometryBuffer &						createGeometryBuffer( );
	virtual void									destroyGeometryBuffer( const GeometryBuffer & buffer );
	virtual void									destroyAllGeometryBuffers( );
	virtual TextureTarget *							createTextureTarget( );
	virtual void									destroyTextureTarget( TextureTarget * target );
	virtual void									destroyAllTextureTargets( );
	virtual Texture &								createTexture( );
	virtual Texture &								createTexture( const String & filename, const String & resourceGroup );
	virtual Texture &								createTexture( const Size & size );
	virtual void									destroyTexture( Texture & texture );
	virtual void									destroyAllTextures( );
	virtual void									beginRendering( );
	virtual void									endRendering( );
	virtual void									setDisplaySize( const Size & sz );
	virtual const Size &							getDisplaySize( ) const;
	virtual const Vector2 &							getDisplayDPI( ) const;
	virtual uint									getMaxTextureSize( ) const;
	virtual const String &							getIdentifierString( ) const;
	virtual void									setupRenderingBlendMode( const BlendMode mode );
};

// // //// // //// // //// //
// GUITexture
// //// // //// // //// //

INLINE void GUITexture::setAllSize( const Size & size ) {
	d_dataSize = d_size = size;
	d_texelScaling = Vector2( 1.0f / size.d_width, 1.0f / size.d_height );
}

INLINE void GUITexture::setSize( const Size & size ) {
	d_size = size;
	d_texelScaling = Vector2( 1.0f / size.d_width, 1.0f / size.d_height );
}

// // //// // //// // //// //
// GUIFont
// //// // //// // //// //

INLINE float GUIFont::getPointSize( ) const {
	return d_pointSize;
}

INLINE void GUIFont::setPointSize( const float point_size ) {
	d_pointSize = point_size;
}

INLINE bool GUIFont::isAntiAliased( ) const {
	return true;
}

}

#endif
