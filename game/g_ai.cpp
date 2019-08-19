#include "precompiled.h"
#pragma hdrstop

bool enemy_vis;
bool enemy_infront;
int enemy_range;
float enemy_yaw;

void Game::AI_SetSightClient( ) {
	Entity * ent;
	int start, check;
	if( level.sight_client == NULL )
		start = 1;
	else
		start = ( int )( level.sight_client - g_edicts );
	check = start;
	while( 1 ) {
		check++;
		if( check > maxClients )
			check = 1;
		ent = &g_edicts[ check ];
		if( ent->inuse && ent->health > 0 && !( ent->flags & FL_NOTARGET ) ) {
			level.sight_client = ent;
			return; // got one
		}
		if( check == start ) {
			level.sight_client = NULL;
			return; // nobody to see
		}
	}
}

void Game::ai_move( Entity * self, float dist ) {
	M_walkmove( self, self->s.angles[ YAW ], dist );
}

void Game::ai_stand( Entity * self, float dist ) {
	Vec3 v;
	if( dist )
		M_walkmove( self, self->s.angles[ YAW ], dist );
	if( self->monsterinfo.aiflags & AI_STAND_GROUND ) {
		if( self->enemy ) {
			v = self->enemy->s.origin - self->s.origin;
			self->eal_yaw = vectoyaw( v );
			if( self->s.angles[ YAW ] != self->eal_yaw && self->monsterinfo.aiflags & AI_TEMP_STAND_GROUND ) {
				self->monsterinfo.aiflags &= ~( AI_STAND_GROUND | AI_TEMP_STAND_GROUND );
				self->monsterinfo.run( self );
			}
			M_ChangeYaw( self );
			ai_checkattack( self, 0 );
		}
		else
			FindTarget( self );
		return;
	}
	if( FindTarget( self ) )
		return;
	if( level.time > self->monsterinfo.pausetime ) {
		self->monsterinfo.walk( self );
		return;
	}
	if( !( self->spawnflags & 1 ) &&( self->monsterinfo.le ) &&( level.time > self->monsterinfo.le_time ) ) {
		if( self->monsterinfo.le_time ) {
			self->monsterinfo.le( self );
			self->monsterinfo.le_time = level.time + 15 + random( ) * 15;
		} else {
			self->monsterinfo.le_time = level.time + random( ) * 15;
		}
	}
}

void Game::ai_walk( Entity * self, float dist ) {
	M_MoveToGoal( self, dist );
	// check for noticing a player
	if( FindTarget( self ) )
		return;
	if( ( self->monsterinfo.search ) &&( level.time > self->monsterinfo.le_time ) ) {
		if( self->monsterinfo.le_time ) {
			self->monsterinfo.search( self );
			self->monsterinfo.le_time = level.time + 15 + random( ) * 15;
		} else {
			self->monsterinfo.le_time = level.time + random( ) * 15;
		}
	}
}

void Game::ai_charge( Entity * self, float dist ) {
	Vec3 v;
	v = self->enemy->s.origin - self->s.origin;
	self->eal_yaw = vectoyaw( v );
	M_ChangeYaw( self );
	if( dist )
		M_walkmove( self, self->s.angles[ YAW ], dist );
}

void Game::ai_turn( Entity * self, float dist ) {
	if( dist )
		M_walkmove( self, self->s.angles[ YAW ], dist );
	if( FindTarget( self ) )
		return;
	M_ChangeYaw( self );
}


/*

.enemy

Will be world if not currently angry at anyone.

.movetarget

The next path spot to walk toward.  If .enemy, ignore .movetarget.
When an enemy is killed, the monster will try to return to it's path.

.hunt_time

Set to time + something when the player is in sight, but movement straight for
him is blocked.  This causes the monster to use wall following code for
movement direction instead of sighting on the player.

.eal_yaw

A yaw angle of the intended direction, which will be turned towards at up
to 45 deg / state.  If the enemy is in view and hunt_time is not active, 
this will be the exact line towards the enemy.

.pausetime

A monster will leave it's stand state and head towards it's .movetarget when
time > .pausetime.

walkmove( angle, speed ) primitive is all or nothing
*/
/*
=============
range

returns the range catagorization of an entity reletive to self

0	melee range, will become hostile even if back is turned
1	visibility and infront, or visibility and show hostile
2	infront and show hostile
3	only triggered by damage
=============
*/
int Game::range( Entity * self, Entity * other ) {
	Vec3 v;
	float len;

	v = self->s.origin - other->s.origin;
	len = v.Length( );
	if( len < MELEE_DISTANCE )
		return RANGE_MELEE;
	if( len < 500 )
		return RANGE_NEAR;
	if( len < 1000 )
		return RANGE_MID;
	return RANGE_FAR;
}

