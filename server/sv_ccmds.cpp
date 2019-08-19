#include "precompiled.h"
#pragma hdrstop

void Server::SV_SetMaster_f( ) {
	int		i, slot;
	// only dedicated servers send heartbeats
	if( !Common::dedicated.GetBool( ) ) {
		Common::Com_Printf( "Only dedicated servers use masters.\n" );
		return;
	}
	// make sure the server is listed public
	CVarSystem::SetCVarBool( "public", true );
	for( i = 1; i < MAX_MASTERS; i++ )
		master_adr[ i ].Clear( );
	slot = 1;		// slot 0 will always contain the  master
	for( i = 1; i < Command::Cmd_Argc( ); i++ ) {
		if( slot == MAX_MASTERS )
			break;
		if( !Network::NET_StringToAdr( Command::Cmd_Argv( i ), &master_adr[ i ] ) ) {
			Common::Com_Printf( "Bad address: %s\n", Command::Cmd_Argv( i ).c_str( ) );
			continue;
		}
		if( master_adr[ slot ].port == 0 )
			master_adr[ slot ].port = BigShort( PORT_MASTER );
		Common::Com_Printf( "Master server at %s\n", Network::NET_AdrToString( master_adr[ slot ] ).c_str( ) );
		Common::Com_Printf( "Sending a ping.\n" );
		MessageBufferT< MAX_MSGLEN >	messageBuffer;
		messageBuffer.Init( );
		messageBuffer.WriteShort( CONNECTIONLESS_MESSAGE_ID );
		messageBuffer.WriteString( "ping" );
		svs.d_serverPort.SendPacket( master_adr[ i ], messageBuffer );
		slot++;
	}
	svs.last_heartbeat = -9999999;
}

bool Server::SV_SetPlayer( ) {
	client_t	* cl;
	int			i;
	int			num;
	Str			s;
	if( Command::Cmd_Argc( ) < 2 )
		return false;
	s = Command::Cmd_Argv( 1 );
	// numeric values are just slot numbers
	if( s[ 0 ] >= '0' && s[ 0 ] <= '9' ) {
		num = atoi( Command::Cmd_Argv( 1 ) );
		if( num < 0 || num >= Common::maxclients.GetInt( ) ) {
			Common::Com_Printf( "Bad client slot: %i\n", num );
			return false;
		}
		sv_client = &svs.clients[ num ];
		sv_player = sv_client->edict;
		if( !sv_client->state ) {
			Common::Com_Printf( "Client %i is not active\n", num );
			return false;
		}
		return true;
	}
	// check for a name match
	for( i = 0, cl = svs.clients; i < Common::maxclients.GetInt( ); i++, cl++ ) {
		if( !cl->state )
			continue;
		if( cl->name == s ) {
			sv_client = cl;
			sv_player = sv_client->edict;
			return true;
		}
	}
	Common::Com_Printf( "Userid %s is not on the server\n", s.c_str( ) );
	return false;
}

void Server::SV_WipeSavegame( const Str & savename ) {
	Str	name;
	Str	s;
	Common::Com_DPrintf( "SV_WipeSaveGame( %s )\n", savename.c_str( ) );
	sprintf( name, "save/%s/server.ssv", savename.c_str( ) );
	remove( name );
	sprintf( name, "save/%s/game.ssv", savename.c_str( ) );
	remove( name );
	sprintf( name, "save/%s/*.sav", savename.c_str( ) );
	s = System::Sys_FindFirst( name, 0, 0 );
	while( s.Length( ) ) {
		remove( s );
		s = System::Sys_FindNext( 0, 0 );
	}
	System::Sys_FindClose( );
	sprintf( name, "save/%s/*.sv2", savename.c_str( ) );
	s = System::Sys_FindFirst( name, 0, 0 );
	while( s.Length( ) ) {
		remove( s );
		s = System::Sys_FindNext( 0, 0 );
	}
	System::Sys_FindClose( );
}

