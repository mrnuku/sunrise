#include "precompiled.h"
#pragma hdrstop

mmove_t mmove_reloc;

/*
============
InitGame


This will be called when the dll is first loaded, which
only happens when a new game is started or a save game
is loaded.
============
*/
bool Game::Init( ) {

	Common::Com_DPrintf( "==== InitGame ====\n" );

	// items
	InitItems( );

	helpmessage1.Clear( );
	helpmessage2.Clear( );

	// initialize all entities for this game
	maxEntities = maxentities.GetInt( );
	//g_edicts =( Entity * )Z_TagMalloc( maxEntities * sizeof( Entity ), TAG_GAME );
	edicts = g_edicts;
	max_edicts = maxEntities;
	edict_size = sizeof( Entity );

	// initialize all clients for this game
	maxClients = Common::maxclients.GetInt( );
	//clients =( gclient_t * )Z_TagMalloc( maxClients * sizeof( gclient_t ), TAG_GAME );
	num_edicts = maxClients+1;

	return true;
}

//=========================================================

void Game::WriteField1( FileBase * f, const field_t * field, byte * base ) {

	void * p;
	int len;
	int index;

	if( field->flags & FFL_SPAWNTEMP )
		return;

	p =( void * )( base + field->ofs );
	switch( field->type )
	{
	case F_INT:
	case F_FLOAT:
	case F_ANGLEHACK:
	case F_VECTOR:
	case F_IGNORE:
		break;

	case F_LSTRING:
	case F_GSTRING:
		if( *( char * * )p )
			len = ( int )strlen( *( char * * )p ) + 1;
		else
			len = 0;
		* ( int * )p = len;
		break;
	case F_EDICT:
		if( *( Entity * * )p == NULL )
			index = -1;
		else
			index = ( int )( * ( Entity * * )p - g_edicts );
		* ( int * )p = index;
		break;
	case F_CLIENT:
		if( *( gclient_t * * )p == NULL )
			index = -1;
		else
			index = ( int )( * ( gclient_t * * )p - clients );
		* ( int * )p = index;
		break;
	case F_ITEM:
		if( *( Entity * * )p == NULL )
			index = -1;
		else
			index = ( int )( * ( GameItem * * )p - itemlist );
		* ( int * )p = index;
		break;

		//relative to code segment
	case F_FUNCTION:
		if( *( byte * * )p == NULL )
			index = 0;
		else
			index = ( int )( * ( byte * * )p -( byte * )Init );
		* ( int * )p = index;
		break;

		//relative to data segment
	case F_MMOVE:
		if( *( byte * * )p == NULL )
			index = 0;
		else
			index = ( int )( * ( byte * * )p -( byte * )&mmove_reloc );
		* ( int * )p = index;
		break;

	default:
		Server::PF_error( "WriteEdict: unknown field type" );
	}
}


void Game::WriteField2( FileBase * f, const field_t * field, byte * base ) {

	int len;
	void * p;

	if( field->flags & FFL_SPAWNTEMP )
		return;

	p =( void * )( base + field->ofs );
	switch( field->type )
	{
	case F_LSTRING:
		if( *( char * * )p )
		{
			len = ( int )strlen( *( char * * )p ) + 1;
			f->Write( *( char * * )p, len );
		}
		break;
	}
}

void Game::ReadField( FileBase * f, const field_t * field, byte * base ) {

	void * p;
	int len;
	int index;

	if( field->flags & FFL_SPAWNTEMP )
		return;

	p =( void * )( base + field->ofs );
	switch( field->type )
	{
	case F_INT:
	case F_FLOAT:
	case F_ANGLEHACK:
	case F_VECTOR:
	case F_IGNORE:
		break;

	case F_LSTRING:
		len = * ( int * )p;
		if( !len )
			* ( char * * )p = NULL;
		else
		{
			* ( char * * )p =( char * )Z_TagMalloc( len, TAG_LEVEL );
			f->Read( *( char * * )p, len );
		}
		break;
	case F_EDICT:
		index = * ( int * )p;
		if( index == -1 )
			* ( Entity * * )p = NULL;
		else
			* ( Entity * * )p = &g_edicts[ index ];
		break;
	case F_CLIENT:
		index = * ( int * )p;
		if( index == -1 )
			* ( gclient_t * * )p = NULL;
		else
			* ( gclient_t * * )p = &clients[ index ];
		break;
	case F_ITEM:
		index = * ( int * )p;
		if( index == -1 )
			*( GameItem * * )p = NULL;
		else
			*( const GameItem ** )p = &itemlist[ index ];
		break;

		//relative to code segment
	case F_FUNCTION:
		index = * ( int * )p;
		if( index == 0 )
			* ( byte * * )p = NULL;
		else
			* ( byte * * )p =( ( byte * )Init ) + index;
		break;

		//relative to data segment
	case F_MMOVE:
		index = * ( int * )p;
		if( index == 0 )
			* ( byte * * )p = NULL;
		else
			* ( byte * * )p =( byte * )&mmove_reloc + index;
		break;

	default:
		Server::PF_error( "ReadEdict: unknown field type" );
	}
}

