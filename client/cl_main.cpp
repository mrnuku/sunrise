#include "precompiled.h"
#pragma hdrstop

entity_state_t		Client::cl_parse_entities[ MAX_PARSE_ENTITIES ];

MessageBufferT< MAX_MSGLEN >		Client::cl_messageBuffer;
MessageBufferT< MAX_MSGLEN >		Client::cl_messageBufferReliable;

void frame_t::Clear( ) {
	valid			= false;
	serverframe		= 0;
	servertime		= 0;
	deltaframe		= 0;
	Common::Com_Memset( areabits, 0, sizeof( areabits ) );
	playerstate.Clear( );
	num_entities	= 0;
	parse_entities	= 0;
}

void centity_t::Clear( ) {
	baseline.Clear( );
	current.Clear( );
	prev.Clear( );
	serverframe		= 0;
	trailcount		= 0;
	lerp_origin.Zero( );
	fly_stoptime	= 0;
}

void clientinfo_t::Clear( ) {
	name.Clear( );
	cinfo.Clear( );
	skin			= NULL;
	icon			= NULL;
	iconname.Clear( );
	model			= NULL;
	weaponmodel.Clear( );
}

void ClientStaticData::Clear( ) {
	state			= ca_uninitialized;
	key_dest		= key_game;
	framecount		= 0;
	realtime		= 0;
	frametime		= 0;
	disable_screen	= 0;
	disable_servercount	= 0;
	servername.Clear( );
	connect_time	= 0;
	quakePort		= 0;
	assert( !d_clientPort.GetPort( ) );
	d_clientChannel.Shutdown( );
	serverProtocol	= 0;
	challenge		= 0;
	download		= NULL;
	downloadtempname.Clear( );
	downloadname.Clear( );
	downloadnumber	= 0;
	downloadtype	= dl_none;
	downloadpercent	= 0;
	demorecording	= false;
	demowaiting		= false;
	demofile		= NULL;
}

void ClientStateData::Clear( ) {
	timeoutcount		= 0;
	timedemo_frames		= 0;
	timedemo_start		= 0;
	refresh_prepped		= false;
	sound_prepped		= false;
	force_refdef		= false;
	parse_entities		= 0;
	cmd.Clear( );
	for( int i = 0; i < CMD_BACKUP; i++ )
		cmds[ i ].Clear( );
	for( int i = 0; i < CMD_BACKUP; i++ )
		cmd_time[ i ] = 0;
	for( int i = 0; i < CMD_BACKUP; i++ ) {
		predicted_origins[ i ][ 0 ] = 0;
		predicted_origins[ i ][ 1 ] = 0;
		predicted_origins[ i ][ 2 ] = 0;
	}
	predicted_step		= 0;
	predicted_step_time	= 0;
	predicted_origin.Zero( );
	predicted_angles.Zero( );
	prediction_error.Zero( );
	frame.Clear( );
	surpressCount		= 0;
	for( int i = 0; i < UPDATE_BACKUP; i++ )
		frames[ i ].Clear( );
	viewangles.Zero( );
	time				= 0;
	lerpfrac			= 0;
	refdef.Clear( );
	v_forward.Zero( );
	v_right.Zero( );
	v_up.Zero( );
	layout.Clear( );
	for( int i = 0; i < MAX_ITEMS; i++ )
		inventory[ i ]		= 0;
	attractloop			= false;
	servercount			= 0;
	gamedir.Clear( );
	playernum			= 0;
	for( int i = 0; i < MAX_CONFIGSTRINGS; i++ )
		configstrings[ i ].Clear( );
	for( int i = 0; i < MAX_MODELS; i++ )
		model_draw[ i ] = NULL;
	for( int i = 0; i < MAX_MODELS; i++ )
		model_clip[ i ] = NULL;
	for( int i = 0; i < MAX_SOUNDS; i++ )
		sound_precache[ i ] = NULL;
	for( int i = 0; i < MAX_IMAGES; i++ )
		image_precache[ i ] = NULL;
	for( int i = 0; i < MAX_CLIENTS; i++ )
		clientinfo[ i ].Clear( );
	baseclientinfo.Clear( );
}

void Client::CL_WriteDemoMessage( MessageBuffer & msg_buffer ) {
	size_t	len;
	int		swlen;
	// the first eight bytes are just packet sequencing stuff
	len = msg_buffer.GetSize( ) - 8;
	swlen = LittleLong( ( int )len );
	cls.demofile->Write( &swlen, 4 );
	cls.demofile->Write( msg_buffer.GetData( ) + 8, len );
}

void Client::CL_Stop_f( ) {
	if( !cls.demorecording ) {
		Common::Com_Printf( "Not recording a demo.\n" );
		return;
	}
	// finish up
	int len = -1;
	cls.demofile->Write( &len, 4 );
	FileSystem::CloseFile( cls.demofile );
	cls.demorecording = false;
	Common::Com_Printf( "Stopped demo.\n" );
}

