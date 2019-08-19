#include "precompiled.h"
#pragma hdrstop

/*
============
WStr::ReAllocate
============
*/
void WStr::ReAllocate( int amount, bool keepold ) {
	assert( data && amount > 0 );
	wchar_t * newbuffer;
	if( amount > WSTR_ALLOC_BASE ) {
		int mod = amount % STR_ALLOC_GRAN;
		int newsize = amount + STR_ALLOC_GRAN - mod;
		alloced = newsize;
#ifdef USE_STRING_DATA_ALLOCATOR
		newbuffer = ( wchar_t * )g_stringDataAllocator.Alloc( alloced * 2 );
#else
		newbuffer = new wchar_t[ d_alloced ];
#endif
	} else {
		newbuffer = baseBuffer;
		alloced = WSTR_ALLOC_BASE;
	}
	if( keepold ) {
		len = Min( alloced - 1, len );
		data[ len ] = '\0';
		wcsncpy( newbuffer, data, len );
	} else {
		len = 0;
		newbuffer[ 0 ] = '\0';
	}
	FreeData( );
	data = newbuffer;
}

/*
============
WStr::FreeData
============
*/
void WStr::FreeData( void ) {
	assert( data );
	if( data != baseBuffer ) {
#ifdef USE_STRING_DATA_ALLOCATOR
		g_stringDataAllocator.Free( ( char * )data );
#else
		delete[ ] data;
#endif
	}
}

/*
============
WStr::operator=
============
*/
void WStr::operator=( const wchar_t *text ) {
	int l;
	int diff;
	int i;

	if ( text == NULL ) {
		// safe behaviour if NULL
		EnsureAlloced( 1, false );
		data[ 0 ] = L'\0';
		len = 0;
		return;
	}

	if ( text == data ) {
		return; // copying same thing
	}

	// check if we're aliasing
	if ( text >= data && text <= &data[ len ] ) {
		diff = ( int )( text - data );

		assert( WStr::Length( text ) < len );

		for ( i = 0; text[ i ]; i++ ) {
			data[ i ] = text[ i ];
		}

		data[ i ] = L'\0';

		len -= diff;

		return;
	}

	l = Length( text );
	EnsureAlloced( l + 1, false );
	wcscpy( data, text );
	len = l;
}

/*
============
WStr::FindChar

returns INVALID_POSITION if not found otherwise the index of the char
============
*/
int WStr::FindChar( const wchar_t *str, const wchar_t c, int start, int end ) {
	int i;

	if ( end == INVALID_POSITION ) {
		end = Length( str ) - 1;
	}
	for ( i = start; i <= end; i++ ) {
		if ( str[i] == c ) {
			return i;
		}
	}
	return INVALID_POSITION;
}

/*
============
WStr::FindText

returns INVALID_POSITION if not found otherwise the index of the text
============
*/
int WStr::FindText( const wchar_t *str, const wchar_t *text, bool casesensitive, int start, int end ) {
	int l, i, j;

	if ( end == INVALID_POSITION ) {
		end = Length( str );
	}
	l = end - Length( text );
	for ( i = start; i <= l; i++ ) {
		if ( casesensitive ) {
			for ( j = 0; text[j]; j++ ) {
				if ( str[i+j] != text[j] ) {
					break;
				}
			}
		} else {
			for ( j = 0; text[j]; j++ ) {
				if ( towupper( str[i+j] ) != towupper( text[j] ) ) {
					break;
				}
			}
		}
		if ( !text[j] ) {
			return i;
		}
	}
	return INVALID_POSITION;
}

/*
============
WStr::ReplaceChar
============
*/
void WStr::ReplaceChar( wchar_t oldChar, wchar_t newChar ) {
	int i;
	for ( i = 0; i < len; i++ ) {
		if ( data[ i ] != oldChar ) {
			continue;
		}

		data[ i ] = newChar;
	}
}

/*
============
WStr::Replace
============
*/
void WStr::Replace( const wchar_t *old, const wchar_t *nw ) {
	if ( Length( old ) == 0 ) {
		return;
	}

	int		oldLen, newLen, i, count;

	oldLen = Length( old );
	newLen = Length( nw );

	// Work out how big the new string will be
	count = 0;
	for ( i = 0; i < len; i++ ) {
		if( !Cmpn( &data[i], old, oldLen ) ) {
			count++;
			i += oldLen - 1;
		}
	}

	if ( count ) {
		WStr	oldString( data );
		int		j;

		EnsureAlloced( len + ( ( newLen - oldLen ) * count ) + 2, false );

		// Replace the old data with the new data
		for ( i = 0, j = 0; i < oldString.Length(); i++ ) {
			if( !Cmpn( &oldString[i], old, oldLen ) ) {
				wmemcpy( data + j, nw, newLen );
				i += oldLen - 1;
				j += newLen;
			} else {
				data[j] = oldString[i];
				j++;
			}
		}
		data[j] = L'\0';
		len = Length( data );
	}
}

