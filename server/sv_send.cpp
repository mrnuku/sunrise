#include "precompiled.h"
#pragma hdrstop

void Server::SV_ClientPrintf( client_t * cl, int level, const char * fmt, ... ) {
	va_list		argptr;
	Str			string;	
	if( level < cl->messagelevel )
		return;	
	va_start( argptr, fmt );
	vsprintf( string, fmt, argptr );
	va_end( argptr );	
	//Message::MSG_WriteByte( &cl->netchan.message, svc_print );
	//Message::MSG_WriteByte( &cl->netchan.message, level );
	//Message::MSG_WriteString( &cl->netchan.message, string );
	cl->d_clientBufferReliable.WriteByte( svc_print );
	cl->d_clientBufferReliable.WriteByte( level );
	cl->d_clientBufferReliable.WriteString( string );
}

void Server::SV_BroadcastPrintf( int level, const char * fmt, ... ) {
	va_list		argptr;
	Str			string;
	client_t *	cl = svs.clients;
	va_start( argptr, fmt );
	vsprintf( string, fmt, argptr );
	va_end( argptr );	
	// echo to console
	if( Common::dedicated.GetBool( ) )
		Common::Com_Printf( string );
	for( int i = 0; i < Common::maxclients.GetInt( ); i++, cl++ ) {
		if( cl->state != cs_spawned || level < cl->messagelevel )
			continue;
		//Message::MSG_WriteByte( &cl->netchan.message, svc_print );
		//Message::MSG_WriteByte( &cl->netchan.message, level );
		//Message::MSG_WriteString( &cl->netchan.message, string );
		cl->d_clientBufferReliable.WriteByte( svc_print );
		cl->d_clientBufferReliable.WriteByte( level );
		cl->d_clientBufferReliable.WriteString( string );
	}
}

void Server::SV_BroadcastCommand( const char * fmt, ... ) {
	va_list		argptr;
	Str			string;	
	if( !sv.state )
		return;
	va_start( argptr, fmt );
	vsprintf( string, fmt, argptr );
	va_end( argptr );
	//Message::MSG_WriteByte( &sv.multicast, svc_stufftext );
	//Message::MSG_WriteString( &sv.multicast, string );
	sv.d_multicastBuffer.WriteByte( svc_stufftext );
	sv.d_multicastBuffer.WriteString( string );
	SV_Multicast( vec3_origin, MULTICAST_ALL_R );
}

