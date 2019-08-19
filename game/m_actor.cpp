#include "precompiled.h"
#pragma hdrstop

#include "m_actor.h"

mframe_t Game::actor_frames_stand[ ] = {

	ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, 
	ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, 
	ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, 
	ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL
};

mmove_t Game::actor_move_stand = { FRAME_stand101, FRAME_stand140, actor_frames_stand, NULL };

void Game::actor_stand( Entity * self ) {

	self->monsterinfo.currentmove = &actor_move_stand;

	// randomize on startup
	if( level.time < 1.0f )
		self->s.frame = self->monsterinfo.currentmove->firstframe +( rand( ) %( self->monsterinfo.currentmove->lastframe - self->monsterinfo.currentmove->firstframe + 1 ) );
}


mframe_t Game::actor_frames_walk[ ] = {

	ai_walk, 0, NULL, ai_walk, 6, NULL, ai_walk, 10, NULL, ai_walk, 3, NULL, ai_walk, 2, NULL, ai_walk, 7, NULL, ai_walk, 10, NULL, ai_walk, 1, NULL, ai_walk, 4, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL
};

mmove_t Game::actor_move_walk = { FRAME_walk01, FRAME_walk08, actor_frames_walk, NULL };

void Game::actor_walk( Entity * self ) {

	self->monsterinfo.currentmove = &actor_move_walk;
}


mframe_t Game::actor_frames_run[ ] = {

	ai_run, 4, NULL, ai_run, 15, NULL, ai_run, 15, NULL, ai_run, 8, NULL, ai_run, 20, NULL, ai_run, 15, NULL, ai_run, 8, NULL, ai_run, 17, NULL, ai_run, 12, NULL, ai_run, -2, NULL, ai_run, -2, NULL, ai_run, -1, NULL
};

mmove_t Game::actor_move_run = { FRAME_run02, FRAME_run07, actor_frames_run, NULL };

void Game::actor_run( Entity * self ) {

	if( ( level.time < self->pain_debounce_time ) &&( !self->enemy ) ) {
		if( self->movetarget )
			actor_walk( self );
		else
			actor_stand( self );
		return;
	}

	if( self->monsterinfo.aiflags & AI_STAND_GROUND )
	{
		actor_stand( self );
		return;
	}

	self->monsterinfo.currentmove = &actor_move_run;
}


mframe_t Game::actor_frames_pain1[ ] = {

	ai_move, -5, NULL, ai_move, 4, NULL, ai_move, 1, NULL
};

mmove_t Game::actor_move_pain1 = { FRAME_pain101, FRAME_pain103, actor_frames_pain1, actor_run };

mframe_t Game::actor_frames_pain2[ ] = {

	ai_move, -4, NULL, ai_move, 4, NULL, ai_move, 0, NULL
};

mmove_t Game::actor_move_pain2 = { FRAME_pain201, FRAME_pain203, actor_frames_pain2, actor_run };

mframe_t Game::actor_frames_pain3[ ] = {

	ai_move, -1, NULL, ai_move, 1, NULL, ai_move, 0, NULL
};

mmove_t Game::actor_move_pain3 = { FRAME_pain301, FRAME_pain303, actor_frames_pain3, actor_run };

mframe_t Game::actor_frames_flipoff[ ] = {

	ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL
};

mmove_t Game::actor_move_flipoff = { FRAME_flip01, FRAME_flip14, actor_frames_flipoff, actor_run };

mframe_t Game::actor_frames_taunt[ ] = {

	ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL, ai_turn, 0, NULL
};

mmove_t Game::actor_move_taunt = { FRAME_taunt01, FRAME_taunt17, actor_frames_taunt, actor_run };

void Game::actor_pain( Entity * self, Entity * other, float kick, int damage ) {

	int n;

	if( self->health <( self->max_health / 2 ) ) self->s.skinnum = 1;

	if( level.time < self->pain_debounce_time )
		return;

	self->pain_debounce_time = level.time + 3.0f;
	//	Server::PF_StartSound( self, CHAN_VOICE, actor.sound_pain, 1, ATTN_NORM, 0 );

	if( ( other->client ) &&( random( ) < 0.4f ) ) {
		Vec3 v;
		Str name;

		v = other->s.origin - self->s.origin;
		self->eal_yaw = vectoyaw( v );
		if( random( ) < 0.5f )
			self->monsterinfo.currentmove = &actor_move_flipoff;
		else
			self->monsterinfo.currentmove = &actor_move_taunt;
		name = actor_names[( self - g_edicts )%8];
		Server::PF_cprintf( other, PRINT_CHAT, "%s: %s!\n", name.c_str( ), actor_messages[rand( )%3].c_str( ) );
		return;
	}

	n = rand( ) % 3;
	if( n == 0 )
		self->monsterinfo.currentmove = &actor_move_pain1;
	else if( n == 1 )
		self->monsterinfo.currentmove = &actor_move_pain2;
	else
		self->monsterinfo.currentmove = &actor_move_pain3;
}

