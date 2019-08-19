#ifndef RENDERER_H
#define RENDERER_H

// // //// // //// // //// //
/// PerlinNoise
// //// // //// // //// //

template< int static_size, class math_type > class PerlinNoise {
private:
	int									d_size;
	int									d_mask;
	CAList< int, static_size >			d_permutationTable;
	CAList< math_type, static_size >	d_gradientX;
	CAList< math_type, static_size >	d_gradientY;
	CAList< math_type, static_size >	d_gradientZ;
	Mat3								d_rotation;

public:
										PerlinNoise( );

	void								ReSize( int new_size );
	void								SetRotation( const Vec3 & axis, float r );

	math_type							Noise( float coord ) const;
	math_type							Noise( const Vec2 & coord ) const;
	math_type							Noise( const Vec3 & coord ) const;

	void								FillTexture( GLTexture * texture ) const;
	void								FillTexture( GLTexture * texture, const VecT2i & size ) const;
};

template< int static_size, class math_type >
PerlinNoise< static_size, math_type >::PerlinNoise( ) {
	ReSize( static_size );
	SetRotation( Vec3( 0.16f, 0.67f, 0.43f ), 0.34521f );
}

template< int static_size, class math_type >
void PerlinNoise< static_size, math_type >::SetRotation( const Vec3 & axis, float r ) {
	Vec3 axn = axis;
	axn.Normalize( );
	// Compute the rotation matrix  
	float c = cosf( r );
	float s = sinf( r );
	float c1 = 1.0f - c;
	Vec3 axns = axn * s;
	d_rotation[ 0 ][ 0 ] = ( axn.x * axn.x ) * c1 + c;
	d_rotation[ 0 ][ 1 ] = ( axn.y * axn.x ) * c1 + axns.z;
	d_rotation[ 0 ][ 2 ] = ( axn.z * axn.x ) * c1 - axns.y;
	d_rotation[ 1 ][ 0 ] = ( axn.x * axn.y ) * c1 - axns.z;
	d_rotation[ 1 ][ 1 ] = ( axn.y * axn.y ) * c1 + c;
	d_rotation[ 1 ][ 2 ] = ( axn.z * axn.y ) * c1 + axns.x;
	d_rotation[ 2 ][ 0 ] = ( axn.x * axn.z ) * c1 + axns.y;
	d_rotation[ 2 ][ 1 ] = ( axn.y * axn.z ) * c1 - axns.x;
	d_rotation[ 2 ][ 2 ] = ( axn.z * axn.z ) * c1 + c;
}

template< int static_size, class math_type >
void PerlinNoise< static_size, math_type >::ReSize( int new_size ) {
	d_size = new_size;
	d_mask = new_size - 1;
	d_permutationTable.SetNum( d_size );
	d_gradientX.SetNum( d_size );
	d_gradientY.SetNum( d_size );
	d_gradientZ.SetNum( d_size );
	for( int i = 0; i < d_size; i++ )
		d_permutationTable[ i ] = i;
	srand( System::Sys_MillisecondsUint( ) );
	for( int i = 0; i < d_size; i++ ) {
		int randomNumber = rand( ) % d_size;
		Swap( d_permutationTable[ i ], d_permutationTable[ randomNumber ] );
	}
	for( int i = 0; i < d_size; i++ ) {
		d_gradientX[ i ] = ( math_type )( rand( ) ) / ( RAND_MAX / 2 ) - ( math_type )1;
		d_gradientY[ i ] = ( math_type )( rand( ) ) / ( RAND_MAX / 2 ) - ( math_type )1;
		d_gradientZ[ i ] = ( math_type )( rand( ) ) / ( RAND_MAX / 2 ) - ( math_type )1;
	}
}

template< int static_size, class math_type >
math_type PerlinNoise< static_size, math_type >::Noise( float coord ) const {
	math_type tx0, tx1, v0;
	int qx0, qx1;
	// Compute what gradients to use
	qx0 = ( int )floorf( coord );
	qx1 = qx0 + 1;
	tx0 = coord - ( math_type )qx0;
	tx1 = tx0 - 1;
	// Make sure we don't come outside the lookup table
	qx0 = qx0 & d_mask;
	qx1 = qx1 & d_mask;
	// Compute the dotproduct between the vectors and the gradients
	v0 = d_gradientX[ qx0 ] * tx0;
	//math_type v1 = d_gradientX[ qx1 ] * tx1;
	// Modulate with the weight function
	//math_type wx = ( 3 - 2 * tx0 ) * tx0 * tx0;
	//return v0 - wy * ( v0 - v1 );
	return v0 - ( ( 3 - 2 * tx0 ) * tx0 * tx0 ) * ( v0 - ( d_gradientX[ qx1 ] * tx1 ) );
}

