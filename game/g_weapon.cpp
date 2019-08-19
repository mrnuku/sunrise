#include "precompiled.h"
#pragma hdrstop

/*

=================

check_dodge



This is a support routine used when a client is firing

a non-instant attack weapon.  It checks to see if a

monster's dodge function should be called.

=================

*/
void Game::check_dodge( Entity * self, Vec3 & start, Vec3 & dir, int speed ) {

	Vec3 end;
	Vec3 v;
	CTrace tr;
	float eta;

	// easy mode only ducks one quarter the time
	if( !skill.GetBool( ) &&( random( ) > 0.25f ) ) return;

	end = start.Magnitude( 8192.0f, dir );
	tr = Server::SV_Trace( start, vec3_origin, vec3_origin, end, self, MASK_SHOT );
	if( ( tr.ent ) &&( tr.ent->svflags & SVF_MONSTER ) &&( tr.ent->health > 0 ) &&( tr.ent->monsterinfo.dodge ) && infront( tr.ent, self ) ) {

		v = tr.endpos - start;
		eta =( v.Length( ) - tr.ent->maxs[ 0 ] ) / speed;
		tr.ent->monsterinfo.dodge( tr.ent, self, eta );
	}
}


/*

=================

fire_hit



Used for all impact( hit/punch/slash ) attacks

=================

*/
bool Game::fire_hit( Entity * self, Vec3 & aim, int damage, int kick ) {

	CTrace tr;
	Vec3 forward, right, up;
	Vec3 v;
	Vec3 point;
	float range;
	Vec3 dir;

	//see if enemy is in range
	dir = self->enemy->s.origin - self->s.origin;
	range = dir.Length( );
	if( range > aim[ 0 ] ) return false;

	if( aim[ 1 ] > self->mins[ 0 ] && aim[ 1 ] < self->maxs[ 0 ] ) {

		// the hit is straight on so back the range up to the edge of their bbox
		range -= self->enemy->maxs[ 0 ];
	} else {
		// this is a side hit so adjust the "right" value out to the edge of their bbox
		if( aim[ 1 ] < 0 ) aim[ 1 ] = self->enemy->mins[ 0 ];
		else aim[ 1 ] = self->enemy->maxs[ 0 ];
	}

	point = self->s.origin.Magnitude( range, dir );

	tr = Server::SV_Trace( self->s.origin, vec3_origin, vec3_origin, point, self, MASK_SHOT );
	if( tr.fraction < 1 ) {

		if( !tr.ent->takedamage ) return false;
		// if it will hit any client/monster then hit the one we wanted to hit
		if( ( tr.ent->svflags & SVF_MONSTER ) ||( tr.ent->client ) ) tr.ent = self->enemy;
	}

	self->s.angles.AngleVectors( &forward, &right, &up );
	point = self->s.origin.Magnitude( range, forward );
	point = point.Magnitude( aim[ 1 ], right );
	point = point.Magnitude( aim[ 2 ], up );
	dir = point - self->enemy->s.origin;

	// do the damage
	T_Damage( tr.ent, self, self, dir, point, vec3_origin, damage, kick/2, DAMAGE_NO_KNOCKBACK, MOD_HIT );

	if( !( tr.ent->svflags & SVF_MONSTER ) &&( !tr.ent->client ) ) return false;

	// do our special form of knockback here
	v = self->enemy->absmin.Magnitude( 0.5f, self->enemy->size );
	v -= point;
	v.Normalize( );
	self->enemy->velocity = self->enemy->velocity.Magnitude( ( float )kick, v );
	if( self->enemy->velocity[ 2 ] > 0 )
		self->enemy->groundentity = NULL;
	return true;
}


