#include "precompiled.h"
#pragma hdrstop

void Game::UpdateChaseCam( Entity * ent ) {

	Vec3 o, ownerv, goal;
	Entity * targ;
	Vec3 forward, right;
	CTrace trace;
	int i;
	Vec3 oldgoal;
	Vec3 angles;

	// is our chase target gone?
	if( !ent->client->chase_target->inuse
		|| ent->client->chase_target->client->resp.spectator ) {
			Entity * old = ent->client->chase_target;
			ChaseNext( ent );
			if( ent->client->chase_target == old ) {
				ent->client->chase_target = NULL;
				ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
				return;
			}
	}

	targ = ent->client->chase_target;

	ownerv = targ->s.origin;
	oldgoal = ent->s.origin;

	ownerv[ 2 ] += targ->viewheight;

	angles = targ->client->v_angle;
	if( angles[ PITCH ] > 56 )
		angles[ PITCH ] = 56;
	angles.AngleVectors( &forward, &right, NULL );
	forward.Normalize( );
	o = ownerv.Magnitude( -30.0f, forward );

	if( o[ 2 ] < targ->s.origin[ 2 ] + 20 )
		o[ 2 ] = targ->s.origin[ 2 ] + 20;

	// jump animation lifts
	if( !targ->groundentity )
		o[ 2 ] += 16;

	trace = Server::SV_Trace( ownerv, vec3_origin, vec3_origin, o, targ, MASK_SOLID );

	goal = trace.endpos;

	goal = goal.Magnitude( 2.0f, forward );

	// pad for floors and ceilings
	o = goal;
	o[ 2 ] += 6;
	trace = Server::SV_Trace( goal, vec3_origin, vec3_origin, o, targ, MASK_SOLID );
	if( trace.fraction < 1 ) {
		goal = trace.endpos;
		goal[ 2 ] -= 6;
	}

	o = goal;
	o[ 2 ] -= 6;
	trace = Server::SV_Trace( goal, vec3_origin, vec3_origin, o, targ, MASK_SOLID );
	if( trace.fraction < 1 ) {
		goal = trace.endpos;
		goal[ 2 ] += 6;
	}

	if( targ->deadflag )
		ent->client->ps.pmove.pm_type = PM_DEAD;
	else
		ent->client->ps.pmove.pm_type = PM_FREEZE;

	ent->s.origin = goal;
	for( i = 0; i<3; i++ )
		ent->client->ps.pmove.delta_angles[ i ] = ANGLE2SHORT( targ->client->v_angle[ i ] - ent->client->resp.cmd_angles[ i ] );

	if( targ->deadflag ) {
		ent->client->ps.viewangles[ ROLL ] = 40;
		ent->client->ps.viewangles[ PITCH ] = -15;
		ent->client->ps.viewangles[ YAW ] = targ->client->killer_yaw;
	} else {
		ent->client->ps.viewangles = targ->client->v_angle;
		ent->client->v_angle = targ->client->v_angle;
	}

	ent->viewheight = 0;
	ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	Server::SV_LinkEdict( ent );
}

void Game::ChaseNext( Entity * ent ) {

	int i;
	Entity * e;

	if( !ent->client->chase_target )
		return;

	i = ( int )( ent->client->chase_target - g_edicts );
	do {
		i++;
		if( i > Common::maxclients.GetInt( ) )
			i = 1;
		e = g_edicts + i;
		if( !e->inuse )
			continue;
		if( !e->client->resp.spectator )
			break;
	} while( e != ent->client->chase_target );

	ent->client->chase_target = e;
	ent->client->update_chase = true;
}

void Game::ChasePrev( Entity * ent ) {

	int i;
	Entity * e;

	if( !ent->client->chase_target )
		return;

	i = ( int )( ent->client->chase_target - g_edicts );
	do {
		i--;
		if( i < 1 )
			i = Common::maxclients.GetInt( );
		e = g_edicts + i;
		if( !e->inuse )
			continue;
		if( !e->client->resp.spectator )
			break;
	} while( e != ent->client->chase_target );

	ent->client->chase_target = e;
	ent->client->update_chase = true;
}

void Game::GetChaseTarget( Entity * ent ) {

	int i;
	Entity * other;

	for( i = 1; i <= Common::maxclients.GetInt( ); i++ ) {
		other = g_edicts + i;
		if( other->inuse && !other->client->resp.spectator ) {
			ent->client->chase_target = other;
			ent->client->update_chase = true;
			UpdateChaseCam( ent );
			return;
		}
	}
	Server::PF_centerprintf( ent, "No other players to chase." );
}
