#if 0
// plane types are used to speed some tests
// 0-2 are axial planes
typedef enum {

	PLANE_X,
	PLANE_Y,
	PLANE_Z,
	PLANE_NON_AXIAL

} planeType_e;
#endif

INLINE float RSqrt( float x ) {
	long i	= 0x5f3759df - ( *reinterpret_cast<long *>( &x ) >> 1 );
	float r	= *reinterpret_cast<float *>( &i );
	return r * ( 1.5f - r * r * ( x * 0.5f ) );
}

INLINE float InvSqrt( float x ) {
	return 1.0f / sqrtf( x );
}

INLINE void SinCos( float a, float & s, float & c ) {
	s = sinf( a );
	c = cosf( a );
}

INLINE float ACos( float a ) {
	if( a <= -1.0f )
		return M_PI;
	if( a >= 1.0f )
		return 0.0f;
	return acosf( a );
}

INLINE float Sin16( float a ) {
	float s;
	if( ( a < 0.0f ) || ( a >= TWO_PI ) )
		a -= floorf( a / TWO_PI ) * TWO_PI;
	if( a < M_PI ) {
		if( a > HALF_PI )
			a = M_PI - a;
	} else {
		if( a > M_PI + HALF_PI )
			a = a - TWO_PI;
		else
			a = M_PI - a;
	}
	s = a * a;
	return a * ( ( ( ( ( -2.39e-08f * s + 2.7526e-06f ) * s - 1.98409e-04f ) * s + 8.3333315e-03f ) * s - 1.666666664e-01f ) * s + 1.0f );
}

INLINE float ATan16( float a ) {
	float s;
	if( fabs( a ) > 1.0f ) {
		a = 1.0f / a;
		s = a * a;
		s = - ( ( ( ( ( ( ( ( ( 0.0028662257f * s - 0.0161657367f ) * s + 0.0429096138f ) * s - 0.0752896400f )
				* s + 0.1065626393f ) * s - 0.1420889944f ) * s + 0.1999355085f ) * s - 0.3333314528f ) * s ) + 1.0f ) * a;
		if( FLOATSIGNBITSET( a ) )
			return s - HALF_PI;
		else
			return s + HALF_PI;
	} else {
		s = a * a;
		return ( ( ( ( ( ( ( ( ( 0.0028662257f * s - 0.0161657367f ) * s + 0.0429096138f ) * s - 0.0752896400f )
			* s + 0.1065626393f ) * s - 0.1420889944f ) * s + 0.1999355085f ) * s - 0.3333314528f ) * s ) + 1.0f ) * a;
	}
}

INLINE float ATan16( float y, float x ) {
	float a, s;
	if( fabs( y ) > fabs( x ) ) {
		a = x / y;
		s = a * a;
		s = - ( ( ( ( ( ( ( ( ( 0.0028662257f * s - 0.0161657367f ) * s + 0.0429096138f ) * s - 0.0752896400f )
				* s + 0.1065626393f ) * s - 0.1420889944f ) * s + 0.1999355085f ) * s - 0.3333314528f ) * s ) + 1.0f ) * a;
		if( FLOATSIGNBITSET( a ) )
			return s - HALF_PI;
		else
			return s + HALF_PI;
	} else {
		a = y / x;
		s = a * a;
		return ( ( ( ( ( ( ( ( ( 0.0028662257f * s - 0.0161657367f ) * s + 0.0429096138f ) * s - 0.0752896400f )
			* s + 0.1065626393f ) * s - 0.1420889944f ) * s + 0.1999355085f ) * s - 0.3333314528f ) * s ) + 1.0f ) * a;
	}
}

INLINE float Tan16( float a ) {
	float s;
	bool reciprocal;
	if( ( a < 0.0f ) || ( a >= M_PI ) )
		a -= floorf( a / M_PI ) * M_PI;
	if( a < HALF_PI ) {
		if( a > ONEFOURTH_PI ) {
			a = HALF_PI - a;
			reciprocal = true;
		} else
			reciprocal = false;
	} else {
		if( a > HALF_PI + ONEFOURTH_PI ) {
			a = a - M_PI;
			reciprocal = false;
		} else {
			a = HALF_PI - a;
			reciprocal = true;
		}
	}
	s = a * a;
	s = a * ( ( ( ( ( ( 9.5168091e-03f * s + 2.900525e-03f ) * s + 2.45650893e-02f ) * s + 5.33740603e-02f ) * s + 1.333923995e-01f ) * s + 3.333314036e-01f ) * s + 1.0f );
	if( reciprocal )
		return 1.0f / s;
	else
		return s;
}

INLINE float Rint( float f ) {
	return floorf( f + 0.5f );
}

INLINE int IPow( int x, int y ) {
	int r; for( r = x; y > 1; y-- ) { r *= x; } return r;
}

// // //// // //// // //// //
/// Vec2
// //// // //// // //// //

class Vec2 {
public:
	float					d_x;
	float					d_y;

							Vec2( );
							explicit Vec2( const float x, const float y );
	void 					Set( const float x, const float y );
	void					Zero( );
	float					operator[ ]( int index ) const;
	float &					operator[ ]( int index );
	Vec2					operator -( ) const;
	float					operator *( const Vec2 & a ) const;
	Vec2					operator *( const float a ) const;
	Vec2					operator /( const float a ) const;
	Vec2					operator +( const Vec2 & a ) const;
	Vec2					operator -( const Vec2 & a ) const;
	Vec2 &					operator +=( const Vec2 & a );
	Vec2 &					operator -=( const Vec2 & a );
	Vec2 &					operator /=( const Vec2 & a );
	Vec2 &					operator /=( const float a );
	Vec2 &					operator *=( const float a );
	friend Vec2				operator *( const float a, const Vec2 b );
	/// exact compare, no epsilon
	bool					Compare( const Vec2 & a ) const;
	/// compare with epsilon
	bool					Compare( const Vec2 & a, const float epsilon ) const;
	/// exact compare, no epsilon
	bool					operator ==( const Vec2 & a ) const;
	/// exact compare, no epsilon
	bool					operator !=( const Vec2 & a ) const;
	float					Length( ) const;
	float					LengthFast( ) const;
	float					LengthSqr( ) const;
	/// returns length
	float					Normalize( );
	/// returns length
	float					NormalizeFast( );
	/// cap length
	Vec2 &					Truncate( float length );
	void					Clamp( const Vec2 & min, const Vec2 & max );
	/// snap to closest integer value
	void					Snap( );
	/// snap towards integer (floor)
	void					SnapInt( );
	void					Ceil( );
	int						GetDimension( ) const;
	const float *			ToFloatPtr( ) const;
	float *					ToFloatPtr( );
	const Str				ToString( int precision = 2 ) const;
	void					Lerp( const Vec2 & v1, const Vec2 & v2, const float l );
							operator VecT2f( ) const;
							operator VecT2hf( ) const;
};

extern const Vec2 vec2_origin;

class Vec4;
class Angles;

extern Vec3 vec3_origin;

//===============================================================
//
//	Vec4 - 4D vector
//
//===============================================================

class Vec4 {

public:

	float			x;
	float			y;
	float			z;
	float			w;

					Vec4( );
					explicit Vec4( const Vec3 & vec, const float w = 1.0f );
					explicit Vec4( const float x, const float y, const float z, const float w = 1.0f );

	void 			Set( const float x, const float y, const float z, const float w = 1.0f );
	void			Zero( );

	Vec4 &			operator =( const Vec3 & a );		// required because of a msvc 6 & 7 bug
	float			operator [ ]( const int index ) const;
	float &			operator [ ]( const int index );
	Vec4			operator - ( ) const;
	float			operator *( const Vec4 & a ) const;
	Vec4			operator *( const float a ) const;
	Vec4			operator /( const float a ) const;
	Vec4			operator +( const Vec4 & a ) const;
	Vec4			operator -( const Vec4 & a ) const;
	Vec4 &			operator +=( const Vec4 & a );
	Vec4 &			operator -=( const Vec4 & a );
	Vec4 &			operator /=( const Vec4 & a );
	Vec4 &			operator /=( const float a );
	Vec4 &			operator *=( const float a );

	friend Vec4	operator*( const float a, const Vec4 b );

	int				GetSignBits( ) const;

	bool			Compare( const Vec4 & a ) const;							// exact compare, no epsilon
	bool			Compare( const Vec4 & a, const float epsilon ) const;		// compare with epsilon
	bool			operator ==(	const Vec4 & a ) const;						// exact compare, no epsilon
	bool			operator !=(	const Vec4 & a ) const;						// exact compare, no epsilon

	float			Length( ) const;
	float			LengthSqr( ) const;
	float			Normalize( );			// returns length
	float			NormalizeFast( );		// returns length

	int				GetDimension( ) const;

	const Vec3 &	ToVec3( ) const;
	Vec3 &			ToVec3( );
	const float *	ToFloatPtr( ) const;
	float *			ToFloatPtr( );
	const Str		ToString( int precision = 2 ) const;

	void			Lerp( const Vec4 & v1, const Vec4 & v2, const float l );
};

extern Vec4 vec4_origin;
#define vec4_zero vec4_origin

//===============================================================
//
//	Vec5 - 5D vector
//
//===============================================================

