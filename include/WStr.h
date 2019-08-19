// Copyright (C) 2007 Id Software, Inc.
//

#ifndef __WSTR_H__
#define __WSTR_H__

/*
===============================================================================

	Wide Character string

===============================================================================
*/

const int W_COLOR_BITS				= 31;

// color escape character
const int WC_COLOR_ESCAPE			= L'^';
const int WC_COLOR_DEFAULT			= L'0';
const int WC_COLOR_RED				= L'1';
const int WC_COLOR_GREEN			= L'2';
const int WC_COLOR_YELLOW			= L'3';
const int WC_COLOR_BLUE				= L'4';
const int WC_COLOR_CYAN				= L'5';
const int WC_COLOR_MAGENTA			= L'6';
const int WC_COLOR_WHITE			= L'7';
const int WC_COLOR_GRAY				= L'8';
const int WC_COLOR_BLACK			= L'9';
const int WC_COLOR_LTGREY			= L':';
const int WC_COLOR_MDGREEN			= L'<';
const int WC_COLOR_MDYELLOW			= L'=';
const int WC_COLOR_MDBLUE			= L'>';
const int WC_COLOR_MDRED			= L'?';
const int WC_COLOR_LTORANGE			= L'A';
const int WC_COLOR_MDCYAN			= L'B';
const int WC_COLOR_MDPURPLE			= L'C';
const int WC_COLOR_ORANGE			= L'D';

// color escape string
#define WS_COLOR_DEFAULT			L"^0"
#define WS_COLOR_RED				L"^1"
#define WS_COLOR_GREEN				L"^2"
#define WS_COLOR_YELLOW				L"^3"
#define WS_COLOR_BLUE				L"^4"
#define WS_COLOR_CYAN				L"^5"
#define WS_COLOR_MAGENTA			L"^6"
#define WS_COLOR_WHITE				L"^7"
#define WS_COLOR_GRAY				L"^8"
#define WS_COLOR_BLACK				L"^9"
#define WS_COLOR_LTGREY				L"^:"
#define WS_COLOR_MDGREEN			L"^<"
#define WS_COLOR_MDYELLOW			L"^="
#define WS_COLOR_MDBLUE				L"^>"
#define WS_COLOR_MDRED				L"^?"
#define WS_COLOR_LTORANGE			L"^A"
#define WS_COLOR_MDCYAN				L"^B"
#define WS_COLOR_MDPURPLE			L"^C"
#define WS_COLOR_ORANGE				L"^D"

// the size of a wchar over the network
// stick to _WIN32's wchar size (2 bytes)
const int NET_SIZEOF_WCHAR = 2;

#if defined (_M_AMD64) || defined (_M_X64)
#define WSTR_ALLOC_BASE 24
#else
#define WSTR_ALLOC_BASE 26
#endif

class WStr {

public:

						WStr( void );
						WStr( const Str& text );
						WStr( const WStr& text );
						WStr( const WStr& text, int start, int end );
						WStr( const wchar_t* text );
						WStr( const wchar_t* text, int start, int end );
						~WStr( void );

	size_t				Size( void ) const;
	const wchar_t*		c_str( void ) const;
	operator			const wchar_t *( ) const;
	operator			const wchar_t *( );

	wchar_t				operator[]( int index ) const;
	wchar_t &			operator[]( int index );

	void				operator=( const Str& text );
	void				operator=( const WStr& text );
	void				operator=( const wchar_t* text );
	void				operator=( char c );

	friend WStr			operator+( const Str &a, const WStr &b );
	friend WStr			operator+( const Str &a, const wchar_t *b );
	friend WStr			operator+( const wchar_t *a, const WStr &b );

	WStr &				operator+=( const WStr &a );
	WStr &				operator+=( const wchar_t *a );
	WStr &				operator+=( const wchar_t a );

						// case sensitive compare
	bool				operator ==( const WStr &b ) const;
	bool				operator ==( const wchar_t *b ) const;

						// case sensitive compare
	bool				operator !=( const WStr &b ) const;
	bool				operator !=( const wchar_t *b ) const;

