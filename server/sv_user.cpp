#include "precompiled.h"
#pragma hdrstop

Entity *	Server::sv_player;

void Server::SV_BeginDemoserver( ) {
	Str		name;
	sprintf( name, "demos/%s", sv.name.c_str( ) );
	sv.demofile = FileSystem::OpenFileRead( name );
}

void Server::SV_New_f( MessageBuffer & msg_buffer ) {
	Common::Com_DPrintf( "New( ) from %s\n", sv_client->name.c_str( ) );
	if( sv_client->state != cs_connected ) {
		Common::Com_Printf( "New not valid -- already spawned\n" );
		return;
	}
	// demo servers just dump the file message
	if( sv.state == ss_demo ) {
		SV_BeginDemoserver( );
		return;
	}
	// serverdata needs to go over for all types of servers
	// to make sure the protocol is right, and to set the gamedir
	// send the serverdata
	//Message::MSG_WriteByte( &sv_client->netchan.message, svc_serverdata );
	//Message::MSG_WriteLong( &sv_client->netchan.message, PROTOCOL_VERSION );
	//Message::MSG_WriteLong( &sv_client->netchan.message, svs.spawncount );
	//Message::MSG_WriteByte( &sv_client->netchan.message, sv.attractloop );
	//Message::MSG_WriteString( &sv_client->netchan.message, CVarSystem::GetCVarStr( "fs_gamedir" ) );
	sv_client->d_clientBufferReliable.WriteByte( svc_serverdata );
	sv_client->d_clientBufferReliable.WriteByte( PROTOCOL_VERSION );
	sv_client->d_clientBufferReliable.WriteLong( svs.spawncount );
	sv_client->d_clientBufferReliable.WriteByte( sv.attractloop );
	sv_client->d_clientBufferReliable.WriteString( CVarSystem::GetCVarStr( "fs_gamedir" ) );
	int playernum;
	if( sv.state == ss_cinematic || sv.state == ss_pic )
		playernum = -1;
	else
		playernum = ( int )( sv_client - svs.clients );
	//Message::MSG_WriteShort( &sv_client->netchan.message, playernum );
	sv_client->d_clientBufferReliable.WriteShort( playernum );
	// send full levelname
	//Message::MSG_WriteString( &sv_client->netchan.message, sv.configstrings[ CS_NAME ] );
	sv_client->d_clientBufferReliable.WriteString( sv.configstrings[ CS_NAME ] );
	// game server
	if( sv.state == ss_game ) {
		// set up the entity for the client
		//Entity * ent = EDICT_NUM( playernum + 1 );
		//ent->s.number = playernum + 1;
		//sv_client->edict = ent;
		sv_client->lastcmd.Clear( );
		// begin fetching configstrings
		//Message::MSG_WriteByte( &sv_client->netchan.message, svc_stufftext );
		//Message::MSG_WriteString( &sv_client->netchan.message, va( "cmd configstrings %i 0\n", svs.spawncount ) );
		sv_client->d_clientBufferReliable.WriteByte( svc_stufftext );
		sv_client->d_clientBufferReliable.WriteString( va( "cmd configstrings %i 0\n", svs.spawncount ) );
	}
}

void Server::SV_Configstrings_f( MessageBuffer & msg_buffer ) {
	int			start;
	Common::Com_DPrintf( "Configstrings( ) from %s\n", sv_client->name.c_str( ) );
	if( sv_client->state != cs_connected ) {
		Common::Com_Printf( "configstrings not valid -- already spawned\n" );
		return;
	}
	// handle the case of a level changing while a client was connecting
	if( atoi( Command::Cmd_Argv( 1 ) ) != svs.spawncount ) {
	//if( msg_buffer.ReadLong( ) != svs.spawncount ) {
		Common::Com_Printf( "SV_Configstrings_f from different level\n" );
		SV_New_f( msg_buffer );
		return;
	}	
	start = atoi( Command::Cmd_Argv( 2 ) );
	// write a packet full of data
	while( sv_client->d_clientBufferReliable.GetSize( ) < sv_client->d_clientBufferReliable.GetMaxSize( ) / 2 && start < MAX_CONFIGSTRINGS ) {
		if( sv.configstrings[ start ].Length( ) ) {
			sv_client->d_clientBufferReliable.WriteByte( svc_configstring );
			sv_client->d_clientBufferReliable.WriteShort( start );
			sv_client->d_clientBufferReliable.WriteString( sv.configstrings[ start ] );
		}
		start++;
	}
	// send next command
	if( start == MAX_CONFIGSTRINGS ) {
		sv_client->d_clientBufferReliable.WriteByte( svc_stufftext );
		sv_client->d_clientBufferReliable.WriteString( va( "cmd baselines %i 0\n", svs.spawncount ) );
	} else {
		sv_client->d_clientBufferReliable.WriteByte( svc_stufftext );
		sv_client->d_clientBufferReliable.WriteString( va( "cmd configstrings %i %i\n", svs.spawncount, start ) );
	}
}

