#include "precompiled.h"
#pragma hdrstop

#define	MAX_ALIAS_NAME	32

typedef struct cmdalias_s {

	Str					name;
	Str					value;

	struct cmdalias_s * 		next;

} cmdalias_t;

cmdalias_t	* cmd_alias;

bool	cmd_wait;

#define	ALIAS_LOOP_COUNT	16
int		alias_count;		// for detecting runaway loops


//=============================================================================

/*
============
Cmd_Wait_f

Causes execution of the remainder of the command buffer to be delayed until
next frame.  This allows commands like:
bind g "impulse 5; +attack; wait; -attack; impulse 2"
============
*/
void Command::Cmd_Wait_f( ) {

	cmd_wait = true;
}


/*
=============================================================================

						COMMAND BUFFER

=============================================================================
*/

MessageBufferT< 8192 >	g_textBuffer;
MessageBufferT< 8192 >	g_deferBuffer;
MessageBufferT< 8192 >	g_tempBuffer;

/*
============
Cbuf_Init
============
*/
void CBuffer::Cbuf_Init( ) {
	g_textBuffer.Init( );
	g_deferBuffer.Init( );
	g_tempBuffer.Init( );
}

/*
============
Cbuf_AddText

Adds command text at the end of the buffer
============
*/
void CBuffer::Cbuf_AddText( const Str & text ) {
	if( text.Length( ) > g_textBuffer.GetRemainingSpace( ) )
		Common::Com_Printf( "Cbuf_AddText: overflow\n" );
	else
		g_textBuffer.WriteString( text );
}


/*
============
Cbuf_InsertText

Adds command text immediately after the current command
Adds a \n to the text
FIXME: actually change the command buffer to do less copying
============
*/
void CBuffer::Cbuf_InsertText( const Str & text ) {
	g_tempBuffer.WriteData( g_textBuffer.GetData( ), g_textBuffer.GetSize( ) );
	g_textBuffer.BeginWriting( );
	// add the entire text of the file
	Cbuf_AddText( text );
	g_textBuffer.WriteData( g_tempBuffer.GetData( ), g_tempBuffer.GetSize( ) );
	g_tempBuffer.BeginWriting( );
}

void CBuffer::Cbuf_CopyToDefer( ) {
	g_deferBuffer.WriteData( g_textBuffer.GetData( ), g_textBuffer.GetSize( ) );
	g_textBuffer.BeginWriting( );
}

void CBuffer::Cbuf_InsertFromDefer( ) {
	g_textBuffer.WriteData( g_deferBuffer.GetData( ), g_deferBuffer.GetSize( ) );
	g_deferBuffer.BeginWriting( );
}

void CBuffer::Cbuf_ExecuteText( int exec_when, const Str & text ) {
	switch( exec_when ) {
		case EXEC_NOW:
			Command::Cmd_ExecuteString( text );
			break;
		case EXEC_INSERT:
			Cbuf_InsertText( text );
			break;
		case EXEC_APPEND:
			Cbuf_AddText( text );
			break;
		default:
			Common::Com_Error( ERR_FATAL, "Cbuf_ExecuteText: bad exec_when" );
	}
}

void CBuffer::Cbuf_Execute( ) {
	Str text;
	alias_count = 0;		// don't allow infinite alias loops
	while( g_textBuffer.GetRemainingReadBits( ) ) {
		int textLen = g_textBuffer.ReadString( text );
		int textEnd, textStart = 0;
		// find a \n or; line break
		int quotes = 0;
		for( textEnd = textStart; textEnd < textLen; textEnd++ ) {
			const char & textChar = text[ textEnd ];
			if( textChar == '"' )
				quotes++;
			if( !( quotes & 1 ) && textChar == ';' )
				break;	// don't break if inside a quoted string
			if( textChar == '\n' )
				break;
		}
		assert( ( textLen - textEnd ) <= 1 );
		Str line( text, textStart, textEnd );
		textStart = textEnd + 1;
		// delete the text from the command buffer and move remaining commands down
		// this is necessary because commands( exec, alias ) can insert data at the
		// beginning of the text buffer
		// execute the command line
		Command::Cmd_ExecuteString( line );
		if( cmd_wait ) {
			// skip out while text still remains in buffer, leaving it
			// for next frame
			cmd_wait = false;
			break;
		}
	}
	g_textBuffer.BeginReading( );
	g_textBuffer.BeginWriting( );
}


