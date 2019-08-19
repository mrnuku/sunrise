#include "precompiled.h"
#pragma hdrstop

cvar_t	* cvar_vars;

/*
============
Cvar_InfoValidate
============
*/
static bool Cvar_InfoValidate( char * s ) {

	if( strstr( s, "\\" ) ) return false;
	if( strstr( s, "\"" ) ) return false;
	if( strstr( s, ";" ) ) return false;
	return true;
}

/*
============
Cvar_FindVar
============
*/
static cvar_t * Cvar_FindVar( char * var_name ) {

	cvar_t	* var;
	
	for( var = cvar_vars; var; var = var->next )
		if( !strcmp( var_name, var->name ) ) 	return var;

	return NULL;
}

/*
============
Cvar_VariableValue
============
*/
float Cvar_VariableValue( char * var_name ) {

	cvar_t	* var;
	
	var = Cvar_FindVar( var_name );
	if( !var )
		return 0;
	return atof( var->string );
}


/*
============
Cvar_VariableString
============
*/
char * Cvar_VariableString( char * var_name ) {

	cvar_t * var;
	
	var = Cvar_FindVar( var_name );
	if( !var )
		return "";
	return var->string;
}


/*
============
Cvar_CompleteVariable
============
*/
char * Cvar_CompleteVariable( char * partial ) {

	cvar_t		* cvar;
	int			len;
	
	len = strlen( partial );
	
	if( !len )
		return NULL;
		
	// check exact match
	for( cvar = cvar_vars; cvar; cvar = cvar->next )
		if( !strcmp( partial, cvar->name ) ) 	return cvar->name;

	// check partial match
	for( cvar = cvar_vars; cvar; cvar = cvar->next )
		if( !Str::Cmpn( partial, cvar->name, len ) ) return cvar->name;

	return NULL;
}


/*
============
Cvar_Get

If the variable already exists, the value will not be set
The flags will be or'ed in if the variable exists.
============
*/
cvar_t * Cvar_Get( char * var_name, char * var_value, int flags ) {

	cvar_t	* var;
	
	if( flags &( CVAR_USERINFO | CVAR_SERVERINFO ) ) {
		if( !Cvar_InfoValidate( var_name ) )
		{
			Common::Com_Printf( "invalid info cvar name\n" );
			return NULL;
		}
	}

	var = Cvar_FindVar( var_name );
	if( var )
	{
		var->flags |= flags;
		return var;
	}

	if( !var_value )
		return NULL;

	if( flags &( CVAR_USERINFO | CVAR_SERVERINFO ) ) {
		if( !Cvar_InfoValidate( var_value ) )
		{
			Common::Com_Printf( "invalid info cvar value\n" );
			return NULL;
		}
	}

	var =( cvar_t * )Z_Malloc( sizeof( *var ) );
	var->name = CopyString( var_name );
	var->string = CopyString( var_value );
	var->modified = true;
	var->value = atof( var->string );

	// link the variable in
	var->next = cvar_vars;
	cvar_vars = var;

	var->flags = flags;

	return var;
}

/*
============
Cvar_Set2
============
*/
cvar_t * Cvar_Set2( char * var_name, char * value, bool force ) {

	cvar_t	* var;

	var = Cvar_FindVar( var_name );
	if( !var )
	{	// create it
		return Cvar_Get( var_name, value, 0 );
	}

	if( var->flags &( CVAR_USERINFO | CVAR_SERVERINFO ) ) {
		if( !Cvar_InfoValidate( value ) )
		{
			Common::Com_Printf( "invalid info cvar value\n" );
			return var;
		}
	}

	if( !force )
	{
		if( var->flags & CVAR_NOSET )
		{
			Common::Com_Printf( "%s is write protected.\n", var_name );
			return var;
		}

		if( var->flags & CVAR_LATCH )
		{
			if( var->latched_string )
			{
				if( strcmp( value, var->latched_string ) == 0 )
					return var;
				Z_Free( var->latched_string );
			}
			else
			{
				if( strcmp( value, var->string ) == 0 )
					return var;
			}

			if( Common::Com_ServerState( ) ) {

				Common::Com_Printf( "%s will be changed for next game.\n", var_name );
				var->latched_string = CopyString( value );
			} else {

				var->string = CopyString( value );
				var->value = atof( var->string );
				if( !strcmp( var->name, "game" ) ) {

					FS_SetGamedir( var->string );
					FS_ExecAutoexec( );
				}
			}
			return var;
		}
	}
	else
	{
		if( var->latched_string )
		{
			Z_Free( var->latched_string );
			var->latched_string = NULL;
		}
	}

	if( !strcmp( value, var->string ) ) return var;		// not changed

	var->modified = true;

	if( var->flags & CVAR_USERINFO )
		userinfo_modified = true;	// transmit at next oportunity
	
	Z_Free( var->string );	// free the old value string
	
	var->string = CopyString( value );
	var->value = atof( var->string );

	return var;
}

/*
============
Cvar_ForceSet
============
*/
cvar_t * Cvar_ForceSet( char * var_name, char * value ) {

	return Cvar_Set2( var_name, value, true );
}

/*
============
Cvar_Set
============
*/
cvar_t * Cvar_Set( char * var_name, char * value ) {

	return Cvar_Set2( var_name, value, false );
}

