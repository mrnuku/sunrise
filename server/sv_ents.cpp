#include "precompiled.h"
#pragma hdrstop

byte		fatpvs[ 65536/8 ];	// 32767 is MAX_MAP_LEAFS

void Server::SV_FatPVS( Vec3 & org ) {
#if 0
	int		leafs[ 64 ];
	int		i, j, count;
	int		longs;
	byte	* src;
	Vec3	mins, maxs;
	for( i = 0; i < 3; i++ ) {
		mins[ i ] = org[ i ] - 8;
		maxs[ i ] = org[ i ] + 8;
	}
	count = CollisionModel::CM_BoxLeafnums( mins, maxs, leafs, 64, NULL );
	if( count < 1 )
		Common::Com_Error( ERR_FATAL, "SV_FatPVS: count < 1" );
	longs =( CollisionModel::CM_NumClusters( ) + 31 ) >> 5;
	// convert leafs to clusters
	for( i = 0; i < count; i++ )
		leafs[ i ] = CollisionModel::CM_LeafCluster( leafs[ i ] );
	memcpy( fatpvs, CollisionModel::CM_ClusterPVS( leafs[ 0 ] ), longs << 2 );
	// or in all the other leaf bits
	for( i = 1; i < count; i++ ) {
		for( j = 0; j < i; j++ ) {
			if( leafs[ i ] == leafs[ j ] )
				break;
		}
		if( j != i )
			continue;		// already have the cluster we want
		src = CollisionModel::CM_ClusterPVS( leafs[ i ] );
		for( j = 0; j < longs; j++ )
			( ( long * )fatpvs )[ j ] |=( ( long * )src )[ j ];
	}
#endif
}

