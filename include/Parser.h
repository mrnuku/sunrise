// Copyright ( C) 2004 Id Software, Inc.
//

#ifndef __PARSER_H__
#define __PARSER_H__

/*
===============================================================================

	C/C++ compatible pre-compiler

===============================================================================
*/

#define DEFINE_FIXED			0x0001

#define BUILTIN_LINE			1
#define BUILTIN_FILE			2
#define BUILTIN_DATE			3
#define BUILTIN_TIME			4
#define BUILTIN_STDC			5

#define INDENT_IF				0x0001
#define INDENT_ELSE				0x0002
#define INDENT_ELIF				0x0004
#define INDENT_IFDEF			0x0008
#define INDENT_IFNDEF			0x0010

// macro definitions
typedef struct define_s {
	char *			name;						// define name
	int				flags;						// define flags
	int				builtin;					// > 0 if builtin define
	int				numparms;					// number of define parameters
	Token *		parms;						// define parameters
	Token *		tokens;						// macro tokens ( possibly containing parm tokens)
	struct define_s	*next;						// next defined macro in a list
	struct define_s	*hashnext;					// next define in the hash chain
} define_t;

// indents used for conditional compilation directives:
// #if, #else, #elif, #ifdef, #ifndef
typedef struct indent_s {
	int				type;						// indent type
	int				skip;						// true if skipping current indent
	Lexer *		script;						// script the indent was in
	struct indent_s	*next;						// next indent on the indent stack
} indent_t;


class Parser {

public:
					// constructor
					Parser( );
					Parser( int flags );
					Parser( const char *filename, int flags = 0 );
					Parser( const char *ptr, int length, const char *name, int flags = 0 );
					// destructor
					~Parser( );
					// load a source file
	int				LoadFile( const char *filename );
					// load a source from the given memory with the given length
					// NOTE: the ptr is expected to point at a valid C string: ptr[length] == '\0'
	int				LoadMemory( const char *ptr, int length, const char *name );
					// free the current source
	void			FreeSource( bool keepDefines = false );
					// returns true if a source is loaded
	int				IsLoaded( ) const { return Parser::loaded; }
					// read a token from the source
	int				ReadToken( Token *token );
					// expect a certain token, reads the token when available
	int				ExpectTokenString( const char *string );
					// expect a certain token type
	int				ExpectTokenType( int type, int subtype, Token *token );
					// expect a token
	int				ExpectAnyToken( Token *token );
					// returns true if the next token equals the given string and removes the token from the source
	int				CheckTokenString( const char *string );
					// returns true if the next token equals the given type and removes the token from the source
	int				CheckTokenType( int type, int subtype, Token *token );
					// returns true if the next token equals the given string but does not remove the token from the source
	int				PeekTokenString( const char *string );
					// returns true if the next token equals the given type but does not remove the token from the source
	int				PeekTokenType( int type, int subtype, Token *token );
					// skip tokens until the given token string is read
	int				SkipUntilString( const char *string );
					// skip the rest of the current line
	int				SkipRestOfLine( );
					// skip the braced section
	int				SkipBracedSection( bool parseFirstBrace = true );
					// parse a braced section into a string
	const char *	ParseBracedSection( Str &out, int tabs = -1 );
					// parse a braced section into a string, maintaining indents and newlines
	const char *	ParseBracedSectionExact( Str &out, int tabs = -1 );
					// parse the rest of the line
	const char *	ParseRestOfLine( Str &out );
					// unread the given token
	void			UnreadToken( Token *token );
					// read a token only if on the current line
	int				ReadTokenOnLine( Token *token );
					// read a signed integer
	int				ParseInt( );
					// read a boolean
	bool			ParseBool( );
					// read a floating point number
	float			ParseFloat( );
					// parse matrices with floats
	int				Parse1DMatrix( int x, float *m );
	int				Parse2DMatrix( int y, int x, float *m );
	int				Parse3DMatrix( int z, int y, int x, float *m );
					// get the white space before the last read token
	int				GetLastWhiteSpace( Str &whiteSpace ) const;
					// Set a marker in the source file ( there is only one marker)
	void			SetMarker( );
					// Get the string from the marker to the current position
	void			GetStringFromMarker( Str& out, bool clean = false );
					// add a define to the source
	int				AddDefine( const char *string );
					// add builtin defines
	void			AddBuiltinDefines( );
					// set the source include path
	void			SetIncludePath( const char *path );
					// set the punctuation set
	void			SetPunctuations( const punctuation_t *p );
					// returns a pointer to the punctuation with the given id
	const char *	GetPunctuationFromId( int  );
					// get the  for the given punctuation
	int				GetPunctuationId( const char *p );
					// set lexer flags
	void			SetFlags( int flags );
					// get lexer flags
	int				GetFlags( ) const;
					// returns the current filename
	const char *	GetFileName( ) const;
					// get current offset in current script
	const int		GetFileOffset( ) const;
					// get file time for current script
	timeStamp_t		GetFileTime( ) const;
					// returns the current line number
	const int		GetLineNum( ) const;
					// print an error message
	void			Error( const char *str, ... ) const id_attribute( ( format( printf,2,3) ));
					// print a warning message
	void			Warning( const char *str, ... ) const id_attribute( ( format( printf,2,3) ));

