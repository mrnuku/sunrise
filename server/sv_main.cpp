#include "precompiled.h"
#pragma hdrstop

netadr_t	Server::master_adr[ MAX_MASTERS ];	// address of group servers
client_t *	Server::sv_client;			// current client

//============================================================================

void Server::SV_DropClient( client_t * drop ) {
	// add the disconnect
	//Message::MSG_WriteByte( &drop->netchan.message, svc_disconnect );
	drop->d_clientBufferReliable.WriteByte( svc_disconnect );
	if( drop->state == cs_spawned ) {
		// call the prog function for removing a client
		// this will remove the body, among other things
		Game::ClientDisconnect( drop->edict );
	}
	if( drop->download.IsValid( ) )
		FileSystem::FreeFile( drop->download );
	drop->state = cs_zombie;		// become free in a few seconds
	drop->name.Clear( );
}

const Str Server::SV_StatusString( ) {
	Str	player;
	Str	status;
	int		i;
	client_t	* cl;

	status = CVarSystem::Serverinfo( );
	status.Append( "\n" );
	for( i = 0; i < Common::maxclients.GetInt( ); i++ ) {
		cl = &svs.clients[ i ];
		if( cl->state == cs_connected || cl->state == cs_spawned ) {
			sprintf( player, "%i %i \"%s\"\n", cl->edict->client->ps.stats[ STAT_FRAGS ], cl->ping, cl->name.c_str( ) );
			status.Append( player );
		}
	}
	return status;
}

void Server::SVC_Status( MessageBuffer & msg_buffer, netadr_t & msg_from ) {
	MessageBufferT< MAX_MSGLEN >	messageBuffer;
	messageBuffer.Init( );
	messageBuffer.WriteShort( CONNECTIONLESS_MESSAGE_ID );
	messageBuffer.WriteString( va( "print\n%s", SV_StatusString( ).c_str( ) ) );
	svs.d_serverPort.SendPacket( msg_from, messageBuffer );
}

void Server::SVC_Ack( MessageBuffer & msg_buffer, netadr_t & msg_from ) {
	Common::Com_Printf( "Ping acknowledge from %s\n", Network::NET_AdrToString( msg_from ).c_str( ) );
}

void Server::SVC_Info( MessageBuffer & msg_buffer, netadr_t & msg_from ) {
	Str	string;
	int		count = 0;
	if( Common::maxclients.GetInt( ) == 1 )
		return;		// ignore in single player
	int version = atoi( Command::Cmd_Argv( 1 ) );
	if( version != PROTOCOL_VERSION )
		sprintf( string, "%s: wrong version\n", hostname.GetString( ).c_str( ) );
	else {
		for( int i = 0; i < Common::maxclients.GetInt( ); i++ )
			if( svs.clients[ i ].state >= cs_connected )
				count++;
		sprintf( string, "%16s %8s %2i/%2i\n", hostname.GetString( ).c_str( ), sv.name.c_str( ), count, Common::maxclients.GetInt( ) );
	}
	MessageBufferT< MAX_MSGLEN >	messageBuffer;
	messageBuffer.Init( );
	messageBuffer.WriteShort( CONNECTIONLESS_MESSAGE_ID );
	messageBuffer.WriteString( va( "info\n%s", string.c_str( ) ) );
	svs.d_serverPort.SendPacket( msg_from, messageBuffer );
}

void Server::SVC_Ping( MessageBuffer & msg_buffer, netadr_t & msg_from ) {
	MessageBufferT< MAX_MSGLEN >	messageBuffer;
	messageBuffer.Init( );
	messageBuffer.WriteShort( CONNECTIONLESS_MESSAGE_ID );
	messageBuffer.WriteString( "ack" );
	svs.d_serverPort.SendPacket( msg_from, messageBuffer );
}

