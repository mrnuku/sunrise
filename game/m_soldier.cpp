#include "precompiled.h"
#pragma hdrstop

#include "m_soldier.h"

void Game::soldier_idle( Entity * self ) {

	if( random( ) > 0.8f )
		Server::PF_StartSound( self, CHAN_VOICE, soldier_sound_idle, 1, ATTN_IDLE, 0 );
}

void Game::soldier_cock( Entity * self ) {

	if( self->s.frame == FRAME_stand322 )
		Server::PF_StartSound( self, CHAN_WEAPON, soldier_sound_cock, 1, ATTN_IDLE, 0 );
	else
		Server::PF_StartSound( self, CHAN_WEAPON, soldier_sound_cock, 1, ATTN_NORM, 0 );
}


// STAND

mframe_t Game::soldier_frames_stand1 [] = {

	ai_stand, 0, soldier_idle, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, 
	ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, 
	ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL
};
mmove_t Game::soldier_move_stand1 = {FRAME_stand101, FRAME_stand130, soldier_frames_stand1, soldier_stand};

mframe_t Game::soldier_frames_stand3 [] = {

	ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, 
	ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, 
	ai_stand, 0, NULL, ai_stand, 0, soldier_cock, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, 
	ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL, ai_stand, 0, NULL
};
mmove_t Game::soldier_move_stand3 = {FRAME_stand301, FRAME_stand339, soldier_frames_stand3, soldier_stand};
void Game::soldier_stand( Entity * self ) {

	if( ( self->monsterinfo.currentmove == &soldier_move_stand3 ) ||( random( ) < 0.8f ) ) self->monsterinfo.currentmove = &soldier_move_stand1;
	else
		self->monsterinfo.currentmove = &soldier_move_stand3;
}


//
// WALK
//

void Game::soldier_walk1_random( Entity * self ) {

	if( random( ) > 0.1f )
		self->monsterinfo.nextframe = FRAME_walk101;
}

mframe_t Game::soldier_frames_walk1 [] = {

	ai_walk, 3, NULL, ai_walk, 6, NULL, ai_walk, 2, NULL, ai_walk, 2, NULL, ai_walk, 2, NULL, ai_walk, 1, NULL, ai_walk, 6, NULL, ai_walk, 5, NULL, ai_walk, 3, NULL, ai_walk, -1, soldier_walk1_random, ai_walk, 0, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL, ai_walk, 0, NULL
};
mmove_t Game::soldier_move_walk1 = {FRAME_walk101, FRAME_walk133, soldier_frames_walk1, NULL};

mframe_t Game::soldier_frames_walk2 [] = {

	ai_walk, 4, NULL, ai_walk, 4, NULL, ai_walk, 9, NULL, ai_walk, 8, NULL, ai_walk, 5, NULL, ai_walk, 1, NULL, ai_walk, 3, NULL, ai_walk, 7, NULL, ai_walk, 6, NULL, ai_walk, 7, NULL
};
mmove_t Game::soldier_move_walk2 = {FRAME_walk209, FRAME_walk218, soldier_frames_walk2, NULL};

void Game::soldier_walk( Entity * self ) {

	if( random( ) < 0.5f )
		self->monsterinfo.currentmove = &soldier_move_walk1;
	else
		self->monsterinfo.currentmove = &soldier_move_walk2;
}


//
// RUN
//

mframe_t Game::soldier_frames_start_run [] = {

	ai_run, 7, NULL, ai_run, 5, NULL
};
mmove_t Game::soldier_move_start_run = {FRAME_run01, FRAME_run02, soldier_frames_start_run, soldier_run};

mframe_t Game::soldier_frames_run [] = {

	ai_run, 10, NULL, ai_run, 11, NULL, ai_run, 11, NULL, ai_run, 16, NULL, ai_run, 10, NULL, ai_run, 15, NULL
};
mmove_t Game::soldier_move_run = {FRAME_run03, FRAME_run08, soldier_frames_run, NULL};

