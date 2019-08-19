#include "precompiled.h"
#pragma hdrstop

const char * units[ 2 ][ 5 ] = {
	{ "B", "Kb", "Mb", "Gb", "Tb" },
	{ "bps", "Kbps", "Mbps", "Gbps", "Tbps" }
};

void Str::ReAllocate( int amount, bool keepold ) {
	assert( d_data && amount > 0 );
	char * newbuffer;
	if( amount > STR_ALLOC_BASE ) {
		int mod = amount % STR_ALLOC_GRAN;
		int newsize = amount + STR_ALLOC_GRAN - mod;
		d_alloced = newsize;
#ifdef USE_STRING_DATA_ALLOCATOR
		newbuffer = g_stringDataAllocator.Alloc( d_alloced );
#else
		newbuffer = new char[ d_alloced ];
#endif
	} else {
		newbuffer = d_baseBuffer;
		d_alloced = STR_ALLOC_BASE;
	}
	if( keepold ) {
		d_len = Min( d_alloced - 1, d_len );
		Copynz( newbuffer, d_data, d_len + 1 );
	} else {
		d_len = 0;
		newbuffer[ 0 ] = 0;
	}
	FreeData( );
	d_data = newbuffer;
}

void Str::FreeData( ) {
	assert( d_data );
	if( d_data != d_baseBuffer ) {
#ifdef USE_STRING_DATA_ALLOCATOR
		g_stringDataAllocator.Free( d_data );
#else
		delete[ ] d_data;
#endif
	}
}

bool Str::BlockCmp( const char * a, const char * b, int len ) {
	for( int left, offs = 0; ( left = len - offs ); ) {
#if defined (_M_AMD64) || defined (_M_X64)
		if( left >> 3 ) {
			if( !MEMCMP64( a + offs, b + offs ) )
				return false;
			offs += 8;
		} else
#endif
		if( left >> 2 ) {
			if( !MEMCMP32( a + offs, b + offs ) )
				return false;
			offs += 4;
		} else if( left ) {
			if( !MEMCMP8( a + offs, b + offs ) )
				return false;
			offs += 1;
		}
	}
	return true;
}

bool Str::IBlockCmp( const char * a, const char * b, int len ) {
	for( int left, offs = 0; ( left = len - offs ); ) {
#if defined (_M_AMD64) || defined (_M_X64)
		if( left >> 3 ) {
			if( !MEMCMPMASK64( a + offs, b + offs, MASKICMP64 ) )
				return false;
			offs += 8;
		} else
#endif
		if( left >> 2 ) {
			if( !MEMCMPMASK32( a + offs, b + offs, MASKICMP32 ) )
				return false;
			offs += 4;
		} else if( left ) {
			if( !MEMCMPMASK8( a + offs, b + offs, MASKICMP8 ) )
				return false;
			offs += 1;
		}
	}
	return true;
}

int Str::BlockFindText( const char * str, int str_len, const char * text, int text_len, int start_offs ) {
	int checkEnd = ( str_len - text_len ) + 1;
	if( text_len < 4 ) {
		bool do16c = ( text_len >> 1 ) > 0, do8c = ( text_len & 1 ) > 0;
		int offs8c = text_len & 2;
		for( int i = start_offs; i < checkEnd; i++ ) {
			if( do16c && !MEMCMP16( str + i, text ) )
				continue;
			if( do8c && !MEMCMP8( str + i + offs8c, text + offs8c ) )
				continue;
			return i;
		}
	} else {
		for( int i = start_offs; i < checkEnd; i++ ) {
			if( BlockCmp( str + i, text, text_len ) )
				return i;
		}
	}
	return -1;
}

int Str::IBlockFindText( const char * str, int str_len, const char * text, int text_len, int start_offs ) {
	int checkEnd = ( str_len - text_len ) + 1;
	if( text_len < 4 ) {
		bool do16c = ( text_len >> 1 ) > 0, do8c = ( text_len & 1 ) > 0;
		int offs8c = text_len & 2;
		for( int i = start_offs; i < checkEnd; i++ ) {
			if( do16c && !MEMCMPMASK16( str + i, text, MASKICMP16 ) )
				continue;
			if( do8c && !MEMCMPMASK8( str + i + offs8c, text + offs8c, MASKICMP8 ) )
				continue;
			return i;
		}
	} else {
		for( int i = start_offs; i < checkEnd; i++ ) {
			if( IBlockCmp( str + i, text, text_len ) )
				return i;
		}
	}
	return -1;
}