/*

=================

fire_lead



This is an internal support routine used for bullet/pellet based weapons.

=================

*/
void Game::fire_lead( Entity * self, Vec3 & start, Vec3 & aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod ) {

	CTrace tr;
	Vec3 dir;
	Vec3 forward, right, up;
	Vec3 end;
	float r;
	float u;
	Vec3 water_start;
	bool water = false;
	int content_mask = MASK_SHOT | MASK_WATER;

	tr = Server::SV_Trace( self->s.origin, vec3_origin, vec3_origin, start, self, MASK_SHOT );
	if( !( tr.fraction < 1.0f ) ) {
		vectoangles( aimdir, &dir );
		dir.AngleVectors( &forward, &right, &up );

		r = crandom( )* hspread;
		u = crandom( )* vspread;
		end = start.Magnitude( 8192.0f, forward );
		end = end.Magnitude( r, right );
		end = end.Magnitude( u, up );

		if( Server::SV_PointContents( start ) & MASK_WATER ) {

			water = true;
			water_start = start;
			content_mask &= ~MASK_WATER;
		}

		tr = Server::SV_Trace( start, vec3_origin, vec3_origin, end, self, content_mask );

		// see if we hit water
		if( tr.contents & MASK_WATER ) {

			int color;

			water = true;
			water_start = tr.endpos;

			if( start != tr.endpos ) {

				if( tr.contents & CONTENTS_WATER ) {

#if 0
					if( strcmp( tr.surface->name, "*brwater" ) == 0 ) color = SPLASH_BROWN_WATER;
					else 
#endif
					color = SPLASH_BLUE_WATER;
				}
				else if( tr.contents & CONTENTS_SLIME ) color = SPLASH_SLIME;
				else if( tr.contents & CONTENTS_LAVA ) color = SPLASH_LAVA;
				else color = SPLASH_UNKNOWN;

				if( color != SPLASH_UNKNOWN ) {

					Server::PF_WriteByte( svc_temp_entity );
					Server::PF_WriteByte( TE_SPLASH );
					Server::PF_WriteByte( 8 );
					Server::PF_WritePos( tr.endpos );
					Server::PF_WriteDir( tr.plane.Normal( ) );
					Server::PF_WriteByte( color );
					Server::SV_Multicast( tr.endpos, MULTICAST_PVS );
				}

				// change bullet's course when it enters water
				dir = end - start;
				vectoangles( dir, &dir );
				dir.AngleVectors( &forward, &right, &up );
				r = crandom( )* hspread* 2;
				u = crandom( )* vspread* 2;
				end = water_start.Magnitude( 8192, forward );
				end = end.Magnitude( r, right );
				end = end.Magnitude( u, up );
			}

			// re-trace ignoring water this time
			tr = Server::SV_Trace( water_start, vec3_origin, vec3_origin, end, self, MASK_SHOT );
		}
	}

#if 0
	// send gun puff / flash
	if( !( ( tr.surface ) &&( tr.surface->flags & SURF_SKY ) ) )
	{
		if( tr.fraction < 1.0f )
		{
			if( tr.ent->takedamage )
			{
				T_Damage( tr.ent, self, self, aimdir, tr.endpos, tr.plane.Normal( ), damage, kick, DAMAGE_BULLET, mod );
			}
			else
			{
				if( Str::Cmpn( tr.surface->name, "sky", 3 ) != 0 )
				{
					Server::PF_WriteByte( svc_temp_entity );
					Server::PF_WriteByte( te_impact );
					Server::PF_WritePos( tr.endpos );
					Server::PF_WriteDir( tr.plane.Normal( ) );
					Server::SV_Multicast( tr.endpos, MULTICAST_PVS );

					if( self->client )
						PlayerNoise( self, tr.endpos, PNOISE_IMPACT );
				}
			}
		}
	}
#endif

	// if went through water, determine where the end and make a bubble trail
	if( water )
	{
		Vec3 pos;

		dir = tr.endpos - water_start;
		dir.Normalize( );
		pos = tr.endpos.Magnitude( -2.0f, dir );
		if( Server::SV_PointContents( pos ) & MASK_WATER )
			tr.endpos = pos;
		else
			tr = Server::SV_Trace( pos, vec3_origin, vec3_origin, water_start, tr.ent, MASK_WATER );

		pos = water_start + tr.endpos;
		pos *= 0.5f;

		Server::PF_WriteByte( svc_temp_entity );
		Server::PF_WriteByte( TE_BUBBLETRAIL );
		Server::PF_WritePos( water_start );
		Server::PF_WritePos( tr.endpos );
		Server::SV_Multicast( pos, MULTICAST_PVS );
	}
}


