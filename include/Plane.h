// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __MATH_PLANE_H__
#define __MATH_PLANE_H__

class Vec3;
class Mat3;

#define	ON_EPSILON					0.1f
#define DEGENERATE_DIST_EPSILON		1e-4f

#define	SIDE_FRONT					0
#define	SIDE_BACK					1
#define	SIDE_ON						2
#define	SIDE_CROSS					3

// plane sides
#define PLANESIDE_FRONT				0
#define PLANESIDE_BACK				1
#define PLANESIDE_ON				2
#define PLANESIDE_CROSS				3

// plane types
#define PLANETYPE_X					0
#define PLANETYPE_Y					1
#define PLANETYPE_Z					2
#define PLANETYPE_NEGX				3
#define PLANETYPE_NEGY				4
#define PLANETYPE_NEGZ				5
#define PLANETYPE_TRUEAXIAL			6	// all types < 6 are true axial planes
#define PLANETYPE_ZEROX				6
#define PLANETYPE_ZEROY				7
#define PLANETYPE_ZEROZ				8
#define PLANETYPE_NONAXIAL			9

// // //// // //// // //// //
/// Plane
//
/// 3D plane with equation: a * x + b * y + c * z + d = 0
// //// // //// // //// //
class Plane {
private:
	/// plane normal x axis
	float						d_a;
	/// plane normal y axis
	float						d_b;
	/// plane normal z axis
	float						d_c;
	/// plane distance
	float						d_d;

public:

								Plane( );
								Plane( float a, float b, float c, float d );
								Plane( const Vec3 &normal, const float dist );

	float						operator[ ]( int index ) const;
	float &						operator[ ]( int index );
	/// flips plane
	Plane						operator-( ) const;
	/// sets normal and sets Plane::d to zero
	Plane &						operator=( const Vec3 & v );
	/// add plane equations
	Plane						operator+( const Plane & p ) const;
	/// subtract plane equations
	Plane						operator-( const Plane & p ) const;
	/// Normal( ) *= m
	Plane &						operator*=( const Mat3 & m );
	Plane						operator *( const float f ) const;
	friend Plane				operator*( const float f, const Plane & b );
	/// exact compare, no epsilon
	bool						Compare( const Plane & p ) const;
	/// compare with epsilon
	bool						Compare( const Plane & p, const float epsilon ) const;
	/// compare with epsilon
	bool						Compare( const Plane & p, const float normalEps, const float distEps ) const;
	/// exact compare, no epsilon
	bool						operator==(	const Plane & p ) const;
	/// exact compare, no epsilon
	bool						operator!=(	const Plane & p ) const;
	/// zero plane
	void						Zero( );
	/// sets the normal
	void						SetNormal( const Vec3 & normal );
	/// reference to const normal
	const Vec3 &				Normal( ) const;
	/// reference to normal
	Vec3 &						Normal( );
	/// only normalizes the plane normal, does not adjust d
	float						Normalize( bool fixDegenerate = true );
	/// fix degenerate normal
	bool						FixDegenerateNormal( );
	/// fix degenerate normal and dist
	bool						FixDegeneracies( float distEpsilon );
	/// returns: -d
	float						Dist( ) const;
	/// sets: d = -dist
	void						SetDist( const float dist );
	/// returns plane type
	int							Type( ) const;
	bool						FromPoints( const Vec3 & p1, const Vec3 & p2, const Vec3 & p3, bool fixDegenerate = true );
	bool						FromVecs( const Vec3 & dir1, const Vec3 & dir2, const Vec3 & p, bool fixDegenerate = true );
	/// assumes normal is valid
	void						FitThroughPoint( const Vec3 & p );
	bool						HeightFit( const Vec3 * points, const int numPoints );
	Plane						Translate( const Vec3 & translation ) const;
	Plane &						TranslateSelf( const Vec3 & translation );
	Plane						Rotate( const Vec3 & origin, const Mat3 & axis ) const;
	Plane &						RotateSelf( const Vec3 & origin, const Mat3 & axis );
	float						Distance( const Vec3 & v ) const;
	int							Side( const Vec3 & v, const float epsilon = 0.0f ) const;
	bool						LineIntersection( const Vec3 & start, const Vec3 & end ) const;
	bool						LineIntersection( const Vec3 & start, const Vec3 & end, float & fraction ) const;
	/// intersection point is start + dir * scale
	bool						RayIntersection( const Vec3 & start, const Vec3 & dir, float & scale ) const;
	bool						PlaneIntersection( const Plane & plane, Vec3 & start, Vec3 & dir ) const;
	bool						PlaneIntersection( const Plane & plane ) const;
	int							GetDimension( ) const;
	const Vec4 &				ToVec4( ) const;
	Vec4 &						ToVec4( );
	const float *				ToFloatPtr( ) const;
	float *						ToFloatPtr( );
	const char *				ToString( int precision = 2 ) const;
	Plane &						operator =( const VecT4hf & a );
								operator VecT4hf( ) const;
};