void Server::SV_WriteDeltaEntity( entity_state_t * from, entity_state_t * to, MessageBuffer & msg_buffer, bool force, bool newentity ) {
	if( !to->number )
		Common::Com_Error( ERR_FATAL, "Unset entity number" );
	if( to->number >= MAX_EDICTS )
		Common::Com_Error( ERR_FATAL, "Entity number >= MAX_EDICTS" );
	// send an update
	int bits = 0;
	if( to->number >= 256 )
		bits |= U_NUMBER16;		// number8 is implicit otherwise
	if( to->origin[ 0 ] != from->origin[ 0 ] )
		bits |= U_ORIGIN1;
	if( to->origin[ 1 ] != from->origin[ 1 ] )
		bits |= U_ORIGIN2;
	if( to->origin[ 2 ] != from->origin[ 2 ] )
		bits |= U_ORIGIN3;
	if( to->angles[ 0 ] != from->angles[ 0 ] )
		bits |= U_ANGLE1;		
	if( to->angles[ 1 ] != from->angles[ 1 ] )
		bits |= U_ANGLE2;
	if( to->angles[ 2 ] != from->angles[ 2 ] )
		bits |= U_ANGLE3;		
	if( to->skinnum != from->skinnum ) {
		if( to->skinnum < 256 )
			bits |= U_SKIN8;
		else if( to->skinnum < 0x10000 )
			bits |= U_SKIN16;
		else
			bits |=( U_SKIN8 | U_SKIN16 );
	}		
	if( to->frame != from->frame ) {
		if( to->frame < 256 )
			bits |= U_FRAME8;
		else
			bits |= U_FRAME16;
	}
	if( to->effects != from->effects ) {
		if( to->effects < 256 )
			bits |= U_EFFECTS8;
		else if( to->effects < 0x8000 )
			bits |= U_EFFECTS16;
		else
			bits |= U_EFFECTS8 | U_EFFECTS16;
	}	
	if( to->renderfx != from->renderfx ) {
		if( to->renderfx < 256 )
			bits |= U_RENDERFX8;
		else if( to->renderfx < 0x8000 )
			bits |= U_RENDERFX16;
		else
			bits |= U_RENDERFX8 | U_RENDERFX16;
	}	
	if( to->solid != from->solid )
		bits |= U_SOLID;
	// event is not delta compressed, just 0 compressed
	if( to->eventNum )
		bits |= U_EVENT;	
	if( to->modelindex  != from->modelindex  )
		bits |= U_MODEL;
	if( to->modelindex2 != from->modelindex2 )
		bits |= U_MODEL2;
	if( to->modelindex3 != from->modelindex3 )
		bits |= U_MODEL3;
	if( to->modelindex4 != from->modelindex4 )
		bits |= U_MODEL4;
	if( to->sound != from->sound )
		bits |= U_SOUND;
	if( newentity ||( to->renderfx & RF_BEAM ) )
		bits |= U_OLDORIGIN;
	// write the message
	if( !bits && !force )
		return;		// nothing to send!
	//----------
	if( bits & 0xFF000000 )
		bits |= U_MOREBITS3 | U_MOREBITS2 | U_MOREBITS1;
	else if( bits & 0x00FF0000 )
		bits |= U_MOREBITS2 | U_MOREBITS1;
	else if( bits & 0x0000FF00 )
		bits |= U_MOREBITS1;
	msg_buffer.WriteByte( bits & 255 );
	if( bits & 0xFF000000 ) {
		msg_buffer.WriteByte( ( bits >> 8  ) & 255 );
		msg_buffer.WriteByte( ( bits >> 16 ) & 255 );
		msg_buffer.WriteByte( ( bits >> 24 ) & 255 );
	} else if( bits & 0x00FF0000 ) {
		msg_buffer.WriteByte( ( bits >> 8  ) & 255 );
		msg_buffer.WriteByte( ( bits >> 16 ) & 255 );
	} else if( bits & 0x0000FF00 ) {
		msg_buffer.WriteByte( ( bits >> 8  ) & 255 );
	}
	if( bits & U_NUMBER16 )
		msg_buffer.WriteShort( to->number );
	else
		msg_buffer.WriteByte( to->number );
	if( bits & U_MODEL  )
		msg_buffer.WriteByte( to->modelindex  );
	if( bits & U_MODEL2 )
		msg_buffer.WriteByte( to->modelindex2 );
	if( bits & U_MODEL3 )
		msg_buffer.WriteByte( to->modelindex3 );
	if( bits & U_MODEL4 )
		msg_buffer.WriteByte( to->modelindex4 );
	if( bits & U_FRAME8  )
		msg_buffer.WriteByte( to->frame );
	if( bits & U_FRAME16 )
		msg_buffer.WriteShort( to->frame );
	if( ( bits & U_SKIN8 ) &&( bits & U_SKIN16 ) )
		msg_buffer.WriteLong( to->skinnum ); //used for laser colors
	else if( bits & U_SKIN8 )
		msg_buffer.WriteByte( to->skinnum );
	else if( bits & U_SKIN16 )
		msg_buffer.WriteShort( to->skinnum );
	if( ( bits &( U_EFFECTS8 | U_EFFECTS16 ) ) ==( U_EFFECTS8 | U_EFFECTS16 ) )
		msg_buffer.WriteLong( to->effects );
	else if( bits & U_EFFECTS8 )
		msg_buffer.WriteByte( to->effects );
	else if( bits & U_EFFECTS16 )
		msg_buffer.WriteShort( to->effects );
	if( ( bits &( U_RENDERFX8 | U_RENDERFX16 ) ) ==( U_RENDERFX8 | U_RENDERFX16 ) )
		msg_buffer.WriteLong( to->renderfx );
	else if( bits & U_RENDERFX8 )
		msg_buffer.WriteByte( to->renderfx );
	else if( bits & U_RENDERFX16 )
		msg_buffer.WriteShort( to->renderfx );
	if( bits & U_ORIGIN1 )
		msg_buffer.WriteFloat( to->origin[ 0 ] );	
	if( bits & U_ORIGIN2 )
		msg_buffer.WriteFloat( to->origin[ 1 ] );
	if( bits & U_ORIGIN3 )
		msg_buffer.WriteFloat( to->origin[ 2 ] );
	if( bits & U_ANGLE1 )
		msg_buffer.WriteFloat( to->angles[ 0 ] );
	if( bits & U_ANGLE2 )
		msg_buffer.WriteFloat( to->angles[ 1 ] );
	if( bits & U_ANGLE3 )
		msg_buffer.WriteFloat( to->angles[ 2 ] );
	if( bits & U_OLDORIGIN )
		msg_buffer.WriteVec3( to->old_origin );
	if( bits & U_SOUND )
		msg_buffer.WriteByte( to->sound );
	if( bits & U_EVENT )
		msg_buffer.WriteByte( to->eventNum );
	if( bits & U_SOLID )
		msg_buffer.WriteShort( to->solid );
}