/*
===============
Cbuf_AddEarlyCommands

Adds command line parameters as script statements
Commands lead with a +, and continue until another +

Set commands are added early, so they are guaranteed to be set before
the client and server initialize for the first time.

Other commands are added late, after all initialization is complete.
===============
*/
void CBuffer::Cbuf_AddEarlyCommands( bool clear ) {
	int		i;
	Str s, text;
	for( i = 0; i < Common::COM_Argc( ); i++ ) {
		s = Common::COM_Argv( i );
		if( s != "+set" )
			continue;
		sprintf( text, "set %s %s\n", Common::COM_Argv( i + 1 ).c_str( ), Common::COM_Argv( i + 2 ).c_str( ) );
		Cbuf_AddText( text );
		if( clear ) {
			Common::COM_ClearArgv( i );
			Common::COM_ClearArgv( i + 1 );
			Common::COM_ClearArgv( i + 2 );
		}
		i += 2;
	}
}

/*
=================
Cbuf_AddLateCommands

Adds command line parameters as script statements
Commands lead with a + and continue until another + or -
quake +vid_ref gl +map amlev1

Returns true if any late commands were added, which
will keep the demoloop from immediately starting
=================
*/
bool CBuffer::Cbuf_AddLateCommands( ) {
	size_t	s;
	char	* text, * build, c;
	int		argc;
	bool	ret;
	// build the combined string to parse from
	s = 0;
	argc = Common::COM_Argc( );
	for( int i = 1; i<argc; i++ ) {
		s += strlen( Common::COM_Argv( i ) ) + 1;
	}
	if( !s )
		return false;		
	text = ( char * )Z_Malloc( s + 1 );
	text[ 0 ] = 0;
	for( int i = 1; i < argc; i++ ) {
		strcat( text, Common::COM_Argv( i ) );
		if( i != argc-1 )
			strcat( text, " " );
	}	
	// pull out the commands
	build = ( char * )Z_Malloc( s + 1 );
	build[ 0 ] = 0;	
	for( int i = 0; i < s - 1; i++ ) {
		if( text[ i ] == '+' ) {
			int j;
			i++;
			for( j = i;( text[ j ] != '+' ) &&( text[ j ] != '-' ) && ( text[ j ] != 0 ); j++ );
			c = text[ j ];
			text[ j ] = 0;			
			strcat( build, text+i );
			strcat( build, "\n" );
			text[ j ] = c;
			i = j-1;
		}
	}
	ret =( build[ 0 ] != 0 );
	if( ret )
		Cbuf_AddText( build );	
	Z_Free( text );
	Z_Free( build );
	return ret;
}


/*
==============================================================================

						SCRIPT COMMANDS

==============================================================================
*/


/*
===============
Cmd_Exec_f
===============
*/
void Command::Cmd_Exec_f( ) {

	if( Cmd_Argc( ) != 2 ) {

		Common::Com_Printf( "exec <filename> : execute a script file\n" );
		return;
	}

	File_Memory fm = FileSystem::ReadFile( Cmd_Argv( 1 ) );
	if( !fm.IsValid( ) ) return;
	Common::Com_Printf( "execing %s\n", Cmd_Argv( 1 ).c_str( ) );

	CBuffer::Cbuf_InsertText( ( char * )fm.GetDataPtr( ) );

	FileSystem::FreeFile( fm );
}


/*
===============
Cmd_Echo_f

Just prints the rest of the line to the console
===============
*/
void Command::Cmd_Echo_f( ) {
	
	Common::Com_Printf( "%s\n", Cmd_Args( ).c_str( ) );
}