void Game::soldier_run( Entity * self ) {

	if( self->monsterinfo.aiflags & AI_STAND_GROUND )
	{
		self->monsterinfo.currentmove = &soldier_move_stand1;
		return;
	}

	if( self->monsterinfo.currentmove == &soldier_move_walk1 ||
		self->monsterinfo.currentmove == &soldier_move_walk2 ||
		self->monsterinfo.currentmove == &soldier_move_start_run )
	{
		self->monsterinfo.currentmove = &soldier_move_run;
	}
	else
	{
		self->monsterinfo.currentmove = &soldier_move_start_run;
	}
}


//
// PAIN
//

mframe_t Game::soldier_frames_pain1 [] = {

	ai_move, -3, NULL, ai_move, 4, NULL, ai_move, 1, NULL, ai_move, 1, NULL, ai_move, 0, NULL
};
mmove_t Game::soldier_move_pain1 = {FRAME_pain101, FRAME_pain105, soldier_frames_pain1, soldier_run};

mframe_t Game::soldier_frames_pain2 [] = {

	ai_move, -13, NULL, ai_move, -1, NULL, ai_move, 2, NULL, ai_move, 4, NULL, ai_move, 2, NULL, ai_move, 3, NULL, ai_move, 2, NULL
};
mmove_t Game::soldier_move_pain2 = {FRAME_pain201, FRAME_pain207, soldier_frames_pain2, soldier_run};

mframe_t Game::soldier_frames_pain3 [] = {

	ai_move, -8, NULL, ai_move, 10, NULL, ai_move, -4, NULL, ai_move, -1, NULL, ai_move, -3, NULL, ai_move, 0, NULL, ai_move, 3, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 1, NULL, ai_move, 0, NULL, ai_move, 1, NULL, ai_move, 2, NULL, ai_move, 4, NULL, ai_move, 3, NULL, ai_move, 2, NULL
};
mmove_t Game::soldier_move_pain3 = {FRAME_pain301, FRAME_pain318, soldier_frames_pain3, soldier_run};

mframe_t Game::soldier_frames_pain4 [] = {

	ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, -10, NULL, ai_move, -6, NULL, ai_move, 8, NULL, ai_move, 4, NULL, ai_move, 1, NULL, ai_move, 0, NULL, ai_move, 2, NULL, ai_move, 5, NULL, ai_move, 2, NULL, ai_move, -1, NULL, ai_move, -1, NULL, ai_move, 3, NULL, ai_move, 2, NULL, ai_move, 0, NULL
};
mmove_t Game::soldier_move_pain4 = {FRAME_pain401, FRAME_pain417, soldier_frames_pain4, soldier_run};


void Game::soldier_pain( Entity * self, Entity * other, float kick, int damage ) {

	float r;
	int n;

	if( self->health <( self->max_health / 2 ) ) self->s.skinnum |= 1;

	if( level.time < self->pain_debounce_time )
	{
		if( ( self->velocity[ 2 ] > 100 ) &&( ( self->monsterinfo.currentmove == &soldier_move_pain1 ) ||( self->monsterinfo.currentmove == &soldier_move_pain2 ) ||( self->monsterinfo.currentmove == &soldier_move_pain3 ) ) )
			self->monsterinfo.currentmove = &soldier_move_pain4;
		return;
	}

	self->pain_debounce_time = level.time + 3.0f;

	n = self->s.skinnum | 1;
	if( n == 1 )
		Server::PF_StartSound( self, CHAN_VOICE, soldier_sound_pain_light, 1, ATTN_NORM, 0 );
	else if( n == 3 )
		Server::PF_StartSound( self, CHAN_VOICE, soldier_sound_pain, 1, ATTN_NORM, 0 );
	else
		Server::PF_StartSound( self, CHAN_VOICE, soldier_sound_pain_ss, 1, ATTN_NORM, 0 );

	if( self->velocity[ 2 ] > 100 )
	{
		self->monsterinfo.currentmove = &soldier_move_pain4;
		return;
	}

	if( skill.GetInt( ) == 3 )
		return; // no pain anims in nightmare

	r = random( );

	if( r < 0.33f )
		self->monsterinfo.currentmove = &soldier_move_pain1;
	else if( r < 0.66f )
		self->monsterinfo.currentmove = &soldier_move_pain2;
	else
		self->monsterinfo.currentmove = &soldier_move_pain3;
}


//
// ATTACK
//

