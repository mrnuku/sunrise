#include "precompiled.h"
#pragma hdrstop

void BSPTree::BSPBuildParams::PushAndClipBounds( const Plane & clipping_plane, bool back_clip ) {

	Vec3 clippingNormal = back_clip ? -clipping_plane.Normal( ) : clipping_plane.Normal( );
	bool negativeSide = false;
	int index;
	for( int i = 0; i < 3; i++ ) {
		if( FLOATSIGNBITSET( clippingNormal[ i ] ) ) {
			index = i;
			negativeSide = true;
			break;
		}
	}
	if( !negativeSide ) {
		for( int i = 0; i < 3; i++ ) {
			if( clippingNormal[ i ] > 0.0f ) {
				index = i;
				break;
			}
		}
	}
	bool found = false;
	for( int i = 0; i < d_keyPlanes.Num( ); i++ ) {
		if( d_keyPlanes[ i ].Normal( ) == clippingNormal ) {
			d_keyPlanes.RemoveIndex( i );
			found = true;
			break;
		}
	}
	d_stackBounds.Append( d_keyBounds );
	d_keyBounds[ negativeSide ][ index ] = negativeSide ? clipping_plane[ 3 ] : -clipping_plane[ 3 ];
}

void BSPTree::FillNodeBounds_r( BSPNode * node ) {

	if( !node )
		return;
	if( node->d_object ) {
		assert( !node->d_childs[ PLANESIDE_FRONT ] && !node->d_childs[ PLANESIDE_BACK ] );
		node->d_key->d_bounds.Clear( );
		for( int i = 0; i < node->d_object->Num( ); i++ )
			node->d_key->d_bounds.AddBounds( ( *node->d_object )[ i ]->GetBounds( ) );
		return;
	}
	if( node->d_childs[ PLANESIDE_FRONT ] ) {
		FillNodeBounds_r( node->d_childs[ PLANESIDE_FRONT ] );
		node->d_key->d_bounds.AddBounds( node->d_childs[ PLANESIDE_FRONT ]->d_key->d_bounds );
	}
	if( node->d_childs[ PLANESIDE_BACK ] ) {
		FillNodeBounds_r( node->d_childs[ PLANESIDE_BACK ] );
		node->d_key->d_bounds.AddBounds( node->d_childs[ PLANESIDE_BACK ]->d_key->d_bounds );
	}
}

void BSPTree::AddRenderModelStaticList( List< RenderModelStatic * > & ren_mod_list, const Bounds & ren_mod_bounds ) {

	Vec3 mapCenter = ren_mod_bounds.GetCenter( );
	List< Plane > boundAxisPlanes;
	BSPBuildParams buildParams;
	int brushCenterNum = -1;
	float brushLengthSqr = INFINITY;
	for( int i = 0; i < ren_mod_list.Num( ); i++ ) {
		RenderModelStatic * RenderModelStatic = ren_mod_list[ i ];
		float lenSqr = ( RenderModelStatic->GetBounds( ).GetCenter( ) - mapCenter ).LengthSqr( );
		if( brushLengthSqr > lenSqr ) {
			brushLengthSqr = lenSqr;
			brushCenterNum = i;
		}
	}
	if( brushCenterNum != -1 ) {
		RenderModelStatic * RenderModelStatic = ren_mod_list[ brushCenterNum ];
		buildParams.d_keyPlanes.SetNum( 6 );
		RenderModelStatic->GetBounds( ).ToPlanes( buildParams.d_keyPlanes.Ptr( ) );
		buildParams.d_keyBounds = RenderModelStatic->GetBounds( );
		buildParams.d_object = RenderModelStatic;
		if( !d_root )
			d_root = AllocForPlane( &buildParams, 0 );
		AddBrush_r( d_root, &buildParams );
	}
	for( int i = 0; i < ren_mod_list.Num( ); i++ ) {
		if( i == brushCenterNum )
			continue;
		RenderModelStatic * RenderModelStatic = ren_mod_list[ i ];
		buildParams.d_keyPlanes.SetNum( 6 );
		RenderModelStatic->GetBounds( ).ToPlanes( buildParams.d_keyPlanes.Ptr( ) );
		buildParams.d_keyBounds = RenderModelStatic->GetBounds( );
		buildParams.d_object = RenderModelStatic;
		if( !d_root )
			d_root = AllocForPlane( &buildParams, 0 );
		AddBrush_r( d_root, &buildParams );
	}
	FillNodeBounds( );
}

int BSPTree::FindBestNextPlane( BSPNode * node, BSPBuildParams * params ) {

	int best = 0;
	float bestDotAbs = INFINITY;
	for( int i = 0; i < params->d_keyPlanes.Num( ); i++ ) {
		float dotAbs = fabs( node->d_key->d_plane.Normal( ) * params->d_keyPlanes[ i ].Normal( ) );
		if( bestDotAbs > dotAbs ) {
			best = i;
			bestDotAbs = dotAbs;
		}
	}
	return best;
}

BSPNode * BSPTree::AllocForPlane( BSPBuildParams * params, int plane_index ) {

	BSPNode * node = AllocNode( );
	node->d_key = d_keyAllocator.Alloc( 1 );
	node->d_key->d_plane = params->d_keyPlanes[ plane_index ];
	node->d_key->d_bounds.Clear( );
	params->d_keyPlanes.RemoveIndex( plane_index );
	return node;
}

void BSPTree::AllocForObject( BSPNode * node, BSPBuildParams * params, int plane_side ) {

	if( node->d_childs[ plane_side ] )
		return;
	node->d_childs[ plane_side ] = AllocLeaf( );
	node->d_childs[ plane_side ]->d_object = new BSPObjectType;
	node->d_childs[ plane_side ]->d_object->Append( params->d_object );
	node->d_childs[ plane_side ]->d_key = d_keyAllocator.Alloc( 1 );
	node->d_childs[ plane_side ]->d_key->d_plane.Zero( );
}