void Client::CL_Record_f( ) {
	Str name;
	MessageBufferT< MAX_MSGLEN >	msgBuffer;
	int i;
	int len;
	entity_state_t * ent;
	entity_state_t nullstate;
	if( Command::Cmd_Argc( ) != 2 ) {
		Common::Com_Printf( "record <demoname>\n" );
		return;
	}
	if( cls.demorecording ) {	
		Common::Com_Printf( "Already recording.\n" );
		return;
	}
	if( cls.state != ca_active ) {
		Common::Com_Printf( "You must be in a level to record.\n" );
		return;
	}
	// open the demo file
	sprintf( name, "demos/%s.dm2", Command::Cmd_Argv( 1 ).c_str( ) );
	Common::Com_Printf( "recording to %s.\n", name.c_str( ) );
	cls.demofile = FileSystem::OpenFileByMode( name, FS_WRITE );
	cls.demorecording = true;
	// don't start saving messages until a non-delta compressed message is received
	cls.demowaiting = true;
	// write out messages to hold the startup information
	msgBuffer.Init( );
	// send the serverdata
	msgBuffer.WriteByte( svc_serverdata );
	msgBuffer.WriteLong( PROTOCOL_VERSION );
	msgBuffer.WriteLong( 0x10000 + cl.servercount );
	msgBuffer.WriteByte( 1 ); // demos are always attract loops
	msgBuffer.WriteString( cl.gamedir );
	msgBuffer.WriteShort( cl.playernum );
	msgBuffer.WriteString( cl.configstrings[ CS_NAME ] );
	// configstrings
	for( i = 0; i < MAX_CONFIGSTRINGS; i++ ) {
		if( cl.configstrings[ i ].Length( ) ) {
			if( msgBuffer.GetSize( ) + cl.configstrings[ i ].Length( ) + 32 > msgBuffer.GetMaxSize( ) ) { // write it out
				len = LittleLong( msgBuffer.GetSize( ) );
				cls.demofile->Write( &len, 4 );
				cls.demofile->Write( msgBuffer.GetData( ), msgBuffer.GetSize( ) );
				msgBuffer.BeginWriting( );
			}
			msgBuffer.WriteByte( svc_configstring );
			msgBuffer.WriteShort( i );
			msgBuffer.WriteString( cl.configstrings[ i ] );
		}
	}
	// baselines
	nullstate.Clear( );
	for( i = 0; i<MAX_EDICTS; i++ ) {
		ent = &cl_entities[ i ].baseline;
		if( !ent->modelindex )
			continue;
		if( msgBuffer.GetSize( ) + 64 > msgBuffer.GetMaxSize( ) ) { // write it out
			len = LittleLong( msgBuffer.GetSize( ) );
			cls.demofile->Write( &len, 4 );
			cls.demofile->Write( msgBuffer.GetData( ), msgBuffer.GetSize( ) );
			msgBuffer.BeginWriting( );
		}
		msgBuffer.WriteByte( svc_spawnbaseline );
		Server::SV_WriteDeltaEntity( &nullstate, &cl_entities[ i ].baseline, msgBuffer, true, true );
	}
	msgBuffer.WriteByte( svc_stufftext );
	msgBuffer.WriteString( "precache\n" );
	// write it to the demo file
	len = LittleLong( msgBuffer.GetSize( ) );
	cls.demofile->Write( &len, 4 );
	cls.demofile->Write( msgBuffer.GetData( ), msgBuffer.GetSize( ) );
	// the rest of the demo file will be individual frames
}

void Client::CL_ForwardToServer_f( ) {
	if( cls.state != ca_connected && cls.state != ca_active ) {
		Common::Com_Printf( "Can't \"%s\", not connected\n", Command::Cmd_Argv( 0 ).c_str( ) );
		return;
	}
	// don't forward the first argument
	if( Command::Cmd_Argc( ) > 1 ) {
		cl_messageBufferReliable.WriteByte( clc_stringcmd );
		cl_messageBufferReliable.WriteString( Command::Cmd_Args( ) );
	}
}

void Client::CL_Pause_f( ) {
	// never pause in multiplayer
	if( Common::maxclients.GetInt( ) > 1 || !Common::Com_ServerState( ) ) {
		Common::paused.SetBool( false );
		return;
	}
	Common::paused.SetBool( !Common::paused.GetBool( ) );
}

void Client::CL_Quit_f( ) {
	CL_Disconnect( );
	Common::Com_Quit( );
}

void Client::CL_Drop( ) {
	if( cls.state == ca_uninitialized )
		return;
	if( cls.state == ca_disconnected )
		return;
	CL_Disconnect( );
	// drop loading plaque unless this is the initial game start
	if( cls.disable_servercount != -1 )
		Screen::SCR_EndLoadingPlaque( ); // get rid of loading plaque
}

void Client::InitPort( ) {
	if( !cls.d_clientPort.GetPort( ) ) {
		cls.d_clientPort.InitForPort( PORT_CLIENT );
		if( !cls.d_clientPort.GetPort( ) )
			cls.d_clientPort.InitForPort( PORT_ANY );
	}
}

void Client::CL_SendConnectPacket( netadr_t & to ) {
	if( cls.state != ca_challenge )
		return;
	cls.state = ca_connecting;
	//int qport = CVarSystem::GetCVarInt( "qport" );
	cls.quakePort = System::Sys_MillisecondsUint( ) & 0xFFFF;
	Common::userinfo_modified.SetBool( false );
	//Netchan::Netchan_OutOfBandPrint( NS_CLIENT, adr, "connect %i %i %i \"%s\"\n", PROTOCOL_VERSION, port, cls.challenge, CVarSystem::Userinfo( ).c_str( ) );
	MessageBufferT< MAX_MSGLEN >	msgBuffer;
	msgBuffer.Init( );
	msgBuffer.WriteShort( CONNECTIONLESS_MESSAGE_ID );
	msgBuffer.WriteString( "connect" );
	msgBuffer.WriteByte( PROTOCOL_VERSION );
	msgBuffer.WriteShort( cls.quakePort );
	msgBuffer.WriteLong( cls.challenge );
	msgBuffer.WriteString( CVarSystem::Userinfo( ) );
	cls.d_clientPort.SendPacket( to, msgBuffer );
}