void Server::SV_Baselines_f( MessageBuffer & msg_buffer ) {
	int		start;
	entity_state_t	nullstate;
	entity_state_t	* base;
	Common::Com_DPrintf( "Baselines( ) from %s\n", sv_client->name.c_str( ) );
	if( sv_client->state != cs_connected ) {
		Common::Com_Printf( "baselines not valid -- already spawned\n" );
		return;
	}	
	// handle the case of a level changing while a client was connecting
	if( atoi( Command::Cmd_Argv( 1 ) ) != svs.spawncount ) {
		Common::Com_Printf( "SV_Baselines_f from different level\n" );
		SV_New_f( msg_buffer );
		return;
	}	
	start = atoi( Command::Cmd_Argv( 2 ) );
	nullstate.Clear( );
	// write a packet full of data
	while( sv_client->d_clientBufferReliable.GetSize( ) < sv_client->d_clientBufferReliable.GetMaxSize( ) / 2 && start < MAX_EDICTS ) {
		base = &sv.baselines[ start ];
		if( base->modelindex || base->sound || base->effects ) {
			sv_client->d_clientBufferReliable.WriteByte( svc_spawnbaseline );
			SV_WriteDeltaEntity( &nullstate, base, sv_client->d_clientBufferReliable, true, true );
		}
		start++;
	}
	// send next command
	if( start == MAX_EDICTS ) {
		sv_client->d_clientBufferReliable.WriteByte( svc_stufftext );
		sv_client->d_clientBufferReliable.WriteString( va( "precache %i\n", svs.spawncount ) );
	} else {
		sv_client->d_clientBufferReliable.WriteByte( svc_stufftext );
		sv_client->d_clientBufferReliable.WriteString( va( "cmd baselines %i %i\n", svs.spawncount, start ) );
	}
}

void Server::SV_Begin_f( MessageBuffer & msg_buffer ) {
	Common::Com_DPrintf( "Begin( ) from %s\n", sv_client->name.c_str( ) );
	// handle the case of a level changing while a client was connecting
	if( atoi( Command::Cmd_Argv( 1 ) ) != svs.spawncount ) {
		Common::Com_Printf( "SV_Begin_f from different level\n" );
		SV_New_f( msg_buffer );
		return;
	}
	sv_client->state = cs_spawned;	
	// call the game begin function
	Game::ClientBegin( sv_player );
	CBuffer::Cbuf_InsertFromDefer( );
}

void Server::SV_NextDownload_f( MessageBuffer & msg_buffer ) {
	if( !sv_client->download.IsValid( ) )
		return;
	size_t r = sv_client->downloadsize - sv_client->downloadcount;
	if( r > 1024 )
		r = 1024;
	sv_client->d_clientBufferReliable.WriteByte( svc_download );
	sv_client->d_clientBufferReliable.WriteShort( ( int )r );
	sv_client->downloadcount += r;
	size_t size = sv_client->downloadsize;
	if( !size )
		size = 1;
	size_t percent = sv_client->downloadcount * 100 / size;
	sv_client->d_clientBufferReliable.WriteByte( ( int )percent );
	sv_client->d_clientBufferReliable.WriteData( ( ( byte * )sv_client->download.GetDataPtr( ) ) + sv_client->downloadcount - r, ( int )r );
	if( sv_client->downloadcount != sv_client->downloadsize )
		return;
	FileSystem::FreeFile( sv_client->download );
}

