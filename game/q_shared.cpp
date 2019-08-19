#include "precompiled.h"
#pragma hdrstop

const Vec2		vec2_origin( 0.0f, 0.0f );
Vec3			vec3_origin( 0.0f, 0.0f, 0.0f );

const VecT2i	vecT2i_origin( 0, 0 );

//============================================================================

Vec3 RotatePointAroundVector( const Vec3 & dir, const Vec3 & point, float degrees ) {

	float	m[ 3 ][ 3 ];
	float	im[ 3 ][ 3 ];
	float	zrot[ 3 ][ 3 ];
	float	tmpmat[ 3 ][ 3 ];
	float	rot[ 3 ][ 3 ];
	int	i;
	Vec3 vr, vup, vf;
	Vec3 dst;

	vf[ 0 ] = dir[ 0 ];
	vf[ 1 ] = dir[ 1 ];
	vf[ 2 ] = dir[ 2 ];

	vr = PerpendicularVector( dir );
	vup = vr.Cross( vf );

	m[ 0 ][ 0 ] = vr[ 0 ];
	m[ 1 ][ 0 ] = vr[ 1 ];
	m[ 2 ][ 0 ] = vr[ 2 ];

	m[ 0 ][ 1 ] = vup[ 0 ];
	m[ 1 ][ 1 ] = vup[ 1 ];
	m[ 2 ][ 1 ] = vup[ 2 ];

	m[ 0 ][ 2 ] = vf[ 0 ];
	m[ 1 ][ 2 ] = vf[ 1 ];
	m[ 2 ][ 2 ] = vf[ 2 ];

	memcpy( im, m, sizeof( im ) );

	im[ 0 ][ 1 ] = m[ 1 ][ 0 ];
	im[ 0 ][ 2 ] = m[ 2 ][ 0 ];
	im[ 1 ][ 0 ] = m[ 0 ][ 1 ];
	im[ 1 ][ 2 ] = m[ 2 ][ 1 ];
	im[ 2 ][ 0 ] = m[ 0 ][ 2 ];
	im[ 2 ][ 1 ] = m[ 1 ][ 2 ];

	Common::Com_Memset( zrot, 0, sizeof( zrot ) );
	zrot[ 0 ][ 0 ] = zrot[ 1 ][ 1 ] = zrot[ 2 ][ 2 ] = 1.0F;

	zrot[ 0 ][ 0 ] = cos( DEG2RAD( degrees ) );
	zrot[ 0 ][ 1 ] = sin( DEG2RAD( degrees ) );
	zrot[ 1 ][ 0 ] = -sin( DEG2RAD( degrees ) );
	zrot[ 1 ][ 1 ] = cos( DEG2RAD( degrees ) );

	R_ConcatRotations( m, zrot, tmpmat );
	R_ConcatRotations( tmpmat, im, rot );

	for( i = 0; i < 3; i++ )
	{
		dst[ i ] = rot[ i ][ 0 ] * point[ 0 ] + rot[ i ][ 1 ] * point[ 1 ] + rot[ i ][ 2 ] * point[ 2 ];
	}

	return dst;
}

Vec3 ProjectPointOnPlane( const Vec3 & p, const Vec3 & normal ) {

	float d;
	Vec3 n;
	float inv_denom;
	Vec3 dst;

	inv_denom = 1.0f /( normal * normal );

	d =( normal * p ) * inv_denom;

	n[ 0 ] = normal[ 0 ] * inv_denom;
	n[ 1 ] = normal[ 1 ] * inv_denom;
	n[ 2 ] = normal[ 2 ] * inv_denom;

	dst[ 0 ] = p[ 0 ] - d * n[ 0 ];
	dst[ 1 ] = p[ 1 ] - d * n[ 1 ];
	dst[ 2 ] = p[ 2 ] - d * n[ 2 ];

	return dst;
}

/*
* * assumes "src" is normalized
*/
Vec3 PerpendicularVector( const Vec3 & src ) {

	int	pos;
	int i;
	float minelem = 1.0F;
	Vec3 tempvec;
	Vec3 dst;

	/*
	* * find the smallest magnitude axially aligned vector
	*/
	for( pos = 0, i = 0; i < 3; i++ )
	{
		if( fabs( src[ i ] ) < minelem )
		{
			pos = i;
			minelem = fabs( src[ i ] );
		}
	}
	tempvec[ 0 ] = tempvec[ 1 ] = tempvec[ 2 ] = 0.0F;
	tempvec[ pos ] = 1.0F;

	/*
	* * project the point onto the plane defined by src
	*/
	dst = ProjectPointOnPlane( tempvec, src );

	/*
	* * normalize the result
	*/
	dst.Normalize( );

	return dst;
}