/*
============
WStr::ReplaceFirst
============
*/
void WStr::ReplaceFirst( const wchar_t *old, const wchar_t *nw ) {
	if( Length( old ) == 0 ) {
		return;
	}

	int		oldLen, newLen, i;
	bool	present;

	oldLen = Length( old );
	newLen = Length( nw );

	// Work out how big the new string will be
	present = false;
	for ( i = 0; i < len; i++ ) {
		if ( !Cmpn( &data[i], old, oldLen ) ) {
			present = true;
			i += oldLen - 1;
			break;
		}
	}

	if ( present ) {
		WStr	oldString( data );
		int		j;

		EnsureAlloced( len + ( newLen - oldLen ) + 2, false );

		// Replace the old data with the new data
		for ( i = 0, j = 0; i < oldString.Length(); i++ ) {
			if ( !Cmpn( &oldString[i], old, oldLen ) ) {
				wmemcpy( data + j, nw, newLen );
				i += oldLen;
				j += newLen;
				break;
			} else {
				data[j] = oldString[i];
				j++;
			}
		}
		::wmemcpy( data + j, &oldString[i], oldString.Length() - i );
		data[j + oldString.Length() - i] = L'\0';
		len = Length( data );
	}
}

/*
============
WStr::Mid
============
*/
const wchar_t *WStr::Mid( int start, int len, WStr& result ) const {
	int i;

	assert( &result != this );

	result.Empty();

	i = Length();
	if ( i == 0 || len <= 0 || start >= i ) {
		return NULL;
	}

	if ( start + len >= i ) {
		len = i - start;
	}

	result.Append( &data[ start ], len );
	return result.c_str();
}

/*
============
WStr::Mid
============
*/
WStr WStr::Mid( int start, int len ) const {
	int i;
	WStr result;

	i = Length();
	if ( i == 0 || len <= 0 || start >= i ) {
		return result;
	}

	if ( start + len >= i ) {
		len = i - start;
	}

	result.Append( &data[ start ], len );
	return result;
}

/*
=====================================================================

  char * methods to replace library functions

=====================================================================
*/

/*
================
WStr::StripFilename
================
*/
wchar_t* WStr::StripFilename( wchar_t* string ) {
	int pos;
	
	pos = WStr::Length( string ) - 1;
	while( ( pos > 0 ) && ( string[ pos ] != L'/' ) && ( string[ pos ] != L'\\' ) ) {
		pos--;
	}

	if ( pos < 0 ) {
		pos = 0;
	}

	string[ pos ] = L'\0';

	return string;
}

/*
================
WStr::Cmp
================
*/
int WStr::Cmp( const wchar_t *s1, const wchar_t *s2 ) {
	int c1, c2, d;

	do {
		c1 = *s1++;
		c2 = *s2++;

		d = c1 - c2;
		if ( d ) {
			return ( INTSIGNBITNOTSET( d ) << 1 ) - 1;
		}
	} while( c1 );

	return 0;		// strings are equal
}

/*
================
WStr::Cmpn
================
*/
int WStr::Cmpn( const wchar_t *s1, const wchar_t *s2, int n ) {
	int c1, c2, d;

	assert( n >= 0 );

	do {
		c1 = *s1++;
		c2 = *s2++;

		if ( !n-- ) {
			return 0;		// strings are equal until end point
		}

		d = c1 - c2;
		if ( d ) {
			return ( INTSIGNBITNOTSET( d ) << 1 ) - 1;
		}
	} while( c1 );

	return 0;		// strings are equal
}

/*
================
WStr::Icmp
================
*/
int WStr::Icmp( const wchar_t *s1, const wchar_t *s2 ) {
	int c1, c2, d;

	do {
		c1 = *s1++;
		c2 = *s2++;

		d = c1 - c2;
		while( d ) {
			if ( c1 <= 'Z' && c1 >= 'A' ) {
				d += ('a' - 'A');
				if ( !d ) {
					break;
				}
			}
			if ( c2 <= 'Z' && c2 >= 'A' ) {
				d -= ('a' - 'A');
				if ( !d ) {
					break;
				}
			}
			return ( INTSIGNBITNOTSET( d ) << 1 ) - 1;
		}
	} while( c1 );

	return 0;		// strings are equal
}

