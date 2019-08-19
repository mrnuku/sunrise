#ifndef STR_H
#define STR_H

// these library functions should not be used for cross platform compatibility
#define strcmp			Str::Cmp		// use_idStr_Cmp
#define strncmp			use_idStr_Cmpn
#define StrCmpN			use_idStr_Cmpn
#define strcmpi			use_idStr_Icmp
#define StrCmpI			use_idStr_Icmp
#define stricmp			Str::Icmp		// use_idStr_Icmp
#define _stricmp		use_idStr_Icmp
#define strcasecmp		use_idStr_Icmp
#define strnicmp		use_idStr_Icmpn
#define _strnicmp		use_idStr_Icmpn
#define _memicmp		use_idStr_Icmpn
#define StrCmpNI		use_idStr_Icmpn
#define snprintf		use_idStr_snPrintf
#define _snprintf		use_idStr_snPrintf
#define vsnprintf		use_idStr_vsnPrintf
#define _vsnprintf		use_idStr_vsnPrintf

#ifndef FILE_HASH_SIZE
#define FILE_HASH_SIZE		1024
#endif

// color escape character
const int C_COLOR_ESCAPE			= '^';
const int C_COLOR_DEFAULT			= '0';
const int C_COLOR_RED				= '1';
const int C_COLOR_GREEN				= '2';
const int C_COLOR_YELLOW			= '3';
const int C_COLOR_BLUE				= '4';
const int C_COLOR_CYAN				= '5';
const int C_COLOR_MAGENTA			= '6';
const int C_COLOR_WHITE				= '7';
const int C_COLOR_GRAY				= '8';
const int C_COLOR_BLACK				= '9';

// color escape string
#define S_COLOR_DEFAULT				"^0"
#define S_COLOR_RED					"^1"
#define S_COLOR_GREEN				"^2"
#define S_COLOR_YELLOW				"^3"
#define S_COLOR_BLUE				"^4"
#define S_COLOR_CYAN				"^5"
#define S_COLOR_MAGENTA				"^6"
#define S_COLOR_WHITE				"^7"
#define S_COLOR_GRAY				"^8"
#define S_COLOR_BLACK				"^9"

#define MASKICMP64	0xDFDFDFDFDFDFDFDF
#define MASKICMP32	0xDFDFDFDF
#define MASKICMP16	0xDFDF
#define MASKICMP8	0xDF

// make Str a multiple of 16 bytes long
// don't make too large to keep memory requirements to a minimum
#if defined (_M_AMD64) || defined (_M_X64)
#define STR_ALLOC_BASE	48
#define STR_ALLOC_GRAN	64
#else
#define STR_ALLOC_BASE	20
#define STR_ALLOC_GRAN	32
#endif

typedef enum {
	MEASURE_SIZE = 0,
	MEASURE_BANDWIDTH
} Measure_t;

class Str {
protected:
	char				d_baseBuffer[ STR_ALLOC_BASE ];
	char *				d_data;
	int					d_len;
	int					d_alloced;

	// initialize string using base buffer
	void				Init( );
	// ensure string data buffer is large anough
	void				EnsureAlloced( int amount, bool keepold = true );
	// free allocated string memory
	void				FreeData( );
	// case sensitive block compare, without length check
	static bool			BlockCmp( const char * a, const char * b, int len );
	// case insensitive block compare, without length check
	static bool			IBlockCmp( const char * a, const char * b, int len );
	// returns -1 if not found otherwise the index of the text
	static int			BlockFindText( const char * str, int str_len, const char * text, int text_len, int start_offs );
	static int			IBlockFindText( const char * str, int str_len, const char * text, int text_len, int start_offs );

public:
						Str( );
						Str( const Str & text );
						Str( const Str & text, int start, int end );
						Str( const char * text );
						Str( const char * text, int start, int end );
						Str( const char * text, int len );
						Str( const CEGUI::String & text );
						explicit Str( const bool b );
						explicit Str( const char c );
						explicit Str( const int i );
						explicit Str( const unsigned u );
						explicit Str( const float f );
						~Str( );

	size_t				Size( ) const;
	const char *		c_str( ) const;
	CEGUI::String		cegui_str( ) const;
	operator			const char *( ) const;
	operator			const char *( );

	char				operator[ ]( int index ) const;
	char &				operator[ ]( int index );

	void				operator =( const Str & text );
	void				operator =( const char * text );
	void				operator =( const CEGUI::String & text );

	friend Str			operator+( const Str & a, const Str & b );
	friend Str			operator+( const Str & a, const char * b );
	friend Str			operator+( const char * a, const Str & b );

	friend Str			operator+( const Str & a, const float b );
	friend Str			operator+( const Str & a, const int b );
	friend Str			operator+( const Str & a, const unsigned b );
	friend Str			operator+( const Str & a, const bool b );
	friend Str			operator+( const Str & a, const char b );

	Str &				operator+=( const Str & a );
	Str &				operator+=( const char * a );
	Str &				operator+=( const float a );
	Str &				operator+=( const char a );
	Str &				operator+=( const int a );
	Str &				operator+=( const unsigned a );
	Str &				operator+=( const bool a );
	Str &				operator+=( const CEGUI::String & a );

	// case sensitive compare
	bool				operator ==( const Str & b ) const;
	bool				operator ==( const char * b ) const;
	bool				operator ==( const char b ) const;