void Server::SV_Multicast( Vec3 & origin, multicast_t to ) {
	client_t *	client = svs.clients;
	byte *		mask;
	int			leafnum = 0, cluster;
	bool		reliable = false;
	int			area1 = 0, area2;
	if( to != MULTICAST_ALL_R && to != MULTICAST_ALL ) {
		leafnum = CollisionModel::CM_PointLeafnum( origin );
		area1 = CollisionModel::CM_LeafArea( leafnum );
	}
	// if doing a serverrecord, store everything
	if( svs.demofile )
		svs.d_demoBuffer.WriteData( sv.d_multicastBuffer.GetData( ), sv.d_multicastBuffer.GetSize( ) );
		//Message::SZ_Write( &svs.demo_multicast, sv.multicast.data, sv.multicast.cursize );	
	switch( to ) {
		case MULTICAST_ALL_R:
			reliable = true;	// intentional fallthrough
		case MULTICAST_ALL:
			leafnum = 0;
			mask = NULL;
			break;
		case MULTICAST_PHS_R:
			reliable = true;	// intentional fallthrough
		case MULTICAST_PHS:
			leafnum = CollisionModel::CM_PointLeafnum( origin );
			cluster = CollisionModel::CM_LeafCluster( leafnum );
			mask = CollisionModel::CM_ClusterPHS( cluster );
			break;
		case MULTICAST_PVS_R:
			reliable = true;	// intentional fallthrough
		case MULTICAST_PVS:
			leafnum = CollisionModel::CM_PointLeafnum( origin );
			cluster = CollisionModel::CM_LeafCluster( leafnum );
			mask = CollisionModel::CM_ClusterPVS( cluster );
			break;
		default:
			mask = NULL;
			Common::Com_Error( ERR_FATAL, "SV_Multicast: bad to:%i", to );
	}
	// send the data to all relevent clients
	for( int j = 0; j < Common::maxclients.GetInt( ); j++, client++ ) {
		if( client->state == cs_free || client->state == cs_zombie )
			continue;
		if( client->state != cs_spawned && !reliable )
			continue;
		if( mask ) {
			leafnum = CollisionModel::CM_PointLeafnum( client->edict->s.origin );
			cluster = CollisionModel::CM_LeafCluster( leafnum );
			area2 = CollisionModel::CM_LeafArea( leafnum );
			if( !CollisionModel::CM_AreasConnected( area1, area2 ) )
				continue;
			if( mask &&( !( mask[ cluster >> 3 ] &( 1 <<( cluster & 7 ) ) ) ) )
				continue;
		}
		if( reliable )
			client->d_clientBufferReliable.WriteData( sv.d_multicastBuffer.GetData( ), sv.d_multicastBuffer.GetSize( ) );
			//Message::SZ_Write( &client->netchan.message, sv.multicast.data, sv.multicast.cursize );
		else
			client->d_clientBuffer.WriteData( sv.d_multicastBuffer.GetData( ), sv.d_multicastBuffer.GetSize( ) );
			//Message::SZ_Write( &client->datagram, sv.multicast.data, sv.multicast.cursize );
	}
	//Message::SZ_Clear( &sv.multicast );
	sv.d_multicastBuffer.BeginWriting( );
}

void Server::SV_StartSound( Vec3 * origin, Entity * entity, int channel, int soundindex, float volume, float attenuation, float timeofs ) {      
    int			flags = 0;
	int			ent = NUM_FOR_EDICT( entity );
	Vec3		origin_v;
	bool		use_phs = true;
	if( volume < 0 || volume > 1.0f )
		Common::Com_Error( ERR_FATAL, "SV_StartSound: volume = %f", volume );
	if( attenuation < 0 || attenuation > 4 )
		Common::Com_Error( ERR_FATAL, "SV_StartSound: attenuation = %f", attenuation );
	if( timeofs < 0 || timeofs > 0.255f )
		Common::Com_Error( ERR_FATAL, "SV_StartSound: timeofs = %f", timeofs );
	if( channel & 8 ) { // no PHS flag
		use_phs = false;
		channel &= 7;
	}
	int sendchan = ( ent << 3 ) | ( channel & 7 );
	if( volume != DEFAULT_SOUND_PACKET_VOLUME )
		flags |= SND_VOLUME;
	if( attenuation != DEFAULT_SOUND_PACKET_ATTENUATION )
		flags |= SND_ATTENUATION;
	// the client doesn't know that bmodels have weird origins
	// the origin can also be explicitly set
	if( ( entity->svflags & SVF_NOCLIENT ) ||( entity->solid == SOLID_BSP ) || origin )
		flags |= SND_POS;
	// always send the entity number for channel overrides
	flags |= SND_ENT;
	if( timeofs )
		flags |= SND_OFFSET;
	// use the entity origin unless it is a bmodel or explicitly specified
	if( !origin ) {
		if( entity->solid == SOLID_BSP ) {
			for( int i = 0; i < 3; i++ )
				origin_v[ i ] = entity->s.origin[ i ] + 0.5f * ( entity->mins[ i ] + entity->maxs[ i ] );
		} else
			origin_v = entity->s.origin;
	} else
		origin_v = *origin;
	//Message::MSG_WriteByte( &sv.multicast, svc_sound );
	//Message::MSG_WriteByte( &sv.multicast, flags );
	//Message::MSG_WriteByte( &sv.multicast, soundindex );
	sv.d_multicastBuffer.WriteByte( svc_sound );
	sv.d_multicastBuffer.WriteByte( flags );
	sv.d_multicastBuffer.WriteByte( soundindex );
	if( flags & SND_VOLUME )
		sv.d_multicastBuffer.WriteByte( ( int )volume * 255 );
		//Message::MSG_WriteByte( &sv.multicast, ( int )volume * 255 );
	if( flags & SND_ATTENUATION )
		sv.d_multicastBuffer.WriteByte( ( int )attenuation * 64 );
		//Message::MSG_WriteByte( &sv.multicast, ( int )attenuation * 64 );
	if( flags & SND_OFFSET )
		sv.d_multicastBuffer.WriteByte( ( int )timeofs * 1000 );
		//Message::MSG_WriteByte( &sv.multicast, ( int )timeofs * 1000 );
	if( flags & SND_ENT )
		sv.d_multicastBuffer.WriteShort( sendchan );
		//Message::MSG_WriteShort( &sv.multicast, sendchan );
	if( flags & SND_POS )
		sv.d_multicastBuffer.WriteVec3( origin_v );
		//Message::MSG_WritePos( &sv.multicast, origin_v );
	// if the sound doesn't attenuate, send it to everyone
	//( global radio chatter, voiceovers, etc )
	if( attenuation == ATTN_NONE )
		use_phs = false;
	if( channel & CHAN_RELIABLE )
		SV_Multicast( origin_v, use_phs ? MULTICAST_PHS_R : MULTICAST_ALL_R );
	else
		SV_Multicast( origin_v, use_phs ? MULTICAST_PHS : MULTICAST_ALL );
}

