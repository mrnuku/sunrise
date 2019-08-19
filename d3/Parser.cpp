// Copyright( C ) 2004 Id Software, Inc.
//

#include "precompiled.h"
#pragma hdrstop

//#define DEBUG_EVAL
#define MAX_DEFINEPARMS				128
#define DEFINEHASHSIZE				2048

#define TOKEN_FL_RECURSIVE_DEFINE	1

define_t * Parser::globaldefines;

/*
================
Parser::AddGlobalDefine
================
*/
int Parser::AddGlobalDefine( const char * string ) {
	define_t * define;

	define = Parser::DefineFromString( string );
	if( !define ) {
		return false;
	}
	define->next = globaldefines;
	globaldefines = define;
	return true;
}

/*
================
Parser::RemoveGlobalDefine
================
*/
int Parser::RemoveGlobalDefine( const char * name ) {
	define_t * d, * prev;

	for( prev = NULL, d = Parser::globaldefines; d; prev = d, d = d->next ) {
		if( !strcmp( d->name, name ) ) {
			break;
		}
	}
	if( d ) {
		if( prev ) {
			prev->next = d->next;
		}
		else {
			Parser::globaldefines = d->next;
		}
		Parser::FreeDefine( d );
		return true;
	}
	return false;
}

/*
================
Parser::RemoveAllGlobalDefines
================
*/
void Parser::RemoveAllGlobalDefines( ) {
	define_t * define;

	for( define = globaldefines; define; define = globaldefines ) {
		globaldefines = globaldefines->next;
		Parser::FreeDefine( define );
	}
}


/*
===============================================================================

Parser

===============================================================================
*/

/*
================
Parser::PrintDefine
================
*/
void Parser::PrintDefine( define_t * define ) {
	Common::Com_DPrintf( "define->name = %s\n", define->name );
	Common::Com_DPrintf( "define->flags = %d\n", define->flags );
	Common::Com_DPrintf( "define->builtin = %d\n", define->builtin );
	Common::Com_DPrintf( "define->numparms = %d\n", define->numparms );
}

/*
================
PC_PrintDefineHashTable
================
* /
static void PC_PrintDefineHashTable( define_t * *definehash ) {
	int i;
	define_t * d;

	for( i = 0; i < DEFINEHASHSIZE; i++ ) {
		Log_Write( "%4d:", i );
		for( d = definehash[ i ]; d; d = d->hashnext ) {
			Log_Write( " %s", d->name );
		}
		Log_Write( "\n" );
	}
}
*/

/*
================
PC_NameHash
================
*/
INLINE int PC_NameHash( const char * name ) {
	int hash, i;

	hash = 0;
	for( i = 0; name[ i ] != '\0'; i++ ) {
		hash += name[ i ] * ( 119 + i );
	}
	hash =( hash ^( hash >> 10 ) ^( hash >> 20 ) ) &( DEFINEHASHSIZE-1 );
	return hash;
}

/*
================
Parser::AddDefineToHash
================
*/
void Parser::AddDefineToHash( define_t * define, define_t * *definehash ) {
	int hash;

	hash = PC_NameHash( define->name );
	define->hashnext = definehash[ hash ];
	definehash[ hash ] = define;
}

/*
================
FindHashedDefine
================
*/
define_t * Parser::FindHashedDefine( define_t * *definehash, const char * name ) {
	define_t * d;
	int hash;

	hash = PC_NameHash( name );
	for( d = definehash[ hash ]; d; d = d->hashnext ) {
		if( !strcmp( d->name, name ) ) {
			return d;
		}
	}
	return NULL;
}

/*
================
Parser::FindDefine
================
*/
define_t * Parser::FindDefine( define_t * defines, const char * name ) {
	define_t * d;

	for( d = defines; d; d = d->next ) {
		if( !strcmp( d->name, name ) ) {
			return d;
		}
	}
	return NULL;
}

/*
================
Parser::FindDefineParm
================
*/
int Parser::FindDefineParm( define_t * define, const char * name ) {
	Token * p;
	int i;

	i = 0;
	for( p = define->parms; p; p = p->next ) {
		if( ( *p ) == name ) {
			return i;
		}
		i++;
	}
	return -1;
}

/*
================
Parser::CopyDefine
================
*/
define_t * Parser::CopyDefine( define_t * define ) {
	define_t * newdefine;
	Token * token, * newtoken, * lasttoken;

	newdefine =( define_t * ) Mem_Alloc( sizeof( define_t ) + strlen( define->name ) + 1 );
	//copy the define name
	newdefine->name =( char * ) newdefine + sizeof( define_t );
	strcpy( newdefine->name, define->name );
	newdefine->flags = define->flags;
	newdefine->builtin = define->builtin;
	newdefine->numparms = define->numparms;
	//the define is not linked
	newdefine->next = NULL;
	newdefine->hashnext = NULL;
	//copy the define tokens
	newdefine->tokens = NULL;
	for( lasttoken = NULL, token = define->tokens; token; token = token->next ) {
		newtoken = new Token( token );
		newtoken->next = NULL;
		if( lasttoken ) lasttoken->next = newtoken;
		else newdefine->tokens = newtoken;
		lasttoken = newtoken;
	}
	//copy the define parameters
	newdefine->parms = NULL;
	for( lasttoken = NULL, token = define->parms; token; token = token->next ) {
		newtoken = new Token( token );
		newtoken->next = NULL;
		if( lasttoken ) lasttoken->next = newtoken;
		else newdefine->parms = newtoken;
		lasttoken = newtoken;
	}
	return newdefine;
}

/*
================
Parser::FreeDefine
================
*/
void Parser::FreeDefine( define_t * define ) {
	Token * t, * next;

	//free the define parameters
	for( t = define->parms; t; t = next ) {
		next = t->next;
		delete t;
	}
	//free the define tokens
	for( t = define->tokens; t; t = next ) {
		next = t->next;
		delete t;
	}
	//free the define
	Mem_Free( define );
}

/*
================
Parser::DefineFromString
================
*/
define_t * Parser::DefineFromString( const char * string ) {
	Parser src;
	define_t * def;

	if( !src.LoadMemory( string, ( int )strlen( string ), "*defineString" ) ) {
		return NULL;
	}
	// create a define from the source
	if( !src.Directive_define( ) ) {
		src.FreeSource( );
		return NULL;
	}
	def = src.CopyFirstDefine( );
	src.FreeSource( );
	//if the define was created succesfully
	return def;
}

/*
================
Parser::Error
================
*/
void Parser::Error( const char * str, ... ) const {
	char text[ MAX_STRING_CHARS ];
	va_list ap;

	va_start( ap, str );
	vsprintf( text, str, ap );
	va_end( ap );
	if( Parser::scriptstack ) {
		Parser::scriptstack->Error( text );
	}
}

/*
================
Parser::Warning
================
*/
void Parser::Warning( const char * str, ... ) const {
	char text[ MAX_STRING_CHARS ];
	va_list ap;

	va_start( ap, str );
	vsprintf( text, str, ap );
	va_end( ap );
	if( Parser::scriptstack ) {
		Parser::scriptstack->Warning( text );
	}
}

/*
================
Parser::PushIndent
================
*/
void Parser::PushIndent( int type, int skip ) {
	indent_t * indent;

	indent =( indent_t * ) Mem_Alloc( sizeof( indent_t ) );
	indent->type = type;
	indent->script = Parser::scriptstack;
	indent->skip =( skip != 0 );
	Parser::skip += indent->skip;
	indent->next = Parser::indentstack;
	Parser::indentstack = indent;
}

/*
================
Parser::PopIndent
================
*/
void Parser::PopIndent( int * type, int * skip ) {
	indent_t * indent;

	* type = 0;
	* skip = 0;

	indent = Parser::indentstack;
	if( !indent ) return;

	// must be an indent from the current script
	if( Parser::indentstack->script != Parser::scriptstack ) {
		return;
	}

	* type = indent->type;
	* skip = indent->skip;
	Parser::indentstack = Parser::indentstack->next;
	Parser::skip -= indent->skip;
	Mem_Free( indent );
}

/*
================
Parser::PushScript
================
*/
void Parser::PushScript( Lexer * script ) {
	Lexer * s;

	for( s = Parser::scriptstack; s; s = s->next ) {
		if( !Str::Icmp( s->GetFileName( ), script->GetFileName( ) ) ) {
			Parser::Warning( "'%s' recursively included", script->GetFileName( ) );
			return;
		}
	}
	//push the script on the script stack
	script->next = Parser::scriptstack;
	Parser::scriptstack = script;
}

/*
================
Parser::ReadSourceToken
================
*/
int Parser::ReadSourceToken( Token * token ) {
	Token * t;
	Lexer * script;
	int type, skip, changedScript;

	if( !Parser::scriptstack ) {
		Common::Com_Error( ERR_DISCONNECT, "Parser::ReadSourceToken: not loaded" );
		return false;
	}
	changedScript = 0;
	// if there's no token already available
	while( !Parser::tokens ) {
		// if there's a token to read from the script
		if( Parser::scriptstack->ReadToken( token ) ) {
			token->linesCrossed += changedScript;

			// set the marker based on the start of the token read in
			if( !marker_p ) {
				marker_p = token->whiteSpaceEnd_p;
			}
			return true;
		}
		// if at the end of the script
		if( Parser::scriptstack->EndOfFile( ) ) {
			// remove all indents of the script
			while( Parser::indentstack && Parser::indentstack->script == Parser::scriptstack ) {
				Parser::Warning( "missing #endif" );
				Parser::PopIndent( &type, &skip );
			}
			changedScript = 1;
		}
		// if this was the initial script
		if( !Parser::scriptstack->next ) {
			return false;
		}
		// remove the script and return to the previous one
		script = Parser::scriptstack;
		Parser::scriptstack = Parser::scriptstack->next;
		delete script;
	}
	// copy the already available token
	* token = Parser::tokens;
	// remove the token from the source
	t = Parser::tokens;
	Parser::tokens = Parser::tokens->next;
	delete t;
	return true;
}

/*
================
Parser::UnreadSourceToken
================
*/
int Parser::UnreadSourceToken( Token * token ) {
	Token * t;

	t = new Token( token );
	t->next = Parser::tokens;
	Parser::tokens = t;
	return true;
}

