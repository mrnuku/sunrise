#include "precompiled.h"
#pragma hdrstop

/*

=============

M_CheckBottom



Returns false if any part of the bottom of the entity is off an edge that

is not a staircase.



=============

*/
bool Game::M_CheckBottom( Entity * ent ) {

	Vec3 mins, maxs, start, stop;
	CTrace trace;
	int x, y;
	float mid, bottom;

	mins = ent->s.origin + ent->mins;
	maxs = ent->s.origin + ent->maxs;

	// if all of the points under the corners are solid world, don't bother
	// with the tougher checks
	// the corners must be within 16 of the midpoint
	start[ 2 ] = mins[ 2 ] - 1;
	for( x = 0; x<= 1; x++ )
		for( y = 0; y<= 1; y++ )
		{
			start[ 0 ] = x ? maxs[ 0 ] : mins[ 0 ];
			start[ 1 ] = y ? maxs[ 1 ] : mins[ 1 ];
			if( Server::SV_PointContents( start ) != CONTENTS_SOLID )
				goto realcheck;
		}

		return true; // we got out easy

realcheck:
		//
		// check it for real...
		//
		start[ 2 ] = mins[ 2 ];

		// the midpoint must be within 16 of the bottom
		start[ 0 ] = stop[ 0 ] =( mins[ 0 ] + maxs[ 0 ] )* 0.5f;
		start[ 1 ] = stop[ 1 ] =( mins[ 1 ] + maxs[ 1 ] )* 0.5f;
		stop[ 2 ] = start[ 2 ] - 2* 18;
		trace = Server::SV_Trace( start, vec3_origin, vec3_origin, stop, ent, MASK_MONSTERSOLID );

		if( trace.fraction == 1.0f )
			return false;
		mid = bottom = trace.endpos[ 2 ];

		// the corners must be within 16 of the midpoint	
		for( x = 0; x<= 1; x++ )
			for( y = 0; y<= 1; y++ )
			{
				start[ 0 ] = stop[ 0 ] = x ? maxs[ 0 ] : mins[ 0 ];
				start[ 1 ] = stop[ 1 ] = y ? maxs[ 1 ] : mins[ 1 ];

				trace = Server::SV_Trace( start, vec3_origin, vec3_origin, stop, ent, MASK_MONSTERSOLID );

				if( trace.fraction != 1.0f && trace.endpos[ 2 ] > bottom )
					bottom = trace.endpos[ 2 ];
				if( trace.fraction == 1.0f || mid - trace.endpos[ 2 ] > 18 )
					return false;
			}

			return true;
}