void Server::SV_BeginDownload_f( MessageBuffer & msg_buffer ) {
	Str		name;
	size_t offset = 0;
	//msg_buffer.ReadString( name );
	//size_t offset = msg_buffer.ReadLong( );
	name = Command::Cmd_Argv( 1 );
	if( Command::Cmd_Argc( ) > 2 )
		offset = atoi( Command::Cmd_Argv( 2 ) ); // downloaded offset
	// hacked by zoid to allow more conrol over download
	// first off, no .. or global allow check
	if( name.Find( ".." ) != -1 || !allow_download.GetBool( )
		// leading dot is no good
		|| name[ 0 ] == '.' 
		// leading slash bad as well, must be in subdir
		|| name[ 0 ] == '/'
		// next up, skin check
		||( Str::Cmpn( name, "players/", 6 ) == 0 && !allow_download_players.GetBool( ) )
		// now models
		||( Str::Cmpn( name, "models/", 6 ) == 0 && !allow_download_models.GetBool( ) )
		// now sounds
		||( Str::Cmpn( name, "sound/", 6 ) == 0 && !allow_download_sounds.GetBool( ) )
		// now maps( note special case for maps, must not be in pak )
		||( Str::Cmpn( name, "maps/", 6 ) == 0 && !allow_download_maps.GetBool( ) )
		// MUST be in a subdirectory	
		|| name.Find( '/' ) == -1 )	
	{	// don't allow anything with .. path
		sv_client->d_clientBufferReliable.WriteByte( svc_download );
		sv_client->d_clientBufferReliable.WriteShort( -1 );
		sv_client->d_clientBufferReliable.WriteByte( 0 );
		return;
	}
	if( sv_client->download.IsValid( ) )
		FileSystem::FreeFile( sv_client->download );
	sv_client->download = FileSystem::ReadFile( name );
	sv_client->downloadsize = sv_client->download.Length( );
	sv_client->downloadcount = ( int )offset;
	if( offset > sv_client->downloadsize )
		sv_client->downloadcount = ( int )sv_client->downloadsize;
	if( !sv_client->download.IsValid( )
		// special check for maps, if it came from a pak file, don't allow
		// download  ZOID
		|| ( Str::Cmpn( name, "maps/", 5 ) == 0 ) ) {
		Common::Com_DPrintf( "Couldn't download %s to %s\n", name.c_str( ), sv_client->name.c_str( ) );
		if( sv_client->download.IsValid( ) )
			FileSystem::FreeFile( sv_client->download );
		sv_client->d_clientBufferReliable.WriteByte( svc_download );
		sv_client->d_clientBufferReliable.WriteShort( -1 );
		sv_client->d_clientBufferReliable.WriteByte( 0 );
		return;
	}
	SV_NextDownload_f( msg_buffer );
	Common::Com_DPrintf( "Downloading %s to %s\n", name.c_str( ), sv_client->name.c_str( ) );
}



//============================================================================


/*
=================
SV_Disconnect_f

The client is going to disconnect, so remove the connection immediately
=================
*/
void Server::SV_Disconnect_f( MessageBuffer & msg_buffer ) {

//	SV_EndRedirect( );
	SV_DropClient( sv_client );	
}


/*
==================
SV_ShowServerinfo_f

Dumps the serverinfo info string
==================
*/
void Server::SV_ShowServerinfo_f( MessageBuffer & msg_buffer ) {

	ClientServerInfo::Info_Print( CVarSystem::Serverinfo( ) );
}

void Server::SV_Nextserver( ) {
	Str v;
	//ZOID, ss_pic can be nextserver'd in coop mode
	if( sv.state == ss_game ||( sv.state == ss_pic && !CVarSystem::GetCVarBool( "coop" ) ) )
		return;		// can't nextserver while playing a normal game
	svs.spawncount++;	// make sure another doesn't sneak in
	v = CVarSystem::GetCVarStr( "nextserver" );
	if( v.IsEmpty( ) )
		CBuffer::Cbuf_AddText( "killserver\n" );
	else {
		CBuffer::Cbuf_AddText( v );
		CBuffer::Cbuf_AddText( "\n" );
	}
	CVarSystem::SetCVarStr( "nextserver", "" );
}

void Server::SV_Nextserver_f( MessageBuffer & msg_buffer ) {
	if( atoi( Command::Cmd_Argv( 1 ) ) != svs.spawncount ) {
		Common::Com_DPrintf( "Nextserver( ) from wrong level, from %s\n", sv_client->name.c_str( ) );
		return;		// leftover from last server
	}
	Common::Com_DPrintf( "Nextserver( ) from %s\n", sv_client->name.c_str( ) );
	SV_Nextserver( );
}

