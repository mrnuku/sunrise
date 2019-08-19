#include "precompiled.h"
#pragma hdrstop

void Game::InitTrigger( Entity * self ) {

	if( self->s.angles != vec3_origin ) G_SetMovedir( self->s.angles, &self->movedir );

	self->solid = SOLID_TRIGGER;
	self->movetype = MOVETYPE_NONE;
	Server::PF_setmodel( self, self->model );
	self->svflags = SVF_NOCLIENT;
}


// the wait time has passed, so set back up for another activation
void Game::multi_wait( Entity * ent ) {

	ent->nextthink = 0;
}


// the trigger was just activated
// ent->activator should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
void Game::multi_trigger( Entity * ent ) {

	if( ent->nextthink )
		return; // already been triggered

	G_UseTargets( ent, ent->activator );

	if( ent->wait > 0 )
	{
		ent->think = multi_wait;
		ent->nextthink = level.time + ent->wait;
	}
	else
	{ // we can't just remove( self ) here, because this is a touch function
		// called while looping through area links...
		ent->touch = NULL;
		ent->nextthink = level.time + FRAMETIME;
		ent->think = G_FreeEdict;
	}
}

void Game::Use_Multi( Entity * ent, Entity * other, Entity * activator ) {

	ent->activator = activator;
	multi_trigger( ent );
}

void Game::Touch_Multi( Entity * self, Entity * other, Plane & plane, int surf ) {

	if( other->client )
	{
		if( self->spawnflags & 2 )
			return;
	}
	else if( other->svflags & SVF_MONSTER )
	{
		if( !( self->spawnflags & 1 ) ) 	return;
	}
	else
		return;

	if( self->movedir != vec3_origin ) {

		Vec3 forward;

		other->s.angles.AngleVectors( &forward, NULL, NULL );
		if( forward * self->movedir < 0.0f )
			return;
	}

	self->activator = other;
	multi_trigger( self );
}

/* QUAKED trigger_multiple( .5 .5 .5 ) ? MONSTER NOT_PLAYER TRIGGERED

Variable sized repeatable trigger.  Must be targeted at one or more entities.

If "delay" is set, the trigger waits some time after activating before firing.

"wait" : Seconds between triggerings.( .2 default )

sounds

1 )	secret

2 )	beep beep

3 )	large switch

4 )

set "message" to text string

*/
void Game::trigger_enable( Entity * self, Entity * other, Entity * activator ) {

	self->solid = SOLID_TRIGGER;
	self->use = Use_Multi;
	Server::SV_LinkEdict( self );
}

void Game::SP_trigger_multiple( Entity * ent ) {

	if( ent->sounds == 1 )
		ent->noise_index = Server::SV_SoundIndex( "misc/secret.wav" );
	else if( ent->sounds == 2 )
		ent->noise_index = Server::SV_SoundIndex( "misc/talk.wav" );
	else if( ent->sounds == 3 )
		ent->noise_index = Server::SV_SoundIndex( "misc/trigger1.wav" );

	if( !ent->wait )
		ent->wait = 0.2f;
	ent->touch = Touch_Multi;
	ent->movetype = MOVETYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;


	if( ent->spawnflags & 4 )
	{
		ent->solid = SOLID_NOT;
		ent->use = trigger_enable;
	}
	else
	{
		ent->solid = SOLID_TRIGGER;
		ent->use = Use_Multi;
	}

	if( ent->s.angles != vec3_origin ) G_SetMovedir( ent->s.angles, &ent->movedir );

	Server::PF_setmodel( ent, ent->model );
	Server::SV_LinkEdict( ent );
}