template< int static_size, class math_type >
math_type PerlinNoise< static_size, math_type >::Noise( const Vec2 & coord ) const {
	math_type tx0, tx1, ty0, ty1, v00, v01, v10, v11, wx, v0;
	int qx0, qx1, qy0, qy1, q00, q01, q10, q11;
	// Compute what gradients to use
	qx0 = ( int )floorf( coord.x );
	qx1 = qx0 + 1;
	tx0 = coord.x - ( math_type )qx0;
	tx1 = tx0 - 1;
	qy0 = ( int )floorf( coord.y );
	qy1 = qy0 + 1;
	ty0 = coord.y - ( math_type )qy0;
	ty1 = ty0 - 1;
	// Make sure we don't come outside the lookup table
	qx0 = qx0 & d_mask;
	qx1 = qx1 & d_mask;
	qy0 = qy0 & d_mask;
	qy1 = qy1 & d_mask;
	// Permutate values to get pseudo randomly chosen gradients
	q00 = d_permutationTable[ ( qy0 + d_permutationTable[ qx0 ] ) & d_mask ];
	q01 = d_permutationTable[ ( qy0 + d_permutationTable[ qx1 ] ) & d_mask ];
	q10 = d_permutationTable[ ( qy1 + d_permutationTable[ qx0 ] ) & d_mask ];
	q11 = d_permutationTable[ ( qy1 + d_permutationTable[ qx1 ] ) & d_mask ];
	// Compute the dotproduct between the vectors and the gradients
	v00 = d_gradientX[ q00 ] * tx0 + d_gradientY[ q00 ] * ty0;
	v01 = d_gradientX[ q01 ] * tx1 + d_gradientY[ q01 ] * ty0;
	v10 = d_gradientX[ q10 ] * tx0 + d_gradientY[ q10 ] * ty1;
	v11 = d_gradientX[ q11 ] * tx1 + d_gradientY[ q11 ] * ty1;
	// Modulate with the weight function
	wx = ( 3 - 2 * tx0 ) * tx0 * tx0;
	v0 = v00 - wx * ( v00 - v01 );
	//math_type v1 = v10 - wx * (v 10 - v11 );
	//math_type wy = ( 3 - 2 * ty0 ) * ty0 * ty0;
	//return v0 - wy * ( v0 - v1 );
	return v0 - ( ( 3 - 2 * ty0 ) * ty0 * ty0 ) * ( v0 - ( v10 - wx * (v 10 - v11 ) ) );
}

template< int static_size, class math_type >
math_type PerlinNoise< static_size, math_type >::Noise( const Vec3 & coord ) const {
	math_type tx0, tx1, ty0, ty1, tz0, tz1, v000, v001, v010, v011, v100, v101, v110, v111, wx, v00, v01, v10, v11, wy, v0;
	int qx0, qx1, qy0, qy1, qz0, qz1, q000, q001, q010, q011, q100, q101, q110, q111;
	// Compute what gradients to use
	qx0 = ( int )floorf( coord.x );
	qx1 = qx0 + 1;
	tx0 = coord.x - ( math_type )qx0;
	tx1 = tx0 - 1;
	qy0 = ( int )floorf( coord.y );
	qy1 = qy0 + 1;
	ty0 = coord.y - ( math_type )qy0;
	ty1 = ty0 - 1;
	qz0 = ( int )floorf( coord.z );
	qz1 = qz0 + 1;
	tz0 = coord.z - ( math_type )qz0;
	tz1 = tz0 - 1;
	// Make sure we don't come outside the lookup table
	qx0 = qx0 & d_mask;
	qx1 = qx1 & d_mask;
	qy0 = qy0 & d_mask;
	qy1 = qy1 & d_mask;
	qz0 = qz0 & d_mask;
	qz1 = qz1 & d_mask;
	// Permutate values to get pseudo randomly chosen gradients
	q000 = d_permutationTable[ ( qz0 + d_permutationTable[ ( qy0 + d_permutationTable[ qx0 ] ) & d_mask ] ) & d_mask ];
	q001 = d_permutationTable[ ( qz0 + d_permutationTable[ ( qy0 + d_permutationTable[ qx1 ] ) & d_mask ] ) & d_mask ];
	q010 = d_permutationTable[ ( qz0 + d_permutationTable[ ( qy1 + d_permutationTable[ qx0 ] ) & d_mask ] ) & d_mask ];
	q011 = d_permutationTable[ ( qz0 + d_permutationTable[ ( qy1 + d_permutationTable[ qx1 ] ) & d_mask ] ) & d_mask ];
	q100 = d_permutationTable[ ( qz1 + d_permutationTable[ ( qy0 + d_permutationTable[ qx0 ] ) & d_mask ] ) & d_mask ];
	q101 = d_permutationTable[ ( qz1 + d_permutationTable[ ( qy0 + d_permutationTable[ qx1 ] ) & d_mask ] ) & d_mask ];
	q110 = d_permutationTable[ ( qz1 + d_permutationTable[ ( qy1 + d_permutationTable[ qx0 ] ) & d_mask ] ) & d_mask ];
	q111 = d_permutationTable[ ( qz1 + d_permutationTable[ ( qy1 + d_permutationTable[ qx1 ] ) & d_mask ] ) & d_mask ];
	// Compute the dotproduct between the vectors and the gradients
	v000 = d_gradientX[ q000 ] * tx0 + d_gradientY[ q000 ] * ty0 + d_gradientZ[ q000 ] * tz0;
	v001 = d_gradientX[ q001 ] * tx1 + d_gradientY[ q001 ] * ty0 + d_gradientZ[ q001 ] * tz0;
	v010 = d_gradientX[ q010 ] * tx0 + d_gradientY[ q010 ] * ty1 + d_gradientZ[ q010 ] * tz0;
	v011 = d_gradientX[ q011 ] * tx1 + d_gradientY[ q011 ] * ty1 + d_gradientZ[ q011 ] * tz0;
	v100 = d_gradientX[ q100 ] * tx0 + d_gradientY[ q100 ] * ty0 + d_gradientZ[ q100 ] * tz1;
	v101 = d_gradientX[ q101 ] * tx1 + d_gradientY[ q101 ] * ty0 + d_gradientZ[ q101 ] * tz1;
	v110 = d_gradientX[ q110 ] * tx0 + d_gradientY[ q110 ] * ty1 + d_gradientZ[ q110 ] * tz1;
	v111 = d_gradientX[ q111 ] * tx1 + d_gradientY[ q111 ] * ty1 + d_gradientZ[ q111 ] * tz1;
	// Modulate with the weight function
	wx = ( 3 - 2 * tx0 ) * tx0 * tx0;
	v00 = v000 - wx * ( v000 - v001 );
	v01 = v010 - wx * ( v010 - v011 );
	v10 = v100 - wx * ( v100 - v101 );
	v11 = v110 - wx * ( v110 - v111 );
	wy = ( 3 - 2 * ty0 ) * ty0 * ty0;
	v0 = v00 - wy * ( v00 - v01 );
	//math_type v1 = v10 - wy * ( v10 - v11 );
	//math_type wz = ( 3 - 2 * tz0 ) * tz0 * tz0;
	//return v0 - wz * ( v0 - v1 );
	return v0 - ( ( 3 - 2 * tz0 ) * tz0 * tz0 ) * ( v0 - ( v10 - wy * ( v10 - v11 ) ) );
}