extern Plane plane_origin;

INLINE Plane::Plane( ) {
}

INLINE Plane::Plane( float a, float b, float c, float d ) {
	d_a = a;
	d_b = b;
	d_c = c;
	d_d = d;
}

INLINE Plane::Plane( const Vec3 &normal, const float dist ) {
	d_a = normal.x;
	d_b = normal.y;
	d_c = normal.z;
	d_d = -dist;
}

INLINE float Plane::operator[]( int index ) const {
	return ( &d_a )[ index ];
}

INLINE float& Plane::operator[]( int index ) {
	return ( &d_a )[ index ];
}

INLINE Plane Plane::operator-() const {
	return Plane( -d_a, -d_b, -d_c, -d_d );
}

INLINE Plane &Plane::operator=( const Vec3 & v ) {
	d_a = v.x;
	d_b = v.y;
	d_c = v.z;
	d_d = 0;
	return *this;
}

INLINE Plane Plane::operator+( const Plane & p ) const {
	return Plane( d_a + p.d_a, d_b + p.d_b, d_c + p.d_c, d_d + p.d_d );
}

INLINE Plane Plane::operator-( const Plane & p ) const {
	return Plane( d_a - p.d_a, d_b - p.d_b, d_c - p.d_c, d_d - p.d_d );
}

INLINE Plane &Plane::operator*=( const Mat3 & m ) {
	Normal( ) *= m;
	return *this;
}

INLINE Plane Plane::operator *( const float f ) const {
	return Plane( d_a * f, d_b * f, d_c * f, d_d * f );
}

INLINE Plane operator*( const float f, const Plane & b ) {
	return Plane( b.d_a * f, b.d_b * f, b.d_c * f, b.d_d * f );
}

INLINE bool Plane::Compare( const Plane & p ) const {
	return ( d_a == p.d_a && d_b == p.d_b && d_c == p.d_c && d_d == p.d_d );
}

INLINE bool Plane::Compare( const Plane & p, const float normalEps, const float distEps ) const {
	if( fabsf( d_d - p.d_d ) > distEps )
		return false;
	if( !Normal( ).Compare( p.Normal( ), normalEps ) )
		return false;
	return true;
}

INLINE bool Plane::operator==( const Plane & p ) const {
	return Compare( p );
}

INLINE bool Plane::operator!=( const Plane & p ) const {
	return !Compare( p );
}

INLINE void Plane::Zero( ) {
	d_a = d_b = d_c = d_d = 0.0f;
}

INLINE void Plane::SetNormal( const Vec3 & normal ) {
	d_a = normal.x;
	d_b = normal.y;
	d_c = normal.z;
}

INLINE const Vec3 &Plane::Normal( ) const {
	return *reinterpret_cast< const Vec3 * >( &d_a );
}

INLINE Vec3 &Plane::Normal( ) {
	return *reinterpret_cast< Vec3 * >( &d_a );
}

INLINE float Plane::Normalize( bool fixDegenerate ) {
	float length = reinterpret_cast< Vec3 * >( &d_a )->Normalize( );
	if ( fixDegenerate )
		FixDegenerateNormal( );
	return length;
}

INLINE bool Plane::FixDegenerateNormal( ) {
	return Normal( ).FixDegenerateNormal( );
}

INLINE float Plane::Dist( ) const {
	return -d_d;
}

INLINE void Plane::SetDist( const float dist ) {
	d_d = -dist;
}

INLINE void Plane::FitThroughPoint( const Vec3 & p ) {
	d_d = -( Normal( ) * p );
}

INLINE Plane Plane::Translate( const Vec3 & translation ) const {
	return Plane( d_a, d_b, d_c, d_d - translation * Normal( ) );
}

INLINE Plane & Plane::TranslateSelf( const Vec3 & translation ) {
	d_d -= translation * Normal( );
	return *this;
}