void Server::SV_CopySaveGame( const Str & src, const Str & dst ) {
	Str	name, name2;
	int		len;
	Str	found;
	Common::Com_DPrintf( "SV_CopySaveGame( %s, %s )\n", src.c_str( ), dst.c_str( ) );
	SV_WipeSavegame( dst );
	// copy the savegame over
	sprintf( name, "save/%s/server.ssv", src.c_str( ) );
	sprintf( name2, "save/%s/server.ssv", dst.c_str( ) );
	FileSystem::FileCopy( name, name2 );
	sprintf( name, "save/%s/game.ssv", src.c_str( ) );
	sprintf( name2, "save/%s/game.ssv", dst.c_str( ) );
	FileSystem::FileCopy( name, name2 );
	sprintf( name, "save/%s/", src.c_str( ) );
	len = name.Length( );
	sprintf( name, "save/%s/* .sav", src.c_str( ) );
	found = System::Sys_FindFirst( name, 0, 0 );
	// FIXME
	/* while( found )
	{
		strcpy( name+len, found+len );

		sprintf( name2, "save/%s/%s", dst, found+len );
		CopyFile( name, name2 );

		// change sav to sv2
		l = strlen( name );
		strcpy( name+l-3, "sv2" );
		l = strlen( name2 );
		strcpy( name2+l-3, "sv2" );
		CopyFile( name, name2 );

		found = Sys_FindNext( 0, 0 );
	}*/
	System::Sys_FindClose( );
}

void Server::SV_WriteLevelFile( ) {
	Str		name;
	FileBase *	f;
	Common::Com_DPrintf( "SV_WriteLevelFile( )\n " );
	sprintf( name, "save/current/%s.sv2", sv.name.c_str( ) );
	f = FileSystem::OpenFileByMode( name, FS_WRITE );
	f->Write( sv.configstrings, sizeof( sv.configstrings ) ); // FIXME NOW
	CollisionModel::CM_WritePortalState( f );
	FileSystem::CloseFile( f );
	sprintf( name, "save/current/%s.sav", sv.name.c_str( ) );
	Game::WriteLevel( name );
}

void Server::SV_ReadLevelFile( ) {
	Str	name;
	Common::Com_DPrintf( "SV_ReadLevelFile( )\n" );
	sprintf( name, "save/current/%s.sv2", sv.name.c_str( ) );
	FileBase * f = FileSystem::OpenFileRead( name );
	f->Read( sv.configstrings, sizeof( sv.configstrings ) ); // FIXME NOW
	CollisionModel::CM_ReadPortalState( f );
	FileSystem::CloseFile( f );
	sprintf( name, "save/current/%s.sav", sv.name.c_str( ) );
	Game::ReadLevel( name );
}

void Server::SV_WriteServerFile( bool autosave ) {
	Str	string;
	Str	comment;
	time_t	aclock;
	struct tm * newtime;
	Common::Com_DPrintf( "SV_WriteServerFile( %s )\n", autosave ? "true" : "false" );
	FileBase * f = FileSystem::OpenFileByMode( "save/current/server.ssv", FS_WRITE );
	// write the comment field
	if( !f )
		return;
	if( !autosave ) {
		time( &aclock );
		newtime = localtime( &aclock );
		sprintf( comment, "%2i:%i%i %2i/%2i  ", newtime->tm_hour, newtime->tm_min/10, newtime->tm_min%10, newtime->tm_mon+1, newtime->tm_mday );
		comment.Append( sv.configstrings[ CS_NAME ] );
	} else
		sprintf( comment, "ENTERING %s", sv.configstrings[ CS_NAME ].c_str( ) ); // autosaved
	f->Write( comment, sizeof( comment ) );
	// write the mapcmd
	f->Write( svs.mapcmd, sizeof( svs.mapcmd ) );
	// write all CVAR_LATCH cvars
	// these will be things like coop, skill, deathmatch, etc
	/*for( var = cvar_vars; var; var = var->next ) {

		if( !( var->flags & CVAR_LATCH ) ) continue;
		 Common::Com_Memset( name, 0, sizeof( name ) );
		Common::Com_Memset( string, 0, sizeof( string ) );
		strcpy( name, var->name );
		strcpy( string, var->string );
		fwrite( name, 1, sizeof( name ), f );
		fwrite( string, 1, sizeof( string ), f );
	}*/ // FIXME NOW
	FileSystem::CloseFile( f );
	// write game state
	Game::WriteGame( "save/current/game.ssv", autosave );
}

