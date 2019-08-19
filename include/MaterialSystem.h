#ifndef MATERIALSYSTEM_H
#define MATERIALSYSTEM_H

const int PIXEL_DIMENSIONS = 3;

// // //// // //// // //// //
/// ImagePixel
// //// // //// // //// //

class ImagePixel {
public:
	byte											x[ PIXEL_DIMENSIONS ];
	int												operator -( const ImagePixel & p2 ) const;
};

// // //// // //// // //// //
/// ImageBlock
// //// // //// // //// //

class ImageBlock {
private:
	ImagePixel										d_minCorner;
	ImagePixel										d_maxCorner;
	ImagePixel *									d_points;
	int												d_pointsLength;

public:
													ImageBlock( ) { }
													ImageBlock( ImagePixel * points, int pointsLength );

	ImagePixel *									GetPoints( );
	int												NumPoints( ) const;
	int												LongestSideIndex( ) const;
	int												LongestSideLength( ) const;
	void											Shrink( );

	bool											operator <( const ImageBlock & rhs ) const;
	int												operator -( const ImageBlock & other ) const;
};

// // //// // //// // //// //
/// ImagePixelCoordComp
// //// // //// // //// //

template< int index > class ImagePixelCoordComp {
public:
	bool											operator( )( const ImagePixel & left, const ImagePixel & right );
};

// // //// // //// // //// //
/// ImagePalette
// //// // //// // //// //

class ImagePalette {
private:
	CAList< SSE_Data, 128 >							d_paletteMemList;
	SSE_Data										d_imageMem;

public:
													ImagePalette( const CAListBase< ImagePixel > & palette );

	void											LookupLine( const byte * input, bool alpha, CAListBase< byte > & output );
};

// surface description flags
const unsigned long DDSF_CAPS           = 0x00000001l;
const unsigned long DDSF_HEIGHT         = 0x00000002l;
const unsigned long DDSF_WIDTH          = 0x00000004l;
const unsigned long DDSF_PITCH          = 0x00000008l;
const unsigned long DDSF_PIXELFORMAT    = 0x00001000l;
const unsigned long DDSF_MIPMAPCOUNT    = 0x00020000l;
const unsigned long DDSF_LINEARSIZE     = 0x00080000l;
const unsigned long DDSF_DEPTH          = 0x00800000l;
// pixel format flags
const unsigned long DDSF_ALPHAPIXELS    = 0x00000001l;
const unsigned long DDSF_FOURCC         = 0x00000004l;
const unsigned long DDSF_RGB            = 0x00000040l;
const unsigned long DDSF_RGBA           = 0x00000041l;
// dwCaps1 flags
const unsigned long DDSF_COMPLEX         = 0x00000008l;
const unsigned long DDSF_TEXTURE         = 0x00001000l;
const unsigned long DDSF_MIPMAP          = 0x00400000l;

#define DDS_MAKEFOURCC(a, b, c, d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))

typedef struct {
	unsigned long dwSize;
	unsigned long dwFlags;
	unsigned long dwFourCC;
	unsigned long dwRGBBitCount;
	unsigned long dwRBitMask;
	unsigned long dwGBitMask;
	unsigned long dwBBitMask;
	unsigned long dwABitMask;
} ddsFilePixelFormat_t;

typedef struct {
	unsigned long dwFourCC;
	unsigned long dwSize;
	unsigned long dwFlags;
	unsigned long dwHeight;
	unsigned long dwWidth;
	unsigned long dwPitchOrLinearSize;
	unsigned long dwDepth;
	unsigned long dwMipMapCount;
	unsigned long dwReserved1[11];
	ddsFilePixelFormat_t ddspf;
	unsigned long dwCaps1;
	unsigned long dwCaps2;
	unsigned long dwReserved2[3];
} ddsFileHeader_t;

// // //// // //// // //// //
/// Image
//
/// Handle image loading and conversion for GL's texture format
// //// // //// // //// //

class Image : public NamedObject {
	friend class									TextureSimple;
	
private:
	CAList< byte >									d_data;
	VecT2i											d_size;
	bool											d_alpha;
	bool											d_dxt;
	int												d_numMips;
	GLenum											d_internalFormat;
	GLenum											d_format;