//=========================================================

/*

==============

WriteClient



All pointer variables( except function pointers ) must be handled specially.

==============

*/
void Game::WriteClient( FileBase * f, gclient_t * client ) {

	const field_t * field;
	gclient_t temp;

	// all of the ints, floats, and vectors stay as they are
	temp = * client;

	// change the pointers to lengths or indexes
	for( field = clientfields; field->name.Length( ); field++ ) WriteField1( f, field, ( byte * )&temp );

	// write the block
	f->Write( &temp, sizeof( temp ) );

	// now write any allocated data following the edict
	for( field = clientfields; field->name.Length( ); field++ ) WriteField2( f, field, ( byte * )client );
}

/*

==============

ReadClient



All pointer variables( except function pointers ) must be handled specially.

==============

*/
void Game::ReadClient( FileBase * f, gclient_t * client ) {

	const field_t * field;

	f->Read( client, sizeof( gclient_t ) );

	for( field = clientfields; field->name.Length( ); field++ ) ReadField( f, field, ( byte * )client );
}

/*

============

WriteGame



This will be called whenever the game goes to a new level, 
and when the user explicitly saves the 



Game information include cross level data, like multi level

triggers, help computer info, and all client states.



A single player death will automatically restore from the

last save position.

============

*/
void Game::WriteGame( const Str & filename, bool autosave ) {

	int i;
	char str[ 16 ];

	if( !autosave ) SaveClientData( );

	FileBase * f = FileSystem::OpenFileByMode( filename, FS_WRITE );

	Common::Com_Memset( str, 0, sizeof( str ) );
	strcpy( str, __DATE__ );
	f->Write( str, sizeof( str ) );

	autosaved = autosave;
	// FIXME fwrite( &game, sizeof( game ), 1, f );
	autosaved = false;

	for( i = 0; i<maxClients; i++ ) WriteClient( f, &clients[ i ] );

	FileSystem::CloseFile( f );
}

void Game::ReadGame( const Str & filename ) {

	char str[ 16 ];

	Z_FreeTags( TAG_GAME );

	FileBase * f = FileSystem::OpenFileRead( filename );

	f->Read( str, sizeof( str ) );
	if( strcmp( str, __DATE__ ) ) {

		FileSystem::CloseFile( f );
		Server::PF_error( "Savegame from an older version.\n" );
	}

	//g_edicts =( Entity * )Z_TagMalloc( maxEntities * sizeof( Entity ), TAG_GAME );
	edicts = g_edicts;

	// FIXME fread( &game, sizeof( game ), 1, f );
	//clients =( gclient_t * )Z_TagMalloc( maxClients * sizeof( gclient_t ), TAG_GAME );
	for( int i = 0; i < maxClients; i++ ) ReadClient( f, &clients[ i ] );

	FileSystem::CloseFile( f );
}

//==========================================================


/*

==============

WriteEdict



All pointer variables( except function pointers ) must be handled specially.

==============

*/
void Game::WriteEdict( FileBase * f, Entity * ent ) {

	const field_t * field;
	Entity temp;

	// all of the ints, floats, and vectors stay as they are
	temp = * ent;

	// change the pointers to lengths or indexes
	for( field = fields; field->name; field++ )
	{
		WriteField1( f, field, ( byte * )&temp );
	}

	// write the block
	f->Write( &temp, sizeof( temp ) );

	// now write any allocated data following the edict
	for( field = fields; field->name; field++ )
	{
		WriteField2( f, field, ( byte * )ent );
	}

}