void Client::CL_CheckForResend( ) {
	netadr_t adr;
	// if the local server is running and we aren't
	// then connect
	if( cls.state == ca_disconnected && Common::Com_ServerState( ) ) {
		cls.state = ca_challenge;
		cls.servername = "localhost";
		// we don't need a challenge on the localhost
		//CL_SendConnectPacket( );
		//return;
		//cls.connect_time = cls.realtime;
		//		cls.connect_time = -99999;	// CL_CheckForResend( ) will fire immediately
	}
	// resend if we haven't gotten a reply yet
	if( cls.state != ca_challenge )
		return;
	if( ( cls.realtime - cls.connect_time ) < 3000 )
		return;
	if( !Network::NET_StringToAdr( cls.servername, &adr ) ) {
		Common::Com_Printf( "Bad server address\n" );
		cls.state = ca_disconnected;
		return;
	}
	if( adr.port == 0 )
		adr.port = BigShort( PORT_SERVER );
	cls.connect_time = cls.realtime; // for retransmit requests
	Common::Com_Printf( "Connecting to %s...\n", cls.servername.c_str( ) );
	InitPort( );
	MessageBufferT< MAX_MSGLEN >	msgBuffer;
	msgBuffer.Init( );
	msgBuffer.WriteShort( CONNECTIONLESS_MESSAGE_ID );
	msgBuffer.WriteString( "getchallenge" );
	cls.d_clientPort.SendPacket( adr, msgBuffer );
	//Netchan::Netchan_OutOfBandPrint( NS_CLIENT, adr, "getchallenge\n" );
}

void Client::CL_Connect_f( ) {
	Str server;
	if( Command::Cmd_Argc( ) != 2 ) {
		Common::Com_Printf( "usage: connect <server>\n" );
		return;
	}
	if( Common::Com_ServerState( ) ) { // if running a local server, kill it and reissue
		Server::SV_Shutdown( "Server quit\n", false );
	} else
		CL_Disconnect( );
	server = Command::Cmd_Argv( 1 );
	//Network::NET_Config( true ); // allow remote
	CL_Disconnect( );
	cls.state = ca_challenge;
	cls.servername = server;
	cls.connect_time = -99999; // CL_CheckForResend( ) will fire immediately
}

void Client::CL_ClearState( ) {
	//Sound::S_StopAllSounds( );
	SoundSystem::StopAllSounds( );
	CL_ClearEffects( );
	CL_ClearTEnts( );
	// wipe the entire cl structure
	cl.Clear( );
	for( int i = 0; i < MAX_EDICTS; i++ )
		cl_entities[ i ].Clear( );
	//Message::SZ_Clear( &cls.netchan.message );
}

void Client::CL_Disconnect( ) {
	if( cls.state == ca_disconnected )
		return;
	if( cl_timedemo.GetBool( ) ) {
		timeType time;
		time = System::Sys_Milliseconds( ) - cl.timedemo_start;
		if( time > 0 )
			Common::Com_Printf( "%i frames, %3.1f seconds: %3.1f fps\n", cl.timedemo_frames, time / 1000.0, cl.timedemo_frames * 1000.0 / time );
	}
	vec3_origin.CopyTo( cl.refdef.blend );
	//Renderer::CinematicSetPalette( NULL );
	//Menu::M_ForceMenuOff( );
	cls.connect_time = 0;
	//Screen::SCR_StopCinematic( );
	if( cls.demorecording )
		CL_Stop_f( );
	// send a disconnect message to the server
	cl_messageBufferReliable.BeginWriting( );
	cl_messageBufferReliable.WriteByte( clc_stringcmd );
	cl_messageBufferReliable.WriteString( "disconnect" );
	cls.d_clientChannel.ClearReliableMessages( );
	cls.d_clientChannel.SendReliableMessage( cl_messageBufferReliable );
	cl_messageBufferReliable.BeginWriting( );
	cls.d_clientChannel.SendMessage( cls.d_clientPort, cls.realtime, cl_messageBufferReliable );
	cls.d_clientChannel.Shutdown( );
	cls.d_clientPort.Close( );
	CL_ClearState( );
	// stop download
	if( cls.download )
		FileSystem::CloseFile( cls.download );
	cls.state = ca_disconnected;
}

void Client::CL_Disconnect_f( ) {
	Common::Com_Error( ERR_DROP, "Disconnected from server" );
}

void Client::CL_Packet_f( ) {
	char send[ 2048 ];
	int i, l;
	Str in;
	char * out;
	netadr_t adr;
	if( Command::Cmd_Argc( ) != 3 ) {
		Common::Com_Printf( "packet <destination> <contents>\n" );
		return;
	}
	//Network::NET_Config( true ); // allow remote
	if( !Network::NET_StringToAdr( Command::Cmd_Argv( 1 ), &adr ) ) {
		Common::Com_Printf( "Bad address\n" );
		return;
	}
	if( !adr.port )
		adr.port = BigShort( PORT_SERVER );
	in = Command::Cmd_Argv( 2 );
	out = send+4;
	send[ 0 ] = send[ 1 ] = send[ 2 ] = send[ 3 ] = ( char )0xFF;
	l = ( int )strlen( in );
	for( i = 0; i<l; i++ ) {
		if( in[ i ] == '\\' && in[ i+1 ] == 'n' ) {
			* out++ = '\n';
			i++;
		} else
			* out++ = in[ i ];
	}
	* out = 0;
	Network::NET_SendPacket( NS_CLIENT, out-send, send, adr );
}

void Client::CL_Changing_f( ) {
	//ZOID
	//if we are downloading, we don't change!  This so we don't suddenly stop downloading a map
	if( cls.download )
		return;
	Screen::SCR_BeginLoadingPlaque( );
	cls.state = ca_connected; // not active anymore, but not disconnected
	Common::Com_Printf( "\nChanging map...\n" );
}

void Client::CL_Reconnect_f( ) {
	//ZOID
	//if we are downloading, we don't change!  This so we don't suddenly stop downloading a map
	if( cls.download )
		return;
	//Sound::S_StopAllSounds( );
	SoundSystem::StopAllSounds( );
	if( cls.state == ca_connected ) {
		Common::Com_Printf( "reconnecting...\n" );
		cls.state = ca_connected;
		cl_messageBufferReliable.WriteChar( clc_stringcmd );
		cl_messageBufferReliable.WriteString( "new" );
		return;
	}
	if( *cls.servername ) {
		if( cls.state >= ca_connected ) {
			CL_Disconnect( );
			cls.connect_time = cls.realtime - 1500;
		} else
			cls.connect_time = -99999; // fire immediately
		cls.state = ca_challenge;
		Common::Com_Printf( "reconnecting...\n" );
	}
}