/*
================
Parser::ReadDefineParms
================
*/
int Parser::ReadDefineParms( define_t * define, Token * *parms, int maxparms ) {
	define_t * newdefine;
	Token token, * t, * last;
	int i, done, lastcomma, numparms, indent;

	if( !Parser::ReadSourceToken( &token ) ) {
		Parser::Error( "define '%s' missing parameters", define->name );
		return false;
	}

	if( define->numparms > maxparms ) {
		Parser::Error( "define with more than %d parameters", maxparms );
		return false;
	}

	for( i = 0; i < define->numparms; i++ ) {
		parms[ i ] = NULL;
	}
	// if no leading "( "
	if( token != "( " ) {
		Parser::UnreadSourceToken( &token );
		Parser::Error( "define '%s' missing parameters", define->name );
		return false;
	}
	// read the define parameters
	for( done = 0, numparms = 0, indent = 1; !done; ) {
		if( numparms >= maxparms ) {
			Parser::Error( "define '%s' with too many parameters", define->name );
			return false;
		}
		parms[ numparms ] = NULL;
		lastcomma = 1;
		last = NULL;
		while( !done ) {

			if( !Parser::ReadSourceToken( &token ) ) {
				Parser::Error( "define '%s' incomplete", define->name );
				return false;
			}

			if( token == ", " ) {
				if( indent <= 1 ) {
					if( lastcomma ) {
						Parser::Warning( "too many comma's" );
					}
					if( numparms >= define->numparms ) {
						Parser::Warning( "too many define parameters" );
					}
					lastcomma = 1;
					break;
				}
			}
			else if( token == "( " ) {
				indent++;
			}
			else if( token == " )" ) {
				indent--;
				if( indent <= 0 ) {
					if( !parms[ define->numparms-1 ] ) {
						Parser::Warning( "too few define parameters" );
					}
					done = 1;
					break;
				}
			}
			else if( token.type == TT_NAME ) {
				newdefine = FindHashedDefine( Parser::definehash, token.c_str( ) );
				if( newdefine ) {
					if( !Parser::ExpandDefineIntoSource( &token, newdefine ) ) {
						return false;
					}
					continue;
				}
			}

			lastcomma = 0;

			if( numparms < define->numparms ) {

				t = new Token( token );
				t->next = NULL;
				if( last ) last->next = t;
				else parms[ numparms ] = t;
				last = t;
			}
		}
		numparms++;
	}
	return true;
}

/*
================
Parser::StringizeTokens
================
*/
int Parser::StringizeTokens( Token * tokens, Token * token ) {
	Token * t;

	token->type = TT_STRING;
	token->whiteSpaceStart_p = NULL;
	token->whiteSpaceEnd_p = NULL;
	( *token ) = "";
	for( t = tokens; t; t = t->next ) {
		token->Append( t->c_str( ) );
	}
	return true;
}

/*
================
Parser::MergeTokens
================
*/
int Parser::MergeTokens( Token * t1, Token * t2 ) {
	// merging of a name with a name or number
	if( t1->type == TT_NAME &&( t2->type == TT_NAME ||( t2->type == TT_NUMBER && !( t2->subtype & TT_FLOAT ) ) ) ) {
		t1->Append( t2->c_str( ) );
		return true;
	}
	// merging of two strings
	if( t1->type == TT_STRING && t2->type == TT_STRING ) {
		t1->Append( t2->c_str( ) );
		return true;
	}
	// merging of two numbers
	if( t1->type == TT_NUMBER && t2->type == TT_NUMBER &&
			!( t1->subtype &( TT_HEX|TT_BINARY ) ) && !( t2->subtype &( TT_HEX|TT_BINARY ) ) &&
			( !( t1->subtype & TT_FLOAT ) || !( t2->subtype & TT_FLOAT ) ) ) {
		t1->Append( t2->c_str( ) );
		return true;
	}

	return false;
}

/*
================
Parser::AddBuiltinDefines
================
*/
void Parser::AddBuiltinDefines( ) {
	int i;
	define_t * define;
	struct builtin
	{
		const char * string;
		int id;
	} builtin[] = {
		{ "__LINE__", 	BUILTIN_LINE }, { "__FILE__", 	BUILTIN_FILE }, { "__DATE__", 	BUILTIN_DATE }, { "__TIME__", 	BUILTIN_TIME }, { "__STDC__", BUILTIN_STDC }, { NULL, 0 }
	};

	for( i = 0; builtin[ i ].string; i++ ) {
		define =( define_t * ) Mem_Alloc( sizeof( define_t ) + strlen( builtin[ i ].string ) + 1 );
		define->name =( char * ) define + sizeof( define_t );
		strcpy( define->name, builtin[ i ].string );
		define->flags = DEFINE_FIXED;
		define->builtin = builtin[ i ].id;
		define->numparms = 0;
		define->parms = NULL;
		define->tokens = NULL;
		// add the define to the source
		AddDefineToHash( define, Parser::definehash );
	}
}

/*
================
Parser::CopyFirstDefine
================
*/
define_t * Parser::CopyFirstDefine( ) {
	int i;

	for( i = 0; i < DEFINEHASHSIZE; i++ ) {
		if( Parser::definehash[ i ] ) {
			return CopyDefine( Parser::definehash[ i ] );
		}
	}
	return NULL;
}

/*
================
Parser::ExpandBuiltinDefine
================
*/
int Parser::ExpandBuiltinDefine( Token * deftoken, define_t * define, Token * *firsttoken, Token * *lasttoken ) {
	Token * token;
	time_t t;
	char * curtime;
	char buf[ MAX_STRING_CHARS ];

	token = new Token( deftoken );
	switch( define->builtin ) {
		case BUILTIN_LINE: {
			sprintf( buf, "%d", deftoken->line );
			( *token ) = buf;
			token->intvalue = deftoken->line;
			token->floatvalue = deftoken->line;
			token->type = TT_NUMBER;
			token->subtype = TT_DECIMAL | TT_INTEGER | TT_VALUESVALID;
			token->line = deftoken->line;
			token->linesCrossed = deftoken->linesCrossed;
			token->flags = 0;
			* firsttoken = token;
			* lasttoken = token;
			break;
		}
		case BUILTIN_FILE: {
			( *token ) = Parser::scriptstack->GetFileName( );
			token->type = TT_NAME;
			token->subtype = token->Length( );
			token->line = deftoken->line;
			token->linesCrossed = deftoken->linesCrossed;
			token->flags = 0;
			* firsttoken = token;
			* lasttoken = token;
			break;
		}
		case BUILTIN_DATE: {
			t = time( NULL );
			curtime = ctime( &t );
			( *token ) = "\"";
			token->Append( curtime+4 );
			token[ 7 ] = '\0';
			token->Append( curtime+20 );
			token[ 10 ] = '\0';
			token->Append( "\"" );
			free( curtime );
			token->type = TT_STRING;
			token->subtype = token->Length( );
			token->line = deftoken->line;
			token->linesCrossed = deftoken->linesCrossed;
			token->flags = 0;
			* firsttoken = token;
			* lasttoken = token;
			break;
		}
		case BUILTIN_TIME: {
			t = time( NULL );
			curtime = ctime( &t );
			( *token ) = "\"";
			token->Append( curtime+11 );
			token[ 8 ] = '\0';
			token->Append( "\"" );
			free( curtime );
			token->type = TT_STRING;
			token->subtype = token->Length( );
			token->line = deftoken->line;
			token->linesCrossed = deftoken->linesCrossed;
			token->flags = 0;
			* firsttoken = token;
			* lasttoken = token;
			break;
		}
		case BUILTIN_STDC: {
			Parser::Warning( "__STDC__ not supported\n" );
			* firsttoken = NULL;
			* lasttoken = NULL;
			break;
		}
		default: {
			* firsttoken = NULL;
			* lasttoken = NULL;
			break;
		}
	}
	return true;
}

/*
================
Parser::ExpandDefine
================
*/
int Parser::ExpandDefine( Token * deftoken, define_t * define, Token * *firsttoken, Token * *lasttoken ) {
	Token * parms[ MAX_DEFINEPARMS ], * dt, * pt, * t;
	Token * t1, * t2, * first, * last, * nextpt, token;
	int parmnum, i;

	// if it is a builtin define
	if( define->builtin ) {
		return Parser::ExpandBuiltinDefine( deftoken, define, firsttoken, lasttoken );
	}
	// if the define has parameters
	if( define->numparms ) {
		if( !Parser::ReadDefineParms( define, parms, MAX_DEFINEPARMS ) ) {
			return false;
		}
#ifdef DEBUG_EVAL
		for( i = 0; i < define->numparms; i++ ) {
			Log_Write( "define parms %d:", i );
			for( pt = parms[ i ]; pt; pt = pt->next ) {
				Log_Write( "%s", pt->c_str( ) );
			}
		}
#endif //DEBUG_EVAL
	}
	// empty list at first
	first = NULL;
	last = NULL;
	// create a list with tokens of the expanded define
	for( dt = define->tokens; dt; dt = dt->next ) {
		parmnum = -1;
		// if the token is a name, it could be a define parameter
		if( dt->type == TT_NAME ) {
			parmnum = FindDefineParm( define, dt->c_str( ) );
		}
		// if it is a define parameter
		if( parmnum >= 0 ) {
			for( pt = parms[ parmnum ]; pt; pt = pt->next ) {
				t = new Token( pt );
				//add the token to the list
				t->next = NULL;
				if( last ) last->next = t;
				else first = t;
				last = t;
			}
		}
		else {
			// if stringizing operator
			if( ( *dt ) == "#" ) {
				// the stringizing operator must be followed by a define parameter
				if( dt->next ) {
					parmnum = FindDefineParm( define, dt->next->c_str( ) );
				}
				else {
					parmnum = -1;
				}

				if( parmnum >= 0 ) {
					// step over the stringizing operator
					dt = dt->next;
					// stringize the define parameter tokens
					if( !Parser::StringizeTokens( parms[ parmnum ], &token ) ) {
						Parser::Error( "can't stringize tokens" );
						return false;
					}
					t = new Token( token );
					t->line = deftoken->line;
				}
				else {
					Parser::Warning( "stringizing operator without define parameter" );
					continue;
				}
			}
			else {
				t = new Token( dt );
				t->line = deftoken->line;
			}
			// add the token to the list
			t->next = NULL;
// the token being read from the define list should use the line number of
// the original file, not the header file			
			t->line = deftoken->line;

			if( last ) last->next = t;
			else first = t;
			last = t;
		}
	}
	// check for the merging operator
	for( t = first; t; ) {
		if( t->next ) {
			// if the merging operator
			if( ( *t->next ) == "##" ) {
				t1 = t;
				t2 = t->next->next;
				if( t2 ) {
					if( !Parser::MergeTokens( t1, t2 ) ) {
						Parser::Error( "can't merge '%s' with '%s'", t1->c_str( ), t2->c_str( ) );
						return false;
					}
					delete t1->next;
					t1->next = t2->next;
					if( t2 == last ) last = t1;
					delete t2;
					continue;
				}
			}
		}
		t = t->next;
	}
	// store the first and last token of the list
	* firsttoken = first;
	* lasttoken = last;
	// free all the parameter tokens
	for( i = 0; i < define->numparms; i++ ) {
		for( pt = parms[ i ]; pt; pt = nextpt ) {
			nextpt = pt->next;
			delete pt;
		}
	}

	return true;
}

