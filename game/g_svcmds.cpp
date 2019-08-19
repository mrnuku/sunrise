#include "precompiled.h"
#pragma hdrstop

void Game::Svcmd_Test_f( ) {

	Server::PF_cprintf( NULL, PRINT_HIGH, "Svcmd_Test_f( )\n" );
}

/*
==============================================================================

PACKET FILTERING


You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and any unspecified digits will match any value, so you can specify an entire class C network with "addip 192.246.40".
Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip

Prints the current list of filters.

writeip

Dumps "addip <ip>" commands to listip.cfg so it can be execed at a later date.  The filter lists are not saved and restored by default, because I beleive it would cause too much confusion.

filterban <0 or 1>

If 1( the default ), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.

==============================================================================
*/
typedef struct ipfilter_s {

	unsigned mask;
	unsigned compare;
} ipfilter_t;



ipfilter_t ipfilters[ 1024 ];
int numipfilters;

bool Game::StringToFilter( const Str & s, ipfilter_t * f ) {
	char num[ 128 ];
	int i, j, k = 0;
	byte b[ 4 ];
	byte m[ 4 ];
	for( i = 0; i<4; i++ ) {
		b[ i ] = 0;
		m[ i ] = 0;
	}
	for( i = 0; i < 4; i++ ) {
		if( s[ k ] < '0' || s[ k ] > '9' ) {
			Server::PF_cprintf( NULL, PRINT_HIGH, "Bad filter address: %s\n", s.c_str( ) );
			return false;
		}
		j = 0;
		while( *s >= '0' && * s <= '9' )
			num[ j++ ] = s[ k++ ];
		num[ j ] = 0;
		b[ i ] = atoi( num );
		if( b[ i ] != 0 ) m[ i ] = 255;

		if( !s[ k ] ) break;
		k++;
	}
	f->mask = * ( unsigned * )m;
	f->compare = * ( unsigned * )b;
	return true;
}

bool Game::SV_FilterPacket( const Str & from ) {
	int i;
	unsigned in;
	byte m[ 4 ];
	const char * p;
	i = 0;
	p = from;
	while( *p && i < 4 ) {
		m[ i ] = 0;
		while( *p >= '0' && * p <= '9' ) {
			m[ i ] = m[ i ]* 10 +( *p - '0' );
			p++;
		}
		if( !* p || * p == ':' )
			break;
		i++, p++;
	}
	in = * ( unsigned * )m;
	for( i = 0; i<numipfilters; i++ )
		if( ( in & ipfilters[ i ].mask ) == ipfilters[ i ].compare )
			return filterban.GetBool( );
	return !filterban.GetBool( );
}

void Game::SVCmd_AddIP_f( ) {
	int i;
	if( Command::Cmd_Argc( ) < 3 ) {
		Server::PF_cprintf( NULL, PRINT_HIGH, "Usage:  addip <ip-mask>\n" );
		return;
	}
	for( i = 0; i<numipfilters; i++ )
		if( ipfilters[ i ].compare == 0xFFffffff )
			break; // free spot
	if( i == numipfilters ) {
		if( numipfilters == 1024 ) {
			Server::PF_cprintf( NULL, PRINT_HIGH, "IP filter list is full\n" );
			return;
		}
		numipfilters++;
	}
	if( !StringToFilter( Command::Cmd_Argv( 2 ), &ipfilters[ i ] ) )
		ipfilters[ i ].compare = 0xFFFFFFFF;
}

void Game::SVCmd_RemoveIP_f( ) {
	ipfilter_t f;
	int i, j;
	if( Command::Cmd_Argc( ) < 3 ) {
		Server::PF_cprintf( NULL, PRINT_HIGH, "Usage:  sv removeip <ip-mask>\n" );
		return;
	}
	if( !StringToFilter( Command::Cmd_Argv( 2 ), &f ) )
		return;
	for( i = 0; i<numipfilters; i++ ) {
		if( ipfilters[ i ].mask == f.mask && ipfilters[ i ].compare == f.compare ) {
			for( j = i+1; j<numipfilters; j++ )
				ipfilters[ j-1 ] = ipfilters[ j ];
			numipfilters--;
			Server::PF_cprintf( NULL, PRINT_HIGH, "Removed.\n" );
			return;
		}
		Server::PF_cprintf( NULL, PRINT_HIGH, "Didn't find %s.\n", Command::Cmd_Argv( 2 ).c_str( ) );
	}
}

void Game::SVCmd_ListIP_f( ) {
	int i;
	byte b[ 4 ];
	Server::PF_cprintf( NULL, PRINT_HIGH, "Filter list:\n" );
	for( i = 0; i<numipfilters; i++ ) {
		* ( unsigned * )b = ipfilters[ i ].compare;
		Server::PF_cprintf( NULL, PRINT_HIGH, "%3i.%3i.%3i.%3i\n", b[ 0 ], b[ 1 ], b[ 2 ], b[ 3 ] );
	}
}

void Game::SVCmd_WriteIP_f( ) {
	Str name = "listip.cfg";
	byte b[ 4 ];
	int i;
	Server::PF_cprintf( NULL, PRINT_HIGH, "Writing %s.\n", name.c_str( ) );
	FileBase * f = FileSystem::OpenFileByMode( name, FS_WRITE );
	f->Printf( "set filterban %s\n", filterban.GetString( ).c_str( ) );
	for( i = 0; i<numipfilters; i++ ) {
		*( unsigned * )b = ipfilters[ i ].compare;
		f->Printf( "sv addip %i.%i.%i.%i\n", b[ 0 ], b[ 1 ], b[ 2 ], b[ 3 ] );
	}
	FileSystem::CloseFile( f );
}

void Game::ServerCommand( ) {
	Str cmd;
	cmd = Command::Cmd_Argv( 1 );
	if( cmd == "test" )
		Svcmd_Test_f( );
	else if( cmd == "addip" )
		SVCmd_AddIP_f( );
	else if( cmd == "removeip" )
		SVCmd_RemoveIP_f( );
	else if( cmd == "listip" )
		SVCmd_ListIP_f( );
	else if( cmd == "writeip" )
		SVCmd_WriteIP_f( );
	else
		Server::PF_cprintf( NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd.c_str( ) );
}