void Client::CL_ParseStatusMessage( MessageBuffer & msg_buffer ) {
	Str s;
	msg_buffer.ReadString( s );
	Common::Com_Printf( "%s\n", s.c_str( ) );
	//Menu::M_AddToServerList( net_from, s );
}

void Client::CL_PingServers_f( ) {
	int i;
	netadr_t adr;
	Str name;
	Str adrstring;
	MessageBufferT< MAX_MSGLEN >	msgBuffer;
	msgBuffer.Init( );
	//Network::NET_Config( true ); // allow remote
	// send a broadcast packet
	Common::Com_Printf( "pinging broadcast...\n" );
	if( !CVarSystem::GetCVarBool( "noudp" ) ) {
		adr.type = NA_BROADCAST;
		adr.port = BigShort( PORT_SERVER );
		msgBuffer.WriteShort( CONNECTIONLESS_MESSAGE_ID );
		msgBuffer.WriteString( va( "info %i", PROTOCOL_VERSION ) );
		cls.d_clientPort.SendPacket( adr, msgBuffer );
	}
	// send a packet to each address book entry
	for( i = 0; i<16; i++ ) {
		sprintf( name, "adr%i", i );
		adrstring = CVarSystem::GetCVarStr( name );
		if( !adrstring || !adrstring[ 0 ] )
			continue;
		Common::Com_Printf( "pinging %s...\n", adrstring.c_str( ) );
		if( !Network::NET_StringToAdr( adrstring, &adr ) ) {
			Common::Com_Printf( "Bad address: %s\n", adrstring.c_str( ) );
			continue;
		}
		if( !adr.port )
			adr.port = BigShort( PORT_SERVER );
		msgBuffer.BeginWriting( );
		msgBuffer.WriteShort( CONNECTIONLESS_MESSAGE_ID );
		msgBuffer.WriteString( va( "info %i", PROTOCOL_VERSION ) );
		cls.d_clientPort.SendPacket( adr, msgBuffer );
	}
}

void Client::CL_Skins_f( ) {
	for( int i = 0; i < MAX_CLIENTS; i++ ) {
		if( cl.configstrings[ CS_PLAYERSKINS + i ].IsEmpty( ) )
			continue;
		Common::Com_Printf( "client %i: %s\n", i, cl.configstrings[ CS_PLAYERSKINS + i ].c_str( ) );
		Screen::SCR_UpdateScreen( );
		CL_ParseClientinfo( i );
	}
}

void Client::CL_ConnectionlessPacket( MessageBuffer & msg_buffer, netadr_t & msg_from ) {
	//Message::MSG_BeginReading( &cl_net_message );
	//Message::MSG_ReadLong( &cl_net_message ); // skip the -1
	//Str s = Message::MSG_ReadStringLine( &cl_net_message );
	Str c, s;
	bool breakParse = false;
	while( msg_buffer.GetRemaingData( ) ) {
		msg_buffer.ReadString( c );
		//Command::Cmd_TokenizeString( s, false );
		//Str c = Command::Cmd_Argv( 0 );
		Common::Com_Printf( "CLCP %s %s\n", Network::NET_AdrToString( msg_from ).c_str( ), c.c_str( ) );
		if( c == "client_connect" ) { // server connection
			if( cls.state == ca_connected )
				return; // Duplicated connects
			//Netchan::Netchan_Setup( NS_CLIENT, &cls.netchan, msg_from, cls.quakePort );
			cls.quakePort = msg_buffer.ReadShort( );
			cls.d_clientChannel.Init( msg_from, cls.quakePort );
			//Message::MSG_WriteChar( &cls.netchan.message, clc_stringcmd );
			//Message::MSG_WriteString( &cls.netchan.message, "new" );
			cl_messageBufferReliable.WriteByte( clc_stringcmd );
			cl_messageBufferReliable.WriteString( "new" );
			cls.state = ca_connected;

		} else if( c == "info" ) { // server responding to a status broadcast
			CL_ParseStatusMessage( msg_buffer );

		} else if( c == "cmd" ) { // remote command from gui front end
			if( !Network::NET_IsLocalAddress( msg_from ) ) {
				Common::Com_Printf( "Command packet from remote host.  Ignored.\n" );
			} else {
				System::Sys_AppActivate( );
				msg_buffer.ReadString( s );
				//s = Message::MSG_ReadString( &cl_net_message );
				CBuffer::Cbuf_AddText( s );
				CBuffer::Cbuf_AddText( "\n" );
			}
		} else if( c == "print" ) { // print command from somewhere
			//s = Message::MSG_ReadString( &cl_net_message );
			msg_buffer.ReadString( s );
			Common::Com_Printf( "%s", s.c_str( ) );

		} else if( c == "ping" ) { // ping from somewhere
			MessageBufferT< MAX_MSGLEN > msgBuffer;
			msgBuffer.Init( );
			msgBuffer.WriteShort( CONNECTIONLESS_MESSAGE_ID );
			msgBuffer.WriteString( "ack" );
			cls.d_clientPort.SendPacket( msg_from, msgBuffer );

		} else if( c == "challenge" ) { // challenge from the server we are connecting to
			cls.challenge = msg_buffer.ReadLong( );//atoi( Command::Cmd_Argv( 1 ) );
			CL_SendConnectPacket( msg_from );

		} else if( c == "echo" ) { // echo request from server
			MessageBufferT< MAX_MSGLEN > msgBuffer;
			msgBuffer.Init( );
			msgBuffer.WriteShort( CONNECTIONLESS_MESSAGE_ID );
			msgBuffer.WriteString( Command::Cmd_Argv( 1 ) );
			cls.d_clientPort.SendPacket( msg_from, msgBuffer );
		} else {
			if( breakParse )
				break;
			breakParse = true;
			Common::Com_Printf( "Unknown command.\n" );
		}
	}
}

void Client::CL_DumpPackets( ) {
	netadr_t		msgFrom;
	MessageBufferT< MAX_MSGLEN >	msg;
	msg.Init( );
	while( cls.d_clientPort.GetPacket( msgFrom, msg ) )
		Common::Com_Printf( "dumnping a packet\n" );
}

