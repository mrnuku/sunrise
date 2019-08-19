#include "precompiled.h"
#pragma hdrstop

/*

=========================================================



PLATS



movement options:



linear

smooth start, hard stop

smooth start, smooth stop



start

end

acceleration

speed

deceleration

begin sound

end sound

target fired when reaching end

wait at end



object characteristics that use move segments

---------------------------------------------

movetype_push, or movetype_stop

action when touched

action when blocked

action when used

disabled?

auto trigger spawning





=========================================================

*/
//
// Support routines for movement( changes in origin using velocity )
//

void Game::Move_Done( Entity * ent ) {

	ent->velocity = vec3_origin;
	ent->moveinfo.endfunc( ent );
}

void Game::Move_Final( Entity * ent ) {

	if( ent->moveinfo.remaining_distance == 0 )
	{
		Move_Done( ent );
		return;
	}

	ent->velocity = ent->moveinfo.dir * ( ent->moveinfo.remaining_distance / FRAMETIME );

	ent->think = Move_Done;
	ent->nextthink = level.time + FRAMETIME;
}

void Game::Move_Begin( Entity * ent ) {

	timeType frames;

	if( ( ent->moveinfo.speed * FRAMETIME ) >= ent->moveinfo.remaining_distance )
	{
		Move_Final( ent );
		return;
	}
	ent->velocity = ent->moveinfo.dir * ent->moveinfo.speed;
	frames = ( timeType )floor( ( ent->moveinfo.remaining_distance / ent->moveinfo.speed ) / FRAMETIME );
	ent->moveinfo.remaining_distance -= frames * ent->moveinfo.speed * FRAMETIME;
	ent->nextthink = level.time + ( frames * FRAMETIME );
	ent->think = Move_Final;
}

void Game::Move_Calc( Entity * ent, Vec3 & dest, void( *func )( Entity * ) ) {

	ent->velocity = vec3_origin;
	ent->moveinfo.dir = dest - ent->s.origin;
	ent->moveinfo.remaining_distance = ent->moveinfo.dir.Normalize( );
	ent->moveinfo.endfunc = func;

	if( ent->moveinfo.speed == ent->moveinfo.accel && ent->moveinfo.speed == ent->moveinfo.decel )
	{
		if( level.current_entity ==( ( ent->flags & FL_TEAMSLAVE ) ? ent->teammaster : ent ) )
		{
			Move_Begin( ent );
		}
		else
		{
			ent->nextthink = level.time + FRAMETIME;
			ent->think = Move_Begin;
		}
	}
	else
	{
		// accelerative
		ent->moveinfo.current_speed = 0;
		ent->think = Think_AccelMove;
		ent->nextthink = level.time + FRAMETIME;
	}
}


//
// Support routines for angular movement( changes in angle using avelocity )
//

void Game::AngleMove_Done( Entity * ent ) {

	ent->avelocity = vec3_origin;
	ent->moveinfo.endfunc( ent );
}

void Game::AngleMove_Final( Entity * ent ) {

	Vec3 move;

	if( ent->moveinfo.state == 2 ) move = ent->moveinfo.end_angles - ent->s.angles;
	else move = ent->moveinfo.start_angles - ent->s.angles;

	if( move == vec3_origin ) {

		AngleMove_Done( ent );
		return;
	}

	ent->avelocity = move * ( 1.0f / FRAMETIME );

	ent->think = AngleMove_Done;
	ent->nextthink = level.time + FRAMETIME;
}

void Game::AngleMove_Begin( Entity * ent ) {

	Vec3 destdelta;
	float len;
	float traveltime;

	// set destdelta to the vector needed to move
	destdelta =( ent->moveinfo.state == 2 )?( ent->moveinfo.end_angles - ent->s.angles ):( ent->moveinfo.start_angles - ent->s.angles );

	// calculate length of vector
	len = destdelta.Length( );

	// divide by speed to get time to reach dest
	traveltime = len / ent->moveinfo.speed;

	if( traveltime < FRAMETIME )
	{
		AngleMove_Final( ent );
		return;
	}

	timeType frames = ( timeType )floor( traveltime / FRAMETIME );

	// scale the destdelta vector by the time spent traveling to get velocity
	ent->avelocity = destdelta * ( 1.0f / traveltime );

	// set nextthink to trigger a think when dest is reached
	ent->nextthink = level.time + frames * FRAMETIME;
	ent->think = AngleMove_Final;
}

void Game::AngleMove_Calc( Entity * ent, void( *func )( Entity * ) ) {

	ent->avelocity = vec3_origin;
	ent->moveinfo.endfunc = func;
	if( level.current_entity ==( ( ent->flags & FL_TEAMSLAVE ) ? ent->teammaster : ent ) ) {
		AngleMove_Begin( ent );
	}
	else
	{
		ent->nextthink = level.time + FRAMETIME;
		ent->think = AngleMove_Begin;
	}
}


/*

==============

Think_AccelMove



The team has completed a frame of movement, so

change the speed for the next frame

==============

*/
void Game::plat_CalcAcceleratedMove( moveinfo_t * moveinfo ) {

	float accel_dist;
	float decel_dist;

	moveinfo->move_speed = moveinfo->speed;

	if( moveinfo->remaining_distance < moveinfo->accel )
	{
		moveinfo->current_speed = moveinfo->remaining_distance;
		return;
	}

	accel_dist =( moveinfo->speed * ( ( moveinfo->speed / moveinfo->accel ) + 1 ) / 2 );
	decel_dist =( moveinfo->speed * ( ( moveinfo->speed / moveinfo->decel ) + 1 ) / 2 );

	if( ( moveinfo->remaining_distance - accel_dist - decel_dist ) < 0 )
	{
		float f;

		f =( moveinfo->accel + moveinfo->decel ) /( moveinfo->accel * moveinfo->decel );
		moveinfo->move_speed =( -2 + sqrt( 4 - 4 * f * ( -2 * moveinfo->remaining_distance ) ) ) /( 2 * f );
		decel_dist =( moveinfo->move_speed * ( ( moveinfo->move_speed / moveinfo->decel ) + 1 ) / 2 );
	}

	moveinfo->decel_distance = decel_dist;
};