/* QUAKED trigger_once( .5 .5 .5 ) ? x x TRIGGERED

Triggers once, then removes itself.

You must set the key "target" to the name of another object in the level that has a matching "targetname".



If TRIGGERED, this trigger must be triggered before it is live.



sounds

1 )	secret

2 )	beep beep

3 )	large switch

4 )



"message"	string to be displayed when triggered

*/
void Game::SP_trigger_once( Entity * ent ) {

	// make old maps work because I messed up on flag assignments here
	// triggered was on bit 1 when it should have been on bit 4
	if( ent->spawnflags & 1 )
	{
		Vec3 v;

		v = ent->mins.Magnitude( 0.5f, ent->size );
		ent->spawnflags &= ~1;
		ent->spawnflags |= 4;
		Common::Com_DPrintf( "fixed TRIGGERED flag on %s at %s\n", ent->classname.c_str( ), vtos( v ).c_str( ) );
	}

	ent->wait = -1;
	SP_trigger_multiple( ent );
}

/* QUAKED trigger_relay( .5 .5 .5 )( -8 -8 -8 )( 8 8 8 )

This fixed size trigger cannot be touched, it can only be fired by other events.

*/
void Game::trigger_relay_use( Entity * self, Entity * other, Entity * activator ) {

	G_UseTargets( self, activator );
}

void Game::SP_trigger_relay( Entity * self ) {

	self->use = trigger_relay_use;
}


/*

==============================================================================



trigger_key



==============================================================================

*/
/* QUAKED trigger_key( .5 .5 .5 )( -8 -8 -8 )( 8 8 8 )

A relay trigger that only fires it's targets if player has the proper key.

Use "item" to specify the required key, for example "key_data_cd"

*/
void Game::trigger_key_use( Entity * self, Entity * other, Entity * activator ) {

	int index;

	if( !self->item )
		return;
	if( !activator->client )
		return;

	index = ITEM_INDEX( self->item );
	if( !activator->client->pers.inventory[ index ] )
	{
		if( level.time < self->touch_debounce_time )
			return;
		self->touch_debounce_time = level.time + 5.0f;
		Server::PF_centerprintf( activator, "You need the %s", self->item->pickup_name.c_str( ) );
		Server::PF_StartSound( activator, CHAN_AUTO, Server::SV_SoundIndex( "misc/keytry.wav" ), 1, ATTN_NORM, 0 );
		return;
	}

	Server::PF_StartSound( activator, CHAN_AUTO, Server::SV_SoundIndex( "misc/keyuse.wav" ), 1, ATTN_NORM, 0 );
	if( coop.GetBool( ) )
	{
		int player;
		Entity * ent;

		if( strcmp( self->item->classname, "key_power_cube" ) == 0 )
		{
			int cube;

			for( cube = 0; cube < 8; cube++ )
				if( activator->client->pers.power_cubes &( 1 << cube ) ) 			break;
			for( player = 1; player <= maxClients; player++ )
			{
				ent = &g_edicts[ player ];
				if( !ent->inuse )
					continue;
				if( !ent->client )
					continue;
				if( ent->client->pers.power_cubes &( 1 << cube ) )
				{
					ent->client->pers.inventory[ index ]--;
					ent->client->pers.power_cubes &= ~( 1 << cube );
				}
			}
		}
		else
		{
			for( player = 1; player <= maxClients; player++ )
			{
				ent = &g_edicts[ player ];
				if( !ent->inuse )
					continue;
				if( !ent->client )
					continue;
				ent->client->pers.inventory[ index ] = 0;
			}
		}
	}
	else
	{
		activator->client->pers.inventory[ index ]--;
	}

	G_UseTargets( self, activator );

	self->use = NULL;
}

void Game::SP_trigger_key( Entity * self ) {

	if( !st.item )
	{
		Common::Com_DPrintf( "no key item for trigger_key at %s\n", vtos( self->s.origin ).c_str( ) );
		return;
	}
	self->item = FindItemByClassname( st.item );

	if( !self->item )
	{
		Common::Com_DPrintf( "item %s not found for trigger_key at %s\n", st.item.c_str( ), vtos( self->s.origin ).c_str( ) );
		return;
	}

	if( !self->target )
	{
		Common::Com_DPrintf( "%s at %s has no target\n", self->classname.c_str( ), vtos( self->s.origin ).c_str( ) );
		return;
	}

	Server::SV_SoundIndex( "misc/keytry.wav" );
	Server::SV_SoundIndex( "misc/keyuse.wav" );

	self->use = trigger_key_use;
}