bool Game::visible( Entity * self, Entity * other ) {
	Vec3 spot1;
	Vec3 spot2;
	CTrace trace;
	spot1 = self->s.origin;
	spot1[ 2 ] += self->viewheight;
	spot2 = other->s.origin;
	spot2[ 2 ] += other->viewheight;
	trace = Server::SV_Trace( spot1, vec3_origin, vec3_origin, spot2, self, MASK_OPAQUE );
	if( trace.fraction == 1.0f )
		return true;
	return false;
}

bool Game::infront( Entity * self, Entity * other ) {
	Vec3 vec;
	float dot;
	Vec3 forward;
	self->s.angles.AngleVectors( &forward, NULL, NULL );
	vec = other->s.origin - self->s.origin;
	vec.Normalize( );
	dot = vec * forward;
	if( dot > 0.3f )
		return true;
	return false;
}

void Game::HuntTarget( Entity * self ) {
	Vec3 vec;
	self->goalentity = self->enemy;
	if( self->monsterinfo.aiflags & AI_STAND_GROUND )
		self->monsterinfo.stand( self );
	else
		self->monsterinfo.run( self );
	vec = self->enemy->s.origin - self->s.origin;
	self->eal_yaw = vectoyaw( vec );
	// wait a while before first attack
	if( !( self->monsterinfo.aiflags & AI_STAND_GROUND ) ) AttackFinished( self, 1 );
}

void Game::FoundTarget( Entity * self ) {
	// let other monsters see this monster for a while
	if( self->enemy->client ) {
		level.sight_entity = self;
		level.sight_entity_framenum = level.framenum;
		level.sight_entity->light_level = 128;
	}
	self->show_hostile = true; // wake up other monsters
	self->monsterinfo.last_sighting = self->enemy->s.origin;
	self->monsterinfo.trail_time = level.time;
	if( !self->combattarget ) {
		HuntTarget( self );
		return;
	}
	self->goalentity = self->movetarget = G_PickTarget( self->combattarget );
	if( !self->movetarget ) {
		self->goalentity = self->movetarget = self->enemy;
		HuntTarget( self );
		Common::Com_DPrintf( "%s at %s, combattarget %s not found\n", self->classname.c_str( ), vtos( self->s.origin ).c_str( ), self->combattarget.c_str( ) );
		return;
	}
	// clear out our combattarget, these are a one shot deal
	self->combattarget = NULL;
	self->monsterinfo.aiflags |= AI_COMBAT_POINT;
	// clear the targetname, that point is ours!
	self->movetarget->targetname = NULL;
	self->monsterinfo.pausetime = 0;
	// run for it
	self->monsterinfo.run( self );
}