void Game::plat_Accelerate( moveinfo_t * moveinfo ) {

	// are we decelerating?
	if( moveinfo->remaining_distance <= moveinfo->decel_distance )
	{
		if( moveinfo->remaining_distance < moveinfo->decel_distance )
		{
			if( moveinfo->next_speed )
			{
				moveinfo->current_speed = moveinfo->next_speed;
				moveinfo->next_speed = 0;
				return;
			}
			if( moveinfo->current_speed > moveinfo->decel )
				moveinfo->current_speed -= moveinfo->decel;
		}
		return;
	}

	// are we at full speed and need to start decelerating during this move?
	if( moveinfo->current_speed == moveinfo->move_speed )
		if( ( moveinfo->remaining_distance - moveinfo->current_speed ) < moveinfo->decel_distance )
		{
			float p1_distance;
			float p2_distance;
			float distance;

			p1_distance = moveinfo->remaining_distance - moveinfo->decel_distance;
			p2_distance = moveinfo->move_speed * ( 1.0f -( p1_distance / moveinfo->move_speed ) );
			distance = p1_distance + p2_distance;
			moveinfo->current_speed = moveinfo->move_speed;
			moveinfo->next_speed = moveinfo->move_speed - moveinfo->decel * ( p2_distance / distance );
			return;
		}

		// are we accelerating?
		if( moveinfo->current_speed < moveinfo->speed )
		{
			float old_speed;
			float p1_distance;
			float p1_speed;
			float p2_distance;
			float distance;

			old_speed = moveinfo->current_speed;

			// figure simple acceleration up to move_speed
			moveinfo->current_speed += moveinfo->accel;
			if( moveinfo->current_speed > moveinfo->speed )
				moveinfo->current_speed = moveinfo->speed;

			// are we accelerating throughout this entire move?
			if( ( moveinfo->remaining_distance - moveinfo->current_speed ) >= moveinfo->decel_distance )
				return;

			// during this move we will accelrate from current_speed to move_speed
			// and cross over the decel_distance; figure the average speed for the
			// entire move
			p1_distance = moveinfo->remaining_distance - moveinfo->decel_distance;
			p1_speed =( old_speed + moveinfo->move_speed ) / 2.0f;
			p2_distance = moveinfo->move_speed * ( 1.0f -( p1_distance / p1_speed ) );
			distance = p1_distance + p2_distance;
			moveinfo->current_speed =( p1_speed * ( p1_distance / distance ) ) +( moveinfo->move_speed * ( p2_distance / distance ) );
			moveinfo->next_speed = moveinfo->move_speed - moveinfo->decel * ( p2_distance / distance );
			return;
		}

		// we are at constant velocity( move_speed )
		return;
};

void Game::Think_AccelMove( Entity * ent ) {

	ent->moveinfo.remaining_distance -= ent->moveinfo.current_speed;

	if( ent->moveinfo.current_speed == 0 ) // starting or blocked
		plat_CalcAcceleratedMove( &ent->moveinfo );

	plat_Accelerate( &ent->moveinfo );

	// will the entire move complete on next frame?
	if( ent->moveinfo.remaining_distance <= ent->moveinfo.current_speed )
	{
		Move_Final( ent );
		return;
	}

	ent->velocity = ent->moveinfo.dir * ( ent->moveinfo.current_speed * 10.0f );
	ent->nextthink = level.time + FRAMETIME;
	ent->think = Think_AccelMove;
}

void Game::plat_hit_top( Entity * ent ) {

	if( !( ent->flags & FL_TEAMSLAVE ) ) {
		if( ent->moveinfo.sound_end )
			Server::PF_StartSound( ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_end, 1, ATTN_STATIC, 0 );
		ent->s.sound = 0;
	}
	ent->moveinfo.state = 0;

	ent->think = plat_go_down;
	ent->nextthink = level.time + 3;
}

void Game::plat_hit_bottom( Entity * ent ) {

	if( !( ent->flags & FL_TEAMSLAVE ) ) {
		if( ent->moveinfo.sound_end )
			Server::PF_StartSound( ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_end, 1, ATTN_STATIC, 0 );
		ent->s.sound = 0;
	}
	ent->moveinfo.state = 1;
}

void Game::plat_go_down( Entity * ent ) {

	if( !( ent->flags & FL_TEAMSLAVE ) ) {
		if( ent->moveinfo.sound_start )
			Server::PF_StartSound( ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_start, 1, ATTN_STATIC, 0 );
		ent->s.sound = ent->moveinfo.sound_middle;
	}
	ent->moveinfo.state = 3;
	Move_Calc( ent, ent->moveinfo.end_origin, plat_hit_bottom );
}

void Game::plat_go_up( Entity * ent ) {

	if( !( ent->flags & FL_TEAMSLAVE ) ) {
		if( ent->moveinfo.sound_start )
			Server::PF_StartSound( ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_start, 1, ATTN_STATIC, 0 );
		ent->s.sound = ent->moveinfo.sound_middle;
	}
	ent->moveinfo.state = 2;
	Move_Calc( ent, ent->moveinfo.start_origin, plat_hit_top );
}

void Game::plat_blocked( Entity * self, Entity * other ) {

	if( !( other->svflags & SVF_MONSTER ) &&( !other->client ) )
	{
		// give it a chance to go away on it's own terms( like gibs )
		T_Damage( other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH );
		// if it's still there, nuke it
		if( other )
			BecomeExplosion1( other );
		return;
	}

	T_Damage( other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH );

	if( self->moveinfo.state == 2 )
		plat_go_down( self );
	else if( self->moveinfo.state == 3 )
		plat_go_up( self );
}


void Game::Use_Plat( Entity * ent, Entity * other, Entity * activator ) {

	if( ent->think )
		return; // already down
	plat_go_down( ent );
}


void Game::Touch_Plat_Center( Entity * ent, Entity * other, Plane & plane, int surf ) {

	if( !other->client )
		return;

	if( other->health <= 0 )
		return;

	ent = ent->enemy; // now point at the plat, not the trigger
	if( ent->moveinfo.state == 1 )
		plat_go_up( ent );
	else if( ent->moveinfo.state == 0 )
		ent->nextthink = level.time + 1; // the player is still on the plat, so delay going down
}

void Game::plat_spawn_inside_trigger( Entity * ent ) {

	Entity * trigger;
	Vec3 tmin, tmax;

	//
	// middle trigger
	//	
	trigger = G_Spawn( );
	trigger->touch = Touch_Plat_Center;
	trigger->movetype = MOVETYPE_NONE;
	trigger->solid = SOLID_TRIGGER;
	trigger->enemy = ent;

	tmin[ 0 ] = ent->mins[ 0 ] + 25;
	tmin[ 1 ] = ent->mins[ 1 ] + 25;
	tmin[ 2 ] = ent->mins[ 2 ];

	tmax[ 0 ] = ent->maxs[ 0 ] - 25;
	tmax[ 1 ] = ent->maxs[ 1 ] - 25;
	tmax[ 2 ] = ent->maxs[ 2 ] + 8;

	tmin[ 2 ] = tmax[ 2 ] -( ent->pos1[ 2 ] - ent->pos2[ 2 ] + st.lip );

	if( ent->spawnflags & 1 )
		tmax[ 2 ] = tmin[ 2 ] + 8;

	if( tmax[ 0 ] - tmin[ 0 ] <= 0 )
	{
		tmin[ 0 ] =( ent->mins[ 0 ] + ent->maxs[ 0 ] ) * 0.5f;
		tmax[ 0 ] = tmin[ 0 ] + 1;
	}
	if( tmax[ 1 ] - tmin[ 1 ] <= 0 )
	{
		tmin[ 1 ] =( ent->mins[ 1 ] + ent->maxs[ 1 ] ) * 0.5f;
		tmax[ 1 ] = tmin[ 1 ] + 1;
	}

	trigger->mins = tmin;
	trigger->maxs = tmax;

	Server::SV_LinkEdict( trigger );
}