/*
================
WStr::Icmpn
================
*/
int WStr::Icmpn( const wchar_t *s1, const wchar_t *s2, int n ) {
	int c1, c2, d;

	assert( n >= 0 );

	do {
		c1 = *s1++;
		c2 = *s2++;

		if ( !n-- ) {
			return 0;		// strings are equal until end point
		}

		d = c1 - c2;
		while( d ) {
			if ( c1 <= 'Z' && c1 >= 'A' ) {
				d += ('a' - 'A');
				if ( !d ) {
					break;
				}
			}
			if ( c2 <= 'Z' && c2 >= 'A' ) {
				d -= ('a' - 'A');
				if ( !d ) {
					break;
				}
			}
			return ( INTSIGNBITNOTSET( d ) << 1 ) - 1;
		}
	} while( c1 );

	return 0;		// strings are equal
}

/*
================
WStr::Icmp
================
*/
int WStr::IcmpNoColor( const wchar_t *s1, const wchar_t *s2 ) {
	int c1, c2, d;

	do {
		while ( IsColor( s1 ) ) {
			s1 += 2;
		}
		while ( IsColor( s2 ) ) {
			s2 += 2;
		}
		c1 = *s1++;
		c2 = *s2++;

		d = c1 - c2;
		while( d ) {
			if ( c1 <= 'Z' && c1 >= 'A' ) {
				d += ('a' - 'A');
				if ( !d ) {
					break;
				}
			}
			if ( c2 <= 'Z' && c2 >= 'A' ) {
				d -= ('a' - 'A');
				if ( !d ) {
					break;
				}
			}
			return ( INTSIGNBITNOTSET( d ) << 1 ) - 1;
		}
	} while( c1 );

	return 0;		// strings are equal
}

/*
=============
WStr::Copynz
 
Safe strncpy that ensures a trailing zero
=============
*/
void WStr::Copynz( wchar_t *dest, const wchar_t *src, int destsize ) {

	if ( src == NULL || destsize < 1 ) return;

	::wcsncpy( dest, src, destsize-1 );
    dest[destsize-1] = L'\0';
}

/*
================
WStr::Append

  never goes past bounds or leaves without a terminating 0
================
*/
void WStr::Append( wchar_t *dest, int size, const wchar_t *src ) {
	int		l1;

	l1 = Length( dest );
	Copynz( dest + l1, src, size - l1 );
}

/*
================
WStr::LengthWithoutColors
================
*/
int WStr::LengthWithoutColors( const wchar_t *s ) {
	int len;
	const wchar_t *p;

	if ( !s ) {
		return 0;
	}

	len = 0;
	p = s;
	while( *p != L'\0' ) {
		if ( IsColor( p ) ) {
			p += 2;
			continue;
		}
		p++;
		len++;
	}

	return len;
}

/*
================
WStr::RemoveColors
================
*/
wchar_t *WStr::RemoveColors( wchar_t *string ) {
	wchar_t *d;
	wchar_t *s;
	int c;

	s = string;
	d = string;
	while( (c = *s) != 0 ) {
		if ( IsColor( s ) ) {
			s++;
		}		
		else {
			*d++ = c;
		}
		s++;
	}
	*d = L'\0';

	return string;
}

/*
================
WStr::snPrintf
================
*/
int WStr::snPrintf( wchar_t *dest, int size, const wchar_t *fmt, ...) {
	int ret;
	va_list argptr;

#ifdef _WIN32
#undef _vsnwprintf
	va_start( argptr, fmt );
	ret = _vsnwprintf( dest, size-1, fmt, argptr );
	va_end( argptr );
#else
	// there is a vswprintf( WStr ), so no #undef
	va_start( argptr, fmt );
	ret = vswprintf( dest, size, fmt, argptr );
	va_end( argptr );
#endif
	dest[size-1] = L'\0';

	// never pass a %s formatting, always use %hs or %ls (%s works only on windows)
	assert( FindText( fmt, L"%s" ) == INVALID_POSITION );

	if ( ret < 0 || ret >= size ) {
		return -1;
	}
	return ret;
}

/*
============
WStr::vsnPrintf

see Str::vsnPrintf
same _WIN32 vs rest of the world for _vsnwprintf/vswprintf

you can call vsnPrintf( buffer, size ) with wchar_t buffer[size]
always has a terminating null character after call
will return -1 on error or overflow
============
*/
int WStr::vsnPrintf( wchar_t *dest, int size, const wchar_t *fmt, va_list argptr ) {
	int ret;

#ifdef _WIN32
#undef _vsnwprintf
	ret = _vsnwprintf( dest, size-1, fmt, argptr );
#else
	// there is a vswprintf( WStr ), so no #undef
	ret = vswprintf( dest, size, fmt, argptr );
#endif
	dest[size-1] = L'\0';

	// never pass a %s formatting, always use %hs or %ls (%s works only on windows)
	assert( FindText( fmt, L"%s" ) == INVALID_POSITION );

	if ( ret < 0 || ret >= size ) {
		return -1;
	}
	return ret;
}

