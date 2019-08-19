#ifndef LEXER_H
#define LEXER_H

// lexer flags
typedef enum {

	LEXFL_NOERRORS						= BIT( 0 ),		// don't print any errors
	LEXFL_NOWARNINGS					= BIT( 1 ),		// don't print any warnings
	LEXFL_NOFATALERRORS					= BIT( 2 ),		// errors aren't fatal
	LEXFL_NOSTRINGCONCAT				= BIT( 3 ),		// multiple strings seperated by whitespaces are not concatenated
	LEXFL_NOSTRINGESCAPECHARS			= BIT( 4 ),		// no escape characters inside strings
	LEXFL_NODOLLARPRECOMPILE			= BIT( 5 ),		// don't use the $ sign for precompilation
	LEXFL_NOBASEINCLUDES				= BIT( 6 ),		// don't include files embraced with < >
	LEXFL_ALLOWPATHNAMES				= BIT( 7 ),		// allow path seperators in names
	LEXFL_ALLOWNUMBERNAMES				= BIT( 8 ),		// allow names to start with a number
	LEXFL_ALLOWIPADDRESSES				= BIT( 9 ),		// allow ip addresses to be parsed as numbers
	LEXFL_ALLOWFLOATEXCEPTIONS			= BIT( 10 ),	// allow float exceptions like 1.#INF or 1.#IND to be parsed
	LEXFL_ALLOWMULTICHARLITERALS		= BIT( 11 ),	// allow multi character literals
	LEXFL_ALLOWBACKSLASHSTRINGCONCAT	= BIT( 12 ),	// allow multiple strings seperated by '\' to be concatenated
	LEXFL_ONLYSTRINGS					= BIT( 13 )		// parse as whitespace deliminated strings ( quoted strings keep quotes)

} lexerFlags_t;

// punctuation s
enum {

	P_RSHIFT_ASSIGN = 1, P_LSHIFT_ASSIGN,
	P_PARMS,
	P_PRECOMPMERGE,

	P_LOGIC_AND, P_LOGIC_OR, P_LOGIC_GEQ, P_LOGIC_LEQ, P_LOGIC_EQ, P_LOGIC_UNEQ,

	P_MUL_ASSIGN, P_DIV_ASSIGN, P_MOD_ASSIGN, P_ADD_ASSIGN, P_SUB_ASSIGN,
	P_INC, P_DEC,

	P_BIN_AND_ASSIGN, P_BIN_OR_ASSIGN, P_BIN_XOR_ASSIGN,
	P_RSHIFT, P_LSHIFT,

	P_POINTERREF,
	P_CPP1, P_CPP2,
	P_MUL, P_DIV, P_MOD, P_ADD, P_SUB, P_ASSIGN,

	P_BIN_AND, P_BIN_OR, P_BIN_XOR, P_BIN_NOT,

	P_LOGIC_NOT, P_LOGIC_GREATER, P_LOGIC_LESS,

	P_REF,
	P_COMMA,
	P_SEMICOLON,
	P_COLON,
	P_QUESTIONMARK,

	P_PARENTHESESOPEN, P_PARENTHESESCLOSE,
	P_BRACEOPEN, P_BRACECLOSE,
	P_SQBRACKETOPEN, P_SQBRACKETCLOSE,
	P_BACKSLASH,

	P_PRECOMP,
	P_DOLLAR
};

// punctuation
typedef struct punctuation_s {

	const char * p;						// punctuation character( s)
	int n;							// punctuation id

} punctuation_t;

/*
===============================================================================

	Lexicographical parser

	Does not use memory allocation during parsing. The lexer uses no
	memory allocation if a source is loaded with LoadMemory( ).
	However, Token may still allocate memory for large strings.
	
	A number directly following the escape character '\' in a string is
	assumed to be in decimal format instead of octal. Binary numbers of
	the form 0b.. or 0B.. can also be used.

===============================================================================
*/

class Lexer {
private:

	friend class	Parser;

	int				loaded;					// set when a script file is loaded from file or memory
	Str				filename;				// file name of the script
	int				allocated;				// true if buffer memory was allocated
	const char *	buffer;					// buffer containing the script
	const char *	script_p;				// current pointer in the script
	const char *	end_p;					// pointer to the end of the script
	const char *	lastScript_p;			// script pointer before reading token
	const char *	whiteSpaceStart_p;		// start of last white space
	const char *	whiteSpaceEnd_p;		// end of last white space
	timeStamp_t		fileTime;				// file time
	int				length;					// length of the script in bytes
	int				line;					// current line in script
	int				lastline;				// line before reading token
	int				tokenavailable;			// set by unreadToken
	int				flags;					// several script flags
	const punctuation_t *punctuations;		// the punctuations used in the script
	int *			punctuationtable;		// ASCII table with punctuations
	int *			nextpunctuation;		// next punctuation in chain
	Token			token;					// available token
	Lexer *			next;					// next script in a chain
	bool			hadError;				// set by Lexer::Error, even if the error is supressed