/* QUAKED func_plat( 0 .5 .8 ) ? PLAT_LOW_TRIGGER

speed	default 150



Plats are always drawn in the extended position, so they will light correctly.



If the plat is the target of another trigger or button, it will start out disabled in the extended position until it is trigger, when it will lower and become a normal plat.



"speed"	overrides default 200.

"accel" overrides default 500

"lip"	overrides default 8 pixel lip



If the "height" key is set, that will determine the amount the plat moves, instead of being implicitly determoveinfoned by the model's height.



Set "sounds" to one of the following:

1 ) base fast

2 ) chain slow

*/
void Game::SP_func_plat( Entity * ent ) {

	ent->s.angles = vec3_origin;
	ent->solid = SOLID_BSP;
	ent->movetype = MOVETYPE_PUSH;

	Server::PF_setmodel( ent, ent->model );

	ent->blocked = plat_blocked;

	if( !ent->speed )
		ent->speed = 20;
	else
		ent->speed *= 0.1f;

	if( !ent->accel )
		ent->accel = 5;
	else
		ent->accel *= 0.1f;

	if( !ent->decel )
		ent->decel = 5;
	else
		ent->decel *= 0.1f;

	if( !ent->dmg )
		ent->dmg = 2;

	if( !st.lip )
		st.lip = 8;

	// pos1 is the top position, pos2 is the bottom
	ent->pos1 = ent->s.origin;
	ent->pos2 = ent->s.origin;
	if( st.height )
		ent->pos2[ 2 ] -= st.height;
	else
		ent->pos2[ 2 ] -=( ent->maxs[ 2 ] - ent->mins[ 2 ] ) - st.lip;

	ent->use = Use_Plat;

	plat_spawn_inside_trigger( ent ); // the "start moving" trigger	

	if( ent->targetname )
	{
		ent->moveinfo.state = 2;
	}
	else
	{
		ent->s.origin = ent->pos2;
		Server::SV_LinkEdict( ent );
		ent->moveinfo.state = 1;
	}

	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;
	ent->moveinfo.start_origin = ent->pos1;
	ent->moveinfo.start_angles = ent->s.angles;
	ent->moveinfo.end_origin = ent->pos2;
	ent->moveinfo.end_angles = ent->s.angles;

	ent->moveinfo.sound_start = Server::SV_SoundIndex( "plats/pt1_strt.wav" );
	ent->moveinfo.sound_middle = Server::SV_SoundIndex( "plats/pt1_mid.wav" );
	ent->moveinfo.sound_end = Server::SV_SoundIndex( "plats/pt1_end.wav" );
}

//====================================================================

/* QUAKED func_rotating( 0 .5 .8 ) ? START_ON REVERSE X_AXIS Y_AXIS TOUCH_PAIN STOP ANIMATED ANIMATED_FAST

You need to have an origin brush as part of this entity.  The center of that brush will be

the point around which it is rotated. It will rotate around the Z axis by default.  You can

check either the X_AXIS or Y_AXIS box to change that.



"speed" determines how fast it moves; default value is 100.

"dmg"	damage to inflict when blocked( 2 default )



REVERSE will cause the it to rotate in the opposite direction.

STOP mean it will stop moving instead of pushing entities

*/
void Game::rotating_blocked( Entity * self, Entity * other ) {

	T_Damage( other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH );
}

void Game::rotating_touch( Entity * self, Entity * other, Plane & plane, int surf ) {

	if( self->avelocity[ 0 ] || self->avelocity[ 1 ] || self->avelocity[ 2 ] )
		T_Damage( other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH );
}

void Game::rotating_use( Entity * self, Entity * other, Entity * activator ) {

	if( self->avelocity != vec3_origin ) {

		self->s.sound = 0;
		self->avelocity = vec3_origin;
		self->touch = NULL;
	} else {

		self->s.sound = self->moveinfo.sound_middle;
		self->avelocity = self->movedir * self->speed;
		if( self->spawnflags & 16 ) self->touch = rotating_touch;
	}
}

void Game::SP_func_rotating( Entity * ent ) {

	ent->solid = SOLID_BSP;
	if( ent->spawnflags & 32 )
		ent->movetype = MOVETYPE_STOP;
	else
		ent->movetype = MOVETYPE_PUSH;

	// set the axis of rotation
	ent->movedir = vec3_origin;
	if( ent->spawnflags & 4 )
		ent->movedir[ 2 ] = 1.0f;
	else if( ent->spawnflags & 8 )
		ent->movedir[ 0 ] = 1.0f;
	else // Z_AXIS
		ent->movedir[ 1 ] = 1.0f;

	// check for reverse rotation
	if( ent->spawnflags & 2 ) ent->movedir *= -1.0f;

	if( !ent->speed )
		ent->speed = 100;
	if( !ent->dmg )
		ent->dmg = 2;

	//	ent->moveinfo.sound_middle = "doors/hydro1.wav";

	ent->use = rotating_use;
	if( ent->dmg )
		ent->blocked = rotating_blocked;

	if( ent->spawnflags & 1 )
		ent->use( ent, NULL, NULL );

	if( ent->spawnflags & 64 )
		ent->s.effects |= EF_ANIM_ALL;
	if( ent->spawnflags & 128 )
		ent->s.effects |= EF_ANIM_ALLFAST;

	Server::PF_setmodel( ent, ent->model );
	Server::SV_LinkEdict( ent );
}

/*

======================================================================



BUTTONS



======================================================================

*/
/* QUAKED func_button( 0 .5 .8 ) ?

When a button is touched, it moves some distance in the direction of it's angle, triggers all of it's targets, waits some time, then returns to it's original position where it can be triggered again.



"angle"		determines the opening direction

"target"	all entities with a matching targetname will be used

"speed"		override the default 40 speed

"wait"		override the default 1 second wait( -1 = never return )

"lip"		override the default 4 pixel lip remaining at end of move

"health"	if set, the button must be killed instead of touched

"sounds"

1 ) silent

2 ) steam metal

3 ) wooden clunk

4 ) metallic click

5 ) in-out

*/
void Game::button_done( Entity * self ) {

	self->moveinfo.state = 1;
	self->s.effects &= ~EF_ANIM23;
	self->s.effects |= EF_ANIM01;
}

void Game::button_return( Entity * self ) {

	self->moveinfo.state = 3;

	Move_Calc( self, self->moveinfo.start_origin, button_done );

	self->s.frame = 0;

	if( self->health )
		self->takedamage = DAMAGE_YES;
}

void Game::button_wait( Entity * self ) {

	self->moveinfo.state = 0;
	self->s.effects &= ~EF_ANIM01;
	self->s.effects |= EF_ANIM23;

	G_UseTargets( self, self->activator );
	self->s.frame = 1;
	if( self->moveinfo.wait >= 0 )
	{
		self->nextthink = level.time + self->moveinfo.wait;
		self->think = button_return;
	}
}

void Game::button_fire( Entity * self ) {

	if( self->moveinfo.state == 2 || self->moveinfo.state == 0 )
		return;

	self->moveinfo.state = 2;
	if( self->moveinfo.sound_start && !( self->flags & FL_TEAMSLAVE ) ) Server::PF_StartSound( self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, ATTN_STATIC, 0 );
	Move_Calc( self, self->moveinfo.end_origin, button_wait );
}

void Game::button_use( Entity * self, Entity * other, Entity * activator ) {

	self->activator = activator;
	button_fire( self );
}

void Game::button_touch( Entity * self, Entity * other, Plane & plane, int surf ) {

	if( !other->client )
		return;

	if( other->health <= 0 )
		return;

	self->activator = other;
	button_fire( self );
}

void Game::button_killed( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point ) {

	self->activator = attacker;
	self->health = self->max_health;
	self->takedamage = DAMAGE_NO;
	button_fire( self );
}

