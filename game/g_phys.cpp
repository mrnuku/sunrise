#include "precompiled.h"
#pragma hdrstop

typedef struct {

	Entity * ent;
	Vec3 origin;
	Vec3 angles;
	float deltayaw;
} pushed_t;
pushed_t pushed[ MAX_EDICTS ], * pushed_p;

Entity * obstacle;

/*
pushmove objects do not obey gravity, and do not interact with each other or trigger fields, but block normal movement and push normal objects when they move.

onground is set for toss objects when they come to a complete rest.  it is set for steping or walking objects 

doors, plats, etc are SOLID_BSP, and MOVETYPE_PUSH
bonus items are SOLID_TRIGGER touch, and MOVETYPE_TOSS
corpses are SOLID_NOT and MOVETYPE_TOSS
crates are SOLID_BBOX and MOVETYPE_TOSS
walking monsters are SOLID_SLIDEBOX and MOVETYPE_STEP
flying/floating monsters are SOLID_SLIDEBOX and MOVETYPE_FLY

solid_edge items only clip against bsp models.
*/

Entity * Game::SV_TestEntityPosition( Entity * ent ) {
	CTrace trace;
	int mask;
	if( ent->clipmask )
		mask = ent->clipmask;
	else
		mask = MASK_SOLID;
	trace = Server::SV_Trace( ent->s.origin, ent->mins, ent->maxs, ent->s.origin, ent, mask );
	if( trace.startsolid )
		return g_edicts;
	return NULL;
}

void Game::SV_CheckVelocity( Entity * ent ) {
	// bound velocity
	for( int i = 0; i<3; i++ ) {
		if( ent->velocity[ i ] > sv_maxvelocity.GetFloat( ) )
			ent->velocity[ i ] = sv_maxvelocity.GetFloat( );
		else if( ent->velocity[ i ] < -sv_maxvelocity.GetFloat( ) )
			ent->velocity[ i ] = -sv_maxvelocity.GetFloat( );
	}
}

bool Game::SV_RunThink( Entity * ent ) {
	timeTypeReal thinktime;
	thinktime = ent->nextthink;
	if( thinktime <= 0 )
		return true;
	if( thinktime > level.time + 0.001 )
		return true;
	ent->nextthink = 0;
	if( !ent->think )
		Server::PF_error( "NULL ent->think" );
	ent->think( ent );
	return false;
}

void Game::SV_Impact( Entity * e1, CTrace & trace ) {
	Entity * e2;
	//	Plane	backplane;
	e2 = trace.ent;
	if( e1->touch && e1->solid != SOLID_NOT )
		e1->touch( e1, e2, trace.plane, SURF_NULL );
	if( e2->touch && e2->solid != SOLID_NOT )
		e2->touch( e2, e1, plane_origin, SURF_NULL );
}

int Game::ClipVelocity( Vec3 & in, Vec3 & normal, Vec3 * out, float overbounce ) {
	float backoff;
	float change;
	int i, blocked;
	blocked = 0;
	if( normal[ 2 ] > 0 )
		blocked |= 1; // floor
	if( !normal[ 2 ] )
		blocked |= 2; // step
	backoff =( in * normal ) * overbounce;
	for( i = 0; i<3; i++ ) {
		change = normal[ i ]* backoff;
		out->ToFloatPtr( )[ i ] = in[ i ] - change;
		if( out->ToFloatPtr( )[ i ] > -0.1f && out->ToFloatPtr( )[ i ] < 0.1f )
			out->ToFloatPtr( )[ i ] = 0;
	}
	return blocked;
}

