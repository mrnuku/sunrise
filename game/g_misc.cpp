#include "precompiled.h"
#pragma hdrstop

/* QUAKED func_group( 0 0 0 ) ?

Used to group brushes together just for editor convenience.

*/
//=====================================================

void Game::Use_Areaportal( Entity * ent, Entity * other, Entity * activator ) {

	ent->count ^= 1; // toggle state
	//	Common::Com_DPrintf( "portalstate: %i = %i\n", ent->style, ent->count );
	CollisionModel::CM_SetAreaPortalState( ent->style, ent->count > 0 );
}

/* QUAKED func_areaportal( 0 0 0 ) ?



This is a non-visible object that divides the world into

areas that are seperated when this portal is not activated.

Usually enclosed in the middle of a door.

*/
void Game::SP_func_areaportal( Entity * ent ) {

	ent->use = Use_Areaportal;
	ent->count = 0; // always start closed;
}

//=====================================================


/*

=================

Misc functions

=================

*/
Vec3 Game::VelocityForDamage( int damage ) {

	Vec3 v;

	v[ 0 ] = 100.0f * crandom( );
	v[ 1 ] = 100.0f * crandom( );
	v[ 2 ] = 200.0f + 100.0f * random( );

	if( damage < 50 ) v *= 0.7f;
	else v *= 1.2f;

	return v;
}

void Game::ClipGibVelocity( Entity * ent ) {

	if( ent->velocity[ 0 ] < -300 )
		ent->velocity[ 0 ] = -300;
	else if( ent->velocity[ 0 ] > 300 )
		ent->velocity[ 0 ] = 300;
	if( ent->velocity[ 1 ] < -300 )
		ent->velocity[ 1 ] = -300;
	else if( ent->velocity[ 1 ] > 300 )
		ent->velocity[ 1 ] = 300;
	if( ent->velocity[ 2 ] < 200 )
		ent->velocity[ 2 ] = 200; // always some upwards
	else if( ent->velocity[ 2 ] > 500 )
		ent->velocity[ 2 ] = 500;
}


/*

=================

gibs

=================

*/
void Game::gib_think( Entity * self ) {

	self->s.frame++;
	self->nextthink = level.time + FRAMETIME;

	if( self->s.frame == 10 )
	{
		self->think = G_FreeEdict;
		self->nextthink = level.time + 8 + random( )* 10;
	}
}

void Game::gib_touch( Entity * self, Entity * other, Plane & plane, int surf ) {

	Vec3 normal_angles, right;

	if( !self->groundentity )
		return;

	self->touch = NULL;

	if( plane!= plane_origin )
	{
		Server::PF_StartSound( self, CHAN_VOICE, Server::SV_SoundIndex( "misc/fhit3.wav" ), 1, ATTN_NORM, 0 );

		vectoangles( plane.Normal( ), &normal_angles );
		normal_angles.AngleVectors( NULL, &right, NULL );
		vectoangles( right, &self->s.angles );

		if( self->s.modelindex == sm_meat_index )
		{
			self->s.frame++;
			self->think = gib_think;
			self->nextthink = level.time + FRAMETIME;
		}
	}
}

void Game::gib_die( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point ) {

	G_FreeEdict( self );
}

void Game::ThrowGib( Entity * self, const Str & gibname, int damage, int type ) {

	Entity * gib;
	Vec3 vd;
	Vec3 origin;
	Vec3 size;
	float vscale;

	gib = G_Spawn( );

	size = self->size * 0.5f;
	origin = self->absmin + size;
	gib->s.origin[ 0 ] = origin[ 0 ] + crandom( ) * size[ 0 ];
	gib->s.origin[ 1 ] = origin[ 1 ] + crandom( ) * size[ 1 ];
	gib->s.origin[ 2 ] = origin[ 2 ] + crandom( ) * size[ 2 ];

	Server::PF_setmodel( gib, gibname );
	gib->solid = SOLID_NOT;
	gib->s.effects |= EF_GIB;
	gib->flags |= FL_NO_KNOCKBACK;
	gib->takedamage = DAMAGE_YES;
	gib->die = gib_die;

	if( type == GIB_ORGANIC ) {

		gib->movetype = MOVETYPE_TOSS;
		gib->touch = gib_touch;
		vscale = 0.5f;

	} else {

		gib->movetype = MOVETYPE_BOUNCE;
		vscale = 1.0f;
	}

	vd = VelocityForDamage( damage );
	gib->velocity = self->velocity.Magnitude( vscale, vd );
	ClipGibVelocity( gib );
	gib->avelocity[ 0 ] = random( )* 600;
	gib->avelocity[ 1 ] = random( )* 600;
	gib->avelocity[ 2 ] = random( )* 600;

	gib->think = G_FreeEdict;
	gib->nextthink = level.time + 10 + random( )* 10;

	Server::SV_LinkEdict( gib );
}

void Game::ThrowHead( Entity * self, const Str & gibname, int damage, int type ) {

	Vec3 vd;
	float vscale;

	self->s.skinnum = 0;
	self->s.frame = 0;
	self->mins = vec3_origin;
	self->maxs = vec3_origin;

	self->s.modelindex2 = 0;
	Server::PF_setmodel( self, gibname );
	self->solid = SOLID_NOT;
	self->s.effects |= EF_GIB;
	self->s.effects &= ~EF_FLIES;
	self->s.sound = 0;
	self->flags |= FL_NO_KNOCKBACK;
	self->svflags &= ~SVF_MONSTER;
	self->takedamage = DAMAGE_YES;
	self->die = gib_die;

	if( type == GIB_ORGANIC ) {

		self->movetype = MOVETYPE_TOSS;
		self->touch = gib_touch;
		vscale = 0.5f;

	} else {

		self->movetype = MOVETYPE_BOUNCE;
		vscale = 1.0f;
	}

	vd = VelocityForDamage( damage );
	self->velocity = self->velocity.Magnitude( vscale, vd );
	ClipGibVelocity( self );

	self->avelocity[ YAW ] = crandom( )* 600;

	self->think = G_FreeEdict;
	self->nextthink = level.time + 10 + random( )* 10;

	Server::SV_LinkEdict( self );
}


void Game::ThrowClientHead( Entity * self, int damage ) {

	Vec3 vd;
	Str gibname;

	if( rand( )&1 )
	{
		gibname = headGib;
		self->s.skinnum = 1; // second skin is player
	}
	else
	{
		gibname = "models/objects/gibs/skull/tris.md2";
		self->s.skinnum = 0;
	}

	self->s.origin[ 2 ] += 32;
	self->s.frame = 0;
	Server::PF_setmodel( self, gibname );
	self->mins = Vec3( -16, -16, 0 );
	self->maxs = Vec3( 16, 16, 16 );

	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;
	self->s.effects = EF_GIB;
	self->s.sound = 0;
	self->flags |= FL_NO_KNOCKBACK;

	self->movetype = MOVETYPE_BOUNCE;
	vd = VelocityForDamage( damage );
	self->velocity = self->velocity + vd;

	if( self->client ) // bodies in the queue don't have a client anymore
	{
		self->client->anim_priority = ANIM_DEATH;
		self->client->anim_end = self->s.frame;
	}
	else
	{
		self->think = NULL;
		self->nextthink = 0;
	}

	Server::SV_LinkEdict( self );
}


/*

=================

debris

=================

*/
void Game::debris_die( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point ) {

	G_FreeEdict( self );
}

