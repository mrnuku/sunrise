#include "precompiled.h"
#pragma hdrstop

void ServerLocalData::Clear( ) {
	state			= ss_dead;
	attractloop		= false;
	loadgame		= false;
	timedemo		= false;
	time			= 0;
	framenum		= 0;
	demofile		= NULL;
	name.Clear( );
	d_multicastBuffer.Init( );
	d_config.Clear( );
	for( int i = 0; i < MAX_MODELS; i++ )
		models[ i ] = NULL;
	for( int i = 0; i < MAX_CONFIGSTRINGS; i++ )
		configstrings[ i ].Clear( );
	for( int i = 0; i < MAX_EDICTS; i++ )
		baselines[ i ];
}

void ServerPersistantData::Clear( ) {
	assert( !d_serverPort.GetPort( ) );
	initialized		= false;
	realtime		= 0;
	mapcmd.Clear( );
	spawncount		= 0;
	clients			= NULL;
	num_client_entities	= 0;
	next_client_entities	= 0;
	client_entities	= NULL;
	last_heartbeat	= 0;
	for( int i = 0; i < MAX_CHALLENGES; i++ )
		challenges[ i ].Clear( );
	demofile		= false;
	d_demoBuffer.Init( );
}

void client_frame_t::Clear( ) {
	areabytes		= 0;
	Common::Com_Memset( areabits, 0, sizeof( areabits ) );
	ps.Clear( );
	num_entities	= 0;
	first_entity	= 0;
	senttime		= 0;
}

void client_t::Clear( ) {
	state			= cs_free;
	userinfo.Clear( );
	lastframe		= 0;
	lastcmd.Clear( );
	commandMsec		= 0;
	for( int i = 0; i < LATENCY_COUNTS; i++ )
		frame_latency[ i ] = 0;
	ping			= 0;
	surpressCount	= 0;
	edict			= NULL;
	name.Clear( );
	messagelevel	= 0;
	for( int i = 0; i < UPDATE_BACKUP; i++ )
		frames[ i ].Clear( );
	assert( !download.IsValid( ) );
	downloadsize	= 0;
	downloadcount	= 0;
	lastmessage		= 0;
	lastconnect		= 0;
	challenge		= 0;
	d_clientChannel.Shutdown( );
	d_clientBuffer.Init( );
	d_clientBufferReliable.Init( );
}

Str Server::SV_GetConfigName( int index ) {
	switch( index ) {
		case CS_NAME:
			return "CS_NAME";
		case CS_SKY:
			return "CS_SKY";
		case CS_SKYAXIS:
			return "CS_SKYAXIS";
		case CS_SKYROTATE:
			return "CS_SKYROTATE";
		case CS_STATUSBAR:
			return "CS_STATUSBAR";
		case CS_AIRACCEL:
			return "CS_AIRACCEL";
		case CS_MAXCLIENTS:
			return "CS_MAXCLIENTS";
		case CS_MAPCHECKSUM:
			return "CS_MAPCHECKSUM";
		case MAX_CONFIGSTRINGS:
			return "MAX_CONFIGSTRINGS";
	}
	assert( index < MAX_CONFIGSTRINGS );
	if( index >= CS_GENERAL )
		return Str( "CS_GENERAL" ) + ( CS_GENERAL - index );
	if( index >= CS_PLAYERSKINS )
		return Str( "CS_PLAYERSKINS" ) + ( CS_PLAYERSKINS - index );
	if( index >= CS_ITEMS )
		return Str( "CS_ITEMS" ) + ( CS_ITEMS - index );
	if( index >= CS_LIGHTS )
		return Str( "CS_LIGHTS" ) + ( CS_LIGHTS - index );
	if( index >= CS_IMAGES )
		return Str( "CS_IMAGES" ) + ( CS_IMAGES - index );
	if( index >= CS_SOUNDS )
		return Str( "CS_SOUNDS" ) + ( CS_SOUNDS - index );
	if( index >= CS_MODELS )
		return Str( "CS_MODELS" ) + ( CS_MODELS - index );
	assert( 0 );
	return "";
}