/*
============
Cvar_FullSet
============
*/
cvar_t * Cvar_FullSet( char * var_name, char * value, int flags ) {

	cvar_t	* var;
	
	var = Cvar_FindVar( var_name );
	if( !var )
	{	// create it
		return Cvar_Get( var_name, value, flags );
	}

	var->modified = true;

	if( var->flags & CVAR_USERINFO )
		userinfo_modified = true;	// transmit at next oportunity
	
	Z_Free( var->string );	// free the old value string
	
	var->string = CopyString( value );
	var->value = atof( var->string );
	var->flags = flags;

	return var;
}

/*
============
Cvar_SetValue
============
*/
void Cvar_SetValue( char * var_name, float value ) {

	char	val[ 32 ];

	if( value ==( int )value )
		Com_sprintf( val, sizeof( val ), "%i", ( int )value );
	else
		Com_sprintf( val, sizeof( val ), "%f", value );
	Cvar_Set( var_name, val );
}


/*
============
Cvar_GetLatchedVars

Any variables with latched values will now be updated
============
*/
void Cvar_GetLatchedVars( ) {

	cvar_t	* var;

	for( var = cvar_vars; var; var = var->next )
	{
		if( !var->latched_string )
			continue;
		Z_Free( var->string );
		var->string = var->latched_string;
		var->latched_string = NULL;
		var->value = atof( var->string );
		if( !strcmp( var->name, "game" ) )
		{
			FS_SetGamedir( var->string );
			FS_ExecAutoexec( );
		}
	}
}

/*
============
Cvar_Command

Handles variable inspection and changing from the console
============
*/
bool Cvar_Command( ) {

	cvar_t			* v;

// check variables
	v = Cvar_FindVar( Cmd_Argv( 0 ) );
	if( !v )
		return false;
		
// perform a variable print or set
	if( Cmd_Argc( ) == 1 )
	{
		Common::Com_Printf( "\"%s\" is \"%s\"\n", v->name, v->string );
		return true;
	}

	Cvar_Set( v->name, Cmd_Argv( 1 ) );
	return true;
}


/*
============
Cvar_Set_f

Allows setting and defining of arbitrary cvars from console
============
*/
void Cvar_Set_f( ) {

	int		c;
	int		flags;

	c = Cmd_Argc( );
	if( c != 3 && c != 4 )
	{
		Common::Com_Printf( "usage: set <variable> <value> [u / s]\n" );
		return;
	}

	if( c == 4 )
	{
		if( !strcmp( Cmd_Argv( 3 ), "u" ) ) 	flags = CVAR_USERINFO;
		else if( !strcmp( Cmd_Argv( 3 ), "s" ) ) 	flags = CVAR_SERVERINFO;
		else
		{
			Common::Com_Printf( "flags can only be 'u' or 's'\n" );
			return;
		}
		Cvar_FullSet( Cmd_Argv( 1 ), Cmd_Argv( 2 ), flags );
	}
	else
		Cvar_Set( Cmd_Argv( 1 ), Cmd_Argv( 2 ) );
}


/*
============
Cvar_WriteVariables

Appends lines containing "set variable value" for all variables
with the archive flag set to true.
============
*/
void Cvar_WriteVariables( char * path ) {

	cvar_t	* var;
	char	buffer[ 1024 ];
	FILE	* f;

	f = fopen( path, "a" );
	for( var = cvar_vars; var; var = var->next )
	{
		if( var->flags & CVAR_ARCHIVE )
		{
			Com_sprintf( buffer, sizeof( buffer ), "set %s \"%s\"\n", var->name, var->string );
			fprintf( f, "%s", buffer );
		}
	}
	fclose( f );
}

/*
============
Cvar_List_f

============
*/
void Cvar_List_f( ) {

	cvar_t	* var;
	int		i;

	i = 0;
	for( var = cvar_vars; var; var = var->next, i++ )
	{
		if( var->flags & CVAR_ARCHIVE )
			Common::Com_Printf( "* " );
		else
			Common::Com_Printf( " " );
		if( var->flags & CVAR_USERINFO )
			Common::Com_Printf( "U" );
		else
			Common::Com_Printf( " " );
		if( var->flags & CVAR_SERVERINFO )
			Common::Com_Printf( "S" );
		else
			Common::Com_Printf( " " );
		if( var->flags & CVAR_NOSET )
			Common::Com_Printf( "-" );
		else if( var->flags & CVAR_LATCH )
			Common::Com_Printf( "L" );
		else
			Common::Com_Printf( " " );
		Common::Com_Printf( " %s \"%s\"\n", var->name, var->string );
	}
	Common::Com_Printf( "%i cvars\n", i );
}


bool userinfo_modified;


char	* Cvar_BitInfo( int bit ) {

	static char	info[ MAX_INFO_STRING ];
	cvar_t	* var;

	info[ 0 ] = 0;

	for( var = cvar_vars; var; var = var->next )
	{
		if( var->flags & bit )
			Info_SetValueForKey( info, var->name, var->string );
	}
	return info;
}

// returns an info string containing all the CVAR_USERINFO cvars
char	* Cvar_Userinfo( ) {

	return Cvar_BitInfo( CVAR_USERINFO );
}

// returns an info string containing all the CVAR_SERVERINFO cvars
char	* Cvar_Serverinfo( ) {

	return Cvar_BitInfo( CVAR_SERVERINFO );
}

/*
============
Cvar_Init

Reads in all archived cvars
============
*/
void Cvar_Init( ) {

	Cmd_AddCommand( "set", Cvar_Set_f );
	Cmd_AddCommand( "cvarlist", Cvar_List_f );

}