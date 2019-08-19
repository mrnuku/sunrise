#include "precompiled.h"
#pragma hdrstop

//===============================================================
//
//	idVecX
//
//===============================================================

float	VecX::temp[VECX_MAX_TEMP+4];
float *	VecX::tempPtr = (float *) ( ( (int) VecX::temp + 15 ) & ~15 );
int		VecX::tempIndex = 0;

Mat2 mat2_identity( Vec2( 1, 0 ), Vec2( 0, 1 ) );

//===============================================================
//
//	Mat3
//
//===============================================================

Mat3 mat3_zero( Vec3( 0, 0, 0 ), Vec3( 0, 0, 0 ), Vec3( 0, 0, 0 ) );
Mat3 mat3_identity( Vec3( 1, 0, 0 ), Vec3( 0, 1, 0 ), Vec3( 0, 0, 1 ) );

/*
============
Mat3::ToAngles
============
*/
Angles Mat3::ToAngles( void ) const {
	Angles	angles;
	float		theta;
	float		cp;
	float		sp;

	sp = mat[ 0 ][ 2 ];

	// cap off our sin value so that we don't get any NANs
	if ( sp > 1.0f ) {
		sp = 1.0f;
	} else if ( sp < -1.0f ) {
		sp = -1.0f;
	}

	theta = -asin( sp );
	cp = cos( theta );

	if ( cp > 8192.0f * FLT_EPSILON ) {
		angles.pitch	= RAD2DEG( theta );
		angles.yaw		= RAD2DEG( atan2( mat[ 0 ][ 1 ], mat[ 0 ][ 0 ] ) );
		angles.roll		= RAD2DEG( atan2( mat[ 1 ][ 2 ], mat[ 2 ][ 2 ] ) );
	} else {
		angles.pitch	= RAD2DEG( theta );
		angles.yaw		= RAD2DEG( -atan2( mat[ 1 ][ 0 ], mat[ 1 ][ 1 ] ) );
		angles.roll		= 0;
	}
	return angles;
}

/*
============
Mat3::ToQuat
============
*/
Quat Mat3::ToQuat( void ) const {
	Quat		q;
	float		trace;
	float		s;
	float		t;
	int     	i;
	int			j;
	int			k;

	static int 	next[ 3 ] = { 1, 2, 0 };

	trace = mat[ 0 ][ 0 ] + mat[ 1 ][ 1 ] + mat[ 2 ][ 2 ];

	if ( trace > 0.0f ) {

		t = trace + 1.0f;
		s = InvSqrt( t ) * 0.5f;

		q[3] = s * t;
		q[0] = ( mat[ 2 ][ 1 ] - mat[ 1 ][ 2 ] ) * s;
		q[1] = ( mat[ 0 ][ 2 ] - mat[ 2 ][ 0 ] ) * s;
		q[2] = ( mat[ 1 ][ 0 ] - mat[ 0 ][ 1 ] ) * s;

	} else {

		i = 0;
		if ( mat[ 1 ][ 1 ] > mat[ 0 ][ 0 ] ) {
			i = 1;
		}
		if ( mat[ 2 ][ 2 ] > mat[ i ][ i ] ) {
			i = 2;
		}
		j = next[ i ];
		k = next[ j ];

		t = ( mat[ i ][ i ] - ( mat[ j ][ j ] + mat[ k ][ k ] ) ) + 1.0f;
		s = InvSqrt( t ) * 0.5f;

		q[i] = s * t;
		q[3] = ( mat[ k ][ j ] - mat[ j ][ k ] ) * s;
		q[j] = ( mat[ j ][ i ] + mat[ i ][ j ] ) * s;
		q[k] = ( mat[ k ][ i ] + mat[ i ][ k ] ) * s;
	}
	return q;
}

/*
============
Mat3::ToCQuat
============
*/
CQuat Mat3::ToCQuat( void ) const {
	Quat q = ToQuat();
	if ( q.w < 0.0f ) {
		return CQuat( -q.x, -q.y, -q.z );
	}
	return CQuat( q.x, q.y, q.z );
}

/*
============
Mat3::ToRotation
============
*/
Rotation Mat3::ToRotation( void ) const {
	Rotation	r;
	float		trace;
	float		s;
	float		t;
	int     	i;
	int			j;
	int			k;
	static int 	next[ 3 ] = { 1, 2, 0 };

	trace = mat[ 0 ][ 0 ] + mat[ 1 ][ 1 ] + mat[ 2 ][ 2 ];
	if ( trace > 0.0f ) {

		t = trace + 1.0f;
		s = InvSqrt( t ) * 0.5f;
    
		r.angle = s * t;
		r.vec[0] = ( mat[ 2 ][ 1 ] - mat[ 1 ][ 2 ] ) * s;
		r.vec[1] = ( mat[ 0 ][ 2 ] - mat[ 2 ][ 0 ] ) * s;
		r.vec[2] = ( mat[ 1 ][ 0 ] - mat[ 0 ][ 1 ] ) * s;

	} else {

		i = 0;
		if ( mat[ 1 ][ 1 ] > mat[ 0 ][ 0 ] ) {
			i = 1;
		}
		if ( mat[ 2 ][ 2 ] > mat[ i ][ i ] ) {
			i = 2;
		}
		j = next[ i ];  
		k = next[ j ];
    
		t = ( mat[ i ][ i ] - ( mat[ j ][ j ] + mat[ k ][ k ] ) ) + 1.0f;
		s = InvSqrt( t ) * 0.5f;
    
		r.vec[i]	= s * t;
		r.angle		= ( mat[ k ][ j ] - mat[ j ][ k ] ) * s;
		r.vec[j]	= ( mat[ j ][ i ] + mat[ i ][ j ] ) * s;
		r.vec[k]	= ( mat[ k ][ i ] + mat[ i ][ k ] ) * s;
	}
	r.angle = ACos( r.angle );
	if ( fabs( r.angle ) < 1e-10f ) {
		r.vec.Set( 0.0f, 0.0f, 1.0f );
		r.angle = 0.0f;
	} else {
		//vec *= (1.0f / sin( angle ));
		r.vec.Normalize();
		r.vec.FixDegenerateNormal();
		r.angle *= RAD2DEG( 2.0f );
	}

	r.origin.Zero();
	r.axis = *this;
	r.axisValid = true;
	return r;
}

/*
=================
Mat3::ToAngularVelocity
=================
*/
Vec3 Mat3::ToAngularVelocity( void ) const {
	Rotation rotation = ToRotation();
	return rotation.GetVec() * DEG2RAD( rotation.GetAngle() );
}

/*
============
Mat3::Determinant
============
*/
float Mat3::Determinant( void ) const {

	float det2_12_01 = mat[1][0] * mat[2][1] - mat[1][1] * mat[2][0];
	float det2_12_02 = mat[1][0] * mat[2][2] - mat[1][2] * mat[2][0];
	float det2_12_12 = mat[1][1] * mat[2][2] - mat[1][2] * mat[2][1];

	return mat[0][0] * det2_12_12 - mat[0][1] * det2_12_02 + mat[0][2] * det2_12_01;
}

/*
============
Mat3::InverseSelf
============
*/
bool Mat3::InverseSelf( void ) {
	// 18+3+9 = 30 multiplications
	//			 1 division
	Mat3 inverse;
	float det, invDet;

	inverse[0][0] = mat[1][1] * mat[2][2] - mat[1][2] * mat[2][1];
	inverse[1][0] = mat[1][2] * mat[2][0] - mat[1][0] * mat[2][2];
	inverse[2][0] = mat[1][0] * mat[2][1] - mat[1][1] * mat[2][0];

	det = mat[0][0] * inverse[0][0] + mat[0][1] * inverse[1][0] + mat[0][2] * inverse[2][0];

	if ( fabs( det ) < MATRIX_INVERSE_EPSILON ) {
		return false;
	}

	invDet = 1.0f / det;

	inverse[0][1] = mat[0][2] * mat[2][1] - mat[0][1] * mat[2][2];
	inverse[0][2] = mat[0][1] * mat[1][2] - mat[0][2] * mat[1][1];
	inverse[1][1] = mat[0][0] * mat[2][2] - mat[0][2] * mat[2][0];
	inverse[1][2] = mat[0][2] * mat[1][0] - mat[0][0] * mat[1][2];
	inverse[2][1] = mat[0][1] * mat[2][0] - mat[0][0] * mat[2][1];
	inverse[2][2] = mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];

	mat[0][0] = inverse[0][0] * invDet;
	mat[0][1] = inverse[0][1] * invDet;
	mat[0][2] = inverse[0][2] * invDet;

	mat[1][0] = inverse[1][0] * invDet;
	mat[1][1] = inverse[1][1] * invDet;
	mat[1][2] = inverse[1][2] * invDet;

	mat[2][0] = inverse[2][0] * invDet;
	mat[2][1] = inverse[2][1] * invDet;
	mat[2][2] = inverse[2][2] * invDet;

	return true;
}

/*
============
Mat3::InverseFastSelf
============
*/
bool Mat3::InverseFastSelf( void ) {
#if 1
	// 18+3+9 = 30 multiplications
	//			 1 division
	Mat3 inverse;
	float det, invDet;

	inverse[0][0] = mat[1][1] * mat[2][2] - mat[1][2] * mat[2][1];
	inverse[1][0] = mat[1][2] * mat[2][0] - mat[1][0] * mat[2][2];
	inverse[2][0] = mat[1][0] * mat[2][1] - mat[1][1] * mat[2][0];

	det = mat[0][0] * inverse[0][0] + mat[0][1] * inverse[1][0] + mat[0][2] * inverse[2][0];

	if ( fabs( det ) < MATRIX_INVERSE_EPSILON ) {
		return false;
	}

	invDet = 1.0f / det;

	inverse[0][1] = mat[0][2] * mat[2][1] - mat[0][1] * mat[2][2];
	inverse[0][2] = mat[0][1] * mat[1][2] - mat[0][2] * mat[1][1];
	inverse[1][1] = mat[0][0] * mat[2][2] - mat[0][2] * mat[2][0];
	inverse[1][2] = mat[0][2] * mat[1][0] - mat[0][0] * mat[1][2];
	inverse[2][1] = mat[0][1] * mat[2][0] - mat[0][0] * mat[2][1];
	inverse[2][2] = mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];

	mat[0][0] = inverse[0][0] * invDet;
	mat[0][1] = inverse[0][1] * invDet;
	mat[0][2] = inverse[0][2] * invDet;

	mat[1][0] = inverse[1][0] * invDet;
	mat[1][1] = inverse[1][1] * invDet;
	mat[1][2] = inverse[1][2] * invDet;

	mat[2][0] = inverse[2][0] * invDet;
	mat[2][1] = inverse[2][1] * invDet;
	mat[2][2] = inverse[2][2] * invDet;

	return true;
#elif 0
	// 3*10 = 30 multiplications
	//		   3 divisions
	float *mat = reinterpret_cast<float *>(this);
	float s;
	float d, di;

	di = mat[0];
	s = di;
	mat[0] = d = 1.0f / di;
	mat[1] *= d;
	mat[2] *= d;
	d = -d;
	mat[3] *= d;
	mat[6] *= d;
	d = mat[3] * di;
	mat[4] += mat[1] * d;
	mat[5] += mat[2] * d;
	d = mat[6] * di;
	mat[7] += mat[1] * d;
	mat[8] += mat[2] * d;
	di = mat[4];
	s *= di;
	mat[4] = d = 1.0f / di;
	mat[3] *= d;
	mat[5] *= d;
	d = -d;
	mat[1] *= d;
	mat[7] *= d;
	d = mat[1] * di;
	mat[0] += mat[3] * d;
	mat[2] += mat[5] * d;
	d = mat[7] * di;
	mat[6] += mat[3] * d;
	mat[8] += mat[5] * d;
	di = mat[8];
	s *= di;
	mat[8] = d = 1.0f / di;
	mat[6] *= d;
	mat[7] *= d;
	d = -d;
	mat[2] *= d;
	mat[5] *= d;
	d = mat[2] * di;
	mat[0] += mat[6] * d;
	mat[1] += mat[7] * d;
	d = mat[5] * di;
	mat[3] += mat[6] * d;
	mat[4] += mat[7] * d;

	return ( s != 0.0f && !FLOAT_IS_NAN( s ) );
#else
	//	4*2+4*4 = 24 multiplications
	//		2*1 =  2 divisions
	idMat2 r0;
	float r1[2], r2[2], r3;
	float det, invDet;
	float *mat = reinterpret_cast<float *>(this);

	// r0 = m0.Inverse();	// 2x2
	det = mat[0*3+0] * mat[1*3+1] - mat[0*3+1] * mat[1*3+0];

	if ( fabs( det ) < MATRIX_INVERSE_EPSILON ) {
		return false;
	}

	invDet = 1.0f / det;

	r0[0][0] =   mat[1*3+1] * invDet;
	r0[0][1] = - mat[0*3+1] * invDet;
	r0[1][0] = - mat[1*3+0] * invDet;
	r0[1][1] =   mat[0*3+0] * invDet;

	// r1 = r0 * m1;		// 2x1 = 2x2 * 2x1
	r1[0] = r0[0][0] * mat[0*3+2] + r0[0][1] * mat[1*3+2];
	r1[1] = r0[1][0] * mat[0*3+2] + r0[1][1] * mat[1*3+2];

	// r2 = m2 * r1;		// 1x1 = 1x2 * 2x1
	r2[0] = mat[2*3+0] * r1[0] + mat[2*3+1] * r1[1];

	// r3 = r2 - m3;		// 1x1 = 1x1 - 1x1
	r3 = r2[0] - mat[2*3+2];

	// r3.InverseSelf();
	if ( fabs( r3 ) < MATRIX_INVERSE_EPSILON ) {
		return false;
	}

	r3 = 1.0f / r3;

	// r2 = m2 * r0;		// 1x2 = 1x2 * 2x2
	r2[0] = mat[2*3+0] * r0[0][0] + mat[2*3+1] * r0[1][0];
	r2[1] = mat[2*3+0] * r0[0][1] + mat[2*3+1] * r0[1][1];

	// m2 = r3 * r2;		// 1x2 = 1x1 * 1x2
	mat[2*3+0] = r3 * r2[0];
	mat[2*3+1] = r3 * r2[1];

	// m0 = r0 - r1 * m2;	// 2x2 - 2x1 * 1x2
	mat[0*3+0] = r0[0][0] - r1[0] * mat[2*3+0];
	mat[0*3+1] = r0[0][1] - r1[0] * mat[2*3+1];
	mat[1*3+0] = r0[1][0] - r1[1] * mat[2*3+0];
	mat[1*3+1] = r0[1][1] - r1[1] * mat[2*3+1];

	// m1 = r1 * r3;		// 2x1 = 2x1 * 1x1
	mat[0*3+2] = r1[0] * r3;
	mat[1*3+2] = r1[1] * r3;

	// m3 = -r3;
	mat[2*3+2] = -r3;

	return true;
#endif
}

/*
============
Mat3::InertiaTranslate
============
*/
Mat3 Mat3::InertiaTranslate( const float mass, const Vec3 &centerOfMass, const Vec3 &translation ) const {
	Mat3 m;
	Vec3 newCenter;

	newCenter = centerOfMass + translation;

	m[0][0] = mass * ( ( centerOfMass[1] * centerOfMass[1] + centerOfMass[2] * centerOfMass[2] )
				- ( newCenter[1] * newCenter[1] + newCenter[2] * newCenter[2] ) );
	m[1][1] = mass * ( ( centerOfMass[0] * centerOfMass[0] + centerOfMass[2] * centerOfMass[2] )
				- ( newCenter[0] * newCenter[0] + newCenter[2] * newCenter[2] ) );
	m[2][2] = mass * ( ( centerOfMass[0] * centerOfMass[0] + centerOfMass[1] * centerOfMass[1] )
				- ( newCenter[0] * newCenter[0] + newCenter[1] * newCenter[1] ) );

	m[0][1] = m[1][0] = mass * ( newCenter[0] * newCenter[1] - centerOfMass[0] * centerOfMass[1] );
	m[1][2] = m[2][1] = mass * ( newCenter[1] * newCenter[2] - centerOfMass[1] * centerOfMass[2] );
	m[0][2] = m[2][0] = mass * ( newCenter[0] * newCenter[2] - centerOfMass[0] * centerOfMass[2] );

	return (*this) + m;
}

/*
============
Mat3::InertiaTranslateSelf
============
*/
Mat3 &Mat3::InertiaTranslateSelf( const float mass, const Vec3 &centerOfMass, const Vec3 &translation ) {
	Mat3 m;
	Vec3 newCenter;

	newCenter = centerOfMass + translation;

	m[0][0] = mass * ( ( centerOfMass[1] * centerOfMass[1] + centerOfMass[2] * centerOfMass[2] )
				- ( newCenter[1] * newCenter[1] + newCenter[2] * newCenter[2] ) );
	m[1][1] = mass * ( ( centerOfMass[0] * centerOfMass[0] + centerOfMass[2] * centerOfMass[2] )
				- ( newCenter[0] * newCenter[0] + newCenter[2] * newCenter[2] ) );
	m[2][2] = mass * ( ( centerOfMass[0] * centerOfMass[0] + centerOfMass[1] * centerOfMass[1] )
				- ( newCenter[0] * newCenter[0] + newCenter[1] * newCenter[1] ) );

	m[0][1] = m[1][0] = mass * ( newCenter[0] * newCenter[1] - centerOfMass[0] * centerOfMass[1] );
	m[1][2] = m[2][1] = mass * ( newCenter[1] * newCenter[2] - centerOfMass[1] * centerOfMass[2] );
	m[0][2] = m[2][0] = mass * ( newCenter[0] * newCenter[2] - centerOfMass[0] * centerOfMass[2] );

	(*this) += m;

	return (*this);
}

/*
============
Mat3::InertiaRotate
============
*/
Mat3 Mat3::InertiaRotate( const Mat3 &rotation ) const {
	// NOTE: the rotation matrix is stored column-major
	return rotation.Transpose() * (*this) * rotation;
}

/*
============
Mat3::InertiaRotateSelf
============
*/
Mat3 &Mat3::InertiaRotateSelf( const Mat3 &rotation ) {
	// NOTE: the rotation matrix is stored column-major
	*this = rotation.Transpose() * (*this) * rotation;
	return *this;
}

/*
=============
Mat3::ToString
=============
*/
const char *Mat3::ToString( int precision ) const {
	return Str::FloatArrayToString( ToFloatPtr(), GetDimension(), precision );
}


//===============================================================
//
//	Mat4
//
//===============================================================

const Mat4 mat4_zero( Vec4( 0, 0, 0, 0 ), Vec4( 0, 0, 0, 0 ), Vec4( 0, 0, 0, 0 ), Vec4( 0, 0, 0, 0 ) );
const Mat4 mat4_identity( Vec4( 1, 0, 0, 0 ), Vec4( 0, 1, 0, 0 ), Vec4( 0, 0, 1, 0 ), Vec4( 0, 0, 0, 1 ) );
const Mat4 mat4_bias( 0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f );

/*
============
Mat4::Transpose
============
*/
Mat4 Mat4::Transpose( void ) const {
	Mat4	transpose;
	int		i, j;
   
	for( i = 0; i < 4; i++ ) {
		for( j = 0; j < 4; j++ ) {
			transpose[ i ][ j ] = mat[ j ][ i ];
        }
	}
	return transpose;
}

/*
============
Mat4::TransposeSelf
============
*/
Mat4 &Mat4::TransposeSelf( void ) {
	float	temp;
	int		i, j;
   
	for( i = 0; i < 4; i++ ) {
		for( j = i + 1; j < 4; j++ ) {
			temp = mat[ i ][ j ];
			mat[ i ][ j ] = mat[ j ][ i ];
			mat[ j ][ i ] = temp;
        }
	}
	return *this;
}

/*
============
Mat4::Determinant
============
*/
float Mat4::Determinant( void ) const {

	// 2x2 sub-determinants
	float det2_01_01 = mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];
	float det2_01_02 = mat[0][0] * mat[1][2] - mat[0][2] * mat[1][0];
	float det2_01_03 = mat[0][0] * mat[1][3] - mat[0][3] * mat[1][0];
	float det2_01_12 = mat[0][1] * mat[1][2] - mat[0][2] * mat[1][1];
	float det2_01_13 = mat[0][1] * mat[1][3] - mat[0][3] * mat[1][1];
	float det2_01_23 = mat[0][2] * mat[1][3] - mat[0][3] * mat[1][2];

	// 3x3 sub-determinants
	float det3_201_012 = mat[2][0] * det2_01_12 - mat[2][1] * det2_01_02 + mat[2][2] * det2_01_01;
	float det3_201_013 = mat[2][0] * det2_01_13 - mat[2][1] * det2_01_03 + mat[2][3] * det2_01_01;
	float det3_201_023 = mat[2][0] * det2_01_23 - mat[2][2] * det2_01_03 + mat[2][3] * det2_01_02;
	float det3_201_123 = mat[2][1] * det2_01_23 - mat[2][2] * det2_01_13 + mat[2][3] * det2_01_12;

	return ( - det3_201_123 * mat[3][0] + det3_201_023 * mat[3][1] - det3_201_013 * mat[3][2] + det3_201_012 * mat[3][3] );
}