/*

=================

fire_bullet



Fires a single round.  Used for machinegun and chaingun.  Would be fine for

pistols, rifles, etc....

=================

*/
void Game::fire_bullet( Entity * self, Vec3 & start, Vec3 & aimdir, int damage, int kick, int hspread, int vspread, int mod ) {

	fire_lead( self, start, aimdir, damage, kick, TE_GUNSHOT, hspread, vspread, mod );
}


/*

=================

fire_shotgun



Shoots shotgun pellets.  Used by shotgun and super shotgun.

=================

*/
void Game::fire_shotgun( Entity * self, Vec3 & start, Vec3 & aimdir, int damage, int kick, int hspread, int vspread, int count, int mod ) {

	int i;

	for( i = 0; i < count; i++ )
		fire_lead( self, start, aimdir, damage, kick, TE_SHOTGUN, hspread, vspread, mod );
}


/*

=================

fire_blaster



Fires a single blaster bolt.  Used by the blaster and hyper blaster.

=================

*/
void Game::blaster_touch( Entity * self, Entity * other, Plane & plane, int surf ) {

	int mod;

	if( other == self->owner )
		return;

	if( surf &&( surf & SURF_SKY ) ) {
		G_FreeEdict( self );
		return;
	}

	if( self->owner->client )
		PlayerNoise( self->owner, self->s.origin, PNOISE_IMPACT );

	if( other->takedamage )
	{
		if( self->spawnflags & 1 )
			mod = MOD_HYPERBLASTER;
		else
			mod = MOD_BLASTER;
		T_Damage( other, self, self->owner, self->velocity, self->s.origin, plane.Normal( ), self->dmg, 1, DAMAGE_ENERGY, mod );
	}
	else
	{
		Server::PF_WriteByte( svc_temp_entity );
		Server::PF_WriteByte( TE_BLASTER );
		Server::PF_WritePos( self->s.origin );
		Server::PF_WriteDir( plane.Normal( ) );
		Server::SV_Multicast( self->s.origin, MULTICAST_PVS );
	}

	G_FreeEdict( self );
}

void Game::fire_blaster( Entity * self, Vec3 & start, Vec3 & dir, int damage, int speed, int effect, bool vhyper ) {

	Entity * bolt;
	CTrace tr;

	dir.Normalize( );

	bolt = G_Spawn( );
	bolt->svflags = SVF_DEADMONSTER;
	// yes, I know it looks weird that projectiles are deadmonsters
	// what this means is that when prediction is used against the object
	//( blaster/hyperblaster shots ), the player won't be solid clipped against
	// the object.  Right now trying to run into a firing hyperblaster
	// is very jerky since you are predicted 'against' the shots.
	bolt->s.origin = start;
	bolt->s.old_origin = start;
	vectoangles( dir, &bolt->s.angles );
	bolt->velocity = dir * ( float )speed;
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= effect;
	bolt->mins = vec3_origin;
	bolt->maxs = vec3_origin;
	bolt->s.modelindex = Server::SV_ModelIndex( "models/objects/laser/tris.md2" );
	bolt->s.sound = Server::SV_SoundIndex( "misc/lasfly.wav" );
	bolt->owner = self;
	bolt->touch = blaster_touch;
	bolt->nextthink = level.time + 2;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "bolt";
	if( vhyper )
		bolt->spawnflags = 1;
	Server::SV_LinkEdict( bolt );

	if( self->client )
		check_dodge( self, bolt->s.origin, dir, speed );

	tr = Server::SV_Trace( self->s.origin, vec3_origin, vec3_origin, bolt->s.origin, bolt, MASK_SHOT );
	if( tr.fraction < 1.0f )
	{
		bolt->s.origin = bolt->s.origin.Magnitude( -10.0f, dir );
		bolt->touch( bolt, tr.ent, plane_origin, SURF_NULL );
	}
}