void Game::actorMachineGun( Entity * self ) {

	Vec3 start, target;
	Vec3 forward, right;

	self->s.angles.AngleVectors( &forward, &right, NULL );
	G_ProjectSource( self->s.origin, monster_flash_offset[ MZ2_ACTOR_MACHINEGUN_1 ], forward, right, start );
	if( self->enemy )
	{
		if( self->enemy->health > 0 )
		{
			target = self->enemy->s.origin.Magnitude( -0.2f, self->enemy->velocity );
			target[ 2 ] += self->enemy->viewheight;
		}
		else
		{
			target = self->enemy->absmin;
			target[ 2 ] +=( self->enemy->size[ 2 ] / 2 );
		}
		forward = target - start;
		forward.Normalize( );
	}
	else
	{
		self->s.angles.AngleVectors( &forward, NULL, NULL );
	}
	monster_fire_bullet( self, start, forward, 3, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MZ2_ACTOR_MACHINEGUN_1 );
}

void Game::actor_dead( Entity * self ) {

	self->mins = Vec3( -16, -16, -24 );
	self->maxs = Vec3( 16, 16, -8 );
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	Server::SV_LinkEdict( self );
}

mframe_t Game::actor_frames_death1[ ] = {

	ai_move, 0, NULL, ai_move, 0, NULL, ai_move, -13, NULL, ai_move, 14, NULL, ai_move, 3, NULL, ai_move, -2, NULL, ai_move, 1, NULL
};

mmove_t Game::actor_move_death1 = { FRAME_death101, FRAME_death107, actor_frames_death1, actor_dead };

mframe_t Game::actor_frames_death2[ ] = {

	ai_move, 0, NULL, ai_move, 7, NULL, ai_move, -6, NULL, ai_move, -5, NULL, ai_move, 1, NULL, ai_move, 0, NULL, ai_move, -1, NULL, ai_move, -2, NULL, ai_move, -1, NULL, ai_move, -9, NULL, ai_move, -13, NULL, ai_move, -13, NULL, ai_move, 0, NULL
};

mmove_t Game::actor_move_death2 = { FRAME_death201, FRAME_death213, actor_frames_death2, actor_dead };

void Game::actor_die( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point ) {

	int n;

	// check for gib
	if( self->health <= -80 )
	{
		//		Server::PF_StartSound( self, CHAN_VOICE, actor.sound_gib, 1, ATTN_NORM, 0 );
		for( n = 0; n < 2; n++ ) ThrowGib( self, boneGib, damage, GIB_ORGANIC );
		for( n = 0; n < 4; n++ ) ThrowGib( self, meatGib, damage, GIB_ORGANIC );
		ThrowHead( self, headGib, damage, GIB_ORGANIC );
		self->deadflag = DEAD_DEAD;
		return;
	}

	if( self->deadflag == DEAD_DEAD )
		return;

	// regular death
	//	Server::PF_StartSound( self, CHAN_VOICE, actor.sound_die, 1, ATTN_NORM, 0 );
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	n = rand( ) % 2;
	if( n == 0 )
		self->monsterinfo.currentmove = &actor_move_death1;
	else
		self->monsterinfo.currentmove = &actor_move_death2;
}


void Game::actor_fire( Entity * self ) {

	actorMachineGun( self );

	if( level.time >= self->monsterinfo.pausetime )
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
}

mframe_t Game::actor_frames_attack[ ] = {

	ai_charge, -2, actor_fire, ai_charge, -2, NULL, ai_charge, 3, NULL, ai_charge, 2, NULL
};

mmove_t Game::actor_move_attack = { FRAME_attak01, FRAME_attak04, actor_frames_attack, actor_run };

void Game::actor_attack( Entity * self ) {

	int n;

	self->monsterinfo.currentmove = &actor_move_attack;
	n =( rand( ) & 15 ) + 3 + 7;
	self->monsterinfo.pausetime = level.time + n * FRAMETIME;
}


void Game::actor_use( Entity * self, Entity * other, Entity * activator ) {

	Vec3 v;

	self->goalentity = self->movetarget = G_PickTarget( self->target );
	if( ( !self->movetarget ) ||( strcmp( self->movetarget->classname, "target_actor" ) != 0 ) ) {
		Common::Com_DPrintf( "%s has bad target %s at %s\n", self->classname.c_str( ), self->target.c_str( ), vtos( self->s.origin ).c_str( ) );
		self->target = NULL;
		self->monsterinfo.pausetime = 100000000.0;
		self->monsterinfo.stand( self );
		return;
	}

	v = self->goalentity->s.origin - self->s.origin;
	self->eal_yaw = self->s.angles[ YAW ] = vectoyaw( v );
	self->monsterinfo.walk( self );
	self->target = NULL;
}