int Server::SV_FindIndex( const Str & name, int start, int max, bool create ) {
	int		i;	
	if( name.IsEmpty( ) )
		return 0;
	for( i = 1; i < max; i++ ) {
		if( sv.configstrings[ start + i ].IsEmpty( ) )
			break;
		if( sv.configstrings[ start + i ] == name )
			return i;
	}
	if( !create )
		return 0;
	if( i == max )
		Common::Com_Error( ERR_DROP, "*Index: overflow" );
	sv.configstrings[ start + i ] = name;
	if( sv.state != ss_loading ) {	// send the update to everyone
		assert( !sv.d_multicastBuffer.GetSize( ) );
		sv.d_multicastBuffer.WriteChar( svc_configstring );
		sv.d_multicastBuffer.WriteShort( start + i );
		sv.d_multicastBuffer.WriteString( name );
		SV_Multicast( vec3_origin, MULTICAST_ALL_R );
	}
	return i;
}

int Server::SV_ModelIndex( const Str & name ) {
	return SV_FindIndex( name, CS_MODELS, MAX_MODELS, true );
}

int Server::SV_SoundIndex( const Str & name ) {
	return SV_FindIndex( name, CS_SOUNDS, MAX_SOUNDS, true );
}

int Server::SV_ImageIndex( const Str & name ) {
	return SV_FindIndex( name, CS_IMAGES, MAX_IMAGES, true );
}

void Server::SV_CreateBaseline( ) {
	Entity			* svent;
	int				entnum;	
	for( entnum = 1; entnum < Game::num_edicts; entnum++ ) {
		svent = EDICT_NUM( entnum );
		if( !svent->inuse )
			continue;
		if( !svent->s.modelindex && !svent->s.sound && !svent->s.effects )
			continue;
		svent->s.number = entnum;
		// take current state as baseline
		svent->s.old_origin = svent->s.origin;
		sv.baselines[ entnum ] = svent->s;
	}
}

void Server::SV_CheckForSavegame( ) {
	Str		name;
	if( sv_noreload.GetBool( ) )
		return;
	if( CVarSystem::GetCVarBool( "deathmatch" ) )
		return;
	sprintf( name, "save/current/%s.sav", sv.name.c_str( ) );

	if( !FileSystem::TouchFile( name ) )
		return;		// no savegame
	SV_ClearWorld( );
	// get configstrings and areaportals
	SV_ReadLevelFile( );
	if( !sv.loadgame ) {
		// coming back to a level after being in a different
		// level, so run it for ten seconds

		// rlava2 was sending too many lightstyles, and overflowing the
		// reliable data. temporarily changing the server state to loading
		// prevents these from being passed down.
		server_state_t		previousState;		// PGM
		previousState = sv.state;				// PGM
		sv.state = ss_loading;					// PGM
		for( int i = 0; i < 100; i++ )
			Game::RunFrame( );
		sv.state = previousState;				// PGM
	}
}