/*

=================

fire_grenade

=================

*/
void Game::Grenade_Explode( Entity * ent ) {

	Vec3 origin;
	int mod;

	if( ent->owner->client )
		PlayerNoise( ent->owner, ent->s.origin, PNOISE_IMPACT );

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if( ent->enemy )
	{
		float points;
		Vec3 v;
		Vec3 dir;

		v = ent->enemy->mins + ent->enemy->maxs;
		v = ent->enemy->s.origin.Magnitude( 0.5f, v );
		v = ent->s.origin - v;
		points = ent->dmg - 0.5f * v.Length( );
		dir = ent->enemy->s.origin - ent->s.origin;
		if( ent->spawnflags & 1 )
			mod = MOD_HANDGRENADE;
		else
			mod = MOD_GRENADE;
		T_Damage( ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, ( int )points, ( int )points, DAMAGE_RADIUS, mod );
	}

	if( ent->spawnflags & 2 )
		mod = MOD_HELD_GRENADE;
	else if( ent->spawnflags & 1 )
		mod = MOD_HG_SPLASH;
	else
		mod = MOD_G_SPLASH;
	T_RadiusDamage( ent, ent->owner, ( float )ent->dmg, ent->enemy, ent->dmg_radius, mod );

	origin = ent->s.origin.Magnitude( -0.02f, ent->velocity );
	Server::PF_WriteByte( svc_temp_entity );
	if( ent->waterlevel )
	{
		if( ent->groundentity )
			Server::PF_WriteByte( TE_GRENADE_EXPLOSION_WATER );
		else
			Server::PF_WriteByte( TE_ROCKET_EXPLOSION_WATER );
	}
	else
	{
		if( ent->groundentity )
			Server::PF_WriteByte( TE_GRENADE_EXPLOSION );
		else
			Server::PF_WriteByte( TE_ROCKET_EXPLOSION );
	}
	Server::PF_WritePos( origin );
	Server::SV_Multicast( ent->s.origin, MULTICAST_PHS );

	G_FreeEdict( ent );
}

void Game::Grenade_Touch( Entity * ent, Entity * other, Plane & plane, int surf ) {

	if( other == ent->owner )
		return;

	if( surf & SURF_SKY ) {

		G_FreeEdict( ent );
		return;
	}

	if( !other->takedamage ) {

		if( ent->spawnflags & 1 ) {

			if( random( ) > 0.5f )
				Server::PF_StartSound( ent, CHAN_VOICE, Server::SV_SoundIndex( "weapons/hgrenb1a.wav" ), 1, ATTN_NORM, 0 );
			else
				Server::PF_StartSound( ent, CHAN_VOICE, Server::SV_SoundIndex( "weapons/hgrenb2a.wav" ), 1, ATTN_NORM, 0 );
		}
		else
		{
			Server::PF_StartSound( ent, CHAN_VOICE, Server::SV_SoundIndex( "weapons/grenlb1b.wav" ), 1, ATTN_NORM, 0 );
		}
		return;
	}

	ent->enemy = other;
	Grenade_Explode( ent );
}

void Game::fire_grenade( Entity * self, Vec3 & start, Vec3 & aimdir, int damage, int speed, float timer, float damage_radius ) {

	Entity * grenade;
	Vec3 dir;
	Vec3 forward, right, up;

	vectoangles( aimdir, &dir );
	dir.AngleVectors( &forward, &right, &up );

	grenade = G_Spawn( );
	grenade->s.origin = start;
	grenade->velocity = aimdir * ( float )speed;
	grenade->velocity = grenade->velocity.Magnitude( 200.0f + crandom( ) * 10.0f, up );
	grenade->velocity = grenade->velocity.Magnitude( crandom( ) * 10.0f, right );
	grenade->avelocity = Vec3( 300, 300, 300 );
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	grenade->mins = vec3_origin;
	grenade->maxs = vec3_origin;
	grenade->s.modelindex = Server::SV_ModelIndex( "models/objects/grenade/tris.md2" );
	grenade->owner = self;
	grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";

	Server::SV_LinkEdict( grenade );
}