template< int static_size, class math_type >
INLINE void PerlinNoise< static_size, math_type >::FillTexture( GLTexture * texture ) const {
	FillTexture( texture, VecT2i( d_size, d_size ) );
}

template< int static_size, class math_type >
void PerlinNoise< static_size, math_type >::FillTexture( GLTexture * texture, const VecT2i & size ) const {
	Vec3 p, d;
	CAList< int > imageData;
	imageData.SetNum( size[ 0 ] * size[ 1 ] );
	float z = System::Sys_Milliseconds( ) / 1000.0f;
	for( int iy = 0; iy < size[ 1 ]; iy++ ) {
		// Compute the starting position from the y and z coordinate
		float y = iy / 16.0f;
		p.Set( y * d_rotation[ 0 ][ 1 ] + z * d_rotation[ 0 ][ 2 ], y * d_rotation[ 1 ][ 1 ] + z * d_rotation[ 1 ][ 2 ], y * d_rotation[ 2 ][ 1 ] + z * d_rotation[ 2 ][ 2 ] );
		// This represents movements along the x axis
		float x = 1 / 16.0f;
		d.Set( x * d_rotation[ 0 ][ 0 ], x * d_rotation[ 1 ][ 0 ], x * d_rotation[ 2 ][ 0 ] );
		for( int ix = 0; ix < size[ 0 ]; ix++ ) {
			byte n = ( byte )( 255 * 0.5f * ( Noise( p ) + 1 ) );
			imageData[ ix + iy * size[ 0 ] ] = ( n << 16 ) | ( n << 8 ) | n;
			p += d;
		}
	}
	GLScopedLock glLock;
	texture->Bind( );
	GLImports::glTexImage2D( texture->GetTarget( ), 0, GL_RGBA8, size[ 0 ], size[ 1 ], 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData.Ptr( ) );
	texture->UnBind( );
}

// // //// // //// // //// //
/// MD5Vertex
// //// // //// // //// //

class MD5Vertex {
	friend class			MD5Mesh;

private:
	Vec2					d_st[ 2 ];
	int						d_weightStart;
	int						d_weightCount;

public:
	const Vec2 &			GetTexCoord( int index ) const;
	int						GetStart( ) const;
	int						GetCount( ) const;
};

// // //// // //// // //// //
/// MD5Triangle
// //// // //// // //// //

class MD5Triangle {
	friend class			MD5Mesh;

private:
	int						d_indices[ 3 ];

public:
	const int &				operator[ ]( int index ) const;
};

// // //// // //// // //// //
/// MD5JointBase
// //// // //// // //// //

class MD5JointBase {
	friend class			MD5Anim;
protected:
	Quat					d_orientation;
	Vec3					d_position;

public:
	const Quat &			GetOrientation( ) const;
	const Vec3 &			GetPosition( ) const;
};