class Vec5 {
public:
	float			x;
	float			y;
	float			z;
	float			s;
	float			t;

					Vec5( );
					explicit Vec5( const float x, const float y, const float z, const float s, const float t );

	float			operator[]( int index ) const;
	float &			operator[]( int index );
	Vec5 &			operator=( const Vec3 & a );

	int				GetDimension( ) const;

	const Vec3 &	ToVec3( ) const;
	Vec3 &			ToVec3( );
	const float *	ToFloatPtr( ) const;
	float *			ToFloatPtr( );
	const char *	ToString( int precision = 2 ) const;

	void			Lerp( const Vec5 & v1, const Vec5 & v2, const float l );
};

extern Vec5 vec5_origin;
#define vec5_zero vec5_origin

//===============================================================
//
//	Vec6 - 6D vector
//
//===============================================================

class Vec6 {
public:	
					Vec6( );
					explicit Vec6( const float *a );
					explicit Vec6( const float a1, const float a2, const float a3, const float a4, const float a5, const float a6 );

	void 			Set( const float a1, const float a2, const float a3, const float a4, const float a5, const float a6 );
	void			Zero( );

	float			operator[]( const int index ) const;
	float &			operator[]( const int index );
	Vec6			operator-() const;
	Vec6			operator*( const float a ) const;
	Vec6			operator/( const float a ) const;
	float			operator*( const Vec6 & a ) const;
	Vec6			operator-( const Vec6 & a ) const;
	Vec6			operator+( const Vec6 & a ) const;
	Vec6 &			operator*=( const float a );
	Vec6 &			operator/=( const float a );
	Vec6 &			operator+=( const Vec6 & a );
	Vec6 &			operator-=( const Vec6 & a );

	friend Vec6		operator*( const float a, const Vec6 b );

	bool			Compare( const Vec6 & a ) const;							// exact compare, no epsilon
	bool			Compare( const Vec6 & a, const float epsilon ) const;		// compare with epsilon
	bool			operator==(	const Vec6 & a ) const;						// exact compare, no epsilon
	bool			operator!=(	const Vec6 & a ) const;						// exact compare, no epsilon

	float			Length( ) const;
	float			LengthSqr( ) const;
	float			Normalize( );			// returns length
	float			NormalizeFast( );		// returns length

	int				GetDimension( ) const;

	const Vec3 &	SubVec3( int index ) const;
	Vec3 &		SubVec3( int index );
	const float *	ToFloatPtr( ) const;
	float *			ToFloatPtr( );
	const char *	ToString( int precision = 2 ) const;

private:
	float			p[6];
};

extern Vec6 vec6_origin;
#define vec6_zero vec6_origin
extern Vec6 vec6_infinity;

//===============================================================
//
//	VecX - arbitrary sized vector
//
//  The vector lives on 16 byte aligned and 16 byte padded memory.
//
//	NOTE: due to the temporary memory pool VecX cannot be used by multiple threads
//
//===============================================================

#define VECX_MAX_TEMP		1024
#define VECX_QUAD( x )		( ( ( ( x ) + 3 ) & ~3 ) * sizeof( float ) )
#define VECX_CLEAREND()		int s = size; while( s < ( ( s + 3) & ~3 ) ) { p[s++] = 0.0f; }
#define VECX_ALLOCA( n )	( (float *) _alloca16( VECX_QUAD( n ) ) )
//#define VECX_SIMD

class VecX {
	friend class MatX;

public:	
					VecX( );
					explicit VecX( int length );
					explicit VecX( int length, float *data );
					~VecX( );

	float			operator[]( const int index ) const;
	float &			operator[]( const int index );
	VecX			operator-() const;
	VecX &			operator=( const VecX &a );
	VecX			operator*( const float a ) const;
	VecX			operator/( const float a ) const;
	float			operator*( const VecX &a ) const;
	VecX			operator-( const VecX &a ) const;
	VecX			operator+( const VecX &a ) const;
	VecX &			operator*=( const float a );
	VecX &			operator/=( const float a );
	VecX &			operator+=( const VecX &a );
	VecX &			operator-=( const VecX &a );

	friend VecX		operator*( const float a, const VecX b );

	bool			Compare( const VecX &a ) const;							// exact compare, no epsilon
	bool			Compare( const VecX &a, const float epsilon ) const;		// compare with epsilon
	bool			operator==(	const VecX &a ) const;						// exact compare, no epsilon
	bool			operator!=(	const VecX &a ) const;						// exact compare, no epsilon

	void			SetSize( int size );
	void			ChangeSize( int size, bool makeZero = false );
	int				GetSize( ) const { return size; }
	void			SetData( int length, float *data );
	void			Zero( );
	void			Zero( int length );
	void			Random( int seed, float l = 0.0f, float u = 1.0f );
	void			Random( int length, int seed, float l = 0.0f, float u = 1.0f );
	void			Negate( );
	void			Clamp( float min, float max );
	VecX &			SwapElements( int e1, int e2 );

	float			Length( ) const;
	float			LengthSqr( ) const;
	VecX			Normalize( ) const;
	float			NormalizeSelf( );

	int				GetDimension( ) const;

	const Vec3 &	SubVec3( int index ) const;
	Vec3 &			SubVec3( int index );
	const Vec6 &	SubVec6( int index ) const;
	Vec6 &			SubVec6( int index );
	const float *	ToFloatPtr( ) const;
	float *			ToFloatPtr( );
	const char *	ToString( int precision = 2 ) const;

private:
	int				size;					// size of the vector
	int				alloced;				// if -1 p points to data set with SetData
	float *			p;						// memory the vector is stored

	static float	temp[VECX_MAX_TEMP+4];	// used to store intermediate results
	static float *	tempPtr;				// pointer to 16 byte aligned temporary memory
	static int		tempIndex;				// index into memory pool, wraps around

private:
	void			SetTempSize( int size );
};

// // //// // //// // //// //
// Vec2
// //// // //// // //// //

INLINE Vec2::Vec2( ) {
}

INLINE Vec2::Vec2( const float x, const float y ) {
	d_x = x;
	d_y = y;
}

INLINE void Vec2::Set( const float x, const float y ) {
	d_x = x;
	d_y = y;
}

INLINE void Vec2::Zero( ) {
	d_x = d_y = 0.0f;
}

INLINE bool Vec2::Compare( const Vec2 & a ) const {
	return ( ( d_x == a.d_x ) && ( d_y == a.d_y ) );
}

INLINE bool Vec2::Compare( const Vec2 & a, const float epsilon ) const {
	if( fabsf( d_x - a.d_x ) > epsilon )
		return false;			
	if( fabsf( d_y - a.d_y ) > epsilon )
		return false;
	return true;
}

INLINE bool Vec2::operator==( const Vec2 & a ) const {
	return Compare( a );
}

INLINE bool Vec2::operator!=( const Vec2 & a ) const {
	return !Compare( a );
}

INLINE float Vec2::operator[ ]( int index ) const {
	return ( &d_x )[ index ];
}

INLINE float & Vec2::operator[ ]( int index ) {
	return ( &d_x )[ index ];
}

INLINE float Vec2::Length( ) const {
	return ( float )sqrtf( d_x * d_x + d_y * d_y );
}

INLINE float Vec2::LengthFast( ) const {
	float sqrLength = d_x * d_x + d_y * d_y;
	return sqrLength * RSqrt( sqrLength );
}

INLINE float Vec2::LengthSqr( ) const {
	return ( d_x * d_x + d_y * d_y );
}

INLINE float Vec2::Normalize( ) {
	float sqrLength = d_x * d_x + d_y * d_y;
	float invLength = 1.0f / sqrtf( sqrLength );
	d_x *= invLength;
	d_y *= invLength;
	return invLength * sqrLength;
}

INLINE float Vec2::NormalizeFast( ) {
	float lengthSqr = d_x * d_x + d_y * d_y;
	float invLength = RSqrt( lengthSqr );
	d_x *= invLength;
	d_y *= invLength;
	return invLength * lengthSqr;
}

INLINE Vec2 & Vec2::Truncate( float length ) {
	if( !length )
		Zero( );
	else {
		float length2 = LengthSqr();
		if( length2 > length * length ) {
			float ilength = length * 1.0f / sqrtf( length2 );
			d_x *= ilength;
			d_y *= ilength;
		}
	}
	return *this;
}

INLINE void Vec2::Clamp( const Vec2 & min, const Vec2 & max ) {
	if( d_x < min.d_x )
		d_x = min.d_x;
	else if( d_x > max.d_x )
		d_x = max.d_x;
	if( d_y < min.d_y )
		d_y = min.d_y;
	else if( d_y > max.d_y )
		d_y = max.d_y;
}

INLINE void Vec2::Snap( ) {
	d_x = floorf( d_x + 0.5f );
	d_y = floorf( d_y + 0.5f );
}

INLINE void Vec2::SnapInt( ) {
	d_x = float( int( d_x ) );
	d_y = float( int( d_y ) );
}

INLINE void Vec2::Ceil( ) {
	d_x = ceilf( d_x );
	d_y = ceilf( d_y );
}

INLINE Vec2 Vec2::operator-( ) const {
	return Vec2( -d_x, -d_y );
}
	