/*
============
Mat4::InverseSelf
============
*/
bool Mat4::InverseSelf( void ) {
	// 84+4+16 = 104 multiplications
	//			   1 division
	float det, invDet;

	// 2x2 sub-determinants required to calculate 4x4 determinant
	float det2_01_01 = mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];
	float det2_01_02 = mat[0][0] * mat[1][2] - mat[0][2] * mat[1][0];
	float det2_01_03 = mat[0][0] * mat[1][3] - mat[0][3] * mat[1][0];
	float det2_01_12 = mat[0][1] * mat[1][2] - mat[0][2] * mat[1][1];
	float det2_01_13 = mat[0][1] * mat[1][3] - mat[0][3] * mat[1][1];
	float det2_01_23 = mat[0][2] * mat[1][3] - mat[0][3] * mat[1][2];

	// 3x3 sub-determinants required to calculate 4x4 determinant
	float det3_201_012 = mat[2][0] * det2_01_12 - mat[2][1] * det2_01_02 + mat[2][2] * det2_01_01;
	float det3_201_013 = mat[2][0] * det2_01_13 - mat[2][1] * det2_01_03 + mat[2][3] * det2_01_01;
	float det3_201_023 = mat[2][0] * det2_01_23 - mat[2][2] * det2_01_03 + mat[2][3] * det2_01_02;
	float det3_201_123 = mat[2][1] * det2_01_23 - mat[2][2] * det2_01_13 + mat[2][3] * det2_01_12;

	det = ( - det3_201_123 * mat[3][0] + det3_201_023 * mat[3][1] - det3_201_013 * mat[3][2] + det3_201_012 * mat[3][3] );

	if ( fabs( det ) < MATRIX_INVERSE_EPSILON ) {
		return false;
	}

	invDet = 1.0f / det;

	// remaining 2x2 sub-determinants
	float det2_03_01 = mat[0][0] * mat[3][1] - mat[0][1] * mat[3][0];
	float det2_03_02 = mat[0][0] * mat[3][2] - mat[0][2] * mat[3][0];
	float det2_03_03 = mat[0][0] * mat[3][3] - mat[0][3] * mat[3][0];
	float det2_03_12 = mat[0][1] * mat[3][2] - mat[0][2] * mat[3][1];
	float det2_03_13 = mat[0][1] * mat[3][3] - mat[0][3] * mat[3][1];
	float det2_03_23 = mat[0][2] * mat[3][3] - mat[0][3] * mat[3][2];

	float det2_13_01 = mat[1][0] * mat[3][1] - mat[1][1] * mat[3][0];
	float det2_13_02 = mat[1][0] * mat[3][2] - mat[1][2] * mat[3][0];
	float det2_13_03 = mat[1][0] * mat[3][3] - mat[1][3] * mat[3][0];
	float det2_13_12 = mat[1][1] * mat[3][2] - mat[1][2] * mat[3][1];
	float det2_13_13 = mat[1][1] * mat[3][3] - mat[1][3] * mat[3][1];
	float det2_13_23 = mat[1][2] * mat[3][3] - mat[1][3] * mat[3][2];

	// remaining 3x3 sub-determinants
	float det3_203_012 = mat[2][0] * det2_03_12 - mat[2][1] * det2_03_02 + mat[2][2] * det2_03_01;
	float det3_203_013 = mat[2][0] * det2_03_13 - mat[2][1] * det2_03_03 + mat[2][3] * det2_03_01;
	float det3_203_023 = mat[2][0] * det2_03_23 - mat[2][2] * det2_03_03 + mat[2][3] * det2_03_02;
	float det3_203_123 = mat[2][1] * det2_03_23 - mat[2][2] * det2_03_13 + mat[2][3] * det2_03_12;

	float det3_213_012 = mat[2][0] * det2_13_12 - mat[2][1] * det2_13_02 + mat[2][2] * det2_13_01;
	float det3_213_013 = mat[2][0] * det2_13_13 - mat[2][1] * det2_13_03 + mat[2][3] * det2_13_01;
	float det3_213_023 = mat[2][0] * det2_13_23 - mat[2][2] * det2_13_03 + mat[2][3] * det2_13_02;
	float det3_213_123 = mat[2][1] * det2_13_23 - mat[2][2] * det2_13_13 + mat[2][3] * det2_13_12;

	float det3_301_012 = mat[3][0] * det2_01_12 - mat[3][1] * det2_01_02 + mat[3][2] * det2_01_01;
	float det3_301_013 = mat[3][0] * det2_01_13 - mat[3][1] * det2_01_03 + mat[3][3] * det2_01_01;
	float det3_301_023 = mat[3][0] * det2_01_23 - mat[3][2] * det2_01_03 + mat[3][3] * det2_01_02;
	float det3_301_123 = mat[3][1] * det2_01_23 - mat[3][2] * det2_01_13 + mat[3][3] * det2_01_12;

	mat[0][0] =	- det3_213_123 * invDet;
	mat[1][0] = + det3_213_023 * invDet;
	mat[2][0] = - det3_213_013 * invDet;
	mat[3][0] = + det3_213_012 * invDet;

	mat[0][1] = + det3_203_123 * invDet;
	mat[1][1] = - det3_203_023 * invDet;
	mat[2][1] = + det3_203_013 * invDet;
	mat[3][1] = - det3_203_012 * invDet;

	mat[0][2] = + det3_301_123 * invDet;
	mat[1][2] = - det3_301_023 * invDet;
	mat[2][2] = + det3_301_013 * invDet;
	mat[3][2] = - det3_301_012 * invDet;

	mat[0][3] = - det3_201_123 * invDet;
	mat[1][3] = + det3_201_023 * invDet;
	mat[2][3] = - det3_201_013 * invDet;
	mat[3][3] = + det3_201_012 * invDet;

	return true;
}

/*
============
Mat4::InverseFastSelf
============
*/
bool Mat4::InverseFastSelf( void ) {
#if 0
	// 84+4+16 = 104 multiplications
	//			   1 division
	float det, invDet;

	// 2x2 sub-determinants required to calculate 4x4 determinant
	float det2_01_01 = mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];
	float det2_01_02 = mat[0][0] * mat[1][2] - mat[0][2] * mat[1][0];
	float det2_01_03 = mat[0][0] * mat[1][3] - mat[0][3] * mat[1][0];
	float det2_01_12 = mat[0][1] * mat[1][2] - mat[0][2] * mat[1][1];
	float det2_01_13 = mat[0][1] * mat[1][3] - mat[0][3] * mat[1][1];
	float det2_01_23 = mat[0][2] * mat[1][3] - mat[0][3] * mat[1][2];

	// 3x3 sub-determinants required to calculate 4x4 determinant
	float det3_201_012 = mat[2][0] * det2_01_12 - mat[2][1] * det2_01_02 + mat[2][2] * det2_01_01;
	float det3_201_013 = mat[2][0] * det2_01_13 - mat[2][1] * det2_01_03 + mat[2][3] * det2_01_01;
	float det3_201_023 = mat[2][0] * det2_01_23 - mat[2][2] * det2_01_03 + mat[2][3] * det2_01_02;
	float det3_201_123 = mat[2][1] * det2_01_23 - mat[2][2] * det2_01_13 + mat[2][3] * det2_01_12;

	det = ( - det3_201_123 * mat[3][0] + det3_201_023 * mat[3][1] - det3_201_013 * mat[3][2] + det3_201_012 * mat[3][3] );

	if ( fabs( det ) < MATRIX_INVERSE_EPSILON ) {
		return false;
	}

	invDet = 1.0f / det;

	// remaining 2x2 sub-determinants
	float det2_03_01 = mat[0][0] * mat[3][1] - mat[0][1] * mat[3][0];
	float det2_03_02 = mat[0][0] * mat[3][2] - mat[0][2] * mat[3][0];
	float det2_03_03 = mat[0][0] * mat[3][3] - mat[0][3] * mat[3][0];
	float det2_03_12 = mat[0][1] * mat[3][2] - mat[0][2] * mat[3][1];
	float det2_03_13 = mat[0][1] * mat[3][3] - mat[0][3] * mat[3][1];
	float det2_03_23 = mat[0][2] * mat[3][3] - mat[0][3] * mat[3][2];

	float det2_13_01 = mat[1][0] * mat[3][1] - mat[1][1] * mat[3][0];
	float det2_13_02 = mat[1][0] * mat[3][2] - mat[1][2] * mat[3][0];
	float det2_13_03 = mat[1][0] * mat[3][3] - mat[1][3] * mat[3][0];
	float det2_13_12 = mat[1][1] * mat[3][2] - mat[1][2] * mat[3][1];
	float det2_13_13 = mat[1][1] * mat[3][3] - mat[1][3] * mat[3][1];
	float det2_13_23 = mat[1][2] * mat[3][3] - mat[1][3] * mat[3][2];

	// remaining 3x3 sub-determinants
	float det3_203_012 = mat[2][0] * det2_03_12 - mat[2][1] * det2_03_02 + mat[2][2] * det2_03_01;
	float det3_203_013 = mat[2][0] * det2_03_13 - mat[2][1] * det2_03_03 + mat[2][3] * det2_03_01;
	float det3_203_023 = mat[2][0] * det2_03_23 - mat[2][2] * det2_03_03 + mat[2][3] * det2_03_02;
	float det3_203_123 = mat[2][1] * det2_03_23 - mat[2][2] * det2_03_13 + mat[2][3] * det2_03_12;

	float det3_213_012 = mat[2][0] * det2_13_12 - mat[2][1] * det2_13_02 + mat[2][2] * det2_13_01;
	float det3_213_013 = mat[2][0] * det2_13_13 - mat[2][1] * det2_13_03 + mat[2][3] * det2_13_01;
	float det3_213_023 = mat[2][0] * det2_13_23 - mat[2][2] * det2_13_03 + mat[2][3] * det2_13_02;
	float det3_213_123 = mat[2][1] * det2_13_23 - mat[2][2] * det2_13_13 + mat[2][3] * det2_13_12;

	float det3_301_012 = mat[3][0] * det2_01_12 - mat[3][1] * det2_01_02 + mat[3][2] * det2_01_01;
	float det3_301_013 = mat[3][0] * det2_01_13 - mat[3][1] * det2_01_03 + mat[3][3] * det2_01_01;
	float det3_301_023 = mat[3][0] * det2_01_23 - mat[3][2] * det2_01_03 + mat[3][3] * det2_01_02;
	float det3_301_123 = mat[3][1] * det2_01_23 - mat[3][2] * det2_01_13 + mat[3][3] * det2_01_12;

	mat[0][0] =	- det3_213_123 * invDet;
	mat[1][0] = + det3_213_023 * invDet;
	mat[2][0] = - det3_213_013 * invDet;
	mat[3][0] = + det3_213_012 * invDet;

	mat[0][1] = + det3_203_123 * invDet;
	mat[1][1] = - det3_203_023 * invDet;
	mat[2][1] = + det3_203_013 * invDet;
	mat[3][1] = - det3_203_012 * invDet;

	mat[0][2] = + det3_301_123 * invDet;
	mat[1][2] = - det3_301_023 * invDet;
	mat[2][2] = + det3_301_013 * invDet;
	mat[3][2] = - det3_301_012 * invDet;

	mat[0][3] = - det3_201_123 * invDet;
	mat[1][3] = + det3_201_023 * invDet;
	mat[2][3] = - det3_201_013 * invDet;
	mat[3][3] = + det3_201_012 * invDet;

	return true;
#elif 0
	// 4*18 = 72 multiplications
	//		   4 divisions
	float *mat = reinterpret_cast<float *>(this);
	float s;
	float d, di;

	di = mat[0];
	s = di;
	mat[0] = d = 1.0f / di;
	mat[1] *= d;
	mat[2] *= d;
	mat[3] *= d;
	d = -d;
	mat[4] *= d;
	mat[8] *= d;
	mat[12] *= d;
	d = mat[4] * di;
	mat[5] += mat[1] * d;
	mat[6] += mat[2] * d;
	mat[7] += mat[3] * d;
	d = mat[8] * di;
	mat[9] += mat[1] * d;
	mat[10] += mat[2] * d;
	mat[11] += mat[3] * d;
	d = mat[12] * di;
	mat[13] += mat[1] * d;
	mat[14] += mat[2] * d;
	mat[15] += mat[3] * d;
	di = mat[5];
	s *= di;
	mat[5] = d = 1.0f / di;
	mat[4] *= d;
	mat[6] *= d;
	mat[7] *= d;
	d = -d;
	mat[1] *= d;
	mat[9] *= d;
	mat[13] *= d;
	d = mat[1] * di;
	mat[0] += mat[4] * d;
	mat[2] += mat[6] * d;
	mat[3] += mat[7] * d;
	d = mat[9] * di;
	mat[8] += mat[4] * d;
	mat[10] += mat[6] * d;
	mat[11] += mat[7] * d;
	d = mat[13] * di;
	mat[12] += mat[4] * d;
	mat[14] += mat[6] * d;
	mat[15] += mat[7] * d;
	di = mat[10];
	s *= di;
	mat[10] = d = 1.0f / di;
	mat[8] *= d;
	mat[9] *= d;
	mat[11] *= d;
	d = -d;
	mat[2] *= d;
	mat[6] *= d;
	mat[14] *= d;
	d = mat[2] * di;
	mat[0] += mat[8] * d;
	mat[1] += mat[9] * d;
	mat[3] += mat[11] * d;
	d = mat[6] * di;
	mat[4] += mat[8] * d;
	mat[5] += mat[9] * d;
	mat[7] += mat[11] * d;
	d = mat[14] * di;
	mat[12] += mat[8] * d;
	mat[13] += mat[9] * d;
	mat[15] += mat[11] * d;
	di = mat[15];
	s *= di;
	mat[15] = d = 1.0f / di;
	mat[12] *= d;
	mat[13] *= d;
	mat[14] *= d;
	d = -d;
	mat[3] *= d;
	mat[7] *= d;
	mat[11] *= d;
	d = mat[3] * di;
	mat[0] += mat[12] * d;
	mat[1] += mat[13] * d;
	mat[2] += mat[14] * d;
	d = mat[7] * di;
	mat[4] += mat[12] * d;
	mat[5] += mat[13] * d;
	mat[6] += mat[14] * d;
	d = mat[11] * di;
	mat[8] += mat[12] * d;
	mat[9] += mat[13] * d;
	mat[10] += mat[14] * d;

	return ( s != 0.0f && !FLOAT_IS_NAN( s ) );
#else
	//	6*8+2*6 = 60 multiplications
	//		2*1 =  2 divisions
	Mat2 r0, r1, r2, r3;
	float a, det, invDet;
	float *mat = reinterpret_cast<float *>(this);

	// r0 = m0.Inverse();
	det = mat[0*4+0] * mat[1*4+1] - mat[0*4+1] * mat[1*4+0];

	if ( fabs( det ) < MATRIX_INVERSE_EPSILON ) {
		return false;
	}

	invDet = 1.0f / det;

	r0[0][0] =   mat[1*4+1] * invDet;
	r0[0][1] = - mat[0*4+1] * invDet;
	r0[1][0] = - mat[1*4+0] * invDet;
	r0[1][1] =   mat[0*4+0] * invDet;

	// r1 = r0 * m1;
	r1[0][0] = r0[0][0] * mat[0*4+2] + r0[0][1] * mat[1*4+2];
	r1[0][1] = r0[0][0] * mat[0*4+3] + r0[0][1] * mat[1*4+3];
	r1[1][0] = r0[1][0] * mat[0*4+2] + r0[1][1] * mat[1*4+2];
	r1[1][1] = r0[1][0] * mat[0*4+3] + r0[1][1] * mat[1*4+3];

	// r2 = m2 * r1;
	r2[0][0] = mat[2*4+0] * r1[0][0] + mat[2*4+1] * r1[1][0];
	r2[0][1] = mat[2*4+0] * r1[0][1] + mat[2*4+1] * r1[1][1];
	r2[1][0] = mat[3*4+0] * r1[0][0] + mat[3*4+1] * r1[1][0];
	r2[1][1] = mat[3*4+0] * r1[0][1] + mat[3*4+1] * r1[1][1];

	// r3 = r2 - m3;
	r3[0][0] = r2[0][0] - mat[2*4+2];
	r3[0][1] = r2[0][1] - mat[2*4+3];
	r3[1][0] = r2[1][0] - mat[3*4+2];
	r3[1][1] = r2[1][1] - mat[3*4+3];

	// r3.InverseSelf();
	det = r3[0][0] * r3[1][1] - r3[0][1] * r3[1][0];

	if ( fabs( det ) < MATRIX_INVERSE_EPSILON ) {
		return false;
	}

	invDet = 1.0f / det;

	a = r3[0][0];
	r3[0][0] =   r3[1][1] * invDet;
	r3[0][1] = - r3[0][1] * invDet;
	r3[1][0] = - r3[1][0] * invDet;
	r3[1][1] =   a * invDet;

	// r2 = m2 * r0;
	r2[0][0] = mat[2*4+0] * r0[0][0] + mat[2*4+1] * r0[1][0];
	r2[0][1] = mat[2*4+0] * r0[0][1] + mat[2*4+1] * r0[1][1];
	r2[1][0] = mat[3*4+0] * r0[0][0] + mat[3*4+1] * r0[1][0];
	r2[1][1] = mat[3*4+0] * r0[0][1] + mat[3*4+1] * r0[1][1];

	// m2 = r3 * r2;
	mat[2*4+0] = r3[0][0] * r2[0][0] + r3[0][1] * r2[1][0];
	mat[2*4+1] = r3[0][0] * r2[0][1] + r3[0][1] * r2[1][1];
	mat[3*4+0] = r3[1][0] * r2[0][0] + r3[1][1] * r2[1][0];
	mat[3*4+1] = r3[1][0] * r2[0][1] + r3[1][1] * r2[1][1];

	// m0 = r0 - r1 * m2;
	mat[0*4+0] = r0[0][0] - r1[0][0] * mat[2*4+0] - r1[0][1] * mat[3*4+0];
	mat[0*4+1] = r0[0][1] - r1[0][0] * mat[2*4+1] - r1[0][1] * mat[3*4+1];
	mat[1*4+0] = r0[1][0] - r1[1][0] * mat[2*4+0] - r1[1][1] * mat[3*4+0];
	mat[1*4+1] = r0[1][1] - r1[1][0] * mat[2*4+1] - r1[1][1] * mat[3*4+1];

	// m1 = r1 * r3;
	mat[0*4+2] = r1[0][0] * r3[0][0] + r1[0][1] * r3[1][0];
	mat[0*4+3] = r1[0][0] * r3[0][1] + r1[0][1] * r3[1][1];
	mat[1*4+2] = r1[1][0] * r3[0][0] + r1[1][1] * r3[1][0];
	mat[1*4+3] = r1[1][0] * r3[0][1] + r1[1][1] * r3[1][1];

	// m3 = -r3;
	mat[2*4+2] = -r3[0][0];
	mat[2*4+3] = -r3[0][1];
	mat[3*4+2] = -r3[1][0];
	mat[3*4+3] = -r3[1][1];

	return true;
#endif
}

/*
=============
Mat4::ToString
=============
*/
const char *Mat4::ToString( int precision ) const {
	return Str::FloatArrayToString( ToFloatPtr(), GetDimension(), precision );
}

//===============================================================
//
//  MatX
//
//===============================================================

float	MatX::temp[MATX_MAX_TEMP+4];
float *	MatX::tempPtr = (float *) ( ( (int) MatX::temp + 15 ) & ~15 );
int		MatX::tempIndex = 0;


/*
============
MatX::ChangeSize
============
*/
void MatX::ChangeSize( int rows, int columns, bool makeZero ) {
	int alloc = ( rows * columns + 3 ) & ~3;
	if ( alloc > alloced && alloced != -1 ) {
		float *oldMat = mat;
		mat = (float *) malloc( alloc * sizeof( float ) );
		if ( makeZero ) {
			Common::Com_Memset( mat, 0, alloc * sizeof( float ) );
		}
		alloced = alloc;
		if ( oldMat ) {
			int minRow = Min( numRows, rows );
			int minColumn = Min( numColumns, columns );
			for ( int i = 0; i < minRow; i++ ) {
				for ( int j = 0; j < minColumn; j++ ) {
					mat[ i * columns + j ] = oldMat[ i * numColumns + j ];
				}
			}
			free( oldMat );
		}
	} else {
		if ( columns < numColumns ) {
			int minRow = Min( numRows, rows );
			for ( int i = 0; i < minRow; i++ ) {
				for ( int j = 0; j < columns; j++ ) {
					mat[ i * columns + j ] = mat[ i * numColumns + j ];
				}
			}
		} else if ( columns > numColumns ) {
			for ( int i = Min( numRows, rows ) - 1; i >= 0; i-- ) {
				if ( makeZero ) {
					for ( int j = columns - 1; j >= numColumns; j-- ) {
						mat[ i * columns + j ] = 0.0f;
					}
				}
				for ( int j = numColumns - 1; j >= 0; j-- ) {
					mat[ i * columns + j ] = mat[ i * numColumns + j ];
				}
			}
		}
		if ( makeZero && rows > numRows ) {
			Common::Com_Memset( mat + numRows * columns, 0, ( rows - numRows ) * columns * sizeof( float ) );
		}
	}
	numRows = rows;
	numColumns = columns;
	MATX_CLEAREND();
}

/*
============
MatX::RemoveRow
============
*/
MatX &MatX::RemoveRow( int r ) {
	int i;

	assert( r < numRows );

	numRows--;

	for ( i = r; i < numRows; i++ ) {
		memcpy( &mat[i * numColumns], &mat[( i + 1 ) * numColumns], numColumns * sizeof( float ) );
	}

	return *this;
}

/*
============
MatX::RemoveColumn
============
*/
MatX &MatX::RemoveColumn( int r ) {
	int i;

	assert( r < numColumns );

	numColumns--;

	for ( i = 0; i < numRows - 1; i++ ) {
		memmove( &mat[i * numColumns + r], &mat[i * ( numColumns + 1 ) + r + 1], numColumns * sizeof( float ) );
	}
	memmove( &mat[i * numColumns + r], &mat[i * ( numColumns + 1 ) + r + 1], ( numColumns - r ) * sizeof( float ) );

	return *this;
}

/*
============
MatX::RemoveRowColumn
============
*/
MatX &MatX::RemoveRowColumn( int r ) {
	int i;

	assert( r < numRows && r < numColumns );

	numRows--;
	numColumns--;

	if ( r > 0 ) {
		for ( i = 0; i < r - 1; i++ ) {
			memmove( &mat[i * numColumns + r], &mat[i * ( numColumns + 1 ) + r + 1], numColumns * sizeof( float ) );
		}
		memmove( &mat[i * numColumns + r], &mat[i * ( numColumns + 1 ) + r + 1], ( numColumns - r ) * sizeof( float ) );
	}

	memcpy( &mat[r * numColumns], &mat[( r + 1 ) * ( numColumns + 1 )], r * sizeof( float ) );

	for ( i = r; i < numRows - 1; i++ ) {
		memcpy( &mat[i * numColumns + r], &mat[( i + 1 ) * ( numColumns + 1 ) + r + 1], numColumns * sizeof( float ) );
	}
	memcpy( &mat[i * numColumns + r], &mat[( i + 1 ) * ( numColumns + 1 ) + r + 1], ( numColumns - r ) * sizeof( float ) );

	return *this;
}

/*
============
MatX::IsOrthogonal

  returns true if (*this) * this->Transpose() == Identity
============
*/
bool MatX::IsOrthogonal( const float epsilon ) const {
	float *ptr1, *ptr2, sum;

	if ( !IsSquare() ) {
		return false;
	}

	ptr1 = mat;
	for ( int i = 0; i < numRows; i++ ) {
		for ( int j = 0; j < numColumns; j++ ) {
			ptr2 = mat + j;
			sum = ptr1[0] * ptr2[0] - (float) ( i == j );
			for ( int n = 1; n < numColumns; n++ ) {
				ptr2 += numColumns;
				sum += ptr1[n] * ptr2[0];
			}
			if ( fabs( sum ) > epsilon ) {
				return false;
			}
		}
		ptr1 += numColumns;
	}
	return true;
}

/*
============
MatX::IsOrthonormal

  returns true if (*this) * this->Transpose() == Identity and the length of each column vector is 1
============
*/
bool MatX::IsOrthonormal( const float epsilon ) const {
	float *ptr1, *ptr2, sum;

	if ( !IsSquare() ) {
		return false;
	}

	ptr1 = mat;
	for ( int i = 0; i < numRows; i++ ) {
		for ( int j = 0; j < numColumns; j++ ) {
			ptr2 = mat + j;
			sum = ptr1[0] * ptr2[0] - (float) ( i == j );
			for ( int n = 1; n < numColumns; n++ ) {
				ptr2 += numColumns;
				sum += ptr1[n] * ptr2[0];
			}
			if ( fabs( sum ) > epsilon ) {
				return false;
			}
		}
		ptr1 += numColumns;

		ptr2 = mat + i;
		sum = ptr2[0] * ptr2[0] - 1.0f;
		for ( i = 1; i < numRows; i++ ) {
			ptr2 += numColumns;
			sum += ptr2[i] * ptr2[i];
		}
		if ( fabs( sum ) > epsilon ) {
			return false;
		}
	}
	return true;
}