void Game::ThrowDebris( Entity * self, const Str & modelname, float speed, Vec3 & origin ) {

	Entity * chunk;
	Vec3 v;

	chunk = G_Spawn( );
	chunk->s.origin = origin;
	Server::PF_setmodel( chunk, modelname );
	v[ 0 ] = 100 * crandom( );
	v[ 1 ] = 100 * crandom( );
	v[ 2 ] = 100 + 100 * crandom( );
	chunk->velocity = self->velocity.Magnitude( speed, v );
	chunk->movetype = MOVETYPE_BOUNCE;
	chunk->solid = SOLID_NOT;
	chunk->avelocity[ 0 ] = random( )* 600;
	chunk->avelocity[ 1 ] = random( )* 600;
	chunk->avelocity[ 2 ] = random( )* 600;
	chunk->think = G_FreeEdict;
	chunk->nextthink = level.time + 5 + random( )* 5;
	chunk->s.frame = 0;
	chunk->flags = 0;
	chunk->classname = "debris";
	chunk->takedamage = DAMAGE_YES;
	chunk->die = debris_die;
	Server::SV_LinkEdict( chunk );
}


void Game::BecomeExplosion1( Entity * self ) {

	Server::PF_WriteByte( svc_temp_entity );
	Server::PF_WriteByte( TE_EXPLOSION1 );
	Server::PF_WritePos( self->s.origin );
	Server::SV_Multicast( self->s.origin, MULTICAST_PVS );

	G_FreeEdict( self );
}


void Game::BecomeExplosion2( Entity * self ) {

	Server::PF_WriteByte( svc_temp_entity );
	Server::PF_WriteByte( TE_EXPLOSION2 );
	Server::PF_WritePos( self->s.origin );
	Server::SV_Multicast( self->s.origin, MULTICAST_PVS );

	G_FreeEdict( self );
}


/* QUAKED path_corner( .5 .3 0 )( -8 -8 -8 )( 8 8 8 ) TELEPORT

Target: next path corner

Pathtarget: gets used when an entity that has

this path_corner targeted touches it

*/
void Game::path_corner_touch( Entity * self, Entity * other, Plane & plane, int surf ) {

	Vec3 v;
	Entity * next;

	if( other->movetarget != self )
		return;

	if( other->enemy )
		return;

	if( self->pathtarget ) {

		Str savetarget;

		savetarget = self->target;
		self->target = self->pathtarget;
		G_UseTargets( self, other );
		self->target = savetarget;
	}

	if( self->target ) next = G_PickTarget( self->target );
	else next = NULL;

	if( ( next ) &&( next->spawnflags & 1 ) ) {

		v = next->s.origin;
		v[ 2 ] += next->mins[ 2 ];
		v[ 2 ] -= other->mins[ 2 ];
		other->s.origin = v;
		next = G_PickTarget( next->target );
		other->s.eventNum = EV_OTHER_TELEPORT;
	}

	other->goalentity = other->movetarget = next;

	if( self->wait )
	{
		other->monsterinfo.pausetime = level.time + self->wait;
		other->monsterinfo.stand( other );
		return;
	}

	if( !other->movetarget )
	{
		other->monsterinfo.pausetime = level.time + 100000000.0f;
		other->monsterinfo.stand( other );
	}
	else
	{
		v = other->goalentity->s.origin - other->s.origin;
		other->eal_yaw = vectoyaw( v );
	}
}

void Game::SP_path_corner( Entity * self ) {

	if( !self->targetname )
	{
		Common::Com_DPrintf( "path_corner with no targetname at %s\n", vtos( self->s.origin ).c_str( ) );
		G_FreeEdict( self );
		return;
	}

	self->solid = SOLID_TRIGGER;
	self->touch = path_corner_touch;
	self->mins = Vec3( -8, -8, -8 );
	self->maxs = Vec3( 8, 8, 8 );
	self->svflags |= SVF_NOCLIENT;
	Server::SV_LinkEdict( self );
}


/* QUAKED point_combat( 0.5f 0.3f 0 )( -8 -8 -8 )( 8 8 8 ) Hold

Makes this the target of a monster and it will head here

when first activated before going after the activator.  If

hold is selected, it will stay here.

*/
void Game::point_combat_touch( Entity * self, Entity * other, Plane & plane, int surf ) {

	Entity * activator;

	if( other->movetarget != self )
		return;

	if( self->target ) {

		other->target = self->target;
		other->goalentity = other->movetarget = G_PickTarget( other->target );
		if( !other->goalentity )
		{
			Common::Com_DPrintf( "%s at %s target %s does not exist\n", self->classname.c_str( ), vtos( self->s.origin ).c_str( ), self->target.c_str( ) );
			other->movetarget = self;
		}
		self->target = NULL;

	} else if( ( self->spawnflags & 1 ) && !( other->flags &( FL_SWIM|FL_FLY ) ) ) {

		other->monsterinfo.pausetime = level.time + 100000000.0f;
		other->monsterinfo.aiflags |= AI_STAND_GROUND;
		other->monsterinfo.stand( other );
	}

	if( other->movetarget == self ) {

		other->target = NULL;
		other->movetarget = NULL;
		other->goalentity = other->enemy;
		other->monsterinfo.aiflags &= ~AI_COMBAT_POINT;
	}

	if( self->pathtarget ) {

		Str savetarget;

		savetarget = self->target;
		self->target = self->pathtarget;
		if( other->enemy && other->enemy->client )
			activator = other->enemy;
		else if( other->oldenemy && other->oldenemy->client )
			activator = other->oldenemy;
		else if( other->activator && other->activator->client )
			activator = other->activator;
		else
			activator = other;
		G_UseTargets( self, activator );
		self->target = savetarget;
	}
}

void Game::SP_point_combat( Entity * self ) {

	if( deathmatch.GetBool( ) ) {

		G_FreeEdict( self );
		return;
	}
	self->solid = SOLID_TRIGGER;
	self->touch = point_combat_touch;
	self->mins = Vec3( -8, -8, -16 );
	self->maxs = Vec3( 8, 8, 16 );
	self->svflags = SVF_NOCLIENT;
	Server::SV_LinkEdict( self );
};


/* QUAKED viewthing( 0 .5 .8 )( -8 -8 -8 )( 8 8 8 )

Just for the debugging level.  Don't use

*/
void Game::TH_viewthing( Entity * ent ) {

	ent->s.frame =( ent->s.frame + 1 ) % 7;
	ent->nextthink = level.time + FRAMETIME;
}

void Game::SP_viewthing( Entity * ent ) {

	Common::Com_DPrintf( "viewthing spawned\n" );

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.renderfx = RF_FRAMELERP;
	ent->mins = Vec3( -16, -16, -24 );
	ent->maxs = Vec3( 16, 16, 32 );
	ent->s.modelindex = Server::SV_ModelIndex( "models/objects/banner/tris.md2" );
	Server::SV_LinkEdict( ent );
	ent->nextthink = level.time + 0.5f;
	ent->think = TH_viewthing;
	return;
}


/* QUAKED info_null( 0 0.5f 0 )( -4 -4 -4 )( 4 4 4 )

Used as a positional target for spotlights, etc.

*/
void Game::SP_info_null( Entity * self ) {

	G_FreeEdict( self );
};