void Str::operator =( const char * text ) {
	if( !text ) {
		// safe behaviour if NULL
		EnsureAlloced( 1, false );
		d_data[ 0 ] = 0;
		d_len = 0;
		return;
	}
	if( text == d_data )
		return; // copying same thing
	// check if we're aliasing
	if( text >= d_data && text <= d_data + d_len ) {
		int diff = ( int )( text - d_data );
		assert( Length( text ) < d_len );
		int i;
		for( i = 0; text[ i ]; i++ )
			d_data[ i ] = text[ i ];
		d_data[ i ] = 0;
		d_len -= diff;
		return;
	}
	int l = Length( text );
	EnsureAlloced( l + 1, false );
	Copynz( d_data, text, l + 1 );
	d_len = l;
}

int Str::FindChar( const char * str, const char c, int start, int end ) {
	if( end == -1 )
		end = Length( str ) - 1;
	for( int i = start; i <= end; i++ ) {
		if( str[ i ] == c )
			return i;
	}
	return -1;
}

int Str::FindText( const char * str, const char * text, int start, int end ) {
	if( end == -1 )
		end = Length( str );
	int l = end - Length( text );
	for( int i = start, j; i <= l; i++ ) {
		for( j = 0; text[ j ]; j++ ) {
			if( str[ i + j ] != text[ j ] )
				break;
		}
		if( !text[ j ] )
			return i;
	}
	return -1;
}

int Str::IfindText( const char * str, const char * text, int start, int end ) {
	if( end == -1 )
		end = Length( str );
	int l = end - Length( text );
	for( int i = start, j; i <= l; i++ ) {
		for( j = 0; text[ j ]; j++ ) {
			if( ::toupper( str[ i+j ] ) != ::toupper( text[ j ] ) )
				break;
		}
		if( !text[ j ] )
			return i;
	}
	return -1;
}

bool Str::Filter( const char * filter, const char * name, bool casesensitive ) {
	while( *filter ) {
		if( *filter == '*' ) {
			filter++;
			Str buf;
			for( int i = 0; *filter; i++ ) {
				if( *filter == '*' || * filter == '?' ||( *filter == '[' && * ( filter+1 ) != '[' ) )
					break;
				buf += *filter;
				if( *filter == '[' )
					filter++;
				filter++;
			}
			if( buf.Length( ) ) {
				int index = Str( name ).Find( buf.c_str( ), casesensitive );
				if( index == -1 )
					return false;
				name += index + Length( buf );
			}
		} else if( *filter == '?' ) {
			filter++;
			name++;
		} else if( *filter == '[' ) {
			if( *( filter + 1 ) == '[' ) {
				if( *name != '[' )
					return false;
				filter += 2;
				name++;
			} else {
				filter++;
				bool found = false;
				while( *filter && !found ) {
					if( *filter == ']' && * ( filter+1 ) != ']' )
						break;
					if( *( filter+1 ) == '-' && * ( filter+2 ) &&( *( filter+2 ) != ']' || * ( filter+3 ) == ']' ) ) {
						if( casesensitive ) {
							if( *name >= * filter && * name <= * ( filter+2 ) )
								found = true;
						} else {
							if( ::toupper( *name ) >= ::toupper( *filter ) && ::toupper( *name ) <= ::toupper( *( filter+2 ) ) )
								found = true;
						}
						filter += 3;
					} else {
						if( casesensitive ) {
							if( *filter == * name )
								found = true;
						} else {
							if( ::toupper( *filter ) == ::toupper( *name ) )
								found = true;
						}
						filter++;
					}
				}
				if( !found )
					return false;
				while( *filter ) {
					if( *filter == ']' && * ( filter+1 ) != ']' )
						break;
					filter++;
				}
				filter++;
				name++;
			}
		} else {
			if( casesensitive ) {
				if( *filter != * name )
					return false;
			} else {
				if( ::toupper( *filter ) != ::toupper( *name ) )
					return false;
			}
			filter++;
			name++;
		}
	}
	return true;
}

void Str::StripMediaName( const char * name, Str & mediaName ) {
	mediaName.Empty( );
	for( char c = *name; c; c = *( ++name ) ) {
		if( c == '.' ) // truncate at an extension
			break;
		if( c == '\\' ) // convert backslashes to forward slashes
			mediaName.Append( '/' );
		else
			mediaName.Append( Str::ToLower( c ) );
	}
}