void Game::fire_grenade2( Entity * self, Vec3 & start, Vec3 & aimdir, int damage, int speed, float timer, float damage_radius, bool held ) {

	Entity * grenade;
	Vec3 dir;
	Vec3 forward, right, up;

	vectoangles( aimdir, &dir );
	dir.AngleVectors( &forward, &right, &up );

	grenade = G_Spawn( );
	grenade->s.origin = start;
	grenade->velocity = aimdir * ( float )speed;
	grenade->velocity = grenade->velocity.Magnitude( 200.0f + crandom( ) * 10.0f, up );
	grenade->velocity = grenade->velocity.Magnitude( crandom( ) * 10.0f, right );
	grenade->avelocity = Vec3( 300, 300, 300 );
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	grenade->mins = vec3_origin;
	grenade->maxs = vec3_origin;
	grenade->s.modelindex = Server::SV_ModelIndex( "models/objects/grenade2/tris.md2" );
	grenade->owner = self;
	grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "hgrenade";
	if( held )
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;
	grenade->s.sound = Server::SV_SoundIndex( "weapons/hgrenc1b.wav" );

	if( timer <= 0.0f )
		Grenade_Explode( grenade );
	else
	{
		Server::PF_StartSound( self, CHAN_WEAPON, Server::SV_SoundIndex( "weapons/hgrent1a.wav" ), 1, ATTN_NORM, 0 );
		Server::SV_LinkEdict( grenade );
	}
}


/*

=================

fire_rocket

=================

*/
void Game::rocket_touch( Entity * ent, Entity * other, Plane & plane, int surf ) {

	Vec3 origin;
	int n;

	if( other == ent->owner )
		return;

	if( surf & SURF_SKY ) {

		G_FreeEdict( ent );
		return;
	}

	if( ent->owner->client )
		PlayerNoise( ent->owner, ent->s.origin, PNOISE_IMPACT );

	// calculate position for the explosion entity
	origin = ent->s.origin.Magnitude( -0.02f, ent->velocity );

	if( other->takedamage )
	{
		T_Damage( other, ent, ent->owner, ent->velocity, ent->s.origin, plane.Normal( ), ent->dmg, 0, 0, MOD_ROCKET );
	}
	else
	{
		// don't throw any debris in net games
		if( !deathmatch.GetBool( ) && !coop.GetBool( ) )
		{
			if( surf )
			{
				n = rand( ) % 5;
				while( n-- )
					ThrowDebris( ent, "models/objects/debris2/tris.md2", 2, ent->s.origin );
			}
		}
	}

	T_RadiusDamage( ent, ent->owner, ( float )ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH );

	Server::PF_WriteByte( svc_temp_entity );
	if( ent->waterlevel )
		Server::PF_WriteByte( TE_ROCKET_EXPLOSION_WATER );
	else
		Server::PF_WriteByte( TE_ROCKET_EXPLOSION );
	Server::PF_WritePos( origin );
	Server::SV_Multicast( ent->s.origin, MULTICAST_PHS );

	G_FreeEdict( ent );
}

void Game::fire_rocket( Entity * self, Vec3 & start, Vec3 & dir, int damage, int speed, float damage_radius, int radius_damage ) {

	Entity * rocket;

	rocket = G_Spawn( );
	rocket->s.origin = start;
	rocket->movedir = dir;
	vectoangles( dir, &rocket->s.angles );
	rocket->velocity = dir * ( float )speed;
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->s.effects |= EF_ROCKET;
	rocket->mins = vec3_origin;
	rocket->maxs = vec3_origin;
	rocket->s.modelindex = Server::SV_ModelIndex( "models/objects/rocket/tris.md2" );
	rocket->owner = self;
	rocket->touch = rocket_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = Server::SV_SoundIndex( "weapons/rockfly.wav" );
	rocket->classname = "rocket";

	if( self->client )
		check_dodge( self, rocket->s.origin, dir, speed );

	Server::SV_LinkEdict( rocket );
}


