#include "precompiled.h"
#pragma hdrstop

// // //// // //// // //// //
// RenderSurfaceMap
// //// // //// // //// //

template< > RenderSurfaceMap::RenderSurface( GLShader * surface_shader, BufferAllocatorMap * gl_buffer, Surface * surface_data ) {
	d_surfaceShader				= surface_shader;
	int vertexCount				= surface_data->GetNumVertices( );
	d_surfaceVertexBlock		= gl_buffer->AllocVertex( vertexCount );
	d_surfaceBaseVertex			= d_surfaceVertexBlock->ObjectNum( );
	for( int i = 0; i < vertexCount; i++ )
		( *d_surfaceVertexBlock )[ i ] = ( *surface_data )[ i ];
	gl_buffer->UploadVertex( d_surfaceVertexBlock );	
	int indexCount				= surface_data->GetNumIndexes( );
	const int * indexes			= surface_data->GetIndexes( );
	for( int i = 0; i < indexCount; i++ )
		d_surfaceIndexes.Append( indexes[ i ] + d_surfaceBaseVertex );
	d_surfaceAllocator			= gl_buffer;
	d_renderClass				= -1;
}

template< > RenderSurfaceMap::RenderSurface( GLShader * surface_shader, BufferAllocatorMap * gl_buffer, const MapBrush & brush, const MapBrushSide & side ) {
	Vec3 texX, texY, brushTexMat[ 2 ];
	d_surfaceShader				= surface_shader;
	int vertexCount				= side.GetNumPoints( );
	d_surfaceVertexBlock		= gl_buffer->AllocVertex( vertexCount );
	d_surfaceBaseVertex			= d_surfaceVertexBlock->ObjectNum( );
	side.GetTextureMatrix( brushTexMat[ 0 ], brushTexMat[ 1 ] );
	MapBrushSide::ComputeAxisBase( side.GetPlane( ).Normal( ), texX, texY );
	int indexLookup[ MAX_POINTS_ON_WINDING ];
	for( int i = 0; i < vertexCount; i++ ) {
		Vertex3D & vertOut = ( *d_surfaceVertexBlock )[ i ];
		const Vec3 & vert = brush.GetVertex( side.GetPoint( i ) );
		float x = vert * texX;
		float y = vert * texY;
		vertOut.xyz = vert;
		vertOut.st[ 0 ] = brushTexMat[ 0 ][ 0 ] * x + brushTexMat[ 0 ][ 1 ] * y + brushTexMat[ 0 ][ 2 ];
		vertOut.st[ 1 ] = brushTexMat[ 1 ][ 0 ] * x + brushTexMat[ 1 ][ 1 ] * y + brushTexMat[ 1 ][ 2 ];
		vertOut.normal = side.GetPlane( );
		indexLookup[ side.GetPoint( i ) ] = i;
	}
	gl_buffer->UploadVertex( d_surfaceVertexBlock );
	for( int i = 2; i < side.GetNumPoints( ); i++ ) {
		d_surfaceIndexes.Append( indexLookup[ side.GetPoint( 0 ) ] + d_surfaceBaseVertex );
		d_surfaceIndexes.Append( indexLookup[ side.GetPoint( i - 1 ) ] + d_surfaceBaseVertex );
		d_surfaceIndexes.Append( indexLookup[ side.GetPoint( i ) ] + d_surfaceBaseVertex );
	}
	d_surfaceAllocator			= gl_buffer;
	d_renderClass				= -1;
}

template< > RenderSurfaceModel::RenderSurface( GLShader * surface_shader, BufferAllocatorModel * gl_buffer, const MD5Mesh & mesh_data ) {
	d_surfaceShader				= surface_shader;
	d_surfaceVertexBlock		= gl_buffer->AllocVertex( mesh_data.GetNumVertices( ) );
	d_surfaceBaseVertex			= d_surfaceVertexBlock->ObjectNum( );
	gl_buffer->UploadVertex( d_surfaceVertexBlock );
	d_surfaceIndexBlock			= gl_buffer->AllocIndex( mesh_data.GetNumTriangles( ) * 3 );
	d_surfaceBaseIndex			= d_surfaceIndexBlock->ObjectNum( );
	for( int i = 0; i < mesh_data.GetNumTriangles( ); i++ ) {
		( *d_surfaceIndexBlock )[ i*3+0 ] = d_surfaceBaseVertex + mesh_data.GetTriangle( i )[ 0 ];
		( *d_surfaceIndexBlock )[ i*3+1 ] = d_surfaceBaseVertex + mesh_data.GetTriangle( i )[ 1 ];
		( *d_surfaceIndexBlock )[ i*3+2 ] = d_surfaceBaseVertex + mesh_data.GetTriangle( i )[ 2 ];
	}
	gl_buffer->UploadIndex( d_surfaceIndexBlock );
	d_surfaceAllocator			= gl_buffer;
	d_renderClass				= -1;
}