/*
================
Parser::ExpandDefineIntoSource
================
*/
int Parser::ExpandDefineIntoSource( Token * deftoken, define_t * define ) {
	Token * firsttoken, * lasttoken;

	if( !Parser::ExpandDefine( deftoken, define, &firsttoken, &lasttoken ) ) {
		return false;
	}
	// if the define is not empty
	if( firsttoken && lasttoken ) {
		firsttoken->linesCrossed += deftoken->linesCrossed;
		lasttoken->next = Parser::tokens;
		Parser::tokens = firsttoken;
	}
	return true;
}

/*
================
Parser::ReadLine

reads a token from the current line, continues reading on the next
line only if a backslash '\' is found
================
*/
int Parser::ReadLine( Token * token ) {
	int crossline;

	crossline = 0;
	do {
		if( !Parser::ReadSourceToken( token ) ) {
			return false;
		}
		
		if( token->linesCrossed > crossline ) {
			Parser::UnreadSourceToken( token );
			return false;
		}
		crossline = 1;
	} while( ( *token ) == "\\" );
	return true;
}

/*
================
Parser::Directive_include
================
*/
int Parser::Directive_include( ) {
	Lexer * script;
	Token token;
	Str path;

	if( !Parser::ReadSourceToken( &token ) ) {
		Parser::Error( "#include without file name" );
		return false;
	}
	if( token.linesCrossed > 0 ) {
		Parser::Error( "#include without file name" );
		return false;
	}
	if( token.type == TT_STRING ) {
		script = new Lexer;
		// try relative to the current file
		path = scriptstack->GetFileName( );
		path.StripFilename( );
		path += "/";
		path += token;
		if( !script->LoadFile( path ) ) {
			// try absolute path
			path = token;
			if( !script->LoadFile( path ) ) {
				// try from the include path
				path = includepath + token;
				if( !script->LoadFile( path ) ) {
					delete script;
					script = NULL;
				}
			}
		}
	}
	else if( token.type == TT_PUNCTUATION && token == "<" ) {
		path = Parser::includepath;
		while( Parser::ReadSourceToken( &token ) ) {
			if( token.linesCrossed > 0 ) {
				Parser::UnreadSourceToken( &token );
				break;
			}
			if( token.type == TT_PUNCTUATION && token == ">" ) {
				break;
			}
			path += token;
		}
		if( token != ">" ) {
			Parser::Warning( "#include missing trailing >" );
		}
		if( !path.Length( ) ) {
			Parser::Error( "#include without file name between < >" );
			return false;
		}
		if( Parser::flags & LEXFL_NOBASEINCLUDES ) {
			return true;
		}
		script = new Lexer;
		if( !script->LoadFile( includepath + path ) ) {
			delete script;
			script = NULL;
		}
	}
	else {
		Parser::Error( "#include without file name" );
		return false;
	}
	if( !script ) {
		Parser::Error( "file '%s' not found", path.c_str( ) );
		return false;
	}
	script->SetFlags( Parser::flags );
	script->SetPunctuations( Parser::punctuations );
	Parser::PushScript( script );
	return true;
}

/*
================
Parser::Directive_undef
================
*/
int Parser::Directive_undef( ) {
	Token token;
	define_t * define, * lastdefine;
	int hash;

	//
	if( !Parser::ReadLine( &token ) ) {
		Parser::Error( "undef without name" );
		return false;
	}
	if( token.type != TT_NAME ) {
		Parser::UnreadSourceToken( &token );
		Parser::Error( "expected name but found '%s'", token.c_str( ) );
		return false;
	}

	hash = PC_NameHash( token.c_str( ) );
	for( lastdefine = NULL, define = Parser::definehash[ hash ]; define; define = define->hashnext ) {
		if( !strcmp( define->name, token.c_str( ) ) )
		{
			if( define->flags & DEFINE_FIXED ) {
				Parser::Warning( "can't undef '%s'", token.c_str( ) );
			}
			else {
				if( lastdefine ) {
					lastdefine->hashnext = define->hashnext;
				}
				else {
					Parser::definehash[ hash ] = define->hashnext;
				}
				FreeDefine( define );
			}
			break;
		}
		lastdefine = define;
	}
	return true;
}

/*
================
Parser::Directive_define
================
*/
int Parser::Directive_define( ) {
	Token token, * t, * last;
	define_t * define;

	if( !Parser::ReadLine( &token ) ) {
		Parser::Error( "#define without name" );
		return false;
	}
	if( token.type != TT_NAME ) {
		Parser::UnreadSourceToken( &token );
		Parser::Error( "expected name after #define, found '%s'", token.c_str( ) );
		return false;
	}
	// check if the define already exists
	define = FindHashedDefine( Parser::definehash, token.c_str( ) );
	if( define ) {
		if( define->flags & DEFINE_FIXED ) {
			Parser::Error( "can't redefine '%s'", token.c_str( ) );
			return false;
		}
		Parser::Warning( "redefinition of '%s'", token.c_str( ) );
		// unread the define name before executing the #undef directive
		Parser::UnreadSourceToken( &token );
		if( !Parser::Directive_undef( ) )
			return false;
		// if the define was not removed( define->flags & DEFINE_FIXED )
		define = FindHashedDefine( Parser::definehash, token.c_str( ) );
	}
	// allocate define
	define =( define_t * ) Mem_ClearedAlloc( sizeof( define_t ) + token.Length( ) + 1 );
	define->name =( char * ) define + sizeof( define_t );
	strcpy( define->name, token.c_str( ) );
	// add the define to the source
	AddDefineToHash( define, Parser::definehash );
	// if nothing is defined, just return
	if( !Parser::ReadLine( &token ) ) {
		return true;
	}
	// if it is a define with parameters
	if( token.WhiteSpaceBeforeToken( ) == 0 && token == "( " ) {
		// read the define parameters
		last = NULL;
		if( !Parser::CheckTokenString( " )" ) ) {
			while( 1 ) {
				if( !Parser::ReadLine( &token ) ) {
					Parser::Error( "expected define parameter" );
					return false;
				}
				// if it isn't a name
				if( token.type != TT_NAME ) {
					Parser::Error( "invalid define parameter" );
					return false;
				}

				if( FindDefineParm( define, token.c_str( ) ) >= 0 ) {
					Parser::Error( "two the same define parameters" );
					return false;
				}
				// add the define parm
				t = new Token( token );
				t->ClearTokenWhiteSpace( );
				t->next = NULL;
				if( last ) last->next = t;
				else define->parms = t;
				last = t;
				define->numparms++;
				// read next token
				if( !Parser::ReadLine( &token ) ) {
					Parser::Error( "define parameters not terminated" );
					return false;
				}

				if( token == " )" ) {
					break;
				}
				// then it must be a comma
				if( token != ", " ) {
					Parser::Error( "define not terminated" );
					return false;
				}
			}
		}
		if( !Parser::ReadLine( &token ) ) {
			return true;
		}
	}
	// read the defined stuff
	last = NULL;
	do
	{
		t = new Token( token );
		if( t->type == TT_NAME && !strcmp( t->c_str( ), define->name ) ) {
			t->flags |= TOKEN_FL_RECURSIVE_DEFINE;
			Parser::Warning( "recursive define( removed recursion )" );
		}
		t->ClearTokenWhiteSpace( );
		t->next = NULL;
		if( last ) last->next = t;
		else define->tokens = t;
		last = t;
	} while( Parser::ReadLine( &token ) );

	if( last ) {
		// check for merge operators at the beginning or end
		if( ( *define->tokens ) == "##" ||( *last ) == "##" ) {
			Parser::Error( "define with misplaced ##" );
			return false;
		}
	}
	return true;
}

/*
================
Parser::AddDefine
================
*/
int Parser::AddDefine( const char * string ) {
	define_t * define;

	define = DefineFromString( string );
	if( !define ) {
		return false;
	}
	AddDefineToHash( define, Parser::definehash );
	return true;
}

/*
================
Parser::AddGlobalDefinesToSource
================
*/
void Parser::AddGlobalDefinesToSource( ) {
	define_t * define, * newdefine;

	for( define = globaldefines; define; define = define->next ) {
		newdefine = CopyDefine( define );
		AddDefineToHash( newdefine, Parser::definehash );
	}
}

/*
================
Parser::Directive_if_def
================
*/
int Parser::Directive_if_def( int type ) {
	Token token;
	define_t * d;
	int skip;

	if( !Parser::ReadLine( &token ) ) {
		Parser::Error( "#ifdef without name" );
		return false;
	}
	if( token.type != TT_NAME ) {
		Parser::UnreadSourceToken( &token );
		Parser::Error( "expected name after #ifdef, found '%s'", token.c_str( ) );
		return false;
	}
	d = FindHashedDefine( Parser::definehash, token.c_str( ) );
	skip =( type == INDENT_IFDEF ) ==( d == NULL );
	Parser::PushIndent( type, skip );
	return true;
}