						// case sensitive compare
	int					Cmp( const wchar_t *text ) const;
	int					Cmpn( const wchar_t *text, int n ) const;
	int					CmpPrefix( const wchar_t *text ) const;

						// case insensitive compare
	int					Icmp( const wchar_t *text ) const;
	int					Icmpn( const wchar_t *text, int n ) const;
	int					IcmpPrefix( const wchar_t *text ) const;

						// case insensitive compare ignoring color
	int					IcmpNoColor( const wchar_t *text ) const;

	int					Length( void ) const;
	int					Allocated( void ) const;
	void				Empty( void );
	bool				IsEmpty( void ) const;
	void				Clear( void );
	void				Append( const wchar_t a );
	void				Append( const WStr &text );
	void				Append( const wchar_t *text );
	void				Append( const wchar_t *text, int len );
	void				Append( int count, const wchar_t c );
	void				Insert( const wchar_t a, int index );
	void				Insert( const wchar_t *text, int index );
	bool				IsColor( void ) const;
	int					LengthWithoutColors( void ) const;
	WStr &				RemoveColors( void );
	void				CapLength( int );
	void				Fill( const wchar_t ch, int newlen );
	void				SwapStrings( WStr& rhs );
	WStr&				CollapseColors( void );

	int					Find( const wchar_t c, int start = 0, int end = INVALID_POSITION ) const;
	int					Find( const wchar_t *text, bool casesensitive = true, int start = 0, int end = INVALID_POSITION ) const;
	const wchar_t *		Mid( int start, int len, WStr &result ) const;	// store 'len' characters starting at 'start' in result
	WStr				Mid( int start, int len ) const;					// return 'len' characters starting at 'start'
	void				Strip( const char *string );						// strip string from front and end as many times as the string occurs
	void				Replace( const wchar_t *old, const wchar_t *nw );
	void				ReplaceFirst( const wchar_t *old, const wchar_t *nw );
	void				ReplaceChar( wchar_t oldChar, wchar_t newChar );
	void				EraseRange( int start, int len = INVALID_POSITION );
	void				EraseChar( const wchar_t c, int start = 0 );	

	WStr&				StripFileExtension();

	// char * methods to replace library functions
	static int			Length( const wchar_t* s );
	static bool			IsColor( const wchar_t *s );
	static int			LengthWithoutColors( const wchar_t *s );
	static wchar_t *	RemoveColors( wchar_t *s );
	static wchar_t *	StripFilename( wchar_t *s );
	static int			Cmp( const wchar_t *s1, const wchar_t *s2 );
	static int			Cmpn( const wchar_t *s1, const wchar_t *s2, int n );
	static int			Icmp( const wchar_t *s1, const wchar_t *s2 );
	static int			Icmpn( const wchar_t *s1, const wchar_t *s2, int n );
	static int			IcmpNoColor( const wchar_t *s1, const wchar_t *s2 );
	static void			Append( wchar_t *dest, int size, const wchar_t *src );
	static void			Copynz( wchar_t *dest, const wchar_t *src, int destsize );
	static int			snPrintf( wchar_t *dest, int size, const wchar_t *fmt, ... );
	static int			vsnPrintf( wchar_t *dest, int size, const wchar_t *fmt, va_list argptr );
	static int			FindChar( const wchar_t *str, const wchar_t c, int start = 0, int end = INVALID_POSITION );
	static int			FindText( const wchar_t *str, const wchar_t *text, bool casesensitive = true, int start = 0, int end = INVALID_POSITION );

	// character methods
	static int			ColorIndex( wchar_t c );

	friend int			swprintf( WStr &dest, const wchar_t *fmt, ... );
	friend int			vswprintf( WStr &dest, const wchar_t *fmt, va_list ap );

	void				ReAllocate( int amount, bool keepold );				// reallocate string data buffer

protected:

	int					len;
	wchar_t *			data;
	int					alloced;
	wchar_t				baseBuffer[ WSTR_ALLOC_BASE ];

	void				Init( void );										// initialize string using base buffer
	void				EnsureAlloced( int amount, bool keepold = true );	// ensure string data buffer is large enough
	// free allocated string memory
	void				FreeData( );

public:

	static const int	INVALID_POSITION = -1;
};

/*
============
va utility funcs
============
*/

wchar_t* va( const wchar_t *fmt, ... );


INLINE void WStr::EnsureAlloced( int amount, bool keepold ) {
	if ( amount > alloced ) {
		ReAllocate( amount, keepold );
	}
}

INLINE void WStr::Init( void ) {
	len = 0;
	alloced = WSTR_ALLOC_BASE;
	data = baseBuffer;
	data[ 0 ] = L'\0';
#ifdef ID_DEBUG_UNINITIALIZED_MEMORY
	::wmemset( baseBuffer, 0, sizeof( baseBuffer ) );
#endif
}

INLINE WStr::WStr( void ) {
	Init();
}

INLINE WStr::WStr( const Str &text ) {

	Init( );
	int l = text.Length( );
	EnsureAlloced( l + 1 );
	mbstowcs( data, text, l + 1 );
	len = l;
}

INLINE WStr::WStr( const WStr &text ) {

	Init();
	int l = text.Length();
	EnsureAlloced( l + 1 );
	wcscpy( data, text.data );
	len = l;
}

INLINE WStr::WStr( const WStr &text, int start, int end ) {
	int i;
	int l;

	Init();
	if ( end > text.Length() ) {
		end = text.Length();
	}
	if ( start > text.Length() ) {
		start = text.Length();
	} else if ( start < 0 ) {
		start = 0;
	}

	l = end - start;
	if ( l < 0 ) {
		l = 0;
	}

	EnsureAlloced( l + 1 );

	for ( i = 0; i < l; i++ ) {
		data[ i ] = text[ start + i ];
	}

	data[ l ] = '\0';
	len = l;
}

INLINE WStr::WStr( const wchar_t *text ) {
	int l;

	Init();
	if ( text ) {
		l = Length( text );
		EnsureAlloced( l + 1 );
		wcscpy( data, text );
		len = l;
	}
}

INLINE WStr::WStr( const wchar_t *text, int start, int end ) {
	int i;
	int l = Length( text );

	Init();
	if ( end > l ) {
		end = l;
	}
	if ( start > l ) {
		start = l;
	} else if ( start < 0 ) {
		start = 0;
	}

	l = end - start;
	if ( l < 0 ) {
		l = 0;
	}

	EnsureAlloced( l + 1 );

	for ( i = 0; i < l; i++ ) {
		data[ i ] = text[ start + i ];
	}

	data[ l ] = '\0';
	len = l;
}

INLINE WStr::~WStr( void ) {
	Clear();
}

INLINE size_t WStr::Size( void ) const {
	return sizeof( *this ) + Allocated() * sizeof( wchar_t );
}

INLINE const wchar_t *WStr::c_str( void ) const {
	return data;
}

INLINE WStr::operator const wchar_t *( ) {
	return data;
}

INLINE WStr::operator const wchar_t *( ) const {
	return data;
}

INLINE wchar_t WStr::operator[]( int index ) const {
	assert( ( index >= 0 ) && ( index <= len ) );
	return data[ index ];
}

INLINE wchar_t &WStr::operator[]( int index ) {
	assert( ( index >= 0 ) && ( index <= len ) );
	return data[ index ];
}

INLINE void WStr::operator=( const Str &text ) {

	int l = text.Length( );
	EnsureAlloced( l + 1, false );
	mbstowcs( data, text, l + 1 );
	len = l;
}

INLINE void WStr::operator=( const WStr &text ) {

	int l = text.Length();
	EnsureAlloced( l + 1, false );
	::wmemcpy( data, text.data, l );
	data[l] = L'\0';
	len = l;
}

INLINE void WStr::operator=( char c ) {

	EnsureAlloced( 2, false );
	mbtowc( data, &c, 1 );
	data[ 1 ] = L'\0';
	len = 1;
}

INLINE WStr operator+( const WStr &a, const WStr &b ) {
	WStr result( a );
	result.Append( b );
	return result;
}

INLINE WStr operator+( const WStr &a, const wchar_t *b ) {
	WStr result( a );
	result.Append( b );
	return result;
}