void Server::SV_EmitPacketEntities( client_frame_t * from, client_frame_t * to, MessageBuffer & msg_buffer ) {
	entity_state_t	* oldent, * newent;
	int		oldindex, newindex;
	int		oldnum, newnum;
	int		from_num_entities;
	int		bits;
	msg_buffer.WriteByte( svc_packetentities );
	if( !from )
		from_num_entities = 0;
	else
		from_num_entities = from->num_entities;
	newindex = 0;
	oldindex = 0;
	while( newindex < to->num_entities || oldindex < from_num_entities ) {
		if( newindex >= to->num_entities )
			newnum = 9999;
		else {
			newent = &svs.client_entities[ ( to->first_entity + newindex ) % svs.num_client_entities ];
			newnum = newent->number;
		}
		if( oldindex >= from_num_entities )
			oldnum = 9999;
		else {
			oldent = &svs.client_entities[ ( from->first_entity + oldindex ) % svs.num_client_entities ];
			oldnum = oldent->number;
		}
		if( newnum == oldnum ) {
			// delta update from old position
			// because the force parm is false, this will not result
			// in any bytes being emited if the entity has not changed at all
			// note that players are always 'newentities', this updates their oldorigin always
			// and prevents warping
			SV_WriteDeltaEntity( oldent, newent, msg_buffer, false, newent->number <= Common::maxclients.GetInt( ) );
			oldindex++;
			newindex++;
			continue;
		}
		if( newnum < oldnum ) {	// this is a new entity, send it from the baseline
			SV_WriteDeltaEntity( &sv.baselines[ newnum ], newent, msg_buffer, true, true );
			newindex++;
			continue;
		}
		if( newnum > oldnum ) {	// the old entity isn't present in the new message
			bits = U_REMOVE;
			if( oldnum >= 256 )
				bits |= U_NUMBER16 | U_MOREBITS1;
			msg_buffer.WriteByte( bits & 255 );
			if( bits & 0x0000FF00 )
				msg_buffer.WriteByte( ( bits >> 8 ) & 255 );
			if( bits & U_NUMBER16 )
				msg_buffer.WriteShort( oldnum );
			else
				msg_buffer.WriteByte( oldnum );
			oldindex++;
			continue;
		}
	}
	msg_buffer.WriteShort( 0 );	// end of packetentities
}