INLINE Plane Plane::Rotate( const Vec3 & origin, const Mat3 & axis ) const {
	Plane p;
	p.Normal( ) = Normal( ) * axis;
	p.d_d = d_d + origin * Normal( ) - origin * p.Normal( );
	return p;
}

INLINE Plane &Plane::RotateSelf( const Vec3 & origin, const Mat3 & axis ) {
	d_d += origin * Normal( );
	Normal( ) *= axis;
	d_d -= origin * Normal( );
	return *this;
}

INLINE float Plane::Distance( const Vec3 & v ) const {
	return d_a * v.x + d_b * v.y + d_c * v.z + d_d;
}

INLINE int Plane::Side( const Vec3 &v, const float epsilon ) const {
	float dist = Distance( v );
	if( dist > epsilon )
		return PLANESIDE_FRONT;
	else if( dist < -epsilon )
		return PLANESIDE_BACK;
	else
		return PLANESIDE_ON;
}

INLINE int Plane::GetDimension( ) const {
	return 4;
}

INLINE const Vec4 &Plane::ToVec4( ) const {
	return *reinterpret_cast< const Vec4 * >( &d_a );
}

INLINE Vec4 &Plane::ToVec4( ) {
	return *reinterpret_cast< Vec4 * >( &d_a );
}

INLINE const float *Plane::ToFloatPtr( ) const {
	return reinterpret_cast< const float * >( &d_a );
}

INLINE float *Plane::ToFloatPtr( ) {
	return reinterpret_cast< float * >( &d_a );
}

INLINE bool Plane::PlaneIntersection( const Plane & plane ) const {
	float n01 = Normal( ) * plane.Normal( );
	if( fabs( Normal( ).LengthSqr( ) * plane.Normal( ).LengthSqr( ) - n01 * n01 ) < 1e-6f )
		return false;
	return true;
}

INLINE Plane::operator VecT4hf( ) const {
	return VecT4hf( d_a, d_b, d_c, d_d );
}

INLINE Plane & Plane::operator =( const VecT4hf & a ) {
	d_a = a[0];
	d_b = a[1];
	d_c = a[2];
	d_d = a[3];
	return *this;
}

INLINE void Mat4::ToPlanes( Plane planes[ 6 ] ) const {
	// left, right, bottom, top, near, far
	for( int i = 0; i < 6; i++ ) {
		planes[ i ].ToVec4( ) = -( ( i & 1 ) ? mat[ 3 ] - mat[ i >> 1 ] : mat[ i >> 1 ] + mat[ 3 ] );
		planes[ i ].ToVec4( ) /= planes[ i ].Normal( ).Length( );
		planes[ i ].FixDegeneracies( 0.0f );
	}
}

// // //// // //// // //// //
/// Plane2D
//
/// 2D plane with equation: a * x + b * y + d = 0
// //// // //// // //// //
class Plane2D {
private:
	/// plane normal x axis
	float						d_a;
	/// plane normal y axis
	float						d_b;
	/// plane distance
	float						d_d;
	float						d_align;

public:

								Plane2D( );
								Plane2D( float a, float b, float d );
								Plane2D( const Vec2 & normal, const float dist );