	// case sensitive compare
	bool				operator !=( const Str & b ) const;
	bool				operator !=( const char * b ) const;
	bool				operator !=( const char b ) const;

	// case sensitive compare
	int					Cmp( const char * text ) const;
	int					Cmpn( const char * text, int n ) const;
	int					CmpPrefix( const char * text ) const;

	// case insensitive compare
	int					Icmp( const char * text ) const;
	int					Icmpn( const char * text, int n ) const;
	int					IcmpPrefix( const char * text ) const;
	bool				IcmpFast( const char * text ) const;
	bool				IcmpFast( const Str & text ) const;
	bool				IcmpnFast( const char * text, int n ) const;

	// case insensitive compare ignoring color
	int					IcmpNoColor( const char * text ) const;

	// compares paths and makes sure folders come first
	int					IcmpPath( const char * text ) const;
	int					IcmpnPath( const char * text, int n ) const;
	int					IcmpPrefixPath( const char * text ) const;

	int					Length( ) const;
	int					Allocated( ) const;
	void				Empty( );
	bool				IsEmpty( ) const;
	void				Clear( );
	void				Append( const char a );
	void				Append( const Str & text );
	void				Append( const char * text );
	void				Append( const char * text, int len );
	void				Insert( const char a, int index );
	void				Insert( const char * text, int index );
	void				Set( const Str & text );
	void				Set( const char * text );
	void				Set( const char * text, int len );
	void				ToLower( );
	void				ToUpper( );
	bool				IsNumeric( ) const;
	bool				IsColor( ) const;
	bool				HasLower( ) const;
	bool				HasUpper( ) const;
	int					LengthWithoutColors( ) const;
	Str &				RemoveColors( );
	void				CapLength( int );
	void				Fill( const char ch, int newlen );

	int					Find( const char c, int start = 0, int end = -1 ) const;
	int					Find( const char * text, int start = 0, int end = -1 ) const;
	int					Ifind( const char * text, int start = 0, int end = -1 ) const;
	int					IfindFast( const char * text, int start = 0, int end = -1 ) const;
	bool				Filter( const char * filter, bool casesensitive ) const;
	// return the index to the last occurance of 'c', returns -1 if not found
	int					Last( const char c ) const;
	// store the leftmost 'len' characters in the result
	const char *		Left( int len, Str & result ) const;
	// store the rightmost 'len' characters in the result
	const char *		Right( int len, Str & result ) const;
	// store 'len' characters starting at 'start' in result
	const char *		Mid( int start, int len, Str & result ) const;
	// return the leftmost 'len' characters
	Str					Left( int len ) const;
	// return the rightmost 'len' characters
	Str					Right( int len ) const;
	// return 'len' characters starting at 'start'
	Str					Mid( int start, int len ) const;
	// strip char from front as many times as the char occurs
	void				StripLeading( const char c );
	// strip string from front as many times as the string occurs
	void				StripLeading( const char * string );
	// strip string from front just once if it occurs
	bool				StripLeadingOnce( const char * string );
	// strip char from end as many times as the char occurs
	void				StripTrailing( const char c );
	// strip string from end as many times as the string occurs
	void				StripTrailing( const char * string );
	// strip string from end just once if it occurs
	bool				StripTrailingOnce( const char * string );
	// strip char from front and end as many times as the char occurs
	void				Strip( const char c );
	// strip string from front and end as many times as the string occurs
	void				Strip( const char * string );
	// strip trailing white space characters
	void				StripTrailingWhitespace( );
	// Removes the quotes from the beginning and end of the string
	Str &				StripQuotes( );
	void				Replace( const char *old, const char *nw );

	// file name methods
	// hash key for the filename ( skips extension)
	int					FileNameHash( ) const;
	// convert slashes
	Str &				BackSlashesToSlashes( );
	// set the given file extension
	Str &				SetFileExtension( const char * extension );
	// remove any file extension
	Str &				StripFileExtension( );
	// remove any file extension looking from front ( useful if there are multiple .'s)
	Str &				StripAbsoluteFileExtension( );
	// if there's no file extension use the default
	Str &				DefaultFileExtension( const char * extension );
	// if there's no path use the default
	Str &				DefaultPath( const char * basepath );
	// append a partial path
	void				AppendPath( const char * text );
	// remove the filename from a path
	Str &				StripFilename( );
	// remove the path from the filename
	Str &				StripPath( );
	// copy the file path to another string
	void				ExtractFilePath( Str & dest ) const;
	// copy the filename to another string
	void				ExtractFileName( Str & dest ) const;
	// copy the filename minus the extension to another string
	void				ExtractFileBase( Str & dest ) const;
	// copy the file extension to another string
	void				ExtractFileExtension( Str & dest ) const;
	bool				CheckExtension( const char * ext );

