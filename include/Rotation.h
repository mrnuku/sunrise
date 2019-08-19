// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __MATH_ROTATION_H__
#define __MATH_ROTATION_H__

/*
===============================================================================

	Describes a complete rotation in degrees about an abritray axis.
	A local rotation matrix is stored for fast rotation of multiple points.

===============================================================================
*/


class Angles;
class Quat;
class Mat3;

class Rotation {

	friend class Angles;
	friend class Quat;
	friend class Mat3;

public:
						Rotation( void );
						Rotation( const Vec3 &rotationOrigin, const Vec3 &rotationVec, const float rotationAngle );

	void				Set( const Vec3 &rotationOrigin, const Vec3 &rotationVec, const float rotationAngle );
	void				SetOrigin( const Vec3 &rotationOrigin );
	void				SetVec( const Vec3 &rotationVec );					// has to be normalized
	void				SetVec( const float x, const float y, const float z );	// has to be normalized
	void				SetAngle( const float rotationAngle );
	void				Scale( const float s );
	void				ReCalculateMatrix( void );
	const Vec3 &		GetOrigin( void ) const;
	const Vec3 &		GetVec( void ) const;
	float				GetAngle( void ) const;

	Rotation			operator-() const;										// flips rotation
	Rotation			operator*( const float s ) const;						// scale rotation
	Rotation			operator/( const float s ) const;						// scale rotation
	Rotation &			operator*=( const float s );							// scale rotation
	Rotation &			operator/=( const float s );							// scale rotation
	Vec3				operator*( const Vec3 &v ) const;						// rotate vector

	friend Rotation		operator*( const float s, const Rotation &r );		// scale rotation
	friend Vec3			operator*( const Vec3 &v, const Rotation &r );		// rotate vector
	friend Vec3 &		operator*=( Vec3 &v, const Rotation &r );			// rotate vector

	Angles				ToAngles( void ) const;
	Quat				ToQuat( void ) const;
	const Mat3 &		ToMat3( void ) const;
	Mat4				ToMat4( void ) const;
	Vec3				ToAngularVelocity( void ) const;

	void				RotatePoint( Vec3 &point ) const;

	void				Normalize180( void );
	void				Normalize360( void );

private:
	Vec3				origin;			// origin of rotation
	Vec3				vec;			// normalized vector to rotate around
	float				angle;			// angle of rotation in degrees
	mutable Mat3		axis;			// rotation axis
	mutable bool		axisValid;		// true if rotation axis is valid
};


INLINE Rotation::Rotation( void ) {
}

INLINE Rotation::Rotation( const Vec3 &rotationOrigin, const Vec3 &rotationVec, const float rotationAngle ) {
	origin = rotationOrigin;
	vec = rotationVec;
	angle = rotationAngle;
	axisValid = false;
}

INLINE void Rotation::Set( const Vec3 &rotationOrigin, const Vec3 &rotationVec, const float rotationAngle ) {
	origin = rotationOrigin;
	vec = rotationVec;
	angle = rotationAngle;
	axisValid = false;
}

INLINE void Rotation::SetOrigin( const Vec3 &rotationOrigin ) {
	origin = rotationOrigin;
}

INLINE void Rotation::SetVec( const Vec3 &rotationVec ) {
	vec = rotationVec;
	axisValid = false;
}

INLINE void Rotation::SetVec( float x, float y, float z ) {
	vec[0] = x;
	vec[1] = y;
	vec[2] = z;
	axisValid = false;
}

INLINE void Rotation::SetAngle( const float rotationAngle ) {
	angle = rotationAngle;
	axisValid = false;
}

INLINE void Rotation::Scale( const float s ) {
	angle *= s;
	axisValid = false;
}

INLINE void Rotation::ReCalculateMatrix( void ) {
	axisValid = false;
	ToMat3();
}

INLINE const Vec3 &Rotation::GetOrigin( void ) const {
	return origin;
}

INLINE const Vec3 &Rotation::GetVec( void ) const  {
	return vec;
}

INLINE float Rotation::GetAngle( void ) const  {
	return angle;
}

INLINE Rotation Rotation::operator-() const {
	return Rotation( origin, vec, -angle );
}

INLINE Rotation Rotation::operator*( const float s ) const {
	return Rotation( origin, vec, angle * s );
}

INLINE Rotation Rotation::operator/( const float s ) const {
	assert( s != 0.0f );
	return Rotation( origin, vec, angle / s );
}

INLINE Rotation &Rotation::operator*=( const float s ) {
	angle *= s;
	axisValid = false;
	return *this;
}

INLINE Rotation &Rotation::operator/=( const float s ) {
	assert( s != 0.0f );
	angle /= s;
	axisValid = false;
	return *this;
}

INLINE Vec3 Rotation::operator*( const Vec3 &v ) const {
	if ( !axisValid ) {
		ToMat3();
	}
	return ((v - origin) * axis + origin);
}

INLINE Rotation operator*( const float s, const Rotation &r ) {
	return r * s;
}

INLINE Vec3 operator*( const Vec3 &v, const Rotation &r ) {
	return r * v;
}

INLINE Vec3 &operator*=( Vec3 &v, const Rotation &r ) {
	v = r * v;
	return v;
}

INLINE void Rotation::RotatePoint( Vec3 &point ) const {
	if ( !axisValid ) {
		ToMat3();
	}
	point = ((point - origin) * axis + origin);
}

#endif /* !__MATH_ROTATION_H__ */