/*
============
swprintf

Sets the value of the string using a printf interface.
============
*/
int swprintf( WStr &string, const wchar_t *fmt, ... ) {
	static const int BUFFER_SIZE = 32000;
	int l;
	va_list argptr;
	wchar_t buffer[BUFFER_SIZE];
	
	va_start( argptr, fmt );
	l = WStr::vsnPrintf( buffer, BUFFER_SIZE, fmt, argptr );
	va_end( argptr );

	string = buffer;
	return l;
}

/*
============
vswprintf

Sets the value of the string using a vprintf interface.
============
*/
int vswprintf( WStr &string, const wchar_t *fmt, va_list argptr ) {
	static const int BUFFER_SIZE = 32000;
	int l;
	wchar_t buffer[BUFFER_SIZE];
	
	l = WStr::vsnPrintf( buffer, BUFFER_SIZE, fmt, argptr );
	
	string = buffer;
	return l;
}

/*
============
va

does a varargs printf into a temp buffer
NOTE: not thread safe
============
*/
#define VA_BUF_LEN 16384
wchar_t *va( const wchar_t *fmt, ... ) {
	va_list argptr;
	static int index = 0;
	static wchar_t string[4][VA_BUF_LEN];	// in case called by nested functions
	wchar_t *buf;

	buf = string[index];
	index = (index + 1) & 3;

	va_start( argptr, fmt );
	WStr::vsnPrintf( buf, VA_BUF_LEN, fmt, argptr );
	va_end( argptr );

	return buf;
}

/*
============
WStr::EraseRange
============
*/
void WStr::EraseRange( int start, int len ) {
	if( IsEmpty() || len == 0 ) {
		return;
	}

	if( start < 0 ) {
		start = 0;
	}

	if( start >= this->len ) {
		return;
	}

	int totalLength = Length();
	if( len == INVALID_POSITION ) {
		len = totalLength - start;
	}
	
	if( len == totalLength ) {
		// erase the whole thing
		Empty();
		return;
	}


	if ( totalLength - start - len ) {
		::wmemmove( &data[ start ], &data[ start + len ], totalLength - start - len );
	}
	
	data[ totalLength - len ] = L'\0';
	this->len -= len;
}


/*
============
WStr::EraseChar
============
*/
void WStr::EraseChar( const wchar_t c, int start ) {
	if( start < 0 ) {
		start = 0;
	}

	int totalLength = Length();
	while( start < totalLength - 1 ) {
		int offset = start + 1;
		while( data[ start ] == c && offset < totalLength ) {
			Swap( data[ start ], data[ offset ] );
			offset++;
		}
		start++;
	}

	start = totalLength - 1;
	while( start > 0 && data[ start ] == c ) {
		data[ start ] = L'\0';
		start--;
	}
	len = start + 1;
}


/*
============
WStr::Append
============
*/
void WStr::Append( int count, const wchar_t c ) {
	EnsureAlloced( len + count + 1 );
	int start = len;
	int end = len + count;
	while( start < end ) {
		data[ start ] = c;
		start++;
	}
	data[ start ] = L'\0';
	len += count;
}

/*
============
WStr::CollapseColors
============
*/
WStr& WStr::CollapseColors( void ) {
	int colorBegin = -1;
	int lastColor = -1;
	for( int i = 0; i < len; i++ ) {
		while( WStr::IsColor( &data[ i ] ) && i < len ) {
			if( colorBegin == -1 ) {
				colorBegin = i;
			}
			lastColor = i;
			i += 2;
		}
		if( colorBegin != -1 && lastColor != colorBegin ) {
			EraseRange( colorBegin, lastColor - colorBegin );
			i -= lastColor - colorBegin;
		}
		colorBegin = -1;
		lastColor = -1;
	}
	return *this;
}

/*
============
WStr::StripFileExtension
============
*/
WStr &WStr::StripFileExtension( void ) {
	int i;

	for ( i = len-1; i >= 0; i-- ) {
		if ( data[i] == L'/' || data[i] == L'\\' ) {
			break;
		}
		if ( data[i] == L'.' ) {
			data[i] = L'\0';
			len = i;
			break;
		}
	}
	return *this;
}