	// char * methods to replace library functions
	static int			Length( const char * s );
	static char *		ToLower( char * s );
	static char *		ToUpper( char * s );
	// Checks a string to see if it contains only numerical values.
	static bool			IsNumeric( const char * s );
	static bool			IsColor( const char * s );
	// Checks if a string has any lowercase chars
	static bool			HasLower( const char * s );
	// Checks if a string has any uppercase chars
	static bool			HasUpper( const char * s );
	static int			LengthWithoutColors( const char * s );
	static char *		RemoveColors( char * s );
	static int			Cmp( const char * s1, const char * s2 );
	static int			Cmpn( const char * s1, const char * s2, int n );
	static int			Icmp( const char * s1, const char * s2 );
	static int			Icmpn( const char * s1, const char * s2, int n );
	static bool			IcmpFast( const char * s1, const char * s2 );
	static int			IcmpNoColor( const char * s1, const char * s2 );
	// compares paths and makes sure folders come first
	static int			IcmpPath( const char * s1, const char * s2 );
	// compares paths and makes sure folders come first
	static int			IcmpnPath( const char * s1, const char * s2, int n );
	// never goes past bounds or leaves without a terminating 0
	static void			Append( char * dest, int size, const char * src );
	// Safe strncpy that ensures a trailing zero
	static void			Copynz( char * dest, const char * src, int destsize );
	static int			snPrintf( char * dest, int size, const char * fmt, ... ) id_attribute( ( format( printf,3,4) ));
	static int			vsnPrintf( char * dest, int size, const char * fmt, va_list argptr );
	// returns -1 if not found otherwise the index of the char
	static int			FindChar( const char * str, const char c, int start = 0, int end = -1 );
	// returns -1 if not found otherwise the index of the text
	static int			FindText( const char * str, const char * text, int start = 0, int end = -1 );
	static int			IfindText( const char * str, const char * text, int start = 0, int end = -1 );
	// Returns true if the string conforms the given filter. Several metacharacter may be used in the filter.
	// *				match any string of zero or more characters
	// ?				match any single character
	// [ abc... ]		match any of the enclosed characters; a hyphen can be used to specify a range( e.g. a-z, A-Z, 0-9 )
	static bool			Filter( const char * filter, const char * name, bool casesensitive );
	// makes the string lower case, replaces backslashes with forward slashes, and removes extension
	static void			StripMediaName( const char * name, Str & mediaName );
	static bool			CheckExtension( const char * name, const char * ext );
	static void			StripFloatZeros( char * str, int & len );
	static const char *	FloatArrayToString( const float * farray, const int length, const int precision );

	// hash keys
	static int			Hash( const char * string );
	static int			Hash( const char * string, int length );
	// case insensitive
	static int			IHash( const char * string );
	// case insensitive
	static int			IHash( const char * string, int length );

	// character methods
	static char			ToLower( char c );
	static char			ToUpper( char c );
	static bool			CharIsPrintable( int c );
	static bool			CharIsLower( int c );
	static bool			CharIsUpper( int c );
	static bool			CharIsAlpha( int c );
	static bool			CharIsNumeric( int c );
	static bool			CharIsNewLine( char c );
	static bool			CharIsTab( char c );
	static int			ColorIndex( int c );

	// Sets the value of the string using a printf interface.
	friend int			sprintf( Str & dest, const char * fmt, ... );
	// Sets the value of the string using a vprintf interface.
	friend int			vsprintf( Str & dest, const char * fmt, va_list ap );

	// reallocate string data buffer
	void				ReAllocate( int amount, bool keepold );
	char *				ReSize( int space_amount, int new_size );

	// format value in the given measurement with the best unit, returns the best unit
	int					BestUnit( float value, Measure_t measure, const char * format = "%f " );
	int					BestUnit( int value, Measure_t measure, const char * format = "%i " );
	// format value in the requested unit and measurement
	void				SetUnit( float value, int unit, Measure_t measure, const char * format = "%f " );
	void				SetUnit( int value, int unit, Measure_t measure, const char * format = "%i " );

	static void			InitMemory( );
	static void			ShutdownMemory( );
	static void			PurgeMemory( );
	static void			ShowMemoryUsage_f( const CmdArgs & args );

	int					DynamicMemoryUsed( ) const;
	static Str			FormatNumber( int number );
};

#ifdef USE_STRING_DATA_ALLOCATOR
	extern DynamicBlockAlloc< char, 1 << 18, 128 >	g_stringDataAllocator;
#endif

extern const Str		null_string;
extern const Str		true_string;
extern const Str		false_string;

//char *					va( const char * fmt, ... ) id_attribute( ( format( printf, 1, 2 ) ) );

INLINE void Str::EnsureAlloced( int amount, bool keepold ) {
	if( amount > d_alloced )
		ReAllocate( amount, keepold );
}

INLINE void Str::Init( ) {
	d_data		= d_baseBuffer;
	d_data[ 0 ]	= 0;
	d_len		= 0;
	d_alloced	= STR_ALLOC_BASE;
#ifdef ID_DEBUG_UNINITIALIZED_MEMORY
	Common::Com_Memset( baseBuffer, 0, sizeof( baseBuffer ) );
#endif
}

INLINE Str::Str( ) {
	Init( );
}

INLINE Str::Str( const Str & text ) {
	Init( );
	int l = text.Length( );
	EnsureAlloced( l + 1 );
	Common::Com_Memcpy( d_data, text.d_data, l );
	d_data[ l ] = 0;
	d_len = l;
}

INLINE Str::Str( const Str & text, int start, int end ) {
	int l = text.Length( );
	assert( start >= 0 && start <= l && end <= l );
	Init( );
	l = Max( end - start, 0 );
	EnsureAlloced( l + 1 );
	Common::Com_Memcpy( d_data, text.d_data + start, l );
	d_data[ l ] = 0;
	d_len = l;
}

INLINE Str::Str( const char * text ) {
	Init( );
	if( text ) {
		int l = Length( text );
		EnsureAlloced( l + 1 );
		Copynz( d_data, text, l + 1 );
		d_len = l;
	}
}