int Game::blaster_flash [] = {MZ2_SOLDIER_BLASTER_1, MZ2_SOLDIER_BLASTER_2, MZ2_SOLDIER_BLASTER_3, MZ2_SOLDIER_BLASTER_4, MZ2_SOLDIER_BLASTER_5, MZ2_SOLDIER_BLASTER_6, MZ2_SOLDIER_BLASTER_7, MZ2_SOLDIER_BLASTER_8};
int Game::shotgun_flash [] = {MZ2_SOLDIER_SHOTGUN_1, MZ2_SOLDIER_SHOTGUN_2, MZ2_SOLDIER_SHOTGUN_3, MZ2_SOLDIER_SHOTGUN_4, MZ2_SOLDIER_SHOTGUN_5, MZ2_SOLDIER_SHOTGUN_6, MZ2_SOLDIER_SHOTGUN_7, MZ2_SOLDIER_SHOTGUN_8};
int Game::machinegun_flash [] = {MZ2_SOLDIER_MACHINEGUN_1, MZ2_SOLDIER_MACHINEGUN_2, MZ2_SOLDIER_MACHINEGUN_3, MZ2_SOLDIER_MACHINEGUN_4, MZ2_SOLDIER_MACHINEGUN_5, MZ2_SOLDIER_MACHINEGUN_6, MZ2_SOLDIER_MACHINEGUN_7, MZ2_SOLDIER_MACHINEGUN_8};

void Game::soldier_fire( Entity * self, int flash_number ) {

	Vec3 start;
	Vec3 forward, right, up;
	Vec3 aim;
	Vec3 dir;
	Vec3 end;
	float r, u;
	int flash_index;

	if( self->s.skinnum < 2 )
		flash_index = blaster_flash[ flash_number ];
	else if( self->s.skinnum < 4 )
		flash_index = shotgun_flash[ flash_number ];
	else
		flash_index = machinegun_flash[ flash_number ];

	self->s.angles.AngleVectors( &forward, &right, NULL );
	G_ProjectSource( self->s.origin, monster_flash_offset[ flash_index ], forward, right, start );

	if( flash_number == 5 || flash_number == 6 )
	{
		aim = forward;
	}
	else
	{
		end = self->enemy->s.origin;
		end[ 2 ] += self->enemy->viewheight;
		aim = end - start;
		vectoangles( aim, &dir );
		dir.AngleVectors( &forward, &right, &up );

		r = crandom( )* 1000;
		u = crandom( )* 500;
		end = start.Magnitude( 8192.0f, forward );
		end = end.Magnitude( r, right );
		end = end.Magnitude( u, up );

		aim = end - start;
		aim.Normalize( );
	}

	if( self->s.skinnum <= 1 )
	{
		monster_fire_blaster( self, start, aim, 5, 600, flash_index, EF_BLASTER );
	}
	else if( self->s.skinnum <= 3 )
	{
		monster_fire_shotgun( self, start, aim, 2, 1, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SHOTGUN_COUNT, flash_index );
	}
	else
	{
		if( !( self->monsterinfo.aiflags & AI_HOLD_FRAME ) ) 	self->monsterinfo.pausetime = level.time + ( 3 + rand( ) % 8 ) * FRAMETIME;

		monster_fire_bullet( self, start, aim, 2, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, flash_index );

		if( level.time >= self->monsterinfo.pausetime )
			self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
		else
			self->monsterinfo.aiflags |= AI_HOLD_FRAME;
	}
}

// ATTACK1( blaster/shotgun )

void Game::soldier_fire1( Entity * self ) {

	soldier_fire( self, 0 );
}

void Game::soldier_attack1_refire1( Entity * self ) {

	if( self->s.skinnum > 1 )
		return;

	if( self->enemy->health <= 0 )
		return;

	if( ( ( skill.GetInt( ) == 3 ) &&( random( ) < 0.5f ) ) ||( range( self, self->enemy ) == RANGE_MELEE ) )
		self->monsterinfo.nextframe = FRAME_attak102;
	else
		self->monsterinfo.nextframe = FRAME_attak110;
}

