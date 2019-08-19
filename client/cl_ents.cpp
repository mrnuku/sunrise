#include "precompiled.h"
#pragma hdrstop

extern RenderModel * cl_mod_powerscreen;

//PGM
int vidref_val;
//PGM

int bitcounts[ 32 ]; /// just for protocol profiling
int Client::CL_ParseEntityBits( MessageBuffer & msg_buffer, unsigned * bits ) {
	unsigned b, total;
	int i;
	int number;
	total = msg_buffer.ReadByte( );
	if( total & U_MOREBITS1 ) {
		b = msg_buffer.ReadByte( );
		total |= b<<8;
	}
	if( total & U_MOREBITS2 ) {
		b = msg_buffer.ReadByte( );
		total |= b<<16;
	}
	if( total & U_MOREBITS3 ) {
		b = msg_buffer.ReadByte( );
		total |= b<<24;
	}
	// count the bits for net profiling
	for( i = 0; i<32; i++ )
		if( total&( 1<<i ) ) 	bitcounts[ i ]++;
	if( total & U_NUMBER16 )
		number = msg_buffer.ReadShort( );
	else
		number = msg_buffer.ReadByte( );
	* bits = total;
	return number;
}

void Client::CL_ParseDelta( MessageBuffer & msg_buffer, entity_state_t * from, entity_state_t * to, int number, int bits ) {
	// set everything to the state we are delta'ing from
	* to = * from;
	to->old_origin = from->origin;
	to->number = number;
	if( bits & U_MODEL )
		to->modelindex = msg_buffer.ReadByte( );
	if( bits & U_MODEL2 )
		to->modelindex2 = msg_buffer.ReadByte( );
	if( bits & U_MODEL3 )
		to->modelindex3 = msg_buffer.ReadByte( );
	if( bits & U_MODEL4 )
		to->modelindex4 = msg_buffer.ReadByte( );
	if( bits & U_FRAME8 )
		to->frame = msg_buffer.ReadByte( );
	if( bits & U_FRAME16 )
		to->frame = msg_buffer.ReadShort( );
	if( ( bits & U_SKIN8 ) &&( bits & U_SKIN16 ) ) //used for laser colors
		to->skinnum = msg_buffer.ReadLong( );
	else if( bits & U_SKIN8 )
		to->skinnum = msg_buffer.ReadByte( );
	else if( bits & U_SKIN16 )
		to->skinnum = msg_buffer.ReadShort( );
	if( ( bits &( U_EFFECTS8|U_EFFECTS16 ) ) ==( U_EFFECTS8|U_EFFECTS16 ) )
		to->effects = msg_buffer.ReadLong( );
	else if( bits & U_EFFECTS8 )
		to->effects = msg_buffer.ReadByte( );
	else if( bits & U_EFFECTS16 )
		to->effects = msg_buffer.ReadShort( );
	if( ( bits &( U_RENDERFX8|U_RENDERFX16 ) ) ==( U_RENDERFX8|U_RENDERFX16 ) )
		to->renderfx = msg_buffer.ReadLong( );
	else if( bits & U_RENDERFX8 )
		to->renderfx = msg_buffer.ReadByte( );
	else if( bits & U_RENDERFX16 )
		to->renderfx = msg_buffer.ReadShort( );
	if( bits & U_ORIGIN1 )
		to->origin[ 0 ] = msg_buffer.ReadFloat( );
	if( bits & U_ORIGIN2 )
		to->origin[ 1 ] = msg_buffer.ReadFloat( );
	if( bits & U_ORIGIN3 )
		to->origin[ 2 ] = msg_buffer.ReadFloat( );
	if( bits & U_ANGLE1 )
		to->angles[ 0 ] = msg_buffer.ReadFloat( );
	if( bits & U_ANGLE2 )
		to->angles[ 1 ] = msg_buffer.ReadFloat( );
	if( bits & U_ANGLE3 )
		to->angles[ 2 ] = msg_buffer.ReadFloat( );
	if( bits & U_OLDORIGIN )
		msg_buffer.ReadVec3( to->old_origin );
	if( bits & U_SOUND )
		to->sound = msg_buffer.ReadByte( );
	if( bits & U_EVENT )
		to->eventNum =( entity_event_t )msg_buffer.ReadByte( );
	else
		to->eventNum = EV_NONE;
	if( bits & U_SOLID )
		to->solid = msg_buffer.ReadShort( );
}