int Game::SV_FlyMove( Entity * ent, float time, int mask ) {
	Entity * hit;
	int bumpcount, numbumps;
	Vec3 dir;
	float d;
	int numplanes;
	Vec3 planes[ 5 ];
	Vec3 primal_velocity, original_velocity, new_velocity;
	int i, j;
	CTrace trace;
	Vec3 end;
	float time_left;
	int blocked;
	numbumps = 4;
	blocked = 0;
	original_velocity = ent->velocity;
	primal_velocity = ent->velocity;
	numplanes = 0;
	time_left = time;
	ent->groundentity = NULL;
	for( bumpcount = 0; bumpcount<numbumps; bumpcount++ ) {
		for( i = 0; i<3; i++ )
			end[ i ] = ent->s.origin[ i ] + time_left * ent->velocity[ i ];
		trace = Server::SV_Trace( ent->s.origin, ent->mins, ent->maxs, end, ent, mask );
		if( trace.allsolid ) { // entity is trapped in another solid
			ent->velocity = vec3_origin;
			return 3;
		}
		if( trace.fraction > 0 ) { // actually covered some distance
			ent->s.origin = trace.endpos;
			original_velocity = ent->velocity;
			numplanes = 0;
		}
		if( trace.fraction == 1 )
			break; // moved the entire distance
		hit = trace.ent;
		if( trace.plane.Normal( )[ 2 ] > 0.7f ) {
			blocked |= 1; // floor
			if( hit->solid == SOLID_BSP ) {
				ent->groundentity = hit;
				ent->groundentity_linkcount = hit->linkcount;
			}
		}
		if( !trace.plane.Normal( )[ 2 ] ) {
			blocked |= 2; // step
		}
		// run the impact function
		SV_Impact( ent, trace );
		if( !ent->inuse )
			break; // removed by the impact function
		time_left -= time_left * trace.fraction;
		// cliped to another plane
		if( numplanes >= 5 ) { // this shouldn't really happen
			ent->velocity = vec3_origin;
			return 3;
		}
		planes[ numplanes ] = trace.plane.Normal( );
		numplanes++;
		// modify original_velocity so it parallels all of the clip planes
		for( i = 0; i<numplanes; i++ ) {
			ClipVelocity( original_velocity, planes[ i ], &new_velocity, 1 );
			for( j = 0; j<numplanes; j++ ) {
				if( ( j != i ) && planes[ i ] != planes[ j ] ) {
					if( new_velocity * planes[ j ] < 0.0f )
						break; // not ok
				}
			}
			if( j == numplanes )
				break;
		}
		if( i != numplanes ) // go along this plane
			ent->velocity = new_velocity;
		else { // go along the crease
			if( numplanes != 2 ) {
				//				Common::Com_DPrintf( "clip velocity, numplanes == %i\n", numplanes );
				ent->velocity = vec3_origin;
				return 7;
			}
			dir = planes[ 0 ].Cross( planes[ 1 ] );
			d = dir * ent->velocity;
			ent->velocity = dir * d;
		}
		// if original velocity is against the original velocity, stop dead
		// to avoid tiny occilations in sloping corners
		if( ent->velocity * primal_velocity <= 0.0f ) {
			ent->velocity = vec3_origin;
			return blocked;
		}
	}
	return blocked;
}

void Game::SV_AddGravity( Entity * ent ) {
	ent->velocity[ 2 ] -= ent->gravity * sv_gravity.GetFloat( ) * FRAMETIME;
}

CTrace Game::SV_PushEntity( Entity * ent, Vec3 & push ) {
	CTrace trace;
	Vec3 start;
	Vec3 end;
	int mask;
	start = ent->s.origin;
	end = start + push;
retry:
	if( ent->clipmask )
		mask = ent->clipmask;
	else
		mask = MASK_SOLID;
	trace = Server::SV_Trace( start, ent->mins, ent->maxs, end, ent, mask );
	ent->s.origin = trace.endpos;
	Server::SV_LinkEdict( ent );
	if( trace.fraction != 1.0f ) {
		SV_Impact( ent, trace );
		// if the pushed entity went away and the pusher is still there
		if( !trace.ent->inuse && ent->inuse ) {
			// move the pusher back and try again
			ent->s.origin = start;
			Server::SV_LinkEdict( ent );
			goto retry;
		}
	}
	if( ent->inuse )
		G_TouchTriggers( ent );
	return trace;
}