void Server::SV_ReadServerFile( ) {
	Str	string;
	Str	comment;
	Str	mapcmd;
	Common::Com_DPrintf( "SV_ReadServerFile( )\n" );
	FileBase * f = FileSystem::OpenFileByMode( "save/current/server.ssv", FS_READ );
	// read the comment field
	//FS_Read( comment, sizeof( comment ), f ); // FIXME NOW
	// read the mapcmd
	//FS_Read( mapcmd, sizeof( mapcmd ), f ); // FIXME NOW
	// read all CVAR_LATCH cvars
	// these will be things like coop, skill, deathmatch, etc
	while( 1 ) {
		//if( !fread( name, 1, sizeof( name ), f ) ) break; // FIXME NOW
		//FS_Read( string, sizeof( string ), f ); // FIXME NOW
		//Common::Com_DPrintf( "Set %s = %s\n", name, string );
		//Cvar_ForceSet( name, string );
	}
	FileSystem::CloseFile( f );
	// start a new game fresh with new cvars
	SV_InitGame( );
	svs.mapcmd = mapcmd;
	// read game state
	Game::ReadGame( "save/current/game.ssv" );
}

void Server::SV_DemoMap_f( ) {
	SV_Map( true, Command::Cmd_Argv( 1 ), false );
}

void Server::SV_GameMap_f( ) {
	Str		map;
	int			i;
	client_t	* cl;
	bool * savedInuse;
	if( Command::Cmd_Argc( ) != 2 ) {
		Common::Com_Printf( "USAGE: gamemap <map>\n" );
		return;
	}
	Common::Com_DPrintf( "SV_GameMap( %s )\n", Command::Cmd_Argv( 1 ).c_str( ) );
	// check for clearing the current savegame
	map = Command::Cmd_Argv( 1 );
	if( map[ 0 ] == '*' ) {
		// wipe all the * .sav files
		SV_WipeSavegame( "current" );
	} else {	// save the map just exited
		if( sv.state == ss_game ) {
			// clear all the client inuse flags before saving so that
			// when the level is re-entered, the clients will spawn
			// at spawn points instead of occupying body shells
			savedInuse =( bool * )malloc( Common::maxclients.GetInt( ) * sizeof( bool ) );
			for( i = 0, cl = svs.clients; i < Common::maxclients.GetInt( ); i++, cl++ ) {
				savedInuse[ i ] = cl->edict->inuse;
				cl->edict->inuse = false;
			}
			SV_WriteLevelFile( );
			// we must restore these for clients to transfer over correctly
			for( i = 0, cl = svs.clients; i < Common::maxclients.GetInt( ); i++, cl++ )
				cl->edict->inuse = savedInuse[ i ];
			free( savedInuse );
		}
	}
	// start up the next map
	SV_Map( false, Command::Cmd_Argv( 1 ), false );
	// archive server state
	svs.mapcmd = Command::Cmd_Argv( 1 );
	// copy off the level to the autosave slot
	if( !Common::dedicated.GetBool( ) ) {
		SV_WriteServerFile( true );
		SV_CopySaveGame( Str( "current" ), Str( "save0" ) );
	}
}

void Server::SV_Map_f( ) {
	Str	map;
	Str	expanded;
	// if not a pcx, demo, or cinematic, check to make sure the level exists
	map = Command::Cmd_Argv( 1 );
	if( !strstr( map, "." ) ) {
		sprintf( expanded, "maps/%s.map", map.c_str( ) );
		if( !FileSystem::TouchFile( expanded ) ) {
			Common::Com_Printf( "Can't find %s\n", expanded.c_str( ) );
			return;
		}
	}
	sv.state = ss_dead;		// don't save current level when changing
	SV_WipeSavegame( Str( "current" ) );
	SV_GameMap_f( );
}

void Server::SV_Loadgame_f( ) {
	Str	name;
	Str	dir;
	if( Command::Cmd_Argc( ) != 2 ) {
		Common::Com_Printf( "USAGE: loadgame <directory>\n" );
		return;
	}
	Common::Com_Printf( "Loading game...\n" );
	dir = Command::Cmd_Argv( 1 );
	if( strstr( dir, ".." ) || strstr( dir, "/" ) || strstr( dir, "\\" ) )
		Common::Com_Printf( "Bad savedir.\n" );
	// make sure the server.ssv file exists
	sprintf( name, "save/%s/server.ssv", Command::Cmd_Argv( 1 ).c_str( ) );
	FileBase * f = FileSystem::OpenFileRead( name );
	FileSystem::CloseFile( f );
	SV_CopySaveGame( Command::Cmd_Argv( 1 ), Str( "current" ) );
	SV_ReadServerFile( );
	// go to the map
	sv.state = ss_dead;		// don't save current level when changing
	SV_Map( false, svs.mapcmd, true );
}

