// Copyright( C ) 2004 Id Software, Inc.
//

#include "precompiled.h"
#pragma hdrstop

#define PUNCTABLE

//longer punctuations first
punctuation_t default_punctuations[ ] = {

	//binary operators
	{">>=", P_RSHIFT_ASSIGN},
	{"<<=", P_LSHIFT_ASSIGN},
	{"...", P_PARMS},						//define merge operator
	{"##", P_PRECOMPMERGE}, 				// pre-compiler

	//logic operators
	{"&&", P_LOGIC_AND}, 					// pre-compiler
	{"||", P_LOGIC_OR}, 					// pre-compiler
	{">=", P_LOGIC_GEQ}, 					// pre-compiler
	{"<=", P_LOGIC_LEQ}, 					// pre-compiler
	{"==", P_LOGIC_EQ}, 					// pre-compiler
	{"!=", P_LOGIC_UNEQ}, 					// pre-compiler

	//arithmatic operators
	{"*=", P_MUL_ASSIGN},
	{"/=", P_DIV_ASSIGN},
	{"%=", P_MOD_ASSIGN},
	{"+=", P_ADD_ASSIGN},
	{"-=", P_SUB_ASSIGN},
	{"++", P_INC},
	{"--", P_DEC}, //binary operators
	{"&=", P_BIN_AND_ASSIGN},
	{"|=", P_BIN_OR_ASSIGN},
	{"^=", P_BIN_XOR_ASSIGN},
	{">>", P_RSHIFT}, 					// pre-compiler
	{"<<", P_LSHIFT}, 					// pre-compiler

	//reference operators
	{"->", P_POINTERREF},				//C++
	{"::", P_CPP1},
	{".* ", P_CPP2},					//arithmatic operators
	{"*", P_MUL}, 						// pre-compiler
	{"/", P_DIV}, 						// pre-compiler
	{"%", P_MOD}, 						// pre-compiler
	{"+", P_ADD}, 						// pre-compiler
	{"-", P_SUB}, 						// pre-compiler
	{"=", P_ASSIGN},
	
	//binary operators
	{"&", P_BIN_AND}, 					// pre-compiler
	{"|", P_BIN_OR}, 					// pre-compiler
	{"^", P_BIN_XOR}, 					// pre-compiler
	{"~", P_BIN_NOT}, 					// pre-compiler

	//logic operators
	{"!", P_LOGIC_NOT}, 				// pre-compiler
	{">", P_LOGIC_GREATER}, 			// pre-compiler
	{"<", P_LOGIC_LESS}, 				// pre-compiler

	//reference operator
	{".", P_REF},						//seperators
	{",", P_COMMA}, 					// pre-compiler
	{";", P_SEMICOLON},					//label indication
	{":", P_COLON}, 					// pre-compiler

	//if statement
	{"?", P_QUESTIONMARK}, 				// pre-compiler

	//embracements
	{"(", P_PARENTHESESOPEN}, 			// pre-compiler
	{")", P_PARENTHESESCLOSE}, 		// pre-compiler
	{"{", P_BRACEOPEN}, 				// pre-compiler
	{"}", P_BRACECLOSE}, 				// pre-compiler
	{"[", P_SQBRACKETOPEN},
	{"]", P_SQBRACKETCLOSE},
	{"\\", P_BACKSLASH},				//precompiler operator
	{"#", P_PRECOMP}, 					// pre-compiler
	{"$", P_DOLLAR},
	{NULL, 0}

};

int default_punctuationtable[ 256 ];
int default_nextpunctuation[sizeof( default_punctuations ) / sizeof( punctuation_t )];
int default_setup;

/*
================
Lexer::CreatePunctuationTable
================
*/
void Lexer::CreatePunctuationTable( const punctuation_t * punctuations ) {
	int i, n, lastp;
	const punctuation_t * p, * newp;

	//get memory for the table
	if( punctuations == default_punctuations ) {
		Lexer::punctuationtable = default_punctuationtable;
		Lexer::nextpunctuation = default_nextpunctuation;
		if( default_setup ) {
			return;
		}
		default_setup = true;
		i = sizeof( default_punctuations ) / sizeof( punctuation_t );
	}
	else {
		if( !Lexer::punctuationtable || Lexer::punctuationtable == default_punctuationtable ) {
			Lexer::punctuationtable =( int * ) Mem_Alloc( 256 * sizeof( int ) );
		}
		if( Lexer::nextpunctuation && Lexer::nextpunctuation != default_nextpunctuation ) {
			Mem_Free( Lexer::nextpunctuation );
		}
		for( i = 0; punctuations[ i ].p; i++ ) {
		}
		Lexer::nextpunctuation =( int * ) Mem_Alloc( i * sizeof( int ) );
	}
	Common::Com_Memset( Lexer::punctuationtable, 0xFF, 256 * sizeof( int ) );
	Common::Com_Memset( Lexer::nextpunctuation, 0xFF, i * sizeof( int ) );
	//add the punctuations in the list to the punctuation table
	for( i = 0; punctuations[ i ].p; i++ ) {
		newp = &punctuations[ i ];
		lastp = -1;
		//sort the punctuations in this table entry on length( longer punctuations first )
		for( n = Lexer::punctuationtable[( unsigned int ) newp->p[ 0 ]]; n >= 0; n = Lexer::nextpunctuation[ n ] ) {
			p = &punctuations[ n ];
			if( strlen( p->p ) < strlen( newp->p ) ) {
				Lexer::nextpunctuation[ i ] = n;
				if( lastp >= 0 ) {
					Lexer::nextpunctuation[ lastp ] = i;
				}
				else {
					Lexer::punctuationtable[( unsigned int ) newp->p[ 0 ]] = i;
				}
				break;
			}
			lastp = n;
		}
		if( n < 0 ) {
			Lexer::nextpunctuation[ i ] = -1;
			if( lastp >= 0 ) {
				Lexer::nextpunctuation[ lastp ] = i;
			}
			else {
				Lexer::punctuationtable[( unsigned int ) newp->p[ 0 ]] = i;
			}
		}
	}
}