/* QUAKED info_notnull( 0 0.5f 0 )( -4 -4 -4 )( 4 4 4 )

Used as a positional target for lightning.

*/
void Game::SP_info_notnull( Entity * self ) {

	self->absmin = self->s.origin;
	self->absmax = self->s.origin;
};


/* QUAKED light( 0 1 0 )( -8 -8 -8 )( 8 8 8 ) START_OFF

Non-displayed light.

Default light value is 300.

Default style is 0.

If targeted, will toggle between on and off.

Default _cone value is 10( used to set size of light for spotlights )

*/
void Game::light_use( Entity * self, Entity * other, Entity * activator ) {

	if( self->spawnflags & 1 )
	{
		Server::PF_Configstring( CS_LIGHTS+self->style, "m" );
		self->spawnflags &= ~1;
	}
	else
	{
		Server::PF_Configstring( CS_LIGHTS+self->style, "a" );
		self->spawnflags |= 1;
	}
}

void Game::SP_light( Entity * self ) {

	// no targeted lights in deathmatch, because they cause global messages
	if( !self->targetname || deathmatch.GetBool( ) )
	{
		G_FreeEdict( self );
		return;
	}

	if( self->style >= 32 )
	{
		self->use = light_use;
		if( self->spawnflags & 1 )
			Server::PF_Configstring( CS_LIGHTS+self->style, "a" );
		else
			Server::PF_Configstring( CS_LIGHTS+self->style, "m" );
	}
}


/* QUAKED func_wall( 0 .5 .8 ) ? TRIGGER_SPAWN TOGGLE START_ON ANIMATED ANIMATED_FAST

This is just a solid wall if not inhibited



TRIGGER_SPAWN	the wall will not be present until triggered

it will then blink in to existance; it will

kill anything that was in it's way



TOGGLE			only valid for TRIGGER_SPAWN walls

this allows the wall to be turned on and off



START_ON		only valid for TRIGGER_SPAWN walls

the wall will initially be present

*/
void Game::func_wall_use( Entity * self, Entity * other, Entity * activator ) {

	if( self->solid == SOLID_NOT )
	{
		self->solid = SOLID_BSP;
		self->svflags &= ~SVF_NOCLIENT;
		KillBox( self );
	}
	else
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}
	Server::SV_LinkEdict( self );

	if( !( self->spawnflags & 2 ) ) self->use = NULL;
}

void Game::SP_func_wall( Entity * self ) {

	self->movetype = MOVETYPE_PUSH;
	Server::PF_setmodel( self, self->model );

	if( self->spawnflags & 8 )
		self->s.effects |= EF_ANIM_ALL;
	if( self->spawnflags & 16 )
		self->s.effects |= EF_ANIM_ALLFAST;

	// just a wall
	if( ( self->spawnflags & 7 ) == 0 )
	{
		self->solid = SOLID_BSP;
		Server::SV_LinkEdict( self );
		return;
	}

	// it must be TRIGGER_SPAWN
	if( !( self->spawnflags & 1 ) ) {
		//		Common::Com_DPrintf( "func_wall missing TRIGGER_SPAWN\n" );
		self->spawnflags |= 1;
	}

	// yell if the spawnflags are odd
	if( self->spawnflags & 4 )
	{
		if( !( self->spawnflags & 2 ) )
		{
			Common::Com_DPrintf( "func_wall START_ON without TOGGLE\n" );
			self->spawnflags |= 2;
		}
	}

	self->use = func_wall_use;
	if( self->spawnflags & 4 )
	{
		self->solid = SOLID_BSP;
	}
	else
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}
	Server::SV_LinkEdict( self );
}


/* QUAKED func_object( 0 .5 .8 ) ? TRIGGER_SPAWN ANIMATED ANIMATED_FAST

This is solid bmodel that will fall if it's support it removed.

*/
void Game::func_object_touch( Entity * self, Entity * other, Plane & plane, int surf ) {

	// only squash thing we fall on top of
	if( plane == plane_origin )
		return;
	if( plane.Normal( )[ 2 ] < 1.0f )
		return;
	if( other->takedamage == DAMAGE_NO )
		return;
	T_Damage( other, self, self, vec3_origin, self->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH );
}

void Game::func_object_release( Entity * self ) {

	self->movetype = MOVETYPE_TOSS;
	self->touch = func_object_touch;
}

void Game::func_object_use( Entity * self, Entity * other, Entity * activator ) {

	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
	self->use = NULL;
	KillBox( self );
	func_object_release( self );
}

void Game::SP_func_object( Entity * self ) {

	Server::PF_setmodel( self, self->model );

	self->mins[ 0 ] += 1;
	self->mins[ 1 ] += 1;
	self->mins[ 2 ] += 1;
	self->maxs[ 0 ] -= 1;
	self->maxs[ 1 ] -= 1;
	self->maxs[ 2 ] -= 1;

	if( !self->dmg )
		self->dmg = 100;

	if( self->spawnflags == 0 )
	{
		self->solid = SOLID_BSP;
		self->movetype = MOVETYPE_PUSH;
		self->think = func_object_release;
		self->nextthink = level.time + 2.0f * FRAMETIME;
	}
	else
	{
		self->solid = SOLID_NOT;
		self->movetype = MOVETYPE_PUSH;
		self->use = func_object_use;
		self->svflags |= SVF_NOCLIENT;
	}

	if( self->spawnflags & 2 )
		self->s.effects |= EF_ANIM_ALL;
	if( self->spawnflags & 4 )
		self->s.effects |= EF_ANIM_ALLFAST;

	self->clipmask = MASK_MONSTERSOLID;

	Server::SV_LinkEdict( self );
}


/* QUAKED func_explosive( 0 .5 .8 ) ? Trigger_Spawn ANIMATED ANIMATED_FAST

Any brush that you want to explode or break apart.  If you want an

ex0plosion, set dmg and it will do a radius explosion of that amount

at the center of the bursh.



If targeted it will not be shootable.



health defaults to 100.



mass defaults to 75.  This determines how much debris is emitted when

it explodes.  You get one large chunk per 100 of mass( up to 8 ) and

one small chunk per 25 of mass( up to 16 ).  So 800 gives the most.

*/
void Game::func_explosive_explode( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point ) {

	Vec3 origin;
	Vec3 chunkorigin;
	Vec3 size;
	int count;
	int mass;

	// bmodel origins are( 0 0 0 ), we need to adjust that here
	size = self->size * 0.5f;
	origin = self->absmin + size;
	self->s.origin = origin;

	self->takedamage = DAMAGE_NO;

	if( self->dmg )
		T_RadiusDamage( self, attacker, ( float )( self->dmg ), NULL, ( float )( self->dmg+40 ), MOD_EXPLOSIVE );

	self->velocity = self->s.origin - inflictor->s.origin;
	self->velocity.Normalize( );
	self->velocity *= 150.0f;

	// start chunks towards the center
	size *= 0.5f;

	mass = self->mass;
	if( !mass )
		mass = 75;

	// big chunks
	if( mass >= 100 )
	{
		count = mass / 100;
		if( count > 8 )
			count = 8;
		while( count-- )
		{
			chunkorigin[ 0 ] = origin[ 0 ] + crandom( ) * size[ 0 ];
			chunkorigin[ 1 ] = origin[ 1 ] + crandom( ) * size[ 1 ];
			chunkorigin[ 2 ] = origin[ 2 ] + crandom( ) * size[ 2 ];
			ThrowDebris( self, "models/objects/debris1/tris.md2", 1, chunkorigin );
		}
	}

	// small chunks
	count = mass / 25;
	if( count > 16 )
		count = 16;
	while( count-- )
	{
		chunkorigin[ 0 ] = origin[ 0 ] + crandom( ) * size[ 0 ];
		chunkorigin[ 1 ] = origin[ 1 ] + crandom( ) * size[ 1 ];
		chunkorigin[ 2 ] = origin[ 2 ] + crandom( ) * size[ 2 ];
		ThrowDebris( self, "models/objects/debris2/tris.md2", 2, chunkorigin );
	}

	G_UseTargets( self, attacker );

	if( self->dmg )
		BecomeExplosion1( self );
	else
		G_FreeEdict( self );
}