/*

==============

WriteLevelLocals



All pointer variables( except function pointers ) must be handled specially.

==============

*/
void Game::WriteLevelLocals( FileBase * f ) {

	const field_t * field;
	level_locals_t temp;

	// all of the ints, floats, and vectors stay as they are
	temp = level;

	// change the pointers to lengths or indexes
	for( field = levelfields; field->name; field++ )
	{
		WriteField1( f, field, ( byte * )&temp );
	}

	// write the block
	f->Write( &temp, sizeof( temp ) );

	// now write any allocated data following the edict
	for( field = levelfields; field->name; field++ )
	{
		WriteField2( f, field, ( byte * )&level );
	}
}


/*

==============

ReadEdict



All pointer variables( except function pointers ) must be handled specially.

==============

*/
void Game::ReadEdict( FileBase * f, Entity * ent ) {

	const field_t * field;

	f->Read( ent, sizeof( Entity ) );

	for( field = fields; field->name; field++ )
	{
		ReadField( f, field, ( byte * )ent );
	}
}

/*

==============

ReadLevelLocals



All pointer variables( except function pointers ) must be handled specially.

==============

*/
void Game::ReadLevelLocals( FileBase * f ) {

	const field_t * field;

	f->Read( &level, sizeof( level ) );

	for( field = levelfields; field->name; field++ )
	{
		ReadField( f, field, ( byte * )&level );
	}
}

/*

=================

WriteLevel



=================

*/
void Game::WriteLevel( const Str & filename ) {

	int i;
	Entity * ent;
	void * base;

	FileBase * f = FileSystem::OpenFileByMode( filename, FS_WRITE );

	// write out edict size for checking
	i = sizeof( Entity );
	f->Write( &i, sizeof( int ) );

	// write out a function pointer for checking
	// FIXME base =( void * )InitGame;
	f->Write( &base, sizeof( base ) );

	// write out level_locals_t
	WriteLevelLocals( f );

	// write out all the entities
	for( i = 0; i<num_edicts; i++ )
	{
		ent = &g_edicts[ i ];
		if( !ent->inuse )
			continue;
		f->Write( &i, sizeof( int ) );
		WriteEdict( f, ent );
	}
	i = -1;
	f->Write( &i, sizeof( i ) );

	FileSystem::CloseFile( f );
}


/*

=================

ReadLevel



SpawnEntities will allready have been called on the

level the same way it was when the level was saved.



That is necessary to get the baselines

set up entically.



The server will have cleared all of the world links before

calling ReadLevel.



No clients are connected yet.

=================

*/
void Game::ReadLevel( const Str & filename ) {

	int entnum;
	int i;
	void * base;
	Entity * ent;

	FileBase * f = FileSystem::OpenFileRead( filename );

	// free any dynamic memory allocated by loading the level
	// base state
	Z_FreeTags( TAG_LEVEL );

	// wipe all the entities
	for( int i = 0; i < maxEntities; i++ )
		g_edicts[ i ].Clear( );
	num_edicts = Common::maxclients.GetInt( ) + 1;

	// check edict size
	f->Read( &i, sizeof( i ) );
	if( i != sizeof( Entity ) ) {
		FileSystem::CloseFile( f );
		Server::PF_error( "ReadLevel: mismatched edict size" );
	}

	// check function pointer base address
	f->Read( &base, sizeof( base ) );

	// FIXME if( base !=( void * )InitGame )
	{
		FileSystem::CloseFile( f );
		Server::PF_error( "ReadLevel: function pointers have moved" );
	}




	// load the level locals
	ReadLevelLocals( f );

	// load all the entities
	while( 1 )
	{
		if( f->Read( &entnum, sizeof( entnum ) ) != 1 )
		{
			FileSystem::CloseFile( f );
			Server::PF_error( "ReadLevel: failed to read entnum" );
		}
		if( entnum == -1 )
			break;
		if( entnum >= num_edicts )
			num_edicts = entnum+1;

		ent = &g_edicts[ entnum ];
		ReadEdict( f, ent );

		// let the server rebuild world links for this ent
		ent->area.Clear( );
		Server::SV_LinkEdict( ent );
	}

	FileSystem::CloseFile( f );

	// mark all clients as unconnected
	for( i = 0; i < Common::maxclients.GetInt( ); i++ ) {

		ent = &g_edicts[ i+1 ];
		ent->client = clients + i;
		ent->client->pers.connected = false;
	}

	// do any load time things at this point
	for( i = 0; i<num_edicts; i++ )
	{
		ent = &g_edicts[ i ];

		if( !ent->inuse )
			continue;

		// fire any cross-level triggers
		if( ent->classname )
			if( strcmp( ent->classname, "target_crosslevel_target" ) == 0 )
				ent->nextthink = level.time + ent->delay;
	}
}