void Game::soldier_attack1_refire2( Entity * self ) {

	if( self->s.skinnum < 2 )
		return;

	if( self->enemy->health <= 0 )
		return;

	if( ( ( skill.GetInt( ) == 3 ) &&( random( ) < 0.5f ) ) ||( range( self, self->enemy ) == RANGE_MELEE ) )
		self->monsterinfo.nextframe = FRAME_attak102;
}

mframe_t Game::soldier_frames_attack1 [] = {

	ai_charge, 0, NULL, ai_charge, 0, NULL, ai_charge, 0, soldier_fire1, ai_charge, 0, NULL, ai_charge, 0, NULL, ai_charge, 0, soldier_attack1_refire1, ai_charge, 0, NULL, ai_charge, 0, soldier_cock, ai_charge, 0, soldier_attack1_refire2, ai_charge, 0, NULL, ai_charge, 0, NULL, ai_charge, 0, NULL
};
mmove_t Game::soldier_move_attack1 = {FRAME_attak101, FRAME_attak112, soldier_frames_attack1, soldier_run};

// ATTACK2( blaster/shotgun )

void Game::soldier_fire2( Entity * self ) {

	soldier_fire( self, 1 );
}

void Game::soldier_attack2_refire1( Entity * self ) {

	if( self->s.skinnum > 1 )
		return;

	if( self->enemy->health <= 0 )
		return;

	if( ( ( skill.GetInt( ) == 3 ) &&( random( ) < 0.5f ) ) ||( range( self, self->enemy ) == RANGE_MELEE ) )
		self->monsterinfo.nextframe = FRAME_attak204;
	else
		self->monsterinfo.nextframe = FRAME_attak216;
}

void Game::soldier_attack2_refire2( Entity * self ) {

	if( self->s.skinnum < 2 )
		return;

	if( self->enemy->health <= 0 )
		return;

	if( ( ( skill.GetInt( ) == 3 ) &&( random( ) < 0.5f ) ) ||( range( self, self->enemy ) == RANGE_MELEE ) )
		self->monsterinfo.nextframe = FRAME_attak204;
}

mframe_t Game::soldier_frames_attack2 [] = {

	ai_charge, 0, NULL, ai_charge, 0, NULL, ai_charge, 0, NULL, ai_charge, 0, NULL, ai_charge, 0, soldier_fire2, ai_charge, 0, NULL, ai_charge, 0, NULL, ai_charge, 0, soldier_attack2_refire1, ai_charge, 0, NULL, ai_charge, 0, NULL, ai_charge, 0, NULL, ai_charge, 0, NULL, ai_charge, 0, soldier_cock, ai_charge, 0, NULL, ai_charge, 0, soldier_attack2_refire2, ai_charge, 0, NULL, ai_charge, 0, NULL, ai_charge, 0, NULL
};
mmove_t Game::soldier_move_attack2 = {FRAME_attak201, FRAME_attak218, soldier_frames_attack2, soldier_run};

// ATTACK3( duck and shoot )

void Game::soldier_duck_down( Entity * self ) {

	if( self->monsterinfo.aiflags & AI_DUCKED )
		return;
	self->monsterinfo.aiflags |= AI_DUCKED;
	self->maxs[ 2 ] -= 32;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.pausetime = level.time + 1;
	Server::SV_LinkEdict( self );
}

void Game::soldier_duck_up( Entity * self ) {

	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->maxs[ 2 ] += 32;
	self->takedamage = DAMAGE_AIM;
	Server::SV_LinkEdict( self );
}

void Game::soldier_fire3( Entity * self ) {

	soldier_duck_down( self );
	soldier_fire( self, 2 );
}

void Game::soldier_attack3_refire( Entity * self ) {

	if( ( level.time + 0.4f ) < self->monsterinfo.pausetime )
		self->monsterinfo.nextframe = FRAME_attak303;
}

mframe_t Game::soldier_frames_attack3 [] = {

	ai_charge, 0, NULL, ai_charge, 0, NULL, ai_charge, 0, soldier_fire3, ai_charge, 0, NULL, ai_charge, 0, NULL, ai_charge, 0, soldier_attack3_refire, ai_charge, 0, soldier_duck_up, ai_charge, 0, NULL, ai_charge, 0, NULL
};
mmove_t Game::soldier_move_attack3 = {FRAME_attak301, FRAME_attak309, soldier_frames_attack3, soldier_run};

