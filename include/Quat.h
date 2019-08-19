// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __MATH_QUAT_H__
#define __MATH_QUAT_H__

/*
===============================================================================

	Quaternion

===============================================================================
*/


class Vec3;
class Angles;
class Rotation;
class Mat3;
class Mat4;
class CQuat;

class Quat {
public:
	float			x;
	float			y;
	float			z;
	float			w;

					Quat( void );
					Quat( float x, float y, float z, float w );

	void 			Set( float x, float y, float z, float w );

	float			operator[]( int index ) const;
	float &			operator[]( int index );
	Quat			operator-() const;
	Quat &		operator=( const Quat &a );
	Quat			operator+( const Quat &a ) const;
	Quat &		operator+=( const Quat &a );
	Quat			operator-( const Quat &a ) const;
	Quat &		operator-=( const Quat &a );
	Quat			operator*( const Quat &a ) const;
	Vec3			operator*( const Vec3 &a ) const;
	Quat			operator*( float a ) const;
	Quat &		operator*=( const Quat &a );
	Quat &		operator*=( float a );

	friend Quat	operator*( const float a, const Quat &b );
	friend Vec3	operator*( const Vec3 &a, const Quat &b );

	bool			Compare( const Quat &a ) const;						// exact compare, no epsilon
	bool			Compare( const Quat &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==(	const Quat &a ) const;					// exact compare, no epsilon
	bool			operator!=(	const Quat &a ) const;					// exact compare, no epsilon

	Quat			Inverse( void ) const;
	float			Length( void ) const;
	Quat &		Normalize( void );

	float			CalcW( void ) const;
	int				GetDimension( void ) const;

	Angles		ToAngles( void ) const;
	Rotation		ToRotation( void ) const;
	Mat3			ToMat3( void ) const;
	Mat4			ToMat4( void ) const;
	CQuat			ToCQuat( void ) const;
	Vec3			ToAngularVelocity( void ) const;
	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;

	Quat &		Slerp( const Quat &from, const Quat &to, float t );
};

INLINE Quat::Quat( void ) {
}

INLINE Quat::Quat( float x, float y, float z, float w ) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

INLINE float Quat::operator[]( int index ) const {
	assert( ( index >= 0 ) && ( index < 4 ) );
	return ( &x )[ index ];
}

INLINE float& Quat::operator[]( int index ) {
	assert( ( index >= 0 ) && ( index < 4 ) );
	return ( &x )[ index ];
}

INLINE Quat Quat::operator-() const {
	return Quat( -x, -y, -z, -w );
}

INLINE Quat &Quat::operator=( const Quat &a ) {
	x = a.x;
	y = a.y;
	z = a.z;
	w = a.w;

	return *this;
}

INLINE Quat Quat::operator+( const Quat &a ) const {
	return Quat( x + a.x, y + a.y, z + a.z, w + a.w );
}

INLINE Quat& Quat::operator+=( const Quat &a ) {
	x += a.x;
	y += a.y;
	z += a.z;
	w += a.w;

	return *this;
}

INLINE Quat Quat::operator-( const Quat &a ) const {
	return Quat( x - a.x, y - a.y, z - a.z, w - a.w );
}

INLINE Quat& Quat::operator-=( const Quat &a ) {
	x -= a.x;
	y -= a.y;
	z -= a.z;
	w -= a.w;

	return *this;
}

INLINE Quat Quat::operator*( const Quat &a ) const {
	return Quat(	w*a.x + x*a.w + y*a.z - z*a.y,
					w*a.y + y*a.w + z*a.x - x*a.z,
					w*a.z + z*a.w + x*a.y - y*a.x,
					w*a.w - x*a.x - y*a.y - z*a.z );
}

INLINE Vec3 Quat::operator*( const Vec3 &a ) const {
#if 0
	// it's faster to do the conversion to a 3x3 matrix and multiply the vector by this 3x3 matrix
	return ( ToMat3() * a );
#else
	// result = this->Inverse() * Quat( a.x, a.y, a.z, 0.0f ) * (*this)
	float xxzz = x*x - z*z;
	float wwyy = w*w - y*y;

	float xw2 = x*w*2.0f;
	float xy2 = x*y*2.0f;
	float xz2 = x*z*2.0f;
	float yw2 = y*w*2.0f;
	float yz2 = y*z*2.0f;
	float zw2 = z*w*2.0f;

	return Vec3(
		(xxzz + wwyy)*a.x		+ (xy2 + zw2)*a.y		+ (xz2 - yw2)*a.z,
		(xy2 - zw2)*a.x			+ (y*y+w*w-x*x-z*z)*a.y	+ (yz2 + xw2)*a.z,
		(xz2 + yw2)*a.x			+ (yz2 - xw2)*a.y		+ (wwyy - xxzz)*a.z
	);
#endif
}

INLINE Quat Quat::operator*( float a ) const {
	return Quat( x * a, y * a, z * a, w * a );
}

INLINE Quat operator*( const float a, const Quat &b ) {
	return b * a;
}

INLINE Vec3 operator*( const Vec3 &a, const Quat &b ) {
	return b * a;
}

INLINE Quat& Quat::operator*=( const Quat &a ) {
	*this = *this * a;

	return *this;
}

INLINE Quat& Quat::operator*=( float a ) {
	x *= a;
	y *= a;
	z *= a;
	w *= a;

	return *this;
}

INLINE bool Quat::Compare( const Quat &a ) const {
	return ( ( x == a.x ) && ( y == a.y ) && ( z == a.z ) && ( w == a.w ) );
}

INLINE bool Quat::Compare( const Quat &a, const float epsilon ) const {
	if ( fabs( x - a.x ) > epsilon ) {
		return false;
	}
	if ( fabs( y - a.y ) > epsilon ) {
		return false;
	}
	if ( fabs( z - a.z ) > epsilon ) {
		return false;
	}
	if ( fabs( w - a.w ) > epsilon ) {
		return false;
	}
	return true;
}

INLINE bool Quat::operator==( const Quat &a ) const {
	return Compare( a );
}

INLINE bool Quat::operator!=( const Quat &a ) const {
	return !Compare( a );
}

INLINE void Quat::Set( float x, float y, float z, float w ) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

INLINE Quat Quat::Inverse( void ) const {
	return Quat( -x, -y, -z, w );
}

INLINE float Quat::Length( void ) const {
	float len;

	len = x * x + y * y + z * z + w * w;
	return sqrtf( len );
}

INLINE Quat& Quat::Normalize( void ) {
	float len;
	float ilength;

	len = this->Length();
	if ( len ) {
		ilength = 1 / len;
		x *= ilength;
		y *= ilength;
		z *= ilength;
		w *= ilength;
	}
	return *this;
}

INLINE float Quat::CalcW( void ) const {
	// take the absolute value because floating point rounding may cause the dot of x,y,z to be larger than 1
	return sqrt( fabs( 1.0f - ( x * x + y * y + z * z ) ) );
}

INLINE int Quat::GetDimension( void ) const {
	return 4;
}

INLINE const float *Quat::ToFloatPtr( void ) const {
	return &x;
}

INLINE float *Quat::ToFloatPtr( void ) {
	return &x;
}


/*
===============================================================================

	Compressed quaternion

===============================================================================
*/

class CQuat {
public:
	float			x;
	float			y;
	float			z;