/*

==============================================================================



trigger_counter



==============================================================================

*/
/* QUAKED trigger_counter( .5 .5 .5 ) ? nomessage

Acts as an intermediary for an action that takes multiple inputs.



If nomessage is not set, t will print "1 more.. " etc when triggered and "sequence complete" when finished.



After the counter has been triggered "count" times( default 2 ), it will fire all of it's targets and remove itself.

*/
void Game::trigger_counter_use( Entity * self, Entity * other, Entity * activator ) {

	if( self->count == 0 )
		return;

	self->count--;

	if( self->count )
	{
		if( !( self->spawnflags & 1 ) )
		{
			Server::PF_centerprintf( activator, "%i more to go...", self->count );
			Server::PF_StartSound( activator, CHAN_AUTO, Server::SV_SoundIndex( "misc/talk1.wav" ), 1, ATTN_NORM, 0 );
		}
		return;
	}

	if( !( self->spawnflags & 1 ) ) {
		Server::PF_centerprintf( activator, "Sequence completed!" );
		Server::PF_StartSound( activator, CHAN_AUTO, Server::SV_SoundIndex( "misc/talk1.wav" ), 1, ATTN_NORM, 0 );
	}
	self->activator = activator;
	multi_trigger( self );
}

void Game::SP_trigger_counter( Entity * self ) {

	self->wait = -1;
	if( !self->count )
		self->count = 2;

	self->use = trigger_counter_use;
}


/*

==============================================================================



trigger_always



==============================================================================

*/
/* QUAKED trigger_always( .5 .5 .5 )( -8 -8 -8 )( 8 8 8 )

This trigger will always fire.  It is activated by the world.

*/
void Game::SP_trigger_always( Entity * ent ) {

	// we must have some delay to make sure our use targets are present
	if( ent->delay < 0.2f )
		ent->delay = 0.2f;
	G_UseTargets( ent, ent );
}


/*

==============================================================================



trigger_push



==============================================================================

*/
static int windsound;

void Game::trigger_push_touch( Entity * self, Entity * other, Plane & plane, int surf ) {

	if( strcmp( other->classname, "grenade" ) == 0 )
	{
		other->velocity = self->movedir * ( self->speed * 10.0f );
	}
	else if( other->health > 0 )
	{
		other->velocity = self->movedir * ( self->speed * 10.0f );

		if( other->client )
		{
			// don't take falling damage immediately from this
			other->client->oldvelocity = other->velocity;
			if( other->fly_sound_debounce_time < level.time )
			{
				other->fly_sound_debounce_time = level.time + 1.5f;
				Server::PF_StartSound( other, CHAN_AUTO, windsound, 1, ATTN_NORM, 0 );
			}
		}
	}
	if( self->spawnflags & 1 )
		G_FreeEdict( self );
}


/* QUAKED trigger_push( .5 .5 .5 ) ? PUSH_ONCE

Pushes the player

"speed"		defaults to 1000

*/
void Game::SP_trigger_push( Entity * self ) {

	InitTrigger( self );
	windsound = Server::SV_SoundIndex( "misc/windfly.wav" );
	self->touch = trigger_push_touch;
	if( !self->speed )
		self->speed = 1000;
	Server::SV_LinkEdict( self );
}


/*

==============================================================================



trigger_hurt



==============================================================================

*/
/* QUAKED trigger_hurt( .5 .5 .5 ) ? START_OFF TOGGLE SILENT NO_PROTECTION SLOW

Any entity that touches this will be hurt.



It does dmg points of damage each server frame



SILENT			supresses playing the sound

SLOW			changes the damage rate to once per second

NO_PROTECTION	* nothing* stops the damage



"dmg"			default 5( whole numbers only )



*/
void Game::hurt_use( Entity * self, Entity * other, Entity * activator ) {

	if( self->solid == SOLID_NOT )
		self->solid = SOLID_TRIGGER;
	else
		self->solid = SOLID_NOT;
	Server::SV_LinkEdict( self );

	if( !( self->spawnflags & 2 ) ) self->use = NULL;
}