/*
===========
FindTarget

Self is currently not attacking anything, so try to find a target

Returns TRUE if an enemy was sighted

When a player fires a missile, the point of impact becomes a fakeplayer so
that monsters that see the impact will respond as if they had seen the
player.

To avoid spending too much time, only a single client( or fakeclient ) is
checked each frame.  This means multi player games will have slightly
slower noticing monsters.
============
*/
bool Game::FindTarget( Entity * self ) {
	Entity * client;
	bool heardit;
	int r;
	if( self->monsterinfo.aiflags & AI_GOOD_GUY ) {
		if( self->goalentity && self->goalentity->inuse && self->goalentity->classname ) {
			if( strcmp( self->goalentity->classname, "target_actor" ) == 0 )
				return false;
		}
		//FIXME look for monsters?
		return false;
	}
	// if we're going to a combat point, just proceed
	if( self->monsterinfo.aiflags & AI_COMBAT_POINT )
		return false;
	// if the first spawnflag bit is set, the monster will only wake up on
	// really seeing the player, not another monster getting angry or hearing
	// something
	// revised behavior so they will wake up if they "see" a player make a noise
	// but not weapon impact/explosion noises
	heardit = false;
	if( ( level.sight_entity_framenum >=( level.framenum - 1 ) ) && !( self->spawnflags & 1 ) ) {
		client = level.sight_entity;
		if( client->enemy == self->enemy ) {
			return false;
		}
	} else if( level.sound_entity_framenum >=( level.framenum - 1 ) ) {
		client = level.sound_entity;
		heardit = true;
	} else if( !( self->enemy ) &&( level.sound2_entity_framenum >=( level.framenum - 1 ) ) && !( self->spawnflags & 1 ) ) {
		client = level.sound2_entity;
		heardit = true;
	} else {
		client = level.sight_client;
		if( !client )
			return false; // no clients to get mad at
	}
	// if the entity went away, forget it
	if( !client->inuse )
		return false;
	if( client == self->enemy )
		return true; // JDC false;
	if( client->client ) {
		if( client->flags & FL_NOTARGET )
			return false;
	} else if( client->svflags & SVF_MONSTER ) {
		if( !client->enemy )
			return false;
		if( client->enemy->flags & FL_NOTARGET )
			return false;
	} else if( heardit ) {
		if( client->owner->flags & FL_NOTARGET )
			return false;
	} else
		return false;
	if( !heardit ) {
		r = range( self, client );
		if( r == RANGE_FAR )
			return false;
		// this is where we would check invisibility
		// is client in an spot too dark to be seen?
		if( client->light_level <= 5 )
			return false;
		if( !visible( self, client ) )
			return false;
		if( r == RANGE_NEAR ) {
			if( client->show_hostile && !infront( self, client ) )
				return false;
		} else if( r == RANGE_MID ) {
			if( !infront( self, client ) )
				return false;
		}
		self->enemy = client;
		if( strcmp( self->enemy->classname, "player_noise" ) != 0 ) {
			self->monsterinfo.aiflags &= ~AI_SOUND_TARGET;
			if( !self->enemy->client ) {
				self->enemy = self->enemy->enemy;
				if( !self->enemy->client ) {
					self->enemy = NULL;
					return false;
				}
			}
		}
	} else { // heardit
		Vec3 temp;
		if( self->spawnflags & 1 ) {
			if( !visible( self, client ) )
				return false;
		} else {
			if( !Server::PF_inPHS( self->s.origin, client->s.origin ) )
				return false;
		}
		temp = client->s.origin - self->s.origin;
		if( temp.Length( ) > 1000.0f ) // too far to hear
			return false;
		// check area portals - if they are different and not connected then we can't hear it
		if( client->areanum != self->areanum )
			if( !CollisionModel::CM_AreasConnected( self->areanum, client->areanum ) )
				return false;
		self->eal_yaw = vectoyaw( temp );
		M_ChangeYaw( self );
		// hunt the sound for a bit; hopefully find the real player
		self->monsterinfo.aiflags |= AI_SOUND_TARGET;
		self->enemy = client;
	}
	// got one
	FoundTarget( self );
	if( !( self->monsterinfo.aiflags & AI_SOUND_TARGET ) &&( self->monsterinfo.sight ) )
		self->monsterinfo.sight( self, self->enemy );
	return true;
}

bool Game::FacingIdeal( Entity * self ) {
	float delta;
	delta = anglemod( self->s.angles[ YAW ] - self->eal_yaw );
	if( delta > 45 && delta < 315 )
		return false;
	return true;
}

bool Game::M_CheckAttack( Entity * self ) {
	Vec3 spot1, spot2;
	float chance;
	CTrace tr;
	if( self->enemy->health > 0 ) {
		// see if any entities are in the way of the shot
		spot1 = self->s.origin;
		spot1[ 2 ] += self->viewheight;
		spot2 = self->enemy->s.origin;
		spot2[ 2 ] += self->enemy->viewheight;
		tr = Server::SV_Trace( spot1, vec3_origin, vec3_origin, spot2, self, CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_SLIME|CONTENTS_LAVA|CONTENTS_OPAQUE );
		// do we have a clear shot?
		if( tr.ent != self->enemy )
			return false;
	}
	// melee attack
	if( enemy_range == RANGE_MELEE ) {
		// don't always melee in easy mode
		if( !skill.GetBool( ) &&( rand( )&3 ) )
			return false;
		if( self->monsterinfo.melee )
			self->monsterinfo.attack_state = AS_MELEE;
		else
			self->monsterinfo.attack_state = AS_MISSILE;
		return true;
	}
	// missile attack
	if( !self->monsterinfo.attack )
		return false;
	if( level.time < self->monsterinfo.attack_finished )
		return false;
	if( enemy_range == RANGE_FAR )
		return false;
	if( self->monsterinfo.aiflags & AI_STAND_GROUND )
		chance = 0.4f;
	else if( enemy_range == RANGE_MELEE )
		chance = 0.2f;
	else if( enemy_range == RANGE_NEAR )
		chance = 0.1f;
	else if( enemy_range == RANGE_MID )
		chance = 0.02f;
	else
		return false;
	if( !skill.GetBool( ) )
		chance *= 0.5f;
	else if( skill.GetInt( ) >= 2 )
		chance *= 2;
	if( random( ) < chance ) {
		self->monsterinfo.attack_state = AS_MISSILE;
		self->monsterinfo.attack_finished = level.time + 2* random( );
		return true;
	}
	if( self->flags & FL_FLY ) {
		if( random( ) < 0.3f )
			self->monsterinfo.attack_state = AS_SLIDING;
		else
			self->monsterinfo.attack_state = AS_STRAIGHT;
	}
	return false;
}