void Server::SVC_GetChallenge( MessageBuffer & msg_buffer, netadr_t & msg_from ) {
	int			i;
	int			oldest;
	timeType	oldestTime;
	oldest = 0;
	oldestTime = 999999;
	// see if we already have a challenge for this ip
	for( i = 0; i < MAX_CHALLENGES; i++ ) {
		if( Network::NET_CompareBaseAdr( msg_from, svs.challenges[ i ].adr ) )
			break;
		if( svs.challenges[ i ].time < oldestTime ) {
			oldestTime = svs.challenges[ i ].time;
			oldest = i;
		}
	}
	if( i == MAX_CHALLENGES ) {
		// overwrite the oldest
		svs.challenges[ oldest ].challenge = rand( ) & 0x7FFF;
		svs.challenges[ oldest ].adr = msg_from;
		svs.challenges[ oldest ].time = System::Sys_LastTime( );
		i = oldest;
	}
	// send it back
	//Netchan::Netchan_OutOfBandPrint( NS_SERVER, msg_from, "challenge %i", svs.challenges[ i ].challenge );
	MessageBufferT< 1024 >	messageBuffer;
	messageBuffer.Init( );
	messageBuffer.WriteShort( CONNECTIONLESS_MESSAGE_ID );
	messageBuffer.WriteString( "challenge" );
	messageBuffer.WriteLong( svs.challenges[ i ].challenge );
	svs.d_serverPort.SendPacket( msg_from, messageBuffer );
}

void Server::SVC_DirectConnect( MessageBuffer & msg_buffer, netadr_t & msg_from ) {
	Common::Com_DPrintf( "SVC_DirectConnect\n" );
	//int version = atoi( Command::Cmd_Argv( 1 ) );
	MessageBufferT< MAX_MSGLEN >	messageBuffer;
	messageBuffer.Init( );
	int version = msg_buffer.ReadByte( );
	if( version != PROTOCOL_VERSION ) {
		messageBuffer.WriteShort( CONNECTIONLESS_MESSAGE_ID );
		messageBuffer.WriteString( va( "print\nServer is version %4.2f.\n", VERSION ) );
		svs.d_serverPort.SendPacket( msg_from, messageBuffer );
		Common::Com_DPrintf( "    rejected connect from version %i\n", version );
		return;
	}
	//int qport = atoi( Command::Cmd_Argv( 2 ) );
	int qport = msg_buffer.ReadShort( );
	//int challenge = atoi( Command::Cmd_Argv( 3 ) );
	int challenge = msg_buffer.ReadLong( );
	Str userinfo;// = Command::Cmd_Argv( 4 );
	msg_buffer.ReadString( userinfo );
	// force the IP key/value pair so the game can filter based on ip
	ClientServerInfo::Info_SetValueForKey( userinfo, "ip", Network::NET_AdrToString( msg_from ) );
	// attractloop servers are ONLY for local clients
	if( sv.attractloop ) {
		if( !Network::NET_IsLocalAddress( msg_from ) ) {
			Common::Com_Printf( "Remote connect in attract loop.  Ignored.\n" );
			messageBuffer.WriteShort( CONNECTIONLESS_MESSAGE_ID );
			messageBuffer.WriteString( "print\nConnection refused.\n" );
			svs.d_serverPort.SendPacket( msg_from, messageBuffer );
			return;
		}
	}
	// see if the challenge is valid
	int i;
	if( !Network::NET_IsLocalAddress( msg_from ) ) {
		for( i = 0; i < MAX_CHALLENGES; i++ ) {
			if( Network::NET_CompareBaseAdr( msg_from, svs.challenges[ i ].adr ) ) {
				if( challenge == svs.challenges[ i ].challenge )
					break;		// good
				messageBuffer.WriteShort( CONNECTIONLESS_MESSAGE_ID );
				messageBuffer.WriteString( "print\nBad challenge.\n" );
				svs.d_serverPort.SendPacket( msg_from, messageBuffer );
				return;
			}
		}
		if( i == MAX_CHALLENGES ) {
			messageBuffer.WriteShort( CONNECTIONLESS_MESSAGE_ID );
			messageBuffer.WriteString( "print\nNo challenge for address.\n" );
			svs.d_serverPort.SendPacket( msg_from, messageBuffer );
			return;
		}
	}
	client_t * cl, * newcl = NULL, * freecl = NULL;
	// if there is already a slot for this ip, reuse it
	for( i = 0, cl = svs.clients; i < Common::maxclients.GetInt( ); i++, cl++ ) {
		if( !freecl && cl->state == cs_free )
			freecl = cl;
		if( cl->state == cs_free )
			continue;
		if( Network::NET_CompareBaseAdr( msg_from, cl->d_clientChannel.GetRemoteAddress( ) ) && ( cl->d_clientChannel.GetID( ) == qport || msg_from.port == cl->d_clientChannel.GetRemoteAddress( ).port ) ) {
			if( !Network::NET_IsLocalAddress( msg_from ) && ( svs.realtime - cl->lastconnect ) < ( sv_reconnect_limit.GetFloat( ) * 1000.0 ) ) {
				Common::Com_DPrintf( "%s:reconnect rejected : too soon\n", Network::NET_AdrToString( msg_from ).c_str( ) );
				return;
			}
			Common::Com_Printf( "%s:reconnect\n", Network::NET_AdrToString( msg_from ).c_str( ) );
			newcl = cl;
			break;
		}
	}
	if( !newcl )
		newcl = freecl;
	if( !newcl ) {
		messageBuffer.WriteShort( CONNECTIONLESS_MESSAGE_ID );
		messageBuffer.WriteString( "print\nServer is full.\n" );
		svs.d_serverPort.SendPacket( msg_from, messageBuffer );
		Common::Com_DPrintf( "Rejected a connection.\n" );
		return;
	}	
	// build a new connection
	// accept the new client
	// this is the only place a client_t is ever initialized
	//newcl->Clear( );
	sv_client = newcl;
	//int edictnum = ( int )( newcl - svs.clients ) + 1;
	//Entity * ent = EDICT_NUM( edictnum );
	//newcl->edict = ent;
	sv_client->challenge = challenge; // save challenge for checksumming
	// get the game a chance to reject this connection or modify the userinfo
	if( !Game::ClientConnect( sv_client->edict, userinfo ) ) {
		messageBuffer.WriteShort( CONNECTIONLESS_MESSAGE_ID );
		if( ClientServerInfo::Info_ValueForKey( userinfo, "rejmsg" ).Length( ) )
			messageBuffer.WriteString( va( "print\n%s\nConnection refused.\n", ClientServerInfo::Info_ValueForKey( userinfo, "rejmsg" ).c_str( ) ) );
		else
			messageBuffer.WriteString( "print\nConnection refused.\n" );
		svs.d_serverPort.SendPacket( msg_from, messageBuffer );
		Common::Com_DPrintf( "Game rejected a connection.\n" );
		return;
	}
	// parse some info from the info strings
	sv_client->userinfo = userinfo;
	SV_UserinfoChanged( sv_client );
	// send the connect packet to the client
	//Netchan::Netchan_OutOfBandPrint( NS_SERVER, adr, "client_connect" );
	//Netchan::Netchan_Setup( NS_SERVER, &newcl->netchan , adr, qport );
	
	messageBuffer.BeginWriting( );
	messageBuffer.WriteShort( CONNECTIONLESS_MESSAGE_ID );
	messageBuffer.WriteString( "client_connect" );
	messageBuffer.WriteShort( qport );
	svs.d_serverPort.SendPacket( msg_from, messageBuffer );
	sv_client->d_clientBuffer.Init( );
	sv_client->d_clientBuffer.SetAllowOverflow( true );
	sv_client->d_clientBufferReliable.Init( );
	sv_client->d_clientChannel.Init( msg_from, qport );

	sv_client->state = cs_connected;	
	//Message::SZ_Init( &newcl->datagram, newcl->datagram_buf, sizeof( newcl->datagram_buf ) );
	//newcl->datagram.allowoverflow = true;
	sv_client->lastmessage = svs.realtime;	// don't timeout
	sv_client->lastconnect = svs.realtime;
}