/*
================
Lexer::GetPunctuationFromId
================
*/
const char * Lexer::GetPunctuationFromId( int id ) {
	int i;

	for( i = 0; Lexer::punctuations[ i ].p; i++ ) {
		if( Lexer::punctuations[ i ].n == id ) {
			return Lexer::punctuations[ i ].p;
		}
	}
	return "unkown punctuation";
}

/*
================
Lexer::GetPunctuationId
================
*/
int Lexer::GetPunctuationId( const char * p ) {
	int i;

	for( i = 0; Lexer::punctuations[ i ].p; i++ ) {
		if( !strcmp( Lexer::punctuations[ i ].p, p ) ) {
			return Lexer::punctuations[ i ].n;
		}
	}
	return 0;
}

/*
================
Lexer::Error
================
*/
void Lexer::Error( const char * str, ... ) {
	char text[ MAX_STRING_CHARS ];
	va_list ap;

	hadError = true;

	if( Lexer::flags & LEXFL_NOERRORS ) {
		return;
	}

	va_start( ap, str );
	vsprintf( text, str, ap );
	va_end( ap );

	if( Lexer::flags & LEXFL_NOFATALERRORS ) {
		Common::Com_DPrintf( "file %s, line %d: %s", Lexer::filename.c_str( ), Lexer::line, text );
	} else {
		Common::Com_Error( ERR_DISCONNECT, "file %s, line %d: %s", Lexer::filename.c_str( ), Lexer::line, text );
	}
}

/*
================
Lexer::Warning
================
*/
void Lexer::Warning( const char * str, ... ) {
	char text[ MAX_STRING_CHARS ];
	va_list ap;

	if( Lexer::flags & LEXFL_NOWARNINGS ) {
		return;
	}

	va_start( ap, str );
	vsprintf( text, str, ap );
	va_end( ap );
	Common::Com_DPrintf( "file %s, line %d: %s", Lexer::filename.c_str( ), Lexer::line, text );
}

/*
================
Lexer::SetPunctuations
================
*/
void Lexer::SetPunctuations( const punctuation_t * p ) {
#ifdef PUNCTABLE
	if( p ) {
		Lexer::CreatePunctuationTable( p );
	}
	else {
		Lexer::CreatePunctuationTable( default_punctuations );
	}
#endif //PUNCTABLE
	if( p ) {
		Lexer::punctuations = p;
	}
	else {
		Lexer::punctuations = default_punctuations;
	}
}

/*
================
Lexer::ReadWhiteSpace

Reads spaces, tabs, C-like comments etc.
When a newline character is found the scripts line counter is increased.
================
*/
int Lexer::ReadWhiteSpace( ) {
	while( 1 ) {
		// skip white space
		while( *Lexer::script_p <= ' ' ) {
			if( !* Lexer::script_p ) {
				return 0;
			}
			if( *Lexer::script_p == '\n' ) {
				Lexer::line++;
			}
			Lexer::script_p++;
		}
		// skip comments
		if( *Lexer::script_p == '/' ) {
			// comments //
			if( *( Lexer::script_p+1 ) == '/' ) {
				Lexer::script_p++;
				do {
					Lexer::script_p++;
					if( !* Lexer::script_p ) {
						return 0;
					}
				}
				while( *Lexer::script_p != '\n' );
				Lexer::line++;
				Lexer::script_p++;
				if( !* Lexer::script_p ) {
					return 0;
				}
				continue;
			}
			// comments /* */
			else if( *( Lexer::script_p+1 ) == '*' ) {
				Lexer::script_p++;
				while( 1 ) {
					Lexer::script_p++;
					if( !* Lexer::script_p ) {
						return 0;
					}
					if( *Lexer::script_p == '\n' ) {
						Lexer::line++;
					}
					else if( *Lexer::script_p == '/' ) {
						if( *( Lexer::script_p-1 ) == '*' ) {
							break;
						}
						if( *( Lexer::script_p+1 ) == '*' ) {
							Lexer::Warning( "nested comment" );
						}
					}
				}
				Lexer::script_p++;
				if( !* Lexer::script_p ) {
					return 0;
				}
				Lexer::script_p++;
				if( !* Lexer::script_p ) {
					return 0;
				}
				continue;
			}
		}
		break;
	}
	return 1;
}