void Game::ai_run_melee( Entity * self ) {
	self->eal_yaw = enemy_yaw;
	M_ChangeYaw( self );
	if( FacingIdeal( self ) ) {
		self->monsterinfo.melee( self );
		self->monsterinfo.attack_state = AS_STRAIGHT;
	}
}

void Game::ai_run_missile( Entity * self ) {
	self->eal_yaw = enemy_yaw;
	M_ChangeYaw( self );
	if( FacingIdeal( self ) ) {
		self->monsterinfo.attack( self );
		self->monsterinfo.attack_state = AS_STRAIGHT;
	}
};

void Game::ai_run_slide( Entity * self, float distance ) {
	float ofs;
	self->eal_yaw = enemy_yaw;
	M_ChangeYaw( self );
	if( self->monsterinfo.lefty )
		ofs = 90;
	else
		ofs = -90;
	if( M_walkmove( self, self->eal_yaw + ofs, distance ) )
		return;
	self->monsterinfo.lefty = 1 - self->monsterinfo.lefty;
	M_walkmove( self, self->eal_yaw - ofs, distance );
}

bool Game::ai_checkattack( Entity * self, float dist ) {
	Vec3 temp;
	bool hesDeadJim;
	// this causes monsters to run blindly to the combat point w/o firing
	if( self->goalentity ) {
		if( self->monsterinfo.aiflags & AI_COMBAT_POINT )
			return false;
		if( self->monsterinfo.aiflags & AI_SOUND_TARGET ) {
			if( ( level.time - self->enemy->teleport_time ) > 5.0 ) {
				if( self->goalentity == self->enemy )
					if( self->movetarget )
						self->goalentity = self->movetarget;
					else
						self->goalentity = NULL;
				self->monsterinfo.aiflags &= ~AI_SOUND_TARGET;
				if( self->monsterinfo.aiflags & AI_TEMP_STAND_GROUND )
					self->monsterinfo.aiflags &= ~( AI_STAND_GROUND | AI_TEMP_STAND_GROUND );
			} else {
				self->show_hostile = true;
				return false;
			}
		}
	}
	enemy_vis = false;
	// see if the enemy is dead
	hesDeadJim = false;
	if( ( !self->enemy ) ||( !self->enemy->inuse ) )
		hesDeadJim = true;
	else if( self->monsterinfo.aiflags & AI_MEDIC ) {
		if( self->enemy->health > 0 ) {
			hesDeadJim = true;
			self->monsterinfo.aiflags &= ~AI_MEDIC;
		}
	} else {
		if( self->monsterinfo.aiflags & AI_BRUTAL ) {
			if( self->enemy->health <= -80 )
				hesDeadJim = true;
		} else {
			if( self->enemy->health <= 0 )
				hesDeadJim = true;
		}
	}
	if( hesDeadJim ) {
		self->enemy = NULL;
		// FIXME: look all around for other targets
		if( self->oldenemy && self->oldenemy->health > 0 ) {
			self->enemy = self->oldenemy;
			self->oldenemy = NULL;
			HuntTarget( self );
		} else {
			if( self->movetarget ) {
				self->goalentity = self->movetarget;
				self->monsterinfo.walk( self );
			} else {
				// we need the pausetime otherwise the stand code
				// will just revert to walking with no target and
				// the monsters will wonder around aimlessly trying
				// to hunt the world entity
				self->monsterinfo.pausetime = level.time + 100000000.0f;
				self->monsterinfo.stand( self );
			}
			return true;
		}
	}
	self->show_hostile = true; // wake up other monsters
	// check knowledge of enemy
	enemy_vis = visible( self, self->enemy );
	if( enemy_vis ) {
		self->monsterinfo.search_time = level.time + 5;
		self->monsterinfo.last_sighting = self->enemy->s.origin;
	}
	// look for other coop players here
	//	if( coop && self->monsterinfo.search_time < level.time )
	//	{
	//		if( FindTarget( self ) ) /			return true;
	//	}
	enemy_infront = infront( self, self->enemy );
	enemy_range = range( self, self->enemy );
	temp = self->enemy->s.origin - self->s.origin;
	enemy_yaw = vectoyaw( temp );
	// JDC self->eal_yaw = enemy_yaw;
	if( self->monsterinfo.attack_state == AS_MISSILE ) {
		ai_run_missile( self );
		return true;
	}
	if( self->monsterinfo.attack_state == AS_MELEE ) {
		ai_run_melee( self );
		return true;
	}
	// if enemy is not currently visible, we will never attack
	if( !enemy_vis )
		return false;
	return self->monsterinfo.checkattack( self );
}