INLINE WStr operator+( const wchar_t *a, const WStr &b ) {
	WStr result( a );
	result.Append( b );
	return result;
}

INLINE WStr operator+( const WStr &a, const wchar_t b ) {
	WStr result( a );
	result.Append( b );
	return result;
}

INLINE WStr &WStr::operator+=( const WStr &a ) {
	Append( a );
	return *this;
}

INLINE WStr &WStr::operator+=( const wchar_t *a ) {
	Append( a );
	return *this;
}

INLINE WStr &WStr::operator+=( const wchar_t a ) {
	Append( a );
	return *this;
}

INLINE bool WStr::operator== ( const WStr &b ) const {
	//return ( !WStr::Cmp( data, b.data ) );
	if( len != b.len ) return false;
	int left, offs = 0;
	while( ( left = len- offs ) ) {
#if defined (_M_AMD64) || defined (_M_X64)
		if( left >> 2 ) {
			if( *( __int64 * )( data + offs ) != *( __int64 * )( b.data + offs ) ) return false;
			offs += 4; continue;
		}
#endif
		if( left >> 1 ) {
			if( *( __int32 * )( data + offs ) != *( __int32 * )( b.data + offs ) ) return false;
			offs += 2; continue;
		}
		if( left ) {
			if( *( __int16 * )( data + offs ) != *( __int16 * )( b.data + offs ) ) return false;
			offs += 1; continue;
		}
	}
	return true;
}

	
INLINE bool WStr::operator== ( const wchar_t *b ) const {
	return !WStr::Cmp( data, b );
}

INLINE bool WStr::operator!= ( const WStr &b ) const {
	//return !( a == b );
	if( len != b.len ) return true;
	int left, offs = 0;
	while( ( left = len - offs ) ) {
#if defined (_M_AMD64) || defined (_M_X64)
		if( left >> 2 ) {
			if( *( __int64 * )( data + offs ) != *( __int64 * )( b.data + offs ) ) return true;
			offs += 4; continue;
		}
#endif
		if( left >> 1 ) {
			if( *( __int32 * )( data + offs ) != *( __int32 * )( b.data + offs ) ) return true;
			offs += 2; continue;
		}
		if( left ) {
			if( *( __int16 * )( data + offs ) != *( __int16 * )( b.data + offs ) ) return true;
			offs += 1; continue;
		}
	}
	return false;
}

INLINE bool WStr::operator!= ( const wchar_t *b ) const {
	return WStr::Cmp( data, b ) != 0;
}

INLINE int WStr::Cmp( const wchar_t *text ) const {
	assert( text );
	return WStr::Cmp( data, text );
}

INLINE int WStr::Cmpn( const wchar_t *text, int n ) const {
	assert( text );
	return WStr::Cmpn( data, text, n );
}

INLINE int WStr::CmpPrefix( const wchar_t *text ) const {
	assert( text );
	return Cmpn( data, text, Length( text ) );
}

INLINE int WStr::Icmp( const wchar_t *text ) const {
	assert( text );
	return Icmp( data, text );
}

INLINE int WStr::Icmpn( const wchar_t *text, int n ) const {
	assert( text );
	return Icmpn( data, text, n );
}

INLINE int WStr::IcmpPrefix( const wchar_t *text ) const {
	assert( text );
	return Icmpn( data, text, Length( text ) );
}

INLINE int WStr::IcmpNoColor( const wchar_t *text ) const {
	assert( text );
	return IcmpNoColor( data, text );
}

INLINE int WStr::Length( void ) const {
	return len;
}

INLINE int WStr::Allocated( void ) const {
	if ( data != baseBuffer ) {
		return alloced;
	} else {
		return 0;
	}
}

INLINE void WStr::Empty( void ) {
	EnsureAlloced( 1 );
	data[ 0 ] = L'\0';
	len = 0;
}

INLINE bool WStr::IsEmpty( void ) const {
	return ( Cmp( data, L"" ) == 0 );
}

INLINE void WStr::Clear( void ) {
	FreeData();
	Init();
}