/*
================
Lexer::ReadEscapeCharacter
================
*/
int Lexer::ReadEscapeCharacter( char * ch ) {
	int c, val, i;

	// step over the leading '\\'
	Lexer::script_p++;
	// determine the escape character
	switch( *Lexer::script_p ) {
		case '\\': c = '\\'; break;
		case 'n': c = '\n'; break;
		case 'r': c = '\r'; break;
		case 't': c = '\t'; break;
		case 'v': c = '\v'; break;
		case 'b': c = '\b'; break;
		case 'f': c = '\f'; break;
		case 'a': c = '\a'; break;
		case '\'': c = '\''; break;
		case '\"': c = '\"'; break;
		case '\?': c = '\?'; break;
		case 'x':
		{
			Lexer::script_p++;
			for( i = 0, val = 0;; i++, Lexer::script_p++ ) {
				c = * Lexer::script_p;
				if( c >= '0' && c <= '9' )
					c = c - '0';
				else if( c >= 'A' && c <= 'Z' )
					c = c - 'A' + 10;
				else if( c >= 'a' && c <= 'z' )
					c = c - 'a' + 10;
				else
					break;
				val =( val << 4 ) + c;
			}
			Lexer::script_p--;
			if( val > 0xFF ) {
				Lexer::Warning( "too large value in escape character" );
				val = 0xFF;
			}
			c = val;
			break;
		}
		default: //NOTE: decimal ASCII code, NOT octal
		{
			if( *Lexer::script_p < '0' || * Lexer::script_p > '9' ) {
				Lexer::Error( "unknown escape char" );
			}
			for( i = 0, val = 0;; i++, Lexer::script_p++ ) {
				c = * Lexer::script_p;
				if( c >= '0' && c <= '9' )
					c = c - '0';
				else
					break;
				val = val * 10 + c;
			}
			Lexer::script_p--;
			if( val > 0xFF ) {
				Lexer::Warning( "too large value in escape character" );
				val = 0xFF;
			}
			c = val;
			break;
		}
	}
	// step over the escape character or the last digit of the number
	Lexer::script_p++;
	// store the escape character
	* ch = c;
	// succesfully read escape character
	return 1;
}

/*
================
Lexer::ReadString

Escape characters are interpretted.
Reads two strings with only a white space between them as one string.
================
*/
int Lexer::ReadString( Token * token, int quote ) {
	int tmpline;
	const char * tmpscript_p;
	char ch;

	if( quote == '\"' ) {
		token->type = TT_STRING;
	} else {
		token->type = TT_LITERAL;
	}

	// leading quote
	Lexer::script_p++;

	while( 1 ) {
		// if there is an escape character and escape characters are allowed
		if( *Lexer::script_p == '\\' && !( Lexer::flags & LEXFL_NOSTRINGESCAPECHARS ) ) {
			if( !Lexer::ReadEscapeCharacter( &ch ) ) {
				return 0;
			}
			token->AppendDirty( ch );
		}
		// if a trailing quote
		else if( *Lexer::script_p == quote ) {
			// step over the quote
			Lexer::script_p++;
			// if consecutive strings should not be concatenated
			if( ( Lexer::flags & LEXFL_NOSTRINGCONCAT ) &&
					( !( Lexer::flags & LEXFL_ALLOWBACKSLASHSTRINGCONCAT ) ||( quote != '\"' ) ) ) {
				break;
			}

			tmpscript_p = Lexer::script_p;
			tmpline = Lexer::line;
			// read white space between possible two consecutive strings
			if( !Lexer::ReadWhiteSpace( ) ) {
				Lexer::script_p = tmpscript_p;
				Lexer::line = tmpline;
				break;
			}

			if( Lexer::flags & LEXFL_NOSTRINGCONCAT ) {
				if( *Lexer::script_p != '\\' ) {
					Lexer::script_p = tmpscript_p;
					Lexer::line = tmpline;
					break;
				}
				// step over the '\\'
				Lexer::script_p++;
				if( !Lexer::ReadWhiteSpace( ) ||( *Lexer::script_p != quote ) ) {
					Lexer::Error( "expecting string after '\' terminated line" );
					return 0;
				}
			}

			// if there's no leading qoute
			if( *Lexer::script_p != quote ) {
				Lexer::script_p = tmpscript_p;
				Lexer::line = tmpline;
				break;
			}
			// step over the new leading quote
			Lexer::script_p++;
		}
		else {
			if( *Lexer::script_p == '\0' ) {
				Lexer::Error( "missing trailing quote" );
				return 0;
			}
			if( *Lexer::script_p == '\n' ) {
				Lexer::Error( "newline inside string" );
				return 0;
			}
			token->AppendDirty( *Lexer::script_p++ );
		}
	}
	token->d_data[ token->d_len ] = '\0';

	if( token->type == TT_LITERAL ) {
		if( !( Lexer::flags & LEXFL_ALLOWMULTICHARLITERALS ) ) {
			if( token->Length( ) != 1 ) {
				Lexer::Warning( "literal is not one character long" );
			}
		}
		token->subtype =( *token )[ 0 ];
	}
	else {
		// the sub type is the length of the string
		token->subtype = token->Length( );
	}
	return 1;
}

/*
================
Lexer::ReadName
================
*/
int Lexer::ReadName( Token * token ) {
	char c;

	token->type = TT_NAME;
	do {
		token->AppendDirty( *Lexer::script_p++ );
		c = * Lexer::script_p;
	} while( ( c >= 'a' && c <= 'z' ) ||
				( c >= 'A' && c <= 'Z' ) ||
				( c >= '0' && c <= '9' ) ||
				c == '_' ||
				// if treating all tokens as strings, don't parse '-' as a seperate token
				( ( Lexer::flags & LEXFL_ONLYSTRINGS ) &&( c == '-' ) ) ||
				// if special path name characters are allowed
				( ( Lexer::flags & LEXFL_ALLOWPATHNAMES ) &&( c == '/' || c == '\\' || c == ':' || c == '.' ) ) );
	token->d_data[ token->d_len ] = '\0';
	//the sub type is the length of the name
	token->subtype = token->Length( );
	return 1;
}

/*
================
Lexer::CheckString
================
*/
INLINE int Lexer::CheckString( const char * str ) const {
	int i;

	for( i = 0; str[ i ]; i++ ) {
		if( Lexer::script_p[ i ] != str[ i ] ) {
			return false;
		}
	}
	return true;
}

