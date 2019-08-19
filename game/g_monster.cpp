#include "precompiled.h"
#pragma hdrstop

//
// monster weapons
//

//FIXME mosnters should call these with a totally accurate direction
// and we can mess it up based on skill.  Spread should be for normal
// and we can tighten or loosen based on skill.  We could muck with
// the damages too, but I'm not sure that's such a good ea.
void Game::monster_fire_bullet( Entity * self, Vec3 & start, Vec3 & dir, int damage, int kick, int hspread, int vspread, int flashtype ) {

	fire_bullet( self, start, dir, damage, kick, hspread, vspread, MOD_UNKNOWN );

	Server::PF_WriteByte( svc_muzzleflash2 );
	Server::PF_WriteShort( ( int )( self - g_edicts ) );
	Server::PF_WriteByte( flashtype );
	Server::SV_Multicast( start, MULTICAST_PVS );
}

void Game::monster_fire_shotgun( Entity * self, Vec3 & start, Vec3 & aimdir, int damage, int kick, int hspread, int vspread, int count, int flashtype ) {

	fire_shotgun( self, start, aimdir, damage, kick, hspread, vspread, count, MOD_UNKNOWN );

	Server::PF_WriteByte( svc_muzzleflash2 );
	Server::PF_WriteShort( ( int )( self - g_edicts ) );
	Server::PF_WriteByte( flashtype );
	Server::SV_Multicast( start, MULTICAST_PVS );
}

void Game::monster_fire_blaster( Entity * self, Vec3 & start, Vec3 & dir, int damage, int speed, int flashtype, int effect ) {

	fire_blaster( self, start, dir, damage, speed, effect, false );

	Server::PF_WriteByte( svc_muzzleflash2 );
	Server::PF_WriteShort( ( int )( self - g_edicts ) );
	Server::PF_WriteByte( flashtype );
	Server::SV_Multicast( start, MULTICAST_PVS );
}

void Game::monster_fire_grenade( Entity * self, Vec3 & start, Vec3 & aimdir, int damage, int speed, int flashtype ) {

	fire_grenade( self, start, aimdir, damage, speed, 2.5f, ( float )( damage+40 ) );

	Server::PF_WriteByte( svc_muzzleflash2 );
	Server::PF_WriteShort( ( int )( self - g_edicts ) );
	Server::PF_WriteByte( flashtype );
	Server::SV_Multicast( start, MULTICAST_PVS );
}

void Game::monster_fire_rocket( Entity * self, Vec3 & start, Vec3 & dir, int damage, int speed, int flashtype ) {

	fire_rocket( self, start, dir, damage, speed, ( float )( damage+20 ), damage );

	Server::PF_WriteByte( svc_muzzleflash2 );
	Server::PF_WriteShort( ( int )( self - g_edicts ) );
	Server::PF_WriteByte( flashtype );
	Server::SV_Multicast( start, MULTICAST_PVS );
}

void Game::monster_fire_railgun( Entity * self, Vec3 & start, Vec3 & aimdir, int damage, int kick, int flashtype ) {

	fire_rail( self, start, aimdir, damage, kick );

	Server::PF_WriteByte( svc_muzzleflash2 );
	Server::PF_WriteShort( ( int )( self - g_edicts ) );
	Server::PF_WriteByte( flashtype );
	Server::SV_Multicast( start, MULTICAST_PVS );
}

void Game::monster_fire_bfg( Entity * self, Vec3 & start, Vec3 & aimdir, int damage, int speed, int kick, float damage_radius, int flashtype ) {

	fire_bfg( self, start, aimdir, damage, speed, damage_radius );

	Server::PF_WriteByte( svc_muzzleflash2 );
	Server::PF_WriteShort( ( int )( self - g_edicts ) );
	Server::PF_WriteByte( flashtype );
	Server::SV_Multicast( start, MULTICAST_PVS );
}



//
// Monster utility functions
//

void Game::M_FliesOff( Entity * self ) {

	self->s.effects &= ~EF_FLIES;
	self->s.sound = 0;
}

void Game::M_FliesOn( Entity * self ) {

	if( self->waterlevel )
		return;
	self->s.effects |= EF_FLIES;
	self->s.sound = Server::SV_SoundIndex( "infantry/inflies1.wav" );
	self->think = M_FliesOff;
	self->nextthink = level.time + 60;
}

void Game::M_FlyCheck( Entity * self ) {

	if( self->waterlevel )
		return;

	if( random( ) > 0.5f )
		return;

	self->think = M_FliesOn;
	self->nextthink = level.time + 5 + 10 * random( );
}