// ATTACK4( machinegun )

void Game::soldier_fire4( Entity * self ) {

	soldier_fire( self, 3 );
	//
	//	if( self->enemy->health <= 0 )
	//		return;
	//
	//	if( ( ( skill.GetInt( ) == 3 ) &&( random( ) < 0.5f ) ) ||( range( self, self->enemy ) == RANGE_MELEE ) )
	//		self->monsterinfo.nextframe = FRAME_attak402;
}

mframe_t Game::soldier_frames_attack4 [] = {

	ai_charge, 0, NULL, ai_charge, 0, NULL, ai_charge, 0, soldier_fire4, ai_charge, 0, NULL, ai_charge, 0, NULL, ai_charge, 0, NULL
};
mmove_t Game::soldier_move_attack4 = {FRAME_attak401, FRAME_attak406, soldier_frames_attack4, soldier_run};
// ATTACK6( run & shoot )

void Game::soldier_fire8( Entity * self ) {

	soldier_fire( self, 7 );
}

void Game::soldier_attack6_refire( Entity * self ) {

	if( self->enemy->health <= 0 )
		return;

	if( range( self, self->enemy ) < RANGE_MID )
		return;

	if( skill.GetInt( ) == 3 )
		self->monsterinfo.nextframe = FRAME_runs03;
}

mframe_t Game::soldier_frames_attack6 [] = {

	ai_charge, 10, NULL, ai_charge, 4, NULL, ai_charge, 12, NULL, ai_charge, 11, soldier_fire8, ai_charge, 13, NULL, ai_charge, 18, NULL, ai_charge, 15, NULL, ai_charge, 14, NULL, ai_charge, 11, NULL, ai_charge, 8, NULL, ai_charge, 11, NULL, ai_charge, 12, NULL, ai_charge, 12, NULL, ai_charge, 17, soldier_attack6_refire
};
mmove_t Game::soldier_move_attack6 = {FRAME_runs01, FRAME_runs14, soldier_frames_attack6, soldier_run};

void Game::soldier_attack( Entity * self ) {

	if( self->s.skinnum < 4 )
	{
		if( random( ) < 0.5f )
			self->monsterinfo.currentmove = &soldier_move_attack1;
		else
			self->monsterinfo.currentmove = &soldier_move_attack2;
	}
	else
	{
		self->monsterinfo.currentmove = &soldier_move_attack4;
	}
}


//
// SIGHT
//

void Game::soldier_sight( Entity * self, Entity * other ) {

	if( random( ) < 0.5f )
		Server::PF_StartSound( self, CHAN_VOICE, soldier_sound_sight1, 1, ATTN_NORM, 0 );
	else
		Server::PF_StartSound( self, CHAN_VOICE, soldier_sound_sight2, 1, ATTN_NORM, 0 );

	if( ( skill.GetBool( ) ) &&( range( self, self->enemy ) >= RANGE_MID ) ) {
		if( random( ) > 0.5f )
			self->monsterinfo.currentmove = &soldier_move_attack6;
	}
}

//
// DUCK
//

void Game::soldier_duck_hold( Entity * self ) {

	if( level.time >= self->monsterinfo.pausetime )
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
}

mframe_t Game::soldier_frames_duck [] = {

	ai_move, 5, soldier_duck_down, ai_move, -1, soldier_duck_hold, ai_move, 1, NULL, ai_move, 0, soldier_duck_up, ai_move, 5, NULL
};
mmove_t Game::soldier_move_duck = {FRAME_duck01, FRAME_duck05, soldier_frames_duck, soldier_run};

void Game::soldier_dodge( Entity * self, Entity * attacker, float eta ) {

	float r;

	r = random( );
	if( r > 0.25f )
		return;

	if( !self->enemy )
		self->enemy = attacker;

	if( !skill.GetBool( ) ) {

		self->monsterinfo.currentmove = &soldier_move_duck;
		return;
	}

	self->monsterinfo.pausetime = level.time + eta + 0.3f;
	r = random( );

	if( skill.GetInt( ) == 1 )
	{
		if( r > 0.33f )
			self->monsterinfo.currentmove = &soldier_move_duck;
		else
			self->monsterinfo.currentmove = &soldier_move_attack3;
		return;
	}

	if( skill.GetInt( ) >= 2 )
	{
		if( r > 0.66f )
			self->monsterinfo.currentmove = &soldier_move_duck;
		else
			self->monsterinfo.currentmove = &soldier_move_attack3;
		return;
	}

	self->monsterinfo.currentmove = &soldier_move_attack3;
}