INLINE Vec2 Vec2::operator-( const Vec2 & a ) const {
	return Vec2( d_x - a.d_x, d_y - a.d_y );
}

INLINE float Vec2::operator*( const Vec2 & a ) const {
	return d_x * a.d_x + d_y * a.d_y;
}

INLINE Vec2 Vec2::operator*( const float a ) const {
	return Vec2( d_x * a, d_y * a );
}

INLINE Vec2 Vec2::operator/( const float a ) const {
	float inva = 1.0f / a;
	return Vec2( d_x * inva, d_y * inva );
}

INLINE Vec2 operator*( const float a, const Vec2 b ) {
	return Vec2( b.d_x * a, b.d_y * a );
}

INLINE Vec2 Vec2::operator+( const Vec2 & a ) const {
	return Vec2( d_x + a.d_x, d_y + a.d_y );
}

INLINE Vec2 & Vec2::operator+=( const Vec2 & a ) {
	d_x += a.d_x;
	d_y += a.d_y;
	return *this;
}

INLINE Vec2 & Vec2::operator/=( const Vec2 & a ) {
	d_x /= a.d_x;
	d_y /= a.d_y;
	return *this;
}

INLINE Vec2 & Vec2::operator/=( const float a ) {
	float inva = 1.0f / a;
	d_x *= inva;
	d_y *= inva;
	return *this;
}

INLINE Vec2 & Vec2::operator-=( const Vec2 & a ) {
	d_x -= a.d_x;
	d_y -= a.d_y;
	return *this;
}

INLINE Vec2 & Vec2::operator*=( const float a ) {
	d_x *= a;
	d_y *= a;
	return *this;
}

INLINE int Vec2::GetDimension( ) const {
	return 2;
}

INLINE const float *Vec2::ToFloatPtr( ) const {
	return &d_x;
}

INLINE float *Vec2::ToFloatPtr( ) {
	return &d_x;
}

INLINE void Vec2::Lerp( const Vec2 & v1, const Vec2 & v2, const float l ) {
	if( l <= 0.0f )
		( *this ) = v1;
	else if( l >= 1.0f )
		( *this ) = v2;
	else
		( *this ) = v1 + l * ( v2 - v1 );
}

// // //// // //// // //// //
// Vec3
// //// // //// // //// //

INLINE Vec3::Vec3( ) {
}

INLINE Vec3::Vec3( const Vec4 & vec ) {
	Common::Com_Memcpy( this, &vec, NumSizeOf( float, GetDimension( ) ) );
	//*this /= vec.w;
}

INLINE Vec3::Vec3( const float x, const float y, const float z ) {
	this->x = x;
	this->y = y;
	this->z = z;
}

INLINE bool Vec3::FixDegenerateNormal( ) {

	if( x == 0.0f ) {
		if( y == 0.0f ) {
			if( z > 0.0f ) {
				if( z != 1.0f ) {
					z = 1.0f;
					return true;
				}
			} else {
				if( z != -1.0f ) {
					z = -1.0f;
					return true;
				}
			}
			return false;
		} else if( z == 0.0f ) {
			if( y > 0.0f ) {
				if( y != 1.0f ) {
					y = 1.0f;
					return true;
				}
			} else {
				if( y != -1.0f ) {
					y = -1.0f;
					return true;
				}
			}
			return false;
		}
	} else if( y == 0.0f ) {
		if( z == 0.0f ) {
			if( x > 0.0f ) {
				if( x != 1.0f ) {
					x = 1.0f;
					return true;
				}
			} else {
				if( x != -1.0f ) {
					x = -1.0f;
					return true;
				}
			}
			return false;
		}
	}
	if( fabs( x ) == 1.0f ) {
		if( y != 0.0f || z != 0.0f ) {
			y = z = 0.0f;
			return true;
		}
		return false;
	} else if( fabs( y ) == 1.0f ) {
		if( x != 0.0f || z != 0.0f ) {
			x = z = 0.0f;
			return true;
		}
		return false;
	} else if( fabs( z ) == 1.0f ) {
		if( x != 0.0f || y != 0.0f ) {
			x = y = 0.0f;
			return true;
		}
		return false;
	}
	return false;
}

INLINE float Vec3::operator[]( const int index ) const {
	return( &x )[ index ];
}

INLINE float &Vec3::operator[]( const int index ) {
	return( &x )[ index ];
}

INLINE void Vec3::Set( const float x, const float y, const float z ) {
	this->x = x;
	this->y = y;
	this->z = z;
}

INLINE void Vec3::Zero( ) {
	x = y = z = 0.0f;
}

#if 0
INLINE planeType_e Vec3::GetPlaneType( ) const {

	if( x == 1.0f ) return PLANE_X;
	if( y == 1.0f ) return PLANE_Y;
	if( z == 1.0f ) return PLANE_Z;

	return PLANE_NON_AXIAL;
}
#endif

INLINE int Vec4::GetSignBits( ) const {

	int bits = 0;

	if( x < 0 ) bits |= 1 << 0;
	if( y < 0 ) bits |= 1 << 1;
	if( z < 0 ) bits |= 1 << 2;

	return bits;
}

INLINE Vec3 Vec3::operator-( ) const {
	return Vec3( -x, -y, -z );
}

INLINE Vec3 & Vec3::operator =( const Vec3 & a ) {
	x = a.x;
	y = a.y;
	z = a.z;
	return * this;
}

INLINE Vec3 Vec3::operator-( const Vec3 & a ) const {

	return Vec3( x - a.x, y - a.y, z - a.z );
}

INLINE float Vec3::operator* ( const Vec3 & a ) const {

	return x * a.x + y * a.y + z * a.z;
}

INLINE float Vec3::operator* ( const Vec4 & a ) const {

	return x * a.x + y * a.y + z * a.z;
}

INLINE Vec3 Vec3::operator* ( const float a ) const {
	return Vec3( x * a, y * a, z * a );
}

INLINE Vec3 Vec3::operator/( const float a ) const {
	float inva = 1.0f / a;
	return Vec3( x * inva, y * inva, z * inva );
}

INLINE Vec3 operator* ( const float a, const Vec3 b ) {
	return Vec3( b.x * a, b.y * a, b.z * a );
}

INLINE Vec3 Vec3::operator+( const Vec3 & a ) const {
	return Vec3( x + a.x, y + a.y, z + a.z );
}

INLINE Vec3 & Vec3::operator+=( const Vec3 & a ) {
	x += a.x;
	y += a.y;
	z += a.z;

	return * this;
}

INLINE Vec3 & Vec3::operator/=( const Vec3 & a ) {
	x /= a.x;
	y /= a.y;
	z /= a.z;

	return * this;
}

INLINE Vec3 & Vec3::operator/=( const float a ) {
	float inva = 1.0f / a;
	x *= inva;
	y *= inva;
	z *= inva;

	return * this;
}

INLINE Vec3 & Vec3::operator-=( const Vec3 & a ) {
	x -= a.x;
	y -= a.y;
	z -= a.z;

	return * this;
}

INLINE Vec3 & Vec3::operator*=( const float a ) {
	x *= a;
	y *= a;
	z *= a;

	return * this;
}

INLINE bool Vec3::Compare( const Vec3 & a ) const {
	return( ( x == a.x ) &&( y == a.y ) &&( z == a.z ) );
}

INLINE bool Vec3::Compare( const Vec3 & a, const float epsilon ) const {
	if( fabs( x - a.x ) > epsilon ) {
		return false;
	}
			
	if( fabs( y - a.y ) > epsilon ) {
		return false;
	}

	if( fabs( z - a.z ) > epsilon ) {
		return false;
	}

	return true;
}

INLINE bool Vec3::operator ==( const Vec3 & a ) const {
	return Compare( a );
}

INLINE bool Vec3::operator!=( const Vec3 & a ) const {
	return !Compare( a );
}

INLINE float Vec3::NormalizeFast( ) {
	float sqrLength, invLength;

	sqrLength = x * x + y * y + z * z;
	invLength = 1.0f / sqrt( sqrLength );
	x *= invLength;
	y *= invLength;
	z *= invLength;
	return invLength * sqrLength;
}

INLINE bool Vec3::FixDenormals( ) {

	bool denormal = false;

	if( fabs( x ) < 1e-30f ) {
		x = 0.0f;
		denormal = true;
	}

	if( fabs( y ) < 1e-30f ) {
		y = 0.0f;
		denormal = true;
	}

	if( fabs( z ) < 1e-30f ) {
		z = 0.0f;
		denormal = true;
	}

	return denormal;
}

INLINE Vec3 Vec3::Cross( const Vec3 & a ) const {
	return Vec3( y * a.z - z * a.y, z * a.x - x * a.z, x * a.y - y * a.x );
}

INLINE Vec3 & Vec3::Cross( const Vec3 & a, const Vec3 & b ) {
	x = a.y * b.z - a.z * b.y;
	y = a.z * b.x - a.x * b.z;
	z = a.x * b.y - a.y * b.x;

	return * this;
}

INLINE float Vec3::Length( ) const {
	return( float )sqrt( x * x + y * y + z * z );
}

INLINE float Vec3::LengthSqr( ) const {
	return( x * x + y * y + z * z );
}