void Game::func_explosive_use( Entity * self, Entity * other, Entity * activator ) {

	func_explosive_explode( self, self, other, self->health, vec3_origin );
}

void Game::func_explosive_spawn( Entity * self, Entity * other, Entity * activator ) {

	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
	self->use = NULL;
	KillBox( self );
	Server::SV_LinkEdict( self );
}

void Game::SP_func_explosive( Entity * self ) {

	if( deathmatch.GetBool( ) )
	{ // auto-remove for deathmatch
		G_FreeEdict( self );
		return;
	}

	self->movetype = MOVETYPE_PUSH;

	Server::SV_ModelIndex( "models/objects/debris1/tris.md2" );
	Server::SV_ModelIndex( "models/objects/debris2/tris.md2" );

	Server::PF_setmodel( self, self->model );

	if( self->spawnflags & 1 )
	{
		self->svflags |= SVF_NOCLIENT;
		self->solid = SOLID_NOT;
		self->use = func_explosive_spawn;
	}
	else
	{
		self->solid = SOLID_BSP;
		if( self->targetname )
			self->use = func_explosive_use;
	}

	if( self->spawnflags & 2 )
		self->s.effects |= EF_ANIM_ALL;
	if( self->spawnflags & 4 )
		self->s.effects |= EF_ANIM_ALLFAST;

	if( self->use != func_explosive_use )
	{
		if( !self->health )
			self->health = 100;
		self->die = func_explosive_explode;
		self->takedamage = DAMAGE_YES;
	}

	Server::SV_LinkEdict( self );
}


/* QUAKED misc_explobox( 0 .5 .8 )( -16 -16 0 )( 16 16 40 )

Large exploding box.  You can override its mass( 100 ), 
health( 80 ), and dmg( 150 ).

*/
void Game::barrel_touch( Entity * self, Entity * other, Plane & plane, int surf ) {

	float ratio;
	Vec3 v;

	if( ( !other->groundentity ) ||( other->groundentity == self ) ) return;

	ratio =( float )other->mass /( float )self->mass;
	v = self->s.origin - other->s.origin;
	M_walkmove( self, vectoyaw( v ), 20.0f * ratio * FRAMETIME );
}

void Game::barrel_explode( Entity * self ) {

	Vec3 org;
	float spd;
	Vec3 save;

	T_RadiusDamage( self, self->activator, ( float )( self->dmg ), NULL, ( float )( self->dmg+40 ), MOD_BARREL );

	save = self->s.origin;
	self->s.origin = self->absmin.Magnitude( 0.5f, self->size );

	// a few big chunks
	spd = 1.5f * ( float )self->dmg / 200.0f;
	org[ 0 ] = self->s.origin[ 0 ] + crandom( ) * self->size[ 0 ];
	org[ 1 ] = self->s.origin[ 1 ] + crandom( ) * self->size[ 1 ];
	org[ 2 ] = self->s.origin[ 2 ] + crandom( ) * self->size[ 2 ];
	ThrowDebris( self, "models/objects/debris1/tris.md2", spd, org );
	org[ 0 ] = self->s.origin[ 0 ] + crandom( ) * self->size[ 0 ];
	org[ 1 ] = self->s.origin[ 1 ] + crandom( ) * self->size[ 1 ];
	org[ 2 ] = self->s.origin[ 2 ] + crandom( ) * self->size[ 2 ];
	ThrowDebris( self, "models/objects/debris1/tris.md2", spd, org );

	// bottom corners
	spd = 1.75f * ( float )self->dmg / 200.0f;
	org = self->absmin;
	ThrowDebris( self, "models/objects/debris3/tris.md2", spd, org );
	org = self->absmin;
	org[ 0 ] += self->size[ 0 ];
	ThrowDebris( self, "models/objects/debris3/tris.md2", spd, org );
	org = self->absmin;
	org[ 1 ] += self->size[ 1 ];
	ThrowDebris( self, "models/objects/debris3/tris.md2", spd, org );
	org = self->absmin;
	org[ 0 ] += self->size[ 0 ];
	org[ 1 ] += self->size[ 1 ];
	ThrowDebris( self, "models/objects/debris3/tris.md2", spd, org );

	// a bunch of little chunks
	spd = 2 * ( float )self->dmg / 200;
	org[ 0 ] = self->s.origin[ 0 ] + crandom( ) * self->size[ 0 ];
	org[ 1 ] = self->s.origin[ 1 ] + crandom( ) * self->size[ 1 ];
	org[ 2 ] = self->s.origin[ 2 ] + crandom( ) * self->size[ 2 ];
	ThrowDebris( self, "models/objects/debris2/tris.md2", spd, org );
	org[ 0 ] = self->s.origin[ 0 ] + crandom( ) * self->size[ 0 ];
	org[ 1 ] = self->s.origin[ 1 ] + crandom( ) * self->size[ 1 ];
	org[ 2 ] = self->s.origin[ 2 ] + crandom( ) * self->size[ 2 ];
	ThrowDebris( self, "models/objects/debris2/tris.md2", spd, org );
	org[ 0 ] = self->s.origin[ 0 ] + crandom( ) * self->size[ 0 ];
	org[ 1 ] = self->s.origin[ 1 ] + crandom( ) * self->size[ 1 ];
	org[ 2 ] = self->s.origin[ 2 ] + crandom( ) * self->size[ 2 ];
	ThrowDebris( self, "models/objects/debris2/tris.md2", spd, org );
	org[ 0 ] = self->s.origin[ 0 ] + crandom( ) * self->size[ 0 ];
	org[ 1 ] = self->s.origin[ 1 ] + crandom( ) * self->size[ 1 ];
	org[ 2 ] = self->s.origin[ 2 ] + crandom( ) * self->size[ 2 ];
	ThrowDebris( self, "models/objects/debris2/tris.md2", spd, org );
	org[ 0 ] = self->s.origin[ 0 ] + crandom( ) * self->size[ 0 ];
	org[ 1 ] = self->s.origin[ 1 ] + crandom( ) * self->size[ 1 ];
	org[ 2 ] = self->s.origin[ 2 ] + crandom( ) * self->size[ 2 ];
	ThrowDebris( self, "models/objects/debris2/tris.md2", spd, org );
	org[ 0 ] = self->s.origin[ 0 ] + crandom( ) * self->size[ 0 ];
	org[ 1 ] = self->s.origin[ 1 ] + crandom( ) * self->size[ 1 ];
	org[ 2 ] = self->s.origin[ 2 ] + crandom( ) * self->size[ 2 ];
	ThrowDebris( self, "models/objects/debris2/tris.md2", spd, org );
	org[ 0 ] = self->s.origin[ 0 ] + crandom( ) * self->size[ 0 ];
	org[ 1 ] = self->s.origin[ 1 ] + crandom( ) * self->size[ 1 ];
	org[ 2 ] = self->s.origin[ 2 ] + crandom( ) * self->size[ 2 ];
	ThrowDebris( self, "models/objects/debris2/tris.md2", spd, org );
	org[ 0 ] = self->s.origin[ 0 ] + crandom( ) * self->size[ 0 ];
	org[ 1 ] = self->s.origin[ 1 ] + crandom( ) * self->size[ 1 ];
	org[ 2 ] = self->s.origin[ 2 ] + crandom( ) * self->size[ 2 ];
	ThrowDebris( self, "models/objects/debris2/tris.md2", spd, org );

	self->s.origin = save;
	if( self->groundentity )
		BecomeExplosion2( self );
	else
		BecomeExplosion1( self );
}

