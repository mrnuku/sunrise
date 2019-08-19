#ifndef PREDEFINES_H
#define PREDEFINES_H

#if defined _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#define __stosb( dest, fill_value, count ) __asm__( "rep stosb" : : "c" ( count ), "a" ( fill_value ), "D" ( dest ) : )
#define __stosd( dest, fill_value, count ) __asm__( "rep stosl" : : "c" ( count ), "a" ( fill_value ), "D" ( dest ) : )
#define __movsb( dest, src, count ) __asm__( "rep movsb" : : "c" ( count ), "S" ( src ), "D" ( dest ) : )
#define __movsd( dest, src, count ) __asm__( "rep movsl" : : "c" ( count ), "S" ( src ), "D" ( dest ) : )
#endif

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef unsigned int uint;
typedef unsigned __int64 qword;
typedef __m128i xmmword;
typedef __int64 timeTypeSystem;
typedef __int64 timeType;
typedef float timeTypeReal;

#define MEMCMP64( ptr1, ptr2 ) ( *( unsigned __int64 * )( ptr1 ) == *( unsigned __int64 * )( ptr2 ) )
#define MEMCMP32( ptr1, ptr2 ) ( *( unsigned __int32 * )( ptr1 ) == *( unsigned __int32 * )( ptr2 ) )
#define MEMCMP16( ptr1, ptr2 ) ( *( unsigned __int16 * )( ptr1 ) == *( unsigned __int16 * )( ptr2 ) )
#define MEMCMP8( ptr1, ptr2 )  ( *( unsigned __int8  * )( ptr1 ) == *( unsigned __int8  * )( ptr2 ) )

#define MEMCMPMASK64( ptr1, ptr2, mask ) ( ( *( unsigned __int64 * )( ptr1 ) & mask ) == ( *( unsigned __int64 * )( ptr2 ) & mask ) )
#define MEMCMPMASK32( ptr1, ptr2, mask ) ( ( *( unsigned __int32 * )( ptr1 ) & mask ) == ( *( unsigned __int32 * )( ptr2 ) & mask ) )
#define MEMCMPMASK16( ptr1, ptr2, mask ) ( ( *( unsigned __int16 * )( ptr1 ) & mask ) == ( *( unsigned __int16 * )( ptr2 ) & mask ) )
#define MEMCMPMASK8( ptr1, ptr2, mask )  ( ( *( unsigned __int8  * )( ptr1 ) & mask ) == ( *( unsigned __int8  * )( ptr2 ) & mask ) )

#define PATHSEPERATOR_STR "\\"
#define PATHSEPERATOR_CHAR '\\'
#define	MAX_STRING_CHARS 1024
#define BIT( num )( 1 <<( num ) )
#define IEEE_FLT_MANTISSA_BITS	23
#define IEEE_FLT_EXPONENT_BITS	8
#define IEEE_FLT_EXPONENT_BIAS	127
#define IEEE_FLT_SIGN_BIT		31
#define INTSIGNBITNOTSET( i )( ( ~( ( const unsigned long )( i ) ) ) >> 31 )
#define SEC2MS( t )( ( int )( ( t ) * 0.001f ) )
#define FLOAT_INF 1e30f

#ifdef __GNUC__
#define _alloca __builtin_alloca
#define INLINE inline
#define STATIC_TEMPLATE
#define id_attribute( x ) __attribute__( x )
#define ALIGN( x ) __attribute__ ( ( aligned ( x ) ) )
#define ENUM( type, name ) typedef type name; enum
#else
#define INLINE __forceinline
#define STATIC_TEMPLATE static
#define id_attribute( x )
#define ALIGN( x ) __declspec( align( x ) )
#define ENUM( type, name ) enum name : type
#endif

#define	PITCH	0
#define	YAW		1
#define	ROLL	2

#ifndef INFINITY
#define INFINITY	1e30f
#endif

#ifndef M_PI
#define M_PI		3.14159265358979323846f	// matches value in gcc v2 math.h
#endif

#define TWO_PI			( 2.0f * M_PI )
#define HALF_PI			( 0.5f * M_PI )
#define ONEFOURTH_PI	( 0.25f * M_PI )

#define SQRT_1OVER2 0.70710678118654752440f

#define DEG2RAD( a )	( ( a ) * ( M_PI / 180.0f ) )
#define RAD2DEG( a )	( ( a ) * ( 180.0f / M_PI ) )

#define NumSizeOf( type, num ) ( sizeof( type ) * ( num ) )
template<class T> INLINE T	Granulate( const T & gran, const T & size ) { T newsize = size + gran - 1; return newsize - ( newsize % gran ); }
template<class T> INLINE T	Pow2( const T & num ) { return ( ( T )1 << num ); }
template<class T> INLINE bool	IsPow2( const T & num ) { return ( num & ( num - 1 ) ) == 0 && num > 0; }
template<class T> INLINE T	InvPow2( const T & num ) {assert(IsPow2(num));for(int i=1;i<((sizeof(T)>>3)-1);i++)if((num<<i)&1)return i;}
template<class T> INLINE T	Max( T x, T y ) { return ( x > y ) ? x : y; }
template<class T> INLINE T	Min( T x, T y ) { return ( x < y ) ? x : y; }
template<class T> INLINE T &MaxRef( T & x, T & y ) { return ( x > y ) ? x : y; }
template<class T> INLINE T &MinRef( T & x, T & y ) { return ( x < y ) ? x : y; }
template< class T > INLINE void	Clamp( T & x, T min_val, T max_val ) { if( x < min_val ) x = min_val; if( x > max_val ) x = max_val; }
template<class T> INLINE T	Square( T x ) { return x * x; }
template<class T> INLINE int	Min3Index( T x, T y, T z ) { return ( x < y ) ? ( ( x < z ) ? 0 : 2 ) : ( ( y < z ) ? 1 : 2 ); }
template< class type > INLINE void Swap( type &a, type &b ) { type c = a; a = b; b = c; }

#define FLOATSIGNBITSET(f)		((*(const unsigned long *)&(f)) >> 31)
#define FLOATSIGNBITNOTSET(f)	((~(*(const unsigned long *)&(f))) >> 31)
#define FLOATNOTZERO(f)			((*(const unsigned long *)&(f)) & ~(1<<31) )

#define INTSIGNBITSET(i)		(((const unsigned long)(i)) >> 31)

#define _alloca16( x )			((void *)((((int)_alloca( (x)+15 )) + 15) & ~15))

#define	ANGLE2BYTE( x )			( ( int )( ( x ) * 256.0f / 360.0f ) & 255 )
#define	BYTE2ANGLE( x )			( ( x ) * ( 360.0f / 256.0f ) )

INLINE uint FloatCRC( const float & f ) {
	return *( uint * )&f;
}

INLINE uint StringCRC( const char * str ) {
	int crc = 0;
	const unsigned char * ptr = ( unsigned char * )str;
	for( int i = 0; str[ i ]; i++ )
		crc ^= str[ i ] << ( i & 3 );
	return crc;
}

#endif