void Client::CL_DeltaEntity( MessageBuffer & msg_buffer, frame_t * frame, int newnum, entity_state_t * old, int bits ) {
	centity_t * ent;
	entity_state_t * state;
	ent = &cl_entities[ newnum ];
	state = &cl_parse_entities[ cl.parse_entities &( MAX_PARSE_ENTITIES - 1 ) ];
	cl.parse_entities++;
	frame->num_entities++;
	CL_ParseDelta( msg_buffer, old, state, newnum, bits );
	// some data changes will force no lerping
	if( state->modelindex != ent->current.modelindex
		|| state->modelindex2 != ent->current.modelindex2
		|| state->modelindex3 != ent->current.modelindex3
		|| state->modelindex4 != ent->current.modelindex4
		|| abs( state->origin[ 0 ] - ent->current.origin[ 0 ] ) > 512
		|| abs( state->origin[ 1 ] - ent->current.origin[ 1 ] ) > 512
		|| abs( state->origin[ 2 ] - ent->current.origin[ 2 ] ) > 512
		|| state->eventNum == EV_PLAYER_TELEPORT
		|| state->eventNum == EV_OTHER_TELEPORT
		 ) {
		ent->serverframe = -99;
	}
	if( ent->serverframe != cl.frame.serverframe - 1 ) { // wasn't in last update, so initialize some things
		ent->trailcount = 1024; // for diminishing rocket / grenade trails
		// duplicate the current state so lerping doesn't hurt anything
		ent->prev = * state;
		if( state->eventNum == EV_OTHER_TELEPORT ) {
			ent->prev.origin = state->origin;
			ent->lerp_origin = state->origin;
		} else {
			ent->prev.origin = state->old_origin;
			ent->lerp_origin = state->old_origin;
		}
	} else { // shuffle the last state to previous
		ent->prev = ent->current;
	}
	ent->serverframe = cl.frame.serverframe;
	ent->current = * state;
}

void Client::CL_ParsePacketEntities( MessageBuffer & msg_buffer, frame_t * oldframe, frame_t * newframe ) {
	int newnum;
	uint bits;
	entity_state_t * oldstate;
	int oldindex, oldnum;
	newframe->parse_entities = cl.parse_entities;
	newframe->num_entities = 0;
	// delta from the entities present in oldframe
	oldindex = 0;
	if( !oldframe )
		oldnum = 99999;
	else {
		if( oldindex >= oldframe->num_entities )
			oldnum = 99999;
		else {
			oldstate = &cl_parse_entities[( oldframe->parse_entities+oldindex ) &( MAX_PARSE_ENTITIES-1 )];
			oldnum = oldstate->number;
		}
	}
	while( 1 ) {
		newnum = CL_ParseEntityBits( msg_buffer, &bits );
		if( newnum >= MAX_EDICTS )
			Common::Com_Error( ERR_DROP, "CL_ParsePacketEntities: bad number:%i", newnum );
		if( msg_buffer.GetReadCount( ) > msg_buffer.GetSize( ) )
			Common::Com_Error( ERR_DROP, "CL_ParsePacketEntities: end of message" );
		if( !newnum )
			break;
		while( oldnum < newnum ) { // one or more entities from the old packet are unchanged
			if( cl_shownet.GetInt( ) == 3 )
				Common::Com_Printf( "   unchanged: %i\n", oldnum );
			CL_DeltaEntity( msg_buffer, newframe, oldnum, oldstate, 0 );
			oldindex++;
			if( oldindex >= oldframe->num_entities )
				oldnum = 99999;
			else {
				oldstate = &cl_parse_entities[( oldframe->parse_entities+oldindex ) &( MAX_PARSE_ENTITIES-1 )];
				oldnum = oldstate->number;
			}
		}
		if( bits & U_REMOVE ) { // the entity present in oldframe is not in the current frame
			if( cl_shownet.GetInt( ) == 3 )
				Common::Com_Printf( "   remove: %i\n", newnum );
			if( oldnum != newnum )
				Common::Com_Printf( "U_REMOVE: oldnum != newnum\n" );
			oldindex++;
			if( oldindex >= oldframe->num_entities )
				oldnum = 99999;
			else {
				oldstate = &cl_parse_entities[( oldframe->parse_entities+oldindex ) &( MAX_PARSE_ENTITIES-1 )];
				oldnum = oldstate->number;
			}
			continue;
		}
		if( oldnum == newnum ) { // delta from previous state
			if( cl_shownet.GetInt( ) == 3 )
				Common::Com_Printf( "   delta: %i\n", newnum );
			CL_DeltaEntity( msg_buffer, newframe, newnum, oldstate, bits );
			oldindex++;
			if( oldindex >= oldframe->num_entities )
				oldnum = 99999;
			else {
				oldstate = &cl_parse_entities[( oldframe->parse_entities+oldindex ) &( MAX_PARSE_ENTITIES-1 )];
				oldnum = oldstate->number;
			}
			continue;
		}
		if( oldnum > newnum ) { // delta from baseline
			if( cl_shownet.GetInt( ) == 3 )
				Common::Com_Printf( "   baseline: %i\n", newnum );
			CL_DeltaEntity( msg_buffer, newframe, newnum, &cl_entities[ newnum ].baseline, bits );
			continue;
		}
	}
	// any remaining entities in the old frame are copied over
	while( oldnum != 99999 ) { // one or more entities from the old packet are unchanged
		if( cl_shownet.GetInt( ) == 3 )
			Common::Com_Printf( "   unchanged: %i\n", oldnum );
		CL_DeltaEntity( msg_buffer, newframe, oldnum, oldstate, 0 );
		oldindex++;
		if( oldindex >= oldframe->num_entities )
			oldnum = 99999;
		else {
			oldstate = &cl_parse_entities[( oldframe->parse_entities+oldindex ) &( MAX_PARSE_ENTITIES-1 )];
			oldnum = oldstate->number;
		}
	}
}

