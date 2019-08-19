// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __Polytope_H__
#define __Polytope_H__

/*
===============================================================================

	Polytope surface.

	NOTE: vertexes are not duplicated for texture coordinates.

===============================================================================
*/

class PlaneSet;

class Polytope : public Surface {
public:
						Polytope( void );

	void				FromPlanes( const Plane *planes, const int numPlanes );
	Polytope &			FromPlanes( const PlaneSet & plane_set );

	void				SetupHelix( const Vec3 & origin, float radius, int twists );
	void				SetupTetrahedron( const Bounds &bounds );
	void				SetupHexahedron( const Bounds &bounds );
	void				SetupOctahedron( const Bounds &bounds );
	void				SetupDodecahedron( const Bounds &bounds );
	void				SetupIcosahedron( const Bounds &bounds );
	void				SetupCylinder( const Bounds &bounds, const int numSides );
	void				SetupCone( const Bounds &bounds, const int numSides );

	int					SplitPolytope( const Plane &plane, const float epsilon, Polytope **front, Polytope **back ) const;

protected:

};

/*
====================
Polytope::Polytope
====================
*/
INLINE Polytope::Polytope( void ) {
}

#endif /* !__Polytope_H__ */