// // //// // //// // //// //
/// MD5Joint
// //// // //// // //// //

class MD5Joint : public NamedObject, public MD5JointBase {
	friend class			MD5Mesh;
	friend class			MD5Anim;

private:
	int						d_parent;

public:
	int						GetParent( ) const;
};

// // //// // //// // //// //
/// MD5Skeleton
// //// // //// // //// //

class MD5Skeleton {
	friend class			MD5Mesh;
	friend class			MD5Anim;

private:
	List< MD5Joint >		d_joints;
	Bounds					d_bbox;

public:
	int						GetNumJoints( ) const;
	const MD5Joint &		operator[ ]( int index ) const;
	const Bounds &			GetBounds( ) const;
};

// // //// // //// // //// //
/// MD5Weight
// //// // //// // //// //

class MD5Weight {
	friend class			MD5Mesh;

private:
	Vec3					d_position;
	float					d_bias;
	int						d_joint;	

public:
	const Vec3 &			GetPosition( ) const;
	float					GetBias( ) const;
	int						GetJoint( ) const;	
};

// // //// // //// // //// //
/// MD5Mesh
// //// // //// // //// //

class MD5Mesh {
private:
	List< MD5Weight >		d_weights;
	List< MD5Vertex >		d_vertices;
	List< MD5Triangle >		d_triangles;
	MD5Skeleton				d_baseSkeleton;
	Str						d_shader;

	bool					ParseMesh( Lexer & lexer );
	bool					ParseBaseSkeleton( Lexer & lexer );

public:
	bool					Parse( Lexer & lexer );
	void					ComputeSkeletonBounds( const MD5Skeleton & skeleton, Bounds & skeleton_bounds ) const;

	const Str &				GetShader( ) const;
	const MD5Skeleton &		GetBaseSkeleton( ) const;
	int						GetNumWeights( ) const;
	int						GetNumVertices( ) const;
	int						GetNumTriangles( ) const;
	const MD5Weight &		GetWeight( int index ) const;
	const MD5Vertex &		GetVertex( int index ) const;
	const MD5Triangle &		GetTriangle( int index ) const;
};

// // //// // //// // //// //
/// MD5JointInfo
// //// // //// // //// //

class MD5JointInfo : public NamedObject {
	friend class			MD5Anim;

private:
	int						d_flags;
	int						d_parent;
	int						d_startIndex;

public:
	int						GetFlags( ) const;
	int						GetParent( ) const;
	int						GetStartIndex( ) const;
};

// // //// // //// // //// //
/// MD5Anim
// //// // //// // //// //

class MD5Anim {
private:
	List< MD5Skeleton >		d_frames;
	int						d_frameRate;

	bool					ParseHierarchy( Lexer & lexer, List< MD5JointInfo > & h_list );
	bool					ParseBounds( Lexer & lexer );
	bool					ParseBaseFrame( Lexer & lexer, List< MD5JointBase > & bf_list );
	bool					ParseFrame( Lexer & lexer, List< float > & af_list );
	void					BuildFrameSkeleton( const List< MD5JointInfo > & h_list, const List< MD5JointBase > & bf_list, const List< float > & af_list, int frame_num );

public:
	bool					Parse( Lexer & lexer );

	int						GetNumFrames( ) const;
	const MD5Skeleton &		operator[ ]( int index ) const;
	int						GetFrameRate( ) const;
};

// // //// // //// // //// //
/// RenderModel
// //// // //// // //// //

class RenderModel : public NamedObject {
private:
	MD5Mesh								d_mesh;
	List< MD5Anim >						d_anims;
	RenderSurfaceModel *				d_renderSurface;

public:
	static RenderModel *				Parse( const Str & filename );

	void								Setup( BufferAllocatorModel * gl_buffer );
	void								Animate( int anim_id, int frame );
	RenderSurfaceModel *				GetSurface( ) const		{ return d_renderSurface; }
};

// // //// // //// // //// //
// MD5Vertex
// //// // //// // //// //

INLINE const Vec2 & MD5Vertex::GetTexCoord( int index ) const {
	assert( ( index == 0 ) || ( index == 1 ) );
	return d_st[ index ];
}

INLINE int MD5Vertex::GetStart( ) const {
	return d_weightStart;
}

INLINE int MD5Vertex::GetCount( ) const {
	return d_weightCount;
}

// // //// // //// // //// //
// MD5Triangle
// //// // //// // //// //

INLINE const int & MD5Triangle::operator[ ]( int index ) const {
	return d_indices[ index ];
}

// // //// // //// // //// //
// MD5JointBase
// //// // //// // //// //

INLINE const Quat & MD5JointBase::GetOrientation( ) const {
	return d_orientation;
}

INLINE const Vec3 & MD5JointBase::GetPosition( ) const {
	return d_position;
}

// // //// // //// // //// //
// MD5Joint
// //// // //// // //// //