	void			CreatePunctuationTable( const punctuation_t *punctuations );
	int				ReadWhiteSpace( );
	int				ReadEscapeCharacter( char *ch );
	int				ReadString( Token *token, int quote );
	int				ReadName( Token *token );
	int				ReadNumber( Token *token );
	int				ReadPunctuation( Token *token );
	int				ReadPrimitive( Token *token );
	int				CheckString( const char *str ) const;
	int				NumLinesCrossed( );

public:
					/// constructor
					Lexer( );
					Lexer( int flags );
					Lexer( const char *filename, int flags = 0 );
					Lexer( const char *ptr, int length, const char *name, int flags = 0 );

					/// destructor
					~Lexer( );

	/// load a script from the given file at the given offset with the given length
	int				LoadFile( const char *filename );

	/// load a script from the given memory with the given length and a specified line offset,
	/// so source strings extracted from a file can still refer to proper line numbers in the file
	/// NOTE: the ptr is expected to point at a valid C string: ptr[length] == '\0'
	int				LoadMemory( const char *ptr, int length, const char *name, int startLine = 1 );

	/// free the script
	void			FreeSource( );

	/// returns true if a script is loaded
	int				IsLoaded( ) { return Lexer::loaded; };

	/// read a token
	int				ReadToken( Token *token );

	/// expect a certain token, reads the token when available
	int				ExpectTokenString( const char *string );

	/// expect a certain token, reads the token when available
	int				ExpectTokenChar( const char c );

	/// expect a certain token type
	int				ExpectTokenType( int type, int subtype, Token *token );

	/// expect a token
	int				ExpectAnyToken( Token *token );

	/// returns true when the token is available
	int				CheckTokenString( const char *string );

	/// returns true an reads the token when a token with the given type is available
	int				CheckTokenType( int type, int subtype, Token *token );

	/// returns true if the next token equals the given string but does not remove the token from the source
	int				PeekTokenString( const char *string );

	/// returns true if the next token equals the given type but does not remove the token from the source
	int				PeekTokenType( int type, int subtype, Token *token );

	/// skip tokens until the given token string is read
	int				SkipUntilString( const char *string );

	/// skip the rest of the current line
	int				SkipRestOfLine( );

	/// skip the braced section
	int				SkipBracedSection( bool parseFirstBrace = true );

	/// unread the given token
	void			UnreadToken( const Token *token );

	/// read a token only if on the same line
	int				ReadTokenOnLine( Token *token );
		
	///Returns the rest of the current line
	const char*		ReadRestOfLine( Str& out);

	/// read a signed integer
	int				ParseInt( );

	/// read a boolean
	bool			ParseBool( );

	/// read a floating point number.  If errorFlag is NULL, a non-numeric token will
	/// issue an Error( ).  If it isn't NULL, it will issue a Warning( ) and set *errorFlag = true
	float			ParseFloat( bool *errorFlag = NULL );
	int				ParseFloatArray( int x, float *m );

	/// parse matrices with floats
	int				Parse1DMatrix( int x, float *m );
	int				Parse2DMatrix( int y, int x, float *m );
	int				Parse3DMatrix( int z, int y, int x, float *m );

	/// parse a braced section into a string
	const char *	ParseBracedSection( Str &out );

	/// parse a braced section into a string, maintaining indents and newlines
	const char *	ParseBracedSectionExact ( Str &out, int tabs = -1 );

	/// parse the rest of the line
	const char *	ParseRestOfLine( Str &out );

	/// retrieves the white space characters before the last read token
	int				GetLastWhiteSpace( Str &whiteSpace ) const;

	/// returns start index into text buffer of last white space
	int				GetLastWhiteSpaceStart( ) const;

	/// returns end index into text buffer of last white space
	int				GetLastWhiteSpaceEnd( ) const;

	/// set an array with punctuations, NULL restores default C/C++ set, see default_punctuations for an example
	void			SetPunctuations( const punctuation_t *p );

	/// returns a pointer to the punctuation with the given id
	const char *	GetPunctuationFromId( int  );