/*
============
MatX::IsPMatrix

  returns true if the matrix is a P-matrix
  A square matrix is a P-matrix if all its principal minors are positive.
============
*/
bool MatX::IsPMatrix( const float epsilon ) const {
	int i, j;
	float d;
	MatX m;

	if ( !IsSquare() ) {
		return false;
	}

	if ( numRows <= 0 ) {
		return true;
	}

	if ( (*this)[0][0] <= epsilon ) {
		return false;
	}

	if ( numRows <= 1 ) {
		return true;
	}

	m.SetData( numRows - 1, numColumns - 1, MATX_ALLOCA( ( numRows - 1 ) * ( numColumns - 1 ) ) );

	for ( i = 1; i < numRows; i++ ) {
		for ( j = 1; j < numColumns; j++ ) {
			m[i-1][j-1] = (*this)[i][j];
		}
	}

	if ( !m.IsPMatrix( epsilon ) ) {
		return false;
	}

	for ( i = 1; i < numRows; i++ ) {
		d = (*this)[i][0] / (*this)[0][0];
		for ( j = 1; j < numColumns; j++ ) {
			m[i-1][j-1] = (*this)[i][j] - d * (*this)[0][j];
		}
	}

	if ( !m.IsPMatrix( epsilon ) ) {
		return false;
	}

	return true;
}

/*
============
MatX::IsZMatrix

  returns true if the matrix is a Z-matrix
  A square matrix M is a Z-matrix if M[i][j] <= 0 for all i != j.
============
*/
bool MatX::IsZMatrix( const float epsilon ) const {
	int i, j;

	if ( !IsSquare() ) {
		return false;
	}

	for ( i = 0; i < numRows; i++ ) {
		for ( j = 0; j < numColumns; j++ ) {
			if ( (*this)[i][j] > epsilon && i != j ) {
				return false;
			}
		}
	}
	return true;
}

/*
============
MatX::IsPositiveDefinite

  returns true if the matrix is Positive Definite (PD)
  A square matrix M of order n is said to be PD if y'My > 0 for all vectors y of dimension n, y != 0.
============
*/
bool MatX::IsPositiveDefinite( const float epsilon ) const {
	int i, j, k;
	float d, s;
	MatX m;

	// the matrix must be square
	if ( !IsSquare() ) {
		return false;
	}

	// copy matrix
	m.SetData( numRows, numColumns, MATX_ALLOCA( numRows * numColumns ) );
	m = *this;

	// add transpose
	for ( i = 0; i < numRows; i++ ) {
		for ( j = 0; j < numColumns; j++ ) {
			m[i][j] += (*this)[j][i];
		}
	}

	// test Positive Definiteness with Gaussian pivot steps
	for ( i = 0; i < numRows; i++ ) {

		for ( j = i; j < numColumns; j++ ) {
			if ( m[j][j] <= epsilon ) {
				return false;
			}
		}

		d = 1.0f / m[i][i];
		for ( j = i + 1; j < numColumns; j++ ) {
			s = d * m[j][i];
			m[j][i] = 0.0f;
			for ( k = i + 1; k < numRows; k++ ) {
				m[j][k] -= s * m[i][k];
			}
		}
	}

	return true;
}

/*
============
MatX::IsSymmetricPositiveDefinite

  returns true if the matrix is Symmetric Positive Definite (PD)
============
*/
bool MatX::IsSymmetricPositiveDefinite( const float epsilon ) const {
	MatX m;

	// the matrix must be symmetric
	if ( !IsSymmetric( epsilon ) ) {
		return false;
	}

	// copy matrix
	m.SetData( numRows, numColumns, MATX_ALLOCA( numRows * numColumns ) );
	m = *this;

	// being able to obtain Cholesky factors is both a necessary and sufficient condition for positive definiteness
	return m.Cholesky_Factor();
}

/*
============
MatX::IsPositiveSemiDefinite

  returns true if the matrix is Positive Semi Definite (PSD)
  A square matrix M of order n is said to be PSD if y'My >= 0 for all vectors y of dimension n, y != 0.
============
*/
bool MatX::IsPositiveSemiDefinite( const float epsilon ) const {
	int i, j, k;
	float d, s;
	MatX m;

	// the matrix must be square
	if ( !IsSquare() ) {
		return false;
	}

	// copy original matrix
	m.SetData( numRows, numColumns, MATX_ALLOCA( numRows * numColumns ) );
	m = *this;

	// add transpose
	for ( i = 0; i < numRows; i++ ) {
		for ( j = 0; j < numColumns; j++ ) {
			m[i][j] += (*this)[j][i];
		}
	}

	// test Positive Semi Definiteness with Gaussian pivot steps
	for ( i = 0; i < numRows; i++ ) {

		for ( j = i; j < numColumns; j++ ) {
			if ( m[j][j] < -epsilon ) {
				return false;
			}
			if ( m[j][j] > epsilon ) {
				continue;
			}
			for ( k = 0; k < numRows; k++ ) {
				if ( fabs( m[k][j] ) > epsilon ) {
					return false;
				}
				if ( fabs( m[j][k] ) > epsilon ) {
					return false;
				}
			}
		}

		if ( m[i][i] <= epsilon ) {
			continue;
		}

		d = 1.0f / m[i][i];
		for ( j = i + 1; j < numColumns; j++ ) {
			s = d * m[j][i];
			m[j][i] = 0.0f;
			for ( k = i + 1; k < numRows; k++ ) {
				m[j][k] -= s * m[i][k];
			}
		}
	}

	return true;
}

/*
============
MatX::IsSymmetricPositiveSemiDefinite

  returns true if the matrix is Symmetric Positive Semi Definite (PSD)
============
*/
bool MatX::IsSymmetricPositiveSemiDefinite( const float epsilon ) const {

	// the matrix must be symmetric
	if ( !IsSymmetric( epsilon ) ) {
		return false;
	}

	return IsPositiveSemiDefinite( epsilon );
}

/*
============
MatX::LowerTriangularInverse

  in-place inversion of the lower triangular matrix
============
*/
bool MatX::LowerTriangularInverse( void ) {
	int i, j, k;
	float d, sum;

	for ( i = 0; i < numRows; i++ ) {
		d = (*this)[i][i];
		if ( d == 0.0f ) {
			return false;
		}
		(*this)[i][i] = d = 1.0f / d;

		for ( j = 0; j < i; j++ ) {
			sum = 0.0f;
			for ( k = j; k < i; k++ ) {
				sum -= (*this)[i][k] * (*this)[k][j];
			}
			(*this)[i][j] = sum * d;
		}
	}
	return true;
}

/*
============
MatX::UpperTriangularInverse

  in-place inversion of the upper triangular matrix
============
*/
bool MatX::UpperTriangularInverse( void ) {
	int i, j, k;
	float d, sum;

	for ( i = numRows-1; i >= 0; i-- ) {
		d = (*this)[i][i];
		if ( d == 0.0f ) {
			return false;
		}
		(*this)[i][i] = d = 1.0f / d;

		for ( j = numRows-1; j > i; j-- ) {
			sum = 0.0f;
			for ( k = j; k > i; k-- ) {
				sum -= (*this)[i][k] * (*this)[k][j];
			}
			(*this)[i][j] = sum * d;
		}
	}
	return true;
}

/*
=============
MatX::ToString
=============
*/
const char *MatX::ToString( int precision ) const {
	return Str::FloatArrayToString( ToFloatPtr(), GetDimension(), precision );
}

/*
============
MatX::Update_RankOne

  Updates the matrix to obtain the matrix: A + alpha * v * w'
============
*/
void MatX::Update_RankOne( const VecX &v, const VecX &w, float alpha ) {
	int i, j;
	float s;

	assert( v.GetSize() >= numRows );
	assert( w.GetSize() >= numColumns );

	for ( i = 0; i < numRows; i++ ) {
		s = alpha * v[i];
		for ( j = 0; j < numColumns; j++ ) {
			(*this)[i][j] += s * w[j];
		}
	}
}

/*
============
MatX::Update_RankOneSymmetric

  Updates the matrix to obtain the matrix: A + alpha * v * v'
============
*/
void MatX::Update_RankOneSymmetric( const VecX &v, float alpha ) {
	int i, j;
	float s;

	assert( numRows == numColumns );
	assert( v.GetSize() >= numRows );

	for ( i = 0; i < numRows; i++ ) {
		s = alpha * v[i];
		for ( j = 0; j < numColumns; j++ ) {
			(*this)[i][j] += s * v[j];
		}
	}
}

/*
============
MatX::Update_RowColumn

  Updates the matrix to obtain the matrix:

      [ 0  a  0 ]
  A + [ d  b  e ]
      [ 0  c  0 ]

  where: a = v[0,r-1], b = v[r], c = v[r+1,numRows-1], d = w[0,r-1], w[r] = 0.0f, e = w[r+1,numColumns-1]
============
*/
void MatX::Update_RowColumn( const VecX &v, const VecX &w, int r ) {
	int i;

	assert( w[r] == 0.0f );
	assert( v.GetSize() >= numColumns );
	assert( w.GetSize() >= numRows );

	for ( i = 0; i < numRows; i++ ) {
		(*this)[i][r] += v[i];
	}
	for ( i = 0; i < numColumns; i++ ) {
		(*this)[r][i] += w[i];
	}
}

/*
============
MatX::Update_RowColumnSymmetric

  Updates the matrix to obtain the matrix:

      [ 0  a  0 ]
  A + [ a  b  c ]
      [ 0  c  0 ]

  where: a = v[0,r-1], b = v[r], c = v[r+1,numRows-1]
============
*/
void MatX::Update_RowColumnSymmetric( const VecX &v, int r ) {
	int i;

	assert( numRows == numColumns );
	assert( v.GetSize() >= numRows );

	for ( i = 0; i < r; i++ ) {
		(*this)[i][r] += v[i];
		(*this)[r][i] += v[i];
	}
	(*this)[r][r] += v[r];
	for ( i = r+1; i < numRows; i++ ) {
		(*this)[i][r] += v[i];
		(*this)[r][i] += v[i];
	}
}

/*
============
MatX::Update_Increment

  Updates the matrix to obtain the matrix:

  [ A  a ]
  [ c  b ]

  where: a = v[0,numRows-1], b = v[numRows], c = w[0,numColumns-1]], w[numColumns] = 0
============
*/
void MatX::Update_Increment( const VecX &v, const VecX &w ) {
	int i;

	assert( numRows == numColumns );
	assert( v.GetSize() >= numRows+1 );
	assert( w.GetSize() >= numColumns+1 );

	ChangeSize( numRows+1, numColumns+1, false );

	for ( i = 0; i < numRows; i++ ) {
		(*this)[i][numColumns-1] = v[i];
	}
	for ( i = 0; i < numColumns-1; i++ ) {
		(*this)[numRows-1][i] = w[i];
	}
}

/*
============
MatX::Update_IncrementSymmetric

  Updates the matrix to obtain the matrix:

  [ A  a ]
  [ a  b ]

  where: a = v[0,numRows-1], b = v[numRows]
============
*/
void MatX::Update_IncrementSymmetric( const VecX &v ) {
	int i;

	assert( numRows == numColumns );
	assert( v.GetSize() >= numRows+1 );

	ChangeSize( numRows+1, numColumns+1, false );

	for ( i = 0; i < numRows-1; i++ ) {
		(*this)[i][numColumns-1] = v[i];
	}
	for ( i = 0; i < numColumns; i++ ) {
		(*this)[numRows-1][i] = v[i];
	}
}

/*
============
MatX::Update_Decrement

  Updates the matrix to obtain a matrix with row r and column r removed.
============
*/
void MatX::Update_Decrement( int r ) {
	RemoveRowColumn( r );
}

/*
============
MatX::Inverse_GaussJordan

  in-place inversion using Gauss-Jordan elimination
============
*/
bool MatX::Inverse_GaussJordan( void ) {
	int i, j, k, r, c;
	float d, max;

	assert( numRows == numColumns );

	int *columnIndex = (int *) _alloca16( numRows * sizeof( int ) );
	int *rowIndex = (int *) _alloca16( numRows * sizeof( int ) );
	bool *pivot = (bool *) _alloca16( numRows * sizeof( bool ) );

	Common::Com_Memset( pivot, 0, numRows * sizeof( bool ) );

	// elimination with full pivoting
	for ( i = 0; i < numRows; i++ ) {

		// search the whole matrix except for pivoted rows for the maximum absolute value
		max = 0.0f;
		r = c = 0;
		for ( j = 0; j < numRows; j++ ) {
			if ( !pivot[j] ) {
				for ( k = 0; k < numRows; k++ ) {
					if ( !pivot[k] ) {
						d = fabs( (*this)[j][k] );
						if ( d > max ) {
							max = d;
							r = j;
							c = k;
						}
					}
				}
			}
		}

		if ( max == 0.0f ) {
			// matrix is not invertible
			return false;
		}

		pivot[c] = true;

		// swap rows such that entry (c,c) has the pivot entry
		if ( r != c ) {
			SwapRows( r, c );
		}

		// keep track of the row permutation
		rowIndex[i] = r;
		columnIndex[i] = c;

		// scale the row to make the pivot entry equal to 1
		d = 1.0f / (*this)[c][c];
		(*this)[c][c] = 1.0f;
		for ( k = 0; k < numRows; k++ ) {
			(*this)[c][k] *= d;
		}

		// zero out the pivot column entries in the other rows
		for ( j = 0; j < numRows; j++ ) {
			if ( j != c ) {
				d = (*this)[j][c];
				(*this)[j][c] = 0.0f;
				for ( k = 0; k < numRows; k++ ) {
					(*this)[j][k] -= (*this)[c][k] * d;
				}
			}
		}
	}

	// reorder rows to store the inverse of the original matrix
	for ( j = numRows - 1; j >= 0; j-- ) {
		if ( rowIndex[j] != columnIndex[j] ) {
			for ( k = 0; k < numRows; k++ ) {
				d = (*this)[k][rowIndex[j]];
				(*this)[k][rowIndex[j]] = (*this)[k][columnIndex[j]];
				(*this)[k][columnIndex[j]] = d;
			}
		}
	}

	return true;
}

/*
============
MatX::Inverse_UpdateRankOne

  Updates the in-place inverse using the Sherman-Morrison formula to obtain the inverse for the matrix: A + alpha * v * w'
============
*/
bool MatX::Inverse_UpdateRankOne( const VecX &v, const VecX &w, float alpha ) {
	int i, j;
	float beta, s;
	VecX y, z;

	assert( numRows == numColumns );
	assert( v.GetSize() >= numColumns );
	assert( w.GetSize() >= numRows );

	y.SetData( numRows, VECX_ALLOCA( numRows ) );
	z.SetData( numRows, VECX_ALLOCA( numRows ) );

	Multiply( y, v );
	TransposeMultiply( z, w );
	beta = 1.0f + ( w * y );

	if ( beta == 0.0f ) {
		return false;
	}

	alpha /= beta;

	for ( i = 0; i < numRows; i++ ) {
		s = y[i] * alpha;
		for ( j = 0; j < numColumns; j++ ) {
			(*this)[i][j] -= s * z[j];
		}
	}
	return true;
}

/*
============
MatX::Inverse_UpdateRowColumn

  Updates the in-place inverse to obtain the inverse for the matrix:

      [ 0  a  0 ]
  A + [ d  b  e ]
      [ 0  c  0 ]

  where: a = v[0,r-1], b = v[r], c = v[r+1,numRows-1], d = w[0,r-1], w[r] = 0.0f, e = w[r+1,numColumns-1]
============
*/
bool MatX::Inverse_UpdateRowColumn( const VecX &v, const VecX &w, int r ) {
	VecX s;

	assert( numRows == numColumns );
	assert( v.GetSize() >= numColumns );
	assert( w.GetSize() >= numRows );
	assert( r >= 0 && r < numRows && r < numColumns );
	assert( w[r] == 0.0f );

	s.SetData( Max( numRows, numColumns ), VECX_ALLOCA( Max( numRows, numColumns ) ) );
	s.Zero();
	s[r] = 1.0f;

	if ( !Inverse_UpdateRankOne( v, s, 1.0f ) ) {
		return false;
	}
	if ( !Inverse_UpdateRankOne( s, w, 1.0f ) ) {
		return false;
	}
	return true;
}

/*
============
MatX::Inverse_UpdateIncrement

  Updates the in-place inverse to obtain the inverse for the matrix:

  [ A  a ]
  [ c  b ]

  where: a = v[0,numRows-1], b = v[numRows], c = w[0,numColumns-1], w[numColumns] = 0
============
*/
bool MatX::Inverse_UpdateIncrement( const VecX &v, const VecX &w ) {
	VecX v2;

	assert( numRows == numColumns );
	assert( v.GetSize() >= numRows+1 );
	assert( w.GetSize() >= numColumns+1 );

	ChangeSize( numRows+1, numColumns+1, true );
	(*this)[numRows-1][numRows-1] = 1.0f;

	v2.SetData( numRows, VECX_ALLOCA( numRows ) );
	v2 = v;
	v2[numRows-1] -= 1.0f;

	return Inverse_UpdateRowColumn( v2, w, numRows-1 );
}

/*
============
MatX::Inverse_UpdateDecrement

  Updates the in-place inverse to obtain the inverse of the matrix with row r and column r removed.
  v and w should store the column and row of the original matrix respectively.
============
*/
bool MatX::Inverse_UpdateDecrement( const VecX &v, const VecX &w, int r ) {
	VecX v1, w1;

	assert( numRows == numColumns );
	assert( v.GetSize() >= numRows );
	assert( w.GetSize() >= numColumns );
	assert( r >= 0 && r < numRows && r < numColumns );

	v1.SetData( numRows, VECX_ALLOCA( numRows ) );
	w1.SetData( numRows, VECX_ALLOCA( numRows ) );

	// update the row and column to identity
	v1 = -v;
	w1 = -w;
	v1[r] += 1.0f;
	w1[r] = 0.0f;

	if ( !Inverse_UpdateRowColumn( v1, w1, r ) ) {
		return false;
	}

	// physically remove the row and column
	Update_Decrement( r );

	return true;
}

/*
============
MatX::Inverse_Solve

  Solve Ax = b with A inverted
============
*/
void MatX::Inverse_Solve( VecX &x, const VecX &b ) const {
	Multiply( x, b );
}

/*
============
MatX::LU_Factor

  in-place factorization: LU
  L is a triangular matrix stored in the lower triangle.
  L has ones on the diagonal that are not stored.
  U is a triangular matrix stored in the upper triangle.
  If index != NULL partial pivoting is used for numerical stability.
  If index != NULL it must point to an array of numRow integers and is used to keep track of the row permutation.
  If det != NULL the determinant of the matrix is calculated and stored.
============
*/
bool MatX::LU_Factor( int *index, float *det ) {
	int i, j, k, newi, min;
	float s, t, d, w;

	// if partial pivoting should be used
	if ( index ) {
		for ( i = 0; i < numRows; i++ ) {
			index[i] = i;
		}
	}

	w = 1.0f;
	min = Min( numRows, numColumns );
	for ( i = 0; i < min; i++ ) {

		newi = i;
		s = fabs( (*this)[i][i] );

		if ( index ) {
			// find the largest absolute pivot
			for ( j = i + 1; j < numRows; j++ ) {
				t = fabs( (*this)[j][i] );
				if ( t > s ) {
					newi = j;
					s = t;
				}
			}
		}

		if ( s == 0.0f ) {
			return false;
		}

		if ( newi != i ) {

			w = -w;

			// swap index elements
			k = index[i];
			index[i] = index[newi];
			index[newi] = k;

			// swap rows
			for ( j = 0; j < numColumns; j++ ) {
				t = (*this)[newi][j];
				(*this)[newi][j] = (*this)[i][j];
				(*this)[i][j] = t;
			}
		}

		if ( i < numRows ) {
			d = 1.0f / (*this)[i][i];
			for ( j = i + 1; j < numRows; j++ ) {
				(*this)[j][i] *= d;
			}
		}

		if ( i < min-1 ) {
			for ( j = i + 1; j < numRows; j++ ) {
				d = (*this)[j][i];
				for ( k = i + 1; k < numColumns; k++ ) {
					(*this)[j][k] -= d * (*this)[i][k];
				}
			}
		}
	}

	if ( det ) {
		for ( i = 0; i < numRows; i++ ) {
			w *= (*this)[i][i];
		}
		*det = w;
	}

	return true;
}   

/*
============
MatX::LU_UpdateRankOne

  Updates the in-place LU factorization to obtain the factors for the matrix: LU + alpha * v * w'
============
*/
bool MatX::LU_UpdateRankOne( const VecX &v, const VecX &w, float alpha, int *index ) {
	int i, j, max;
	float *y, *z;
	float diag, beta, p0, p1, d;

	assert( v.GetSize() >= numColumns );
	assert( w.GetSize() >= numRows );

	y = (float *) _alloca16( v.GetSize() * sizeof( float ) );
	z = (float *) _alloca16( w.GetSize() * sizeof( float ) );

	if ( index != NULL ) {
		for ( i = 0; i < numRows; i++ ) {
			y[i] = alpha * v[index[i]];
		}
	} else {
		for ( i = 0; i < numRows; i++ ) {
			y[i] = alpha * v[i];
		}
	}

	memcpy( z, w.ToFloatPtr(), w.GetSize() * sizeof( float ) );

	max = Min( numRows, numColumns );
	for ( i = 0; i < max; i++ ) {
		diag = (*this)[i][i];

		p0 = y[i];
		p1 = z[i];
		diag += p0 * p1;

		if ( diag == 0.0f ) {
			return false;
		}

		beta = p1 / diag;

		(*this)[i][i] = diag;

		for ( j = i+1; j < numColumns; j++ ) {

			d = (*this)[i][j];

			d += p0 * z[j];
			z[j] -= beta * d;

			(*this)[i][j] = d;
		}

		for ( j = i+1; j < numRows; j++ ) {

			d = (*this)[j][i];

			y[j] -= p0 * d;
			d += beta * y[j];

			(*this)[j][i] = d;
		}
	}
	return true;
}