bool Server::Rcon_Validate( ) {
	if( Command::Cmd_Argv( 1 ) == CVarSystem::GetCVarStr( "rcon_password" ) )
		return true;
	return false;
}

void Server::SVC_RemoteCommand( MessageBuffer & msg_buffer, netadr_t & msg_from ) {
	Str	remaining;
	if( !Rcon_Validate( ) )
		Common::Com_Printf( "Bad rcon from %s:\n%s\n", Network::NET_AdrToString( msg_from ).c_str( ), msg_buffer.GetData( ) + 4 );
	else
		Common::Com_Printf( "Rcon from %s:\n%s\n", Network::NET_AdrToString( msg_from ).c_str( ), msg_buffer.GetData( ) + 4 );

	//Common::Com_BeginRedirect( RD_PACKET, sv_outputbuf, Server::SV_FlushRedirect );
	Str redirectBuffer;
	Common::Com_BeginRedirect( redirectBuffer );

	if( !Rcon_Validate( ) ) {
		Common::Com_Printf( "Bad rcon_password.\n" );
	} else {
		for( int i = 2; i < Command::Cmd_Argc( ); i++ ) {
			remaining.Append( Command::Cmd_Argv( i ) );
			remaining.Append( " " );
		}
		Command::Cmd_ExecuteString( remaining );
	}

	Common::Com_EndRedirect( );
	MessageBufferT< MAX_MSGLEN >	messageBuffer;
	messageBuffer.Init( );
	messageBuffer.WriteShort( CONNECTIONLESS_MESSAGE_ID );
	messageBuffer.WriteString( "print" );
	messageBuffer.WriteString( redirectBuffer );
	svs.d_serverPort.SendPacket( msg_from, messageBuffer );
}