void Game::AttackFinished( Entity * self, float time ) {

	self->monsterinfo.attack_finished = level.time + time;
}


void Game::M_CheckGround( Entity * ent ) {

	Vec3 point;
	CTrace trace;

	if( ent->flags &( FL_SWIM|FL_FLY ) ) return;

	if( ent->velocity[ 2 ] > 100 )
	{
		ent->groundentity = NULL;
		return;
	}

	// if the hull point one-quarter unit down is solid the entity is on ground
	point[ 0 ] = ent->s.origin[ 0 ];
	point[ 1 ] = ent->s.origin[ 1 ];
	point[ 2 ] = ent->s.origin[ 2 ] - 0.25f;

	trace = Server::SV_Trace( ent->s.origin, ent->mins, ent->maxs, point, ent, MASK_MONSTERSOLID );

	// check steepness
	if( trace.plane.Normal( )[ 2 ] < 0.7f && !trace.startsolid )
	{
		ent->groundentity = NULL;
		return;
	}

	//	ent->groundentity = trace.ent;
	//	ent->groundentity_linkcount = trace.ent->linkcount;
	//	if( !trace.startsolid && !trace.allsolid )
	//		VectorCopy( trace.endpos, ent->s.origin );
	if( !trace.startsolid && !trace.allsolid )
	{
		ent->s.origin = trace.endpos;
		ent->groundentity = trace.ent;
		ent->groundentity_linkcount = trace.ent->linkcount;
		ent->velocity[ 2 ] = 0;
	}
}


void Game::M_CatagorizePosition( Entity * ent ) {

	Vec3 point;
	int cont;

	//
	// get waterlevel
	//
	point[ 0 ] = ent->s.origin[ 0 ];
	point[ 1 ] = ent->s.origin[ 1 ];
	point[ 2 ] = ent->s.origin[ 2 ] + ent->mins[ 2 ] + 1;
	cont = Server::SV_PointContents( point );

	if( !( cont & MASK_WATER ) ) {
		ent->waterlevel = 0;
		ent->watertype = 0;
		return;
	}

	ent->watertype = cont;
	ent->waterlevel = 1;
	point[ 2 ] += 26;
	cont = Server::SV_PointContents( point );
	if( !( cont & MASK_WATER ) ) return;

	ent->waterlevel = 2;
	point[ 2 ] += 22;
	cont = Server::SV_PointContents( point );
	if( cont & MASK_WATER )
		ent->waterlevel = 3;
}


void Game::M_WorldEffects( Entity * ent ) {

	int dmg;

	if( ent->health > 0 )
	{
		if( !( ent->flags & FL_SWIM ) )
		{
			if( ent->waterlevel < 3 )
			{
				ent->air_finished = level.time + 12;
			}
			else if( ent->air_finished < level.time )
			{ // drown!
				if( ent->pain_debounce_time < level.time )
				{
					dmg = 2 + 2 * ( int )floorf( level.time - ent->air_finished );
					if( dmg > 15 )
						dmg = 15;
					T_Damage( ent, world, world, vec3_origin, ent->s.origin, vec3_origin, dmg, 0, DAMAGE_NO_ARMOR, MOD_WATER );
					ent->pain_debounce_time = level.time + 1.0f;
				}
			}
		}
		else
		{
			if( ent->waterlevel > 0 )
			{
				ent->air_finished = level.time + 9;
			}
			else if( ent->air_finished < level.time )
			{ // suffocate!
				if( ent->pain_debounce_time < level.time )
				{
					dmg = 2 + 2 * ( int )floorf( level.time - ent->air_finished );
					if( dmg > 15 )
						dmg = 15;
					T_Damage( ent, world, world, vec3_origin, ent->s.origin, vec3_origin, dmg, 0, DAMAGE_NO_ARMOR, MOD_WATER );
					ent->pain_debounce_time = level.time + 1.0f;
				}
			}
		}
	}

	if( ent->waterlevel == 0 )
	{
		if( ent->flags & FL_INWATER )
		{
			Server::PF_StartSound( ent, CHAN_BODY, Server::SV_SoundIndex( "player/watr_out.wav" ), 1, ATTN_NORM, 0 );
			ent->flags &= ~FL_INWATER;
		}
		return;
	}

	if( ( ent->watertype & CONTENTS_LAVA ) && !( ent->flags & FL_IMMUNE_LAVA ) ) {
		if( ent->damage_debounce_time < level.time )
		{
			ent->damage_debounce_time = level.time + 0.2f;
			T_Damage( ent, world, world, vec3_origin, ent->s.origin, vec3_origin, 10* ent->waterlevel, 0, 0, MOD_LAVA );
		}
	}
	if( ( ent->watertype & CONTENTS_SLIME ) && !( ent->flags & FL_IMMUNE_SLIME ) ) {
		if( ent->damage_debounce_time < level.time )
		{
			ent->damage_debounce_time = level.time + 1.0f;
			T_Damage( ent, world, world, vec3_origin, ent->s.origin, vec3_origin, 4* ent->waterlevel, 0, 0, MOD_SLIME );
		}
	}

	if( !( ent->flags & FL_INWATER ) )
	{
		if( !( ent->svflags & SVF_DEADMONSTER ) )
		{
			if( ent->watertype & CONTENTS_LAVA )
				if( random( ) <= 0.5f )
					Server::PF_StartSound( ent, CHAN_BODY, Server::SV_SoundIndex( "player/lava1.wav" ), 1, ATTN_NORM, 0 );
				else
					Server::PF_StartSound( ent, CHAN_BODY, Server::SV_SoundIndex( "player/lava2.wav" ), 1, ATTN_NORM, 0 );
			else if( ent->watertype & CONTENTS_SLIME )
				Server::PF_StartSound( ent, CHAN_BODY, Server::SV_SoundIndex( "player/watr_in.wav" ), 1, ATTN_NORM, 0 );
			else if( ent->watertype & CONTENTS_WATER )
				Server::PF_StartSound( ent, CHAN_BODY, Server::SV_SoundIndex( "player/watr_in.wav" ), 1, ATTN_NORM, 0 );
		}

		ent->flags |= FL_INWATER;
		ent->damage_debounce_time = 0.0;
	}
}