void Server::SV_ExecuteUserCommand( const Str & uc, MessageBuffer & msg_buffer ) {	
	Command::Cmd_TokenizeString( uc, true );
	sv_player = sv_client->edict;
//	SV_BeginRedirect( RD_CLIENT );
	const ucmd_t * u;
	for( u = ucmds; u->d_name.Length( ); u++ ) {
		if( Command::Cmd_Argv( 0 ) == u->d_name ) {
			u->d_func( msg_buffer );
			break;
		}
	}
	if( u->d_name.IsEmpty( ) && sv.state == ss_game )
		Game::ClientCommand( sv_player );
//	SV_EndRedirect( );
}

/*
===========================================================================

USER CMD EXECUTION

===========================================================================
*/



void Server::SV_ClientThink( client_t * cl, usercmd_t * cmd ) {

	cl->commandMsec -= cmd->msec;

	if( cl->commandMsec < 0 && sv_enforcetime.GetBool( ) )
	{
		Common::Com_DPrintf( "commandMsec underflow from %s\n", cl->name.c_str( ) );
		return;
	}

	Game::ClientThink( cl->edict, cmd );
}

#define	MAX_STRINGCMDS	8

void SV_ReadDeltaUsercmd( MessageBuffer & msg_buffer, usercmd_t & from, usercmd_t & cmd ) {
#if 0
	//Common::Com_Memcpy( &move, &from, sizeof( usercmd_t ) );
	move = from;
	int bits			= msg_buffer.ReadByte( );		
	// read current angles
	if( bits & CM_ANGLE1 )
		move.angles[ 0 ]	= msg_buffer.ReadShort( );
	if( bits & CM_ANGLE2 )
		move.angles[ 1 ]	= msg_buffer.ReadShort( );
	if( bits & CM_ANGLE3 )
		move.angles[ 2 ]	= msg_buffer.ReadShort( );		
	// read movement
	if( bits & CM_FORWARD )
		move.forwardmove	= msg_buffer.ReadShort( );
	if( bits & CM_SIDE )
		move.sidemove		= msg_buffer.ReadShort( );
	if( bits & CM_UP )	
		move.upmove			= msg_buffer.ReadShort( );	
	// read buttons
	if( bits & CM_BUTTONS )
		move.buttons		= msg_buffer.ReadByte( );
	if( bits & CM_IMPULSE )
		move.impulse		= msg_buffer.ReadByte( );
	// read time to run command
	move.msec			= msg_buffer.ReadByte( );
	// read the light level
	move.lightlevel		= msg_buffer.ReadByte( );
#endif
	cmd.angles[ 0 ] = msg_buffer.ReadDeltaShort( from.angles[ 0 ] );
	cmd.angles[ 1 ] = msg_buffer.ReadDeltaShort( from.angles[ 1 ] );
	cmd.angles[ 2 ] = msg_buffer.ReadDeltaShort( from.angles[ 2 ] );	
	cmd.forwardmove = msg_buffer.ReadDeltaShort( from.forwardmove );
	cmd.sidemove = msg_buffer.ReadDeltaShort( from.sidemove );
	cmd.upmove = msg_buffer.ReadDeltaShort( from.upmove );
	cmd.buttons = msg_buffer.ReadDeltaByte( from.buttons );
	cmd.impulse = msg_buffer.ReadDeltaByte( from.impulse );
	cmd.msec = msg_buffer.ReadDeltaByte( from.msec );
	cmd.lightlevel = msg_buffer.ReadDeltaByte( from.lightlevel );
}

void SV_ReadUsercmd( MessageBuffer & msg_buffer, usercmd_t & cmd ) {
	cmd.angles[ 0 ] = msg_buffer.ReadDeltaShort( 0 );
	cmd.angles[ 1 ] = msg_buffer.ReadDeltaShort( 0 );
	cmd.angles[ 2 ] = msg_buffer.ReadDeltaShort( 0 );	
	cmd.forwardmove = msg_buffer.ReadDeltaShort( 0 );
	cmd.sidemove = msg_buffer.ReadDeltaShort( 0 );
	cmd.upmove = msg_buffer.ReadDeltaShort( 0 );
	cmd.buttons = msg_buffer.ReadDeltaByte( 0 );
	cmd.impulse = msg_buffer.ReadDeltaByte( 0 );
	cmd.msec = msg_buffer.ReadDeltaByte( 0 );
	cmd.lightlevel = msg_buffer.ReadDeltaByte( 0 );
}