void Client::CL_ParsePlayerstate( MessageBuffer & msg_buffer, frame_t * oldframe, frame_t * newframe ) {
	int flags;
	player_state_t * state;
	int i;
	int statbits;
	state = &newframe->playerstate;
	// clear to old value before delta parsing
	if( oldframe )
		*state = oldframe->playerstate;
	else
		state->Clear( );
	flags = msg_buffer.ReadShort( );
	// parse the pmove_state_t
	if( flags & PS_M_TYPE )
		state->pmove.pm_type =( pmtype_t )msg_buffer.ReadByte( );
	if( flags & PS_M_ORIGIN ) {
		state->pmove.origin[ 0 ] = msg_buffer.ReadShort( );
		state->pmove.origin[ 1 ] = msg_buffer.ReadShort( );
		state->pmove.origin[ 2 ] = msg_buffer.ReadShort( );
	}
	if( flags & PS_M_VELOCITY ) {
		state->pmove.velocity[ 0 ] = msg_buffer.ReadShort( );
		state->pmove.velocity[ 1 ] = msg_buffer.ReadShort( );
		state->pmove.velocity[ 2 ] = msg_buffer.ReadShort( );
	}
	if( flags & PS_M_TIME )
		state->pmove.pm_time = msg_buffer.ReadByte( );
	if( flags & PS_M_FLAGS )
		state->pmove.pm_flags = msg_buffer.ReadByte( );
	if( flags & PS_M_GRAVITY )
		state->pmove.gravity = msg_buffer.ReadShort( );
	if( flags & PS_M_DELTA_ANGLES ) {
		state->pmove.delta_angles[ 0 ] = msg_buffer.ReadShort( );
		state->pmove.delta_angles[ 1 ] = msg_buffer.ReadShort( );
		state->pmove.delta_angles[ 2 ] = msg_buffer.ReadShort( );
	}
	if( cl.attractloop )
		state->pmove.pm_type = PM_FREEZE; // demo playback
	// parse the rest of the player_state_t
	if( flags & PS_VIEWOFFSET ) {
		state->viewoffset[ 0 ] = msg_buffer.ReadChar( ) * 0.25f;
		state->viewoffset[ 1 ] = msg_buffer.ReadChar( ) * 0.25f;
		state->viewoffset[ 2 ] = msg_buffer.ReadChar( ) * 0.25f;
	}
	if( flags & PS_VIEWANGLES ) {
		msg_buffer.ReadVec3( state->viewangles );
	}
	if( flags & PS_KICKANGLES ) {
		state->kick_angles[ 0 ] = msg_buffer.ReadChar( ) * 0.25f;
		state->kick_angles[ 1 ] = msg_buffer.ReadChar( ) * 0.25f;
		state->kick_angles[ 2 ] = msg_buffer.ReadChar( ) * 0.25f;
	}
	if( flags & PS_WEAPONINDEX )
		state->gunindex = msg_buffer.ReadByte( );
	if( flags & PS_WEAPONFRAME ) {
		state->gunframe = msg_buffer.ReadByte( );
		state->gunoffset[ 0 ] = msg_buffer.ReadChar( ) * 0.25f;
		state->gunoffset[ 1 ] = msg_buffer.ReadChar( ) * 0.25f;
		state->gunoffset[ 2 ] = msg_buffer.ReadChar( ) * 0.25f;
		state->gunangles[ 0 ] = msg_buffer.ReadChar( ) * 0.25f;
		state->gunangles[ 1 ] = msg_buffer.ReadChar( ) * 0.25f;
		state->gunangles[ 2 ] = msg_buffer.ReadChar( ) * 0.25f;
	}
	if( flags & PS_BLEND ) {
		state->blend[ 0 ] = msg_buffer.ReadByte( )/255.0f;
		state->blend[ 1 ] = msg_buffer.ReadByte( )/255.0f;
		state->blend[ 2 ] = msg_buffer.ReadByte( )/255.0f;
		state->blend[ 3 ] = msg_buffer.ReadByte( )/255.0f;
	}
	if( flags & PS_FOV )
		state->fov = ( float )msg_buffer.ReadByte( );
	if( flags & PS_RDFLAGS )
		state->rdflags = msg_buffer.ReadByte( );
	// parse stats
	statbits = msg_buffer.ReadLong( );
	for( i = 0; i<MAX_STATS; i++ )
		if( statbits &( 1<<i ) )
			state->stats[ i ] = msg_buffer.ReadShort( );
}

void Client::CL_FireEntityEvents( frame_t * frame ) {
	entity_state_t * s1;
	int pnum, num;
	for( pnum = 0; pnum<frame->num_entities; pnum++ ) {
		num = ( frame->parse_entities + pnum ) & ( MAX_PARSE_ENTITIES - 1 );
		s1 = &cl_parse_entities[ num ];
		if( s1->eventNum )
			CL_EntityEvent( s1 );
		// EF_TELEPORTER acts like an event, but is not cleared each frame
		if( s1->effects & EF_TELEPORTER )
			CL_TeleporterParticles( s1 );
	}
}