void Game::M_droptofloor( Entity * ent ) {

	Vec3 end;
	CTrace trace;

	ent->s.origin[ 2 ] += 1;
	end = ent->s.origin;
	end[ 2 ] -= 256;

	trace = Server::SV_Trace( ent->s.origin, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID );

	if( trace.fraction == 1 || trace.allsolid )
		return;

	ent->s.origin = trace.endpos;

	Server::SV_LinkEdict( ent );
	M_CheckGround( ent );
	M_CatagorizePosition( ent );
}


void Game::M_SetEffects( Entity * ent ) {

	ent->s.effects &= ~( EF_COLOR_SHELL|EF_POWERSCREEN );
	ent->s.renderfx &= ~( RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE );

	if( ent->monsterinfo.aiflags & AI_RESURRECTING )
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= RF_SHELL_RED;
	}

	if( ent->health <= 0 )
		return;

	if( ent->powerarmor_time > level.time )
	{
		if( ent->monsterinfo.power_armor_type == POWER_ARMOR_SCREEN )
		{
			ent->s.effects |= EF_POWERSCREEN;
		}
		else if( ent->monsterinfo.power_armor_type == POWER_ARMOR_SHIELD )
		{
			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_GREEN;
		}
	}
}


void Game::M_MoveFrame( Entity * self ) {

	mmove_t * move;
	int index;

	move = self->monsterinfo.currentmove;
	self->nextthink = level.time + FRAMETIME;

	if( ( self->monsterinfo.nextframe ) &&( self->monsterinfo.nextframe >= move->firstframe ) &&( self->monsterinfo.nextframe <= move->lastframe ) ) {
		self->s.frame = self->monsterinfo.nextframe;
		self->monsterinfo.nextframe = 0;
	}
	else
	{
		if( self->s.frame == move->lastframe )
		{
			if( move->endfunc )
			{
				move->endfunc( self );

				// regrab move, endfunc is very likely to change it
				move = self->monsterinfo.currentmove;

				// check for death
				if( self->svflags & SVF_DEADMONSTER )
					return;
			}
		}

		if( self->s.frame < move->firstframe || self->s.frame > move->lastframe )
		{
			self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
			self->s.frame = move->firstframe;
		}
		else
		{
			if( !( self->monsterinfo.aiflags & AI_HOLD_FRAME ) )
			{
				self->s.frame++;
				if( self->s.frame > move->lastframe )
					self->s.frame = move->firstframe;
			}
		}
	}

	index = self->s.frame - move->firstframe;
	if( move->frame[ index ].aifunc )
		if( !( self->monsterinfo.aiflags & AI_HOLD_FRAME ) ) 	move->frame[ index ].aifunc( self, move->frame[ index ].dist * self->monsterinfo.scale );
		else
			move->frame[ index ].aifunc( self, 0 );

	if( move->frame[ index ].thinkfunc )
		move->frame[ index ].thinkfunc( self );
}