template< > void RenderSurfaceMap::ComputeIndexes( ) {
	d_surfaceIndexBlock			= d_surfaceAllocator->AllocIndex( d_surfaceIndexes.Num( ) );
	d_surfaceBaseIndex			= d_surfaceIndexBlock->ObjectNum( );
	Common::Com_Memcpy( d_surfaceIndexBlock->d_data, d_surfaceIndexes.Ptr( ), d_surfaceIndexes.MemoryUsed( ) );
	d_surfaceAllocator->UploadIndex( d_surfaceIndexBlock );
}

template< > int RenderSurfaceMap::ShaderAndSVarCmp( RenderSurfaceMap * const * a, RenderSurfaceMap * const * b ) {
	if( ( *a )->d_surfaceShader > ( *b )->d_surfaceShader )
		return -1;
	if( ( *a )->d_surfaceShader < ( *b )->d_surfaceShader )
		return 1;
	if( ( *a )->d_surfaceParams.Num( ) > ( *b )->d_surfaceParams.Num( ) )
		return -1;
	if( ( *a )->d_surfaceParams.Num( ) < ( *b )->d_surfaceParams.Num( ) )
		return 1;
	for( int i = 0; i < ( *a )->d_surfaceParams.Num( ); i++ ) {
		SVar * aParam = ( *a )->d_surfaceParams[ i ];
		SVar * bParam = ( *b )->d_surfaceParams[ i ];
		// determine the type & name the same
		int sVarTypeCmpVal = SVar::TypeAndNameCompare( &aParam, &bParam );
		if( !sVarTypeCmpVal && *aParam == SVar_Sampler2D ) {
			// when sampler2d svars match, determine the textures gl id usage
			int sVarSampler2DCmpVal = SVar::Sampler2DComapre( &aParam, &bParam );
			if( sVarSampler2DCmpVal ) {
				// if fails, qsort sorts by gl id
				return sVarSampler2DCmpVal;
			}
			// if match, check another params
			continue;
		}
		// maybe more checks needed ?
		return -1;
	}
	// full match
	return 0;
}

template< > int RenderSurfaceMap::ShaderAndSVarAndIndexCmp( RenderSurfaceMap * const * a, RenderSurfaceMap * const * b ) {
	if( ( *a )->d_surfaceShader > ( *b )->d_surfaceShader )
		return -1;
	if( ( *a )->d_surfaceShader < ( *b )->d_surfaceShader )
		return 1;
	if( ( *a )->d_surfaceParams.Num( ) > ( *b )->d_surfaceParams.Num( ) )
		return -1;
	if( ( *a )->d_surfaceParams.Num( ) < ( *b )->d_surfaceParams.Num( ) )
		return 1;
	for( int i = 0; i < ( *a )->d_surfaceParams.Num( ); i++ ) {
		SVar * aParam = ( *a )->d_surfaceParams[ i ];
		SVar * bParam = ( *b )->d_surfaceParams[ i ];
		// determine the type & name the same
		int sVarTypeCmpVal = SVar::TypeAndNameCompare( &aParam, &bParam );
		if( !sVarTypeCmpVal && *aParam == SVar_Sampler2D ) {
			// when sampler2d svars match, determine the textures gl id usage
			int sVarSampler2DCmpVal = SVarSampler2D::Sampler2DComapre( &aParam, &bParam );
			if( sVarSampler2DCmpVal ) {
				// if fails, qsort sorts by gl id
				return sVarSampler2DCmpVal;
			}
			// if match, check another params
			continue;
		}
		// maybe more checks needed ?
		return -1;
	}
	// full match
	if( ( *a )->BaseVertex( ) < ( *b )->BaseVertex( ) )
		return -1;
	return 1;
}

// // //// // //// // //// //
// RenderModelStatic
// //// // //// // //// //

RenderModelStatic::~RenderModelStatic( ) {
	d_renderSurface.DeleteContents( true );
}