bool Str::CheckExtension( const char * name, const char * ext ) {
	const char * s1 = name + Length( name ) - 1;
	const char * s2 = ext + Length( ext ) - 1;
	do {
		int c1 = *s1--;
		int c2 = *s2--;
		int d = c1 - c2;
		while( d ) {
			if( c1 <= 'Z' && c1 >= 'A' ) {
				d +=( 'a' - 'A' );
				if( !d )
					break;
			}
			if( c2 <= 'Z' && c2 >= 'A' ) {
				d -=( 'a' - 'A' );
				if( !d )
					break;
			}
			return false;
		}
	} while( s1 > name && s2 > ext );
	return( s1 >= name );
}

const char * Str::FloatArrayToString( const float * farray, const int length, const int precision ) {
	static int			index = 0;
	static char			str[ 4 ][ 16384 ];	// in case called by nested functions
	// use an array of string so that multiple calls won't collide
	char * s = str[ index ];
	index = ( index + 1 ) & 3;
	Str format( " %." );
	format += precision;
	format += 'f';
	int n = 0;
	for( int i = 0; i < length; i++ ) {
		n += snPrintf( s + n, sizeof( str[ 0 ] ) - n, i ? format.d_data : format.d_data + 1, farray[ i ] );
		if( precision > 0 )
			StripFloatZeros( s, n );
	}
	return s;
}

void Str::Replace( const char * old, const char * nw ) {
	Str	oldString( d_data );
	int oldLen = Length( old );
	int newLen = Length( nw );
	// Work out how big the new string will be
	int count = 0;
	for( int i = 0; i < oldString.Length( ); i++ ) {
		if( !Str::Cmpn( &oldString[ i ], old, oldLen ) ) {
			count++;
			i += oldLen - 1;
		}
	}
	if( count ) {
		EnsureAlloced( d_len +( ( newLen - oldLen ) * count ) + 2, false );
		// Replace the old data with the new data
		int j = 0;
		for( int i = 0; i < oldString.Length( ); i++ ) {
			if( !Str::Cmpn( &oldString[ i ], old, oldLen ) ) {
				Common::Com_Memcpy( d_data + j, nw, newLen );
				i += oldLen - 1;
				j += newLen;
			} else {
				d_data[ j ] = oldString[ i ];
				j++;
			}
		}
		d_data[ j ] = 0;
		d_len = Length( d_data );
	}
}

const char * Str::Mid( int start, int len, Str & result ) const {
	result.Empty( );
	int i = d_len;
	if( i == 0 || len <= 0 || start >= i )
		return NULL;
	if( start + len >= i )
		len = i - start;
	result.Append( &d_data[ start ], len );
	return result;
}

Str Str::Mid( int start, int len ) const {
	int i = d_len;
	if( i == 0 || len <= 0 || start >= i )
		return null_string;

	if( start + len >= i ) {
		len = i - start;
	}
	Str result;
	result.Append( &d_data[ start ], len );
	return result;
}

Str & Str::StripQuotes( ) {
	if( d_data[ 0 ] != '\"' )
		return *this;	
	// Remove the trailing quote first
	if( d_data[ d_len - 1 ] == '\"' ) {
		d_data[ d_len - 1 ] = 0;
		d_len--;
	}
	// Strip the leading quote now
	d_len--;	
	memmove( &d_data[ 0 ], &d_data[ 1 ], d_len );
	d_data[ d_len ] = 0;	
	return *this;
}

int Str::FileNameHash( ) const {
	long hash = 0;
	int i = 0;
	while( d_data[ i ] != 0 ) {
		char letter = Str::ToLower( d_data[ i ] );
		if( letter == '.' )
			break;				// don't include extension
		if( letter =='\\' )
			letter = '/';
		hash += ( long )( letter ) * ( i + 119 );
		i++;
	}
	hash &= ( FILE_HASH_SIZE - 1 );
	return hash;
}

void Str::AppendPath( const char * text ) {
	int i = 0;
	if( text && text[ i ] ) {
		int pos = d_len;
		EnsureAlloced( d_len + Length( text ) + 2 );
		if( pos ) {
			if( d_data[ pos - 1 ] != '/' )
				d_data[ pos++ ] = '/';
		}
		if( text[ i ] == '/' )
			i++;
		for( ; text[ i ]; i++ ) {
			if( text[ i ] == '\\' )
				d_data[ pos++ ] = '/';
			else
				d_data[ pos++ ] = text[ i ];
		}
		d_len = pos;
		d_data[ pos ] = 0;
	}
}