INLINE float Vec3::Normalize( ) {

	float length = x * x + y * y + z * z;
	length = sqrtf( length );

	if( length < FLT_EPSILON ) {

		return 0.0f;
	}

	float ilength = 1.0f / length;

	x *= ilength;
	y *= ilength;
	z *= ilength;
		
	return length;
}

INLINE Vec3 & Vec3::Truncate( float length ) {
	float length2;
	float ilength;

	if( !length ) {
		Zero( );
	}
	else {
		length2 = LengthSqr( );
		if( length2 > length * length ) {
			ilength = length * -sqrt( length2 );
			x *= ilength;
			y *= ilength;
			z *= ilength;
		}
	}

	return * this;
}

INLINE void Vec3::Clamp( const Vec3 & min, const Vec3 & max ) {
	if( x < min.x ) {
		x = min.x;
	} else if( x > max.x ) {
		x = max.x;
	}
	if( y < min.y ) {
		y = min.y;
	} else if( y > max.y ) {
		y = max.y;
	}
	if( z < min.z ) {
		z = min.z;
	} else if( z > max.z ) {
		z = max.z;
	}
}

INLINE void Vec3::Snap( ) {
	x = floor( x + 0.5f );
	y = floor( y + 0.5f );
	z = floor( z + 0.5f );
}

INLINE void Vec3::SnapInt( ) {
	x = float( int( x ) );
	y = float( int( y ) );
	z = float( int( z ) );
}

INLINE const float * Vec3::ToFloatPtr( ) const {
	return &x;
}

INLINE float * Vec3::ToFloatPtr( ) {
	return &x;
}

INLINE void Vec3::NormalVectors( Vec3 & left, Vec3 & down ) const {
	float d;

	d = x * x + y * y;
	if( !d ) {
		left[ 0 ] = 1;
		left[ 1 ] = 0;
		left[ 2 ] = 0;
	} else {
		d = -sqrt( d );
		left[ 0 ] = -y * d;
		left[ 1 ] = x * d;
		left[ 2 ] = 0;
	}
	down = left.Cross( *this );
}

INLINE void Vec3::OrthogonalBasis( Vec3 & left, Vec3 & up ) const {
	float l, s;

	if( fabs( z ) > 0.7f ) {
		l = y * y + z * z;
		s = -sqrt( l );
		up[ 0 ] = 0;
		up[ 1 ] = z * s;
		up[ 2 ] = -y * s;
		left[ 0 ] = l * s;
		left[ 1 ] = -x * up[ 2 ];
		left[ 2 ] = x * up[ 1 ];
	}
	else {
		l = x * x + y * y;
		s = -sqrt( l );
		left[ 0 ] = -y * s;
		left[ 1 ] = x * s;
		left[ 2 ] = 0;
		up[ 0 ] = -z * left[ 1 ];
		up[ 1 ] = z * left[ 0 ];
		up[ 2 ] = l * s;
	}
}

INLINE void Vec3::ProjectOntoPlane( const Vec3 & normal, const float overBounce ) {
	float backoff;
	
	backoff = * this * normal;
	
	if( overBounce != 1.0f ) {
		if( backoff < 0 ) {
			backoff *= overBounce;
		} else {
			backoff /= overBounce;
		}
	}

	* this -= backoff * normal;
}

INLINE bool Vec3::ProjectAlongPlane( const Vec3 & normal, const float epsilon, const float overBounce ) {
	Vec3 cross;
	float len;

	cross = this->Cross( normal ).Cross( ( *this ) );
	// normalize so a fixed epsilon can be used
	cross.Normalize( );
	len = normal * cross;
	if( fabs( len ) < epsilon ) {
		return false;
	}
	cross *= overBounce * ( normal * ( *this ) ) / len;
	( *this ) -= cross;
	return true;
}

/*
=============
Lerp

Linearly inperpolates one vector to another.
=============
*/
INLINE void Vec3::Lerp( const Vec3 & v1, const Vec3 & v2, const float l ) {
	if( l <= 0.0f ) {
		( *this ) = v1;
	} else if( l >= 1.0f ) {
		( *this ) = v2;
	} else {
		( *this ) = v1 + l * ( v2 - v1 );
	}
}

/*
=============
SLerp

Spherical linear interpolation from v1 to v2.
Vectors are expected to be normalized.
=============
*/
#define LERP_DELTA 1e-6

INLINE void Vec3::SLerp( const Vec3 & v1, const Vec3 & v2, const float t ) {
	float omega, cosom, sinom, scale0, scale1;

	if( t <= 0.0f ) {
		( *this ) = v1;
		return;
	} else if( t >= 1.0f ) {
		( *this ) = v2;
		return;
	}

	cosom = v1 * v2;
	if( ( 1.0f - cosom ) > LERP_DELTA ) {
		omega = acos( cosom );
		sinom = sin( omega );
		scale0 = sin( ( 1.0f - t ) * omega ) / sinom;
		scale1 = sin( t * omega ) / sinom;
	} else {
		scale0 = 1.0f - t;
		scale1 = t;
	}

	( *this ) =( v1 * scale0 + v2 * scale1 );
}

INLINE void Vec3::AnglesNormalize( ) {

	while( x > 360.0f ) x -= 360.0f;
	while( x < 0.0f ) x += 360.0f;
	while( y > 360.0f ) y -= 360.0f;
	while( y < 0.0f ) y += 360.0f;
}

INLINE Vec3 Vec3::Magnitude( float scale, Vec3 & v2 ) {

	return( v2 * scale ) + * this;
}

INLINE void Vec3::CopyTo( float * dest ) {

	Common::Com_Memcpy( dest, this, sizeof( float ) * 3 );
}

INLINE Vec3 Vec3::Abs( ) const {

	return Vec3( fabs( x ), fabs( y ), fabs( z ) );
}

INLINE void Vec3::AngleVectors( Vec3 * forward, Vec3 * right, Vec3 * up ) const {

	float		angle;
	float		sr, sp, sy, cr, cp, cy;

	angle = ( *this )[ YAW ] * ( M_PI * 2.0f / 360.0f );
	SinCos( angle, sy, cy );
	angle = ( *this )[ PITCH ] * ( M_PI * 2.0f / 360.0f );
	SinCos( angle, sp, cp );
	angle = ( *this )[ ROLL ] * ( M_PI * 2.0f / 360.0f );
	SinCos( angle, sr, cr );

	if( forward ) {

		( *forward )[ 0 ] = cp * cy;
		( *forward )[ 1 ] = cp * sy;
		( *forward )[ 2 ] = -sp;
	}
	if( right ) {

		( *right )[ 0 ] =( -1.0f * sr * sp*  cy + -1.0f * cr * -sy );
		( *right )[ 1 ] =( -1.0f* sr * sp* sy + -1.0f * cr * cy );
		( *right )[ 2 ] = -1.0f * sr * cp;
	}
	if( up ) {

		( *up )[ 0 ] =( cr * sp * cy + -sr * -sy );
		( *up )[ 1 ] =( cr * sp * sy + -sr * cy );
		( *up )[ 2 ] = cr * cp;
	}
}

//===============================================================
//
//	Vec4 - 4D vector
//
//===============================================================

INLINE Vec4::Vec4( ) {
}

INLINE Vec4::Vec4( const Vec3 & vec, const float w ) {
	Common::Com_Memcpy( this, &vec, NumSizeOf( float, vec.GetDimension( ) ) );
	this->w = w;
}