void Game::SP_func_button( Entity * ent ) {

	Vec3 abs_movedir;
	float dist;

	G_SetMovedir( ent->s.angles, &ent->movedir );
	ent->movetype = MOVETYPE_STOP;
	ent->solid = SOLID_BSP;
	Server::PF_setmodel( ent, ent->model );

	if( ent->sounds != 1 )
		ent->moveinfo.sound_start = Server::SV_SoundIndex( "switches/butn2.wav" );

	if( !ent->speed )
		ent->speed = 40;
	if( !ent->accel )
		ent->accel = ent->speed;
	if( !ent->decel )
		ent->decel = ent->speed;

	if( !ent->wait )
		ent->wait = 3;
	if( !st.lip )
		st.lip = 4;

	ent->pos1 = ent->s.origin;
	abs_movedir[ 0 ] = fabs( ent->movedir[ 0 ] );
	abs_movedir[ 1 ] = fabs( ent->movedir[ 1 ] );
	abs_movedir[ 2 ] = fabs( ent->movedir[ 2 ] );
	dist = abs_movedir[ 0 ] * ent->size[ 0 ] + abs_movedir[ 1 ] * ent->size[ 1 ] + abs_movedir[ 2 ] * ent->size[ 2 ] - st.lip;
	ent->pos2 = ent->pos1.Magnitude( dist, ent->movedir );

	ent->use = button_use;
	ent->s.effects |= EF_ANIM01;

	if( ent->health )
	{
		ent->max_health = ent->health;
		ent->die = button_killed;
		ent->takedamage = DAMAGE_YES;
	}
	else if( ! ent->targetname )
		ent->touch = button_touch;

	ent->moveinfo.state = 1;

	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;
	ent->moveinfo.start_origin = ent->pos1;
	ent->moveinfo.start_angles = ent->s.angles;
	ent->moveinfo.end_origin = ent->pos2;
	ent->moveinfo.end_angles = ent->s.angles;

	Server::SV_LinkEdict( ent );
}

/*

======================================================================



DOORS



spawn a trigger surrounding the entire team unless it is

already targeted by another



======================================================================

*/
/* QUAKED func_door( 0 .5 .8 ) ? START_OPEN x CRUSHER NOMONSTER ANIMATED TOGGLE ANIMATED_FAST

TOGGLE		wait in both the start and end states for a trigger event.

START_OPEN	the door to moves to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered( not useful for touch or takedamage doors ).

NOMONSTER	monsters will not trigger this door



"message"	is printed when the door is touched if it is a trigger door and it hasn't been fired yet

"angle"		determines the opening direction

"targetname" if set, no touch field will be spawned and a remote button or trigger field activates the door.

"health"	if set, door must be shot open

"speed"		movement speed( 100 default )

"wait"		wait before returning( 3 default, -1 = never return )

"lip"		lip remaining at end of move( 8 default )

"dmg"		damage to inflict when blocked( 2 default )

"sounds"

1 )	silent

2 )	light

3 )	medium

4 )	heavy

*/
void Game::door_use_areaportals( Entity * self, bool open ) {

	Entity * t = NULL;

	if( !self->target )
		return;

	while( ( t = G_Find( t, FOFS( targetname ), self->target ) ) )
	{
		if( t->classname.IcmpFast( "func_areaportal" ) )
		{
			CollisionModel::CM_SetAreaPortalState( t->style, open );
		}
	}
}

void Game::door_hit_top( Entity * self ) {

	if( !( self->flags & FL_TEAMSLAVE ) ) {
		if( self->moveinfo.sound_end )
			Server::PF_StartSound( self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, ATTN_STATIC, 0 );
		self->s.sound = 0;
	}
	self->moveinfo.state = 0;
	if( self->spawnflags & 32 )
		return;
	if( self->moveinfo.wait >= 0 )
	{
		self->think = door_go_down;
		self->nextthink = level.time + self->moveinfo.wait;
	}
}

void Game::door_hit_bottom( Entity * self ) {

	if( !( self->flags & FL_TEAMSLAVE ) ) {
		if( self->moveinfo.sound_end )
			Server::PF_StartSound( self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, ATTN_STATIC, 0 );
		self->s.sound = 0;
	}
	self->moveinfo.state = 1;
	door_use_areaportals( self, false );
}

void Game::door_go_down( Entity * self ) {

	if( !( self->flags & FL_TEAMSLAVE ) ) {
		if( self->moveinfo.sound_start )
			Server::PF_StartSound( self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, ATTN_STATIC, 0 );
		self->s.sound = self->moveinfo.sound_middle;
	}
	if( self->max_health )
	{
		self->takedamage = DAMAGE_YES;
		self->health = self->max_health;
	}

	self->moveinfo.state = 3;
	if( strcmp( self->classname, "func_door" ) == 0 )
		Move_Calc( self, self->moveinfo.start_origin, door_hit_bottom );
	else if( strcmp( self->classname, "func_door_rotating" ) == 0 )
		AngleMove_Calc( self, door_hit_bottom );
}

void Game::door_go_up( Entity * self, Entity * activator ) {

	if( self->moveinfo.state == 2 )
		return; // already going up

	if( self->moveinfo.state == 0 )
	{ // reset top wait time
		if( self->moveinfo.wait >= 0 )
			self->nextthink = level.time + self->moveinfo.wait;
		return;
	}

	if( !( self->flags & FL_TEAMSLAVE ) ) {
		if( self->moveinfo.sound_start )
			Server::PF_StartSound( self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, ATTN_STATIC, 0 );
		self->s.sound = self->moveinfo.sound_middle;
	}
	self->moveinfo.state = 2;
	if( strcmp( self->classname, "func_door" ) == 0 )
		Move_Calc( self, self->moveinfo.end_origin, door_hit_top );
	else if( strcmp( self->classname, "func_door_rotating" ) == 0 )
		AngleMove_Calc( self, door_hit_top );

	G_UseTargets( self, activator );
	door_use_areaportals( self, true );
}

void Game::door_use( Entity * self, Entity * other, Entity * activator ) {

	Entity * ent;

	if( self->flags & FL_TEAMSLAVE )
		return;

	if( self->spawnflags & 32 )
	{
		if( self->moveinfo.state == 2 || self->moveinfo.state == 0 )
		{
			// trigger all paired doors
			for( ent = self; ent; ent = ent->teamchain )
			{
				ent->message = NULL;
				ent->touch = NULL;
				door_go_down( ent );
			}
			return;
		}
	}

	// trigger all paired doors
	for( ent = self; ent; ent = ent->teamchain )
	{
		ent->message = NULL;
		ent->touch = NULL;
		door_go_up( ent, activator );
	}
};

void Game::Touch_DoorTrigger( Entity * self, Entity * other, Plane & plane, int surf ) {

	if( other->health <= 0 )
		return;

	if( !( other->svflags & SVF_MONSTER ) &&( !other->client ) ) return;

	if( ( self->owner->spawnflags & 8 ) &&( other->svflags & SVF_MONSTER ) ) return;

	if( level.time < self->touch_debounce_time )
		return;
	self->touch_debounce_time = level.time + 1.0f;

	door_use( self->owner, other, other );
}