int Str::Cmp( const char * s1, const char * s2 ) {
	int c1;
	do {
		c1 = *s1++;
		int c2 = *s2++;
		int d = c1 - c2;
		if( d )
			return( INTSIGNBITNOTSET( d ) << 1 ) - 1;
	} while( c1 );
	return 0;		// strings are equal
}

int Str::Cmpn( const char * s1, const char * s2, int n ) {
	assert( n >= 0 );
	int c1;
	do {
		c1 = *s1++;
		int c2 = *s2++;
		if( !n-- )
			return 0;		// strings are equal until end point
		int d = c1 - c2;
		if( d )
			return( INTSIGNBITNOTSET( d ) << 1 ) - 1;
	} while( c1 );
	return 0;		// strings are equal
}

int Str::Icmp( const char * s1, const char * s2 ) {
	int c1;
	do {
		c1 = *s1++;
		int c2 = *s2++;
		int d = c1 - c2;
		while( d ) {
			if( c1 <= 'Z' && c1 >= 'A' ) {
				d += ( 'a' - 'A' );
				if( !d )
					break;
			}
			if( c2 <= 'Z' && c2 >= 'A' ) {
				d -= ( 'a' - 'A' );
				if( !d )
					break;
			}
			return( INTSIGNBITNOTSET( d ) << 1 ) - 1;
		}
	} while( c1 );
	return 0;		// strings are equal
}

int Str::Icmpn( const char * s1, const char * s2, int n ) {
	assert( n >= 0 );
	int c1;
	do {
		c1 = *s1++;
		int c2 = *s2++;
		if( !n-- )
			return 0;		// strings are equal until end point
		int d = c1 - c2;
		while( d ) {
			if( c1 <= 'Z' && c1 >= 'A' ) {
				d += ( 'a' - 'A' );
				if( !d )
					break;
			}
			if( c2 <= 'Z' && c2 >= 'A' ) {
				d -= ( 'a' - 'A' );
				if( !d )
					break;
			}
			return( INTSIGNBITNOTSET( d ) << 1 ) - 1;
		}
	} while( c1 );
	return 0;		// strings are equal
}

int Str::IcmpNoColor( const char * s1, const char * s2 ) {
	int c1;
	do {
		while( Str::IsColor( s1 ) )
			s1 += 2;
		while( Str::IsColor( s2 ) )
			s2 += 2;
		c1 = *s1++;
		int c2 = *s2++;
		int d = c1 - c2;
		while( d ) {
			if( c1 <= 'Z' && c1 >= 'A' ) {
				d += ( 'a' - 'A' );
				if( !d )
					break;
			}
			if( c2 <= 'Z' && c2 >= 'A' ) {
				d -= ( 'a' - 'A' );
				if( !d )
					break;
			}
			return( INTSIGNBITNOTSET( d ) << 1 ) - 1;
		}
	} while( c1 );
	return 0;		// strings are equal
}

int Str::IcmpPath( const char * s1, const char * s2 ) {
	int c1;
#if 0
//#if !defined( _WIN32 )
	Lib::common->Printf( "WARNING: IcmpPath used on a case-sensitive filesystem?\n" );
#endif
	do {
		c1 = *s1++;
		int c2 = *s2++;
		int d = c1 - c2;
		while( d ) {
			if( c1 <= 'Z' && c1 >= 'A' ) {
				d += ( 'a' - 'A' );
				if( !d )
					break;
			}
			if( c1 == '\\' ) {
				d += ( '/' - '\\' );
				if( !d )
					break;
			}
			if( c2 <= 'Z' && c2 >= 'A' ) {
				d -= ( 'a' - 'A' );
				if( !d )
					break;
			}
			if( c2 == '\\' ) {
				d -= ( '/' - '\\' );
				if( !d )
					break;
			}
			// make sure folders come first
			while( c1 ) {
				if( c1 == '/' || c1 == '\\' )
					break;
				c1 = * s1++;
			}
			while( c2 ) {
				if( c2 == '/' || c2 == '\\' )
					break;
				c2 = * s2++;
			}
			if( c1 && !c2 )
				return -1;
			else if( !c1 && c2 )
				return 1;
			// same folder depth so use the regular compare
			return( INTSIGNBITNOTSET( d ) << 1 ) - 1;
		}
	} while( c1 );
	return 0;
}