/*
================
Parser::Directive_ifdef
================
*/
int Parser::Directive_ifdef( ) {
	return Parser::Directive_if_def( INDENT_IFDEF );
}

/*
================
Parser::Directive_ifndef
================
*/
int Parser::Directive_ifndef( ) {
	return Parser::Directive_if_def( INDENT_IFNDEF );
}

/*
================
Parser::Directive_else
================
*/
int Parser::Directive_else( ) {
	int type, skip;

	Parser::PopIndent( &type, &skip );
	if( !type ) {
		Parser::Error( "misplaced #else" );
		return false;
	}
	if( type == INDENT_ELSE ) {
		Parser::Error( "#else after #else" );
		return false;
	}
	Parser::PushIndent( INDENT_ELSE, !skip );
	return true;
}

/*
================
Parser::Directive_endif
================
*/
int Parser::Directive_endif( ) {
	int type, skip;

	Parser::PopIndent( &type, &skip );
	if( !type ) {
		Parser::Error( "misplaced #endif" );
		return false;
	}
	return true;
}

/*
================
Parser::EvaluateTokens
================
*/
typedef struct operator_s {

	int op;
	int priority;
	int parentheses;
	struct operator_s * prev, * next;
} operator_t;

typedef struct value_s {

	signed long int intvalue;
	double floatvalue;
	int parentheses;
	struct value_s * prev, * next;
} value_t;

int PC_OperatorPriority( int op ) {
	switch( op ) {
		case P_MUL: return 15;
		case P_DIV: return 15;
		case P_MOD: return 15;
		case P_ADD: return 14;
		case P_SUB: return 14;

		case P_LOGIC_AND: return 7;
		case P_LOGIC_OR: return 6;
		case P_LOGIC_GEQ: return 12;
		case P_LOGIC_LEQ: return 12;
		case P_LOGIC_EQ: return 11;
		case P_LOGIC_UNEQ: return 11;

		case P_LOGIC_NOT: return 16;
		case P_LOGIC_GREATER: return 12;
		case P_LOGIC_LESS: return 12;

		case P_RSHIFT: return 13;
		case P_LSHIFT: return 13;

		case P_BIN_AND: return 10;
		case P_BIN_OR: return 8;
		case P_BIN_XOR: return 9;
		case P_BIN_NOT: return 16;

		case P_COLON: return 5;
		case P_QUESTIONMARK: return 5;
	}
	return false;
}

//#define AllocValue( )			GetClearedMemory( sizeof( value_t ) );
//#define FreeValue( val )		FreeMemory( val )
//#define AllocOperator( op )		op =( operator_t * ) GetClearedMemory( sizeof( operator_t ) );
//#define FreeOperator( op )		FreeMemory( op );

#define MAX_VALUES		64
#define MAX_OPERATORS	64

#define AllocValue( val )									\
	if( numvalues >= MAX_VALUES ) {					\
		Parser::Error( "out of value space\n" );		\
		error = 1;										\
		break;											\
	}													\
	else {												\
		val = &value_heap[ numvalues++ ];					\
	}

#define FreeValue( val )

#define AllocOperator( op )								\
	if( numoperators >= MAX_OPERATORS ) {				\
		Parser::Error( "out of operator space\n" );	\
		error = 1;										\
		break;											\
	}													\
	else {												\
		op = &operator_heap[ numoperators++ ];			\
	}

#define FreeOperator( op )