/*
===============
Cmd_Alias_f

Creates a new command that executes a command string( possibly; seperated )
===============
*/
void Command::Cmd_Alias_f( ) {

	cmdalias_t	* a;
	char		cmd[ 1024 ];
	int			i, c;
	Str		s;

	if( Cmd_Argc( ) == 1 ) {

		Common::Com_Printf( "Current alias commands:\n" );
		for( a = cmd_alias; a; a = a->next ) Common::Com_Printf( "%s : %s\n", a->name.c_str( ), a->value.c_str( ) );
		return;
	}

	s = Cmd_Argv( 1 );
	if( strlen( s ) >= MAX_ALIAS_NAME ) {

		Common::Com_Printf( "Alias name is too long\n" );
		return;
	}

	// if the alias already exists, reuse it
	for( a = cmd_alias; a; a = a->next ) {

		if( s == a->name ) {

			a->value.Clear( );
			break;
		}
	}

	if( !a )
	{
		a =( cmdalias_t * )Z_Malloc( sizeof( cmdalias_t ) );
		a->next = cmd_alias;
		cmd_alias = a;
	}
	a->name = s;

// copy the rest of the command line
	cmd[ 0 ] = 0;		// start out with a null string
	c = Cmd_Argc( );
	for( i = 2; i< c; i++ )
	{
		strcat( cmd, Cmd_Argv( i ) );
		if( i !=( c - 1 ) ) 	strcat( cmd, " " );
	}
	strcat( cmd, "\n" );
	
	a->value = cmd;
}

/*
=============================================================================

					COMMAND EXECUTION

=============================================================================
*/

class cmd_function_t {
public:
	cmd_function_t *		next;
	Str						name;
	xcommand_t				function;
};

cmd_function_t *	Command::cmd_functions;

/*
============
Cmd_Argc
============
*/
int Command::Cmd_Argc( ) {

	return cmd_argv.Num( );
}

/*
============
Cmd_Argv
============
*/
const Str & Command::Cmd_Argv( int arg ) {

	if( arg >= Cmd_Argc( ) ) return null_string;

	return cmd_argv[ arg ];	
}

/*
============
Cmd_Args

Returns a single string containing argv( 1 ) to argv( argc( )-1 )
============
*/
const Str & Command::Cmd_Args( ) {

	return cmd_args;
}

/*
============
Cmd_TokenizeString

Parses the given string into command line tokens.
$ <Cvars will be expanded unless they are in a quoted token
============
*/
void Command::Cmd_TokenizeString( const Str & text, bool macroExpand ) {

	// clear the args from the last string
	for( int i = 0; i < cmd_argv.Num( ); cmd_argv[ i++ ].Clear( ) );
	cmd_argv.SetNum( 0, false );

	if( text.Find( ' ' ) )	cmd_args = text.Right( text.Length( ) -( text.Find( ' ' ) + 1 ) );
	else					cmd_args.Clear( );

	Lexer lexer( text.c_str( ), text.Length( ), "CommandTokenizer", LEXFL_NOERRORS | LEXFL_NOWARNINGS | LEXFL_NOSTRINGCONCAT | LEXFL_ALLOWPATHNAMES | LEXFL_ALLOWIPADDRESSES | LEXFL_ONLYSTRINGS );
	Token token;

	while( lexer.ReadToken( &token ) ) {

		if( Cmd_Argc( ) < MAX_STRING_TOKENS ) {

			if( macroExpand && token.subtype == P_DOLLAR ) {

				if( !lexer.ReadToken( &token ) ) return;
				token = CVarSystem::GetCVarStr( token );
			}

			cmd_argv.Append( token );
		}
	}
	
}

/*
============
Cmd_AddCommand
============
*/
void Command::Cmd_AddCommand( const Str & cmd_name, xcommand_t function ) {	
	if( CVarSystem::Find( cmd_name ) ) { // fail if the command is a variable name
		Common::Com_Printf( "Cmd_AddCommand: %s already defined as a var\n", cmd_name.c_str( ) );
		return;
	}	
	cmd_function_t	* cmd;
	for( cmd = cmd_functions; cmd; cmd = cmd->next ) { // fail if the command already exists
		if( cmd_name == cmd->name ) {
			Common::Com_Printf( "Cmd_AddCommand: %s already defined\n", cmd_name.c_str( ) );
			return;
		}
	}
	cmd = new cmd_function_t;
	cmd->name = cmd_name;
	cmd->function = function;
	cmd->next = cmd_functions;
	cmd_functions = cmd;
}

/*
============
Cmd_RemoveCommand
============
*/
void Command::Cmd_RemoveCommand( const Str & cmd_name ) {
	cmd_function_t	* cmd, * *back;
	back = &cmd_functions;
	while( 1 ) {
		cmd = * back;
		if( !cmd ) {
			Common::Com_Printf( "Cmd_RemoveCommand: %s not added\n", cmd_name.c_str( ) );
			return;
		}
		if( cmd_name == cmd->name ) {
			* back = cmd->next;
			delete cmd;
			return;
		}
		back = &cmd->next;
	}
}