void Server::SV_Savegame_f( ) {
	Str	dir;
	if( sv.state != ss_game ) {
		Common::Com_Printf( "You must be in a game to save.\n" );
		return;
	}
	if( Command::Cmd_Argc( ) != 2 ) {
		Common::Com_Printf( "USAGE: savegame <directory>\n" );
		return;
	}
	if( CVarSystem::GetCVarBool( "deathmatch" ) ) {
		Common::Com_Printf( "Can't savegame in a deathmatch\n" );
		return;
	}
	if( Command::Cmd_Argv( 1 ) == "current" ) {
		Common::Com_Printf( "Can't save to 'current'\n" );
		return;
	}
	if( Common::maxclients.GetInt( ) == 1 && svs.clients[ 0 ].edict->client->ps.stats[ STAT_HEALTH ] <= 0 ) {
		Common::Com_Printf( "\nCan't savegame while dead!\n" );
		return;
	}
	dir = Command::Cmd_Argv( 1 );
	if( strstr( dir, ".." ) || strstr( dir, "/" ) || strstr( dir, "\\" ) ) Common::Com_Printf( "Bad savedir.\n" );
	Common::Com_Printf( "Saving game...\n" );
	// archive current level, including all client edicts.
	// when the level is reloaded, they will be shells awaiting
	// a connecting client
	SV_WriteLevelFile( );
	// save server state
	SV_WriteServerFile( false );
	// copy it off
	SV_CopySaveGame( Str( "current" ), dir );
	Common::Com_Printf( "Done.\n" );
}

void Server::SV_Kick_f( ) {
	if( !svs.initialized ) {
		Common::Com_Printf( "No server running.\n" );
		return;
	}
	if( Command::Cmd_Argc( ) != 2 ) {
		Common::Com_Printf( "Usage: kick <userid>\n" );
		return;
	}
	if( !SV_SetPlayer( ) )
		return;
	SV_BroadcastPrintf( PRINT_HIGH, "%s was kicked\n", sv_client->name.c_str( ) );
	// print directly, because the dropped client won't get the
	// SV_BroadcastPrintf message
	SV_ClientPrintf( sv_client, PRINT_HIGH, "You were kicked from the game\n" );
	SV_DropClient( sv_client );
	sv_client->lastmessage = svs.realtime;	// min case there is a funny zombie
}

void Server::SV_Status_f( ) {
	int			i, j, l;
	client_t	* cl;
	timeType		ping;
	if( !svs.clients ) {
		Common::Com_Printf( "No server running.\n" );
		return;
	}
	Str build, redir;
	sprintf( build, "map: \"%s\"\n", sv.name.c_str( ) );
	redir += build;
	sprintf( build, "num score ping name            lastmsg address               irate orate\n" );
	redir += build;
	sprintf( build, "--- ----- ---- --------------- ------- --------------------- ----- -----\n" );
	redir += build;
	for( i = 0, cl = svs.clients; i < Common::maxclients.GetInt( ); i++, cl++ ) {
		if( !cl->state )
			continue;
		sprintf( build, "%3i ", i );
		redir += build;
		sprintf( build, "%5i ", cl->edict->client->ps.stats[ STAT_FRAGS ] );
		redir += build;
		if( cl->state == cs_connected )
			redir += "CNCT ";
		else if( cl->state == cs_zombie )
			redir += "ZMBI ";
		else {
			ping = cl->ping < 9999 ? cl->ping : 9999;
			sprintf( build, "%4i ", ( int )ping );
			redir += build;
		}
		sprintf( build, "%s", cl->name.c_str( ) );
		redir += build;
		l = 16 - cl->name.Length( );
		for( j = 0; j < l; j++ )
			redir += " ";
		sprintf( build, "%7i ", cl->d_clientChannel.GetLastIncomingTime( ) );
		redir += build;
		Str s = Network::NET_AdrToString( cl->d_clientChannel.GetRemoteAddress( ) );
		sprintf( build, "%s", s.c_str( ) );
		redir += build;
		l = 22 - s.Length( );
		for( j = 0; j < l; j++ )
			redir += " ";
		sprintf( build, "%5i ", cl->d_clientChannel.GetIncomingRate( ) );
		redir += build;
		sprintf( build, "%5i", cl->d_clientChannel.GetOutgoingRate( ) );
		redir += build;
		redir += "\n";
	}
	redir += "\n";
	Common::Com_Printf( redir );
}