void Game::Think_CalcMoveSpeed( Entity * self ) {

	Entity * ent;
	float min;
	float time;
	float newspeed;
	float ratio;
	float dist;

	if( self->flags & FL_TEAMSLAVE )
		return; // only the team master does this

	// find the smallest distance any member of the team will be moving
	min = fabs( self->moveinfo.distance );
	for( ent = self->teamchain; ent; ent = ent->teamchain )
	{
		dist = fabs( ent->moveinfo.distance );
		if( dist < min )
			min = dist;
	}

	time = min / self->moveinfo.speed;

	// adjust speeds so they will all complete at the same time
	for( ent = self; ent; ent = ent->teamchain )
	{
		newspeed = fabs( ent->moveinfo.distance ) / time;
		ratio = newspeed / ent->moveinfo.speed;
		if( ent->moveinfo.accel == ent->moveinfo.speed )
			ent->moveinfo.accel = newspeed;
		else
			ent->moveinfo.accel *= ratio;
		if( ent->moveinfo.decel == ent->moveinfo.speed )
			ent->moveinfo.decel = newspeed;
		else
			ent->moveinfo.decel *= ratio;
		ent->moveinfo.speed = newspeed;
	}
}

void Game::Think_SpawnDoorTrigger( Entity * ent ) {

	Entity * other;
	Vec3 mins, maxs;

	if( ent->flags & FL_TEAMSLAVE )
		return; // only the team leader spawns a trigger

	mins = ent->absmin;
	maxs = ent->absmax;

	for( other = ent->teamchain; other; other = other->teamchain )
	{
		AddPointToBounds( other->absmin, &mins, &maxs );
		AddPointToBounds( other->absmax, &mins, &maxs );
	}

	// expand 
	mins[ 0 ] -= 60;
	mins[ 1 ] -= 60;
	maxs[ 0 ] += 60;
	maxs[ 1 ] += 60;

	other = G_Spawn( );
	other->mins = mins;
	other->maxs = maxs;
	other->owner = ent;
	other->solid = SOLID_TRIGGER;
	other->movetype = MOVETYPE_NONE;
	other->touch = Touch_DoorTrigger;
	Server::SV_LinkEdict( other );

	if( ent->spawnflags & 1 )
		door_use_areaportals( ent, true );

	Think_CalcMoveSpeed( ent );
}

void Game::door_blocked( Entity * self, Entity * other ) {

	Entity * ent;

	if( !( other->svflags & SVF_MONSTER ) &&( !other->client ) )
	{
		// give it a chance to go away on it's own terms( like gibs )
		T_Damage( other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH );
		// if it's still there, nuke it
		if( other )
			BecomeExplosion1( other );
		return;
	}

	T_Damage( other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH );

	if( self->spawnflags & 4 )
		return;


	// if a door has a negative wait, it would never come back if blocked, // so let it just squash the object to death real fast
	if( self->moveinfo.wait >= 0 )
	{
		if( self->moveinfo.state == 3 )
		{
			for( ent = self->teammaster; ent; ent = ent->teamchain )
				door_go_up( ent, ent->activator );
		}
		else
		{
			for( ent = self->teammaster; ent; ent = ent->teamchain )
				door_go_down( ent );
		}
	}
}

void Game::door_killed( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point ) {

	Entity * ent;

	for( ent = self->teammaster; ent; ent = ent->teamchain )
	{
		ent->health = ent->max_health;
		ent->takedamage = DAMAGE_NO;
	}
	door_use( self->teammaster, attacker, attacker );
}

void Game::door_touch( Entity * self, Entity * other, Plane & plane, int surf ) {

	if( !other->client )
		return;

	if( level.time < self->touch_debounce_time )
		return;
	self->touch_debounce_time = level.time + 5.0f;

	Server::PF_centerprintf( other, "%s", self->message.c_str( ) );
	Server::PF_StartSound( other, CHAN_AUTO, Server::SV_SoundIndex( "misc/talk1.wav" ), 1, ATTN_NORM, 0 );
}

void Game::SP_func_door( Entity * ent ) {

	Vec3 abs_movedir;

	if( ent->sounds != 1 )
	{
		ent->moveinfo.sound_start = Server::SV_SoundIndex( "doors/dr1_strt.wav" );
		ent->moveinfo.sound_middle = Server::SV_SoundIndex( "doors/dr1_mid.wav" );
		ent->moveinfo.sound_end = Server::SV_SoundIndex( "doors/dr1_end.wav" );
	}

	G_SetMovedir( ent->s.angles, &ent->movedir );
	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	Server::PF_setmodel( ent, ent->model );

	ent->blocked = door_blocked;
	ent->use = door_use;

	if( !ent->speed )
		ent->speed = 100;
	if( deathmatch.GetBool( ) ) ent->speed *= 2;

	if( !ent->accel )
		ent->accel = ent->speed;
	if( !ent->decel )
		ent->decel = ent->speed;

	if( !ent->wait )
		ent->wait = 3;
	if( !st.lip )
		st.lip = 8;
	if( !ent->dmg )
		ent->dmg = 2;

	// calculate second position
	ent->pos1 = ent->s.origin;
	abs_movedir[ 0 ] = fabs( ent->movedir[ 0 ] );
	abs_movedir[ 1 ] = fabs( ent->movedir[ 1 ] );
	abs_movedir[ 2 ] = fabs( ent->movedir[ 2 ] );
	ent->moveinfo.distance = abs_movedir[ 0 ] * ent->size[ 0 ] + abs_movedir[ 1 ] * ent->size[ 1 ] + abs_movedir[ 2 ] * ent->size[ 2 ] - st.lip;
	ent->pos2 = ent->pos1.Magnitude( ent->moveinfo.distance, ent->movedir );

	// if it starts open, switch the positions
	if( ent->spawnflags & 1 )
	{
		ent->s.origin = ent->pos2;
		ent->pos2 = ent->pos1;
		ent->pos1 = ent->s.origin;
	}

	ent->moveinfo.state = 1;

	if( ent->health )
	{
		ent->takedamage = DAMAGE_YES;
		ent->die = door_killed;
		ent->max_health = ent->health;
	}
	else if( ent->targetname && ent->message )
	{
		Server::SV_SoundIndex( "misc/talk.wav" );
		ent->touch = door_touch;
	}

	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;
	ent->moveinfo.start_origin = ent->pos1;
	ent->moveinfo.start_angles = ent->s.angles;
	ent->moveinfo.end_origin = ent->pos2;
	ent->moveinfo.end_angles = ent->s.angles;

	if( ent->spawnflags & 16 )
		ent->s.effects |= EF_ANIM_ALL;
	if( ent->spawnflags & 64 )
		ent->s.effects |= EF_ANIM_ALLFAST;

	// to simplify logic elsewhere, make non-teamed doors into a team of one
	if( !ent->team )
		ent->teammaster = ent;

	Server::SV_LinkEdict( ent );

	ent->nextthink = level.time + FRAMETIME;
	if( ent->health || ent->targetname )
		ent->think = Think_CalcMoveSpeed;
	else
		ent->think = Think_SpawnDoorTrigger;
}