void Game::monster_think( Entity * self ) {

	M_MoveFrame( self );
	if( self->linkcount != self->monsterinfo.linkcount )
	{
		self->monsterinfo.linkcount = self->linkcount;
		M_CheckGround( self );
	}
	M_CatagorizePosition( self );
	M_WorldEffects( self );
	M_SetEffects( self );
}


/*

================

monster_use



Using a monster makes it angry at the current activator

================

*/
void Game::monster_use( Entity * self, Entity * other, Entity * activator ) {

	if( self->enemy )
		return;
	if( self->health <= 0 )
		return;
	if( activator->flags & FL_NOTARGET )
		return;
	if( !( activator->client ) && !( activator->monsterinfo.aiflags & AI_GOOD_GUY ) ) return;

	// delay reaction so if the monster is teleported, its sound is still heard
	self->enemy = activator;
	FoundTarget( self );
}

void Game::monster_triggered_spawn( Entity * self ) {

	self->s.origin[ 2 ] += 1;
	KillBox( self );

	self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_STEP;
	self->svflags &= ~SVF_NOCLIENT;
	self->air_finished = level.time + 12;
	Server::SV_LinkEdict( self );

	monster_start_go( self );

	if( self->enemy && !( self->spawnflags & 1 ) && !( self->enemy->flags & FL_NOTARGET ) ) {
		FoundTarget( self );
	}
	else
	{
		self->enemy = NULL;
	}
}

void Game::monster_triggered_spawn_use( Entity * self, Entity * other, Entity * activator ) {

	// we have a one frame delay here so we don't telefrag the guy who activated us
	self->think = monster_triggered_spawn;
	self->nextthink = level.time + FRAMETIME;
	if( activator->client )
		self->enemy = activator;
	self->use = monster_use;
}

void Game::monster_triggered_start( Entity * self ) {

	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	self->nextthink = 0;
	self->use = monster_triggered_spawn_use;
}


/*

================

monster_death_use



When a monster dies, it fires all of its targets with the current

enemy as activator.

================

*/
void Game::monster_death_use( Entity * self ) {

	self->flags &= ~( FL_FLY|FL_SWIM );
	self->monsterinfo.aiflags &= AI_GOOD_GUY;

	if( self->item )
	{
		Drop_Item( self, self->item );
		self->item = NULL;
	}

	if( self->deathtarget )
		self->target = self->deathtarget;

	if( !self->target )
		return;

	G_UseTargets( self, self->enemy );
}


//============================================================================

bool Game::monster_start( Entity * self ) {

	if( deathmatch.GetBool( ) )
	{
		G_FreeEdict( self );
		return false;
	}

	if( ( self->spawnflags & 4 ) && !( self->monsterinfo.aiflags & AI_GOOD_GUY ) ) {
		self->spawnflags &= ~4;
		self->spawnflags |= 1;
		//		Common::Com_DPrintf( "fixed spawnflags on %s at %s\n", self->classname.c_str( ), vtos( self->s.origin ).c_str( ) );
	}

	if( !( self->monsterinfo.aiflags & AI_GOOD_GUY ) ) level.total_monsters++;

	self->nextthink = level.time + FRAMETIME;
	self->svflags |= SVF_MONSTER;
	self->s.renderfx |= RF_FRAMELERP;
	self->takedamage = DAMAGE_AIM;
	self->air_finished = level.time + 12;
	self->use = monster_use;
	self->max_health = self->health;
	self->clipmask = MASK_MONSTERSOLID;

	self->s.skinnum = 0;
	self->deadflag = DEAD_NO;
	self->svflags &= ~SVF_DEADMONSTER;

	if( !self->monsterinfo.checkattack )
		self->monsterinfo.checkattack = M_CheckAttack;
	self->s.old_origin = self->s.origin;

	if( st.item )
	{
		self->item = FindItemByClassname( st.item );
		if( !self->item )
			Common::Com_DPrintf( "%s at %s has bad item: %s\n", self->classname.c_str( ), vtos( self->s.origin ).c_str( ), st.item.c_str( ) );
	}

	// randomize what frame they start on
	if( self->monsterinfo.currentmove )
		self->s.frame = self->monsterinfo.currentmove->firstframe +( rand( ) %( self->monsterinfo.currentmove->lastframe - self->monsterinfo.currentmove->firstframe + 1 ) );

	return true;
}