int Parser::EvaluateTokens( Token * tokens, signed long int * intvalue, double * floatvalue, int integer ) {
	operator_t * o, * firstoperator, * lastoperator;
	value_t * v, * firstvalue, * lastvalue, * v1, * v2;
	Token * t;
	int brace = 0;
	int parentheses = 0;
	int error = 0;
	int lastwasvalue = 0;
	int negativevalue = 0;
	int questmarkintvalue = 0;
	double questmarkfloatvalue = 0;
	int gotquestmarkvalue = false;
	int lastoperatortype = 0;
	//
	operator_t operator_heap[ MAX_OPERATORS ];
	int numoperators = 0;
	value_t value_heap[ MAX_VALUES ];
	int numvalues = 0;

	firstoperator = lastoperator = NULL;
	firstvalue = lastvalue = NULL;
	if( intvalue ) * intvalue = 0;
	if( floatvalue ) * floatvalue = 0;
	for( t = tokens; t; t = t->next ) {
		switch( t->type ) {
			case TT_NAME:
			{
				if( lastwasvalue || negativevalue ) {
					Parser::Error( "syntax error in #if/#elif" );
					error = 1;
					break;
				}
				if( ( *t ) != "defined" ) {
					Parser::Error( "undefined name '%s' in #if/#elif", t->c_str( ) );
					error = 1;
					break;
				}
				t = t->next;
				if( ( *t ) == "( " ) {
					brace = true;
					t = t->next;
				}
				if( !t || t->type != TT_NAME ) {
					Parser::Error( "defined( ) without name in #if/#elif" );
					error = 1;
					break;
				}
				//v =( value_t * ) GetClearedMemory( sizeof( value_t ) );
				AllocValue( v );
				if( FindHashedDefine( Parser::definehash, t->c_str( ) ) ) {
					v->intvalue = 1;
					v->floatvalue = 1;
				}
				else {
					v->intvalue = 0;
					v->floatvalue = 0;
				}
				v->parentheses = parentheses;
				v->next = NULL;
				v->prev = lastvalue;
				if( lastvalue ) lastvalue->next = v;
				else firstvalue = v;
				lastvalue = v;
				if( brace ) {
					t = t->next;
					if( !t ||( *t ) != " )" ) {
						Parser::Error( "defined missing ) in #if/#elif" );
						error = 1;
						break;
					}
				}
				brace = false;
				// defined( ) creates a value
				lastwasvalue = 1;
				break;
			}
			case TT_NUMBER:
			{
				if( lastwasvalue ) {
					Parser::Error( "syntax error in #if/#elif" );
					error = 1;
					break;
				}
				//v =( value_t * ) GetClearedMemory( sizeof( value_t ) );
				AllocValue( v );
				if( negativevalue ) {
					v->intvalue = - t->GetIntValue( );
					v->floatvalue = - t->GetFloatValue( );
				}
				else {
					v->intvalue = t->GetIntValue( );
					v->floatvalue = t->GetFloatValue( );
				}
				v->parentheses = parentheses;
				v->next = NULL;
				v->prev = lastvalue;
				if( lastvalue ) lastvalue->next = v;
				else firstvalue = v;
				lastvalue = v;
				//last token was a value
				lastwasvalue = 1;
				//
				negativevalue = 0;
				break;
			}
			case TT_PUNCTUATION:
			{
				if( negativevalue ) {
					Parser::Error( "misplaced minus sign in #if/#elif" );
					error = 1;
					break;
				}
				if( t->subtype == P_PARENTHESESOPEN ) {
					parentheses++;
					break;
				}
				else if( t->subtype == P_PARENTHESESCLOSE ) {
					parentheses--;
					if( parentheses < 0 ) {
						Parser::Error( "too many ) in #if/#elsif" );
						error = 1;
					}
					break;
				}
				//check for invalid operators on floating point values
				if( !integer ) {
					if( t->subtype == P_BIN_NOT || t->subtype == P_MOD ||
						t->subtype == P_RSHIFT || t->subtype == P_LSHIFT ||
						t->subtype == P_BIN_AND || t->subtype == P_BIN_OR ||
						t->subtype == P_BIN_XOR ) {
						Parser::Error( "illigal operator '%s' on floating point operands\n", t->c_str( ) );
						error = 1;
						break;
					}
				}
				switch( t->subtype ) {
					case P_LOGIC_NOT:
					case P_BIN_NOT:
					{
						if( lastwasvalue ) {
							Parser::Error( "! or ~ after value in #if/#elif" );
							error = 1;
							break;
						}
						break;
					}
					case P_INC:
					case P_DEC:
					{
						Parser::Error( "++ or -- used in #if/#elif" );
						break;
					}
					case P_SUB:
					{
						if( !lastwasvalue ) {
							negativevalue = 1;
							break;
						}
					}
					
					case P_MUL:
					case P_DIV:
					case P_MOD:
					case P_ADD:

					case P_LOGIC_AND:
					case P_LOGIC_OR:
					case P_LOGIC_GEQ:
					case P_LOGIC_LEQ:
					case P_LOGIC_EQ:
					case P_LOGIC_UNEQ:

					case P_LOGIC_GREATER:
					case P_LOGIC_LESS:

					case P_RSHIFT:
					case P_LSHIFT:

					case P_BIN_AND:
					case P_BIN_OR:
					case P_BIN_XOR:

					case P_COLON:
					case P_QUESTIONMARK:
					{
						if( !lastwasvalue ) {
							Parser::Error( "operator '%s' after operator in #if/#elif", t->c_str( ) );
							error = 1;
							break;
						}
						break;
					}
					default:
					{
						Parser::Error( "invalid operator '%s' in #if/#elif", t->c_str( ) );
						error = 1;
						break;
					}
				}
				if( !error && !negativevalue ) {
					//o =( operator_t * ) GetClearedMemory( sizeof( operator_t ) );
					AllocOperator( o );
					o->op = t->subtype;
					o->priority = PC_OperatorPriority( t->subtype );
					o->parentheses = parentheses;
					o->next = NULL;
					o->prev = lastoperator;
					if( lastoperator ) lastoperator->next = o;
					else firstoperator = o;
					lastoperator = o;
					lastwasvalue = 0;
				}
				break;
			}
			default:
			{
				Parser::Error( "unknown '%s' in #if/#elif", t->c_str( ) );
				error = 1;
				break;
			}
		}
		if( error ) {
			break;
		}
	}
	if( !error ) {
		if( !lastwasvalue ) {
			Parser::Error( "trailing operator in #if/#elif" );
			error = 1;
		}
		else if( parentheses ) {
			Parser::Error( "too many( in #if/#elif" );
			error = 1;
		}
	}
	//
	gotquestmarkvalue = false;
	questmarkintvalue = 0;
	questmarkfloatvalue = 0;
	//while there are operators
	while( !error && firstoperator ) {
		v = firstvalue;
		for( o = firstoperator; o->next; o = o->next ) {
			//if the current operator is nested deeper in parentheses
			//than the next operator
			if( o->parentheses > o->next->parentheses ) {
				break;
			}
			//if the current and next operator are nested equally deep in parentheses
			if( o->parentheses == o->next->parentheses ) {
				//if the priority of the current operator is equal or higher
				//than the priority of the next operator
				if( o->priority >= o->next->priority ) {
					break;
				}
			}
			//if the arity of the operator isn't equal to 1
			if( o->op != P_LOGIC_NOT && o->op != P_BIN_NOT ) {
				v = v->next;
			}
			//if there's no value or no next value
			if( !v ) {
				Parser::Error( "mising values in #if/#elif" );
				error = 1;
				break;
			}
		}
		if( error ) {
			break;
		}
		v1 = v;
		v2 = v->next;
#ifdef DEBUG_EVAL
		if( integer ) {
			Log_Write( "operator %s, value1 = %d", Parser::scriptstack->getPunctuationFromId( o->op ), v1->intvalue );
			if( v2 ) Log_Write( "value2 = %d", v2->intvalue );
		}
		else {
			Log_Write( "operator %s, value1 = %f", Parser::scriptstack->getPunctuationFromId( o->op ), v1->floatvalue );
			if( v2 ) Log_Write( "value2 = %f", v2->floatvalue );
		}
#endif //DEBUG_EVAL
		switch( o->op ) {
			case P_LOGIC_NOT:		v1->intvalue = !v1->intvalue;
									v1->floatvalue = !v1->floatvalue; break;
			case P_BIN_NOT:			v1->intvalue = ~v1->intvalue;
									break;
			case P_MUL:				v1->intvalue *= v2->intvalue;
									v1->floatvalue *= v2->floatvalue; break;
			case P_DIV:				if( !v2->intvalue || !v2->floatvalue )
									{
										Parser::Error( "divide by zero in #if/#elif\n" );
										error = 1;
										break;
									}
									v1->intvalue /= v2->intvalue;
									v1->floatvalue /= v2->floatvalue; break;
			case P_MOD:				if( !v2->intvalue )
									{
										Parser::Error( "divide by zero in #if/#elif\n" );
										error = 1;
										break;
									}
									v1->intvalue %= v2->intvalue; break;
			case P_ADD:				v1->intvalue += v2->intvalue;
									v1->floatvalue += v2->floatvalue; break;
			case P_SUB:				v1->intvalue -= v2->intvalue;
									v1->floatvalue -= v2->floatvalue; break;
			case P_LOGIC_AND:		v1->intvalue = v1->intvalue && v2->intvalue;
									v1->floatvalue = v1->floatvalue && v2->floatvalue; break;
			case P_LOGIC_OR:		v1->intvalue = v1->intvalue || v2->intvalue;
									v1->floatvalue = v1->floatvalue || v2->floatvalue; break;
			case P_LOGIC_GEQ:		v1->intvalue = v1->intvalue >= v2->intvalue;
									v1->floatvalue = v1->floatvalue >= v2->floatvalue; break;
			case P_LOGIC_LEQ:		v1->intvalue = v1->intvalue <= v2->intvalue;
									v1->floatvalue = v1->floatvalue <= v2->floatvalue; break;
			case P_LOGIC_EQ:		v1->intvalue = v1->intvalue == v2->intvalue;
									v1->floatvalue = v1->floatvalue == v2->floatvalue; break;
			case P_LOGIC_UNEQ:		v1->intvalue = v1->intvalue != v2->intvalue;
									v1->floatvalue = v1->floatvalue != v2->floatvalue; break;
			case P_LOGIC_GREATER:	v1->intvalue = v1->intvalue > v2->intvalue;
									v1->floatvalue = v1->floatvalue > v2->floatvalue; break;
			case P_LOGIC_LESS:		v1->intvalue = v1->intvalue < v2->intvalue;
									v1->floatvalue = v1->floatvalue < v2->floatvalue; break;
			case P_RSHIFT:			v1->intvalue >>= v2->intvalue;
									break;
			case P_LSHIFT:			v1->intvalue <<= v2->intvalue;
									break;
			case P_BIN_AND:			v1->intvalue &= v2->intvalue;
									break;
			case P_BIN_OR:			v1->intvalue |= v2->intvalue;
									break;
			case P_BIN_XOR:			v1->intvalue ^= v2->intvalue;
									break;
			case P_COLON:
			{
				if( !gotquestmarkvalue ) {
					Parser::Error( ": without ? in #if/#elif" );
					error = 1;
					break;
				}
				if( integer ) {
					if( !questmarkintvalue )
						v1->intvalue = v2->intvalue;
				}
				else {
					if( !questmarkfloatvalue )
						v1->floatvalue = v2->floatvalue;
				}
				gotquestmarkvalue = false;
				break;
			}
			case P_QUESTIONMARK:
			{
				if( gotquestmarkvalue ) {
					Parser::Error( "? after ? in #if/#elif" );
					error = 1;
					break;
				}
				questmarkintvalue = v1->intvalue;
				questmarkfloatvalue = v1->floatvalue;
				gotquestmarkvalue = true;
				break;
			}
		}
#ifdef DEBUG_EVAL
		if( integer ) Log_Write( "result value = %d", v1->intvalue );
		else Log_Write( "result value = %f", v1->floatvalue );
#endif //DEBUG_EVAL
		if( error )
			break;
		lastoperatortype = o->op;
		//if not an operator with arity 1
		if( o->op != P_LOGIC_NOT && o->op != P_BIN_NOT ) {
			//remove the second value if not question mark operator
			if( o->op != P_QUESTIONMARK ) {
				v = v->next;
			}
			//
			if( v->prev ) v->prev->next = v->next;
			else firstvalue = v->next;
			if( v->next ) v->next->prev = v->prev;
			else lastvalue = v->prev;
			//FreeMemory( v );
			FreeValue( v );
		}
		//remove the operator
		if( o->prev ) o->prev->next = o->next;
		else firstoperator = o->next;
		if( o->next ) o->next->prev = o->prev;
		else lastoperator = o->prev;
		//FreeMemory( o );
		FreeOperator( o );
	}
	if( firstvalue ) {
		if( intvalue ) * intvalue = firstvalue->intvalue;
		if( floatvalue ) * floatvalue = firstvalue->floatvalue;
	}
	for( o = firstoperator; o; o = lastoperator ) {
		lastoperator = o->next;
		//FreeMemory( o );
		FreeOperator( o );
	}
	for( v = firstvalue; v; v = lastvalue ) {
		lastvalue = v->next;
		//FreeMemory( v );
		FreeValue( v );
	}
	if( !error ) {
		return true;
	}
	if( intvalue ) {
		* intvalue = 0;
	}
	if( floatvalue ) {
		* floatvalue = 0;
	}
	return false;
}

/*
================
Parser::Evaluate
================
*/
int Parser::Evaluate( signed long int * intvalue, double * floatvalue, int integer ) {
	Token token, * firsttoken, * lasttoken;
	Token * t, * nexttoken;
	define_t * define;
	int defined = false;

	if( intvalue ) {
		* intvalue = 0;
	}
	if( floatvalue ) {
		* floatvalue = 0;
	}
	//
	if( !Parser::ReadLine( &token ) ) {
		Parser::Error( "no value after #if/#elif" );
		return false;
	}
	firsttoken = NULL;
	lasttoken = NULL;
	do {
		//if the token is a name
		if( token.type == TT_NAME ) {
			if( defined ) {
				defined = false;
				t = new Token( token );
				t->next = NULL;
				if( lasttoken ) lasttoken->next = t;
				else firsttoken = t;
				lasttoken = t;
			}
			else if( token == "defined" ) {
				defined = true;
				t = new Token( token );
				t->next = NULL;
				if( lasttoken ) lasttoken->next = t;
				else firsttoken = t;
				lasttoken = t;
			}
			else {
				//then it must be a define
				define = FindHashedDefine( Parser::definehash, token.c_str( ) );
				if( !define ) {
					Parser::Error( "can't Evaluate '%s', not defined", token.c_str( ) );
					return false;
				}
				if( !Parser::ExpandDefineIntoSource( &token, define ) ) {
					return false;
				}
			}
		}
		//if the token is a number or a punctuation
		else if( token.type == TT_NUMBER || token.type == TT_PUNCTUATION ) {
			t = new Token( token );
			t->next = NULL;
			if( lasttoken ) lasttoken->next = t;
			else firsttoken = t;
			lasttoken = t;
		}
		else {
			Parser::Error( "can't Evaluate '%s'", token.c_str( ) );
			return false;
		}
	} while( Parser::ReadLine( &token ) );
	//
	if( !Parser::EvaluateTokens( firsttoken, intvalue, floatvalue, integer ) ) {
		return false;
	}
	//
#ifdef DEBUG_EVAL
	Log_Write( "eval:" );
#endif //DEBUG_EVAL
	for( t = firsttoken; t; t = nexttoken ) {
#ifdef DEBUG_EVAL
		Log_Write( " %s", t->c_str( ) );
#endif //DEBUG_EVAL
		nexttoken = t->next;
		delete t;
	} //end for
#ifdef DEBUG_EVAL
	if( integer ) Log_Write( "eval result: %d", * intvalue );
	else Log_Write( "eval result: %f", * floatvalue );
#endif //DEBUG_EVAL
	//
	return true;
}