void Server::SV_WritePlayerstateToClient( client_frame_t * from, client_frame_t * to, MessageBuffer & msg_buffer ) {
	int				pflags;
	player_state_t	* ps, * ops;
	player_state_t	dummy;
	int				statbits;
	ps = &to->ps;
	if( !from ) {
		dummy.Clear( );
		ops = &dummy;
	}
	else
		ops = &from->ps;
	// determine what needs to be sent
	pflags = 0;
	if( ps->pmove.pm_type != ops->pmove.pm_type )
		pflags |= PS_M_TYPE;
	if( ps->pmove.origin[ 0 ] != ops->pmove.origin[ 0 ] || ps->pmove.origin[ 1 ] != ops->pmove.origin[ 1 ] || ps->pmove.origin[ 2 ] != ops->pmove.origin[ 2 ] )
		pflags |= PS_M_ORIGIN;
	if( ps->pmove.velocity[ 0 ] != ops->pmove.velocity[ 0 ] || ps->pmove.velocity[ 1 ] != ops->pmove.velocity[ 1 ] || ps->pmove.velocity[ 2 ] != ops->pmove.velocity[ 2 ] )
		pflags |= PS_M_VELOCITY;
	if( ps->pmove.pm_time != ops->pmove.pm_time )
		pflags |= PS_M_TIME;
	if( ps->pmove.pm_flags != ops->pmove.pm_flags )
		pflags |= PS_M_FLAGS;
	if( ps->pmove.gravity != ops->pmove.gravity )
		pflags |= PS_M_GRAVITY;
	if( ps->pmove.delta_angles[ 0 ] != ops->pmove.delta_angles[ 0 ] || ps->pmove.delta_angles[ 1 ] != ops->pmove.delta_angles[ 1 ] || ps->pmove.delta_angles[ 2 ] != ops->pmove.delta_angles[ 2 ] )
		pflags |= PS_M_DELTA_ANGLES;
	if( ps->viewoffset[ 0 ] != ops->viewoffset[ 0 ] || ps->viewoffset[ 1 ] != ops->viewoffset[ 1 ] || ps->viewoffset[ 2 ] != ops->viewoffset[ 2 ] )
		pflags |= PS_VIEWOFFSET;
	if( ps->viewangles[ 0 ] != ops->viewangles[ 0 ] || ps->viewangles[ 1 ] != ops->viewangles[ 1 ] || ps->viewangles[ 2 ] != ops->viewangles[ 2 ] )
		pflags |= PS_VIEWANGLES;
	if( ps->kick_angles[ 0 ] != ops->kick_angles[ 0 ] || ps->kick_angles[ 1 ] != ops->kick_angles[ 1 ] || ps->kick_angles[ 2 ] != ops->kick_angles[ 2 ] )
		pflags |= PS_KICKANGLES;
	if( ps->blend[ 0 ] != ops->blend[ 0 ] || ps->blend[ 1 ] != ops->blend[ 1 ] || ps->blend[ 2 ] != ops->blend[ 2 ] || ps->blend[ 3 ] != ops->blend[ 3 ] )
		pflags |= PS_BLEND;
	if( ps->fov != ops->fov )
		pflags |= PS_FOV;
	if( ps->rdflags != ops->rdflags )
		pflags |= PS_RDFLAGS;
	if( ps->gunframe != ops->gunframe )
		pflags |= PS_WEAPONFRAME;
	pflags |= PS_WEAPONINDEX;
	// write it
	msg_buffer.WriteByte( svc_playerinfo );
	msg_buffer.WriteShort( pflags );
	// write the pmove_state_t
	if( pflags & PS_M_TYPE )
		msg_buffer.WriteByte( ps->pmove.pm_type );
	if( pflags & PS_M_ORIGIN ) {
		msg_buffer.WriteShort( ps->pmove.origin[ 0 ] );
		msg_buffer.WriteShort( ps->pmove.origin[ 1 ] );
		msg_buffer.WriteShort( ps->pmove.origin[ 2 ] );
	}
	if( pflags & PS_M_VELOCITY ) {
		msg_buffer.WriteShort( ps->pmove.velocity[ 0 ] );
		msg_buffer.WriteShort( ps->pmove.velocity[ 1 ] );
		msg_buffer.WriteShort( ps->pmove.velocity[ 2 ] );
	}
	if( pflags & PS_M_TIME )
		msg_buffer.WriteByte( ps->pmove.pm_time );
	if( pflags & PS_M_FLAGS )
		msg_buffer.WriteByte( ps->pmove.pm_flags );
	if( pflags & PS_M_GRAVITY )
		msg_buffer.WriteShort( ps->pmove.gravity );
	if( pflags & PS_M_DELTA_ANGLES ) {
		msg_buffer.WriteShort( ps->pmove.delta_angles[ 0 ] );
		msg_buffer.WriteShort( ps->pmove.delta_angles[ 1 ] );
		msg_buffer.WriteShort( ps->pmove.delta_angles[ 2 ] );
	}
	// write the rest of the player_state_t
	if( pflags & PS_VIEWOFFSET ) {
		msg_buffer.WriteChar( ( int )ps->viewoffset[ 0 ] * 4 );
		msg_buffer.WriteChar( ( int )ps->viewoffset[ 1 ] * 4 );
		msg_buffer.WriteChar( ( int )ps->viewoffset[ 2 ] * 4 );
	}
	if( pflags & PS_VIEWANGLES ) {
		msg_buffer.WriteVec3( ps->viewangles );
	}
	if( pflags & PS_KICKANGLES ) {
		msg_buffer.WriteChar( ( int )ps->kick_angles[ 0 ] * 4 );
		msg_buffer.WriteChar( ( int )ps->kick_angles[ 1 ] * 4 );
		msg_buffer.WriteChar( ( int )ps->kick_angles[ 2 ] * 4 );
	}
	if( pflags & PS_WEAPONINDEX )
		msg_buffer.WriteByte( ps->gunindex );
	if( pflags & PS_WEAPONFRAME ) {
		msg_buffer.WriteByte( ps->gunframe );
		msg_buffer.WriteChar( ( int )ps->gunoffset[ 0 ] * 4 );
		msg_buffer.WriteChar( ( int )ps->gunoffset[ 1 ] * 4 );
		msg_buffer.WriteChar( ( int )ps->gunoffset[ 2 ] * 4 );
		msg_buffer.WriteChar( ( int )ps->gunangles[ 0 ] * 4 );
		msg_buffer.WriteChar( ( int )ps->gunangles[ 1 ] * 4 );
		msg_buffer.WriteChar( ( int )ps->gunangles[ 2 ] * 4 );
	}
	if( pflags & PS_BLEND ) {
		msg_buffer.WriteByte( ( int )ps->blend[ 0 ] * 255 );
		msg_buffer.WriteByte( ( int )ps->blend[ 1 ] * 255 );
		msg_buffer.WriteByte( ( int )ps->blend[ 2 ] * 255 );
		msg_buffer.WriteByte( ( int )ps->blend[ 3 ] * 255 );
	}
	if( pflags & PS_FOV )
		msg_buffer.WriteByte( ( int )ps->fov );
	if( pflags & PS_RDFLAGS )
		msg_buffer.WriteByte( ps->rdflags );
	// send stats
	statbits = 0;
	for( int i = 0; i < MAX_STATS; i++ ) {
		if( ps->stats[ i ] != ops->stats[ i ] )
			statbits |= 1<<i;
	}
	msg_buffer.WriteLong( statbits );
	for( int i = 0; i<MAX_STATS; i++ ) {
		if( statbits &( 1 << i ) )
			msg_buffer.WriteShort( ps->stats[ i ] );
	}
}

