#ifndef VEC3_H
#define VEC3_H

template< class type, int dimension, class math_type = float > class VecT;

typedef VecT< int, 2 >		VecT2i;
typedef VecT< float, 2 >	VecT2f;
typedef VecT< float, 3 >	VecT3f;
typedef VecT< Float16, 2 >	VecT2hf;
typedef VecT< Float16, 4 >	VecT4hf;

class Vec4;
class Angles;
class Str;

class Vec3 {

public:	

	float			x;
	float			y;
	float			z;
	float			w;

					Vec3( );
					explicit Vec3( const Vec4 & vec );
					explicit Vec3( const float x, const float y, const float z );

	void 			Set( const float x, const float y, const float z );
	void			Zero( );

	//planeType_e		GetPlaneType( ) const;

	float			operator[]( const int index ) const;
	float &			operator[]( const int index );
	Vec3			operator-( ) const;
	Vec3 &			operator =( const Vec3 &a );		// required because of a msvc 6 & 7 bug
	float			operator* ( const Vec3 &a ) const;
	float			operator* ( const Vec4 &a ) const;
	Vec3			operator* ( const float a ) const;
	Vec3			operator/( const float a ) const;
	Vec3			operator+( const Vec3 &a ) const;
	Vec3			operator-( const Vec3 &a ) const;
	Vec3 &			operator+=( const Vec3 &a );
	Vec3 &			operator-=( const Vec3 &a );
	Vec3 &			operator/=( const Vec3 &a );
	Vec3 &			operator/=( const float a );
	Vec3 &			operator*=( const float a );

	friend Vec3		operator* ( const float a, const Vec3 b );

	bool			Compare( const Vec3 &a ) const;							// exact compare, no epsilon
	bool			Compare( const Vec3 &a, const float epsilon ) const;		// compare with epsilon
	bool			operator ==( 	const Vec3 &a ) const;						// exact compare, no epsilon
	bool			operator!=( 	const Vec3 &a ) const;						// exact compare, no epsilon

	const float &	Smallest( ) const	{ return MinRef( x, MinRef( y, z ) ); }
	const float &	Largest( ) const	{ return MaxRef( x, MaxRef( y, z ) ); }

	bool			FixDegenerateNormal( );	// fix degenerate axial cases
	bool			FixDenormals( );			// change tiny numbers to zero

	Vec3			Cross( const Vec3 &a ) const;
	Vec3 &			Cross( const Vec3 &a, const Vec3 &b );
	float			Length( ) const;
	float			LengthSqr( ) const;
	float			LengthFast( ) const;
	float			Normalize( );				// returns length
	float			NormalizeFast( );			// returns length
	Vec3 &			Truncate( float length );		// cap length
	void			Clamp( const Vec3 &min, const Vec3 &max );
	void			Snap( );					// snap to closest integer value
	void			SnapInt( );				// snap towards integer( floor )

	int				GetDimension( ) const { return 3; }

	float			ToYaw( ) const;
	float			ToPitch( ) const;
	Angles			ToAngles( void ) const;
	const float * 	ToFloatPtr( ) const;
	float * 		ToFloatPtr( );
	const Str	 	ToString( int precision = 2 ) const;
	class Mat3		ToMat3( void ) const;


	void			NormalVectors( Vec3 &left, Vec3 &down ) const;	// vector should be normalized
	void			OrthogonalBasis( Vec3 &left, Vec3 &up ) const;

	void			ProjectOntoPlane( const Vec3 &normal, const float overBounce = 1.0f );
	bool			ProjectAlongPlane( const Vec3 &normal, const float epsilon, const float overBounce = 1.0f );
	void			ProjectSelfOntoSphere( const float radius );

	void			Lerp( const Vec3 &v1, const Vec3 &v2, const float l );
	void			SLerp( const Vec3 &v1, const Vec3 &v2, const float l );

	void			AngleVectors( Vec3 * forward, Vec3 * right, Vec3 * up ) const;
	void			AnglesNormalize( );
	Vec3			Magnitude( float scale, Vec3 & v2 );
	void			CopyTo( float * dest );
	Vec3			Abs( ) const;

	Vec3 &			operator =( const VecT3f & a );
					operator VecT3f( ) const;
};

#endif