void Server::SV_ConnectionlessPacket( MessageBuffer & msg_buffer, netadr_t & msg_from ) {
	//Message::MSG_BeginReading( &net_message );
	//Message::MSG_ReadLong( &net_message );		// skip the -1 marker
	//Str s = Message::MSG_ReadStringLine( &net_message );
	Str c;
	bool breakParse = false;
	while( msg_buffer.GetRemaingData( ) ) {
		msg_buffer.ReadString( c );
		//Command::Cmd_TokenizeString( s, false );
		//Str c = Command::Cmd_Argv( 0 );
		Common::Com_DPrintf( "SVCP %s %s\n", Network::NET_AdrToString( msg_from ).c_str( ), c.c_str( ) );
		if( c == "ping" )
			SVC_Ping( msg_buffer, msg_from );
		else if( c == "ack" )
			SVC_Ack( msg_buffer, msg_from );
		else if( c == "status" )
			SVC_Status( msg_buffer, msg_from );
		else if( c == "info" )
			SVC_Info( msg_buffer, msg_from );
		else if( c == "getchallenge" )
			SVC_GetChallenge( msg_buffer, msg_from );
		else if( c == "connect" )
			SVC_DirectConnect( msg_buffer, msg_from );
		else if( c == "rcon" )
			SVC_RemoteCommand( msg_buffer, msg_from );
		else {
			if( breakParse )
				break;
			breakParse = true;
			Common::Com_Printf( "bad command packet from %s:\n%s\n", Network::NET_AdrToString( msg_from ).c_str( ), c.c_str( ) );
		}
	}
}

void Server::SV_CalcPings( ) {
	client_t	* cl;
	timeType	total, count;
	for( int i = 0; i < Common::maxclients.GetInt( ); i++ ) {
		cl = &svs.clients[ i ];
		if( cl->state != cs_spawned )
			continue;
		total = count = 0;
		for( int j = 0; j < LATENCY_COUNTS; j++ ) {
			if( cl->frame_latency[ j ] > 0 ) {
				count++;
				total += cl->frame_latency[ j ];
			}
		}
		if( !count )
			cl->ping = 0;
		else
			cl->ping = total / count;
		// let the game dll know about the ping
		cl->edict->client->ping = cl->ping;
	}
}

void Server::SV_GiveMsec( ) {
	client_t	* cl;
	if( sv.framenum & 15 )
		return;
	for( int i = 0; i < Common::maxclients.GetInt( ); i++ ) {
		cl = &svs.clients[ i ];
		if( cl->state == cs_free )
			continue;		
		cl->commandMsec = 1800;		// 1600 + some slop
	}
}

void Server::SV_ReadPackets( ) {
	int				msgId, msgSeq, msgTime = System::Sys_MillisecondsInt( );
	netadr_t		msgFrom;
	MessageBufferT< MAX_MSGLEN >	msg;
	MessageBufferT< MAX_MSGLEN >	msgReliable;
	msg.Init( );
	msgReliable.Init( );

	while( svs.d_serverPort.GetPacket( msgFrom, msg ) ) {
		
		msgId = msg.ReadShort( );
		if( msgId == CONNECTIONLESS_MESSAGE_ID ) {
			SV_ConnectionlessPacket( msg, msgFrom );
			continue;
		}
		if( msg.GetRemaingData( ) < 4 ) {
			Common::Com_Printf( "%s: %ib packet\n", Network::NET_AdrToString( msgFrom ).c_str( ), msg.GetRemaingData( ) );
			continue;
		}
		// check for packets from connected clients
		int i = 0;
		for( client_t * cl = svs.clients; i < Common::maxclients.GetInt( ); i++, cl++ ) {
			if( cl->state == cs_free )
				continue;
			if( cl->d_clientChannel.GetID( ) != msgId )
				continue;
			if( cl->d_clientChannel.Process( msgFrom, msgTime, msg, msgSeq ) ) {
				while( cl->d_clientChannel.GetReliableMessage( msgReliable ) )
					SV_ExecuteClientMessage( cl, msgReliable );
				if( cl->state != cs_zombie ) {
					cl->lastmessage = svs.realtime;	// don't timeout
					SV_ExecuteClientMessage( cl, msg );
				}
			}
			break;
		}		
		if( i != Common::maxclients.GetInt( ) )
			continue;
	}
}