void Server::SV_ExecuteClientMessage( client_t * cl, MessageBuffer & msg_buffer ) {
	Str			s;
	usercmd_t	nullcmd, oldest, oldcmd, newcmd;
	int			c, net_drop, checksum, calculatedChecksum, lastframe, oldframe;
	size_t		checksumIndex;
	sv_client = cl;
	sv_player = sv_client->edict;
	// only allow one move command
	bool move_issued = false;
	int stringCmdCount = 0;
	while( 1 ) {
		if( msg_buffer.GetReadCount( ) > msg_buffer.GetSize( ) ) {
			Common::Com_Printf( "SV_ReadClientMessage: badread\n" );
			SV_DropClient( cl );
			return;
		}
		if( !msg_buffer.GetRemaingData( ) )
			break;
		c = msg_buffer.ReadByte( );	
		switch( c ) {
			default:
				Common::Com_Printf( "SV_ReadClientMessage: unknown command #%i\n", c );
				SV_DropClient( cl );
				return;						
			case clc_nop:
				break;
			case clc_userinfo:
				msg_buffer.ReadString( cl->userinfo );
				SV_UserinfoChanged( cl );
				break;
			case clc_move:
				if( move_issued )
					return;		// someone is trying to cheat...
				move_issued = true;
				msg_buffer.ReadByteAlign( );
				checksumIndex = msg_buffer.GetReadCount( );
				checksum = msg_buffer.ReadByte( );
				lastframe = msg_buffer.ReadLong( );
				oldframe = -1;
				if( lastframe != cl->lastframe ) {
					oldframe = cl->lastframe;
					cl->lastframe = lastframe;
					if( cl->lastframe > 0 )
						cl->frame_latency[ cl->lastframe & ( LATENCY_COUNTS - 1 ) ] = svs.realtime - cl->frames[ cl->lastframe & UPDATE_MASK ].senttime;
				}
				SV_ReadUsercmd( msg_buffer, oldest );
				SV_ReadDeltaUsercmd( msg_buffer, oldest, oldcmd );
				SV_ReadDeltaUsercmd( msg_buffer, oldcmd, newcmd );
				msg_buffer.ReadByteAlign( );
				//nullcmd.Clear( );
				//SV_ReadDeltaUsercmd( msg_buffer, nullcmd, oldest );
				//SV_ReadDeltaUsercmd( msg_buffer, oldest, oldcmd );
				//SV_ReadDeltaUsercmd( msg_buffer, oldcmd, newcmd );
				if( cl->state != cs_spawned ) {
					cl->lastframe = -1;
					break;
				}
				// if the checksum fails, ignore the rest of the packet
				calculatedChecksum = Common::COM_BlockSequenceCRCByte( msg_buffer.GetData( ) + checksumIndex + 1, msg_buffer.GetReadCount( ) - checksumIndex - 1, cl->lastframe );
				if( calculatedChecksum != checksum ) {
					Common::Com_DPrintf( "CRC error! Client: \"%s\" Frame: %d\n", cl->name.c_str( ), cl->lastframe );
					return;
				}
				if( !Common::paused.GetBool( ) ) {
					net_drop = ( oldframe == -1 ) ? 0 : ( ( lastframe - oldframe ) - 1 );
					if( net_drop < 20 ) {
						while( net_drop > 2 ) {
							SV_ClientThink( cl, &cl->lastcmd );
							net_drop--;
						}
						if( net_drop > 1 )
							SV_ClientThink( cl, &oldest );
						if( net_drop > 0 )
							SV_ClientThink( cl, &oldcmd );
					}
					SV_ClientThink( cl, &newcmd );
				}
				cl->lastcmd = newcmd;
				break;
			case clc_stringcmd:	
				msg_buffer.ReadString( s );
				// malicious users may try using too many string commands
				if( ++stringCmdCount < MAX_STRINGCMDS )
					SV_ExecuteUserCommand( s, msg_buffer );
				if( cl->state == cs_zombie )
					return;	// disconnect command
				break;
		}
	}
}