/*
================
R_ConcatRotations
================
*/
void R_ConcatRotations( float in1[ 3 ][ 3 ], float in2[ 3 ][ 3 ], float out[ 3 ][ 3 ] ) {

	out[ 0 ][ 0 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 0 ] +
				in1[ 0 ][ 2 ] * in2[ 2 ][ 0 ];
	out[ 0 ][ 1 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 1 ] +
				in1[ 0 ][ 2 ] * in2[ 2 ][ 1 ];
	out[ 0 ][ 2 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 2 ] +
				in1[ 0 ][ 2 ] * in2[ 2 ][ 2 ];
	out[ 1 ][ 0 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 0 ] +
				in1[ 1 ][ 2 ] * in2[ 2 ][ 0 ];
	out[ 1 ][ 1 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 1 ] +
				in1[ 1 ][ 2 ] * in2[ 2 ][ 1 ];
	out[ 1 ][ 2 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 2 ] +
				in1[ 1 ][ 2 ] * in2[ 2 ][ 2 ];
	out[ 2 ][ 0 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 0 ] +
				in1[ 2 ][ 2 ] * in2[ 2 ][ 0 ];
	out[ 2 ][ 1 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 1 ] +
				in1[ 2 ][ 2 ] * in2[ 2 ][ 1 ];
	out[ 2 ][ 2 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 2 ] +
				in1[ 2 ][ 2 ] * in2[ 2 ][ 2 ];
}


/*
================
R_ConcatTransforms
================
*/
void R_ConcatTransforms( float in1[ 3 ][ 4 ], float in2[ 3 ][ 4 ], float out[ 3 ][ 4 ] ) {

	out[ 0 ][ 0 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 0 ] +
				in1[ 0 ][ 2 ] * in2[ 2 ][ 0 ];
	out[ 0 ][ 1 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 1 ] +
				in1[ 0 ][ 2 ] * in2[ 2 ][ 1 ];
	out[ 0 ][ 2 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 2 ] +
				in1[ 0 ][ 2 ] * in2[ 2 ][ 2 ];
	out[ 0 ][ 3 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 3 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 3 ] +
				in1[ 0 ][ 2 ] * in2[ 2 ][ 3 ] + in1[ 0 ][ 3 ];
	out[ 1 ][ 0 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 0 ] +
				in1[ 1 ][ 2 ] * in2[ 2 ][ 0 ];
	out[ 1 ][ 1 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 1 ] +
				in1[ 1 ][ 2 ] * in2[ 2 ][ 1 ];
	out[ 1 ][ 2 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 2 ] +
				in1[ 1 ][ 2 ] * in2[ 2 ][ 2 ];
	out[ 1 ][ 3 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 3 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 3 ] +
				in1[ 1 ][ 2 ] * in2[ 2 ][ 3 ] + in1[ 1 ][ 3 ];
	out[ 2 ][ 0 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 0 ] +
				in1[ 2 ][ 2 ] * in2[ 2 ][ 0 ];
	out[ 2 ][ 1 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 1 ] +
				in1[ 2 ][ 2 ] * in2[ 2 ][ 1 ];
	out[ 2 ][ 2 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 2 ] +
				in1[ 2 ][ 2 ] * in2[ 2 ][ 2 ];
	out[ 2 ][ 3 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 3 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 3 ] +
				in1[ 2 ][ 2 ] * in2[ 2 ][ 3 ] + in1[ 2 ][ 3 ];
}


//============================================================================


float Q_fabs( float f ) {

#if 0
	if( f >= 0 )
		return f;
	return -f;
#else
	int tmp = * ( int * ) &f;
	tmp &= 0x7FFFFFFF;
	return * ( float * ) &tmp;
#endif
}

/*
===============
LerpAngle

===============
*/
float LerpAngle( float a2, float a1, float frac ) {

	if( a1 - a2 > 180 )
		a1 -= 360;
	if( a1 - a2 < -180 )
		a1 += 360;
	return a2 + frac * ( a1 - a2 );
}