void Server::SV_SpawnServer( const Str & server, Str & spawnpoint, server_state_t serverstate, bool attractloop, bool loadgame ) {
	int			i;
	unsigned	checksum;
	if( attractloop )
		Common::paused.SetBool( false );
	Common::Com_Printf( "------- Server Initialization -------\n" );
	Common::Com_DPrintf( "SpawnServer: %s\n", server.c_str( ) );
	if( sv.demofile )
		FileSystem::CloseFile( sv.demofile );
	svs.spawncount++;		// any partially connected client will be
							// restarted
	sv.state = ss_dead;
	Common::Com_SetServerState( sv.state );
	// wipe the entire per-level structure
	sv.Clear( );
	svs.realtime = 0;
	sv.loadgame = loadgame;
	sv.attractloop = attractloop;
	// save name for levels that don't set message
	sv.configstrings[ CS_NAME ] = server;
	sv.d_config.Set( SV_GetConfigName( CS_NAME ), server );
	if( CVarSystem::GetCVarBool( "deathmatch" ) ) {
		sv.configstrings[ CS_AIRACCEL ] = sv_airaccelerate.GetString( );
		sv.d_config.SetFloat( SV_GetConfigName( CS_AIRACCEL ), sv_airaccelerate.GetFloat( ) );
		pm_airaccelerate = sv_airaccelerate.GetFloat( );
	} else {
		sv.configstrings[ CS_AIRACCEL ] = "0";
		sv.d_config.SetFloat( SV_GetConfigName( CS_AIRACCEL ), 0 );
		pm_airaccelerate = 0;
	}
	//Message::SZ_Init( &sv.multicast, sv.multicast_buf, sizeof( sv.multicast_buf ) );
	sv.d_multicastBuffer.Init( );
	sv.name = server;
	// leave slots at start for clients only
	for( i = 0; i < Common::maxclients.GetInt( ); i++ ) {
		// needs to reconnect
		if( svs.clients[ i ].state > cs_connected )
			svs.clients[ i ].state = cs_connected;
		svs.clients[ i ].lastframe = -1;
	}
	sv.time = 1000;	
	sv.name = server;
	sv.configstrings[ CS_NAME ] = server;
	sv.d_config.Set( SV_GetConfigName( CS_NAME ), server );
	if( serverstate != ss_game )
		sv.models[ 1 ] = CollisionModel::CM_LoadMap( "", false, checksum );	// no real map
	else {
		sprintf( sv.configstrings[ CS_MODELS + 1 ], "maps/%s.map", server.c_str( ) );
		sv.d_config.Set( SV_GetConfigName( CS_MODELS + 1 ), va( "maps/%s.map", server.c_str( ) ) );
		sv.models[ 1 ] = CollisionModel::CM_LoadMap( sv.configstrings[ CS_MODELS + 1 ], false, checksum );
	}
	sprintf( sv.configstrings[ CS_MAPCHECKSUM ], "%i", checksum );
	sv.d_config.SetInt( SV_GetConfigName( CS_MAPCHECKSUM ), checksum );
	// clear physics interaction links
	SV_ClearWorld( );	
#if 0
	for( i = 1; i < CollisionModel::CM_NumInlineModels( ); i++ ){

		sprintf( sv.configstrings[ CS_MODELS + 1 + i ], "*%i", i );
		sv.models[ i + 1 ] = CollisionModel::CM_InlineModel( sv.configstrings[ CS_MODELS + 1 + i ] );
	}
#endif
	// spawn the rest of the entities on the map
	// precache and static commands can be issued during
	// map initialization
	sv.state = ss_loading;
	Common::Com_SetServerState( sv.state );
	// load and spawn all other entities
	Game::SpawnEntities( sv.name, spawnpoint );
	// run two frames to allow everything to settle
	Game::RunFrame( );
	Game::RunFrame( );
	// all precaches are complete
	sv.state = serverstate;
	Common::Com_SetServerState( sv.state );	
	// create a baseline for more efficient communications
	SV_CreateBaseline( );
	// check for a savegame
	SV_CheckForSavegame( );
	// set serverinfo variable
	sv_mapname.SetString( sv.name );
	Common::Com_Printf( "-------------------------------------\n" );
}

void Server::SV_InitGame( ) {
	Str	master;
	if( svs.initialized ) {
		// cause any connected clients to reconnect
		SV_Shutdown( "Server restarted\n", true );
	}
#ifndef DEDICATED_ONLY
	else if( !Common::dedicated.GetBool( ) ) {
		// make sure the client is down
		Client::CL_Drop( );
		Screen::SCR_BeginLoadingPlaque( );
	}
#endif
	//Program::FreeData( );
	FileSystem::Restart( );
	// get any latched variable changes( maxclients, etc )
	CVarSystem::GetLatchedVars( );
	svs.initialized = true;
	if( CVarSystem::GetCVarBool( "coop" ) && CVarSystem::GetCVarBool( "deathmatch" ) ) {
		Common::Com_Printf( "Deathmatch and Coop both set, disabling Coop\n" );
		CVarSystem::SetCVarBool( "coop", false );
	}
	// dedicated servers are can't be single player and are usually DM
	// so unless they explicity set coop, force it to deathmatch
	if( Common::dedicated.GetBool( ) && !CVarSystem::GetCVarBool( "coop" ) )
		CVarSystem::SetCVarBool( "deathmatch", true );
	// init clients
	if( CVarSystem::GetCVarBool( "deathmatch" ) ) {
		if( Common::maxclients.GetInt( ) <= 1 )
			Common::maxclients.SetInt( 8 );
		else if( Common::maxclients.GetInt( ) > MAX_CLIENTS )
			Common::maxclients.SetInt( MAX_CLIENTS );
	} else if( CVarSystem::GetCVarBool( "coop" ) ) {
		if( Common::maxclients.GetInt( ) <= 1 || Common::maxclients.GetInt( ) > 4 )
			Common::maxclients.SetInt( 4 );
	} else
		Common::maxclients.SetInt( 1 );
#ifdef MM_TIMER
	svs.spawncount = WindowsNS::timeGetTime( ) & 0x00000FEF;
#else
	svs.spawncount = System::Sys_MillisecondsInt( ) & 0x00000FEF;
#endif
	svs.clients = new client_t[ Common::maxclients.GetInt( ) ];
	svs.num_client_entities = Common::maxclients.GetInt( ) * UPDATE_BACKUP * 64;
	svs.client_entities = new entity_state_t[ svs.num_client_entities ];
	// init network stuff
	//Network::NET_Config( Common::maxclients.GetInt( ) > 1 );
	svs.d_serverPort.InitForPort( PORT_SERVER );
	// heartbeats will always be sent to the  master
	svs.last_heartbeat = -99999;		// send immediately
	//sprintf( master, "192.246.40.37:%i", PORT_MASTER );
	//sprintf( master, "192.168.1.1:%i", 80 );
	//Network::NET_StringToAdr( master, &master_adr[ 0 ] );
	// init game
	SV_InitGameProgs( );
	for( int i = 0; i < Common::maxclients.GetInt( ); i++ ) {
		svs.clients[ i ].edict = EDICT_NUM( i + 1 );
	}
}