void Client::CL_ParseFrame( MessageBuffer & msg_buffer ) {
	int cmd;
	int len;
	frame_t * old;
	cl.frame.Clear( );
	cl.frame.serverframe = msg_buffer.ReadLong( );
	cl.frame.deltaframe = msg_buffer.ReadLong( );
	cl.frame.servertime = ( timeType )( cl.frame.serverframe * 100 );
	// BIG HACK to let old demos continue to work
	if( cls.serverProtocol != 26 )
		cl.surpressCount = msg_buffer.ReadByte( );
	if( cl_shownet.GetInt( ) == 3 )
		Common::Com_Printf( "   frame:%i  delta:%i\n", cl.frame.serverframe, cl.frame.deltaframe );
	// If the frame is delta compressed from data that we
	// no longer have available, we must suck up the rest of
	// the frame, but not use it, then ask for a non-compressed
	// message 
	if( cl.frame.deltaframe <= 0 ) {
		cl.frame.valid = true; // uncompressed frame
		old = NULL;
		cls.demowaiting = false; // we can start recording now
	} else {
		old = &cl.frames[cl.frame.deltaframe & UPDATE_MASK];
		if( !old->valid )
			Common::Com_Printf( "Delta from invalid frame( not supposed to happen! ).\n" );
		if( old->serverframe != cl.frame.deltaframe ) { // The frame that the server did the delta from is too old, so we can't reconstruct it properly.
			Common::Com_Printf( "Delta frame too old.\n" );
		} else if( cl.parse_entities - old->parse_entities > MAX_PARSE_ENTITIES-128 ) {
			Common::Com_Printf( "Delta parse_entities too old.\n" );
		} else
			cl.frame.valid = true; // valid delta parse
	}
	// clamp time 
	if( cl.time > cl.frame.servertime )
		cl.time = cl.frame.servertime;
	else if( cl.time < cl.frame.servertime - 100 )
		cl.time = cl.frame.servertime - 100;
	// read areabits
	len = msg_buffer.ReadByte( );
	msg_buffer.ReadData( &cl.frame.areabits, len );
	// read playerinfo
	cmd = msg_buffer.ReadByte( );
	SHOWNET( msg_buffer, svc_strings[ cmd ] );
	if( cmd != svc_playerinfo )
		Common::Com_Error( ERR_DROP, "CL_ParseFrame: not playerinfo" );
	CL_ParsePlayerstate( msg_buffer, old, &cl.frame );
	// read packet entities
	cmd = msg_buffer.ReadByte( );
	SHOWNET( msg_buffer, svc_strings[ cmd ] );
	if( cmd != svc_packetentities )
		Common::Com_Error( ERR_DROP, "CL_ParseFrame: not packetentities" );
	CL_ParsePacketEntities( msg_buffer, old, &cl.frame );
	// save the frame off in the backup array for later delta comparisons
	cl.frames[cl.frame.serverframe & UPDATE_MASK] = cl.frame;
	if( cl.frame.valid ) {
		// getting a valid frame message ends the connection process
		if( cls.state != ca_active ) {
			cls.state = ca_active;
			cl.force_refdef = true;
			cl.predicted_origin[ 0 ] = ( float )cl.frame.playerstate.pmove.origin[ 0 ] * 0.125f;
			cl.predicted_origin[ 1 ] = ( float )cl.frame.playerstate.pmove.origin[ 1 ] * 0.125f;
			cl.predicted_origin[ 2 ] = ( float )cl.frame.playerstate.pmove.origin[ 2 ] * 0.125f;
			cl.predicted_angles = cl.frame.playerstate.viewangles;
			if( cls.disable_servercount != cl.servercount && cl.refresh_prepped )
				Screen::SCR_EndLoadingPlaque( ); // get rid of loading plaque
		}
		cl.sound_prepped = true; // can start mixing ambient sounds
		// fire entity events
		CL_FireEntityEvents( &cl.frame );
		CL_CheckPredictionError( );
	}
}