int Str::IcmpnPath( const char * s1, const char * s2, int n ) {
	assert( n >= 0 );
	int c1;
#if 0
//#if !defined( _WIN32 )
	Lib::common->Printf( "WARNING: IcmpPath used on a case-sensitive filesystem?\n" );
#endif
	do {
		c1 = *s1++;
		int c2 = *s2++;
		if( !n-- )
			return 0;		// strings are equal until end point
		int d = c1 - c2;
		while( d ) {
			if( c1 <= 'Z' && c1 >= 'A' ) {
				d += ( 'a' - 'A' );
				if( !d )
					break;
			}
			if( c1 == '\\' ) {
				d += ( '/' - '\\' );
				if( !d )
					break;
			}
			if( c2 <= 'Z' && c2 >= 'A' ) {
				d -= ( 'a' - 'A' );
				if( !d )
					break;
			}
			if( c2 == '\\' ) {
				d -= ( '/' - '\\' );
				if( !d )
					break;
			}
			// make sure folders come first
			while( c1 ) {
				if( c1 == '/' || c1 == '\\' )
					break;
				c1 = * s1++;
			}
			while( c2 ) {
				if( c2 == '/' || c2 == '\\' )
					break;
				c2 = * s2++;
			}
			if( c1 && !c2 )
				return -1;
			else if( !c1 && c2 )
				return 1;
			// same folder depth so use the regular compare
			return( INTSIGNBITNOTSET( d ) << 1 ) - 1;
		}
	} while( c1 );
	return 0;
}

int Str::LengthWithoutColors( const char * s ) {
	if( !s )
		return 0;
	int len = 0;
	const char * p = s;
	while( *p ) {
		if( Str::IsColor( p ) ) {
			p += 2;
			continue;
		}
		p++;
		len++;
	}
	return len;
}

char * Str::RemoveColors( char * string ) {
	int c;
	char * s = string;
	char * d = string;
	while( ( c = * s ) != 0 ) {
		if( Str::IsColor( s ) )
			s++;
		else
			*d++ = c;
		s++;
	}
	*d = 0;
	return string;
}

int Str::snPrintf( char * dest, int size, const char * fmt, ... ) {
	va_list argptr;
	char buffer[ 32000 ];	// big, but small enough to fit in PPC stack
	va_start( argptr, fmt );
	int len = vsprintf( buffer, fmt, argptr );
	va_end( argptr );
	if( len >= sizeof( buffer ) )
		Common::Com_Error( ERR_DISCONNECT, "Str::snPrintf: overflowed buffer" );
	if( len >= size ) {
		Common::Com_DPrintf( "Str::snPrintf: overflow of %i in %i\n", len, size );
		len = size;
	}
	Str::Copynz( dest, buffer, size );
	return len;
}

/*
============
Str::vsnPrintf

vsnprintf portability:

C99 standard: vsnprintf returns the number of characters( excluding the trailing
'\0' ) which would have been written to the final string if enough space had been available
snprintf and vsnprintf do not write more than size bytes( including the trailing '\0' )

win32: _vsnprintf returns the number of characters written, not including the terminating null character, or a negative value if an output error occurs. If the number of characters to write exceeds count, then count 
characters are written and -1 is returned and no trailing '\0' is added.

Str::vsnPrintf: always appends a trailing '\0', returns number of characters written( not including terminal \0 )
or returns -1 on failure or if the buffer would be overflowed.
============
*/
int Str::vsnPrintf( char * dest, int size, const char * fmt, va_list argptr ) {
#ifdef _WIN32
#undef _vsnprintf
	int ret = _vsnprintf( dest, size-1, fmt, argptr );
#define _vsnprintf	use_Str_vsnPrintf
#else
#undef vsnprintf
	int ret = vsnprintf( dest, size, fmt, argptr );
#define vsnprintf	use_Str_vsnPrintf
#endif
	dest[ size-1 ] = 0;
	if( ret < 0 || ret >= size )
		return -1;
	return ret;
}

int sprintf( Str & string, const char * fmt, ... ) {
	va_list argptr;
	char buffer[ 32000 ];	
	va_start( argptr, fmt );
	int l = Str::vsnPrintf( buffer, sizeof( buffer )-1, fmt, argptr );
	va_end( argptr );
	buffer[ sizeof( buffer ) - 1 ] = 0;
	string.Set( buffer, l );
	return l;
}

int vsprintf( Str & string, const char * fmt, va_list argptr ) {
	char buffer[ 32000 ];	
	int l = Str::vsnPrintf( buffer, sizeof( buffer )-1, fmt, argptr );
	buffer[ sizeof( buffer ) - 1 ] = 0;	
	string.Set( buffer, l );
	return l;
}