void Server::SV_CheckTimeouts( ) {
	int		i;
	client_t	* cl;
	timeType	droppoint;
	timeType	zombiepoint;
	droppoint = svs.realtime - ( timeType )( 1000 * timeout.GetFloat( ) );
	zombiepoint = svs.realtime - ( timeType )( 1000 * zombietime.GetFloat( ) );
	for( i = 0, cl = svs.clients; i < Common::maxclients.GetInt( ); i++, cl++ ) {
		// message times may be wrong across a changelevel
		if( cl->lastmessage > svs.realtime )
			cl->lastmessage = svs.realtime;
		if( cl->state == cs_zombie && cl->lastmessage < zombiepoint ) {
			cl->state = cs_free;	// can now be reused
			continue;
		}
		if( ( cl->state == cs_connected || cl->state == cs_spawned ) && cl->lastmessage < droppoint ) {
			SV_BroadcastPrintf( PRINT_HIGH, "%s timed out\n", cl->name.c_str( ) );
			SV_DropClient( cl ); 
			cl->state = cs_free;	// don't bother with zombie state
		}
	}
}


void Server::SV_PrepWorldFrame( ) {
	Entity	* ent;
	for( int i = 0; i < Game::num_edicts; i++, ent++ ) {
		ent = EDICT_NUM( i );
		// events only last for a single message
		ent->s.eventNum = EV_NONE;
	}
}

void Server::SV_RunGameFrame( ) {
	// we always need to bump framenum, even if we
	// don't run the world, otherwise the delta
	// compression can get confused when a client
	// has the "current" frame
	sv.framenum++;
	sv.time = sv.framenum * 100;
	// don't run if paused
	if( Common::maxclients.GetInt( ) > 1 ) {
		Game::RunFrame( );
		// never get more than one tic behind
		if( sv.time < svs.realtime ) {
			if( CVarSystem::GetCVarBool( "showclamp" ) )
				Common::Com_Printf( "sv highclamp\n" );
			svs.realtime = sv.time;
		}
	}
}

void Server::SV_Frame( timeType msec ) {
	// if server is not active, do nothing
	if( !svs.initialized )
		return;

    //svs.realtime += msec;
	svs.realtime = System::Sys_LastTime( );

	// keep the random time dependent
	rand( );

	// check timeouts
	SV_CheckTimeouts( );

	// get packets from clients
	SV_ReadPackets( );

	// move autonomous things around if enough time has passed
	if( !CVarSystem::GetCVarBool( "timedemo" ) && svs.realtime < sv.time ) {
		// never let the time get too far off
		if( sv.time - svs.realtime > 100 ) {
			if( CVarSystem::GetCVarBool( "showclamp" ) )
				Common::Com_Printf( "sv lowclamp\n" );
			svs.realtime = sv.time - 100;
		}
		Network::NET_Sleep( ( int )( sv.time - svs.realtime ) );
		return;
	}

	// update ping based on the last known frame from all clients
	SV_CalcPings( );

	// give the clients some timeslices
	SV_GiveMsec( );

	// let everything in the world think and move
	SV_RunGameFrame( );

	// send messages back to the clients that had packets read this frame
	SV_SendClientMessages( );

	// save the entire world state if recording a serverdemo
	SV_RecordDemoMessage( );

	// send a heartbeat to the master if needed
	Master_Heartbeat( );

	// clear teleport flags, etc for next frame
	SV_PrepWorldFrame( );

}

#define	HEARTBEAT_SECONDS	300