void Client::CL_AddPacketEntities( frame_t * frame ) {
	SceneEntity ent;
	entity_state_t * s1;
	float autorotate;
	int i;
	int pnum;
	centity_t * cent;
	int autoanim;
	clientinfo_t * ci;
	unsigned int effects, renderfx;
	// bonus items rotate at a fixed rate
	autorotate = anglemod( ( float )( cl.time/10 ) );
	// brush models can auto animate their frames
	autoanim = 2 * ( int )( cl.time/1000 );
	ent.Clear( );
	for( pnum = 0; pnum<frame->num_entities; pnum++ ) {
		s1 = &cl_parse_entities[( frame->parse_entities+pnum )&( MAX_PARSE_ENTITIES-1 )];
		cent = &cl_entities[ s1->number ];
		effects = s1->effects;
		renderfx = s1->renderfx;
		// set frame
		if( effects & EF_ANIM01 )
			ent.frame = autoanim & 1;
		else if( effects & EF_ANIM23 )
			ent.frame = 2 +( autoanim & 1 );
		else if( effects & EF_ANIM_ALL )
			ent.frame = autoanim;
		else if( effects & EF_ANIM_ALLFAST )
			ent.frame = ( int )( cl.time / 100 );
		else
			ent.frame = s1->frame;
		// quad and pent can do different things on client
		if( effects & EF_PENT ) {
			effects &= ~EF_PENT;
			effects |= EF_COLOR_SHELL;
			renderfx |= RF_SHELL_RED;
		}
		if( effects & EF_QUAD ) {
			effects &= ~EF_QUAD;
			effects |= EF_COLOR_SHELL;
			renderfx |= RF_SHELL_BLUE;
		}
		if( effects & EF_DOUBLE ) {
			effects &= ~EF_DOUBLE;
			effects |= EF_COLOR_SHELL;
			renderfx |= RF_SHELL_DOUBLE;
		}
		if( effects & EF_HALF_DAMAGE ) {
			effects &= ~EF_HALF_DAMAGE;
			effects |= EF_COLOR_SHELL;
			renderfx |= RF_SHELL_HALF_DAM;
		}
		ent.oldframe = cent->prev.frame;
		ent.backlerp = 1.0f - cl.lerpfrac;
		if( renderfx &( RF_FRAMELERP|RF_BEAM ) ) { // step origin discretely, because the frames
			// do the animation properly
			ent.origin = cent->current.origin;
			ent.oldorigin = cent->current.old_origin;
		} else { // interpolate origin
			for( i = 0; i < 3; i++ )
				ent.origin[ i ] = ent.oldorigin[ i ] = cent->prev.origin[ i ] + cl.lerpfrac * ( cent->current.origin[ i ] - cent->prev.origin[ i ] );
		}
		// create a new entity
		// tweak the color of beams
		if( renderfx & RF_BEAM ) { // the four beam colors are encoded in 32 bits of skinnum( hack )
			ent.alpha = 0.30f;
			ent.skinnum =( s1->skinnum >>( ( rand( ) % 4 )* 8 ) ) & 0xFF;
			ent.model = NULL;
		} else {
			// set skin
			if( s1->modelindex == 255 ) { // use custom player skin
				ent.skinnum = 0;
				ci = &cl.clientinfo[s1->skinnum & 0xFF];
				ent.skin = ci->skin;
				ent.model = ci->model;
				if( !ent.skin || !ent.model ) {
					ent.skin = cl.baseclientinfo.skin;
					ent.model = cl.baseclientinfo.model;
				}
				if( renderfx & RF_USE_DISGUISE ) {
					if( !Str::Cmpn( ( char * )ent.skin, "players/male" , 12 ) ) {
						ent.skin = MaterialSystem::FindTexture( "players/male/disguise.pcx", "model" );
						ent.model = Renderer::RegisterModel( Str( "players/male/tris.md2" ) );
					} else if( !Str::Cmpn( ( char * )ent.skin, "players/female", 14 ) ) {
						ent.skin = MaterialSystem::FindTexture( "players/female/disguise.pcx", "model" );
						ent.model = Renderer::RegisterModel( Str( "players/female/tris.md2" ) );
					} else if( !Str::Cmpn( ( char * )ent.skin, "players/cyborg", 14 ) ) {
						ent.skin = MaterialSystem::FindTexture( "players/cyborg/disguise.pcx", "model" );
						ent.model = Renderer::RegisterModel( Str( "players/cyborg/tris.md2" ) );
					}
				}
			} else {
				ent.skinnum = s1->skinnum;
				ent.skin = NULL;
				ent.model = cl.model_draw[ s1->modelindex ];
			}
		}
		// only used for black hole model right now, FIXME: do better
		if( renderfx == RF_TRANSLUCENT )
			ent.alpha = 0.70f;
		// render effects( fullbright, translucent, etc )
		if( ( effects & EF_COLOR_SHELL ) )
			ent.flags = 0; // renderfx go on color shell entity
		else
			ent.flags = renderfx;
		// calculate angles
		if( effects & EF_ROTATE ) { // some bonus items auto-rotate
			ent.angles[ 0 ] = 0;
			ent.angles[ 1 ] = autorotate;
			ent.angles[ 2 ] = 0;
		} else if( effects & EF_SPINNINGLIGHTS ) {
			ent.angles[ 0 ] = 0.0f;
			ent.angles[ 1 ] = anglemod( ( float )( cl.time/2 ) ) + s1->angles[ 1 ];
			ent.angles[ 2 ] = 180.0f;
			{
				Vec3 forward;
				Vec3 start;
				ent.angles.AngleVectors( &forward, NULL, NULL );
				start = ent.origin.Magnitude( 64.0f, forward );
				View::V_AddLight( start, 100.0f, 1.0f, 0.0f, 0.0f );
			}
		} else { // interpolate angles
			float a1, a2;
			for( i = 0; i < 3; i++ ) {
				a1 = cent->current.angles[ i ];
				a2 = cent->prev.angles[ i ];
				ent.angles[ i ] = LerpAngle( a2, a1, cl.lerpfrac );
			}
		}
		if( s1->number == cl.playernum+1 ) 	{
			ent.flags |= RF_VIEWERMODEL; // only draw from mirrors
			// FIXME: still pass to refresh
			if( effects & EF_FLAG1 )
				View::V_AddLight( ent.origin, 225.0f, 1.0f, 0.1f, 0.1f );
			else if( effects & EF_FLAG2 )
				View::V_AddLight( ent.origin, 225.0f, 0.1f, 0.1f, 1.0f );
			else if( effects & EF_TAGTRAIL ) //PGM
				View::V_AddLight( ent.origin, 225.0f, 1.0f, 1.0f, 0.0f ); //PGM
			else if( effects & EF_TRACKERTRAIL ) //PGM
				View::V_AddLight( ent.origin, 225.0f, -1.0f, -1.0f, -1.0f ); //PGM
			continue;
		}
		// if set to invisible, skip
		if( !s1->modelindex )
			continue;
		if( effects & EF_BFG ) {
			ent.flags |= RF_TRANSLUCENT;
			ent.alpha = 0.30f;
		}
		if( effects & EF_PLASMA ) {
			ent.flags |= RF_TRANSLUCENT;
			ent.alpha = 0.6f;
		}
		if( effects & EF_SPHERETRANS ) {
			ent.flags |= RF_TRANSLUCENT;
			// PMM - * sigh*  yet more EF overloading
			if( effects & EF_TRACKERTRAIL )
				ent.alpha = 0.6f;
			else
				ent.alpha = 0.3f;
		}
		// add to refresh list
		View::V_AddEntity( ent );
		// color shells generate a seperate entity for the main model
		if( effects & EF_COLOR_SHELL ) {
			// PMM - at this point, all of the shells have been handled
			// if we're in the rogue pack, set up the custom mixing, otherwise just
			// keep going
			//			if( Developer_searchpath( 2 ) == 2 )
			//			{
			// all of the solo colors are fine.  we need to catch any of the combinations that look bad
			//( double & half ) and turn them into the appropriate color, and make double/quad something special
			
			//			}
			// pmm
			ent.flags = renderfx | RF_TRANSLUCENT;
			ent.alpha = 0.30f;
			View::V_AddEntity( ent );
		}
		ent.skin = NULL; // never use a custom skin on others
		ent.skinnum = 0;
		ent.flags = 0;
		ent.alpha = 0;
		// duplicate for linked models
		if( s1->modelindex2 ) 	{
			if( s1->modelindex2 == 255 ) { // custom weapon
				ci = &cl.clientinfo[s1->skinnum & 0xFF];
				i =( s1->skinnum >> 8 ); // 0 is default weapon model
				ent.model = ci->weaponmodel[ i ];
				if( !ent.model ) {
					if( i != 0 )
						ent.model = ci->weaponmodel[ 0 ];
					if( !ent.model )
						ent.model = cl.baseclientinfo.weaponmodel[ 0 ];
				}
			}
			else
				ent.model = cl.model_draw[ s1->modelindex2 ];
			// PMM - check for the defender sphere shell .. make it translucent
			// replaces the previous version which used the high bit on modelindex2 to determine transparency
			if( cl.configstrings[ CS_MODELS + s1->modelindex2 ] == "models/items/shell/tris.md2" ) {
				ent.alpha = 0.32f;
				ent.flags = RF_TRANSLUCENT;
			}
			View::V_AddEntity( ent );
			//PGM - make sure these get reset.
			ent.flags = 0;
			ent.alpha = 0.0f;
			//PGM
		}
		if( s1->modelindex3 ) {
			ent.model = cl.model_draw[ s1->modelindex3 ];
			View::V_AddEntity( ent );
		}
		if( s1->modelindex4 ) {
			ent.model = cl.model_draw[ s1->modelindex4 ];
			View::V_AddEntity( ent );
		}
		if( effects & EF_POWERSCREEN ) {
			ent.model = cl_mod_powerscreen;
			ent.oldframe = 0;
			ent.frame = 0;
			ent.flags |=( RF_TRANSLUCENT | RF_SHELL_GREEN );
			ent.alpha = 0.30f;
			View::V_AddEntity( ent );
		}
		// add automatic particle trails
		if( ( effects&~EF_ROTATE ) ) {
			if( effects & EF_ROCKET ) {
				CL_RocketTrail( cent->lerp_origin, ent.origin, cent );
				View::V_AddLight( ent.origin, 200, 1, 1, 0 );
			}
			// PGM - Do not reorder EF_BLASTER and EF_HYPERBLASTER. 
			// EF_BLASTER | EF_TRACKER is a special case for EF_BLASTER2... Cheese!
			else if( effects & EF_BLASTER ) {
				if( effects & EF_TRACKER ) { // lame... problematic?
					CL_BlasterTrail2( cent->lerp_origin, ent.origin );
					View::V_AddLight( ent.origin, 200, 0, 1, 0 );
				} else {
					CL_BlasterTrail( cent->lerp_origin, ent.origin );
					View::V_AddLight( ent.origin, 200, 1, 1, 0 );
				}
			} else if( effects & EF_HYPERBLASTER ) {
				if( effects & EF_TRACKER ) // PGM	overloaded for blaster2.
					View::V_AddLight( ent.origin, 200, 0, 1, 0 ); // PGM
				else // PGM
					View::V_AddLight( ent.origin, 200, 1, 1, 0 );
			} else if( effects & EF_GIB ) {
				CL_DiminishingTrail( cent->lerp_origin, ent.origin, cent, effects );
			} else if( effects & EF_GRENADE ) {
				CL_DiminishingTrail( cent->lerp_origin, ent.origin, cent, effects );
			} else if( effects & EF_FLIES ) {
				CL_FlyEffect( cent, ent.origin );
			} else if( effects & EF_BFG ) {
				static int			bfg_lightramp[ 6 ] = {300, 400, 600, 300, 150, 75};
				i = bfg_lightramp[ s1->frame ];
				View::V_AddLight( ent.origin, ( float )i, 0.0f, 1.0f, 0.0f );
			} else if( effects & EF_TRAP ) {
				ent.origin[ 2 ] += 32;
				i =( rand( )%100 ) + 100;
				View::V_AddLight( ent.origin, ( float )i, 1.0f, 0.8f, 0.1f );
			} else if( effects & EF_FLAG1 ) {
				CL_FlagTrail( cent->lerp_origin, ent.origin, 242.0f );
				View::V_AddLight( ent.origin, 225.0f, 1.0f, 0.1f, 0.1f );
			} else if( effects & EF_FLAG2 ) {
				CL_FlagTrail( cent->lerp_origin, ent.origin, 115.0f );
				View::V_AddLight( ent.origin, 225.0f, 0.1f, 0.1f, 1.0f );
			} else if( effects & EF_TAGTRAIL ) {
				CL_TagTrail( cent->lerp_origin, ent.origin, 220.0f );
				View::V_AddLight( ent.origin, 225.0f, 1.0f, 1.0f, 0.0f );
			} else if( effects & EF_TRACKERTRAIL ) {
				if( effects & EF_TRACKER ) {
					float intensity = 50.0f +( 500.0f * ( sin( cl.time / 500.0f ) + 1.0f ) );
					// FIXME - check out this effect in rendition
					View::V_AddLight( ent.origin, intensity, -1.0f, -1.0f, -1.0f );
				} else {
					CL_Tracker_Shell( cent->lerp_origin );
					View::V_AddLight( ent.origin, 155.0f, -1.0f, -1.0f, -1.0f );
				}
			} else if( effects & EF_TRACKER ) {
				CL_TrackerTrail( cent->lerp_origin, ent.origin, 0 );
				// FIXME - check out this effect in rendition
				View::V_AddLight( ent.origin, 200.0f, -1.0f, -1.0f, -1.0f );
			} else if( effects & EF_GREENGIB ) {
				CL_DiminishingTrail( cent->lerp_origin, ent.origin, cent, effects );
			} else if( effects & EF_IONRIPPER ) {
				CL_IonripperTrail( cent->lerp_origin, ent.origin );
				View::V_AddLight( ent.origin, 100.0f, 1.0f, 0.5f, 0.5f );
			} else if( effects & EF_BLUEHYPERBLASTER ) {
				View::V_AddLight( ent.origin, 200.0f, 0.0f, 0.0f, 1.0f );
			} else if( effects & EF_PLASMA ) {
				if( effects & EF_ANIM_ALLFAST ) {
					CL_BlasterTrail( cent->lerp_origin, ent.origin );
				}
				View::V_AddLight( ent.origin, 130.0f, 1.0f, 0.5f, 0.5f );
			}
		}
		cent->lerp_origin = ent.origin;
	}
}