void Game::hurt_touch( Entity * self, Entity * other, Plane & plane, int surf ) {

	int dflags;

	if( !other->takedamage )
		return;

	if( self->timestamp > level.time )
		return;

	if( self->spawnflags & 16 )
		self->timestamp = level.time + 1.0f;
	else
		self->timestamp = level.time + FRAMETIME;

	if( !( self->spawnflags & 4 ) ) {
		if( ( level.framenum % 10 ) == 0 )
			Server::PF_StartSound( other, CHAN_AUTO, self->noise_index, 1, ATTN_NORM, 0 );
	}

	if( self->spawnflags & 8 )
		dflags = DAMAGE_NO_PROTECTION;
	else
		dflags = 0;
	T_Damage( other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, self->dmg, dflags, MOD_TRIGGER_HURT );
}

void Game::SP_trigger_hurt( Entity * self ) {

	InitTrigger( self );

	self->noise_index = Server::SV_SoundIndex( "world/electro.wav" );
	self->touch = hurt_touch;

	if( !self->dmg )
		self->dmg = 5;

	if( self->spawnflags & 1 )
		self->solid = SOLID_NOT;
	else
		self->solid = SOLID_TRIGGER;

	if( self->spawnflags & 2 )
		self->use = hurt_use;

	Server::SV_LinkEdict( self );
}


/*

==============================================================================



trigger_gravity



==============================================================================

*/
/* QUAKED trigger_gravity( .5 .5 .5 ) ?

Changes the touching entites gravity to

the value of "gravity".  1.0f is standard

gravity for the level.

*/
void Game::trigger_gravity_touch( Entity * self, Entity * other, Plane & plane, int surf ) {

	other->gravity = self->gravity;
}

void Game::SP_trigger_gravity( Entity * self ) {

	if( st.gravity.IsEmpty( ) ) {
		Common::Com_DPrintf( "trigger_gravity without gravity set at %s\n", vtos( self->s.origin ).c_str( ) );
		G_FreeEdict( self );
		return;
	}

	InitTrigger( self );
	self->gravity = ( float )atoi( st.gravity );
	self->touch = trigger_gravity_touch;
}


/*

==============================================================================



trigger_monsterjump



==============================================================================

*/
/* QUAKED trigger_monsterjump( .5 .5 .5 ) ?

Walking monsters that touch this will jump in the direction of the trigger's angle

"speed" default to 200, the speed thrown forward

"height" default to 200, the speed thrown upwards

*/
void Game::trigger_monsterjump_touch( Entity * self, Entity * other, Plane & plane, int surf ) {

	if( other->flags &( FL_FLY | FL_SWIM ) )
		return;
	if( other->svflags & SVF_DEADMONSTER )
		return;
	if( !( other->svflags & SVF_MONSTER ) ) return;

	// set XY even if not on ground, so the jump will clear lips
	other->velocity[ 0 ] = self->movedir[ 0 ] * self->speed;
	other->velocity[ 1 ] = self->movedir[ 1 ] * self->speed;

	if( !other->groundentity )
		return;

	other->groundentity = NULL;
	other->velocity[ 2 ] = self->movedir[ 2 ];
}

void Game::SP_trigger_monsterjump( Entity * self ) {

	if( !self->speed )
		self->speed = 200;
	if( !st.height )
		st.height = 200;
	if( self->s.angles[ YAW ] == 0 )
		self->s.angles[ YAW ] = 360;
	InitTrigger( self );
	self->touch = trigger_monsterjump_touch;
	self->movedir[ 2 ] = ( float )st.height;
}