void Client::CL_ReadPackets( ) {
	int				msgId, msgSeq;
	netadr_t		msgFrom;
	MessageBufferT< MAX_MSGLEN >	msg;
	MessageBufferT< MAX_MSGLEN >	msgReliable;
	msg.Init( );
	msgReliable.Init( );

	while( cls.d_clientPort.GetPacket( msgFrom, msg ) ) {
		
		msgId = msg.ReadShort( );
		if( msgId == CONNECTIONLESS_MESSAGE_ID ) {
			CL_ConnectionlessPacket( msg, msgFrom );
			continue;
		}
		if( cls.state <= ca_disconnected )
			continue; // dump it if not connected
		if( msg.GetRemaingData( ) < 4 ) {
			Common::Com_Printf( "%s: %ib packet\n", Network::NET_AdrToString( msgFrom ).c_str( ), msg.GetRemaingData( ) );
			continue;
		}
		// packet from server
		if( cls.quakePort != msgId )
			continue;
		if( !cls.d_clientChannel.Process( msgFrom, cls.realtime, msg, msgSeq ) )
			continue; // wasn't accepted for some reason
		while( cls.d_clientChannel.GetReliableMessage( msgReliable ) )
			CL_ParseServerMessage( msgReliable );
		if( cls.state <= ca_connecting )
			continue;
		CL_ParseServerMessage( msg );
	}
	// check timeout
	if( cls.state >= ca_connected && cls.realtime - cls.d_clientChannel.GetLastIncomingTime( ) > cl_timeout.GetInt( ) * 1000 ) {
		if( ++cl.timeoutcount > 5 ) { // timeoutcount saves debugger
			Common::Com_Printf( "\nServer connection timed out.\n" );
			CL_Disconnect( );
			return;
		}
	} else
		cl.timeoutcount = 0;
}

void Client::CL_Userinfo_f( ) {
	Common::Com_Printf( "User info settings:\n" );
	ClientServerInfo::Info_Print( CVarSystem::Userinfo( ) );
}

void Client::CL_Snd_Restart_f( ) {
	//Sound::S_Shutdown( );
	SoundSystem::Shutdown( );
	//Sound::S_Init( );
	SoundSystem::Init( );
	CL_RegisterSounds( );
}

int precache_check; // for autodownload of precache items
int precache_spawncount;
int precache_tex;
int precache_model_skin;

File_Memory precache_model; // used for skin checking in alias models

// ENV_CNT is map load, ENV_CNT+1 is first env map

typedef struct {
	int         ident;
	int         version;
	int         skinwidth;
	int         skinheight;
	int         framesize;      // byte size of each frame

	int         num_skins;
	int         num_xyz;
	int         num_st;         // greater than num_xyz for seams
	int         num_tris;
	int         num_glcmds;     // dwords in strip/fan command list
	int         num_frames;

	int         ofs_skins;      // each skin is a MAX_SKINNAME string
	int         ofs_st;         // byte offset from start for stverts
	int         ofs_tris;       // offset for dtriangles
	int         ofs_frames;     // offset for first frame
	int         ofs_glcmds; 
	int         ofs_end;        // end of file
} dmdl_t;

#define IDALIASHEADER       (('2'<<24)+('P'<<16)+('D'<<8)+'I')
#define ALIAS_VERSION   8
#define MAX_SKINNAME    64