void Client::CL_AddViewWeapon( player_state_t * ps, player_state_t * ops ) {
	SceneEntity gun; // view model
	int i;
	// allow the gun to be completely removed
	if( !cl_gun.GetBool( ) )
		return;
	// don't draw gun if in wide angle view
	if( ps->fov > 90 )
		return;
	gun.Clear( );
	if( View::gun_model )
		gun.model = View::gun_model; // development tool
	else
		gun.model = cl.model_draw[ ps->gunindex ];
	if( !gun.model )
		return;
	// set up gun position
	for( i = 0; i < 3; i++ ) {
		gun.origin[ i ] = cl.refdef.vieworg[ i ] + ops->gunoffset[ i ]
		+ cl.lerpfrac * ( ps->gunoffset[ i ] - ops->gunoffset[ i ] );
		gun.angles[ i ] = cl.refdef.viewangles[ i ] + LerpAngle( ops->gunangles[ i ], ps->gunangles[ i ], cl.lerpfrac );
	}
	if( View::gun_frame ) {
		gun.frame = View::gun_frame; // development tool
		gun.oldframe = View::gun_frame; // development tool
	} else {
		gun.frame = ps->gunframe;
		if( gun.frame == 0 )
			gun.oldframe = 0; // just changed weapons, don't lerp from old
		else
			gun.oldframe = ops->gunframe;
	}
	gun.flags = RF_MINLIGHT | RF_DEPTHHACK | RF_WEAPONMODEL;
	gun.backlerp = 1.0f - cl.lerpfrac;
	gun.oldorigin = gun.origin; // don't lerp at all
	View::V_AddEntity( gun );
}