bool Game::SV_Push( Entity * pusher, Vec3 & move, Vec3 & amove ) {
	int i, e;
	Entity * check, * block;
	Vec3 mins, maxs;
	pushed_t * p;
	Vec3 org, org2, move2, forward, right, up;
	// clamp the move to 1/8 units, so the position will
	// be accurate for client side prediction
	for( i = 0; i<3; i++ ) {
		float temp;
		temp = move[ i ]* 8.0f;
		if( temp > 0.0f )
			temp += 0.5f;
		else
			temp -= 0.5f;
		move[ i ] = 0.125f * ( int )temp;
	}
	// find the bounding box
	for( i = 0; i<3; i++ ) {
		mins[ i ] = pusher->absmin[ i ] + move[ i ];
		maxs[ i ] = pusher->absmax[ i ] + move[ i ];
	}
	// we need this for pushing things later
	org = vec3_origin - amove;
	org.AngleVectors( &forward, &right, &up );
	// save the pusher's original position
	pushed_p->ent = pusher;
	pushed_p->origin = pusher->s.origin;
	pushed_p->angles = pusher->s.angles;
	if( pusher->client )
		pushed_p->deltayaw = pusher->client->ps.pmove.delta_angles[ YAW ];
	pushed_p++;
	// move the pusher to it's final position
	pusher->s.origin = pusher->s.origin + move;
	pusher->s.angles = pusher->s.angles + amove;
	Server::SV_LinkEdict( pusher );
	// see if any solid entities are inside the final position
	check = g_edicts+1;
	for( e = 1; e < num_edicts; e++, check++ ) {
		if( !check->inuse )
			continue;
		if( check->movetype == MOVETYPE_PUSH || check->movetype == MOVETYPE_STOP ||
			check->movetype == MOVETYPE_NONE || check->movetype == MOVETYPE_NOCLIP )
			continue;
		if( !check->area.prev )
			continue; // not linked in anywhere
		// if the entity is standing on the pusher, it will definitely be moved
		if( check->groundentity != pusher ) {
			// see if the ent needs to be tested
			if( check->absmin[ 0 ] >= maxs[ 0 ] || check->absmin[ 1 ] >= maxs[ 1 ] || check->absmin[ 2 ] >= maxs[ 2 ] ||
				check->absmax[ 0 ] <= mins[ 0 ] || check->absmax[ 1 ] <= mins[ 1 ] || check->absmax[ 2 ] <= mins[ 2 ] )
				continue;
			// see if the ent's bbox is inside the pusher's final position
			if( !SV_TestEntityPosition( check ) )
				continue;
		}
		if( ( pusher->movetype == MOVETYPE_PUSH ) ||( check->groundentity == pusher ) ) {
			// move this entity
			pushed_p->ent = check;
			pushed_p->origin = check->s.origin;
			pushed_p->angles = check->s.angles;
			pushed_p++;
			// try moving the contacted entity 
			check->s.origin = check->s.origin + move;
			if( check->client ) // FIXME: doesn't rotate monsters?
				check->client->ps.pmove.delta_angles[ YAW ] += ( short )amove[ YAW ];
			// figure movement due to the pusher's amove
			org = check->s.origin - pusher->s.origin;
			org2[ 0 ] = org * forward;
			org2[ 1 ] = -( org * right );
			org2[ 2 ] = org * up;
			move2 = org2 - org;
			check->s.origin = check->s.origin + move2;
			// may have pushed them off an edge
			if( check->groundentity != pusher )
				check->groundentity = NULL;
			block = SV_TestEntityPosition( check );
			if( !block ) { // pushed ok
				Server::SV_LinkEdict( check );
				// impact?
				continue;
			}
			// if it is ok to leave in the old position, do it
			// this is only relevent for riding entities, not pushed
			// FIXME: this doesn't acount for rotation
			check->s.origin = check->s.origin - move;
			block = SV_TestEntityPosition( check );
			if( !block ) {
				pushed_p--;
				continue;
			}
		}
		// save off the obstacle so we can call the block function
		obstacle = check;
		// move back any entities we already moved
		// go backwards, so if the same entity was pushed
		// twice, it goes back to the original position
		for( p = pushed_p-1; p>= pushed; p-- ) {
			p->ent->s.origin = p->origin;
			p->ent->s.angles = p->angles;
			if( p->ent->client )
				p->ent->client->ps.pmove.delta_angles[ YAW ] = ( short )p->deltayaw;
			Server::SV_LinkEdict( p->ent );
		}
		return false;
	}
	//FIXME: is there a better way to handle this?
	// see if anything we moved has touched a trigger
	for( p = pushed_p-1; p>= pushed; p-- )
		G_TouchTriggers( p->ent );
	return true;
}

void Game::SV_Physics_Pusher( Entity * ent ) {
	Vec3 move, amove;
	Entity * part, * mv;
	// if not a team captain, so movement will be handled elsewhere
	if( ent->flags & FL_TEAMSLAVE )
		return;
	// make sure all team slaves can move before commiting
	// any moves or calling any think functions
	// if the move is blocked, all moved objects will be backed out
	//retry:
	pushed_p = pushed;
	for( part = ent; part; part = part->teamchain ) {
		if( part->velocity[ 0 ] || part->velocity[ 1 ] || part->velocity[ 2 ] ||
			part->avelocity[ 0 ] || part->avelocity[ 1 ] || part->avelocity[ 2 ] ) { // object is moving
			move = part->velocity * FRAMETIME;
			amove = part->avelocity * FRAMETIME;
			if( !SV_Push( part, move, amove ) )
				break; // move was blocked
		}
	}
	if( pushed_p > &pushed[ MAX_EDICTS ] )
		Server::PF_error( "pushed_p > &pushed[ MAX_EDICTS ], memory corrupted" );
	if( part ) {
		// the move failed, bump all nextthink times and back out moves
		for( mv = ent; mv; mv = mv->teamchain ) {
			if( mv->nextthink > 0 )
				mv->nextthink += FRAMETIME;
		}
		// if the pusher has a "blocked" function, call it
		// otherwise, just stay in place until the obstacle is gone
		if( part->blocked )
			part->blocked( part, obstacle );
	} else {
		// the move succeeded, so call all think functions
		for( part = ent; part; part = part->teamchain ) {
			SV_RunThink( part );
		}
	}
}