float	anglemod( float a ) {

#if 0
	if( a >= 0 )
		a -= 360* ( int )( a/360 );
	else
		a += 360* ( 1 +( int )( -a/360 ) );
#endif

	a = ( 360.0f / 65536.0f ) * ( ( int )( a * ( 65536.0f / 360.0f ) ) & 65535 );

	return a;
}

/*
==================
BoxOnPlaneSide

Returns 1, 2, or 1 + 2
==================
*/
int BoxOnPlaneSide( Vec3 & emins, Vec3 & emaxs, Plane & p ) {

	Box box( Bounds( emins, emaxs ) );

	return box.PlaneSide( p );

	/*float	dist1, dist2;
	int		sides;

	// fast axial cases
	if( p->type < 3 ) {

		if( p.Dist( ) <= emins[ p->type ] ) return 1;
		if( p.Dist( ) >= emaxs[ p->type ] ) return 2;
		return 3;
	}
	
	// general case
	switch( p->signbits ) {

	case 0:
		dist1 = p->normal[ 0 ]* emaxs[ 0 ] + p->normal[ 1 ]* emaxs[ 1 ] + p->normal[ 2 ]* emaxs[ 2 ];
		dist2 = p->normal[ 0 ]* emins[ 0 ] + p->normal[ 1 ]* emins[ 1 ] + p->normal[ 2 ]* emins[ 2 ];
		break;
	case 1:
		dist1 = p->normal[ 0 ]* emins[ 0 ] + p->normal[ 1 ]* emaxs[ 1 ] + p->normal[ 2 ]* emaxs[ 2 ];
		dist2 = p->normal[ 0 ]* emaxs[ 0 ] + p->normal[ 1 ]* emins[ 1 ] + p->normal[ 2 ]* emins[ 2 ];
		break;
	case 2:
		dist1 = p->normal[ 0 ]* emaxs[ 0 ] + p->normal[ 1 ]* emins[ 1 ] + p->normal[ 2 ]* emaxs[ 2 ];
		dist2 = p->normal[ 0 ]* emins[ 0 ] + p->normal[ 1 ]* emaxs[ 1 ] + p->normal[ 2 ]* emins[ 2 ];
		break;
	case 3:
		dist1 = p->normal[ 0 ]* emins[ 0 ] + p->normal[ 1 ]* emins[ 1 ] + p->normal[ 2 ]* emaxs[ 2 ];
		dist2 = p->normal[ 0 ]* emaxs[ 0 ] + p->normal[ 1 ]* emaxs[ 1 ] + p->normal[ 2 ]* emins[ 2 ];
		break;
	case 4:
		dist1 = p->normal[ 0 ]* emaxs[ 0 ] + p->normal[ 1 ]* emaxs[ 1 ] + p->normal[ 2 ]* emins[ 2 ];
		dist2 = p->normal[ 0 ]* emins[ 0 ] + p->normal[ 1 ]* emins[ 1 ] + p->normal[ 2 ]* emaxs[ 2 ];
		break;
	case 5:
		dist1 = p->normal[ 0 ]* emins[ 0 ] + p->normal[ 1 ]* emaxs[ 1 ] + p->normal[ 2 ]* emins[ 2 ];
		dist2 = p->normal[ 0 ]* emaxs[ 0 ] + p->normal[ 1 ]* emins[ 1 ] + p->normal[ 2 ]* emaxs[ 2 ];
		break;
	case 6:
		dist1 = p->normal[ 0 ]* emaxs[ 0 ] + p->normal[ 1 ]* emins[ 1 ] + p->normal[ 2 ]* emins[ 2 ];
		dist2 = p->normal[ 0 ]* emins[ 0 ] + p->normal[ 1 ]* emaxs[ 1 ] + p->normal[ 2 ]* emaxs[ 2 ];
		break;
	case 7:
		dist1 = p->normal[ 0 ]* emins[ 0 ] + p->normal[ 1 ]* emins[ 1 ] + p->normal[ 2 ]* emins[ 2 ];
		dist2 = p->normal[ 0 ]* emaxs[ 0 ] + p->normal[ 1 ]* emaxs[ 1 ] + p->normal[ 2 ]* emaxs[ 2 ];
		break;
	default:
		dist1 = dist2 = 0;		// shut up compiler
		assert( 0 );
		break;
	}

	sides = 0;
	if( dist1 >= p.Dist( ) ) sides = 1;
	if( dist2 < p.Dist( ) ) sides |= 2;

	assert( sides != 0 );

	return sides;*/
}