void Server::SV_ConSay_f( ) {
	Str		text;
	if( Command::Cmd_Argc( ) < 2 )
		return;
	text = "con: ";
	text.Append( Command::Cmd_Args( ) );
	for( int j = 0; j < Common::maxclients.GetInt( ); j++ ) {
		if( svs.clients[ j ].state != cs_spawned )
			continue;
		SV_ClientPrintf( &svs.clients[ j ], PRINT_CHAT, "%s\n", text.c_str( ) );
	}
}

void Server::SV_Heartbeat_f( ) {
	svs.last_heartbeat = -9999999;
}

void Server::SV_Serverinfo_f( ) {
	Common::Com_Printf( "Server info settings:\n" );
	ClientServerInfo::Info_Print( CVarSystem::Serverinfo( ) );
}

void Server::SV_DumpUser_f( ) {
	if( Command::Cmd_Argc( ) != 2 ) {
		Common::Com_Printf( "Usage: info <userid>\n" );
		return;
	}
	if( !SV_SetPlayer( ) )
		return;
	Common::Com_Printf( "userinfo\n" );
	Common::Com_Printf( "--------\n" );
	ClientServerInfo::Info_Print( sv_client->userinfo );
}

void Server::SV_ServerRecord_f( ) {
	Str	name;
	MessageBufferT< 32768 >	msgBuffer;
	int		len;
	if( Command::Cmd_Argc( ) != 2 ) {
		Common::Com_Printf( "serverrecord <demoname>\n" );
		return;
	}
	if( svs.demofile ) {
		Common::Com_Printf( "Already recording.\n" );
		return;
	}
	if( sv.state != ss_game ) {
		Common::Com_Printf( "You must be in a level to record.\n" );
		return;
	}
	// open the demo file
	sprintf( name, "demos/%s.dm2", Command::Cmd_Argv( 1 ).c_str( ) );
	Common::Com_Printf( "recording to %s.\n", name.c_str( ) );
	svs.demofile = FileSystem::OpenFileByMode( name, FS_WRITE );
	// setup a buffer to catch all multicasts
	//Message::SZ_Init( &svs.demo_multicast, svs.demo_multicast_buf, sizeof( svs.demo_multicast_buf ) );
	svs.d_demoBuffer.Init( );
	// write a single giant fake message with all the startup info
	msgBuffer.Init( );
	// serverdata needs to go over for all types of servers
	// to make sure the protocol is right, and to set the gamedir
	// send the serverdata
	msgBuffer.WriteByte( svc_serverdata );
	msgBuffer.WriteByte( PROTOCOL_VERSION );
	msgBuffer.WriteLong( svs.spawncount );
	// 2 means server demo
	msgBuffer.WriteByte( 2 );	// demos are always attract loops
	msgBuffer.WriteString( CVarSystem::GetCVarStr( "fs_gamedir" ) );
	msgBuffer.WriteShort( -1 );
	// send full levelname
	msgBuffer.WriteString( sv.configstrings[ CS_NAME ] );
	for( int i = 0; i < MAX_CONFIGSTRINGS; i++ ) {
		if( sv.configstrings[ i ].Length( ) ) {
			msgBuffer.WriteByte( svc_configstring );
			msgBuffer.WriteShort( i );
			msgBuffer.WriteString( sv.configstrings[ i ] );
		}
	}
	// write it to the demo file
	Common::Com_DPrintf( "signon message length: %i\n", msgBuffer.GetSize( ) );
	len = LittleLong( msgBuffer.GetSize( ) );
	svs.demofile->Write( &len, 4 );
	svs.demofile->Write( msgBuffer.GetData( ), msgBuffer.GetSize( ) );
	// the rest of the demo file will be individual frames
}

void Server::SV_ServerStop_f( ) {
	if( !svs.demofile ) {
		Common::Com_Printf( "Not doing a serverrecord.\n" );
		return;
	}
	FileSystem::CloseFile( svs.demofile );
	Common::Com_Printf( "Recording completed.\n" );
}