void BSPTree::AddBrush_r( BSPNode * node, BSPBuildParams * params ) {

	if( node->d_object ) {
		node->d_object->Append( params->d_object );
		return;
	}
	if( !params->d_keyPlanes.Num( ) ) {
		AllocForObject( node, params, PLANESIDE_BACK );
		return;
	}
	int planeSide = params->d_keyBounds.PlaneSide( node->d_key->d_plane, -ON_EPSILON );	
	switch( planeSide ) {

		case PLANESIDE_CROSS:
			params->PushPlanes( );
			params->PushAndClipBounds( node->d_key->d_plane, false );
			if( !params->d_keyPlanes.Num( ) )
				AllocForObject( node, params, PLANESIDE_FRONT );

		case PLANESIDE_FRONT:
			if( params->d_keyPlanes.Num( ) ) {			
				if( !node->d_childs[ PLANESIDE_FRONT ] )
					node->d_childs[ PLANESIDE_FRONT ] = AllocForPlane( params, FindBestNextPlane( node, params ) );
				AddBrush_r( node->d_childs[ PLANESIDE_FRONT ], params );
			}
			if( planeSide == PLANESIDE_FRONT )
				break;
			params->PopPlanes( );
			params->PopBounds( );
			params->PushAndClipBounds( node->d_key->d_plane, true );
			if( !params->d_keyPlanes.Num( ) )
				AllocForObject( node, params, PLANESIDE_BACK );
		
		case PLANESIDE_BACK:			
			if( params->d_keyPlanes.Num( ) ) {
				if( !node->d_childs[ PLANESIDE_BACK ] )
					node->d_childs[ PLANESIDE_BACK ] = AllocForPlane( params, FindBestNextPlane( node, params ) );
				else if( params->ContainsPlane( node->d_key->d_plane ) )
					params->RemovePlane( node->d_key->d_plane );
				AddBrush_r( node->d_childs[ PLANESIDE_BACK ], params );
			}
			if( planeSide == PLANESIDE_BACK )
				break;
			params->PopBounds( );
	}
}

bool BSPTree::BspPvsParams::KeyCull( const Bounds & bounds ) {

	bool haveCrossed = false;
	for( int i = 0; i < d_pvsPlanes.Num( ); i++ ) {
		int side = bounds.PlaneSide( d_pvsPlanes[ i ], 0.0f );
		if( side == PLANESIDE_FRONT )
			return false;
		else if( side == PLANESIDE_CROSS )
			haveCrossed = true;
	}
	if( !haveCrossed && d_enableKeyCheck ) {
		d_pvsLockoutLevel = d_funcLevel;
		d_enableKeyCheck = false;
	}
	return true;
}

void BSPTree::BspPvsParams::AddObject( BSPObjectType * object ) {

	for( int i = 0; i < object->Num( ); i++ ) {
		if( !( *object )[ i ]->GetPvsRef( d_brushRefIndex ) ) {
			bool objectVisible = true;
			const Bounds & bounds = ( *object )[ i ]->GetBounds( );
			for( int j = 0; j < d_pvsPlanes.Num( ); j++ ) {
				if( bounds.PlaneSide( d_pvsPlanes[ j ], 0.0f ) == PLANESIDE_FRONT ) {
					objectVisible = false;
					break;
				}
			}
			if( objectVisible ) {
				const CAListBase< RenderSurfaceMap * > & surfList = ( *object )[ i ]->GetSurfaces( );
				for( int j = 0; j < surfList.Num( ); j++ )
					d_visTree.Add( surfList[ j ], VisTreeKey( surfList[ j ]->GetRenderClass( ), surfList[ j ]->BaseIndex( ) ) );
				( *object )[ i ]->SetPvsRef( d_brushRefIndex, 1 );
			} else
				( *object )[ i ]->SetPvsRef( d_brushRefIndex, -1 );
		}
	}
}

BSPTree::BspPvsParams::BspPvsParams( const CAListBase< Plane > & pvs_planes, VisTree & vis_tree, int brush_index ) :
d_pvsPlanes( pvs_planes ), d_visTree( vis_tree ) {

	d_brushRefIndex = brush_index;
	d_funcLevel = 0;
	d_pvsLockoutLevel = 65536;
	d_enableKeyCheck = true;
}

void BSPTree::GetPVS_r( BSPNode & node, BspPvsParams & params ) {

	if( node.d_object ) {
		params.AddObject( node.d_object );
		return;
	}
	if( params.d_enableKeyCheck && !params.KeyCull( node.d_key->d_bounds ) )
		return;
	params.d_funcLevel++;
	if( node.d_childs[ PLANESIDE_FRONT ] )
		GetPVS_r( *node.d_childs[ PLANESIDE_FRONT ], params );
	if( node.d_childs[ PLANESIDE_BACK  ] )
		GetPVS_r( *node.d_childs[ PLANESIDE_BACK  ], params );

	params.d_funcLevel--;
	if( params.d_pvsLockoutLevel == params.d_funcLevel )
		params.d_enableKeyCheck = true;
}

void BSPTree::GetPVS( const CAListBase< Plane > & pvs_planes, VisTree & ren_mod_tree, int brush_index ) {

	BspPvsParams visParams( pvs_planes, ren_mod_tree, brush_index );
	GetPVS_r( *d_root, visParams );
}

void BSPTree::Clear( ) {

	d_nodeAllocator.Shutdown( );
	d_leafAllocator.Shutdown( );
	d_root = NULL;
	d_nodeAllocator.Init( );
	d_leafAllocator.Init( );
}