RenderSurfaceMap * RenderModelStatic::LoadSurface( GLShader * surface_shader, BufferAllocatorMap * gl_buffer, Surface * surface_data ) {
	RenderSurfaceMap * loadSurf = new RenderSurfaceMap( surface_shader, gl_buffer, surface_data );
	d_renderSurface.Append( loadSurf );
	return loadSurf;
}

RenderSurfaceMap * RenderModelStatic::LoadBrushSide( GLShader * surface_shader, BufferAllocatorMap * gl_buffer, const MapBrush & brush, const MapBrushSide & side ) {
	RenderSurfaceMap * loadSurf = new RenderSurfaceMap( surface_shader, gl_buffer, brush, side );
	d_renderSurface.Append( loadSurf );
	return loadSurf;
}

// // //// // //// // //// //
// MD5Mesh
// //// // //// // //// //

bool MD5Mesh::ParseMesh( Lexer & lexer ) {
	Token	token;
	int		numVerts = 0, numTris = 0, numWeights = 0;

	if( !lexer.ReadToken( &token ) || token != '{' )
		return false;

	while( 1 ) {
		if( !lexer.ReadToken( &token ) )
			break;
		if( token == '}' )
			return numVerts && numTris && numWeights;

		if( token == "shader" ) {
			if( !lexer.ReadToken( &token ) )
				break;
			d_shader = token;
		} else if( token == "numverts" ) {
			numVerts = lexer.ParseInt( );
			d_vertices.SetNum( numVerts );
		} else if( token == "numtris" ) {
			numTris = lexer.ParseInt( );
			d_triangles.SetNum( numTris );
		} else if( token == "numweights" ) {
			numWeights = lexer.ParseInt( );
			d_weights.SetNum( numWeights );
		} else if( token == "vert" ) {
			int dataIndex = lexer.ParseInt( );
			lexer.Parse1DMatrix( 2, d_vertices[ dataIndex ].d_st[0].ToFloatPtr( ) );
			d_vertices[ dataIndex ].d_st[1].Zero( );
			d_vertices[ dataIndex ].d_weightStart = lexer.ParseInt( );
			d_vertices[ dataIndex ].d_weightCount = lexer.ParseInt( );
		} else if( token == "tri" ) {
			int dataIndex = lexer.ParseInt( );
			d_triangles[ dataIndex ].d_indices[ 0 ] = lexer.ParseInt( );
			d_triangles[ dataIndex ].d_indices[ 1 ] = lexer.ParseInt( );
			d_triangles[ dataIndex ].d_indices[ 2 ] = lexer.ParseInt( );
		} else if( token == "weight" ) {
			int dataIndex = lexer.ParseInt( );
			d_weights[ dataIndex ].d_joint = lexer.ParseInt( );
			d_weights[ dataIndex ].d_bias = lexer.ParseFloat( );
			lexer.Parse1DMatrix( 3, d_weights[ dataIndex ].d_position.ToFloatPtr( ) );
		} else {
			assert( 0 );
		}
	}
	return false;
}

bool MD5Mesh::ParseBaseSkeleton( Lexer & lexer ) {
	Token	token;
	if( !lexer.ReadToken( &token ) || token != '{' )
		return false;

	while( 1 ) {
		if( !lexer.ReadToken( &token ) )
			break;
		if( token == '}' )
			return true;

		MD5Joint & joint		= d_baseSkeleton.d_joints.Alloc( );
		joint.d_name			= token;
		joint.d_parent			= lexer.ParseInt( );
		lexer.Parse1DMatrix( 3, joint.d_position.ToFloatPtr( ) );
		lexer.Parse1DMatrix( 3, joint.d_orientation.ToFloatPtr( ) );
		joint.d_orientation.w	= joint.d_orientation.CalcW( );
	}
	return false;
}

bool MD5Mesh::Parse( Lexer & lexer ) {
	Token	token;
	int		numJoints = 0, numMeshes = 0;
	bool	validMesh = false;

	while( 1 ) {
		if( !lexer.ReadToken( &token ) )
			break;

		if( token == "numJoints" ) {
			numJoints = lexer.ParseInt( );
		} else if( token == "numMeshes" ) {
			numMeshes = lexer.ParseInt( );
		} else if( token == "joints" ) {
			if( !ParseBaseSkeleton( lexer ) )
				return false;
		} else if( token == "mesh" ) {
			if( !ParseMesh( lexer ) )
				return false;
			validMesh = true;
		} else {
			assert( 0 );
		}
	}
	d_baseSkeleton.d_bbox.Clear( );
	ComputeSkeletonBounds( d_baseSkeleton, d_baseSkeleton.d_bbox );
	return validMesh && ( numMeshes == 1 ) && ( d_baseSkeleton.GetNumJoints( ) == numJoints );
}