void Game::SV_Physics_None( Entity * ent ) {
	// regular thinking
	SV_RunThink( ent );
}

void Game::SV_Physics_Noclip( Entity * ent ) {
	// regular thinking
	if( !SV_RunThink( ent ) )
		return;
	ent->s.angles = ent->s.angles.Magnitude( FRAMETIME, ent->avelocity );
	ent->s.origin = ent->s.origin.Magnitude( FRAMETIME, ent->velocity );
	Server::SV_LinkEdict( ent );
}

void Game::SV_Physics_Toss( Entity * ent ) {
	CTrace trace;
	Vec3 move;
	float backoff;
	Entity * slave;
	bool wasinwater;
	bool isinwater;
	Vec3 old_origin;
	// regular thinking
	SV_RunThink( ent );
	// if not a team captain, so movement will be handled elsewhere
	if( ent->flags & FL_TEAMSLAVE )
		return;
	if( ent->velocity[ 2 ] > 0 )
		ent->groundentity = NULL;
	// check for the groundentity going away
	if( ent->groundentity )
		if( !ent->groundentity->inuse )
			ent->groundentity = NULL;
	// if onground, return without moving
	if( ent->groundentity )
		return;
	old_origin = ent->s.origin;
	SV_CheckVelocity( ent );
	// add gravity
	if( ent->movetype != MOVETYPE_FLY && ent->movetype != MOVETYPE_FLYMISSILE )
		SV_AddGravity( ent );
	// move angles
	ent->s.angles = ent->s.angles.Magnitude( FRAMETIME, ent->avelocity );
	// move origin
	move = ent->velocity * FRAMETIME;
	trace = SV_PushEntity( ent, move );
	if( !ent->inuse )
		return;
	if( trace.fraction < 1 ) {
		if( ent->movetype == MOVETYPE_BOUNCE )
			backoff = 1.5f;
		else
			backoff = 1;
		ClipVelocity( ent->velocity, trace.plane.Normal( ), &ent->velocity, backoff );
		// stop if on ground
		if( trace.plane.Normal( )[ 2 ] > 0.7f ) {
			if( ent->velocity[ 2 ] < 60 || ent->movetype != MOVETYPE_BOUNCE ) {
				ent->groundentity = trace.ent;
				ent->groundentity_linkcount = trace.ent->linkcount;
				ent->velocity = vec3_origin;
				ent->avelocity = vec3_origin;
			}
		}
		//		if( ent->touch )
		//			ent->touch( ent, trace.ent, &trace.plane, trace.surface );
	}
	// check for water transition
	wasinwater = ( ent->watertype & MASK_WATER ) > 0;
	ent->watertype = Server::SV_PointContents( ent->s.origin );
	isinwater = ( ent->watertype & MASK_WATER ) > 0;
	if( isinwater )
		ent->waterlevel = 1;
	else
		ent->waterlevel = 0;
	if( !wasinwater && isinwater )
		Server::SV_StartSound( &old_origin, g_edicts, CHAN_AUTO, Server::SV_SoundIndex( "misc/h2ohit1.wav" ), 1, 1, 0 );
	else if( wasinwater && !isinwater )
		Server::SV_StartSound( &ent->s.origin, g_edicts, CHAN_AUTO, Server::SV_SoundIndex( "misc/h2ohit1.wav" ), 1, 1, 0 );
	// move teamslaves
	for( slave = ent->teamchain; slave; slave = slave->teamchain ) {
		slave->s.origin = ent->s.origin;
		Server::SV_LinkEdict( slave );
	}
}

void Game::SV_AddRotationalFriction( Entity * ent ) {
	int n;
	float adjustment;
	ent->s.angles = ent->s.angles.Magnitude( FRAMETIME, ent->avelocity );
	adjustment = FRAMETIME * 100 * 6;
	for( n = 0; n < 3; n++ ) {
		if( ent->avelocity[ n ] > 0 ) {
			ent->avelocity[ n ] -= adjustment;
			if( ent->avelocity[ n ] < 0 )
				ent->avelocity[ n ] = 0;
		} else {
			ent->avelocity[ n ] += adjustment;
			if( ent->avelocity[ n ] > 0 )
				ent->avelocity[ n ] = 0;
		}
	}
}

