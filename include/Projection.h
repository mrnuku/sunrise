#ifndef PROJECTION_H
#define PROJECTION_H

// // //// // //// // //// //
/// Projection
// //// // //// // //// //

class Projection : public Mat4 {
private:
	float											d_near;
	float											d_far;

public:											
	Projection &									OrthogonalSelf( float in_left, float in_right, float in_bottom, float in_top, float in_near, float in_far );
	static Projection								Orthogonal( float in_left, float in_right, float in_bottom, float in_top, float in_near, float in_far );
	Projection &									PerspectiveSelf( float in_near, float in_far, float in_fov_y, float in_aspect );
	static Projection								Perspective( float in_near, float in_far, float in_fov_y, float in_aspect );

	Projection &									OrthoLightSelf( float in_dist );
	static Projection								OrthoLight( float in_dist );

	void											ToInvPlanes( const Bounds2D & scissor, Plane planes[ 6 ] ) const;
};

// // //// // //// // //// //
// Projection
// //// // //// // //// //

INLINE Projection Projection::Orthogonal( float in_left, float in_right, float in_bottom, float in_top, float in_near, float in_far ) {
	Projection newProjection;
	newProjection.OrthogonalSelf( in_left, in_right, in_bottom, in_top, in_near, in_far );
	return newProjection;
}

INLINE Projection Projection::Perspective( float in_near, float in_far, float in_fov_y, float in_aspect ) {
	Projection newProjection;
	newProjection.PerspectiveSelf( in_near, in_far, in_fov_y, in_aspect );
	return newProjection;
}

INLINE Projection Projection::OrthoLight( float in_dist ) {
	Projection newProjection;
	newProjection.OrthoLightSelf( in_dist );
	return newProjection;
}

#endif