void Game::barrel_delay( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point ) {

	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + 2.0f * FRAMETIME;
	self->think = barrel_explode;
	self->activator = attacker;
}

void Game::SP_misc_explobox( Entity * self ) {

	if( deathmatch.GetBool( ) )
	{ // auto-remove for deathmatch
		G_FreeEdict( self );
		return;
	}

	Server::SV_ModelIndex( "models/objects/debris1/tris.md2" );
	Server::SV_ModelIndex( "models/objects/debris2/tris.md2" );
	Server::SV_ModelIndex( "models/objects/debris3/tris.md2" );

	self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_STEP;

	self->model = "models/objects/barrels/tris.md2";
	self->s.modelindex = Server::SV_ModelIndex( self->model );
	self->mins = Vec3( -16, -16, 0 );
	self->maxs = Vec3( 16, 16, 40 );

	if( !self->mass )
		self->mass = 400;
	if( !self->health )
		self->health = 10;
	if( !self->dmg )
		self->dmg = 150;

	self->die = barrel_delay;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.aiflags = AI_NOSTEP;

	self->touch = barrel_touch;

	self->think = M_droptofloor;
	self->nextthink = level.time + 2.0f * FRAMETIME;

	Server::SV_LinkEdict( self );
}


//
// miscellaneous specialty items
//

/* QUAKED misc_blackhole( 1 .5 0 )( -8 -8 -8 )( 8 8 8 )

*/
void Game::misc_blackhole_use( Entity * ent, Entity * other, Entity * activator ) {

	/*

	Server::PF_WriteByte( svc_temp_entity );

	Server::PF_WriteByte( TE_BOSSTPORT );

	Server::PF_WritePos( ent->s.origin );

	Server::SV_Multicast( ent->s.origin, MULTICAST_PVS );

	*/
	G_FreeEdict( ent );
}

void Game::misc_blackhole_think( Entity * self ) {

	if( ++self->s.frame < 19 )
		self->nextthink = level.time + FRAMETIME;
	else
	{
		self->s.frame = 0;
		self->nextthink = level.time + FRAMETIME;
	}
}

void Game::SP_misc_blackhole( Entity * ent ) {

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->mins = Vec3( -64, -64, 0 );
	ent->maxs = Vec3( 64, 64, 8 );
	ent->s.modelindex = Server::SV_ModelIndex( "models/objects/black/tris.md2" );
	ent->s.renderfx = RF_TRANSLUCENT;
	ent->use = misc_blackhole_use;
	ent->think = misc_blackhole_think;
	ent->nextthink = level.time + 2.0f * FRAMETIME;
	Server::SV_LinkEdict( ent );
}

/* QUAKED misc_eastertank( 1 .5 0 )( -32 -32 -16 )( 32 32 32 )

*/
void Game::misc_eastertank_think( Entity * self ) {

	if( ++self->s.frame < 293 )
		self->nextthink = level.time + FRAMETIME;
	else
	{
		self->s.frame = 254;
		self->nextthink = level.time + FRAMETIME;
	}
}

void Game::SP_misc_eastertank( Entity * ent ) {

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->mins = Vec3( -32, -32, -16 );
	ent->maxs = Vec3( 32, 32, 32 );
	ent->s.modelindex = Server::SV_ModelIndex( "models/monsters/tank/tris.md2" );
	ent->s.frame = 254;
	ent->think = misc_eastertank_think;
	ent->nextthink = level.time + 2.0f * FRAMETIME;
	Server::SV_LinkEdict( ent );
}

/* QUAKED misc_easterchick( 1 .5 0 )( -32 -32 0 )( 32 32 32 )

*/

void Game::misc_easterchick_think( Entity * self ) {

	if( ++self->s.frame < 247 )
		self->nextthink = level.time + FRAMETIME;
	else
	{
		self->s.frame = 208;
		self->nextthink = level.time + FRAMETIME;
	}
}

void Game::SP_misc_easterchick( Entity * ent ) {

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->mins = Vec3( -32, -32, 0 );
	ent->maxs = Vec3( 32, 32, 32 );
	ent->s.modelindex = Server::SV_ModelIndex( "models/monsters/bitch/tris.md2" );
	ent->s.frame = 208;
	ent->think = misc_easterchick_think;
	ent->nextthink = level.time + 2.0f * FRAMETIME;
	Server::SV_LinkEdict( ent );
}

/* QUAKED misc_easterchick2( 1 .5 0 )( -32 -32 0 )( 32 32 32 )

*/
void Game::misc_easterchick2_think( Entity * self ) {

	if( ++self->s.frame < 287 )
		self->nextthink = level.time + FRAMETIME;
	else
	{
		self->s.frame = 248;
		self->nextthink = level.time + FRAMETIME;
	}
}

void Game::SP_misc_easterchick2( Entity * ent ) {

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->mins = Vec3( -32, -32, 0 );
	ent->maxs = Vec3( 32, 32, 32 );
	ent->s.modelindex = Server::SV_ModelIndex( "models/monsters/bitch/tris.md2" );
	ent->s.frame = 248;
	ent->think = misc_easterchick2_think;
	ent->nextthink = level.time + 2.0f * FRAMETIME;
	Server::SV_LinkEdict( ent );
}


/* QUAKED monster_commander_body( 1 .5 0 )( -32 -32 0 )( 32 32 48 )

Not really a monster, this is the Tank Commander's decapitated body.

There should be a item_commander_head that has this as it's target.

*/
void Game::commander_body_think( Entity * self ) {

	if( ++self->s.frame < 24 )
		self->nextthink = level.time + FRAMETIME;
	else
		self->nextthink = 0;

	if( self->s.frame == 22 )
		Server::PF_StartSound( self, CHAN_BODY, Server::SV_SoundIndex( "tank/thud.wav" ), 1, ATTN_NORM, 0 );
}

void Game::commander_body_use( Entity * self, Entity * other, Entity * activator ) {

	self->think = commander_body_think;
	self->nextthink = level.time + FRAMETIME;
	Server::PF_StartSound( self, CHAN_BODY, Server::SV_SoundIndex( "tank/pain.wav" ), 1, ATTN_NORM, 0 );
}

void Game::commander_body_drop( Entity * self ) {

	self->movetype = MOVETYPE_TOSS;
	self->s.origin[ 2 ] += 2;
}