void Client::CL_CalcViewValues( ) {
	// find the previous frame to interpolate from
	player_state_t * ps = &cl.frame.playerstate;
	frame_t * oldframe = &cl.frames[ ( cl.frame.serverframe - 1 ) & UPDATE_MASK ];
	if( oldframe->serverframe != cl.frame.serverframe-1 || !oldframe->valid )
		oldframe = &cl.frame; // previous frame was dropped or involid
	player_state_t * ops = &oldframe->playerstate;
	// see if the player entity was teleported this frame
	if( abs( ops->pmove.origin[ 0 ] - ps->pmove.origin[ 0 ] ) > 256 * 8 || abs( ops->pmove.origin[ 1 ] - ps->pmove.origin[ 1 ] ) > 256 * 8
		|| abs( ops->pmove.origin[ 2 ] - ps->pmove.origin[ 2 ] ) > 256 * 8 )
		ops = ps; // don't interpolate
	centity_t * ent = &cl_entities[ cl.playernum+1 ];
	float lerp = cl.lerpfrac;
	// calculate the origin
	if( ( cl_predict.GetBool( ) ) && !( cl.frame.playerstate.pmove.pm_flags & PMF_NO_PREDICTION ) ) { // use predicted values
		unsigned delta;
		float backlerp = 1.0f - lerp;
		for( int i = 0; i < 3; i++ ) {
			cl.refdef.vieworg[ i ] = cl.predicted_origin[ i ] + ops->viewoffset[ i ] + cl.lerpfrac * ( ps->viewoffset[ i ] - ops->viewoffset[ i ] ) - backlerp * cl.prediction_error[ i ];
		}
		// smooth out stair climbing
		delta = ( unsigned int )( cls.realtime - cl.predicted_step_time );
		if( delta < 100 )
			cl.refdef.vieworg[ 2 ] -= cl.predicted_step * ( 100 - delta ) * 0.01f;
	} else { // just use interpolated values
		for( int i = 0; i<3; i++ )
			cl.refdef.vieworg[ i ] = ops->pmove.origin[ i ]* 0.125f + ops->viewoffset[ i ] + lerp * ( ps->pmove.origin[ i ]* 0.125f + ps->viewoffset[ i ] -( ops->pmove.origin[ i ]* 0.125f + ops->viewoffset[ i ] ) );
	}
	// if not running a demo or on a locked frame, add the local angle movement
	if( cl.frame.playerstate.pmove.pm_type < PM_DEAD ) { // use predicted values
		for( int i = 0; i<3; i++ )
			cl.refdef.viewangles[ i ] = cl.predicted_angles[ i ];
	} else { // just use interpolated values
		for( int i = 0; i<3; i++ )
			cl.refdef.viewangles[ i ] = LerpAngle( ops->viewangles[ i ], ps->viewangles[ i ], lerp );
	}
	for( int i = 0; i<3; i++ )
		cl.refdef.viewangles[ i ] += LerpAngle( ops->kick_angles[ i ], ps->kick_angles[ i ], lerp );
	cl.refdef.viewangles.AngleVectors( &cl.v_forward, &cl.v_right, &cl.v_up );
	// interpolate field of view
	cl.refdef.fov_x = ops->fov + lerp * ( ps->fov - ops->fov );
	// don't interpolate blend color
	for( int i = 0; i < 4; i++ )
		cl.refdef.blend[ i ] = ps->blend[ i ];
	cl.refdef.d_zNear = 4.0;
	cl.refdef.d_zFar = 20000.0f;
	// add the weapon
	CL_AddViewWeapon( ps, ops );
}