	bool											LoadTGA( const void * data_ptr, size_t data_size );
	bool											LoadPNG( const void * data_ptr, size_t data_size );
	bool											LoadDXT( const void * data_ptr, size_t data_size );

	bool											SavePNG( const Str & file_name );

	void											MakePalette( CAListBase< ImagePixel > & result );
	
public:
													Image( );
													~Image( );

	byte *											Alloc( int width, int height, bool alpha );

	bool											Load( const Str & filename );
	bool											Save( const Str & filename );
	void											LoadMem( const void * data_ptr, size_t data_size, const Str & image_type );
	void											LoadRaw( const void * data_ptr, size_t data_size, const Str & image_type, const VecT2i & image_size, bool image_flip );
	const VecT2i &									GetSize( ) const	{ return d_size; }
	void *											GetPtr( )			{ return d_data.Ptr( ); }
	bool											Alpha( ) const		{ return d_alpha; }
	bool											IsDXT( ) const		{ return d_dxt; }
	GLenum											GetInternalFormat( ) const	{ return d_internalFormat; }
	GLenum											GetFormat( ) const	{ return d_format; }
	void											Crop( const BoundsT2i & region );
	void											Flip( );
	void											Flop( );

	/// fills non-alpha type image with solid transparent data
	void											ForceAlpha( );

	void											GLTexUpLoad( GLenum tex_target ) const;
};

// // //// // //// // //// //
/// TextureBase
//
/// Base class for all texture, implements transparent code between single and multiple (allocated) textures
// //// // //// // //// //

class TextureBase : public NamedObject, public GLTexture {
private:
	Image *											d_image;
	Str												d_tag;

public:
													TextureBase( const Str & name, const Str & tag, Image * image );

	Image *											GetImage( );
	const Str &										GetTag( ) const;
};

// // //// // //// // //// //
/// DynamicTexture
// //// // //// // //// //

class DynamicTexture : public GLTexture {
private:
	Mat3											d_relocationMatrix;
	RectTi											d_rect;
	bool											d_90ccw;

public:
													DynamicTexture( );
													DynamicTexture( GLTexture * frame_texture, const RectTi & frame_rect );

	void											SetupSplit( const VecT2i & frame_size, const VecT2i & image_size, CAListBase< RectTi > & remainders );

	const RectTi &									GetRect( ) const;
	VecT2i											GetSize( ) const;
	bool											Is90ccw( ) const;
	Vec2											RemapTexCoord( const Vec2 & src ) const;
};

// // //// // //// // //// //
/// Overlay
// //// // //// // //// //

class Overlay {
private:
	typedef BTreeNode< DynamicTexture, RectTi >		node_t;
	BTree< DynamicTexture, RectTi, 4 >				d_freeAreas;
	VecT2i											d_size;
	GLShader *										d_shader;
	GLFrameBuffer *									d_frameBuffer;
	GLTexture *										d_frameTexture;
	GLTexture *										d_uploadTexture;
	Projection										d_projectionMatrix;
	CAList< DynamicTexture *, 256 >					d_textureList;
	BlockAlloc< DynamicTexture, 256 >				d_dataAllocator;

	void											AddFreeTexture( const RectTi & texture_rect );

public:
													Overlay( );
													~Overlay( );

	DynamicTexture *								LoadFromImage( Image * image );
	DynamicTexture *								LoadFromFile( const Str & file_name );
	void											FreeTexture( DynamicTexture * texture );

	GLFrameBuffer *									GetFrameBuffer( );
	GLShader *										GetShader( );
};

// // //// // //// // //// //
/// RenderSurface
// //// // //// // //// //