INLINE Str::Str( const char * text, int start, int end ) {
	int l = Length( text );
	assert( start >= 0 && start <= l && end <= l );
	Init( );
	l = Max( end - start, 0 );
	EnsureAlloced( l + 1 );
	Common::Com_Memcpy( d_data, text + start, l );
	d_data[ l ] = 0;
	d_len = l;
}

INLINE Str::Str( const char * text, int len ) {
	Init( );
	EnsureAlloced( len + 1 );
	Common::Com_Memcpy( d_data, text, len );
	d_data[ len ] = 0;
	d_len = len;
}

INLINE Str::Str( const CEGUI::String & text ) {
	Init( );
	int l = ( int )text.d_cplength;
	EnsureAlloced( l + 1 );
	text.encode( text.ptr( ), ( CEGUI::utf8 * )d_data, text.d_cplength + 1, text.d_cplength );
	d_data[ l ] = 0;
	d_len = l;
}

INLINE Str::Str( const bool b ) {
	Init( );
	const Str & boolStr = b ? true_string : false_string;
	int l = boolStr.Length( );
	Common::Com_Memcpy( d_data, boolStr.d_data, l );
	d_data[ l ] = 0;
	d_len = l;
}

INLINE Str::Str( const char c ) {
	Init( );
	EnsureAlloced( 2 );
	d_data[ 0 ] = c;
	d_data[ 1 ] = 0;
	d_len = 1;
}

INLINE Str::Str( const int i ) {
	char text[ 64 ];
	Init( );
	int l = sprintf( text, "%d", i );
	EnsureAlloced( l + 1 );
	Copynz( d_data, text, l + 1 );
	d_len = l;
}

INLINE Str::Str( const unsigned u ) {
	char text[ 64 ];
	Init( );
	int l = sprintf( text, "%u", u );
	EnsureAlloced( l + 1 );
	Copynz( d_data, text, l + 1 );
	d_len = l;
}

INLINE Str::Str( const float f ) {
	char text[ 64 ];
	Init( );
	int l = Str::snPrintf( text, sizeof( text ), "%f", f );
	StripFloatZeros( text, l );
	EnsureAlloced( l + 1 );
	Copynz( d_data, text, l + 1 );
	d_len = l;
}

INLINE Str::~Str( ) {
	FreeData( );
}

INLINE size_t Str::Size( ) const {
	return sizeof( *this ) + Allocated( );
}

INLINE const char * Str::c_str( ) const {
	return d_data;
}

INLINE CEGUI::String Str::cegui_str( ) const {
	return CEGUI::String( d_data, ( CEGUI::String::size_type )d_len );
}

INLINE Str::operator const char *( ) {
	return d_data;
}

INLINE Str::operator const char *( ) const {
	return d_data;
}

INLINE char Str::operator[]( int index ) const {
	assert( ( index >= 0 ) && ( index <= d_len ) );
	return d_data[ index ];
}

INLINE char & Str::operator[]( int index ) {
	assert( ( index >= 0 ) && ( index <= d_len ) );
	return d_data[ index ];
}

INLINE void Str::operator =( const Str & text ) {
	int l = text.Length( );
	EnsureAlloced( l + 1, false );
	Common::Com_Memcpy( d_data, text.d_data, l );
	d_data[ l ] = 0;
	d_len = l;
}

INLINE void Str::operator =( const CEGUI::String & text ) {
	int l = ( int )text.d_cplength;
	EnsureAlloced( l + 1 );
	text.encode( text.ptr( ), ( CEGUI::utf8 * )d_data, text.d_cplength + 1, text.d_cplength );
	d_data[ l ] = 0;
	d_len = l;
}

INLINE Str operator+( const Str & a, const Str & b ) {
	Str result( a );
	result.Append( b );
	return result;
}

INLINE Str operator+( const Str & a, const char * b ) {
	Str result( a );
	result.Append( b );
	return result;
}

INLINE Str operator+( const char * a, const Str & b ) {
	Str result( a );
	result.Append( b );
	return result;
}

INLINE Str operator+( const Str & a, const bool b ) {
	Str result( a );
	result.Append( b ? "true" : "false" );
	return result;
}

INLINE Str operator+( const Str & a, const char b ) {
	Str result( a );
	result.Append( b );
	return result;
}

INLINE Str operator+( const Str & a, const float b ) {
	char	text[ 64 ];
	Str	result( a );
	sprintf( text, "%f", b );
	result.Append( text );
	return result;
}

INLINE Str operator+( const Str & a, const int b ) {
	char	text[ 64 ];
	Str	result( a );
	sprintf( text, "%d", b );
	result.Append( text );
	return result;
}

INLINE Str operator+( const Str & a, const unsigned b ) {
	char	text[ 64 ];
	Str	result( a );
	sprintf( text, "%u", b );
	result.Append( text );
	return result;
}

INLINE Str & Str::operator+=( const float a ) {
	char text[ 64 ];
	sprintf( text, "%f", a );
	Append( text );
	return *this;
}

INLINE Str & Str::operator+=( const int a ) {
	char text[ 64 ];
	sprintf( text, "%d", a );
	Append( text );
	return *this;
}

INLINE Str & Str::operator+=( const unsigned a ) {
	char text[ 64 ];
	sprintf( text, "%u", a );
	Append( text );
	return *this;
}