//
// DEATH
//

void Game::soldier_fire6( Entity * self ) {

	soldier_fire( self, 5 );
}

void Game::soldier_fire7( Entity * self ) {

	soldier_fire( self, 6 );
}

void Game::soldier_dead( Entity * self ) {

	self->mins = Vec3( -16, -16, -24 );
	self->maxs = Vec3( 16, 16, -8 );
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	Server::SV_LinkEdict( self );
}

mframe_t Game::soldier_frames_death1 [] = {

	ai_move, 0, NULL, ai_move, -10, NULL, ai_move, -10, NULL, ai_move, -10, NULL, ai_move, -5, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, 
	ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, 
	ai_move, 0, NULL, ai_move, 0, soldier_fire6, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, soldier_fire7, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, 
	ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL
};
mmove_t Game::soldier_move_death1 = {FRAME_death101, FRAME_death136, soldier_frames_death1, soldier_dead};

mframe_t Game::soldier_frames_death2 [] = {

	ai_move, -5, NULL, ai_move, -5, NULL, ai_move, -5, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, 
	ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, 
	ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, 
	ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL
};
mmove_t Game::soldier_move_death2 = {FRAME_death201, FRAME_death235, soldier_frames_death2, soldier_dead};

mframe_t Game::soldier_frames_death3 [] = {

	ai_move, -5, NULL, ai_move, -5, NULL, ai_move, -5, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, 
	ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, 
	ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, 
	ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, 
	ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, };
mmove_t Game::soldier_move_death3 = {FRAME_death301, FRAME_death345, soldier_frames_death3, soldier_dead};

mframe_t Game::soldier_frames_death4 [] = {

	ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, 
	ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, 
	ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, 
	ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, 
	ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, 
	ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL
};
mmove_t Game::soldier_move_death4 = {FRAME_death401, FRAME_death453, soldier_frames_death4, soldier_dead};

mframe_t Game::soldier_frames_death5 [] = {

	ai_move, -5, NULL, ai_move, -5, NULL, ai_move, -5, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, 
	ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, 
	ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL
};
mmove_t Game::soldier_move_death5 = {FRAME_death501, FRAME_death524, soldier_frames_death5, soldier_dead};

mframe_t Game::soldier_frames_death6 [] = {

	ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL
};
mmove_t Game::soldier_move_death6 = {FRAME_death601, FRAME_death610, soldier_frames_death6, soldier_dead};

void Game::soldier_die( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point ) {

	int n;

	// check for gib
	if( self->health <= self->gib_health )
	{
		Server::PF_StartSound( self, CHAN_VOICE, Server::SV_SoundIndex( "misc/udeath.wav" ), 1, ATTN_NORM, 0 );
		for( n = 0; n < 3; n++ )
			ThrowGib( self, meatGib, damage, GIB_ORGANIC );
		ThrowGib( self, "models/objects/gibs/chest/tris.md2", damage, GIB_ORGANIC );
		ThrowHead( self, headGib, damage, GIB_ORGANIC );
		self->deadflag = DEAD_DEAD;
		return;
	}

	if( self->deadflag == DEAD_DEAD )
		return;

	// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->s.skinnum |= 1;

	if( self->s.skinnum == 1 )
		Server::PF_StartSound( self, CHAN_VOICE, soldier_sound_death_light, 1, ATTN_NORM, 0 );
	else if( self->s.skinnum == 3 )
		Server::PF_StartSound( self, CHAN_VOICE, soldier_sound_death, 1, ATTN_NORM, 0 );
	else //( self->s.skinnum == 5 )
		Server::PF_StartSound( self, CHAN_VOICE, soldier_sound_death_ss, 1, ATTN_NORM, 0 );

	if( fabs( ( self->s.origin[ 2 ] + self->viewheight ) - point[ 2 ] ) <= 4 )
	{
		// head shot
		self->monsterinfo.currentmove = &soldier_move_death3;
		return;
	}

	n = rand( ) % 5;
	if( n == 0 )
		self->monsterinfo.currentmove = &soldier_move_death1;
	else if( n == 1 )
		self->monsterinfo.currentmove = &soldier_move_death2;
	else if( n == 2 )
		self->monsterinfo.currentmove = &soldier_move_death4;
	else if( n == 3 )
		self->monsterinfo.currentmove = &soldier_move_death5;
	else
		self->monsterinfo.currentmove = &soldier_move_death6;
}


