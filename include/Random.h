// Copyright ( C) 2004 Id Software, Inc.
//

#ifndef __MATH_RANDOM_H__
#define __MATH_RANDOM_H__

/*
===============================================================================

	Random number generator

===============================================================================
*/

class Random {
public:
						Random( int seed = 0 );

	void				SetSeed( int seed );
	int					GetSeed( ) const;

	int					RandomInt( );			// random integer in the range [0, MAX_RAND]
	int					RandomInt( int max );		// random integer in the range [0, max[
	float				RandomFloat( );		// random number in the range [0.0f, 1.0f]
	float				CRandomFloat( );		// random number in the range [-1.0f, 1.0f]

	static const int	MAX_RAND = 0x7FFF;

private:
	int					seed;
};

INLINE Random::Random( int seed ) {
	this->seed = seed;
}

INLINE void Random::SetSeed( int seed ) {
	this->seed = seed;
}

INLINE int Random::GetSeed( ) const {
	return seed;
}

INLINE int Random::RandomInt( ) {
	seed = 69069 * seed + 1;
	return ( seed & Random::MAX_RAND );
}

INLINE int Random::RandomInt( int max ) {
	if ( max == 0 ) {
		return 0;			// avoid divide by zero error
	}
	return RandomInt( ) % max;
}

INLINE float Random::RandomFloat( ) {
	return ( RandomInt( ) / ( float )( Random::MAX_RAND + 1 ) );
}

INLINE float Random::CRandomFloat( ) {
	return ( 2.0f * ( RandomFloat( ) - 0.5f ) );
}


/*
===============================================================================

	Random number generator

===============================================================================
*/

class Random2 {
public:
							Random2( unsigned long seed = 0 );

	void					SetSeed( unsigned long seed );
	unsigned long			GetSeed( ) const;

	int						RandomInt( );			// random integer in the range [0, MAX_RAND]
	int						RandomInt( int max );		// random integer in the range [0, max]
	float					RandomFloat( );		// random number in the range [0.0f, 1.0f]
	float					CRandomFloat( );		// random number in the range [-1.0f, 1.0f]
	static int				ILog2( float f );
	static int				BitsForInteger( int i );

	static const int		MAX_RAND = 0x7FFF;

private:
	unsigned long			seed;

	static const unsigned long	IEEE_ONE = 0x3F800000;
	static const unsigned long	IEEE_MASK = 0x007FFFFF;
};

INLINE int Random2::ILog2( float f ) {
	return ( ( ( *reinterpret_cast<int *>( &f) ) >> IEEE_FLT_MANTISSA_BITS ) & ( ( 1 << IEEE_FLT_EXPONENT_BITS ) - 1 ) ) - IEEE_FLT_EXPONENT_BIAS;
}

INLINE int Random2::BitsForInteger( int i ) {
	return ILog2( ( float)i ) + 1;
}

INLINE Random2::Random2( unsigned long seed ) {
	this->seed = seed;
}

INLINE void Random2::SetSeed( unsigned long seed ) {
	this->seed = seed;
}

INLINE unsigned long Random2::GetSeed( ) const {
	return seed;
}

INLINE int Random2::RandomInt( ) {
	seed = 1664525L * seed + 1013904223L;
	return ( ( int) seed & Random2::MAX_RAND );
}

INLINE int Random2::RandomInt( int max ) {
	if ( max == 0 ) {
		return 0;		// avoid divide by zero error
	}
	return ( RandomInt( ) >> ( 16 - BitsForInteger( max ) ) ) % max;
}

INLINE float Random2::RandomFloat( ) {
	unsigned long i;
	seed = 1664525L * seed + 1013904223L;
	i = Random2::IEEE_ONE | ( seed & Random2::IEEE_MASK );
	return ( ( *( float *)&i ) - 1.0f );
}

INLINE float Random2::CRandomFloat( ) {
	unsigned long i;
	seed = 1664525L * seed + 1013904223L;
	i = Random2::IEEE_ONE | ( seed & Random2::IEEE_MASK );
	return ( 2.0f * ( *( float *)&i ) - 3.0f );
}

#endif /* !__MATH_RANDOM_H__ */