INLINE int MD5Joint::GetParent( ) const {
	return d_parent;
}

// // //// // //// // //// //
// MD5Skeleton
// //// // //// // //// //

INLINE int MD5Skeleton::GetNumJoints( ) const {
	return d_joints.Num( );
}

INLINE const MD5Joint & MD5Skeleton::operator[ ]( int index ) const {
	return d_joints[ index ];
}

INLINE const Bounds & MD5Skeleton::GetBounds( ) const {
	return d_bbox;
}

// // //// // //// // //// //
// MD5Weight
// //// // //// // //// //

INLINE const Vec3 & MD5Weight::GetPosition( ) const {
	return d_position;
}

INLINE float MD5Weight::GetBias( ) const {
	return d_bias;
}

INLINE int MD5Weight::GetJoint( ) const {
	return d_joint;
}

// // //// // //// // //// //
// MD5Mesh
// //// // //// // //// //

INLINE const Str & MD5Mesh::GetShader( ) const {
	return d_shader;
}

INLINE const MD5Skeleton & MD5Mesh::GetBaseSkeleton( ) const {
	return d_baseSkeleton;
}

INLINE int MD5Mesh::GetNumWeights( ) const {
	return d_weights.Num( );
}

INLINE int MD5Mesh::GetNumVertices( ) const {
	return d_vertices.Num( );
}

INLINE int MD5Mesh::GetNumTriangles( ) const {
	return d_triangles.Num( );
}

INLINE const MD5Weight & MD5Mesh::GetWeight( int index ) const {
	return d_weights[ index ];
}

INLINE const MD5Vertex & MD5Mesh::GetVertex( int index ) const {
	return d_vertices[ index ];
}

INLINE const MD5Triangle & MD5Mesh::GetTriangle( int index ) const {
	return d_triangles[ index ];
}

// // //// // //// // //// //
// MD5JointInfo
// //// // //// // //// //

INLINE int MD5JointInfo::GetFlags( ) const {
	return d_flags;
}

INLINE int MD5JointInfo::GetParent( ) const {
	return d_parent;
}

INLINE int MD5JointInfo::GetStartIndex( ) const {
	return d_startIndex;
}

// // //// // //// // //// //
// MD5Anim
// //// // //// // //// //

INLINE int MD5Anim::GetNumFrames( ) const {
	return d_frames.Num( );
}

INLINE const MD5Skeleton & MD5Anim::operator[ ]( int index ) const {
	return d_frames[ index ];
}

INLINE int MD5Anim::GetFrameRate( ) const {
	return d_frameRate;
}

// // //// // //// // //// //
/// SceneEntity
// //// // //// // //// //

class SceneEntity {
public:
	RenderModel *									model;
	Vec3											angles;

	// most recent data
	Vec3											origin;		// also used as RF_BEAM's "from"
	int												frame;			// also used as RF_BEAM's diameter
	
	// previous data for lerping
	Vec3											oldorigin;	// also used as RF_BEAM's "to"
	int												oldframe;

	// misc
	float											backlerp;				// 0.0 = current, 1.0 = old
	int												skinnum;				// also used as RF_BEAM's palette index

	int												lightstyle;				// for flashing entities
	float											alpha;					// ignore if RF_TRANSLUCENT isn't set

	TextureBase *									skin;			// NULL for INLINE skin
	int												flags;

	void											Clear( );
};

// // //// // //// // //// //
/// SceneLight
// //// // //// // //// //

class SceneLight {
public:
													SceneLight( ) { }
													SceneLight( const Vec3 & origin, const Vec3 & color, float intensity );

	Vec3											lightOrigin;
	Vec3											lightColor;
	float											lightIntensity;
};

// // //// // //// // //// //
/// Scene
// //// // //// // //// //

class Scene {
public:
	int												x;
	int												y;
	int												width;
	int												height;

	float											fov_x;
	float											fov_y;
	Vec3											vieworg;
	Vec3											viewangles;
	float											blend[ 4 ];			// rgba 0-1 full screen blend
	float											time;				// time is uesed to auto animate
	int												rdflags;			// RDF_UNDERWATER, etc

	float											d_zFar;
	float											d_zNear;
	Mat4											d_viewMatrix;
	Projection										d_viewProjectionMatrix;
	
	List< SceneEntity >								entities;
	List< SceneLight >								dlights;

	void											Clear( );
	void											CalculateProjection( );
};

// // //// // //// // //// //
/// RenderModelStatic
// //// // //// // //// //

class RenderModelStatic {
private:
	Bounds											d_bounds;
	CAList< RenderSurfaceMap *, 16 >					d_renderSurface;
	CAList< int, 16 >								d_pvsReferenced;

public:
													~RenderModelStatic( );

	RenderSurfaceMap *								LoadSurface( GLShader * surface_shader, BufferAllocatorMap * gl_buffer, Surface * surface_data );
	RenderSurfaceMap *								LoadBrushSide( GLShader * surface_shader, BufferAllocatorMap * gl_buffer, const class MapBrush & brush, const class MapBrushSide & side );
	const CAListBase< RenderSurfaceMap * > &		GetSurfaces( ) const					{ return d_renderSurface; }