	float						operator[ ]( int index ) const;
	float &						operator[ ]( int index );
	/// flips plane
	Plane2D						operator-( ) const;
	/// sets normal and sets Plane::d to zero
	Plane2D &					operator=( const Vec2 & v );
	/// add plane equations
	Plane2D						operator+( const Plane2D & p ) const;
	/// subtract plane equations
	Plane2D						operator-( const Plane2D & p ) const;
	/// Normal( ) *= m
	Plane2D &					operator*=( const Mat2 & m );
	Plane2D						operator *( const float f ) const;
	friend Plane2D				operator*( const float f, const Plane2D & b );
	/// exact compare, no epsilon
	bool						Compare( const Plane2D & p ) const;
	/// compare with epsilon
	bool						Compare( const Plane2D & p, const float epsilon ) const;
	/// compare with epsilon
	bool						Compare( const Plane2D & p, const float normalEps, const float distEps ) const;
	/// exact compare, no epsilon
	bool						operator==(	const Plane2D & p ) const;
	/// exact compare, no epsilon
	bool						operator!=(	const Plane2D & p ) const;
	/// zero plane
	void						Zero( );
	/// sets the normal
	void						SetNormal( const Vec2 & normal );
	/// reference to const normal
	const Vec2 &				Normal( ) const;
	/// reference to normal
	Vec2 &						Normal( );
	/// only normalizes the plane normal, does not adjust d
	float						Normalize( );
	/// fix degenerate normal and dist
	bool						FixDegeneracies( float distEpsilon );
	/// returns: -d
	float						Dist( ) const;
	/// sets: d = -dist
	void						SetDist( const float dist );
	/// returns plane type
	int							Type( ) const;
	void						FromPoints( const Vec2 & start, const Vec2 & end, const bool normalize = false );
	void						FromVecs( const Vec2 & start, const Vec2 & dir, const bool normalize = false );
	/// assumes normal is valid
	void						FitThroughPoint( const Vec2 & p );
	bool						HeightFit( const Vec2 * points, const int numPoints );
	Plane2D						Translate( const Vec2 & translation ) const;
	Plane2D &					TranslateSelf( const Vec2 & translation );
	Plane2D						Rotate( const Vec2 & origin, const Mat2 & axis ) const;
	Plane2D &					RotateSelf( const Vec2 & origin, const Mat2 & axis );
	float						Distance( const Vec2 & v ) const;
	int							Side( const Vec2 & v, const float epsilon = 0.0f ) const;
	bool						LineIntersection( const Vec2 & start, const Vec2 & end ) const;
	bool						LineIntersection( const Vec2 & start, const Vec2 & end, float & fraction ) const;
	/// intersection point is start + dir * scale
	bool						RayIntersection( const Vec2 & start, const Vec2 & dir, float & scale ) const;
	bool						PlaneIntersection( const Plane2D & plane, Vec2 & point ) const;
	int							GetDimension( ) const;
	const Vec3 &				ToVec3( ) const;
	Vec3 &						ToVec3( );
	const float *				ToFloatPtr( ) const;
	float *						ToFloatPtr( );
	const char *				ToString( int precision = 2 ) const;
	bool						GetAxialBevel( const Plane2D & plane1, const Plane2D & plane2, const Vec2 & point );
};

extern Plane2D plane2d_origin;

// // //// // //// // //// //
// Plane2D
// //// // //// // //// //

INLINE Plane2D::Plane2D( ) {
}

INLINE Plane2D::Plane2D( float a, float b, float d ) {
	d_a = a;
	d_b = b;
	d_d = d;
}

INLINE Plane2D::Plane2D( const Vec2 & normal, const float dist ) {
	d_a = normal.d_x;
	d_b = normal.d_y;
	d_d = -dist;
}

INLINE float Plane2D::operator[ ]( int index ) const {
	return ( &d_a )[ index ];
}

INLINE float& Plane2D::operator[ ]( int index ) {
	return ( &d_a )[ index ];
}

INLINE Plane2D Plane2D::operator-( ) const {
	return Plane2D( -d_a, -d_b, -d_d );
}

INLINE Plane2D & Plane2D::operator =( const Vec2 & v ) {
	d_a = v.d_x;
	d_b = v.d_y;
	d_d = 0;
	return *this;
}

INLINE Plane2D Plane2D::operator+( const Plane2D & p ) const {
	return Plane2D( d_a + p.d_a, d_b + p.d_b, d_d + p.d_d );
}

INLINE Plane2D Plane2D::operator-( const Plane2D & p ) const {
	return Plane2D( d_a - p.d_a, d_b - p.d_b, d_d - p.d_d );
}

INLINE Plane2D & Plane2D::operator *=( const Mat2 & m ) {
	Normal( ) *= m;
	return *this;
}

INLINE Plane2D Plane2D::operator *( const float f ) const {
	return Plane2D( d_a * f, d_b * f, d_d * f );
}

INLINE Plane2D operator*( const float f, const Plane2D & b ) {
	return Plane2D( b.d_a * f, b.d_b * f, b.d_d * f );
}

INLINE bool Plane2D::Compare( const Plane2D & p ) const {
	return ( d_a == p.d_a && d_b == p.d_b && d_d == p.d_d );
}

INLINE bool Plane2D::Compare( const Plane2D & p, const float normalEps, const float distEps ) const {
	if( fabsf( d_d - p.d_d ) > distEps )
		return false;
	if( !Normal( ).Compare( p.Normal( ), normalEps ) )
		return false;
	return true;
}

INLINE bool Plane2D::operator ==( const Plane2D & p ) const {
	return Compare( p );
}