void Game::SV_Physics_Step( Entity * ent ) {
	bool wasonground;
	bool hitsound = false;
	float speed, newspeed, control;
	float friction;
	Entity * groundentity;
	int mask;
	// airborn monsters should always check for ground
	if( !ent->groundentity )
		M_CheckGround( ent );
	groundentity = ent->groundentity;
	SV_CheckVelocity( ent );
	if( groundentity )
		wasonground = true;
	else
		wasonground = false;
	if( ent->avelocity[ 0 ] || ent->avelocity[ 1 ] || ent->avelocity[ 2 ] )
		SV_AddRotationalFriction( ent );
	// add gravity except:
	//   flying monsters
	//   swimming monsters who are in the water
	if( ! wasonground )
		if( !( ent->flags & FL_FLY ) )
			if( !( ( ent->flags & FL_SWIM ) &&( ent->waterlevel > 2 ) ) ) {
				if( ent->velocity[ 2 ] < sv_gravity.GetFloat( ) * -0.1f )
					hitsound = true;
				if( ent->waterlevel == 0 )
					SV_AddGravity( ent );
			}
			// friction for flying monsters that have been given vertical velocity
			if( ( ent->flags & FL_FLY ) &&( ent->velocity[ 2 ] != 0 ) ) {
				speed = fabs( ent->velocity[ 2 ] );
				control = speed < 100 ? 100 : speed;
				friction = 6/3;
				newspeed = speed -( FRAMETIME * control * friction );
				if( newspeed < 0 )
					newspeed = 0;
				newspeed /= speed;
				ent->velocity[ 2 ] *= newspeed;
			}
			// friction for flying monsters that have been given vertical velocity
			if( ( ent->flags & FL_SWIM ) &&( ent->velocity[ 2 ] != 0 ) ) {
				speed = fabs( ent->velocity[ 2 ] );
				control = speed < 100 ? 100 : speed;
				newspeed = speed -( FRAMETIME * control * 1 * ent->waterlevel );
				if( newspeed < 0 )
					newspeed = 0;
				newspeed /= speed;
				ent->velocity[ 2 ] *= newspeed;
			}
			if( ent->velocity[ 2 ] || ent->velocity[ 1 ] || ent->velocity[ 0 ] ) {
				// apply friction
				// let dead monsters who aren't completely onground slide
				if( ( wasonground ) ||( ent->flags &( FL_SWIM|FL_FLY ) ) ) {
					if( !( ent->health <= 0.0f && !M_CheckBottom( ent ) ) ) {
						speed = sqrt( ent->velocity[ 0 ] * ent->velocity[ 0 ] + ent->velocity[ 1 ] * ent->velocity[ 1 ] );
						if( speed ) {
							friction = 6;
							control = speed < 100 ? 100 : speed;
							newspeed = speed - FRAMETIME* control* friction;
							if( newspeed < 0 )
								newspeed = 0;
							newspeed /= speed;
							ent->velocity[ 0 ] *= newspeed;
							ent->velocity[ 1 ] *= newspeed;
						}
					}
					if( ent->svflags & SVF_MONSTER )
						mask = MASK_MONSTERSOLID;
					else
						mask = MASK_SOLID;
					SV_FlyMove( ent, FRAMETIME, mask );
					Server::SV_LinkEdict( ent );
					G_TouchTriggers( ent );
					if( !ent->inuse )
						return;
					if( ent->groundentity ) {
						if( !wasonground ) {
							if( hitsound )
								Server::PF_StartSound( ent, 0, Server::SV_SoundIndex( "world/land.wav" ), 1, 1, 0 );
						}
					}
				}
			}
			// regular thinking
			SV_RunThink( ent );
}

void Game::G_RunEntity( Entity * ent ) {
	if( ent->prethink )
		ent->prethink( ent );
	switch( ent->movetype ) {
		case MOVETYPE_PUSH:
		case MOVETYPE_STOP:
			SV_Physics_Pusher( ent );
			break;
		case MOVETYPE_NONE:
			SV_Physics_None( ent );
			break;
		case MOVETYPE_NOCLIP:
			SV_Physics_Noclip( ent );
			break;
		case MOVETYPE_STEP:
			SV_Physics_Step( ent );
			break;
		case MOVETYPE_TOSS:
		case MOVETYPE_BOUNCE:
		case MOVETYPE_FLY:
		case MOVETYPE_FLYMISSILE:
			SV_Physics_Toss( ent );
			break;
		default:
			Server::PF_error( "SV_Physics: bad movetype %i", ent->movetype );
	}
}