INLINE Str & Str::operator+=( const Str & a ) {
	Append( a );
	return *this;
}

INLINE Str & Str::operator+=( const char * a ) {
	Append( a );
	return *this;
}

INLINE Str & Str::operator+=( const char a ) {
	Append( a );
	return *this;
}

INLINE Str & Str::operator+=( const bool a ) {
	Append( a ? true_string : false_string );
	return *this;
}

INLINE Str & Str::operator+=( const CEGUI::String & a ) {
	int newLen = d_len + ( int )a.d_cplength;
	EnsureAlloced( newLen + 1 );
	a.encode( a.ptr( ), ( CEGUI::utf8 * )( d_data + d_len ), a.d_cplength + 1, a.d_cplength );
	d_data[ newLen ] = 0;
	d_len = newLen;
	return *this;
}

INLINE bool Str::operator ==( const Str & b ) const {
	if( d_len != b.d_len )
		return false;
	return BlockCmp( d_data, b.d_data, d_len );
}

INLINE bool Str::operator ==( const char * b ) const {
	if( d_len != Length( b ) )
		return false;
	return BlockCmp( d_data, b, d_len );
}

INLINE bool Str::operator ==( const char b ) const {
	return ( d_len == 1 ) && ( d_data[ 0 ] == b );
}

INLINE bool Str::operator !=( const Str & b ) const {
	if( d_len != b.d_len )
		return true;
	return !BlockCmp( d_data, b.d_data, d_len );
}

INLINE bool Str::operator !=( const char * b ) const {
	if( d_len != Length( b ) )
		return true;
	return !BlockCmp( d_data, b, d_len );
}

INLINE bool Str::operator !=( const char b ) const {
	return ( d_data[ 0 ] != b ) || ( d_len != 1 );
}

INLINE int Str::Cmp( const char * text ) const {
	assert( text );
	return Str::Cmp( d_data, text );
}

INLINE int Str::Cmpn( const char * text, int n ) const {
	assert( text );
	return Str::Cmpn( d_data, text, n );
}

INLINE int Str::CmpPrefix( const char * text ) const {
	assert( text );
	return Str::Cmpn( d_data, text, Length( text ) );
}

INLINE int Str::Icmp( const char * text ) const {
	assert( text );
	return Str::Icmp( d_data, text );
}

INLINE int Str::Icmpn( const char * text, int n ) const {
	assert( text );
	return Str::Icmpn( d_data, text, n );
}

INLINE int Str::IcmpPrefix( const char * text ) const {
	assert( text );
	return Str::Icmpn( d_data, text, Length( text ) );
}

INLINE bool Str::IcmpFast( const char * text ) const {
	assert( text );
	if( d_len != Length( text ) )
		return false;
	return IBlockCmp( d_data, text, d_len );
}

INLINE bool Str::IcmpFast( const Str & text ) const {
	if( d_len != text.Length( ) )
		return false;
	return IBlockCmp( d_data, text, d_len );
}

INLINE bool Str::IcmpnFast( const char * text, int n ) const {
	assert( text && d_len >= n && Length( text ) >= n );
	return IBlockCmp( d_data, text, n );
}

INLINE int Str::IcmpNoColor( const char * text ) const {
	assert( text );
	return Str::IcmpNoColor( d_data, text );
}

INLINE int Str::IcmpPath( const char * text ) const {
	assert( text );
	return Str::IcmpPath( d_data, text );
}

INLINE int Str::IcmpnPath( const char * text, int n ) const {
	assert( text );
	return Str::IcmpnPath( d_data, text, n );
}

INLINE int Str::IcmpPrefixPath( const char * text ) const {
	assert( text );
	return Str::IcmpnPath( d_data, text, Length( text ) );
}

INLINE int Str::Length( ) const {
	return d_len;
}

INLINE int Str::Allocated( ) const {
	return ( d_data != d_baseBuffer ) ? d_alloced : 0;
}

INLINE void Str::Empty( ) {
	EnsureAlloced( 1 );
	d_data[ 0 ] = 0;
	d_len = 0;
}

INLINE bool Str::IsEmpty( ) const {
	return d_data[ 0 ] == 0;
}

INLINE void Str::Clear( ) {
	FreeData( );
	Init( );
}

INLINE void Str::Append( const char a ) {
	EnsureAlloced( d_len + 2 );
	d_data[ d_len ] = a;
	d_len++;
	d_data[ d_len ] = 0;
}

INLINE void Str::Append( const Str & text ) {
	int l = text.Length( );
	int newLen = d_len + l;
	EnsureAlloced( newLen + 1 );
	Common::Com_Memcpy( d_data + d_len, text.d_data, l );
	d_data[ newLen ] = 0;
	d_len = newLen;
}

INLINE void Str::Append( const char * text ) {
	assert( text );
	int l = Length( text );
	int newLen = d_len + l;
	EnsureAlloced( newLen + 1 );
	Common::Com_Memcpy( d_data + d_len, text, l );
	d_data[ newLen ] = 0;
	d_len = newLen;
}

INLINE void Str::Append( const char * text, int l ) {
	assert( text && l );
	int newLen = d_len + l;
	EnsureAlloced( newLen + 1 );
	Common::Com_Memcpy( d_data + d_len, text, l );
	d_data[ newLen ] = 0;
	d_len = newLen;
}