void AddPointToBounds( Vec3 & v, Vec3 * mins, Vec3 * maxs ) {

	for( int i = 0; i < 3; i++ ) {

		float val = v[ i ];
		if( val < mins->ToFloatPtr( )[ i ] ) mins->ToFloatPtr( )[ i ] = val;
		if( val > maxs->ToFloatPtr( )[ i ] ) maxs->ToFloatPtr( )[ i ] = val;
	}
}

int Q_log2( int val ) {

	int answer = 0;
	while( val>>= 1 )
		answer++;
	return answer;
}

//====================================================================================

/*
============================================================================

					BYTE ORDER FUNCTIONS

============================================================================
*/

bool	bigendien;

// can't just use function pointers, or dll linkage can
// mess up when qcommon is included in multiple places
short	( *_BigShort )( short l );
short	( *_LittleShort )( short l );
int		( *_BigLong )( int l );
int		( *_LittleLong )( int l );
float	( *_BigFloat )( float l );
float	( *_LittleFloat )( float l );

short	BigShort( short l ){return _BigShort( l );}
short	LittleShort( short l ) {return _LittleShort( l );}
int		BigLong( int l ) {return _BigLong( l );}
int		LittleLong( int l ) {return _LittleLong( l );}
float	BigFloat( float l ) {return _BigFloat( l );}
float	LittleFloat( float l ) {return _LittleFloat( l );}

short   ShortSwap( short l ) {

	byte    b1, b2;

	b1 = l & 255;
	b2 = ( l >> 8 ) & 255;

	return ( b1 << 8 ) + b2;
}

short	ShortNoSwap( short l ) {

	return l;
}

int    LongSwap( int l ) {

	byte    b1, b2, b3, b4;

	b1 = l & 255;
	b2 = ( l >> 8 ) & 255;
	b3 = ( l >> 16 ) & 255;
	b4 = ( l >> 24 ) & 255;

	return( ( int )b1 << 24 ) +( ( int )b2 << 16 ) +( ( int )b3 << 8 ) + b4;
}

int	LongNoSwap( int l ) {

	return l;
}

float FloatSwap( float f ) {

	union {

		float	f;
		byte	b[ 4 ];

	} dat1, dat2;
	
	
	dat1.f = f;
	dat2.b[ 0 ] = dat1.b[ 3 ];
	dat2.b[ 1 ] = dat1.b[ 2 ];
	dat2.b[ 2 ] = dat1.b[ 1 ];
	dat2.b[ 3 ] = dat1.b[ 0 ];

	return dat2.f;
}

float FloatNoSwap( float f ) {

	return f;
}

/*
================
Swap_Init
================
*/
void Swap_Init( ) {

	byte	swaptest[ 2 ] = { 1, 0 };

// set the byte swapping variables in a portable manner	
	if( *( short * )swaptest == 1 )
	{
		bigendien = false;
		_BigShort = ShortSwap;
		_LittleShort = ShortNoSwap;
		_BigLong = LongSwap;
		_LittleLong = LongNoSwap;
		_BigFloat = FloatSwap;
		_LittleFloat = FloatNoSwap;
	}
	else
	{
		bigendien = true;
		_BigShort = ShortNoSwap;
		_LittleShort = ShortSwap;
		_BigLong = LongNoSwap;
		_LittleLong = LongSwap;
		_BigFloat = FloatNoSwap;
		_LittleFloat = FloatSwap;
	}

}



/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday
============
*/
const Str va( const char * format, ... ) {

	va_list		argptr;
	Str			string;
	
	va_start( argptr, format );
	vsprintf( string, format, argptr );
	va_end( argptr );

	return string;
}

char	com_token[ MAX_TOKEN_CHARS ];