	/// get the  for the given punctuation
	int				GetPunctuationId( const char *p );

	/// set lexer flags
	void			SetFlags( int flags );

	/// get lexer flags
	int				GetFlags( );

	/// reset the lexer
	void			Reset( );

	/// returns true if at the end of the file
	bool			EndOfFile( ) const;

	/// returns the current filename
	const char *	GetFileName( );

	/// get offset in script
	const int		GetFileOffset( );

	/// get file time
	timeStamp_t		GetFileTime( );

	/// returns the current line number
	const int		GetLineNum( );

	/// print an error message
	void			Error( const char *str, ... ) id_attribute( ( format( printf,2,3) ));

	/// print a warning message
	void			Warning( const char *str, ... ) id_attribute( ( format( printf,2,3) ));

	/// returns true if Error( ) was called with LEXFL_NOFATALERRORS or LEXFL_NOERRORS set
	bool			HadError( ) const;
};

INLINE Lexer::Lexer( ) {
	Lexer::loaded = false;
	Lexer::filename = "";
	Lexer::flags = 0;
	Lexer::SetPunctuations( NULL );
	Lexer::allocated = false;
	Lexer::fileTime = 0;
	Lexer::length = 0;
	Lexer::line = 0;
	Lexer::lastline = 0;
	Lexer::tokenavailable = 0;
	Lexer::token = "";
	Lexer::next = NULL;
	Lexer::hadError = false;
}

INLINE Lexer::Lexer( int flags ) {
	Lexer::loaded = false;
	Lexer::filename = "";
	Lexer::flags = flags;
	Lexer::SetPunctuations( NULL );
	Lexer::allocated = false;
	Lexer::fileTime = 0;
	Lexer::length = 0;
	Lexer::line = 0;
	Lexer::lastline = 0;
	Lexer::tokenavailable = 0;
	Lexer::token = "";
	Lexer::next = NULL;
	Lexer::hadError = false;
}

INLINE Lexer::Lexer( const char * filename, int flags ) {
	Lexer::loaded = false;
	Lexer::flags = flags;
	Lexer::SetPunctuations( NULL );
	Lexer::allocated = false;
	Lexer::token = "";
	Lexer::next = NULL;
	Lexer::hadError = false;
	Lexer::LoadFile( filename );
}

INLINE Lexer::Lexer( const char * ptr, int length, const char * name, int flags ) {
	//Lexer::loaded = false;
	Lexer::flags = flags;
	Lexer::SetPunctuations( NULL );
	Lexer::allocated = false;
	Lexer::token = "";
	Lexer::next = NULL;
	Lexer::hadError = false;
	//Lexer::LoadMemory( ptr, length, name );
	Lexer::filename = name;
	Lexer::buffer = ptr;
	Lexer::fileTime = 0;
	Lexer::length = length;
	// pointer in script buffer
	Lexer::script_p = Lexer::buffer;
	// pointer in script buffer before reading token
	Lexer::lastScript_p = Lexer::buffer;
	// pointer to end of script buffer
	Lexer::end_p = &( Lexer::buffer[ length ] );
	Lexer::tokenavailable = 0;
	Lexer::line = 1;
	Lexer::lastline = 1;
	Lexer::allocated = false;
	Lexer::loaded = true;
}

INLINE Lexer::~Lexer( ) {
	Lexer::FreeSource( );
}

INLINE const char *Lexer::GetFileName( ) {
	return Lexer::filename;
}

INLINE const int Lexer::GetFileOffset( ) {
	return ( int )( Lexer::script_p - Lexer::buffer );
}

INLINE timeStamp_t Lexer::GetFileTime( ) {
	return Lexer::fileTime;
}

INLINE const int Lexer::GetLineNum( ) {
	return Lexer::line;
}

INLINE void Lexer::SetFlags( int flags ) {
	Lexer::flags = flags;
}

INLINE int Lexer::GetFlags( ) {
	return Lexer::flags;
}

INLINE int Lexer::GetLastWhiteSpaceStart( ) const {
	return ( int )( whiteSpaceStart_p - buffer );
}

INLINE int Lexer::GetLastWhiteSpaceEnd( ) const {
	return ( int )( whiteSpaceEnd_p - buffer );
}

INLINE bool Lexer::EndOfFile( ) const {
	return Lexer::script_p >= Lexer::end_p;
}

INLINE int Lexer::NumLinesCrossed( ) {
	return Lexer::line - Lexer::lastline;
}

INLINE bool Lexer::HadError( ) const {
	return hadError;
}

#endif