void Client::CL_RequestNextDownload( ) {
	unsigned map_checksum; // for detecting cheater maps
	Str fn;
	dmdl_t * pheader;
	if( cls.state != ca_connected )
		return;
	if( !CVarSystem::GetCVarBool( "allow_download" ) && precache_check <( CS_PLAYERSKINS + MAX_CLIENTS * 5 ) )
		precache_check =( CS_PLAYERSKINS + MAX_CLIENTS * 5 );
	if( precache_check == CS_MODELS ) { // confirm map
		precache_check = CS_MODELS + 2; // 0 isn't used
		if( CVarSystem::GetCVarBool( "allow_download_maps" ) && !CL_CheckOrDownloadFile( cl.configstrings[ CS_MODELS + 1 ] ) )
			return; // started a download
	}
	if( precache_check >= CS_MODELS && precache_check < CS_MODELS+MAX_MODELS ) {
		if( CVarSystem::GetCVarBool( "allow_download_models") ) {
			while( precache_check < CS_MODELS+MAX_MODELS && cl.configstrings[ precache_check ].Length( ) ) {
					if( cl.configstrings[ precache_check ][ 0 ] == '*' || cl.configstrings[ precache_check ][ 0 ] == '#' ) {
						precache_check++;
						continue;
					}
					if( precache_model_skin == 0 ) {
						if( !CL_CheckOrDownloadFile( cl.configstrings[ precache_check ] ) ) {
							precache_model_skin = 1;
							return; // started a download
						}
						precache_model_skin = 1;
					}
					// checking for skins in the model
					if( !precache_model.IsValid( ) ) {
						precache_model = FileSystem::ReadFile( cl.configstrings[ precache_check ] );
						if( !precache_model.IsValid( ) ) {
							precache_model_skin = 0;
							precache_check++;
							continue; // couldn't load it
						}
						if( LittleLong( *( unsigned * )precache_model.GetDataPtr( ) ) != IDALIASHEADER ) {
							// not an alias model
							FileSystem::FreeFile( precache_model );
							precache_model_skin = 0;
							precache_check++;
							continue;
						}
						pheader =( dmdl_t * )precache_model.GetDataPtr( );
						if( LittleLong( pheader->version ) != ALIAS_VERSION ) {
							precache_check++;
							precache_model_skin = 0;
							continue; // couldn't load it
						}
					}
					pheader =( dmdl_t * )precache_model.GetDataPtr( );
					while( precache_model_skin - 1 < LittleLong( pheader->num_skins ) ) {
						if( !CL_CheckOrDownloadFile( Str( ( char * )precache_model.GetDataPtr( ) + LittleLong( pheader->ofs_skins ) +( precache_model_skin - 1 ) * MAX_SKINNAME ) ) ) {
							precache_model_skin++;
							return; // started a download
						}
						precache_model_skin++;
					}
					if( precache_model.IsValid( ) )
						FileSystem::FreeFile( precache_model );
					precache_model_skin = 0;
					precache_check++;
			}
		}
		precache_check = CS_SOUNDS;
	}
	if( precache_check >= CS_SOUNDS && precache_check < CS_SOUNDS+MAX_SOUNDS ) {
		if( CVarSystem::GetCVarBool( "allow_download_sounds" ) ) {
			if( precache_check == CS_SOUNDS )
				precache_check++; // zero is blank
			while( precache_check < CS_SOUNDS+MAX_SOUNDS && cl.configstrings[ precache_check ].Length( ) ) {
					if( cl.configstrings[ precache_check ][ 0 ] == '*' ) {
						precache_check++;
						continue;
					}
					sprintf( fn, "sound/%s", cl.configstrings[ precache_check++ ].c_str( ) );
					if( !CL_CheckOrDownloadFile( fn ) )
						return; // started a download
			}
		}
		precache_check = CS_IMAGES;
	}
	if( precache_check >= CS_IMAGES && precache_check < CS_IMAGES+MAX_IMAGES ) {
		if( precache_check == CS_IMAGES )
			precache_check++; // zero is blank
		while( precache_check < CS_IMAGES+MAX_IMAGES && cl.configstrings[ precache_check ].Length( ) ) {			
				if( !CL_CheckOrDownloadFile( cl.configstrings[ precache_check++ ] ) )
					return; // started a download
		}
		precache_check = CS_PLAYERSKINS;
	}
	// skins are special, since a player has three things to download:
	// model, weapon model and skin
	// so precache_check is now * 3
	if( precache_check >= CS_PLAYERSKINS && precache_check < CS_PLAYERSKINS + MAX_CLIENTS * 5 ) {
		if( CVarSystem::GetCVarBool( "allow_download_players" ) ) {
			while( precache_check < CS_PLAYERSKINS + MAX_CLIENTS * 5 ) {
				Str model, skin, p;
				int i =( precache_check - CS_PLAYERSKINS )/5;
				int n =( precache_check - CS_PLAYERSKINS )%5;
				if( cl.configstrings[ CS_PLAYERSKINS+i ].IsEmpty( ) ) {
					precache_check = CS_PLAYERSKINS +( i + 1 ) * 5;
					continue;
				}
				p = strchr( cl.configstrings[ CS_PLAYERSKINS+i ], '\\' );
				if( p.Length( ) )
					p = p.Right( p.Length( ) - 1 );
				else
					p = cl.configstrings[ CS_PLAYERSKINS+i ];
				model = p;
				p = strchr( model, '/' );
				if( p.IsEmpty( ) )
					p = strchr( model, '\\' );
				if( p ) {
					//* p++ = 0;
					skin = p; // FIXMENOW
				}
				switch( n ) {
				case 0: // model
					sprintf( fn, "players/%s/tris.md2", model.c_str( ) );
					if( !CL_CheckOrDownloadFile( fn ) ) {
						precache_check = CS_PLAYERSKINS + i * 5 + 1;
						return; // started a download
					}
					n++;
					/* FALL THROUGH*/
				case 1: // weapon model
					sprintf( fn, "players/%s/weapon.md2", model.c_str( ) );
					if( !CL_CheckOrDownloadFile( fn ) ) {
						precache_check = CS_PLAYERSKINS + i * 5 + 2;
						return; // started a download
					}
					n++;
					/* FALL THROUGH*/
				case 2: // weapon skin
					sprintf( fn, "players/%s/weapon.pcx", model.c_str( ) );
					if( !CL_CheckOrDownloadFile( fn ) ) {
						precache_check = CS_PLAYERSKINS + i * 5 + 3;
						return; // started a download
					}
					n++;
					/* FALL THROUGH*/
				case 3: // skin
					sprintf( fn, "players/%s/%s.pcx", model.c_str( ), skin.c_str( ) );
					if( !CL_CheckOrDownloadFile( fn ) ) {
						precache_check = CS_PLAYERSKINS + i * 5 + 4;
						return; // started a download
					}
					n++;
					/* FALL THROUGH*/
				case 4: // skin_i
					sprintf( fn, "players/%s/%s_i.pcx", model.c_str( ), skin.c_str( ) );
					if( !CL_CheckOrDownloadFile( fn ) ) {
						precache_check = CS_PLAYERSKINS + i * 5 + 5;
						return; // started a download
					}
					// move on to next model
					precache_check = CS_PLAYERSKINS +( i + 1 ) * 5;
				}
			}
		}
		// precache phase completed
		precache_check =( CS_PLAYERSKINS + MAX_CLIENTS * 5 );
	}
	if( precache_check ==( CS_PLAYERSKINS + MAX_CLIENTS * 5 ) ) {
		precache_check =( CS_PLAYERSKINS + MAX_CLIENTS * 5 ) + 1;
		CollisionModel::CM_LoadMap( cl.configstrings[ CS_MODELS + 1 ], true, map_checksum );
		if( map_checksum != atoi( cl.configstrings[ CS_MAPCHECKSUM ] ) ) {
			Common::Com_Error( ERR_DROP, "Local map version differs from server: %i != '%s'\n", map_checksum, cl.configstrings[ CS_MAPCHECKSUM ].c_str( ) );
			return;
		}
	}

#if 0
	if( precache_check >( CS_PLAYERSKINS + MAX_CLIENTS * 5 ) && precache_check <( ( CS_PLAYERSKINS + MAX_CLIENTS * 5 )+13 ) ) {
		if( CVarSystem::GetCVarBool( "allow_download" ) && CVarSystem::GetCVarBool( "allow_download_maps" ) ) {
			while( precache_check <( ( CS_PLAYERSKINS + MAX_CLIENTS * 5 )+13 ) ) {
				int n = precache_check++ -( CS_PLAYERSKINS + MAX_CLIENTS * 5 ) - 1;
				if( n & 1 )
					sprintf( fn, "env/%s%s.pcx", cl.configstrings[ CS_SKY ].c_str( ), env_suf[ n/2 ].c_str( ) );
				else
					sprintf( fn, "env/%s%s.tga", cl.configstrings[ CS_SKY ].c_str( ), env_suf[ n/2 ].c_str( ) );
				if( !CL_CheckOrDownloadFile( fn ) )
					return; // started a download
			}
		}
		precache_check =( ( CS_PLAYERSKINS + MAX_CLIENTS * 5 )+13 );
	}
#endif

	if( precache_check ==( ( CS_PLAYERSKINS + MAX_CLIENTS * 5 )+13 ) ) {
		precache_check =( ( CS_PLAYERSKINS + MAX_CLIENTS * 5 )+13 )+1;
		precache_tex = 0;
	}
#if 0
	// confirm existance of textures, download any that don't exist
	if( precache_check == ( ( CS_PLAYERSKINS + MAX_CLIENTS * 5 ) + 13 ) + 1 ) {
		// from qcommon/cmodel.c
		extern int numtexinfo;
		extern mapsurface_t map_surfaces[];
		if( CVarSystem::GetCVarBool( "allow_download" ) && CVarSystem::GetCVarBool( "allow_download_maps" ) ) {
			while( precache_tex < numtexinfo ) {
				sprintf( fn, "textures/%s.wal", map_surfaces[ precache_tex++ ].rname );
				if( !CL_CheckOrDownloadFile( fn ) )
					return; // started a download
			}
		}
		precache_check =( ( CS_PLAYERSKINS + MAX_CLIENTS * 5 )+13 )+999;
	}
#endif
	//ZOID
	CL_RegisterSounds( );
	CL_PrepRefresh( );
	cl_messageBufferReliable.WriteByte( clc_stringcmd );
	cl_messageBufferReliable.WriteString( va( "begin %i\n", precache_spawncount ) );
	//Message::MSG_WriteByte( &cls.netchan.message, clc_stringcmd );
	//Message::MSG_WriteString( &cls.netchan.message, va( "begin %i\n", precache_spawncount ) );
}