template< typename allocator_type > class RenderSurface {
private:
	typedef typename allocator_type::t_indexType	t_indexType;
	typedef typename allocator_type::t_vertexBlock	t_vertexBlock;
	typedef typename allocator_type::t_indexBlock	t_indexBlock;

	GLShader *										d_surfaceShader;
	CANamedList< SVar *, 16 >						d_surfaceParams;
	CAList< t_indexType, 256 >						d_surfaceIndexes;
	t_indexType										d_surfaceBaseVertex;
	t_indexType										d_surfaceBaseIndex;
	t_vertexBlock *									d_surfaceVertexBlock;
	t_indexBlock *									d_surfaceIndexBlock;
	allocator_type *								d_surfaceAllocator;

	int												d_renderClass;

public:
													RenderSurface( GLShader * surface_shader, allocator_type * gl_buffer, Surface * surface_data );
													RenderSurface( GLShader * surface_shader, allocator_type * gl_buffer, const class MapBrush & brush, const class MapBrushSide & side );
													RenderSurface( GLShader * surface_shader, allocator_type * gl_buffer, const class MD5Mesh & mesh_data );
													~RenderSurface( );

	void											AddParam( SVar * param );
	int												IndexNum( ) const		{ return d_surfaceIndexBlock->d_size; }
	int												VertexNum( ) const		{ return d_surfaceVertexBlock->d_size; }
	t_indexType										BaseVertex( ) const		{ return d_surfaceBaseVertex; }
	t_indexType										BaseIndex( ) const		{ return d_surfaceBaseIndex; }
	void											ComputeIndexes( );

	t_indexBlock *									BeginRemapIndex( )		{ d_surfaceAllocator->RemapIndex( d_surfaceIndexBlock ); return d_surfaceIndexBlock; }
	void											EndRemapIndex( )		{ d_surfaceAllocator->UploadIndex( d_surfaceIndexBlock ); }
	t_vertexBlock *									BeginRemapVertex( )		{ d_surfaceAllocator->RemapVertex( d_surfaceVertexBlock ); return d_surfaceVertexBlock; }
	void											EndRemapVertex( )		{ d_surfaceAllocator->UploadVertex( d_surfaceVertexBlock ); }

	int												GetRenderClass( ) const	{ return d_renderClass; }
	void											SetRenderClass( int class_num );
	bool											RenderableWith( RenderSurface * other_surface );
	GLShader *										GetShader( );
	void											UpdateShaderParams( GLShader * renderer_shader );

	static int										ShaderAndSVarCmp( RenderSurface< allocator_type > * const * a, RenderSurface< allocator_type > * const * b );
	static int										ShaderAndSVarAndIndexCmp( RenderSurface< allocator_type > * const * a, RenderSurface< allocator_type > * const * b );
};

typedef RenderSurface< BufferAllocatorMap >			RenderSurfaceMap;
typedef RenderSurface< BufferAllocatorModel >		RenderSurfaceModel;

// // //// // //// // //// //
/// MaterialEntry
// //// // //// // //// //

class MaterialEntry : public NamedObject {
private:
	TextureBase *									d_entryTexture;
	SVar &											d_entrySVar;

public:
													MaterialEntry( const Str & entry_name, SVar & entry_svar, TextureBase * entry_text );
													~MaterialEntry( );

	TextureBase *									GetTexture( )		{ return d_entryTexture; }
	SVar &											GetSVar( )			{ return d_entrySVar; }
};

// // //// // //// // //// //
/// Material
// //// // //// // //// //

class Material : public NamedObject {
private:
	CANamedList< MaterialEntry *, 16 >				d_entries;
	GLShader *										d_matShader;

public:
													Material( const Str & mat_name );
													~Material( );

	void											AddEntry( const Str & entry_name, TextureBase * entry_text );
	MaterialEntry &									operator[ ]( const int index )		{ return *d_entries[ index ]; }
	int												GetNumEntries( ) const				{ return d_entries.Num( ); }

	void											SetShader( GLShader * mat_shader )	{ d_matShader = mat_shader; }
	GLShader *										GetShader( )						{ return d_matShader; }

	void											ConfigRenderSurface( RenderSurfaceMap * ren_surf );
	void											ConfigRenderSurface( RenderSurfaceModel * ren_surf );
};

// // //// // //// // //// //
/// MaterialSystem
// //// // //// // //// //

class MaterialSystem {
private:
	static TextureBase *							g_missingTexture;

	static CANamedList< TextureBase *, 256 >		g_loadedTextures;
	static CANamedList< Image *, 256 >				g_loadedImages;
	static CANamedList< Material *, 256 >			g_loadedMaterials;

public:
	static Image *									FindImage( const Str & filename );

	static TextureBase *							LoadTexture( Image * image, const Str & tag );
	static TextureBase *							FindTexture( const Str & filename, const Str & tag );

	static Material *								FindMaterial( const Str & mat_name );

	static void										Init( );
	static void										Shutdown( );
};

// // //// // //// // //// //
// ImagePixel
// //// // //// // //// //