/*

=============

SV_movestep



Called by monster program code.

The move will be adjusted for slopes and stairs, but if the move isn't

possible, no move is done, false is returned, and

pr_global_struct->trace_normal is set to the normal of the blocking wall

=============

*/
//FIXME since we need to test end position contents here, can we avoid doing
//it again later in catagorize position?
bool Game::SV_movestep( Entity * ent, Vec3 & move, bool relink ) {

	float dz;
	Vec3 oldorg, neworg, end;
	CTrace trace;
	int i;
	float stepsize;
	Vec3 test;
	int contents;

	// try the move	
	oldorg = ent->s.origin;
	neworg = ent->s.origin + move;

	// flying monsters don't step up
	if( ent->flags &( FL_SWIM | FL_FLY ) )
	{
		// try one move with vertical motion, then one without
		for( i = 0; i<2; i++ )
		{
			neworg = ent->s.origin + move;
			if( i == 0 && ent->enemy )
			{
				if( !ent->goalentity )
					ent->goalentity = ent->enemy;
				dz = ent->s.origin[ 2 ] - ent->goalentity->s.origin[ 2 ];
				if( ent->goalentity->client )
				{
					if( dz > 40 )
						neworg[ 2 ] -= 8;
					if( !( ( ent->flags & FL_SWIM ) &&( ent->waterlevel < 2 ) ) )
						if( dz < 30 )
							neworg[ 2 ] += 8;
				}
				else
				{
					if( dz > 8 )
						neworg[ 2 ] -= 8;
					else if( dz > 0 )
						neworg[ 2 ] -= dz;
					else if( dz < -8 )
						neworg[ 2 ] += 8;
					else
						neworg[ 2 ] += dz;
				}
			}
			trace = Server::SV_Trace( ent->s.origin, ent->mins, ent->maxs, neworg, ent, MASK_MONSTERSOLID );

			// fly monsters don't enter water voluntarily
			if( ent->flags & FL_FLY )
			{
				if( !ent->waterlevel )
				{
					test[ 0 ] = trace.endpos[ 0 ];
					test[ 1 ] = trace.endpos[ 1 ];
					test[ 2 ] = trace.endpos[ 2 ] + ent->mins[ 2 ] + 1;
					contents = Server::SV_PointContents( test );
					if( contents & MASK_WATER )
						return false;
				}
			}

			// swim monsters don't exit water voluntarily
			if( ent->flags & FL_SWIM )
			{
				if( ent->waterlevel < 2 )
				{
					test[ 0 ] = trace.endpos[ 0 ];
					test[ 1 ] = trace.endpos[ 1 ];
					test[ 2 ] = trace.endpos[ 2 ] + ent->mins[ 2 ] + 1;
					contents = Server::SV_PointContents( test );
					if( !( contents & MASK_WATER ) )				return false;
				}
			}

			if( trace.fraction == 1 )
			{
				ent->s.origin = trace.endpos;
				if( relink )
				{
					Server::SV_LinkEdict( ent );
					G_TouchTriggers( ent );
				}
				return true;
			}

			if( !ent->enemy )
				break;
		}

		return false;
	}

	// push down from a step height above the wished position
	if( !( ent->monsterinfo.aiflags & AI_NOSTEP ) ) stepsize = 18;
	else
		stepsize = 1;

	neworg[ 2 ] += stepsize;
	end = neworg;
	end[ 2 ] -= stepsize* 2;

	trace = Server::SV_Trace( neworg, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID );

	if( trace.allsolid )
		return false;

	if( trace.startsolid )
	{
		neworg[ 2 ] -= stepsize;
		trace = Server::SV_Trace( neworg, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID );
		if( trace.allsolid || trace.startsolid )
			return false;
	}


	// don't go in to water
	if( ent->waterlevel == 0 )
	{
		test[ 0 ] = trace.endpos[ 0 ];
		test[ 1 ] = trace.endpos[ 1 ];
		test[ 2 ] = trace.endpos[ 2 ] + ent->mins[ 2 ] + 1;
		contents = Server::SV_PointContents( test );

		if( contents & MASK_WATER )
			return false;
	}

	if( trace.fraction == 1 )
	{
		// if monster had the ground pulled out, go ahead and fall
		if( ent->flags & FL_PARTIALGROUND )
		{
			ent->s.origin = ent->s.origin + move;
			if( relink )
			{
				Server::SV_LinkEdict( ent );
				G_TouchTriggers( ent );
			}
			ent->groundentity = NULL;
			return true;
		}

		return false; // walked off an edge
	}

	// check point traces down for dangling corners
	ent->s.origin = trace.endpos;

	if( !M_CheckBottom( ent ) ) {
		if( ent->flags & FL_PARTIALGROUND )
		{ // entity had floor mostly pulled out from underneath it
			// and is trying to correct
			if( relink )
			{
				Server::SV_LinkEdict( ent );
				G_TouchTriggers( ent );
			}
			return true;
		}
		ent->s.origin = oldorg;
		return false;
	}

	if( ent->flags & FL_PARTIALGROUND )
	{
		ent->flags &= ~FL_PARTIALGROUND;
	}
	ent->groundentity = trace.ent;
	ent->groundentity_linkcount = trace.ent->linkcount;

	// the move is ok
	if( relink )
	{
		Server::SV_LinkEdict( ent );
		G_TouchTriggers( ent );
	}
	return true;
}


//============================================================================

/*

===============

M_ChangeYaw



===============

*/
void Game::M_ChangeYaw( Entity * ent ) {

	float eal;
	float current;
	float move;
	float speed;

	current = anglemod( ent->s.angles[ YAW ] );
	eal = ent->eal_yaw;

	if( current == eal )
		return;

	move = eal - current;
	speed = ent->yaw_speed;
	if( eal > current )
	{
		if( move >= 180 )
			move = move - 360;
	}
	else
	{
		if( move <= -180 )
			move = move + 360;
	}
	if( move > 0 )
	{
		if( move > speed )
			move = speed;
	}
	else
	{
		if( move < -speed )
			move = -speed;
	}

	ent->s.angles[ YAW ] = anglemod( current + move );
}


/*

======================

SV_StepDirection



Turns to the movement direction, and walks the current distance if

facing it.



======================

*/
bool Game::SV_StepDirection( Entity * ent, float yaw, float dist ) {

	Vec3 move, oldorigin;
	float delta;

	ent->eal_yaw = yaw;
	M_ChangeYaw( ent );

	yaw = yaw* M_PI* 2 / 360;
	move[ 0 ] = cos( yaw )* dist;
	move[ 1 ] = sin( yaw )* dist;
	move[ 2 ] = 0;

	oldorigin = ent->s.origin;
	if( SV_movestep( ent, move, false ) ) {
		delta = ent->s.angles[ YAW ] - ent->eal_yaw;
		if( delta > 45 && delta < 315 )
		{ // not turned far enough, so don't take the step
			ent->s.origin = oldorigin;
		}
		Server::SV_LinkEdict( ent );
		G_TouchTriggers( ent );
		return true;
	}
	Server::SV_LinkEdict( ent );
	G_TouchTriggers( ent );
	return false;
}