bool Server::SV_SendClientDatagram( client_t * client ) {
	//byte		msg_buf[ MAX_MSGLEN ];
	//sizebuf_t	msg;
	MessageBufferT< MAX_MSGLEN >	msgBuffer;
	if( client->d_clientBufferReliable.GetSize( ) ) {
		client->d_clientChannel.SendReliableMessage( client->d_clientBufferReliable );
		client->d_clientBufferReliable.BeginWriting( );
	}
	if( !client->d_clientChannel.ReadyToSend( svs.realtime ) )
		return false;
	while( client->d_clientChannel.UnsentFragmentsLeft( ) ) {
		client->d_clientChannel.SendNextFragment( svs.d_serverPort, svs.realtime );
		if( !client->d_clientChannel.ReadyToSend( svs.realtime ) )
			return false;
	}
	msgBuffer.Init( );
	msgBuffer.SetAllowOverflow( true );
	SV_BuildClientFrame( client );
	//Message::SZ_Init( &msg, msg_buf, sizeof( msg_buf ) );
	//msg.allowoverflow = true;
	// send over all the relevant entity_state_t
	// and the player_state_t
	SV_WriteFrameToClient( client, msgBuffer );
	// copy the accumulated multicast datagram
	// for this client out to the message
	// it is necessary for this to be after the WriteEntities
	// so that entity references will be current
	if( msgBuffer.IsOverflowed( ) )
		Common::Com_Printf( "WARNING: datagram overflowed for %s\n", client->name.c_str( ) );
	else
		msgBuffer.WriteData( client->d_clientBuffer.GetData( ), client->d_clientBuffer.GetSize( ) );
		//Message::SZ_Write( &msg, client->datagram.data, client->datagram.cursize );
	//Message::SZ_Clear( &client->datagram );
	client->d_clientBuffer.BeginWriting( );
	if( msgBuffer.IsOverflowed( ) ) {	// must have room left for the packet header
		Common::Com_Printf( "WARNING: msg overflowed for %s\n", client->name.c_str( ) );
		//Message::SZ_Clear( &msg );
		msgBuffer.BeginWriting( );
	}
	// send the datagram
	client->d_clientChannel.SendMessage( svs.d_serverPort, svs.realtime, msgBuffer );
	while( client->d_clientChannel.UnsentFragmentsLeft( ) ) {
		if( !client->d_clientChannel.ReadyToSend( svs.realtime ) )
			return false;
		client->d_clientChannel.SendNextFragment( svs.d_serverPort, svs.realtime );
	}
	//Netchan::Netchan_Transmit( &client->netchan, msg.cursize, msg.data );
	// record the size for rate estimation
	//client->message_size[ sv.framenum % RATE_MESSAGES ] = msg.cursize;
	return true;
}