/*
================
Parser::DollarEvaluate
================
*/
int Parser::DollarEvaluate( signed long int * intvalue, double * floatvalue, int integer ) {
	int indent, defined = false;
	Token token, * firsttoken, * lasttoken;
	Token * t, * nexttoken;
	define_t * define;

	if( intvalue ) {
		* intvalue = 0;
	}
	if( floatvalue ) {
		* floatvalue = 0;
	}
	//
	if( !Parser::ReadSourceToken( &token ) ) {
		Parser::Error( "no leading( after $ <evalint/$ <evalfloat" );
		return false;
	}
	if( !Parser::ReadSourceToken( &token ) ) {
		Parser::Error( "nothing to Evaluate" );
		return false;
	}
	indent = 1;
	firsttoken = NULL;
	lasttoken = NULL;
	do {
		//if the token is a name
		if( token.type == TT_NAME ) {
			if( defined ) {
				defined = false;
				t = new Token( token );
				t->next = NULL;
				if( lasttoken ) lasttoken->next = t;
				else firsttoken = t;
				lasttoken = t;
			}
			else if( token == "defined" ) {
				defined = true;
				t = new Token( token );
				t->next = NULL;
				if( lasttoken ) lasttoken->next = t;
				else firsttoken = t;
				lasttoken = t;
			}
			else {
				//then it must be a define
				define = FindHashedDefine( Parser::definehash, token.c_str( ) );
				if( !define ) {
					Parser::Warning( "can't Evaluate '%s', not defined", token.c_str( ) );
					return false;
				}
				if( !Parser::ExpandDefineIntoSource( &token, define ) ) {
					return false;
				}
			}
		}
		//if the token is a number or a punctuation
		else if( token.type == TT_NUMBER || token.type == TT_PUNCTUATION ) {
			if( token[ 0 ] == '(' ) indent++;
			else if( token[ 0 ] == ')' ) indent--;
			if( indent <= 0 ) {
				break;
			}
			t = new Token( token );
			t->next = NULL;
			if( lasttoken ) lasttoken->next = t;
			else firsttoken = t;
			lasttoken = t;
		}
		else {
			Parser::Error( "can't Evaluate '%s'", token.c_str( ) );
			return false;
		}
	} while( Parser::ReadSourceToken( &token ) );
	//
	if( !Parser::EvaluateTokens( firsttoken, intvalue, floatvalue, integer ) ) {
		return false;
	}
	//
#ifdef DEBUG_EVAL
	Log_Write( "$ <eval:" );
#endif //DEBUG_EVAL
	for( t = firsttoken; t; t = nexttoken ) {
#ifdef DEBUG_EVAL
		Log_Write( " %s", t->c_str( ) );
#endif //DEBUG_EVAL
		nexttoken = t->next;
		delete t;
	} //end for
#ifdef DEBUG_EVAL
	if( integer ) Log_Write( "$ <eval result: %d", * intvalue );
	else Log_Write( "$ <eval result: %f", * floatvalue );
#endif //DEBUG_EVAL
	//
	return true;
}

/*
================
Parser::Directive_elif
================
*/
int Parser::Directive_elif( ) {
	signed long int value;
	int type, skip;

	Parser::PopIndent( &type, &skip );
	if( !type || type == INDENT_ELSE ) {
		Parser::Error( "misplaced #elif" );
		return false;
	}
	if( !Parser::Evaluate( &value, NULL, true ) ) {
		return false;
	}
	skip =( value == 0 );
	Parser::PushIndent( INDENT_ELIF, skip );
	return true;
}

/*
================
Parser::Directive_if
================
*/
int Parser::Directive_if( ) {
	signed long int value;
	int skip;

	if( !Parser::Evaluate( &value, NULL, true ) ) {
		return false;
	}
	skip =( value == 0 );
	Parser::PushIndent( INDENT_IF, skip );
	return true;
}

/*
================
Parser::Directive_line
================
*/
int Parser::Directive_line( ) {
	Token token;

	Parser::Error( "#line directive not supported" );
	while( Parser::ReadLine( &token ) ) {
	}
	return true;
}

/*
================
Parser::Directive_error
================
*/
int Parser::Directive_error( ) {
	Token token;

	if( !Parser::ReadLine( &token ) || token.type != TT_STRING ) {
		Parser::Error( "#error without string" );
		return false;
	}
	Parser::Error( "#error: %s", token.c_str( ) );
	return true;
}

/*
================
Parser::Directive_warning
================
*/
int Parser::Directive_warning( ) {
	Token token;

	if( !Parser::ReadLine( &token ) || token.type != TT_STRING ) {
		Parser::Warning( "#warning without string" );
		return false;
	}
	Parser::Warning( "#warning: %s", token.c_str( ) );
	return true;
}

/*
================
Parser::Directive_pragma
================
*/
int Parser::Directive_pragma( ) {
	Token token;

	Parser::Warning( "#pragma directive not supported" );
	while( Parser::ReadLine( &token ) ) {
	}
	return true;
}

/*
================
Parser::UnreadSignToken
================
*/
void Parser::UnreadSignToken( ) {
	Token token;

	token.line = Parser::scriptstack->GetLineNum( );
	token.whiteSpaceStart_p = NULL;
	token.whiteSpaceEnd_p = NULL;
	token.linesCrossed = 0;
	token.flags = 0;
	token = "-";
	token.type = TT_PUNCTUATION;
	token.subtype = P_SUB;
	Parser::UnreadSourceToken( &token );
}

/*
================
Parser::Directive_eval
================
*/
int Parser::Directive_eval( ) {
	signed long int value;
	Token token;
	char buf[ 128 ];

	if( !Parser::Evaluate( &value, NULL, true ) ) {
		return false;
	}

	token.line = Parser::scriptstack->GetLineNum( );
	token.whiteSpaceStart_p = NULL;
	token.whiteSpaceEnd_p = NULL;
	token.linesCrossed = 0;
	token.flags = 0;
	sprintf( buf, "%d", abs( value ) );
	token = buf;
	token.type = TT_NUMBER;
	token.subtype = TT_INTEGER|TT_LONG|TT_DECIMAL;
	Parser::UnreadSourceToken( &token );
	if( value < 0 ) {
		Parser::UnreadSignToken( );
	}
	return true;
}

/*
================
Parser::Directive_evalfloat
================
*/
int Parser::Directive_evalfloat( ) {
	double value;
	Token token;
	char buf[ 128 ];

	if( !Parser::Evaluate( NULL, &value, false ) ) {
		return false;
	}

	token.line = Parser::scriptstack->GetLineNum( );
	token.whiteSpaceStart_p = NULL;
	token.whiteSpaceEnd_p = NULL;
	token.linesCrossed = 0;
	token.flags = 0;
	sprintf( buf, "%1.2f", fabs( value ) );
	token = buf;
	token.type = TT_NUMBER;
	token.subtype = TT_FLOAT|TT_LONG|TT_DECIMAL;
	Parser::UnreadSourceToken( &token );
	if( value < 0 ) {
		Parser::UnreadSignToken( );
	}
	return true;
}

/*
================
Parser::ReadDirective
================
*/
int Parser::ReadDirective( ) {
	Token token;

	//read the directive name
	if( !Parser::ReadSourceToken( &token ) ) {
		Parser::Error( "found '#' without name" );
		return false;
	}
	//directive name must be on the same line
	if( token.linesCrossed > 0 ) {
		Parser::UnreadSourceToken( &token );
		Parser::Error( "found '#' at end of line" );
		return false;
	}
	//if if is a name
	if( token.type == TT_NAME ) {
		if( token == "if" ) {
			return Parser::Directive_if( );
		}
		else if( token == "ifdef" ) {
			return Parser::Directive_ifdef( );
		}
		else if( token == "ifndef" ) {
			return Parser::Directive_ifndef( );
		}
		else if( token == "elif" ) {
			return Parser::Directive_elif( );
		}
		else if( token == "else" ) {
			return Parser::Directive_else( );
		}
		else if( token == "endif" ) {
			return Parser::Directive_endif( );
		}
		else if( Parser::skip > 0 ) {
			// skip the rest of the line
			while( Parser::ReadLine( &token ) ) {
			}
			return true;
		}
		else {
			if( token == "include" ) {
				return Parser::Directive_include( );
			}
			else if( token == "define" ) {
				return Parser::Directive_define( );
			}
			else if( token == "undef" ) {
				return Parser::Directive_undef( );
			}
			else if( token == "line" ) {
				return Parser::Directive_line( );
			}
			else if( token == "error" ) {
				return Parser::Directive_error( );
			}
			else if( token == "warning" ) {
				return Parser::Directive_warning( );
			}
			else if( token == "pragma" ) {
				return Parser::Directive_pragma( );
			}
			else if( token == "eval" ) {
				return Parser::Directive_eval( );
			}
			else if( token == "evalfloat" ) {
				return Parser::Directive_evalfloat( );
			}
		}
	}
	Parser::Error( "unknown precompiler directive '%s'", token.c_str( ) );
	return false;
}

/*
================
Parser::DollarDirective_evalint
================
*/
int Parser::DollarDirective_evalint( ) {
	signed long int value;
	Token token;
	char buf[ 128 ];

	if( !Parser::DollarEvaluate( &value, NULL, true ) ) {
		return false;
	}

	token.line = Parser::scriptstack->GetLineNum( );
	token.whiteSpaceStart_p = NULL;
	token.whiteSpaceEnd_p = NULL;
	token.linesCrossed = 0;
	token.flags = 0;
	sprintf( buf, "%d", abs( value ) );
	token = buf;
	token.type = TT_NUMBER;
	token.subtype = TT_INTEGER | TT_LONG | TT_DECIMAL | TT_VALUESVALID;
	token.intvalue = abs( value );
	token.floatvalue = abs( value );
	Parser::UnreadSourceToken( &token );
	if( value < 0 ) {
		Parser::UnreadSignToken( );
	}
	return true;
}

/*
================
Parser::DollarDirective_evalfloat
================
*/
int Parser::DollarDirective_evalfloat( ) {
	double value;
	Token token;
	char buf[ 128 ];

	if( !Parser::DollarEvaluate( NULL, &value, false ) ) {
		return false;
	}

	token.line = Parser::scriptstack->GetLineNum( );
	token.whiteSpaceStart_p = NULL;
	token.whiteSpaceEnd_p = NULL;
	token.linesCrossed = 0;
	token.flags = 0;
	sprintf( buf, "%1.2f", fabs( value ) );
	token = buf;
	token.type = TT_NUMBER;
	token.subtype = TT_FLOAT | TT_LONG | TT_DECIMAL | TT_VALUESVALID;
	token.intvalue =( unsigned long ) fabs( value );
	token.floatvalue = fabs( value );
	Parser::UnreadSourceToken( &token );
	if( value < 0 ) {
		Parser::UnreadSignToken( );
	}
	return true;
}