/*
============
MatX::LU_UpdateRowColumn

  Updates the in-place LU factorization to obtain the factors for the matrix:

       [ 0  a  0 ]
  LU + [ d  b  e ]
       [ 0  c  0 ]

  where: a = v[0,r-1], b = v[r], c = v[r+1,numRows-1], d = w[0,r-1], w[r] = 0.0f, e = w[r+1,numColumns-1]
============
*/
bool MatX::LU_UpdateRowColumn( const VecX &v, const VecX &w, int r, int *index ) {
#if 0

	VecX s;

	assert( v.GetSize() >= numColumns );
	assert( w.GetSize() >= numRows );
	assert( r >= 0 && r < numRows && r < numColumns );
	assert( w[r] == 0.0f );

	s.SetData( Max( numRows, numColumns ), VECX_ALLOCA( Max( numRows, numColumns ) ) );
	s.Zero();
	s[r] = 1.0f;

	if ( !LU_UpdateRankOne( v, s, 1.0f, index ) ) {
		return false;
	}
	if ( !LU_UpdateRankOne( s, w, 1.0f, index ) ) {
		return false;
	}
	return true;

#else

	int i, j, min, max, rp;
	float *y0, *y1, *z0, *z1;
	float diag, beta0, beta1, p0, p1, q0, q1, d;

	assert( v.GetSize() >= numColumns );
	assert( w.GetSize() >= numRows );
	assert( r >= 0 && r < numColumns && r < numRows );
	assert( w[r] == 0.0f );

	y0 = (float *) _alloca16( v.GetSize() * sizeof( float ) );
	z0 = (float *) _alloca16( w.GetSize() * sizeof( float ) );
	y1 = (float *) _alloca16( v.GetSize() * sizeof( float ) );
	z1 = (float *) _alloca16( w.GetSize() * sizeof( float ) );

	if ( index != NULL ) {
		for ( i = 0; i < numRows; i++ ) {
			y0[i] = v[index[i]];
		}
		rp = r;
		for ( i = 0; i < numRows; i++ ) {
			if ( index[i] == r ) {
				rp = i;
				break;
			}
		}
	} else {
		memcpy( y0, v.ToFloatPtr(), v.GetSize() * sizeof( float ) );
		rp = r;
	}

	Common::Com_Memset( y1, 0, v.GetSize() * sizeof( float ) );
	y1[rp] = 1.0f;

	Common::Com_Memset( z0, 0, w.GetSize() * sizeof( float ) );
	z0[r] = 1.0f;

	memcpy( z1, w.ToFloatPtr(), w.GetSize() * sizeof( float ) );

	// update the beginning of the to be updated row and column
	min = Min( r, rp );
	for ( i = 0; i < min; i++ ) {
		p0 = y0[i];
		beta1 = z1[i] / (*this)[i][i];

		(*this)[i][r] += p0;
		for ( j = i+1; j < numColumns; j++ ) {
			z1[j] -= beta1 * (*this)[i][j];
		}
		for ( j = i+1; j < numRows; j++ ) {
			y0[j] -= p0 * (*this)[j][i];
		}
		(*this)[rp][i] += beta1;
	}

	// update the lower right corner starting at r,r
	max = Min( numRows, numColumns );
	for ( i = min; i < max; i++ ) {
		diag = (*this)[i][i];

		p0 = y0[i];
		p1 = z0[i];
		diag += p0 * p1;

		if ( diag == 0.0f ) {
			return false;
		}

		beta0 = p1 / diag;

		q0 = y1[i];
		q1 = z1[i];
		diag += q0 * q1;

		if ( diag == 0.0f ) {
			return false;
		}

		beta1 = q1 / diag;

		(*this)[i][i] = diag;

		for ( j = i+1; j < numColumns; j++ ) {

			d = (*this)[i][j];

			d += p0 * z0[j];
			z0[j] -= beta0 * d;

			d += q0 * z1[j];
			z1[j] -= beta1 * d;

			(*this)[i][j] = d;
		}

		for ( j = i+1; j < numRows; j++ ) {

			d = (*this)[j][i];

			y0[j] -= p0 * d;
			d += beta0 * y0[j];

			y1[j] -= q0 * d;
			d += beta1 * y1[j];

			(*this)[j][i] = d;
		}
	}
	return true;

#endif
}

/*
============
MatX::LU_UpdateIncrement

  Updates the in-place LU factorization to obtain the factors for the matrix:

  [ A  a ]
  [ c  b ]

  where: a = v[0,numRows-1], b = v[numRows], c = w[0,numColumns-1], w[numColumns] = 0
============
*/
bool MatX::LU_UpdateIncrement( const VecX &v, const VecX &w, int *index ) {
	int i, j;
	float sum;

	assert( numRows == numColumns );
	assert( v.GetSize() >= numRows+1 );
	assert( w.GetSize() >= numColumns+1 );

	ChangeSize( numRows+1, numColumns+1, true );

	// add row to L
	for ( i = 0; i < numRows - 1; i++ ) {
		sum = w[i];
		for ( j = 0; j < i; j++ ) {
			sum -= (*this)[numRows - 1][j] * (*this)[j][i];
		}
		(*this)[numRows - 1 ][i] = sum / (*this)[i][i];
	}

	// add row to the permutation index
	if ( index != NULL ) {
		index[numRows - 1] = numRows - 1;
	}

	// add column to U
	for ( i = 0; i < numRows; i++ ) {
		if ( index != NULL ) {
			sum = v[index[i]];
		} else {
			sum = v[i];
		}
		for ( j = 0; j < i; j++ ) {
			sum -= (*this)[i][j] * (*this)[j][numRows - 1];
		}
		(*this)[i][numRows - 1] = sum;
	}

	return true;
}

/*
============
MatX::LU_UpdateDecrement

  Updates the in-place LU factorization to obtain the factors for the matrix with row r and column r removed.
  v and w should store the column and row of the original matrix respectively.
  If index != NULL then u should store row index[r] of the original matrix. If index == NULL then u = w.
============
*/
bool MatX::LU_UpdateDecrement( const VecX &v, const VecX &w, const VecX &u, int r, int *index ) {
	int i, p;
	VecX v1, w1;

	assert( numRows == numColumns );
	assert( v.GetSize() >= numColumns );
	assert( w.GetSize() >= numRows );
	assert( r >= 0 && r < numRows && r < numColumns );

	v1.SetData( numRows, VECX_ALLOCA( numRows ) );
	w1.SetData( numRows, VECX_ALLOCA( numRows ) );

	if ( index != NULL ) {

		// find the pivot row
		for ( p = i = 0; i < numRows; i++ ) {
			if ( index[i] == r ) {
				p = i;
				break;
			}
		}

		// update the row and column to identity
		v1 = -v;
		w1 = -u;

		if ( p != r ) {
			Swap( v1[index[r]], v1[index[p]] );
			Swap( index[r], index[p] );
		}

		v1[r] += 1.0f;
		w1[r] = 0.0f;

		if ( !LU_UpdateRowColumn( v1, w1, r, index ) ) {
			return false;
		}

		if ( p != r ) {

			if ( fabs( u[p] ) < 1e-4f ) {
				// NOTE: an additional row interchange is required for numerical stability
			}

			// move row index[r] of the original matrix to row index[p] of the original matrix
			v1.Zero();
			v1[index[p]] = 1.0f;
			w1 = u - w;

			if ( !LU_UpdateRankOne( v1, w1, 1.0f, index ) ) {
				return false;
			}
		}

		// remove the row from the permutation index
		for ( i = r; i < numRows - 1; i++ ) {
			index[i] = index[i+1];
		}
		for ( i = 0; i < numRows - 1; i++ ) {
			if ( index[i] > r ) {
				index[i]--;
			}
		}

	} else {

		v1 = -v;
		w1 = -w;
		v1[r] += 1.0f;
		w1[r] = 0.0f;

		if ( !LU_UpdateRowColumn( v1, w1, r, index ) ) {
			return false;
		}
	}

	// physically remove the row and column
	Update_Decrement( r );

	return true;
}

/*
============
MatX::LU_Solve

  Solve Ax = b with A factored in-place as: LU
============
*/
void MatX::LU_Solve( VecX &x, const VecX &b, const int *index ) const {
	int i, j;
	float sum;

	assert( x.GetSize() == numColumns && b.GetSize() == numRows );

	// solve L
	for ( i = 0; i < numRows; i++ ) {
		if ( index != NULL ) {
			sum = b[index[i]];
		} else {
			sum = b[i];
		}
		for ( j = 0; j < i; j++ ) {
			sum -= (*this)[i][j] * x[j];
		}
		x[i] = sum;
	}

	// solve U
	for ( i = numRows - 1; i >= 0; i-- ) {
		sum = x[i];
		for ( j = i + 1; j < numRows; j++ ) {
			sum -= (*this)[i][j] * x[j];
		}
		x[i] = sum / (*this)[i][i];
	}
}

/*
============
MatX::LU_Inverse

  Calculates the inverse of the matrix which is factored in-place as LU
============
*/
void MatX::LU_Inverse( MatX &inv, const int *index ) const {
	int i, j;
	VecX x, b;

	assert( numRows == numColumns );

	x.SetData( numRows, VECX_ALLOCA( numRows ) );
	b.SetData( numRows, VECX_ALLOCA( numRows ) );
	b.Zero();
	inv.SetSize( numRows, numColumns );

	for ( i = 0; i < numRows; i++ ) {

		b[i] = 1.0f;
		LU_Solve( x, b, index );
		for ( j = 0; j < numRows; j++ ) {
			inv[j][i] = x[j];
		}
		b[i] = 0.0f;
	}
}

/*
============
MatX::LU_UnpackFactors

  Unpacks the in-place LU factorization.
============
*/
void MatX::LU_UnpackFactors( MatX &L, MatX &U ) const {
	int i, j;

	L.Zero( numRows, numColumns );
	U.Zero( numRows, numColumns );
	for ( i = 0; i < numRows; i++ ) {
		for ( j = 0; j < i; j++ ) {
			L[i][j] = (*this)[i][j];
		}
		L[i][i] = 1.0f;
		for ( j = i; j < numColumns; j++ ) {
			U[i][j] = (*this)[i][j];
		}
	}
}

/*
============
MatX::LU_MultiplyFactors

  Multiplies the factors of the in-place LU factorization to form the original matrix.
============
*/
void MatX::LU_MultiplyFactors( MatX &m, const int *index ) const {
	int r, rp, i, j;
	float sum;

	m.SetSize( numRows, numColumns );

	for ( r = 0; r < numRows; r++ ) {

		if ( index != NULL ) {
			rp = index[r];
		} else {
			rp = r;
		}

		// calculate row of matrix
		for ( i = 0; i < numColumns; i++ ) {
			if ( i >= r ) {
				sum = (*this)[r][i];
			} else {
				sum = 0.0f;
			}
			for ( j = 0; j <= i && j < r; j++ ) {
				sum += (*this)[r][j] * (*this)[j][i];
			}
			m[rp][i] = sum;
		}
	}
}

/*
============
MatX::QR_Factor

  in-place factorization: QR
  Q is an orthogonal matrix represented as a product of Householder matrices stored in the lower triangle and c.
  R is a triangular matrix stored in the upper triangle except for the diagonal elements which are stored in d.
  The initial matrix has to be square.
============
*/
bool MatX::QR_Factor( VecX &c, VecX &d ) {
	int i, j, k;
	float scale, s, t, sum;
	bool singular = false;

	assert( numRows == numColumns );
	assert( c.GetSize() >= numRows && d.GetSize() >= numRows );

	for ( k = 0; k < numRows-1; k++ ) {

		scale = 0.0f;
		for ( i = k; i < numRows; i++ ) {
			s = fabs( (*this)[i][k] );
			if ( s > scale ) {
				scale = s;
			}
		}
		if ( scale == 0.0f ) {
			singular = true;
			c[k] = d[k] = 0.0f;
		} else {

			s = 1.0f / scale;
			for ( i = k; i < numRows; i++ ) {
				(*this)[i][k] *= s;
			}

			sum = 0.0f;
			for ( i = k; i < numRows; i++ ) {
				s = (*this)[i][k];
				sum += s * s;
			}

			s = sqrtf( sum );
			if ( (*this)[k][k] < 0.0f ) {
				s = -s;
			}
			(*this)[k][k] += s;
			c[k] = s * (*this)[k][k];
			d[k] = -scale * s;

			for ( j = k+1; j < numRows; j++ ) {

				sum = 0.0f;
				for ( i = k; i < numRows; i++ ) {
					sum += (*this)[i][k] * (*this)[i][j];
				}
				t = sum / c[k];
				for ( i = k; i < numRows; i++ ) {
					(*this)[i][j] -= t * (*this)[i][k];
				}
			}
		}
	}
	d[numRows-1] = (*this)[ (numRows-1) ][ (numRows-1) ];
	if ( d[numRows-1] == 0.0f ) {
		singular = true;
	}

	return !singular;
}

/*
============
MatX::QR_Rotate

  Performs a Jacobi rotation on the rows i and i+1 of the unpacked QR factors.
============
*/
void MatX::QR_Rotate( MatX &R, int i, float a, float b ) {
	int j;
	float f, c, s, w, y;

	if ( a == 0.0f ) {
		c = 0.0f;
		s = ( b >= 0.0f ) ? 1.0f : -1.0f;
	} else if ( fabs( a ) > fabs( b ) ) {
		f = b / a;
		c = fabs( 1.0f / sqrtf( 1.0f + f * f ) );
		if ( a < 0.0f ) {
			c = -c;
		}
		s = f * c;
	} else {
		f = a / b;
		s = fabs( 1.0f / sqrtf( 1.0f + f * f ) );
		if ( b < 0.0f ) {
			s = -s;
		}
		c = f * s;
	}
	for ( j = i; j < numRows; j++ ) {
		y = R[i][j];
		w = R[i+1][j];
		R[i][j] = c * y - s * w;
		R[i+1][j] = s * y + c * w;
	}
	for ( j = 0; j < numRows; j++ ) {
		y = (*this)[j][i];
		w = (*this)[j][i+1];
		(*this)[j][i] = c * y - s * w;
		(*this)[j][i+1] = s * y + c * w;
	}
}

/*
============
MatX::QR_UpdateRankOne

  Updates the unpacked QR factorization to obtain the factors for the matrix: QR + alpha * v * w'
============
*/
bool MatX::QR_UpdateRankOne( MatX &R, const VecX &v, const VecX &w, float alpha ) {
	int i, k;
	float f;
	VecX u;

	assert( v.GetSize() >= numColumns );
	assert( w.GetSize() >= numRows );

	u.SetData( v.GetSize(), VECX_ALLOCA( v.GetSize() ) );
	TransposeMultiply( u, v );
	u *= alpha;

	for ( k = v.GetSize()-1; k > 0; k-- ) {
		if ( u[k] != 0.0f ) {
			break;
		}
	}
	for ( i = k-1; i >= 0; i-- ) {
		QR_Rotate( R, i, u[i], -u[i+1] );
		if ( u[i] == 0.0f ) {
			u[i] = fabs( u[i+1] );
		} else if ( fabs( u[i] ) > fabs( u[i+1] ) ) {
			f = u[i+1] / u[i];
			u[i] = fabs( u[i] ) * sqrtf( 1.0f + f * f );
		} else {
			f = u[i] / u[i+1];
			u[i] = fabs( u[i+1] ) * sqrtf( 1.0f + f * f );
		}
	}
	for ( i = 0; i < v.GetSize(); i++ ) {
		R[0][i] += u[0] * w[i];
	}
	for ( i = 0; i < k; i++ ) {
		QR_Rotate( R, i, -R[i][i], R[i+1][i] );
	}
	return true;
}

/*
============
MatX::QR_UpdateRowColumn

  Updates the unpacked QR factorization to obtain the factors for the matrix:

       [ 0  a  0 ]
  QR + [ d  b  e ]
       [ 0  c  0 ]

  where: a = v[0,r-1], b = v[r], c = v[r+1,numRows-1], d = w[0,r-1], w[r] = 0.0f, e = w[r+1,numColumns-1]
============
*/
bool MatX::QR_UpdateRowColumn( MatX &R, const VecX &v, const VecX &w, int r ) {
	VecX s;

	assert( v.GetSize() >= numColumns );
	assert( w.GetSize() >= numRows );
	assert( r >= 0 && r < numRows && r < numColumns );
	assert( w[r] == 0.0f );

	s.SetData( Max( numRows, numColumns ), VECX_ALLOCA( Max( numRows, numColumns ) ) );
	s.Zero();
	s[r] = 1.0f;

	if ( !QR_UpdateRankOne( R, v, s, 1.0f ) ) {
		return false;
	}
	if ( !QR_UpdateRankOne( R, s, w, 1.0f ) ) {
		return false;
	}
	return true;
}

/*
============
MatX::QR_UpdateIncrement

  Updates the unpacked QR factorization to obtain the factors for the matrix:

  [ A  a ]
  [ c  b ]

  where: a = v[0,numRows-1], b = v[numRows], c = w[0,numColumns-1], w[numColumns] = 0
============
*/
bool MatX::QR_UpdateIncrement( MatX &R, const VecX &v, const VecX &w ) {
	VecX v2;

	assert( numRows == numColumns );
	assert( v.GetSize() >= numRows+1 );
	assert( w.GetSize() >= numColumns+1 );

	ChangeSize( numRows+1, numColumns+1, true );
	(*this)[numRows-1][numRows-1] = 1.0f;

	R.ChangeSize( R.numRows+1, R.numColumns+1, true );
	R[R.numRows-1][R.numRows-1] = 1.0f;

	v2.SetData( numRows, VECX_ALLOCA( numRows ) );
	v2 = v;
	v2[numRows-1] -= 1.0f;

	return QR_UpdateRowColumn( R, v2, w, numRows-1 );
}

/*
============
MatX::QR_UpdateDecrement

  Updates the unpacked QR factorization to obtain the factors for the matrix with row r and column r removed.
  v and w should store the column and row of the original matrix respectively.
============
*/
bool MatX::QR_UpdateDecrement( MatX &R, const VecX &v, const VecX &w, int r ) {
	VecX v1, w1;

	assert( numRows == numColumns );
	assert( v.GetSize() >= numRows );
	assert( w.GetSize() >= numColumns );
	assert( r >= 0 && r < numRows && r < numColumns );

	v1.SetData( numRows, VECX_ALLOCA( numRows ) );
	w1.SetData( numRows, VECX_ALLOCA( numRows ) );

	// update the row and column to identity
	v1 = -v;
	w1 = -w;
	v1[r] += 1.0f;
	w1[r] = 0.0f;

	if ( !QR_UpdateRowColumn( R, v1, w1, r ) ) {
		return false;
	}

	// physically remove the row and column
	Update_Decrement( r );
	R.Update_Decrement( r );

	return true;
}

/*
============
MatX::QR_Solve

  Solve Ax = b with A factored in-place as: QR
============
*/
void MatX::QR_Solve( VecX &x, const VecX &b, const VecX &c, const VecX &d ) const {
	int i, j;
	float sum, t;

	assert( numRows == numColumns );
	assert( x.GetSize() >= numRows && b.GetSize() >= numRows );
	assert( c.GetSize() >= numRows && d.GetSize() >= numRows );

	for ( i = 0; i < numRows; i++ ) {
		x[i] = b[i];
	}

	// multiply b with transpose of Q
	for ( i = 0; i < numRows-1; i++ ) {

		sum = 0.0f;
		for ( j = i; j < numRows; j++ ) {
			sum += (*this)[j][i] * x[j];
		}
		t = sum / c[i];
		for ( j = i; j < numRows; j++ ) {
			x[j] -= t * (*this)[j][i];
		}
	}

	// backsubstitution with R
	for ( i = numRows-1; i >= 0; i-- ) {

		sum = x[i];
		for ( j = i + 1; j < numRows; j++ ) {
			sum -= (*this)[i][j] * x[j];
		}
		x[i] = sum / d[i];
	}
}

/*
============
MatX::QR_Solve

  Solve Ax = b with A factored as: QR
============
*/
void MatX::QR_Solve( VecX &x, const VecX &b, const MatX &R ) const {
	int i, j;
	float sum;

	assert( numRows == numColumns );

	// multiply b with transpose of Q
	TransposeMultiply( x, b );

	// backsubstitution with R
	for ( i = numRows-1; i >= 0; i-- ) {

		sum = x[i];
		for ( j = i + 1; j < numRows; j++ ) {
			sum -= R[i][j] * x[j];
		}
		x[i] = sum / R[i][i];
	}
}

/*
============
MatX::QR_Inverse

  Calculates the inverse of the matrix which is factored in-place as: QR
============
*/
void MatX::QR_Inverse( MatX &inv, const VecX &c, const VecX &d ) const {
	int i, j;
	VecX x, b;

	assert( numRows == numColumns );

	x.SetData( numRows, VECX_ALLOCA( numRows ) );
	b.SetData( numRows, VECX_ALLOCA( numRows ) );
	b.Zero();
	inv.SetSize( numRows, numColumns );

	for ( i = 0; i < numRows; i++ ) {

		b[i] = 1.0f;
		QR_Solve( x, b, c, d );
		for ( j = 0; j < numRows; j++ ) {
			inv[j][i] = x[j];
		}
		b[i] = 0.0f;
	}
}

/*
============
MatX::QR_UnpackFactors

  Unpacks the in-place QR factorization.
============
*/
void MatX::QR_UnpackFactors( MatX &Q, MatX &R, const VecX &c, const VecX &d ) const {
	int i, j, k;
	float sum;

	Q.Identity( numRows, numColumns );
	for ( i = 0; i < numColumns-1; i++ ) {
		if ( c[i] == 0.0f ) {
			continue;
		}
		for ( j = 0; j < numRows; j++ ) {
			sum = 0.0f;
			for ( k = i; k < numColumns; k++ ) {
				sum += (*this)[k][i] * Q[j][k];
			}
			sum /= c[i];
			for ( k = i; k < numColumns; k++ ) {
				Q[j][k] -= sum * (*this)[k][i];
			}
		}
	}

	R.Zero( numRows, numColumns );
	for ( i = 0; i < numRows; i++ ) {
		R[i][i] = d[i];
		for ( j = i+1; j < numColumns; j++ ) {
			R[i][j] = (*this)[i][j];
		}
	}
}

/*
============
MatX::QR_MultiplyFactors

  Multiplies the factors of the in-place QR factorization to form the original matrix.
============
*/
void MatX::QR_MultiplyFactors( MatX &m, const VecX &c, const VecX &d ) const {
	int i, j, k;
	float sum;
	MatX Q;

	Q.Identity( numRows, numColumns );
	for ( i = 0; i < numColumns-1; i++ ) {
		if ( c[i] == 0.0f ) {
			continue;
		}
		for ( j = 0; j < numRows; j++ ) {
			sum = 0.0f;
			for ( k = i; k < numColumns; k++ ) {
				sum += (*this)[k][i] * Q[j][k];
			}
			sum /= c[i];
			for ( k = i; k < numColumns; k++ ) {
				Q[j][k] -= sum * (*this)[k][i];
			}
		}
	}

	for ( i = 0; i < numRows; i++ ) {
		for ( j = 0; j < numColumns; j++ ) {
			sum = Q[i][j] * d[i];
			for ( k = 0; k < i; k++ ) {
				sum += Q[i][k] * (*this)[j][k];
			}
			m[i][j] = sum;
		}
	}
}

/*
============
MatX::Pythag

  Computes (a^2 + b^2)^1/2 without underflow or overflow.
============
*/
float MatX::Pythag( float a, float b ) const {
	float at, bt, ct;

	at = fabs( a );
	bt = fabs( b );
	if ( at > bt ) {
		ct = bt / at;
		return at * sqrtf( 1.0f + ct * ct );
	} else {
		if ( bt ) {
			ct = at / bt;
			return bt * sqrtf( 1.0f + ct * ct );
		} else {
			return 0.0f;
		}
	}
}

/*
============
MatX::SVD_BiDiag
============
*/
void MatX::SVD_BiDiag( VecX &w, VecX &rv1, float &anorm ) {
	int i, j, k, l;
	float f, h, r, g, s, scale;

	anorm = 0.0f;
	g = s = scale = 0.0f;
	for ( i = 0; i < numColumns; i++ ) {
		l = i + 1;
		rv1[i] = scale * g;
		g = s = scale = 0.0f;
		if ( i < numRows ) {
			for ( k = i; k < numRows; k++ ) {
				scale += fabs( (*this)[k][i] );
			}
			if ( scale ) {
				for ( k = i; k < numRows; k++ ) {
					(*this)[k][i] /= scale;
					s += (*this)[k][i] * (*this)[k][i];
				}
				f = (*this)[i][i];
				g = sqrtf( s );
				if ( f >= 0.0f ) {
					g = -g;
				}
				h = f * g - s;
				(*this)[i][i] = f - g;
				if ( i != (numColumns-1) ) {
					for ( j = l; j < numColumns; j++ ) {
						for ( s = 0.0f, k = i; k < numRows; k++ ) {
							s += (*this)[k][i] * (*this)[k][j];
						}
						f = s / h;
						for ( k = i; k < numRows; k++ ) {
							(*this)[k][j] += f * (*this)[k][i];
						}
					}
				}
				for ( k = i; k < numRows; k++ ) {
					(*this)[k][i] *= scale;
				}
			}
		}
		w[i] = scale * g;
		g = s = scale = 0.0f;
		if ( i < numRows && i != (numColumns-1) ) {
			for ( k = l; k < numColumns; k++ ) {
				scale += fabs( (*this)[i][k] );
			}
			if ( scale ) {
				for ( k = l; k < numColumns; k++ ) {
					(*this)[i][k] /= scale;
					s += (*this)[i][k] * (*this)[i][k];
				}
				f = (*this)[i][l];
				g = sqrtf( s );
				if ( f >= 0.0f ) {
					g = -g;
				}
				h = 1.0f / ( f * g - s );
				(*this)[i][l] = f - g;
				for ( k = l; k < numColumns; k++ ) {
					rv1[k] = (*this)[i][k] * h;
				}
				if ( i != (numRows-1) ) {
					for ( j = l; j < numRows; j++ ) {
						for ( s = 0.0f, k = l; k < numColumns; k++ ) {
							s += (*this)[j][k] * (*this)[i][k];
						}
						for ( k = l; k < numColumns; k++ ) {
							(*this)[j][k] += s * rv1[k];
						}
					}
				}
				for ( k = l; k < numColumns; k++ ) {
					(*this)[i][k] *= scale;
				}
			}
		}
		r = fabs( w[i] ) + fabs( rv1[i] );
		if ( r > anorm ) {
			anorm = r;
		}
	}
}