void Client::CL_Precache_f( ) {
	precache_check = CS_MODELS;
	precache_spawncount = atoi( Command::Cmd_Argv( 1 ) );
	precache_model_skin = 0;
	CL_RequestNextDownload( );
}

void Client::CL_InitLocal( ) {
	cls.state = ca_disconnected;
	cls.realtime = System::Sys_Milliseconds( );
	CL_InitInput( );
	// register our commands
	Command::Cmd_AddCommand( "cmd", CL_ForwardToServer_f );
	Command::Cmd_AddCommand( "pause", CL_Pause_f );
	Command::Cmd_AddCommand( "pingservers", CL_PingServers_f );
	Command::Cmd_AddCommand( "skins", CL_Skins_f );
	Command::Cmd_AddCommand( "userinfo", CL_Userinfo_f );
	Command::Cmd_AddCommand( "snd_restart", CL_Snd_Restart_f );
	Command::Cmd_AddCommand( "changing", CL_Changing_f );
	Command::Cmd_AddCommand( "disconnect", CL_Disconnect_f );
	Command::Cmd_AddCommand( "record", CL_Record_f );
	Command::Cmd_AddCommand( "stop", CL_Stop_f );
	Command::Cmd_AddCommand( "quit", CL_Quit_f );
	Command::Cmd_AddCommand( "connect", CL_Connect_f );
	Command::Cmd_AddCommand( "reconnect", CL_Reconnect_f );
	//Command::Cmd_AddCommand( "packet", CL_Packet_f ); // this is dangerous to leave in
	Command::Cmd_AddCommand( "precache", CL_Precache_f );
	Command::Cmd_AddCommand( "download", CL_Download_f );
	// forward to server commands
	// the only thing this does is allow command completion
	// to work -- all unknown commands are automatically
	// forwarded to the server
	Command::Cmd_AddCommand( "wave", NULL );
	Command::Cmd_AddCommand( "inven", NULL );
	Command::Cmd_AddCommand( "kill", NULL );
	Command::Cmd_AddCommand( "use", NULL );
	Command::Cmd_AddCommand( "drop", NULL );
	Command::Cmd_AddCommand( "say", NULL );
	Command::Cmd_AddCommand( "say_team", NULL );
	Command::Cmd_AddCommand( "info", NULL );
	Command::Cmd_AddCommand( "prog", NULL );
	Command::Cmd_AddCommand( "give", NULL );
	Command::Cmd_AddCommand( "god", NULL );
	Command::Cmd_AddCommand( "notarget", NULL );
	Command::Cmd_AddCommand( "noclip", NULL );
	Command::Cmd_AddCommand( "invuse", NULL );
	Command::Cmd_AddCommand( "invprev", NULL );
	Command::Cmd_AddCommand( "invnext", NULL );
	Command::Cmd_AddCommand( "invdrop", NULL );
	Command::Cmd_AddCommand( "weapnext", NULL );
	Command::Cmd_AddCommand( "weapprev", NULL );
}

void Client::CL_WriteConfiguration( ) {
	if( cls.state == ca_uninitialized || Common::dedicated.GetBool( ) )
		return;
	FileBase * f = FileSystem::OpenFileByMode( "config.lua", FS_WRITE );
	f->Printf( "-- user CVar list\n" );
	CVarSystem::WriteCVars( f );
	f->Printf( "\n-- user key bindings\n" );
	Keyboard::Key_WriteBindings( f );
	FileSystem::CloseFile( f );
}