/*
================
Parser::ReadDollarDirective
================
*/
int Parser::ReadDollarDirective( ) {
	Token token;

	// read the directive name
	if( !Parser::ReadSourceToken( &token ) ) {
		Parser::Error( "found '$' without name" );
		return false;
	}
	// directive name must be on the same line
	if( token.linesCrossed > 0 ) {
		Parser::UnreadSourceToken( &token );
		Parser::Error( "found '$' at end of line" );
		return false;
	}
	// if if is a name
	if( token.type == TT_NAME ) {
		if( token == "evalint" ) {
			return Parser::DollarDirective_evalint( );
		}
		else if( token == "evalfloat" ) {
			return Parser::DollarDirective_evalfloat( );
		}
	}
	Parser::UnreadSourceToken( &token );
	return false;
}

/*
================
Parser::ReadToken
================
*/
int Parser::ReadToken( Token * token ) {
	define_t * define;

	while( 1 ) {
		if( !Parser::ReadSourceToken( token ) ) {
			return false;
		}
		// check for precompiler directives
		if( token->type == TT_PUNCTUATION &&( *token )[ 0 ] == '#' &&( *token )[ 1 ] == '\0' ) {
			// read the precompiler directive
			if( !Parser::ReadDirective( ) ) {
				return false;
			}
			continue;
		}
		// if skipping source because of conditional compilation
		if( Parser::skip ) {
			continue;
		}
		// recursively concatenate strings that are behind each other still resolving defines
		if( token->type == TT_STRING && !( Parser::scriptstack->GetFlags( ) & LEXFL_NOSTRINGCONCAT ) ) {
			Token newtoken;
			if( Parser::ReadToken( &newtoken ) ) {
				if( newtoken.type == TT_STRING ) {
					token->Append( newtoken.c_str( ) );
				}
				else {
					Parser::UnreadSourceToken( &newtoken );
				}
			}
		}
		//
		if( !( Parser::scriptstack->GetFlags( ) & LEXFL_NODOLLARPRECOMPILE ) ) {
			// check for special precompiler directives
			if( token->type == TT_PUNCTUATION &&( *token )[ 0 ] == '$' &&( *token )[ 1 ] == '\0' ) {
				// read the precompiler directive
				if( Parser::ReadDollarDirective( ) ) {
					continue;
				}
			}
		}
		// if the token is a name
		if( token->type == TT_NAME && !( token->flags & TOKEN_FL_RECURSIVE_DEFINE ) ) {
			// check if the name is a define macro
			define = FindHashedDefine( Parser::definehash, token->c_str( ) );
			// if it is a define macro
			if( define ) {
				// expand the defined macro
				if( !Parser::ExpandDefineIntoSource( token, define ) ) {
					return false;
				}
				continue;
			}
		}
		// found a token
		return true;
	}
}

/*
================
Parser::ExpectTokenString
================
*/
int Parser::ExpectTokenString( const char * string ) {
	Token token;

	if( !Parser::ReadToken( &token ) ) {
		Parser::Error( "couldn't find expected '%s'", string );
		return false;
	}

	if( token != string ) {
		Parser::Error( "expected '%s' but found '%s'", string, token.c_str( ) );
		return false;
	}
	return true;
}

/*
================
Parser::ExpectTokenType
================
*/
int Parser::ExpectTokenType( int type, int subtype, Token * token ) {
	Str str;

	if( !Parser::ReadToken( token ) ) {
		Parser::Error( "couldn't read expected token" );
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
		Parser::Error( "expected a %s but found '%s'", str.c_str( ), token->c_str( ) );
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
			Parser::Error( "expected %s but found '%s'", str.c_str( ), token->c_str( ) );
			return 0;
		}
	}
	else if( token->type == TT_PUNCTUATION ) {
		if( subtype < 0 ) {
			Parser::Error( "BUG: wrong punctuation subtype" );
			return 0;
		}
		if( token->subtype != subtype ) {
			Parser::Error( "expected '%s' but found '%s'", scriptstack->GetPunctuationFromId( subtype ), token->c_str( ) );
			return 0;
		}
	}
	return 1;
}

/*
================
Parser::ExpectAnyToken
================
*/
int Parser::ExpectAnyToken( Token * token ) {
	if( !Parser::ReadToken( token ) ) {
		Parser::Error( "couldn't read expected token" );
		return false;
	}
	else {
		return true;
	}
}

/*
================
Parser::CheckTokenString
================
*/
int Parser::CheckTokenString( const char * string ) {
	Token tok;

	if( !ReadToken( &tok ) ) {
		return false;
	}
	//if the token is available
	if( tok == string ) {
		return true;
	}

	UnreadSourceToken( &tok );
	return false;
}

/*
================
Parser::CheckTokenType
================
*/
int Parser::CheckTokenType( int type, int subtype, Token * token ) {
	Token tok;

	if( !ReadToken( &tok ) ) {
		return false;
	}
	//if the type matches
	if( tok.type == type &&( tok.subtype & subtype ) == subtype ) {
		* token = tok;
		return true;
	}

	UnreadSourceToken( &tok );
	return false;
}

/*
================
Parser::PeekTokenString
================
*/
int Parser::PeekTokenString( const char * string ) {
	Token tok;

	if( !ReadToken( &tok ) ) {
		return false;
	}

	UnreadSourceToken( &tok );

	// if the token is available
	if( tok == string ) {
		return true;
	}
	return false;
}

/*
================
Parser::PeekTokenType
================
*/
int Parser::PeekTokenType( int type, int subtype, Token * token ) {
	Token tok;

	if( !ReadToken( &tok ) ) {
		return false;
	}

	UnreadSourceToken( &tok );

	// if the type matches
	if( tok.type == type &&( tok.subtype & subtype ) == subtype ) {
		* token = tok;
		return true;
	}
	return false;
}

/*
================
Parser::SkipUntilString
================
*/
int Parser::SkipUntilString( const char * string ) {
	Token token;

	while( Parser::ReadToken( &token ) ) {
		if( token == string ) {
			return true;
		}
	}
	return false;
}

/*
================
Parser::SkipRestOfLine
================
*/
int Parser::SkipRestOfLine( ) {
	Token token;

	while( Parser::ReadToken( &token ) ) {
		if( token.linesCrossed ) {
			Parser::UnreadSourceToken( &token );
			return true;
		}
	}
	return false;
}

/*
=================
Parser::SkipBracedSection

Skips until a matching close brace is found.
Internal brace depths are properly skipped.
=================
*/
int Parser::SkipBracedSection( bool parseFirstBrace ) {
	Token token;
	int depth;

	depth = parseFirstBrace ? 0 : 1;
	do {
		if( !ReadToken( &token ) ) {
			return false;
		}
		if( token.type == TT_PUNCTUATION ) {
			if( token == "{" ) {
				depth++;
			} else if( token == "}" ) {
				depth--;
			}
		}
	} while( depth );
	return true;
}

/*
=================
Parser::ParseBracedSectionExact

The next token should be an open brace.
Parses until a matching close brace is found.
Maintains the exact formating of the braced section

  FIXME: what about precompilation ?
=================
*/
const char * Parser::ParseBracedSectionExact( Str &out, int tabs ) {
	return scriptstack->ParseBracedSectionExact( out, tabs );
}