INLINE bool Plane2D::operator !=( const Plane2D & p ) const {
	return !Compare( p );
}

INLINE void Plane2D::Zero( ) {
	d_a = d_b = d_d = 0.0f;
}

INLINE void Plane2D::SetNormal( const Vec2 & normal ) {
	d_a = normal.d_x;
	d_b = normal.d_y;
}

INLINE const Vec2 & Plane2D::Normal( ) const {
	return *reinterpret_cast< const Vec2 * >( &d_a );
}

INLINE Vec2 & Plane2D::Normal( ) {
	return *reinterpret_cast< Vec2 * >( &d_a );
}

INLINE float Plane2D::Normalize( ) {
	float length = reinterpret_cast< Vec2 * >( &d_a )->Normalize( );
	return length;
}

INLINE float Plane2D::Dist( ) const {
	return -d_d;
}

INLINE void Plane2D::SetDist( const float dist ) {
	d_d = -dist;
}

INLINE void Plane2D::FitThroughPoint( const Vec2 & p ) {
	d_d = -( Normal( ) * p );
}

INLINE Plane2D Plane2D::Translate( const Vec2 & translation ) const {
	return Plane2D( d_a, d_b, d_d - translation * Normal( ) );
}

INLINE Plane2D & Plane2D::TranslateSelf( const Vec2 & translation ) {
	d_d -= translation * Normal( );
	return *this;
}

INLINE Plane2D Plane2D::Rotate( const Vec2 & origin, const Mat2 & axis ) const {
	Plane2D p;
	p.Normal( ) = Normal( ) * axis;
	p.d_d = d_d + origin * Normal( ) - origin * p.Normal( );
	return p;
}

INLINE Plane2D & Plane2D::RotateSelf( const Vec2 & origin, const Mat2 & axis ) {
	d_d += origin * Normal( );
	Normal( ) *= axis;
	d_d -= origin * Normal( );
	return *this;
}

INLINE float Plane2D::Distance( const Vec2 & v ) const {
	return d_a * v.d_x + d_b * v.d_y + d_d;
}

INLINE int Plane2D::Side( const Vec2 & v, const float epsilon ) const {
	float dist = Distance( v );
	if( dist > epsilon )
		return PLANESIDE_FRONT;
	else if( dist < -epsilon )
		return PLANESIDE_BACK;
	else
		return PLANESIDE_ON;
}

INLINE int Plane2D::GetDimension( ) const {
	return 3;
}

INLINE const Vec3 & Plane2D::ToVec3( ) const {
	return *reinterpret_cast< const Vec3 * >( &d_a );
}

INLINE Vec3 & Plane2D::ToVec3( ) {
	return *reinterpret_cast< Vec3 * >( &d_a );
}

INLINE const float * Plane2D::ToFloatPtr( ) const {
	return reinterpret_cast< const float * >( &d_a );
}

INLINE float * Plane2D::ToFloatPtr( ) {
	return reinterpret_cast< float * >( &d_a );
}

INLINE void Plane2D::FromPoints( const Vec2 & start, const Vec2 & end, const bool normalize ) {
	d_a = start.d_y - end.d_y;
	d_b = end.d_x - start.d_x;
	if( normalize )
		Normalize( );
	d_d = -( start.d_x * d_a + start.d_y * d_b );
}

INLINE void Plane2D::FromVecs( const Vec2 & start, const Vec2 & dir, const bool normalize ) {
	d_a = -dir.d_y;
	d_b = dir.d_x;
	if( normalize )
		Normalize( );
	d_d = -( start.d_x * d_a + start.d_y * d_b );
}

INLINE bool Plane2D::PlaneIntersection( const Plane2D & plane, Vec2 & point ) const {
	float n00 = d_a * d_a + d_b * d_b;
	float n01 = d_a * plane.d_a + d_b * plane.d_b;
	float n11 = plane.d_a * plane.d_a + plane.d_b * plane.d_b;
	float det = n00 * n11 - n01 * n01;
	if( fabsf( det ) < 1e-6f )
		return false;
	float invDet = 1.0f / det;
	float f0 = ( n01 * plane.d_d - n11 * d_d ) * invDet;
	float f1 = ( n01 * d_d - n00 * plane.d_d ) * invDet;
	point.d_x = f0 * d_a + f1 * plane.d_a;
	point.d_y = f0 * d_b + f1 * plane.d_b;
	return true;
}

#endif