/* QUAKED func_door_rotating( 0 .5 .8 ) ? START_OPEN REVERSE CRUSHER NOMONSTER ANIMATED TOGGLE X_AXIS Y_AXIS

TOGGLE causes the door to wait in both the start and end states for a trigger event.



START_OPEN	the door to moves to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered( not useful for touch or takedamage doors ).

NOMONSTER	monsters will not trigger this door



You need to have an origin brush as part of this entity.  The center of that brush will be

the point around which it is rotated. It will rotate around the Z axis by default.  You can

check either the X_AXIS or Y_AXIS box to change that.



"distance" is how many degrees the door will be rotated.

"speed" determines how fast the door moves; default value is 100.



REVERSE will cause the door to rotate in the opposite direction.



"message"	is printed when the door is touched if it is a trigger door and it hasn't been fired yet

"angle"		determines the opening direction

"targetname" if set, no touch field will be spawned and a remote button or trigger field activates the door.

"health"	if set, door must be shot open

"speed"		movement speed( 100 default )

"wait"		wait before returning( 3 default, -1 = never return )

"dmg"		damage to inflict when blocked( 2 default )

"sounds"

1 )	silent

2 )	light

3 )	medium

4 )	heavy

*/
void Game::SP_func_door_rotating( Entity * ent ) {

	ent->s.angles = vec3_origin;

	// set the axis of rotation
	ent->movedir = vec3_origin;
	if( ent->spawnflags & 64 )
		ent->movedir[ 2 ] = 1.0f;
	else if( ent->spawnflags & 128 )
		ent->movedir[ 0 ] = 1.0f;
	else // Z_AXIS
		ent->movedir[ 1 ] = 1.0f;

	// check for reverse rotation
	if( ent->spawnflags & 2 ) ent->movedir *= -1.0f;

	if( !st.distance )
	{
		Common::Com_DPrintf( "%s at %s with no distance set\n", ent->classname.c_str( ), vtos( ent->s.origin ).c_str( ) );
		st.distance = 90;
	}

	ent->pos1 = ent->s.angles;
	ent->pos2 = ent->s.angles.Magnitude( ( float )st.distance, ent->movedir );
	ent->moveinfo.distance = ( float )st.distance;

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	Server::PF_setmodel( ent, ent->model );

	ent->blocked = door_blocked;
	ent->use = door_use;

	if( !ent->speed )
		ent->speed = 100;
	if( !ent->accel )
		ent->accel = ent->speed;
	if( !ent->decel )
		ent->decel = ent->speed;

	if( !ent->wait )
		ent->wait = 3;
	if( !ent->dmg )
		ent->dmg = 2;

	if( ent->sounds != 1 )
	{
		ent->moveinfo.sound_start = Server::SV_SoundIndex( "doors/dr1_strt.wav" );
		ent->moveinfo.sound_middle = Server::SV_SoundIndex( "doors/dr1_mid.wav" );
		ent->moveinfo.sound_end = Server::SV_SoundIndex( "doors/dr1_end.wav" );
	}

	// if it starts open, switch the positions
	if( ent->spawnflags & 1 )
	{
		ent->s.angles = ent->pos2;
		ent->pos2 = ent->pos1;
		ent->pos1 = ent->s.angles;
		ent->movedir *= -1.0f;
	}

	if( ent->health )
	{
		ent->takedamage = DAMAGE_YES;
		ent->die = door_killed;
		ent->max_health = ent->health;
	}

	if( ent->targetname && ent->message )
	{
		Server::SV_SoundIndex( "misc/talk.wav" );
		ent->touch = door_touch;
	}

	ent->moveinfo.state = 1;
	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;
	ent->moveinfo.start_origin = ent->s.origin;
	ent->moveinfo.start_angles = ent->pos1;
	ent->moveinfo.end_origin = ent->s.origin;
	ent->moveinfo.end_angles = ent->pos2;

	if( ent->spawnflags & 16 )
		ent->s.effects |= EF_ANIM_ALL;

	// to simplify logic elsewhere, make non-teamed doors into a team of one
	if( !ent->team )
		ent->teammaster = ent;

	Server::SV_LinkEdict( ent );

	ent->nextthink = level.time + FRAMETIME;
	if( ent->health || ent->targetname )
		ent->think = Think_CalcMoveSpeed;
	else
		ent->think = Think_SpawnDoorTrigger;
}


/* QUAKED func_water( 0 .5 .8 ) ? START_OPEN

func_water is a moveable water brush.  It must be targeted to operate.  Use a non-water texture at your own risk.



START_OPEN causes the water to move to its destination when spawned and operate in reverse.



"angle"		determines the opening direction( up or down only )

"speed"		movement speed( 25 default )

"wait"		wait before returning( -1 default, -1 = TOGGLE )

"lip"		lip remaining at end of move( 0 default )

"sounds"	( yes, these need to be changed )

0 )	no sound

1 )	water

2 )	lava

*/
void Game::SP_func_water( Entity * self ) {

	Vec3 abs_movedir;

	G_SetMovedir( self->s.angles, &self->movedir );
	self->movetype = MOVETYPE_PUSH;
	self->solid = SOLID_BSP;
	Server::PF_setmodel( self, self->model );

	switch( self->sounds )
	{
	default:
		break;

	case 1: // water
		self->moveinfo.sound_start = Server::SV_SoundIndex( "world/mov_watr.wav" );
		self->moveinfo.sound_end = Server::SV_SoundIndex( "world/stp_watr.wav" );
		break;

	case 2: // lava
		self->moveinfo.sound_start = Server::SV_SoundIndex( "world/mov_watr.wav" );
		self->moveinfo.sound_end = Server::SV_SoundIndex( "world/stp_watr.wav" );
		break;
	}

	// calculate second position
	self->pos1 = self->s.origin;
	abs_movedir[ 0 ] = fabs( self->movedir[ 0 ] );
	abs_movedir[ 1 ] = fabs( self->movedir[ 1 ] );
	abs_movedir[ 2 ] = fabs( self->movedir[ 2 ] );
	self->moveinfo.distance = abs_movedir[ 0 ] * self->size[ 0 ] + abs_movedir[ 1 ] * self->size[ 1 ] + abs_movedir[ 2 ] * self->size[ 2 ] - st.lip;
	self->pos2 = self->pos1.Magnitude( self->moveinfo.distance, self->movedir );

	// if it starts open, switch the positions
	if( self->spawnflags & 1 )
	{
		self->s.origin = self->pos2;
		self->pos2 = self->pos1;
		self->pos1 = self->s.origin;
	}

	self->moveinfo.start_origin = self->pos1;
	self->moveinfo.start_angles = self->s.angles;
	self->moveinfo.end_origin = self->pos2;
	self->moveinfo.end_angles = self->s.angles;

	self->moveinfo.state = 1;

	if( !self->speed )
		self->speed = 25;
	self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed = self->speed;

	if( !self->wait )
		self->wait = -1;
	self->moveinfo.wait = self->wait;

	self->use = door_use;

	if( self->wait == -1 )
		self->spawnflags |= 32;

	self->classname = "func_door";

	Server::SV_LinkEdict( self );
}






/* QUAKED func_train( 0 .5 .8 ) ? START_ON TOGGLE BLOCK_STOPS

Trains are moving platforms that players can ride.

The targets origin specifies the min point of the train at each corner.

The train spawns at the first target it is pointing at.

If the train is the target of a button or trigger, it will not begin moving until activated.

speed	default 100

dmg		default	2

noise	looping sound to play when the train is in motion



*/
void Game::train_blocked( Entity * self, Entity * other ) {

	if( !( other->svflags & SVF_MONSTER ) &&( !other->client ) )
	{
		// give it a chance to go away on it's own terms( like gibs )
		T_Damage( other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH );
		// if it's still there, nuke it
		if( other )
			BecomeExplosion1( other );
		return;
	}

	if( level.time < self->touch_debounce_time )
		return;

	if( !self->dmg )
		return;
	self->touch_debounce_time = level.time + 0.5f;
	T_Damage( other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH );
}