void Server::Master_Heartbeat( ) {
	return; // FIXME
	// pgm post3.19 change, cvar pointer not validated before dereferencing
	if( !Common::dedicated.GetBool( ) )
		return;		// only dedicated servers send heartbeats
	// pgm post3.19 change, cvar pointer not validated before dereferencing
	if( !public_server.GetBool( ) )
		return;		// a private dedicated game
	// check for time wraparound
	if( svs.last_heartbeat > svs.realtime )
		svs.last_heartbeat = svs.realtime;
	if( svs.realtime - svs.last_heartbeat < HEARTBEAT_SECONDS * 1000 )
		return;		// not time to send yet
	svs.last_heartbeat = svs.realtime;
	// send the same string that we would give for a status OOB command
	Str string = SV_StatusString( );
	MessageBufferT< MAX_MSGLEN >	messageBuffer;
	messageBuffer.Init( );
	// send to group master
	for( int i = 0; i < MAX_MASTERS; i++ ) {
		if( master_adr[ i ].port ) {
			Common::Com_Printf( "Sending heartbeat to %s\n", Network::NET_AdrToString( master_adr[ i ] ).c_str( ) );
			messageBuffer.BeginWriting( );
			messageBuffer.WriteShort( CONNECTIONLESS_MESSAGE_ID );
			messageBuffer.WriteString( va( "heartbeat\n%s", string.c_str( ) ) );
			svs.d_serverPort.SendPacket( master_adr[ i ], messageBuffer );
		}
	}
}

void Server::Master_Shutdown( ) {
	return; // FIXME
	// pgm post3.19 change, cvar pointer not validated before dereferencing
	if( !Common::dedicated.GetBool( ) )
		return;		// only dedicated servers send heartbeats
	// pgm post3.19 change, cvar pointer not validated before dereferencing
	if( !public_server.GetBool( ) )
		return;		// a private dedicated game
	MessageBufferT< MAX_MSGLEN >	messageBuffer;
	messageBuffer.Init( );
	// send to group master
	for( int i = 0; i < MAX_MASTERS; i++ ) {
		if( master_adr[ i ].port ) {
			Common::Com_Printf( "Sending heartbeat to %s\n", Network::NET_AdrToString( master_adr[ i ] ).c_str( ) );
			messageBuffer.BeginWriting( );
			messageBuffer.WriteShort( CONNECTIONLESS_MESSAGE_ID );
			messageBuffer.WriteString( "shutdown" );
			svs.d_serverPort.SendPacket( master_adr[ i ], messageBuffer );
		}
	}
}

void Server::SV_UserinfoChanged( client_t * cl ) {
	// call prog code to allow overrides
	Game::ClientUserinfoChanged( cl->edict, cl->userinfo );	
	// name for C code
	cl->name = ClientServerInfo::Info_ValueForKey( cl->userinfo, "name" );
	// mask off high bit
	//for( i = 0; i < sizeof( cl->name ); i++ ) cl->name[ i ] &= 127; // FIXME NOW
	// rate command
	Str val = ClientServerInfo::Info_ValueForKey( cl->userinfo, "rate" );
	if( val.Length( ) ) {
		int cRate = atoi( val );
		if( cRate < 2500 )
			cRate = 2500;
		if( cRate > 50000 )
			cRate = 50000;
		cl->d_clientChannel.SetMaxOutgoingRate( cRate );
	}
	// msg command
	val = ClientServerInfo::Info_ValueForKey( cl->userinfo, "msg" );
	if( val.Length( ) )
		cl->messagelevel = atoi( val );
}

void Server::SV_Init( ) {
	SV_InitOperatorCommands( );	
}

void Server::SV_FinalMessage( const Str & message, bool reconnect ) {
	SV_BroadcastPrintf( PRINT_HIGH, message );
	sv.d_multicastBuffer.WriteByte( reconnect ? svc_reconnect : svc_disconnect );
	SV_Multicast( vec3_origin, MULTICAST_ALL_R );
}

void Server::SV_Shutdown( const Str & finalmsg, bool reconnect ) {
	if( svs.clients )
		SV_FinalMessage( finalmsg, reconnect );
	Master_Shutdown( );
	SV_ShutdownGameProgs( );
	// free current level
	if( sv.demofile )
		FileSystem::CloseFile( sv.demofile );
	svs.d_serverPort.Close( );
	sv.Clear( );
	Common::Com_SetServerState( sv.state );
	// free server static data
	if( svs.clients )
		delete[ ] svs.clients;
	if( svs.client_entities )
		delete[ ] svs.client_entities;
	if( svs.demofile )
		FileSystem::CloseFile( svs.demofile );
	svs.Clear( );
}