void Server::SV_WriteFrameToClient( client_t * client, MessageBuffer & msg_buffer ) {
	client_frame_t * frame, * oldframe;
	int					lastframe;
	// this is the frame we are creating
	frame = &client->frames[ sv.framenum & UPDATE_MASK ];
	if( client->lastframe <= 0 ) {	// client is asking for a retransmit
		oldframe = NULL;
		lastframe = -1;
	} else if( sv.framenum - client->lastframe >= ( UPDATE_BACKUP - 3 ) ) {	// client hasn't gotten a good message through in a long time
		oldframe = NULL;
		lastframe = -1;
	} else {	// we have a valid message to delta from
		oldframe = &client->frames[ client->lastframe & UPDATE_MASK ];
		lastframe = client->lastframe;
	}
	msg_buffer.WriteByte( svc_frame );
	msg_buffer.WriteLong( sv.framenum );
	msg_buffer.WriteLong( lastframe );	// what we are delta'ing from
	msg_buffer.WriteByte( client->surpressCount );	// rate dropped packets
	client->surpressCount = 0;
	// send over the areabits
	msg_buffer.WriteByte( frame->areabytes );
	msg_buffer.WriteData( frame->areabits, frame->areabytes );
	// delta encode the playerstate
	SV_WritePlayerstateToClient( oldframe, frame, msg_buffer );
	// delta encode the entities
	SV_EmitPacketEntities( oldframe, frame, msg_buffer );
}