void Game::train_wait( Entity * self ) {

	if( self->target_ent->pathtarget ) {

		Str savetarget;
		Entity * ent;

		ent = self->target_ent;
		savetarget = ent->target;
		ent->target = ent->pathtarget;
		G_UseTargets( ent, self->activator );
		ent->target = savetarget;

		// make sure we didn't get killed by a killtarget
		if( !self->inuse ) return;
	}

	if( self->moveinfo.wait )
	{
		if( self->moveinfo.wait > 0 )
		{
			self->nextthink = level.time + self->moveinfo.wait;
			self->think = train_next;
		}
		else if( self->spawnflags & 2 ) // && wait < 0
		{
			train_next( self );
			self->spawnflags &= ~1;
			self->velocity = vec3_origin;
			self->nextthink = 0;
		}

		if( !( self->flags & FL_TEAMSLAVE ) )
		{
			if( self->moveinfo.sound_end )
				Server::PF_StartSound( self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, ATTN_STATIC, 0 );
			self->s.sound = 0;
		}
	}
	else
	{
		train_next( self );
	}

}

void Game::train_next( Entity * self ) {

	Entity * ent;
	Vec3 dest;
	bool first;

	first = true;
again:
	if( !self->target )
	{
		//		Common::Com_DPrintf( "train_next: no next target\n" );
		return;
	}

	ent = G_PickTarget( self->target );
	if( !ent )
	{
		Common::Com_DPrintf( "train_next: bad target %s\n", self->target.c_str( ) );
		return;
	}

	self->target = ent->target;

	// check for a teleport path_corner
	if( ent->spawnflags & 1 )
	{
		if( !first )
		{
			Common::Com_DPrintf( "connected teleport path_corners, see %s at %s\n", ent->classname.c_str( ), vtos( ent->s.origin ).c_str( ) );
			return;
		}
		first = false;
		ent->s.origin -= self->mins;
		self->s.old_origin = self->s.origin;
		self->s.eventNum = EV_OTHER_TELEPORT;
		Server::SV_LinkEdict( self );
		goto again;
	}

	self->moveinfo.wait = ent->wait;
	self->target_ent = ent;

	if( !( self->flags & FL_TEAMSLAVE ) ) {
		if( self->moveinfo.sound_start )
			Server::PF_StartSound( self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, ATTN_STATIC, 0 );
		self->s.sound = self->moveinfo.sound_middle;
	}

	dest = ent->s.origin - self->mins;
	self->moveinfo.state = 0;
	self->moveinfo.start_origin = self->s.origin;
	self->moveinfo.end_origin = dest;
	Move_Calc( self, dest, train_wait );
	self->spawnflags |= 1;
}

void Game::train_resume( Entity * self ) {

	Entity * ent;
	Vec3 dest;

	ent = self->target_ent;

	dest = ent->s.origin - self->mins;
	self->moveinfo.state = 0;
	self->moveinfo.start_origin = self->s.origin;
	self->moveinfo.end_origin = dest;
	Move_Calc( self, dest, train_wait );
	self->spawnflags |= 1;
}

void Game::func_train_find( Entity * self ) {

	Entity * ent;

	if( !self->target )
	{
		Common::Com_DPrintf( "train_find: no target\n" );
		return;
	}
	ent = G_PickTarget( self->target );
	if( !ent )
	{
		Common::Com_DPrintf( "train_find: target %s not found\n", self->target.c_str( ) );
		return;
	}
	self->target = ent->target;

	self->s.origin = ent->s.origin - self->mins;
	Server::SV_LinkEdict( self );

	// if not triggered, start immediately
	if( !self->targetname )
		self->spawnflags |= 1;

	if( self->spawnflags & 1 )
	{
		self->nextthink = level.time + FRAMETIME;
		self->think = train_next;
		self->activator = self;
	}
}

void Game::train_use( Entity * self, Entity * other, Entity * activator ) {

	self->activator = activator;

	if( self->spawnflags & 1 )
	{
		if( !( self->spawnflags & 2 ) ) 	return;
		self->spawnflags &= ~1;
		self->velocity = vec3_origin;
		self->nextthink = 0;
	}
	else
	{
		if( self->target_ent )
			train_resume( self );
		else
			train_next( self );
	}
}

void Game::SP_func_train( Entity * self ) {

	self->movetype = MOVETYPE_PUSH;

	self->s.angles = vec3_origin;
	self->blocked = train_blocked;
	if( self->spawnflags & 4 )
		self->dmg = 0;
	else
	{
		if( !self->dmg )
			self->dmg = 100;
	}
	self->solid = SOLID_BSP;
	Server::PF_setmodel( self, self->model );

	if( st.noise )
		self->moveinfo.sound_middle = Server::SV_SoundIndex( st.noise );

	if( !self->speed )
		self->speed = 100;

	self->moveinfo.speed = self->speed;
	self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed;

	self->use = train_use;

	Server::SV_LinkEdict( self );

	if( self->target )
	{
		// start trains on the second frame, to make sure their targets have had
		// a chance to spawn
		self->nextthink = level.time + FRAMETIME;
		self->think = func_train_find;
	}
	else
	{
		Common::Com_DPrintf( "func_train without a target at %s\n", vtos( self->absmin ).c_str( ) );
	}
}


/* QUAKED trigger_elevator( 0.3f 0.1f 0.6f )( -8 -8 -8 )( 8 8 8 )

*/
void Game::trigger_elevator_use( Entity * self, Entity * other, Entity * activator ) {

	Entity * target;

	if( self->movetarget->nextthink )
	{
		//		Common::Com_DPrintf( "elevator busy\n" );
		return;
	}

	if( !other->pathtarget )
	{
		Common::Com_DPrintf( "elevator used with no pathtarget\n" );
		return;
	}

	target = G_PickTarget( other->pathtarget );
	if( !target )
	{
		Common::Com_DPrintf( "elevator used with bad pathtarget: %s\n", other->pathtarget.c_str( ) );
		return;
	}

	self->movetarget->target_ent = target;
	train_resume( self->movetarget );
}

void Game::trigger_elevator_init( Entity * self ) {

	if( !self->target )
	{
		Common::Com_DPrintf( "trigger_elevator has no target\n" );
		return;
	}
	self->movetarget = G_PickTarget( self->target );
	if( !self->movetarget )
	{
		Common::Com_DPrintf( "trigger_elevator unable to find target %s\n", self->target.c_str( ) );
		return;
	}
	if( strcmp( self->movetarget->classname, "func_train" ) != 0 )
	{
		Common::Com_DPrintf( "trigger_elevator target %s is not a train\n", self->target.c_str( ) );
		return;
	}

	self->use = trigger_elevator_use;
	self->svflags = SVF_NOCLIENT;

}

void Game::SP_trigger_elevator( Entity * self ) {

	self->think = trigger_elevator_init;
	self->nextthink = level.time + FRAMETIME;
}