/*
================
Lexer::ReadNumber
================
*/
int Lexer::ReadNumber( Token * token ) {
	int i;
	int dot;
	char c, c2;

	token->type = TT_NUMBER;
	token->subtype = 0;
	token->intvalue = 0;
	token->floatvalue = 0;

	c = * Lexer::script_p;
	c2 = * ( Lexer::script_p + 1 );

	if( c == '0' && c2 != '.' ) {
		// check for a hexadecimal number
		if( c2 == 'x' || c2 == 'X' ) {
			token->AppendDirty( *Lexer::script_p++ );
			token->AppendDirty( *Lexer::script_p++ );
			c = * Lexer::script_p;
			while( ( c >= '0' && c <= '9' ) ||
						( c >= 'a' && c <= 'f' ) ||
						( c >= 'A' && c <= 'F' ) ) {
				token->AppendDirty( c );
				c = * ( ++Lexer::script_p );
			}
			token->subtype = TT_HEX | TT_INTEGER;
		}
		// check for a binary number
		else if( c2 == 'b' || c2 == 'B' ) {
			token->AppendDirty( *Lexer::script_p++ );
			token->AppendDirty( *Lexer::script_p++ );
			c = * Lexer::script_p;
			while( c == '0' || c == '1' ) {
				token->AppendDirty( c );
				c = * ( ++Lexer::script_p );
			}
			token->subtype = TT_BINARY | TT_INTEGER;
		}
		// its an octal number
		else {
			token->AppendDirty( *Lexer::script_p++ );
			c = * Lexer::script_p;
			while( c >= '0' && c <= '7' ) {
				token->AppendDirty( c );
				c = * ( ++Lexer::script_p );
			}
			token->subtype = TT_OCTAL | TT_INTEGER;
		}
	}
	else {
		// decimal integer or floating point number or ip address
		dot = 0;
		while( 1 ) {
			if( c >= '0' && c <= '9' ) {
			}
			else if( c == '.' ) {
				dot++;
			}
			else {
				break;
			}
			token->AppendDirty( c );
			c = * ( ++Lexer::script_p );
		}
		if( c == 'e' && dot == 0 ) {
			//We have scientific notation without a decimal point
			dot++;
		}
		// if a floating point number
		if( dot == 1 ) {
			token->subtype = TT_DECIMAL | TT_FLOAT;
			// check for floating point exponent
			if( c == 'e' ) {
				//Append the e so that GetFloatValue code works
				token->AppendDirty( c );
				c = * ( ++Lexer::script_p );
				if( c == '-' ) {
					token->AppendDirty( c );
					c = * ( ++Lexer::script_p );
				}
				else if( c == '+' ) {
					token->AppendDirty( c );
					c = * ( ++Lexer::script_p );
				}
				while( c >= '0' && c <= '9' ) {
					token->AppendDirty( c );
					c = * ( ++Lexer::script_p );
				}
			}
			// check for floating point exception infinite 1.#INF or indefinite 1.#IND or NaN
			else if( c == '#' ) {
				c2 = 4;
				if( CheckString( "INF" ) ) {
					token->subtype |= TT_INFINITE;
				}
				else if( CheckString( "IND" ) ) {
					token->subtype |= TT_INDEFINITE;
				}
				else if( CheckString( "NAN" ) ) {
					token->subtype |= TT_NAN;
				}
				else if( CheckString( "QNAN" ) ) {
					token->subtype |= TT_NAN;
					c2++;
				}
				else if( CheckString( "SNAN" ) ) {
					token->subtype |= TT_NAN;
					c2++;
				}
				for( i = 0; i < c2; i++ ) {
					token->AppendDirty( c );
					c = * ( ++Lexer::script_p );
				}
				while( c >= '0' && c <= '9' ) {
					token->AppendDirty( c );
					c = * ( ++Lexer::script_p );
				}
				if( !( Lexer::flags & LEXFL_ALLOWFLOATEXCEPTIONS ) ) {
					token->AppendDirty( 0 );	// zero terminate for c_str
					Lexer::Error( "parsed %s", token->c_str( ) );
				}
			}
		}
		else if( dot > 1 ) {
			if( !( Lexer::flags & LEXFL_ALLOWIPADDRESSES ) ) {
				Lexer::Error( "more than one dot in number" );
				return 0;
			}
			if( dot != 3 ) {
				Lexer::Error( "ip address should have three dots" );
				return 0;
			}
			token->subtype = TT_IPADDRESS;
		}
		else {
			token->subtype = TT_DECIMAL | TT_INTEGER;
		}
	}

	if( token->subtype & TT_FLOAT ) {
		if( c > ' ' ) {
			// single-precision: float
			if( c == 'f' || c == 'F' ) {
				token->subtype |= TT_SINGLE_PRECISION;
				Lexer::script_p++;
			}
			// extended-precision: long double
			else if( c == 'l' || c == 'L' ) {
				token->subtype |= TT_EXTENDED_PRECISION;
				Lexer::script_p++;
			}
			// default is double-precision: double
			else {
				token->subtype |= TT_DOUBLE_PRECISION;
			}
		}
		else {
			token->subtype |= TT_DOUBLE_PRECISION;
		}
	}
	else if( token->subtype & TT_INTEGER ) {
		if( c > ' ' ) {
			// default: signed long
			for( i = 0; i < 2; i++ ) {
				// long integer
				if( c == 'l' || c == 'L' ) {
					token->subtype |= TT_LONG;
				}
				// unsigned integer
				else if( c == 'u' || c == 'U' ) {
					token->subtype |= TT_UNSIGNED;
				}
				else {
					break;
				}
				c = * ( ++Lexer::script_p );
			}
		}
	}
	else if( token->subtype & TT_IPADDRESS ) {
		if( c == ':' ) {
			token->AppendDirty( c );
			c = * ( ++Lexer::script_p );
			while( c >= '0' && c <= '9' ) {
				token->AppendDirty( c );
				c = * ( ++Lexer::script_p );
			}
			token->subtype |= TT_IPPORT;
		}
	}
	token->d_data[ token->d_len ] = '\0';
	return 1;
}