/*
==============
COM_Parse

Parse a token out of a string
==============
*/
char * COM_Parse( char ** data_p ) {

	int		c;
	int		len;
	char	* data;

	data = * data_p;
	len = 0;
	com_token[ 0 ] = 0;
	
	if( !data )
	{
		* data_p = NULL;
		return "";
	}
		
// skip whitespace
skipwhite:
	while( ( c = * data ) <= ' ' )
	{
		if( c == 0 )
		{
			* data_p = NULL;
			return "";
		}
		data++;
	}
	
// skip // comments
	if( c =='/' && data[ 1 ] == '/' )
	{
		while( *data && * data != '\n' )
			data++;
		goto skipwhite;
	}

// handle quoted strings specially
	if( c == '\"' )
	{
		data++;
		while( 1 )
		{
			c = * data++;
			if( c =='\"' || !c )
			{
				com_token[ len ] = 0;
				* data_p = data;
				return com_token;
			}
			if( len < MAX_TOKEN_CHARS )
			{
				com_token[ len ] = c;
				len++;
			}
		}
	}

// parse a regular word
	do
	{
		if( len < MAX_TOKEN_CHARS )
		{
			com_token[ len ] = c;
			len++;
		}
		data++;
		c = * data;
	} while( c>32 );

	if( len == MAX_TOKEN_CHARS )
	{
//		Common::Com_Printf( "Token exceeded %i chars, discarded.\n", MAX_TOKEN_CHARS );
		len = 0;
	}
	com_token[ len ] = 0;

	* data_p = data;
	return com_token;
}


/*
===============
Com_PageInMemory

===============
*/
int	paged_total;

void Com_PageInMemory( byte * buffer, int size ) {

	int		i;

	for( i = size-1; i>0; i-= 4096 )
		paged_total += buffer[ i ];
}

/*
=====================================================================

  INFO STRINGS

=====================================================================
*/

/*
==================
Info_Validate

Some characters are illegal in info strings because they
can mess up the server's parsing
==================
*/
bool ClientServerInfo::Info_Validate( const Str & s ) {

	if( s.Find( '"' ) != -1 && s.Find( ';' ) != -1 ) return false;
	return true;
}

/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
===============
*/
const Str ClientServerInfo::Info_ValueForKey( const Str & s, const Str & key ) {
	if( !s )
		return null_string;
	// TP - info strings layout: "var 1" = "value 1", "var 2" = "value 2"
	Lexer lexer( s.c_str( ), s.Length( ), "ClientServerInfo::Info_ValueForKey", LEXFL_NOSTRINGCONCAT | LEXFL_ALLOWPATHNAMES | LEXFL_ALLOWIPADDRESSES );
	Token name, val;
	while( 1 ) {
		lexer.ReadToken( &name );
		lexer.ExpectTokenType( TT_PUNCTUATION, P_ASSIGN, &val );
		lexer.ReadToken( &val );
		if( name == key )
			return val;
		if( lexer.EndOfFile( ) )
			break;
		lexer.ExpectTokenType( TT_PUNCTUATION, P_COMMA, &val );
	}
	return null_string;
}

void ClientServerInfo::Info_SetValueForKey( Str & s, const Str & key, const Str & value ) {

	if( value.IsEmpty( ) || !Info_Validate( key ) || !Info_Validate( value ) ) return;
	if( s.IsEmpty( ) ) {
		
		sprintf( s, "%s = %s", key.c_str( ), value.c_str( ) );
		return;
	}

	Lexer lexer( s.c_str( ), s.Length( ), "ClientServerInfo::Info_ValueForKey", LEXFL_NOSTRINGCONCAT | LEXFL_ALLOWPATHNAMES );

	bool found = false;
	Str build, news;
	Token name, val;
	int index = 0;

	while( 1 ) {

		lexer.ReadToken( &name );
		lexer.ExpectTokenType( TT_PUNCTUATION, P_ASSIGN, &val );
		lexer.ReadToken( &val );

		sprintf( build, "%s%s = %s", index++ ? ", " : "", name.c_str( ), ( name == key ) ? ( found = true, value.c_str( ) ) : val.c_str( ) );
		news.Append( build );

		if( lexer.EndOfFile( ) ) break;
		lexer.ExpectTokenType( TT_PUNCTUATION, P_COMMA, &val );
	}

	if( !found ) {

		sprintf( build, ", %s = %s", key.c_str( ), value.c_str( ) );
		s.Append( build );

	} else s = news;
}

void ClientServerInfo::Info_Print( const Str & s ) {

		Common::Com_Printf( "%s\n", s.c_str( ) );
}

//====================================================================