	const Bounds &									GetBounds( ) const						{ return d_bounds; }
	void											SetBounds( const Bounds & b )			{ d_bounds = b; }

	void											SetPvsRefCount( int count )				{ d_pvsReferenced.SetNum( count ); }
	int												GetPvsRefCount( ) const					{ return d_pvsReferenced.Num( ); }
	const int &										GetPvsRef( const int index ) const		{ return d_pvsReferenced[ index ]; }
	void											ClearPvsRef( const int index )			{ d_pvsReferenced[ index ] = 0; }
	void											SetPvsRef( const int index, int pass )	{ d_pvsReferenced[ index ] = pass; }
};

// // //// // //// // //// //
/// VisTreeKey
// //// // //// // //// //

class VisTreeKey {

public:

	union {
		struct {
			int									d_renderClass;
			int									d_blockOffset;
		};
		#if defined (_M_AMD64) || defined (_M_X64)
		__int64								d_i64data;
		#endif
	} d_data;

													VisTreeKey( ) { }
													VisTreeKey( int r_class, int b_offset )	{ d_data.d_renderClass = r_class; d_data.d_blockOffset = b_offset; }

	void											operator =( const int n )	{ d_data.d_renderClass = d_data.d_blockOffset = n; }
	bool											operator >( const VisTreeKey & other ) const {

	#if defined (_M_AMD64) || defined (_M_X64)
		return d_data.d_i64data > other.d_data.d_i64data;
	#else
		return ( d_data.d_renderClass == other.d_data.d_renderClass ) ?
				( d_data.d_blockOffset > other.d_data.d_blockOffset ) :
				( d_data.d_renderClass > other.d_data.d_renderClass );
	#endif
	}
	bool											operator <=( const VisTreeKey & other ) const {

	#if defined (_M_AMD64) || defined (_M_X64)
		return d_data.d_i64data <= other.d_data.d_i64data;
	#else
		return ( d_data.d_renderClass == other.d_data.d_renderClass ) ?
				( d_data.d_blockOffset <= other.d_data.d_blockOffset ) :
				( d_data.d_renderClass <= other.d_data.d_renderClass );
	#endif
	}
} ALIGN( 8 );

typedef BTree< RenderSurfaceMap, VisTreeKey, 10 >		VisTree;
typedef BTreeNode< RenderSurfaceMap, VisTreeKey >		VisTreeNode;

// // //// // //// // //// //
/// BSPTreeKey
// //// // //// // //// //

class BSPTreeKey {
public:
	Plane											d_plane;
	Bounds											d_bounds;

													BSPTreeKey( ) { d_plane.Zero( ); d_bounds.Clear( ); }
};

typedef CAList< RenderModelStatic *, 8 >						BSPObjectType;
typedef BinaryTreeNode< BSPObjectType, BSPTreeKey >		BSPNode;

// // //// // //// // //// //
/// BSPTree
// //// // //// // //// //

class BSPTree : public BinaryTree< BSPObjectType, BSPTreeKey > {
private:
	DynamicBlockAlloc< BSPTreeKey, 256, 32 >		d_keyAllocator;

	class BSPBuildParams {
	public:
		Bounds											d_keyBounds;
		CAList< Plane, 6 >								d_keyPlanes;
		RenderModelStatic *									d_object;
		List< Bounds >									d_stackBounds;
		CAList< CAList< Plane, 6 >, 128 >				d_stackPlanes;

		bool											ContainsPlane( const Plane & plane ) const;
		void											RemovePlane( const Plane & plane );
		void											PushPlanes( );
		void											PopPlanes( );
		void											PushAndClipBounds( const Plane & clipping_plane, bool back_clip );
		void											PopBounds( );
	};

	class BspPvsParams {
	public:
		const CAListBase< Plane > & 					d_pvsPlanes;
		VisTree &										d_visTree;
		bool											d_enableKeyCheck;
		int												d_brushRefIndex;
		int												d_funcLevel;
		int												d_pvsLockoutLevel;
		
		bool											KeyCull( const Bounds & bounds );
		void											AddObject( BSPObjectType * object );

														BspPvsParams( const CAListBase< Plane > & pvs_planes, VisTree & vis_tree, int brush_index );
	};

	int												FindBestNextPlane( BSPNode * node, BSPBuildParams * params );
	BSPNode *										AllocForPlane( BSPBuildParams * params, int plane_index );
	void											AllocForObject( BSPNode * node, BSPBuildParams * params, int plane_side );

	void											AddBrush_r( BSPNode * node, BSPBuildParams * params );

	void											FillNodeBounds_r( BSPNode * node );
	void											FillNodeBounds( ) { FillNodeBounds_r( d_root ); }

	void											GetPVS_r( BSPNode & node, BspPvsParams & params );

public:
													BSPTree( ) { };
													~BSPTree( ) { };