void MD5Mesh::ComputeSkeletonBounds( const MD5Skeleton & skeleton, Bounds & skeleton_bounds ) const {
	for( int i = 0; i < d_vertices.Num( ); i++ ) { // Setup vertices
		Vec3 finalVertex = vec3_origin;
		for( int j = 0; j < d_vertices[ i ].GetCount( ); j++ ) { // Calculate final vertex to draw with weights
			const MD5Weight & weight = d_weights[ d_vertices[ i ].GetStart( ) + j ];
			const MD5Joint & joint = skeleton[ weight.GetJoint( ) ]; // Calculate transformed vertex for this weight
			Vec3 wv = joint.GetOrientation( ) * weight.GetPosition( );
			finalVertex[0] += ( joint.GetPosition( )[0] + wv[0] ) * weight.GetBias( ); // The sum of all weight->bias should be 1.0
			finalVertex[1] += ( joint.GetPosition( )[1] + wv[1] ) * weight.GetBias( );
			finalVertex[2] += ( joint.GetPosition( )[2] + wv[2] ) * weight.GetBias( );
		}
		skeleton_bounds.AddPoint( finalVertex );
	}
}

// // //// // //// // //// //
// MD5Anim
// //// // //// // //// //

bool MD5Anim::ParseHierarchy( Lexer & lexer, List< MD5JointInfo > & h_list ) {
	Token	token;
	if( !lexer.ReadToken( &token ) || token != '{' )
		return false;

	while( 1 ) {
		if( !lexer.ReadToken( &token ) )
			break;
		if( token == '}' )
			return true;

		MD5JointInfo & joint	= h_list.Alloc( );
		joint.d_name			= token;
		joint.d_parent			= lexer.ParseInt( );
		joint.d_flags			= lexer.ParseInt( );
		joint.d_startIndex		= lexer.ParseInt( );
	}
	return false;
}

bool MD5Anim::ParseBounds( Lexer & lexer ) {
	Token	token;
	assert( !d_frames.Num( ) );
	if( !lexer.ReadToken( &token ) || token != '{' )
		return false;

	while( 1 ) {
		if( !lexer.ReadToken( &token ) )
			break;
		if( token == '}' )
			return true;
		
		lexer.UnreadToken( &token );
		MD5Skeleton & skeleton	= d_frames.Alloc( );
		lexer.Parse1DMatrix( 3, skeleton.d_bbox[0].ToFloatPtr( ) );
		lexer.Parse1DMatrix( 3, skeleton.d_bbox[1].ToFloatPtr( ) );
	}
	return false;
}

bool MD5Anim::ParseBaseFrame( Lexer & lexer, List< MD5JointBase > & bf_list ) {
	Token	token;
	if( !lexer.ReadToken( &token ) || token != '{' )
		return false;

	while( 1 ) {
		if( !lexer.ReadToken( &token ) )
			break;
		if( token == '}' )
			return true;

		lexer.UnreadToken( &token );
		MD5JointBase & baseJoint	= bf_list.Alloc( );
		lexer.Parse1DMatrix( 3, baseJoint.d_position.ToFloatPtr( ) );
		lexer.Parse1DMatrix( 3, baseJoint.d_orientation.ToFloatPtr( ) );
		baseJoint.d_orientation.w	= baseJoint.d_orientation.CalcW( );
	}
	return false;
}

bool MD5Anim::ParseFrame( Lexer & lexer, List< float > & af_list ) {
	Token	token;
	if( !lexer.ReadToken( &token ) || token != '{' )
		return false;

	while( 1 ) {
		if( !lexer.ReadToken( &token ) )
			break;
		if( token == '}' )
			return true;

		lexer.UnreadToken( &token );
		af_list.Append( lexer.ParseFloat( ) ) ;
	}
	return false;
}