					CQuat( void );
					CQuat( float x, float y, float z );

	void 			Set( float x, float y, float z );

	float			operator[]( int index ) const;
	float &			operator[]( int index );

	bool			Compare( const CQuat &a ) const;						// exact compare, no epsilon
	bool			Compare( const CQuat &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==(	const CQuat &a ) const;					// exact compare, no epsilon
	bool			operator!=(	const CQuat &a ) const;					// exact compare, no epsilon

	int				GetDimension( void ) const;

	Angles		ToAngles( void ) const;
	Rotation		ToRotation( void ) const;
	Mat3			ToMat3( void ) const;
	Mat4			ToMat4( void ) const;
	Quat			ToQuat( void ) const;
	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;
};

INLINE CQuat::CQuat( void ) {
}

INLINE CQuat::CQuat( float x, float y, float z ) {
	this->x = x;
	this->y = y;
	this->z = z;
}

INLINE void CQuat::Set( float x, float y, float z ) {
	this->x = x;
	this->y = y;
	this->z = z;
}

INLINE float CQuat::operator[]( int index ) const {
	assert( ( index >= 0 ) && ( index < 3 ) );
	return ( &x )[ index ];
}

INLINE float& CQuat::operator[]( int index ) {
	assert( ( index >= 0 ) && ( index < 3 ) );
	return ( &x )[ index ];
}

INLINE bool CQuat::Compare( const CQuat &a ) const {
	return ( ( x == a.x ) && ( y == a.y ) && ( z == a.z ) );
}

INLINE bool CQuat::Compare( const CQuat &a, const float epsilon ) const {
	if ( fabs( x - a.x ) > epsilon ) {
		return false;
	}
	if ( fabs( y - a.y ) > epsilon ) {
		return false;
	}
	if ( fabs( z - a.z ) > epsilon ) {
		return false;
	}
	return true;
}

INLINE bool CQuat::operator==( const CQuat &a ) const {
	return Compare( a );
}

INLINE bool CQuat::operator!=( const CQuat &a ) const {
	return !Compare( a );
}

INLINE int CQuat::GetDimension( void ) const {
	return 3;
}

INLINE Quat CQuat::ToQuat( void ) const {
	// take the absolute value because floating point rounding may cause the dot of x,y,z to be larger than 1
	return Quat( x, y, z, sqrt( fabs( 1.0f - ( x * x + y * y + z * z ) ) ) );
}

INLINE const float *CQuat::ToFloatPtr( void ) const {
	return &x;
}

INLINE float *CQuat::ToFloatPtr( void ) {
	return &x;
}

#endif /* !__MATH_QUAT_H__ */