INLINE void Str::Set( const Str & text ) {
	assert( text );
	int l = text.Length( );
	EnsureAlloced( l + 1 );
	Common::Com_Memcpy( d_data, text, l );
	d_data[ l ] = 0;
	d_len = l;
}

INLINE void Str::Set( const char * text ) {
	assert( text );
	int l = Length( text );
	EnsureAlloced( l + 1 );
	Common::Com_Memcpy( d_data, text, l );
	d_data[ l ] = 0;
	d_len = l;
}

INLINE void Str::Set( const char * text, int l ) {
	assert( text && l );
	EnsureAlloced( l + 1 );
	Common::Com_Memcpy( d_data, text, l );
	d_data[ l ] = 0;
	d_len = l;
}

INLINE void Str::Insert( const char a, int index ) {
	assert( ( index >= 0 ) && ( index <= d_len ) );
	const int l = 1;
	EnsureAlloced( d_len + l + 1 );
	memmove( d_data + index + l, d_data + index, ( d_len - index ) + 1 );
	d_data[ index ] = a;
	d_len++;
}

INLINE void Str::Insert( const char * text, int index ) {
	assert( ( index >= 0 ) && ( index <= d_len ) );
	int l = Length( text );
	EnsureAlloced( d_len + l + 1 );
	memmove( d_data + index + l, d_data + index, ( d_len - index ) + 1 );
	Common::Com_Memcpy( d_data + index, text, l );
	d_len += l;
}

INLINE void Str::ToLower( ) {
	for( int i = 0; d_data[ i ]; i++ ) {
		if( CharIsUpper( d_data[ i ] ) )
			d_data[ i ] += ( 'a' - 'A' );
	}
}

INLINE void Str::ToUpper( ) {
	for( int i = 0; d_data[ i ]; i++ ) {
		if( CharIsLower( d_data[ i ] ) )
			d_data[ i ] -= ( 'a' - 'A' );
	}
}

INLINE bool Str::IsNumeric( ) const {
	return Str::IsNumeric( d_data );
}

INLINE bool Str::IsColor( ) const {
	return Str::IsColor( d_data );
}

INLINE bool Str::HasLower( ) const {
	return Str::HasLower( d_data );
}

INLINE bool Str::HasUpper( ) const {
	return Str::HasUpper( d_data );
}

INLINE Str & Str::RemoveColors( ) {
	Str::RemoveColors( d_data );
	d_len = Length( d_data );
	return *this;
}

INLINE int Str::LengthWithoutColors( ) const {
	return Str::LengthWithoutColors( d_data );
}

INLINE void Str::CapLength( int newlen ) {
	if( d_len <= newlen )
		return;
	d_data[ newlen ] = 0;
	d_len = newlen;
}

INLINE void Str::Fill( const char ch, int newlen ) {
	EnsureAlloced( newlen + 1 );
	d_len = newlen;
	Common::Com_Memset( d_data, ch, d_len );
	d_data[ d_len ] = 0;
}

INLINE int Str::Find( const char c, int start, int end ) const {
	if( end == -1 )
		end = d_len;
	return Str::FindChar( d_data, c, start, end );
}

INLINE int Str::Find( const char * text, int start, int end ) const {
	if( end == -1 )
		end = d_len;
	return BlockFindText( d_data, end, text, Length( text ), start );
}

INLINE int Str::Ifind( const char * text, int start, int end ) const {
	if( end == -1 )
		end = d_len;
	return Str::IfindText( d_data, text, start, end );
}

INLINE int Str::IfindFast( const char * text, int start, int end ) const {
	if( end == -1 )
		end = d_len;
	return IBlockFindText( d_data, end, text, Length( text ), start );
}

INLINE bool Str::Filter( const char * filter, bool casesensitive ) const {
	return Str::Filter( filter, d_data, casesensitive );
}

INLINE int Str::Last( const char c ) const {	
	for( int i = d_len; i > 0; i-- ) {
		if( d_data[ i - 1 ] == c )
			return i - 1;
	}
	return -1;
}

INLINE const char * Str::Left( int len, Str & result ) const {
	return Mid( 0, len, result );
}

INLINE const char * Str::Right( int len, Str & result ) const {
	if( len >= d_len ) {
		result = *this;
		return result;
	}
	return Mid( d_len - len, len, result );
}

INLINE Str Str::Left( int len ) const {
	return Mid( 0, len );
}

INLINE Str Str::Right( int len ) const {
	if( len >= d_len )
		return *this;
	return Mid( d_len - len, len );
}

INLINE void Str::StripLeading( const char c ) {
	while( d_data[ 0 ] == c ) {
		memmove( &d_data[ 0 ], &d_data[ 1 ], d_len );
		d_len--;
	}
}

INLINE void Str::StripLeading( const char * string ) {
	int l = Length( string );
	if( l > 0 ) {
		while( !Cmpn( string, l ) ) {
			memmove( d_data, d_data + l, d_len - l + 1 );
			d_len -= l;
		}
	}
}

INLINE bool Str::StripLeadingOnce( const char * string ) {
	int l = Length( string );
	if( ( l > 0 ) && !Cmpn( string, l ) ) {
		memmove( d_data, d_data + l, d_len - l + 1 );
		d_len -= l;
		return true;
	}
	return false;
}

INLINE void Str::StripTrailing( const char c ) {	
	for( int i = d_len; i > 0 && d_data[ i - 1 ] == c; i-- ) {
		d_data[ i - 1 ] = 0;
		d_len--;
	}
}