/*
============
va

does a varargs printf into a temp buffer
NOTE: not thread safe
============
*/
/*char * va( const char * fmt, ... ) {
	va_list argptr;
	static int			index = 0;
	static char			string[ 4 ][ 16384 ];	// in case called by nested functions
	char * buf;

	buf = string[ index ];
	index =( index + 1 ) & 3;

	va_start( argptr, fmt );
	vsprintf( buf, fmt, argptr );
	va_end( argptr );

	return buf;
}*/

int Str::BestUnit( float value, Measure_t measure, const char * format ) {
	int unit = 1;
	while( unit <= 4 && ( 1 << ( unit * 10 ) < value ) )
		unit++;
	unit--;
	value /= 1 << ( unit * 10 );
	sprintf( *this, format, value );
	Append( units[ measure ][ unit ] );
	return unit;
}

int Str::BestUnit( int value, Measure_t measure, const char * format ) {
	int unit = 1;
	while( unit <= 4 && ( 1 << ( unit * 10 ) < value ) )
		unit++;
	unit--;
	value = ( int )( ( float )value / ( float )( 1 << ( unit * 10 ) ) );
	sprintf( *this, format, value );
	Append( units[ measure ][ unit ] );
	return unit;
}

void Str::SetUnit( float value, int unit, Measure_t measure, const char * format ) {
	value /= 1 << ( unit * 10 );
	sprintf( *this, format, value );
	Append( units[ measure ][ unit ] );
}

void Str::SetUnit( int value, int unit, Measure_t measure, const char * format ) {
	value = ( int )( ( float )value / ( float )( 1 << ( unit * 10 ) ) );
	sprintf( *this, format, value );
	Append( units[ measure ][ unit ] );
}

void Str::InitMemory( ) {
#ifdef USE_STRING_DATA_ALLOCATOR
	g_stringDataAllocator.Init( );
#endif
}

void Str::ShutdownMemory( ) {
#ifdef USE_STRING_DATA_ALLOCATOR
	g_stringDataAllocator.Shutdown( );
#endif
}

void Str::PurgeMemory( ) {
#ifdef USE_STRING_DATA_ALLOCATOR
	g_stringDataAllocator.FreeEmptyBaseBlocks( );
#endif
}

void Str::ShowMemoryUsage_f( const CmdArgs &args ) {
#ifdef USE_STRING_DATA_ALLOCATOR
	Common::Com_DPrintf( "%6d KB string memory( %d KB free in %d blocks, %d empty base blocks )\n", g_stringDataAllocator.GetBaseBlockMemory( ) >> 10, g_stringDataAllocator.GetFreeBlockMemory( ) >> 10, g_stringDataAllocator.GetNumFreeBlocks( ), g_stringDataAllocator.GetNumEmptyBaseBlocks( ) );
#endif
}

struct formatList_t {
	int			gran;
	int			count;
};

// elements of list need to decend in size
formatList_t formatList[] = {
	{ 1000000000, 0 },
	{ 1000000, 0 },
	{ 1000, 0 }
};

#define NUM_FORMAT_LIST ( sizeof( formatList ) / sizeof( formatList[ 0 ] ) )

Str Str::FormatNumber( int number ) {
	for( int i = 0; i < NUM_FORMAT_LIST; i++ ) { // reset
		formatList_t * li = formatList + i;
		li->count = 0;
	}
	bool hit;
	do { // main loop
		hit = false;
		for( int i = 0; i < NUM_FORMAT_LIST; i++ ) {
			formatList_t * li = formatList + i;
			if( number >= li->gran ) {
				li->count++;
				number -= li->gran;
				hit = true;
				break;
			}
		}
	} while( hit );
	Str string;
	bool found = false; // print out
	for( int i = 0; i < NUM_FORMAT_LIST; i++ ) {
		formatList_t * li = formatList + i;
		if( li->count ) {
			if( !found )
				string += va( "%i, ", li->count );
			else
				string += va( "%3.3i, ", li->count );
			found = true;
		} else if( found )
			string += va( "%3.3i, ", li->count );
	}
	if( found )
		string += va( "%3.3i", number );
	else
		string += va( "%i", number );
	int count = 11 - string.Length( ); // pad to proper size
	for( int i = 0; i < count; i++ )
		string.Insert( " ", 0 );
	return string;
}