/*
============
Cmd_Exists
============
*/
bool Command::Cmd_Exists( const Str & cmd_name ) {

	cmd_function_t	* cmd;

	for( cmd = cmd_functions; cmd; cmd = cmd->next ) {

		if( cmd_name == cmd->name ) return true;
	}

	return false;
}



/*
============
Cmd_CompleteCommand
============
*/
const Str Command::Cmd_CompleteCommand( const Str & partial ) {

	cmd_function_t	* cmd;
	int			len;
	cmdalias_t		* a;
	
	len = ( int )strlen( partial );
	
	if( !len )
		return NULL;
		
// check for exact match
	for( cmd = cmd_functions; cmd; cmd = cmd->next )
		if( partial == cmd->name ) return cmd->name;

	for( a = cmd_alias; a; a = a->next )
		if( partial == a->name ) return a->name;

// check for partial match
	for( cmd = cmd_functions; cmd; cmd = cmd->next )
		if( ! Str::Cmpn( partial, cmd->name, len ) )
			return cmd->name;
	for( a = cmd_alias; a; a = a->next )
		if( !Str::Cmpn( partial, a->name, len ) )
			return a->name;

	return NULL;
}


/*
============
Cmd_ExecuteString

A complete command line has been parsed, so try to execute it
FIXME: lookupnoadd the token to speed search?
============
*/
void Command::Cmd_ExecuteString( const Str & text ) {	
	Cmd_TokenizeString( text, true );			
	// execute the command line
	if( !Cmd_Argc( ) )
		return;		// no tokens
	// check functions
	for( cmd_function_t * cmd = cmd_functions; cmd; cmd = cmd->next ) {
		if( cmd_argv[ 0 ] == cmd->name ) {
			if( !cmd->function )
				Cmd_ExecuteString( Str( "cmd " + text ) ); // forward to server command
			else
				cmd->function( );
			return;
		}
	}
	// check alias
	for( cmdalias_t * a = cmd_alias; a; a = a->next ) {
		if( cmd_argv[ 0 ] == a->name ) {
			if( ++alias_count == ALIAS_LOOP_COUNT ) {
				Common::Com_Printf( "ALIAS_LOOP_COUNT\n" );
				return;
			}
			CBuffer::Cbuf_InsertText( a->value );
			return;
		}
	}	
	// check cvars
	if( CVarSystem::Command( ) )
		return;
	// send it as a server command if we are connected
	Client::Cmd_ForwardToServer( );
}

/*
============
Cmd_List_f
============
*/
void Command::Cmd_List_f( ) {

	int i = 0;
	
	for( cmd_function_t	* cmd = cmd_functions; cmd; cmd = cmd->next, i++ ) Common::Com_Printf( "%s\n", cmd->name.c_str( ) );

	Common::Com_Printf( "%i commands\n", i );
}

/*
============
Cmd_Init
============
*/
void Command::Cmd_Init( ) {

//
// register our commands
//
	Cmd_AddCommand( "cmdlist", Cmd_List_f );
	Cmd_AddCommand( "exec", Cmd_Exec_f );
	Cmd_AddCommand( "echo", Cmd_Echo_f );
	Cmd_AddCommand( "alias", Cmd_Alias_f );
	Cmd_AddCommand( "wait", Cmd_Wait_f );
}

/*

===================

Cmd_ForwardToServer



adds the current command line as a clc_stringcmd to the client message.

things like godmode, noclip, etc, are commands directed to the server, 
so when they are typed in at the console, they will need to be forwarded.

===================

*/
void Client::Cmd_ForwardToServer( ) {
#ifndef DEDICATED_ONLY
	Str cmd = Command::Cmd_Argv( 0 );
	if( cls.state <= ca_connected || * cmd == '-' || * cmd == '+' ) {
		Common::Com_Printf( "Unknown command \"%s\"\n", cmd.c_str( ) );
		return;
	}
	cl_messageBufferReliable.WriteByte( clc_stringcmd );
	cl_messageBufferReliable.WriteString( cmd );
	if( Command::Cmd_Argc( ) > 1 ) {
		cl_messageBufferReliable.WriteString( " " );
		cl_messageBufferReliable.WriteString( Command::Cmd_Args( ) );
	}
#endif
}