void Server::SV_Map( bool attractloop, const Str & levelstring, bool loadgame ) {
	Str	level = levelstring;
	Str	extension = level.Right( 4 );
	Str	spawnpoint;
	sv.loadgame = loadgame;
	sv.attractloop = attractloop;
	if( sv.state == ss_dead && !sv.loadgame )
		SV_InitGame( );	// the game is just starting
	// if there is a + in the map, set nextserver to the remainder
	if( level.Find( "+" ) != - 1 )
		CVarSystem::SetCVarStr( "nextserver", va( "gamemap \"%s\"", level.Right( level.Length( ) -( level.Find( "+" ) + 1 ) ).c_str( ) ) );
	else
		CVarSystem::SetCVarStr( "nextserver", "" );
	//ZOID special hack for end game screen in coop mode
	if( CVarSystem::GetCVarBool( "coop" ) && level == "victory.pcx" )
		CVarSystem::SetCVarStr( "nextserver", "gamemap \"*base1\"" );
	// if there is a $, use the remainder as a spawnpoint
	if( level.Find( "$" ) != - 1 )
		spawnpoint = level.Right( level.Length( ) -( level.Find( "$" ) + 1 ) );
	else
		spawnpoint.Clear( );
	// skip the end-of-unit flag if necessary
	if( level[ 0 ] == '*' ) level = level.Right( level.Length( ) - 1 );
	if( extension == ".cin" ) {
#ifndef DEDICATED_ONLY
		if( !Common::dedicated.GetBool( ) )
			Screen::SCR_BeginLoadingPlaque( );			// for local system
#endif
		SV_BroadcastCommand( "changing\n" );
		SV_SpawnServer( level, spawnpoint, ss_cinematic, attractloop, loadgame );

	} else if( extension == ".dm2" ) {
#ifndef DEDICATED_ONLY
		if( !Common::dedicated.GetBool( ) )
			Screen::SCR_BeginLoadingPlaque( );			// for local system
#endif
		SV_BroadcastCommand( "changing\n" );
		SV_SpawnServer( level, spawnpoint, ss_demo, attractloop, loadgame );

	} else if( extension == ".pcx" ) {
#ifndef DEDICATED_ONLY
		if( !Common::dedicated.GetBool( ) )
			Screen::SCR_BeginLoadingPlaque( );			// for local system
#endif
		SV_BroadcastCommand( "changing\n" );
		SV_SpawnServer( level, spawnpoint, ss_pic, attractloop, loadgame );

	} else {
#ifndef DEDICATED_ONLY
		if( !Common::dedicated.GetBool( ) )
			Screen::SCR_BeginLoadingPlaque( );			// for local system
#endif
		SV_BroadcastCommand( "changing\n" );
		SV_SendClientMessages( );
		SV_SpawnServer( level, spawnpoint, ss_game, attractloop, loadgame );
		CBuffer::Cbuf_CopyToDefer( );
	}
	SV_BroadcastCommand( "reconnect\n" );
}