/* QUAKED misc_actor( 1 .5 0 )( -16 -16 -24 )( 16 16 32 )

*/
void Game::SP_misc_actor( Entity * self ) {

	if( deathmatch.GetBool( ) )
	{
		G_FreeEdict( self );
		return;
	}

	if( !self->targetname )
	{
		Common::Com_DPrintf( "untargeted %s at %s\n", self->classname.c_str( ), vtos( self->s.origin ).c_str( ) );
		G_FreeEdict( self );
		return;
	}

	if( !self->target )
	{
		Common::Com_DPrintf( "%s with no target at %s\n", self->classname.c_str( ), vtos( self->s.origin ).c_str( ) );
		G_FreeEdict( self );
		return;
	}

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = Server::SV_ModelIndex( "players/male/tris.md2" );
	self->mins = Vec3( -16, -16, -24 );
	self->maxs = Vec3( 16, 16, 32 );

	if( !self->health )
		self->health = 100;
	self->mass = 200;

	self->pain = actor_pain;
	self->die = actor_die;

	self->monsterinfo.stand = actor_stand;
	self->monsterinfo.walk = actor_walk;
	self->monsterinfo.run = actor_run;
	self->monsterinfo.attack = actor_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = NULL;

	self->monsterinfo.aiflags |= AI_GOOD_GUY;

	Server::SV_LinkEdict( self );

	self->monsterinfo.currentmove = &actor_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start( self );

	// actors always start in a dormant state, they * must* be used to get going
	self->use = actor_use;
}


/* QUAKED target_actor( .5 .3 0 )( -8 -8 -8 )( 8 8 8 ) JUMP SHOOT ATTACK x HOLD BRUTAL

JUMP			jump in set direction upon reaching this target

SHOOT			take a single shot at the pathtarget

ATTACK			attack pathtarget until it or actor is dead 



"target"		next target_actor

"pathtarget"	target of any action to be taken at this point

"wait"			amount of time actor should pause at this point

"message"		actor will "say" this to the player



for JUMP only:

"speed"			speed thrown forward( default 200 )

"height"		speed thrown upwards( default 200 )

*/
void Game::target_actor_touch( Entity * self, Entity * other, Plane & plane, int surf ) {

	Vec3 v;

	if( other->movetarget != self )
		return;

	if( other->enemy )
		return;

	other->goalentity = other->movetarget = NULL;

	if( self->message )
	{
		int n;
		Entity * ent;

		for( n = 1; n <= maxClients; n++ )
		{
			ent = &g_edicts[ n ];
			if( !ent->inuse )
				continue;
			Server::PF_cprintf( ent, PRINT_CHAT, "%s: %s\n", actor_names[( other - g_edicts )%8].c_str( ), self->message.c_str( ) );
		}
	}

	if( self->spawnflags & 1 ) //jump
	{
		other->velocity[ 0 ] = self->movedir[ 0 ] * self->speed;
		other->velocity[ 1 ] = self->movedir[ 1 ] * self->speed;

		if( other->groundentity )
		{
			other->groundentity = NULL;
			other->velocity[ 2 ] = self->movedir[ 2 ];
			Server::PF_StartSound( other, CHAN_VOICE, Server::SV_SoundIndex( "player/male/jump1.wav" ), 1, ATTN_NORM, 0 );
		}
	}

	if( self->spawnflags & 2 ) //shoot
	{
	}
	else if( self->spawnflags & 4 ) //attack
	{
		other->enemy = G_PickTarget( self->pathtarget );
		if( other->enemy )
		{
			other->goalentity = other->enemy;
			if( self->spawnflags & 32 )
				other->monsterinfo.aiflags |= AI_BRUTAL;
			if( self->spawnflags & 16 )
			{
				other->monsterinfo.aiflags |= AI_STAND_GROUND;
				actor_stand( other );
			}
			else
			{
				actor_run( other );
			}
		}
	}

	if( !( self->spawnflags & 6 ) &&( self->pathtarget ) ) {
		Str savetarget;

		savetarget = self->target;
		self->target = self->pathtarget;
		G_UseTargets( self, other );
		self->target = savetarget;
	}

	other->movetarget = G_PickTarget( self->target );

	if( !other->goalentity )
		other->goalentity = other->movetarget;

	if( !other->movetarget && !other->enemy )
	{
		other->monsterinfo.pausetime = level.time + 100000000.0f;
		other->monsterinfo.stand( other );
	}
	else if( other->movetarget == other->goalentity )
	{
		v = other->movetarget->s.origin - other->s.origin;
		other->eal_yaw = vectoyaw( v );
	}
}

void Game::SP_target_actor( Entity * self ) {

	if( !self->targetname )
		Common::Com_DPrintf( "%s with no targetname at %s\n", self->classname.c_str( ), vtos( self->s.origin ).c_str( ) );

	self->solid = SOLID_TRIGGER;
	self->touch = target_actor_touch;
	self->mins = Vec3( -8, -8, -8 );
	self->maxs = Vec3( 8, 8, 8 );
	self->svflags = SVF_NOCLIENT;

	if( self->spawnflags & 1 )
	{
		if( !self->speed )
			self->speed = 200;
		if( !st.height )
			st.height = 200;
		if( self->s.angles[ YAW ] == 0 )
			self->s.angles[ YAW ] = 360;
		G_SetMovedir( self->s.angles, &self->movedir );
		self->movedir[ 2 ] = ( float )st.height;
	}

	Server::SV_LinkEdict( self );
}
