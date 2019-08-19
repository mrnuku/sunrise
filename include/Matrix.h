#ifndef MATH_MATRIX_H
#define MATH_MATRIX_H

/*
===============================================================================

  Matrix classes, all matrices are row-major except Mat3

===============================================================================
*/

#define MATRIX_INVERSE_EPSILON		1e-14
#define MATRIX_EPSILON				1e-6

class Angles;
class Quat;
class CQuat;
class Rotation;
class Mat4;

// // //// // //// // //// //
/// Mat2
// //// // //// // //// //

class Mat2 {
private:
	Vec2					d_mat[ 2 ];

public:
							Mat2( );
							explicit Mat2( const Vec2 & x, const Vec2 & y );
							explicit Mat2( const float xx, const float xy, const float yx, const float yy );
							explicit Mat2( const float src[ 2 ][ 2 ] );
	const Vec2 &			operator [ ]( int index ) const;
	Vec2 &					operator [ ]( int index );
	Mat2					operator -( ) const;
	Mat2					operator *( const float a ) const;
	Vec2					operator *( const Vec2 & vec ) const;
	Mat2					operator *( const Mat2 & a ) const;
	Mat2					operator +( const Mat2 & a ) const;
	Mat2					operator -( const Mat2 & a ) const;
	Mat2 &					operator *=( const float a );
	Mat2 &					operator *=( const Mat2 & a );
	Mat2 &					operator +=( const Mat2 & a );
	Mat2 &					operator -=( const Mat2 & a );
	friend Mat2				operator *( const float a, const Mat2 & mat );
	friend Vec2				operator *( const Vec2 & vec, const Mat2 & mat );
	friend Vec2 &			operator *=( Vec2 & vec, const Mat2 & mat );
	/// exact compare, no epsilon
	bool					Compare( const Mat2 & a ) const;
	/// compare with epsilon
	bool					Compare( const Mat2 & a, const float epsilon ) const;
	/// exact compare, no epsilon
	bool					operator ==( const Mat2 & a ) const;
	/// exact compare, no epsilon
	bool					operator !=( const Mat2 & a ) const;
	void					Zero( );
	void					Identity( );
	bool					IsIdentity( const float epsilon = MATRIX_EPSILON ) const;
	bool					IsSymmetric( const float epsilon = MATRIX_EPSILON ) const;
	bool					IsDiagonal( const float epsilon = MATRIX_EPSILON ) const;
	float					Trace( ) const;
	float					Determinant( ) const;
	/// returns transpose
	Mat2					Transpose( ) const;
	Mat2 &					TransposeSelf( );
	/// returns the inverse ( m * m.Inverse( ) = identity )
	Mat2					Inverse( ) const;
	/// returns false if determinant is zero
	bool					InverseSelf( );
	/// returns the inverse ( m * m.Inverse( ) = identity )
	Mat2					InverseFast( ) const;
	/// returns false if determinant is zero
	bool					InverseFastSelf( );
	int						GetDimension( ) const;
	const float *			ToFloatPtr( ) const;
	float *					ToFloatPtr( );
	const char *			ToString( int precision = 2 ) const;
};

extern Mat2 mat2_zero;
extern Mat2 mat2_identity;

// // //// // //// // //// //
// Mat2
// //// // //// // //// //

INLINE Mat2::Mat2( ) {
}

INLINE Mat2::Mat2( const Vec2 & x, const Vec2 & y ) {
	d_mat[ 0 ].d_x = x.d_x;
	d_mat[ 0 ].d_y = x.d_y;
	d_mat[ 1 ].d_x = y.d_x;
	d_mat[ 1 ].d_y = y.d_y;
}

INLINE Mat2::Mat2( const float xx, const float xy, const float yx, const float yy ) {
	d_mat[ 0 ].d_x = xx;
	d_mat[ 0 ].d_y = xy;
	d_mat[ 1 ].d_x = yx;
	d_mat[ 1 ].d_y = yy;
}

INLINE Mat2::Mat2( const float src[ 2 ][ 2 ] ) {
	Common::Com_Memcpy( d_mat, src, NumSizeOf( float, 2 * 2 ) );
}

INLINE const Vec2 & Mat2::operator [ ]( int index ) const {
	assert( ( index >= 0 ) && ( index < 2 ) );
	return d_mat[ index ];
}

INLINE Vec2 & Mat2::operator [ ]( int index ) {
	assert( ( index >= 0 ) && ( index < 2 ) );
	return d_mat[ index ];
}

INLINE Mat2 Mat2::operator -( ) const {
	return Mat2( -d_mat[ 0 ][ 0 ], -d_mat[ 0 ][ 1 ], -d_mat[ 1 ][ 0 ], -d_mat[ 1 ][ 1 ] );
}

INLINE Vec2 Mat2::operator *( const Vec2 & vec ) const {
	return Vec2( d_mat[ 0 ].d_x * vec.d_x + d_mat[ 0 ].d_y * vec.d_y, d_mat[ 1 ].d_x * vec.d_x + d_mat[ 1 ].d_y * vec.d_y );
}

INLINE Mat2 Mat2::operator *( const Mat2 & a ) const {
	return Mat2(
		d_mat[ 0 ].d_x * a[ 0 ].d_x + d_mat[ 0 ].d_y * a[ 1 ].d_x,
		d_mat[ 0 ].d_x * a[ 0 ].d_y + d_mat[ 0 ].d_y * a[ 1 ].d_y,
		d_mat[ 1 ].d_x * a[ 0 ].d_x + d_mat[ 1 ].d_y * a[ 1 ].d_x,
		d_mat[ 1 ].d_x * a[ 0 ].d_y + d_mat[ 1 ].d_y * a[ 1 ].d_y );
}

INLINE Mat2 Mat2::operator *( const float a ) const {
	return Mat2( d_mat[ 0 ].d_x * a, d_mat[ 0 ].d_y * a, d_mat[ 1 ].d_x * a, d_mat[ 1 ].d_y * a );
}

INLINE Mat2 Mat2::operator +( const Mat2 & a ) const {
	return Mat2( d_mat[ 0 ].d_x + a[ 0 ].d_x, d_mat[ 0 ].d_y + a[ 0 ].d_y, d_mat[ 1 ].d_x + a[ 1 ].d_x, d_mat[ 1 ].d_y + a[ 1 ].d_y );
}
    
INLINE Mat2 Mat2::operator -( const Mat2 & a ) const {
	return Mat2( d_mat[ 0 ].d_x - a[ 0 ].d_x, d_mat[ 0 ].d_y - a[ 0 ].d_y, d_mat[ 1 ].d_x - a[ 1 ].d_x, d_mat[ 1 ].d_y - a[ 1 ].d_y );
}

INLINE Mat2 & Mat2::operator *=( const float a ) {
	d_mat[ 0 ].d_x *= a;
	d_mat[ 0 ].d_y *= a;
	d_mat[ 1 ].d_x *= a;
	d_mat[ 1 ].d_y *= a;
    return *this;
}

INLINE Mat2 & Mat2::operator *=( const Mat2 & a ) {
	float x = d_mat[ 0 ].d_x;
	float y = d_mat[ 0 ].d_y;
	d_mat[ 0 ].d_x = x * a[ 0 ].d_x + y * a[ 1 ].d_x;
	d_mat[ 0 ].d_y = x * a[ 0 ].d_y + y * a[ 1 ].d_y;
	x = d_mat[ 1 ].d_x;
	y = d_mat[ 1 ].d_y;
	d_mat[ 1 ].d_x = x * a[ 0 ].d_x + y * a[ 1 ].d_x;
	d_mat[ 1 ].d_y = x * a[ 0 ].d_y + y * a[ 1 ].d_y;
	return *this;
}

INLINE Mat2 & Mat2::operator +=( const Mat2 & a ) {
	d_mat[ 0 ].d_x += a[ 0 ].d_x;
	d_mat[ 0 ].d_y += a[ 0 ].d_y;
	d_mat[ 1 ].d_x += a[ 1 ].d_x;
	d_mat[ 1 ].d_y += a[ 1 ].d_y;
    return *this;
}

INLINE Mat2 & Mat2::operator -=( const Mat2 & a ) {
	d_mat[ 0 ].d_x -= a[ 0 ].d_x;
	d_mat[ 0 ].d_y -= a[ 0 ].d_y;
	d_mat[ 1 ].d_x -= a[ 1 ].d_x;
	d_mat[ 1 ].d_y -= a[ 1 ].d_y;
    return *this;
}

INLINE Vec2 operator *( const Vec2 & vec, const Mat2 & mat ) {
	return mat * vec;
}

INLINE Mat2 operator *( const float a, Mat2 const & mat ) {
	return mat * a;
}

INLINE Vec2 & operator *=( Vec2 & vec, const Mat2 & mat ) {
	vec = mat * vec;
	return vec;
}

INLINE bool Mat2::Compare( const Mat2 & a ) const {
	if( d_mat[ 0 ].Compare( a[ 0 ] ) && d_mat[ 1 ].Compare( a[ 1 ] ) )
		return true;
	return false;
}

INLINE bool Mat2::Compare( const Mat2 & a, const float epsilon ) const {
	if( d_mat[ 0 ].Compare( a[ 0 ], epsilon ) && d_mat[ 1 ].Compare( a[ 1 ], epsilon ) )
		return true;
	return false;
}

INLINE bool Mat2::operator ==( const Mat2 & a ) const {
	return Compare( a );
}

INLINE bool Mat2::operator !=( const Mat2 & a ) const {
	return !Compare( a );
}

INLINE void Mat2::Zero( ) {
	d_mat[ 0 ].Zero( );
	d_mat[ 1 ].Zero( );
}

INLINE void Mat2::Identity( ) {
	*this = mat2_identity;
}

INLINE bool Mat2::IsIdentity( const float epsilon ) const {
	return Compare( mat2_identity, epsilon );
}

INLINE bool Mat2::IsSymmetric( const float epsilon ) const {
	return ( fabsf( d_mat[ 0 ][ 1 ] - d_mat[ 1 ][ 0 ] ) < epsilon );
}

INLINE bool Mat2::IsDiagonal( const float epsilon ) const {
	if( fabsf( d_mat[ 0 ][ 1 ] ) > epsilon || fabs( d_mat[ 1 ][ 0 ] ) > epsilon )
		return false;
	return true;
}

INLINE float Mat2::Trace( ) const {
	return ( d_mat[ 0 ][ 0 ] + d_mat[ 1 ][ 1 ] );
}

INLINE float Mat2::Determinant( ) const {
	return d_mat[ 0 ][ 0 ] * d_mat[ 1 ][ 1 ] - d_mat[ 0 ][ 1 ] * d_mat[ 1 ][ 0 ];
}

INLINE Mat2 Mat2::Transpose( ) const {
	return Mat2( d_mat[ 0 ][ 0 ], d_mat[ 1 ][ 0 ], d_mat[ 0 ][ 1 ], d_mat[ 1 ][ 1 ] );
}

INLINE Mat2 & Mat2::TransposeSelf( ) {
	Swap( d_mat[ 0 ][ 1 ], d_mat[ 1 ][ 0 ] );
	return *this;
}

INLINE Mat2 Mat2::Inverse( ) const {
	Mat2 invMat = *this;
	int r = invMat.InverseSelf( );
	assert( r );
	return invMat;
}

INLINE Mat2 Mat2::InverseFast( ) const {
	Mat2 invMat = *this;
	int r = invMat.InverseFastSelf( );
	assert( r );
	return invMat;
}

INLINE int Mat2::GetDimension( ) const {
	return 4;
}

INLINE const float *Mat2::ToFloatPtr( ) const {
	return d_mat[ 0 ].ToFloatPtr( );
}

INLINE float *Mat2::ToFloatPtr( ) {
	return d_mat[ 0 ].ToFloatPtr( );
}

INLINE bool Mat2::InverseSelf( ) {
	float det = d_mat[ 0 ][ 0 ] * d_mat[ 1 ][ 1 ] - d_mat[ 0 ][ 1 ] * d_mat[ 1 ][ 0 ];
	if( fabsf( det ) < MATRIX_INVERSE_EPSILON )
		return false;
	float invDet = 1.0f / det;
	float a = d_mat[ 0 ][ 0 ];
	d_mat[ 0 ][ 0 ] =   d_mat[ 1 ][ 1 ] * invDet;
	d_mat[ 0 ][ 1 ] = - d_mat[ 0 ][ 1 ] * invDet;
	d_mat[ 1 ][ 0 ] = - d_mat[ 1 ][ 0 ] * invDet;
	d_mat[ 1 ][ 1 ] =   a * invDet;
	return true;
}

INLINE bool Mat2::InverseFastSelf( ) {
	float det = d_mat[ 0 ][ 0 ] * d_mat[ 1 ][ 1 ] - d_mat[ 0 ][ 1 ] * d_mat[ 1 ][ 0 ];
	if( fabsf( det ) < MATRIX_INVERSE_EPSILON )
		return false;
	float invDet = 1.0f / det;
	float a = d_mat[ 0 ][ 0 ];
	d_mat[ 0 ][ 0 ] =   d_mat[ 1 ][ 1 ] * invDet;
	d_mat[ 0 ][ 1 ] = - d_mat[ 0 ][ 1 ] * invDet;
	d_mat[ 1 ][ 0 ] = - d_mat[ 1 ][ 0 ] * invDet;
	d_mat[ 1 ][ 1 ] =   a * invDet;
	return true;
}

//===============================================================
//
//	Mat3 - 3x3 matrix
//
//	NOTE:	matrix is column-major
//
//===============================================================

class Mat3 {
public:
					Mat3( );
					explicit Mat3( const Vec3 & x, const Vec3 & y, const Vec3 & z );
					explicit Mat3( const float xx, const float xy, const float xz, const float yx, const float yy, const float yz, const float zx, const float zy, const float zz );
					explicit Mat3( const float src[ 3 ][ 3 ] );

	const Vec3 &	operator [ ]( int index ) const;
	Vec3 &			operator [ ]( int index );
	Mat3			operator -( ) const;
	Mat3			operator *( const float a ) const;
	Vec3			operator *( const Vec3 & vec ) const;
	Mat3			operator *( const Mat3 & a ) const;
	Mat3			operator +( const Mat3 & a ) const;
	Mat3			operator -( const Mat3 & a ) const;
	Mat3 &			operator *=( const float a );
	Mat3 &			operator *=( const Mat3 & a );
	Mat3 &			operator +=( const Mat3 & a );
	Mat3 &			operator -=( const Mat3 & a );

	friend Mat3		operator *( const float a, const Mat3 & mat );
	friend Vec3		operator *( const Vec3 & vec, const Mat3 & mat );
	friend Vec3 &	operator *=( Vec3 & vec, const Mat3 & mat );

	bool			Compare( const Mat3 & a ) const;						// exact compare, no epsilon
	bool			Compare( const Mat3 & a, const float epsilon ) const;	// compare with epsilon
	bool			operator ==( const Mat3 & a ) const;					// exact compare, no epsilon
	bool			operator !=( const Mat3 & a ) const;					// exact compare, no epsilon

	void			Zero( );
	void			Identity( );
	bool			IsIdentity( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetric( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsDiagonal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsRotated( ) const;

	void			ProjectVector( const Vec3 & src, Vec3 & dst ) const;
	void			UnprojectVector( const Vec3 & src, Vec3 & dst ) const;

	bool			FixDegeneracies( );	// fix degenerate axial cases
	bool			FixDenormals( );		// change tiny numbers to zero

	float			Trace( ) const;
	float			Determinant( ) const;
	Mat3			OrthoNormalize( ) const;
	Mat3 &			OrthoNormalizeSelf( );
	Mat3			Transpose( ) const;	// returns transpose
	Mat3 &			TransposeSelf( );
	Mat3			Inverse( ) const;		// returns the inverse ( m * m.Inverse( ) = identity )
	bool			InverseSelf( );		// returns false if determinant is zero
	Mat3			InverseFast( ) const;	// returns the inverse ( m * m.Inverse( ) = identity )
	bool			InverseFastSelf( );	// returns false if determinant is zero
	Mat3			TransposeMultiply( const Mat3 & b ) const;

	Mat3			InertiaTranslate( const float mass, const Vec3 & centerOfMass, const Vec3 & translation ) const;
	Mat3 &			InertiaTranslateSelf( const float mass, const Vec3 & centerOfMass, const Vec3 & translation );
	Mat3			InertiaRotate( const Mat3 & rotation ) const;
	Mat3 &			InertiaRotateSelf( const Mat3 & rotation );

	int				GetDimension( ) const;

	Angles			ToAngles( ) const;
	Quat			ToQuat( ) const;
	CQuat			ToCQuat( ) const;
	Rotation		ToRotation( ) const;
	Mat4			ToMat4( ) const;
	Vec3			ToAngularVelocity( ) const;
	const float *	ToFloatPtr( ) const;
	float *			ToFloatPtr( );
	const char *	ToString( int precision = 2 ) const;

	friend void		TransposeMultiply( const Mat3 & inv, const Mat3 & b, Mat3 & dst );
	friend Mat3		SkewSymmetric( Vec3 const & src );

private:
	Vec3			mat[ 3 ];
};

extern Mat3 mat3_zero;
extern Mat3 mat3_identity;
#define mat3_default	mat3_identity

INLINE Mat3::Mat3( ) {
}

INLINE Mat3::Mat3( const Vec3 & x, const Vec3 & y, const Vec3 & z ) {
	mat[ 0 ].x = x.x; mat[ 0 ].y = x.y; mat[ 0 ].z = x.z;
	mat[ 1 ].x = y.x; mat[ 1 ].y = y.y; mat[ 1 ].z = y.z;
	mat[ 2 ].x = z.x; mat[ 2 ].y = z.y; mat[ 2 ].z = z.z;
}

INLINE Mat3::Mat3( const float xx, const float xy, const float xz, const float yx, const float yy, const float yz, const float zx, const float zy, const float zz ) {
	mat[ 0 ].x = xx; mat[ 0 ].y = xy; mat[ 0 ].z = xz;
	mat[ 1 ].x = yx; mat[ 1 ].y = yy; mat[ 1 ].z = yz;
	mat[ 2 ].x = zx; mat[ 2 ].y = zy; mat[ 2 ].z = zz;
}

INLINE Mat3::Mat3( const float src[ 3 ][ 3 ] ) {
	memcpy( mat, src, 3 * 3 * sizeof( float ) );
}

INLINE const Vec3 & Mat3::operator [ ]( int index ) const {
	assert( ( index >= 0 ) && ( index < 3 ) );
	return mat[ index ];
}

INLINE Vec3 & Mat3::operator [ ]( int index ) {
	assert( ( index >= 0 ) && ( index < 3 ) );
	return mat[ index ];
}

INLINE Mat3 Mat3::operator -( ) const {
	return Mat3(	-mat[ 0 ][ 0 ], -mat[ 0 ][ 1 ], -mat[ 0 ][ 2 ],
					-mat[ 1 ][ 0 ], -mat[ 1 ][ 1 ], -mat[ 1 ][ 2 ],
					-mat[ 2 ][ 0 ], -mat[ 2 ][ 1 ], -mat[ 2 ][ 2 ] );
}

INLINE Vec3 Mat3::operator *( const Vec3 & vec ) const {
	return Vec3(
		mat[ 0 ].x * vec.x + mat[ 1 ].x * vec.y + mat[ 2 ].x * vec.z,
		mat[ 0 ].y * vec.x + mat[ 1 ].y * vec.y + mat[ 2 ].y * vec.z,
		mat[ 0 ].z * vec.x + mat[ 1 ].z * vec.y + mat[ 2 ].z * vec.z );
}

INLINE Mat3 Mat3::operator *( const Mat3 & a ) const {
	int i, j;
	const float *m1Ptr, *m2Ptr;
	float *dstPtr;
	Mat3 dst;

	m1Ptr = reinterpret_cast<const float *>(this);
	m2Ptr = reinterpret_cast<const float *>(&a);
	dstPtr = reinterpret_cast<float *>(&dst);

	for( i = 0; i < 3; i++ ) {
		for( j = 0; j < 3; j++ ) {
			*dstPtr = m1Ptr[ 0 ] * m2Ptr[ 0 * 3 + j ]
					+ m1Ptr[ 1 ] * m2Ptr[ 1 * 3 + j ]
					+ m1Ptr[ 2 ] * m2Ptr[ 2 * 3 + j ];
			dstPtr++;
		}
		m1Ptr += 3;
	}
	return dst;
}

INLINE Mat3 Mat3::operator *( const float a ) const {
	return Mat3(
		mat[ 0 ].x * a, mat[ 0 ].y * a, mat[ 0 ].z * a,
		mat[ 1 ].x * a, mat[ 1 ].y * a, mat[ 1 ].z * a,
		mat[ 2 ].x * a, mat[ 2 ].y * a, mat[ 2 ].z * a );
}

INLINE Mat3 Mat3::operator +( const Mat3 & a ) const {
	return Mat3(
		mat[ 0 ].x + a[ 0 ].x, mat[ 0 ].y + a[ 0 ].y, mat[ 0 ].z + a[ 0 ].z,
		mat[ 1 ].x + a[ 1 ].x, mat[ 1 ].y + a[ 1 ].y, mat[ 1 ].z + a[ 1 ].z,
		mat[ 2 ].x + a[ 2 ].x, mat[ 2 ].y + a[ 2 ].y, mat[ 2 ].z + a[ 2 ].z );
}
    
INLINE Mat3 Mat3::operator -( const Mat3 & a ) const {
	return Mat3(
		mat[ 0 ].x - a[ 0 ].x, mat[ 0 ].y - a[ 0 ].y, mat[ 0 ].z - a[ 0 ].z,
		mat[ 1 ].x - a[ 1 ].x, mat[ 1 ].y - a[ 1 ].y, mat[ 1 ].z - a[ 1 ].z,
		mat[ 2 ].x - a[ 2 ].x, mat[ 2 ].y - a[ 2 ].y, mat[ 2 ].z - a[ 2 ].z );
}

INLINE Mat3 & Mat3::operator *=( const float a ) {
	mat[ 0 ].x *= a; mat[ 0 ].y *= a; mat[ 0 ].z *= a;
	mat[ 1 ].x *= a; mat[ 1 ].y *= a; mat[ 1 ].z *= a; 
	mat[ 2 ].x *= a; mat[ 2 ].y *= a; mat[ 2 ].z *= a;

    return *this;
}

INLINE Mat3 & Mat3::operator *=( const Mat3 & a ) {
	int i, j;
	const float *m2Ptr;
	float *m1Ptr, dst[ 3 ];

	m1Ptr = reinterpret_cast<float *>(this);
	m2Ptr = reinterpret_cast<const float *>(&a);

	for( i = 0; i < 3; i++ ) {
		for( j = 0; j < 3; j++ ) {
			dst[j]  = m1Ptr[ 0 ] * m2Ptr[ 0 * 3 + j ]
					+ m1Ptr[ 1 ] * m2Ptr[ 1 * 3 + j ]
					+ m1Ptr[ 2 ] * m2Ptr[ 2 * 3 + j ];
		}
		m1Ptr[ 0 ] = dst[ 0 ]; m1Ptr[ 1 ] = dst[ 1 ]; m1Ptr[ 2 ] = dst[ 2 ];
		m1Ptr += 3;
	}
	return *this;
}

INLINE Mat3 & Mat3::operator +=( const Mat3 & a ) {
	mat[ 0 ].x += a[ 0 ].x; mat[ 0 ].y += a[ 0 ].y; mat[ 0 ].z += a[ 0 ].z;
	mat[ 1 ].x += a[ 1 ].x; mat[ 1 ].y += a[ 1 ].y; mat[ 1 ].z += a[ 1 ].z;
	mat[ 2 ].x += a[ 2 ].x; mat[ 2 ].y += a[ 2 ].y; mat[ 2 ].z += a[ 2 ].z;

    return *this;
}

INLINE Mat3 & Mat3::operator -=( const Mat3 & a ) {
	mat[ 0 ].x -= a[ 0 ].x; mat[ 0 ].y -= a[ 0 ].y; mat[ 0 ].z -= a[ 0 ].z;
	mat[ 1 ].x -= a[ 1 ].x; mat[ 1 ].y -= a[ 1 ].y; mat[ 1 ].z -= a[ 1 ].z;
	mat[ 2 ].x -= a[ 2 ].x; mat[ 2 ].y -= a[ 2 ].y; mat[ 2 ].z -= a[ 2 ].z;

    return *this;
}

INLINE Vec3 operator *( const Vec3 & vec, const Mat3 & mat ) {
	return mat * vec;
}

INLINE Mat3 operator *( const float a, const Mat3 & mat ) {
	return mat * a;
}

INLINE Vec3 & operator *=( Vec3 & vec, const Mat3 & mat ) {
	float x = mat[ 0 ].x * vec.x + mat[ 1 ].x * vec.y + mat[ 2 ].x * vec.z;
	float y = mat[ 0 ].y * vec.x + mat[ 1 ].y * vec.y + mat[ 2 ].y * vec.z;
	vec.z = mat[ 0 ].z * vec.x + mat[ 1 ].z * vec.y + mat[ 2 ].z * vec.z;
	vec.x = x;
	vec.y = y;
	return vec;
}

INLINE bool Mat3::Compare( const Mat3 & a ) const {
	if( mat[ 0 ].Compare( a[ 0 ] ) &&
		mat[ 1 ].Compare( a[ 1 ] ) &&
		mat[ 2 ].Compare( a[ 2 ] ) ) {
		return true;
	}
	return false;
}

INLINE bool Mat3::Compare( const Mat3 & a, const float epsilon ) const {
	if( mat[ 0 ].Compare( a[ 0 ], epsilon ) &&
		mat[ 1 ].Compare( a[ 1 ], epsilon ) &&
		mat[ 2 ].Compare( a[ 2 ], epsilon ) ) {
		return true;
	}
	return false;
}

INLINE bool Mat3::operator ==( const Mat3 & a ) const {
	return Compare( a );
}

INLINE bool Mat3::operator !=( const Mat3 & a ) const {
	return !Compare( a );
}

INLINE void Mat3::Zero( ) {
	Common::Com_Memset( mat, 0, sizeof( Mat3 ) );
}

INLINE void Mat3::Identity( ) {
	*this = mat3_identity;
}

INLINE bool Mat3::IsIdentity( const float epsilon ) const {
	return Compare( mat3_identity, epsilon );
}

INLINE bool Mat3::IsSymmetric( const float epsilon ) const {
	if( fabs( mat[ 0 ][ 1 ] - mat[ 1 ][ 0 ] ) > epsilon ) {
		return false;
	}
	if( fabs( mat[ 0 ][ 2 ] - mat[ 2 ][ 0 ] ) > epsilon ) {
		return false;
	}
	if( fabs( mat[ 1 ][ 2 ] - mat[ 2 ][ 1 ] ) > epsilon ) {
		return false;
	}
	return true;
}

INLINE bool Mat3::IsDiagonal( const float epsilon ) const {
	if( fabs( mat[ 0 ][ 1 ] ) > epsilon ||
		fabs( mat[ 0 ][ 2 ] ) > epsilon ||
		fabs( mat[ 1 ][ 0 ] ) > epsilon ||
		fabs( mat[ 1 ][ 2 ] ) > epsilon ||
		fabs( mat[ 2 ][ 0 ] ) > epsilon ||
		fabs( mat[ 2 ][ 1 ] ) > epsilon ) {
		return false;
	}
	return true;
}

INLINE bool Mat3::IsRotated( ) const {
	return !Compare( mat3_identity );
}

INLINE void Mat3::ProjectVector( const Vec3 & src, Vec3 & dst ) const {
	dst.x = src * mat[ 0 ];
	dst.y = src * mat[ 1 ];
	dst.z = src * mat[ 2 ];
}

INLINE void Mat3::UnprojectVector( const Vec3 & src, Vec3 & dst ) const {
	dst = mat[ 0 ] * src.x + mat[ 1 ] * src.y + mat[ 2 ] * src.z;
}

INLINE bool Mat3::FixDegeneracies( ) {
	bool r = mat[ 0 ].FixDegenerateNormal( );
	r |= mat[ 1 ].FixDegenerateNormal( );
	r |= mat[ 2 ].FixDegenerateNormal( );
	return r;
}

INLINE bool Mat3::FixDenormals( ) {
	bool r = mat[ 0 ].FixDenormals( );
	r |= mat[ 1 ].FixDenormals( );
	r |= mat[ 2 ].FixDenormals( );
	return r;
}

INLINE float Mat3::Trace( ) const {
	return ( mat[ 0 ][ 0 ] + mat[ 1 ][ 1 ] + mat[ 2 ][ 2 ] );
}

INLINE Mat3 Mat3::OrthoNormalize( ) const {
	Mat3 ortho;

	ortho = *this;
	ortho[ 0 ].Normalize( );
	ortho[ 2 ].Cross( mat[ 0 ], mat[ 1 ] );
	ortho[ 2 ].Normalize( );
	ortho[ 1 ].Cross( mat[ 2 ], mat[ 0 ] );
	ortho[ 1 ].Normalize( );
	return ortho;
}

INLINE Mat3 & Mat3::OrthoNormalizeSelf( ) {
	mat[ 0 ].Normalize( );
	mat[ 2 ].Cross( mat[ 0 ], mat[ 1 ] );
	mat[ 2 ].Normalize( );
	mat[ 1 ].Cross( mat[ 2 ], mat[ 0 ] );
	mat[ 1 ].Normalize( );
	return *this;
}

INLINE Mat3 Mat3::Transpose( ) const {
	return Mat3(	mat[ 0 ][ 0 ], mat[ 1 ][ 0 ], mat[ 2 ][ 0 ],
					mat[ 0 ][ 1 ], mat[ 1 ][ 1 ], mat[ 2 ][ 1 ],
					mat[ 0 ][ 2 ], mat[ 1 ][ 2 ], mat[ 2 ][ 2 ] );
}

INLINE Mat3 & Mat3::TransposeSelf( ) {
	float tmp0, tmp1, tmp2;

	tmp0 = mat[ 0 ][ 1 ];
	mat[ 0 ][ 1 ] = mat[ 1 ][ 0 ];
	mat[ 1 ][ 0 ] = tmp0;
	tmp1 = mat[ 0 ][ 2 ];
	mat[ 0 ][ 2 ] = mat[ 2 ][ 0 ];
	mat[ 2 ][ 0 ] = tmp1;
	tmp2 = mat[ 1 ][ 2 ];
	mat[ 1 ][ 2 ] = mat[ 2 ][ 1 ];
	mat[ 2 ][ 1 ] = tmp2;

	return *this;
}

INLINE Mat3 Mat3::Inverse( ) const {
	Mat3 invMat;

	invMat = *this;
	int r = invMat.InverseSelf( );
	assert( r );
	return invMat;
}

INLINE Mat3 Mat3::InverseFast( ) const {
	Mat3 invMat;

	invMat = *this;
	int r = invMat.InverseFastSelf( );
	assert( r );
	return invMat;
}

INLINE Mat3 Mat3::TransposeMultiply( const Mat3 & b ) const {
	return Mat3(	mat[ 0 ].x * b[ 0 ].x + mat[ 1 ].x * b[ 1 ].x + mat[ 2 ].x * b[ 2 ].x,
					mat[ 0 ].x * b[ 0 ].y + mat[ 1 ].x * b[ 1 ].y + mat[ 2 ].x * b[ 2 ].y,
					mat[ 0 ].x * b[ 0 ].z + mat[ 1 ].x * b[ 1 ].z + mat[ 2 ].x * b[ 2 ].z,
					mat[ 0 ].y * b[ 0 ].x + mat[ 1 ].y * b[ 1 ].x + mat[ 2 ].y * b[ 2 ].x,
					mat[ 0 ].y * b[ 0 ].y + mat[ 1 ].y * b[ 1 ].y + mat[ 2 ].y * b[ 2 ].y,
					mat[ 0 ].y * b[ 0 ].z + mat[ 1 ].y * b[ 1 ].z + mat[ 2 ].y * b[ 2 ].z,
					mat[ 0 ].z * b[ 0 ].x + mat[ 1 ].z * b[ 1 ].x + mat[ 2 ].z * b[ 2 ].x,
					mat[ 0 ].z * b[ 0 ].y + mat[ 1 ].z * b[ 1 ].y + mat[ 2 ].z * b[ 2 ].y,
					mat[ 0 ].z * b[ 0 ].z + mat[ 1 ].z * b[ 1 ].z + mat[ 2 ].z * b[ 2 ].z );
}

INLINE void TransposeMultiply( const Mat3 & transpose, const Mat3 & b, Mat3 & dst ) {
	dst[ 0 ].x = transpose[ 0 ].x * b[ 0 ].x + transpose[ 1 ].x * b[ 1 ].x + transpose[ 2 ].x * b[ 2 ].x;
	dst[ 0 ].y = transpose[ 0 ].x * b[ 0 ].y + transpose[ 1 ].x * b[ 1 ].y + transpose[ 2 ].x * b[ 2 ].y;
	dst[ 0 ].z = transpose[ 0 ].x * b[ 0 ].z + transpose[ 1 ].x * b[ 1 ].z + transpose[ 2 ].x * b[ 2 ].z;
	dst[ 1 ].x = transpose[ 0 ].y * b[ 0 ].x + transpose[ 1 ].y * b[ 1 ].x + transpose[ 2 ].y * b[ 2 ].x;
	dst[ 1 ].y = transpose[ 0 ].y * b[ 0 ].y + transpose[ 1 ].y * b[ 1 ].y + transpose[ 2 ].y * b[ 2 ].y;
	dst[ 1 ].z = transpose[ 0 ].y * b[ 0 ].z + transpose[ 1 ].y * b[ 1 ].z + transpose[ 2 ].y * b[ 2 ].z;
	dst[ 2 ].x = transpose[ 0 ].z * b[ 0 ].x + transpose[ 1 ].z * b[ 1 ].x + transpose[ 2 ].z * b[ 2 ].x;
	dst[ 2 ].y = transpose[ 0 ].z * b[ 0 ].y + transpose[ 1 ].z * b[ 1 ].y + transpose[ 2 ].z * b[ 2 ].y;
	dst[ 2 ].z = transpose[ 0 ].z * b[ 0 ].z + transpose[ 1 ].z * b[ 1 ].z + transpose[ 2 ].z * b[ 2 ].z;
}

INLINE Mat3 SkewSymmetric( Vec3 const & src ) {
	return Mat3( 0.0f, -src.z,  src.y, src.z,   0.0f, -src.x, -src.y,  src.x,   0.0f );
}

INLINE int Mat3::GetDimension( ) const {
	return 9;
}

INLINE const float *Mat3::ToFloatPtr( ) const {
	return mat[ 0 ].ToFloatPtr( );
}

INLINE float *Mat3::ToFloatPtr( ) {
	return mat[ 0 ].ToFloatPtr( );
}

class Plane;

//===============================================================
//
//	Mat4 - 4x4 matrix
//
//===============================================================

class Mat4 {
public:
					Mat4( );
					explicit Mat4( const Vec4 & x, const Vec4 & y, const Vec4 & z, const Vec4 & w );
					explicit Mat4(const float xx, const float xy, const float xz, const float xw,
									const float yx, const float yy, const float yz, const float yw,
									const float zx, const float zy, const float zz, const float zw,
									const float wx, const float wy, const float wz, const float ww );
					explicit Mat4( const Mat3 & rotation, const Vec3 & translation );
					explicit Mat4( const float src[ 4 ][ 4 ] );

	const Vec4 &	operator [ ]( int index ) const;
	Vec4 &			operator [ ]( int index );
	Mat4			operator *( const float a ) const;
	Vec4			operator *( const Vec4 & vec ) const;
	Vec3			operator *( const Vec3 & vec ) const;
	Mat4			operator *( const Mat4 & a ) const;
	Mat4			operator +( const Mat4 & a ) const;
	Mat4			operator -( const Mat4 & a ) const;
	Mat4 &			operator *=( const float a );
	Mat4 &			operator *=( const Mat4 & a );
	Mat4 &			operator +=( const Mat4 & a );
	Mat4 &			operator -=( const Mat4 & a );

	friend Mat4		operator *( const float a, const Mat4 & mat );
	friend Vec4		operator *( const Vec4 & vec, const Mat4 & mat );
	friend Vec3		operator *( const Vec3 & vec, const Mat4 & mat );
	friend Vec4 &	operator *=( Vec4 & vec, const Mat4 & mat );
	friend Vec3 &	operator *=( Vec3 & vec, const Mat4 & mat );

	bool			Compare( const Mat4 & a ) const;						// exact compare, no epsilon
	bool			Compare( const Mat4 & a, const float epsilon ) const;	// compare with epsilon
	bool			operator ==( const Mat4 & a ) const;					// exact compare, no epsilon
	bool			operator !=( const Mat4 & a ) const;					// exact compare, no epsilon

	void			Zero( );
	void			Identity( );
	Mat4 &			Set( const Mat4 & a );
	bool			IsIdentity( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetric( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsDiagonal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsRotated( ) const;

	void			ProjectVector( const Vec4 & src, Vec4 & dst ) const;
	void			InvProjectVector( const Vec4 & src, Vec4 & dst ) const;
	void			UnprojectVector( const Vec4 & src, Vec4 & dst ) const;

	void			ProjectVectorArray( const Vec4 * src, Vec4 * dst, int num ) const;
	void			InvProjectVectorArray( const Vec4 * src, Vec4 * dst, int num ) const;

	float			Trace( ) const;
	float			Determinant( ) const;
	Mat4			Transpose( ) const;	// returns transpose
	Mat4 &			TransposeSelf( );
	Mat4			Inverse( ) const;		// returns the inverse ( m * m.Inverse( ) = identity )
	bool			InverseSelf( );		// returns false if determinant is zero
	Mat4			InverseFast( ) const;	// returns the inverse ( m * m.Inverse( ) = identity )
	bool			InverseFastSelf( );	// returns false if determinant is zero
	Mat4			TransposeMultiply( const Mat4 & b ) const;

	int				GetDimension( ) const;

	const float *	ToFloatPtr( ) const;
	float *			ToFloatPtr( );
	const char *	ToString( int precision = 2 ) const;

	void			ToPlanes( Plane planes[ 6 ] ) const;
	void			ToVectors( Vec4 vecs[ 6 ] ) const;

	Mat4 &			TranslateSelf( const Vec3 & vec );
	Mat4			Translate( const Vec3 & vec ) const;

protected:
	Vec4			mat[ 4 ];
};

extern const Mat4 mat4_zero;
extern const Mat4 mat4_identity;
extern const Mat4 mat4_bias;
#define mat4_default	mat4_identity

INLINE Mat4::Mat4( ) {
}

INLINE Mat4::Mat4( const Vec4 & x, const Vec4 & y, const Vec4 & z, const Vec4 & w ) {
	mat[ 0 ] = x;
	mat[ 1 ] = y;
	mat[ 2 ] = z;
	mat[ 3 ] = w;
}

INLINE Mat4::Mat4( const float xx, const float xy, const float xz, const float xw,
							const float yx, const float yy, const float yz, const float yw,
							const float zx, const float zy, const float zz, const float zw,
							const float wx, const float wy, const float wz, const float ww ) {
	mat[ 0 ][ 0 ] = xx; mat[ 0 ][ 1 ] = xy; mat[ 0 ][ 2 ] = xz; mat[ 0 ][ 3 ] = xw;
	mat[ 1 ][ 0 ] = yx; mat[ 1 ][ 1 ] = yy; mat[ 1 ][ 2 ] = yz; mat[ 1 ][ 3 ] = yw;
	mat[ 2 ][ 0 ] = zx; mat[ 2 ][ 1 ] = zy; mat[ 2 ][ 2 ] = zz; mat[ 2 ][ 3 ] = zw;
	mat[ 3 ][ 0 ] = wx; mat[ 3 ][ 1 ] = wy; mat[ 3 ][ 2 ] = wz; mat[ 3 ][ 3 ] = ww;
}

INLINE Mat4::Mat4( const Mat3 & rotation, const Vec3 & translation ) {
	// NOTE: Mat3 is transposed because it is column-major
	mat[ 0 ][ 0 ] = rotation[ 0 ][ 0 ];
	mat[ 0 ][ 1 ] = rotation[ 1 ][ 0 ];
	mat[ 0 ][ 2 ] = rotation[ 2 ][ 0 ];
	mat[ 0 ][ 3 ] = translation[ 0 ];
	mat[ 1 ][ 0 ] = rotation[ 0 ][ 1 ];
	mat[ 1 ][ 1 ] = rotation[ 1 ][ 1 ];
	mat[ 1 ][ 2 ] = rotation[ 2 ][ 1 ];
	mat[ 1 ][ 3 ] = translation[ 1 ];
	mat[ 2 ][ 0 ] = rotation[ 0 ][ 2 ];
	mat[ 2 ][ 1 ] = rotation[ 1 ][ 2 ];
	mat[ 2 ][ 2 ] = rotation[ 2 ][ 2 ];
	mat[ 2 ][ 3 ] = translation[ 2 ];
	mat[ 3 ][ 0 ] = 0.0f;
	mat[ 3 ][ 1 ] = 0.0f;
	mat[ 3 ][ 2 ] = 0.0f;
	mat[ 3 ][ 3 ] = 1.0f;
}

INLINE Mat4::Mat4( const float src[ 4 ][ 4 ] ) {
	memcpy( mat, src, 4 * 4 * sizeof( float ) );
}

INLINE const Vec4 & Mat4::operator [ ]( int index ) const {
	assert( ( index >= 0 ) && ( index < 4 ) );
	return mat[ index ];
}

INLINE Vec4 & Mat4::operator [ ]( int index ) {
	assert( ( index >= 0 ) && ( index < 4 ) );
	return mat[ index ];
}

INLINE Mat4 Mat4::operator *( const float a ) const {
	return Mat4(
		mat[ 0 ].x * a, mat[ 0 ].y * a, mat[ 0 ].z * a, mat[ 0 ].w * a,
		mat[ 1 ].x * a, mat[ 1 ].y * a, mat[ 1 ].z * a, mat[ 1 ].w * a,
		mat[ 2 ].x * a, mat[ 2 ].y * a, mat[ 2 ].z * a, mat[ 2 ].w * a,
		mat[ 3 ].x * a, mat[ 3 ].y * a, mat[ 3 ].z * a, mat[ 3 ].w * a );
}

INLINE Vec4 Mat4::operator *( const Vec4 & vec ) const {
	/*return Vec4(
		mat[ 0 ].x * vec.x + mat[ 0 ].x * vec.y + mat[ 0 ].x * vec.z + mat[ 0 ].x * vec.w,
		mat[ 1 ].y * vec.x + mat[ 1 ].y * vec.y + mat[ 1 ].y * vec.z + mat[ 1 ].y * vec.w,
		mat[ 2 ].z * vec.x + mat[ 2 ].z * vec.y + mat[ 2 ].z * vec.z + mat[ 2 ].z * vec.w,
		mat[ 3 ].w * vec.x + mat[ 3 ].w * vec.y + mat[ 3 ].w * vec.z + mat[ 3 ].w * vec.w );*/

	return Vec4(
		mat[ 0 ].x * vec.x + mat[ 0 ].y * vec.y + mat[ 0 ].z * vec.z + mat[ 0 ].w * vec.w,
		mat[ 1 ].x * vec.x + mat[ 1 ].y * vec.y + mat[ 1 ].z * vec.z + mat[ 1 ].w * vec.w,
		mat[ 2 ].x * vec.x + mat[ 2 ].y * vec.y + mat[ 2 ].z * vec.z + mat[ 2 ].w * vec.w,
		mat[ 3 ].x * vec.x + mat[ 3 ].y * vec.y + mat[ 3 ].z * vec.z + mat[ 3 ].w * vec.w );
}

INLINE Vec3 Mat4::operator *( const Vec3 & vec ) const {
	/*float s = mat[ 0 ].w * vec.x + mat[ 1 ].w * vec.y + mat[ 2 ].w * vec.z + mat[ 3 ].w;
	if( s == 0.0f ) {
		return Vec3( 0.0f, 0.0f, 0.0f );
	}
	if( s == 1.0f ) {
		return Vec3(
			mat[ 0 ].x * vec.x + mat[ 0 ].x * vec.y + mat[ 0 ].x * vec.z + mat[ 0 ].x,
			mat[ 1 ].y * vec.x + mat[ 1 ].y * vec.y + mat[ 1 ].y * vec.z + mat[ 1 ].y,
			mat[ 2 ].z * vec.x + mat[ 2 ].z * vec.y + mat[ 2 ].z * vec.z + mat[ 2 ].z );
	}
	else {
		float invS = 1.0f / s;
		return Vec3(
			(mat[ 0 ].x * vec.x + mat[ 0 ].x * vec.y + mat[ 0 ].x * vec.z + mat[ 0 ].x) * invS,
			(mat[ 1 ].y * vec.x + mat[ 1 ].y * vec.y + mat[ 1 ].y * vec.z + mat[ 1 ].y) * invS,
			(mat[ 2 ].z * vec.x + mat[ 2 ].z * vec.y + mat[ 2 ].z * vec.z + mat[ 2 ].z) * invS );
	}*/

	float s = mat[ 3 ].x * vec.x + mat[ 3 ].y * vec.y + mat[ 3 ].z * vec.z + mat[ 3 ].w;
	if( s == 0.0f ) {
		return Vec3( 0.0f, 0.0f, 0.0f );
	}
	if( s == 1.0f ) {
		return Vec3(
			mat[ 0 ].x * vec.x + mat[ 0 ].y * vec.y + mat[ 0 ].z * vec.z + mat[ 0 ].w,
			mat[ 1 ].x * vec.x + mat[ 1 ].y * vec.y + mat[ 1 ].z * vec.z + mat[ 1 ].w,
			mat[ 2 ].x * vec.x + mat[ 2 ].y * vec.y + mat[ 2 ].z * vec.z + mat[ 2 ].w );
	}
	else {
		float invS = 1.0f / s;
		return Vec3(
			(mat[ 0 ].x * vec.x + mat[ 0 ].y * vec.y + mat[ 0 ].z * vec.z + mat[ 0 ].w) * invS,
			(mat[ 1 ].x * vec.x + mat[ 1 ].y * vec.y + mat[ 1 ].z * vec.z + mat[ 1 ].w) * invS,
			(mat[ 2 ].x * vec.x + mat[ 2 ].y * vec.y + mat[ 2 ].z * vec.z + mat[ 2 ].w) * invS );
	}
}

INLINE Mat4 Mat4::operator *( const Mat4 & a ) const {
	int i, j;
	const float *m1Ptr, *m2Ptr;
	float *dstPtr;
	Mat4 dst;

	m1Ptr = reinterpret_cast<const float *>(this);
	m2Ptr = reinterpret_cast<const float *>(&a);
	dstPtr = reinterpret_cast<float *>(&dst);

	for( i = 0; i < 4; i++ ) {
		for( j = 0; j < 4; j++ ) {
			*dstPtr = m1Ptr[ 0 ] * m2Ptr[ 0 * 4 + j ]
					+ m1Ptr[ 1 ] * m2Ptr[ 1 * 4 + j ]
					+ m1Ptr[ 2 ] * m2Ptr[ 2 * 4 + j ]
					+ m1Ptr[ 3 ] * m2Ptr[ 3 * 4 + j ];
			dstPtr++;
		}
		m1Ptr += 4;
	}
	return dst;
}

INLINE Mat4 Mat4::operator +( const Mat4 & a ) const {
	return Mat4( 
		mat[ 0 ].x + a[ 0 ].x, mat[ 0 ].y + a[ 0 ].y, mat[ 0 ].z + a[ 0 ].z, mat[ 0 ].w + a[ 0 ].w,
		mat[ 1 ].x + a[ 1 ].x, mat[ 1 ].y + a[ 1 ].y, mat[ 1 ].z + a[ 1 ].z, mat[ 1 ].w + a[ 1 ].w,
		mat[ 2 ].x + a[ 2 ].x, mat[ 2 ].y + a[ 2 ].y, mat[ 2 ].z + a[ 2 ].z, mat[ 2 ].w + a[ 2 ].w,
		mat[ 3 ].x + a[ 3 ].x, mat[ 3 ].y + a[ 3 ].y, mat[ 3 ].z + a[ 3 ].z, mat[ 3 ].w + a[ 3 ].w );
}
    
INLINE Mat4 Mat4::operator -( const Mat4 & a ) const {
	return Mat4( 
		mat[ 0 ].x - a[ 0 ].x, mat[ 0 ].y - a[ 0 ].y, mat[ 0 ].z - a[ 0 ].z, mat[ 0 ].w - a[ 0 ].w,
		mat[ 1 ].x - a[ 1 ].x, mat[ 1 ].y - a[ 1 ].y, mat[ 1 ].z - a[ 1 ].z, mat[ 1 ].w - a[ 1 ].w,
		mat[ 2 ].x - a[ 2 ].x, mat[ 2 ].y - a[ 2 ].y, mat[ 2 ].z - a[ 2 ].z, mat[ 2 ].w - a[ 2 ].w,
		mat[ 3 ].x - a[ 3 ].x, mat[ 3 ].y - a[ 3 ].y, mat[ 3 ].z - a[ 3 ].z, mat[ 3 ].w - a[ 3 ].w );
}

INLINE Mat4 & Mat4::operator *=( const float a ) {
	mat[ 0 ].x *= a; mat[ 0 ].y *= a; mat[ 0 ].z *= a; mat[ 0 ].w *= a;
	mat[ 1 ].x *= a; mat[ 1 ].y *= a; mat[ 1 ].z *= a; mat[ 1 ].w *= a;
	mat[ 2 ].x *= a; mat[ 2 ].y *= a; mat[ 2 ].z *= a; mat[ 2 ].w *= a;
	mat[ 3 ].x *= a; mat[ 3 ].y *= a; mat[ 3 ].z *= a; mat[ 3 ].w *= a;
    return *this;
}

INLINE Mat4 & Mat4::operator *=( const Mat4 & a ) {
	*this = (*this) * a;
	return *this;
}

INLINE Mat4 & Mat4::operator +=( const Mat4 & a ) {
	mat[ 0 ].x += a[ 0 ].x; mat[ 0 ].y += a[ 0 ].y; mat[ 0 ].z += a[ 0 ].z; mat[ 0 ].w += a[ 0 ].w;
	mat[ 1 ].x += a[ 1 ].x; mat[ 1 ].y += a[ 1 ].y; mat[ 1 ].z += a[ 1 ].z; mat[ 1 ].w += a[ 1 ].w;
	mat[ 2 ].x += a[ 2 ].x; mat[ 2 ].y += a[ 2 ].y; mat[ 2 ].z += a[ 2 ].z; mat[ 2 ].w += a[ 2 ].w;
	mat[ 3 ].x += a[ 3 ].x; mat[ 3 ].y += a[ 3 ].y; mat[ 3 ].z += a[ 3 ].z; mat[ 3 ].w += a[ 3 ].w;
    return *this;
}

INLINE Mat4 & Mat4::operator -=( const Mat4 & a ) {
	mat[ 0 ].x -= a[ 0 ].x; mat[ 0 ].y -= a[ 0 ].y; mat[ 0 ].z -= a[ 0 ].z; mat[ 0 ].w -= a[ 0 ].w;
	mat[ 1 ].x -= a[ 1 ].x; mat[ 1 ].y -= a[ 1 ].y; mat[ 1 ].z -= a[ 1 ].z; mat[ 1 ].w -= a[ 1 ].w;
	mat[ 2 ].x -= a[ 2 ].x; mat[ 2 ].y -= a[ 2 ].y; mat[ 2 ].z -= a[ 2 ].z; mat[ 2 ].w -= a[ 2 ].w;
	mat[ 3 ].x -= a[ 3 ].x; mat[ 3 ].y -= a[ 3 ].y; mat[ 3 ].z -= a[ 3 ].z; mat[ 3 ].w -= a[ 3 ].w;
    return *this;
}

INLINE Mat4 operator *( const float a, const Mat4 & mat ) {
	return mat * a;
}

INLINE Vec4 operator *( const Vec4 & vec, const Mat4 & mat ) {
	return mat * vec;
}

INLINE Vec3 operator *( const Vec3 & vec, const Mat4 & mat ) {
	return mat * vec;
}

INLINE Vec4 & operator *=( Vec4 & vec, const Mat4 & mat ) {
	vec = mat * vec;
	return vec;
}

INLINE Vec3 & operator *=( Vec3 & vec, const Mat4 & mat ) {
	vec = mat * vec;
	return vec;
}

INLINE bool Mat4::Compare( const Mat4 & a ) const {
	dword i;
	const float *ptr1, *ptr2;

	ptr1 = reinterpret_cast<const float *>(mat);
	ptr2 = reinterpret_cast<const float *>(a.mat);
	for( i = 0; i < 4*4; i++ ) {
		if( ptr1[i] != ptr2[i] ) {
			return false;
		}
	}
	return true;
}

INLINE bool Mat4::Compare( const Mat4 & a, const float epsilon ) const {
	dword i;
	const float *ptr1, *ptr2;

	ptr1 = reinterpret_cast<const float *>(mat);
	ptr2 = reinterpret_cast<const float *>(a.mat);
	for( i = 0; i < 4*4; i++ ) {
		if( fabs( ptr1[i] - ptr2[i] ) > epsilon ) {
			return false;
		}
	}
	return true;
}

INLINE bool Mat4::operator ==( const Mat4 & a ) const {
	return Compare( a );
}

INLINE bool Mat4::operator !=( const Mat4 & a ) const {
	return !Compare( a );
}

INLINE void Mat4::Zero( ) {
	Common::Com_Memset( mat, 0, sizeof( Mat4 ) );
}

INLINE void Mat4::Identity( ) {
	*this = mat4_identity;
}

INLINE Mat4 & Mat4::Set( const Mat4 & a ) {

	*this = a;
	return *this;
}

INLINE bool Mat4::IsIdentity( const float epsilon ) const {
	return Compare( mat4_identity, epsilon );
}

INLINE bool Mat4::IsSymmetric( const float epsilon ) const {
	for( int i = 1; i < 4; i++ ) {
		for( int j = 0; j < i; j++ ) {
			if( fabs( mat[i][j] - mat[j][i] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

INLINE bool Mat4::IsDiagonal( const float epsilon ) const {
	for( int i = 0; i < 4; i++ ) {
		for( int j = 0; j < 4; j++ ) {
			if( i != j && fabs( mat[i][j] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

INLINE bool Mat4::IsRotated( ) const {
	if( !mat[ 0 ][ 1 ] && !mat[ 0 ][ 2 ] &&
		!mat[ 1 ][ 0 ] && !mat[ 1 ][ 2 ] &&
		!mat[ 2 ][ 0 ] && !mat[ 2 ][ 1 ] ) {
		return false;
	}
	return true;
}

INLINE void Mat4::ProjectVector( const Vec4 & src, Vec4 & dst ) const {
	dst.x = src * mat[ 0 ];
	dst.y = src * mat[ 1 ];
	dst.z = src * mat[ 2 ];
	dst.w = src * mat[ 3 ];
}

INLINE void Mat4::InvProjectVector( const Vec4 & src, Vec4 & dst ) const {
	dst = Inverse( ) * src;
}

INLINE void Mat4::UnprojectVector( const Vec4 & src, Vec4 & dst ) const {
	dst = mat[ 0 ] * src.x + mat[ 1 ] * src.y + mat[ 2 ] * src.z + mat[ 3 ] * src.w;
}

INLINE void Mat4::ProjectVectorArray( const Vec4 * src, Vec4 * dst, int num ) const {
	for( int i = 0; i < num; i++ )
		dst[ i ].Set( src[ i ] * mat[ 0 ], src[ i ] * mat[ 1 ], src[ i ] * mat[ 2 ], src[ i ] * mat[ 3 ] );
}

INLINE void Mat4::InvProjectVectorArray( const Vec4 * src, Vec4 * dst, int num ) const {
	Mat4 invMat = Inverse( );
	for( int i = 0; i < num; i++ )
		dst[ i ] = invMat * src[ i ];
}

INLINE float Mat4::Trace( ) const {
	return ( mat[ 0 ][ 0 ] + mat[ 1 ][ 1 ] + mat[ 2 ][ 2 ] + mat[ 3 ][ 3 ] );
}

INLINE Mat4 Mat4::Inverse( ) const {
	Mat4 invMat;

	invMat = *this;
	int r = invMat.InverseSelf( );
	assert( r );
	return invMat;
}

INLINE Mat4 Mat4::InverseFast( ) const {
	Mat4 invMat;

	invMat = *this;
	int r = invMat.InverseFastSelf( );
	assert( r );
	return invMat;
}

INLINE Mat4 Mat3::ToMat4( ) const {
	// NOTE: Mat3 is transposed because it is column-major
	return Mat4(	mat[ 0 ][ 0 ],	mat[ 1 ][ 0 ],	mat[ 2 ][ 0 ],	0.0f,
					mat[ 0 ][ 1 ],	mat[ 1 ][ 1 ],	mat[ 2 ][ 1 ],	0.0f,
					mat[ 0 ][ 2 ],	mat[ 1 ][ 2 ],	mat[ 2 ][ 2 ],	0.0f,
					0.0f,		0.0f,		0.0f,		1.0f );
}

INLINE int Mat4::GetDimension( ) const {
	return 16;
}

INLINE const float *Mat4::ToFloatPtr( ) const {
	return mat[ 0 ].ToFloatPtr( );
}

INLINE float *Mat4::ToFloatPtr( ) {
	return mat[ 0 ].ToFloatPtr( );
}

INLINE void Mat4::ToVectors( Vec4 vecs[ 6 ] ) const {
	for( int i = 0; i < 6; i++ )
		vecs[ i ] = ( i & 1 ) ? mat[ 3 ] - mat[ i >> 1 ] : mat[ i >> 1 ] + mat[ 3 ];
}

INLINE Mat4 & Mat4::TranslateSelf( const Vec3 & vec ) {
	Mat4 Trans = mat4_identity;
	Trans[ 0 ][ 3 ] = vec[ 0 ];
	Trans[ 1 ][ 3 ] = vec[ 1 ];
	Trans[ 2 ][ 3 ] = vec[ 2 ];	
	*this *= Trans;
	return *this;
}

INLINE Mat4 Mat4::Translate( const Vec3 & vec ) const {
	Mat4 Trans( *this );
	Trans.TranslateSelf( vec );
	return Trans;
}

//===============================================================
//
//	Mat5 - 5x5 matrix
//
//===============================================================

class Mat5 {
public:
					Mat5( );
					explicit Mat5( const Vec5 & v0, const Vec5 & v1, const Vec5 & v2, const Vec5 & v3, const Vec5 & v4 );
					explicit Mat5( const float src[ 5 ][ 5 ] );

	const Vec5 &	operator [ ]( int index ) const;
	Vec5 &			operator [ ]( int index );
	Mat5			operator *( const float a ) const;
	Vec5			operator *( const Vec5 & vec ) const;
	Mat5			operator *( const Mat5 & a ) const;
	Mat5			operator +( const Mat5 & a ) const;
	Mat5			operator -( const Mat5 & a ) const;
	Mat5 &			operator *=( const float a );
	Mat5 &			operator *=( const Mat5 & a );
	Mat5 &			operator +=( const Mat5 & a );
	Mat5 &			operator -=( const Mat5 & a );

	friend Mat5		operator *( const float a, const Mat5 & mat );
	friend Vec5		operator *( const Vec5 & vec, const Mat5 & mat );
	friend Vec5 &	operator *=( Vec5 & vec, const Mat5 & mat );

	bool			Compare( const Mat5 & a ) const;						// exact compare, no epsilon
	bool			Compare( const Mat5 & a, const float epsilon ) const;	// compare with epsilon
	bool			operator ==( const Mat5 & a ) const;					// exact compare, no epsilon
	bool			operator !=( const Mat5 & a ) const;					// exact compare, no epsilon

	void			Zero( );
	void			Identity( );
	bool			IsIdentity( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetric( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsDiagonal( const float epsilon = MATRIX_EPSILON ) const;

	float			Trace( ) const;
	float			Determinant( ) const;
	Mat5			Transpose( ) const;	// returns transpose
	Mat5 &			TransposeSelf( );
	Mat5			Inverse( ) const;		// returns the inverse ( m * m.Inverse( ) = identity )
	bool			InverseSelf( );		// returns false if determinant is zero
	Mat5			InverseFast( ) const;	// returns the inverse ( m * m.Inverse( ) = identity )
	bool			InverseFastSelf( );	// returns false if determinant is zero

	int				GetDimension( ) const;

	const float *	ToFloatPtr( ) const;
	float *			ToFloatPtr( );
	const char *	ToString( int precision = 2 ) const;

private:
	Vec5			mat[ 5 ];
};

extern Mat5 mat5_zero;
extern Mat5 mat5_identity;
#define mat5_default	mat5_identity

INLINE Mat5::Mat5( ) {
}

INLINE Mat5::Mat5( const float src[ 5 ][ 5 ] ) {
	memcpy( mat, src, 5 * 5 * sizeof( float ) );
}

INLINE Mat5::Mat5( const Vec5 & v0, const Vec5 & v1, const Vec5 & v2, const Vec5 & v3, const Vec5 & v4 ) {
	mat[ 0 ] = v0;
	mat[ 1 ] = v1;
	mat[ 2 ] = v2;
	mat[ 3 ] = v3;
	mat[ 4 ] = v4;
}

INLINE const Vec5 & Mat5::operator [ ]( int index ) const {
	assert( ( index >= 0 ) && ( index < 5 ) );
	return mat[ index ];
}

INLINE Vec5 & Mat5::operator [ ]( int index ) {
	assert( ( index >= 0 ) && ( index < 5 ) );
	return mat[ index ];
}

INLINE Mat5 Mat5::operator *( const Mat5 & a ) const {
	int i, j;
	const float *m1Ptr, *m2Ptr;
	float *dstPtr;
	Mat5 dst;

	m1Ptr = reinterpret_cast<const float *>(this);
	m2Ptr = reinterpret_cast<const float *>(&a);
	dstPtr = reinterpret_cast<float *>(&dst);

	for( i = 0; i < 5; i++ ) {
		for( j = 0; j < 5; j++ ) {
			*dstPtr = m1Ptr[ 0 ] * m2Ptr[ 0 * 5 + j ]
					+ m1Ptr[ 1 ] * m2Ptr[ 1 * 5 + j ]
					+ m1Ptr[ 2 ] * m2Ptr[ 2 * 5 + j ]
					+ m1Ptr[ 3 ] * m2Ptr[ 3 * 5 + j ]
					+ m1Ptr[ 4 ] * m2Ptr[ 4 * 5 + j ];
			dstPtr++;
		}
		m1Ptr += 5;
	}
	return dst;
}

INLINE Mat5 Mat5::operator *( const float a ) const {
	return Mat5(
		Vec5( mat[ 0 ][ 0 ] * a, mat[ 0 ][ 1 ] * a, mat[ 0 ][ 2 ] * a, mat[ 0 ][ 3 ] * a, mat[ 0 ][ 4 ] * a ),
		Vec5( mat[ 1 ][ 0 ] * a, mat[ 1 ][ 1 ] * a, mat[ 1 ][ 2 ] * a, mat[ 1 ][ 3 ] * a, mat[ 1 ][ 4 ] * a ),
		Vec5( mat[ 2 ][ 0 ] * a, mat[ 2 ][ 1 ] * a, mat[ 2 ][ 2 ] * a, mat[ 2 ][ 3 ] * a, mat[ 2 ][ 4 ] * a ),
		Vec5( mat[ 3 ][ 0 ] * a, mat[ 3 ][ 1 ] * a, mat[ 3 ][ 2 ] * a, mat[ 3 ][ 3 ] * a, mat[ 3 ][ 4 ] * a ),
		Vec5( mat[ 4 ][ 0 ] * a, mat[ 4 ][ 1 ] * a, mat[ 4 ][ 2 ] * a, mat[ 4 ][ 3 ] * a, mat[ 4 ][ 4 ] * a ) );
}

INLINE Vec5 Mat5::operator *( const Vec5 & vec ) const {
	return Vec5(
		mat[ 0 ][ 0 ] * vec[ 0 ] + mat[ 0 ][ 1 ] * vec[ 1 ] + mat[ 0 ][ 2 ] * vec[ 2 ] + mat[ 0 ][ 3 ] * vec[ 3 ] + mat[ 0 ][ 4 ] * vec[ 4 ],
		mat[ 1 ][ 0 ] * vec[ 0 ] + mat[ 1 ][ 1 ] * vec[ 1 ] + mat[ 1 ][ 2 ] * vec[ 2 ] + mat[ 1 ][ 3 ] * vec[ 3 ] + mat[ 1 ][ 4 ] * vec[ 4 ],
		mat[ 2 ][ 0 ] * vec[ 0 ] + mat[ 2 ][ 1 ] * vec[ 1 ] + mat[ 2 ][ 2 ] * vec[ 2 ] + mat[ 2 ][ 3 ] * vec[ 3 ] + mat[ 2 ][ 4 ] * vec[ 4 ],
		mat[ 3 ][ 0 ] * vec[ 0 ] + mat[ 3 ][ 1 ] * vec[ 1 ] + mat[ 3 ][ 2 ] * vec[ 2 ] + mat[ 3 ][ 3 ] * vec[ 3 ] + mat[ 3 ][ 4 ] * vec[ 4 ],
		mat[ 4 ][ 0 ] * vec[ 0 ] + mat[ 4 ][ 1 ] * vec[ 1 ] + mat[ 4 ][ 2 ] * vec[ 2 ] + mat[ 4 ][ 3 ] * vec[ 3 ] + mat[ 4 ][ 4 ] * vec[ 4 ] );
}

INLINE Mat5 Mat5::operator +( const Mat5 & a ) const {
	return Mat5(
		Vec5( mat[ 0 ][ 0 ] + a[ 0 ][ 0 ], mat[ 0 ][ 1 ] + a[ 0 ][ 1 ], mat[ 0 ][ 2 ] + a[ 0 ][ 2 ], mat[ 0 ][ 3 ] + a[ 0 ][ 3 ], mat[ 0 ][ 4 ] + a[ 0 ][ 4 ] ),
		Vec5( mat[ 1 ][ 0 ] + a[ 1 ][ 0 ], mat[ 1 ][ 1 ] + a[ 1 ][ 1 ], mat[ 1 ][ 2 ] + a[ 1 ][ 2 ], mat[ 1 ][ 3 ] + a[ 1 ][ 3 ], mat[ 1 ][ 4 ] + a[ 1 ][ 4 ] ),
		Vec5( mat[ 2 ][ 0 ] + a[ 2 ][ 0 ], mat[ 2 ][ 1 ] + a[ 2 ][ 1 ], mat[ 2 ][ 2 ] + a[ 2 ][ 2 ], mat[ 2 ][ 3 ] + a[ 2 ][ 3 ], mat[ 2 ][ 4 ] + a[ 2 ][ 4 ] ),
		Vec5( mat[ 3 ][ 0 ] + a[ 3 ][ 0 ], mat[ 3 ][ 1 ] + a[ 3 ][ 1 ], mat[ 3 ][ 2 ] + a[ 3 ][ 2 ], mat[ 3 ][ 3 ] + a[ 3 ][ 3 ], mat[ 3 ][ 4 ] + a[ 3 ][ 4 ] ),
		Vec5( mat[ 4 ][ 0 ] + a[ 4 ][ 0 ], mat[ 4 ][ 1 ] + a[ 4 ][ 1 ], mat[ 4 ][ 2 ] + a[ 4 ][ 2 ], mat[ 4 ][ 3 ] + a[ 4 ][ 3 ], mat[ 4 ][ 4 ] + a[ 4 ][ 4 ] ) );
}

INLINE Mat5 Mat5::operator -( const Mat5 & a ) const {
	return Mat5(
		Vec5( mat[ 0 ][ 0 ] - a[ 0 ][ 0 ], mat[ 0 ][ 1 ] - a[ 0 ][ 1 ], mat[ 0 ][ 2 ] - a[ 0 ][ 2 ], mat[ 0 ][ 3 ] - a[ 0 ][ 3 ], mat[ 0 ][ 4 ] - a[ 0 ][ 4 ] ),
		Vec5( mat[ 1 ][ 0 ] - a[ 1 ][ 0 ], mat[ 1 ][ 1 ] - a[ 1 ][ 1 ], mat[ 1 ][ 2 ] - a[ 1 ][ 2 ], mat[ 1 ][ 3 ] - a[ 1 ][ 3 ], mat[ 1 ][ 4 ] - a[ 1 ][ 4 ] ),
		Vec5( mat[ 2 ][ 0 ] - a[ 2 ][ 0 ], mat[ 2 ][ 1 ] - a[ 2 ][ 1 ], mat[ 2 ][ 2 ] - a[ 2 ][ 2 ], mat[ 2 ][ 3 ] - a[ 2 ][ 3 ], mat[ 2 ][ 4 ] - a[ 2 ][ 4 ] ),
		Vec5( mat[ 3 ][ 0 ] - a[ 3 ][ 0 ], mat[ 3 ][ 1 ] - a[ 3 ][ 1 ], mat[ 3 ][ 2 ] - a[ 3 ][ 2 ], mat[ 3 ][ 3 ] - a[ 3 ][ 3 ], mat[ 3 ][ 4 ] - a[ 3 ][ 4 ] ),
		Vec5( mat[ 4 ][ 0 ] - a[ 4 ][ 0 ], mat[ 4 ][ 1 ] - a[ 4 ][ 1 ], mat[ 4 ][ 2 ] - a[ 4 ][ 2 ], mat[ 4 ][ 3 ] - a[ 4 ][ 3 ], mat[ 4 ][ 4 ] - a[ 4 ][ 4 ] ) );
}

INLINE Mat5 & Mat5::operator *=( const float a ) {
	mat[ 0 ][ 0 ] *= a; mat[ 0 ][ 1 ] *= a; mat[ 0 ][ 2 ] *= a; mat[ 0 ][ 3 ] *= a; mat[ 0 ][ 4 ] *= a;
	mat[ 1 ][ 0 ] *= a; mat[ 1 ][ 1 ] *= a; mat[ 1 ][ 2 ] *= a; mat[ 1 ][ 3 ] *= a; mat[ 1 ][ 4 ] *= a;
	mat[ 2 ][ 0 ] *= a; mat[ 2 ][ 1 ] *= a; mat[ 2 ][ 2 ] *= a; mat[ 2 ][ 3 ] *= a; mat[ 2 ][ 4 ] *= a;
	mat[ 3 ][ 0 ] *= a; mat[ 3 ][ 1 ] *= a; mat[ 3 ][ 2 ] *= a; mat[ 3 ][ 3 ] *= a; mat[ 3 ][ 4 ] *= a;
	mat[ 4 ][ 0 ] *= a; mat[ 4 ][ 1 ] *= a; mat[ 4 ][ 2 ] *= a; mat[ 4 ][ 3 ] *= a; mat[ 4 ][ 4 ] *= a;
	return *this;
}

INLINE Mat5 & Mat5::operator *=( const Mat5 & a ) {
	*this = *this * a;
	return *this;
}

INLINE Mat5 & Mat5::operator +=( const Mat5 & a ) {
	mat[ 0 ][ 0 ] += a[ 0 ][ 0 ]; mat[ 0 ][ 1 ] += a[ 0 ][ 1 ]; mat[ 0 ][ 2 ] += a[ 0 ][ 2 ]; mat[ 0 ][ 3 ] += a[ 0 ][ 3 ]; mat[ 0 ][ 4 ] += a[ 0 ][ 4 ];
	mat[ 1 ][ 0 ] += a[ 1 ][ 0 ]; mat[ 1 ][ 1 ] += a[ 1 ][ 1 ]; mat[ 1 ][ 2 ] += a[ 1 ][ 2 ]; mat[ 1 ][ 3 ] += a[ 1 ][ 3 ]; mat[ 1 ][ 4 ] += a[ 1 ][ 4 ];
	mat[ 2 ][ 0 ] += a[ 2 ][ 0 ]; mat[ 2 ][ 1 ] += a[ 2 ][ 1 ]; mat[ 2 ][ 2 ] += a[ 2 ][ 2 ]; mat[ 2 ][ 3 ] += a[ 2 ][ 3 ]; mat[ 2 ][ 4 ] += a[ 2 ][ 4 ];
	mat[ 3 ][ 0 ] += a[ 3 ][ 0 ]; mat[ 3 ][ 1 ] += a[ 3 ][ 1 ]; mat[ 3 ][ 2 ] += a[ 3 ][ 2 ]; mat[ 3 ][ 3 ] += a[ 3 ][ 3 ]; mat[ 3 ][ 4 ] += a[ 3 ][ 4 ];
	mat[ 4 ][ 0 ] += a[ 4 ][ 0 ]; mat[ 4 ][ 1 ] += a[ 4 ][ 1 ]; mat[ 4 ][ 2 ] += a[ 4 ][ 2 ]; mat[ 4 ][ 3 ] += a[ 4 ][ 3 ]; mat[ 4 ][ 4 ] += a[ 4 ][ 4 ];
	return *this;
}

INLINE Mat5 & Mat5::operator -=( const Mat5 & a ) {
	mat[ 0 ][ 0 ] -= a[ 0 ][ 0 ]; mat[ 0 ][ 1 ] -= a[ 0 ][ 1 ]; mat[ 0 ][ 2 ] -= a[ 0 ][ 2 ]; mat[ 0 ][ 3 ] -= a[ 0 ][ 3 ]; mat[ 0 ][ 4 ] -= a[ 0 ][ 4 ];
	mat[ 1 ][ 0 ] -= a[ 1 ][ 0 ]; mat[ 1 ][ 1 ] -= a[ 1 ][ 1 ]; mat[ 1 ][ 2 ] -= a[ 1 ][ 2 ]; mat[ 1 ][ 3 ] -= a[ 1 ][ 3 ]; mat[ 1 ][ 4 ] -= a[ 1 ][ 4 ];
	mat[ 2 ][ 0 ] -= a[ 2 ][ 0 ]; mat[ 2 ][ 1 ] -= a[ 2 ][ 1 ]; mat[ 2 ][ 2 ] -= a[ 2 ][ 2 ]; mat[ 2 ][ 3 ] -= a[ 2 ][ 3 ]; mat[ 2 ][ 4 ] -= a[ 2 ][ 4 ];
	mat[ 3 ][ 0 ] -= a[ 3 ][ 0 ]; mat[ 3 ][ 1 ] -= a[ 3 ][ 1 ]; mat[ 3 ][ 2 ] -= a[ 3 ][ 2 ]; mat[ 3 ][ 3 ] -= a[ 3 ][ 3 ]; mat[ 3 ][ 4 ] -= a[ 3 ][ 4 ];
	mat[ 4 ][ 0 ] -= a[ 4 ][ 0 ]; mat[ 4 ][ 1 ] -= a[ 4 ][ 1 ]; mat[ 4 ][ 2 ] -= a[ 4 ][ 2 ]; mat[ 4 ][ 3 ] -= a[ 4 ][ 3 ]; mat[ 4 ][ 4 ] -= a[ 4 ][ 4 ];
	return *this;
}

INLINE Vec5 operator *( const Vec5 & vec, const Mat5 & mat ) {
	return mat * vec;
}

INLINE Mat5 operator *( const float a, Mat5 const & mat ) {
	return mat * a;
}

INLINE Vec5 & operator *=( Vec5 & vec, const Mat5 & mat ) {
	vec = mat * vec;
	return vec;
}

INLINE bool Mat5::Compare( const Mat5 & a ) const {
	dword i;
	const float *ptr1, *ptr2;

	ptr1 = reinterpret_cast<const float *>(mat);
	ptr2 = reinterpret_cast<const float *>(a.mat);
	for( i = 0; i < 5*5; i++ ) {
		if( ptr1[i] != ptr2[i] ) {
			return false;
		}
	}
	return true;
}

INLINE bool Mat5::Compare( const Mat5 & a, const float epsilon ) const {
	dword i;
	const float *ptr1, *ptr2;

	ptr1 = reinterpret_cast<const float *>(mat);
	ptr2 = reinterpret_cast<const float *>(a.mat);
	for( i = 0; i < 5*5; i++ ) {
		if( fabs( ptr1[i] - ptr2[i] ) > epsilon ) {
			return false;
		}
	}
	return true;
}

INLINE bool Mat5::operator ==( const Mat5 & a ) const {
	return Compare( a );
}

INLINE bool Mat5::operator !=( const Mat5 & a ) const {
	return !Compare( a );
}

INLINE void Mat5::Zero( ) {
	Common::Com_Memset( mat, 0, sizeof( Mat5 ) );
}

INLINE void Mat5::Identity( ) {
	*this = mat5_identity;
}

INLINE bool Mat5::IsIdentity( const float epsilon ) const {
	return Compare( mat5_identity, epsilon );
}

INLINE bool Mat5::IsSymmetric( const float epsilon ) const {
	for( int i = 1; i < 5; i++ ) {
		for( int j = 0; j < i; j++ ) {
			if( fabs( mat[i][j] - mat[j][i] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

INLINE bool Mat5::IsDiagonal( const float epsilon ) const {
	for( int i = 0; i < 5; i++ ) {
		for( int j = 0; j < 5; j++ ) {
			if( i != j && fabs( mat[i][j] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

INLINE float Mat5::Trace( ) const {
	return ( mat[ 0 ][ 0 ] + mat[ 1 ][ 1 ] + mat[ 2 ][ 2 ] + mat[ 3 ][ 3 ] + mat[ 4 ][ 4 ] );
}

INLINE Mat5 Mat5::Inverse( ) const {
	Mat5 invMat;

	invMat = *this;
	int r = invMat.InverseSelf( );
	assert( r );
	return invMat;
}

INLINE Mat5 Mat5::InverseFast( ) const {
	Mat5 invMat;

	invMat = *this;
	int r = invMat.InverseFastSelf( );
	assert( r );
	return invMat;
}

INLINE int Mat5::GetDimension( ) const {
	return 25;
}

INLINE const float *Mat5::ToFloatPtr( ) const {
	return mat[ 0 ].ToFloatPtr( );
}

INLINE float *Mat5::ToFloatPtr( ) {
	return mat[ 0 ].ToFloatPtr( );
}

/*
============
Mat5::Transpose
============
*/
INLINE Mat5 Mat5::Transpose( ) const {
	Mat5	transpose;
	int		i, j;
   
	for( i = 0; i < 5; i++ ) {
		for( j = 0; j < 5; j++ ) {
			transpose[ i ][ j ] = mat[ j ][ i ];
        }
	}
	return transpose;
}

/*
============
Mat5::TransposeSelf
============
*/
INLINE Mat5 & Mat5::TransposeSelf( ) {
	float	temp;
	int		i, j;
   
	for( i = 0; i < 5; i++ ) {
		for( j = i + 1; j < 5; j++ ) {
			temp = mat[ i ][ j ];
			mat[ i ][ j ] = mat[ j ][ i ];
			mat[ j ][ i ] = temp;
        }
	}
	return *this;
}

/*
============
Mat5::Determinant
============
*/
INLINE float Mat5::Determinant( ) const {

	// 2x2 sub-determinants required to calculate 5x5 determinant
	float det2_34_01 = mat[ 3 ][ 0 ] * mat[ 4 ][ 1 ] - mat[ 3 ][ 1 ] * mat[ 4 ][ 0 ];
	float det2_34_02 = mat[ 3 ][ 0 ] * mat[ 4 ][ 2 ] - mat[ 3 ][ 2 ] * mat[ 4 ][ 0 ];
	float det2_34_03 = mat[ 3 ][ 0 ] * mat[ 4 ][ 3 ] - mat[ 3 ][ 3 ] * mat[ 4 ][ 0 ];
	float det2_34_04 = mat[ 3 ][ 0 ] * mat[ 4 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 4 ][ 0 ];
	float det2_34_12 = mat[ 3 ][ 1 ] * mat[ 4 ][ 2 ] - mat[ 3 ][ 2 ] * mat[ 4 ][ 1 ];
	float det2_34_13 = mat[ 3 ][ 1 ] * mat[ 4 ][ 3 ] - mat[ 3 ][ 3 ] * mat[ 4 ][ 1 ];
	float det2_34_14 = mat[ 3 ][ 1 ] * mat[ 4 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 4 ][ 1 ];
	float det2_34_23 = mat[ 3 ][ 2 ] * mat[ 4 ][ 3 ] - mat[ 3 ][ 3 ] * mat[ 4 ][ 2 ];
	float det2_34_24 = mat[ 3 ][ 2 ] * mat[ 4 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 4 ][ 2 ];
	float det2_34_34 = mat[ 3 ][ 3 ] * mat[ 4 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 4 ][ 3 ];

	// 3x3 sub-determinants required to calculate 5x5 determinant
	float det3_234_012 = mat[ 2 ][ 0 ] * det2_34_12 - mat[ 2 ][ 1 ] * det2_34_02 + mat[ 2 ][ 2 ] * det2_34_01;
	float det3_234_013 = mat[ 2 ][ 0 ] * det2_34_13 - mat[ 2 ][ 1 ] * det2_34_03 + mat[ 2 ][ 3 ] * det2_34_01;
	float det3_234_014 = mat[ 2 ][ 0 ] * det2_34_14 - mat[ 2 ][ 1 ] * det2_34_04 + mat[ 2 ][ 4 ] * det2_34_01;
	float det3_234_023 = mat[ 2 ][ 0 ] * det2_34_23 - mat[ 2 ][ 2 ] * det2_34_03 + mat[ 2 ][ 3 ] * det2_34_02;
	float det3_234_024 = mat[ 2 ][ 0 ] * det2_34_24 - mat[ 2 ][ 2 ] * det2_34_04 + mat[ 2 ][ 4 ] * det2_34_02;
	float det3_234_034 = mat[ 2 ][ 0 ] * det2_34_34 - mat[ 2 ][ 3 ] * det2_34_04 + mat[ 2 ][ 4 ] * det2_34_03;
	float det3_234_123 = mat[ 2 ][ 1 ] * det2_34_23 - mat[ 2 ][ 2 ] * det2_34_13 + mat[ 2 ][ 3 ] * det2_34_12;
	float det3_234_124 = mat[ 2 ][ 1 ] * det2_34_24 - mat[ 2 ][ 2 ] * det2_34_14 + mat[ 2 ][ 4 ] * det2_34_12;
	float det3_234_134 = mat[ 2 ][ 1 ] * det2_34_34 - mat[ 2 ][ 3 ] * det2_34_14 + mat[ 2 ][ 4 ] * det2_34_13;
	float det3_234_234 = mat[ 2 ][ 2 ] * det2_34_34 - mat[ 2 ][ 3 ] * det2_34_24 + mat[ 2 ][ 4 ] * det2_34_23;

	// 4x4 sub-determinants required to calculate 5x5 determinant
	float det4_1234_0123 = mat[ 1 ][ 0 ] * det3_234_123 - mat[ 1 ][ 1 ] * det3_234_023 + mat[ 1 ][ 2 ] * det3_234_013 - mat[ 1 ][ 3 ] * det3_234_012;
	float det4_1234_0124 = mat[ 1 ][ 0 ] * det3_234_124 - mat[ 1 ][ 1 ] * det3_234_024 + mat[ 1 ][ 2 ] * det3_234_014 - mat[ 1 ][ 4 ] * det3_234_012;
	float det4_1234_0134 = mat[ 1 ][ 0 ] * det3_234_134 - mat[ 1 ][ 1 ] * det3_234_034 + mat[ 1 ][ 3 ] * det3_234_014 - mat[ 1 ][ 4 ] * det3_234_013;
	float det4_1234_0234 = mat[ 1 ][ 0 ] * det3_234_234 - mat[ 1 ][ 2 ] * det3_234_034 + mat[ 1 ][ 3 ] * det3_234_024 - mat[ 1 ][ 4 ] * det3_234_023;
	float det4_1234_1234 = mat[ 1 ][ 1 ] * det3_234_234 - mat[ 1 ][ 2 ] * det3_234_134 + mat[ 1 ][ 3 ] * det3_234_124 - mat[ 1 ][ 4 ] * det3_234_123;

	// determinant of 5x5 matrix
	return mat[ 0 ][ 0 ] * det4_1234_1234 - mat[ 0 ][ 1 ] * det4_1234_0234 + mat[ 0 ][ 2 ] * det4_1234_0134 - mat[ 0 ][ 3 ] * det4_1234_0124 + mat[ 0 ][ 4 ] * det4_1234_0123;
}

/*
============
Mat5::InverseSelf
============
*/
INLINE bool Mat5::InverseSelf( ) {
	// 280+5+25 = 310 multiplications
	//				1 division
	float det, invDet;

	// 2x2 sub-determinants required to calculate 5x5 determinant
	float det2_34_01 = mat[ 3 ][ 0 ] * mat[ 4 ][ 1 ] - mat[ 3 ][ 1 ] * mat[ 4 ][ 0 ];
	float det2_34_02 = mat[ 3 ][ 0 ] * mat[ 4 ][ 2 ] - mat[ 3 ][ 2 ] * mat[ 4 ][ 0 ];
	float det2_34_03 = mat[ 3 ][ 0 ] * mat[ 4 ][ 3 ] - mat[ 3 ][ 3 ] * mat[ 4 ][ 0 ];
	float det2_34_04 = mat[ 3 ][ 0 ] * mat[ 4 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 4 ][ 0 ];
	float det2_34_12 = mat[ 3 ][ 1 ] * mat[ 4 ][ 2 ] - mat[ 3 ][ 2 ] * mat[ 4 ][ 1 ];
	float det2_34_13 = mat[ 3 ][ 1 ] * mat[ 4 ][ 3 ] - mat[ 3 ][ 3 ] * mat[ 4 ][ 1 ];
	float det2_34_14 = mat[ 3 ][ 1 ] * mat[ 4 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 4 ][ 1 ];
	float det2_34_23 = mat[ 3 ][ 2 ] * mat[ 4 ][ 3 ] - mat[ 3 ][ 3 ] * mat[ 4 ][ 2 ];
	float det2_34_24 = mat[ 3 ][ 2 ] * mat[ 4 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 4 ][ 2 ];
	float det2_34_34 = mat[ 3 ][ 3 ] * mat[ 4 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 4 ][ 3 ];

	// 3x3 sub-determinants required to calculate 5x5 determinant
	float det3_234_012 = mat[ 2 ][ 0 ] * det2_34_12 - mat[ 2 ][ 1 ] * det2_34_02 + mat[ 2 ][ 2 ] * det2_34_01;
	float det3_234_013 = mat[ 2 ][ 0 ] * det2_34_13 - mat[ 2 ][ 1 ] * det2_34_03 + mat[ 2 ][ 3 ] * det2_34_01;
	float det3_234_014 = mat[ 2 ][ 0 ] * det2_34_14 - mat[ 2 ][ 1 ] * det2_34_04 + mat[ 2 ][ 4 ] * det2_34_01;
	float det3_234_023 = mat[ 2 ][ 0 ] * det2_34_23 - mat[ 2 ][ 2 ] * det2_34_03 + mat[ 2 ][ 3 ] * det2_34_02;
	float det3_234_024 = mat[ 2 ][ 0 ] * det2_34_24 - mat[ 2 ][ 2 ] * det2_34_04 + mat[ 2 ][ 4 ] * det2_34_02;
	float det3_234_034 = mat[ 2 ][ 0 ] * det2_34_34 - mat[ 2 ][ 3 ] * det2_34_04 + mat[ 2 ][ 4 ] * det2_34_03;
	float det3_234_123 = mat[ 2 ][ 1 ] * det2_34_23 - mat[ 2 ][ 2 ] * det2_34_13 + mat[ 2 ][ 3 ] * det2_34_12;
	float det3_234_124 = mat[ 2 ][ 1 ] * det2_34_24 - mat[ 2 ][ 2 ] * det2_34_14 + mat[ 2 ][ 4 ] * det2_34_12;
	float det3_234_134 = mat[ 2 ][ 1 ] * det2_34_34 - mat[ 2 ][ 3 ] * det2_34_14 + mat[ 2 ][ 4 ] * det2_34_13;
	float det3_234_234 = mat[ 2 ][ 2 ] * det2_34_34 - mat[ 2 ][ 3 ] * det2_34_24 + mat[ 2 ][ 4 ] * det2_34_23;

	// 4x4 sub-determinants required to calculate 5x5 determinant
	float det4_1234_0123 = mat[ 1 ][ 0 ] * det3_234_123 - mat[ 1 ][ 1 ] * det3_234_023 + mat[ 1 ][ 2 ] * det3_234_013 - mat[ 1 ][ 3 ] * det3_234_012;
	float det4_1234_0124 = mat[ 1 ][ 0 ] * det3_234_124 - mat[ 1 ][ 1 ] * det3_234_024 + mat[ 1 ][ 2 ] * det3_234_014 - mat[ 1 ][ 4 ] * det3_234_012;
	float det4_1234_0134 = mat[ 1 ][ 0 ] * det3_234_134 - mat[ 1 ][ 1 ] * det3_234_034 + mat[ 1 ][ 3 ] * det3_234_014 - mat[ 1 ][ 4 ] * det3_234_013;
	float det4_1234_0234 = mat[ 1 ][ 0 ] * det3_234_234 - mat[ 1 ][ 2 ] * det3_234_034 + mat[ 1 ][ 3 ] * det3_234_024 - mat[ 1 ][ 4 ] * det3_234_023;
	float det4_1234_1234 = mat[ 1 ][ 1 ] * det3_234_234 - mat[ 1 ][ 2 ] * det3_234_134 + mat[ 1 ][ 3 ] * det3_234_124 - mat[ 1 ][ 4 ] * det3_234_123;

	// determinant of 5x5 matrix
	det = mat[ 0 ][ 0 ] * det4_1234_1234 - mat[ 0 ][ 1 ] * det4_1234_0234 + mat[ 0 ][ 2 ] * det4_1234_0134 - mat[ 0 ][ 3 ] * det4_1234_0124 + mat[ 0 ][ 4 ] * det4_1234_0123;

	if( fabs( det ) < MATRIX_INVERSE_EPSILON ) {  
		return false;
	}

	invDet = 1.0f / det;

	// remaining 2x2 sub-determinants
	float det2_23_01 = mat[ 2 ][ 0 ] * mat[ 3 ][ 1 ] - mat[ 2 ][ 1 ] * mat[ 3 ][ 0 ];
	float det2_23_02 = mat[ 2 ][ 0 ] * mat[ 3 ][ 2 ] - mat[ 2 ][ 2 ] * mat[ 3 ][ 0 ];
	float det2_23_03 = mat[ 2 ][ 0 ] * mat[ 3 ][ 3 ] - mat[ 2 ][ 3 ] * mat[ 3 ][ 0 ];
	float det2_23_04 = mat[ 2 ][ 0 ] * mat[ 3 ][ 4 ] - mat[ 2 ][ 4 ] * mat[ 3 ][ 0 ];
	float det2_23_12 = mat[ 2 ][ 1 ] * mat[ 3 ][ 2 ] - mat[ 2 ][ 2 ] * mat[ 3 ][ 1 ];
	float det2_23_13 = mat[ 2 ][ 1 ] * mat[ 3 ][ 3 ] - mat[ 2 ][ 3 ] * mat[ 3 ][ 1 ];
	float det2_23_14 = mat[ 2 ][ 1 ] * mat[ 3 ][ 4 ] - mat[ 2 ][ 4 ] * mat[ 3 ][ 1 ];
	float det2_23_23 = mat[ 2 ][ 2 ] * mat[ 3 ][ 3 ] - mat[ 2 ][ 3 ] * mat[ 3 ][ 2 ];
	float det2_23_24 = mat[ 2 ][ 2 ] * mat[ 3 ][ 4 ] - mat[ 2 ][ 4 ] * mat[ 3 ][ 2 ];
	float det2_23_34 = mat[ 2 ][ 3 ] * mat[ 3 ][ 4 ] - mat[ 2 ][ 4 ] * mat[ 3 ][ 3 ];
	float det2_24_01 = mat[ 2 ][ 0 ] * mat[ 4 ][ 1 ] - mat[ 2 ][ 1 ] * mat[ 4 ][ 0 ];
	float det2_24_02 = mat[ 2 ][ 0 ] * mat[ 4 ][ 2 ] - mat[ 2 ][ 2 ] * mat[ 4 ][ 0 ];
	float det2_24_03 = mat[ 2 ][ 0 ] * mat[ 4 ][ 3 ] - mat[ 2 ][ 3 ] * mat[ 4 ][ 0 ];
	float det2_24_04 = mat[ 2 ][ 0 ] * mat[ 4 ][ 4 ] - mat[ 2 ][ 4 ] * mat[ 4 ][ 0 ];
	float det2_24_12 = mat[ 2 ][ 1 ] * mat[ 4 ][ 2 ] - mat[ 2 ][ 2 ] * mat[ 4 ][ 1 ];
	float det2_24_13 = mat[ 2 ][ 1 ] * mat[ 4 ][ 3 ] - mat[ 2 ][ 3 ] * mat[ 4 ][ 1 ];
	float det2_24_14 = mat[ 2 ][ 1 ] * mat[ 4 ][ 4 ] - mat[ 2 ][ 4 ] * mat[ 4 ][ 1 ];
	float det2_24_23 = mat[ 2 ][ 2 ] * mat[ 4 ][ 3 ] - mat[ 2 ][ 3 ] * mat[ 4 ][ 2 ];
	float det2_24_24 = mat[ 2 ][ 2 ] * mat[ 4 ][ 4 ] - mat[ 2 ][ 4 ] * mat[ 4 ][ 2 ];
	float det2_24_34 = mat[ 2 ][ 3 ] * mat[ 4 ][ 4 ] - mat[ 2 ][ 4 ] * mat[ 4 ][ 3 ];

	// remaining 3x3 sub-determinants
	float det3_123_012 = mat[ 1 ][ 0 ] * det2_23_12 - mat[ 1 ][ 1 ] * det2_23_02 + mat[ 1 ][ 2 ] * det2_23_01;
	float det3_123_013 = mat[ 1 ][ 0 ] * det2_23_13 - mat[ 1 ][ 1 ] * det2_23_03 + mat[ 1 ][ 3 ] * det2_23_01;
	float det3_123_014 = mat[ 1 ][ 0 ] * det2_23_14 - mat[ 1 ][ 1 ] * det2_23_04 + mat[ 1 ][ 4 ] * det2_23_01;
	float det3_123_023 = mat[ 1 ][ 0 ] * det2_23_23 - mat[ 1 ][ 2 ] * det2_23_03 + mat[ 1 ][ 3 ] * det2_23_02;
	float det3_123_024 = mat[ 1 ][ 0 ] * det2_23_24 - mat[ 1 ][ 2 ] * det2_23_04 + mat[ 1 ][ 4 ] * det2_23_02;
	float det3_123_034 = mat[ 1 ][ 0 ] * det2_23_34 - mat[ 1 ][ 3 ] * det2_23_04 + mat[ 1 ][ 4 ] * det2_23_03;
	float det3_123_123 = mat[ 1 ][ 1 ] * det2_23_23 - mat[ 1 ][ 2 ] * det2_23_13 + mat[ 1 ][ 3 ] * det2_23_12;
	float det3_123_124 = mat[ 1 ][ 1 ] * det2_23_24 - mat[ 1 ][ 2 ] * det2_23_14 + mat[ 1 ][ 4 ] * det2_23_12;
	float det3_123_134 = mat[ 1 ][ 1 ] * det2_23_34 - mat[ 1 ][ 3 ] * det2_23_14 + mat[ 1 ][ 4 ] * det2_23_13;
	float det3_123_234 = mat[ 1 ][ 2 ] * det2_23_34 - mat[ 1 ][ 3 ] * det2_23_24 + mat[ 1 ][ 4 ] * det2_23_23;
	float det3_124_012 = mat[ 1 ][ 0 ] * det2_24_12 - mat[ 1 ][ 1 ] * det2_24_02 + mat[ 1 ][ 2 ] * det2_24_01;
	float det3_124_013 = mat[ 1 ][ 0 ] * det2_24_13 - mat[ 1 ][ 1 ] * det2_24_03 + mat[ 1 ][ 3 ] * det2_24_01;
	float det3_124_014 = mat[ 1 ][ 0 ] * det2_24_14 - mat[ 1 ][ 1 ] * det2_24_04 + mat[ 1 ][ 4 ] * det2_24_01;
	float det3_124_023 = mat[ 1 ][ 0 ] * det2_24_23 - mat[ 1 ][ 2 ] * det2_24_03 + mat[ 1 ][ 3 ] * det2_24_02;
	float det3_124_024 = mat[ 1 ][ 0 ] * det2_24_24 - mat[ 1 ][ 2 ] * det2_24_04 + mat[ 1 ][ 4 ] * det2_24_02;
	float det3_124_034 = mat[ 1 ][ 0 ] * det2_24_34 - mat[ 1 ][ 3 ] * det2_24_04 + mat[ 1 ][ 4 ] * det2_24_03;
	float det3_124_123 = mat[ 1 ][ 1 ] * det2_24_23 - mat[ 1 ][ 2 ] * det2_24_13 + mat[ 1 ][ 3 ] * det2_24_12;
	float det3_124_124 = mat[ 1 ][ 1 ] * det2_24_24 - mat[ 1 ][ 2 ] * det2_24_14 + mat[ 1 ][ 4 ] * det2_24_12;
	float det3_124_134 = mat[ 1 ][ 1 ] * det2_24_34 - mat[ 1 ][ 3 ] * det2_24_14 + mat[ 1 ][ 4 ] * det2_24_13;
	float det3_124_234 = mat[ 1 ][ 2 ] * det2_24_34 - mat[ 1 ][ 3 ] * det2_24_24 + mat[ 1 ][ 4 ] * det2_24_23;
	float det3_134_012 = mat[ 1 ][ 0 ] * det2_34_12 - mat[ 1 ][ 1 ] * det2_34_02 + mat[ 1 ][ 2 ] * det2_34_01;
	float det3_134_013 = mat[ 1 ][ 0 ] * det2_34_13 - mat[ 1 ][ 1 ] * det2_34_03 + mat[ 1 ][ 3 ] * det2_34_01;
	float det3_134_014 = mat[ 1 ][ 0 ] * det2_34_14 - mat[ 1 ][ 1 ] * det2_34_04 + mat[ 1 ][ 4 ] * det2_34_01;
	float det3_134_023 = mat[ 1 ][ 0 ] * det2_34_23 - mat[ 1 ][ 2 ] * det2_34_03 + mat[ 1 ][ 3 ] * det2_34_02;
	float det3_134_024 = mat[ 1 ][ 0 ] * det2_34_24 - mat[ 1 ][ 2 ] * det2_34_04 + mat[ 1 ][ 4 ] * det2_34_02;
	float det3_134_034 = mat[ 1 ][ 0 ] * det2_34_34 - mat[ 1 ][ 3 ] * det2_34_04 + mat[ 1 ][ 4 ] * det2_34_03;
	float det3_134_123 = mat[ 1 ][ 1 ] * det2_34_23 - mat[ 1 ][ 2 ] * det2_34_13 + mat[ 1 ][ 3 ] * det2_34_12;
	float det3_134_124 = mat[ 1 ][ 1 ] * det2_34_24 - mat[ 1 ][ 2 ] * det2_34_14 + mat[ 1 ][ 4 ] * det2_34_12;
	float det3_134_134 = mat[ 1 ][ 1 ] * det2_34_34 - mat[ 1 ][ 3 ] * det2_34_14 + mat[ 1 ][ 4 ] * det2_34_13;
	float det3_134_234 = mat[ 1 ][ 2 ] * det2_34_34 - mat[ 1 ][ 3 ] * det2_34_24 + mat[ 1 ][ 4 ] * det2_34_23;

	// remaining 4x4 sub-determinants
	float det4_0123_0123 = mat[ 0 ][ 0 ] * det3_123_123 - mat[ 0 ][ 1 ] * det3_123_023 + mat[ 0 ][ 2 ] * det3_123_013 - mat[ 0 ][ 3 ] * det3_123_012;
	float det4_0123_0124 = mat[ 0 ][ 0 ] * det3_123_124 - mat[ 0 ][ 1 ] * det3_123_024 + mat[ 0 ][ 2 ] * det3_123_014 - mat[ 0 ][ 4 ] * det3_123_012;
	float det4_0123_0134 = mat[ 0 ][ 0 ] * det3_123_134 - mat[ 0 ][ 1 ] * det3_123_034 + mat[ 0 ][ 3 ] * det3_123_014 - mat[ 0 ][ 4 ] * det3_123_013;
	float det4_0123_0234 = mat[ 0 ][ 0 ] * det3_123_234 - mat[ 0 ][ 2 ] * det3_123_034 + mat[ 0 ][ 3 ] * det3_123_024 - mat[ 0 ][ 4 ] * det3_123_023;
	float det4_0123_1234 = mat[ 0 ][ 1 ] * det3_123_234 - mat[ 0 ][ 2 ] * det3_123_134 + mat[ 0 ][ 3 ] * det3_123_124 - mat[ 0 ][ 4 ] * det3_123_123;
	float det4_0124_0123 = mat[ 0 ][ 0 ] * det3_124_123 - mat[ 0 ][ 1 ] * det3_124_023 + mat[ 0 ][ 2 ] * det3_124_013 - mat[ 0 ][ 3 ] * det3_124_012;
	float det4_0124_0124 = mat[ 0 ][ 0 ] * det3_124_124 - mat[ 0 ][ 1 ] * det3_124_024 + mat[ 0 ][ 2 ] * det3_124_014 - mat[ 0 ][ 4 ] * det3_124_012;
	float det4_0124_0134 = mat[ 0 ][ 0 ] * det3_124_134 - mat[ 0 ][ 1 ] * det3_124_034 + mat[ 0 ][ 3 ] * det3_124_014 - mat[ 0 ][ 4 ] * det3_124_013;
	float det4_0124_0234 = mat[ 0 ][ 0 ] * det3_124_234 - mat[ 0 ][ 2 ] * det3_124_034 + mat[ 0 ][ 3 ] * det3_124_024 - mat[ 0 ][ 4 ] * det3_124_023;
	float det4_0124_1234 = mat[ 0 ][ 1 ] * det3_124_234 - mat[ 0 ][ 2 ] * det3_124_134 + mat[ 0 ][ 3 ] * det3_124_124 - mat[ 0 ][ 4 ] * det3_124_123;
	float det4_0134_0123 = mat[ 0 ][ 0 ] * det3_134_123 - mat[ 0 ][ 1 ] * det3_134_023 + mat[ 0 ][ 2 ] * det3_134_013 - mat[ 0 ][ 3 ] * det3_134_012;
	float det4_0134_0124 = mat[ 0 ][ 0 ] * det3_134_124 - mat[ 0 ][ 1 ] * det3_134_024 + mat[ 0 ][ 2 ] * det3_134_014 - mat[ 0 ][ 4 ] * det3_134_012;
	float det4_0134_0134 = mat[ 0 ][ 0 ] * det3_134_134 - mat[ 0 ][ 1 ] * det3_134_034 + mat[ 0 ][ 3 ] * det3_134_014 - mat[ 0 ][ 4 ] * det3_134_013;
	float det4_0134_0234 = mat[ 0 ][ 0 ] * det3_134_234 - mat[ 0 ][ 2 ] * det3_134_034 + mat[ 0 ][ 3 ] * det3_134_024 - mat[ 0 ][ 4 ] * det3_134_023;
	float det4_0134_1234 = mat[ 0 ][ 1 ] * det3_134_234 - mat[ 0 ][ 2 ] * det3_134_134 + mat[ 0 ][ 3 ] * det3_134_124 - mat[ 0 ][ 4 ] * det3_134_123;
	float det4_0234_0123 = mat[ 0 ][ 0 ] * det3_234_123 - mat[ 0 ][ 1 ] * det3_234_023 + mat[ 0 ][ 2 ] * det3_234_013 - mat[ 0 ][ 3 ] * det3_234_012;
	float det4_0234_0124 = mat[ 0 ][ 0 ] * det3_234_124 - mat[ 0 ][ 1 ] * det3_234_024 + mat[ 0 ][ 2 ] * det3_234_014 - mat[ 0 ][ 4 ] * det3_234_012;
	float det4_0234_0134 = mat[ 0 ][ 0 ] * det3_234_134 - mat[ 0 ][ 1 ] * det3_234_034 + mat[ 0 ][ 3 ] * det3_234_014 - mat[ 0 ][ 4 ] * det3_234_013;
	float det4_0234_0234 = mat[ 0 ][ 0 ] * det3_234_234 - mat[ 0 ][ 2 ] * det3_234_034 + mat[ 0 ][ 3 ] * det3_234_024 - mat[ 0 ][ 4 ] * det3_234_023;
	float det4_0234_1234 = mat[ 0 ][ 1 ] * det3_234_234 - mat[ 0 ][ 2 ] * det3_234_134 + mat[ 0 ][ 3 ] * det3_234_124 - mat[ 0 ][ 4 ] * det3_234_123;

	mat[ 0 ][ 0 ] =  det4_1234_1234 * invDet;
	mat[ 0 ][ 1 ] = -det4_0234_1234 * invDet;
	mat[ 0 ][ 2 ] =  det4_0134_1234 * invDet;
	mat[ 0 ][ 3 ] = -det4_0124_1234 * invDet;
	mat[ 0 ][ 4 ] =  det4_0123_1234 * invDet;

	mat[ 1 ][ 0 ] = -det4_1234_0234 * invDet;
	mat[ 1 ][ 1 ] =  det4_0234_0234 * invDet;
	mat[ 1 ][ 2 ] = -det4_0134_0234 * invDet;
	mat[ 1 ][ 3 ] =  det4_0124_0234 * invDet;
	mat[ 1 ][ 4 ] = -det4_0123_0234 * invDet;

	mat[ 2 ][ 0 ] =  det4_1234_0134 * invDet;
	mat[ 2 ][ 1 ] = -det4_0234_0134 * invDet;
	mat[ 2 ][ 2 ] =  det4_0134_0134 * invDet;
	mat[ 2 ][ 3 ] = -det4_0124_0134 * invDet;
	mat[ 2 ][ 4 ] =  det4_0123_0134 * invDet;

	mat[ 3 ][ 0 ] = -det4_1234_0124 * invDet;
	mat[ 3 ][ 1 ] =  det4_0234_0124 * invDet;
	mat[ 3 ][ 2 ] = -det4_0134_0124 * invDet;
	mat[ 3 ][ 3 ] =  det4_0124_0124 * invDet;
	mat[ 3 ][ 4 ] = -det4_0123_0124 * invDet;

	mat[ 4 ][ 0 ] =  det4_1234_0123 * invDet;
	mat[ 4 ][ 1 ] = -det4_0234_0123 * invDet;
	mat[ 4 ][ 2 ] =  det4_0134_0123 * invDet;
	mat[ 4 ][ 3 ] = -det4_0124_0123 * invDet;
	mat[ 4 ][ 4 ] =  det4_0123_0123 * invDet;

	return true;
}

/*
============
Mat5::InverseFastSelf
============
*/
INLINE bool Mat5::InverseFastSelf( ) {
#if 0
	// 280+5+25 = 310 multiplications
	//				1 division
	float det, invDet;

	// 2x2 sub-determinants required to calculate 5x5 determinant
	float det2_34_01 = mat[ 3 ][ 0 ] * mat[ 4 ][ 1 ] - mat[ 3 ][ 1 ] * mat[ 4 ][ 0 ];
	float det2_34_02 = mat[ 3 ][ 0 ] * mat[ 4 ][ 2 ] - mat[ 3 ][ 2 ] * mat[ 4 ][ 0 ];
	float det2_34_03 = mat[ 3 ][ 0 ] * mat[ 4 ][ 3 ] - mat[ 3 ][ 3 ] * mat[ 4 ][ 0 ];
	float det2_34_04 = mat[ 3 ][ 0 ] * mat[ 4 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 4 ][ 0 ];
	float det2_34_12 = mat[ 3 ][ 1 ] * mat[ 4 ][ 2 ] - mat[ 3 ][ 2 ] * mat[ 4 ][ 1 ];
	float det2_34_13 = mat[ 3 ][ 1 ] * mat[ 4 ][ 3 ] - mat[ 3 ][ 3 ] * mat[ 4 ][ 1 ];
	float det2_34_14 = mat[ 3 ][ 1 ] * mat[ 4 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 4 ][ 1 ];
	float det2_34_23 = mat[ 3 ][ 2 ] * mat[ 4 ][ 3 ] - mat[ 3 ][ 3 ] * mat[ 4 ][ 2 ];
	float det2_34_24 = mat[ 3 ][ 2 ] * mat[ 4 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 4 ][ 2 ];
	float det2_34_34 = mat[ 3 ][ 3 ] * mat[ 4 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 4 ][ 3 ];

	// 3x3 sub-determinants required to calculate 5x5 determinant
	float det3_234_012 = mat[ 2 ][ 0 ] * det2_34_12 - mat[ 2 ][ 1 ] * det2_34_02 + mat[ 2 ][ 2 ] * det2_34_01;
	float det3_234_013 = mat[ 2 ][ 0 ] * det2_34_13 - mat[ 2 ][ 1 ] * det2_34_03 + mat[ 2 ][ 3 ] * det2_34_01;
	float det3_234_014 = mat[ 2 ][ 0 ] * det2_34_14 - mat[ 2 ][ 1 ] * det2_34_04 + mat[ 2 ][ 4 ] * det2_34_01;
	float det3_234_023 = mat[ 2 ][ 0 ] * det2_34_23 - mat[ 2 ][ 2 ] * det2_34_03 + mat[ 2 ][ 3 ] * det2_34_02;
	float det3_234_024 = mat[ 2 ][ 0 ] * det2_34_24 - mat[ 2 ][ 2 ] * det2_34_04 + mat[ 2 ][ 4 ] * det2_34_02;
	float det3_234_034 = mat[ 2 ][ 0 ] * det2_34_34 - mat[ 2 ][ 3 ] * det2_34_04 + mat[ 2 ][ 4 ] * det2_34_03;
	float det3_234_123 = mat[ 2 ][ 1 ] * det2_34_23 - mat[ 2 ][ 2 ] * det2_34_13 + mat[ 2 ][ 3 ] * det2_34_12;
	float det3_234_124 = mat[ 2 ][ 1 ] * det2_34_24 - mat[ 2 ][ 2 ] * det2_34_14 + mat[ 2 ][ 4 ] * det2_34_12;
	float det3_234_134 = mat[ 2 ][ 1 ] * det2_34_34 - mat[ 2 ][ 3 ] * det2_34_14 + mat[ 2 ][ 4 ] * det2_34_13;
	float det3_234_234 = mat[ 2 ][ 2 ] * det2_34_34 - mat[ 2 ][ 3 ] * det2_34_24 + mat[ 2 ][ 4 ] * det2_34_23;

	// 4x4 sub-determinants required to calculate 5x5 determinant
	float det4_1234_0123 = mat[ 1 ][ 0 ] * det3_234_123 - mat[ 1 ][ 1 ] * det3_234_023 + mat[ 1 ][ 2 ] * det3_234_013 - mat[ 1 ][ 3 ] * det3_234_012;
	float det4_1234_0124 = mat[ 1 ][ 0 ] * det3_234_124 - mat[ 1 ][ 1 ] * det3_234_024 + mat[ 1 ][ 2 ] * det3_234_014 - mat[ 1 ][ 4 ] * det3_234_012;
	float det4_1234_0134 = mat[ 1 ][ 0 ] * det3_234_134 - mat[ 1 ][ 1 ] * det3_234_034 + mat[ 1 ][ 3 ] * det3_234_014 - mat[ 1 ][ 4 ] * det3_234_013;
	float det4_1234_0234 = mat[ 1 ][ 0 ] * det3_234_234 - mat[ 1 ][ 2 ] * det3_234_034 + mat[ 1 ][ 3 ] * det3_234_024 - mat[ 1 ][ 4 ] * det3_234_023;
	float det4_1234_1234 = mat[ 1 ][ 1 ] * det3_234_234 - mat[ 1 ][ 2 ] * det3_234_134 + mat[ 1 ][ 3 ] * det3_234_124 - mat[ 1 ][ 4 ] * det3_234_123;

	// determinant of 5x5 matrix
	det = mat[ 0 ][ 0 ] * det4_1234_1234 - mat[ 0 ][ 1 ] * det4_1234_0234 + mat[ 0 ][ 2 ] * det4_1234_0134 - mat[ 0 ][ 3 ] * det4_1234_0124 + mat[ 0 ][ 4 ] * det4_1234_0123;

	if( fabs( det ) < MATRIX_INVERSE_EPSILON ) {  
		return false;
	}

	invDet = 1.0f / det;

	// remaining 2x2 sub-determinants
	float det2_23_01 = mat[ 2 ][ 0 ] * mat[ 3 ][ 1 ] - mat[ 2 ][ 1 ] * mat[ 3 ][ 0 ];
	float det2_23_02 = mat[ 2 ][ 0 ] * mat[ 3 ][ 2 ] - mat[ 2 ][ 2 ] * mat[ 3 ][ 0 ];
	float det2_23_03 = mat[ 2 ][ 0 ] * mat[ 3 ][ 3 ] - mat[ 2 ][ 3 ] * mat[ 3 ][ 0 ];
	float det2_23_04 = mat[ 2 ][ 0 ] * mat[ 3 ][ 4 ] - mat[ 2 ][ 4 ] * mat[ 3 ][ 0 ];
	float det2_23_12 = mat[ 2 ][ 1 ] * mat[ 3 ][ 2 ] - mat[ 2 ][ 2 ] * mat[ 3 ][ 1 ];
	float det2_23_13 = mat[ 2 ][ 1 ] * mat[ 3 ][ 3 ] - mat[ 2 ][ 3 ] * mat[ 3 ][ 1 ];
	float det2_23_14 = mat[ 2 ][ 1 ] * mat[ 3 ][ 4 ] - mat[ 2 ][ 4 ] * mat[ 3 ][ 1 ];
	float det2_23_23 = mat[ 2 ][ 2 ] * mat[ 3 ][ 3 ] - mat[ 2 ][ 3 ] * mat[ 3 ][ 2 ];
	float det2_23_24 = mat[ 2 ][ 2 ] * mat[ 3 ][ 4 ] - mat[ 2 ][ 4 ] * mat[ 3 ][ 2 ];
	float det2_23_34 = mat[ 2 ][ 3 ] * mat[ 3 ][ 4 ] - mat[ 2 ][ 4 ] * mat[ 3 ][ 3 ];
	float det2_24_01 = mat[ 2 ][ 0 ] * mat[ 4 ][ 1 ] - mat[ 2 ][ 1 ] * mat[ 4 ][ 0 ];
	float det2_24_02 = mat[ 2 ][ 0 ] * mat[ 4 ][ 2 ] - mat[ 2 ][ 2 ] * mat[ 4 ][ 0 ];
	float det2_24_03 = mat[ 2 ][ 0 ] * mat[ 4 ][ 3 ] - mat[ 2 ][ 3 ] * mat[ 4 ][ 0 ];
	float det2_24_04 = mat[ 2 ][ 0 ] * mat[ 4 ][ 4 ] - mat[ 2 ][ 4 ] * mat[ 4 ][ 0 ];
	float det2_24_12 = mat[ 2 ][ 1 ] * mat[ 4 ][ 2 ] - mat[ 2 ][ 2 ] * mat[ 4 ][ 1 ];
	float det2_24_13 = mat[ 2 ][ 1 ] * mat[ 4 ][ 3 ] - mat[ 2 ][ 3 ] * mat[ 4 ][ 1 ];
	float det2_24_14 = mat[ 2 ][ 1 ] * mat[ 4 ][ 4 ] - mat[ 2 ][ 4 ] * mat[ 4 ][ 1 ];
	float det2_24_23 = mat[ 2 ][ 2 ] * mat[ 4 ][ 3 ] - mat[ 2 ][ 3 ] * mat[ 4 ][ 2 ];
	float det2_24_24 = mat[ 2 ][ 2 ] * mat[ 4 ][ 4 ] - mat[ 2 ][ 4 ] * mat[ 4 ][ 2 ];
	float det2_24_34 = mat[ 2 ][ 3 ] * mat[ 4 ][ 4 ] - mat[ 2 ][ 4 ] * mat[ 4 ][ 3 ];

	// remaining 3x3 sub-determinants
	float det3_123_012 = mat[ 1 ][ 0 ] * det2_23_12 - mat[ 1 ][ 1 ] * det2_23_02 + mat[ 1 ][ 2 ] * det2_23_01;
	float det3_123_013 = mat[ 1 ][ 0 ] * det2_23_13 - mat[ 1 ][ 1 ] * det2_23_03 + mat[ 1 ][ 3 ] * det2_23_01;
	float det3_123_014 = mat[ 1 ][ 0 ] * det2_23_14 - mat[ 1 ][ 1 ] * det2_23_04 + mat[ 1 ][ 4 ] * det2_23_01;
	float det3_123_023 = mat[ 1 ][ 0 ] * det2_23_23 - mat[ 1 ][ 2 ] * det2_23_03 + mat[ 1 ][ 3 ] * det2_23_02;
	float det3_123_024 = mat[ 1 ][ 0 ] * det2_23_24 - mat[ 1 ][ 2 ] * det2_23_04 + mat[ 1 ][ 4 ] * det2_23_02;
	float det3_123_034 = mat[ 1 ][ 0 ] * det2_23_34 - mat[ 1 ][ 3 ] * det2_23_04 + mat[ 1 ][ 4 ] * det2_23_03;
	float det3_123_123 = mat[ 1 ][ 1 ] * det2_23_23 - mat[ 1 ][ 2 ] * det2_23_13 + mat[ 1 ][ 3 ] * det2_23_12;
	float det3_123_124 = mat[ 1 ][ 1 ] * det2_23_24 - mat[ 1 ][ 2 ] * det2_23_14 + mat[ 1 ][ 4 ] * det2_23_12;
	float det3_123_134 = mat[ 1 ][ 1 ] * det2_23_34 - mat[ 1 ][ 3 ] * det2_23_14 + mat[ 1 ][ 4 ] * det2_23_13;
	float det3_123_234 = mat[ 1 ][ 2 ] * det2_23_34 - mat[ 1 ][ 3 ] * det2_23_24 + mat[ 1 ][ 4 ] * det2_23_23;
	float det3_124_012 = mat[ 1 ][ 0 ] * det2_24_12 - mat[ 1 ][ 1 ] * det2_24_02 + mat[ 1 ][ 2 ] * det2_24_01;
	float det3_124_013 = mat[ 1 ][ 0 ] * det2_24_13 - mat[ 1 ][ 1 ] * det2_24_03 + mat[ 1 ][ 3 ] * det2_24_01;
	float det3_124_014 = mat[ 1 ][ 0 ] * det2_24_14 - mat[ 1 ][ 1 ] * det2_24_04 + mat[ 1 ][ 4 ] * det2_24_01;
	float det3_124_023 = mat[ 1 ][ 0 ] * det2_24_23 - mat[ 1 ][ 2 ] * det2_24_03 + mat[ 1 ][ 3 ] * det2_24_02;
	float det3_124_024 = mat[ 1 ][ 0 ] * det2_24_24 - mat[ 1 ][ 2 ] * det2_24_04 + mat[ 1 ][ 4 ] * det2_24_02;
	float det3_124_034 = mat[ 1 ][ 0 ] * det2_24_34 - mat[ 1 ][ 3 ] * det2_24_04 + mat[ 1 ][ 4 ] * det2_24_03;
	float det3_124_123 = mat[ 1 ][ 1 ] * det2_24_23 - mat[ 1 ][ 2 ] * det2_24_13 + mat[ 1 ][ 3 ] * det2_24_12;
	float det3_124_124 = mat[ 1 ][ 1 ] * det2_24_24 - mat[ 1 ][ 2 ] * det2_24_14 + mat[ 1 ][ 4 ] * det2_24_12;
	float det3_124_134 = mat[ 1 ][ 1 ] * det2_24_34 - mat[ 1 ][ 3 ] * det2_24_14 + mat[ 1 ][ 4 ] * det2_24_13;
	float det3_124_234 = mat[ 1 ][ 2 ] * det2_24_34 - mat[ 1 ][ 3 ] * det2_24_24 + mat[ 1 ][ 4 ] * det2_24_23;
	float det3_134_012 = mat[ 1 ][ 0 ] * det2_34_12 - mat[ 1 ][ 1 ] * det2_34_02 + mat[ 1 ][ 2 ] * det2_34_01;
	float det3_134_013 = mat[ 1 ][ 0 ] * det2_34_13 - mat[ 1 ][ 1 ] * det2_34_03 + mat[ 1 ][ 3 ] * det2_34_01;
	float det3_134_014 = mat[ 1 ][ 0 ] * det2_34_14 - mat[ 1 ][ 1 ] * det2_34_04 + mat[ 1 ][ 4 ] * det2_34_01;
	float det3_134_023 = mat[ 1 ][ 0 ] * det2_34_23 - mat[ 1 ][ 2 ] * det2_34_03 + mat[ 1 ][ 3 ] * det2_34_02;
	float det3_134_024 = mat[ 1 ][ 0 ] * det2_34_24 - mat[ 1 ][ 2 ] * det2_34_04 + mat[ 1 ][ 4 ] * det2_34_02;
	float det3_134_034 = mat[ 1 ][ 0 ] * det2_34_34 - mat[ 1 ][ 3 ] * det2_34_04 + mat[ 1 ][ 4 ] * det2_34_03;
	float det3_134_123 = mat[ 1 ][ 1 ] * det2_34_23 - mat[ 1 ][ 2 ] * det2_34_13 + mat[ 1 ][ 3 ] * det2_34_12;
	float det3_134_124 = mat[ 1 ][ 1 ] * det2_34_24 - mat[ 1 ][ 2 ] * det2_34_14 + mat[ 1 ][ 4 ] * det2_34_12;
	float det3_134_134 = mat[ 1 ][ 1 ] * det2_34_34 - mat[ 1 ][ 3 ] * det2_34_14 + mat[ 1 ][ 4 ] * det2_34_13;
	float det3_134_234 = mat[ 1 ][ 2 ] * det2_34_34 - mat[ 1 ][ 3 ] * det2_34_24 + mat[ 1 ][ 4 ] * det2_34_23;

	// remaining 4x4 sub-determinants
	float det4_0123_0123 = mat[ 0 ][ 0 ] * det3_123_123 - mat[ 0 ][ 1 ] * det3_123_023 + mat[ 0 ][ 2 ] * det3_123_013 - mat[ 0 ][ 3 ] * det3_123_012;
	float det4_0123_0124 = mat[ 0 ][ 0 ] * det3_123_124 - mat[ 0 ][ 1 ] * det3_123_024 + mat[ 0 ][ 2 ] * det3_123_014 - mat[ 0 ][ 4 ] * det3_123_012;
	float det4_0123_0134 = mat[ 0 ][ 0 ] * det3_123_134 - mat[ 0 ][ 1 ] * det3_123_034 + mat[ 0 ][ 3 ] * det3_123_014 - mat[ 0 ][ 4 ] * det3_123_013;
	float det4_0123_0234 = mat[ 0 ][ 0 ] * det3_123_234 - mat[ 0 ][ 2 ] * det3_123_034 + mat[ 0 ][ 3 ] * det3_123_024 - mat[ 0 ][ 4 ] * det3_123_023;
	float det4_0123_1234 = mat[ 0 ][ 1 ] * det3_123_234 - mat[ 0 ][ 2 ] * det3_123_134 + mat[ 0 ][ 3 ] * det3_123_124 - mat[ 0 ][ 4 ] * det3_123_123;
	float det4_0124_0123 = mat[ 0 ][ 0 ] * det3_124_123 - mat[ 0 ][ 1 ] * det3_124_023 + mat[ 0 ][ 2 ] * det3_124_013 - mat[ 0 ][ 3 ] * det3_124_012;
	float det4_0124_0124 = mat[ 0 ][ 0 ] * det3_124_124 - mat[ 0 ][ 1 ] * det3_124_024 + mat[ 0 ][ 2 ] * det3_124_014 - mat[ 0 ][ 4 ] * det3_124_012;
	float det4_0124_0134 = mat[ 0 ][ 0 ] * det3_124_134 - mat[ 0 ][ 1 ] * det3_124_034 + mat[ 0 ][ 3 ] * det3_124_014 - mat[ 0 ][ 4 ] * det3_124_013;
	float det4_0124_0234 = mat[ 0 ][ 0 ] * det3_124_234 - mat[ 0 ][ 2 ] * det3_124_034 + mat[ 0 ][ 3 ] * det3_124_024 - mat[ 0 ][ 4 ] * det3_124_023;
	float det4_0124_1234 = mat[ 0 ][ 1 ] * det3_124_234 - mat[ 0 ][ 2 ] * det3_124_134 + mat[ 0 ][ 3 ] * det3_124_124 - mat[ 0 ][ 4 ] * det3_124_123;
	float det4_0134_0123 = mat[ 0 ][ 0 ] * det3_134_123 - mat[ 0 ][ 1 ] * det3_134_023 + mat[ 0 ][ 2 ] * det3_134_013 - mat[ 0 ][ 3 ] * det3_134_012;
	float det4_0134_0124 = mat[ 0 ][ 0 ] * det3_134_124 - mat[ 0 ][ 1 ] * det3_134_024 + mat[ 0 ][ 2 ] * det3_134_014 - mat[ 0 ][ 4 ] * det3_134_012;
	float det4_0134_0134 = mat[ 0 ][ 0 ] * det3_134_134 - mat[ 0 ][ 1 ] * det3_134_034 + mat[ 0 ][ 3 ] * det3_134_014 - mat[ 0 ][ 4 ] * det3_134_013;
	float det4_0134_0234 = mat[ 0 ][ 0 ] * det3_134_234 - mat[ 0 ][ 2 ] * det3_134_034 + mat[ 0 ][ 3 ] * det3_134_024 - mat[ 0 ][ 4 ] * det3_134_023;
	float det4_0134_1234 = mat[ 0 ][ 1 ] * det3_134_234 - mat[ 0 ][ 2 ] * det3_134_134 + mat[ 0 ][ 3 ] * det3_134_124 - mat[ 0 ][ 4 ] * det3_134_123;
	float det4_0234_0123 = mat[ 0 ][ 0 ] * det3_234_123 - mat[ 0 ][ 1 ] * det3_234_023 + mat[ 0 ][ 2 ] * det3_234_013 - mat[ 0 ][ 3 ] * det3_234_012;
	float det4_0234_0124 = mat[ 0 ][ 0 ] * det3_234_124 - mat[ 0 ][ 1 ] * det3_234_024 + mat[ 0 ][ 2 ] * det3_234_014 - mat[ 0 ][ 4 ] * det3_234_012;
	float det4_0234_0134 = mat[ 0 ][ 0 ] * det3_234_134 - mat[ 0 ][ 1 ] * det3_234_034 + mat[ 0 ][ 3 ] * det3_234_014 - mat[ 0 ][ 4 ] * det3_234_013;
	float det4_0234_0234 = mat[ 0 ][ 0 ] * det3_234_234 - mat[ 0 ][ 2 ] * det3_234_034 + mat[ 0 ][ 3 ] * det3_234_024 - mat[ 0 ][ 4 ] * det3_234_023;
	float det4_0234_1234 = mat[ 0 ][ 1 ] * det3_234_234 - mat[ 0 ][ 2 ] * det3_234_134 + mat[ 0 ][ 3 ] * det3_234_124 - mat[ 0 ][ 4 ] * det3_234_123;

	mat[ 0 ][ 0 ] =  det4_1234_1234 * invDet;
	mat[ 0 ][ 1 ] = -det4_0234_1234 * invDet;
	mat[ 0 ][ 2 ] =  det4_0134_1234 * invDet;
	mat[ 0 ][ 3 ] = -det4_0124_1234 * invDet;
	mat[ 0 ][ 4 ] =  det4_0123_1234 * invDet;

	mat[ 1 ][ 0 ] = -det4_1234_0234 * invDet;
	mat[ 1 ][ 1 ] =  det4_0234_0234 * invDet;
	mat[ 1 ][ 2 ] = -det4_0134_0234 * invDet;
	mat[ 1 ][ 3 ] =  det4_0124_0234 * invDet;
	mat[ 1 ][ 4 ] = -det4_0123_0234 * invDet;

	mat[ 2 ][ 0 ] =  det4_1234_0134 * invDet;
	mat[ 2 ][ 1 ] = -det4_0234_0134 * invDet;
	mat[ 2 ][ 2 ] =  det4_0134_0134 * invDet;
	mat[ 2 ][ 3 ] = -det4_0124_0134 * invDet;
	mat[ 2 ][ 4 ] =  det4_0123_0134 * invDet;

	mat[ 3 ][ 0 ] = -det4_1234_0124 * invDet;
	mat[ 3 ][ 1 ] =  det4_0234_0124 * invDet;
	mat[ 3 ][ 2 ] = -det4_0134_0124 * invDet;
	mat[ 3 ][ 3 ] =  det4_0124_0124 * invDet;
	mat[ 3 ][ 4 ] = -det4_0123_0124 * invDet;

	mat[ 4 ][ 0 ] =  det4_1234_0123 * invDet;
	mat[ 4 ][ 1 ] = -det4_0234_0123 * invDet;
	mat[ 4 ][ 2 ] =  det4_0134_0123 * invDet;
	mat[ 4 ][ 3 ] = -det4_0124_0123 * invDet;
	mat[ 4 ][ 4 ] =  det4_0123_0123 * invDet;

	return true;
#elif 0
	// 5*28 = 140 multiplications
	//			5 divisions
	float *mat = reinterpret_cast<float *>(this);
	float s;
	float d, di;

	di = mat[ 0 ];
	s = di;
	mat[ 0 ] = d = 1.0f / di;
	mat[ 1 ] *= d;
	mat[ 2 ] *= d;
	mat[ 3 ] *= d;
	mat[ 4 ] *= d;
	d = -d;
	mat[ 5 ] *= d;
	mat[ 10 ] *= d;
	mat[ 15 ] *= d;
	mat[ 20 ] *= d;
	d = mat[ 5 ] * di;
	mat[ 6 ] += mat[ 1 ] * d;
	mat[ 7 ] += mat[ 2 ] * d;
	mat[ 8 ] += mat[ 3 ] * d;
	mat[ 9 ] += mat[ 4 ] * d;
	d = mat[ 10 ] * di;
	mat[ 11 ] += mat[ 1 ] * d;
	mat[ 12 ] += mat[ 2 ] * d;
	mat[ 13 ] += mat[ 3 ] * d;
	mat[ 14 ] += mat[ 4 ] * d;
	d = mat[ 15 ] * di;
	mat[ 16 ] += mat[ 1 ] * d;
	mat[ 17 ] += mat[ 2 ] * d;
	mat[ 18 ] += mat[ 3 ] * d;
	mat[ 19 ] += mat[ 4 ] * d;
	d = mat[ 20 ] * di;
	mat[ 21 ] += mat[ 1 ] * d;
	mat[ 22 ] += mat[ 2 ] * d;
	mat[ 23 ] += mat[ 3 ] * d;
	mat[ 24 ] += mat[ 4 ] * d;
	di = mat[ 6 ];
	s *= di;
	mat[ 6 ] = d = 1.0f / di;
	mat[ 5 ] *= d;
	mat[ 7 ] *= d;
	mat[ 8 ] *= d;
	mat[ 9 ] *= d;
	d = -d;
	mat[ 1 ] *= d;
	mat[ 11 ] *= d;
	mat[ 16 ] *= d;
	mat[ 21 ] *= d;
	d = mat[ 1 ] * di;
	mat[ 0 ] += mat[ 5 ] * d;
	mat[ 2 ] += mat[ 7 ] * d;
	mat[ 3 ] += mat[ 8 ] * d;
	mat[ 4 ] += mat[ 9 ] * d;
	d = mat[ 11 ] * di;
	mat[ 10 ] += mat[ 5 ] * d;
	mat[ 12 ] += mat[ 7 ] * d;
	mat[ 13 ] += mat[ 8 ] * d;
	mat[ 14 ] += mat[ 9 ] * d;
	d = mat[ 16 ] * di;
	mat[ 15 ] += mat[ 5 ] * d;
	mat[ 17 ] += mat[ 7 ] * d;
	mat[ 18 ] += mat[ 8 ] * d;
	mat[ 19 ] += mat[ 9 ] * d;
	d = mat[ 21 ] * di;
	mat[ 20 ] += mat[ 5 ] * d;
	mat[ 22 ] += mat[ 7 ] * d;
	mat[ 23 ] += mat[ 8 ] * d;
	mat[ 24 ] += mat[ 9 ] * d;
	di = mat[ 12 ];
	s *= di;
	mat[ 12 ] = d = 1.0f / di;
	mat[ 10 ] *= d;
	mat[ 11 ] *= d;
	mat[ 13 ] *= d;
	mat[ 14 ] *= d;
	d = -d;
	mat[ 2 ] *= d;
	mat[ 7 ] *= d;
	mat[ 17 ] *= d;
	mat[ 22 ] *= d;
	d = mat[ 2 ] * di;
	mat[ 0 ] += mat[ 10 ] * d;
	mat[ 1 ] += mat[ 11 ] * d;
	mat[ 3 ] += mat[ 13 ] * d;
	mat[ 4 ] += mat[ 14 ] * d;
	d = mat[ 7 ] * di;
	mat[ 5 ] += mat[ 10 ] * d;
	mat[ 6 ] += mat[ 11 ] * d;
	mat[ 8 ] += mat[ 13 ] * d;
	mat[ 9 ] += mat[ 14 ] * d;
	d = mat[ 17 ] * di;
	mat[ 15 ] += mat[ 10 ] * d;
	mat[ 16 ] += mat[ 11 ] * d;
	mat[ 18 ] += mat[ 13 ] * d;
	mat[ 19 ] += mat[ 14 ] * d;
	d = mat[ 22 ] * di;
	mat[ 20 ] += mat[ 10 ] * d;
	mat[ 21 ] += mat[ 11 ] * d;
	mat[ 23 ] += mat[ 13 ] * d;
	mat[ 24 ] += mat[ 14 ] * d;
	di = mat[ 18 ];
	s *= di;
	mat[ 18 ] = d = 1.0f / di;
	mat[ 15 ] *= d;
	mat[ 16 ] *= d;
	mat[ 17 ] *= d;
	mat[ 19 ] *= d;
	d = -d;
	mat[ 3 ] *= d;
	mat[ 8 ] *= d;
	mat[ 13 ] *= d;
	mat[ 23 ] *= d;
	d = mat[ 3 ] * di;
	mat[ 0 ] += mat[ 15 ] * d;
	mat[ 1 ] += mat[ 16 ] * d;
	mat[ 2 ] += mat[ 17 ] * d;
	mat[ 4 ] += mat[ 19 ] * d;
	d = mat[ 8 ] * di;
	mat[ 5 ] += mat[ 15 ] * d;
	mat[ 6 ] += mat[ 16 ] * d;
	mat[ 7 ] += mat[ 17 ] * d;
	mat[ 9 ] += mat[ 19 ] * d;
	d = mat[ 13 ] * di;
	mat[ 10 ] += mat[ 15 ] * d;
	mat[ 11 ] += mat[ 16 ] * d;
	mat[ 12 ] += mat[ 17 ] * d;
	mat[ 14 ] += mat[ 19 ] * d;
	d = mat[ 23 ] * di;
	mat[ 20 ] += mat[ 15 ] * d;
	mat[ 21 ] += mat[ 16 ] * d;
	mat[ 22 ] += mat[ 17 ] * d;
	mat[ 24 ] += mat[ 19 ] * d;
	di = mat[ 24 ];
	s *= di;
	mat[ 24 ] = d = 1.0f / di;
	mat[ 20 ] *= d;
	mat[ 21 ] *= d;
	mat[ 22 ] *= d;
	mat[ 23 ] *= d;
	d = -d;
	mat[ 4 ] *= d;
	mat[ 9 ] *= d;
	mat[ 14 ] *= d;
	mat[ 19 ] *= d;
	d = mat[ 4 ] * di;
	mat[ 0 ] += mat[ 20 ] * d;
	mat[ 1 ] += mat[ 21 ] * d;
	mat[ 2 ] += mat[ 22 ] * d;
	mat[ 3 ] += mat[ 23 ] * d;
	d = mat[ 9 ] * di;
	mat[ 5 ] += mat[ 20 ] * d;
	mat[ 6 ] += mat[ 21 ] * d;
	mat[ 7 ] += mat[ 22 ] * d;
	mat[ 8 ] += mat[ 23 ] * d;
	d = mat[ 14 ] * di;
	mat[ 10 ] += mat[ 20 ] * d;
	mat[ 11 ] += mat[ 21 ] * d;
	mat[ 12 ] += mat[ 22 ] * d;
	mat[ 13 ] += mat[ 23 ] * d;
	d = mat[ 19 ] * di;
	mat[ 15 ] += mat[ 20 ] * d;
	mat[ 16 ] += mat[ 21 ] * d;
	mat[ 17 ] += mat[ 22 ] * d;
	mat[ 18 ] += mat[ 23 ] * d;

	return ( s != 0.0f && !FLOAT_IS_NAN( s ) );
#else
	// 86+30+6 = 122 multiplications
	//	  2*1  =   2 divisions
	Mat3 r0, r1, r2, r3;
	float c0, c1, c2, det, invDet;
	float *mat = reinterpret_cast<float *>(this);

	// r0 = m0.Inverse( );	// 3x3
	c0 = mat[1*5+1] * mat[2*5+2] - mat[1*5+2] * mat[2*5+1];
	c1 = mat[1*5+2] * mat[2*5+0] - mat[1*5+0] * mat[2*5+2];
	c2 = mat[1*5+0] * mat[2*5+1] - mat[1*5+1] * mat[2*5+0];

	det = mat[0*5+0] * c0 + mat[0*5+1] * c1 + mat[0*5+2] * c2;

	if( fabs( det ) < MATRIX_INVERSE_EPSILON ) {
		return false;
	}

	invDet = 1.0f / det;

	r0[ 0 ][ 0 ] = c0 * invDet;
	r0[ 0 ][ 1 ] = ( mat[0*5+2] * mat[2*5+1] - mat[0*5+1] * mat[2*5+2] ) * invDet;
	r0[ 0 ][ 2 ] = ( mat[0*5+1] * mat[1*5+2] - mat[0*5+2] * mat[1*5+1] ) * invDet;
	r0[ 1 ][ 0 ] = c1 * invDet;
	r0[ 1 ][ 1 ] = ( mat[0*5+0] * mat[2*5+2] - mat[0*5+2] * mat[2*5+0] ) * invDet;
	r0[ 1 ][ 2 ] = ( mat[0*5+2] * mat[1*5+0] - mat[0*5+0] * mat[1*5+2] ) * invDet;
	r0[ 2 ][ 0 ] = c2 * invDet;
	r0[ 2 ][ 1 ] = ( mat[0*5+1] * mat[2*5+0] - mat[0*5+0] * mat[2*5+1] ) * invDet;
	r0[ 2 ][ 2 ] = ( mat[0*5+0] * mat[1*5+1] - mat[0*5+1] * mat[1*5+0] ) * invDet;

	// r1 = r0 * m1;		// 3x2 = 3x3 * 3x2
	r1[ 0 ][ 0 ] = r0[ 0 ][ 0 ] * mat[0*5+3] + r0[ 0 ][ 1 ] * mat[1*5+3] + r0[ 0 ][ 2 ] * mat[2*5+3];
	r1[ 0 ][ 1 ] = r0[ 0 ][ 0 ] * mat[0*5+4] + r0[ 0 ][ 1 ] * mat[1*5+4] + r0[ 0 ][ 2 ] * mat[2*5+4];
	r1[ 1 ][ 0 ] = r0[ 1 ][ 0 ] * mat[0*5+3] + r0[ 1 ][ 1 ] * mat[1*5+3] + r0[ 1 ][ 2 ] * mat[2*5+3];
	r1[ 1 ][ 1 ] = r0[ 1 ][ 0 ] * mat[0*5+4] + r0[ 1 ][ 1 ] * mat[1*5+4] + r0[ 1 ][ 2 ] * mat[2*5+4];
	r1[ 2 ][ 0 ] = r0[ 2 ][ 0 ] * mat[0*5+3] + r0[ 2 ][ 1 ] * mat[1*5+3] + r0[ 2 ][ 2 ] * mat[2*5+3];
	r1[ 2 ][ 1 ] = r0[ 2 ][ 0 ] * mat[0*5+4] + r0[ 2 ][ 1 ] * mat[1*5+4] + r0[ 2 ][ 2 ] * mat[2*5+4];

	// r2 = m2 * r1;		// 2x2 = 2x3 * 3x2
	r2[ 0 ][ 0 ] = mat[3*5+0] * r1[ 0 ][ 0 ] + mat[3*5+1] * r1[ 1 ][ 0 ] + mat[3*5+2] * r1[ 2 ][ 0 ];
	r2[ 0 ][ 1 ] = mat[3*5+0] * r1[ 0 ][ 1 ] + mat[3*5+1] * r1[ 1 ][ 1 ] + mat[3*5+2] * r1[ 2 ][ 1 ];
	r2[ 1 ][ 0 ] = mat[4*5+0] * r1[ 0 ][ 0 ] + mat[4*5+1] * r1[ 1 ][ 0 ] + mat[4*5+2] * r1[ 2 ][ 0 ];
	r2[ 1 ][ 1 ] = mat[4*5+0] * r1[ 0 ][ 1 ] + mat[4*5+1] * r1[ 1 ][ 1 ] + mat[4*5+2] * r1[ 2 ][ 1 ];

	// r3 = r2 - m3;		// 2x2 = 2x2 - 2x2
	r3[ 0 ][ 0 ] = r2[ 0 ][ 0 ] - mat[3*5+3];
	r3[ 0 ][ 1 ] = r2[ 0 ][ 1 ] - mat[3*5+4];
	r3[ 1 ][ 0 ] = r2[ 1 ][ 0 ] - mat[4*5+3];
	r3[ 1 ][ 1 ] = r2[ 1 ][ 1 ] - mat[4*5+4];

	// r3.InverseSelf( );	// 2x2
	det = r3[ 0 ][ 0 ] * r3[ 1 ][ 1 ] - r3[ 0 ][ 1 ] * r3[ 1 ][ 0 ];

	if( fabs( det ) < MATRIX_INVERSE_EPSILON ) {
		return false;
	}

	invDet = 1.0f / det;

	c0 = r3[ 0 ][ 0 ];
	r3[ 0 ][ 0 ] =   r3[ 1 ][ 1 ] * invDet;
	r3[ 0 ][ 1 ] = - r3[ 0 ][ 1 ] * invDet;
	r3[ 1 ][ 0 ] = - r3[ 1 ][ 0 ] * invDet;
	r3[ 1 ][ 1 ] =   c0 * invDet;

	// r2 = m2 * r0;		// 2x3 = 2x3 * 3x3
	r2[ 0 ][ 0 ] = mat[3*5+0] * r0[ 0 ][ 0 ] + mat[3*5+1] * r0[ 1 ][ 0 ] + mat[3*5+2] * r0[ 2 ][ 0 ];
	r2[ 0 ][ 1 ] = mat[3*5+0] * r0[ 0 ][ 1 ] + mat[3*5+1] * r0[ 1 ][ 1 ] + mat[3*5+2] * r0[ 2 ][ 1 ];
	r2[ 0 ][ 2 ] = mat[3*5+0] * r0[ 0 ][ 2 ] + mat[3*5+1] * r0[ 1 ][ 2 ] + mat[3*5+2] * r0[ 2 ][ 2 ];
	r2[ 1 ][ 0 ] = mat[4*5+0] * r0[ 0 ][ 0 ] + mat[4*5+1] * r0[ 1 ][ 0 ] + mat[4*5+2] * r0[ 2 ][ 0 ];
	r2[ 1 ][ 1 ] = mat[4*5+0] * r0[ 0 ][ 1 ] + mat[4*5+1] * r0[ 1 ][ 1 ] + mat[4*5+2] * r0[ 2 ][ 1 ];
	r2[ 1 ][ 2 ] = mat[4*5+0] * r0[ 0 ][ 2 ] + mat[4*5+1] * r0[ 1 ][ 2 ] + mat[4*5+2] * r0[ 2 ][ 2 ];

	// m2 = r3 * r2;		// 2x3 = 2x2 * 2x3
	mat[3*5+0] = r3[ 0 ][ 0 ] * r2[ 0 ][ 0 ] + r3[ 0 ][ 1 ] * r2[ 1 ][ 0 ];
	mat[3*5+1] = r3[ 0 ][ 0 ] * r2[ 0 ][ 1 ] + r3[ 0 ][ 1 ] * r2[ 1 ][ 1 ];
	mat[3*5+2] = r3[ 0 ][ 0 ] * r2[ 0 ][ 2 ] + r3[ 0 ][ 1 ] * r2[ 1 ][ 2 ];
	mat[4*5+0] = r3[ 1 ][ 0 ] * r2[ 0 ][ 0 ] + r3[ 1 ][ 1 ] * r2[ 1 ][ 0 ];
	mat[4*5+1] = r3[ 1 ][ 0 ] * r2[ 0 ][ 1 ] + r3[ 1 ][ 1 ] * r2[ 1 ][ 1 ];
	mat[4*5+2] = r3[ 1 ][ 0 ] * r2[ 0 ][ 2 ] + r3[ 1 ][ 1 ] * r2[ 1 ][ 2 ];

	// m0 = r0 - r1 * m2;	// 3x3 = 3x3 - 3x2 * 2x3
	mat[0*5+0] = r0[ 0 ][ 0 ] - r1[ 0 ][ 0 ] * mat[3*5+0] - r1[ 0 ][ 1 ] * mat[4*5+0];
	mat[0*5+1] = r0[ 0 ][ 1 ] - r1[ 0 ][ 0 ] * mat[3*5+1] - r1[ 0 ][ 1 ] * mat[4*5+1];
	mat[0*5+2] = r0[ 0 ][ 2 ] - r1[ 0 ][ 0 ] * mat[3*5+2] - r1[ 0 ][ 1 ] * mat[4*5+2];
	mat[1*5+0] = r0[ 1 ][ 0 ] - r1[ 1 ][ 0 ] * mat[3*5+0] - r1[ 1 ][ 1 ] * mat[4*5+0];
	mat[1*5+1] = r0[ 1 ][ 1 ] - r1[ 1 ][ 0 ] * mat[3*5+1] - r1[ 1 ][ 1 ] * mat[4*5+1];
	mat[1*5+2] = r0[ 1 ][ 2 ] - r1[ 1 ][ 0 ] * mat[3*5+2] - r1[ 1 ][ 1 ] * mat[4*5+2];
	mat[2*5+0] = r0[ 2 ][ 0 ] - r1[ 2 ][ 0 ] * mat[3*5+0] - r1[ 2 ][ 1 ] * mat[4*5+0];
	mat[2*5+1] = r0[ 2 ][ 1 ] - r1[ 2 ][ 0 ] * mat[3*5+1] - r1[ 2 ][ 1 ] * mat[4*5+1];
	mat[2*5+2] = r0[ 2 ][ 2 ] - r1[ 2 ][ 0 ] * mat[3*5+2] - r1[ 2 ][ 1 ] * mat[4*5+2];

	// m1 = r1 * r3;		// 3x2 = 3x2 * 2x2
	mat[0*5+3] = r1[ 0 ][ 0 ] * r3[ 0 ][ 0 ] + r1[ 0 ][ 1 ] * r3[ 1 ][ 0 ];
	mat[0*5+4] = r1[ 0 ][ 0 ] * r3[ 0 ][ 1 ] + r1[ 0 ][ 1 ] * r3[ 1 ][ 1 ];
	mat[1*5+3] = r1[ 1 ][ 0 ] * r3[ 0 ][ 0 ] + r1[ 1 ][ 1 ] * r3[ 1 ][ 0 ];
	mat[1*5+4] = r1[ 1 ][ 0 ] * r3[ 0 ][ 1 ] + r1[ 1 ][ 1 ] * r3[ 1 ][ 1 ];
	mat[2*5+3] = r1[ 2 ][ 0 ] * r3[ 0 ][ 0 ] + r1[ 2 ][ 1 ] * r3[ 1 ][ 0 ];
	mat[2*5+4] = r1[ 2 ][ 0 ] * r3[ 0 ][ 1 ] + r1[ 2 ][ 1 ] * r3[ 1 ][ 1 ];

	// m3 = -r3;			// 2x2 = - 2x2
	mat[3*5+3] = -r3[ 0 ][ 0 ];
	mat[3*5+4] = -r3[ 0 ][ 1 ];
	mat[4*5+3] = -r3[ 1 ][ 0 ];
	mat[4*5+4] = -r3[ 1 ][ 1 ];

	return true;
#endif
}

//===============================================================
//
//	Mat6 - 6x6 matrix
//
//===============================================================

class Mat6 {
public:
					Mat6( );
					explicit Mat6( const Vec6 & v0, const Vec6 & v1, const Vec6 & v2, const Vec6 & v3, const Vec6 & v4, const Vec6 & v5 );
					explicit Mat6( const Mat3 & m0, const Mat3 & m1, const Mat3 & m2, const Mat3 & m3 );
					explicit Mat6( const float src[ 6 ][ 6 ] );

	const Vec6 &	operator [ ]( int index ) const;
	Vec6 &		operator [ ]( int index );
	Mat6			operator *( const float a ) const;
	Vec6			operator *( const Vec6 & vec ) const;
	Mat6			operator *( const Mat6 & a ) const;
	Mat6			operator +( const Mat6 & a ) const;
	Mat6			operator -( const Mat6 & a ) const;
	Mat6 &		operator *=( const float a );
	Mat6 &		operator *=( const Mat6 & a );
	Mat6 &		operator +=( const Mat6 & a );
	Mat6 &		operator -=( const Mat6 & a );

	friend Mat6	operator *( const float a, const Mat6 & mat );
	friend Vec6	operator *( const Vec6 & vec, const Mat6 & mat );
	friend Vec6 &	operator *=( Vec6 & vec, const Mat6 & mat );

	bool			Compare( const Mat6 & a ) const;						// exact compare, no epsilon
	bool			Compare( const Mat6 & a, const float epsilon ) const;	// compare with epsilon
	bool			operator ==( const Mat6 & a ) const;					// exact compare, no epsilon
	bool			operator !=( const Mat6 & a ) const;					// exact compare, no epsilon

	void			Zero( );
	void			Identity( );
	bool			IsIdentity( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetric( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsDiagonal( const float epsilon = MATRIX_EPSILON ) const;

	Mat3			SubMat3( int n ) const;
	float			Trace( ) const;
	float			Determinant( ) const;
	Mat6			Transpose( ) const;	// returns transpose
	Mat6 &		TransposeSelf( );
	Mat6			Inverse( ) const;		// returns the inverse ( m * m.Inverse( ) = identity )
	bool			InverseSelf( );		// returns false if determinant is zero
	Mat6			InverseFast( ) const;	// returns the inverse ( m * m.Inverse( ) = identity )
	bool			InverseFastSelf( );	// returns false if determinant is zero

	int				GetDimension( ) const;

	const float *	ToFloatPtr( ) const;
	float *			ToFloatPtr( );
	const char *	ToString( int precision = 2 ) const;

private:
	Vec6			mat[ 6 ];
};

extern Mat6 mat6_zero;
extern Mat6 mat6_identity;
#define mat6_default	mat6_identity

INLINE Mat6::Mat6( ) {
}

INLINE Mat6::Mat6( const Mat3 & m0, const Mat3 & m1, const Mat3 & m2, const Mat3 & m3 ) {
	mat[ 0 ] = Vec6( m0[ 0 ][ 0 ], m0[ 0 ][ 1 ], m0[ 0 ][ 2 ], m1[ 0 ][ 0 ], m1[ 0 ][ 1 ], m1[ 0 ][ 2 ] );
	mat[ 1 ] = Vec6( m0[ 1 ][ 0 ], m0[ 1 ][ 1 ], m0[ 1 ][ 2 ], m1[ 1 ][ 0 ], m1[ 1 ][ 1 ], m1[ 1 ][ 2 ] );
	mat[ 2 ] = Vec6( m0[ 2 ][ 0 ], m0[ 2 ][ 1 ], m0[ 2 ][ 2 ], m1[ 2 ][ 0 ], m1[ 2 ][ 1 ], m1[ 2 ][ 2 ] );
	mat[ 3 ] = Vec6( m2[ 0 ][ 0 ], m2[ 0 ][ 1 ], m2[ 0 ][ 2 ], m3[ 0 ][ 0 ], m3[ 0 ][ 1 ], m3[ 0 ][ 2 ] );
	mat[ 4 ] = Vec6( m2[ 1 ][ 0 ], m2[ 1 ][ 1 ], m2[ 1 ][ 2 ], m3[ 1 ][ 0 ], m3[ 1 ][ 1 ], m3[ 1 ][ 2 ] );
	mat[ 5 ] = Vec6( m2[ 2 ][ 0 ], m2[ 2 ][ 1 ], m2[ 2 ][ 2 ], m3[ 2 ][ 0 ], m3[ 2 ][ 1 ], m3[ 2 ][ 2 ] );
}

INLINE Mat6::Mat6( const Vec6 & v0, const Vec6 & v1, const Vec6 & v2, const Vec6 & v3, const Vec6 & v4, const Vec6 & v5 ) {
	mat[ 0 ] = v0;
	mat[ 1 ] = v1;
	mat[ 2 ] = v2;
	mat[ 3 ] = v3;
	mat[ 4 ] = v4;
	mat[ 5 ] = v5;
}

INLINE Mat6::Mat6( const float src[ 6 ][ 6 ] ) {
	memcpy( mat, src, 6 * 6 * sizeof( float ) );
}

INLINE const Vec6 & Mat6::operator [ ]( int index ) const {
	assert( ( index >= 0 ) && ( index < 6 ) );
	return mat[ index ];
}

INLINE Vec6 & Mat6::operator [ ]( int index ) {
	assert( ( index >= 0 ) && ( index < 6 ) );
	return mat[ index ];
}

INLINE Mat6 Mat6::operator *( const Mat6 & a ) const {
	int i, j;
	const float *m1Ptr, *m2Ptr;
	float *dstPtr;
	Mat6 dst;

	m1Ptr = reinterpret_cast<const float *>(this);
	m2Ptr = reinterpret_cast<const float *>(&a);
	dstPtr = reinterpret_cast<float *>(&dst);

	for( i = 0; i < 6; i++ ) {
		for( j = 0; j < 6; j++ ) {
			*dstPtr = m1Ptr[ 0 ] * m2Ptr[ 0 * 6 + j ]
					+ m1Ptr[ 1 ] * m2Ptr[ 1 * 6 + j ]
					+ m1Ptr[ 2 ] * m2Ptr[ 2 * 6 + j ]
					+ m1Ptr[ 3 ] * m2Ptr[ 3 * 6 + j ]
					+ m1Ptr[ 4 ] * m2Ptr[ 4 * 6 + j ]
					+ m1Ptr[ 5 ] * m2Ptr[ 5 * 6 + j ];
			dstPtr++;
		}
		m1Ptr += 6;
	}
	return dst;
}

INLINE Mat6 Mat6::operator *( const float a ) const {
	return Mat6(
		Vec6( mat[ 0 ][ 0 ] * a, mat[ 0 ][ 1 ] * a, mat[ 0 ][ 2 ] * a, mat[ 0 ][ 3 ] * a, mat[ 0 ][ 4 ] * a, mat[ 0 ][ 5 ] * a ),
		Vec6( mat[ 1 ][ 0 ] * a, mat[ 1 ][ 1 ] * a, mat[ 1 ][ 2 ] * a, mat[ 1 ][ 3 ] * a, mat[ 1 ][ 4 ] * a, mat[ 1 ][ 5 ] * a ),
		Vec6( mat[ 2 ][ 0 ] * a, mat[ 2 ][ 1 ] * a, mat[ 2 ][ 2 ] * a, mat[ 2 ][ 3 ] * a, mat[ 2 ][ 4 ] * a, mat[ 2 ][ 5 ] * a ),
		Vec6( mat[ 3 ][ 0 ] * a, mat[ 3 ][ 1 ] * a, mat[ 3 ][ 2 ] * a, mat[ 3 ][ 3 ] * a, mat[ 3 ][ 4 ] * a, mat[ 3 ][ 5 ] * a ),
		Vec6( mat[ 4 ][ 0 ] * a, mat[ 4 ][ 1 ] * a, mat[ 4 ][ 2 ] * a, mat[ 4 ][ 3 ] * a, mat[ 4 ][ 4 ] * a, mat[ 4 ][ 5 ] * a ),
		Vec6( mat[ 5 ][ 0 ] * a, mat[ 5 ][ 1 ] * a, mat[ 5 ][ 2 ] * a, mat[ 5 ][ 3 ] * a, mat[ 5 ][ 4 ] * a, mat[ 5 ][ 5 ] * a ) );
}

INLINE Vec6 Mat6::operator *( const Vec6 & vec ) const {
	return Vec6(
		mat[ 0 ][ 0 ] * vec[ 0 ] + mat[ 0 ][ 1 ] * vec[ 1 ] + mat[ 0 ][ 2 ] * vec[ 2 ] + mat[ 0 ][ 3 ] * vec[ 3 ] + mat[ 0 ][ 4 ] * vec[ 4 ] + mat[ 0 ][ 5 ] * vec[ 5 ],
		mat[ 1 ][ 0 ] * vec[ 0 ] + mat[ 1 ][ 1 ] * vec[ 1 ] + mat[ 1 ][ 2 ] * vec[ 2 ] + mat[ 1 ][ 3 ] * vec[ 3 ] + mat[ 1 ][ 4 ] * vec[ 4 ] + mat[ 1 ][ 5 ] * vec[ 5 ],
		mat[ 2 ][ 0 ] * vec[ 0 ] + mat[ 2 ][ 1 ] * vec[ 1 ] + mat[ 2 ][ 2 ] * vec[ 2 ] + mat[ 2 ][ 3 ] * vec[ 3 ] + mat[ 2 ][ 4 ] * vec[ 4 ] + mat[ 2 ][ 5 ] * vec[ 5 ],
		mat[ 3 ][ 0 ] * vec[ 0 ] + mat[ 3 ][ 1 ] * vec[ 1 ] + mat[ 3 ][ 2 ] * vec[ 2 ] + mat[ 3 ][ 3 ] * vec[ 3 ] + mat[ 3 ][ 4 ] * vec[ 4 ] + mat[ 3 ][ 5 ] * vec[ 5 ],
		mat[ 4 ][ 0 ] * vec[ 0 ] + mat[ 4 ][ 1 ] * vec[ 1 ] + mat[ 4 ][ 2 ] * vec[ 2 ] + mat[ 4 ][ 3 ] * vec[ 3 ] + mat[ 4 ][ 4 ] * vec[ 4 ] + mat[ 4 ][ 5 ] * vec[ 5 ],
		mat[ 5 ][ 0 ] * vec[ 0 ] + mat[ 5 ][ 1 ] * vec[ 1 ] + mat[ 5 ][ 2 ] * vec[ 2 ] + mat[ 5 ][ 3 ] * vec[ 3 ] + mat[ 5 ][ 4 ] * vec[ 4 ] + mat[ 5 ][ 5 ] * vec[ 5 ] );
}

INLINE Mat6 Mat6::operator +( const Mat6 & a ) const {
	return Mat6(
		Vec6( mat[ 0 ][ 0 ] + a[ 0 ][ 0 ], mat[ 0 ][ 1 ] + a[ 0 ][ 1 ], mat[ 0 ][ 2 ] + a[ 0 ][ 2 ], mat[ 0 ][ 3 ] + a[ 0 ][ 3 ], mat[ 0 ][ 4 ] + a[ 0 ][ 4 ], mat[ 0 ][ 5 ] + a[ 0 ][ 5 ] ),
		Vec6( mat[ 1 ][ 0 ] + a[ 1 ][ 0 ], mat[ 1 ][ 1 ] + a[ 1 ][ 1 ], mat[ 1 ][ 2 ] + a[ 1 ][ 2 ], mat[ 1 ][ 3 ] + a[ 1 ][ 3 ], mat[ 1 ][ 4 ] + a[ 1 ][ 4 ], mat[ 1 ][ 5 ] + a[ 1 ][ 5 ] ),
		Vec6( mat[ 2 ][ 0 ] + a[ 2 ][ 0 ], mat[ 2 ][ 1 ] + a[ 2 ][ 1 ], mat[ 2 ][ 2 ] + a[ 2 ][ 2 ], mat[ 2 ][ 3 ] + a[ 2 ][ 3 ], mat[ 2 ][ 4 ] + a[ 2 ][ 4 ], mat[ 2 ][ 5 ] + a[ 2 ][ 5 ] ),
		Vec6( mat[ 3 ][ 0 ] + a[ 3 ][ 0 ], mat[ 3 ][ 1 ] + a[ 3 ][ 1 ], mat[ 3 ][ 2 ] + a[ 3 ][ 2 ], mat[ 3 ][ 3 ] + a[ 3 ][ 3 ], mat[ 3 ][ 4 ] + a[ 3 ][ 4 ], mat[ 3 ][ 5 ] + a[ 3 ][ 5 ] ),
		Vec6( mat[ 4 ][ 0 ] + a[ 4 ][ 0 ], mat[ 4 ][ 1 ] + a[ 4 ][ 1 ], mat[ 4 ][ 2 ] + a[ 4 ][ 2 ], mat[ 4 ][ 3 ] + a[ 4 ][ 3 ], mat[ 4 ][ 4 ] + a[ 4 ][ 4 ], mat[ 4 ][ 5 ] + a[ 4 ][ 5 ] ),
		Vec6( mat[ 5 ][ 0 ] + a[ 5 ][ 0 ], mat[ 5 ][ 1 ] + a[ 5 ][ 1 ], mat[ 5 ][ 2 ] + a[ 5 ][ 2 ], mat[ 5 ][ 3 ] + a[ 5 ][ 3 ], mat[ 5 ][ 4 ] + a[ 5 ][ 4 ], mat[ 5 ][ 5 ] + a[ 5 ][ 5 ] ) );
}

INLINE Mat6 Mat6::operator -( const Mat6 & a ) const {
	return Mat6(
		Vec6( mat[ 0 ][ 0 ] - a[ 0 ][ 0 ], mat[ 0 ][ 1 ] - a[ 0 ][ 1 ], mat[ 0 ][ 2 ] - a[ 0 ][ 2 ], mat[ 0 ][ 3 ] - a[ 0 ][ 3 ], mat[ 0 ][ 4 ] - a[ 0 ][ 4 ], mat[ 0 ][ 5 ] - a[ 0 ][ 5 ] ),
		Vec6( mat[ 1 ][ 0 ] - a[ 1 ][ 0 ], mat[ 1 ][ 1 ] - a[ 1 ][ 1 ], mat[ 1 ][ 2 ] - a[ 1 ][ 2 ], mat[ 1 ][ 3 ] - a[ 1 ][ 3 ], mat[ 1 ][ 4 ] - a[ 1 ][ 4 ], mat[ 1 ][ 5 ] - a[ 1 ][ 5 ] ),
		Vec6( mat[ 2 ][ 0 ] - a[ 2 ][ 0 ], mat[ 2 ][ 1 ] - a[ 2 ][ 1 ], mat[ 2 ][ 2 ] - a[ 2 ][ 2 ], mat[ 2 ][ 3 ] - a[ 2 ][ 3 ], mat[ 2 ][ 4 ] - a[ 2 ][ 4 ], mat[ 2 ][ 5 ] - a[ 2 ][ 5 ] ),
		Vec6( mat[ 3 ][ 0 ] - a[ 3 ][ 0 ], mat[ 3 ][ 1 ] - a[ 3 ][ 1 ], mat[ 3 ][ 2 ] - a[ 3 ][ 2 ], mat[ 3 ][ 3 ] - a[ 3 ][ 3 ], mat[ 3 ][ 4 ] - a[ 3 ][ 4 ], mat[ 3 ][ 5 ] - a[ 3 ][ 5 ] ),
		Vec6( mat[ 4 ][ 0 ] - a[ 4 ][ 0 ], mat[ 4 ][ 1 ] - a[ 4 ][ 1 ], mat[ 4 ][ 2 ] - a[ 4 ][ 2 ], mat[ 4 ][ 3 ] - a[ 4 ][ 3 ], mat[ 4 ][ 4 ] - a[ 4 ][ 4 ], mat[ 4 ][ 5 ] - a[ 4 ][ 5 ] ),
		Vec6( mat[ 5 ][ 0 ] - a[ 5 ][ 0 ], mat[ 5 ][ 1 ] - a[ 5 ][ 1 ], mat[ 5 ][ 2 ] - a[ 5 ][ 2 ], mat[ 5 ][ 3 ] - a[ 5 ][ 3 ], mat[ 5 ][ 4 ] - a[ 5 ][ 4 ], mat[ 5 ][ 5 ] - a[ 5 ][ 5 ] ) );
}

INLINE Mat6 & Mat6::operator *=( const float a ) {
	mat[ 0 ][ 0 ] *= a; mat[ 0 ][ 1 ] *= a; mat[ 0 ][ 2 ] *= a; mat[ 0 ][ 3 ] *= a; mat[ 0 ][ 4 ] *= a; mat[ 0 ][ 5 ] *= a;
	mat[ 1 ][ 0 ] *= a; mat[ 1 ][ 1 ] *= a; mat[ 1 ][ 2 ] *= a; mat[ 1 ][ 3 ] *= a; mat[ 1 ][ 4 ] *= a; mat[ 1 ][ 5 ] *= a;
	mat[ 2 ][ 0 ] *= a; mat[ 2 ][ 1 ] *= a; mat[ 2 ][ 2 ] *= a; mat[ 2 ][ 3 ] *= a; mat[ 2 ][ 4 ] *= a; mat[ 2 ][ 5 ] *= a;
	mat[ 3 ][ 0 ] *= a; mat[ 3 ][ 1 ] *= a; mat[ 3 ][ 2 ] *= a; mat[ 3 ][ 3 ] *= a; mat[ 3 ][ 4 ] *= a; mat[ 3 ][ 5 ] *= a;
	mat[ 4 ][ 0 ] *= a; mat[ 4 ][ 1 ] *= a; mat[ 4 ][ 2 ] *= a; mat[ 4 ][ 3 ] *= a; mat[ 4 ][ 4 ] *= a; mat[ 4 ][ 5 ] *= a;
	mat[ 5 ][ 0 ] *= a; mat[ 5 ][ 1 ] *= a; mat[ 5 ][ 2 ] *= a; mat[ 5 ][ 3 ] *= a; mat[ 5 ][ 4 ] *= a; mat[ 5 ][ 5 ] *= a;
	return *this;
}

INLINE Mat6 & Mat6::operator *=( const Mat6 & a ) {
	*this = *this * a;
	return *this;
}

INLINE Mat6 & Mat6::operator +=( const Mat6 & a ) {
	mat[ 0 ][ 0 ] += a[ 0 ][ 0 ]; mat[ 0 ][ 1 ] += a[ 0 ][ 1 ]; mat[ 0 ][ 2 ] += a[ 0 ][ 2 ]; mat[ 0 ][ 3 ] += a[ 0 ][ 3 ]; mat[ 0 ][ 4 ] += a[ 0 ][ 4 ]; mat[ 0 ][ 5 ] += a[ 0 ][ 5 ];
	mat[ 1 ][ 0 ] += a[ 1 ][ 0 ]; mat[ 1 ][ 1 ] += a[ 1 ][ 1 ]; mat[ 1 ][ 2 ] += a[ 1 ][ 2 ]; mat[ 1 ][ 3 ] += a[ 1 ][ 3 ]; mat[ 1 ][ 4 ] += a[ 1 ][ 4 ]; mat[ 1 ][ 5 ] += a[ 1 ][ 5 ];
	mat[ 2 ][ 0 ] += a[ 2 ][ 0 ]; mat[ 2 ][ 1 ] += a[ 2 ][ 1 ]; mat[ 2 ][ 2 ] += a[ 2 ][ 2 ]; mat[ 2 ][ 3 ] += a[ 2 ][ 3 ]; mat[ 2 ][ 4 ] += a[ 2 ][ 4 ]; mat[ 2 ][ 5 ] += a[ 2 ][ 5 ];
	mat[ 3 ][ 0 ] += a[ 3 ][ 0 ]; mat[ 3 ][ 1 ] += a[ 3 ][ 1 ]; mat[ 3 ][ 2 ] += a[ 3 ][ 2 ]; mat[ 3 ][ 3 ] += a[ 3 ][ 3 ]; mat[ 3 ][ 4 ] += a[ 3 ][ 4 ]; mat[ 3 ][ 5 ] += a[ 3 ][ 5 ];
	mat[ 4 ][ 0 ] += a[ 4 ][ 0 ]; mat[ 4 ][ 1 ] += a[ 4 ][ 1 ]; mat[ 4 ][ 2 ] += a[ 4 ][ 2 ]; mat[ 4 ][ 3 ] += a[ 4 ][ 3 ]; mat[ 4 ][ 4 ] += a[ 4 ][ 4 ]; mat[ 4 ][ 5 ] += a[ 4 ][ 5 ];
	mat[ 5 ][ 0 ] += a[ 5 ][ 0 ]; mat[ 5 ][ 1 ] += a[ 5 ][ 1 ]; mat[ 5 ][ 2 ] += a[ 5 ][ 2 ]; mat[ 5 ][ 3 ] += a[ 5 ][ 3 ]; mat[ 5 ][ 4 ] += a[ 5 ][ 4 ]; mat[ 5 ][ 5 ] += a[ 5 ][ 5 ];
	return *this;
}

INLINE Mat6 & Mat6::operator -=( const Mat6 & a ) {
	mat[ 0 ][ 0 ] -= a[ 0 ][ 0 ]; mat[ 0 ][ 1 ] -= a[ 0 ][ 1 ]; mat[ 0 ][ 2 ] -= a[ 0 ][ 2 ]; mat[ 0 ][ 3 ] -= a[ 0 ][ 3 ]; mat[ 0 ][ 4 ] -= a[ 0 ][ 4 ]; mat[ 0 ][ 5 ] -= a[ 0 ][ 5 ];
	mat[ 1 ][ 0 ] -= a[ 1 ][ 0 ]; mat[ 1 ][ 1 ] -= a[ 1 ][ 1 ]; mat[ 1 ][ 2 ] -= a[ 1 ][ 2 ]; mat[ 1 ][ 3 ] -= a[ 1 ][ 3 ]; mat[ 1 ][ 4 ] -= a[ 1 ][ 4 ]; mat[ 1 ][ 5 ] -= a[ 1 ][ 5 ];
	mat[ 2 ][ 0 ] -= a[ 2 ][ 0 ]; mat[ 2 ][ 1 ] -= a[ 2 ][ 1 ]; mat[ 2 ][ 2 ] -= a[ 2 ][ 2 ]; mat[ 2 ][ 3 ] -= a[ 2 ][ 3 ]; mat[ 2 ][ 4 ] -= a[ 2 ][ 4 ]; mat[ 2 ][ 5 ] -= a[ 2 ][ 5 ];
	mat[ 3 ][ 0 ] -= a[ 3 ][ 0 ]; mat[ 3 ][ 1 ] -= a[ 3 ][ 1 ]; mat[ 3 ][ 2 ] -= a[ 3 ][ 2 ]; mat[ 3 ][ 3 ] -= a[ 3 ][ 3 ]; mat[ 3 ][ 4 ] -= a[ 3 ][ 4 ]; mat[ 3 ][ 5 ] -= a[ 3 ][ 5 ];
	mat[ 4 ][ 0 ] -= a[ 4 ][ 0 ]; mat[ 4 ][ 1 ] -= a[ 4 ][ 1 ]; mat[ 4 ][ 2 ] -= a[ 4 ][ 2 ]; mat[ 4 ][ 3 ] -= a[ 4 ][ 3 ]; mat[ 4 ][ 4 ] -= a[ 4 ][ 4 ]; mat[ 4 ][ 5 ] -= a[ 4 ][ 5 ];
	mat[ 5 ][ 0 ] -= a[ 5 ][ 0 ]; mat[ 5 ][ 1 ] -= a[ 5 ][ 1 ]; mat[ 5 ][ 2 ] -= a[ 5 ][ 2 ]; mat[ 5 ][ 3 ] -= a[ 5 ][ 3 ]; mat[ 5 ][ 4 ] -= a[ 5 ][ 4 ]; mat[ 5 ][ 5 ] -= a[ 5 ][ 5 ];
	return *this;
}

INLINE Vec6 operator *( const Vec6 & vec, const Mat6 & mat ) {
	return mat * vec;
}

INLINE Mat6 operator *( const float a, Mat6 const & mat ) {
	return mat * a;
}

INLINE Vec6 & operator *=( Vec6 & vec, const Mat6 & mat ) {
	vec = mat * vec;
	return vec;
}

INLINE bool Mat6::Compare( const Mat6 & a ) const {
	dword i;
	const float *ptr1, *ptr2;

	ptr1 = reinterpret_cast<const float *>(mat);
	ptr2 = reinterpret_cast<const float *>(a.mat);
	for( i = 0; i < 6*6; i++ ) {
		if( ptr1[i] != ptr2[i] ) {
			return false;
		}
	}
	return true;
}

INLINE bool Mat6::Compare( const Mat6 & a, const float epsilon ) const {
	dword i;
	const float *ptr1, *ptr2;

	ptr1 = reinterpret_cast<const float *>(mat);
	ptr2 = reinterpret_cast<const float *>(a.mat);
	for( i = 0; i < 6*6; i++ ) {
		if( fabs( ptr1[i] - ptr2[i] ) > epsilon ) {
			return false;
		}
	}
	return true;
}

INLINE bool Mat6::operator ==( const Mat6 & a ) const {
	return Compare( a );
}

INLINE bool Mat6::operator !=( const Mat6 & a ) const {
	return !Compare( a );
}

INLINE void Mat6::Zero( ) {
	Common::Com_Memset( mat, 0, sizeof( Mat6 ) );
}

INLINE void Mat6::Identity( ) {
	*this = mat6_identity;
}

INLINE bool Mat6::IsIdentity( const float epsilon ) const {
	return Compare( mat6_identity, epsilon );
}

INLINE bool Mat6::IsSymmetric( const float epsilon ) const {
	for( int i = 1; i < 6; i++ ) {
		for( int j = 0; j < i; j++ ) {
			if( fabs( mat[i][j] - mat[j][i] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

INLINE bool Mat6::IsDiagonal( const float epsilon ) const {
	for( int i = 0; i < 6; i++ ) {
		for( int j = 0; j < 6; j++ ) {
			if( i != j && fabs( mat[i][j] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

INLINE Mat3 Mat6::SubMat3( int n ) const {
	assert( n >= 0 && n < 4 );
	int b0 = ((n & 2) >> 1) * 3;
	int b1 = (n & 1) * 3;
	return Mat3(
		mat[b0 + 0][b1 + 0], mat[b0 + 0][b1 + 1], mat[b0 + 0][b1 + 2],
		mat[b0 + 1][b1 + 0], mat[b0 + 1][b1 + 1], mat[b0 + 1][b1 + 2],
		mat[b0 + 2][b1 + 0], mat[b0 + 2][b1 + 1], mat[b0 + 2][b1 + 2] );
}

INLINE float Mat6::Trace( ) const {
	return ( mat[ 0 ][ 0 ] + mat[ 1 ][ 1 ] + mat[ 2 ][ 2 ] + mat[ 3 ][ 3 ] + mat[ 4 ][ 4 ] + mat[ 5 ][ 5 ] );
}

INLINE Mat6 Mat6::Inverse( ) const {
	Mat6 invMat;

	invMat = *this;
	int r = invMat.InverseSelf( );
	assert( r );
	return invMat;
}

INLINE Mat6 Mat6::InverseFast( ) const {
	Mat6 invMat;

	invMat = *this;
	int r = invMat.InverseFastSelf( );
	assert( r );
	return invMat;
}

INLINE int Mat6::GetDimension( ) const {
	return 36;
}

INLINE const float *Mat6::ToFloatPtr( ) const {
	return mat[ 0 ].ToFloatPtr( );
}

INLINE float *Mat6::ToFloatPtr( ) {
	return mat[ 0 ].ToFloatPtr( );
}

/*
============
Mat6::Transpose
============
*/
INLINE Mat6 Mat6::Transpose( ) const {
	Mat6	transpose;
	int		i, j;
   
	for( i = 0; i < 6; i++ ) {
		for( j = 0; j < 6; j++ ) {
			transpose[ i ][ j ] = mat[ j ][ i ];
        }
	}
	return transpose;
}

/*
============
Mat6::TransposeSelf
============
*/
INLINE Mat6 & Mat6::TransposeSelf( ) {
	float	temp;
	int		i, j;
   
	for( i = 0; i < 6; i++ ) {
		for( j = i + 1; j < 6; j++ ) {
			temp = mat[ i ][ j ];
			mat[ i ][ j ] = mat[ j ][ i ];
			mat[ j ][ i ] = temp;
        }
	}
	return *this;
}

/*
============
Mat6::Determinant
============
*/
INLINE float Mat6::Determinant( ) const {

	// 2x2 sub-determinants required to calculate 6x6 determinant
	float det2_45_01 = mat[ 4 ][ 0 ] * mat[ 5 ][ 1 ] - mat[ 4 ][ 1 ] * mat[ 5 ][ 0 ];
	float det2_45_02 = mat[ 4 ][ 0 ] * mat[ 5 ][ 2 ] - mat[ 4 ][ 2 ] * mat[ 5 ][ 0 ];
	float det2_45_03 = mat[ 4 ][ 0 ] * mat[ 5 ][ 3 ] - mat[ 4 ][ 3 ] * mat[ 5 ][ 0 ];
	float det2_45_04 = mat[ 4 ][ 0 ] * mat[ 5 ][ 4 ] - mat[ 4 ][ 4 ] * mat[ 5 ][ 0 ];
	float det2_45_05 = mat[ 4 ][ 0 ] * mat[ 5 ][ 5 ] - mat[ 4 ][ 5 ] * mat[ 5 ][ 0 ];
	float det2_45_12 = mat[ 4 ][ 1 ] * mat[ 5 ][ 2 ] - mat[ 4 ][ 2 ] * mat[ 5 ][ 1 ];
	float det2_45_13 = mat[ 4 ][ 1 ] * mat[ 5 ][ 3 ] - mat[ 4 ][ 3 ] * mat[ 5 ][ 1 ];
	float det2_45_14 = mat[ 4 ][ 1 ] * mat[ 5 ][ 4 ] - mat[ 4 ][ 4 ] * mat[ 5 ][ 1 ];
	float det2_45_15 = mat[ 4 ][ 1 ] * mat[ 5 ][ 5 ] - mat[ 4 ][ 5 ] * mat[ 5 ][ 1 ];
	float det2_45_23 = mat[ 4 ][ 2 ] * mat[ 5 ][ 3 ] - mat[ 4 ][ 3 ] * mat[ 5 ][ 2 ];
	float det2_45_24 = mat[ 4 ][ 2 ] * mat[ 5 ][ 4 ] - mat[ 4 ][ 4 ] * mat[ 5 ][ 2 ];
	float det2_45_25 = mat[ 4 ][ 2 ] * mat[ 5 ][ 5 ] - mat[ 4 ][ 5 ] * mat[ 5 ][ 2 ];
	float det2_45_34 = mat[ 4 ][ 3 ] * mat[ 5 ][ 4 ] - mat[ 4 ][ 4 ] * mat[ 5 ][ 3 ];
	float det2_45_35 = mat[ 4 ][ 3 ] * mat[ 5 ][ 5 ] - mat[ 4 ][ 5 ] * mat[ 5 ][ 3 ];
	float det2_45_45 = mat[ 4 ][ 4 ] * mat[ 5 ][ 5 ] - mat[ 4 ][ 5 ] * mat[ 5 ][ 4 ];

	// 3x3 sub-determinants required to calculate 6x6 determinant
	float det3_345_012 = mat[ 3 ][ 0 ] * det2_45_12 - mat[ 3 ][ 1 ] * det2_45_02 + mat[ 3 ][ 2 ] * det2_45_01;
	float det3_345_013 = mat[ 3 ][ 0 ] * det2_45_13 - mat[ 3 ][ 1 ] * det2_45_03 + mat[ 3 ][ 3 ] * det2_45_01;
	float det3_345_014 = mat[ 3 ][ 0 ] * det2_45_14 - mat[ 3 ][ 1 ] * det2_45_04 + mat[ 3 ][ 4 ] * det2_45_01;
	float det3_345_015 = mat[ 3 ][ 0 ] * det2_45_15 - mat[ 3 ][ 1 ] * det2_45_05 + mat[ 3 ][ 5 ] * det2_45_01;
	float det3_345_023 = mat[ 3 ][ 0 ] * det2_45_23 - mat[ 3 ][ 2 ] * det2_45_03 + mat[ 3 ][ 3 ] * det2_45_02;
	float det3_345_024 = mat[ 3 ][ 0 ] * det2_45_24 - mat[ 3 ][ 2 ] * det2_45_04 + mat[ 3 ][ 4 ] * det2_45_02;
	float det3_345_025 = mat[ 3 ][ 0 ] * det2_45_25 - mat[ 3 ][ 2 ] * det2_45_05 + mat[ 3 ][ 5 ] * det2_45_02;
	float det3_345_034 = mat[ 3 ][ 0 ] * det2_45_34 - mat[ 3 ][ 3 ] * det2_45_04 + mat[ 3 ][ 4 ] * det2_45_03;
	float det3_345_035 = mat[ 3 ][ 0 ] * det2_45_35 - mat[ 3 ][ 3 ] * det2_45_05 + mat[ 3 ][ 5 ] * det2_45_03;
	float det3_345_045 = mat[ 3 ][ 0 ] * det2_45_45 - mat[ 3 ][ 4 ] * det2_45_05 + mat[ 3 ][ 5 ] * det2_45_04;
	float det3_345_123 = mat[ 3 ][ 1 ] * det2_45_23 - mat[ 3 ][ 2 ] * det2_45_13 + mat[ 3 ][ 3 ] * det2_45_12;
	float det3_345_124 = mat[ 3 ][ 1 ] * det2_45_24 - mat[ 3 ][ 2 ] * det2_45_14 + mat[ 3 ][ 4 ] * det2_45_12;
	float det3_345_125 = mat[ 3 ][ 1 ] * det2_45_25 - mat[ 3 ][ 2 ] * det2_45_15 + mat[ 3 ][ 5 ] * det2_45_12;
	float det3_345_134 = mat[ 3 ][ 1 ] * det2_45_34 - mat[ 3 ][ 3 ] * det2_45_14 + mat[ 3 ][ 4 ] * det2_45_13;
	float det3_345_135 = mat[ 3 ][ 1 ] * det2_45_35 - mat[ 3 ][ 3 ] * det2_45_15 + mat[ 3 ][ 5 ] * det2_45_13;
	float det3_345_145 = mat[ 3 ][ 1 ] * det2_45_45 - mat[ 3 ][ 4 ] * det2_45_15 + mat[ 3 ][ 5 ] * det2_45_14;
	float det3_345_234 = mat[ 3 ][ 2 ] * det2_45_34 - mat[ 3 ][ 3 ] * det2_45_24 + mat[ 3 ][ 4 ] * det2_45_23;
	float det3_345_235 = mat[ 3 ][ 2 ] * det2_45_35 - mat[ 3 ][ 3 ] * det2_45_25 + mat[ 3 ][ 5 ] * det2_45_23;
	float det3_345_245 = mat[ 3 ][ 2 ] * det2_45_45 - mat[ 3 ][ 4 ] * det2_45_25 + mat[ 3 ][ 5 ] * det2_45_24;
	float det3_345_345 = mat[ 3 ][ 3 ] * det2_45_45 - mat[ 3 ][ 4 ] * det2_45_35 + mat[ 3 ][ 5 ] * det2_45_34;

	// 4x4 sub-determinants required to calculate 6x6 determinant
	float det4_2345_0123 = mat[ 2 ][ 0 ] * det3_345_123 - mat[ 2 ][ 1 ] * det3_345_023 + mat[ 2 ][ 2 ] * det3_345_013 - mat[ 2 ][ 3 ] * det3_345_012;
	float det4_2345_0124 = mat[ 2 ][ 0 ] * det3_345_124 - mat[ 2 ][ 1 ] * det3_345_024 + mat[ 2 ][ 2 ] * det3_345_014 - mat[ 2 ][ 4 ] * det3_345_012;
	float det4_2345_0125 = mat[ 2 ][ 0 ] * det3_345_125 - mat[ 2 ][ 1 ] * det3_345_025 + mat[ 2 ][ 2 ] * det3_345_015 - mat[ 2 ][ 5 ] * det3_345_012;
	float det4_2345_0134 = mat[ 2 ][ 0 ] * det3_345_134 - mat[ 2 ][ 1 ] * det3_345_034 + mat[ 2 ][ 3 ] * det3_345_014 - mat[ 2 ][ 4 ] * det3_345_013;
	float det4_2345_0135 = mat[ 2 ][ 0 ] * det3_345_135 - mat[ 2 ][ 1 ] * det3_345_035 + mat[ 2 ][ 3 ] * det3_345_015 - mat[ 2 ][ 5 ] * det3_345_013;
	float det4_2345_0145 = mat[ 2 ][ 0 ] * det3_345_145 - mat[ 2 ][ 1 ] * det3_345_045 + mat[ 2 ][ 4 ] * det3_345_015 - mat[ 2 ][ 5 ] * det3_345_014;
	float det4_2345_0234 = mat[ 2 ][ 0 ] * det3_345_234 - mat[ 2 ][ 2 ] * det3_345_034 + mat[ 2 ][ 3 ] * det3_345_024 - mat[ 2 ][ 4 ] * det3_345_023;
	float det4_2345_0235 = mat[ 2 ][ 0 ] * det3_345_235 - mat[ 2 ][ 2 ] * det3_345_035 + mat[ 2 ][ 3 ] * det3_345_025 - mat[ 2 ][ 5 ] * det3_345_023;
	float det4_2345_0245 = mat[ 2 ][ 0 ] * det3_345_245 - mat[ 2 ][ 2 ] * det3_345_045 + mat[ 2 ][ 4 ] * det3_345_025 - mat[ 2 ][ 5 ] * det3_345_024;
	float det4_2345_0345 = mat[ 2 ][ 0 ] * det3_345_345 - mat[ 2 ][ 3 ] * det3_345_045 + mat[ 2 ][ 4 ] * det3_345_035 - mat[ 2 ][ 5 ] * det3_345_034;
	float det4_2345_1234 = mat[ 2 ][ 1 ] * det3_345_234 - mat[ 2 ][ 2 ] * det3_345_134 + mat[ 2 ][ 3 ] * det3_345_124 - mat[ 2 ][ 4 ] * det3_345_123;
	float det4_2345_1235 = mat[ 2 ][ 1 ] * det3_345_235 - mat[ 2 ][ 2 ] * det3_345_135 + mat[ 2 ][ 3 ] * det3_345_125 - mat[ 2 ][ 5 ] * det3_345_123;
	float det4_2345_1245 = mat[ 2 ][ 1 ] * det3_345_245 - mat[ 2 ][ 2 ] * det3_345_145 + mat[ 2 ][ 4 ] * det3_345_125 - mat[ 2 ][ 5 ] * det3_345_124;
	float det4_2345_1345 = mat[ 2 ][ 1 ] * det3_345_345 - mat[ 2 ][ 3 ] * det3_345_145 + mat[ 2 ][ 4 ] * det3_345_135 - mat[ 2 ][ 5 ] * det3_345_134;
	float det4_2345_2345 = mat[ 2 ][ 2 ] * det3_345_345 - mat[ 2 ][ 3 ] * det3_345_245 + mat[ 2 ][ 4 ] * det3_345_235 - mat[ 2 ][ 5 ] * det3_345_234;

	// 5x5 sub-determinants required to calculate 6x6 determinant
	float det5_12345_01234 = mat[ 1 ][ 0 ] * det4_2345_1234 - mat[ 1 ][ 1 ] * det4_2345_0234 + mat[ 1 ][ 2 ] * det4_2345_0134 - mat[ 1 ][ 3 ] * det4_2345_0124 + mat[ 1 ][ 4 ] * det4_2345_0123;
	float det5_12345_01235 = mat[ 1 ][ 0 ] * det4_2345_1235 - mat[ 1 ][ 1 ] * det4_2345_0235 + mat[ 1 ][ 2 ] * det4_2345_0135 - mat[ 1 ][ 3 ] * det4_2345_0125 + mat[ 1 ][ 5 ] * det4_2345_0123;
	float det5_12345_01245 = mat[ 1 ][ 0 ] * det4_2345_1245 - mat[ 1 ][ 1 ] * det4_2345_0245 + mat[ 1 ][ 2 ] * det4_2345_0145 - mat[ 1 ][ 4 ] * det4_2345_0125 + mat[ 1 ][ 5 ] * det4_2345_0124;
	float det5_12345_01345 = mat[ 1 ][ 0 ] * det4_2345_1345 - mat[ 1 ][ 1 ] * det4_2345_0345 + mat[ 1 ][ 3 ] * det4_2345_0145 - mat[ 1 ][ 4 ] * det4_2345_0135 + mat[ 1 ][ 5 ] * det4_2345_0134;
	float det5_12345_02345 = mat[ 1 ][ 0 ] * det4_2345_2345 - mat[ 1 ][ 2 ] * det4_2345_0345 + mat[ 1 ][ 3 ] * det4_2345_0245 - mat[ 1 ][ 4 ] * det4_2345_0235 + mat[ 1 ][ 5 ] * det4_2345_0234;
	float det5_12345_12345 = mat[ 1 ][ 1 ] * det4_2345_2345 - mat[ 1 ][ 2 ] * det4_2345_1345 + mat[ 1 ][ 3 ] * det4_2345_1245 - mat[ 1 ][ 4 ] * det4_2345_1235 + mat[ 1 ][ 5 ] * det4_2345_1234;

	// determinant of 6x6 matrix
	return	mat[ 0 ][ 0 ] * det5_12345_12345 - mat[ 0 ][ 1 ] * det5_12345_02345 + mat[ 0 ][ 2 ] * det5_12345_01345 -
			mat[ 0 ][ 3 ] * det5_12345_01245 + mat[ 0 ][ 4 ] * det5_12345_01235 - mat[ 0 ][ 5 ] * det5_12345_01234;
}

/*
============
Mat6::InverseSelf
============
*/
INLINE bool Mat6::InverseSelf( ) {
	// 810+6+36 = 852 multiplications
	//				1 division
	float det, invDet;

	// 2x2 sub-determinants required to calculate 6x6 determinant
	float det2_45_01 = mat[ 4 ][ 0 ] * mat[ 5 ][ 1 ] - mat[ 4 ][ 1 ] * mat[ 5 ][ 0 ];
	float det2_45_02 = mat[ 4 ][ 0 ] * mat[ 5 ][ 2 ] - mat[ 4 ][ 2 ] * mat[ 5 ][ 0 ];
	float det2_45_03 = mat[ 4 ][ 0 ] * mat[ 5 ][ 3 ] - mat[ 4 ][ 3 ] * mat[ 5 ][ 0 ];
	float det2_45_04 = mat[ 4 ][ 0 ] * mat[ 5 ][ 4 ] - mat[ 4 ][ 4 ] * mat[ 5 ][ 0 ];
	float det2_45_05 = mat[ 4 ][ 0 ] * mat[ 5 ][ 5 ] - mat[ 4 ][ 5 ] * mat[ 5 ][ 0 ];
	float det2_45_12 = mat[ 4 ][ 1 ] * mat[ 5 ][ 2 ] - mat[ 4 ][ 2 ] * mat[ 5 ][ 1 ];
	float det2_45_13 = mat[ 4 ][ 1 ] * mat[ 5 ][ 3 ] - mat[ 4 ][ 3 ] * mat[ 5 ][ 1 ];
	float det2_45_14 = mat[ 4 ][ 1 ] * mat[ 5 ][ 4 ] - mat[ 4 ][ 4 ] * mat[ 5 ][ 1 ];
	float det2_45_15 = mat[ 4 ][ 1 ] * mat[ 5 ][ 5 ] - mat[ 4 ][ 5 ] * mat[ 5 ][ 1 ];
	float det2_45_23 = mat[ 4 ][ 2 ] * mat[ 5 ][ 3 ] - mat[ 4 ][ 3 ] * mat[ 5 ][ 2 ];
	float det2_45_24 = mat[ 4 ][ 2 ] * mat[ 5 ][ 4 ] - mat[ 4 ][ 4 ] * mat[ 5 ][ 2 ];
	float det2_45_25 = mat[ 4 ][ 2 ] * mat[ 5 ][ 5 ] - mat[ 4 ][ 5 ] * mat[ 5 ][ 2 ];
	float det2_45_34 = mat[ 4 ][ 3 ] * mat[ 5 ][ 4 ] - mat[ 4 ][ 4 ] * mat[ 5 ][ 3 ];
	float det2_45_35 = mat[ 4 ][ 3 ] * mat[ 5 ][ 5 ] - mat[ 4 ][ 5 ] * mat[ 5 ][ 3 ];
	float det2_45_45 = mat[ 4 ][ 4 ] * mat[ 5 ][ 5 ] - mat[ 4 ][ 5 ] * mat[ 5 ][ 4 ];

	// 3x3 sub-determinants required to calculate 6x6 determinant
	float det3_345_012 = mat[ 3 ][ 0 ] * det2_45_12 - mat[ 3 ][ 1 ] * det2_45_02 + mat[ 3 ][ 2 ] * det2_45_01;
	float det3_345_013 = mat[ 3 ][ 0 ] * det2_45_13 - mat[ 3 ][ 1 ] * det2_45_03 + mat[ 3 ][ 3 ] * det2_45_01;
	float det3_345_014 = mat[ 3 ][ 0 ] * det2_45_14 - mat[ 3 ][ 1 ] * det2_45_04 + mat[ 3 ][ 4 ] * det2_45_01;
	float det3_345_015 = mat[ 3 ][ 0 ] * det2_45_15 - mat[ 3 ][ 1 ] * det2_45_05 + mat[ 3 ][ 5 ] * det2_45_01;
	float det3_345_023 = mat[ 3 ][ 0 ] * det2_45_23 - mat[ 3 ][ 2 ] * det2_45_03 + mat[ 3 ][ 3 ] * det2_45_02;
	float det3_345_024 = mat[ 3 ][ 0 ] * det2_45_24 - mat[ 3 ][ 2 ] * det2_45_04 + mat[ 3 ][ 4 ] * det2_45_02;
	float det3_345_025 = mat[ 3 ][ 0 ] * det2_45_25 - mat[ 3 ][ 2 ] * det2_45_05 + mat[ 3 ][ 5 ] * det2_45_02;
	float det3_345_034 = mat[ 3 ][ 0 ] * det2_45_34 - mat[ 3 ][ 3 ] * det2_45_04 + mat[ 3 ][ 4 ] * det2_45_03;
	float det3_345_035 = mat[ 3 ][ 0 ] * det2_45_35 - mat[ 3 ][ 3 ] * det2_45_05 + mat[ 3 ][ 5 ] * det2_45_03;
	float det3_345_045 = mat[ 3 ][ 0 ] * det2_45_45 - mat[ 3 ][ 4 ] * det2_45_05 + mat[ 3 ][ 5 ] * det2_45_04;
	float det3_345_123 = mat[ 3 ][ 1 ] * det2_45_23 - mat[ 3 ][ 2 ] * det2_45_13 + mat[ 3 ][ 3 ] * det2_45_12;
	float det3_345_124 = mat[ 3 ][ 1 ] * det2_45_24 - mat[ 3 ][ 2 ] * det2_45_14 + mat[ 3 ][ 4 ] * det2_45_12;
	float det3_345_125 = mat[ 3 ][ 1 ] * det2_45_25 - mat[ 3 ][ 2 ] * det2_45_15 + mat[ 3 ][ 5 ] * det2_45_12;
	float det3_345_134 = mat[ 3 ][ 1 ] * det2_45_34 - mat[ 3 ][ 3 ] * det2_45_14 + mat[ 3 ][ 4 ] * det2_45_13;
	float det3_345_135 = mat[ 3 ][ 1 ] * det2_45_35 - mat[ 3 ][ 3 ] * det2_45_15 + mat[ 3 ][ 5 ] * det2_45_13;
	float det3_345_145 = mat[ 3 ][ 1 ] * det2_45_45 - mat[ 3 ][ 4 ] * det2_45_15 + mat[ 3 ][ 5 ] * det2_45_14;
	float det3_345_234 = mat[ 3 ][ 2 ] * det2_45_34 - mat[ 3 ][ 3 ] * det2_45_24 + mat[ 3 ][ 4 ] * det2_45_23;
	float det3_345_235 = mat[ 3 ][ 2 ] * det2_45_35 - mat[ 3 ][ 3 ] * det2_45_25 + mat[ 3 ][ 5 ] * det2_45_23;
	float det3_345_245 = mat[ 3 ][ 2 ] * det2_45_45 - mat[ 3 ][ 4 ] * det2_45_25 + mat[ 3 ][ 5 ] * det2_45_24;
	float det3_345_345 = mat[ 3 ][ 3 ] * det2_45_45 - mat[ 3 ][ 4 ] * det2_45_35 + mat[ 3 ][ 5 ] * det2_45_34;

	// 4x4 sub-determinants required to calculate 6x6 determinant
	float det4_2345_0123 = mat[ 2 ][ 0 ] * det3_345_123 - mat[ 2 ][ 1 ] * det3_345_023 + mat[ 2 ][ 2 ] * det3_345_013 - mat[ 2 ][ 3 ] * det3_345_012;
	float det4_2345_0124 = mat[ 2 ][ 0 ] * det3_345_124 - mat[ 2 ][ 1 ] * det3_345_024 + mat[ 2 ][ 2 ] * det3_345_014 - mat[ 2 ][ 4 ] * det3_345_012;
	float det4_2345_0125 = mat[ 2 ][ 0 ] * det3_345_125 - mat[ 2 ][ 1 ] * det3_345_025 + mat[ 2 ][ 2 ] * det3_345_015 - mat[ 2 ][ 5 ] * det3_345_012;
	float det4_2345_0134 = mat[ 2 ][ 0 ] * det3_345_134 - mat[ 2 ][ 1 ] * det3_345_034 + mat[ 2 ][ 3 ] * det3_345_014 - mat[ 2 ][ 4 ] * det3_345_013;
	float det4_2345_0135 = mat[ 2 ][ 0 ] * det3_345_135 - mat[ 2 ][ 1 ] * det3_345_035 + mat[ 2 ][ 3 ] * det3_345_015 - mat[ 2 ][ 5 ] * det3_345_013;
	float det4_2345_0145 = mat[ 2 ][ 0 ] * det3_345_145 - mat[ 2 ][ 1 ] * det3_345_045 + mat[ 2 ][ 4 ] * det3_345_015 - mat[ 2 ][ 5 ] * det3_345_014;
	float det4_2345_0234 = mat[ 2 ][ 0 ] * det3_345_234 - mat[ 2 ][ 2 ] * det3_345_034 + mat[ 2 ][ 3 ] * det3_345_024 - mat[ 2 ][ 4 ] * det3_345_023;
	float det4_2345_0235 = mat[ 2 ][ 0 ] * det3_345_235 - mat[ 2 ][ 2 ] * det3_345_035 + mat[ 2 ][ 3 ] * det3_345_025 - mat[ 2 ][ 5 ] * det3_345_023;
	float det4_2345_0245 = mat[ 2 ][ 0 ] * det3_345_245 - mat[ 2 ][ 2 ] * det3_345_045 + mat[ 2 ][ 4 ] * det3_345_025 - mat[ 2 ][ 5 ] * det3_345_024;
	float det4_2345_0345 = mat[ 2 ][ 0 ] * det3_345_345 - mat[ 2 ][ 3 ] * det3_345_045 + mat[ 2 ][ 4 ] * det3_345_035 - mat[ 2 ][ 5 ] * det3_345_034;
	float det4_2345_1234 = mat[ 2 ][ 1 ] * det3_345_234 - mat[ 2 ][ 2 ] * det3_345_134 + mat[ 2 ][ 3 ] * det3_345_124 - mat[ 2 ][ 4 ] * det3_345_123;
	float det4_2345_1235 = mat[ 2 ][ 1 ] * det3_345_235 - mat[ 2 ][ 2 ] * det3_345_135 + mat[ 2 ][ 3 ] * det3_345_125 - mat[ 2 ][ 5 ] * det3_345_123;
	float det4_2345_1245 = mat[ 2 ][ 1 ] * det3_345_245 - mat[ 2 ][ 2 ] * det3_345_145 + mat[ 2 ][ 4 ] * det3_345_125 - mat[ 2 ][ 5 ] * det3_345_124;
	float det4_2345_1345 = mat[ 2 ][ 1 ] * det3_345_345 - mat[ 2 ][ 3 ] * det3_345_145 + mat[ 2 ][ 4 ] * det3_345_135 - mat[ 2 ][ 5 ] * det3_345_134;
	float det4_2345_2345 = mat[ 2 ][ 2 ] * det3_345_345 - mat[ 2 ][ 3 ] * det3_345_245 + mat[ 2 ][ 4 ] * det3_345_235 - mat[ 2 ][ 5 ] * det3_345_234;

	// 5x5 sub-determinants required to calculate 6x6 determinant
	float det5_12345_01234 = mat[ 1 ][ 0 ] * det4_2345_1234 - mat[ 1 ][ 1 ] * det4_2345_0234 + mat[ 1 ][ 2 ] * det4_2345_0134 - mat[ 1 ][ 3 ] * det4_2345_0124 + mat[ 1 ][ 4 ] * det4_2345_0123;
	float det5_12345_01235 = mat[ 1 ][ 0 ] * det4_2345_1235 - mat[ 1 ][ 1 ] * det4_2345_0235 + mat[ 1 ][ 2 ] * det4_2345_0135 - mat[ 1 ][ 3 ] * det4_2345_0125 + mat[ 1 ][ 5 ] * det4_2345_0123;
	float det5_12345_01245 = mat[ 1 ][ 0 ] * det4_2345_1245 - mat[ 1 ][ 1 ] * det4_2345_0245 + mat[ 1 ][ 2 ] * det4_2345_0145 - mat[ 1 ][ 4 ] * det4_2345_0125 + mat[ 1 ][ 5 ] * det4_2345_0124;
	float det5_12345_01345 = mat[ 1 ][ 0 ] * det4_2345_1345 - mat[ 1 ][ 1 ] * det4_2345_0345 + mat[ 1 ][ 3 ] * det4_2345_0145 - mat[ 1 ][ 4 ] * det4_2345_0135 + mat[ 1 ][ 5 ] * det4_2345_0134;
	float det5_12345_02345 = mat[ 1 ][ 0 ] * det4_2345_2345 - mat[ 1 ][ 2 ] * det4_2345_0345 + mat[ 1 ][ 3 ] * det4_2345_0245 - mat[ 1 ][ 4 ] * det4_2345_0235 + mat[ 1 ][ 5 ] * det4_2345_0234;
	float det5_12345_12345 = mat[ 1 ][ 1 ] * det4_2345_2345 - mat[ 1 ][ 2 ] * det4_2345_1345 + mat[ 1 ][ 3 ] * det4_2345_1245 - mat[ 1 ][ 4 ] * det4_2345_1235 + mat[ 1 ][ 5 ] * det4_2345_1234;

	// determinant of 6x6 matrix
	det = mat[ 0 ][ 0 ] * det5_12345_12345 - mat[ 0 ][ 1 ] * det5_12345_02345 + mat[ 0 ][ 2 ] * det5_12345_01345 -
				mat[ 0 ][ 3 ] * det5_12345_01245 + mat[ 0 ][ 4 ] * det5_12345_01235 - mat[ 0 ][ 5 ] * det5_12345_01234;

	if( fabs( det ) < MATRIX_INVERSE_EPSILON ) {
		return false;
	}

	invDet = 1.0f / det;

	// remaining 2x2 sub-determinants
	float det2_34_01 = mat[ 3 ][ 0 ] * mat[ 4 ][ 1 ] - mat[ 3 ][ 1 ] * mat[ 4 ][ 0 ];
	float det2_34_02 = mat[ 3 ][ 0 ] * mat[ 4 ][ 2 ] - mat[ 3 ][ 2 ] * mat[ 4 ][ 0 ];
	float det2_34_03 = mat[ 3 ][ 0 ] * mat[ 4 ][ 3 ] - mat[ 3 ][ 3 ] * mat[ 4 ][ 0 ];
	float det2_34_04 = mat[ 3 ][ 0 ] * mat[ 4 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 4 ][ 0 ];
	float det2_34_05 = mat[ 3 ][ 0 ] * mat[ 4 ][ 5 ] - mat[ 3 ][ 5 ] * mat[ 4 ][ 0 ];
	float det2_34_12 = mat[ 3 ][ 1 ] * mat[ 4 ][ 2 ] - mat[ 3 ][ 2 ] * mat[ 4 ][ 1 ];
	float det2_34_13 = mat[ 3 ][ 1 ] * mat[ 4 ][ 3 ] - mat[ 3 ][ 3 ] * mat[ 4 ][ 1 ];
	float det2_34_14 = mat[ 3 ][ 1 ] * mat[ 4 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 4 ][ 1 ];
	float det2_34_15 = mat[ 3 ][ 1 ] * mat[ 4 ][ 5 ] - mat[ 3 ][ 5 ] * mat[ 4 ][ 1 ];
	float det2_34_23 = mat[ 3 ][ 2 ] * mat[ 4 ][ 3 ] - mat[ 3 ][ 3 ] * mat[ 4 ][ 2 ];
	float det2_34_24 = mat[ 3 ][ 2 ] * mat[ 4 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 4 ][ 2 ];
	float det2_34_25 = mat[ 3 ][ 2 ] * mat[ 4 ][ 5 ] - mat[ 3 ][ 5 ] * mat[ 4 ][ 2 ];
	float det2_34_34 = mat[ 3 ][ 3 ] * mat[ 4 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 4 ][ 3 ];
	float det2_34_35 = mat[ 3 ][ 3 ] * mat[ 4 ][ 5 ] - mat[ 3 ][ 5 ] * mat[ 4 ][ 3 ];
	float det2_34_45 = mat[ 3 ][ 4 ] * mat[ 4 ][ 5 ] - mat[ 3 ][ 5 ] * mat[ 4 ][ 4 ];
	float det2_35_01 = mat[ 3 ][ 0 ] * mat[ 5 ][ 1 ] - mat[ 3 ][ 1 ] * mat[ 5 ][ 0 ];
	float det2_35_02 = mat[ 3 ][ 0 ] * mat[ 5 ][ 2 ] - mat[ 3 ][ 2 ] * mat[ 5 ][ 0 ];
	float det2_35_03 = mat[ 3 ][ 0 ] * mat[ 5 ][ 3 ] - mat[ 3 ][ 3 ] * mat[ 5 ][ 0 ];
	float det2_35_04 = mat[ 3 ][ 0 ] * mat[ 5 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 5 ][ 0 ];
	float det2_35_05 = mat[ 3 ][ 0 ] * mat[ 5 ][ 5 ] - mat[ 3 ][ 5 ] * mat[ 5 ][ 0 ];
	float det2_35_12 = mat[ 3 ][ 1 ] * mat[ 5 ][ 2 ] - mat[ 3 ][ 2 ] * mat[ 5 ][ 1 ];
	float det2_35_13 = mat[ 3 ][ 1 ] * mat[ 5 ][ 3 ] - mat[ 3 ][ 3 ] * mat[ 5 ][ 1 ];
	float det2_35_14 = mat[ 3 ][ 1 ] * mat[ 5 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 5 ][ 1 ];
	float det2_35_15 = mat[ 3 ][ 1 ] * mat[ 5 ][ 5 ] - mat[ 3 ][ 5 ] * mat[ 5 ][ 1 ];
	float det2_35_23 = mat[ 3 ][ 2 ] * mat[ 5 ][ 3 ] - mat[ 3 ][ 3 ] * mat[ 5 ][ 2 ];
	float det2_35_24 = mat[ 3 ][ 2 ] * mat[ 5 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 5 ][ 2 ];
	float det2_35_25 = mat[ 3 ][ 2 ] * mat[ 5 ][ 5 ] - mat[ 3 ][ 5 ] * mat[ 5 ][ 2 ];
	float det2_35_34 = mat[ 3 ][ 3 ] * mat[ 5 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 5 ][ 3 ];
	float det2_35_35 = mat[ 3 ][ 3 ] * mat[ 5 ][ 5 ] - mat[ 3 ][ 5 ] * mat[ 5 ][ 3 ];
	float det2_35_45 = mat[ 3 ][ 4 ] * mat[ 5 ][ 5 ] - mat[ 3 ][ 5 ] * mat[ 5 ][ 4 ];

	// remaining 3x3 sub-determinants
	float det3_234_012 = mat[ 2 ][ 0 ] * det2_34_12 - mat[ 2 ][ 1 ] * det2_34_02 + mat[ 2 ][ 2 ] * det2_34_01;
	float det3_234_013 = mat[ 2 ][ 0 ] * det2_34_13 - mat[ 2 ][ 1 ] * det2_34_03 + mat[ 2 ][ 3 ] * det2_34_01;
	float det3_234_014 = mat[ 2 ][ 0 ] * det2_34_14 - mat[ 2 ][ 1 ] * det2_34_04 + mat[ 2 ][ 4 ] * det2_34_01;
	float det3_234_015 = mat[ 2 ][ 0 ] * det2_34_15 - mat[ 2 ][ 1 ] * det2_34_05 + mat[ 2 ][ 5 ] * det2_34_01;
	float det3_234_023 = mat[ 2 ][ 0 ] * det2_34_23 - mat[ 2 ][ 2 ] * det2_34_03 + mat[ 2 ][ 3 ] * det2_34_02;
	float det3_234_024 = mat[ 2 ][ 0 ] * det2_34_24 - mat[ 2 ][ 2 ] * det2_34_04 + mat[ 2 ][ 4 ] * det2_34_02;
	float det3_234_025 = mat[ 2 ][ 0 ] * det2_34_25 - mat[ 2 ][ 2 ] * det2_34_05 + mat[ 2 ][ 5 ] * det2_34_02;
	float det3_234_034 = mat[ 2 ][ 0 ] * det2_34_34 - mat[ 2 ][ 3 ] * det2_34_04 + mat[ 2 ][ 4 ] * det2_34_03;
	float det3_234_035 = mat[ 2 ][ 0 ] * det2_34_35 - mat[ 2 ][ 3 ] * det2_34_05 + mat[ 2 ][ 5 ] * det2_34_03;
	float det3_234_045 = mat[ 2 ][ 0 ] * det2_34_45 - mat[ 2 ][ 4 ] * det2_34_05 + mat[ 2 ][ 5 ] * det2_34_04;
	float det3_234_123 = mat[ 2 ][ 1 ] * det2_34_23 - mat[ 2 ][ 2 ] * det2_34_13 + mat[ 2 ][ 3 ] * det2_34_12;
	float det3_234_124 = mat[ 2 ][ 1 ] * det2_34_24 - mat[ 2 ][ 2 ] * det2_34_14 + mat[ 2 ][ 4 ] * det2_34_12;
	float det3_234_125 = mat[ 2 ][ 1 ] * det2_34_25 - mat[ 2 ][ 2 ] * det2_34_15 + mat[ 2 ][ 5 ] * det2_34_12;
	float det3_234_134 = mat[ 2 ][ 1 ] * det2_34_34 - mat[ 2 ][ 3 ] * det2_34_14 + mat[ 2 ][ 4 ] * det2_34_13;
	float det3_234_135 = mat[ 2 ][ 1 ] * det2_34_35 - mat[ 2 ][ 3 ] * det2_34_15 + mat[ 2 ][ 5 ] * det2_34_13;
	float det3_234_145 = mat[ 2 ][ 1 ] * det2_34_45 - mat[ 2 ][ 4 ] * det2_34_15 + mat[ 2 ][ 5 ] * det2_34_14;
	float det3_234_234 = mat[ 2 ][ 2 ] * det2_34_34 - mat[ 2 ][ 3 ] * det2_34_24 + mat[ 2 ][ 4 ] * det2_34_23;
	float det3_234_235 = mat[ 2 ][ 2 ] * det2_34_35 - mat[ 2 ][ 3 ] * det2_34_25 + mat[ 2 ][ 5 ] * det2_34_23;
	float det3_234_245 = mat[ 2 ][ 2 ] * det2_34_45 - mat[ 2 ][ 4 ] * det2_34_25 + mat[ 2 ][ 5 ] * det2_34_24;
	float det3_234_345 = mat[ 2 ][ 3 ] * det2_34_45 - mat[ 2 ][ 4 ] * det2_34_35 + mat[ 2 ][ 5 ] * det2_34_34;
	float det3_235_012 = mat[ 2 ][ 0 ] * det2_35_12 - mat[ 2 ][ 1 ] * det2_35_02 + mat[ 2 ][ 2 ] * det2_35_01;
	float det3_235_013 = mat[ 2 ][ 0 ] * det2_35_13 - mat[ 2 ][ 1 ] * det2_35_03 + mat[ 2 ][ 3 ] * det2_35_01;
	float det3_235_014 = mat[ 2 ][ 0 ] * det2_35_14 - mat[ 2 ][ 1 ] * det2_35_04 + mat[ 2 ][ 4 ] * det2_35_01;
	float det3_235_015 = mat[ 2 ][ 0 ] * det2_35_15 - mat[ 2 ][ 1 ] * det2_35_05 + mat[ 2 ][ 5 ] * det2_35_01;
	float det3_235_023 = mat[ 2 ][ 0 ] * det2_35_23 - mat[ 2 ][ 2 ] * det2_35_03 + mat[ 2 ][ 3 ] * det2_35_02;
	float det3_235_024 = mat[ 2 ][ 0 ] * det2_35_24 - mat[ 2 ][ 2 ] * det2_35_04 + mat[ 2 ][ 4 ] * det2_35_02;
	float det3_235_025 = mat[ 2 ][ 0 ] * det2_35_25 - mat[ 2 ][ 2 ] * det2_35_05 + mat[ 2 ][ 5 ] * det2_35_02;
	float det3_235_034 = mat[ 2 ][ 0 ] * det2_35_34 - mat[ 2 ][ 3 ] * det2_35_04 + mat[ 2 ][ 4 ] * det2_35_03;
	float det3_235_035 = mat[ 2 ][ 0 ] * det2_35_35 - mat[ 2 ][ 3 ] * det2_35_05 + mat[ 2 ][ 5 ] * det2_35_03;
	float det3_235_045 = mat[ 2 ][ 0 ] * det2_35_45 - mat[ 2 ][ 4 ] * det2_35_05 + mat[ 2 ][ 5 ] * det2_35_04;
	float det3_235_123 = mat[ 2 ][ 1 ] * det2_35_23 - mat[ 2 ][ 2 ] * det2_35_13 + mat[ 2 ][ 3 ] * det2_35_12;
	float det3_235_124 = mat[ 2 ][ 1 ] * det2_35_24 - mat[ 2 ][ 2 ] * det2_35_14 + mat[ 2 ][ 4 ] * det2_35_12;
	float det3_235_125 = mat[ 2 ][ 1 ] * det2_35_25 - mat[ 2 ][ 2 ] * det2_35_15 + mat[ 2 ][ 5 ] * det2_35_12;
	float det3_235_134 = mat[ 2 ][ 1 ] * det2_35_34 - mat[ 2 ][ 3 ] * det2_35_14 + mat[ 2 ][ 4 ] * det2_35_13;
	float det3_235_135 = mat[ 2 ][ 1 ] * det2_35_35 - mat[ 2 ][ 3 ] * det2_35_15 + mat[ 2 ][ 5 ] * det2_35_13;
	float det3_235_145 = mat[ 2 ][ 1 ] * det2_35_45 - mat[ 2 ][ 4 ] * det2_35_15 + mat[ 2 ][ 5 ] * det2_35_14;
	float det3_235_234 = mat[ 2 ][ 2 ] * det2_35_34 - mat[ 2 ][ 3 ] * det2_35_24 + mat[ 2 ][ 4 ] * det2_35_23;
	float det3_235_235 = mat[ 2 ][ 2 ] * det2_35_35 - mat[ 2 ][ 3 ] * det2_35_25 + mat[ 2 ][ 5 ] * det2_35_23;
	float det3_235_245 = mat[ 2 ][ 2 ] * det2_35_45 - mat[ 2 ][ 4 ] * det2_35_25 + mat[ 2 ][ 5 ] * det2_35_24;
	float det3_235_345 = mat[ 2 ][ 3 ] * det2_35_45 - mat[ 2 ][ 4 ] * det2_35_35 + mat[ 2 ][ 5 ] * det2_35_34;
	float det3_245_012 = mat[ 2 ][ 0 ] * det2_45_12 - mat[ 2 ][ 1 ] * det2_45_02 + mat[ 2 ][ 2 ] * det2_45_01;
	float det3_245_013 = mat[ 2 ][ 0 ] * det2_45_13 - mat[ 2 ][ 1 ] * det2_45_03 + mat[ 2 ][ 3 ] * det2_45_01;
	float det3_245_014 = mat[ 2 ][ 0 ] * det2_45_14 - mat[ 2 ][ 1 ] * det2_45_04 + mat[ 2 ][ 4 ] * det2_45_01;
	float det3_245_015 = mat[ 2 ][ 0 ] * det2_45_15 - mat[ 2 ][ 1 ] * det2_45_05 + mat[ 2 ][ 5 ] * det2_45_01;
	float det3_245_023 = mat[ 2 ][ 0 ] * det2_45_23 - mat[ 2 ][ 2 ] * det2_45_03 + mat[ 2 ][ 3 ] * det2_45_02;
	float det3_245_024 = mat[ 2 ][ 0 ] * det2_45_24 - mat[ 2 ][ 2 ] * det2_45_04 + mat[ 2 ][ 4 ] * det2_45_02;
	float det3_245_025 = mat[ 2 ][ 0 ] * det2_45_25 - mat[ 2 ][ 2 ] * det2_45_05 + mat[ 2 ][ 5 ] * det2_45_02;
	float det3_245_034 = mat[ 2 ][ 0 ] * det2_45_34 - mat[ 2 ][ 3 ] * det2_45_04 + mat[ 2 ][ 4 ] * det2_45_03;
	float det3_245_035 = mat[ 2 ][ 0 ] * det2_45_35 - mat[ 2 ][ 3 ] * det2_45_05 + mat[ 2 ][ 5 ] * det2_45_03;
	float det3_245_045 = mat[ 2 ][ 0 ] * det2_45_45 - mat[ 2 ][ 4 ] * det2_45_05 + mat[ 2 ][ 5 ] * det2_45_04;
	float det3_245_123 = mat[ 2 ][ 1 ] * det2_45_23 - mat[ 2 ][ 2 ] * det2_45_13 + mat[ 2 ][ 3 ] * det2_45_12;
	float det3_245_124 = mat[ 2 ][ 1 ] * det2_45_24 - mat[ 2 ][ 2 ] * det2_45_14 + mat[ 2 ][ 4 ] * det2_45_12;
	float det3_245_125 = mat[ 2 ][ 1 ] * det2_45_25 - mat[ 2 ][ 2 ] * det2_45_15 + mat[ 2 ][ 5 ] * det2_45_12;
	float det3_245_134 = mat[ 2 ][ 1 ] * det2_45_34 - mat[ 2 ][ 3 ] * det2_45_14 + mat[ 2 ][ 4 ] * det2_45_13;
	float det3_245_135 = mat[ 2 ][ 1 ] * det2_45_35 - mat[ 2 ][ 3 ] * det2_45_15 + mat[ 2 ][ 5 ] * det2_45_13;
	float det3_245_145 = mat[ 2 ][ 1 ] * det2_45_45 - mat[ 2 ][ 4 ] * det2_45_15 + mat[ 2 ][ 5 ] * det2_45_14;
	float det3_245_234 = mat[ 2 ][ 2 ] * det2_45_34 - mat[ 2 ][ 3 ] * det2_45_24 + mat[ 2 ][ 4 ] * det2_45_23;
	float det3_245_235 = mat[ 2 ][ 2 ] * det2_45_35 - mat[ 2 ][ 3 ] * det2_45_25 + mat[ 2 ][ 5 ] * det2_45_23;
	float det3_245_245 = mat[ 2 ][ 2 ] * det2_45_45 - mat[ 2 ][ 4 ] * det2_45_25 + mat[ 2 ][ 5 ] * det2_45_24;
	float det3_245_345 = mat[ 2 ][ 3 ] * det2_45_45 - mat[ 2 ][ 4 ] * det2_45_35 + mat[ 2 ][ 5 ] * det2_45_34;

	// remaining 4x4 sub-determinants
	float det4_1234_0123 = mat[ 1 ][ 0 ] * det3_234_123 - mat[ 1 ][ 1 ] * det3_234_023 + mat[ 1 ][ 2 ] * det3_234_013 - mat[ 1 ][ 3 ] * det3_234_012;
	float det4_1234_0124 = mat[ 1 ][ 0 ] * det3_234_124 - mat[ 1 ][ 1 ] * det3_234_024 + mat[ 1 ][ 2 ] * det3_234_014 - mat[ 1 ][ 4 ] * det3_234_012;
	float det4_1234_0125 = mat[ 1 ][ 0 ] * det3_234_125 - mat[ 1 ][ 1 ] * det3_234_025 + mat[ 1 ][ 2 ] * det3_234_015 - mat[ 1 ][ 5 ] * det3_234_012;
	float det4_1234_0134 = mat[ 1 ][ 0 ] * det3_234_134 - mat[ 1 ][ 1 ] * det3_234_034 + mat[ 1 ][ 3 ] * det3_234_014 - mat[ 1 ][ 4 ] * det3_234_013;
	float det4_1234_0135 = mat[ 1 ][ 0 ] * det3_234_135 - mat[ 1 ][ 1 ] * det3_234_035 + mat[ 1 ][ 3 ] * det3_234_015 - mat[ 1 ][ 5 ] * det3_234_013;
	float det4_1234_0145 = mat[ 1 ][ 0 ] * det3_234_145 - mat[ 1 ][ 1 ] * det3_234_045 + mat[ 1 ][ 4 ] * det3_234_015 - mat[ 1 ][ 5 ] * det3_234_014;
	float det4_1234_0234 = mat[ 1 ][ 0 ] * det3_234_234 - mat[ 1 ][ 2 ] * det3_234_034 + mat[ 1 ][ 3 ] * det3_234_024 - mat[ 1 ][ 4 ] * det3_234_023;
	float det4_1234_0235 = mat[ 1 ][ 0 ] * det3_234_235 - mat[ 1 ][ 2 ] * det3_234_035 + mat[ 1 ][ 3 ] * det3_234_025 - mat[ 1 ][ 5 ] * det3_234_023;
	float det4_1234_0245 = mat[ 1 ][ 0 ] * det3_234_245 - mat[ 1 ][ 2 ] * det3_234_045 + mat[ 1 ][ 4 ] * det3_234_025 - mat[ 1 ][ 5 ] * det3_234_024;
	float det4_1234_0345 = mat[ 1 ][ 0 ] * det3_234_345 - mat[ 1 ][ 3 ] * det3_234_045 + mat[ 1 ][ 4 ] * det3_234_035 - mat[ 1 ][ 5 ] * det3_234_034;
	float det4_1234_1234 = mat[ 1 ][ 1 ] * det3_234_234 - mat[ 1 ][ 2 ] * det3_234_134 + mat[ 1 ][ 3 ] * det3_234_124 - mat[ 1 ][ 4 ] * det3_234_123;
	float det4_1234_1235 = mat[ 1 ][ 1 ] * det3_234_235 - mat[ 1 ][ 2 ] * det3_234_135 + mat[ 1 ][ 3 ] * det3_234_125 - mat[ 1 ][ 5 ] * det3_234_123;
	float det4_1234_1245 = mat[ 1 ][ 1 ] * det3_234_245 - mat[ 1 ][ 2 ] * det3_234_145 + mat[ 1 ][ 4 ] * det3_234_125 - mat[ 1 ][ 5 ] * det3_234_124;
	float det4_1234_1345 = mat[ 1 ][ 1 ] * det3_234_345 - mat[ 1 ][ 3 ] * det3_234_145 + mat[ 1 ][ 4 ] * det3_234_135 - mat[ 1 ][ 5 ] * det3_234_134;
	float det4_1234_2345 = mat[ 1 ][ 2 ] * det3_234_345 - mat[ 1 ][ 3 ] * det3_234_245 + mat[ 1 ][ 4 ] * det3_234_235 - mat[ 1 ][ 5 ] * det3_234_234;
	float det4_1235_0123 = mat[ 1 ][ 0 ] * det3_235_123 - mat[ 1 ][ 1 ] * det3_235_023 + mat[ 1 ][ 2 ] * det3_235_013 - mat[ 1 ][ 3 ] * det3_235_012;
	float det4_1235_0124 = mat[ 1 ][ 0 ] * det3_235_124 - mat[ 1 ][ 1 ] * det3_235_024 + mat[ 1 ][ 2 ] * det3_235_014 - mat[ 1 ][ 4 ] * det3_235_012;
	float det4_1235_0125 = mat[ 1 ][ 0 ] * det3_235_125 - mat[ 1 ][ 1 ] * det3_235_025 + mat[ 1 ][ 2 ] * det3_235_015 - mat[ 1 ][ 5 ] * det3_235_012;
	float det4_1235_0134 = mat[ 1 ][ 0 ] * det3_235_134 - mat[ 1 ][ 1 ] * det3_235_034 + mat[ 1 ][ 3 ] * det3_235_014 - mat[ 1 ][ 4 ] * det3_235_013;
	float det4_1235_0135 = mat[ 1 ][ 0 ] * det3_235_135 - mat[ 1 ][ 1 ] * det3_235_035 + mat[ 1 ][ 3 ] * det3_235_015 - mat[ 1 ][ 5 ] * det3_235_013;
	float det4_1235_0145 = mat[ 1 ][ 0 ] * det3_235_145 - mat[ 1 ][ 1 ] * det3_235_045 + mat[ 1 ][ 4 ] * det3_235_015 - mat[ 1 ][ 5 ] * det3_235_014;
	float det4_1235_0234 = mat[ 1 ][ 0 ] * det3_235_234 - mat[ 1 ][ 2 ] * det3_235_034 + mat[ 1 ][ 3 ] * det3_235_024 - mat[ 1 ][ 4 ] * det3_235_023;
	float det4_1235_0235 = mat[ 1 ][ 0 ] * det3_235_235 - mat[ 1 ][ 2 ] * det3_235_035 + mat[ 1 ][ 3 ] * det3_235_025 - mat[ 1 ][ 5 ] * det3_235_023;
	float det4_1235_0245 = mat[ 1 ][ 0 ] * det3_235_245 - mat[ 1 ][ 2 ] * det3_235_045 + mat[ 1 ][ 4 ] * det3_235_025 - mat[ 1 ][ 5 ] * det3_235_024;
	float det4_1235_0345 = mat[ 1 ][ 0 ] * det3_235_345 - mat[ 1 ][ 3 ] * det3_235_045 + mat[ 1 ][ 4 ] * det3_235_035 - mat[ 1 ][ 5 ] * det3_235_034;
	float det4_1235_1234 = mat[ 1 ][ 1 ] * det3_235_234 - mat[ 1 ][ 2 ] * det3_235_134 + mat[ 1 ][ 3 ] * det3_235_124 - mat[ 1 ][ 4 ] * det3_235_123;
	float det4_1235_1235 = mat[ 1 ][ 1 ] * det3_235_235 - mat[ 1 ][ 2 ] * det3_235_135 + mat[ 1 ][ 3 ] * det3_235_125 - mat[ 1 ][ 5 ] * det3_235_123;
	float det4_1235_1245 = mat[ 1 ][ 1 ] * det3_235_245 - mat[ 1 ][ 2 ] * det3_235_145 + mat[ 1 ][ 4 ] * det3_235_125 - mat[ 1 ][ 5 ] * det3_235_124;
	float det4_1235_1345 = mat[ 1 ][ 1 ] * det3_235_345 - mat[ 1 ][ 3 ] * det3_235_145 + mat[ 1 ][ 4 ] * det3_235_135 - mat[ 1 ][ 5 ] * det3_235_134;
	float det4_1235_2345 = mat[ 1 ][ 2 ] * det3_235_345 - mat[ 1 ][ 3 ] * det3_235_245 + mat[ 1 ][ 4 ] * det3_235_235 - mat[ 1 ][ 5 ] * det3_235_234;
	float det4_1245_0123 = mat[ 1 ][ 0 ] * det3_245_123 - mat[ 1 ][ 1 ] * det3_245_023 + mat[ 1 ][ 2 ] * det3_245_013 - mat[ 1 ][ 3 ] * det3_245_012;
	float det4_1245_0124 = mat[ 1 ][ 0 ] * det3_245_124 - mat[ 1 ][ 1 ] * det3_245_024 + mat[ 1 ][ 2 ] * det3_245_014 - mat[ 1 ][ 4 ] * det3_245_012;
	float det4_1245_0125 = mat[ 1 ][ 0 ] * det3_245_125 - mat[ 1 ][ 1 ] * det3_245_025 + mat[ 1 ][ 2 ] * det3_245_015 - mat[ 1 ][ 5 ] * det3_245_012;
	float det4_1245_0134 = mat[ 1 ][ 0 ] * det3_245_134 - mat[ 1 ][ 1 ] * det3_245_034 + mat[ 1 ][ 3 ] * det3_245_014 - mat[ 1 ][ 4 ] * det3_245_013;
	float det4_1245_0135 = mat[ 1 ][ 0 ] * det3_245_135 - mat[ 1 ][ 1 ] * det3_245_035 + mat[ 1 ][ 3 ] * det3_245_015 - mat[ 1 ][ 5 ] * det3_245_013;
	float det4_1245_0145 = mat[ 1 ][ 0 ] * det3_245_145 - mat[ 1 ][ 1 ] * det3_245_045 + mat[ 1 ][ 4 ] * det3_245_015 - mat[ 1 ][ 5 ] * det3_245_014;
	float det4_1245_0234 = mat[ 1 ][ 0 ] * det3_245_234 - mat[ 1 ][ 2 ] * det3_245_034 + mat[ 1 ][ 3 ] * det3_245_024 - mat[ 1 ][ 4 ] * det3_245_023;
	float det4_1245_0235 = mat[ 1 ][ 0 ] * det3_245_235 - mat[ 1 ][ 2 ] * det3_245_035 + mat[ 1 ][ 3 ] * det3_245_025 - mat[ 1 ][ 5 ] * det3_245_023;
	float det4_1245_0245 = mat[ 1 ][ 0 ] * det3_245_245 - mat[ 1 ][ 2 ] * det3_245_045 + mat[ 1 ][ 4 ] * det3_245_025 - mat[ 1 ][ 5 ] * det3_245_024;
	float det4_1245_0345 = mat[ 1 ][ 0 ] * det3_245_345 - mat[ 1 ][ 3 ] * det3_245_045 + mat[ 1 ][ 4 ] * det3_245_035 - mat[ 1 ][ 5 ] * det3_245_034;
	float det4_1245_1234 = mat[ 1 ][ 1 ] * det3_245_234 - mat[ 1 ][ 2 ] * det3_245_134 + mat[ 1 ][ 3 ] * det3_245_124 - mat[ 1 ][ 4 ] * det3_245_123;
	float det4_1245_1235 = mat[ 1 ][ 1 ] * det3_245_235 - mat[ 1 ][ 2 ] * det3_245_135 + mat[ 1 ][ 3 ] * det3_245_125 - mat[ 1 ][ 5 ] * det3_245_123;
	float det4_1245_1245 = mat[ 1 ][ 1 ] * det3_245_245 - mat[ 1 ][ 2 ] * det3_245_145 + mat[ 1 ][ 4 ] * det3_245_125 - mat[ 1 ][ 5 ] * det3_245_124;
	float det4_1245_1345 = mat[ 1 ][ 1 ] * det3_245_345 - mat[ 1 ][ 3 ] * det3_245_145 + mat[ 1 ][ 4 ] * det3_245_135 - mat[ 1 ][ 5 ] * det3_245_134;
	float det4_1245_2345 = mat[ 1 ][ 2 ] * det3_245_345 - mat[ 1 ][ 3 ] * det3_245_245 + mat[ 1 ][ 4 ] * det3_245_235 - mat[ 1 ][ 5 ] * det3_245_234;
	float det4_1345_0123 = mat[ 1 ][ 0 ] * det3_345_123 - mat[ 1 ][ 1 ] * det3_345_023 + mat[ 1 ][ 2 ] * det3_345_013 - mat[ 1 ][ 3 ] * det3_345_012;
	float det4_1345_0124 = mat[ 1 ][ 0 ] * det3_345_124 - mat[ 1 ][ 1 ] * det3_345_024 + mat[ 1 ][ 2 ] * det3_345_014 - mat[ 1 ][ 4 ] * det3_345_012;
	float det4_1345_0125 = mat[ 1 ][ 0 ] * det3_345_125 - mat[ 1 ][ 1 ] * det3_345_025 + mat[ 1 ][ 2 ] * det3_345_015 - mat[ 1 ][ 5 ] * det3_345_012;
	float det4_1345_0134 = mat[ 1 ][ 0 ] * det3_345_134 - mat[ 1 ][ 1 ] * det3_345_034 + mat[ 1 ][ 3 ] * det3_345_014 - mat[ 1 ][ 4 ] * det3_345_013;
	float det4_1345_0135 = mat[ 1 ][ 0 ] * det3_345_135 - mat[ 1 ][ 1 ] * det3_345_035 + mat[ 1 ][ 3 ] * det3_345_015 - mat[ 1 ][ 5 ] * det3_345_013;
	float det4_1345_0145 = mat[ 1 ][ 0 ] * det3_345_145 - mat[ 1 ][ 1 ] * det3_345_045 + mat[ 1 ][ 4 ] * det3_345_015 - mat[ 1 ][ 5 ] * det3_345_014;
	float det4_1345_0234 = mat[ 1 ][ 0 ] * det3_345_234 - mat[ 1 ][ 2 ] * det3_345_034 + mat[ 1 ][ 3 ] * det3_345_024 - mat[ 1 ][ 4 ] * det3_345_023;
	float det4_1345_0235 = mat[ 1 ][ 0 ] * det3_345_235 - mat[ 1 ][ 2 ] * det3_345_035 + mat[ 1 ][ 3 ] * det3_345_025 - mat[ 1 ][ 5 ] * det3_345_023;
	float det4_1345_0245 = mat[ 1 ][ 0 ] * det3_345_245 - mat[ 1 ][ 2 ] * det3_345_045 + mat[ 1 ][ 4 ] * det3_345_025 - mat[ 1 ][ 5 ] * det3_345_024;
	float det4_1345_0345 = mat[ 1 ][ 0 ] * det3_345_345 - mat[ 1 ][ 3 ] * det3_345_045 + mat[ 1 ][ 4 ] * det3_345_035 - mat[ 1 ][ 5 ] * det3_345_034;
	float det4_1345_1234 = mat[ 1 ][ 1 ] * det3_345_234 - mat[ 1 ][ 2 ] * det3_345_134 + mat[ 1 ][ 3 ] * det3_345_124 - mat[ 1 ][ 4 ] * det3_345_123;
	float det4_1345_1235 = mat[ 1 ][ 1 ] * det3_345_235 - mat[ 1 ][ 2 ] * det3_345_135 + mat[ 1 ][ 3 ] * det3_345_125 - mat[ 1 ][ 5 ] * det3_345_123;
	float det4_1345_1245 = mat[ 1 ][ 1 ] * det3_345_245 - mat[ 1 ][ 2 ] * det3_345_145 + mat[ 1 ][ 4 ] * det3_345_125 - mat[ 1 ][ 5 ] * det3_345_124;
	float det4_1345_1345 = mat[ 1 ][ 1 ] * det3_345_345 - mat[ 1 ][ 3 ] * det3_345_145 + mat[ 1 ][ 4 ] * det3_345_135 - mat[ 1 ][ 5 ] * det3_345_134;
	float det4_1345_2345 = mat[ 1 ][ 2 ] * det3_345_345 - mat[ 1 ][ 3 ] * det3_345_245 + mat[ 1 ][ 4 ] * det3_345_235 - mat[ 1 ][ 5 ] * det3_345_234;

	// remaining 5x5 sub-determinants
	float det5_01234_01234 = mat[ 0 ][ 0 ] * det4_1234_1234 - mat[ 0 ][ 1 ] * det4_1234_0234 + mat[ 0 ][ 2 ] * det4_1234_0134 - mat[ 0 ][ 3 ] * det4_1234_0124 + mat[ 0 ][ 4 ] * det4_1234_0123;
	float det5_01234_01235 = mat[ 0 ][ 0 ] * det4_1234_1235 - mat[ 0 ][ 1 ] * det4_1234_0235 + mat[ 0 ][ 2 ] * det4_1234_0135 - mat[ 0 ][ 3 ] * det4_1234_0125 + mat[ 0 ][ 5 ] * det4_1234_0123;
	float det5_01234_01245 = mat[ 0 ][ 0 ] * det4_1234_1245 - mat[ 0 ][ 1 ] * det4_1234_0245 + mat[ 0 ][ 2 ] * det4_1234_0145 - mat[ 0 ][ 4 ] * det4_1234_0125 + mat[ 0 ][ 5 ] * det4_1234_0124;
	float det5_01234_01345 = mat[ 0 ][ 0 ] * det4_1234_1345 - mat[ 0 ][ 1 ] * det4_1234_0345 + mat[ 0 ][ 3 ] * det4_1234_0145 - mat[ 0 ][ 4 ] * det4_1234_0135 + mat[ 0 ][ 5 ] * det4_1234_0134;
	float det5_01234_02345 = mat[ 0 ][ 0 ] * det4_1234_2345 - mat[ 0 ][ 2 ] * det4_1234_0345 + mat[ 0 ][ 3 ] * det4_1234_0245 - mat[ 0 ][ 4 ] * det4_1234_0235 + mat[ 0 ][ 5 ] * det4_1234_0234;
	float det5_01234_12345 = mat[ 0 ][ 1 ] * det4_1234_2345 - mat[ 0 ][ 2 ] * det4_1234_1345 + mat[ 0 ][ 3 ] * det4_1234_1245 - mat[ 0 ][ 4 ] * det4_1234_1235 + mat[ 0 ][ 5 ] * det4_1234_1234;
	float det5_01235_01234 = mat[ 0 ][ 0 ] * det4_1235_1234 - mat[ 0 ][ 1 ] * det4_1235_0234 + mat[ 0 ][ 2 ] * det4_1235_0134 - mat[ 0 ][ 3 ] * det4_1235_0124 + mat[ 0 ][ 4 ] * det4_1235_0123;
	float det5_01235_01235 = mat[ 0 ][ 0 ] * det4_1235_1235 - mat[ 0 ][ 1 ] * det4_1235_0235 + mat[ 0 ][ 2 ] * det4_1235_0135 - mat[ 0 ][ 3 ] * det4_1235_0125 + mat[ 0 ][ 5 ] * det4_1235_0123;
	float det5_01235_01245 = mat[ 0 ][ 0 ] * det4_1235_1245 - mat[ 0 ][ 1 ] * det4_1235_0245 + mat[ 0 ][ 2 ] * det4_1235_0145 - mat[ 0 ][ 4 ] * det4_1235_0125 + mat[ 0 ][ 5 ] * det4_1235_0124;
	float det5_01235_01345 = mat[ 0 ][ 0 ] * det4_1235_1345 - mat[ 0 ][ 1 ] * det4_1235_0345 + mat[ 0 ][ 3 ] * det4_1235_0145 - mat[ 0 ][ 4 ] * det4_1235_0135 + mat[ 0 ][ 5 ] * det4_1235_0134;
	float det5_01235_02345 = mat[ 0 ][ 0 ] * det4_1235_2345 - mat[ 0 ][ 2 ] * det4_1235_0345 + mat[ 0 ][ 3 ] * det4_1235_0245 - mat[ 0 ][ 4 ] * det4_1235_0235 + mat[ 0 ][ 5 ] * det4_1235_0234;
	float det5_01235_12345 = mat[ 0 ][ 1 ] * det4_1235_2345 - mat[ 0 ][ 2 ] * det4_1235_1345 + mat[ 0 ][ 3 ] * det4_1235_1245 - mat[ 0 ][ 4 ] * det4_1235_1235 + mat[ 0 ][ 5 ] * det4_1235_1234;
	float det5_01245_01234 = mat[ 0 ][ 0 ] * det4_1245_1234 - mat[ 0 ][ 1 ] * det4_1245_0234 + mat[ 0 ][ 2 ] * det4_1245_0134 - mat[ 0 ][ 3 ] * det4_1245_0124 + mat[ 0 ][ 4 ] * det4_1245_0123;
	float det5_01245_01235 = mat[ 0 ][ 0 ] * det4_1245_1235 - mat[ 0 ][ 1 ] * det4_1245_0235 + mat[ 0 ][ 2 ] * det4_1245_0135 - mat[ 0 ][ 3 ] * det4_1245_0125 + mat[ 0 ][ 5 ] * det4_1245_0123;
	float det5_01245_01245 = mat[ 0 ][ 0 ] * det4_1245_1245 - mat[ 0 ][ 1 ] * det4_1245_0245 + mat[ 0 ][ 2 ] * det4_1245_0145 - mat[ 0 ][ 4 ] * det4_1245_0125 + mat[ 0 ][ 5 ] * det4_1245_0124;
	float det5_01245_01345 = mat[ 0 ][ 0 ] * det4_1245_1345 - mat[ 0 ][ 1 ] * det4_1245_0345 + mat[ 0 ][ 3 ] * det4_1245_0145 - mat[ 0 ][ 4 ] * det4_1245_0135 + mat[ 0 ][ 5 ] * det4_1245_0134;
	float det5_01245_02345 = mat[ 0 ][ 0 ] * det4_1245_2345 - mat[ 0 ][ 2 ] * det4_1245_0345 + mat[ 0 ][ 3 ] * det4_1245_0245 - mat[ 0 ][ 4 ] * det4_1245_0235 + mat[ 0 ][ 5 ] * det4_1245_0234;
	float det5_01245_12345 = mat[ 0 ][ 1 ] * det4_1245_2345 - mat[ 0 ][ 2 ] * det4_1245_1345 + mat[ 0 ][ 3 ] * det4_1245_1245 - mat[ 0 ][ 4 ] * det4_1245_1235 + mat[ 0 ][ 5 ] * det4_1245_1234;
	float det5_01345_01234 = mat[ 0 ][ 0 ] * det4_1345_1234 - mat[ 0 ][ 1 ] * det4_1345_0234 + mat[ 0 ][ 2 ] * det4_1345_0134 - mat[ 0 ][ 3 ] * det4_1345_0124 + mat[ 0 ][ 4 ] * det4_1345_0123;
	float det5_01345_01235 = mat[ 0 ][ 0 ] * det4_1345_1235 - mat[ 0 ][ 1 ] * det4_1345_0235 + mat[ 0 ][ 2 ] * det4_1345_0135 - mat[ 0 ][ 3 ] * det4_1345_0125 + mat[ 0 ][ 5 ] * det4_1345_0123;
	float det5_01345_01245 = mat[ 0 ][ 0 ] * det4_1345_1245 - mat[ 0 ][ 1 ] * det4_1345_0245 + mat[ 0 ][ 2 ] * det4_1345_0145 - mat[ 0 ][ 4 ] * det4_1345_0125 + mat[ 0 ][ 5 ] * det4_1345_0124;
	float det5_01345_01345 = mat[ 0 ][ 0 ] * det4_1345_1345 - mat[ 0 ][ 1 ] * det4_1345_0345 + mat[ 0 ][ 3 ] * det4_1345_0145 - mat[ 0 ][ 4 ] * det4_1345_0135 + mat[ 0 ][ 5 ] * det4_1345_0134;
	float det5_01345_02345 = mat[ 0 ][ 0 ] * det4_1345_2345 - mat[ 0 ][ 2 ] * det4_1345_0345 + mat[ 0 ][ 3 ] * det4_1345_0245 - mat[ 0 ][ 4 ] * det4_1345_0235 + mat[ 0 ][ 5 ] * det4_1345_0234;
	float det5_01345_12345 = mat[ 0 ][ 1 ] * det4_1345_2345 - mat[ 0 ][ 2 ] * det4_1345_1345 + mat[ 0 ][ 3 ] * det4_1345_1245 - mat[ 0 ][ 4 ] * det4_1345_1235 + mat[ 0 ][ 5 ] * det4_1345_1234;
	float det5_02345_01234 = mat[ 0 ][ 0 ] * det4_2345_1234 - mat[ 0 ][ 1 ] * det4_2345_0234 + mat[ 0 ][ 2 ] * det4_2345_0134 - mat[ 0 ][ 3 ] * det4_2345_0124 + mat[ 0 ][ 4 ] * det4_2345_0123;
	float det5_02345_01235 = mat[ 0 ][ 0 ] * det4_2345_1235 - mat[ 0 ][ 1 ] * det4_2345_0235 + mat[ 0 ][ 2 ] * det4_2345_0135 - mat[ 0 ][ 3 ] * det4_2345_0125 + mat[ 0 ][ 5 ] * det4_2345_0123;
	float det5_02345_01245 = mat[ 0 ][ 0 ] * det4_2345_1245 - mat[ 0 ][ 1 ] * det4_2345_0245 + mat[ 0 ][ 2 ] * det4_2345_0145 - mat[ 0 ][ 4 ] * det4_2345_0125 + mat[ 0 ][ 5 ] * det4_2345_0124;
	float det5_02345_01345 = mat[ 0 ][ 0 ] * det4_2345_1345 - mat[ 0 ][ 1 ] * det4_2345_0345 + mat[ 0 ][ 3 ] * det4_2345_0145 - mat[ 0 ][ 4 ] * det4_2345_0135 + mat[ 0 ][ 5 ] * det4_2345_0134;
	float det5_02345_02345 = mat[ 0 ][ 0 ] * det4_2345_2345 - mat[ 0 ][ 2 ] * det4_2345_0345 + mat[ 0 ][ 3 ] * det4_2345_0245 - mat[ 0 ][ 4 ] * det4_2345_0235 + mat[ 0 ][ 5 ] * det4_2345_0234;
	float det5_02345_12345 = mat[ 0 ][ 1 ] * det4_2345_2345 - mat[ 0 ][ 2 ] * det4_2345_1345 + mat[ 0 ][ 3 ] * det4_2345_1245 - mat[ 0 ][ 4 ] * det4_2345_1235 + mat[ 0 ][ 5 ] * det4_2345_1234;

	mat[ 0 ][ 0 ] =  det5_12345_12345 * invDet;
	mat[ 0 ][ 1 ] = -det5_02345_12345 * invDet;
	mat[ 0 ][ 2 ] =  det5_01345_12345 * invDet;
	mat[ 0 ][ 3 ] = -det5_01245_12345 * invDet;
	mat[ 0 ][ 4 ] =  det5_01235_12345 * invDet;
	mat[ 0 ][ 5 ] = -det5_01234_12345 * invDet;

	mat[ 1 ][ 0 ] = -det5_12345_02345 * invDet;
	mat[ 1 ][ 1 ] =  det5_02345_02345 * invDet;
	mat[ 1 ][ 2 ] = -det5_01345_02345 * invDet;
	mat[ 1 ][ 3 ] =  det5_01245_02345 * invDet;
	mat[ 1 ][ 4 ] = -det5_01235_02345 * invDet;
	mat[ 1 ][ 5 ] =  det5_01234_02345 * invDet;

	mat[ 2 ][ 0 ] =  det5_12345_01345 * invDet;
	mat[ 2 ][ 1 ] = -det5_02345_01345 * invDet;
	mat[ 2 ][ 2 ] =  det5_01345_01345 * invDet;
	mat[ 2 ][ 3 ] = -det5_01245_01345 * invDet;
	mat[ 2 ][ 4 ] =  det5_01235_01345 * invDet;
	mat[ 2 ][ 5 ] = -det5_01234_01345 * invDet;

	mat[ 3 ][ 0 ] = -det5_12345_01245 * invDet;
	mat[ 3 ][ 1 ] =  det5_02345_01245 * invDet;
	mat[ 3 ][ 2 ] = -det5_01345_01245 * invDet;
	mat[ 3 ][ 3 ] =  det5_01245_01245 * invDet;
	mat[ 3 ][ 4 ] = -det5_01235_01245 * invDet;
	mat[ 3 ][ 5 ] =  det5_01234_01245 * invDet;

	mat[ 4 ][ 0 ] =  det5_12345_01235 * invDet;
	mat[ 4 ][ 1 ] = -det5_02345_01235 * invDet;
	mat[ 4 ][ 2 ] =  det5_01345_01235 * invDet;
	mat[ 4 ][ 3 ] = -det5_01245_01235 * invDet;
	mat[ 4 ][ 4 ] =  det5_01235_01235 * invDet;
	mat[ 4 ][ 5 ] = -det5_01234_01235 * invDet;

	mat[ 5 ][ 0 ] = -det5_12345_01234 * invDet;
	mat[ 5 ][ 1 ] =  det5_02345_01234 * invDet;
	mat[ 5 ][ 2 ] = -det5_01345_01234 * invDet;
	mat[ 5 ][ 3 ] =  det5_01245_01234 * invDet;
	mat[ 5 ][ 4 ] = -det5_01235_01234 * invDet;
	mat[ 5 ][ 5 ] =  det5_01234_01234 * invDet;

	return true;
}

/*
============
Mat6::InverseFastSelf
============
*/
INLINE bool Mat6::InverseFastSelf( ) {
#if 0
	// 810+6+36 = 852 multiplications
	//				1 division
	float det, invDet;

	// 2x2 sub-determinants required to calculate 6x6 determinant
	float det2_45_01 = mat[ 4 ][ 0 ] * mat[ 5 ][ 1 ] - mat[ 4 ][ 1 ] * mat[ 5 ][ 0 ];
	float det2_45_02 = mat[ 4 ][ 0 ] * mat[ 5 ][ 2 ] - mat[ 4 ][ 2 ] * mat[ 5 ][ 0 ];
	float det2_45_03 = mat[ 4 ][ 0 ] * mat[ 5 ][ 3 ] - mat[ 4 ][ 3 ] * mat[ 5 ][ 0 ];
	float det2_45_04 = mat[ 4 ][ 0 ] * mat[ 5 ][ 4 ] - mat[ 4 ][ 4 ] * mat[ 5 ][ 0 ];
	float det2_45_05 = mat[ 4 ][ 0 ] * mat[ 5 ][ 5 ] - mat[ 4 ][ 5 ] * mat[ 5 ][ 0 ];
	float det2_45_12 = mat[ 4 ][ 1 ] * mat[ 5 ][ 2 ] - mat[ 4 ][ 2 ] * mat[ 5 ][ 1 ];
	float det2_45_13 = mat[ 4 ][ 1 ] * mat[ 5 ][ 3 ] - mat[ 4 ][ 3 ] * mat[ 5 ][ 1 ];
	float det2_45_14 = mat[ 4 ][ 1 ] * mat[ 5 ][ 4 ] - mat[ 4 ][ 4 ] * mat[ 5 ][ 1 ];
	float det2_45_15 = mat[ 4 ][ 1 ] * mat[ 5 ][ 5 ] - mat[ 4 ][ 5 ] * mat[ 5 ][ 1 ];
	float det2_45_23 = mat[ 4 ][ 2 ] * mat[ 5 ][ 3 ] - mat[ 4 ][ 3 ] * mat[ 5 ][ 2 ];
	float det2_45_24 = mat[ 4 ][ 2 ] * mat[ 5 ][ 4 ] - mat[ 4 ][ 4 ] * mat[ 5 ][ 2 ];
	float det2_45_25 = mat[ 4 ][ 2 ] * mat[ 5 ][ 5 ] - mat[ 4 ][ 5 ] * mat[ 5 ][ 2 ];
	float det2_45_34 = mat[ 4 ][ 3 ] * mat[ 5 ][ 4 ] - mat[ 4 ][ 4 ] * mat[ 5 ][ 3 ];
	float det2_45_35 = mat[ 4 ][ 3 ] * mat[ 5 ][ 5 ] - mat[ 4 ][ 5 ] * mat[ 5 ][ 3 ];
	float det2_45_45 = mat[ 4 ][ 4 ] * mat[ 5 ][ 5 ] - mat[ 4 ][ 5 ] * mat[ 5 ][ 4 ];

	// 3x3 sub-determinants required to calculate 6x6 determinant
	float det3_345_012 = mat[ 3 ][ 0 ] * det2_45_12 - mat[ 3 ][ 1 ] * det2_45_02 + mat[ 3 ][ 2 ] * det2_45_01;
	float det3_345_013 = mat[ 3 ][ 0 ] * det2_45_13 - mat[ 3 ][ 1 ] * det2_45_03 + mat[ 3 ][ 3 ] * det2_45_01;
	float det3_345_014 = mat[ 3 ][ 0 ] * det2_45_14 - mat[ 3 ][ 1 ] * det2_45_04 + mat[ 3 ][ 4 ] * det2_45_01;
	float det3_345_015 = mat[ 3 ][ 0 ] * det2_45_15 - mat[ 3 ][ 1 ] * det2_45_05 + mat[ 3 ][ 5 ] * det2_45_01;
	float det3_345_023 = mat[ 3 ][ 0 ] * det2_45_23 - mat[ 3 ][ 2 ] * det2_45_03 + mat[ 3 ][ 3 ] * det2_45_02;
	float det3_345_024 = mat[ 3 ][ 0 ] * det2_45_24 - mat[ 3 ][ 2 ] * det2_45_04 + mat[ 3 ][ 4 ] * det2_45_02;
	float det3_345_025 = mat[ 3 ][ 0 ] * det2_45_25 - mat[ 3 ][ 2 ] * det2_45_05 + mat[ 3 ][ 5 ] * det2_45_02;
	float det3_345_034 = mat[ 3 ][ 0 ] * det2_45_34 - mat[ 3 ][ 3 ] * det2_45_04 + mat[ 3 ][ 4 ] * det2_45_03;
	float det3_345_035 = mat[ 3 ][ 0 ] * det2_45_35 - mat[ 3 ][ 3 ] * det2_45_05 + mat[ 3 ][ 5 ] * det2_45_03;
	float det3_345_045 = mat[ 3 ][ 0 ] * det2_45_45 - mat[ 3 ][ 4 ] * det2_45_05 + mat[ 3 ][ 5 ] * det2_45_04;
	float det3_345_123 = mat[ 3 ][ 1 ] * det2_45_23 - mat[ 3 ][ 2 ] * det2_45_13 + mat[ 3 ][ 3 ] * det2_45_12;
	float det3_345_124 = mat[ 3 ][ 1 ] * det2_45_24 - mat[ 3 ][ 2 ] * det2_45_14 + mat[ 3 ][ 4 ] * det2_45_12;
	float det3_345_125 = mat[ 3 ][ 1 ] * det2_45_25 - mat[ 3 ][ 2 ] * det2_45_15 + mat[ 3 ][ 5 ] * det2_45_12;
	float det3_345_134 = mat[ 3 ][ 1 ] * det2_45_34 - mat[ 3 ][ 3 ] * det2_45_14 + mat[ 3 ][ 4 ] * det2_45_13;
	float det3_345_135 = mat[ 3 ][ 1 ] * det2_45_35 - mat[ 3 ][ 3 ] * det2_45_15 + mat[ 3 ][ 5 ] * det2_45_13;
	float det3_345_145 = mat[ 3 ][ 1 ] * det2_45_45 - mat[ 3 ][ 4 ] * det2_45_15 + mat[ 3 ][ 5 ] * det2_45_14;
	float det3_345_234 = mat[ 3 ][ 2 ] * det2_45_34 - mat[ 3 ][ 3 ] * det2_45_24 + mat[ 3 ][ 4 ] * det2_45_23;
	float det3_345_235 = mat[ 3 ][ 2 ] * det2_45_35 - mat[ 3 ][ 3 ] * det2_45_25 + mat[ 3 ][ 5 ] * det2_45_23;
	float det3_345_245 = mat[ 3 ][ 2 ] * det2_45_45 - mat[ 3 ][ 4 ] * det2_45_25 + mat[ 3 ][ 5 ] * det2_45_24;
	float det3_345_345 = mat[ 3 ][ 3 ] * det2_45_45 - mat[ 3 ][ 4 ] * det2_45_35 + mat[ 3 ][ 5 ] * det2_45_34;

	// 4x4 sub-determinants required to calculate 6x6 determinant
	float det4_2345_0123 = mat[ 2 ][ 0 ] * det3_345_123 - mat[ 2 ][ 1 ] * det3_345_023 + mat[ 2 ][ 2 ] * det3_345_013 - mat[ 2 ][ 3 ] * det3_345_012;
	float det4_2345_0124 = mat[ 2 ][ 0 ] * det3_345_124 - mat[ 2 ][ 1 ] * det3_345_024 + mat[ 2 ][ 2 ] * det3_345_014 - mat[ 2 ][ 4 ] * det3_345_012;
	float det4_2345_0125 = mat[ 2 ][ 0 ] * det3_345_125 - mat[ 2 ][ 1 ] * det3_345_025 + mat[ 2 ][ 2 ] * det3_345_015 - mat[ 2 ][ 5 ] * det3_345_012;
	float det4_2345_0134 = mat[ 2 ][ 0 ] * det3_345_134 - mat[ 2 ][ 1 ] * det3_345_034 + mat[ 2 ][ 3 ] * det3_345_014 - mat[ 2 ][ 4 ] * det3_345_013;
	float det4_2345_0135 = mat[ 2 ][ 0 ] * det3_345_135 - mat[ 2 ][ 1 ] * det3_345_035 + mat[ 2 ][ 3 ] * det3_345_015 - mat[ 2 ][ 5 ] * det3_345_013;
	float det4_2345_0145 = mat[ 2 ][ 0 ] * det3_345_145 - mat[ 2 ][ 1 ] * det3_345_045 + mat[ 2 ][ 4 ] * det3_345_015 - mat[ 2 ][ 5 ] * det3_345_014;
	float det4_2345_0234 = mat[ 2 ][ 0 ] * det3_345_234 - mat[ 2 ][ 2 ] * det3_345_034 + mat[ 2 ][ 3 ] * det3_345_024 - mat[ 2 ][ 4 ] * det3_345_023;
	float det4_2345_0235 = mat[ 2 ][ 0 ] * det3_345_235 - mat[ 2 ][ 2 ] * det3_345_035 + mat[ 2 ][ 3 ] * det3_345_025 - mat[ 2 ][ 5 ] * det3_345_023;
	float det4_2345_0245 = mat[ 2 ][ 0 ] * det3_345_245 - mat[ 2 ][ 2 ] * det3_345_045 + mat[ 2 ][ 4 ] * det3_345_025 - mat[ 2 ][ 5 ] * det3_345_024;
	float det4_2345_0345 = mat[ 2 ][ 0 ] * det3_345_345 - mat[ 2 ][ 3 ] * det3_345_045 + mat[ 2 ][ 4 ] * det3_345_035 - mat[ 2 ][ 5 ] * det3_345_034;
	float det4_2345_1234 = mat[ 2 ][ 1 ] * det3_345_234 - mat[ 2 ][ 2 ] * det3_345_134 + mat[ 2 ][ 3 ] * det3_345_124 - mat[ 2 ][ 4 ] * det3_345_123;
	float det4_2345_1235 = mat[ 2 ][ 1 ] * det3_345_235 - mat[ 2 ][ 2 ] * det3_345_135 + mat[ 2 ][ 3 ] * det3_345_125 - mat[ 2 ][ 5 ] * det3_345_123;
	float det4_2345_1245 = mat[ 2 ][ 1 ] * det3_345_245 - mat[ 2 ][ 2 ] * det3_345_145 + mat[ 2 ][ 4 ] * det3_345_125 - mat[ 2 ][ 5 ] * det3_345_124;
	float det4_2345_1345 = mat[ 2 ][ 1 ] * det3_345_345 - mat[ 2 ][ 3 ] * det3_345_145 + mat[ 2 ][ 4 ] * det3_345_135 - mat[ 2 ][ 5 ] * det3_345_134;
	float det4_2345_2345 = mat[ 2 ][ 2 ] * det3_345_345 - mat[ 2 ][ 3 ] * det3_345_245 + mat[ 2 ][ 4 ] * det3_345_235 - mat[ 2 ][ 5 ] * det3_345_234;

	// 5x5 sub-determinants required to calculate 6x6 determinant
	float det5_12345_01234 = mat[ 1 ][ 0 ] * det4_2345_1234 - mat[ 1 ][ 1 ] * det4_2345_0234 + mat[ 1 ][ 2 ] * det4_2345_0134 - mat[ 1 ][ 3 ] * det4_2345_0124 + mat[ 1 ][ 4 ] * det4_2345_0123;
	float det5_12345_01235 = mat[ 1 ][ 0 ] * det4_2345_1235 - mat[ 1 ][ 1 ] * det4_2345_0235 + mat[ 1 ][ 2 ] * det4_2345_0135 - mat[ 1 ][ 3 ] * det4_2345_0125 + mat[ 1 ][ 5 ] * det4_2345_0123;
	float det5_12345_01245 = mat[ 1 ][ 0 ] * det4_2345_1245 - mat[ 1 ][ 1 ] * det4_2345_0245 + mat[ 1 ][ 2 ] * det4_2345_0145 - mat[ 1 ][ 4 ] * det4_2345_0125 + mat[ 1 ][ 5 ] * det4_2345_0124;
	float det5_12345_01345 = mat[ 1 ][ 0 ] * det4_2345_1345 - mat[ 1 ][ 1 ] * det4_2345_0345 + mat[ 1 ][ 3 ] * det4_2345_0145 - mat[ 1 ][ 4 ] * det4_2345_0135 + mat[ 1 ][ 5 ] * det4_2345_0134;
	float det5_12345_02345 = mat[ 1 ][ 0 ] * det4_2345_2345 - mat[ 1 ][ 2 ] * det4_2345_0345 + mat[ 1 ][ 3 ] * det4_2345_0245 - mat[ 1 ][ 4 ] * det4_2345_0235 + mat[ 1 ][ 5 ] * det4_2345_0234;
	float det5_12345_12345 = mat[ 1 ][ 1 ] * det4_2345_2345 - mat[ 1 ][ 2 ] * det4_2345_1345 + mat[ 1 ][ 3 ] * det4_2345_1245 - mat[ 1 ][ 4 ] * det4_2345_1235 + mat[ 1 ][ 5 ] * det4_2345_1234;

	// determinant of 6x6 matrix
	det = mat[ 0 ][ 0 ] * det5_12345_12345 - mat[ 0 ][ 1 ] * det5_12345_02345 + mat[ 0 ][ 2 ] * det5_12345_01345 -
				mat[ 0 ][ 3 ] * det5_12345_01245 + mat[ 0 ][ 4 ] * det5_12345_01235 - mat[ 0 ][ 5 ] * det5_12345_01234;

	if( fabs( det ) < MATRIX_INVERSE_EPSILON ) {
		return false;
	}

	invDet = 1.0f / det;

	// remaining 2x2 sub-determinants
	float det2_34_01 = mat[ 3 ][ 0 ] * mat[ 4 ][ 1 ] - mat[ 3 ][ 1 ] * mat[ 4 ][ 0 ];
	float det2_34_02 = mat[ 3 ][ 0 ] * mat[ 4 ][ 2 ] - mat[ 3 ][ 2 ] * mat[ 4 ][ 0 ];
	float det2_34_03 = mat[ 3 ][ 0 ] * mat[ 4 ][ 3 ] - mat[ 3 ][ 3 ] * mat[ 4 ][ 0 ];
	float det2_34_04 = mat[ 3 ][ 0 ] * mat[ 4 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 4 ][ 0 ];
	float det2_34_05 = mat[ 3 ][ 0 ] * mat[ 4 ][ 5 ] - mat[ 3 ][ 5 ] * mat[ 4 ][ 0 ];
	float det2_34_12 = mat[ 3 ][ 1 ] * mat[ 4 ][ 2 ] - mat[ 3 ][ 2 ] * mat[ 4 ][ 1 ];
	float det2_34_13 = mat[ 3 ][ 1 ] * mat[ 4 ][ 3 ] - mat[ 3 ][ 3 ] * mat[ 4 ][ 1 ];
	float det2_34_14 = mat[ 3 ][ 1 ] * mat[ 4 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 4 ][ 1 ];
	float det2_34_15 = mat[ 3 ][ 1 ] * mat[ 4 ][ 5 ] - mat[ 3 ][ 5 ] * mat[ 4 ][ 1 ];
	float det2_34_23 = mat[ 3 ][ 2 ] * mat[ 4 ][ 3 ] - mat[ 3 ][ 3 ] * mat[ 4 ][ 2 ];
	float det2_34_24 = mat[ 3 ][ 2 ] * mat[ 4 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 4 ][ 2 ];
	float det2_34_25 = mat[ 3 ][ 2 ] * mat[ 4 ][ 5 ] - mat[ 3 ][ 5 ] * mat[ 4 ][ 2 ];
	float det2_34_34 = mat[ 3 ][ 3 ] * mat[ 4 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 4 ][ 3 ];
	float det2_34_35 = mat[ 3 ][ 3 ] * mat[ 4 ][ 5 ] - mat[ 3 ][ 5 ] * mat[ 4 ][ 3 ];
	float det2_34_45 = mat[ 3 ][ 4 ] * mat[ 4 ][ 5 ] - mat[ 3 ][ 5 ] * mat[ 4 ][ 4 ];
	float det2_35_01 = mat[ 3 ][ 0 ] * mat[ 5 ][ 1 ] - mat[ 3 ][ 1 ] * mat[ 5 ][ 0 ];
	float det2_35_02 = mat[ 3 ][ 0 ] * mat[ 5 ][ 2 ] - mat[ 3 ][ 2 ] * mat[ 5 ][ 0 ];
	float det2_35_03 = mat[ 3 ][ 0 ] * mat[ 5 ][ 3 ] - mat[ 3 ][ 3 ] * mat[ 5 ][ 0 ];
	float det2_35_04 = mat[ 3 ][ 0 ] * mat[ 5 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 5 ][ 0 ];
	float det2_35_05 = mat[ 3 ][ 0 ] * mat[ 5 ][ 5 ] - mat[ 3 ][ 5 ] * mat[ 5 ][ 0 ];
	float det2_35_12 = mat[ 3 ][ 1 ] * mat[ 5 ][ 2 ] - mat[ 3 ][ 2 ] * mat[ 5 ][ 1 ];
	float det2_35_13 = mat[ 3 ][ 1 ] * mat[ 5 ][ 3 ] - mat[ 3 ][ 3 ] * mat[ 5 ][ 1 ];
	float det2_35_14 = mat[ 3 ][ 1 ] * mat[ 5 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 5 ][ 1 ];
	float det2_35_15 = mat[ 3 ][ 1 ] * mat[ 5 ][ 5 ] - mat[ 3 ][ 5 ] * mat[ 5 ][ 1 ];
	float det2_35_23 = mat[ 3 ][ 2 ] * mat[ 5 ][ 3 ] - mat[ 3 ][ 3 ] * mat[ 5 ][ 2 ];
	float det2_35_24 = mat[ 3 ][ 2 ] * mat[ 5 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 5 ][ 2 ];
	float det2_35_25 = mat[ 3 ][ 2 ] * mat[ 5 ][ 5 ] - mat[ 3 ][ 5 ] * mat[ 5 ][ 2 ];
	float det2_35_34 = mat[ 3 ][ 3 ] * mat[ 5 ][ 4 ] - mat[ 3 ][ 4 ] * mat[ 5 ][ 3 ];
	float det2_35_35 = mat[ 3 ][ 3 ] * mat[ 5 ][ 5 ] - mat[ 3 ][ 5 ] * mat[ 5 ][ 3 ];
	float det2_35_45 = mat[ 3 ][ 4 ] * mat[ 5 ][ 5 ] - mat[ 3 ][ 5 ] * mat[ 5 ][ 4 ];

	// remaining 3x3 sub-determinants
	float det3_234_012 = mat[ 2 ][ 0 ] * det2_34_12 - mat[ 2 ][ 1 ] * det2_34_02 + mat[ 2 ][ 2 ] * det2_34_01;
	float det3_234_013 = mat[ 2 ][ 0 ] * det2_34_13 - mat[ 2 ][ 1 ] * det2_34_03 + mat[ 2 ][ 3 ] * det2_34_01;
	float det3_234_014 = mat[ 2 ][ 0 ] * det2_34_14 - mat[ 2 ][ 1 ] * det2_34_04 + mat[ 2 ][ 4 ] * det2_34_01;
	float det3_234_015 = mat[ 2 ][ 0 ] * det2_34_15 - mat[ 2 ][ 1 ] * det2_34_05 + mat[ 2 ][ 5 ] * det2_34_01;
	float det3_234_023 = mat[ 2 ][ 0 ] * det2_34_23 - mat[ 2 ][ 2 ] * det2_34_03 + mat[ 2 ][ 3 ] * det2_34_02;
	float det3_234_024 = mat[ 2 ][ 0 ] * det2_34_24 - mat[ 2 ][ 2 ] * det2_34_04 + mat[ 2 ][ 4 ] * det2_34_02;
	float det3_234_025 = mat[ 2 ][ 0 ] * det2_34_25 - mat[ 2 ][ 2 ] * det2_34_05 + mat[ 2 ][ 5 ] * det2_34_02;
	float det3_234_034 = mat[ 2 ][ 0 ] * det2_34_34 - mat[ 2 ][ 3 ] * det2_34_04 + mat[ 2 ][ 4 ] * det2_34_03;
	float det3_234_035 = mat[ 2 ][ 0 ] * det2_34_35 - mat[ 2 ][ 3 ] * det2_34_05 + mat[ 2 ][ 5 ] * det2_34_03;
	float det3_234_045 = mat[ 2 ][ 0 ] * det2_34_45 - mat[ 2 ][ 4 ] * det2_34_05 + mat[ 2 ][ 5 ] * det2_34_04;
	float det3_234_123 = mat[ 2 ][ 1 ] * det2_34_23 - mat[ 2 ][ 2 ] * det2_34_13 + mat[ 2 ][ 3 ] * det2_34_12;
	float det3_234_124 = mat[ 2 ][ 1 ] * det2_34_24 - mat[ 2 ][ 2 ] * det2_34_14 + mat[ 2 ][ 4 ] * det2_34_12;
	float det3_234_125 = mat[ 2 ][ 1 ] * det2_34_25 - mat[ 2 ][ 2 ] * det2_34_15 + mat[ 2 ][ 5 ] * det2_34_12;
	float det3_234_134 = mat[ 2 ][ 1 ] * det2_34_34 - mat[ 2 ][ 3 ] * det2_34_14 + mat[ 2 ][ 4 ] * det2_34_13;
	float det3_234_135 = mat[ 2 ][ 1 ] * det2_34_35 - mat[ 2 ][ 3 ] * det2_34_15 + mat[ 2 ][ 5 ] * det2_34_13;
	float det3_234_145 = mat[ 2 ][ 1 ] * det2_34_45 - mat[ 2 ][ 4 ] * det2_34_15 + mat[ 2 ][ 5 ] * det2_34_14;
	float det3_234_234 = mat[ 2 ][ 2 ] * det2_34_34 - mat[ 2 ][ 3 ] * det2_34_24 + mat[ 2 ][ 4 ] * det2_34_23;
	float det3_234_235 = mat[ 2 ][ 2 ] * det2_34_35 - mat[ 2 ][ 3 ] * det2_34_25 + mat[ 2 ][ 5 ] * det2_34_23;
	float det3_234_245 = mat[ 2 ][ 2 ] * det2_34_45 - mat[ 2 ][ 4 ] * det2_34_25 + mat[ 2 ][ 5 ] * det2_34_24;
	float det3_234_345 = mat[ 2 ][ 3 ] * det2_34_45 - mat[ 2 ][ 4 ] * det2_34_35 + mat[ 2 ][ 5 ] * det2_34_34;
	float det3_235_012 = mat[ 2 ][ 0 ] * det2_35_12 - mat[ 2 ][ 1 ] * det2_35_02 + mat[ 2 ][ 2 ] * det2_35_01;
	float det3_235_013 = mat[ 2 ][ 0 ] * det2_35_13 - mat[ 2 ][ 1 ] * det2_35_03 + mat[ 2 ][ 3 ] * det2_35_01;
	float det3_235_014 = mat[ 2 ][ 0 ] * det2_35_14 - mat[ 2 ][ 1 ] * det2_35_04 + mat[ 2 ][ 4 ] * det2_35_01;
	float det3_235_015 = mat[ 2 ][ 0 ] * det2_35_15 - mat[ 2 ][ 1 ] * det2_35_05 + mat[ 2 ][ 5 ] * det2_35_01;
	float det3_235_023 = mat[ 2 ][ 0 ] * det2_35_23 - mat[ 2 ][ 2 ] * det2_35_03 + mat[ 2 ][ 3 ] * det2_35_02;
	float det3_235_024 = mat[ 2 ][ 0 ] * det2_35_24 - mat[ 2 ][ 2 ] * det2_35_04 + mat[ 2 ][ 4 ] * det2_35_02;
	float det3_235_025 = mat[ 2 ][ 0 ] * det2_35_25 - mat[ 2 ][ 2 ] * det2_35_05 + mat[ 2 ][ 5 ] * det2_35_02;
	float det3_235_034 = mat[ 2 ][ 0 ] * det2_35_34 - mat[ 2 ][ 3 ] * det2_35_04 + mat[ 2 ][ 4 ] * det2_35_03;
	float det3_235_035 = mat[ 2 ][ 0 ] * det2_35_35 - mat[ 2 ][ 3 ] * det2_35_05 + mat[ 2 ][ 5 ] * det2_35_03;
	float det3_235_045 = mat[ 2 ][ 0 ] * det2_35_45 - mat[ 2 ][ 4 ] * det2_35_05 + mat[ 2 ][ 5 ] * det2_35_04;
	float det3_235_123 = mat[ 2 ][ 1 ] * det2_35_23 - mat[ 2 ][ 2 ] * det2_35_13 + mat[ 2 ][ 3 ] * det2_35_12;
	float det3_235_124 = mat[ 2 ][ 1 ] * det2_35_24 - mat[ 2 ][ 2 ] * det2_35_14 + mat[ 2 ][ 4 ] * det2_35_12;
	float det3_235_125 = mat[ 2 ][ 1 ] * det2_35_25 - mat[ 2 ][ 2 ] * det2_35_15 + mat[ 2 ][ 5 ] * det2_35_12;
	float det3_235_134 = mat[ 2 ][ 1 ] * det2_35_34 - mat[ 2 ][ 3 ] * det2_35_14 + mat[ 2 ][ 4 ] * det2_35_13;
	float det3_235_135 = mat[ 2 ][ 1 ] * det2_35_35 - mat[ 2 ][ 3 ] * det2_35_15 + mat[ 2 ][ 5 ] * det2_35_13;
	float det3_235_145 = mat[ 2 ][ 1 ] * det2_35_45 - mat[ 2 ][ 4 ] * det2_35_15 + mat[ 2 ][ 5 ] * det2_35_14;
	float det3_235_234 = mat[ 2 ][ 2 ] * det2_35_34 - mat[ 2 ][ 3 ] * det2_35_24 + mat[ 2 ][ 4 ] * det2_35_23;
	float det3_235_235 = mat[ 2 ][ 2 ] * det2_35_35 - mat[ 2 ][ 3 ] * det2_35_25 + mat[ 2 ][ 5 ] * det2_35_23;
	float det3_235_245 = mat[ 2 ][ 2 ] * det2_35_45 - mat[ 2 ][ 4 ] * det2_35_25 + mat[ 2 ][ 5 ] * det2_35_24;
	float det3_235_345 = mat[ 2 ][ 3 ] * det2_35_45 - mat[ 2 ][ 4 ] * det2_35_35 + mat[ 2 ][ 5 ] * det2_35_34;
	float det3_245_012 = mat[ 2 ][ 0 ] * det2_45_12 - mat[ 2 ][ 1 ] * det2_45_02 + mat[ 2 ][ 2 ] * det2_45_01;
	float det3_245_013 = mat[ 2 ][ 0 ] * det2_45_13 - mat[ 2 ][ 1 ] * det2_45_03 + mat[ 2 ][ 3 ] * det2_45_01;
	float det3_245_014 = mat[ 2 ][ 0 ] * det2_45_14 - mat[ 2 ][ 1 ] * det2_45_04 + mat[ 2 ][ 4 ] * det2_45_01;
	float det3_245_015 = mat[ 2 ][ 0 ] * det2_45_15 - mat[ 2 ][ 1 ] * det2_45_05 + mat[ 2 ][ 5 ] * det2_45_01;
	float det3_245_023 = mat[ 2 ][ 0 ] * det2_45_23 - mat[ 2 ][ 2 ] * det2_45_03 + mat[ 2 ][ 3 ] * det2_45_02;
	float det3_245_024 = mat[ 2 ][ 0 ] * det2_45_24 - mat[ 2 ][ 2 ] * det2_45_04 + mat[ 2 ][ 4 ] * det2_45_02;
	float det3_245_025 = mat[ 2 ][ 0 ] * det2_45_25 - mat[ 2 ][ 2 ] * det2_45_05 + mat[ 2 ][ 5 ] * det2_45_02;
	float det3_245_034 = mat[ 2 ][ 0 ] * det2_45_34 - mat[ 2 ][ 3 ] * det2_45_04 + mat[ 2 ][ 4 ] * det2_45_03;
	float det3_245_035 = mat[ 2 ][ 0 ] * det2_45_35 - mat[ 2 ][ 3 ] * det2_45_05 + mat[ 2 ][ 5 ] * det2_45_03;
	float det3_245_045 = mat[ 2 ][ 0 ] * det2_45_45 - mat[ 2 ][ 4 ] * det2_45_05 + mat[ 2 ][ 5 ] * det2_45_04;
	float det3_245_123 = mat[ 2 ][ 1 ] * det2_45_23 - mat[ 2 ][ 2 ] * det2_45_13 + mat[ 2 ][ 3 ] * det2_45_12;
	float det3_245_124 = mat[ 2 ][ 1 ] * det2_45_24 - mat[ 2 ][ 2 ] * det2_45_14 + mat[ 2 ][ 4 ] * det2_45_12;
	float det3_245_125 = mat[ 2 ][ 1 ] * det2_45_25 - mat[ 2 ][ 2 ] * det2_45_15 + mat[ 2 ][ 5 ] * det2_45_12;
	float det3_245_134 = mat[ 2 ][ 1 ] * det2_45_34 - mat[ 2 ][ 3 ] * det2_45_14 + mat[ 2 ][ 4 ] * det2_45_13;
	float det3_245_135 = mat[ 2 ][ 1 ] * det2_45_35 - mat[ 2 ][ 3 ] * det2_45_15 + mat[ 2 ][ 5 ] * det2_45_13;
	float det3_245_145 = mat[ 2 ][ 1 ] * det2_45_45 - mat[ 2 ][ 4 ] * det2_45_15 + mat[ 2 ][ 5 ] * det2_45_14;
	float det3_245_234 = mat[ 2 ][ 2 ] * det2_45_34 - mat[ 2 ][ 3 ] * det2_45_24 + mat[ 2 ][ 4 ] * det2_45_23;
	float det3_245_235 = mat[ 2 ][ 2 ] * det2_45_35 - mat[ 2 ][ 3 ] * det2_45_25 + mat[ 2 ][ 5 ] * det2_45_23;
	float det3_245_245 = mat[ 2 ][ 2 ] * det2_45_45 - mat[ 2 ][ 4 ] * det2_45_25 + mat[ 2 ][ 5 ] * det2_45_24;
	float det3_245_345 = mat[ 2 ][ 3 ] * det2_45_45 - mat[ 2 ][ 4 ] * det2_45_35 + mat[ 2 ][ 5 ] * det2_45_34;

	// remaining 4x4 sub-determinants
	float det4_1234_0123 = mat[ 1 ][ 0 ] * det3_234_123 - mat[ 1 ][ 1 ] * det3_234_023 + mat[ 1 ][ 2 ] * det3_234_013 - mat[ 1 ][ 3 ] * det3_234_012;
	float det4_1234_0124 = mat[ 1 ][ 0 ] * det3_234_124 - mat[ 1 ][ 1 ] * det3_234_024 + mat[ 1 ][ 2 ] * det3_234_014 - mat[ 1 ][ 4 ] * det3_234_012;
	float det4_1234_0125 = mat[ 1 ][ 0 ] * det3_234_125 - mat[ 1 ][ 1 ] * det3_234_025 + mat[ 1 ][ 2 ] * det3_234_015 - mat[ 1 ][ 5 ] * det3_234_012;
	float det4_1234_0134 = mat[ 1 ][ 0 ] * det3_234_134 - mat[ 1 ][ 1 ] * det3_234_034 + mat[ 1 ][ 3 ] * det3_234_014 - mat[ 1 ][ 4 ] * det3_234_013;
	float det4_1234_0135 = mat[ 1 ][ 0 ] * det3_234_135 - mat[ 1 ][ 1 ] * det3_234_035 + mat[ 1 ][ 3 ] * det3_234_015 - mat[ 1 ][ 5 ] * det3_234_013;
	float det4_1234_0145 = mat[ 1 ][ 0 ] * det3_234_145 - mat[ 1 ][ 1 ] * det3_234_045 + mat[ 1 ][ 4 ] * det3_234_015 - mat[ 1 ][ 5 ] * det3_234_014;
	float det4_1234_0234 = mat[ 1 ][ 0 ] * det3_234_234 - mat[ 1 ][ 2 ] * det3_234_034 + mat[ 1 ][ 3 ] * det3_234_024 - mat[ 1 ][ 4 ] * det3_234_023;
	float det4_1234_0235 = mat[ 1 ][ 0 ] * det3_234_235 - mat[ 1 ][ 2 ] * det3_234_035 + mat[ 1 ][ 3 ] * det3_234_025 - mat[ 1 ][ 5 ] * det3_234_023;
	float det4_1234_0245 = mat[ 1 ][ 0 ] * det3_234_245 - mat[ 1 ][ 2 ] * det3_234_045 + mat[ 1 ][ 4 ] * det3_234_025 - mat[ 1 ][ 5 ] * det3_234_024;
	float det4_1234_0345 = mat[ 1 ][ 0 ] * det3_234_345 - mat[ 1 ][ 3 ] * det3_234_045 + mat[ 1 ][ 4 ] * det3_234_035 - mat[ 1 ][ 5 ] * det3_234_034;
	float det4_1234_1234 = mat[ 1 ][ 1 ] * det3_234_234 - mat[ 1 ][ 2 ] * det3_234_134 + mat[ 1 ][ 3 ] * det3_234_124 - mat[ 1 ][ 4 ] * det3_234_123;
	float det4_1234_1235 = mat[ 1 ][ 1 ] * det3_234_235 - mat[ 1 ][ 2 ] * det3_234_135 + mat[ 1 ][ 3 ] * det3_234_125 - mat[ 1 ][ 5 ] * det3_234_123;
	float det4_1234_1245 = mat[ 1 ][ 1 ] * det3_234_245 - mat[ 1 ][ 2 ] * det3_234_145 + mat[ 1 ][ 4 ] * det3_234_125 - mat[ 1 ][ 5 ] * det3_234_124;
	float det4_1234_1345 = mat[ 1 ][ 1 ] * det3_234_345 - mat[ 1 ][ 3 ] * det3_234_145 + mat[ 1 ][ 4 ] * det3_234_135 - mat[ 1 ][ 5 ] * det3_234_134;
	float det4_1234_2345 = mat[ 1 ][ 2 ] * det3_234_345 - mat[ 1 ][ 3 ] * det3_234_245 + mat[ 1 ][ 4 ] * det3_234_235 - mat[ 1 ][ 5 ] * det3_234_234;
	float det4_1235_0123 = mat[ 1 ][ 0 ] * det3_235_123 - mat[ 1 ][ 1 ] * det3_235_023 + mat[ 1 ][ 2 ] * det3_235_013 - mat[ 1 ][ 3 ] * det3_235_012;
	float det4_1235_0124 = mat[ 1 ][ 0 ] * det3_235_124 - mat[ 1 ][ 1 ] * det3_235_024 + mat[ 1 ][ 2 ] * det3_235_014 - mat[ 1 ][ 4 ] * det3_235_012;
	float det4_1235_0125 = mat[ 1 ][ 0 ] * det3_235_125 - mat[ 1 ][ 1 ] * det3_235_025 + mat[ 1 ][ 2 ] * det3_235_015 - mat[ 1 ][ 5 ] * det3_235_012;
	float det4_1235_0134 = mat[ 1 ][ 0 ] * det3_235_134 - mat[ 1 ][ 1 ] * det3_235_034 + mat[ 1 ][ 3 ] * det3_235_014 - mat[ 1 ][ 4 ] * det3_235_013;
	float det4_1235_0135 = mat[ 1 ][ 0 ] * det3_235_135 - mat[ 1 ][ 1 ] * det3_235_035 + mat[ 1 ][ 3 ] * det3_235_015 - mat[ 1 ][ 5 ] * det3_235_013;
	float det4_1235_0145 = mat[ 1 ][ 0 ] * det3_235_145 - mat[ 1 ][ 1 ] * det3_235_045 + mat[ 1 ][ 4 ] * det3_235_015 - mat[ 1 ][ 5 ] * det3_235_014;
	float det4_1235_0234 = mat[ 1 ][ 0 ] * det3_235_234 - mat[ 1 ][ 2 ] * det3_235_034 + mat[ 1 ][ 3 ] * det3_235_024 - mat[ 1 ][ 4 ] * det3_235_023;
	float det4_1235_0235 = mat[ 1 ][ 0 ] * det3_235_235 - mat[ 1 ][ 2 ] * det3_235_035 + mat[ 1 ][ 3 ] * det3_235_025 - mat[ 1 ][ 5 ] * det3_235_023;
	float det4_1235_0245 = mat[ 1 ][ 0 ] * det3_235_245 - mat[ 1 ][ 2 ] * det3_235_045 + mat[ 1 ][ 4 ] * det3_235_025 - mat[ 1 ][ 5 ] * det3_235_024;
	float det4_1235_0345 = mat[ 1 ][ 0 ] * det3_235_345 - mat[ 1 ][ 3 ] * det3_235_045 + mat[ 1 ][ 4 ] * det3_235_035 - mat[ 1 ][ 5 ] * det3_235_034;
	float det4_1235_1234 = mat[ 1 ][ 1 ] * det3_235_234 - mat[ 1 ][ 2 ] * det3_235_134 + mat[ 1 ][ 3 ] * det3_235_124 - mat[ 1 ][ 4 ] * det3_235_123;
	float det4_1235_1235 = mat[ 1 ][ 1 ] * det3_235_235 - mat[ 1 ][ 2 ] * det3_235_135 + mat[ 1 ][ 3 ] * det3_235_125 - mat[ 1 ][ 5 ] * det3_235_123;
	float det4_1235_1245 = mat[ 1 ][ 1 ] * det3_235_245 - mat[ 1 ][ 2 ] * det3_235_145 + mat[ 1 ][ 4 ] * det3_235_125 - mat[ 1 ][ 5 ] * det3_235_124;
	float det4_1235_1345 = mat[ 1 ][ 1 ] * det3_235_345 - mat[ 1 ][ 3 ] * det3_235_145 + mat[ 1 ][ 4 ] * det3_235_135 - mat[ 1 ][ 5 ] * det3_235_134;
	float det4_1235_2345 = mat[ 1 ][ 2 ] * det3_235_345 - mat[ 1 ][ 3 ] * det3_235_245 + mat[ 1 ][ 4 ] * det3_235_235 - mat[ 1 ][ 5 ] * det3_235_234;
	float det4_1245_0123 = mat[ 1 ][ 0 ] * det3_245_123 - mat[ 1 ][ 1 ] * det3_245_023 + mat[ 1 ][ 2 ] * det3_245_013 - mat[ 1 ][ 3 ] * det3_245_012;
	float det4_1245_0124 = mat[ 1 ][ 0 ] * det3_245_124 - mat[ 1 ][ 1 ] * det3_245_024 + mat[ 1 ][ 2 ] * det3_245_014 - mat[ 1 ][ 4 ] * det3_245_012;
	float det4_1245_0125 = mat[ 1 ][ 0 ] * det3_245_125 - mat[ 1 ][ 1 ] * det3_245_025 + mat[ 1 ][ 2 ] * det3_245_015 - mat[ 1 ][ 5 ] * det3_245_012;
	float det4_1245_0134 = mat[ 1 ][ 0 ] * det3_245_134 - mat[ 1 ][ 1 ] * det3_245_034 + mat[ 1 ][ 3 ] * det3_245_014 - mat[ 1 ][ 4 ] * det3_245_013;
	float det4_1245_0135 = mat[ 1 ][ 0 ] * det3_245_135 - mat[ 1 ][ 1 ] * det3_245_035 + mat[ 1 ][ 3 ] * det3_245_015 - mat[ 1 ][ 5 ] * det3_245_013;
	float det4_1245_0145 = mat[ 1 ][ 0 ] * det3_245_145 - mat[ 1 ][ 1 ] * det3_245_045 + mat[ 1 ][ 4 ] * det3_245_015 - mat[ 1 ][ 5 ] * det3_245_014;
	float det4_1245_0234 = mat[ 1 ][ 0 ] * det3_245_234 - mat[ 1 ][ 2 ] * det3_245_034 + mat[ 1 ][ 3 ] * det3_245_024 - mat[ 1 ][ 4 ] * det3_245_023;
	float det4_1245_0235 = mat[ 1 ][ 0 ] * det3_245_235 - mat[ 1 ][ 2 ] * det3_245_035 + mat[ 1 ][ 3 ] * det3_245_025 - mat[ 1 ][ 5 ] * det3_245_023;
	float det4_1245_0245 = mat[ 1 ][ 0 ] * det3_245_245 - mat[ 1 ][ 2 ] * det3_245_045 + mat[ 1 ][ 4 ] * det3_245_025 - mat[ 1 ][ 5 ] * det3_245_024;
	float det4_1245_0345 = mat[ 1 ][ 0 ] * det3_245_345 - mat[ 1 ][ 3 ] * det3_245_045 + mat[ 1 ][ 4 ] * det3_245_035 - mat[ 1 ][ 5 ] * det3_245_034;
	float det4_1245_1234 = mat[ 1 ][ 1 ] * det3_245_234 - mat[ 1 ][ 2 ] * det3_245_134 + mat[ 1 ][ 3 ] * det3_245_124 - mat[ 1 ][ 4 ] * det3_245_123;
	float det4_1245_1235 = mat[ 1 ][ 1 ] * det3_245_235 - mat[ 1 ][ 2 ] * det3_245_135 + mat[ 1 ][ 3 ] * det3_245_125 - mat[ 1 ][ 5 ] * det3_245_123;
	float det4_1245_1245 = mat[ 1 ][ 1 ] * det3_245_245 - mat[ 1 ][ 2 ] * det3_245_145 + mat[ 1 ][ 4 ] * det3_245_125 - mat[ 1 ][ 5 ] * det3_245_124;
	float det4_1245_1345 = mat[ 1 ][ 1 ] * det3_245_345 - mat[ 1 ][ 3 ] * det3_245_145 + mat[ 1 ][ 4 ] * det3_245_135 - mat[ 1 ][ 5 ] * det3_245_134;
	float det4_1245_2345 = mat[ 1 ][ 2 ] * det3_245_345 - mat[ 1 ][ 3 ] * det3_245_245 + mat[ 1 ][ 4 ] * det3_245_235 - mat[ 1 ][ 5 ] * det3_245_234;
	float det4_1345_0123 = mat[ 1 ][ 0 ] * det3_345_123 - mat[ 1 ][ 1 ] * det3_345_023 + mat[ 1 ][ 2 ] * det3_345_013 - mat[ 1 ][ 3 ] * det3_345_012;
	float det4_1345_0124 = mat[ 1 ][ 0 ] * det3_345_124 - mat[ 1 ][ 1 ] * det3_345_024 + mat[ 1 ][ 2 ] * det3_345_014 - mat[ 1 ][ 4 ] * det3_345_012;
	float det4_1345_0125 = mat[ 1 ][ 0 ] * det3_345_125 - mat[ 1 ][ 1 ] * det3_345_025 + mat[ 1 ][ 2 ] * det3_345_015 - mat[ 1 ][ 5 ] * det3_345_012;
	float det4_1345_0134 = mat[ 1 ][ 0 ] * det3_345_134 - mat[ 1 ][ 1 ] * det3_345_034 + mat[ 1 ][ 3 ] * det3_345_014 - mat[ 1 ][ 4 ] * det3_345_013;
	float det4_1345_0135 = mat[ 1 ][ 0 ] * det3_345_135 - mat[ 1 ][ 1 ] * det3_345_035 + mat[ 1 ][ 3 ] * det3_345_015 - mat[ 1 ][ 5 ] * det3_345_013;
	float det4_1345_0145 = mat[ 1 ][ 0 ] * det3_345_145 - mat[ 1 ][ 1 ] * det3_345_045 + mat[ 1 ][ 4 ] * det3_345_015 - mat[ 1 ][ 5 ] * det3_345_014;
	float det4_1345_0234 = mat[ 1 ][ 0 ] * det3_345_234 - mat[ 1 ][ 2 ] * det3_345_034 + mat[ 1 ][ 3 ] * det3_345_024 - mat[ 1 ][ 4 ] * det3_345_023;
	float det4_1345_0235 = mat[ 1 ][ 0 ] * det3_345_235 - mat[ 1 ][ 2 ] * det3_345_035 + mat[ 1 ][ 3 ] * det3_345_025 - mat[ 1 ][ 5 ] * det3_345_023;
	float det4_1345_0245 = mat[ 1 ][ 0 ] * det3_345_245 - mat[ 1 ][ 2 ] * det3_345_045 + mat[ 1 ][ 4 ] * det3_345_025 - mat[ 1 ][ 5 ] * det3_345_024;
	float det4_1345_0345 = mat[ 1 ][ 0 ] * det3_345_345 - mat[ 1 ][ 3 ] * det3_345_045 + mat[ 1 ][ 4 ] * det3_345_035 - mat[ 1 ][ 5 ] * det3_345_034;
	float det4_1345_1234 = mat[ 1 ][ 1 ] * det3_345_234 - mat[ 1 ][ 2 ] * det3_345_134 + mat[ 1 ][ 3 ] * det3_345_124 - mat[ 1 ][ 4 ] * det3_345_123;
	float det4_1345_1235 = mat[ 1 ][ 1 ] * det3_345_235 - mat[ 1 ][ 2 ] * det3_345_135 + mat[ 1 ][ 3 ] * det3_345_125 - mat[ 1 ][ 5 ] * det3_345_123;
	float det4_1345_1245 = mat[ 1 ][ 1 ] * det3_345_245 - mat[ 1 ][ 2 ] * det3_345_145 + mat[ 1 ][ 4 ] * det3_345_125 - mat[ 1 ][ 5 ] * det3_345_124;
	float det4_1345_1345 = mat[ 1 ][ 1 ] * det3_345_345 - mat[ 1 ][ 3 ] * det3_345_145 + mat[ 1 ][ 4 ] * det3_345_135 - mat[ 1 ][ 5 ] * det3_345_134;
	float det4_1345_2345 = mat[ 1 ][ 2 ] * det3_345_345 - mat[ 1 ][ 3 ] * det3_345_245 + mat[ 1 ][ 4 ] * det3_345_235 - mat[ 1 ][ 5 ] * det3_345_234;

	// remaining 5x5 sub-determinants
	float det5_01234_01234 = mat[ 0 ][ 0 ] * det4_1234_1234 - mat[ 0 ][ 1 ] * det4_1234_0234 + mat[ 0 ][ 2 ] * det4_1234_0134 - mat[ 0 ][ 3 ] * det4_1234_0124 + mat[ 0 ][ 4 ] * det4_1234_0123;
	float det5_01234_01235 = mat[ 0 ][ 0 ] * det4_1234_1235 - mat[ 0 ][ 1 ] * det4_1234_0235 + mat[ 0 ][ 2 ] * det4_1234_0135 - mat[ 0 ][ 3 ] * det4_1234_0125 + mat[ 0 ][ 5 ] * det4_1234_0123;
	float det5_01234_01245 = mat[ 0 ][ 0 ] * det4_1234_1245 - mat[ 0 ][ 1 ] * det4_1234_0245 + mat[ 0 ][ 2 ] * det4_1234_0145 - mat[ 0 ][ 4 ] * det4_1234_0125 + mat[ 0 ][ 5 ] * det4_1234_0124;
	float det5_01234_01345 = mat[ 0 ][ 0 ] * det4_1234_1345 - mat[ 0 ][ 1 ] * det4_1234_0345 + mat[ 0 ][ 3 ] * det4_1234_0145 - mat[ 0 ][ 4 ] * det4_1234_0135 + mat[ 0 ][ 5 ] * det4_1234_0134;
	float det5_01234_02345 = mat[ 0 ][ 0 ] * det4_1234_2345 - mat[ 0 ][ 2 ] * det4_1234_0345 + mat[ 0 ][ 3 ] * det4_1234_0245 - mat[ 0 ][ 4 ] * det4_1234_0235 + mat[ 0 ][ 5 ] * det4_1234_0234;
	float det5_01234_12345 = mat[ 0 ][ 1 ] * det4_1234_2345 - mat[ 0 ][ 2 ] * det4_1234_1345 + mat[ 0 ][ 3 ] * det4_1234_1245 - mat[ 0 ][ 4 ] * det4_1234_1235 + mat[ 0 ][ 5 ] * det4_1234_1234;
	float det5_01235_01234 = mat[ 0 ][ 0 ] * det4_1235_1234 - mat[ 0 ][ 1 ] * det4_1235_0234 + mat[ 0 ][ 2 ] * det4_1235_0134 - mat[ 0 ][ 3 ] * det4_1235_0124 + mat[ 0 ][ 4 ] * det4_1235_0123;
	float det5_01235_01235 = mat[ 0 ][ 0 ] * det4_1235_1235 - mat[ 0 ][ 1 ] * det4_1235_0235 + mat[ 0 ][ 2 ] * det4_1235_0135 - mat[ 0 ][ 3 ] * det4_1235_0125 + mat[ 0 ][ 5 ] * det4_1235_0123;
	float det5_01235_01245 = mat[ 0 ][ 0 ] * det4_1235_1245 - mat[ 0 ][ 1 ] * det4_1235_0245 + mat[ 0 ][ 2 ] * det4_1235_0145 - mat[ 0 ][ 4 ] * det4_1235_0125 + mat[ 0 ][ 5 ] * det4_1235_0124;
	float det5_01235_01345 = mat[ 0 ][ 0 ] * det4_1235_1345 - mat[ 0 ][ 1 ] * det4_1235_0345 + mat[ 0 ][ 3 ] * det4_1235_0145 - mat[ 0 ][ 4 ] * det4_1235_0135 + mat[ 0 ][ 5 ] * det4_1235_0134;
	float det5_01235_02345 = mat[ 0 ][ 0 ] * det4_1235_2345 - mat[ 0 ][ 2 ] * det4_1235_0345 + mat[ 0 ][ 3 ] * det4_1235_0245 - mat[ 0 ][ 4 ] * det4_1235_0235 + mat[ 0 ][ 5 ] * det4_1235_0234;
	float det5_01235_12345 = mat[ 0 ][ 1 ] * det4_1235_2345 - mat[ 0 ][ 2 ] * det4_1235_1345 + mat[ 0 ][ 3 ] * det4_1235_1245 - mat[ 0 ][ 4 ] * det4_1235_1235 + mat[ 0 ][ 5 ] * det4_1235_1234;
	float det5_01245_01234 = mat[ 0 ][ 0 ] * det4_1245_1234 - mat[ 0 ][ 1 ] * det4_1245_0234 + mat[ 0 ][ 2 ] * det4_1245_0134 - mat[ 0 ][ 3 ] * det4_1245_0124 + mat[ 0 ][ 4 ] * det4_1245_0123;
	float det5_01245_01235 = mat[ 0 ][ 0 ] * det4_1245_1235 - mat[ 0 ][ 1 ] * det4_1245_0235 + mat[ 0 ][ 2 ] * det4_1245_0135 - mat[ 0 ][ 3 ] * det4_1245_0125 + mat[ 0 ][ 5 ] * det4_1245_0123;
	float det5_01245_01245 = mat[ 0 ][ 0 ] * det4_1245_1245 - mat[ 0 ][ 1 ] * det4_1245_0245 + mat[ 0 ][ 2 ] * det4_1245_0145 - mat[ 0 ][ 4 ] * det4_1245_0125 + mat[ 0 ][ 5 ] * det4_1245_0124;
	float det5_01245_01345 = mat[ 0 ][ 0 ] * det4_1245_1345 - mat[ 0 ][ 1 ] * det4_1245_0345 + mat[ 0 ][ 3 ] * det4_1245_0145 - mat[ 0 ][ 4 ] * det4_1245_0135 + mat[ 0 ][ 5 ] * det4_1245_0134;
	float det5_01245_02345 = mat[ 0 ][ 0 ] * det4_1245_2345 - mat[ 0 ][ 2 ] * det4_1245_0345 + mat[ 0 ][ 3 ] * det4_1245_0245 - mat[ 0 ][ 4 ] * det4_1245_0235 + mat[ 0 ][ 5 ] * det4_1245_0234;
	float det5_01245_12345 = mat[ 0 ][ 1 ] * det4_1245_2345 - mat[ 0 ][ 2 ] * det4_1245_1345 + mat[ 0 ][ 3 ] * det4_1245_1245 - mat[ 0 ][ 4 ] * det4_1245_1235 + mat[ 0 ][ 5 ] * det4_1245_1234;
	float det5_01345_01234 = mat[ 0 ][ 0 ] * det4_1345_1234 - mat[ 0 ][ 1 ] * det4_1345_0234 + mat[ 0 ][ 2 ] * det4_1345_0134 - mat[ 0 ][ 3 ] * det4_1345_0124 + mat[ 0 ][ 4 ] * det4_1345_0123;
	float det5_01345_01235 = mat[ 0 ][ 0 ] * det4_1345_1235 - mat[ 0 ][ 1 ] * det4_1345_0235 + mat[ 0 ][ 2 ] * det4_1345_0135 - mat[ 0 ][ 3 ] * det4_1345_0125 + mat[ 0 ][ 5 ] * det4_1345_0123;
	float det5_01345_01245 = mat[ 0 ][ 0 ] * det4_1345_1245 - mat[ 0 ][ 1 ] * det4_1345_0245 + mat[ 0 ][ 2 ] * det4_1345_0145 - mat[ 0 ][ 4 ] * det4_1345_0125 + mat[ 0 ][ 5 ] * det4_1345_0124;
	float det5_01345_01345 = mat[ 0 ][ 0 ] * det4_1345_1345 - mat[ 0 ][ 1 ] * det4_1345_0345 + mat[ 0 ][ 3 ] * det4_1345_0145 - mat[ 0 ][ 4 ] * det4_1345_0135 + mat[ 0 ][ 5 ] * det4_1345_0134;
	float det5_01345_02345 = mat[ 0 ][ 0 ] * det4_1345_2345 - mat[ 0 ][ 2 ] * det4_1345_0345 + mat[ 0 ][ 3 ] * det4_1345_0245 - mat[ 0 ][ 4 ] * det4_1345_0235 + mat[ 0 ][ 5 ] * det4_1345_0234;
	float det5_01345_12345 = mat[ 0 ][ 1 ] * det4_1345_2345 - mat[ 0 ][ 2 ] * det4_1345_1345 + mat[ 0 ][ 3 ] * det4_1345_1245 - mat[ 0 ][ 4 ] * det4_1345_1235 + mat[ 0 ][ 5 ] * det4_1345_1234;
	float det5_02345_01234 = mat[ 0 ][ 0 ] * det4_2345_1234 - mat[ 0 ][ 1 ] * det4_2345_0234 + mat[ 0 ][ 2 ] * det4_2345_0134 - mat[ 0 ][ 3 ] * det4_2345_0124 + mat[ 0 ][ 4 ] * det4_2345_0123;
	float det5_02345_01235 = mat[ 0 ][ 0 ] * det4_2345_1235 - mat[ 0 ][ 1 ] * det4_2345_0235 + mat[ 0 ][ 2 ] * det4_2345_0135 - mat[ 0 ][ 3 ] * det4_2345_0125 + mat[ 0 ][ 5 ] * det4_2345_0123;
	float det5_02345_01245 = mat[ 0 ][ 0 ] * det4_2345_1245 - mat[ 0 ][ 1 ] * det4_2345_0245 + mat[ 0 ][ 2 ] * det4_2345_0145 - mat[ 0 ][ 4 ] * det4_2345_0125 + mat[ 0 ][ 5 ] * det4_2345_0124;
	float det5_02345_01345 = mat[ 0 ][ 0 ] * det4_2345_1345 - mat[ 0 ][ 1 ] * det4_2345_0345 + mat[ 0 ][ 3 ] * det4_2345_0145 - mat[ 0 ][ 4 ] * det4_2345_0135 + mat[ 0 ][ 5 ] * det4_2345_0134;
	float det5_02345_02345 = mat[ 0 ][ 0 ] * det4_2345_2345 - mat[ 0 ][ 2 ] * det4_2345_0345 + mat[ 0 ][ 3 ] * det4_2345_0245 - mat[ 0 ][ 4 ] * det4_2345_0235 + mat[ 0 ][ 5 ] * det4_2345_0234;
	float det5_02345_12345 = mat[ 0 ][ 1 ] * det4_2345_2345 - mat[ 0 ][ 2 ] * det4_2345_1345 + mat[ 0 ][ 3 ] * det4_2345_1245 - mat[ 0 ][ 4 ] * det4_2345_1235 + mat[ 0 ][ 5 ] * det4_2345_1234;

	mat[ 0 ][ 0 ] =  det5_12345_12345 * invDet;
	mat[ 0 ][ 1 ] = -det5_02345_12345 * invDet;
	mat[ 0 ][ 2 ] =  det5_01345_12345 * invDet;
	mat[ 0 ][ 3 ] = -det5_01245_12345 * invDet;
	mat[ 0 ][ 4 ] =  det5_01235_12345 * invDet;
	mat[ 0 ][ 5 ] = -det5_01234_12345 * invDet;

	mat[ 1 ][ 0 ] = -det5_12345_02345 * invDet;
	mat[ 1 ][ 1 ] =  det5_02345_02345 * invDet;
	mat[ 1 ][ 2 ] = -det5_01345_02345 * invDet;
	mat[ 1 ][ 3 ] =  det5_01245_02345 * invDet;
	mat[ 1 ][ 4 ] = -det5_01235_02345 * invDet;
	mat[ 1 ][ 5 ] =  det5_01234_02345 * invDet;

	mat[ 2 ][ 0 ] =  det5_12345_01345 * invDet;
	mat[ 2 ][ 1 ] = -det5_02345_01345 * invDet;
	mat[ 2 ][ 2 ] =  det5_01345_01345 * invDet;
	mat[ 2 ][ 3 ] = -det5_01245_01345 * invDet;
	mat[ 2 ][ 4 ] =  det5_01235_01345 * invDet;
	mat[ 2 ][ 5 ] = -det5_01234_01345 * invDet;

	mat[ 3 ][ 0 ] = -det5_12345_01245 * invDet;
	mat[ 3 ][ 1 ] =  det5_02345_01245 * invDet;
	mat[ 3 ][ 2 ] = -det5_01345_01245 * invDet;
	mat[ 3 ][ 3 ] =  det5_01245_01245 * invDet;
	mat[ 3 ][ 4 ] = -det5_01235_01245 * invDet;
	mat[ 3 ][ 5 ] =  det5_01234_01245 * invDet;

	mat[ 4 ][ 0 ] =  det5_12345_01235 * invDet;
	mat[ 4 ][ 1 ] = -det5_02345_01235 * invDet;
	mat[ 4 ][ 2 ] =  det5_01345_01235 * invDet;
	mat[ 4 ][ 3 ] = -det5_01245_01235 * invDet;
	mat[ 4 ][ 4 ] =  det5_01235_01235 * invDet;
	mat[ 4 ][ 5 ] = -det5_01234_01235 * invDet;

	mat[ 5 ][ 0 ] = -det5_12345_01234 * invDet;
	mat[ 5 ][ 1 ] =  det5_02345_01234 * invDet;
	mat[ 5 ][ 2 ] = -det5_01345_01234 * invDet;
	mat[ 5 ][ 3 ] =  det5_01245_01234 * invDet;
	mat[ 5 ][ 4 ] = -det5_01235_01234 * invDet;
	mat[ 5 ][ 5 ] =  det5_01234_01234 * invDet;

	return true;
#elif 0
	// 6*40 = 240 multiplications
	//			6 divisions
	float *mat = reinterpret_cast<float *>(this);
	float s;
	float d, di;

	di = mat[ 0 ];
	s = di;
	mat[ 0 ] = d = 1.0f / di;
	mat[ 1 ] *= d;
	mat[ 2 ] *= d;
	mat[ 3 ] *= d;
	mat[ 4 ] *= d;
	mat[ 5 ] *= d;
	d = -d;
	mat[ 6 ] *= d;
	mat[ 12 ] *= d;
	mat[ 18 ] *= d;
	mat[ 24 ] *= d;
	mat[ 30 ] *= d;
	d = mat[ 6 ] * di;
	mat[ 7 ] += mat[ 1 ] * d;
	mat[ 8 ] += mat[ 2 ] * d;
	mat[ 9 ] += mat[ 3 ] * d;
	mat[ 10 ] += mat[ 4 ] * d;
	mat[ 11 ] += mat[ 5 ] * d;
	d = mat[ 12 ] * di;
	mat[ 13 ] += mat[ 1 ] * d;
	mat[ 14 ] += mat[ 2 ] * d;
	mat[ 15 ] += mat[ 3 ] * d;
	mat[ 16 ] += mat[ 4 ] * d;
	mat[ 17 ] += mat[ 5 ] * d;
	d = mat[ 18 ] * di;
	mat[ 19 ] += mat[ 1 ] * d;
	mat[ 20 ] += mat[ 2 ] * d;
	mat[ 21 ] += mat[ 3 ] * d;
	mat[ 22 ] += mat[ 4 ] * d;
	mat[ 23 ] += mat[ 5 ] * d;
	d = mat[ 24 ] * di;
	mat[ 25 ] += mat[ 1 ] * d;
	mat[ 26 ] += mat[ 2 ] * d;
	mat[ 27 ] += mat[ 3 ] * d;
	mat[ 28 ] += mat[ 4 ] * d;
	mat[ 29 ] += mat[ 5 ] * d;
	d = mat[ 30 ] * di;
	mat[ 31 ] += mat[ 1 ] * d;
	mat[ 32 ] += mat[ 2 ] * d;
	mat[ 33 ] += mat[ 3 ] * d;
	mat[ 34 ] += mat[ 4 ] * d;
	mat[ 35 ] += mat[ 5 ] * d;
	di = mat[ 7 ];
	s *= di;
	mat[ 7 ] = d = 1.0f / di;
	mat[ 6 ] *= d;
	mat[ 8 ] *= d;
	mat[ 9 ] *= d;
	mat[ 10 ] *= d;
	mat[ 11 ] *= d;
	d = -d;
	mat[ 1 ] *= d;
	mat[ 13 ] *= d;
	mat[ 19 ] *= d;
	mat[ 25 ] *= d;
	mat[ 31 ] *= d;
	d = mat[ 1 ] * di;
	mat[ 0 ] += mat[ 6 ] * d;
	mat[ 2 ] += mat[ 8 ] * d;
	mat[ 3 ] += mat[ 9 ] * d;
	mat[ 4 ] += mat[ 10 ] * d;
	mat[ 5 ] += mat[ 11 ] * d;
	d = mat[ 13 ] * di;
	mat[ 12 ] += mat[ 6 ] * d;
	mat[ 14 ] += mat[ 8 ] * d;
	mat[ 15 ] += mat[ 9 ] * d;
	mat[ 16 ] += mat[ 10 ] * d;
	mat[ 17 ] += mat[ 11 ] * d;
	d = mat[ 19 ] * di;
	mat[ 18 ] += mat[ 6 ] * d;
	mat[ 20 ] += mat[ 8 ] * d;
	mat[ 21 ] += mat[ 9 ] * d;
	mat[ 22 ] += mat[ 10 ] * d;
	mat[ 23 ] += mat[ 11 ] * d;
	d = mat[ 25 ] * di;
	mat[ 24 ] += mat[ 6 ] * d;
	mat[ 26 ] += mat[ 8 ] * d;
	mat[ 27 ] += mat[ 9 ] * d;
	mat[ 28 ] += mat[ 10 ] * d;
	mat[ 29 ] += mat[ 11 ] * d;
	d = mat[ 31 ] * di;
	mat[ 30 ] += mat[ 6 ] * d;
	mat[ 32 ] += mat[ 8 ] * d;
	mat[ 33 ] += mat[ 9 ] * d;
	mat[ 34 ] += mat[ 10 ] * d;
	mat[ 35 ] += mat[ 11 ] * d;
	di = mat[ 14 ];
	s *= di;
	mat[ 14 ] = d = 1.0f / di;
	mat[ 12 ] *= d;
	mat[ 13 ] *= d;
	mat[ 15 ] *= d;
	mat[ 16 ] *= d;
	mat[ 17 ] *= d;
	d = -d;
	mat[ 2 ] *= d;
	mat[ 8 ] *= d;
	mat[ 20 ] *= d;
	mat[ 26 ] *= d;
	mat[ 32 ] *= d;
	d = mat[ 2 ] * di;
	mat[ 0 ] += mat[ 12 ] * d;
	mat[ 1 ] += mat[ 13 ] * d;
	mat[ 3 ] += mat[ 15 ] * d;
	mat[ 4 ] += mat[ 16 ] * d;
	mat[ 5 ] += mat[ 17 ] * d;
	d = mat[ 8 ] * di;
	mat[ 6 ] += mat[ 12 ] * d;
	mat[ 7 ] += mat[ 13 ] * d;
	mat[ 9 ] += mat[ 15 ] * d;
	mat[ 10 ] += mat[ 16 ] * d;
	mat[ 11 ] += mat[ 17 ] * d;
	d = mat[ 20 ] * di;
	mat[ 18 ] += mat[ 12 ] * d;
	mat[ 19 ] += mat[ 13 ] * d;
	mat[ 21 ] += mat[ 15 ] * d;
	mat[ 22 ] += mat[ 16 ] * d;
	mat[ 23 ] += mat[ 17 ] * d;
	d = mat[ 26 ] * di;
	mat[ 24 ] += mat[ 12 ] * d;
	mat[ 25 ] += mat[ 13 ] * d;
	mat[ 27 ] += mat[ 15 ] * d;
	mat[ 28 ] += mat[ 16 ] * d;
	mat[ 29 ] += mat[ 17 ] * d;
	d = mat[ 32 ] * di;
	mat[ 30 ] += mat[ 12 ] * d;
	mat[ 31 ] += mat[ 13 ] * d;
	mat[ 33 ] += mat[ 15 ] * d;
	mat[ 34 ] += mat[ 16 ] * d;
	mat[ 35 ] += mat[ 17 ] * d;
	di = mat[ 21 ];
	s *= di;
	mat[ 21 ] = d = 1.0f / di;
	mat[ 18 ] *= d;
	mat[ 19 ] *= d;
	mat[ 20 ] *= d;
	mat[ 22 ] *= d;
	mat[ 23 ] *= d;
	d = -d;
	mat[ 3 ] *= d;
	mat[ 9 ] *= d;
	mat[ 15 ] *= d;
	mat[ 27 ] *= d;
	mat[ 33 ] *= d;
	d = mat[ 3 ] * di;
	mat[ 0 ] += mat[ 18 ] * d;
	mat[ 1 ] += mat[ 19 ] * d;
	mat[ 2 ] += mat[ 20 ] * d;
	mat[ 4 ] += mat[ 22 ] * d;
	mat[ 5 ] += mat[ 23 ] * d;
	d = mat[ 9 ] * di;
	mat[ 6 ] += mat[ 18 ] * d;
	mat[ 7 ] += mat[ 19 ] * d;
	mat[ 8 ] += mat[ 20 ] * d;
	mat[ 10 ] += mat[ 22 ] * d;
	mat[ 11 ] += mat[ 23 ] * d;
	d = mat[ 15 ] * di;
	mat[ 12 ] += mat[ 18 ] * d;
	mat[ 13 ] += mat[ 19 ] * d;
	mat[ 14 ] += mat[ 20 ] * d;
	mat[ 16 ] += mat[ 22 ] * d;
	mat[ 17 ] += mat[ 23 ] * d;
	d = mat[ 27 ] * di;
	mat[ 24 ] += mat[ 18 ] * d;
	mat[ 25 ] += mat[ 19 ] * d;
	mat[ 26 ] += mat[ 20 ] * d;
	mat[ 28 ] += mat[ 22 ] * d;
	mat[ 29 ] += mat[ 23 ] * d;
	d = mat[ 33 ] * di;
	mat[ 30 ] += mat[ 18 ] * d;
	mat[ 31 ] += mat[ 19 ] * d;
	mat[ 32 ] += mat[ 20 ] * d;
	mat[ 34 ] += mat[ 22 ] * d;
	mat[ 35 ] += mat[ 23 ] * d;
	di = mat[ 28 ];
	s *= di;
	mat[ 28 ] = d = 1.0f / di;
	mat[ 24 ] *= d;
	mat[ 25 ] *= d;
	mat[ 26 ] *= d;
	mat[ 27 ] *= d;
	mat[ 29 ] *= d;
	d = -d;
	mat[ 4 ] *= d;
	mat[ 10 ] *= d;
	mat[ 16 ] *= d;
	mat[ 22 ] *= d;
	mat[ 34 ] *= d;
	d = mat[ 4 ] * di;
	mat[ 0 ] += mat[ 24 ] * d;
	mat[ 1 ] += mat[ 25 ] * d;
	mat[ 2 ] += mat[ 26 ] * d;
	mat[ 3 ] += mat[ 27 ] * d;
	mat[ 5 ] += mat[ 29 ] * d;
	d = mat[ 10 ] * di;
	mat[ 6 ] += mat[ 24 ] * d;
	mat[ 7 ] += mat[ 25 ] * d;
	mat[ 8 ] += mat[ 26 ] * d;
	mat[ 9 ] += mat[ 27 ] * d;
	mat[ 11 ] += mat[ 29 ] * d;
	d = mat[ 16 ] * di;
	mat[ 12 ] += mat[ 24 ] * d;
	mat[ 13 ] += mat[ 25 ] * d;
	mat[ 14 ] += mat[ 26 ] * d;
	mat[ 15 ] += mat[ 27 ] * d;
	mat[ 17 ] += mat[ 29 ] * d;
	d = mat[ 22 ] * di;
	mat[ 18 ] += mat[ 24 ] * d;
	mat[ 19 ] += mat[ 25 ] * d;
	mat[ 20 ] += mat[ 26 ] * d;
	mat[ 21 ] += mat[ 27 ] * d;
	mat[ 23 ] += mat[ 29 ] * d;
	d = mat[ 34 ] * di;
	mat[ 30 ] += mat[ 24 ] * d;
	mat[ 31 ] += mat[ 25 ] * d;
	mat[ 32 ] += mat[ 26 ] * d;
	mat[ 33 ] += mat[ 27 ] * d;
	mat[ 35 ] += mat[ 29 ] * d;
	di = mat[ 35 ];
	s *= di;
	mat[ 35 ] = d = 1.0f / di;
	mat[ 30 ] *= d;
	mat[ 31 ] *= d;
	mat[ 32 ] *= d;
	mat[ 33 ] *= d;
	mat[ 34 ] *= d;
	d = -d;
	mat[ 5 ] *= d;
	mat[ 11 ] *= d;
	mat[ 17 ] *= d;
	mat[ 23 ] *= d;
	mat[ 29 ] *= d;
	d = mat[ 5 ] * di;
	mat[ 0 ] += mat[ 30 ] * d;
	mat[ 1 ] += mat[ 31 ] * d;
	mat[ 2 ] += mat[ 32 ] * d;
	mat[ 3 ] += mat[ 33 ] * d;
	mat[ 4 ] += mat[ 34 ] * d;
	d = mat[ 11 ] * di;
	mat[ 6 ] += mat[ 30 ] * d;
	mat[ 7 ] += mat[ 31 ] * d;
	mat[ 8 ] += mat[ 32 ] * d;
	mat[ 9 ] += mat[ 33 ] * d;
	mat[ 10 ] += mat[ 34 ] * d;
	d = mat[ 17 ] * di;
	mat[ 12 ] += mat[ 30 ] * d;
	mat[ 13 ] += mat[ 31 ] * d;
	mat[ 14 ] += mat[ 32 ] * d;
	mat[ 15 ] += mat[ 33 ] * d;
	mat[ 16 ] += mat[ 34 ] * d;
	d = mat[ 23 ] * di;
	mat[ 18 ] += mat[ 30 ] * d;
	mat[ 19 ] += mat[ 31 ] * d;
	mat[ 20 ] += mat[ 32 ] * d;
	mat[ 21 ] += mat[ 33 ] * d;
	mat[ 22 ] += mat[ 34 ] * d;
	d = mat[ 29 ] * di;
	mat[ 24 ] += mat[ 30 ] * d;
	mat[ 25 ] += mat[ 31 ] * d;
	mat[ 26 ] += mat[ 32 ] * d;
	mat[ 27 ] += mat[ 33 ] * d;
	mat[ 28 ] += mat[ 34 ] * d;

	return ( s != 0.0f && !FLOAT_IS_NAN( s ) );
#else
	// 6*27+2*30 = 222 multiplications
	//		2*1  =	 2 divisions
	Mat3 r0, r1, r2, r3;
	float c0, c1, c2, det, invDet;
	float *mat = reinterpret_cast<float *>(this);

	// r0 = m0.Inverse( );
	c0 = mat[1*6+1] * mat[2*6+2] - mat[1*6+2] * mat[2*6+1];
	c1 = mat[1*6+2] * mat[2*6+0] - mat[1*6+0] * mat[2*6+2];
	c2 = mat[1*6+0] * mat[2*6+1] - mat[1*6+1] * mat[2*6+0];

	det = mat[0*6+0] * c0 + mat[0*6+1] * c1 + mat[0*6+2] * c2;

	if( fabs( det ) < MATRIX_INVERSE_EPSILON ) {
		return false;
	}

	invDet = 1.0f / det;

	r0[ 0 ][ 0 ] = c0 * invDet;
	r0[ 0 ][ 1 ] = ( mat[0*6+2] * mat[2*6+1] - mat[0*6+1] * mat[2*6+2] ) * invDet;
	r0[ 0 ][ 2 ] = ( mat[0*6+1] * mat[1*6+2] - mat[0*6+2] * mat[1*6+1] ) * invDet;
	r0[ 1 ][ 0 ] = c1 * invDet;
	r0[ 1 ][ 1 ] = ( mat[0*6+0] * mat[2*6+2] - mat[0*6+2] * mat[2*6+0] ) * invDet;
	r0[ 1 ][ 2 ] = ( mat[0*6+2] * mat[1*6+0] - mat[0*6+0] * mat[1*6+2] ) * invDet;
	r0[ 2 ][ 0 ] = c2 * invDet;
	r0[ 2 ][ 1 ] = ( mat[0*6+1] * mat[2*6+0] - mat[0*6+0] * mat[2*6+1] ) * invDet;
	r0[ 2 ][ 2 ] = ( mat[0*6+0] * mat[1*6+1] - mat[0*6+1] * mat[1*6+0] ) * invDet;

	// r1 = r0 * m1;
	r1[ 0 ][ 0 ] = r0[ 0 ][ 0 ] * mat[0*6+3] + r0[ 0 ][ 1 ] * mat[1*6+3] + r0[ 0 ][ 2 ] * mat[2*6+3];
	r1[ 0 ][ 1 ] = r0[ 0 ][ 0 ] * mat[0*6+4] + r0[ 0 ][ 1 ] * mat[1*6+4] + r0[ 0 ][ 2 ] * mat[2*6+4];
	r1[ 0 ][ 2 ] = r0[ 0 ][ 0 ] * mat[0*6+5] + r0[ 0 ][ 1 ] * mat[1*6+5] + r0[ 0 ][ 2 ] * mat[2*6+5];
	r1[ 1 ][ 0 ] = r0[ 1 ][ 0 ] * mat[0*6+3] + r0[ 1 ][ 1 ] * mat[1*6+3] + r0[ 1 ][ 2 ] * mat[2*6+3];
	r1[ 1 ][ 1 ] = r0[ 1 ][ 0 ] * mat[0*6+4] + r0[ 1 ][ 1 ] * mat[1*6+4] + r0[ 1 ][ 2 ] * mat[2*6+4];
	r1[ 1 ][ 2 ] = r0[ 1 ][ 0 ] * mat[0*6+5] + r0[ 1 ][ 1 ] * mat[1*6+5] + r0[ 1 ][ 2 ] * mat[2*6+5];
	r1[ 2 ][ 0 ] = r0[ 2 ][ 0 ] * mat[0*6+3] + r0[ 2 ][ 1 ] * mat[1*6+3] + r0[ 2 ][ 2 ] * mat[2*6+3];
	r1[ 2 ][ 1 ] = r0[ 2 ][ 0 ] * mat[0*6+4] + r0[ 2 ][ 1 ] * mat[1*6+4] + r0[ 2 ][ 2 ] * mat[2*6+4];
	r1[ 2 ][ 2 ] = r0[ 2 ][ 0 ] * mat[0*6+5] + r0[ 2 ][ 1 ] * mat[1*6+5] + r0[ 2 ][ 2 ] * mat[2*6+5];

	// r2 = m2 * r1;
	r2[ 0 ][ 0 ] = mat[3*6+0] * r1[ 0 ][ 0 ] + mat[3*6+1] * r1[ 1 ][ 0 ] + mat[3*6+2] * r1[ 2 ][ 0 ];
	r2[ 0 ][ 1 ] = mat[3*6+0] * r1[ 0 ][ 1 ] + mat[3*6+1] * r1[ 1 ][ 1 ] + mat[3*6+2] * r1[ 2 ][ 1 ];
	r2[ 0 ][ 2 ] = mat[3*6+0] * r1[ 0 ][ 2 ] + mat[3*6+1] * r1[ 1 ][ 2 ] + mat[3*6+2] * r1[ 2 ][ 2 ];
	r2[ 1 ][ 0 ] = mat[4*6+0] * r1[ 0 ][ 0 ] + mat[4*6+1] * r1[ 1 ][ 0 ] + mat[4*6+2] * r1[ 2 ][ 0 ];
	r2[ 1 ][ 1 ] = mat[4*6+0] * r1[ 0 ][ 1 ] + mat[4*6+1] * r1[ 1 ][ 1 ] + mat[4*6+2] * r1[ 2 ][ 1 ];
	r2[ 1 ][ 2 ] = mat[4*6+0] * r1[ 0 ][ 2 ] + mat[4*6+1] * r1[ 1 ][ 2 ] + mat[4*6+2] * r1[ 2 ][ 2 ];
	r2[ 2 ][ 0 ] = mat[5*6+0] * r1[ 0 ][ 0 ] + mat[5*6+1] * r1[ 1 ][ 0 ] + mat[5*6+2] * r1[ 2 ][ 0 ];
	r2[ 2 ][ 1 ] = mat[5*6+0] * r1[ 0 ][ 1 ] + mat[5*6+1] * r1[ 1 ][ 1 ] + mat[5*6+2] * r1[ 2 ][ 1 ];
	r2[ 2 ][ 2 ] = mat[5*6+0] * r1[ 0 ][ 2 ] + mat[5*6+1] * r1[ 1 ][ 2 ] + mat[5*6+2] * r1[ 2 ][ 2 ];

	// r3 = r2 - m3;
	r3[ 0 ][ 0 ] = r2[ 0 ][ 0 ] - mat[3*6+3];
	r3[ 0 ][ 1 ] = r2[ 0 ][ 1 ] - mat[3*6+4];
	r3[ 0 ][ 2 ] = r2[ 0 ][ 2 ] - mat[3*6+5];
	r3[ 1 ][ 0 ] = r2[ 1 ][ 0 ] - mat[4*6+3];
	r3[ 1 ][ 1 ] = r2[ 1 ][ 1 ] - mat[4*6+4];
	r3[ 1 ][ 2 ] = r2[ 1 ][ 2 ] - mat[4*6+5];
	r3[ 2 ][ 0 ] = r2[ 2 ][ 0 ] - mat[5*6+3];
	r3[ 2 ][ 1 ] = r2[ 2 ][ 1 ] - mat[5*6+4];
	r3[ 2 ][ 2 ] = r2[ 2 ][ 2 ] - mat[5*6+5];

	// r3.InverseSelf( );
	r2[ 0 ][ 0 ] = r3[ 1 ][ 1 ] * r3[ 2 ][ 2 ] - r3[ 1 ][ 2 ] * r3[ 2 ][ 1 ];
	r2[ 1 ][ 0 ] = r3[ 1 ][ 2 ] * r3[ 2 ][ 0 ] - r3[ 1 ][ 0 ] * r3[ 2 ][ 2 ];
	r2[ 2 ][ 0 ] = r3[ 1 ][ 0 ] * r3[ 2 ][ 1 ] - r3[ 1 ][ 1 ] * r3[ 2 ][ 0 ];

	det = r3[ 0 ][ 0 ] * r2[ 0 ][ 0 ] + r3[ 0 ][ 1 ] * r2[ 1 ][ 0 ] + r3[ 0 ][ 2 ] * r2[ 2 ][ 0 ];

	if( fabs( det ) < MATRIX_INVERSE_EPSILON ) {
		return false;
	}

	invDet = 1.0f / det;

	r2[ 0 ][ 1 ] = r3[ 0 ][ 2 ] * r3[ 2 ][ 1 ] - r3[ 0 ][ 1 ] * r3[ 2 ][ 2 ];
	r2[ 0 ][ 2 ] = r3[ 0 ][ 1 ] * r3[ 1 ][ 2 ] - r3[ 0 ][ 2 ] * r3[ 1 ][ 1 ];
	r2[ 1 ][ 1 ] = r3[ 0 ][ 0 ] * r3[ 2 ][ 2 ] - r3[ 0 ][ 2 ] * r3[ 2 ][ 0 ];
	r2[ 1 ][ 2 ] = r3[ 0 ][ 2 ] * r3[ 1 ][ 0 ] - r3[ 0 ][ 0 ] * r3[ 1 ][ 2 ];
	r2[ 2 ][ 1 ] = r3[ 0 ][ 1 ] * r3[ 2 ][ 0 ] - r3[ 0 ][ 0 ] * r3[ 2 ][ 1 ];
	r2[ 2 ][ 2 ] = r3[ 0 ][ 0 ] * r3[ 1 ][ 1 ] - r3[ 0 ][ 1 ] * r3[ 1 ][ 0 ];

	r3[ 0 ][ 0 ] = r2[ 0 ][ 0 ] * invDet;
	r3[ 0 ][ 1 ] = r2[ 0 ][ 1 ] * invDet;
	r3[ 0 ][ 2 ] = r2[ 0 ][ 2 ] * invDet;
	r3[ 1 ][ 0 ] = r2[ 1 ][ 0 ] * invDet;
	r3[ 1 ][ 1 ] = r2[ 1 ][ 1 ] * invDet;
	r3[ 1 ][ 2 ] = r2[ 1 ][ 2 ] * invDet;
	r3[ 2 ][ 0 ] = r2[ 2 ][ 0 ] * invDet;
	r3[ 2 ][ 1 ] = r2[ 2 ][ 1 ] * invDet;
	r3[ 2 ][ 2 ] = r2[ 2 ][ 2 ] * invDet;

	// r2 = m2 * r0;
	r2[ 0 ][ 0 ] = mat[3*6+0] * r0[ 0 ][ 0 ] + mat[3*6+1] * r0[ 1 ][ 0 ] + mat[3*6+2] * r0[ 2 ][ 0 ];
	r2[ 0 ][ 1 ] = mat[3*6+0] * r0[ 0 ][ 1 ] + mat[3*6+1] * r0[ 1 ][ 1 ] + mat[3*6+2] * r0[ 2 ][ 1 ];
	r2[ 0 ][ 2 ] = mat[3*6+0] * r0[ 0 ][ 2 ] + mat[3*6+1] * r0[ 1 ][ 2 ] + mat[3*6+2] * r0[ 2 ][ 2 ];
	r2[ 1 ][ 0 ] = mat[4*6+0] * r0[ 0 ][ 0 ] + mat[4*6+1] * r0[ 1 ][ 0 ] + mat[4*6+2] * r0[ 2 ][ 0 ];
	r2[ 1 ][ 1 ] = mat[4*6+0] * r0[ 0 ][ 1 ] + mat[4*6+1] * r0[ 1 ][ 1 ] + mat[4*6+2] * r0[ 2 ][ 1 ];
	r2[ 1 ][ 2 ] = mat[4*6+0] * r0[ 0 ][ 2 ] + mat[4*6+1] * r0[ 1 ][ 2 ] + mat[4*6+2] * r0[ 2 ][ 2 ];
	r2[ 2 ][ 0 ] = mat[5*6+0] * r0[ 0 ][ 0 ] + mat[5*6+1] * r0[ 1 ][ 0 ] + mat[5*6+2] * r0[ 2 ][ 0 ];
	r2[ 2 ][ 1 ] = mat[5*6+0] * r0[ 0 ][ 1 ] + mat[5*6+1] * r0[ 1 ][ 1 ] + mat[5*6+2] * r0[ 2 ][ 1 ];
	r2[ 2 ][ 2 ] = mat[5*6+0] * r0[ 0 ][ 2 ] + mat[5*6+1] * r0[ 1 ][ 2 ] + mat[5*6+2] * r0[ 2 ][ 2 ];

	// m2 = r3 * r2;
	mat[3*6+0] = r3[ 0 ][ 0 ] * r2[ 0 ][ 0 ] + r3[ 0 ][ 1 ] * r2[ 1 ][ 0 ] + r3[ 0 ][ 2 ] * r2[ 2 ][ 0 ];
	mat[3*6+1] = r3[ 0 ][ 0 ] * r2[ 0 ][ 1 ] + r3[ 0 ][ 1 ] * r2[ 1 ][ 1 ] + r3[ 0 ][ 2 ] * r2[ 2 ][ 1 ];
	mat[3*6+2] = r3[ 0 ][ 0 ] * r2[ 0 ][ 2 ] + r3[ 0 ][ 1 ] * r2[ 1 ][ 2 ] + r3[ 0 ][ 2 ] * r2[ 2 ][ 2 ];
	mat[4*6+0] = r3[ 1 ][ 0 ] * r2[ 0 ][ 0 ] + r3[ 1 ][ 1 ] * r2[ 1 ][ 0 ] + r3[ 1 ][ 2 ] * r2[ 2 ][ 0 ];
	mat[4*6+1] = r3[ 1 ][ 0 ] * r2[ 0 ][ 1 ] + r3[ 1 ][ 1 ] * r2[ 1 ][ 1 ] + r3[ 1 ][ 2 ] * r2[ 2 ][ 1 ];
	mat[4*6+2] = r3[ 1 ][ 0 ] * r2[ 0 ][ 2 ] + r3[ 1 ][ 1 ] * r2[ 1 ][ 2 ] + r3[ 1 ][ 2 ] * r2[ 2 ][ 2 ];
	mat[5*6+0] = r3[ 2 ][ 0 ] * r2[ 0 ][ 0 ] + r3[ 2 ][ 1 ] * r2[ 1 ][ 0 ] + r3[ 2 ][ 2 ] * r2[ 2 ][ 0 ];
	mat[5*6+1] = r3[ 2 ][ 0 ] * r2[ 0 ][ 1 ] + r3[ 2 ][ 1 ] * r2[ 1 ][ 1 ] + r3[ 2 ][ 2 ] * r2[ 2 ][ 1 ];
	mat[5*6+2] = r3[ 2 ][ 0 ] * r2[ 0 ][ 2 ] + r3[ 2 ][ 1 ] * r2[ 1 ][ 2 ] + r3[ 2 ][ 2 ] * r2[ 2 ][ 2 ];

	// m0 = r0 - r1 * m2;
	mat[0*6+0] = r0[ 0 ][ 0 ] - r1[ 0 ][ 0 ] * mat[3*6+0] - r1[ 0 ][ 1 ] * mat[4*6+0] - r1[ 0 ][ 2 ] * mat[5*6+0];
	mat[0*6+1] = r0[ 0 ][ 1 ] - r1[ 0 ][ 0 ] * mat[3*6+1] - r1[ 0 ][ 1 ] * mat[4*6+1] - r1[ 0 ][ 2 ] * mat[5*6+1];
	mat[0*6+2] = r0[ 0 ][ 2 ] - r1[ 0 ][ 0 ] * mat[3*6+2] - r1[ 0 ][ 1 ] * mat[4*6+2] - r1[ 0 ][ 2 ] * mat[5*6+2];
	mat[1*6+0] = r0[ 1 ][ 0 ] - r1[ 1 ][ 0 ] * mat[3*6+0] - r1[ 1 ][ 1 ] * mat[4*6+0] - r1[ 1 ][ 2 ] * mat[5*6+0];
	mat[1*6+1] = r0[ 1 ][ 1 ] - r1[ 1 ][ 0 ] * mat[3*6+1] - r1[ 1 ][ 1 ] * mat[4*6+1] - r1[ 1 ][ 2 ] * mat[5*6+1];
	mat[1*6+2] = r0[ 1 ][ 2 ] - r1[ 1 ][ 0 ] * mat[3*6+2] - r1[ 1 ][ 1 ] * mat[4*6+2] - r1[ 1 ][ 2 ] * mat[5*6+2];
	mat[2*6+0] = r0[ 2 ][ 0 ] - r1[ 2 ][ 0 ] * mat[3*6+0] - r1[ 2 ][ 1 ] * mat[4*6+0] - r1[ 2 ][ 2 ] * mat[5*6+0];
	mat[2*6+1] = r0[ 2 ][ 1 ] - r1[ 2 ][ 0 ] * mat[3*6+1] - r1[ 2 ][ 1 ] * mat[4*6+1] - r1[ 2 ][ 2 ] * mat[5*6+1];
	mat[2*6+2] = r0[ 2 ][ 2 ] - r1[ 2 ][ 0 ] * mat[3*6+2] - r1[ 2 ][ 1 ] * mat[4*6+2] - r1[ 2 ][ 2 ] * mat[5*6+2];

	// m1 = r1 * r3;
	mat[0*6+3] = r1[ 0 ][ 0 ] * r3[ 0 ][ 0 ] + r1[ 0 ][ 1 ] * r3[ 1 ][ 0 ] + r1[ 0 ][ 2 ] * r3[ 2 ][ 0 ];
	mat[0*6+4] = r1[ 0 ][ 0 ] * r3[ 0 ][ 1 ] + r1[ 0 ][ 1 ] * r3[ 1 ][ 1 ] + r1[ 0 ][ 2 ] * r3[ 2 ][ 1 ];
	mat[0*6+5] = r1[ 0 ][ 0 ] * r3[ 0 ][ 2 ] + r1[ 0 ][ 1 ] * r3[ 1 ][ 2 ] + r1[ 0 ][ 2 ] * r3[ 2 ][ 2 ];
	mat[1*6+3] = r1[ 1 ][ 0 ] * r3[ 0 ][ 0 ] + r1[ 1 ][ 1 ] * r3[ 1 ][ 0 ] + r1[ 1 ][ 2 ] * r3[ 2 ][ 0 ];
	mat[1*6+4] = r1[ 1 ][ 0 ] * r3[ 0 ][ 1 ] + r1[ 1 ][ 1 ] * r3[ 1 ][ 1 ] + r1[ 1 ][ 2 ] * r3[ 2 ][ 1 ];
	mat[1*6+5] = r1[ 1 ][ 0 ] * r3[ 0 ][ 2 ] + r1[ 1 ][ 1 ] * r3[ 1 ][ 2 ] + r1[ 1 ][ 2 ] * r3[ 2 ][ 2 ];
	mat[2*6+3] = r1[ 2 ][ 0 ] * r3[ 0 ][ 0 ] + r1[ 2 ][ 1 ] * r3[ 1 ][ 0 ] + r1[ 2 ][ 2 ] * r3[ 2 ][ 0 ];
	mat[2*6+4] = r1[ 2 ][ 0 ] * r3[ 0 ][ 1 ] + r1[ 2 ][ 1 ] * r3[ 1 ][ 1 ] + r1[ 2 ][ 2 ] * r3[ 2 ][ 1 ];
	mat[2*6+5] = r1[ 2 ][ 0 ] * r3[ 0 ][ 2 ] + r1[ 2 ][ 1 ] * r3[ 1 ][ 2 ] + r1[ 2 ][ 2 ] * r3[ 2 ][ 2 ];

	// m3 = -r3;
	mat[3*6+3] = -r3[ 0 ][ 0 ];
	mat[3*6+4] = -r3[ 0 ][ 1 ];
	mat[3*6+5] = -r3[ 0 ][ 2 ];
	mat[4*6+3] = -r3[ 1 ][ 0 ];
	mat[4*6+4] = -r3[ 1 ][ 1 ];
	mat[4*6+5] = -r3[ 1 ][ 2 ];
	mat[5*6+3] = -r3[ 2 ][ 0 ];
	mat[5*6+4] = -r3[ 2 ][ 1 ];
	mat[5*6+5] = -r3[ 2 ][ 2 ];

	return true;
#endif
}

//===============================================================
//
//	MatX - arbitrary sized dense real matrix
//
//  The matrix lives on 16 byte aligned and 16 byte padded memory.
//
//	NOTE: due to the temporary memory pool MatX cannot be used by multiple threads.
//
//===============================================================

#define MATX_MAX_TEMP		1024
#define MATX_QUAD( x )		( ( ( ( x ) + 3 ) & ~3 ) * sizeof( float ) )
#define MATX_CLEAREND( )		int s = numRows * numColumns; while( s < ( ( s + 3 ) & ~3 ) ) { mat[s++] = 0.0f; }
#define MATX_ALLOCA( n )	( (float *) _alloca16( MATX_QUAD( n ) ) )
//#define MATX_SIMD

class MatX {
public:
					MatX( );
					explicit MatX( int rows, int columns );
					explicit MatX( int rows, int columns, float *src );
					~MatX( );

	void			Set( int rows, int columns, const float *src );
	void			Set( const Mat3 & m1, const Mat3 & m2 );
	void			Set( const Mat3 & m1, const Mat3 & m2, const Mat3 & m3, const Mat3 & m4 );

	const float *	operator [ ]( int index ) const;
	float *			operator [ ]( int index );
	MatX &		operator =( const MatX & a );
	MatX			operator *( const float a ) const;
	VecX			operator *( const VecX & vec ) const;
	MatX			operator *( const MatX & a ) const;
	MatX			operator +( const MatX & a ) const;
	MatX			operator -( const MatX & a ) const;
	MatX &		operator *=( const float a );
	MatX &		operator *=( const MatX & a );
	MatX &		operator +=( const MatX & a );
	MatX &		operator -=( const MatX & a );

	friend MatX	operator *( const float a, const MatX & m );
	friend VecX	operator *( const VecX & vec, const MatX & m );
	friend VecX &	operator *=( VecX & vec, const MatX & m );

	bool			Compare( const MatX & a ) const;								// exact compare, no epsilon
	bool			Compare( const MatX & a, const float epsilon ) const;			// compare with epsilon
	bool			operator ==( const MatX & a ) const;							// exact compare, no epsilon
	bool			operator !=( const MatX & a ) const;							// exact compare, no epsilon

	void			SetSize( int rows, int columns );								// set the number of rows/columns
	void			ChangeSize( int rows, int columns, bool makeZero = false );		// change the size keeping data intact where possible
	int				GetNumRows( ) const { return numRows; }					// get the number of rows
	int				GetNumColumns( ) const { return numColumns; }				// get the number of columns
	void			SetData( int rows, int columns, float *data );					// set float array pointer
	void			Zero( );													// clear matrix
	void			Zero( int rows, int columns );									// set size and clear matrix
	void			Identity( );												// clear to identity matrix
	void			Identity( int rows, int columns );								// set size and clear to identity matrix
	void			Diag( const VecX & v );										// create diagonal matrix from vector
	void			Random( int seed, float l = 0.0f, float u = 1.0f );				// fill matrix with random values
	void			Random( int rows, int columns, int seed, float l = 0.0f, float u = 1.0f );
	void			Negate( );													// (*this) = - (*this)
	void			Clamp( float min, float max );									// clamp all values
	MatX &		SwapRows( int r1, int r2 );										// swap rows
	MatX &		SwapColumns( int r1, int r2 );									// swap columns
	MatX &		SwapRowsColumns( int r1, int r2 );								// swap rows and columns
	MatX &		RemoveRow( int r );												// remove a row
	MatX &		RemoveColumn( int r );											// remove a column
	MatX &		RemoveRowColumn( int r );										// remove a row and column
	void			ClearUpperTriangle( );										// clear the upper triangle
	void			ClearLowerTriangle( );										// clear the lower triangle
	void			SquareSubMatrix( const MatX & m, int size );					// get square sub-matrix from 0,0 to size,size
	float			MaxDifference( const MatX & m ) const;							// return maximum element difference between this and m

	bool			IsSquare( ) const { return ( numRows == numColumns ); }
	bool			IsZero( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsIdentity( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsDiagonal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsTriDiagonal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetric( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsOrthogonal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsOrthonormal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsPMatrix( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsZMatrix( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsPositiveDefinite( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetricPositiveDefinite( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsPositiveSemiDefinite( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetricPositiveSemiDefinite( const float epsilon = MATRIX_EPSILON ) const;

	float			Trace( ) const;											// returns product of diagonal elements
	float			Determinant( ) const;										// returns determinant of matrix
	MatX			Transpose( ) const;										// returns transpose
	MatX &		TransposeSelf( );											// transposes the matrix itself
	MatX			Inverse( ) const;											// returns the inverse ( m * m.Inverse( ) = identity )
	bool			InverseSelf( );											// returns false if determinant is zero
	MatX			InverseFast( ) const;										// returns the inverse ( m * m.Inverse( ) = identity )
	bool			InverseFastSelf( );										// returns false if determinant is zero

	bool			LowerTriangularInverse( );									// in-place inversion, returns false if determinant is zero
	bool			UpperTriangularInverse( );									// in-place inversion, returns false if determinant is zero

	VecX			Multiply( const VecX & vec ) const;							// (*this) * vec
	VecX			TransposeMultiply( const VecX & vec ) const;					// this->Transpose( ) * vec

	MatX			Multiply( const MatX & a ) const;								// (*this) * a
	MatX			TransposeMultiply( const MatX & a ) const;						// this->Transpose( ) * a

	void			Multiply( VecX & dst, const VecX & vec ) const;				// dst = (*this) * vec
	void			MultiplyAdd( VecX & dst, const VecX & vec ) const;			// dst += (*this) * vec
	void			MultiplySub( VecX & dst, const VecX & vec ) const;			// dst -= (*this) * vec
	void			TransposeMultiply( VecX & dst, const VecX & vec ) const;		// dst = this->Transpose( ) * vec
	void			TransposeMultiplyAdd( VecX & dst, const VecX & vec ) const;	// dst += this->Transpose( ) * vec
	void			TransposeMultiplySub( VecX & dst, const VecX & vec ) const;	// dst -= this->Transpose( ) * vec

	void			Multiply( MatX & dst, const MatX & a ) const;					// dst = (*this) * a
	void			TransposeMultiply( MatX & dst, const MatX & a ) const;		// dst = this->Transpose( ) * a

	int				GetDimension( ) const;										// returns total number of values in matrix

	const Vec6 &	SubVec6( int row ) const;										// interpret beginning of row as a const Vec6
	Vec6 &		SubVec6( int row );												// interpret beginning of row as an Vec6
	const VecX	SubVecX( int row ) const;										// interpret complete row as a const VecX
	VecX			SubVecX( int row );												// interpret complete row as an VecX
	const float *	ToFloatPtr( ) const;										// pointer to const matrix float array
	float *			ToFloatPtr( );												// pointer to matrix float array
	const char *	ToString( int precision = 2 ) const;

	void			Update_RankOne( const VecX & v, const VecX & w, float alpha );
	void			Update_RankOneSymmetric( const VecX & v, float alpha );
	void			Update_RowColumn( const VecX & v, const VecX & w, int r );
	void			Update_RowColumnSymmetric( const VecX & v, int r );
	void			Update_Increment( const VecX & v, const VecX & w );
	void			Update_IncrementSymmetric( const VecX & v );
	void			Update_Decrement( int r );

	bool			Inverse_GaussJordan( );					// invert in-place with Gauss-Jordan elimination
	bool			Inverse_UpdateRankOne( const VecX & v, const VecX & w, float alpha );
	bool			Inverse_UpdateRowColumn( const VecX & v, const VecX & w, int r );
	bool			Inverse_UpdateIncrement( const VecX & v, const VecX & w );
	bool			Inverse_UpdateDecrement( const VecX & v, const VecX & w, int r );
	void			Inverse_Solve( VecX & x, const VecX & b ) const;

	bool			LU_Factor( int *index, float *det = NULL );		// factor in-place: L * U
	bool			LU_UpdateRankOne( const VecX & v, const VecX & w, float alpha, int *index );
	bool			LU_UpdateRowColumn( const VecX & v, const VecX & w, int r, int *index );
	bool			LU_UpdateIncrement( const VecX & v, const VecX & w, int *index );
	bool			LU_UpdateDecrement( const VecX & v, const VecX & w, const VecX & u, int r, int *index );
	void			LU_Solve( VecX & x, const VecX & b, const int *index ) const;
	void			LU_Inverse( MatX & inv, const int *index ) const;
	void			LU_UnpackFactors( MatX & L, MatX & U ) const;
	void			LU_MultiplyFactors( MatX & m, const int *index ) const;

	bool			QR_Factor( VecX & c, VecX & d );				// factor in-place: Q * R
	bool			QR_UpdateRankOne( MatX & R, const VecX & v, const VecX & w, float alpha );
	bool			QR_UpdateRowColumn( MatX & R, const VecX & v, const VecX & w, int r );
	bool			QR_UpdateIncrement( MatX & R, const VecX & v, const VecX & w );
	bool			QR_UpdateDecrement( MatX & R, const VecX & v, const VecX & w, int r );
	void			QR_Solve( VecX & x, const VecX & b, const VecX & c, const VecX & d ) const;
	void			QR_Solve( VecX & x, const VecX & b, const MatX & R ) const;
	void			QR_Inverse( MatX & inv, const VecX & c, const VecX & d ) const;
	void			QR_UnpackFactors( MatX & Q, MatX & R, const VecX & c, const VecX & d ) const;
	void			QR_MultiplyFactors( MatX & m, const VecX & c, const VecX & d ) const;

	bool			SVD_Factor( VecX & w, MatX & V );				// factor in-place: U * Diag(w) * V.Transpose( )
	void			SVD_Solve( VecX & x, const VecX & b, const VecX & w, const MatX & V ) const;
	void			SVD_Inverse( MatX & inv, const VecX & w, const MatX & V ) const;
	void			SVD_MultiplyFactors( MatX & m, const VecX & w, const MatX & V ) const;

	bool			Cholesky_Factor( );						// factor in-place: L * L.Transpose( )
	bool			Cholesky_UpdateRankOne( const VecX & v, float alpha, int offset = 0 );
	bool			Cholesky_UpdateRowColumn( const VecX & v, int r );
	bool			Cholesky_UpdateIncrement( const VecX & v );
	bool			Cholesky_UpdateDecrement( const VecX & v, int r );
	void			Cholesky_Solve( VecX & x, const VecX & b ) const;
	void			Cholesky_Inverse( MatX & inv ) const;
	void			Cholesky_MultiplyFactors( MatX & m ) const;

	bool			LDLT_Factor( );							// factor in-place: L * D * L.Transpose( )
	bool			LDLT_UpdateRankOne( const VecX & v, float alpha, int offset = 0 );
	bool			LDLT_UpdateRowColumn( const VecX & v, int r );
	bool			LDLT_UpdateIncrement( const VecX & v );
	bool			LDLT_UpdateDecrement( const VecX & v, int r );
	void			LDLT_Solve( VecX & x, const VecX & b ) const;
	void			LDLT_Inverse( MatX & inv ) const;
	void			LDLT_UnpackFactors( MatX & L, MatX & D ) const;
	void			LDLT_MultiplyFactors( MatX & m ) const;

	void			TriDiagonal_ClearTriangles( );
	bool			TriDiagonal_Solve( VecX & x, const VecX & b ) const;
	void			TriDiagonal_Inverse( MatX & inv ) const;

	bool			Eigen_SolveSymmetricTriDiagonal( VecX & eigenValues );
	bool			Eigen_SolveSymmetric( VecX & eigenValues );
	bool			Eigen_Solve( VecX & realEigenValues, VecX & imaginaryEigenValues );
	void			Eigen_SortIncreasing( VecX & eigenValues );
	void			Eigen_SortDecreasing( VecX & eigenValues );

	static void		Test( );

private:
	int				numRows;				// number of rows
	int				numColumns;				// number of columns
	int				alloced;				// floats allocated, if -1 then mat points to data set with SetData
	float *			mat;					// memory the matrix is stored

	static float	temp[MATX_MAX_TEMP+4];	// used to store intermediate results
	static float *	tempPtr;				// pointer to 16 byte aligned temporary memory
	static int		tempIndex;				// index into memory pool, wraps around

private:
	void			SetTempSize( int rows, int columns );
	float			DeterminantGeneric( ) const;
	bool			InverseSelfGeneric( );
	void			QR_Rotate( MatX & R, int i, float a, float b );
	float			Pythag( float a, float b ) const;
	void			SVD_BiDiag( VecX & w, VecX & rv1, float & anorm );
	void			SVD_InitialWV( VecX & w, MatX & V, VecX & rv1 );
	void			HouseholderReduction( VecX & diag, VecX & subd );
	bool			QL( VecX & diag, VecX & subd );
	void			HessenbergReduction( MatX & H );
	void			ComplexDivision( float xr, float xi, float yr, float yi, float & cdivr, float & cdivi );
	bool			HessenbergToRealSchur( MatX & H, VecX & realEigenValues, VecX & imaginaryEigenValues );
};

INLINE MatX::MatX( ) {
	numRows = numColumns = alloced = 0;
	mat = NULL;
}

INLINE MatX::~MatX( ) {
	// if not temp memory
	if( mat != NULL && ( mat < MatX::tempPtr || mat > MatX::tempPtr + MATX_MAX_TEMP ) && alloced != -1 ) {
		free( mat );
	}
}

INLINE MatX::MatX( int rows, int columns ) {
	numRows = numColumns = alloced = 0;
	mat = NULL;
	SetSize( rows, columns );
}

INLINE MatX::MatX( int rows, int columns, float *src ) {
	numRows = numColumns = alloced = 0;
	mat = NULL;
	SetData( rows, columns, src );
}

INLINE void MatX::Set( int rows, int columns, const float *src ) {
	SetSize( rows, columns );
	memcpy( this->mat, src, rows * columns * sizeof( float ) );
}

INLINE void MatX::Set( const Mat3 & m1, const Mat3 & m2 ) {
	int i, j;

	SetSize( 3, 6 );
	for( i = 0; i < 3; i++ ) {
		for( j = 0; j < 3; j++ ) {
			mat[(i+0) * numColumns + (j+0)] = m1[i][j];
			mat[(i+0) * numColumns + (j+3)] = m2[i][j];
		}
	}
}

INLINE void MatX::Set( const Mat3 & m1, const Mat3 & m2, const Mat3 & m3, const Mat3 & m4 ) {
	int i, j;

	SetSize( 6, 6 );
	for( i = 0; i < 3; i++ ) {
		for( j = 0; j < 3; j++ ) {
			mat[(i+0) * numColumns + (j+0)] = m1[i][j];
			mat[(i+0) * numColumns + (j+3)] = m2[i][j];
			mat[(i+3) * numColumns + (j+0)] = m3[i][j];
			mat[(i+3) * numColumns + (j+3)] = m4[i][j];
		}
	}
}

INLINE const float *MatX::operator [ ]( int index ) const {
	assert( ( index >= 0 ) && ( index < numRows ) );
	return mat + index * numColumns;
}

INLINE float *MatX::operator [ ]( int index ) {
	assert( ( index >= 0 ) && ( index < numRows ) );
	return mat + index * numColumns;
}

INLINE MatX & MatX::operator =( const MatX & a ) {
	SetSize( a.numRows, a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->Copy16( mat, a.mat, a.numRows * a.numColumns );
#else
	memcpy( mat, a.mat, a.numRows * a.numColumns * sizeof( float ) );
#endif
	MatX::tempIndex = 0;
	return *this;
}

INLINE MatX MatX::operator *( const float a ) const {
	MatX m;

	m.SetTempSize( numRows, numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->Mul16( m.mat, mat, a, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for( i = 0; i < s; i++ ) {
		m.mat[i] = mat[i] * a;
	}
#endif
	return m;
}

INLINE VecX MatX::operator *( const VecX & vec ) const {
	VecX dst;

	assert( numColumns == vec.GetSize( ) );

	dst.SetTempSize( numRows );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyVecX( dst, *this, vec );
#else
	Multiply( dst, vec );
#endif
	return dst;
}

INLINE MatX MatX::operator *( const MatX & a ) const {
	MatX dst;

	assert( numColumns == a.numRows );

	dst.SetTempSize( numRows, a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyMatX( dst, *this, a );
#else
	Multiply( dst, a );
#endif
	return dst;
}

INLINE MatX MatX::operator +( const MatX & a ) const {
	MatX m;

	assert( numRows == a.numRows && numColumns == a.numColumns );
	m.SetTempSize( numRows, numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->Add16( m.mat, mat, a.mat, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for( i = 0; i < s; i++ ) {
		m.mat[i] = mat[i] + a.mat[i];
	}
#endif
	return m;
}

INLINE MatX MatX::operator -( const MatX & a ) const {
	MatX m;

	assert( numRows == a.numRows && numColumns == a.numColumns );
	m.SetTempSize( numRows, numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->Sub16( m.mat, mat, a.mat, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for( i = 0; i < s; i++ ) {
		m.mat[i] = mat[i] - a.mat[i];
	}
#endif
	return m;
}

INLINE MatX & MatX::operator *=( const float a ) {
#ifdef MATX_SIMD
	SIMDProcessor->MulAssign16( mat, a, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for( i = 0; i < s; i++ ) {
		mat[i] *= a;
	}
#endif
	MatX::tempIndex = 0;
	return *this;
}

INLINE MatX & MatX::operator *=( const MatX & a ) {
	*this = *this * a;
	MatX::tempIndex = 0;
	return *this;
}

INLINE MatX & MatX::operator +=( const MatX & a ) {
	assert( numRows == a.numRows && numColumns == a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->AddAssign16( mat, a.mat, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for( i = 0; i < s; i++ ) {
		mat[i] += a.mat[i];
	}
#endif
	MatX::tempIndex = 0;
	return *this;
}

INLINE MatX & MatX::operator -=( const MatX & a ) {
	assert( numRows == a.numRows && numColumns == a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->SubAssign16( mat, a.mat, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for( i = 0; i < s; i++ ) {
		mat[i] -= a.mat[i];
	}
#endif
	MatX::tempIndex = 0;
	return *this;
}

INLINE MatX operator *( const float a, MatX const & m ) {
	return m * a;
}

INLINE VecX operator *( const VecX & vec, const MatX & m ) {
	return m * vec;
}

INLINE VecX & operator *=( VecX & vec, const MatX & m ) {
	vec = m * vec;
	return vec;
}

INLINE bool MatX::Compare( const MatX & a ) const {
	int i, s;

	assert( numRows == a.numRows && numColumns == a.numColumns );

	s = numRows * numColumns;
	for( i = 0; i < s; i++ ) {
		if( mat[i] != a.mat[i] ) {
			return false;
		}
	}
	return true;
}

INLINE bool MatX::Compare( const MatX & a, const float epsilon ) const {
	int i, s;

	assert( numRows == a.numRows && numColumns == a.numColumns );

	s = numRows * numColumns;
	for( i = 0; i < s; i++ ) {
		if( fabs( mat[i] - a.mat[i] ) > epsilon ) {
			return false;
		}
	}
	return true;
}

INLINE bool MatX::operator ==( const MatX & a ) const {
	return Compare( a );
}

INLINE bool MatX::operator !=( const MatX & a ) const {
	return !Compare( a );
}

INLINE void MatX::SetSize( int rows, int columns ) {
	assert( mat < MatX::tempPtr || mat > MatX::tempPtr + MATX_MAX_TEMP );
	int alloc = ( rows * columns + 3 ) & ~3;
	if( alloc > alloced && alloced != -1 ) {
		if( mat != NULL ) {
			free( mat );
		}
		mat = (float *) malloc( alloc * sizeof( float ) );
		alloced = alloc;
	}
	numRows = rows;
	numColumns = columns;
	MATX_CLEAREND( );
}

INLINE void MatX::SetTempSize( int rows, int columns ) {
	int newSize;

	newSize = ( rows * columns + 3 ) & ~3;
	assert( newSize < MATX_MAX_TEMP );
	if( MatX::tempIndex + newSize > MATX_MAX_TEMP ) {
		MatX::tempIndex = 0;
	}
	mat = MatX::tempPtr + MatX::tempIndex;
	MatX::tempIndex += newSize;
	alloced = newSize;
	numRows = rows;
	numColumns = columns;
	MATX_CLEAREND( );
}

INLINE void MatX::SetData( int rows, int columns, float *data ) {
	assert( mat < MatX::tempPtr || mat > MatX::tempPtr + MATX_MAX_TEMP );
	if( mat != NULL && alloced != -1 ) {
		free( mat );
	}
	assert( ( ( (int) data ) & 15 ) == 0 ); // data must be 16 byte aligned
	mat = data;
	alloced = -1;
	numRows = rows;
	numColumns = columns;
	MATX_CLEAREND( );
}

INLINE void MatX::Zero( ) {
#ifdef MATX_SIMD
	SIMDProcessor->Zero16( mat, numRows * numColumns );
#else
	Common::Com_Memset( mat, 0, numRows * numColumns * sizeof( float ) );
#endif
}

INLINE void MatX::Zero( int rows, int columns ) {
	SetSize( rows, columns );
#ifdef MATX_SIMD
	SIMDProcessor->Zero16( mat, numRows * numColumns );
#else
	Common::Com_Memset( mat, 0, rows * columns * sizeof( float ) );
#endif
}

INLINE void MatX::Identity( ) {
	assert( numRows == numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->Zero16( mat, numRows * numColumns );
#else
	Common::Com_Memset( mat, 0, numRows * numColumns * sizeof( float ) );
#endif
	for( int i = 0; i < numRows; i++ ) {
		mat[i * numColumns + i] = 1.0f;
	}
}

INLINE void MatX::Identity( int rows, int columns ) {
	assert( rows == columns );
	SetSize( rows, columns );
	MatX::Identity( );
}

INLINE void MatX::Diag( const VecX & v ) {
	Zero( v.GetSize( ), v.GetSize( ) );
	for( int i = 0; i < v.GetSize( ); i++ ) {
		mat[i * numColumns + i] = v[i];
	}
}

INLINE void MatX::Random( int seed, float l, float u ) {
	int i, s;
	float c;

	c = u - l;
	s = numRows * numColumns;
	for( i = 0; i < s; i++ ) {
		mat[i] = l + rand( ) * c;
	}
}

INLINE void MatX::Random( int rows, int columns, int seed, float l, float u ) {
	int i, s;
	float c;

	SetSize( rows, columns );
	c = u - l;
	s = numRows * numColumns;
	for( i = 0; i < s; i++ ) {
		mat[i] = l + rand( ) * c;
	}
}

INLINE void MatX::Negate( ) {
#ifdef MATX_SIMD
	SIMDProcessor->Negate16( mat, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for( i = 0; i < s; i++ ) {
		mat[i] = -mat[i];
	}
#endif
}

INLINE void MatX::Clamp( float min, float max ) {
	int i, s;
	s = numRows * numColumns;
	for( i = 0; i < s; i++ ) {
		if( mat[i] < min ) {
			mat[i] = min;
		} else if( mat[i] > max ) {
			mat[i] = max;
		}
	}
}

INLINE MatX & MatX::SwapRows( int r1, int r2 ) {
	float *ptr;

	ptr = (float *) _alloca16( numColumns * sizeof( float ) );
	memcpy( ptr, mat + r1 * numColumns, numColumns * sizeof( float ) );
	memcpy( mat + r1 * numColumns, mat + r2 * numColumns, numColumns * sizeof( float ) );
	memcpy( mat + r2 * numColumns, ptr, numColumns * sizeof( float ) );

	return *this;
}

INLINE MatX & MatX::SwapColumns( int r1, int r2 ) {
	int i;
	float tmp, *ptr;

	for( i = 0; i < numRows; i++ ) {
		ptr = mat + i * numColumns;
		tmp = ptr[r1];
		ptr[r1] = ptr[r2];
		ptr[r2] = tmp;
	}

	return *this;
}

INLINE MatX & MatX::SwapRowsColumns( int r1, int r2 ) {

	SwapRows( r1, r2 );
	SwapColumns( r1, r2 );
	return *this;
}

INLINE void MatX::ClearUpperTriangle( ) {
	assert( numRows == numColumns );
	for( int i = numRows-2; i >= 0; i-- ) {
		Common::Com_Memset( mat + i * numColumns + i + 1, 0, (numColumns - 1 - i) * sizeof(float) );
	}
}

INLINE void MatX::ClearLowerTriangle( ) {
	assert( numRows == numColumns );
	for( int i = 1; i < numRows; i++ ) {
		Common::Com_Memset( mat + i * numColumns, 0, i * sizeof(float) );
	}
}

INLINE void MatX::SquareSubMatrix( const MatX & m, int size ) {
	int i;
	assert( size <= m.numRows && size <= m.numColumns );
	SetSize( size, size );
	for( i = 0; i < size; i++ ) {
		memcpy( mat + i * numColumns, m.mat + i * m.numColumns, size * sizeof( float ) );
	}
}

INLINE float MatX::MaxDifference( const MatX & m ) const {
	int i, j;
	float diff, maxDiff;

	assert( numRows == m.numRows && numColumns == m.numColumns );

	maxDiff = -1.0f;
	for( i = 0; i < numRows; i++ ) {
		for( j = 0; j < numColumns; j++ ) {
			diff = fabs( mat[ i * numColumns + j ] - m[i][j] );
			if( maxDiff < 0.0f || diff > maxDiff ) {
				maxDiff = diff;
			}
		}
	}
	return maxDiff;
}

INLINE bool MatX::IsZero( const float epsilon ) const {
	// returns true if(*this) == Zero
	for( int i = 0; i < numRows; i++ ) {
		for( int j = 0; j < numColumns; j++ ) {
			if( fabs( mat[i * numColumns + j] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

INLINE bool MatX::IsIdentity( const float epsilon ) const {
	// returns true if(*this) == Identity
	assert( numRows == numColumns );
	for( int i = 0; i < numRows; i++ ) {
		for( int j = 0; j < numColumns; j++ ) {
			if( fabs( mat[i * numColumns + j] - (float)( i == j ) ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

INLINE bool MatX::IsDiagonal( const float epsilon ) const {
	// returns true if all elements are zero except for the elements on the diagonal
	assert( numRows == numColumns );
	for( int i = 0; i < numRows; i++ ) {
		for( int j = 0; j < numColumns; j++ ) {
			if( i != j && fabs( mat[i * numColumns + j] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

INLINE bool MatX::IsTriDiagonal( const float epsilon ) const {
	// returns true if all elements are zero except for the elements on the diagonal plus or minus one column

	if( numRows != numColumns ) {
		return false;
	}
	for( int i = 0; i < numRows-2; i++ ) {
		for( int j = i+2; j < numColumns; j++ ) {
			if( fabs( (*this)[i][j] ) > epsilon ) {
				return false;
			}
			if( fabs( (*this)[j][i] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

INLINE bool MatX::IsSymmetric( const float epsilon ) const {
	// (*this)[i][j] == (*this)[j][i]
	if( numRows != numColumns ) {
		return false;
	}
	for( int i = 0; i < numRows; i++ ) {
		for( int j = 0; j < numColumns; j++ ) {
			if( fabs( mat[ i * numColumns + j ] - mat[ j * numColumns + i ] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

INLINE float MatX::Trace( ) const {
	float trace = 0.0f;

	assert( numRows == numColumns );

	// sum of elements on the diagonal
	for( int i = 0; i < numRows; i++ ) {
		trace += mat[i * numRows + i];
	}
	return trace;
}

INLINE float MatX::Determinant( ) const {

	assert( numRows == numColumns );

	switch( numRows ) {
		case 1:
			return mat[ 0 ];
		case 2:
			return reinterpret_cast<const Mat2 *>(mat)->Determinant( );
		case 3:
			return reinterpret_cast<const Mat3 *>(mat)->Determinant( );
		case 4:
			return reinterpret_cast<const Mat4 *>(mat)->Determinant( );
		case 5:
			return reinterpret_cast<const Mat5 *>(mat)->Determinant( );
		case 6:
			return reinterpret_cast<const Mat6 *>(mat)->Determinant( );
		default:
			return DeterminantGeneric( );
	}
	return 0.0f;
}

INLINE MatX MatX::Transpose( ) const {
	MatX transpose;
	int i, j;

	transpose.SetTempSize( numColumns, numRows );

	for( i = 0; i < numRows; i++ ) {
		for( j = 0; j < numColumns; j++ ) {
			transpose.mat[j * transpose.numColumns + i] = mat[i * numColumns + j];
		}
	}

	return transpose;
}

INLINE MatX & MatX::TransposeSelf( ) {
	*this = Transpose( );
	return *this;
}

INLINE MatX MatX::Inverse( ) const {
	MatX invMat;

	invMat.SetTempSize( numRows, numColumns );
	memcpy( invMat.mat, mat, numRows * numColumns * sizeof( float ) );
	int r = invMat.InverseSelf( );
	assert( r );
	return invMat;
}

INLINE bool MatX::InverseSelf( ) {

	assert( numRows == numColumns );

	switch( numRows ) {
		case 1:
			if( fabs( mat[ 0 ] ) < MATRIX_INVERSE_EPSILON ) {
				return false;
			}
			mat[ 0 ] = 1.0f / mat[ 0 ];
			return true;
		case 2:
			return reinterpret_cast<Mat2 *>(mat)->InverseSelf( );
		case 3:
			return reinterpret_cast<Mat3 *>(mat)->InverseSelf( );
		case 4:
			return reinterpret_cast<Mat4 *>(mat)->InverseSelf( );
		case 5:
			return reinterpret_cast<Mat5 *>(mat)->InverseSelf( );
		case 6:
			return reinterpret_cast<Mat6 *>(mat)->InverseSelf( );
		default:
			return InverseSelfGeneric( );
	}
	return false;
}

INLINE MatX MatX::InverseFast( ) const {
	MatX invMat;

	invMat.SetTempSize( numRows, numColumns );
	memcpy( invMat.mat, mat, numRows * numColumns * sizeof( float ) );
	int r = invMat.InverseFastSelf( );
	assert( r );
	return invMat;
}

INLINE bool MatX::InverseFastSelf( ) {

	assert( numRows == numColumns );

	switch( numRows ) {
		case 1:
			if( fabs( mat[ 0 ] ) < MATRIX_INVERSE_EPSILON ) {
				return false;
			}
			mat[ 0 ] = 1.0f / mat[ 0 ];
			return true;
		case 2:
			return reinterpret_cast<Mat2 *>(mat)->InverseFastSelf( );
		case 3:
			return reinterpret_cast<Mat3 *>(mat)->InverseFastSelf( );
		case 4:
			return reinterpret_cast<Mat4 *>(mat)->InverseFastSelf( );
		case 5:
			return reinterpret_cast<Mat5 *>(mat)->InverseFastSelf( );
		case 6:
			return reinterpret_cast<Mat6 *>(mat)->InverseFastSelf( );
		default:
			return InverseSelfGeneric( );
	}
	return false;
}

INLINE VecX MatX::Multiply( const VecX & vec ) const {
	VecX dst;

	assert( numColumns == vec.GetSize( ) );

	dst.SetTempSize( numRows );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyVecX( dst, *this, vec );
#else
	Multiply( dst, vec );
#endif
	return dst;
}

INLINE MatX MatX::Multiply( const MatX & a ) const {
	MatX dst;

	assert( numColumns == a.numRows );

	dst.SetTempSize( numRows, a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyMatX( dst, *this, a );
#else
	Multiply( dst, a );
#endif
	return dst;
}

INLINE VecX MatX::TransposeMultiply( const VecX & vec ) const {
	VecX dst;

	assert( numRows == vec.GetSize( ) );

	dst.SetTempSize( numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplyVecX( dst, *this, vec );
#else
	TransposeMultiply( dst, vec );
#endif
	return dst;
}

INLINE MatX MatX::TransposeMultiply( const MatX & a ) const {
	MatX dst;

	assert( numRows == a.numRows );

	dst.SetTempSize( numColumns, a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplyMatX( dst, *this, a );
#else
	TransposeMultiply( dst, a );
#endif
	return dst;
}

INLINE void MatX::Multiply( VecX & dst, const VecX & vec ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyVecX( dst, *this, vec );
#else
	int i, j;
	const float *mPtr, *vPtr;
	float *dstPtr;

	mPtr = mat;
	vPtr = vec.ToFloatPtr( );
	dstPtr = dst.ToFloatPtr( );
	for( i = 0; i < numRows; i++ ) {
		float sum = mPtr[ 0 ] * vPtr[ 0 ];
		for( j = 1; j < numColumns; j++ ) {
			sum += mPtr[j] * vPtr[j];
		}
		dstPtr[i] = sum;
		mPtr += numColumns;
	}
#endif
}

INLINE void MatX::MultiplyAdd( VecX & dst, const VecX & vec ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyAddVecX( dst, *this, vec );
#else
	int i, j;
	const float *mPtr, *vPtr;
	float *dstPtr;

	mPtr = mat;
	vPtr = vec.ToFloatPtr( );
	dstPtr = dst.ToFloatPtr( );
	for( i = 0; i < numRows; i++ ) {
		float sum = mPtr[ 0 ] * vPtr[ 0 ];
		for( j = 1; j < numColumns; j++ ) {
			sum += mPtr[j] * vPtr[j];
		}
		dstPtr[i] += sum;
		mPtr += numColumns;
	}
#endif
}

INLINE void MatX::MultiplySub( VecX & dst, const VecX & vec ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplySubVecX( dst, *this, vec );
#else
	int i, j;
	const float *mPtr, *vPtr;
	float *dstPtr;

	mPtr = mat;
	vPtr = vec.ToFloatPtr( );
	dstPtr = dst.ToFloatPtr( );
	for( i = 0; i < numRows; i++ ) {
		float sum = mPtr[ 0 ] * vPtr[ 0 ];
		for( j = 1; j < numColumns; j++ ) {
			sum += mPtr[j] * vPtr[j];
		}
		dstPtr[i] -= sum;
		mPtr += numColumns;
	}
#endif
}

INLINE void MatX::TransposeMultiply( VecX & dst, const VecX & vec ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplyVecX( dst, *this, vec );
#else
	int i, j;
	const float *mPtr, *vPtr;
	float *dstPtr;

	vPtr = vec.ToFloatPtr( );
	dstPtr = dst.ToFloatPtr( );
	for( i = 0; i < numColumns; i++ ) {
		mPtr = mat + i;
		float sum = mPtr[ 0 ] * vPtr[ 0 ];
		for( j = 1; j < numRows; j++ ) {
			mPtr += numColumns;
			sum += mPtr[ 0 ] * vPtr[j];
		}
		dstPtr[i] = sum;
	}
#endif
}

INLINE void MatX::TransposeMultiplyAdd( VecX & dst, const VecX & vec ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplyAddVecX( dst, *this, vec );
#else
	int i, j;
	const float *mPtr, *vPtr;
	float *dstPtr;

	vPtr = vec.ToFloatPtr( );
	dstPtr = dst.ToFloatPtr( );
	for( i = 0; i < numColumns; i++ ) {
		mPtr = mat + i;
		float sum = mPtr[ 0 ] * vPtr[ 0 ];
		for( j = 1; j < numRows; j++ ) {
			mPtr += numColumns;
			sum += mPtr[ 0 ] * vPtr[j];
		}
		dstPtr[i] += sum;
	}
#endif
}

INLINE void MatX::TransposeMultiplySub( VecX & dst, const VecX & vec ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplySubVecX( dst, *this, vec );
#else
	int i, j;
	const float *mPtr, *vPtr;
	float *dstPtr;

	vPtr = vec.ToFloatPtr( );
	dstPtr = dst.ToFloatPtr( );
	for( i = 0; i < numColumns; i++ ) {
		mPtr = mat + i;
		float sum = mPtr[ 0 ] * vPtr[ 0 ];
		for( j = 1; j < numRows; j++ ) {
			mPtr += numColumns;
			sum += mPtr[ 0 ] * vPtr[j];
		}
		dstPtr[i] -= sum;
	}
#endif
}

INLINE void MatX::Multiply( MatX & dst, const MatX & a ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyMatX( dst, *this, a );
#else
	int i, j, k, l, n;
	float *dstPtr;
	const float *m1Ptr, *m2Ptr;
	float sum;

	assert( numColumns == a.numRows );

	dstPtr = dst.ToFloatPtr( );
	m1Ptr = ToFloatPtr( );
	m2Ptr = a.ToFloatPtr( );
	k = numRows;
	l = a.GetNumColumns( );

	for( i = 0; i < k; i++ ) {
		for( j = 0; j < l; j++ ) {
			m2Ptr = a.ToFloatPtr( ) + j;
			sum = m1Ptr[ 0 ] * m2Ptr[ 0 ];
			for( n = 1; n < numColumns; n++ ) {
				m2Ptr += l;
				sum += m1Ptr[n] * m2Ptr[ 0 ];
			}
			*dstPtr++ = sum;
		}
		m1Ptr += numColumns;
	}
#endif
}

INLINE void MatX::TransposeMultiply( MatX & dst, const MatX & a ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplyMatX( dst, *this, a );
#else
	int i, j, k, l, n;
	float *dstPtr;
	const float *m1Ptr, *m2Ptr;
	float sum;

	assert( numRows == a.numRows );

	dstPtr = dst.ToFloatPtr( );
	m1Ptr = ToFloatPtr( );
	k = numColumns;
	l = a.numColumns;

	for( i = 0; i < k; i++ ) {
		for( j = 0; j < l; j++ ) {
			m1Ptr = ToFloatPtr( ) + i;
			m2Ptr = a.ToFloatPtr( ) + j;
			sum = m1Ptr[ 0 ] * m2Ptr[ 0 ];
			for( n = 1; n < numRows; n++ ) {
				m1Ptr += numColumns;
				m2Ptr += a.numColumns;
				sum += m1Ptr[ 0 ] * m2Ptr[ 0 ];
			}
			*dstPtr++ = sum;
		}
	}
#endif
}

INLINE int MatX::GetDimension( ) const {
	return numRows * numColumns;
}

INLINE const Vec6 & MatX::SubVec6( int row ) const {
	assert( numColumns >= 6 && row >= 0 && row < numRows );
	return *reinterpret_cast<const Vec6 *>(mat + row * numColumns);
}

INLINE Vec6 & MatX::SubVec6( int row ) {
	assert( numColumns >= 6 && row >= 0 && row < numRows );
	return *reinterpret_cast<Vec6 *>(mat + row * numColumns);
}

INLINE const VecX MatX::SubVecX( int row ) const {
	VecX v;
	assert( row >= 0 && row < numRows );
	v.SetData( numColumns, mat + row * numColumns );
	return v;
}

INLINE VecX MatX::SubVecX( int row ) {
	VecX v;
	assert( row >= 0 && row < numRows );
	v.SetData( numColumns, mat + row * numColumns );
	return v;
}

INLINE const float *MatX::ToFloatPtr( ) const {
	return mat;
}

INLINE float *MatX::ToFloatPtr( ) {
	return mat;
}

INLINE Mat3 Vec3::ToMat3( ) const {

	Mat3	mat;
	float	d;

	mat[ 0 ] = *this;
	d = x * x + y * y;
	if( !d ) {
		mat[ 1 ][ 0 ] = 1.0f;
		mat[ 1 ][ 1 ] = 0.0f;
		mat[ 1 ][ 2 ] = 0.0f;
	} else {
		d = InvSqrt( d );
		mat[ 1 ][ 0 ] = -y * d;
		mat[ 1 ][ 1 ] = x * d;
		mat[ 1 ][ 2 ] = 0.0f;
	}
	mat[ 2 ] = Cross( mat[ 1 ] );

	return mat;
}

// // //// // //// // //// //
/// MatT NxN type matrix
// //// // //// // //// //

template< class type, int dimension, class math_type = float > class MatT {
private:
	typedef VecT< type, dimension, math_type >	t_vec;
	t_vec					d_rows[ dimension ];

public:
							MatT( );

	const t_vec &			operator [ ]( int index ) const;
	t_vec &					operator [ ]( int index );
	MatT					operator -( ) const;
	MatT					operator *( const math_type a ) const;
	t_vec					operator *( const t_vec & vec ) const;
	MatT					operator *( const MatT & a ) const;
	MatT					operator +( const MatT & a ) const;
	MatT					operator -( const MatT & a ) const;
	MatT &					operator *=( const math_type a );
	MatT &					operator *=( const MatT & a );
	MatT &					operator +=( const MatT & a );
	MatT &					operator -=( const MatT & a );

	/// exact compare, no epsilon
	bool					Compare( const MatT & a ) const;

	/// exact compare, no epsilon
	bool					operator ==( const MatT & a ) const;

	/// exact compare, no epsilon
	bool					operator !=( const MatT & a ) const;

	void					Zero( );
	void					Identity( );
	bool					IsIdentity( ) const;
	bool					IsSymmetric( ) const;
	bool					IsDiagonal( ) const;

	math_type				Trace( ) const;

	int						GetDimension( ) const;

	const type *			Ptr( ) const;
	type *					Ptr( );
	const char *			ToString( int precision = 2 ) const;
};

template< class type, int dimension, class math_type >
INLINE MatT< type, dimension, math_type >::MatT( ) {
}

template< class type, int dimension, class math_type >
INLINE const VecT< type, dimension, math_type > & MatT< type, dimension, math_type >::operator [ ]( int index ) const {
	assert( ( index >= 0 ) && ( index < dimension ) );
	return d_rows[ index ];
}

template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type > & MatT< type, dimension, math_type >::operator [ ]( int index ) {
	assert( ( index >= 0 ) && ( index < dimension ) );
	return d_rows[ index ];
}

template< class type, int dimension, class math_type >
INLINE MatT< type, dimension, math_type > MatT< type, dimension, math_type >::operator -( ) const {
	MatT retval;
	for( int i = 0; i < dimension; i++ )
		retval.d_rows[ 0 ] = -d_rows[ 0 ];
	return retval;
}

template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type > MatT< type, dimension, math_type >::operator *( const t_vec & vec ) const {
	t_vec retval;
	for( int i = 0; i < dimension; i++ )
		retval[ i ] = d_rows[ i ] * vec;
	return retval;
}

template< class type, int dimension, class math_type >
INLINE MatT< type, dimension, math_type > MatT< type, dimension, math_type >::operator *( const MatT & a ) const {
	const type * m1Ptr = reinterpret_cast< const type * >( this );
	const type * m2Ptr = reinterpret_cast< const type * >( &a );
	MatT retval;
	type * dstPtr = reinterpret_cast< float * >( &retval );
	for( int i = 0; i < dimension; i++ ) {
		for( int j = 0; j < dimension; j++ ) {
			*dstPtr = m1Ptr[ 0 ] * m2Ptr[ 0 * dimension + j ];
			for( int k = 1; k < dimension; k++ )
				*dstPtr += m1Ptr[ k ] * m2Ptr[ k * dimension + j ];
			dstPtr++;
		}
		m1Ptr += dimension;
	}
	return retval;
}

template< class type, int dimension, class math_type >
INLINE MatT< type, dimension, math_type > MatT< type, dimension, math_type >::operator *( const math_type a ) const {
	MatT retval;
	for( int i = 0; i < dimension; i++ )
		retval = d_rows[ i ] * a;
	return retval;
}

template< class type, int dimension, class math_type >
INLINE MatT< type, dimension, math_type > MatT< type, dimension, math_type >::operator +( const MatT & a ) const {
	MatT retval;
	for( int i = 0; i < dimension; i++ )
		retval[ i ] = d_rows[ i ] + a.d_rows[ i ];
	return retval;
}

template< class type, int dimension, class math_type >
INLINE MatT< type, dimension, math_type > MatT< type, dimension, math_type >::operator -( const MatT & a ) const {
	MatT retval;
	for( int i = 0; i < dimension; i++ )
		retval[ i ] = d_rows[ i ] - a.d_rows[ i ];
	return retval;
}

template< class type, int dimension, class math_type >
INLINE MatT< type, dimension, math_type > & MatT< type, dimension, math_type >::operator *=( const math_type a ) {
	for( int i = 0; i < dimension; i++ )
		d_rows[ i ] *= a;
    return *this;
}

template< class type, int dimension, class math_type >
INLINE MatT< type, dimension, math_type > & MatT< type, dimension, math_type >::operator *=( const MatT & a ) {
	const type * m1Ptr = reinterpret_cast< const type * >( this );
	const type * m2Ptr = reinterpret_cast< const type * >( &a );
	MatT retval;
	type * dstPtr = reinterpret_cast< float * >( &this );
	for( int i = 0; i < dimension; i++ ) {
		for( int j = 0; j < dimension; j++ ) {
			*dstPtr = m1Ptr[ 0 ] * m2Ptr[ 0 * dimension + j ];
			for( int k = 1; k < dimension; k++ )
				*dstPtr += m1Ptr[ k ] * m2Ptr[ k * dimension + j ];
			dstPtr++;
		}
		m1Ptr += dimension;
	}
	return *this;
}

template< class type, int dimension, class math_type >
INLINE MatT< type, dimension, math_type > & MatT< type, dimension, math_type >::operator +=( const MatT & a ) {
	for( int i = 0; i < dimension; i++ )
		d_rows[ i ] += a.d_rows[ i ];
    return *this;
}

template< class type, int dimension, class math_type >
INLINE MatT< type, dimension, math_type > & MatT< type, dimension, math_type >::operator -=( const MatT & a ) {
	for( int i = 0; i < dimension; i++ )
		d_rows[ i ] -= a.d_rows[ i ];
    return *this;
}

template< class type, int dimension, class math_type >
INLINE bool MatT< type, dimension, math_type >::Compare( const MatT & a ) const {
	for( int i = 0; i < dimension; i++ )
		if( !d_rows[ i ].Compare( a.d_rows[ i ] ) )
			return false;
	return true;
}

template< class type, int dimension, class math_type >
INLINE bool MatT< type, dimension, math_type >::operator ==( const MatT & a ) const {
	for( int i = 0; i < dimension; i++ )
		if( !d_rows[ i ].Compare( a.d_rows[ i ] ) )
			return false;
	return true;
}

template< class type, int dimension, class math_type >
INLINE bool MatT< type, dimension, math_type >::operator !=( const MatT & a ) const {
	for( int i = 0; i < dimension; i++ )
		if( d_rows[ i ].Compare( a.d_rows[ i ] ) )
			return false;
	return true;
}

template< class type, int dimension, class math_type >
INLINE void MatT< type, dimension, math_type >::Zero( ) {
	for( int i = 0; i < dimension; i++ )
		d_rows[ i ].Zero( );
}

template< class type, int dimension, class math_type >
INLINE void MatT< type, dimension, math_type >::Identity( ) {
	for( int i = 0; i < dimension; i++ )
		for( int j = 0; j < dimension; j++ )
			d_rows[ i ][ j ] = ( type )( ( i == j ) ? 1 : 0 );
}

template< class type, int dimension, class math_type >
INLINE bool MatT< type, dimension, math_type >::IsIdentity( ) const {
	return Compare( mat2_identity, epsilon );
	for( int i = 0; i < dimension; i++ )
		for( int j = 0; j < dimension; j++ )
			if( ( ( i == j ) && ( d_rows[ i ][ i ] != ( type )1 ) ) ||
				( d_rows[ i ][ i ] != ( type )0 ) )
					return false;
	return true;
}

template< class type, int dimension, class math_type >
INLINE bool MatT< type, dimension, math_type >::IsSymmetric( ) const {
	for( int i = 1; i < dimension; i++ )
		for( int j = 0; j < i; j++ )
			if( fabs( ( float )( d_rows[ i ][ j ] - d_rows[ j ][ i ] ) ) )
				return false;
	return true;
}

template< class type, int dimension, class math_type >
INLINE bool MatT< type, dimension, math_type >::IsDiagonal( ) const {
	for( int i = 0; i < dimension; i++ )
		for( int j = 0; j < dimension; j++ )
			if( i != j && fabs( ( float )d_rows[ i ][ j ] ) )
				return false;
	return true;
}

template< class type, int dimension, class math_type >
INLINE math_type MatT< type, dimension, math_type >::Trace( ) const {
	math_type = ( math_type )( d_rows[ 0 ][ 0 ] );
	for( int i = 1; i < dimension; i++ )
		math_type += ( math_type )( d_rows[ i ][ i ] );
}

template< class type, int dimension, class math_type >
INLINE int MatT< type, dimension, math_type >::GetDimension( ) const {
	return dimension;
}

template< class type, int dimension, class math_type >
INLINE const type * MatT< type, dimension, math_type >::Ptr( ) const {
	return d_rows[ 0 ].Ptr( );
}

template< class type, int dimension, class math_type >
INLINE type * MatT< type, dimension, math_type >::Ptr( ) {
	return d_rows[ 0 ].Ptr( );
}

#endif