					// add a global define that will be added to all opened sources
	static int		AddGlobalDefine( const char *string );
					// remove the given global define
	static int		RemoveGlobalDefine( const char *name );
					// remove all global defines
	static void		RemoveAllGlobalDefines( );

private:
	int				loaded;						// set when a source file is loaded from file or memory
	Str			filename;					// file name of the script
	Str			includepath;				// path to include files
	const punctuation_t *punctuations;			// punctuations to use
	int				flags;						// flags used for script parsing
	Lexer *		scriptstack;				// stack with scripts of the source
	Token *		tokens;						// tokens to read first
	define_t *		defines;					// list with macro definitions
	define_t **		definehash;					// hash chain with defines
	indent_t *		indentstack;				// stack with indents
	int				skip;						// > 0 if skipping conditional code
	const char*		marker_p;

	static define_t *globaldefines;				// list with global defines added to every source loaded

private:
	void			PushIndent( int type, int skip );
	void			PopIndent( int *type, int *skip );
	void			PushScript( Lexer *script );
	int				ReadSourceToken( Token *token );
	int				ReadLine( Token *token );
	int				UnreadSourceToken( Token *token );
	int				ReadDefineParms( define_t *define, Token **parms, int maxparms );
	int				StringizeTokens( Token *tokens, Token *token );
	int				MergeTokens( Token *t1, Token *t2 );
	int				ExpandBuiltinDefine( Token *deftoken, define_t *define, Token **firsttoken, Token **lasttoken );
	int				ExpandDefine( Token *deftoken, define_t *define, Token **firsttoken, Token **lasttoken );
	int				ExpandDefineIntoSource( Token *deftoken, define_t *define );
	void			AddGlobalDefinesToSource( );
	define_t *		CopyDefine( define_t *define );
	define_t *		FindHashedDefine( define_t **definehash, const char *name);
	int				FindDefineParm( define_t *define, const char *name );
	void			AddDefineToHash( define_t *define, define_t **definehash);
	static void		PrintDefine( define_t *define );
	static void		FreeDefine( define_t *define );
	static define_t *FindDefine( define_t *defines, const char *name );
	static define_t *DefineFromString( const char *string);
	define_t *		CopyFirstDefine( );
	int				Directive_include( );
	int				Directive_undef( );
	int				Directive_if_def( int type );
	int				Directive_ifdef( );
	int				Directive_ifndef( );
	int				Directive_else( );
	int				Directive_endif( );
	int				EvaluateTokens( Token *tokens, signed long int *intvalue, double *floatvalue, int integer );
	int				Evaluate( signed long int *intvalue, double *floatvalue, int integer );
	int				DollarEvaluate( signed long int *intvalue, double *floatvalue, int integer);
	int				Directive_define( );
	int				Directive_elif( );
	int				Directive_if( );
	int				Directive_line( );
	int				Directive_error( );
	int				Directive_warning( );
	int				Directive_pragma( );
	void			UnreadSignToken( );
	int				Directive_eval( );
	int				Directive_evalfloat( );
	int				ReadDirective( );
	int				DollarDirective_evalint( );
	int				DollarDirective_evalfloat( );
	int				ReadDollarDirective( );
};

INLINE const char *Parser::GetFileName( ) const {
	if ( Parser::scriptstack ) {
		return Parser::scriptstack->GetFileName( );
	}
	else {
		return "";
	}
}

INLINE const int Parser::GetFileOffset( ) const {
	if ( Parser::scriptstack ) {
		return Parser::scriptstack->GetFileOffset( );
	}
	else {
		return 0;
	}
}

INLINE timeStamp_t Parser::GetFileTime( ) const {
	if ( Parser::scriptstack ) {
		return Parser::scriptstack->GetFileTime( );
	}
	else {
		return 0;
	}
}

INLINE const int Parser::GetLineNum( ) const {
	if ( Parser::scriptstack ) {
		return Parser::scriptstack->GetLineNum( );
	}
	else {
		return 0;
	}
}

#endif /* !__PARSER_H__ */