void Client::CL_SendCommand( ) {

	// clear prev events
	KeyEvent::Frame( );

	// get new key events
	System::Sys_SendKeyEvents( );

	// allow mice or other external controllers to add commands
	Input::IN_Commands( );

	int cmdframe = cls.framecount & CMD_BACKUP_MASK;
	cl.cmds[ cmdframe ].Clear( );
	cl.cmd_time[ cmdframe ] = 0;

	// process console commands
	CBuffer::Cbuf_Execute( );

	// fix any cheating cvars
	CVarSystem::GetCheatVars( );

	// resend a connection request if necessary
	CL_CheckForResend( );

	if( cls.d_clientChannel.GetID( ) == -1 )
		return;

	if( Common::userinfo_modified.GetBool( ) ) {
		Common::userinfo_modified.SetBool( false );
		cl_messageBufferReliable.WriteByte( clc_userinfo );
		cl_messageBufferReliable.WriteString( CVarSystem::Userinfo( ) );
	}

	// add reliable msgs if any
	if( cl_messageBufferReliable.GetSize( ) ) {
		cls.d_clientChannel.SendReliableMessage( cl_messageBufferReliable );
		cl_messageBufferReliable.BeginWriting( );
	}

	if( !cls.d_clientChannel.ReadyToSend( cls.realtime ) )
		return;

	// try send fragments from prev packets
	while( cls.d_clientChannel.UnsentFragmentsLeft( ) ) {
		cls.d_clientChannel.SendNextFragment( cls.d_clientPort, cls.realtime );
		if( !cls.d_clientChannel.ReadyToSend( cls.realtime ) )
			return;
	}

	// send intentions now
	if( cls.state == ca_active ) {
		CL_CreateCmd( cl.cmd );
		cl.cmds[ cmdframe ] = cl.cmd;
		cl.cmd_time[ cmdframe ] = cl.time;
		//GUIThread::DebugPrint( "%i %.2f %i %i %i %i\n", cls.framecount, cls.frametime, cl.time, cls.realtime, cl.cmd.msec, cl.cmd.buttons );
		CL_BuildCmd( cl_messageBuffer );
		CL_PredictMovement( );
	}
	
	//if( cls.d_clientChannel.GetIncomingSequence( ) > cls.d_clientChannel.GetOutgoingSequence( ) && cls.d_clientChannel.ReadyToSend( msgTime ) ) {
		cls.d_clientChannel.SendMessage( cls.d_clientPort, cls.realtime, cl_messageBuffer );
		cl_messageBuffer.BeginWriting( );
	//	while( cls.d_clientChannel.UnsentFragmentsLeft( ) && cls.d_clientChannel.ReadyToSend( msgTime ) )
	//		cls.d_clientChannel.SendNextFragment( cls.d_clientPort, msgTime );
	//}
}

void Client::CL_Frame( timeType msec ) {

	cls.extratime += msec;
	if( !cl_timedemo.GetBool( ) ) {
		if( cls.state == ca_connecting && cls.extratime < 100 )
			return; // don't flood packets out while connecting
		timeType ftime = ( timeType )( 1000.0f / cl_maxfps.GetFloat( ) );
		if( cls.extratime < ftime )
			return; // framerate is too high
	}
	// let the mouse activate or deactivate
	Input::IN_Frame( );
	// decide the simulation time
	cls.frametime = ( timeTypeReal )( cls.extratime / 1000.0f );
	cl.time += cls.extratime;
	cls.realtime = System::Sys_LastTime( );
	cls.extratime = 0;
	if( cls.frametime > ( 1.0f / 5 ) )
		cls.frametime = ( 1.0f / 5 );
	// if in the debugger last frame, don't timeout
	//if( msec > 5000 )
	//	cls.netchan.last_received = System::Sys_Milliseconds( );

	// fetch results from server
	CL_ReadPackets( );

	// send a new command message to the server
	CL_SendCommand( );

	// predict all unacknowledged movements
	//CL_PredictMovement( );

	//GUIThread::DebugPrint( "%s\n", cl.predicted_origin.ToString( ).c_str( ) );
	//GUIThread::DebugPrint( "%s\n", cl.predicted_angles.ToString( ).c_str( ) );
	//GUIThread::DebugPrint( "%s\n", cl.prediction_error.ToString( ).c_str( ) );
	//GUIThread::DebugPrint( "%i %.2f %i %i\n", msec, cls.frametime, cl.time, cls.realtime );

	// allow rendering DLL change
	Video::VID_CheckChanges( );

	if( !cl.refresh_prepped && cls.state == ca_active )
		CL_PrepRefresh( );

	// update the screen
	Screen::SCR_UpdateScreen( );

	// update audio
	//Sound::S_Update( cl.refdef.vieworg, cl.v_forward, cl.v_right, cl.v_up );
	SoundSystem::Frame( cl.refdef.vieworg, cl.v_forward, cl.v_right, cl.v_up );

	// advance local effects for next frame
	CL_RunDLights( );
	CL_RunLightStyles( );
	//Screen::SCR_RunCinematic( );
	Screen::SCR_RunConsole( );

	cls.framecount++;
}

void Client::CL_Init( ) {

	// all archived variables will now be loaded
	//Console::Con_Init( );

	Video::VID_Init( );
	//Sound::S_Init( ); // sound must be initialized after window is created
	SoundSystem::Init( );

	View::V_Init( );

	cl_messageBuffer.Init( );
	cl_messageBufferReliable.Init( );

	//Menu::M_Init( );

	Screen::SCR_Init( );
	cls.disable_screen = true; // don't draw yet

	CL_InitLocal( );
	Input::IN_Init( );

	//	Cbuf_AddText( "exec autoexec.cfg\n" );
	//FS_ExecAutoexec( );
	CBuffer::Cbuf_Execute( );
}

void Client::CL_Shutdown( ) {
	static bool			isdown = false;
	if( isdown ) {
		System::Sys_Error( "recursive shutdown\n" );
		return;
	}
	isdown = true;
	CL_WriteConfiguration( );
	//Sound::S_Shutdown( );
	SoundSystem::Shutdown( );
	Input::IN_Shutdown( );
	Video::VID_Shutdown( );
}