INLINE void Str::StripTrailing( const char * string ) {
	int l = Length( string );
	assert( l > 0 );
	while( ( d_len >= l ) && !Cmpn( string, d_data + d_len - l, l ) ) {
		d_len -= l;
		d_data[ d_len ] = 0;
	}
}

INLINE bool Str::StripTrailingOnce( const char * string ) {
	int l = Length( string );
	assert( l > 0 );
	if( ( d_len >= l ) && !Cmpn( string, d_data + d_len - l, l ) ) {
		d_len -= l;
		d_data[ d_len ] = 0;
		return true;
	}
	return false;
}

INLINE void Str::Strip( const char c ) {
	StripLeading( c );
	StripTrailing( c );
}

INLINE void Str::Strip( const char * string ) {
	StripLeading( string );
	StripTrailing( string );
}

INLINE void Str::StripTrailingWhitespace( ) {	
	// cast to unsigned char to prevent stripping off high-ASCII characters
	for( int i = d_len; i > 0 && ( unsigned char )( d_data[ i - 1 ] ) <= ' '; i-- ) {
		d_data[ i - 1 ] = 0;
		d_len--;
	}
}

INLINE Str & Str::BackSlashesToSlashes( ) {
	for( int i = 0; i < d_len; i++ ) {
		if( d_data[ i ] == '\\' )
			d_data[ i ] = '/';
	}
	return *this;
}

INLINE Str & Str::SetFileExtension( const char * extension ) {
	StripFileExtension( );
	if( *extension != '.' )
		Append( '.' );
	Append( extension );
	return *this;
}

INLINE Str & Str::StripFileExtension( ) {
	for( int i = d_len - 1; i >= 0; i-- ) {
		if( d_data[ i ] == '.' ) {
			d_data[ i ] = 0;
			d_len = i;
			break;
		}
	}
	return *this;
}

INLINE Str & Str::StripAbsoluteFileExtension( ) {
	for( int i = 0; i < d_len; i++ ) {
		if( d_data[ i ] == '.' ) {
			d_data[ i ] = 0;
			d_len = i;
			break;
		}
	}
	return *this;
}

INLINE Str & Str::DefaultFileExtension( const char * extension ) {
	for( int i = d_len - 1; i >= 0; i-- ) {
		if( d_data[ i ] == '.' )
			return *this; // do nothing if the string already has an extension
	}
	if( *extension != '.' )
		Append( '.' );
	Append( extension );
	return *this;
}

INLINE Str & Str::DefaultPath( const char * basepath ) {
	if( ( ( *this )[ 0 ] == '/' ) || ( d_data[ 0 ] == '\\' ) ) // absolute path location
		return *this;
	*this = basepath + *this;
	return *this;
}

INLINE Str & Str::StripFilename( ) {
	int pos = d_len - 1;
	while( ( pos > 0 ) && ( d_data[ pos ] != '/' ) && ( d_data[ pos ] != '\\' ) )
		pos--;
	if( pos < 0 )
		pos = 0;
	CapLength( pos );
	return *this;
}

INLINE Str & Str::StripPath( ) {
	int pos = d_len;
	while( ( pos > 0 ) &&( d_data[ pos - 1 ] != '/' ) && ( d_data[ pos - 1 ] != '\\' ) )
		pos--;
	*this = Right( d_len - pos );
	return *this;
}

INLINE void Str::ExtractFilePath( Str & dest ) const {
	int pos = d_len;
	while( ( pos > 0 ) &&( d_data[ pos - 1 ] != '/' ) && ( d_data[ pos - 1 ] != '\\' ) )
		pos--;
	Left( pos, dest );
}

INLINE void Str::ExtractFileName( Str & dest ) const {
	int pos = d_len - 1;
	while( ( pos > 0 ) && ( d_data[ pos - 1 ] != '/' ) && ( d_data[ pos - 1 ] != '\\' ) )
		pos--;
	Right( d_len - pos, dest );
}

INLINE void Str::ExtractFileBase( Str & dest ) const {
	int pos = d_len - 1;
	while( ( pos > 0 ) && ( d_data[ pos - 1 ] != '/' ) && ( d_data[ pos - 1 ] != '\\' ) )
		pos--;
	int start = pos;
	while( ( pos < d_len ) &&( d_data[ pos ] != '.' ) )
		pos++;
	Mid( start, pos - start, dest );
}

INLINE void Str::ExtractFileExtension( Str & dest ) const {
	int pos = d_len - 1;
	while( ( pos > 0 ) && ( d_data[ pos - 1 ] != '.' ) )
		pos--;
	if( !pos ) // no extension
		dest.Empty( );
	else
		Right( d_len - pos, dest );
}

INLINE bool Str::CheckExtension( const char * ext ) {
	return Str::CheckExtension( d_data, ext );
}

INLINE int Str::Length( const char * s ) {
	int i;
	for( i = 0; s[ i ]; i++ );
	return i;
}

INLINE char * Str::ToLower( char * s ) {
	for( int i = 0; s[ i ]; i++ ) {
		if( CharIsUpper( s[ i ] ) )
			s[ i ] += ( 'a' - 'A' );
	}
	return s;
}