	void											Clear( );	
	void											AddRenderModelStaticList( List< RenderModelStatic * > & ren_mod_list, const Bounds & ren_mod_bounds );
	void											GetPVS( const CAListBase< Plane > & pvs_planes, VisTree & ren_mod_tree, int brush_index );
};

// // //// // //// // //// //
/// GUIThread
// //// // //// // //// //

class GUIThread : public WorkerThreadBase, public SingleObject< GUIThread > {
private:
	BufferAllocatorUI *								d_primitiveBuffer;
	BufferPrimitiveUI *								d_screenRectTexture;
	Overlay *										d_overlay;
	GLFrameBuffer *									d_frameBuffer;
	GLTexture *										d_frameTexture;
	SVar *											d_uiMatrix;
	SVar *											d_uiTexture;
	Projection										d_projectionMatrix;

	Str												d_debugText;
	class GUIWindow *								d_debugWindow;
	Lock											d_debugLock;

	void											DebugTextUpdate( );

													GUIThread( );
													~GUIThread( );

public:
	static void										Init( );
	static void										Shutdown( );

	virtual unsigned int							Run( void * param );

	static void										Clear( );
	static void										Draw( );

	static void										SingleDoWork( )			{ g_objectPtr->DoWork( ); }
	static void										SingleWait( )			{ g_objectPtr->Wait( ); }

	static GLShader *								GetShader( )			{ return g_objectPtr->d_overlay->GetShader( ); }
	static Overlay *								GetOverlay( )			{ return g_objectPtr->d_overlay; }
	static BufferAllocatorUI *						GetPrimitiveBuffer( )	{ return g_objectPtr->d_primitiveBuffer; }
	static GLFrameBuffer *							GetFrameBuffer( )		{ return g_objectPtr->d_frameBuffer; }
	static GLTexture *								GetTexture( )			{ return g_objectPtr->d_frameTexture; }
	static Projection &								GetProjection( )		{ return g_objectPtr->d_projectionMatrix; }

	static void										DebugPrint( const char * fmt, ... );
};

// // //// // //// // //// //
/// ScreenShotThread
// //// // //// // //// //

class ScreenShotThread : public WorkerThreadBase {
private:
	Str												d_fileName;
	Image *											d_image;

public:
													ScreenShotThread( const Str & file_name, Image * image );
	virtual unsigned int							Run( void * param );
};

// // //// // //// // //// //
/// RendererThreadParams
// //// // //// // //// //

class RendererThreadParams {
	typedef struct {
		GLShader *	val1;
		GLShader *	val2;
	} t_shaderPair;
public:
	GLContext										d_context;
	Scene											d_scene;
	int												d_indexUploadLimit;
	int												d_indexUploadLimitCounter;

	CAList< BufferPrimitive3D *, 32 >				d_debugPrimitives;

	BufferAllocatorMap *							d_mapBufferAllocator;
	BufferAllocatorModel *							d_modelBufferAllocator;
	BufferAllocatorUI *								d_uiBufferAllocator;

	CAList< t_shaderPair, 256 >						d_shaderTranslator;

	bool											d_pvsLock;
	CAList< Plane, 6 >								d_pvsPlanes;
	int												d_pvsPrimitiveStart;
	VisTree											d_visTree;

	Timer											d_timerFrame;
	Timer											d_timerPVS;
	Timer											d_timerRender;

	int												d_primitiveCountBrush;

	bool											d_takeScreenShot;
	Str												d_nameScreenShot;

	void											ClearCounters( );

	GLShader *										TranslatorAdd( GLShader * local, GLShader * original );
	GLShader *										TranslatorLookup( GLShader * original );

													RendererThreadParams( );
};

// // //// // //// // //// //
/// Renderer
// //// // //// // //// //

class Renderer : public WorkerThreadBase, public SingleObject< Renderer > {
private:
	CVar											g_renSleep;

	static List< RenderModelStatic * >				g_mapBrushes;
	static BSPTree									g_pvsTree;

	static RenderModel *							g_missingModel;
	static CANamedList< RenderModel *, 256 >		g_renderModels;

	static BufferAllocatorMap *						g_mapBufferAllocator;
	static BufferAllocatorModel *					g_modelBufferAllocator;
	static SVarMat4									g_mvpMatrix;
	static SVarMat4									g_viewMatrix;
	static SVarMat4									g_normalMatrix;
	static SVarMat4									g_uiMatrix;
	static SVarVec3									g_cameraOrigin;
	static SVarVec3									g_lightOrigin;
	static SVarVec3									g_lightColor;
	static SVarFloat								g_lightIntensity;
	static SVarPlane								g_sunPlane;
	static SVarVec4									g_debugColor;
	static Mat4										g_localCoordSystemRotation;

	static void										TakeScreenshot( );
	static void										SunPosition( );

	static bool										SetMode( );

	static Vec3										g_sunPosition;
	static Angles									g_sunNormal;
	static bool										g_sunRedraw;