/*
=================
Parser::ParseBracedSection

The next token should be an open brace.
Parses until a matching close brace is found.
Internal brace depths are properly skipped.
=================
*/
const char * Parser::ParseBracedSection( Str &out, int tabs ) {
	Token token;
	int i, depth;
	bool doTabs = false;
	if( tabs >= 0 ) {
		doTabs = true;
	}

	out.Empty( );
	if( !Parser::ExpectTokenString( "{" ) ) {
		return out.c_str( );
	}
	out = "{";
	depth = 1;
	do {
		if( !Parser::ReadToken( &token ) ) {
			Error( "missing closing brace" );
			return out.c_str( );
		}

		// if the token is on a new line
		for( i = 0; i < token.linesCrossed; i++ ) {
			out += "\r\n";
		}

		if( doTabs && token.linesCrossed ) {
			i = tabs;
			if( token[ 0 ] == '}' && i > 0 ) {
				i--;
			}
			while( i-- > 0 ) {
				out += "\t";
			}
		}
		if( token.type == TT_PUNCTUATION ) {
			if( token[ 0 ] == '{' ) {
				depth++;
				if( doTabs ) {
					tabs++;
				}
			}
			else if( token[ 0 ] == '}' ) {
				depth--;
				if( doTabs ) {
					tabs--;
				}
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
Parser::ParseRestOfLine

  parse the rest of the line
=================
*/
const char * Parser::ParseRestOfLine( Str &out ) {
	Token token;

	out.Empty( );
	while( Parser::ReadToken( &token ) ) {
		if( token.linesCrossed ) {
			Parser::UnreadSourceToken( &token );
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
Parser::UnreadToken
================
*/
void Parser::UnreadToken( Token * token ) {
	Parser::UnreadSourceToken( token );
}

/*
================
Parser::ReadTokenOnLine
================
*/
int Parser::ReadTokenOnLine( Token * token ) {
	Token tok;

	if( !Parser::ReadToken( &tok ) ) {
		return false;
	}
	// if no lines were crossed before this token
	if( !tok.linesCrossed ) {
		* token = tok;
		return true;
	}
	//
	Parser::UnreadSourceToken( &tok );
	return false;
}

/*
================
Parser::ParseInt
================
*/
int Parser::ParseInt( ) {
	Token token;

	if( !Parser::ReadToken( &token ) ) {
		Parser::Error( "couldn't read expected integer" );
		return 0;
	}
	if( token.type == TT_PUNCTUATION && token == "-" ) {
		Parser::ExpectTokenType( TT_NUMBER, TT_INTEGER, &token );
		return -( ( signed int ) token.GetIntValue( ) );
	}
	else if( token.type != TT_NUMBER || token.subtype == TT_FLOAT ) {
		Parser::Error( "expected integer value, found '%s'", token.c_str( ) );
	}
	return token.GetIntValue( );
}

/*
================
Parser::ParseBool
================
*/
bool Parser::ParseBool( ) {
	Token token;

	if( !Parser::ExpectTokenType( TT_NUMBER, 0, &token ) ) {
		Parser::Error( "couldn't read expected boolean" );
		return false;
	}
	return( token.GetIntValue( ) != 0 );
}

/*
================
Parser::ParseFloat
================
*/
float Parser::ParseFloat( ) {
	Token token;

	if( !Parser::ReadToken( &token ) ) {
		Parser::Error( "couldn't read expected floating point number" );
		return 0.0f;
	}
	if( token.type == TT_PUNCTUATION && token == "-" ) {
		Parser::ExpectTokenType( TT_NUMBER, 0, &token );
		return -token.GetFloatValue( );
	}
	else if( token.type != TT_NUMBER ) {
		Parser::Error( "expected float value, found '%s'", token.c_str( ) );
	}
	return token.GetFloatValue( );
}

/*
================
Parser::Parse1DMatrix
================
*/
int Parser::Parse1DMatrix( int x, float * m ) {
	int i;

	if( !Parser::ExpectTokenString( "( " ) ) {
		return false;
	}

	for( i = 0; i < x; i++ ) {
		m[ i ] = Parser::ParseFloat( );
	}

	if( !Parser::ExpectTokenString( " )" ) ) {
		return false;
	}
	return true;
}

/*
================
Parser::Parse2DMatrix
================
*/
int Parser::Parse2DMatrix( int y, int x, float * m ) {
	int i;

	if( !Parser::ExpectTokenString( "( " ) ) {
		return false;
	}

	for( i = 0; i < y; i++ ) {
		if( !Parser::Parse1DMatrix( x, m + i * x ) ) {
			return false;
		}
	}

	if( !Parser::ExpectTokenString( " )" ) ) {
		return false;
	}
	return true;
}

/*
================
Parser::Parse3DMatrix
================
*/
int Parser::Parse3DMatrix( int z, int y, int x, float * m ) {
	int i;

	if( !Parser::ExpectTokenString( "( " ) ) {
		return false;
	}

	for( i = 0; i < z; i++ ) {
		if( !Parser::Parse2DMatrix( y, x, m + i * x* y ) ) {
			return false;
		}
	}

	if( !Parser::ExpectTokenString( " )" ) ) {
		return false;
	}
	return true;
}

/*
================
Parser::GetLastWhiteSpace
================
*/
int Parser::GetLastWhiteSpace( Str &whiteSpace ) const {
	if( scriptstack ) {
		scriptstack->GetLastWhiteSpace( whiteSpace );
	} else {
		whiteSpace.Clear( );
	}
	return whiteSpace.Length( );
}

/*
================
Parser::SetMarker
================
*/
void Parser::SetMarker( ) {
	marker_p = NULL;
}

/*
================
Parser::GetStringFromMarker

  FIXME: this is very bad code, the script isn't even garrenteed to still be around
================
*/
void Parser::GetStringFromMarker( Str& out, bool clean ) {
	char* 	p;
	char	save;

	if( marker_p == NULL ) {
		marker_p = scriptstack->buffer;
	}
		
	if( tokens ) {
		p =( char* )tokens->whiteSpaceStart_p;
	} else {
		p =( char* )scriptstack->script_p;
	}
	
	// Set the end character to NULL to give us a complete string
	save = * p;
	* p = 0;
	
	// If cleaning then reparse
	if( clean ) {	
		Parser temp( marker_p, ( int )strlen( marker_p ), "temp", flags );
		Token token;
		while( temp.ReadToken( &token ) ) {
			out += token;
		}
	} else {
		out = marker_p;
	}
	
	// restore the character we set to NULL
	* p = save;		
}

/*
================
Parser::SetIncludePath
================
*/
void Parser::SetIncludePath( const char * path ) {
	Parser::includepath = path;
	// add trailing path seperator
	if( Parser::includepath[Parser::includepath.Length( )-1] != '\\' &&
		Parser::includepath[Parser::includepath.Length( )-1] != '/' ) {
		Parser::includepath += PATHSEPERATOR_STR;
	}
}

/*
================
Parser::SetPunctuations
================
*/
void Parser::SetPunctuations( const punctuation_t * p ) {
	Parser::punctuations = p;
}

/*
================
Parser::SetFlags
================
*/
void Parser::SetFlags( int flags ) {
	Lexer * s;

	Parser::flags = flags;
	for( s = Parser::scriptstack; s; s = s->next ) {
		s->SetFlags( flags );
	}
}

/*
================
Parser::GetFlags
================
*/
int Parser::GetFlags( ) const {
	return Parser::flags;
}

/*
================
Parser::LoadFile
================
*/
int Parser::LoadFile( const char * filename ) {
	Lexer * script;

	if( Parser::loaded ) {
		Common::Com_Error( ERR_DISCONNECT, "Parser::loadFile: another source already loaded" );
		return false;
	}
	script = new Lexer( filename, 0 );
	if( !script->IsLoaded( ) ) {
		delete script;
		return false;
	}
	script->SetFlags( Parser::flags );
	script->SetPunctuations( Parser::punctuations );
	script->next = NULL;
	Parser::filename = filename;
	Parser::scriptstack = script;
	Parser::tokens = NULL;
	Parser::indentstack = NULL;
	Parser::skip = 0;
	Parser::loaded = true;

	if( !Parser::definehash ) {
		Parser::defines = NULL;
		Parser::definehash =( define_t * * ) Mem_ClearedAlloc( DEFINEHASHSIZE * sizeof( define_t * ) );
		Parser::AddGlobalDefinesToSource( );
	}
	return true;
}

/*
================
Parser::LoadMemory
================
*/
int Parser::LoadMemory( const char * ptr, int length, const char * name ) {
	Lexer * script;

	if( Parser::loaded ) {
		Common::Com_Error( ERR_DISCONNECT, "Parser::loadMemory: another source already loaded" );
		return false;
	}
	script = new Lexer( ptr, length, name );
	if( !script->IsLoaded( ) ) {
		delete script;
		return false;
	}
	script->SetFlags( Parser::flags );
	script->SetPunctuations( Parser::punctuations );
	script->next = NULL;
	Parser::filename = name;
	Parser::scriptstack = script;
	Parser::tokens = NULL;
	Parser::indentstack = NULL;
	Parser::skip = 0;
	Parser::loaded = true;

	if( !Parser::definehash ) {
		Parser::defines = NULL;
		Parser::definehash =( define_t * * ) Mem_ClearedAlloc( DEFINEHASHSIZE * sizeof( define_t * ) );
		Parser::AddGlobalDefinesToSource( );
	}
	return true;
}

/*
================
Parser::FreeSource
================
*/
void Parser::FreeSource( bool keepDefines ) {
	Lexer * script;
	Token * token;
	define_t * define;
	indent_t * indent;
	int i;

	// free all the scripts
	while( scriptstack ) {
		script = scriptstack;
		scriptstack = scriptstack->next;
		delete script;
	}
	// free all the tokens
	while( tokens ) {
		token = tokens;
		tokens = tokens->next;
		delete token;
	}
	// free all indents
	while( indentstack ) {
		indent = indentstack;
		indentstack = indentstack->next;
		Mem_Free( indent );
	}
	if( !keepDefines ) {
		// free hash table
		if( definehash ) {
			// free defines
			for( i = 0; i < DEFINEHASHSIZE; i++ ) {
				while( definehash[ i ] ) {
					define = definehash[ i ];
					definehash[ i ] = definehash[ i ]->hashnext;
					FreeDefine( define );
				}
			}
			defines = NULL;
			Mem_Free( Parser::definehash );
			definehash = NULL;
		}
	}
	loaded = false;
}

/*
================
Parser::GetPunctuationFromId
================
*/
const char * Parser::GetPunctuationFromId( int id ) {
	int i;

	if( !Parser::punctuations ) {
		Lexer lex;
		return lex.GetPunctuationFromId( id );
	}

	for( i = 0; Parser::punctuations[ i ].p; i++ ) {
		if( Parser::punctuations[ i ].n == id ) {
			return Parser::punctuations[ i ].p;
		}
	}
	return "unkown punctuation";
}

/*
================
Parser::GetPunctuationId
================
*/
int Parser::GetPunctuationId( const char * p ) {
	int i;

	if( !Parser::punctuations ) {
		Lexer lex;
		return lex.GetPunctuationId( p );
	}

	for( i = 0; Parser::punctuations[ i ].p; i++ ) {
		if( !strcmp( Parser::punctuations[ i ].p, p ) ) {
			return Parser::punctuations[ i ].n;
		}
	}
	return 0;
}

/*
================
Parser::Parser
================
*/
Parser::Parser( ) {
	this->loaded = false;
	this->punctuations = 0;
	this->flags = 0;
	this->scriptstack = NULL;
	this->indentstack = NULL;
	this->definehash = NULL;
	this->defines = NULL;
	this->tokens = NULL;
	this->marker_p = NULL;
}

/*
================
Parser::Parser
================
*/
Parser::Parser( int flags ) {
	this->loaded = false;
	this->punctuations = 0;
	this->flags = flags;
	this->scriptstack = NULL;
	this->indentstack = NULL;
	this->definehash = NULL;
	this->defines = NULL;
	this->tokens = NULL;
	this->marker_p = NULL;
}

/*
================
Parser::Parser
================
*/
Parser::Parser( const char * filename, int flags ) {
	this->loaded = false;
	this->punctuations = 0;
	this->flags = flags;
	this->scriptstack = NULL;
	this->indentstack = NULL;
	this->definehash = NULL;
	this->defines = NULL;
	this->tokens = NULL;
	this->marker_p = NULL;
	LoadFile( filename );
}

/*
================
Parser::Parser
================
*/
Parser::Parser( const char * ptr, int length, const char * name, int flags ) {
	this->loaded = false;
	this->punctuations = 0;
	this->flags = flags;
	this->scriptstack = NULL;
	this->indentstack = NULL;
	this->definehash = NULL;
	this->defines = NULL;
	this->tokens = NULL;
	this->marker_p = NULL;
	LoadMemory( ptr, length, name );
}

/*
================
Parser::~Parser
================
*/
Parser::~Parser( ) {
	Parser::FreeSource( false );
}