void Game::monster_start_go( Entity * self ) {

	Vec3 v;

	if( self->health <= 0 )
		return;

	// check for target to combat_point and change to combattarget
	if( self->target )
	{
		bool notcombat;
		bool fixup;
		Entity * target;

		target = NULL;
		notcombat = false;
		fixup = false;
		while( ( target = G_Find( target, FOFS( targetname ), self->target ) ) != NULL )
		{
			if( strcmp( target->classname, "point_combat" ) == 0 )
			{
				self->combattarget = self->target;
				fixup = true;
			}
			else
			{
				notcombat = true;
			}
		}
		if( notcombat && self->combattarget )
			Common::Com_DPrintf( "%s at %s has target with mixed types\n", self->classname.c_str( ), vtos( self->s.origin ).c_str( ) );
		if( fixup )
			self->target = NULL;
	}

	// validate combattarget
	if( self->combattarget )
	{
		Entity * target;

		target = NULL;
		while( ( target = G_Find( target, FOFS( targetname ), self->combattarget ) ) != NULL )
		{
			if( strcmp( target->classname, "point_combat" ) != 0 )
			{
				Common::Com_DPrintf( "%s at( %i %i %i ) has a bad combattarget %s : %s at( %i %i %i )\n", self->classname.c_str( ), ( int )self->s.origin[ 0 ], ( int )self->s.origin[ 1 ], ( int )self->s.origin[ 2 ], self->combattarget.c_str( ), target->classname.c_str( ), ( int )target->s.origin[ 0 ], ( int )target->s.origin[ 1 ], ( int )target->s.origin[ 2 ] );
			}
		}
	}

	if( self->target )
	{
		self->goalentity = self->movetarget = G_PickTarget( self->target );
		if( !self->movetarget )
		{
			Common::Com_DPrintf( "%s can't find target %s at %s\n", self->classname.c_str( ), self->target.c_str( ), vtos( self->s.origin ).c_str( ) );
			self->target = NULL;
			self->monsterinfo.pausetime = 100000000.0;
			self->monsterinfo.stand( self );
		}
		else if( strcmp( self->movetarget->classname, "path_corner" ) == 0 )
		{
			v = self->goalentity->s.origin - self->s.origin;
			self->eal_yaw = self->s.angles[ YAW ] = vectoyaw( v );
			self->monsterinfo.walk( self );
			self->target = NULL;
		}
		else
		{
			self->goalentity = self->movetarget = NULL;
			self->monsterinfo.pausetime = 100000000.0;
			self->monsterinfo.stand( self );
		}
	}
	else
	{
		self->monsterinfo.pausetime = 100000000.0;
		self->monsterinfo.stand( self );
	}

	self->think = monster_think;
	self->nextthink = level.time + FRAMETIME;
}


void Game::walkmonster_start_go( Entity * self ) {

	if( !( self->spawnflags & 2 ) && level.time < 1 )
	{
		M_droptofloor( self );

		if( self->groundentity )
			if( !M_walkmove( self, 0, 0 ) ) 		Common::Com_DPrintf( "%s in solid at %s\n", self->classname.c_str( ), vtos( self->s.origin ).c_str( ) );
	}

	if( !self->yaw_speed )
		self->yaw_speed = 20;
	self->viewheight = 25;

	monster_start_go( self );

	if( self->spawnflags & 2 )
		monster_triggered_start( self );
}

void Game::walkmonster_start( Entity * self ) {

	self->think = walkmonster_start_go;
	monster_start( self );
}


void Game::flymonster_start_go( Entity * self ) {

	if( !M_walkmove( self, 0, 0 ) ) Common::Com_DPrintf( "%s in solid at %s\n", self->classname.c_str( ), vtos( self->s.origin ).c_str( ) );

	if( !self->yaw_speed )
		self->yaw_speed = 10;
	self->viewheight = 25;

	monster_start_go( self );

	if( self->spawnflags & 2 )
		monster_triggered_start( self );
}


void Game::flymonster_start( Entity * self ) {

	self->flags |= FL_FLY;
	self->think = flymonster_start_go;
	monster_start( self );
}


void Game::swimmonster_start_go( Entity * self ) {

	if( !self->yaw_speed )
		self->yaw_speed = 10;
	self->viewheight = 10;

	monster_start_go( self );

	if( self->spawnflags & 2 )
		monster_triggered_start( self );
}

void Game::swimmonster_start( Entity * self ) {

	self->flags |= FL_SWIM;
	self->think = swimmonster_start_go;
	monster_start( self );
}