INLINE char * Str::ToUpper( char * s ) {
	for( int i = 0; s[ i ]; i++ ) {
		if( CharIsLower( s[ i ] ) )
			s[ i ] -= ( 'a' - 'A' );
	}
	return s;
}

INLINE int Str::Hash( const char * string ) {
	int hash = 0;
	for( int i = 0; *string != 0; i++ )
		hash += ( *string++ ) * ( i + 119 );
	return hash;
}

INLINE int Str::Hash( const char * string, int length ) {
	int hash = 0;
	for( int i = 0; i < length; i++ )
		hash += ( *string++ ) * ( i + 119 );
	return hash;
}

INLINE int Str::IHash( const char * string ) {
	int hash = 0;
	for( int i = 0; *string != 0; i++ )
		hash += ToLower( *string++ ) * ( i + 119 );
	return hash;
}

INLINE int Str::IHash( const char * string, int length ) {
	int hash = 0;
	for( int i = 0; i < length; i++ )
		hash += ToLower( *string++ ) * ( i + 119 );
	return hash;
}

INLINE bool Str::IsNumeric( const char * s ) {
	if( *s == '-' )
		s++;
	bool dot = false;
	for( int i = 0; s[ i ]; i++ ) {
		if( !CharIsNumeric( s[ i ] ) ) {
			if( ( s[ i ] == '.' ) && !dot ) {
				dot = true;
				continue;
			}
			return false;
		}
	}
	return true;
}

INLINE bool Str::IsColor( const char * s ) {
	return s[ 0 ] == C_COLOR_ESCAPE && s[ 1 ] != 0 && s[ 1 ] != ' ';
}

INLINE bool Str::HasLower( const char * s ) {
	assert( s );
	while( *s ) {
		if( CharIsLower( *s ) )
			return true;
		s++;
	}	
	return false;
}

INLINE bool Str::HasUpper( const char * s ) {
	assert( s );
	while( *s ) {
		if( CharIsUpper( *s ) )
			return true;
		s++;
	}	
	return false;
}

INLINE void Str::Copynz( char * dest, const char * src, int destsize ) {
	assert( dest && src && destsize > 0 );
	int i = 0;
	int copyLen = destsize - 1;
	for( ; ( i < copyLen ) && src[ i ]; i++ )
		dest[ i ] = src[ i ];
	for( ; i < destsize; i++ )
		dest[ i ] = 0;
}

INLINE void Str::Append( char * dest, int size, const char * src ) {
	int l1 = Length( dest );
	assert( l1 < size );
	Str::Copynz( dest + l1, src, size - l1 );
}

INLINE char Str::ToLower( char c ) {
	if( c <= 'Z' && c >= 'A' )
		return ( c + ( 'a' - 'A' ) );
	return c;
}

INLINE char Str::ToUpper( char c ) {
	if( c >= 'a' && c <= 'z' )
		return ( c - ( 'a' - 'A' ) );
	return c;
}

INLINE bool Str::CharIsPrintable( int c ) {
	// test for regular ascii and western European high-ascii chars
	return ( c >= 0x20 && c <= 0x7E ) || ( c >= 0xA1 && c <= 0xFF );
}

INLINE bool Str::CharIsLower( int c ) {
	// test for regular ascii and western European high-ascii chars
	return ( c >= 'a' && c <= 'z' ) || ( c >= 0xE0 && c <= 0xFF );
}

INLINE bool Str::CharIsUpper( int c ) {
	// test for regular ascii and western European high-ascii chars
	return ( c <= 'Z' && c >= 'A' ) || ( c >= 0xC0 && c <= 0xDF );
}

INLINE bool Str::CharIsAlpha( int c ) {
	// test for regular ascii and western European high-ascii chars
	return ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= 0xC0 && c <= 0xFF ) );
}

INLINE bool Str::CharIsNumeric( int c ) {
	return ( c <= '9' && c >= '0' );
}

INLINE bool Str::CharIsNewLine( char c ) {
	return ( c == '\n' || c == '\r' || c == '\v' );
}

INLINE bool Str::CharIsTab( char c ) {
	return ( c == '\t' );
}

INLINE int Str::ColorIndex( int c ) {
	return ( c & 15 );
}

INLINE bool Str::IcmpFast( const char * s1, const char * s2 ) {
	int l = Length( s1 );
	if( l != Length( s2 ) )
		return false;
	return IBlockCmp( s1, s2, l );
}

INLINE void Str::StripFloatZeros( char * str, int & len ) {
	while( len > 0 && str[ len - 1 ] == '0' )
		str[ --len ] = 0;
	if( len > 0 && str[ len - 1 ] == '.' )
		str[ --len ] = 0;
}

INLINE int Str::DynamicMemoryUsed( ) const {
	return ( d_data == d_baseBuffer ) ? 0 : d_alloced;
}

INLINE char * Str::ReSize( int space_amount, int new_size ) {
	ReAllocate( space_amount, false );
	d_len = Min( d_alloced - 1, new_size );
	return d_data;
}

INLINE const Str Vec4::ToString( int precision ) const {
	return Str::FloatArrayToString( ToFloatPtr( ), GetDimension( ), precision );
}

INLINE const Str Vec3::ToString( int precision ) const {
	return Str::FloatArrayToString( ToFloatPtr( ), GetDimension( ), precision );
}

INLINE const Str Vec2::ToString( int precision ) const {
	return Str::FloatArrayToString( ToFloatPtr( ), GetDimension( ), precision );
}

#endif