INLINE void WStr::Append( const wchar_t a ) {
	EnsureAlloced( len + 2 );
	data[ len ] = a;
	len++;
	data[ len ] = L'\0';
}

INLINE void WStr::Append( const WStr &text ) {
	int newLen;
	int i;

	newLen = len + text.Length();
	EnsureAlloced( newLen + 1 );
	for ( i = 0; i < text.len; i++ ) {
		data[ len + i ] = text[ i ];
	}
	len = newLen;
	data[ len ] = L'\0';
}

INLINE void WStr::Append( const wchar_t *text ) {
	int newLen;
	int i;

	if ( text ) {
		newLen = len + Length( text );
		EnsureAlloced( newLen + 1 );
		for ( i = 0; text[ i ]; i++ ) {
			data[ len + i ] = text[ i ];
		}
		len = newLen;
		data[ len ] = L'\0';
	}
}

INLINE void WStr::Append( const wchar_t *text, int l ) {
	int newLen;
	int i;

	if ( text && l ) {
		newLen = len + l;
		EnsureAlloced( newLen + 1 );
		for ( i = 0; text[ i ] && i < l; i++ ) {
			data[ len + i ] = text[ i ];
		}
		len = newLen;
		data[ len ] = L'\0';
	}
}

INLINE void WStr::Insert( const wchar_t a, int index ) {
	int i, l;

	if ( index < 0 ) {
		index = 0;
	} else if ( index > len ) {
		index = len;
	}

	l = 1;
	EnsureAlloced( len + l + 1 );
	for ( i = len; i >= index; i-- ) {
		data[i+l] = data[i];
	}
	data[index] = a;
	len++;
}

INLINE void WStr::Insert( const wchar_t *text, int index ) {
	int i, l;

	if ( index < 0 ) {
		index = 0;
	} else if ( index > len ) {
		index = len;
	}

	l = Length( text );
	EnsureAlloced( len + l + 1 );
	for ( i = len; i >= index; i-- ) {
		data[i+l] = data[i];
	}
	for ( i = 0; i < l; i++ ) {
		data[index+i] = text[i];
	}
	len += l;
}

INLINE bool WStr::IsColor( void ) const {
	return IsColor( data );
}

INLINE WStr &WStr::RemoveColors( void ) {
	RemoveColors( data );
	len = Length( data );
	return *this;
}

INLINE int WStr::LengthWithoutColors( void ) const {
	return LengthWithoutColors( data );
}

INLINE void WStr::CapLength( int newlen ) {
	if ( len <= newlen ) {
		return;
	}
	data[ newlen ] = L'\0';
	len = newlen;
}

INLINE void WStr::Fill( const wchar_t ch, int newlen ) {
	EnsureAlloced( newlen + 1 );
	len = newlen;
	::wmemset( data, ch, len );
	data[ len ] = L'\0';
}

INLINE int WStr::Find( const wchar_t c, int start, int end ) const {
	if ( end == INVALID_POSITION ) {
		end = len;
	}
	return FindChar( data, c, start, end );
}

INLINE int WStr::Find( const wchar_t *text, bool casesensitive, int start, int end ) const {
	if ( end == INVALID_POSITION ) {
		end = len;
	}
	return FindText( data, text, casesensitive, start, end );
}

INLINE int WStr::Length( const wchar_t *s ) {
	int i;
	for ( i = 0; s[i] != L'\0'; i++ ) {}
	return i;
}

INLINE bool WStr::IsColor( const wchar_t *s ) {
	return ( s && s[0] == WC_COLOR_ESCAPE && s[1] != L'\0' && s[1] != L' ' );
}

INLINE int WStr::ColorIndex( wchar_t c ) {
	return ( ( c - L'0' ) & W_COLOR_BITS );
}

INLINE void WStr::SwapStrings( WStr& rhs ) {
	if( rhs.data != rhs.baseBuffer && data != baseBuffer ) {
		Swap( data, rhs.data );
		Swap( len, rhs.len);
		Swap( alloced, rhs.alloced );
	} else {
		Swap( *this, rhs );
	}
}

#endif /* !__WSTR_H__ */

