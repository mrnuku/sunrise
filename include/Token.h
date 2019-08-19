// Copyright ( C) 2004 Id Software, Inc.
//

#ifndef __TOKEN_H__
#define __TOKEN_H__

/*
===============================================================================

	Token is a token read from a file or memory with Lexer or Parser

===============================================================================
*/

// token types
enum {

	TT_STRING		= 1,		// string
	TT_LITERAL,					// literal
	TT_NUMBER,							// number
	TT_NAME,				// name
	TT_PUNCTUATION				// punctuation

};

// number sub types
enum {

	TT_INTEGER					= BIT(  0 ),		// integer
	TT_DECIMAL					= BIT(  1 ),		// decimal number
	TT_HEX						= BIT(  2 ),		// hexadecimal number
	TT_OCTAL					= BIT(  3 ),		// octal number
	TT_BINARY					= BIT(  4 ),		// binary number
	TT_LONG						= BIT(  5 ),		// long int
	TT_UNSIGNED					= BIT(  6 ),		// unsigned int
	TT_FLOAT					= BIT(  7 ),		// floating point number
	TT_SINGLE_PRECISION			= BIT(  8 ),		// float
	TT_DOUBLE_PRECISION			= BIT(  9 ),		// double
	TT_EXTENDED_PRECISION		= BIT( 10 ),		// long double
	TT_INFINITE					= BIT( 11 ),		// infinite 1.#INF
	TT_INDEFINITE				= BIT( 12 ),		// indefinite 1.#IND
	TT_NAN						= BIT( 13 ),		// NaN
	TT_IPADDRESS				= BIT( 14 ),		// ip address
	TT_IPPORT					= BIT( 15 ),		// ip port
	TT_VALUESVALID				= BIT( 16 )			// set if intvalue and floatvalue are valid

};

// string sub type is the length of the string
// literal sub type is the ASCII code
// punctuation sub type is the punctuation id
// name sub type is the length of the name

class Token : public Str {

	friend class Parser;
	friend class Lexer;

public:
	int				type;								// token type
	int				subtype;							// token sub type
	int				line;								// line in script the token was on
	int				linesCrossed;						// number of lines crossed in white space before token
	int				flags;								// token flags, used for recursive defines

public:
					Token( );
					Token( const Token *token );
					~Token( );

	void			operator =( const Str& text );
	void			operator =( const char *text );

	void			SetType( int type_to )				{ type = type_to; }
	void			SetType( int type_to, int sub_to )	{ type = type_to; subtype = sub_to; }
	int				StrIntValue( const Str & s )		{ ( *this ) = s; SetType( TT_NUMBER, TT_DECIMAL ); return GetIntValue( ); }
	float			StrFloatValue( const Str & s )		{ ( *this ) = s; SetType( TT_NUMBER, TT_DECIMAL | TT_FLOAT ); return GetFloatValue( ); }

	int				GetIntValueAfter( const char * strip )	{ bool f = StripLeadingOnce( strip ); assert( f ); SetType( TT_NUMBER, TT_DECIMAL ); return GetIntValue( ); }

	double			GetDoubleValue( );				// double value of TT_NUMBER
	float			GetFloatValue( );				// float value of TT_NUMBER
	unsigned long	GetUnsignedLongValue( );		// unsigned long value of TT_NUMBER
	int				GetIntValue( );				// int value of TT_NUMBER
	int				WhiteSpaceBeforeToken( ) const;// returns length of whitespace before token
	void			ClearTokenWhiteSpace( );		// forget whitespace before token

	void			NumberValue( );				// calculate values for a TT_NUMBER

private:
	unsigned long	intvalue;							// integer value
	double			floatvalue;							// floating point value
	const char *	whiteSpaceStart_p;					// start of white space before token, only used by Lexer
	const char *	whiteSpaceEnd_p;					// end of white space before token, only used by Lexer
	Token *		next;								// next token in chain, only used by Parser

	void			AppendDirty( const char a );		// append character without adding trailing zero
};

INLINE Token::Token( ) {
}

INLINE Token::Token( const Token *token ) {
	*this = *token;
}

INLINE Token::~Token( ) {
}

INLINE void Token::operator =( const char *text) {
	*static_cast<Str *>( this) = text;
}

INLINE void Token::operator =( const Str& text ) {
	*static_cast<Str *>( this) = text;
}

INLINE double Token::GetDoubleValue( ) {
	if ( type != TT_NUMBER ) {
		return 0.0;
	}
	if ( !( subtype & TT_VALUESVALID) ) {
		NumberValue( );
	}
	return floatvalue;
}

INLINE float Token::GetFloatValue( ) {
	return ( float) GetDoubleValue( );
}

INLINE unsigned long	Token::GetUnsignedLongValue( ) {
	if ( type != TT_NUMBER ) {
		return 0;
	}
	if ( !( subtype & TT_VALUESVALID) ) {
		NumberValue( );
	}
	return intvalue;
}

INLINE int Token::GetIntValue( ) {
	return ( int) GetUnsignedLongValue( );
}

INLINE int Token::WhiteSpaceBeforeToken( ) const {
	return ( whiteSpaceEnd_p > whiteSpaceStart_p );
}

INLINE void Token::AppendDirty( const char a ) {
	EnsureAlloced( d_len + 2, true );
	d_data[ d_len++ ] = a;
}

#endif /* !__TOKEN_H__ */