/*

=================

fire_rail

=================

*/
void Game::fire_rail( Entity * self, Vec3 & start, Vec3 & aimdir, int damage, int kick ) {

	Vec3 from;
	Vec3 end;
	CTrace tr;
	Entity * ignore;
	int mask;
	bool water;

	end = start.Magnitude( 8192.0f, aimdir );
	from = start;
	ignore = self;
	water = false;
	mask = MASK_SHOT | CONTENTS_SLIME | CONTENTS_LAVA;

	while( ignore )
	{
		tr = Server::SV_Trace( from, vec3_origin, vec3_origin, end, ignore, mask );

		if( tr.contents &( CONTENTS_SLIME|CONTENTS_LAVA ) )
		{
			mask &= ~( CONTENTS_SLIME|CONTENTS_LAVA );
			water = true;
		}
		else
		{
			//ZOID--added so rail goes through SOLID_BBOX entities( gibs, etc )
			if( ( tr.ent->svflags & SVF_MONSTER ) ||( tr.ent->client ) ||
				( tr.ent->solid == SOLID_BBOX ) )
				ignore = tr.ent;
			else
				ignore = NULL;

			if( ( tr.ent != self ) &&( tr.ent->takedamage ) ) 		T_Damage( tr.ent, self, self, aimdir, tr.endpos, tr.plane.Normal( ), damage, kick, 0, MOD_RAILGUN );
		}

		from = tr.endpos;
	}

	// send gun puff / flash
	Server::PF_WriteByte( svc_temp_entity );
	Server::PF_WriteByte( TE_RAILTRAIL );
	Server::PF_WritePos( start );
	Server::PF_WritePos( tr.endpos );
	Server::SV_Multicast( self->s.origin, MULTICAST_PHS );
	//	Server::SV_Multicast( start, MULTICAST_PHS );
	if( water )
	{
		Server::PF_WriteByte( svc_temp_entity );
		Server::PF_WriteByte( TE_RAILTRAIL );
		Server::PF_WritePos( start );
		Server::PF_WritePos( tr.endpos );
		Server::SV_Multicast( tr.endpos, MULTICAST_PHS );
	}

	if( self->client )
		PlayerNoise( self, tr.endpos, PNOISE_IMPACT );
}


/*

=================

fire_bfg

=================

*/
void Game::bfg_explode( Entity * self ) {

	Entity * ent;
	float points;
	Vec3 v;
	float dist;

	if( self->s.frame == 0 )
	{
		// the BFG effect
		ent = NULL;
		while( ( ent = findradius( ent, self->s.origin, self->dmg_radius ) ) != NULL )
		{
			if( !ent->takedamage )
				continue;
			if( ent == self->owner )
				continue;
			if( !CanDamage( ent, self ) ) 		continue;
			if( !CanDamage( ent, self->owner ) ) 		continue;

			v = ent->mins + ent->maxs;
			v = ent->s.origin.Magnitude( 0.5f, v );
			v = self->s.origin - v;
			dist = v.Length( );
			points = self->radius_dmg * ( 1.0f - sqrt( dist / self->dmg_radius ) );
			if( ent == self->owner ) points = points * 0.5f;

			Server::PF_WriteByte( svc_temp_entity );
			Server::PF_WriteByte( TE_BFG_EXPLOSION );
			Server::PF_WritePos( ent->s.origin );
			Server::SV_Multicast( ent->s.origin, MULTICAST_PHS );
			T_Damage( ent, self, self->owner, self->velocity, ent->s.origin, vec3_origin, ( int )points, 0, DAMAGE_ENERGY, MOD_BFG_EFFECT );
		}
	}

	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
	if( self->s.frame == 5 )
		self->think = G_FreeEdict;
}

void Game::bfg_touch( Entity * self, Entity * other, Plane & plane, int surf ) {

	if( other == self->owner )
		return;

	if( surf & SURF_SKY ) {

		G_FreeEdict( self );
		return;
	}

	if( self->owner->client )
		PlayerNoise( self->owner, self->s.origin, PNOISE_IMPACT );

	// core explosion - prevents firing it into the wall/floor
	if( other->takedamage )
		T_Damage( other, self, self->owner, self->velocity, self->s.origin, plane.Normal( ), 200, 0, 0, MOD_BFG_BLAST );
	T_RadiusDamage( self, self->owner, 200, other, 100, MOD_BFG_BLAST );

	Server::PF_StartSound( self, CHAN_VOICE, Server::SV_SoundIndex( "weapons/bfg__x1b.wav" ), 1, ATTN_NORM, 0 );
	self->solid = SOLID_NOT;
	self->touch = NULL;
	self->s.origin = self->s.origin.Magnitude( -1.0f * FRAMETIME, self->velocity );
	self->velocity = vec3_origin;
	self->s.modelindex = Server::SV_ModelIndex( "sprites/s_bfg3.sp2" );
	self->s.frame = 0;
	self->s.sound = 0;
	self->s.effects &= ~EF_ANIM_ALLFAST;
	self->think = bfg_explode;
	self->nextthink = level.time + FRAMETIME;
	self->enemy = other;

	Server::PF_WriteByte( svc_temp_entity );
	Server::PF_WriteByte( TE_BFG_BIGEXPLOSION );
	Server::PF_WritePos( self->s.origin );
	Server::SV_Multicast( self->s.origin, MULTICAST_PVS );
}