INLINE int ImagePixel::operator -( const ImagePixel & p2 ) const {
	return ( (int)(x[0]) - (int)(p2.x[0]) ) + ( (int)(x[1]) - (int)(p2.x[1]) ) + ( (int)(x[2]) - (int)(p2.x[2]) );
}

// // //// // //// // //// //
// ImageBlock
// //// // //// // //// //

INLINE ImagePixel * ImageBlock::GetPoints( ) {
	return d_points;
}

INLINE int ImageBlock::NumPoints( ) const {
	return d_pointsLength;
}

INLINE int ImageBlock::LongestSideLength( ) const {
	int i = LongestSideIndex( );
	return d_maxCorner.x[ i ] - d_minCorner.x[ i ];
}

INLINE bool ImageBlock::operator <( const ImageBlock & rhs ) const {
	return LongestSideLength( ) < rhs.LongestSideLength( );
}

INLINE int ImageBlock::operator -( const ImageBlock & other ) const {
	return LongestSideLength( ) - other.LongestSideLength( );
}

// // //// // //// // //// //
// ImagePixelCoordComp
// //// // //// // //// //

template< int index >
INLINE bool ImagePixelCoordComp< index >::operator( )( const ImagePixel & left, const ImagePixel & right ) {
	return left.x[ index ] < right.x[ index ];
}

// // //// // //// // //// //
// TextureBase
// //// // //// // //// //

INLINE Image * TextureBase::GetImage( ) {
	return d_image;
}

INLINE const Str & TextureBase::GetTag( ) const {
	return d_tag;
}

// // //// // //// // //// //
// RenderSurface
// //// // //// // //// //

template< typename allocator_type >
RenderSurface< allocator_type >::~RenderSurface( ) {
	d_surfaceParams.DeleteContents( true );
	d_surfaceAllocator->FreeIndex( d_surfaceIndexBlock );
	d_surfaceAllocator->FreeVertex( d_surfaceVertexBlock );
}

template< typename allocator_type >
INLINE void RenderSurface< allocator_type >::AddParam( SVar * param ) {
	d_surfaceParams.Append( param );
	d_surfaceParams.Sort( SVar::TypeAndNameCompare );
}

template< typename allocator_type >
INLINE void RenderSurface< allocator_type >::SetRenderClass( int class_num ) {
	d_renderClass = class_num;
}

template< typename allocator_type >
INLINE bool RenderSurface< allocator_type >::RenderableWith( RenderSurface * other_surface ) {
	return d_renderClass == other_surface->d_renderClass;
}

template< typename allocator_type >
INLINE GLShader * RenderSurface< allocator_type >::GetShader( ) {
	return d_surfaceShader;
}

template< typename allocator_type >
INLINE void RenderSurface< allocator_type >::UpdateShaderParams( GLShader * renderer_shader ) {
	for( int i = 0; i < d_surfaceParams.Num( ); i++ ) {
		SVar * sParam = d_surfaceParams[ i ];
		renderer_shader->FindInput( sParam->GetName( ) ) = *sParam;
	}
}

// // //// // //// // //// //
// DynamicTexture
// //// // //// // //// //

INLINE const RectTi & DynamicTexture::GetRect( ) const {
	return d_rect;
}

INLINE VecT2i DynamicTexture::GetSize( ) const {
	return d_rect.GetExtents( ) * 2.0f;
}

INLINE bool DynamicTexture::Is90ccw( ) const {
	return d_90ccw;
}

INLINE Vec2 DynamicTexture::RemapTexCoord( const Vec2 & src ) const {
	Vec3 src3remapped = d_relocationMatrix * Vec3( src[ 0 ], src[ 1 ], 1.0f );
	return Vec2( src3remapped[ 0 ], src3remapped[ 1 ] );
}

// // //// // //// // //// //
// Overlay
// //// // //// // //// //

INLINE void Overlay::AddFreeTexture( const RectTi & texture_rect ) {
	DynamicTexture * newData = d_dataAllocator.Alloc( );
	new ( newData )( DynamicTexture )( d_frameTexture, texture_rect );
	d_freeAreas.Add( newData, texture_rect );
}

INLINE GLFrameBuffer * Overlay::GetFrameBuffer( ) {
	return d_frameBuffer;
}

INLINE GLShader * Overlay::GetShader( ) {
	return d_shader;
}

#endif