/*

======================

SV_FixCheckBottom



======================

*/
void Game::SV_FixCheckBottom( Entity * ent ) {

	ent->flags |= FL_PARTIALGROUND;
}



/*

================

SV_NewChaseDir



================

*/
void Game::SV_NewChaseDir( Entity * actor, Entity * enemy, float dist ) {

	float deltax, deltay;
	float d[ 3 ];
	float tdir, olddir, turnaround;

	//FIXME: how did we get here with no enemy
	if( !enemy )
		return;

	olddir = anglemod( ( actor->eal_yaw/45 )* 45 );
	turnaround = anglemod( olddir - 180 );

	deltax = enemy->s.origin[ 0 ] - actor->s.origin[ 0 ];
	deltay = enemy->s.origin[ 1 ] - actor->s.origin[ 1 ];
	if( deltax>10 )
		d[ 1 ]= 0;
	else if( deltax<-10 )
		d[ 1 ]= 180;
	else
		d[ 1 ]= -1;
	if( deltay<-10 )
		d[ 2 ]= 270;
	else if( deltay>10 )
		d[ 2 ]= 90;
	else
		d[ 2 ]= -1;

	// try direct route
	if( d[ 1 ] != -1 && d[ 2 ] != -1 )
	{
		if( d[ 1 ] == 0 )
			tdir = d[ 2 ] == 90.0f ? 45.0f : 315.0f;
		else
			tdir = d[ 2 ] == 90.0f ? 135.0f : 215.0f;

		if( tdir != turnaround && SV_StepDirection( actor, tdir, dist ) ) 	return;
	}

	// try other directions
	if( ( ( rand( )&3 ) & 1 ) || abs( deltay )>abs( deltax ) ) {
		tdir = d[ 1 ];
		d[ 1 ]= d[ 2 ];
		d[ 2 ]= tdir;
	}

	if( d[ 1 ]!=-1 && d[ 1 ]!= turnaround
		&& SV_StepDirection( actor, d[ 1 ], dist ) )
		return;

	if( d[ 2 ]!=-1 && d[ 2 ]!= turnaround
		&& SV_StepDirection( actor, d[ 2 ], dist ) )
		return;

	/* there is no direct path to the player, so pick another direction */

	if( olddir!=-1 && SV_StepDirection( actor, olddir, dist ) ) return;

	if( rand( )&1 ) /* randomly determine direction of search*/
	{
		for( tdir = 0; tdir<= 315; tdir += 45 )
			if( tdir!= turnaround && SV_StepDirection( actor, tdir, dist ) )
				return;
	}
	else
	{
		for( tdir = 315; tdir >= 0; tdir -= 45 )
			if( tdir!= turnaround && SV_StepDirection( actor, tdir, dist ) )
				return;
	}

	if( turnaround != -1 && SV_StepDirection( actor, turnaround, dist ) )
		return;

	actor->eal_yaw = olddir; // can't move

	// if a bridge was pulled out from underneath a monster, it may not have
	// a valid standing position at all

	if( !M_CheckBottom( actor ) ) SV_FixCheckBottom( actor );
}

/*

======================

SV_CloseEnough



======================

*/
bool Game::SV_CloseEnough( Entity * ent, Entity * goal, float dist ) {

	int i;

	for( i = 0; i<3; i++ )
	{
		if( goal->absmin[ i ] > ent->absmax[ i ] + dist )
			return false;
		if( goal->absmax[ i ] < ent->absmin[ i ] - dist )
			return false;
	}
	return true;
}


/*

======================

M_MoveToGoal

======================

*/
void Game::M_MoveToGoal( Entity * ent, float dist ) {

	Entity * goal;

	goal = ent->goalentity;

	if( !ent->groundentity && !( ent->flags &( FL_FLY|FL_SWIM ) ) )
		return;

	// if the next step hits the enemy, return immediately
	if( ent->enemy && SV_CloseEnough( ent, ent->enemy, dist ) )
		return;

	// bump around...
	if( ( rand( )&3 )== 1 || !SV_StepDirection( ent, ent->eal_yaw, dist ) ) {
		if( ent->inuse )
			SV_NewChaseDir( ent, goal, dist );
	}
}


/*

===============

M_walkmove

===============

*/
bool Game::M_walkmove( Entity * ent, float yaw, float dist ) {

	Vec3 move;

	if( !ent->groundentity && !( ent->flags &( FL_FLY|FL_SWIM ) ) )
		return false;

	yaw = yaw* M_PI* 2 / 360;

	move[ 0 ] = cos( yaw )* dist;
	move[ 1 ] = sin( yaw )* dist;
	move[ 2 ] = 0;

	return SV_movestep( ent, move, true );
}