/*
================
Lexer::ReadPunctuation
================
*/
int Lexer::ReadPunctuation( Token * token ) {
	int l, n, i;
	const char * p;
	const punctuation_t * punc;

#ifdef PUNCTABLE
	for( n = Lexer::punctuationtable[( unsigned int )* ( Lexer::script_p )]; n >= 0; n = Lexer::nextpunctuation[ n ] )
	{
		punc = &( Lexer::punctuations[ n ] );
#else
	int i;

	for( i = 0; Lexer::punctuations[ i ].p; i++ ) {
		punc = &Lexer::punctuations[ i ];
#endif
		p = punc->p;
		// check for this punctuation in the script
		for( l = 0; p[ l ] && Lexer::script_p[ l ]; l++ ) {
			if( Lexer::script_p[ l ] != p[ l ] ) {
				break;
			}
		}
		if( !p[ l ] ) {
			//
			token->EnsureAlloced( l+1, false );
			for( i = 0; i <= l; i++ ) {
				token->d_data[ i ] = p[ i ];
			}
			token->d_len = l;
			//
			Lexer::script_p += l;
			token->type = TT_PUNCTUATION;
			// sub type is the punctuation id
			token->subtype = punc->n;
			return 1;
		}
	}
	return 0;
}

/*
================
Lexer::ReadToken
================
*/
int Lexer::ReadToken( Token * token ) {
	int c;

	if( !loaded ) {
		Common::Com_Error( ERR_DISCONNECT, "Lexer::ReadToken: no file loaded" );
		return 0;
	}

	// if there is a token available( from unreadToken )
	if( tokenavailable ) {
		tokenavailable = 0;
		* token = Lexer::token;
		return 1;
	}
	// save script pointer
	lastScript_p = script_p;
	// save line counter
	lastline = line;
	// clear the token stuff
	token->d_data[ 0 ] = '\0';
	token->d_len = 0;
	// start of the white space
	whiteSpaceStart_p = script_p;
	token->whiteSpaceStart_p = script_p;
	// read white space before token
	if( !ReadWhiteSpace( ) ) {
		return 0;
	}
	// end of the white space
	Lexer::whiteSpaceEnd_p = script_p;
	token->whiteSpaceEnd_p = script_p;
	// line the token is on
	token->line = line;
	// number of lines crossed before token
	token->linesCrossed = line - lastline;
	// clear token flags
	token->flags = 0;

	c = * Lexer::script_p;

	// if we're keeping everything as whitespace deliminated strings
	if( Lexer::flags & LEXFL_ONLYSTRINGS ) {
		// if there is a leading quote
		if( c == '\"' || c == '\'' ) {
			if( !Lexer::ReadString( token, c ) ) {
				return 0;
			}
		} else if( !Lexer::ReadName( token ) ) {
			return 0;
		}
	}
	// if there is a number
	else if( ( c >= '0' && c <= '9' ) ||
			( c == '.' &&( *( Lexer::script_p + 1 ) >= '0' && * ( Lexer::script_p + 1 ) <= '9' ) ) ) {
		if( !Lexer::ReadNumber( token ) ) {
			return 0;
		}
		// if names are allowed to start with a number
		if( Lexer::flags & LEXFL_ALLOWNUMBERNAMES ) {
			c = * Lexer::script_p;
			if( ( c >= 'a' && c <= 'z' ) ||	( c >= 'A' && c <= 'Z' ) || c == '_' ) {
				if( !Lexer::ReadName( token ) ) {
					return 0;
				}
			}
		}
	}
	// if there is a leading quote
	else if( c == '\"' || c == '\'' ) {
		if( !Lexer::ReadString( token, c ) ) {
			return 0;
		}
	}
	// if there is a name
	else if( ( c >= 'a' && c <= 'z' ) ||	( c >= 'A' && c <= 'Z' ) || c == '_' ) {
		if( !Lexer::ReadName( token ) ) {
			return 0;
		}
	}
	// names may also start with a slash when pathnames are allowed
	else if( ( Lexer::flags & LEXFL_ALLOWPATHNAMES ) &&( ( c == '/' || c == '\\' ) || c == '.' ) ) {
		if( !Lexer::ReadName( token ) ) {
			return 0;
		}
	}
	// check for punctuations
	else if( !Lexer::ReadPunctuation( token ) ) {
		Lexer::Error( "unknown punctuation %c", c );
		return 0;
	}
	// succesfully read a token
	return 1;
}

/*
================
Lexer::ExpectTokenString
================
*/
int Lexer::ExpectTokenString( const char * string ) {
	Token token;

	if( !Lexer::ReadToken( &token ) ) {
		Lexer::Error( "couldn't find expected '%s'", string );
		return 0;
	}
	if( token != string ) {
		Lexer::Error( "expected '%s' but found '%s'", string, token.c_str( ) );
		return 0;
	}
	return 1;
}

int Lexer::ExpectTokenChar( const char c ) {
	Token token;

	if( !Lexer::ReadToken( &token ) ) {
		Lexer::Error( "couldn't find expected '%c'", c );
		return 0;
	}
	if( token != c ) {
		Lexer::Error( "expected '%c' but found '%s'", c, token.c_str( ) );
		return 0;
	}
	return 1;
}

/*
================
Lexer::ExpectTokenType
================
*/
int Lexer::ExpectTokenType( int type, int subtype, Token * token ) {
	Str str;

	if( !Lexer::ReadToken( token ) ) {
		Lexer::Error( "couldn't read expected token" );
		return 0;
	}

	if( token->type != type ) {
		switch( type ) {
			case TT_STRING: str = "string"; break;
			case TT_LITERAL: str = "literal"; break;
			case TT_NUMBER: str = "number"; break;
			case TT_NAME: str = "name"; break;
			case TT_PUNCTUATION: str = "punctuation"; break;
			default: str = "unknown type"; break;
		}
		Lexer::Error( "expected a %s but found '%s'", str.c_str( ), token->c_str( ) );
		return 0;
	}
	if( token->type == TT_NUMBER ) {
		if( ( token->subtype & subtype ) != subtype ) {
			str.Clear( );
			if( subtype & TT_DECIMAL ) str = "decimal ";
			if( subtype & TT_HEX ) str = "hex ";
			if( subtype & TT_OCTAL ) str = "octal ";
			if( subtype & TT_BINARY ) str = "binary ";
			if( subtype & TT_UNSIGNED ) str += "unsigned ";
			if( subtype & TT_LONG ) str += "long ";
			if( subtype & TT_FLOAT ) str += "float ";
			if( subtype & TT_INTEGER ) str += "integer ";
			str.StripTrailing( ' ' );
			Lexer::Error( "expected %s but found '%s'", str.c_str( ), token->c_str( ) );
			return 0;
		}
	}
	else if( token->type == TT_PUNCTUATION ) {
		if( subtype < 0 ) {
			Lexer::Error( "BUG: wrong punctuation subtype" );
			return 0;
		}
		if( token->subtype != subtype ) {
			Lexer::Error( "expected '%s' but found '%s'", GetPunctuationFromId( subtype ), token->c_str( ) );
			return 0;
		}
	}
	return 1;
}

/*
================
Lexer::ExpectAnyToken
================
*/
int Lexer::ExpectAnyToken( Token * token ) {
	if( !Lexer::ReadToken( token ) ) {
		Lexer::Error( "couldn't read expected token" );
		return 0;
	}
	else {
		return 1;
	}
}

/*
================
Lexer::CheckTokenString
================
*/
int Lexer::CheckTokenString( const char * string ) {
	Token tok;

	if( !ReadToken( &tok ) ) {
		return 0;
	}
	// if the given string is available
	if( tok == string ) {
		return 1;
	}
	// unread token
	script_p = lastScript_p;
	line = lastline;
	return 0;
}

/*
================
Lexer::CheckTokenType
================
*/
int Lexer::CheckTokenType( int type, int subtype, Token * token ) {
	Token tok;

	if( !ReadToken( &tok ) ) {
		return 0;
	}
	// if the type matches
	if( tok.type == type &&( tok.subtype & subtype ) == subtype ) {
		* token = tok;
		return 1;
	}
	// unread token
	script_p = lastScript_p;
	line = lastline;
	return 0;
}

/*
================
Lexer::PeekTokenString
================
*/
int Lexer::PeekTokenString( const char * string ) {
	Token tok;

	if( !ReadToken( &tok ) ) {
		return 0;
	}

	// unread token
	script_p = lastScript_p;
	line = lastline;

	// if the given string is available
	if( tok == string ) {
		return 1;
	}
	return 0;
}

/*
================
Lexer::PeekTokenType
================
*/
int Lexer::PeekTokenType( int type, int subtype, Token * token ) {
	Token tok;

	if( !ReadToken( &tok ) ) {
		return 0;
	}

	// unread token
	script_p = lastScript_p;
	line = lastline;

	// if the type matches
	if( tok.type == type &&( tok.subtype & subtype ) == subtype ) {
		* token = tok;
		return 1;
	}
	return 0;
}

/*
================
Lexer::SkipUntilString
================
*/
int Lexer::SkipUntilString( const char * string ) {
	Token token;

	while( Lexer::ReadToken( &token ) ) {
		if( token == string ) {
			return 1;
		}
	}
	return 0;
}

/*
================
Lexer::SkipRestOfLine
================
*/
int Lexer::SkipRestOfLine( ) {
	/*Token token;

	while( Lexer::ReadToken( &token ) ) {
		if( token.linesCrossed ) {
			Lexer::script_p = lastScript_p;
			Lexer::line = lastline;
			return 1;
		}
	}
	return 0;*/
	while( 1 ) {
		if( *Lexer::script_p == '\n' ) {
			Lexer::line++;
			Lexer::script_p++;
			break;
		}
		if( !*Lexer::script_p )
			return 0;
		Lexer::script_p++;
	}
	return 1;
}

/*
=================
Lexer::SkipBracedSection

Skips until a matching close brace is found.
Internal brace depths are properly skipped.
=================
*/
int Lexer::SkipBracedSection( bool parseFirstBrace ) {
	Token token;
	int depth;

	depth = parseFirstBrace ? 0 : 1;
	do {
		if( !ReadToken( &token ) ) {
			return false;
		}
		if( token.type == TT_PUNCTUATION ) {
			if( token == '{' ) {
				depth++;
			} else if( token == '}' ) {
				depth--;
			}
		}
	} while( depth );
	return true;
}

/*
================
Lexer::UnreadToken
================
*/
void Lexer::UnreadToken( const Token * token ) {
	if( Lexer::tokenavailable ) {
		Common::Com_Error( ERR_DISCONNECT, "Lexer::unreadToken, unread token twice\n" );
	}
	Lexer::token = * token;
	Lexer::tokenavailable = 1;
}

/*
================
Lexer::ReadTokenOnLine
================
*/
int Lexer::ReadTokenOnLine( Token * token ) {
	Token tok;

	if( !Lexer::ReadToken( &tok ) ) {
		Lexer::script_p = lastScript_p;
		Lexer::line = lastline;
		return false;
	}
	// if no lines were crossed before this token
	if( !tok.linesCrossed ) {
		* token = tok;
		return true;
	}
	// restore our position
	Lexer::script_p = lastScript_p;
	Lexer::line = lastline;
	token->Clear( );
	return false;
}

/*
================
Lexer::ReadRestOfLine
================
*/
const char* 	Lexer::ReadRestOfLine( Str& out ) {
	while( 1 ) {
		if( *Lexer::script_p == '\n' ) {
			Lexer::line++;
			Lexer::script_p++;
			break;
		}
		if( !* Lexer::script_p ) {
			break;
		}
		if( *Lexer::script_p <= ' ' ) {
			out += " ";
		} else {
			out += * Lexer::script_p;
		}
		Lexer::script_p++;
	}
	out.Strip( ' ' );
	return out.c_str( );
}

/*
================
Lexer::ParseInt
================
*/
int Lexer::ParseInt( ) {
	Token token;

	if( !Lexer::ReadToken( &token ) ) {
		Lexer::Error( "couldn't read expected integer" );
		return 0;
	}
	if( token.type == TT_PUNCTUATION && token == '-' ) {
		Lexer::ExpectTokenType( TT_NUMBER, TT_INTEGER, &token );
		return -( ( signed int ) token.GetIntValue( ) );
	}
	else if( token.type != TT_NUMBER || token.subtype == TT_FLOAT ) {
		Lexer::Error( "expected integer value, found '%s'", token.c_str( ) );
	}
	return token.GetIntValue( );
}

/*
================
Lexer::ParseBool
================
*/
bool Lexer::ParseBool( ) {
	Token token;

	if( !Lexer::ExpectTokenType( TT_NUMBER, 0, &token ) ) {
		Lexer::Error( "couldn't read expected boolean" );
		return false;
	}
	return( token.GetIntValue( ) != 0 );
}

/*
================
Lexer::ParseFloat
================
*/
float Lexer::ParseFloat( bool * errorFlag ) {
	Token token;

	if( errorFlag ) {
		* errorFlag = false;
	}

	if( !Lexer::ReadToken( &token ) ) {
		if( errorFlag ) {
			Lexer::Warning( "couldn't read expected floating point number" );
			* errorFlag = true;
		} else {
			Lexer::Error( "couldn't read expected floating point number" );
		}
		return 0;
	}
	if( token.type == TT_PUNCTUATION && token == '-' ) {
		Lexer::ExpectTokenType( TT_NUMBER, 0, &token );
		return -token.GetFloatValue( );
	}
	else if( token.type != TT_NUMBER ) {
		if( errorFlag ) {
			Lexer::Warning( "expected float value, found '%s'", token.c_str( ) );
			* errorFlag = true;
		} else {
			Lexer::Error( "expected float value, found '%s'", token.c_str( ) );
		}
	}
	return token.GetFloatValue( );
}

/*
================
Lexer::ParseFloatArray
================
*/
int Lexer::ParseFloatArray( int x, float * m ) {
	int i;

	for( i = 0; i < x; i++ ) {
		m[ i ] = Lexer::ParseFloat( );
	}

	return true;
}

/*
================
Lexer::Parse1DMatrix
================
*/
int Lexer::Parse1DMatrix( int x, float * m ) {
	int i;

	if( !Lexer::ExpectTokenChar( '(' ) ) {
		return false;
	}

	for( i = 0; i < x; i++ ) {
		m[ i ] = Lexer::ParseFloat( );
	}

	if( !Lexer::ExpectTokenChar( ')' ) ) {
		return false;
	}
	return true;
}

/*
================
Lexer::Parse2DMatrix
================
*/
int Lexer::Parse2DMatrix( int y, int x, float * m ) {
	int i;

	if( !Lexer::ExpectTokenChar( '(' ) ) {
		return false;
	}

	for( i = 0; i < y; i++ ) {
		if( !Lexer::Parse1DMatrix( x, m + i * x ) ) {
			return false;
		}
	}

	if( !Lexer::ExpectTokenChar( ')' ) ) {
		return false;
	}
	return true;
}

/*
================
Lexer::Parse3DMatrix
================
*/
int Lexer::Parse3DMatrix( int z, int y, int x, float * m ) {
	int i;

	if( !Lexer::ExpectTokenChar( '(' ) ) {
		return false;
	}

	for( i = 0; i < z; i++ ) {
		if( !Lexer::Parse2DMatrix( y, x, m + i * x* y ) ) {
			return false;
		}
	}

	if( !Lexer::ExpectTokenChar( ')' ) ) {
		return false;
	}
	return true;
}

/*
=================
Parser::ParseBracedSection

The next token should be an open brace.
Parses until a matching close brace is found.
Maintains exact characters between braces.

  FIXME: this should use ReadToken and replace the token white space with correct indents and newlines
=================
*/
const char * Lexer::ParseBracedSectionExact( Str &out, int tabs ) {
	int		depth;
	bool	doTabs;
	bool	skipWhite;

	out.Empty( );

	if( !Lexer::ExpectTokenChar( '{' ) ) {
		return out.c_str( );
	}

	out = "{";
	depth = 1;	
	skipWhite = false;
	doTabs = tabs >= 0;

	while( depth && * Lexer::script_p ) {
		char c = * ( Lexer::script_p++ );

		switch( c ) {
			case '\t':
			case ' ': {
				if( skipWhite ) {
					continue;
				}
				break;
			}
			case '\n': {
				if( doTabs ) {
					skipWhite = true;
					out += c;
					continue;
				}
				break;
			}
			case '{': {
				depth++;
				tabs++;
				break;
			}
			case '}': {
				depth--;
				tabs--;
				break;				
			}
		}

		if( skipWhite ) {
			int i = tabs;
			if( c == '{' ) {
				i--;
			}
			skipWhite = false;
			for( ; i > 0; i-- ) {
				out += '\t';
			}
		}
		out += c;
	}
	return out.c_str( );
}

/*
=================
Lexer::ParseBracedSection

The next token should be an open brace.
Parses until a matching close brace is found.
Internal brace depths are properly skipped.
=================
*/
const char * Lexer::ParseBracedSection( Str &out ) {
	Token token;
	int i, depth;

	out.Empty( );
	if( !Lexer::ExpectTokenChar( '{' ) ) {
		return out.c_str( );
	}
	out = "{";
	depth = 1;
	do {
		if( !Lexer::ReadToken( &token ) ) {
			Error( "missing closing brace" );
			return out.c_str( );
		}

		// if the token is on a new line
		for( i = 0; i < token.linesCrossed; i++ ) {
			out += "\r\n";
		}

		if( token.type == TT_PUNCTUATION ) {
			if( token[ 0 ] == '{' ) {
				depth++;
			}
			else if( token[ 0 ] == '}' ) {
				depth--;
			}
		}

		if( token.type == TT_STRING ) {
			out += "\"" + token + "\"";
		}
		else {
			out += token;
		}
		out += " ";
	} while( depth );

	return out.c_str( );
}

/*
=================
Lexer::ParseRestOfLine

  parse the rest of the line
=================
*/
const char * Lexer::ParseRestOfLine( Str &out ) {
	Token token;

	out.Empty( );
	while( Lexer::ReadToken( &token ) ) {
		if( token.linesCrossed ) {
			Lexer::script_p = lastScript_p;
			Lexer::line = lastline;
			break;
		}
		if( out.Length( ) ) {
			out += " ";
		}
		out += token;
	}
	return out.c_str( );
}

/*
================
Lexer::GetLastWhiteSpace
================
*/
int Lexer::GetLastWhiteSpace( Str &whiteSpace ) const {
	whiteSpace.Clear( );
	for( const char * p = whiteSpaceStart_p; p < whiteSpaceEnd_p; p++ ) {
		whiteSpace.Append( *p );
	}
	return whiteSpace.Length( );
}

/*
================
Lexer::Reset
================
*/
void Lexer::Reset( ) {
	// pointer in script buffer
	Lexer::script_p = Lexer::buffer;
	// pointer in script buffer before reading token
	Lexer::lastScript_p = Lexer::buffer;
	// begin of white space
	Lexer::whiteSpaceStart_p = NULL;
	// end of white space
	Lexer::whiteSpaceEnd_p = NULL;
	// set if there's a token available in Lexer::token
	Lexer::tokenavailable = 0;

	Lexer::line = 1;
	Lexer::lastline = 1;
	// clear the saved token
	Lexer::token = "";
}

/*
================
Lexer::LoadFile
================
*/
int Lexer::LoadFile( const char * filename ) {
	FileBase * fp;
	Str pathname;
	int length;
	char * buf;

	if( Lexer::loaded ) {

		Common::Com_Error( ERR_DISCONNECT, "Lexer::LoadFile: another script already loaded" );
		return false;
	}
	
	fp = FileSystem::OpenFileRead( filename );

	if( !fp ) return false;

	length = ( int )fp->Length( );
	buf =( char * ) Mem_Alloc( length + 1 );
	buf[ length ] = '\0';
	fp->Read( buf, length );
	Lexer::fileTime = fp->Timestamp( );
	Lexer::filename = fp->GetFullPath( );
	FileSystem::CloseFile( fp );

	Lexer::buffer = buf;
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
	Lexer::allocated = true;
	Lexer::loaded = true;

	return true;
}

/*
================
Lexer::LoadMemory
================
*/
int Lexer::LoadMemory( const char * ptr, int length, const char * name, int startLine ) {
	if( Lexer::loaded ) {
		Common::Com_Error( ERR_DISCONNECT, "Lexer::LoadMemory: another script already loaded" );
		return false;
	}
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
	Lexer::line = startLine;
	Lexer::lastline = startLine;
	Lexer::allocated = false;
	Lexer::loaded = true;

	return true;
}

/*
================
Lexer::FreeSource
================
*/
void Lexer::FreeSource( ) {
#ifdef PUNCTABLE
	if( Lexer::punctuationtable && Lexer::punctuationtable != default_punctuationtable ) {
		Mem_Free( ( void * ) Lexer::punctuationtable );
		Lexer::punctuationtable = NULL;
	}
	if( Lexer::nextpunctuation && Lexer::nextpunctuation != default_nextpunctuation ) {
		Mem_Free( ( void * ) Lexer::nextpunctuation );
		Lexer::nextpunctuation = NULL;
	}
#endif //PUNCTABLE
	if( Lexer::allocated ) {
		Mem_Free( ( void * ) Lexer::buffer );
		Lexer::buffer = NULL;
		Lexer::allocated = false;
	}
	Lexer::tokenavailable = 0;
	Lexer::token = "";
	Lexer::loaded = false;
}