	void											UpdateBrushMark( );
	void											UpdatePvsMark( );

	CVar											g_pvsLock;
	void											UpdatePvsLock( );

	CVar											g_brushMark;
	static int										g_brushMarkIndex;
	CVar											g_pvsMark;
	static int										g_pvsMarkIndex;
	static void										ClearBrushPVSRef( );
	static void										InitPVS( );

	CVar											g_utilDraw;
	CVar											g_ssaoEnable;
	CVar											g_fsaaEnable;
	CVar											g_shadowEnable;
	CVar											g_shadowMapStaticSize;
	CVar											g_shadowMapDynamicSize;
	VecT2i											GetSMStaticSize( )	{ int smsize = (int)powf( 2.0f, (float)( g_shadowMapStaticSize.GetInt( )+8 ) ); return VecT2i( smsize, smsize ); }
	VecT2i											GetSMDynamicSize( )	{ int smsize = (int)powf( 2.0f, (float)( g_shadowMapDynamicSize.GetInt( )+8 ) ); return VecT2i( smsize, smsize ); }

	RendererThreadParams							d_rendererThreadParams;

	virtual unsigned int							Run( void * param );
	void											DebugDrawPrimitive( GLShader * debug_shader, BufferPrimitive3D * primitive, SVar & debug_color );
	static void										DebugSwap( ) { g_objectPtr->d_rendererThreadParams.d_context.SwapBuffers( ); g_objectPtr->d_rendererThreadParams.d_context.SwapBuffers( ); }

													Renderer( );
													~Renderer( );

public:

	static bool										Init( void * hinstance, void * wndproc );
	static void										Shutdown( );

	static void										BeginRegistration( const Str & map );
	static RenderModel *							RegisterModel( const Str & name );

	static void										SetSky( const Str & name, float rotate, Vec3 & axis );
	static void										EndRegistration( );

	static void										RenderFrame( const Scene & scene );

	static void										BeginFrame( );
	static void										EndFrame( );

	static const Mat4 &								GetCoordSystemRotation( )	{ return g_localCoordSystemRotation; }
	static float									GetAspectRatio( );
};

// // //// // //// // //// //
// SceneLight
// //// // //// // //// //

INLINE SceneLight::SceneLight( const Vec3 & origin, const Vec3 & color, float intensity ) {
	this->lightOrigin = origin;
	this->lightColor = color;
	this->lightIntensity = intensity;
}

// // //// // //// // //// //
// BSPTree::BSPBuildParams
// //// // //// // //// //

INLINE bool BSPTree::BSPBuildParams::ContainsPlane( const Plane & plane ) const {
	for( int i = 0; i < d_keyPlanes.Num( ); i++ ) {
		if( d_keyPlanes[ i ] == plane )
			return true;
	}
	return false;
}

INLINE void BSPTree::BSPBuildParams::RemovePlane( const Plane & plane ) {
	for( int i = 0; i < d_keyPlanes.Num( ); i++ ) {
		if( d_keyPlanes[ i ] == plane ) {
			d_keyPlanes.RemoveIndex( i );
			return;
		}
	}
}

INLINE void BSPTree::BSPBuildParams::PushPlanes( ) {
	d_stackPlanes.Append( d_keyPlanes );
}

INLINE void BSPTree::BSPBuildParams::PopPlanes( ) {
	int num = d_stackPlanes.Num( );
	assert( num );
	d_keyPlanes = d_stackPlanes[ num - 1 ];
	d_stackPlanes.RemoveIndex( num - 1 );
}

INLINE void BSPTree::BSPBuildParams::PopBounds( ) {
	int num = d_stackBounds.Num( );
	assert( num );
	d_keyBounds = d_stackBounds[ num - 1 ];
	d_stackBounds.RemoveIndex( num - 1 );
}

// // //// // //// // //// //
// RendererThreadParams
// //// // //// // //// //

INLINE void Renderer::InitPVS( ) {
	if( !g_mapBrushes[ 0 ]->GetPvsRefCount( ) ) {
		for( int i = 0; i < g_mapBrushes.Num( ); i++ )
			g_mapBrushes[ i ]->SetPvsRefCount( 1 );
	}
	ClearBrushPVSRef( );
}

INLINE void Renderer::ClearBrushPVSRef( ) {
	for( int i = 0; i < g_mapBrushes.Num( ); i++ )
		g_mapBrushes[ i ]->ClearPvsRef( 0 );
}

INLINE GLShader * RendererThreadParams::TranslatorAdd( GLShader * local, GLShader * original ) {
	t_shaderPair pair = { original, local };
	d_shaderTranslator.Append( pair );
	return local;
}

INLINE GLShader * RendererThreadParams::TranslatorLookup( GLShader * original ) {
	for( int i = 0; i < d_shaderTranslator.Num( ); i++ ) {
		if( d_shaderTranslator[ i ].val1 == original )
			return d_shaderTranslator[ i ].val2;
	}
	return TranslatorAdd( original->ContextCreate( ), original );
}

#endif