//
// SPAWN
//

void Game::SP_monster_soldier_x( Entity * self ) {


	self->s.modelindex = Server::SV_ModelIndex( "models/monsters/soldier/tris.md2" );
	self->monsterinfo.scale = MODEL_SCALE;
	self->mins = Vec3( -16, -16, -24 );
	self->maxs = Vec3( 16, 16, 32 );
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	soldier_sound_idle = Server::SV_SoundIndex( "soldier/solidle1.wav" );
	soldier_sound_sight1 = Server::SV_SoundIndex( "soldier/solsght1.wav" );
	soldier_sound_sight2 = Server::SV_SoundIndex( "soldier/solsrch1.wav" );
	soldier_sound_cock = Server::SV_SoundIndex( "infantry/infatck3.wav" );

	self->mass = 100;

	self->pain = soldier_pain;
	self->die = soldier_die;

	self->monsterinfo.stand = soldier_stand;
	self->monsterinfo.walk = soldier_walk;
	self->monsterinfo.run = soldier_run;
	self->monsterinfo.dodge = soldier_dodge;
	self->monsterinfo.attack = soldier_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = soldier_sight;

	Server::SV_LinkEdict( self );

	self->monsterinfo.stand( self );

	walkmonster_start( self );
}


/* QUAKED monster_soldier_light( 1 .5 0 )( -16 -16 -24 )( 16 16 32 ) Ambush Trigger_Spawn Sight

*/
void Game::SP_monster_soldier_light( Entity * self ) {

	if( deathmatch.GetBool( ) )
	{
		G_FreeEdict( self );
		return;
	}

	SP_monster_soldier_x( self );

	soldier_sound_pain_light = Server::SV_SoundIndex( "soldier/solpain2.wav" );
	soldier_sound_death_light = Server::SV_SoundIndex( "soldier/soldeth2.wav" );
	Server::SV_ModelIndex( "models/objects/laser/tris.md2" );
	Server::SV_SoundIndex( "misc/lasfly.wav" );
	Server::SV_SoundIndex( "soldier/solatck2.wav" );

	self->s.skinnum = 0;
	self->health = 20;
	self->gib_health = -30;
}

/* QUAKED monster_soldier( 1 .5 0 )( -16 -16 -24 )( 16 16 32 ) Ambush Trigger_Spawn Sight

*/
void Game::SP_monster_soldier( Entity * self ) {

	if( deathmatch.GetBool( ) )
	{
		G_FreeEdict( self );
		return;
	}

	SP_monster_soldier_x( self );

	soldier_sound_pain = Server::SV_SoundIndex( "soldier/solpain1.wav" );
	soldier_sound_death = Server::SV_SoundIndex( "soldier/soldeth1.wav" );
	Server::SV_SoundIndex( "soldier/solatck1.wav" );

	self->s.skinnum = 2;
	self->health = 30;
	self->gib_health = -30;
}

/* QUAKED monster_soldier_ss( 1 .5 0 )( -16 -16 -24 )( 16 16 32 ) Ambush Trigger_Spawn Sight

*/
void Game::SP_monster_soldier_ss( Entity * self ) {

	if( deathmatch.GetBool( ) )
	{
		G_FreeEdict( self );
		return;
	}

	SP_monster_soldier_x( self );

	soldier_sound_pain_ss = Server::SV_SoundIndex( "soldier/solpain3.wav" );
	soldier_sound_death_ss = Server::SV_SoundIndex( "soldier/soldeth3.wav" );
	Server::SV_SoundIndex( "soldier/solatck3.wav" );

	self->s.skinnum = 4;
	self->health = 40;
	self->gib_health = -30;
}
