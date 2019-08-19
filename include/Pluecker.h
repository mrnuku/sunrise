// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __MATH_PLUECKER_H__
#define __MATH_PLUECKER_H__

/*
===============================================================================

	Pluecker coordinate

===============================================================================
*/

class Pluecker {
public:	
					Pluecker( void );
					explicit Pluecker( const float *a );
					explicit Pluecker( const Vec3 &start, const Vec3 &end );
					explicit Pluecker( const float a1, const float a2, const float a3, const float a4, const float a5, const float a6 );

	float			operator[]( const int index ) const;
	float &			operator[]( const int index );
	Pluecker		operator-() const;											// flips the direction
	Pluecker		operator*( const float a ) const;
	Pluecker		operator/( const float a ) const;
	float			operator*( const Pluecker &a ) const;						// permuted inner product
	Pluecker		operator-( const Pluecker &a ) const;
	Pluecker		operator+( const Pluecker &a ) const;
	Pluecker &	operator*=( const float a );
	Pluecker &	operator/=( const float a );
	Pluecker &	operator+=( const Pluecker &a );
	Pluecker &	operator-=( const Pluecker &a );

	bool			Compare( const Pluecker &a ) const;						// exact compare, no epsilon
	bool			Compare( const Pluecker &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==(	const Pluecker &a ) const;					// exact compare, no epsilon
	bool			operator!=(	const Pluecker &a ) const;					// exact compare, no epsilon

	void 			Set( const float a1, const float a2, const float a3, const float a4, const float a5, const float a6 );
	void			Zero( void );

	void			FromLine( const Vec3 &start, const Vec3 &end );			// pluecker from line
	void			FromRay( const Vec3 &start, const Vec3 &dir );			// pluecker from ray
	bool			FromPlanes( const Plane &p1, const Plane &p2 );			// pluecker from intersection of planes
	bool			ToLine( Vec3 &start, Vec3 &end ) const;					// pluecker to line
	bool			ToRay( Vec3 &start, Vec3 &dir ) const;					// pluecker to ray
	void			ToDir( Vec3 &dir ) const;									// pluecker to direction
	float			PermutedInnerProduct( const Pluecker &a ) const;			// pluecker permuted inner product
	float			Distance3DSqr( const Pluecker &a ) const;					// pluecker line distance

	float			Length( void ) const;										// pluecker length
	float			LengthSqr( void ) const;									// pluecker squared length
	Pluecker		Normalize( void ) const;									// pluecker normalize
	float			NormalizeSelf( void );										// pluecker normalize

	int				GetDimension( void ) const;

	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;

private:
	float			p[6];
};

extern Pluecker pluecker_origin;
#define pluecker_zero pluecker_origin

INLINE Pluecker::Pluecker( void ) {
}

INLINE Pluecker::Pluecker( const float *a ) {
	memcpy( p, a, 6 * sizeof( float ) );
}

INLINE Pluecker::Pluecker( const Vec3 &start, const Vec3 &end ) {
	FromLine( start, end );
}

INLINE Pluecker::Pluecker( const float a1, const float a2, const float a3, const float a4, const float a5, const float a6 ) {
	p[0] = a1;
	p[1] = a2;
	p[2] = a3;
	p[3] = a4;
	p[4] = a5;
	p[5] = a6;
}

INLINE Pluecker Pluecker::operator-() const {
	return Pluecker( -p[0], -p[1], -p[2], -p[3], -p[4], -p[5] );
}

INLINE float Pluecker::operator[]( const int index ) const {
	return p[index];
}

INLINE float &Pluecker::operator[]( const int index ) {
	return p[index];
}

INLINE Pluecker Pluecker::operator*( const float a ) const {
	return Pluecker( p[0]*a, p[1]*a, p[2]*a, p[3]*a, p[4]*a, p[5]*a );
}

INLINE float Pluecker::operator*( const Pluecker &a ) const {
	return p[0] * a.p[4] + p[1] * a.p[5] + p[2] * a.p[3] + p[4] * a.p[0] + p[5] * a.p[1] + p[3] * a.p[2];
}

INLINE Pluecker Pluecker::operator/( const float a ) const {
	float inva;

	assert( a != 0.0f );
	inva = 1.0f / a;
	return Pluecker( p[0]*inva, p[1]*inva, p[2]*inva, p[3]*inva, p[4]*inva, p[5]*inva );
}

INLINE Pluecker Pluecker::operator+( const Pluecker &a ) const {
	return Pluecker( p[0] + a[0], p[1] + a[1], p[2] + a[2], p[3] + a[3], p[4] + a[4], p[5] + a[5] );
}

INLINE Pluecker Pluecker::operator-( const Pluecker &a ) const {
	return Pluecker( p[0] - a[0], p[1] - a[1], p[2] - a[2], p[3] - a[3], p[4] - a[4], p[5] - a[5] );
}

INLINE Pluecker &Pluecker::operator*=( const float a ) {
	p[0] *= a;
	p[1] *= a;
	p[2] *= a;
	p[3] *= a;
	p[4] *= a;
	p[5] *= a;
	return *this;
}

INLINE Pluecker &Pluecker::operator/=( const float a ) {
	float inva;

	assert( a != 0.0f );
	inva = 1.0f / a;
	p[0] *= inva;
	p[1] *= inva;
	p[2] *= inva;
	p[3] *= inva;
	p[4] *= inva;
	p[5] *= inva;
	return *this;
}

INLINE Pluecker &Pluecker::operator+=( const Pluecker &a ) {
	p[0] += a[0];
	p[1] += a[1];
	p[2] += a[2];
	p[3] += a[3];
	p[4] += a[4];
	p[5] += a[5];
	return *this;
}

INLINE Pluecker &Pluecker::operator-=( const Pluecker &a ) {
	p[0] -= a[0];
	p[1] -= a[1];
	p[2] -= a[2];
	p[3] -= a[3];
	p[4] -= a[4];
	p[5] -= a[5];
	return *this;
}

INLINE bool Pluecker::Compare( const Pluecker &a ) const {
	return ( ( p[0] == a[0] ) && ( p[1] == a[1] ) && ( p[2] == a[2] ) &&
			( p[3] == a[3] ) && ( p[4] == a[4] ) && ( p[5] == a[5] ) );
}

INLINE bool Pluecker::Compare( const Pluecker &a, const float epsilon ) const {
	if ( fabs( p[0] - a[0] ) > epsilon ) {
		return false;
	}
			
	if ( fabs( p[1] - a[1] ) > epsilon ) {
		return false;
	}

	if ( fabs( p[2] - a[2] ) > epsilon ) {
		return false;
	}

	if ( fabs( p[3] - a[3] ) > epsilon ) {
		return false;
	}

	if ( fabs( p[4] - a[4] ) > epsilon ) {
		return false;
	}

	if ( fabs( p[5] - a[5] ) > epsilon ) {
		return false;
	}

	return true;
}

INLINE bool Pluecker::operator==( const Pluecker &a ) const {
	return Compare( a );
}

INLINE bool Pluecker::operator!=( const Pluecker &a ) const {
	return !Compare( a );
}

INLINE void Pluecker::Set( const float a1, const float a2, const float a3, const float a4, const float a5, const float a6 ) {
	p[0] = a1;
	p[1] = a2;
	p[2] = a3;
	p[3] = a4;
	p[4] = a5;
	p[5] = a6;
}

INLINE void Pluecker::Zero( void ) {
	p[0] = p[1] = p[2] = p[3] = p[4] = p[5] = 0.0f;
}

INLINE void Pluecker::FromLine( const Vec3 &start, const Vec3 &end ) {
	p[0] = start[0] * end[1] - end[0] * start[1];
	p[1] = start[0] * end[2] - end[0] * start[2];
	p[2] = start[0] - end[0];
	p[3] = start[1] * end[2] - end[1] * start[2];
	p[4] = start[2] - end[2];
	p[5] = end[1] - start[1];
}

INLINE void Pluecker::FromRay( const Vec3 &start, const Vec3 &dir ) {
	p[0] = start[0] * dir[1] - dir[0] * start[1];
	p[1] = start[0] * dir[2] - dir[0] * start[2];
	p[2] = -dir[0];
	p[3] = start[1] * dir[2] - dir[1] * start[2];
	p[4] = -dir[2];
	p[5] = dir[1];
}

INLINE bool Pluecker::ToLine( Vec3 &start, Vec3 &end ) const {
	Vec3 dir1, dir2;
	float d;

	dir1[0] = p[3];
	dir1[1] = -p[1];
	dir1[2] = p[0];

	dir2[0] = -p[2];
	dir2[1] = p[5];
	dir2[2] = -p[4];

	d = dir2 * dir2;
	if ( d == 0.0f ) {
		return false; // pluecker coordinate does not represent a line
	}

	start = dir2.Cross(dir1) * (1.0f / d);
	end = start + dir2;
	return true;
}

INLINE bool Pluecker::ToRay( Vec3 &start, Vec3 &dir ) const {
	Vec3 dir1;
	float d;

	dir1[0] = p[3];
	dir1[1] = -p[1];
	dir1[2] = p[0];

	dir[0] = -p[2];
	dir[1] = p[5];
	dir[2] = -p[4];

	d = dir * dir;
	if ( d == 0.0f ) {
		return false; // pluecker coordinate does not represent a line
	}

	start = dir.Cross(dir1) * (1.0f / d);
	return true;
}

INLINE void Pluecker::ToDir( Vec3 &dir ) const {
	dir[0] = -p[2];
	dir[1] = p[5];
	dir[2] = -p[4];
}

INLINE float Pluecker::PermutedInnerProduct( const Pluecker &a ) const {
	return p[0] * a.p[4] + p[1] * a.p[5] + p[2] * a.p[3] + p[4] * a.p[0] + p[5] * a.p[1] + p[3] * a.p[2];
}

INLINE float Pluecker::Length( void ) const {
	return ( float )sqrtf( p[5] * p[5] + p[4] * p[4] + p[2] * p[2] );
}

INLINE float Pluecker::LengthSqr( void ) const {
	return ( p[5] * p[5] + p[4] * p[4] + p[2] * p[2] );
}

INLINE float Pluecker::NormalizeSelf( void ) {
	float l, d;

	l = LengthSqr();
	if ( l == 0.0f ) {
		return l; // pluecker coordinate does not represent a line
	}
	d = 1.0f / sqrtf( l );
	p[0] *= d;
	p[1] *= d;
	p[2] *= d;
	p[3] *= d;
	p[4] *= d;
	p[5] *= d;
	return d * l;
}

INLINE Pluecker Pluecker::Normalize( void ) const {
	float d;

	d = LengthSqr();
	if ( d == 0.0f ) {
		return *this; // pluecker coordinate does not represent a line
	}
	d = 1.0f / sqrtf( d );
	return Pluecker( p[0]*d, p[1]*d, p[2]*d, p[3]*d, p[4]*d, p[5]*d );
}

INLINE int Pluecker::GetDimension( void ) const {
	return 6;
}

INLINE const float *Pluecker::ToFloatPtr( void ) const {
	return p;
}

INLINE float *Pluecker::ToFloatPtr( void ) {
	return p;
}

#endif /* !__MATH_PLUECKER_H__ */