void Server::SV_BuildClientFrame( client_t * client ) {
	int		e, i;
	Vec3	org;
	Entity	* ent;
	Entity	* clent;
	client_frame_t	* frame;
	entity_state_t	* state;
	int		l;
	int		clientarea, clientcluster;
	int		leafnum;
	int		c_fullsend;
	byte	* clientphs;
	byte	* bitvector;
	clent = client->edict;
	if( !clent->client )
		return;		// not in game yet
	// this is the frame we are creating
	frame = &client->frames[ sv.framenum & UPDATE_MASK ];
	frame->senttime = svs.realtime; // save it for ping calc later
	// find the client's PVS
	for( i = 0; i < 3; i++ )
		org[ i ] = ( float )clent->client->ps.pmove.origin[ i ] * 0.125f + clent->client->ps.viewoffset[ i ];
	leafnum = CollisionModel::CM_PointLeafnum( org );
	clientarea = CollisionModel::CM_LeafArea( leafnum );
	clientcluster = CollisionModel::CM_LeafCluster( leafnum );
	// calculate the visible areas
	frame->areabytes = CollisionModel::CM_WriteAreaBits( frame->areabits, clientarea );
	// grab the current player_state_t
	frame->ps = clent->client->ps;
	SV_FatPVS( org );
	clientphs = CollisionModel::CM_ClusterPHS( clientcluster );
	// build up the list of visible entities
	frame->num_entities = 0;
	frame->first_entity = svs.next_client_entities;
	c_fullsend = 0;
	for( e = 1; e < Game::num_edicts; e++ ) {
		ent = EDICT_NUM( e );
		// ignore ents without visible models
		if( ent->svflags & SVF_NOCLIENT )
			continue;
		// ignore ents without visible models unless they have an effect
		if( !ent->s.modelindex && !ent->s.effects && !ent->s.sound && !ent->s.eventNum )
			continue;
		// ignore if not touching a PV leaf
		if( ent != clent ) {
			// check area
			if( !CollisionModel::CM_AreasConnected( clientarea, ent->areanum ) ) {
				// doors can legally straddle two areas, so
				// we may need to check another one
				if( !ent->areanum2 || !CollisionModel::CM_AreasConnected( clientarea, ent->areanum2 ) )
					continue;		// blocked by a door
			}
			// beams just check one point for PHS
			if( ent->s.renderfx & RF_BEAM ) {
				l = ent->clusternums[ 0 ];
				if( !( clientphs[ l >> 3 ] &( 1 <<( l & 7 ) ) ) )
					continue;
			} else {
				// FIXME: if an ent has a model and a sound, but isn't
				// in the PVS, only the PHS, clear the model
				if( ent->s.sound ) {
					bitvector = fatpvs;	// clientphs;
				} else
					bitvector = fatpvs;
				if( ent->num_clusters == -1 ) {	// too many leafs for individual check, go by headnode
					if( !CollisionModel::CM_HeadnodeVisible( ent->headnode, bitvector ) )
						continue;
					c_fullsend++;
				} else {	// check individual leafs
					for( i = 0; i < ent->num_clusters; i++ ) {
						l = ent->clusternums[ i ];
						if( bitvector[ l >> 3 ] &( 1 <<( l & 7 ) ) )
							break;
					}
					if( i == ent->num_clusters )
						continue;		// not visible
				}
				if( !ent->s.modelindex ) {	// don't send sounds if they will be attenuated away
					Vec3	delta;
					float	len;
					delta = org - ent->s.origin;
					len = delta.Length( );
					if( len > 400.0f )
						continue;
				}
			}
		}
		// add it to the circular client_entities array
		state = &svs.client_entities[ svs.next_client_entities%svs.num_client_entities ];
		if( ent->s.number != e ) {
			Common::Com_DPrintf( "FIXING ENT->S.NUMBER!!!\n" );
			ent->s.number = e;
		}
		*state = ent->s;
		// don't mark players missiles as solid
		if( ent->owner == client->edict )
			state->solid = 0;
		svs.next_client_entities++;
		frame->num_entities++;
	}
}

void Server::SV_RecordDemoMessage( ) {
	entity_state_t			nostate;
	MessageBufferT< 32768 >	recordBuffer;
	if( !svs.demofile )
		return;
	nostate.Clear( );
	recordBuffer.Init( );
	// write a frame message that doesn't contain a player_state_t
	recordBuffer.WriteByte( svc_frame );
	recordBuffer.WriteLong( sv.framenum );
	recordBuffer.WriteByte( svc_packetentities );
	for( int e = 1; e < Game::num_edicts; e++ )  {
		Entity * ent = EDICT_NUM( e );
		// ignore ents without visible models unless they have an effect
		if( ent->inuse && ent->s.number &&( ent->s.modelindex || ent->s.effects || ent->s.sound || ent->s.eventNum ) && !( ent->svflags & SVF_NOCLIENT ) )
			SV_WriteDeltaEntity( &nostate, &ent->s, recordBuffer, false, true );
	}
	recordBuffer.WriteShort( 0 );		// end of packetentities
	// now add the accumulated multicast information
	recordBuffer.WriteData( svs.d_demoBuffer.GetData( ), svs.d_demoBuffer.GetSize( ) );
	svs.d_demoBuffer.BeginWriting( );
	// now write the entire message to the file, prefixed by the length
	int len = LittleLong( recordBuffer.GetSize( ) );
	svs.demofile->Write( &len, 4 );
	svs.demofile->Write( recordBuffer.GetData( ), recordBuffer.GetSize( ) );
}