/* QUAKED func_timer( 0.3f 0.1f 0.6f )( -8 -8 -8 )( 8 8 8 ) START_ON

"wait"			base time between triggering all targets, default is 1

"random"		wait variance, default is 0



so, the basic time between firing is a random time between

( wait - random ) and( wait + random )



"delay"			delay before first firing when turned on, default is 0



"pausetime"		additional delay used only the very first time

and only if spawned with START_ON



These can used but not touched.

*/
void Game::func_timer_think( Entity * self ) {

	G_UseTargets( self, self->activator );
	self->nextthink = level.time + self->wait + crandom( ) * self->random;
}

void Game::func_timer_use( Entity * self , Entity * other , Entity * activator ) {

	self->activator = activator;

	// if on, turn it off
	if( self->nextthink )
	{
		self->nextthink = 0;
		return;
	}

	// turn it on
	if( self->delay )
		self->nextthink = level.time + self->delay;
	else
		func_timer_think( self );
}

void Game::SP_func_timer( Entity * self ) {

	if( !self->wait )
		self->wait = 1.0f;

	self->use = func_timer_use;
	self->think = func_timer_think;

	if( self->random >= self->wait )
	{
		self->random = self->wait - FRAMETIME;
		Common::Com_DPrintf( "func_timer at %s has random >= wait\n", vtos( self->s.origin ).c_str( ) );
	}

	if( self->spawnflags & 1 )
	{
		self->nextthink = level.time + 1.0f + st.pausetime + self->delay + self->wait + crandom( ) * self->random;
		self->activator = self;
	}

	self->svflags = SVF_NOCLIENT;
}


/* QUAKED func_conveyor( 0 .5 .8 ) ? START_ON TOGGLE

Conveyors are stationary brushes that move what's on them.

The brush should be have a surface with at least one current content enabled.

speed	default 100

*/
void Game::func_conveyor_use( Entity * self, Entity * other, Entity * activator ) {

	if( self->spawnflags & 1 )
	{
		self->speed = 0;
		self->spawnflags &= ~1;
	}
	else
	{
		self->speed = ( float )self->count;
		self->spawnflags |= 1;
	}

	if( !( self->spawnflags & 2 ) ) self->count = 0;
}

void Game::SP_func_conveyor( Entity * self ) {

	if( !self->speed )
		self->speed = 100;

	if( !( self->spawnflags & 1 ) ) {
		self->count = ( int )self->speed;
		self->speed = 0;
	}

	self->use = func_conveyor_use;

	Server::PF_setmodel( self, self->model );
	self->solid = SOLID_BSP;
	Server::SV_LinkEdict( self );
}


/* QUAKED func_door_secret( 0 .5 .8 ) ? always_shoot 1st_left 1st_down

A secret door.  Slide back and then to the side.



open_once		doors never closes

1st_left		1st move is left of arrow

1st_down		1st move is down from arrow

always_shoot	door is shootebale even if targeted



"angle"		determines the direction

"dmg"		damage to inflic when blocked( default 2 )

"wait"		how long to hold in the open position( default 5, -1 means hold )

*/
void Game::door_secret_use( Entity * self, Entity * other, Entity * activator ) {

	// make sure we're not already moving
	if( self->s.origin != vec3_origin ) return;

	Move_Calc( self, self->pos1, door_secret_move1 );
	door_use_areaportals( self, true );
}

void Game::door_secret_move1( Entity * self ) {

	self->nextthink = level.time + 1.0f;
	self->think = door_secret_move2;
}

void Game::door_secret_move2( Entity * self ) {

	Move_Calc( self, self->pos2, door_secret_move3 );
}

void Game::door_secret_move3( Entity * self ) {

	if( self->wait == -1 )
		return;
	self->nextthink = level.time + self->wait;
	self->think = door_secret_move4;
}

void Game::door_secret_move4( Entity * self ) {

	Move_Calc( self, self->pos1, door_secret_move5 );
}

void Game::door_secret_move5( Entity * self ) {

	self->nextthink = level.time + 1.0f;
	self->think = door_secret_move6;
}

void Game::door_secret_move6( Entity * self ) {

	Move_Calc( self, vec3_origin, door_secret_done );
}

void Game::door_secret_done( Entity * self ) {

	if( !( self->targetname ) ||( self->spawnflags & 1 ) ) {
		self->health = 0;
		self->takedamage = DAMAGE_YES;
	}
	door_use_areaportals( self, false );
}

void Game::door_secret_blocked( Entity * self, Entity * other ) {

	if( !( other->svflags & SVF_MONSTER ) &&( !other->client ) )
	{
		// give it a chance to go away on it's own terms( like gibs )
		T_Damage( other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH );
		// if it's still there, nuke it
		if( other )
			BecomeExplosion1( other );
		return;
	}

	if( level.time < self->touch_debounce_time )
		return;
	self->touch_debounce_time = level.time + 0.5f;

	T_Damage( other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH );
}

void Game::door_secret_die( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point ) {

	self->takedamage = DAMAGE_NO;
	door_secret_use( self, attacker, attacker );
}

void Game::SP_func_door_secret( Entity * ent ) {

	Vec3 forward, right, up;
	float side;
	float width;
	float length;

	ent->moveinfo.sound_start = Server::SV_SoundIndex( "doors/dr1_strt.wav" );
	ent->moveinfo.sound_middle = Server::SV_SoundIndex( "doors/dr1_mid.wav" );
	ent->moveinfo.sound_end = Server::SV_SoundIndex( "doors/dr1_end.wav" );

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	Server::PF_setmodel( ent, ent->model );

	ent->blocked = door_secret_blocked;
	ent->use = door_secret_use;

	if( !( ent->targetname ) ||( ent->spawnflags & 1 ) ) {
		ent->health = 0;
		ent->takedamage = DAMAGE_YES;
		ent->die = door_secret_die;
	}

	if( !ent->dmg )
		ent->dmg = 2;

	if( !ent->wait )
		ent->wait = 5;

	ent->moveinfo.accel =
		ent->moveinfo.decel =
		ent->moveinfo.speed = 50;

	// calculate positions
	ent->s.angles.AngleVectors( &forward, &right, &up );
	ent->s.angles = vec3_origin;
	side = 1.0f -( ent->spawnflags & 2 );
	if( ent->spawnflags & 4 )
		width = fabs( up * ent->size );
	else
		width = fabs( right * ent->size );
	length = fabs( forward * ent->size );
	if( ent->spawnflags & 4 )
		ent->pos1 = ent->s.origin.Magnitude( -1.0f * width, up );
	else
		ent->pos1 = ent->s.origin.Magnitude( side * width, right );
	ent->pos2 = ent->pos1.Magnitude( length, forward );

	if( ent->health )
	{
		ent->takedamage = DAMAGE_YES;
		ent->die = door_killed;
		ent->max_health = ent->health;
	}
	else if( ent->targetname && ent->message )
	{
		Server::SV_SoundIndex( "misc/talk.wav" );
		ent->touch = door_touch;
	}

	ent->classname = "func_door";

	Server::SV_LinkEdict( ent );
}


/* QUAKED func_killbox( 1 0 0 ) ?

Kills everything inside when fired, irrespective of protection.

*/
void Game::use_killbox( Entity * self, Entity * other, Entity * activator ) {

	KillBox( self );
}

void Game::SP_func_killbox( Entity * ent ) {

	Server::PF_setmodel( ent, ent->model );
	ent->use = use_killbox;
	ent->svflags = SVF_NOCLIENT;
}