void Game::ai_run( Entity * self, float dist ) {
	Vec3 v;
	Entity * tempgoal;
	Entity * save;
	bool isNew;
	Entity * marker;
	float d1, d2;
	CTrace tr;
	Vec3 v_forward, v_right;
	float left, center, right;
	Vec3 left_target, right_target;
	// if we're going to a combat point, just proceed
	if( self->monsterinfo.aiflags & AI_COMBAT_POINT ) {
		M_MoveToGoal( self, dist );
		return;
	}
	if( self->monsterinfo.aiflags & AI_SOUND_TARGET ) {
		v = self->s.origin - self->enemy->s.origin;
		if( v.Length( ) < 64.0f ) {
			self->monsterinfo.aiflags |=( AI_STAND_GROUND | AI_TEMP_STAND_GROUND );
			self->monsterinfo.stand( self );
			return;
		}
		M_MoveToGoal( self, dist );
		if( !FindTarget( self ) )
			return;
	}
	if( ai_checkattack( self, dist ) )
		return;
	if( self->monsterinfo.attack_state == AS_SLIDING ) {
		ai_run_slide( self, dist );
		return;
	}
	if( enemy_vis ) {
		//		if( self.aiflags & AI_LOST_SIGHT )
		//			dprint( "regained sight\n" );
		M_MoveToGoal( self, dist );
		self->monsterinfo.aiflags &= ~AI_LOST_SIGHT;
		self->monsterinfo.last_sighting = self->enemy->s.origin;
		self->monsterinfo.trail_time = level.time;
		return;
	}
	// coop will change to another enemy if visible
	if( coop.GetBool( ) ) { // FIXME: insane guys get mad with this, which causes crashes!
		if( FindTarget( self ) )
			return;
	}
	if( ( self->monsterinfo.search_time ) &&( level.time >( self->monsterinfo.search_time + 20 ) ) ) {
		M_MoveToGoal( self, dist );
		self->monsterinfo.search_time = 0;
		//		dprint( "search timeout\n" );
		return;
	}
	save = self->goalentity;
	tempgoal = G_Spawn( );
	self->goalentity = tempgoal;
	isNew = false;
	if( !( self->monsterinfo.aiflags & AI_LOST_SIGHT ) ) {
		// just lost sight of the player, decide where to go first
		//		dprint( "lost sight of player, last seen at " ); dprint( vtos( self.last_sighting ) ); dprint( "\n" );
		self->monsterinfo.aiflags |=( AI_LOST_SIGHT | AI_PURSUIT_LAST_SEEN );
		self->monsterinfo.aiflags &= ~( AI_PURSUE_NEXT | AI_PURSUE_TEMP );
		isNew = true;
	}
	if( self->monsterinfo.aiflags & AI_PURSUE_NEXT ) {
		self->monsterinfo.aiflags &= ~AI_PURSUE_NEXT;
		//		dprint( "reached current goal: " ); dprint( vtos( self.origin ) ); dprint( " " ); dprint( vtos( self.last_sighting ) ); dprint( " " ); dprint( ftos( vlen( self.origin - self.last_sighting ) ) ); dprint( "\n" );
		// give ourself more time since we got this far
		self->monsterinfo.search_time = level.time + 5;
		if( self->monsterinfo.aiflags & AI_PURSUE_TEMP ) {
			//			dprint( "was temp goal; retrying original\n" );
			self->monsterinfo.aiflags &= ~AI_PURSUE_TEMP;
			marker = NULL;
			self->monsterinfo.last_sighting = self->monsterinfo.saved_goal;
			isNew = true;
		} else if( self->monsterinfo.aiflags & AI_PURSUIT_LAST_SEEN ) {
			self->monsterinfo.aiflags &= ~AI_PURSUIT_LAST_SEEN;
			marker = PlayerTrail_PickFirst( self );
		} else {
			marker = PlayerTrail_PickNext( self );
		}
		if( marker ) {
			self->monsterinfo.last_sighting = marker->s.origin;
			self->monsterinfo.trail_time = marker->timestamp;
			self->s.angles[ YAW ] = self->eal_yaw = marker->s.angles[ YAW ];
			//			dprint( "heading is " ); dprint( ftos( self.eal_yaw ) ); dprint( "\n" );
			//			debug_drawline( self.origin, self.last_sighting, 52 );
			isNew = true;
		}
	}
	v = self->s.origin, self->monsterinfo.last_sighting;
	d1 = v.Length( );
	if( d1 <= dist ) {
		self->monsterinfo.aiflags |= AI_PURSUE_NEXT;
		dist = d1;
	}
	self->goalentity->s.origin = self->monsterinfo.last_sighting;
	if( isNew ) {
		//		gi.dprintf( "checking for course correction\n" );
		tr = Server::SV_Trace( self->s.origin, self->mins, self->maxs, self->monsterinfo.last_sighting, self, MASK_PLAYERSOLID );
		if( tr.fraction < 1 ) {
			v = self->goalentity->s.origin - self->s.origin;
			d1 = v.Length( );
			center = tr.fraction;
			d2 = d1 * ( ( center+1 )/2 );
			self->s.angles[ YAW ] = self->eal_yaw = vectoyaw( v );
			self->s.angles.AngleVectors( &v_forward, &v_right, NULL );
			v = Vec3( d2, -16.0f, 0.0f );
			G_ProjectSource( self->s.origin, v, v_forward, v_right, left_target );
			tr = Server::SV_Trace( self->s.origin, self->mins, self->maxs, left_target, self, MASK_PLAYERSOLID );
			left = tr.fraction;
			v = Vec3( d2, 16.0f, 0.0f );
			G_ProjectSource( self->s.origin, v, v_forward, v_right, right_target );
			tr = Server::SV_Trace( self->s.origin, self->mins, self->maxs, right_target, self, MASK_PLAYERSOLID );
			right = tr.fraction;
			center =( d1* center )/d2;
			if( left >= center && left > right ) {
				if( left < 1 ) {
					v = Vec3( d2 * left * 0.5f, -16, 0 );
					G_ProjectSource( self->s.origin, v, v_forward, v_right, left_target );
					//					gi.dprintf( "incomplete path, go part way and adjust again\n" );
				}
				self->monsterinfo.saved_goal = self->monsterinfo.last_sighting;
				self->monsterinfo.aiflags |= AI_PURSUE_TEMP;
				self->goalentity->s.origin = left_target;
				self->monsterinfo.last_sighting = left_target;
				v = self->goalentity->s.origin - self->s.origin;
				self->s.angles[ YAW ] = self->eal_yaw = vectoyaw( v );
				//				gi.dprintf( "adjusted left\n" );
				//				debug_drawline( self.origin, self.last_sighting, 152 );
			} else if( right >= center && right > left ) {
				if( right < 1 ) {
					v = Vec3( d2 * right * 0.5f, 16, 0 );
					G_ProjectSource( self->s.origin, v, v_forward, v_right, right_target );
					//					gi.dprintf( "incomplete path, go part way and adjust again\n" );
				}
				self->monsterinfo.saved_goal = self->monsterinfo.last_sighting;
				self->monsterinfo.aiflags |= AI_PURSUE_TEMP;
				self->goalentity->s.origin = right_target;
				self->monsterinfo.last_sighting = right_target;
				v = self->goalentity->s.origin - self->s.origin;
				self->s.angles[ YAW ] = self->eal_yaw = vectoyaw( v );
				//				gi.dprintf( "adjusted right\n" );
				//				debug_drawline( self.origin, self.last_sighting, 152 );
			}
		}
		//		else gi.dprintf( "course was fine\n" );
	}
	M_MoveToGoal( self, dist );
	G_FreeEdict( tempgoal );
	if( self )
		self->goalentity = save;
}