void Game::bfg_think( Entity * self ) {

	Entity * ent;
	Entity * ignore;
	Vec3 point;
	Vec3 dir;
	Vec3 start;
	Vec3 end;
	int dmg;
	CTrace tr;

	if( deathmatch.GetBool( ) )
		dmg = 5;
	else
		dmg = 10;

	ent = NULL;
	while( ( ent = findradius( ent, self->s.origin, 256 ) ) != NULL )
	{
		if( ent == self )
			continue;

		if( ent == self->owner )
			continue;

		if( !ent->takedamage )
			continue;

		if( !( ent->svflags & SVF_MONSTER ) &&( !ent->client ) &&( strcmp( ent->classname, "misc_explobox" ) != 0 ) ) 	continue;

		point = ent->absmin.Magnitude( 0.5f, ent->size );

		dir = point - self->s.origin;
		dir.Normalize( );

		ignore = self;
		start = self->s.origin;
		end = start.Magnitude( 2048.0f, dir );
		while( 1 )
		{
			tr = Server::SV_Trace( start, vec3_origin, vec3_origin, end, ignore, CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_CORPSE );

			if( !tr.ent )
				break;

			// hurt it if we can
			if( ( tr.ent->takedamage ) && !( tr.ent->flags & FL_IMMUNE_LASER ) &&( tr.ent != self->owner ) ) 		T_Damage( tr.ent, self, self->owner, dir, tr.endpos, vec3_origin, dmg, 1, DAMAGE_ENERGY, MOD_BFG_LASER );

			// if we hit something that's not a monster or player we're done
			if( !( tr.ent->svflags & SVF_MONSTER ) &&( !tr.ent->client ) )
			{
				Server::PF_WriteByte( svc_temp_entity );
				Server::PF_WriteByte( TE_LASER_SPARKS );
				Server::PF_WriteByte( 4 );
				Server::PF_WritePos( tr.endpos );
				Server::PF_WriteDir( tr.plane.Normal( ) );
				Server::PF_WriteByte( self->s.skinnum );
				Server::SV_Multicast( tr.endpos, MULTICAST_PVS );
				break;
			}

			ignore = tr.ent;
			start = tr.endpos;
		}

		Server::PF_WriteByte( svc_temp_entity );
		Server::PF_WriteByte( TE_BFG_LASER );
		Server::PF_WritePos( self->s.origin );
		Server::PF_WritePos( tr.endpos );
		Server::SV_Multicast( self->s.origin, MULTICAST_PHS );
	}

	self->nextthink = level.time + FRAMETIME;
}


void Game::fire_bfg( Entity * self, Vec3 & start, Vec3 & dir, int damage, int speed, float damage_radius ) {

	Entity * bfg;

	bfg = G_Spawn( );
	bfg->s.origin = start;
	bfg->movedir = dir;
	vectoangles( dir, &bfg->s.angles );
	bfg->velocity = dir * ( float )speed;
	bfg->movetype = MOVETYPE_FLYMISSILE;
	bfg->clipmask = MASK_SHOT;
	bfg->solid = SOLID_BBOX;
	bfg->s.effects |= EF_BFG | EF_ANIM_ALLFAST;
	bfg->mins = vec3_origin;
	bfg->maxs = vec3_origin;
	bfg->s.modelindex = Server::SV_ModelIndex( "sprites/s_bfg1.sp2" );
	bfg->owner = self;
	bfg->touch = bfg_touch;
	bfg->nextthink = level.time + 8000/speed;
	bfg->think = G_FreeEdict;
	bfg->radius_dmg = damage;
	bfg->dmg_radius = damage_radius;
	bfg->classname = "bfg blast";
	bfg->s.sound = Server::SV_SoundIndex( "weapons/bfg__l1a.wav" );

	bfg->think = bfg_think;
	bfg->nextthink = level.time + FRAMETIME;
	bfg->teammaster = bfg;
	bfg->teamchain = NULL;

	if( self->client )
		check_dodge( self, bfg->s.origin, dir, speed );

	Server::SV_LinkEdict( bfg );
}