/*
============
MatX::SVD_InitialWV
============
*/
void MatX::SVD_InitialWV( VecX &w, MatX &V, VecX &rv1 ) {
	int i, j, k, l;
	float f, g, s;

	g = 0.0f;
	for ( i = (numColumns-1); i >= 0; i-- ) {
		l = i + 1;
		if ( i < ( numColumns - 1 ) ) {
			if ( g ) {
				for ( j = l; j < numColumns; j++ ) {
					V[j][i] = ((*this)[i][j] / (*this)[i][l]) / g;
				}
				// float division to reduce underflow
				for ( j = l; j < numColumns; j++ ) {
					for ( s = 0.0f, k = l; k < numColumns; k++ ) {
						s += (*this)[i][k] * V[k][j];
					}
					for ( k = l; k < numColumns; k++ ) {
						V[k][j] += s * V[k][i];
					}
				}
			}
			for ( j = l; j < numColumns; j++ ) {
				V[i][j] = V[j][i] = 0.0f;
			}
		}
		V[i][i] = 1.0f;
		g = rv1[i];
	}
	for ( i = numColumns - 1 ; i >= 0; i-- ) {
		l = i + 1;
		g = w[i];
		if ( i < (numColumns-1) ) {
			for ( j = l; j < numColumns; j++ ) {
				(*this)[i][j] = 0.0f;
			}
		}
		if ( g ) {
			g = 1.0f / g;
			if ( i != (numColumns-1) ) {
				for ( j = l; j < numColumns; j++ ) {
					for ( s = 0.0f, k = l; k < numRows; k++ ) {
						s += (*this)[k][i] * (*this)[k][j];
					}
					f = (s / (*this)[i][i]) * g;
					for ( k = i; k < numRows; k++ ) {
						(*this)[k][j] += f * (*this)[k][i];
					}
				}
			}
			for ( j = i; j < numRows; j++ ) {
				(*this)[j][i] *= g;
			}
		}
		else {
			for ( j = i; j < numRows; j++ ) {
				(*this)[j][i] = 0.0f;
			}
		}
		(*this)[i][i] += 1.0f;
	}
}

/*
============
MatX::SVD_Factor

  in-place factorization: U * Diag(w) * V.Transpose()
  known as the Singular Value Decomposition.
  U is a column-orthogonal matrix which overwrites the original matrix.
  w is a diagonal matrix with all elements >= 0 which are the singular values.
  V is the transpose of an orthogonal matrix.
============
*/
bool MatX::SVD_Factor( VecX &w, MatX &V ) {
	int flag, i, its, j, jj, k, l, nm;
	float c, f, h, s, x, y, z, r, g = 0.0f;
	float anorm = 0.0f;
	VecX rv1;

	if ( numRows < numColumns ) {
		return false;
	}

	rv1.SetData( numColumns, VECX_ALLOCA( numColumns ) );
	rv1.Zero();
	w.Zero( numColumns );
	V.Zero( numColumns, numColumns );

	SVD_BiDiag( w, rv1, anorm );
	SVD_InitialWV( w, V, rv1 );

	for ( k = numColumns - 1; k >= 0; k-- ) {
		for ( its = 1; its <= 30; its++ ) {
			flag = 1;
			nm = 0;
			for ( l = k; l >= 0; l-- ) {
				nm = l - 1;
				if ( ( fabs( rv1[l] ) + anorm ) == anorm /* fabs( rv1[l] ) < idMath::FLT_EPSILON */ ) {
					flag = 0;
					break;
				}
				if ( ( fabs( w[nm] ) + anorm ) == anorm /* fabs( w[nm] ) < idMath::FLT_EPSILON */ ) {
					break;
				}
			}
			if ( flag ) {
				c = 0.0f;
				s = 1.0f;
				for ( i = l; i <= k; i++ ) {
					f = s * rv1[i];

					if ( ( fabs( f ) + anorm ) != anorm /* fabs( f ) > idMath::FLT_EPSILON */ ) {
						g = w[i];
						h = Pythag( f, g );
						w[i] = h;
						h = 1.0f / h;
						c = g * h;
						s = -f * h;
						for ( j = 0; j < numRows; j++ ) {
							y = (*this)[j][nm];
							z = (*this)[j][i];
							(*this)[j][nm] = y * c + z * s;
							(*this)[j][i] = z * c - y * s;
						}
					}
				}
			}
			z = w[k];
			if ( l == k ) {
				if ( z < 0.0f ) {
					w[k] = -z;
					for ( j = 0; j < numColumns; j++ ) {
						V[j][k] = -V[j][k];
					}
				}
				break;
			}
			if ( its == 30 ) {
				return false;		// no convergence
			}
			x = w[l];
			nm = k - 1;
			y = w[nm];
			g = rv1[nm];
			h = rv1[k];
			f = ( ( y - z ) * ( y + z ) + ( g - h ) * ( g + h ) ) / ( 2.0f * h * y );
			g = Pythag( f, 1.0f );
			r = ( f >= 0.0f ? g : - g );
			f= ( ( x - z ) * ( x + z ) + h * ( ( y / ( f + r ) ) - h ) ) / x;
			c = s = 1.0f;
			for ( j = l; j <= nm; j++ ) {
				i = j + 1;
				g = rv1[i];
				y = w[i];
				h = s * g;
				g = c * g;
				z = Pythag( f, h );
				rv1[j] = z;
				c = f / z;
				s = h / z;
				f = x * c + g * s;
				g = g * c - x * s;
				h = y * s;
				y = y * c;
				for ( jj = 0; jj < numColumns; jj++ ) {
					x = V[jj][j];
					z = V[jj][i];
					V[jj][j] = x * c + z * s;
					V[jj][i] = z * c - x * s;
				}
				z = Pythag( f, h );
				w[j] = z;
				if ( z ) {
					z = 1.0f / z;
					c = f * z;
					s = h * z;
				}
				f = ( c * g ) + ( s * y );
				x = ( c * y ) - ( s * g );
				for ( jj = 0; jj < numRows; jj++ ) {
					y = (*this)[jj][j];
					z = (*this)[jj][i];
					(*this)[jj][j] = y * c + z * s;
					(*this)[jj][i] = z * c - y * s;
				}
			}
			rv1[l] = 0.0f;
			rv1[k] = f;
			w[k] = x;
		}
	}
	return true;
}

/*
============
MatX::SVD_Solve

  Solve Ax = b with A factored as: U * Diag(w) * V.Transpose()
============
*/
void MatX::SVD_Solve( VecX &x, const VecX &b, const VecX &w, const MatX &V ) const {
	int i, j;
	float sum;
	VecX tmp;

	assert( x.GetSize() >= numColumns );
	assert( b.GetSize() >= numColumns );
	assert( w.GetSize() == numColumns );
	assert( V.GetNumRows() == numColumns && V.GetNumColumns() == numColumns );

	tmp.SetData( numColumns, VECX_ALLOCA( numColumns ) );

	for ( i = 0; i < numColumns; i++ ) {
		sum = 0.0f;
		if ( w[i] >= FLT_EPSILON ) {
			for ( j = 0; j < numRows; j++ ) {
				sum += (*this)[j][i] * b[j];
			}
			sum /= w[i];
		}
		tmp[i] = sum;
	}
	for ( i = 0; i < numColumns; i++ ) {
		sum = 0.0f;
		for ( j = 0; j < numColumns; j++ ) {
			sum += V[i][j] * tmp[j];
		}
		x[i] = sum;
	}
}

/*
============
MatX::SVD_Inverse

  Calculates the inverse of the matrix which is factored in-place as: U * Diag(w) * V.Transpose()
============
*/
void MatX::SVD_Inverse( MatX &inv, const VecX &w, const MatX &V ) const {
	int i, j, k;
	float wi, sum;
	MatX V2;

	assert( numRows == numColumns );

	V2 = V;

	// V * [diag(1/w[i])]
	for ( i = 0; i < numRows; i++ ) {
		wi = w[i];
		wi = ( wi < FLT_EPSILON ) ? 0.0f : 1.0f / wi;
		for ( j = 0; j < numColumns; j++ ) {
			V2[j][i] *= wi;
		}
	}

	// V * [diag(1/w[i])] * Ut
	for ( i = 0; i < numRows; i++ ) {
		for ( j = 0; j < numColumns; j++ ) {
			sum = V2[i][0] * (*this)[j][0];
			for ( k = 1; k < numColumns; k++ ) {
				sum += V2[i][k] * (*this)[j][k];
			}
			inv[i][j] = sum;
		}
	}
}

/*
============
MatX::SVD_MultiplyFactors

  Multiplies the factors of the in-place SVD factorization to form the original matrix.
============
*/
void MatX::SVD_MultiplyFactors( MatX &m, const VecX &w, const MatX &V ) const {
	int r, i, j;
	float sum;

	m.SetSize( numRows, V.GetNumRows() );

	for ( r = 0; r < numRows; r++ ) {
		// calculate row of matrix
		if ( w[r] >= FLT_EPSILON ) {
			for ( i = 0; i < V.GetNumRows(); i++ ) {
				sum = 0.0f;
				for ( j = 0; j < numColumns; j++ ) {
					sum += (*this)[r][j] * V[i][j];
				}
				m[r][i] = sum * w[r];
			}
		} else {
			for ( i = 0; i < V.GetNumRows(); i++ ) {
				m[r][i] = 0.0f;
			}
		}
	}
}

/*
============
MatX::Cholesky_Factor

  in-place Cholesky factorization: LL'
  L is a triangular matrix stored in the lower triangle.
  The upper triangle is not cleared.
  The initial matrix has to be symmetric positive definite.
============
*/
bool MatX::Cholesky_Factor( void ) {
	int i, j, k;
	float *invSqrt;
	float sum;

	assert( numRows == numColumns );

	invSqrt = (float *) _alloca16( numRows * sizeof( float ) );

	for ( i = 0; i < numRows; i++ ) {

		for ( j = 0; j < i; j++ ) {

			sum = (*this)[i][j];
			for ( k = 0; k < j; k++ ) {
				sum -= (*this)[i][k] * (*this)[j][k];
			}
			(*this)[i][j] = sum * invSqrt[j];
		}

		sum = (*this)[i][i];
		for ( k = 0; k < i; k++ ) {
			sum -= (*this)[i][k] * (*this)[i][k];
		}

		if ( sum <= 0.0f ) {
			return false;
		}

		invSqrt[i] = InvSqrt( sum );
		(*this)[i][i] = invSqrt[i] * sum;
	}
	return true;
}

/*
============
MatX::Cholesky_UpdateRankOne

  Updates the in-place Cholesky factorization to obtain the factors for the matrix: LL' + alpha * v * v'
  If offset > 0 only the lower right corner starting at (offset, offset) is updated.
============
*/
bool MatX::Cholesky_UpdateRankOne( const VecX &v, float alpha, int offset ) {
	int i, j;
	float *y;
	float diag, invDiag, diagSqr, newDiag, newDiagSqr, beta, p, d;

	assert( numRows == numColumns );
	assert( v.GetSize() >= numRows );
	assert( offset >= 0 && offset < numRows );

	y = (float *) _alloca16( v.GetSize() * sizeof( float ) );
	memcpy( y, v.ToFloatPtr(), v.GetSize() * sizeof( float ) );

	for ( i = offset; i < numColumns; i++ ) {
		p = y[i];
		diag = (*this)[i][i];
		invDiag = 1.0f / diag;
		diagSqr = diag * diag;
		newDiagSqr = diagSqr + alpha * p * p;

		if ( newDiagSqr <= 0.0f ) {
			return false;
		}

		(*this)[i][i] = newDiag = sqrtf( newDiagSqr );

		alpha /= newDiagSqr;
		beta = p * alpha;
		alpha *= diagSqr;

		for ( j = i+1; j < numRows; j++ ) {

			d = (*this)[j][i] * invDiag;

			y[j] -= p * d;
			d += beta * y[j];

			(*this)[j][i] = d * newDiag;
		}
	}
	return true;
}

/*
============
MatX::Cholesky_UpdateRowColumn

  Updates the in-place Cholesky factorization to obtain the factors for the matrix:

        [ 0  a  0 ]
  LL' + [ a  b  c ]
        [ 0  c  0 ]

  where: a = v[0,r-1], b = v[r], c = v[r+1,numRows-1]
============
*/
bool MatX::Cholesky_UpdateRowColumn( const VecX &v, int r ) {
	int i, j;
	float sum;
	float *original, *y;
	VecX addSub;

	assert( numRows == numColumns );
	assert( v.GetSize() >= numRows );
	assert( r >= 0 && r < numRows );

	addSub.SetData( numColumns, (float *) _alloca16( numColumns * sizeof( float ) ) );

	if ( r == 0 ) {

		if ( numColumns == 1 ) {
			float v0 = v[0];
			sum = (*this)[0][0];
			sum = sum * sum; 
			sum = sum + v0; 
			if ( sum <= 0.0f ) {
				return false;
			}
			(*this)[0][0] = sqrtf( sum );
			return true;
		}
		for ( i = 0; i < numColumns; i++ ) {
			addSub[i] = v[i];
		}

	} else {

		original = (float *) _alloca16( numColumns * sizeof( float ) );
		y = (float *) _alloca16( numColumns * sizeof( float ) );

		// calculate original row/column of matrix
		for ( i = 0; i < numRows; i++ ) {
			sum = 0.0f;
			for ( j = 0; j <= i; j++ ) {
				sum += (*this)[r][j] * (*this)[i][j];
			}
			original[i] = sum;
		}

		// solve for y in L * y = original + v
		for ( i = 0; i < r; i++ ) {
			sum = original[i] + v[i];
			for ( j = 0; j < i; j++ ) {
				sum -= (*this)[r][j] * (*this)[i][j];
			}
			(*this)[r][i] = sum / (*this)[i][i];
		}

		// if the last row/column of the matrix is updated
		if ( r == numColumns - 1 ) {
			// only calculate new diagonal
			sum = original[r] + v[r];
			for ( j = 0; j < r; j++) {
				sum -= (*this)[r][j] * (*this)[r][j];
			}
			if ( sum <= 0.0f ) {
				return false;
			}
			(*this)[r][r] = sqrtf( sum );
			return true;
		}

		// calculate the row/column to be added to the lower right sub matrix starting at (r, r)
		for ( i = r; i < numColumns; i++ ) {
			sum = 0.0f;
			for ( j = 0; j <= r; j++ ) {
				sum += (*this)[r][j] * (*this)[i][j];
			}
			addSub[i] = v[i] - ( sum - original[i] );
		}
	}

	// add row/column to the lower right sub matrix starting at (r, r)

#if 0

	VecX v1, v2;
	float d;

	v1.SetData( numColumns, (float *) _alloca16( numColumns * sizeof( float ) ) );
	v2.SetData( numColumns, (float *) _alloca16( numColumns * sizeof( float ) ) );

	d = sqrtf_1OVER2;
	v1[r] = ( 0.5f * addSub[r] + 1.0f ) * d;
	v2[r] = ( 0.5f * addSub[r] - 1.0f ) * d;
	for ( i = r+1; i < numColumns; i++ ) {
		v1[i] = v2[i] = addSub[i] * d;
	}

	// update
	if ( !Cholesky_UpdateRankOne( v1, 1.0f, r ) ) {
		return false;
	}
	// downdate
	if ( !Cholesky_UpdateRankOne( v2, -1.0f, r ) ) {
		return false;
	}

#else

	float *v1, *v2;
	float diag, invDiag, diagSqr, newDiag, newDiagSqr;
	float alpha1, alpha2, beta1, beta2, p1, p2, d;

	v1 = (float *) _alloca16( numColumns * sizeof( float ) );
	v2 = (float *) _alloca16( numColumns * sizeof( float ) );

	d = SQRT_1OVER2;
	v1[r] = ( 0.5f * addSub[r] + 1.0f ) * d;
	v2[r] = ( 0.5f * addSub[r] - 1.0f ) * d;
	for ( i = r+1; i < numColumns; i++ ) {
		v1[i] = v2[i] = addSub[i] * d;
	}

	alpha1 = 1.0f;
	alpha2 = -1.0f;

	// simultaneous update/downdate of the sub matrix starting at (r, r)
	for ( i = r; i < numColumns; i++ ) {
		p1 = v1[i];
		diag = (*this)[i][i];
		invDiag = 1.0f / diag;
		diagSqr = diag * diag;
		newDiagSqr = diagSqr + alpha1 * p1 * p1;

		if ( newDiagSqr <= 0.0f ) {
			return false;
		}

		alpha1 /= newDiagSqr;
		beta1 = p1 * alpha1;
		alpha1 *= diagSqr;

		p2 = v2[i];
		diagSqr = newDiagSqr;
		newDiagSqr = diagSqr + alpha2 * p2 * p2;

		if ( newDiagSqr <= 0.0f ) {
			return false;
		}

		(*this)[i][i] = newDiag = sqrtf( newDiagSqr );

		alpha2 /= newDiagSqr;
		beta2 = p2 * alpha2;
		alpha2 *= diagSqr;

		for ( j = i+1; j < numRows; j++ ) {

			d = (*this)[j][i] * invDiag;

			v1[j] -= p1 * d;
			d += beta1 * v1[j];

			v2[j] -= p2 * d;
			d += beta2 * v2[j];

			(*this)[j][i] = d * newDiag;
		}
	}

#endif

	return true;
}

/*
============
MatX::Cholesky_UpdateIncrement

  Updates the in-place Cholesky factorization to obtain the factors for the matrix:

  [ A  a ]
  [ a  b ]

  where: a = v[0,numRows-1], b = v[numRows]
============
*/
bool MatX::Cholesky_UpdateIncrement( const VecX &v ) {
	int i, j;
	float *x;
	float sum;

	assert( numRows == numColumns );
	assert( v.GetSize() >= numRows+1 );

	ChangeSize( numRows+1, numColumns+1, false );

	x = (float *) _alloca16( numRows * sizeof( float ) );

	// solve for x in L * x = v
	for ( i = 0; i < numRows - 1; i++ ) {
		sum = v[i];
		for ( j = 0; j < i; j++ ) {
			sum -= (*this)[i][j] * x[j];
		}
		x[i] = sum / (*this)[i][i];
	}

	// calculate new row of L and calculate the square of the diagonal entry
	sum = v[numRows - 1];
	for ( i = 0; i < numRows - 1; i++ ) {
		(*this)[numRows - 1][i] = x[i];
		sum -= x[i] * x[i];
	}

	if ( sum <= 0.0f ) {
		return false;
	}

	// store the diagonal entry
	(*this)[numRows - 1][numRows - 1] = sqrtf( sum );

	return true;
}