INLINE Vec4::Vec4( const float x, const float y, const float z, const float w ) {

	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

INLINE void Vec4::Set( const float x, const float y, const float z, const float w ) {

	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

INLINE void Vec4::Zero( ) {

	x = y = z = w = 0.0f;
}

INLINE Vec4 & Vec4::operator =( const Vec3 & a ) {

	x = a.x;
	y = a.y;
	z = a.z;
	w = 1.0f;
	return * this;
}

INLINE float Vec4::operator[]( int index ) const {

	return ( &x )[ index ];
}

INLINE float& Vec4::operator[]( int index ) {

	return ( &x )[ index ];
}

INLINE Vec4 Vec4::operator-() const {

	return Vec4( -x, -y, -z, -w );
}

INLINE Vec4 Vec4::operator-( const Vec4 & a ) const {

	return Vec4( x - a.x, y - a.y, z - a.z, w - a.w );
}

INLINE float Vec4::operator*( const Vec4 & a ) const {

	return x * a.x + y * a.y + z * a.z + w * a.w;
}

INLINE Vec4 Vec4::operator*( const float a ) const {

	return Vec4( x * a, y * a, z * a, w * a );
}

INLINE Vec4 Vec4::operator/( const float a ) const {

	float inva = 1.0f / a;
	return Vec4( x * inva, y * inva, z * inva, w * inva );
}

INLINE Vec4 operator*( const float a, const Vec4 b ) {

	return Vec4( b.x * a, b.y * a, b.z * a, b.w * a );
}

INLINE Vec4 Vec4::operator+( const Vec4 & a ) const {

	return Vec4( x + a.x, y + a.y, z + a.z, w + a.w );
}

INLINE Vec4 & Vec4::operator+=( const Vec4 & a ) {

	x += a.x;
	y += a.y;
	z += a.z;
	w += a.w;

	return *this;
}

INLINE Vec4 & Vec4::operator/=( const Vec4 & a ) {

	x /= a.x;
	y /= a.y;
	z /= a.z;
	w /= a.w;

	return *this;
}

INLINE Vec4 & Vec4::operator/=( const float a ) {

	float inva = 1.0f / a;
	x *= inva;
	y *= inva;
	z *= inva;
	w *= inva;

	return *this;
}

INLINE Vec4 & Vec4::operator-=( const Vec4 & a ) {

	x -= a.x;
	y -= a.y;
	z -= a.z;
	w -= a.w;

	return *this;
}

INLINE Vec4 & Vec4::operator*=( const float a ) {

	x *= a;
	y *= a;
	z *= a;
	w *= a;

	return *this;
}

INLINE bool Vec4::Compare( const Vec4 & a ) const {

	return ( ( x == a.x ) && ( y == a.y ) && ( z == a.z ) && w == a.w );
}

INLINE bool Vec4::Compare( const Vec4 & a, const float epsilon ) const {

	if( fabs( x - a.x ) > epsilon ) {
		return false;
	}
			
	if( fabs( y - a.y ) > epsilon ) {
		return false;
	}

	if( fabs( z - a.z ) > epsilon ) {
		return false;
	}

	if( fabs( w - a.w ) > epsilon ) {
		return false;
	}

	return true;
}

INLINE bool Vec4::operator==( const Vec4 & a ) const {

	return Compare( a );
}

INLINE bool Vec4::operator!=( const Vec4 & a ) const {

	return !Compare( a );
}

INLINE float Vec4::Length( ) const {

	return ( float )sqrt( x * x + y * y + z * z + w * w );
}

INLINE float Vec4::LengthSqr( ) const {

	return ( x * x + y * y + z * z + w * w );
}

INLINE float Vec4::Normalize( ) {

	float sqrLength, invLength;

	sqrLength = x * x + y * y + z * z + w * w;
	invLength = 1.0f / sqrt( sqrLength );
	x *= invLength;
	y *= invLength;
	z *= invLength;
	w *= invLength;
	return invLength * sqrLength;
}

INLINE float Vec4::NormalizeFast( ) {

	float sqrLength, invLength;

	sqrLength = x * x + y * y + z * z + w * w;
	invLength = 1.0f / sqrt( sqrLength );
	x *= invLength;
	y *= invLength;
	z *= invLength;
	w *= invLength;
	return invLength * sqrLength;
}

INLINE int Vec4::GetDimension( ) const {

	return 4;
}

INLINE const Vec3 & Vec4::ToVec3( ) const {

	return *reinterpret_cast< const Vec3 * >(this);
}

INLINE Vec3 & Vec4::ToVec3( ) {

	return *reinterpret_cast< Vec3 * >(this);
}

INLINE const float *Vec4::ToFloatPtr( ) const {

	return &x;
}

INLINE float *Vec4::ToFloatPtr( ) {

	return &x;
}

/*
=============
Lerp

Linearly inperpolates one vector to another.
=============
*/
INLINE void Vec4::Lerp( const Vec4 & v1, const Vec4 & v2, const float l ) {

	if( l <= 0.0f ) *this = v1;
	else if( l >= 1.0f ) *this = v2;
	else *this = v1 + l * ( v2 - v1 );
}

//===============================================================
//
//	Vec5 - 5D vector
//
//===============================================================

INLINE Vec5::Vec5( ) {

}

INLINE Vec5::Vec5( const float x, const float y, const float z, const float s, const float t ) {

	this->x = x;
	this->y = y;
	this->z = z;
	this->s = s;
	this->t = t;
}

INLINE float Vec5::operator[]( int index ) const {

	return ( &x )[ index ];
}

INLINE float& Vec5::operator[]( int index ) {

	return ( &x )[ index ];
}

INLINE Vec5 & Vec5::operator=( const Vec3 & a ) { 

	x = a.x;
	y = a.y;
	z = a.z;
	s = t = 0;
	return *this;
}

INLINE int Vec5::GetDimension( ) const {

	return 5;
}

INLINE const Vec3 & Vec5::ToVec3( ) const {

	return *reinterpret_cast<const Vec3 *>(this);
}

INLINE Vec3 & Vec5::ToVec3( ) {

	return *reinterpret_cast<Vec3 *>(this);
}

INLINE const float *Vec5::ToFloatPtr( ) const {

	return &x;
}

INLINE float *Vec5::ToFloatPtr( ) {

	return &x;
}

/*
=============
Vec5::Lerp
=============
*/
INLINE void Vec5::Lerp( const Vec5 & v1, const Vec5 & v2, const float l ) {

	if( l <= 0.0f ) {
		(*this) = v1;
	} else if( l >= 1.0f ) {
		(*this) = v2;
	} else {
		x = v1.x + l * ( v2.x - v1.x );
		y = v1.y + l * ( v2.y - v1.y );
		z = v1.z + l * ( v2.z - v1.z );
		s = v1.s + l * ( v2.s - v1.s );
		t = v1.t + l * ( v2.t - v1.t );
	}
}






INLINE Vec6::Vec6( ) {
}

INLINE Vec6::Vec6( const float *a ) {
	Common::Com_Memcpy( p, a, 6 * sizeof( float ) );
}

INLINE Vec6::Vec6( const float a1, const float a2, const float a3, const float a4, const float a5, const float a6 ) {
	p[0] = a1;
	p[1] = a2;
	p[2] = a3;
	p[3] = a4;
	p[4] = a5;
	p[5] = a6;
}

INLINE Vec6 Vec6::operator-() const {
	return Vec6( -p[0], -p[1], -p[2], -p[3], -p[4], -p[5] );
}

INLINE float Vec6::operator[]( const int index ) const {
	return p[index];
}

INLINE float &Vec6::operator[]( const int index ) {
	return p[index];
}

INLINE Vec6 Vec6::operator*( const float a ) const {
	return Vec6( p[0]*a, p[1]*a, p[2]*a, p[3]*a, p[4]*a, p[5]*a );
}

INLINE float Vec6::operator*( const Vec6 & a ) const {
	return p[0] * a[0] + p[1] * a[1] + p[2] * a[2] + p[3] * a[3] + p[4] * a[4] + p[5] * a[5];
}

INLINE Vec6 Vec6::operator/( const float a ) const {
	float inva;

	assert( a != 0.0f );
	inva = 1.0f / a;
	return Vec6( p[0]*inva, p[1]*inva, p[2]*inva, p[3]*inva, p[4]*inva, p[5]*inva );
}

INLINE Vec6 Vec6::operator+( const Vec6 & a ) const {
	return Vec6( p[0] + a[0], p[1] + a[1], p[2] + a[2], p[3] + a[3], p[4] + a[4], p[5] + a[5] );
}

INLINE Vec6 Vec6::operator-( const Vec6 & a ) const {
	return Vec6( p[0] - a[0], p[1] - a[1], p[2] - a[2], p[3] - a[3], p[4] - a[4], p[5] - a[5] );
}

INLINE Vec6 & Vec6::operator*=( const float a ) {
	p[0] *= a;
	p[1] *= a;
	p[2] *= a;
	p[3] *= a;
	p[4] *= a;
	p[5] *= a;
	return *this;
}

INLINE Vec6 & Vec6::operator/=( const float a ) {
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

INLINE Vec6 & Vec6::operator+=( const Vec6 & a ) {
	p[0] += a[0];
	p[1] += a[1];
	p[2] += a[2];
	p[3] += a[3];
	p[4] += a[4];
	p[5] += a[5];
	return *this;
}

INLINE Vec6 & Vec6::operator-=( const Vec6 & a ) {
	p[0] -= a[0];
	p[1] -= a[1];
	p[2] -= a[2];
	p[3] -= a[3];
	p[4] -= a[4];
	p[5] -= a[5];
	return *this;
}

INLINE Vec6 operator*( const float a, const Vec6 b ) {
	return b * a;
}

INLINE bool Vec6::Compare( const Vec6 & a ) const {
	return ( ( p[0] == a[0] ) && ( p[1] == a[1] ) && ( p[2] == a[2] ) &&
			( p[3] == a[3] ) && ( p[4] == a[4] ) && ( p[5] == a[5] ) );
}

INLINE bool Vec6::Compare( const Vec6 & a, const float epsilon ) const {
	if( fabs( p[0] - a[0] ) > epsilon ) {
		return false;
	}
			
	if( fabs( p[1] - a[1] ) > epsilon ) {
		return false;
	}

	if( fabs( p[2] - a[2] ) > epsilon ) {
		return false;
	}

	if( fabs( p[3] - a[3] ) > epsilon ) {
		return false;
	}

	if( fabs( p[4] - a[4] ) > epsilon ) {
		return false;
	}

	if( fabs( p[5] - a[5] ) > epsilon ) {
		return false;
	}

	return true;
}

INLINE bool Vec6::operator==( const Vec6 & a ) const {
	return Compare( a );
}

INLINE bool Vec6::operator!=( const Vec6 & a ) const {
	return !Compare( a );
}

INLINE void Vec6::Set( const float a1, const float a2, const float a3, const float a4, const float a5, const float a6 ) {
	p[0] = a1;
	p[1] = a2;
	p[2] = a3;
	p[3] = a4;
	p[4] = a5;
	p[5] = a6;
}

INLINE void Vec6::Zero( ) {
	p[0] = p[1] = p[2] = p[3] = p[4] = p[5] = 0.0f;
}

INLINE float Vec6::Length( ) const {
	return ( float )sqrtf( p[0] * p[0] + p[1] * p[1] + p[2] * p[2] + p[3] * p[3] + p[4] * p[4] + p[5] * p[5] );
}

INLINE float Vec6::LengthSqr( ) const {
	return ( p[0] * p[0] + p[1] * p[1] + p[2] * p[2] + p[3] * p[3] + p[4] * p[4] + p[5] * p[5] );
}

INLINE float Vec6::Normalize( ) {
	float sqrLength, invLength;

	sqrLength = p[0] * p[0] + p[1] * p[1] + p[2] * p[2] + p[3] * p[3] + p[4] * p[4] + p[5] * p[5];
	invLength = 1.0f / sqrtf( sqrLength );
	p[0] *= invLength;
	p[1] *= invLength;
	p[2] *= invLength;
	p[3] *= invLength;
	p[4] *= invLength;
	p[5] *= invLength;
	return invLength * sqrLength;
}

INLINE float Vec6::NormalizeFast( ) {
	float sqrLength, invLength;

	sqrLength = p[0] * p[0] + p[1] * p[1] + p[2] * p[2] + p[3] * p[3] + p[4] * p[4] + p[5] * p[5];
	invLength = RSqrt( sqrLength );
	p[0] *= invLength;
	p[1] *= invLength;
	p[2] *= invLength;
	p[3] *= invLength;
	p[4] *= invLength;
	p[5] *= invLength;
	return invLength * sqrLength;
}

INLINE int Vec6::GetDimension( ) const {
	return 6;
}

INLINE const Vec3 & Vec6::SubVec3( int index ) const {
	return *reinterpret_cast<const Vec3 *>(p + index * 3);
}

INLINE Vec3 & Vec6::SubVec3( int index ) {
	return *reinterpret_cast<Vec3 *>(p + index * 3);
}

INLINE const float *Vec6::ToFloatPtr( ) const {
	return p;
}

INLINE float *Vec6::ToFloatPtr( ) {
	return p;
}






INLINE VecX::VecX( ) {
	size = alloced = 0;
	p = NULL;
}

INLINE VecX::VecX( int length ) {
	size = alloced = 0;
	p = NULL;
	SetSize( length );
}

INLINE VecX::VecX( int length, float *data ) {
	size = alloced = 0;
	p = NULL;
	SetData( length, data );
}

INLINE VecX::~VecX( ) {
	// if not temp memory
	if( p && ( p < VecX::tempPtr || p >= VecX::tempPtr + VECX_MAX_TEMP ) && alloced != -1 ) {
		free( p );
	}
}

INLINE float VecX::operator[]( const int index ) const {
	assert( index >= 0 && index < size );
	return p[index];
}

INLINE float &VecX::operator[]( const int index ) {
	assert( index >= 0 && index < size );
	return p[index];
}

INLINE VecX VecX::operator-() const {
	int i;
	VecX m;

	m.SetTempSize( size );
	for( i = 0; i < size; i++ ) {
		m.p[i] = -p[i];
	}
	return m;
}

INLINE VecX &VecX::operator=( const VecX &a ) { 
	SetSize( a.size );
#ifdef VECX_SIMD
	SIMDProcessor->Copy16( p, a.p, a.size );
#else
	Common::Com_Memcpy( p, a.p, a.size * sizeof( float ) );
#endif
	VecX::tempIndex = 0;
	return *this;
}

INLINE VecX VecX::operator+( const VecX &a ) const {
	VecX m;

	assert( size == a.size );
	m.SetTempSize( size );
#ifdef VECX_SIMD
	SIMDProcessor->Add16( m.p, p, a.p, size );
#else
	int i;
	for( i = 0; i < size; i++ ) {
		m.p[i] = p[i] + a.p[i];
	}
#endif
	return m;
}

INLINE VecX VecX::operator-( const VecX &a ) const {
	VecX m;

	assert( size == a.size );
	m.SetTempSize( size );
#ifdef VECX_SIMD
	SIMDProcessor->Sub16( m.p, p, a.p, size );
#else
	int i;
	for( i = 0; i < size; i++ ) {
		m.p[i] = p[i] - a.p[i];
	}
#endif
	return m;
}

INLINE VecX &VecX::operator+=( const VecX &a ) {
	assert( size == a.size );
#ifdef VECX_SIMD
	SIMDProcessor->AddAssign16( p, a.p, size );
#else
	int i;
	for( i = 0; i < size; i++ ) {
		p[i] += a.p[i];
	}
#endif
	VecX::tempIndex = 0;
	return *this;
}

INLINE VecX &VecX::operator-=( const VecX &a ) {
	assert( size == a.size );
#ifdef VECX_SIMD
	SIMDProcessor->SubAssign16( p, a.p, size );
#else
	int i;
	for( i = 0; i < size; i++ ) {
		p[i] -= a.p[i];
	}
#endif
	VecX::tempIndex = 0;
	return *this;
}

INLINE VecX VecX::operator*( const float a ) const {
	VecX m;

	m.SetTempSize( size );
#ifdef VECX_SIMD
	SIMDProcessor->Mul16( m.p, p, a, size );
#else
	int i;
	for( i = 0; i < size; i++ ) {
		m.p[i] = p[i] * a;
	}
#endif
	return m;
}

INLINE VecX &VecX::operator*=( const float a ) {
#ifdef VECX_SIMD
	SIMDProcessor->MulAssign16( p, a, size );
#else
	int i;
	for( i = 0; i < size; i++ ) {
		p[i] *= a;
	}
#endif
	return *this;
}

INLINE VecX VecX::operator/( const float a ) const {
	assert( a != 0.0f );
	return (*this) * ( 1.0f / a );
}

INLINE VecX &VecX::operator/=( const float a ) {
	assert( a != 0.0f );
	(*this) *= ( 1.0f / a );
	return *this;
}

INLINE VecX operator*( const float a, const VecX b ) {
	return b * a;
}

INLINE float VecX::operator*( const VecX &a ) const {
	int i;
	float sum = 0.0f;

	assert( size == a.size );
	for( i = 0; i < size; i++ ) {
		sum += p[i] * a.p[i];
	}
	return sum;
}

INLINE bool VecX::Compare( const VecX &a ) const {
	int i;

	assert( size == a.size );
	for( i = 0; i < size; i++ ) {
		if( p[i] != a.p[i] ) {
			return false;
		}
	}
	return true;
}

INLINE bool VecX::Compare( const VecX &a, const float epsilon ) const {
	int i;

	assert( size == a.size );
	for( i = 0; i < size; i++ ) {
		if( fabs( p[i] - a.p[i] ) > epsilon ) {
			return false;
		}
	}
	return true;
}

INLINE bool VecX::operator==( const VecX &a ) const {
	return Compare( a );
}

INLINE bool VecX::operator!=( const VecX &a ) const {
	return !Compare( a );
}

INLINE void VecX::SetSize( int newSize ) {
	int alloc = ( newSize + 3 ) & ~3;
	if( alloc > alloced && alloced != -1 ) {
		if( p ) {
			free( p );
		}
		p = (float *) malloc( alloc * sizeof( float ) );
		alloced = alloc;
	}
	size = newSize;
	VECX_CLEAREND();
}

INLINE void VecX::ChangeSize( int newSize, bool makeZero ) {
	int alloc = ( newSize + 3 ) & ~3;
	if( alloc > alloced && alloced != -1 ) {
		float *oldVec = p;
		p = (float *) malloc( alloc * sizeof( float ) );
		alloced = alloc;
		if( oldVec ) {
			for( int i = 0; i < size; i++ ) {
				p[i] = oldVec[i];
			}
			free( oldVec );
		}
		if( makeZero ) {
			// zero any new elements
			for( int i = size; i < newSize; i++ ) {
				p[i] = 0.0f;
			}
		}
	}
	size = newSize;
	VECX_CLEAREND();
}

INLINE void VecX::SetTempSize( int newSize ) {

	size = newSize;
	alloced = ( newSize + 3 ) & ~3;
	assert( alloced < VECX_MAX_TEMP );
	if( VecX::tempIndex + alloced > VECX_MAX_TEMP ) {
		VecX::tempIndex = 0;
	}
	p = VecX::tempPtr + VecX::tempIndex;
	VecX::tempIndex += alloced;
	VECX_CLEAREND();
}

INLINE void VecX::SetData( int length, float *data ) {
	if( p && ( p < VecX::tempPtr || p >= VecX::tempPtr + VECX_MAX_TEMP ) && alloced != -1 ) {
		free( p );
	}
	assert( ( ( (int) data ) & 15 ) == 0 ); // data must be 16 byte aligned
	p = data;
	size = length;
	alloced = -1;
	VECX_CLEAREND();
}

INLINE void VecX::Zero( ) {
#ifdef VECX_SIMD
	SIMDProcessor->Zero16( p, size );
#else
	Common::Com_Memset( p, 0, size * sizeof( float ) );
#endif
}

INLINE void VecX::Zero( int length ) {
	SetSize( length );
#ifdef VECX_SIMD
	SIMDProcessor->Zero16( p, length );
#else
	Common::Com_Memset( p, 0, size * sizeof( float ) );
#endif
}

INLINE void VecX::Random( int seed, float l, float u ) {
	int i;
	float c;

	c = u - l;
	for( i = 0; i < size; i++ ) {
		p[i] = l + rand( ) * c;
	}
}

INLINE void VecX::Random( int length, int seed, float l, float u ) {
	int i;
	float c;

	SetSize( length );
	c = u - l;
	for( i = 0; i < size; i++ ) {
		p[i] = l + rand( ) * c;
	}
}

INLINE void VecX::Negate( ) {
#ifdef VECX_SIMD
	SIMDProcessor->Negate16( p, size );
#else
	int i;
	for( i = 0; i < size; i++ ) {
		p[i] = -p[i];
	}
#endif
}

INLINE void VecX::Clamp( float min, float max ) {
	int i;
	for( i = 0; i < size; i++ ) {
		if( p[i] < min ) {
			p[i] = min;
		} else if( p[i] > max ) {
			p[i] = max;
		}
	}
}

INLINE VecX &VecX::SwapElements( int e1, int e2 ) {
	float tmp;
	tmp = p[e1];
	p[e1] = p[e2];
	p[e2] = tmp;
	return *this;
}

INLINE float VecX::Length( ) const {
	int i;
	float sum = 0.0f;

	for( i = 0; i < size; i++ ) {
		sum += p[i] * p[i];
	}
	return sqrtf( sum );
}

INLINE float VecX::LengthSqr( ) const {
	int i;
	float sum = 0.0f;

	for( i = 0; i < size; i++ ) {
		sum += p[i] * p[i];
	}
	return sum;
}

INLINE VecX VecX::Normalize( ) const {
	int i;
	VecX m;
	float invSqrt, sum = 0.0f;

	m.SetTempSize( size );
	for( i = 0; i < size; i++ ) {
		sum += p[i] * p[i];
	}
	invSqrt = 1.0f / sqrtf( sum );
	for( i = 0; i < size; i++ ) {
		m.p[i] = p[i] * invSqrt;
	}
	return m;
}

INLINE float VecX::NormalizeSelf( ) {
	float invSqrt, sum = 0.0f;
	int i;
	for( i = 0; i < size; i++ ) {
		sum += p[i] * p[i];
	}
	invSqrt = 1.0f / sqrtf( sum );
	for( i = 0; i < size; i++ ) {
		p[i] *= invSqrt;
	}
	return invSqrt * sum;
}

INLINE int VecX::GetDimension( ) const {
	return size;
}

INLINE Vec3 & VecX::SubVec3( int index ) {
	assert( index >= 0 && index * 3 + 3 <= size );
	return *reinterpret_cast<Vec3 *>(p + index * 3);
}

INLINE const Vec3 & VecX::SubVec3( int index ) const {
	assert( index >= 0 && index * 3 + 3 <= size );
	return *reinterpret_cast<const Vec3 *>(p + index * 3);
}

INLINE Vec6 & VecX::SubVec6( int index ) {
	assert( index >= 0 && index * 6 + 6 <= size );
	return *reinterpret_cast<Vec6 *>(p + index * 6);
}

INLINE const Vec6 & VecX::SubVec6( int index ) const {
	assert( index >= 0 && index * 6 + 6 <= size );
	return *reinterpret_cast<const Vec6 *>(p + index * 6);
}

INLINE const float *VecX::ToFloatPtr( ) const {
	return p;
}

INLINE float *VecX::ToFloatPtr( ) {
	return p;
}

// // //// // //// // //// //
/// VecT
// //// // //// // //// //

template< class type, int dimension, class math_type > class VecT {
protected:
	type					d_data[ dimension ];

public:

							VecT( );
							VecT( const type & x, const type & y );
							VecT( const type & x, const type & y, const type & z );
							VecT( const type & x, const type & y, const type & z, const type & w );
							VecT( const VecT & other );

	template< class T, class Tm >	
	void					Convert( VecT< T, dimension, Tm > & output ) const;
	
	void					Set( const type & x );
	void					Set( const type & x, const type & y );
	void					Set( const type & x, const type & y, const type & z );
	void					Set( const type & x, const type & y, const type & z, const type & w );
	void					Zero( );

	type					operator[ ]( int index ) const;
	type &					operator[ ]( int index );
	VecT					operator -( ) const;
	math_type				operator *( const VecT & a ) const;
	VecT					operator *( const math_type a ) const;
	VecT					operator /( const math_type a ) const;
	VecT					operator +( const VecT & a ) const;
	VecT					operator -( const VecT & a ) const;
	VecT &					operator =( const VecT & a );
	VecT &					operator +=( const VecT & a );
	VecT &					operator -=( const VecT & a );
	VecT &					operator /=( const VecT & a );
	VecT &					operator /=( const math_type a );
	VecT &					operator *=( const math_type a );
	friend VecT				operator *( const math_type a, const VecT & b );

	/// exact compare, no epsilon
	bool					Compare( const VecT & a ) const;

	/// exact compare, no epsilon
	bool					operator ==( const VecT & a ) const;

	/// exact compare, no epsilon
	bool					operator !=( const VecT & a ) const;

	math_type				Length( ) const;
	math_type				LengthFast( ) const;
	math_type				LengthSqr( ) const;

	/// returns length
	math_type				Normalize( );

	/// returns length
	math_type				NormalizeFast( );

	//VecT					Magnitude( const math_type scale, const VecT & v2 ) const;

	/// cap length
	VecT &					Truncate( math_type length );

	void					Clamp( const VecT & min, const VecT & max );

	/// snap to closest integer value
	void					Snap( );

	/// snap towards integer (floor)
	void					SnapInt( );
	void					Ceil( );

	int						GetDimension( ) const;

	const type *			Ptr( ) const;
	type *					Ptr( );
	const Str				ToString( int precision = 2 ) const;

	void					Lerp( const VecT & v1, const VecT & v2, const math_type l );
};

extern const VecT2i		vecT2i_origin;

INLINE Vec2::operator VecT2f( ) const {
	return VecT2f( d_x, d_y );
}

INLINE Vec2::operator VecT2hf( ) const {
	return VecT2hf( d_x, d_y );
}

INLINE Vec3::operator VecT3f( ) const {
	return VecT3f( x, y, z );
}

INLINE Vec3 & Vec3::operator =( const VecT3f & a ) {
	x = a[0];
	y = a[1];
	z = a[2];
	return *this;
}

// // //// // //// // //// //
// VecT
// //// // //// // //// //

template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type >::VecT( ) {
}

template< class type, int dimension, class math_type >
template< class T, class Tm >
INLINE void VecT< type, dimension, math_type >::Convert( VecT< T, dimension, Tm > & output ) const {
	for( int i = 0; i < dimension; i++ )
		output[ i ] = ( T )d_data[ i ];
}

template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type >::VecT( const type & x, const type & y ) {
	d_data[ 0 ] = x;
	d_data[ 1 ] = y;
}

template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type >::VecT( const type & x, const type & y, const type & z ) {
	assert( dimension == 3 );
	d_data[ 0 ] = x;
	d_data[ 1 ] = y;
	d_data[ 2 ] = z;
}

template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type >::VecT( const type & x, const type & y, const type & z, const type & w ) {
	assert( dimension == 4 );
	d_data[ 0 ] = x;
	d_data[ 1 ] = y;
	d_data[ 2 ] = z;
	d_data[ 3 ] = w;
}

template< class type, int dimension, class math_type >
INLINE void VecT< type, dimension, math_type >::Set( const type & x ) {
	assert( dimension == 1 );
	d_data[ 0 ] = x;
}

template< class type, int dimension, class math_type >
INLINE void VecT< type, dimension, math_type >::Set( const type & x, const type & y ) {
	assert( dimension == 2 );
	d_data[ 0 ] = x;
	d_data[ 1 ] = y;
}

template< class type, int dimension, class math_type >
INLINE void VecT< type, dimension, math_type >::Set( const type & x, const type & y, const type & z ) {
	assert( dimension == 3 );
	d_data[ 0 ] = x;
	d_data[ 1 ] = y;
	d_data[ 2 ] = z;
}

template< class type, int dimension, class math_type >
INLINE void VecT< type, dimension, math_type >::Set( const type & x, const type & y, const type & z, const type & w ) {
	assert( dimension == 4 );
	d_data[ 0 ] = x;
	d_data[ 1 ] = y;
	d_data[ 2 ] = z;
	d_data[ 3 ] = w;
}

template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type >::VecT( const VecT & other ) {
	Common::Com_Memcpy( d_data, other.d_data, NumSizeOf( type, dimension ) );
}

template< class type, int dimension, class math_type >
INLINE void VecT< type, dimension, math_type >::Zero( ) {
	Common::Com_Memset( d_data, 0, NumSizeOf( type, dimension ) );
}

template< class type, int dimension, class math_type >
INLINE bool VecT< type, dimension, math_type >::Compare( const VecT & a ) const {
	return !memcmp( d_data, a.d_data, NumSizeOf( type, dimension ) );
}

template< class type, int dimension, class math_type >
INLINE bool VecT< type, dimension, math_type >::operator ==( const VecT & a ) const {
	return !memcmp( d_data, a.d_data, NumSizeOf( type, dimension ) );
}

template< class type, int dimension, class math_type >
INLINE bool VecT< type, dimension, math_type >::operator !=( const VecT & a ) const {
	return memcmp( d_data, a.d_data, NumSizeOf( type, dimension ) ) != 0;
}

template< class type, int dimension, class math_type >
INLINE type VecT< type, dimension, math_type >::operator [ ]( int index ) const {
	return d_data[ index ];
}

template< class type, int dimension, class math_type >
INLINE type & VecT< type, dimension, math_type >::operator [ ]( int index ) {
	return d_data[ index ];
}

template< class type, int dimension, class math_type >
INLINE math_type VecT< type, dimension, math_type >::Length( ) const {
	math_type sqrLength = ( math_type )( d_data[ 0 ] * d_data[ 0 ] );
	for( int i = 1; i < dimension; i++ )
		sqrLength += d_data[ i ] * d_data[ i ];
	return ( math_type )sqrt( sqrLength );
}

template< class type, int dimension, class math_type >
INLINE math_type VecT< type, dimension, math_type >::LengthFast( ) const {
	math_type sqrLength = ( math_type )( d_data[ 0 ] * d_data[ 0 ] );
	for( int i = 1; i < dimension; i++ )
		sqrLength += d_data[ i ] * d_data[ i ];
	return sqrLength * RSqrt( ( float )sqrLength );
}

template< class type, int dimension, class math_type >
INLINE math_type VecT< type, dimension, math_type >::LengthSqr( ) const {
	math_type sqrLength = ( math_type )( d_data[ 0 ] * d_data[ 0 ] );
	for( int i = 1; i < dimension; i++ )
		sqrLength += d_data[ i ] * d_data[ i ];
	return sqrLength;
}

template< class type, int dimension, class math_type >
INLINE math_type VecT< type, dimension, math_type >::Normalize( ) {
	math_type sqrLength = ( math_type )( d_data[ 0 ] * d_data[ 0 ] );
	for( int i = 1; i < dimension; i++ )
		sqrLength += d_data[ i ] * d_data[ i ];
	math_type invLength = ( math_type )( ( math_type )( 1 ) / sqrt( sqrLength ) );
	for( int i = 0; i < dimension; i++ )
		d_data[ i ] = ( type )( ( math_type )( d_data[ i ] ) * invLength );
	return invLength * sqrLength;
}

template< class type, int dimension, class math_type >
INLINE math_type VecT< type, dimension, math_type >::NormalizeFast( ) {
	math_type sqrLength = ( math_type )( d_data[ 0 ] * d_data[ 0 ] );
	for( int i = 1; i < dimension; i++ )
		sqrLength += d_data[ i ] * d_data[ i ];
	math_type invLength = ( math_type )RSqrt( ( float )lengthSqr );
	for( int i = 0; i < dimension; i++ )
		d_data[ i ] = ( type )( ( math_type )( d_data[ i ] ) * invLength );
	return invLength * sqrLength;
}

//template< class type, int dimension, class math_type >
//INLINE VecT< type, dimension, math_type > VecT< type, dimension, math_type >::Magnitude( const math_type scale, const VecT & v2 ) const {
//	return ( v2 * scale ) + *this;
//}

template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type > & VecT< type, dimension, math_type >::Truncate( math_type length ) {
	if( !length )
		Zero( );
	else {
		math_type sqrLength = LengthSqr( );
		if( sqrLength > ( length * length ) ) {
			math_type invLength = length * ( math_type )( 1 ) / sqrt( length2 );
			for( int i = 0; i < dimension; i++ )
				d_data[ i ] = ( type )( ( math_type )( d_data[ i ] ) * invLength );
		}
	}
	return *this;
}

template< class type, int dimension, class math_type >
INLINE void VecT< type, dimension, math_type >::Clamp( const VecT & min, const VecT & max ) {
	for( int i = 0; i < dimension; i++ )
		if( d_data[ i ] < min.d_data[ i ] )
			d_data[ i ] = min.d_data[ i ];
		else if( d_data[ i ] > max.d_data[ i ] )
			d_data[ i ] = max.d_data[ i ];
}

template< class type, int dimension, class math_type >
INLINE void VecT< type, dimension, math_type >::Snap( ) {
	for( int i = 0; i < dimension; i++ )
		d_data[ i ] = ( type )floor( ( math_type )d_data[ i ] + ( math_type )0.5f );
}

template< class type, int dimension, class math_type >
INLINE void VecT< type, dimension, math_type >::SnapInt( ) {
	for( int i = 0; i < dimension; i++ )
		d_data[ i ] = ( type )( ( int )( d_data[ i ] ) );
}

template< class type, int dimension, class math_type >
INLINE void VecT< type, dimension, math_type >::Ceil( ) {
	for( int i = 0; i < dimension; i++ )
		d_data[ i ] = ( type )ceil( ( math_type )d_data[ i ] );
}

template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type > VecT< type, dimension, math_type >::operator -( ) const {
	VecT retval;
	for( int i = 0; i < dimension; i++ )
		retval[ i ] = -d_data[ i ];
	return retval;
}
	
template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type > VecT< type, dimension, math_type >::operator -( const VecT & a ) const {
	VecT retval;
	for( int i = 0; i < dimension; i++ )
		retval[ i ] = d_data[ i ] - a.d_data[ i ];
	return retval;
}

template< class type, int dimension, class math_type >
INLINE math_type VecT< type, dimension, math_type >::operator *( const VecT & a ) const {
	math_type dot = ( math_type )( d_data[ 0 ] * a.d_data[ 0 ] );
	for( int i = 1; i < dimension; i++ )
		dot += ( math_type )( d_data[ i ] * a.d_data[ i ] );
	return dot;
}

template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type > VecT< type, dimension, math_type >::operator *( const math_type a ) const {
	VecT retval;
	for( int i = 0; i < dimension; i++ )
		retval[ i ] = ( type )( ( ( math_type )d_data[ i ] ) * a );
	return retval;
}

template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type > VecT< type, dimension, math_type >::operator /( const math_type a ) const {
	math_type inva = ( math_type )( 1 ) / a;
	VecT retval;
	for( int i = 0; i < dimension; i++ )
		retval[ i ] = ( type )( ( ( math_type )d_data[ i ] ) * inva );
	return retval;
}

template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type > VecT< type, dimension, math_type >::operator +( const VecT & a ) const {
	VecT retval;
	for( int i = 0; i < dimension; i++ )
		retval[ i ] = d_data[ i ] + a.d_data[ i ];
	return retval;
}

template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type > & VecT< type, dimension, math_type >::operator =( const VecT & a ) {
	Common::Com_Memcpy( d_data, a.d_data, NumSizeOf( type, dimension ) );
	return *this;
}

template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type > & VecT< type, dimension, math_type >::operator +=( const VecT & a ) {
	for( int i = 0; i < dimension; i++ )
		d_data[ i ] += a.d_data[ i ];
	return *this;
}

template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type > & VecT< type, dimension, math_type >::operator /=( const VecT & a ) {
	for( int i = 0; i < dimension; i++ )
		d_data[ i ] /= a.d_data[ i ];
	return *this;
}

template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type > & VecT< type, dimension, math_type >::operator /=( const math_type a ) {
	math_type inva = ( math_type )( 1 ) / a;
	for( int i = 0; i < dimension; i++ )
		d_data[ i ] = ( type )( inva * ( math_type )d_data[ i ] );
	return *this;
}

template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type > & VecT< type, dimension, math_type >::operator -=( const VecT & a ) {
	for( int i = 0; i < dimension; i++ )
		d_data[ i ] -= a.d_data[ i ];
	return *this;
}

template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type > & VecT< type, dimension, math_type >::operator *=( const math_type a ) {
	for( int i = 0; i < dimension; i++ )
		d_data[ i ] = ( type )( a * ( math_type )d_data[ i ] );
	return *this;
}

template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type > operator *( const math_type a, const VecT< type, dimension, math_type > & b ) {
	return VecT( b.d_data[ 0 ] * a, b.d_data[ 1 ] * a, b.d_data[ 2 ] * a );
}

template< class type, int dimension, class math_type >
INLINE int VecT< type, dimension, math_type >::GetDimension( ) const {
	return dimension;
}

template< class type, int dimension, class math_type >
INLINE const type * VecT< type, dimension, math_type >::Ptr( ) const {
	return d_data;
}

template< class type, int dimension, class math_type >
INLINE type * VecT< type, dimension, math_type >::Ptr( ) {
	return d_data;
}

template< class type, int dimension, class math_type >
INLINE void VecT< type, dimension, math_type >::Lerp( const VecT & v1, const VecT & v2, const math_type l ) {
	if( l <= ( math_type )( 0 ) )
		( *this ) = v1;
	else if( l >= ( math_type )( 1 ) )
		( *this ) = v2;
	else
		( *this ) = v1 + l * ( v2 - v1 );
}