void Game::SP_monster_commander_body( Entity * self ) {

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_BBOX;
	self->model = "models/monsters/commandr/tris.md2";
	self->s.modelindex = Server::SV_ModelIndex( self->model );
	self->mins = Vec3( -32, -32, 0 );
	self->maxs = Vec3( 32, 32, 48 );
	self->use = commander_body_use;
	self->takedamage = DAMAGE_YES;
	self->flags = FL_GODMODE;
	self->s.renderfx |= RF_FRAMELERP;
	Server::SV_LinkEdict( self );

	Server::SV_SoundIndex( "tank/thud.wav" );
	Server::SV_SoundIndex( "tank/pain.wav" );

	self->think = commander_body_drop;
	self->nextthink = level.time + 5.0f * FRAMETIME;
}


/* QUAKED misc_banner( 1 .5 0 )( -4 -4 -4 )( 4 4 4 )

The origin is the bottom of the banner.

The banner is 128 tall.

*/
void Game::misc_banner_think( Entity * ent ) {

	ent->s.frame =( ent->s.frame + 1 ) % 16;
	ent->nextthink = level.time + FRAMETIME;
}

void Game::SP_misc_banner( Entity * ent ) {

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = Server::SV_ModelIndex( "models/objects/banner/tris.md2" );
	ent->s.frame = rand( ) % 16;
	Server::SV_LinkEdict( ent );

	ent->think = misc_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

/* QUAKED misc_deadsoldier( 1 .5 0 )( -16 -16 0 )( 16 16 16 ) ON_BACK ON_STOMACH BACK_DECAP FETAL_POS SIT_DECAP IMPALED

This is the dead player model. Comes in 6 exciting different poses!

*/
void Game::misc_deadsoldier_die( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point ) {

	int n;

	if( self->health > -80 )
		return;

	Server::PF_StartSound( self, CHAN_BODY, Server::SV_SoundIndex( "misc/udeath.wav" ), 1, ATTN_NORM, 0 );
	for( n = 0; n < 4; n++ )
		ThrowGib( self, meatGib, damage, GIB_ORGANIC );
	ThrowHead( self, headGib, damage, GIB_ORGANIC );
}

void Game::SP_misc_deadsoldier( Entity * ent ) {

	if( deathmatch.GetBool( ) ) { // auto-remove for deathmatch

		G_FreeEdict( ent );
		return;
	}

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.modelindex = Server::SV_ModelIndex( "models/deadbods/dude/tris.md2" );

	// Defaults to frame 0
	if( ent->spawnflags & 2 )
		ent->s.frame = 1;
	else if( ent->spawnflags & 4 )
		ent->s.frame = 2;
	else if( ent->spawnflags & 8 )
		ent->s.frame = 3;
	else if( ent->spawnflags & 16 )
		ent->s.frame = 4;
	else if( ent->spawnflags & 32 )
		ent->s.frame = 5;
	else
		ent->s.frame = 0;

	ent->mins = Vec3( -16, -16, 0 );
	ent->maxs = Vec3( 16, 16, 16 );
	ent->deadflag = DEAD_DEAD;
	ent->takedamage = DAMAGE_YES;
	ent->svflags |= SVF_MONSTER|SVF_DEADMONSTER;
	ent->die = misc_deadsoldier_die;
	ent->monsterinfo.aiflags |= AI_GOOD_GUY;

	Server::SV_LinkEdict( ent );
}

/* QUAKED misc_viper( 1 .5 0 )( -16 -16 0 )( 16 16 32 )

This is the Viper for the flyby bombing.

It is trigger_spawned, so you must have something use it for it to show up.

There must be a path for it to follow once it is activated.



"speed"		How fast the Viper should fly

*/
void Game::misc_viper_use( Entity * self, Entity * other, Entity * activator ) {

	self->svflags &= ~SVF_NOCLIENT;
	self->use = train_use;
	train_use( self, other, activator );
}

void Game::SP_misc_viper( Entity * ent ) {

	if( !ent->target )
	{
		Common::Com_DPrintf( "misc_viper without a target at %s\n", vtos( ent->absmin ).c_str( ) );
		G_FreeEdict( ent );
		return;
	}

	if( !ent->speed )
		ent->speed = 300;

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = Server::SV_ModelIndex( "models/ships/viper/tris.md2" );
	ent->mins = Vec3( -16, -16, 0 );
	ent->maxs = Vec3( 16, 16, 32 );

	ent->think = func_train_find;
	ent->nextthink = level.time + FRAMETIME;
	ent->use = misc_viper_use;
	ent->svflags |= SVF_NOCLIENT;
	ent->moveinfo.accel = ent->moveinfo.decel = ent->moveinfo.speed = ent->speed;

	Server::SV_LinkEdict( ent );
}


/* QUAKED misc_bigviper( 1 .5 0 )( -176 -120 -24 )( 176 120 72 ) 

This is a large stationary viper as seen in Paul's intro

*/
void Game::SP_misc_bigviper( Entity * ent ) {

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->mins = Vec3( -176, -120, -24 );
	ent->maxs = Vec3( 176, 120, 72 );
	ent->s.modelindex = Server::SV_ModelIndex( "models/ships/bigviper/tris.md2" );
	Server::SV_LinkEdict( ent );
}


/* QUAKED misc_viper_bomb( 1 0 0 )( -8 -8 -8 )( 8 8 8 )

"dmg"	how much boom should the bomb make?

*/
void Game::misc_viper_bomb_touch( Entity * self, Entity * other, Plane & plane, int surf ) {

	G_UseTargets( self, self->activator );

	self->s.origin[ 2 ] = self->absmin[ 2 ] + 1;
	T_RadiusDamage( self, self, ( float )( self->dmg ), NULL, ( float )( self->dmg+40 ), MOD_BOMB );
	BecomeExplosion2( self );
}

void Game::misc_viper_bomb_prethink( Entity * self ) {

	Vec3 v;
	float diff;

	self->groundentity = NULL;

	diff = self->timestamp - level.time;
	if( diff < -1.0f )
		diff = -1.0f;

	v = self->moveinfo.dir * ( 1.0f + diff );
	v[ 2 ] = diff;

	diff = self->s.angles[ 2 ];
	vectoangles( v, &self->s.angles );
	self->s.angles[ 2 ] = diff + 10;
}

void Game::misc_viper_bomb_use( Entity * self, Entity * other, Entity * activator ) {

	Entity * viper;

	self->solid = SOLID_BBOX;
	self->svflags &= ~SVF_NOCLIENT;
	self->s.effects |= EF_ROCKET;
	self->use = NULL;
	self->movetype = MOVETYPE_TOSS;
	self->prethink = misc_viper_bomb_prethink;
	self->touch = misc_viper_bomb_touch;
	self->activator = activator;

	viper = G_Find( NULL, FOFS( classname ), "misc_viper" );
	self->velocity = viper->moveinfo.dir * viper->moveinfo.speed;

	self->timestamp = level.time;
	self->moveinfo.dir = viper->moveinfo.dir;
}

void Game::SP_misc_viper_bomb( Entity * self ) {

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->mins = Vec3( -8, -8, -8 );
	self->maxs = Vec3( 8, 8, 8 );

	self->s.modelindex = Server::SV_ModelIndex( "models/objects/bomb/tris.md2" );

	if( !self->dmg )
		self->dmg = 1000;

	self->use = misc_viper_bomb_use;
	self->svflags |= SVF_NOCLIENT;

	Server::SV_LinkEdict( self );
}


/* QUAKED misc_strogg_ship( 1 .5 0 )( -16 -16 0 )( 16 16 32 )

This is a Storgg ship for the flybys.

It is trigger_spawned, so you must have something use it for it to show up.

There must be a path for it to follow once it is activated.



"speed"		How fast it should fly

*/
void Game::misc_strogg_ship_use( Entity * self, Entity * other, Entity * activator ) {

	self->svflags &= ~SVF_NOCLIENT;
	self->use = train_use;
	train_use( self, other, activator );
}

void Game::SP_misc_strogg_ship( Entity * ent ) {

	if( !ent->target )
	{
		Common::Com_DPrintf( "%s without a target at %s\n", ent->classname.c_str( ), vtos( ent->absmin ).c_str( ) );
		G_FreeEdict( ent );
		return;
	}

	if( !ent->speed )
		ent->speed = 300;

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = Server::SV_ModelIndex( "models/ships/strogg1/tris.md2" );
	ent->mins = Vec3( -16, -16, 0 );
	ent->maxs = Vec3( 16, 16, 32 );

	ent->think = func_train_find;
	ent->nextthink = level.time + FRAMETIME;
	ent->use = misc_strogg_ship_use;
	ent->svflags |= SVF_NOCLIENT;
	ent->moveinfo.accel = ent->moveinfo.decel = ent->moveinfo.speed = ent->speed;

	Server::SV_LinkEdict( ent );
}


/* QUAKED misc_satellite_dish( 1 .5 0 )( -64 -64 0 )( 64 64 128 )

*/
void Game::misc_satellite_dish_think( Entity * self ) {

	self->s.frame++;
	if( self->s.frame < 38 )
		self->nextthink = level.time + FRAMETIME;
}

void Game::misc_satellite_dish_use( Entity * self, Entity * other, Entity * activator ) {

	self->s.frame = 0;
	self->think = misc_satellite_dish_think;
	self->nextthink = level.time + FRAMETIME;
}

void Game::SP_misc_satellite_dish( Entity * ent ) {

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->mins = Vec3( -64, -64, 0 );
	ent->maxs = Vec3( 64, 64, 128 );
	ent->s.modelindex = Server::SV_ModelIndex( "models/objects/satellite/tris.md2" );
	ent->use = misc_satellite_dish_use;
	Server::SV_LinkEdict( ent );
}


/* QUAKED light_mine1( 0 1 0 )( -2 -2 -12 )( 2 2 12 )

*/
void Game::SP_light_mine1( Entity * ent ) {

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.modelindex = Server::SV_ModelIndex( "models/objects/minelite/light1/tris.md2" );
	Server::SV_LinkEdict( ent );
}


/* QUAKED light_mine2( 0 1 0 )( -2 -2 -12 )( 2 2 12 )

*/
void Game::SP_light_mine2( Entity * ent ) {

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.modelindex = Server::SV_ModelIndex( "models/objects/minelite/light2/tris.md2" );
	Server::SV_LinkEdict( ent );
}


/* QUAKED misc_gib_arm( 1 0 0 )( -8 -8 -8 )( 8 8 8 )

Intended for use with the target_spawner

*/
void Game::SP_misc_gib_arm( Entity * ent ) {

	Server::PF_setmodel( ent, "models/objects/gibs/arm/tris.md2" );
	ent->solid = SOLID_NOT;
	ent->s.effects |= EF_GIB;
	ent->takedamage = DAMAGE_YES;
	ent->die = gib_die;
	ent->movetype = MOVETYPE_TOSS;
	ent->svflags |= SVF_MONSTER;
	ent->deadflag = DEAD_DEAD;
	ent->avelocity[ 0 ] = random( )* 200;
	ent->avelocity[ 1 ] = random( )* 200;
	ent->avelocity[ 2 ] = random( )* 200;
	ent->think = G_FreeEdict;
	ent->nextthink = level.time + 30;
	Server::SV_LinkEdict( ent );
}

/* QUAKED misc_gib_leg( 1 0 0 )( -8 -8 -8 )( 8 8 8 )

Intended for use with the target_spawner

*/
void Game::SP_misc_gib_leg( Entity * ent ) {

	Server::PF_setmodel( ent, "models/objects/gibs/leg/tris.md2" );
	ent->solid = SOLID_NOT;
	ent->s.effects |= EF_GIB;
	ent->takedamage = DAMAGE_YES;
	ent->die = gib_die;
	ent->movetype = MOVETYPE_TOSS;
	ent->svflags |= SVF_MONSTER;
	ent->deadflag = DEAD_DEAD;
	ent->avelocity[ 0 ] = random( )* 200;
	ent->avelocity[ 1 ] = random( )* 200;
	ent->avelocity[ 2 ] = random( )* 200;
	ent->think = G_FreeEdict;
	ent->nextthink = level.time + 30;
	Server::SV_LinkEdict( ent );
}

/* QUAKED misc_gib_head( 1 0 0 )( -8 -8 -8 )( 8 8 8 )

Intended for use with the target_spawner

*/
void Game::SP_misc_gib_head( Entity * ent ) {

	Server::PF_setmodel( ent, "models/objects/gibs/head/tris.md2" );
	ent->solid = SOLID_NOT;
	ent->s.effects |= EF_GIB;
	ent->takedamage = DAMAGE_YES;
	ent->die = gib_die;
	ent->movetype = MOVETYPE_TOSS;
	ent->svflags |= SVF_MONSTER;
	ent->deadflag = DEAD_DEAD;
	ent->avelocity[ 0 ] = random( )* 200;
	ent->avelocity[ 1 ] = random( )* 200;
	ent->avelocity[ 2 ] = random( )* 200;
	ent->think = G_FreeEdict;
	ent->nextthink = level.time + 30;
	Server::SV_LinkEdict( ent );
}

//=====================================================

/* QUAKED target_character( 0 0 1 ) ?

used with target_string( must be on same "team" )

"count" is position in the string( starts at 1 )

*/
void Game::SP_target_character( Entity * self ) {

	self->movetype = MOVETYPE_PUSH;
	Server::PF_setmodel( self, self->model );
	self->solid = SOLID_BSP;
	self->s.frame = 12;
	Server::SV_LinkEdict( self );
	return;
}


/* QUAKED target_string( 0 0 1 )( -8 -8 -8 )( 8 8 8 )

*/
void Game::target_string_use( Entity * self, Entity * other, Entity * activator ) {

	Entity * e;
	int n, l;
	char c;

	l = ( int )strlen( self->message );
	for( e = self->teammaster; e; e = e->teamchain )
	{
		if( !e->count )
			continue;
		n = e->count - 1;
		if( n > l )
		{
			e->s.frame = 12;
			continue;
		}

		c = self->message[ n ];
		if( c >= '0' && c <= '9' )
			e->s.frame = c - '0';
		else if( c == '-' )
			e->s.frame = 10;
		else if( c == ':' )
			e->s.frame = 11;
		else
			e->s.frame = 12;
	}
}

void Game::SP_target_string( Entity * self ) {

	if( !self->message )
		self->message = "";
	self->use = target_string_use;
}


/* QUAKED func_clock( 0 0 1 )( -8 -8 -8 )( 8 8 8 ) TIMER_UP TIMER_DOWN START_OFF MULTI_USE

target a target_string with this



The default is to be a time of day clock



TIMER_UP and TIMER_DOWN run for "count" seconds and the fire "pathtarget"

If START_OFF, this entity must be used before it starts



"style"		0 "xx"

1 "xx:xx"

2 "xx:xx:xx"

*/
// don't let field width of any clock messages change, or it
// could cause an overwrite after a game load

void Game::func_clock_reset( Entity * self ) {

	self->activator = NULL;
	if( self->spawnflags & 1 )
	{
		self->health = 0;
		self->wait = ( timeTypeReal )self->count;
	}
	else if( self->spawnflags & 2 )
	{
		self->health = self->count;
		self->wait = 0;
	}
}

void Game::func_clock_format_countdown( Entity * self ) {

	if( self->style == 0 )
	{
		sprintf( self->message, "%2i", self->health );
		return;
	}

	if( self->style == 1 )
	{
		sprintf( self->message, "%2i:%2i", self->health / 60, self->health % 60 );
		if( self->message[ 3 ] == ' ' )
			self->message[ 3 ] = '0';
		return;
	}

	if( self->style == 2 )
	{
		sprintf( self->message, "%2i:%2i:%2i", self->health / 3600, ( self->health -( self->health / 3600 ) * 3600 ) / 60, self->health % 60 );
		if( self->message[ 3 ] == ' ' )
			self->message[ 3 ] = '0';
		if( self->message[ 6 ] == ' ' )
			self->message[ 6 ] = '0';
		return;
	}
}

void Game::func_clock_think( Entity * self ) {

	if( !self->enemy )
	{
		self->enemy = G_Find( NULL, FOFS( targetname ), self->target );
		if( !self->enemy )
			return;
	}

	if( self->spawnflags & 1 )
	{
		func_clock_format_countdown( self );
		self->health++;
	}
	else if( self->spawnflags & 2 )
	{
		func_clock_format_countdown( self );
		self->health--;
	}
	else
	{
		struct tm * ltime;
		time_t gmtime;

		time( &gmtime );
		ltime = localtime( &gmtime );
		sprintf( self->message, "%2i:%2i:%2i", ltime->tm_hour, ltime->tm_min, ltime->tm_sec );
		if( self->message[ 3 ] == ' ' )
			self->message[ 3 ] = '0';
		if( self->message[ 6 ] == ' ' )
			self->message[ 6 ] = '0';
	}

	self->enemy->message = self->message;
	self->enemy->use( self->enemy, self, self );

	if( ( ( self->spawnflags & 1 ) &&( self->health > self->wait ) ) ||
		( ( self->spawnflags & 2 ) &&( self->health < self->wait ) ) )
	{
		if( self->pathtarget )
		{
			Str savetarget;
			Str savemessage;

			savetarget = self->target;
			savemessage = self->message;
			self->target = self->pathtarget;
			self->message = NULL;
			G_UseTargets( self, self->activator );
			self->target = savetarget;
			self->message = savemessage;
		}

		if( !( self->spawnflags & 8 ) ) 	return;

		func_clock_reset( self );

		if( self->spawnflags & 4 )
			return;
	}

	self->nextthink = level.time + 1;
}

void Game::func_clock_use( Entity * self, Entity * other, Entity * activator ) {

	if( !( self->spawnflags & 8 ) ) self->use = NULL;
	if( self->activator )
		return;
	self->activator = activator;
	self->think( self );
}

void Game::SP_func_clock( Entity * self ) {

	if( !self->target )
	{
		Common::Com_DPrintf( "%s with no target at %s\n", self->classname.c_str( ), vtos( self->s.origin ).c_str( ) );
		G_FreeEdict( self );
		return;
	}

	if( ( self->spawnflags & 2 ) &&( !self->count ) ) {
		Common::Com_DPrintf( "%s with no count at %s\n", self->classname.c_str( ), vtos( self->s.origin ).c_str( ) );
		G_FreeEdict( self );
		return;
	}

	if( ( self->spawnflags & 1 ) &&( !self->count ) ) self->count = 60* 60;;

	func_clock_reset( self );

	//self->message =( char * )Z_TagMalloc( 16, TAG_LEVEL );

	self->think = func_clock_think;

	if( self->spawnflags & 4 )
		self->use = func_clock_use;
	else
		self->nextthink = level.time + 1;
}

//=================================================================================

void Game::teleporter_touch( Entity * self, Entity * other, Plane & plane, int surf ) {

	Entity * dest;
	int i;

	if( !other->client )
		return;
	dest = G_Find( NULL, FOFS( targetname ), self->target );
	if( !dest )
	{
		Common::Com_DPrintf( "Couldn't find destination\n" );
		return;
	}

	// unlink to make sure it can't possibly interfere with KillBox
	Server::SV_UnlinkEdict( other );

	other->s.origin = dest->s.origin;
	other->s.old_origin = dest->s.origin;
	other->s.origin[ 2 ] += 10;

	// clear the velocity and hold them in place briefly
	other->velocity = vec3_origin;
	other->client->ps.pmove.pm_time = 160>>3; // hold time
	other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	// draw the teleport splash at source and on the player
	self->owner->s.eventNum = EV_PLAYER_TELEPORT;
	other->s.eventNum = EV_PLAYER_TELEPORT;

	// set angles
	for( i = 0; i<3; i++ )
	{
		other->client->ps.pmove.delta_angles[ i ] = ANGLE2SHORT( dest->s.angles[ i ] - other->client->resp.cmd_angles[ i ] );
	}

	other->s.angles = vec3_origin;
	other->client->ps.viewangles = vec3_origin;
	other->client->v_angle = vec3_origin;

	// kill anything at the destination
	KillBox( other );

	Server::SV_LinkEdict( other );
}

/* QUAKED misc_teleporter( 1 0 0 )( -32 -32 -24 )( 32 32 -16 )

Stepping onto this disc will teleport players to the targeted misc_teleporter_dest object.

*/
void Game::SP_misc_teleporter( Entity * ent ) {

	Entity * trig;

	if( !ent->target )
	{
		Common::Com_DPrintf( "teleporter without a target.\n" );
		G_FreeEdict( ent );
		return;
	}

	Server::PF_setmodel( ent, "models/objects/dmspot/tris.md2" );
	ent->s.skinnum = 1;
	ent->s.effects = EF_TELEPORTER;
	ent->s.sound = Server::SV_SoundIndex( "world/amb10.wav" );
	ent->solid = SOLID_BBOX;

	ent->mins = Vec3( -32, -32, -24 );
	ent->maxs = Vec3( 32, 32, -16 );
	Server::SV_LinkEdict( ent );

	trig = G_Spawn( );
	trig->touch = teleporter_touch;
	trig->solid = SOLID_TRIGGER;
	trig->target = ent->target;
	trig->owner = ent;
	trig->s.origin = ent->s.origin;
	trig->mins = Vec3( -8, -8, 8 );
	trig->maxs = Vec3( 8, 8, 24 );
	Server::SV_LinkEdict( trig );

}

/* QUAKED misc_teleporter_dest( 1 0 0 )( -32 -32 -24 )( 32 32 -16 )

Point teleporters at these.

*/
void Game::SP_misc_teleporter_dest( Entity * ent ) {

	Server::PF_setmodel( ent, "models/objects/dmspot/tris.md2" );
	ent->s.skinnum = 0;
	ent->solid = SOLID_BBOX;
	//	ent->s.effects |= EF_FLIES;
	ent->mins = Vec3( -32, -32, -24 );
	ent->maxs = Vec3( 32, 32, -16 );
	Server::SV_LinkEdict( ent );
}