void MD5Anim::BuildFrameSkeleton( const List< MD5JointInfo > & h_list, const List< MD5JointBase > & bf_list, const List< float > & af_list, int frame_num ) {
	for( int i = 0; i < h_list.Num( ); i++ ) {
		const MD5JointBase & baseJoint = bf_list[ i ];
		Vec3 animatedPos = baseJoint.GetPosition( );
		Quat animatedOrient = baseJoint.GetOrientation( );
		int j = 0;		
		if( h_list[ i ].GetFlags( ) & 1 ) { // Tx
			animatedPos[0] = af_list[ h_list[ i ].GetStartIndex( ) + j ];
			j++;
		}		
		if( h_list[ i ].GetFlags( ) & 2 ) { // Ty
			animatedPos[1] = af_list[ h_list[ i ].GetStartIndex( ) + j ];
			j++;
		}		
		if( h_list[ i ].GetFlags( ) & 4 ) { // Tz
			animatedPos[2] = af_list[ h_list[ i ].GetStartIndex( ) + j ];
			j++;
		}		
		if( h_list[ i ].GetFlags( ) & 8 ) { // Qx
			animatedOrient[0] = af_list[ h_list[ i ].GetStartIndex( ) + j ];
			j++;
		}		
		if( h_list[ i ].GetFlags( ) & 16 ) { // Qy
			animatedOrient[1] = af_list[ h_list[ i ].GetStartIndex( ) + j ];
			j++;
		}		
		if( h_list[ i ].GetFlags( ) & 32 ) { // Qz
			animatedOrient[2] = af_list[ h_list[ i ].GetStartIndex( ) + j ];
			j++;
		}		
		// Compute orient quaternion's w value
		animatedOrient.w = animatedOrient.CalcW( );		
		// NOTE: we assume that this joint's parent has already been calculated, i.e. joint's ID should never be smaller than its parent ID.
		MD5Joint & thisJoint	= d_frames[ frame_num ].d_joints.Alloc( );		
		thisJoint.d_parent		= h_list[ i ].d_parent;
		thisJoint.d_name		= h_list[ i ].d_name;		
		// Has parent?
		if( thisJoint.d_parent < 0 ) {
			thisJoint.d_position = animatedPos;
			thisJoint.d_orientation = animatedOrient;
		} else {
			MD5Joint & parentJoint = d_frames[ frame_num ].d_joints[ h_list[ i ].d_parent ];
			Vec3 rpos = parentJoint.d_orientation * animatedPos; // Rotated position			
			// Add positions
			thisJoint.d_position = rpos + parentJoint.d_position;			
			// Concatenate rotations
			thisJoint.d_orientation = parentJoint.d_orientation * animatedOrient;
			thisJoint.d_orientation.Normalize( );
		}
	}
}

bool MD5Anim::Parse( Lexer & lexer ) {
	Token	token;
	int		numFrames = 0, numJoints = 0, numAnimatedComponents = 0;
	d_frameRate = 0;
	List< MD5JointInfo > jointHierarchy;
	List< MD5JointBase > baseFrame;
	List< float > animFrameData;

	while( 1 ) {
		if( !lexer.ReadToken( &token ) )
			break;

		if( token == "numFrames" ) {
			numFrames = lexer.ParseInt( );
		} else if( token == "numJoints" ) {
			numJoints = lexer.ParseInt( );
		} else if( token == "frameRate" ) {
			d_frameRate = lexer.ParseInt( );
		} else if( token == "numAnimatedComponents" ) {
			numAnimatedComponents = lexer.ParseInt( );
		} else if( token == "hierarchy" ) {
			if( !ParseHierarchy( lexer, jointHierarchy ) )
				return false;
		} else if( token == "bounds" ) {
			if( !ParseBounds( lexer ) )
				return false;
		} else if( token == "baseframe" ) {
			if( !ParseBaseFrame( lexer, baseFrame ) )
				return false;
		} else if( token == "frame" ) {
			int dataIndex = lexer.ParseInt( );
			animFrameData.SetNum( 0, false );			
			if( !ParseFrame( lexer, animFrameData ) )
				return false;
			assert( animFrameData.Num( ) == numAnimatedComponents );
			BuildFrameSkeleton( jointHierarchy, baseFrame, animFrameData, dataIndex );
		} else {
			assert( 0 );
		}
	}
	return true;
}

// // //// // //// // //// //
// MD5Surface
// //// // //// // //// //