void Client::CL_AddEntities( ) {
	if( cls.state != ca_active )
		return;
	if( cl.time > cl.frame.servertime ) {
		if( CVarSystem::GetCVarBool( "showclamp" ) )
			Common::Com_Printf( "high clamp %i\n", cl.time - cl.frame.servertime );
		cl.time = cl.frame.servertime;
		cl.lerpfrac = 1.0;
	} else if( cl.time < cl.frame.servertime - 100 ) {
		if( CVarSystem::GetCVarBool( "showclamp" ) )
			Common::Com_Printf( "low clamp %i\n", cl.frame.servertime-100 - cl.time );
		cl.time = cl.frame.servertime - 100;
		cl.lerpfrac = 0;
	} else
		cl.lerpfrac = 1.0f -( cl.frame.servertime - cl.time ) * 0.01f;
	if( cl_timedemo.GetBool( ) )
		cl.lerpfrac = 1.0f;
	CL_CalcViewValues( );
	// PMM - moved this here so the heat beam has the right values for the vieworg, and can lock the beam to the gun
	CL_AddPacketEntities( &cl.frame );
	CL_AddTEnts( );
	CL_AddParticles( );
	CL_AddDLights( );
	CL_AddLightStyles( );
}

Vec3 Client::CL_GetEntitySoundOrigin( int ent ) {
	centity_t * old;
	if( ent < 0 || ent >= MAX_EDICTS )
		Common::Com_Error( ERR_DROP, "CL_GetEntitySoundOrigin: bad ent" );
	old = &cl_entities[ ent ];
	return old->lerp_origin;
	// FIXME: bmodel issues...
}