void Server::SV_KillServer_f( ) {
	if( !svs.initialized )
		return;
	SV_Shutdown( "Server was killed.\n", false );
	//Network::NET_Config( false );	// close network sockets
}

void Server::SV_ServerCommand_f( ) {
	Game::ServerCommand( );
}

void Server::SV_InitOperatorCommands( ) {
	Command::Cmd_AddCommand( "heartbeat", SV_Heartbeat_f );
	Command::Cmd_AddCommand( "kick", SV_Kick_f );
	Command::Cmd_AddCommand( "status", SV_Status_f );
	Command::Cmd_AddCommand( "serverinfo", SV_Serverinfo_f );
	Command::Cmd_AddCommand( "dumpuser", SV_DumpUser_f );
	Command::Cmd_AddCommand( "map", SV_Map_f );
	Command::Cmd_AddCommand( "demomap", SV_DemoMap_f );
	Command::Cmd_AddCommand( "gamemap", SV_GameMap_f );
	Command::Cmd_AddCommand( "setmaster", SV_SetMaster_f );
	if( Common::dedicated.GetBool( ) )
		Command::Cmd_AddCommand( "say", SV_ConSay_f );
	Command::Cmd_AddCommand( "serverrecord", SV_ServerRecord_f );
	Command::Cmd_AddCommand( "serverstop", SV_ServerStop_f );
	Command::Cmd_AddCommand( "save", SV_Savegame_f );
	Command::Cmd_AddCommand( "load", SV_Loadgame_f );
	Command::Cmd_AddCommand( "killserver", SV_KillServer_f );
	Command::Cmd_AddCommand( "sv", SV_ServerCommand_f );
	LuaScript::Register( "Map", lg_Map );
	LuaScript::Register( "GameMap", lg_GameMap );
}

#define SV_ENTRY_CHECK()
#define SV_LINE_CHECK()
#define	SV_RETURN_CHECK(x)	return(x);

int Server::lg_Map( LuaState & state ) {
	SV_ENTRY_CHECK();
	// if not a pcx, demo, or cinematic, check to make sure the level exists
	Str map = state.GetString( 1 );
	if( !strstr( map, "." ) ) {
		Str expanded;
		sprintf( expanded, "maps/%s.map", map.c_str( ) );
		if( !FileSystem::TouchFile( expanded ) ) {
			Common::Com_Printf( "Can't find %s\n", expanded.c_str( ) );
			SV_RETURN_CHECK(0);
		}
	}
	sv.state = ss_dead;		// don't save current level when changing
	SV_WipeSavegame( "current" );
	//state.PushClone( 1 );
	LuaScript::ExecuteInternal( lg_GameMap, 1, 0 );
	SV_RETURN_CHECK(0);
}

int Server::lg_GameMap( LuaState & state ) {
	SV_ENTRY_CHECK();
	if( !state.IsString( 1 ) ) SV_RETURN_CHECK(0);
	Str map = state.GetString( 1 );
	// check for clearing the current savegame
	if( map[ 0 ] == '*' ) {
		// wipe all the * .sav files
		SV_WipeSavegame( "current" );
	} else {	// save the map just exited
		if( sv.state == ss_game ) {
			bool *			savedInuse = new bool[ Common::maxclients.GetInt( ) ];
			int				i;
			client_t *		cl;
			// clear all the client inuse flags before saving so that
			// when the level is re-entered, the clients will spawn
			// at spawn points instead of occupying body shells
			for( i = 0, cl = svs.clients; i < Common::maxclients.GetInt( ); i++, cl++ ) {
				savedInuse[ i ] = cl->edict->inuse;
				cl->edict->inuse = false;
			}
			SV_WriteLevelFile( );
			// we must restore these for clients to transfer over correctly
			for( i = 0, cl = svs.clients; i < Common::maxclients.GetInt( ); i++, cl++ )
				cl->edict->inuse = savedInuse[ i ];
			delete[] savedInuse;
		}
	}
	// start up the next map
	SV_Map( false, map, false );
	// archive server state
	svs.mapcmd = map;
	// copy off the level to the autosave slot
	if( !Common::dedicated.GetBool( ) ) {
		SV_WriteServerFile( true );
		SV_CopySaveGame( "current", "save0" );
	}
	SV_RETURN_CHECK(0);
}