void Server::SV_DemoCompleted( ) {
	if( sv.demofile )
		FileSystem::CloseFile( sv.demofile );
	SV_Nextserver( );
}

bool Server::SV_RateDrop( client_t * c ) {
#if 0
	size_t		total = 0;
	// never drop over the loopback
	if( c->d_clientChannel.GetRemoteAddress( ).type == NA_LOOPBACK )
		return false;
	for( int i = 0; i < RATE_MESSAGES; i++ )
		total += c->message_size[ i ];
	if( total > c->rate ) {
		c->surpressCount++;
		c->message_size[ sv.framenum % RATE_MESSAGES ] = 0;
		return true;
	}
#endif
	return false;
}

void Server::SV_SendClientMessages( ) {
	int			i;
	client_t *	c;
	int			msglen = 0;
	//byte		msgbuf[ MAX_MSGLEN ];
	MessageBufferT< MAX_MSGLEN >	msgBuffer;
	size_t		r;
	// read the next demo message if needed
	if( sv.state == ss_demo && sv.demofile ) {
		if( Common::paused.GetBool( ) )
			msglen = 0;
		else {
			// get the next message
			r = sv.demofile->Read( &msglen, 4 );
			if( r != 1 ) {
				SV_DemoCompleted( );
				return;
			}
			msglen = LittleLong( msglen );
			if( msglen == -1 ) {
				SV_DemoCompleted( );
				return;
			}
			if( msglen > MAX_MSGLEN )
				Common::Com_Error( ERR_DROP, "SV_SendClientMessages: msglen > MAX_MSGLEN" );
			msgBuffer.SetSize( msglen );
			r = sv.demofile->Read( msgBuffer.GetData( ), msglen );
			if( r != 1 ) {
				SV_DemoCompleted( );
				return;
			}
		}
	}
	// send a message to each connected client
	for( i = 0, c = svs.clients; i < Common::maxclients.GetInt( ); i++, c++ ) {
		if( c->state == cs_free )
			continue;
		// if the reliable message overflowed, // drop the client
		//if( c->netchan.message.overflowed ) {
		if( c->d_clientBufferReliable.IsOverflowed( ) ) {
			//Message::SZ_Clear( &c->netchan.message );
			//Message::SZ_Clear( &c->datagram );
			c->d_clientBuffer.BeginWriting( );
			c->d_clientBufferReliable.BeginWriting( );
			SV_BroadcastPrintf( PRINT_HIGH, "%s overflowed\n", c->name.c_str( ) );
			SV_DropClient( c );
		}
		if( sv.state == ss_cinematic || sv.state == ss_demo || sv.state == ss_pic )
			c->d_clientChannel.SendMessage( svs.d_serverPort, svs.realtime, msgBuffer );
			//Netchan::Netchan_Transmit( &c->netchan, msglen, msgbuf );
		else if( c->state == cs_spawned ) {
			// don't overrun bandwidth
			//if( SV_RateDrop( c ) )
			//	continue;
			SV_SendClientDatagram( c );
		} else {
			// just update reliable	if needed
			//if( c->netchan.message.cursize || System::Sys_LastTime( ) - c->netchan.last_sent > 1000 )
			//	Netchan::Netchan_Transmit( &c->netchan, 0, NULL );
			if( c->d_clientBufferReliable.GetSize( ) ) {
				c->d_clientChannel.SendReliableMessage( c->d_clientBufferReliable );
				c->d_clientBufferReliable.BeginWriting( );
			}
			if( c->d_clientChannel.ReadyToSend( svs.realtime ) ) {
				c->d_clientChannel.SendMessage( svs.d_serverPort, svs.realtime, c->d_clientBufferReliable );
			}
		}
	}
}