RenderModel * RenderModel::Parse( const Str & filename ) {
	Lexer lexer( filename, LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );
	Token token;
	int version = 0;
	if( !lexer.IsLoaded( ) )
		return NULL;
	RenderModel * renderModel = new RenderModel( );
	renderModel->d_name = filename;
	while( 1 ) {
		if( lexer.EndOfFile( ) )
			break;
		if( !lexer.ReadToken( &token ) )
			break;

		if( token == "MD5Version" ) {
			version = lexer.ParseInt( );
		} else if( token == "commandline" ) {
			if( !lexer.ReadToken( &token ) )
				break;
			if( token.IcmpnFast( "mesh", 4 ) ) {
				if( !renderModel->d_mesh.Parse( lexer ) )
					return false;
			} else if( token.IcmpnFast( "anim", 4 ) ) {
				if( !renderModel->d_anims.Alloc( ).Parse( lexer ) )
					return false;
			}
		} else {
			assert( 0 );
		}
	}
	return renderModel;
}

void RenderModel::Setup( BufferAllocatorModel * gl_buffer ) {
	Material * meshMaterial = MaterialSystem::FindMaterial( d_mesh.GetShader( ) );
	d_renderSurface = new RenderSurfaceModel( meshMaterial->GetShader( ), gl_buffer, d_mesh );
	meshMaterial->ConfigRenderSurface( d_renderSurface );
	Animate( -1, 0 );
}

void RenderModel::Animate( int anim_id, int frame ) {
	Vec3 v1, v2, v3;
	Plane n1, n2, n3;
	const MD5Skeleton & skeleton = ( anim_id < 0 ) ? d_mesh.GetBaseSkeleton( ) : d_anims[ anim_id ][ frame ];
	BufferVertex3D * vertexBuffer = d_renderSurface->BeginRemapVertex( );
	Common::Com_Memset( vertexBuffer->d_data, 0, vertexBuffer->GetByteSize( ) );
	// Setup vertices
	for( int i = 0; i < d_mesh.GetNumVertices( ); i++ ) {
		// Calculate final vertex to draw with weights
		for( int j = 0; j < d_mesh.GetVertex( i ).GetCount( ); j++ ) {
			const MD5Weight & weight = d_mesh.GetWeight( d_mesh.GetVertex( i ).GetStart( ) + j );
			const MD5Joint & joint = skeleton[ weight.GetJoint( ) ];
			// Calculate transformed vertex for this weight
			v2 = joint.GetOrientation( ) * weight.GetPosition( );
			// The sum of all weight->bias should be 1.0
			v1 = joint.GetPosition( ) + v2;
			v3 = ( *vertexBuffer )[ i ].xyz;
			( *vertexBuffer )[ i ].xyz = v3.Magnitude( weight.GetBias( ), v1 );
		}
		( *vertexBuffer )[ i ].st = d_mesh.GetVertex( i ).GetTexCoord( 0 );
	}
	// Setup vertex indices
	for( int i = 0; i < d_mesh.GetNumTriangles( ); i++ ) {
		Vertex3D & d1 = ( *vertexBuffer )[ d_mesh.GetTriangle( i )[ 0 ] ];
		Vertex3D & d2 = ( *vertexBuffer )[ d_mesh.GetTriangle( i )[ 1 ] ];
		Vertex3D & d3 = ( *vertexBuffer )[ d_mesh.GetTriangle( i )[ 2 ] ];
		v1 = d2.xyz - d3.xyz;
		v2 = d1.xyz - d3.xyz;
		v3.Cross( v1, v2 );
		n1 = d1.normal;
		n2 = d2.normal;
		n3 = d3.normal;
		n1.Normal( ) += v3;
		n2.Normal( ) += v3;
		n3.Normal( ) += v3;
		n1[ 3 ] += 1.0f;
		n2[ 3 ] += 1.0f;
		n3[ 3 ] += 1.0f;
		d1.normal = n1;
		d2.normal = n2;
		d3.normal = n3;
	}
	// Normalize
	for( int i = 0; i < d_mesh.GetNumVertices( ); i++ ) {
		n1 = ( *vertexBuffer )[ i ].normal;
		n1[ 0 ] /= n1[ 3 ];
		n1[ 1 ] /= n1[ 3 ];
		n1[ 2 ] /= n1[ 3 ];
		n1.Normal( ).Normalize( );
		( *vertexBuffer )[ i ].normal = n1;
	}
	d_renderSurface->EndRemapVertex( );
}