/*
============
MatX::Cholesky_UpdateDecrement

  Updates the in-place Cholesky factorization to obtain the factors for the matrix with row r and column r removed.
  v should store the row of the original matrix.
============
*/
bool MatX::Cholesky_UpdateDecrement( const VecX &v, int r ) {
	VecX v1;

	assert( numRows == numColumns );
	assert( v.GetSize() >= numRows );
	assert( r >= 0 && r < numRows );

	v1.SetData( numRows, VECX_ALLOCA( numRows ) );

	// update the row and column to identity
	v1 = -v;
	v1[r] += 1.0f;

	// NOTE:	msvc compiler bug: the this pointer stored in edi is expected to stay
	//			untouched when calling Cholesky_UpdateRowColumn in the if statement
#if 0
	if ( !Cholesky_UpdateRowColumn( v1, r ) ) {
#else
	bool ret = Cholesky_UpdateRowColumn( v1, r );
	if ( !ret ) {
#endif
		return false;
	}

	// physically remove the row and column
	Update_Decrement( r );

	return true;
}

/*
============
MatX::Cholesky_Solve

  Solve Ax = b with A factored in-place as: LL'
============
*/
void MatX::Cholesky_Solve( VecX &x, const VecX &b ) const {
	int i, j;
	float sum;

	assert( numRows == numColumns );
	assert( x.GetSize() >= numRows && b.GetSize() >= numRows );

	// solve L
	for ( i = 0; i < numRows; i++ ) {
		sum = b[i];
		for ( j = 0; j < i; j++ ) {
			sum -= (*this)[i][j] * x[j];
		}
		x[i] = sum / (*this)[i][i];
	}

	// solve Lt
	for ( i = numRows - 1; i >= 0; i-- ) {
		sum = x[i];
		for ( j = i + 1; j < numRows; j++ ) {
			sum -= (*this)[j][i] * x[j];
		}
		x[i] = sum / (*this)[i][i];
	}
}

/*
============
MatX::Cholesky_Inverse

  Calculates the inverse of the matrix which is factored in-place as: LL'
============
*/
void MatX::Cholesky_Inverse( MatX &inv ) const {
	int i, j;
	VecX x, b;

	assert( numRows == numColumns );

	x.SetData( numRows, VECX_ALLOCA( numRows ) );
	b.SetData( numRows, VECX_ALLOCA( numRows ) );
	b.Zero();
	inv.SetSize( numRows, numColumns );

	for ( i = 0; i < numRows; i++ ) {

		b[i] = 1.0f;
		Cholesky_Solve( x, b );
		for ( j = 0; j < numRows; j++ ) {
			inv[j][i] = x[j];
		}
		b[i] = 0.0f;
	}
}

/*
============
MatX::Cholesky_MultiplyFactors

  Multiplies the factors of the in-place Cholesky factorization to form the original matrix.
============
*/
void MatX::Cholesky_MultiplyFactors( MatX &m ) const {
	int r, i, j;
	float sum;

	m.SetSize( numRows, numColumns );

	for ( r = 0; r < numRows; r++ ) {

		// calculate row of matrix
		for ( i = 0; i < numRows; i++ ) {
			sum = 0.0f;
			for ( j = 0; j <= i && j <= r; j++ ) {
				sum += (*this)[r][j] * (*this)[i][j];
			}
			m[r][i] = sum;
		}
	}
}

/*
============
MatX::LDLT_Factor

  in-place factorization: LDL'
  L is a triangular matrix stored in the lower triangle.
  L has ones on the diagonal that are not stored.
  D is a diagonal matrix stored on the diagonal.
  The upper triangle is not cleared.
  The initial matrix has to be symmetric.
============
*/
bool MatX::LDLT_Factor( void ) {
	int i, j, k;
	float *v;
	float d, sum;

	assert( numRows == numColumns );

	v = (float *) _alloca16( numRows * sizeof( float ) );

	for ( i = 0; i < numRows; i++ ) {

		sum = (*this)[i][i];
		for ( j = 0; j < i; j++ ) {
			d = (*this)[i][j];
		    v[j] = (*this)[j][j] * d;
		    sum -= v[j] * d;
		}

		if ( sum == 0.0f ) {
			return false;
		}

		(*this)[i][i] = sum;
		d = 1.0f / sum;

		for ( j = i + 1; j < numRows; j++ ) {
		    sum = (*this)[j][i];
			for ( k = 0; k < i; k++ ) {
				sum -= (*this)[j][k] * v[k];
			}
		    (*this)[j][i] = sum * d;
		}
	}

	return true;
}

/*
============
MatX::LDLT_UpdateRankOne

  Updates the in-place LDL' factorization to obtain the factors for the matrix: LDL' + alpha * v * v'
  If offset > 0 only the lower right corner starting at (offset, offset) is updated.
============
*/
bool MatX::LDLT_UpdateRankOne( const VecX &v, float alpha, int offset ) {
	int i, j;
	float *y;
	float diag, newDiag, beta, p, d;

	assert( numRows == numColumns );
	assert( v.GetSize() >= numRows );
	assert( offset >= 0 && offset < numRows );

	y = (float *) _alloca16( v.GetSize() * sizeof( float ) );
	memcpy( y, v.ToFloatPtr(), v.GetSize() * sizeof( float ) );

	for ( i = offset; i < numColumns; i++ ) {
		p = y[i];
		diag = (*this)[i][i];
		(*this)[i][i] = newDiag = diag + alpha * p * p;

		if ( newDiag == 0.0f ) {
			return false;
		}

		alpha /= newDiag;
		beta = p * alpha;
		alpha *= diag;

		for ( j = i+1; j < numRows; j++ ) {

			d = (*this)[j][i];

			y[j] -= p * d;
			d += beta * y[j];

			(*this)[j][i] = d;
		}
	}

	return true;
}

/*
============
MatX::LDLT_UpdateRowColumn

  Updates the in-place LDL' factorization to obtain the factors for the matrix:

         [ 0  a  0 ]
  LDL' + [ a  b  c ]
         [ 0  c  0 ]

  where: a = v[0,r-1], b = v[r], c = v[r+1,numRows-1]
============
*/
bool MatX::LDLT_UpdateRowColumn( const VecX &v, int r ) {
	int i, j;
	float sum;
	float *original, *y;
	VecX addSub;

	assert( numRows == numColumns );
	assert( v.GetSize() >= numRows );
	assert( r >= 0 && r < numRows );

	addSub.SetData( numColumns, (float *) _alloca16( numColumns * sizeof( float ) ) );

	if ( r == 0 ) {

		if ( numColumns == 1 ) {
			(*this)[0][0] += v[0];
			return true;
		}
		for ( i = 0; i < numColumns; i++ ) {
			addSub[i] = v[i];
		}

	} else {

		original = (float *) _alloca16( numColumns * sizeof( float ) );
		y = (float *) _alloca16( numColumns * sizeof( float ) );

		// calculate original row/column of matrix
		for ( i = 0; i < r; i++ ) {
			y[i] = (*this)[r][i] * (*this)[i][i];
		}
		for ( i = 0; i < numColumns; i++ ) {
			if ( i < r ) {
				sum = (*this)[i][i] * (*this)[r][i];
			} else if ( i == r ) {
				sum = (*this)[r][r];
			} else {
				sum = (*this)[r][r] * (*this)[i][r];
			}
			for ( j = 0; j < i && j < r; j++ ) {
				sum += (*this)[i][j] * y[j];
			}
			original[i] = sum;
		}

		// solve for y in L * y = original + v
		for ( i = 0; i < r; i++ ) {
			sum = original[i] + v[i];
			for ( j = 0; j < i; j++ ) {
				sum -= (*this)[i][j] * y[j];
			}
			y[i] = sum;
		}

		// calculate new row of L
		for ( i = 0; i < r; i++ ) {
			(*this)[r][i] = y[i] / (*this)[i][i];
		}

		// if the last row/column of the matrix is updated
		if ( r == numColumns - 1 ) {
			// only calculate new diagonal
			sum = original[r] + v[r];
			for ( j = 0; j < r; j++ ) {
				sum -= (*this)[r][j] * y[j];
			}
			if ( sum == 0.0f ) {
				return false;
			}
			(*this)[r][r] = sum;
			return true;
		}

		// calculate the row/column to be added to the lower right sub matrix starting at (r, r)
		for ( i = 0; i < r; i++ ) {
			y[i] = (*this)[r][i] * (*this)[i][i];
		}
		for ( i = r; i < numColumns; i++ ) {
			if ( i == r ) {
				sum = (*this)[r][r];
			} else {
				sum = (*this)[r][r] * (*this)[i][r];
			}
			for ( j = 0; j < r; j++ ) {
				sum += (*this)[i][j] * y[j];
			}
			addSub[i] = v[i] - ( sum - original[i] );
		}
	}

	// add row/column to the lower right sub matrix starting at (r, r)

#if 0

	VecX v1, v2;
	float d;

	v1.SetData( numColumns, (float *) _alloca16( numColumns * sizeof( float ) ) );
	v2.SetData( numColumns, (float *) _alloca16( numColumns * sizeof( float ) ) );

	d = sqrtf_1OVER2;
	v1[r] = ( 0.5f * addSub[r] + 1.0f ) * d;
	v2[r] = ( 0.5f * addSub[r] - 1.0f ) * d;
	for ( i = r+1; i < numColumns; i++ ) {
		v1[i] = v2[i] = addSub[i] * d;
	}

	// update
	if ( !LDLT_UpdateRankOne( v1, 1.0f, r ) ) {
		return false;
	}
	// downdate
	if ( !LDLT_UpdateRankOne( v2, -1.0f, r ) ) {
		return false;
	}

#else

	float *v1, *v2;
	float d, diag, newDiag, p1, p2, alpha1, alpha2, beta1, beta2;

	v1 = (float *) _alloca16( numColumns * sizeof( float ) );
	v2 = (float *) _alloca16( numColumns * sizeof( float ) );

	d = SQRT_1OVER2;
	v1[r] = ( 0.5f * addSub[r] + 1.0f ) * d;
	v2[r] = ( 0.5f * addSub[r] - 1.0f ) * d;
	for ( i = r+1; i < numColumns; i++ ) {
		v1[i] = v2[i] = addSub[i] * d;
	}

	alpha1 = 1.0f;
	alpha2 = -1.0f;

	// simultaneous update/downdate of the sub matrix starting at (r, r)
	for ( i = r; i < numColumns; i++ ) {

		diag = (*this)[i][i];
		p1 = v1[i];
		newDiag = diag + alpha1 * p1 * p1;

		if ( newDiag == 0.0f ) {
			return false;
		}

		alpha1 /= newDiag;
		beta1 = p1 * alpha1;
		alpha1 *= diag;

		diag = newDiag;
		p2 = v2[i];
		newDiag = diag + alpha2 * p2 * p2;

		if ( newDiag == 0.0f ) {
			return false;
		}

		alpha2 /= newDiag;
		beta2 = p2 * alpha2;
		alpha2 *= diag;

		(*this)[i][i] = newDiag;

		for ( j = i+1; j < numRows; j++ ) {

			d = (*this)[j][i];

			v1[j] -= p1 * d;
			d += beta1 * v1[j];

			v2[j] -= p2 * d;
			d += beta2 * v2[j];

			(*this)[j][i] = d;
		}
	}

#endif

	return true;
}

/*
============
MatX::LDLT_UpdateIncrement

  Updates the in-place LDL' factorization to obtain the factors for the matrix:

  [ A  a ]
  [ a  b ]

  where: a = v[0,numRows-1], b = v[numRows]
============
*/
bool MatX::LDLT_UpdateIncrement( const VecX &v ) {
	int i, j;
	float *x;
	float sum, d;

	assert( numRows == numColumns );
	assert( v.GetSize() >= numRows+1 );

	ChangeSize( numRows+1, numColumns+1, false );

	x = (float *) _alloca16( numRows * sizeof( float ) );

	// solve for x in L * x = v
	for ( i = 0; i < numRows - 1; i++ ) {
		sum = v[i];
		for ( j = 0; j < i; j++ ) {
			sum -= (*this)[i][j] * x[j];
		}
		x[i] = sum;
	}

	// calculate new row of L and calculate the diagonal entry
	sum = v[numRows - 1];
	for ( i = 0; i < numRows - 1; i++ ) {
		(*this)[numRows - 1][i] = d = x[i] / (*this)[i][i];
		sum -= d * x[i];
	}

	if ( sum == 0.0f ) {
		return false;
	}

	// store the diagonal entry
	(*this)[numRows - 1][numRows - 1] = sum;

	return true;
}

/*
============
MatX::LDLT_UpdateDecrement

  Updates the in-place LDL' factorization to obtain the factors for the matrix with row r and column r removed.
  v should store the row of the original matrix.
============
*/
bool MatX::LDLT_UpdateDecrement( const VecX &v, int r ) {
	VecX v1;

	assert( numRows == numColumns );
	assert( v.GetSize() >= numRows );
	assert( r >= 0 && r < numRows );

	v1.SetData( numRows, VECX_ALLOCA( numRows ) );

	// update the row and column to identity
	v1 = -v;
	v1[r] += 1.0f;

	// NOTE:	msvc compiler bug: the this pointer stored in edi is expected to stay
	//			untouched when calling LDLT_UpdateRowColumn in the if statement
#if 0
	if ( !LDLT_UpdateRowColumn( v1, r ) ) {
#else
	bool ret = LDLT_UpdateRowColumn( v1, r );
	if ( !ret ) {
#endif
		return false;
	}

	// physically remove the row and column
	Update_Decrement( r );

	return true;
}

/*
============
MatX::LDLT_Solve

  Solve Ax = b with A factored in-place as: LDL'
============
*/
void MatX::LDLT_Solve( VecX &x, const VecX &b ) const {
	int i, j;
	float sum;

	assert( numRows == numColumns );
	assert( x.GetSize() >= numRows && b.GetSize() >= numRows );

	// solve L
	for ( i = 0; i < numRows; i++ ) {
		sum = b[i];
		for ( j = 0; j < i; j++ ) {
			sum -= (*this)[i][j] * x[j];
		}
		x[i] = sum;
	}

	// solve D
	for ( i = 0; i < numRows; i++ ) {
		x[i] /= (*this)[i][i];
	}

	// solve Lt
	for ( i = numRows - 2; i >= 0; i-- ) {
		sum = x[i];
		for ( j = i + 1; j < numRows; j++ ) {
			sum -= (*this)[j][i] * x[j];
		}
		x[i] = sum;
	}
}

/*
============
MatX::LDLT_Inverse

  Calculates the inverse of the matrix which is factored in-place as: LDL'
============
*/
void MatX::LDLT_Inverse( MatX &inv ) const {
	int i, j;
	VecX x, b;

	assert( numRows == numColumns );

	x.SetData( numRows, VECX_ALLOCA( numRows ) );
	b.SetData( numRows, VECX_ALLOCA( numRows ) );
	b.Zero();
	inv.SetSize( numRows, numColumns );

	for ( i = 0; i < numRows; i++ ) {

		b[i] = 1.0f;
		LDLT_Solve( x, b );
		for ( j = 0; j < numRows; j++ ) {
			inv[j][i] = x[j];
		}
		b[i] = 0.0f;
	}
}

/*
============
MatX::LDLT_UnpackFactors

  Unpacks the in-place LDL' factorization.
============
*/
void MatX::LDLT_UnpackFactors( MatX &L, MatX &D ) const {
	int i, j;

	L.Zero( numRows, numColumns );
	D.Zero( numRows, numColumns );
	for ( i = 0; i < numRows; i++ ) {
		for ( j = 0; j < i; j++ ) {
			L[i][j] = (*this)[i][j];
		}
		L[i][i] = 1.0f;
		D[i][i] = (*this)[i][i];
	}
}

/*
============
MatX::LDLT_MultiplyFactors

  Multiplies the factors of the in-place LDL' factorization to form the original matrix.
============
*/
void MatX::LDLT_MultiplyFactors( MatX &m ) const {
	int r, i, j;
	float *v;
	float sum;

	v = (float *) _alloca16( numRows * sizeof( float ) );
	m.SetSize( numRows, numColumns );

	for ( r = 0; r < numRows; r++ ) {

		// calculate row of matrix
		for ( i = 0; i < r; i++ ) {
			v[i] = (*this)[r][i] * (*this)[i][i];
		}
		for ( i = 0; i < numColumns; i++ ) {
			if ( i < r ) {
				sum = (*this)[i][i] * (*this)[r][i];
			} else if ( i == r ) {
				sum = (*this)[r][r];
			} else {
				sum = (*this)[r][r] * (*this)[i][r];
			}
			for ( j = 0; j < i && j < r; j++ ) {
				sum += (*this)[i][j] * v[j];
			}
			m[r][i] = sum;
		}
	}
}

/*
============
MatX::TriDiagonal_ClearTriangles
============
*/
void MatX::TriDiagonal_ClearTriangles( void ) {
	int i, j;

	assert( numRows == numColumns );
	for ( i = 0; i < numRows-2; i++ ) {
		for ( j = i+2; j < numColumns; j++ ) {
			(*this)[i][j] = 0.0f;
			(*this)[j][i] = 0.0f;
		}
	}
}

/*
============
MatX::TriDiagonal_Solve

  Solve Ax = b with A being tridiagonal.
============
*/
bool MatX::TriDiagonal_Solve( VecX &x, const VecX &b ) const {
	int i;
	float d;
	VecX tmp;

	assert( numRows == numColumns );
	assert( x.GetSize() >= numRows && b.GetSize() >= numRows );

	tmp.SetData( numRows, VECX_ALLOCA( numRows ) );

	d = (*this)[0][0];
	if ( d == 0.0f ) {
		return false;
	}
	d = 1.0f / d;
	x[0] = b[0] * d;
	for ( i = 1; i < numRows; i++ ) {
		tmp[i] = (*this)[i-1][i] * d;
		d = (*this)[i][i] - (*this)[i][i-1] * tmp[i];
		if ( d == 0.0f ) {
			return false;
		}
		d = 1.0f / d;
		x[i] = ( b[i] - (*this)[i][i-1] * x[i-1] ) * d;
	}
	for ( i = numRows - 2; i >= 0; i-- ) {
		x[i] -= tmp[i+1] * x[i+1];
	}
	return true;
}

/*
============
MatX::TriDiagonal_Inverse

  Calculates the inverse of a tri-diagonal matrix.
============
*/
void MatX::TriDiagonal_Inverse( MatX &inv ) const {
	int i, j;
	VecX x, b;

	assert( numRows == numColumns );

	x.SetData( numRows, VECX_ALLOCA( numRows ) );
	b.SetData( numRows, VECX_ALLOCA( numRows ) );
	b.Zero();
	inv.SetSize( numRows, numColumns );

	for ( i = 0; i < numRows; i++ ) {

		b[i] = 1.0f;
		TriDiagonal_Solve( x, b );
		for ( j = 0; j < numRows; j++ ) {
			inv[j][i] = x[j];
		}
		b[i] = 0.0f;
	}
}

/*
============
MatX::HouseholderReduction

  Householder reduction to symmetric tri-diagonal form.
  The original matrix is replaced by an orthogonal matrix effecting the accumulated householder transformations.
  The diagonal elements of the diagonal matrix are stored in diag.
  The off-diagonal elements of the diagonal matrix are stored in subd.
  The initial matrix has to be symmetric.
============
*/
void MatX::HouseholderReduction( VecX &diag, VecX &subd ) {
	int i0, i1, i2, i3;
	float h, f, g, invH, halfFdivH, scale, invScale, sum;

	assert( numRows == numColumns );

	diag.SetSize( numRows );
	subd.SetSize( numRows );

	for ( i0 = numRows-1, i3 = numRows-2; i0 >= 1; i0--, i3-- ) {
		h = 0.0f;
		scale = 0.0f;

		if ( i3 > 0 ) {
			for ( i2 = 0; i2 <= i3; i2++ ) {
				scale += fabs( (*this)[i0][i2] );
			}
			if ( scale == 0 ) {
				subd[i0] = (*this)[i0][i3];
			} else {
				invScale = 1.0f / scale;
				for (i2 = 0; i2 <= i3; i2++)
				{
					(*this)[i0][i2] *= invScale;
					h += (*this)[i0][i2] * (*this)[i0][i2];
				}
				f = (*this)[i0][i3];
				g = sqrtf( h );
				if ( f > 0.0f ) {
					g = -g;
				}
				subd[i0] = scale * g;
				h -= f * g;
				(*this)[i0][i3] = f - g;
				f = 0.0f;
				invH = 1.0f / h;
				for (i1 = 0; i1 <= i3; i1++) {
					(*this)[i1][i0] = (*this)[i0][i1] * invH;
					g = 0.0f;
					for (i2 = 0; i2 <= i1; i2++) {
						g += (*this)[i1][i2] * (*this)[i0][i2];
					}
					for (i2 = i1+1; i2 <= i3; i2++) {
						g += (*this)[i2][i1] * (*this)[i0][i2];
					}
					subd[i1] = g * invH;
					f += subd[i1] * (*this)[i0][i1];
				}
				halfFdivH = 0.5f * f * invH;
				for ( i1 = 0; i1 <= i3; i1++ ) {
					f = (*this)[i0][i1];
					g = subd[i1] - halfFdivH * f;
					subd[i1] = g;
					for ( i2 = 0; i2 <= i1; i2++ ) {
						(*this)[i1][i2] -= f * subd[i2] + g * (*this)[i0][i2];
					}
				}
            }
		} else {
			subd[i0] = (*this)[i0][i3];
		}

		diag[i0] = h;
	}

	diag[0] = 0.0f;
	subd[0] = 0.0f;
	for ( i0 = 0, i3 = -1; i0 <= numRows-1; i0++, i3++ ) {
		if ( diag[i0] ) {
			for ( i1 = 0; i1 <= i3; i1++ ) {
				sum = 0.0f;
				for (i2 = 0; i2 <= i3; i2++) {
					sum += (*this)[i0][i2] * (*this)[i2][i1];
				}
				for ( i2 = 0; i2 <= i3; i2++ ) {
					(*this)[i2][i1] -= sum * (*this)[i2][i0];
				}
			}
		}
		diag[i0] = (*this)[i0][i0];
		(*this)[i0][i0] = 1.0f;
		for ( i1 = 0; i1 <= i3; i1++ ) {
			(*this)[i1][i0] = 0.0f;
			(*this)[i0][i1] = 0.0f;
		}
	}

	// re-order
	for ( i0 = 1, i3 = 0; i0 < numRows; i0++, i3++ ) {
		subd[i3] = subd[i0];
	}
	subd[numRows-1] = 0.0f;
}

/*
============
MatX::QL

  QL algorithm with implicit shifts to determine the eigenvalues and eigenvectors of a symmetric tri-diagonal matrix.
  diag contains the diagonal elements of the symmetric tri-diagonal matrix on input and is overwritten with the eigenvalues.
  subd contains the off-diagonal elements of the symmetric tri-diagonal matrix and is destroyed.
  This matrix has to be either the identity matrix to determine the eigenvectors for a symmetric tri-diagonal matrix,
  or the matrix returned by the Householder reduction to determine the eigenvalues for the original symmetric matrix.
============
*/
bool MatX::QL( VecX &diag, VecX &subd ) {
    const int maxIter = 32;
	int i0, i1, i2, i3;
	float a, b, f, g, r, p, s, c;

	assert( numRows == numColumns );

	for ( i0 = 0; i0 < numRows; i0++ ) {
		for ( i1 = 0; i1 < maxIter; i1++ ) {
			for ( i2 = i0; i2 <= numRows - 2; i2++ ) {
				a = fabs( diag[i2] ) + fabs( diag[i2+1] );
				if ( fabs( subd[i2] ) + a == a ) {
					break;
				}
			}
			if ( i2 == i0 ) {
				break;
			}

			g = ( diag[i0+1] - diag[i0] ) / ( 2.0f * subd[i0] );
			r = sqrtf( g * g + 1.0f );
			if ( g < 0.0f ) {
				g = diag[i2] - diag[i0] + subd[i0] / ( g - r );
			} else {
				g = diag[i2] - diag[i0] + subd[i0] / ( g + r );
			}
			s = 1.0f;
			c = 1.0f;
			p = 0.0f;
			for ( i3 = i2 - 1; i3 >= i0; i3-- ) {
				f = s * subd[i3];
				b = c * subd[i3];
				if ( fabs( f ) >= fabs( g ) ) {
					c = g / f;
					r = sqrtf( c * c + 1.0f );
					subd[i3+1] = f * r;
					s = 1.0f / r;
					c *= s;
				} else {
					s = f / g;
					r = sqrtf( s * s + 1.0f );
					subd[i3+1] = g * r;
					c = 1.0f / r;
					s *= c;
				}
				g = diag[i3+1] - p;
				r = ( diag[i3] - g ) * s + 2.0f * b * c;
				p = s * r;
				diag[i3+1] = g + p;
				g = c * r - b;

				for ( int i4 = 0; i4 < numRows; i4++ ) {
					f = (*this)[i4][i3+1];
					(*this)[i4][i3+1] = s * (*this)[i4][i3] + c * f;
					(*this)[i4][i3] = c * (*this)[i4][i3] - s * f;
				}
			}
			diag[i0] -= p;
			subd[i0] = g;
			subd[i2] = 0.0f;
		}
		if ( i1 == maxIter ) {
			return false;
		}
	}
	return true;
}

/*
============
MatX::Eigen_SolveSymmetricTriDiagonal

  Determine eigen values and eigen vectors for a symmetric tri-diagonal matrix.
  The eigen values are stored in 'eigenValues'.
  Column i of the original matrix will store the eigen vector corresponding to the eigenValues[i].
  The initial matrix has to be symmetric tri-diagonal.
============
*/
bool MatX::Eigen_SolveSymmetricTriDiagonal( VecX &eigenValues ) {
	int i;
	VecX subd;

	assert( numRows == numColumns );

	subd.SetData( numRows, VECX_ALLOCA( numRows ) );
	eigenValues.SetSize( numRows );

	for ( i = 0; i < numRows-1; i++ ) {
		eigenValues[i] = (*this)[i][i];
		subd[i] = (*this)[i+1][i];
	}
	eigenValues[numRows-1] = (*this)[numRows-1][numRows-1];

	Identity();

	return QL( eigenValues, subd );
}

/*
============
MatX::Eigen_SolveSymmetric

  Determine eigen values and eigen vectors for a symmetric matrix.
  The eigen values are stored in 'eigenValues'.
  Column i of the original matrix will store the eigen vector corresponding to the eigenValues[i].
  The initial matrix has to be symmetric.
============
*/
bool MatX::Eigen_SolveSymmetric( VecX &eigenValues ) {
	VecX subd;

	assert( numRows == numColumns );

	subd.SetData( numRows, VECX_ALLOCA( numRows ) );
	eigenValues.SetSize( numRows );

	HouseholderReduction( eigenValues, subd );
	return QL( eigenValues, subd );
}

/*
============
MatX::HessenbergReduction

  Reduction to Hessenberg form.
============
*/
void MatX::HessenbergReduction( MatX &H ) {
	int i, j, m;
	int low = 0;
	int high = numRows - 1;
	float scale, f, g, h;
	VecX v;

	v.SetData( numRows, VECX_ALLOCA( numRows ) );

	for ( m = low + 1; m <= high - 1; m++ ) {

		scale = 0.0f;
		for ( i = m; i <= high; i++ ) {
			scale = scale + fabs( H[i][m-1] );
		}
		if ( scale != 0.0f ) {

			// compute Householder transformation.
			h = 0.0f;
			for ( i = high; i >= m; i-- ) {
				v[i] = H[i][m-1] / scale;
				h += v[i] * v[i];
			}
			g = sqrtf( h );
			if ( v[m] > 0.0f ) {
				g = -g;
			}
			h = h - v[m] * g;
			v[m] = v[m] - g;

			// apply Householder similarity transformation
			// H = (I-u*u'/h)*H*(I-u*u')/h)
			for ( j = m; j < numRows; j++) {
				f = 0.0f;
				for ( i = high; i >= m; i-- ) {
					f += v[i] * H[i][j];
				}
				f = f / h;
				for ( i = m; i <= high; i++ ) {
					H[i][j] -= f * v[i];
				}
			}

			for ( i = 0; i <= high; i++ ) {
				f = 0.0f;
				for ( j = high; j >= m; j-- ) {
					f += v[j] * H[i][j];
				}
				f = f / h;
				for ( j = m; j <= high; j++ ) {
					H[i][j] -= f * v[j];
				}
			}
			v[m] = scale * v[m];
			H[m][m-1] = scale * g;
		}
	}

	// accumulate transformations
	Identity();
	for ( int m = high - 1; m >= low + 1; m-- ) {
		if ( H[m][m-1] != 0.0f ) {
			for ( i = m + 1; i <= high; i++ ) {
				v[i] = H[i][m-1];
			}
			for ( j = m; j <= high; j++ ) {
				g = 0.0f;
				for ( i = m; i <= high; i++ ) {
					g += v[i] * (*this)[i][j];
				}
				// float division to avoid possible underflow
				g = ( g / v[m] ) / H[m][m-1];
				for ( i = m; i <= high; i++ ) {
					(*this)[i][j] += g * v[i];
				}
			}
		}
	}
}

/*
============
MatX::ComplexDivision

  Complex scalar division.
============
*/
void MatX::ComplexDivision( float xr, float xi, float yr, float yi, float &cdivr, float &cdivi ) {
	float r, d;
	if ( fabs( yr ) > fabs( yi ) ) {
		r = yi / yr;
		d = yr + r * yi;
		cdivr = ( xr + r * xi ) / d;
		cdivi = ( xi - r * xr ) / d;
	} else {
		r = yr / yi;
		d = yi + r * yr;
		cdivr = ( r * xr + xi ) / d;
		cdivi = ( r * xi - xr ) / d;
	}
}

/*
============
MatX::HessenbergToRealSchur

  Reduction from Hessenberg to real Schur form.
============
*/
bool MatX::HessenbergToRealSchur( MatX &H, VecX &realEigenValues, VecX &imaginaryEigenValues ) {
	int i, j, k;
	int n = numRows - 1;
	int low = 0;
	int high = numRows - 1;
	float eps = 2e-16f, exshift = 0.0f;
	float p = 0.0f, q = 0.0f, r = 0.0f, s = 0.0f, z = 0.0f, t, w, x, y;

	// store roots isolated by balanc and compute matrix norm
	float norm = 0.0f;
	for ( i = 0; i < numRows; i++ ) {
		if ( i < low || i > high ) {
			realEigenValues[i] = H[i][i];
			imaginaryEigenValues[i] = 0.0f;
		}
		for ( j = Max( i - 1, 0 ); j < numRows; j++ ) {
			norm = norm + fabs( H[i][j] );
		}
	}

	int iter = 0;
	while( n >= low ) {

		// look for single small sub-diagonal element
		int l = n;
		while ( l > low ) {
			s = fabs( H[l-1][l-1] ) + fabs( H[l][l] );
			if ( s == 0.0f ) {
				s = norm;
			}
			if ( fabs( H[l][l-1] ) < eps * s ) {
				break;
			}
			l--;
		}
	   
		// check for convergence
		if ( l == n ) {			// one root found
			H[n][n] = H[n][n] + exshift;
			realEigenValues[n] = H[n][n];
			imaginaryEigenValues[n] = 0.0f;
			n--;
			iter = 0;
		} else if ( l == n-1 ) {	// two roots found
			w = H[n][n-1] * H[n-1][n];
			p = ( H[n-1][n-1] - H[n][n] ) / 2.0f;
			q = p * p + w;
			z = sqrtf( fabs( q ) );
			H[n][n] = H[n][n] + exshift;
			H[n-1][n-1] = H[n-1][n-1] + exshift;
			x = H[n][n];

			if ( q >= 0.0f ) {		// real pair
				if ( p >= 0.0f ) {
					z = p + z;
				} else {
					z = p - z;
				}
				realEigenValues[n-1] = x + z;
				realEigenValues[n] = realEigenValues[n-1];
				if ( z != 0.0f ) {
					realEigenValues[n] = x - w / z;
				}
				imaginaryEigenValues[n-1] = 0.0f;
				imaginaryEigenValues[n] = 0.0f;
				x = H[n][n-1];
				s = fabs( x ) + fabs( z );
				p = x / s;
				q = z / s;
				r = sqrtf( p * p + q * q );
				p = p / r;
				q = q / r;

				// modify row
				for ( j = n-1; j < numRows; j++ ) {
					z = H[n-1][j];
					H[n-1][j] = q * z + p * H[n][j];
					H[n][j] = q * H[n][j] - p * z;
				}

				// modify column
				for ( i = 0; i <= n; i++ ) {
					z = H[i][n-1];
					H[i][n-1] = q * z + p * H[i][n];
					H[i][n] = q * H[i][n] - p * z;
				}

				// accumulate transformations
				for ( i = low; i <= high; i++ ) {
					z = (*this)[i][n-1];
					(*this)[i][n-1] = q * z + p * (*this)[i][n];
					(*this)[i][n] = q * (*this)[i][n] - p * z;
				}
			} else {		// complex pair
				realEigenValues[n-1] = x + p;
				realEigenValues[n] = x + p;
				imaginaryEigenValues[n-1] = z;
				imaginaryEigenValues[n] = -z;
			}
			n = n - 2;
			iter = 0;

		} else {	// no convergence yet

			// form shift
			x = H[n][n];
			y = 0.0f;
			w = 0.0f;
			if ( l < n ) {
				y = H[n-1][n-1];
				w = H[n][n-1] * H[n-1][n];
			}

			// Wilkinson's original ad hoc shift
			if ( iter == 10 ) {
				exshift += x;
				for ( i = low; i <= n; i++ ) {
					H[i][i] -= x;
				}
				s = fabs( H[n][n-1] ) + fabs( H[n-1][n-2] );
				x = y = 0.75f * s;
				w = -0.4375f * s * s;
			}

			// new ad hoc shift
			if ( iter == 30 ) {
				s = ( y - x ) / 2.0f;
				s = s * s + w;
				if ( s > 0 ) {
					s = sqrtf( s );
					if ( y < x ) {
						s = -s;
					}
					s = x - w / ( ( y - x ) / 2.0f + s );
					for ( i = low; i <= n; i++ ) {
						H[i][i] -= s;
					}
					exshift += s;
					x = y = w = 0.964f;
				}
			}

			iter = iter + 1;

			// look for two consecutive small sub-diagonal elements
			int m;
			for( m = n-2; m >= l; m-- ) {
				z = H[m][m];
				r = x - z;
				s = y - z;
				p = ( r * s - w ) / H[m+1][m] + H[m][m+1];
				q = H[m+1][m+1] - z - r - s;
				r = H[m+2][m+1];
				s = fabs( p ) + fabs( q ) + fabs( r );
				p = p / s;
				q = q / s;
				r = r / s;
				if ( m == l ) {
					break;
				}
				if ( fabs( H[m][m-1] ) * ( fabs( q ) + fabs( r ) ) <
						eps * ( fabs( p ) * ( fabs( H[m-1][m-1] ) + fabs( z ) + fabs( H[m+1][m+1] ) ) ) ) {
					break;
				}
			}

			for ( i = m+2; i <= n; i++ ) {
				H[i][i-2] = 0.0f;
				if ( i > m+2 ) {
					H[i][i-3] = 0.0f;
				}
			}

			// float QR step involving rows l:n and columns m:n
			for ( k = m; k <= n-1; k++ ) {
				bool notlast = ( k != n-1 );
				if ( k != m ) {
					p = H[k][k-1];
					q = H[k+1][k-1];
					r = ( notlast ? H[k+2][k-1] : 0.0f );
					x = fabs( p ) + fabs( q ) + fabs( r );
					if ( x != 0.0f ) {
						p = p / x;
						q = q / x;
						r = r / x;
					}
				}
				if ( x == 0.0f ) {
					break;
				}
				s = sqrtf( p * p + q * q + r * r );
				if ( p < 0.0f ) {
					s = -s;
				}
				if ( s != 0.0f ) {
					if ( k != m ) {
						H[k][k-1] = -s * x;
					} else if ( l != m ) {
						H[k][k-1] = -H[k][k-1];
					}
					p = p + s;
					x = p / s;
					y = q / s;
					z = r / s;
					q = q / p;
					r = r / p;

					// modify row
					for ( j = k; j < numRows; j++ ) {
						p = H[k][j] + q * H[k+1][j];
						if ( notlast ) {
							p = p + r * H[k+2][j];
							H[k+2][j] = H[k+2][j] - p * z;
						}
						H[k][j] = H[k][j] - p * x;
						H[k+1][j] = H[k+1][j] - p * y;
					}

					// modify column
					for ( i = 0; i <= Min( n, k + 3 ); i++ ) {
						p = x * H[i][k] + y * H[i][k+1];
						if ( notlast ) {
							p = p + z * H[i][k+2];
							H[i][k+2] = H[i][k+2] - p * r;
						}
						H[i][k] = H[i][k] - p;
						H[i][k+1] = H[i][k+1] - p * q;
					}

					// accumulate transformations
					for ( i = low; i <= high; i++ ) {
						p = x * (*this)[i][k] + y * (*this)[i][k+1];
						if ( notlast ) {
							p = p + z * (*this)[i][k+2];
							(*this)[i][k+2] = (*this)[i][k+2] - p * r;
						}
						(*this)[i][k] = (*this)[i][k] - p;
						(*this)[i][k+1] = (*this)[i][k+1] - p * q;
					}
				}
			}
		}
	}
	
	// backsubstitute to find vectors of upper triangular form
	if ( norm == 0.0f ) {
		return false;
	}

	for ( n = numRows-1; n >= 0; n-- ) {
		p = realEigenValues[n];
		q = imaginaryEigenValues[n];

		if ( q == 0.0f ) {		// real vector
			int l = n;
			H[n][n] = 1.0f;
			for ( i = n-1; i >= 0; i-- ) {
				w = H[i][i] - p;
				r = 0.0f;
				for ( j = l; j <= n; j++ ) {
					r = r + H[i][j] * H[j][n];
				}
				if ( imaginaryEigenValues[i] < 0.0f ) {
					z = w;
					s = r;
				} else {
					l = i;
					if ( imaginaryEigenValues[i] == 0.0f ) {
						if ( w != 0.0f ) {
							H[i][n] = -r / w;
						} else {
							H[i][n] = -r / ( eps * norm );
						}
					} else {		// solve real equations
						x = H[i][i+1];
						y = H[i+1][i];
						q = ( realEigenValues[i] - p ) * ( realEigenValues[i] - p ) + imaginaryEigenValues[i] * imaginaryEigenValues[i];
						t = ( x * s - z * r ) / q;
						H[i][n] = t;
						if ( fabs(x) > fabs( z ) ) {
							H[i+1][n] = ( -r - w * t ) / x;
						} else {
							H[i+1][n] = ( -s - y * t ) / z;
						}
					}

					// overflow control
					t = fabs(H[i][n]);
					if ( ( eps * t ) * t > 1 ) {
						for ( j = i; j <= n; j++ ) {
							H[j][n] = H[j][n] / t;
						}
					}
				}
			}
		} else if ( q < 0.0f ) {	// complex vector
			int l = n-1;

			// last vector component imaginary so matrix is triangular
			if ( fabs( H[n][n-1] ) > fabs( H[n-1][n] ) ) {
				H[n-1][n-1] = q / H[n][n-1];
				H[n-1][n] = -( H[n][n] - p ) / H[n][n-1];
			} else {
				ComplexDivision( 0.0f, -H[n-1][n], H[n-1][n-1]-p, q, H[n-1][n-1], H[n-1][n] );
			}
			H[n][n-1] = 0.0f;
			H[n][n] = 1.0f;
			for ( i = n-2; i >= 0; i-- ) {
				float ra, sa, vr, vi;
				ra = 0.0f;
				sa = 0.0f;
				for ( j = l; j <= n; j++ ) {
					ra = ra + H[i][j] * H[j][n-1];
					sa = sa + H[i][j] * H[j][n];
				}
				w = H[i][i] - p;

				if ( imaginaryEigenValues[i] < 0.0f ) {
					z = w;
					r = ra;
					s = sa;
				} else {
					l = i;
					if ( imaginaryEigenValues[i] == 0.0f ) {
						ComplexDivision( -ra, -sa, w, q, H[i][n-1], H[i][n] );
					} else {
						// solve complex equations
						x = H[i][i+1];
						y = H[i+1][i];
						vr = ( realEigenValues[i] - p ) * ( realEigenValues[i] - p ) + imaginaryEigenValues[i] * imaginaryEigenValues[i] - q * q;
						vi = ( realEigenValues[i] - p ) * 2.0f * q;
						if ( vr == 0.0f && vi == 0.0f ) {
							vr = eps * norm * ( fabs( w ) + fabs( q ) + fabs( x ) + fabs( y ) + fabs( z ) );
						}
						ComplexDivision( x * r - z * ra + q * sa, x * s - z * sa - q * ra, vr, vi, H[i][n-1], H[i][n] );
						if ( fabs( x ) > ( fabs( z ) + fabs( q ) ) ) {
							H[i+1][n-1] = ( -ra - w * H[i][n-1] + q * H[i][n] ) / x;
							H[i+1][n] = ( -sa - w * H[i][n] - q * H[i][n-1] ) / x;
						} else {
							ComplexDivision( -r - y * H[i][n-1], -s - y * H[i][n], z, q, H[i+1][n-1], H[i+1][n] );
						}
					}

					// overflow control
					t = Max( fabs( H[i][n-1] ), fabs( H[i][n] ) );
					if ( ( eps * t ) * t > 1 ) {
						for ( j = i; j <= n; j++ ) {
							H[j][n-1] = H[j][n-1] / t;
							H[j][n] = H[j][n] / t;
						}
					}
				}
			}
		}
	}

	// vectors of isolated roots
	for ( i = 0; i < numRows; i++ ) {
		if ( i < low || i > high ) {
			for ( j = i; j < numRows; j++ ) {
				(*this)[i][j] = H[i][j];
			}
		}
	}

	// back transformation to get eigenvectors of original matrix
	for ( j = numRows - 1; j >= low; j-- ) {
		for ( i = low; i <= high; i++ ) {
			z = 0.0f;
			for ( k = low; k <= Min( j, high ); k++ ) {
				z = z + (*this)[i][k] * H[k][j];
			}
			(*this)[i][j] = z;
		}
	}

	return true;
}

/*
============
MatX::Eigen_Solve

  Determine eigen values and eigen vectors for a square matrix.
  The eigen values are stored in 'realEigenValues' and 'imaginaryEigenValues'.
  Column i of the original matrix will store the eigen vector corresponding to the realEigenValues[i] and imaginaryEigenValues[i].
============
*/
bool MatX::Eigen_Solve( VecX &realEigenValues, VecX &imaginaryEigenValues ) {
    MatX H;

	assert( numRows == numColumns );

	realEigenValues.SetSize( numRows );
	imaginaryEigenValues.SetSize( numRows );

	H = *this;

    // reduce to Hessenberg form
    HessenbergReduction( H );

    // reduce Hessenberg to real Schur form
    return HessenbergToRealSchur( H, realEigenValues, imaginaryEigenValues );
}

/*
============
MatX::Eigen_SortIncreasing
============
*/
void MatX::Eigen_SortIncreasing( VecX &eigenValues ) {
	int i, j, k;
	float min;

	for ( i = 0, j; i <= numRows - 2; i++ ) {
		j = i;
		min = eigenValues[j];
		for ( k = i + 1; k < numRows; k++ ) {
			if ( eigenValues[k] < min ) {
				j = k;
				min = eigenValues[j];
			}
		}
		if ( j != i ) {
			eigenValues.SwapElements( i, j );
			SwapColumns( i, j );
		}
	}
}

/*
============
MatX::Eigen_SortDecreasing
============
*/
void MatX::Eigen_SortDecreasing( VecX &eigenValues ) {
	int i, j, k;
	float max;

	for ( i = 0, j; i <= numRows - 2; i++ ) {
		j = i;
		max = eigenValues[j];
		for ( k = i + 1; k < numRows; k++ ) {
			if ( eigenValues[k] > max ) {
				j = k;
				max = eigenValues[j];
			}
		}
		if ( j != i ) {
			eigenValues.SwapElements( i, j );
			SwapColumns( i, j );
		}
	}
}

/*
============
MatX::DeterminantGeneric
============
*/
float MatX::DeterminantGeneric( void ) const {
	int *index;
	float det;
	MatX tmp;

	index = (int *) _alloca16( numRows * sizeof( int ) );
	tmp.SetData( numRows, numColumns, MATX_ALLOCA( numRows * numColumns ) );
	tmp = *this;

	if ( !tmp.LU_Factor( index, &det ) ) {
		return 0.0f;
	}

	return det;
}

/*
============
MatX::InverseSelfGeneric
============
*/
bool MatX::InverseSelfGeneric( void ) {
	int i, j, *index;
	MatX tmp;
	VecX x, b;

	index = (int *) _alloca16( numRows * sizeof( int ) );
	tmp.SetData( numRows, numColumns, MATX_ALLOCA( numRows * numColumns ) );
	tmp = *this;

	if ( !tmp.LU_Factor( index ) ) {
		return false;
	}

	x.SetData( numRows, VECX_ALLOCA( numRows ) );
	b.SetData( numRows, VECX_ALLOCA( numRows ) );
	b.Zero();

	for ( i = 0; i < numRows; i++ ) {

		b[i] = 1.0f;
		tmp.LU_Solve( x, b, index );
		for ( j = 0; j < numRows; j++ ) {
			(*this)[j][i] = x[j];
		}
		b[i] = 0.0f;
	}
	return true;
}

/*
============
MatX::Test
============
*/
void MatX::Test( void ) {
	MatX original, m1, m2, m3, q1, q2, r1, r2;
	VecX v, w, u, c, d;
	int offset, size, *index1, *index2;

	size = 6;
	original.Random( size, size, 0 );
	original = original * original.Transpose();

	index1 = (int *) _alloca16( ( size + 1 ) * sizeof( index1[0] ) );
	index2 = (int *) _alloca16( ( size + 1 ) * sizeof( index2[0] ) );

	/*
		MatX::LowerTriangularInverse
	*/

	m1 = original;
	m1.ClearUpperTriangle();
	m2 = m1;

	m2.InverseSelf();
	m1.LowerTriangularInverse();

	if ( !m1.Compare( m2, 1e-4f ) ) {
		Common::Com_Printf( "MatX::LowerTriangularInverse failed" );
	}

	/*
		MatX::UpperTriangularInverse
	*/

	m1 = original;
	m1.ClearLowerTriangle();
	m2 = m1;

	m2.InverseSelf();
	m1.UpperTriangularInverse();

	if ( !m1.Compare( m2, 1e-4f ) ) {
		Common::Com_Printf( "MatX::UpperTriangularInverse failed" );
	}

	/*
		MatX::Inverse_GaussJordan
	*/

	m1 = original;

	m1.Inverse_GaussJordan();
	m1 *= original;

	if ( !m1.IsIdentity( 1e-4f ) ) {
		Common::Com_Printf( "MatX::Inverse_GaussJordan failed" );
	}

	/*
		MatX::Inverse_UpdateRankOne
	*/

	m1 = original;
	m2 = original;

	w.Random( size, 1 );
	v.Random( size, 2 );

	// invert m1
	m1.Inverse_GaussJordan();

	// modify and invert m2 
	m2.Update_RankOne( v, w, 1.0f );
	if ( !m2.Inverse_GaussJordan() ) {
		assert( 0 );
	}

	// update inverse of m1
	m1.Inverse_UpdateRankOne( v, w, 1.0f );

	if ( !m1.Compare( m2, 1e-4f ) ) {
		Common::Com_Printf( "MatX::Inverse_UpdateRankOne failed" );
	}

	/*
		MatX::Inverse_UpdateRowColumn
	*/

	for ( offset = 0; offset < size; offset++ ) {
		m1 = original;
		m2 = original;

		v.Random( size, 1 );
		w.Random( size, 2 );
		w[offset] = 0.0f;

		// invert m1
		m1.Inverse_GaussJordan();

		// modify and invert m2
		m2.Update_RowColumn( v, w, offset );
		if ( !m2.Inverse_GaussJordan() ) {
			assert( 0 );
		}

		// update inverse of m1
		m1.Inverse_UpdateRowColumn( v, w, offset );

		if ( !m1.Compare( m2, 1e-3f ) ) {
			Common::Com_Printf( "MatX::Inverse_UpdateRowColumn failed" );
		}
	}

	/*
		MatX::Inverse_UpdateIncrement
	*/

	m1 = original;
	m2 = original;

	v.Random( size + 1, 1 );
	w.Random( size + 1, 2 );
	w[size] = 0.0f;

	// invert m1
	m1.Inverse_GaussJordan();

	// modify and invert m2 
	m2.Update_Increment( v, w );
	if ( !m2.Inverse_GaussJordan() ) {
		assert( 0 );
	}

	// update inverse of m1
	m1.Inverse_UpdateIncrement( v, w );

	if ( !m1.Compare( m2, 1e-4f ) ) {
		Common::Com_Printf( "MatX::Inverse_UpdateIncrement failed" );
	}

	/*
		MatX::Inverse_UpdateDecrement
	*/

	for ( offset = 0; offset < size; offset++ ) {
		m1 = original;
		m2 = original;

		v.SetSize( 6 );
		w.SetSize( 6 );
		for ( int i = 0; i < size; i++ ) {
			v[i] = original[i][offset];
			w[i] = original[offset][i];
		}

		// invert m1
		m1.Inverse_GaussJordan();

		// modify and invert m2
		m2.Update_Decrement( offset );
		if ( !m2.Inverse_GaussJordan() ) {
			assert( 0 );
		}

		// update inverse of m1
		m1.Inverse_UpdateDecrement( v, w, offset );

		if ( !m1.Compare( m2, 1e-3f ) ) {
			Common::Com_Printf( "MatX::Inverse_UpdateDecrement failed" );
		}
	}

	/*
		MatX::LU_Factor
	*/

	m1 = original;

	m1.LU_Factor( NULL );	// no pivoting
	m1.LU_UnpackFactors( m2, m3 );
	m1 = m2 * m3;

	if ( !original.Compare( m1, 1e-4f ) ) {
		Common::Com_Printf( "MatX::LU_Factor failed" );
	}

	/*
		MatX::LU_UpdateRankOne
	*/

	m1 = original;
	m2 = original;

	w.Random( size, 1 );
	v.Random( size, 2 );

	// factor m1
	m1.LU_Factor( index1 );

	// modify and factor m2 
	m2.Update_RankOne( v, w, 1.0f );
	if ( !m2.LU_Factor( index2 ) ) {
		assert( 0 );
	}
	m2.LU_MultiplyFactors( m3, index2 );
	m2 = m3;

	// update factored m1
	m1.LU_UpdateRankOne( v, w, 1.0f, index1 );
	m1.LU_MultiplyFactors( m3, index1 );
	m1 = m3;

	if ( !m1.Compare( m2, 1e-4f ) ) {
		Common::Com_Printf( "MatX::LU_UpdateRankOne failed" );
	}

	/*
		MatX::LU_UpdateRowColumn
	*/

	for ( offset = 0; offset < size; offset++ ) {
		m1 = original;
		m2 = original;

		v.Random( size, 1 );
		w.Random( size, 2 );
		w[offset] = 0.0f;

		// factor m1
		m1.LU_Factor( index1 );

		// modify and factor m2
		m2.Update_RowColumn( v, w, offset );
		if ( !m2.LU_Factor( index2 ) ) {
			assert( 0 );
		}
		m2.LU_MultiplyFactors( m3, index2 );
		m2 = m3;

		// update m1
		m1.LU_UpdateRowColumn( v, w, offset, index1  );
		m1.LU_MultiplyFactors( m3, index1 );
		m1 = m3;

		if ( !m1.Compare( m2, 1e-3f ) ) {
			Common::Com_Printf( "MatX::LU_UpdateRowColumn failed" );
		}
	}

	/*
		MatX::LU_UpdateIncrement
	*/

	m1 = original;
	m2 = original;

	v.Random( size + 1, 1 );
	w.Random( size + 1, 2 );
	w[size] = 0.0f;

	// factor m1
	m1.LU_Factor( index1 );

	// modify and factor m2 
	m2.Update_Increment( v, w );
	if ( !m2.LU_Factor( index2 ) ) {
		assert( 0 );
	}
	m2.LU_MultiplyFactors( m3, index2 );
	m2 = m3;

	// update factored m1
	m1.LU_UpdateIncrement( v, w, index1 );
	m1.LU_MultiplyFactors( m3, index1 );
	m1 = m3;

	if ( !m1.Compare( m2, 1e-4f ) ) {
		Common::Com_Printf( "MatX::LU_UpdateIncrement failed" );
	}

	/*
		MatX::LU_UpdateDecrement
	*/

	for ( offset = 0; offset < size; offset++ ) {
		m1 = original;
		m2 = original;

		v.SetSize( 6 );
		w.SetSize( 6 );
		for ( int i = 0; i < size; i++ ) {
			v[i] = original[i][offset];
			w[i] = original[offset][i];
		}

		// factor m1
		m1.LU_Factor( index1 );

		// modify and factor m2
		m2.Update_Decrement( offset );
		if ( !m2.LU_Factor( index2 ) ) {
			assert( 0 );
		}
		m2.LU_MultiplyFactors( m3, index2 );
		m2 = m3;

		u.SetSize( 6 );
		for ( int i = 0; i < size; i++ ) {
			u[i] = original[index1[offset]][i];
		}

		// update factors of m1
		m1.LU_UpdateDecrement( v, w, u, offset, index1 );
		m1.LU_MultiplyFactors( m3, index1 );
		m1 = m3;

		if ( !m1.Compare( m2, 1e-3f ) ) {
			Common::Com_Printf( "MatX::LU_UpdateDecrement failed" );
		}
	}

	/*
		MatX::LU_Inverse
	*/

	m2 = original;

	m2.LU_Factor( NULL );
	m2.LU_Inverse( m1, NULL );
	m1 *= original;

	if ( !m1.IsIdentity( 1e-4f ) ) {
		Common::Com_Printf( "MatX::LU_Inverse failed" );
	}

	/*
		MatX::QR_Factor
	*/

	c.SetSize( size );
	d.SetSize( size );

	m1 = original;

	m1.QR_Factor( c, d );
	m1.QR_UnpackFactors( q1, r1, c, d );
	m1 = q1 * r1;

	if ( !original.Compare( m1, 1e-4f ) ) {
		Common::Com_Printf( "MatX::QR_Factor failed" );
	}

	/*
		MatX::QR_UpdateRankOne
	*/

	c.SetSize( size );
	d.SetSize( size );

	m1 = original;
	m2 = original;

	w.Random( size, 0 );
	v = w;

	// factor m1
	m1.QR_Factor( c, d );
	m1.QR_UnpackFactors( q1, r1, c, d );

	// modify and factor m2 
	m2.Update_RankOne( v, w, 1.0f );
	if ( !m2.QR_Factor( c, d ) ) {
		assert( 0 );
	}
	m2.QR_UnpackFactors( q2, r2, c, d );
	m2 = q2 * r2;

	// update factored m1
	q1.QR_UpdateRankOne( r1, v, w, 1.0f );
	m1 = q1 * r1;

	if ( !m1.Compare( m2, 1e-4f ) ) {
		Common::Com_Printf( "MatX::QR_UpdateRankOne failed" );
	}

	/*
		MatX::QR_UpdateRowColumn
	*/

	for ( offset = 0; offset < size; offset++ ) {
		c.SetSize( size );
		d.SetSize( size );

		m1 = original;
		m2 = original;

		v.Random( size, 1 );
		w.Random( size, 2 );
		w[offset] = 0.0f;

		// factor m1
		m1.QR_Factor( c, d );
		m1.QR_UnpackFactors( q1, r1, c, d );

		// modify and factor m2
		m2.Update_RowColumn( v, w, offset );
		if ( !m2.QR_Factor( c, d ) ) {
			assert( 0 );
		}
		m2.QR_UnpackFactors( q2, r2, c, d );
		m2 = q2 * r2;

		// update m1
		q1.QR_UpdateRowColumn( r1, v, w, offset );
		m1 = q1 * r1;

		if ( !m1.Compare( m2, 1e-3f ) ) {
			Common::Com_Printf( "MatX::QR_UpdateRowColumn failed" );
		}
	}

	/*
		MatX::QR_UpdateIncrement
	*/

	c.SetSize( size+1 );
	d.SetSize( size+1 );

	m1 = original;
	m2 = original;

	v.Random( size + 1, 1 );
	w.Random( size + 1, 2 );
	w[size] = 0.0f;

	// factor m1
	m1.QR_Factor( c, d );
	m1.QR_UnpackFactors( q1, r1, c, d );

	// modify and factor m2 
	m2.Update_Increment( v, w );
	if ( !m2.QR_Factor( c, d ) ) {
		assert( 0 );
	}
	m2.QR_UnpackFactors( q2, r2, c, d );
	m2 = q2 * r2;

	// update factored m1
	q1.QR_UpdateIncrement( r1, v, w );
	m1 = q1 * r1;

	if ( !m1.Compare( m2, 1e-4f ) ) {
		Common::Com_Printf( "MatX::QR_UpdateIncrement failed" );
	}

	/*
		MatX::QR_UpdateDecrement
	*/

	for ( offset = 0; offset < size; offset++ ) {
		c.SetSize( size+1 );
		d.SetSize( size+1 );

		m1 = original;
		m2 = original;

		v.SetSize( 6 );
		w.SetSize( 6 );
		for ( int i = 0; i < size; i++ ) {
			v[i] = original[i][offset];
			w[i] = original[offset][i];
		}

		// factor m1
		m1.QR_Factor( c, d );
		m1.QR_UnpackFactors( q1, r1, c, d );

		// modify and factor m2
		m2.Update_Decrement( offset );
		if ( !m2.QR_Factor( c, d ) ) {
			assert( 0 );
		}
		m2.QR_UnpackFactors( q2, r2, c, d );
		m2 = q2 * r2;

		// update factors of m1
		q1.QR_UpdateDecrement( r1, v, w, offset );
		m1 = q1 * r1;

		if ( !m1.Compare( m2, 1e-3f ) ) {
			Common::Com_Printf( "MatX::QR_UpdateDecrement failed" );
		}
	}

	/*
		MatX::QR_Inverse
	*/

	m2 = original;

	m2.QR_Factor( c, d );
	m2.QR_Inverse( m1, c, d );
	m1 *= original;

	if ( !m1.IsIdentity( 1e-4f ) ) {
		Common::Com_Printf( "MatX::QR_Inverse failed" );
	}

	/*
		MatX::SVD_Factor
	*/

	m1 = original;
	m3.Zero( size, size );
	w.Zero( size );

	m1.SVD_Factor( w, m3 );
	m2.Diag( w );
	m3.TransposeSelf();
	m1 = m1 * m2 * m3;

	if ( !original.Compare( m1, 1e-4f ) ) {
		Common::Com_Printf( "MatX::SVD_Factor failed" );
	}

	/*
		MatX::SVD_Inverse
	*/

	m2 = original;

	m2.SVD_Factor( w, m3 );
	m2.SVD_Inverse( m1, w, m3 );
	m1 *= original;

	if ( !m1.IsIdentity( 1e-4f ) ) {
		Common::Com_Printf( "MatX::SVD_Inverse failed" );
	}

	/*
		MatX::Cholesky_Factor
	*/

	m1 = original;

	m1.Cholesky_Factor();
	m1.Cholesky_MultiplyFactors( m2 );

	if ( !original.Compare( m2, 1e-4f ) ) {
		Common::Com_Printf( "MatX::Cholesky_Factor failed" );
	}

	/*
		MatX::Cholesky_UpdateRankOne
	*/

	m1 = original;
	m2 = original;

	w.Random( size, 0 );

	// factor m1
	m1.Cholesky_Factor();
	m1.ClearUpperTriangle();

	// modify and factor m2 
	m2.Update_RankOneSymmetric( w, 1.0f );
	if ( !m2.Cholesky_Factor() ) {
		assert( 0 );
	}
	m2.ClearUpperTriangle();

	// update factored m1
	m1.Cholesky_UpdateRankOne( w, 1.0f, 0 );

	if ( !m1.Compare( m2, 1e-4f ) ) {
		Common::Com_Printf( "MatX::Cholesky_UpdateRankOne failed" );
	}

	/*
		MatX::Cholesky_UpdateRowColumn
	*/

	for ( offset = 0; offset < size; offset++ ) {
		m1 = original;
		m2 = original;

		// factor m1
		m1.Cholesky_Factor();
		m1.ClearUpperTriangle();

		int pdtable[] = { 1, 0, 1, 0, 0, 0 };
		w.Random( size, pdtable[offset] );
		w *= 0.1f;

		// modify and factor m2
		m2.Update_RowColumnSymmetric( w, offset );
		if ( !m2.Cholesky_Factor() ) {
			assert( 0 );
		}
		m2.ClearUpperTriangle();

		// update m1
		m1.Cholesky_UpdateRowColumn( w, offset );

		if ( !m1.Compare( m2, 1e-3f ) ) {
			Common::Com_Printf( "MatX::Cholesky_UpdateRowColumn failed" );
		}
	}

	/*
		MatX::Cholesky_UpdateIncrement
	*/

	m1.Random( size + 1, size + 1, 0 );
	m3 = m1 * m1.Transpose();

	m1.SquareSubMatrix( m3, size );
	m2 = m1;

	w.SetSize( size + 1 );
	for ( int i = 0; i < size + 1; i++ ) {
		w[i] = m3[size][i];
	}

	// factor m1
	m1.Cholesky_Factor();

	// modify and factor m2 
	m2.Update_IncrementSymmetric( w );
	if ( !m2.Cholesky_Factor() ) {
		assert( 0 );
	}

	// update factored m1
	m1.Cholesky_UpdateIncrement( w );

	m1.ClearUpperTriangle();
	m2.ClearUpperTriangle();

	if ( !m1.Compare( m2, 1e-4f ) ) {
		Common::Com_Printf( "MatX::Cholesky_UpdateIncrement failed" );
	}

	/*
		MatX::Cholesky_UpdateDecrement
	*/

	for ( offset = 0; offset < size; offset += size - 1 ) {
		m1 = original;
		m2 = original;

		v.SetSize( 6 );
		for ( int i = 0; i < size; i++ ) {
			v[i] = original[i][offset];
		}

		// factor m1
		m1.Cholesky_Factor();

		// modify and factor m2
		m2.Update_Decrement( offset );
		if ( !m2.Cholesky_Factor() ) {
			assert( 0 );
		}

		// update factors of m1
		m1.Cholesky_UpdateDecrement( v, offset );

		if ( !m1.Compare( m2, 1e-3f ) ) {
			Common::Com_Printf( "MatX::Cholesky_UpdateDecrement failed" );
		}
	}

	/*
		MatX::Cholesky_Inverse
	*/

	m2 = original;

	m2.Cholesky_Factor();
	m2.Cholesky_Inverse( m1 );
	m1 *= original;

	if ( !m1.IsIdentity( 1e-4f ) ) {
		Common::Com_Printf( "MatX::Cholesky_Inverse failed" );
	}

	/*
		MatX::LDLT_Factor
	*/

	m1 = original;

	m1.LDLT_Factor();
	m1.LDLT_MultiplyFactors( m2 );

	if ( !original.Compare( m2, 1e-4f ) ) {
		Common::Com_Printf( "MatX::LDLT_Factor failed" );
	}

	m1.LDLT_UnpackFactors( m2, m3 );
	m2 = m2 * m3 * m2.Transpose();

	if ( !original.Compare( m2, 1e-4f ) ) {
		Common::Com_Printf( "MatX::LDLT_Factor failed" );
	}

	/*
		MatX::LDLT_UpdateRankOne
	*/

	m1 = original;
	m2 = original;

	w.Random( size, 0 );

	// factor m1
	m1.LDLT_Factor();
	m1.ClearUpperTriangle();

	// modify and factor m2 
	m2.Update_RankOneSymmetric( w, 1.0f );
	if ( !m2.LDLT_Factor() ) {
		assert( 0 );
	}
	m2.ClearUpperTriangle();

	// update factored m1
	m1.LDLT_UpdateRankOne( w, 1.0f, 0 );

	if ( !m1.Compare( m2, 1e-4f ) ) {
		Common::Com_Printf( "MatX::LDLT_UpdateRankOne failed" );
	}

	/*
		MatX::LDLT_UpdateRowColumn
	*/

	for ( offset = 0; offset < size; offset++ ) {
		m1 = original;
		m2 = original;

		w.Random( size, 0 );

		// factor m1
		m1.LDLT_Factor();
		m1.ClearUpperTriangle();

		// modify and factor m2
		m2.Update_RowColumnSymmetric( w, offset );
		if ( !m2.LDLT_Factor() ) {
			assert( 0 );
		}
		m2.ClearUpperTriangle();

		// update m1
		m1.LDLT_UpdateRowColumn( w, offset );

		if ( !m1.Compare( m2, 1e-3f ) ) {
			Common::Com_Printf( "MatX::LDLT_UpdateRowColumn failed" );
		}
	}

	/*
		MatX::LDLT_UpdateIncrement
	*/

	m1.Random( size + 1, size + 1, 0 );
	m3 = m1 * m1.Transpose();

	m1.SquareSubMatrix( m3, size );
	m2 = m1;

	w.SetSize( size + 1 );
	for ( int i = 0; i < size + 1; i++ ) {
		w[i] = m3[size][i];
	}

	// factor m1
	m1.LDLT_Factor();

	// modify and factor m2 
	m2.Update_IncrementSymmetric( w );
	if ( !m2.LDLT_Factor() ) {
		assert( 0 );
	}

	// update factored m1
	m1.LDLT_UpdateIncrement( w );

	m1.ClearUpperTriangle();
	m2.ClearUpperTriangle();

	if ( !m1.Compare( m2, 1e-4f ) ) {
		Common::Com_Printf( "MatX::LDLT_UpdateIncrement failed" );
	}

	/*
		MatX::LDLT_UpdateDecrement
	*/

	for ( offset = 0; offset < size; offset++ ) {
		m1 = original;
		m2 = original;

		v.SetSize( 6 );
		for ( int i = 0; i < size; i++ ) {
			v[i] = original[i][offset];
		}

		// factor m1
		m1.LDLT_Factor();

		// modify and factor m2
		m2.Update_Decrement( offset );
		if ( !m2.LDLT_Factor() ) {
			assert( 0 );
		}

		// update factors of m1
		m1.LDLT_UpdateDecrement( v, offset );

		if ( !m1.Compare( m2, 1e-3f ) ) {
			Common::Com_Printf( "MatX::LDLT_UpdateDecrement failed" );
		}
	}

	/*
		MatX::LDLT_Inverse
	*/

	m2 = original;

	m2.LDLT_Factor();
	m2.LDLT_Inverse( m1 );
	m1 *= original;

	if ( !m1.IsIdentity( 1e-4f ) ) {
		Common::Com_Printf( "MatX::LDLT_Inverse failed" );
	}

	/*
		MatX::Eigen_SolveSymmetricTriDiagonal
	*/

	m3 = original;
	m3.TriDiagonal_ClearTriangles();
	m1 = m3;

	v.SetSize( size );

	m1.Eigen_SolveSymmetricTriDiagonal( v );

	m3.TransposeMultiply( m2, m1 );

	for ( int i = 0; i < size; i++ ) {
		for ( int j = 0; j < size; j++ ) {
			m1[i][j] *= v[j];
		}
	}

	if ( !m1.Compare( m2, 1e-4f ) ) {
		Common::Com_Printf( "MatX::Eigen_SolveSymmetricTriDiagonal failed" );
	}

	/*
		MatX::Eigen_SolveSymmetric
	*/

	m3 = original;
	m1 = m3;

	v.SetSize( size );

	m1.Eigen_SolveSymmetric( v );

	m3.TransposeMultiply( m2, m1 );

	for ( int i = 0; i < size; i++ ) {
		for ( int j = 0; j < size; j++ ) {
			m1[i][j] *= v[j];
		}
	}

	if ( !m1.Compare( m2, 1e-4f ) ) {
		Common::Com_Printf( "MatX::Eigen_SolveSymmetric failed" );
	}

	/*
		MatX::Eigen_Solve
	*/

	m3 = original;
	m1 = m3;

	v.SetSize( size );
	w.SetSize( size );

	m1.Eigen_Solve( v, w );

	m3.TransposeMultiply( m2, m1 );

	for ( int i = 0; i < size; i++ ) {
		for ( int j = 0; j < size; j++ ) {
			m1[i][j] *= v[j];
		}
	}

	if ( !m1.Compare( m2, 1e-4f ) ) {
		Common::Com_Printf( "MatX::Eigen_Solve failed" );
	}
}

// Projection

Projection & Projection::OrthogonalSelf( float in_left, float in_right, float in_bottom, float in_top, float in_near, float in_far ) {

	float subRL		= in_right - in_left;
	float subTB		= in_top - in_bottom;
	float subFN		= in_far - in_near;

	//mat[ 0 ].Set( 2.0f / subRL,						0.0f,								0.0f,							0.0f );
	//mat[ 1 ].Set( 0.0f,								2.0f / subTB,						0.0f,							0.0f );
	//mat[ 2 ].Set( 0.0f,								0.0f,								2.0f / subFN,					0.0f );
	//mat[ 3 ].Set( -( in_right + in_left ) / subRL,	-( in_top + in_bottom ) / subTB,	-( in_far + in_near ) / subFN,	1.0f );

	mat[ 0 ].Set( 2.0f / subRL,		0.0f,				0.0f,			-( in_right + in_left ) / subRL );
	mat[ 1 ].Set( 0.0f,				2.0f / subTB,		0.0f,			-( in_top + in_bottom ) / subTB );
	mat[ 2 ].Set( 0.0f,				0.0f,				2.0f / subFN,	-( in_far + in_near ) / subFN );
	mat[ 3 ].Set( 0.0f,				0.0f,				0.0f,			1.0f );

	d_near = in_near;
	d_far = in_far;

	return *this;
}

Projection & Projection::PerspectiveSelf( float in_near, float in_far, float in_fov_y, float in_aspect ) {

	float top		= tan( DEG2RAD( in_fov_y / 2.0f ) ) * in_near;
	float bottom	= -top;
	float left		= -top * in_aspect;
	float right		= top * in_aspect;

	float subRL		= right - left;
	float subTB		= top - bottom;
	float subFN		= in_far - in_near;
	float twiceNear	= 2.0f * in_near;

	mat[ 0 ].Set( twiceNear / subRL,	0.0f,					( right + left ) / subRL /* frustum */,		0.0f );
	mat[ 1 ].Set( 0.0f,					twiceNear / subTB,		( top + bottom ) / subTB /* frustum */,		0.0f );
	mat[ 2 ].Set( 0.0f,					0.0f,					-( in_far + in_near ) / subFN,				-( twiceNear * in_far ) / subFN );
	mat[ 3 ].Set( 0.0f,					0.0f,					-1.0f,										1.0f );

	d_near = in_near;
	d_far = in_far;

	return *this;
}

Projection & Projection::OrthoLightSelf( float in_dist ) {

	float subDist		= in_dist - 1;

	mat[ 0 ].Set( 1.0f,		0.0f,	0.0f,							0.0f );
	mat[ 1 ].Set( 0.0f,		1.0f,	0.0f,							0.0f );
	mat[ 2 ].Set( 0.0f,		0.0f,	-( in_dist + 1.0f ) / subDist,	-( 2.0f * in_dist ) / subDist );
	mat[ 3 ].Set( 0.0f,		0.0f,	-1.0f,							1.0f );

	d_near = 1.0f;
	d_far = in_dist;

	return *this;
}

void Projection::ToInvPlanes( const Bounds2D & scissor, Plane planes[ 6 ] ) const {

	float twiceNear	= 2.0f * d_near;
	float twiceNearAndOne = twiceNear + 1.0f;
	float farAndOne = d_far + 1.0f;
	Vec4 dP[ 8 ] = {
		Vec4( scissor[0].d_x * twiceNear,	scissor[0].d_y * twiceNear,	0, twiceNearAndOne ),
		Vec4( scissor[1].d_x * twiceNear,	scissor[1].d_y * twiceNear,	0, twiceNearAndOne ),
		Vec4( scissor[0].d_x * twiceNear,	scissor[1].d_y * twiceNear,	0, twiceNearAndOne ),
		Vec4( scissor[1].d_x * twiceNear,	scissor[0].d_y * twiceNear,	0, twiceNearAndOne ),
		Vec4( scissor[0].d_x * d_far,		scissor[0].d_y * d_far,		d_far, farAndOne ),
		Vec4( scissor[1].d_x * d_far,		scissor[1].d_y * d_far,		d_far, farAndOne ),
		Vec4( scissor[0].d_x * d_far,		scissor[1].d_y * d_far,		d_far, farAndOne ),
		Vec4( scissor[1].d_x * d_far,		scissor[0].d_y * d_far,		d_far, farAndOne )
	};
	InvProjectVectorArray( dP, dP, 8 );

	planes[0].FromPoints( Vec3( dP[2] ), Vec3( dP[0] ), Vec3( dP[6] ) );
	planes[1].FromPoints( Vec3( dP[3] ), Vec3( dP[1] ), Vec3( dP[7] ) );

	planes[2].FromPoints( Vec3( dP[0] ), Vec3( dP[3] ), Vec3( dP[4] ) );
	planes[3].FromPoints( Vec3( dP[1] ), Vec3( dP[2] ), Vec3( dP[5] ) );

	planes[4].FromPoints( Vec3( dP[1] ), Vec3( dP[0] ), Vec3( dP[2] ) );
	planes[5].FromPoints( Vec3( dP[4] ), Vec3( dP[5] ), Vec3( dP[6] ) );
}
