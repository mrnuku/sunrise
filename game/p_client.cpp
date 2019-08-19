#include "precompiled.h"
#pragma hdrstop

#include "m_player.h"

void client_persistant_t::Clear( ) {
	userinfo.Clear( );
	netname.Clear( );
	hand			= 0;
	connected		= false;
	health			= 0;
	max_health		= 0;
	savedFlags		= 0;
	selected_item	= 0;
	for( int i = 0; i < MAX_ITEMS; i++ )
		inventory[ i ] = 0;
	max_bullets		= 0;
	max_shells		= 0;
	max_rockets		= 0;
	max_grenades	= 0;
	max_cells		= 0;
	max_slugs		= 0;
	weapon			= NULL;
	lastweapon		= NULL;
	power_cubes		= 0;
	score			= 0;
	game_helpchanged	= 0;
	helpchanged		= 0;
	spectator		= false;
}

void client_respawn_t::Clear( ) {
	coop_respawn.Clear( );
	enterframe		= 0;
	score			= 0;
	cmd_angles.Zero( );
	spectator		= false;
}

void gclient_t::Clear( ) {
	ps.Clear( );
	ping			= 0;
	pers.Clear( );
	resp.Clear( );
	old_pmove.Clear( );
	showscores		= false;
	showinventory	= false;
	showhelp		= false;
	showhelpicon	= false;
	ammo_index		= 0;
	buttons			= 0;
	oldbuttons		= 0;
	latched_buttons	= 0;
	weapon_thunk	= false;
	newweapon		= NULL;
	damage_armor	= 0;
	damage_parmor	= 0;
	damage_blood	= 0;
	damage_knockback= 0;
	damage_from.Zero( );
	killer_yaw		= 0;
	weaponstate		= WEAPON_READY;
	kick_angles.Zero( );
	kick_origin.Zero( );
	v_dmg_roll		= 0;
	v_dmg_pitch		= 0;
	v_dmg_time		= 0;
	fall_time		= 0;
	fall_value		= 0;
	damage_alpha	= 0;
	bonus_alpha		= 0;
	damage_blend.Zero( );
	v_angle.Zero( );
	bobtime			= 0;
	oldviewangles.Zero( );
	oldvelocity.Zero( );
	next_drown_time	= 0;
	old_waterlevel	= 0;
	breather_sound	= 0;
	machinegun_shots= 0;
	anim_end		= 0;
	anim_priority	= 0;
	anim_duck		= false;
	anim_run		= false;
	quad_framenum	= 0;
	invincible_framenum	= 0;
	breather_framenum	= 0;
	enviro_framenum		= 0;
	grenade_blew_up	= false;
	grenade_time		= 0;
	silencer_shots		= 0;
	weapon_sound		= 0;
	pickup_msg_time		= 0;
	flood_locktill		= 0;
	for( int i = 0; i < 10; i++ )
		flood_when[ i ] = 0;
	flood_whenhead		= 0;
	respawn_time		= 0;
	chase_target	= NULL;
	update_chase	= false;
}

void Game::SP_FixCoopSpots( Entity * self ) {
	Entity * spot;
	Vec3 d;
	spot = NULL;
	while( 1 ) {
		spot = G_Find( spot, FOFS( classname ), "info_player_start" );
		if( !spot )
			return;
		if( !spot->targetname )
			continue;
		d = self->s.origin - spot->s.origin;
		if( d.Length( ) < 384.0f ) {
			if( ( !self->targetname ) || !self->targetname.IcmpFast( spot->targetname ) ) {
				//				Common::Com_DPrintf( "FixCoopSpots changed %s at %s targetname from %s to %s\n", self->classname, vtos( self->s.origin ), self->targetname, spot->targetname );
				self->targetname = spot->targetname;
			}
			return;
		}
	}
}

void Game::SP_CreateCoopSpots( Entity * self ) {
}

void Game::SP_info_player_start( Entity * self ) {
	if( !coop.GetBool( ) )
		return;
}

void Game::SP_info_player_deathmatch( Entity * self ) {
	if( !deathmatch.GetBool( ) ){
		G_FreeEdict( self );
		return;
	}
	SP_misc_teleporter_dest( self );
}

void Game::SP_info_player_coop( Entity * self ) {
	if( !coop.GetBool( ) ) {
		G_FreeEdict( self );
		return;
	}
}

void Game::SP_info_player_intermission( Entity * ent ) {
}

void Game::player_pain( Entity * self, Entity * other, float kick, int damage ) {
	// player pain is handled at the end of the frame in P_DamageFeedback
}

bool Game::IsFemale( Entity * ent ) {
	Str info;
	if( !ent->client )
		return false;
	info = ClientServerInfo::Info_ValueForKey( ent->client->pers.userinfo, "gender" );
	if( info[ 0 ] == 'f' || info[ 0 ] == 'F' )
		return true;
	return false;
}
bool Game::IsNeutral( Entity * ent ) {
	Str info;
	if( !ent->client )
		return false;
	info = ClientServerInfo::Info_ValueForKey( ent->client->pers.userinfo, "gender" );
	if( info[ 0 ] != 'f' && info[ 0 ] != 'F' && info[ 0 ] != 'm' && info[ 0 ] != 'M' )
		return true;
	return false;
}

void Game::ClientObituary( Entity * self, Entity * inflictor, Entity * attacker ) {
	int mod;
	Str message;
	Str message2;
	bool ff;
	if( coop.GetBool( ) && attacker->client )
		meansOfDeath |= MOD_FRIENDLY_FIRE;
	if( deathmatch.GetBool( ) || coop.GetBool( ) ) {
		ff = ( meansOfDeath & MOD_FRIENDLY_FIRE ) > 0;
		mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
		switch( mod ) {
			case MOD_SUICIDE:
				message = "suicides";
				break;
			case MOD_FALLING:
				message = "cratered";
				break;
			case MOD_CRUSH:
				message = "was squished";
				break;
			case MOD_WATER:
				message = "sank like a rock";
				break;
			case MOD_SLIME:
				message = "melted";
				break;
			case MOD_LAVA:
				message = "does a back flip into the lava";
				break;
			case MOD_EXPLOSIVE:
			case MOD_BARREL:
				message = "blew up";
				break;
			case MOD_EXIT:
				message = "found a way out";
				break;
			case MOD_TARGET_LASER:
				message = "saw the light";
				break;
			case MOD_TARGET_BLASTER:
				message = "got blasted";
				break;
			case MOD_BOMB:
			case MOD_SPLASH:
			case MOD_TRIGGER_HURT:
				message = "was in the wrong place";
				break;
		}
		if( attacker == self ) {
			switch( mod ) {
				case MOD_HELD_GRENADE:
					message = "tried to put the pin back in";
					break;
				case MOD_HG_SPLASH:
				case MOD_G_SPLASH:
					if( IsNeutral( self ) ) 			message = "tripped on its own grenade";
					else if( IsFemale( self ) ) 			message = "tripped on her own grenade";
					else
						message = "tripped on his own grenade";
					break;
				case MOD_R_SPLASH:
					if( IsNeutral( self ) ) 			message = "blew itself up";
					else if( IsFemale( self ) ) 			message = "blew herself up";
					else
						message = "blew himself up";
					break;
				case MOD_BFG_BLAST:
					message = "should have used a smaller gun";
					break;
				default:
					if( IsNeutral( self ) ) 			message = "killed itself";
					else if( IsFemale( self ) ) 			message = "killed herself";
					else
						message = "killed himself";
					break;
			}
		}
		if( message.Length( ) ) {
			Server::SV_BroadcastPrintf( PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname.c_str( ), message.c_str( ) );
			if( deathmatch.GetBool( ) )
				self->client->resp.score--;
			self->enemy = NULL;
			return;
		}
		self->enemy = attacker;
		if( attacker && attacker->client ) {
			switch( mod ) {
				case MOD_BLASTER:
					message = "was blasted by";
					break;
				case MOD_SHOTGUN:
					message = "was gunned down by";
					break;
				case MOD_SSHOTGUN:
					message = "was blown away by";
					message2 = "'s super shotgun";
					break;
				case MOD_MACHINEGUN:
					message = "was machinegunned by";
					break;
				case MOD_CHAINGUN:
					message = "was cut in half by";
					message2 = "'s chaingun";
					break;
				case MOD_GRENADE:
					message = "was popped by";
					message2 = "'s grenade";
					break;
				case MOD_G_SPLASH:
					message = "was shredded by";
					message2 = "'s shrapnel";
					break;
				case MOD_ROCKET:
					message = "ate";
					message2 = "'s rocket";
					break;
				case MOD_R_SPLASH:
					message = "almost dodged";
					message2 = "'s rocket";
					break;
				case MOD_HYPERBLASTER:
					message = "was melted by";
					message2 = "'s hyperblaster";
					break;
				case MOD_RAILGUN:
					message = "was railed by";
					break;
				case MOD_BFG_LASER:
					message = "saw the pretty lights from";
					message2 = "'s BFG";
					break;
				case MOD_BFG_BLAST:
					message = "was disintegrated by";
					message2 = "'s BFG blast";
					break;
				case MOD_BFG_EFFECT:
					message = "couldn't hide from";
					message2 = "'s BFG";
					break;
				case MOD_HANDGRENADE:
					message = "caught";
					message2 = "'s handgrenade";
					break;
				case MOD_HG_SPLASH:
					message = "didn't see";
					message2 = "'s handgrenade";
					break;
				case MOD_HELD_GRENADE:
					message = "feels";
					message2 = "'s pain";
					break;
				case MOD_TELEFRAG:
					message = "tried to invade";
					message2 = "'s personal space";
					break;
			}
			Server::SV_BroadcastPrintf( PRINT_MEDIUM, "%s %s %s%s\n", self->client->pers.netname.c_str( ), message.c_str( ), attacker->client->pers.netname.c_str( ), message2.c_str( ) );
			if( deathmatch.GetBool( ) ) {
				if( ff )
					attacker->client->resp.score--;
				else
					attacker->client->resp.score++;
			}
			return;
		}
	}
	Server::SV_BroadcastPrintf( PRINT_MEDIUM, "%s died.\n", self->client->pers.netname.c_str( ) );
	if( deathmatch.GetBool( ) )
		self->client->resp.score--;
}

void Game::TossClientWeapon( Entity * self ) {
	GameItem * item;
	Entity * drop;
	bool quad;
	float spread;
	if( !deathmatch.GetBool( ) )
		return;
	item = self->client->pers.weapon;
	if( ! self->client->pers.inventory[ self->client->ammo_index ] )
		item = NULL;
	if( item &&( strcmp( item->pickup_name, "Blaster" ) == 0 ) )
		item = NULL;
	if( !( dmflags.GetInt( ) & DF_QUAD_DROP ) )
		quad = false;
	else
		quad =( self->client->quad_framenum >( level.framenum + 10 ) );
	if( item && quad )
		spread = 22.5f;
	else
		spread = 0.0f;
	if( item ) {
		self->client->v_angle[ YAW ] -= spread;
		drop = Drop_Item( self, item );
		self->client->v_angle[ YAW ] += spread;
		drop->spawnflags = DROPPED_PLAYER_ITEM;
	}
	if( quad ) {
		self->client->v_angle[ YAW ] += spread;
		drop = Drop_Item( self, FindItemByClassname( "item_quad" ) );
		self->client->v_angle[ YAW ] -= spread;
		drop->spawnflags |= DROPPED_PLAYER_ITEM;
		drop->touch = Touch_Item;
		drop->nextthink = level.time +( self->client->quad_framenum - level.framenum ) * FRAMETIME;
		drop->think = G_FreeEdict;
	}
}

void Game::LookAtKiller( Entity * self, Entity * inflictor, Entity * attacker ) {
	Vec3 dir;
	if( attacker && attacker != world && attacker != self )
		dir = attacker->s.origin - self->s.origin;
	else if( inflictor && inflictor != world && inflictor != self )
		dir = inflictor->s.origin - self->s.origin;
	else {
		self->client->killer_yaw = self->s.angles[ YAW ];
		return;
	}
	if( dir[ 0 ] )
		self->client->killer_yaw = 180/M_PI* atan2( dir[ 1 ], dir[ 0 ] );
	else {
		self->client->killer_yaw = 0;
		if( dir[ 1 ] > 0 )
			self->client->killer_yaw = 90;
		else if( dir[ 1 ] < 0 )
			self->client->killer_yaw = -90;
	}
	if( self->client->killer_yaw < 0 )
		self->client->killer_yaw += 360;
}

void Game::player_die( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point ) {
	int n;
	self->avelocity = vec3_origin;
	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;
	self->s.modelindex2 = 0; // remove linked weapon model
	self->s.angles[ 0 ] = 0;
	self->s.angles[ 2 ] = 0;
	self->s.sound = 0;
	self->client->weapon_sound = 0;
	self->maxs[ 2 ] = -8;
	//	self->solid = SOLID_NOT;
	self->svflags |= SVF_DEADMONSTER;
	if( !self->deadflag ) {
		self->client->respawn_time = level.time + 1.0f;
		LookAtKiller( self, inflictor, attacker );
		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary( self, inflictor, attacker );
		TossClientWeapon( self );
		if( deathmatch.GetBool( ) )
			Cmd_Help_f( self ); // show scores
		// clear inventory
		// this is kind of ugly, but it's how we want to handle keys in coop
		for( n = 0; n < num_items; n++ ) {
			if( coop.GetBool( ) && itemlist[ n ].flags & IT_KEY )
				self->client->resp.coop_respawn.inventory[ n ] = self->client->pers.inventory[ n ];
			self->client->pers.inventory[ n ] = 0;
		}
	}
	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;
	self->flags &= ~FL_POWER_ARMOR;
	if( self->health < -40 ) { // gib
		Server::PF_StartSound( self, CHAN_BODY, Server::SV_SoundIndex( "misc/udeath.wav" ), 1, ATTN_NORM, 0 );
		for( n = 0; n < 4; n++ )
			ThrowGib( self, meatGib, damage, GIB_ORGANIC );
		ThrowClientHead( self, damage );
		self->takedamage = DAMAGE_NO;
	} else { // normal death
		if( !self->deadflag ) {
			static int			i;
			i =( i+1 )%3;
			// start a death animation
			self->client->anim_priority = ANIM_DEATH;
			if( self->client->ps.pmove.pm_flags & PMF_DUCKED ) {
				self->s.frame = FRAME_crdeath1-1;
				self->client->anim_end = FRAME_crdeath5;
			} else {
				switch( i ) {
					case 0:
						self->s.frame = FRAME_death101-1;
						self->client->anim_end = FRAME_death106;
						break;
					case 1:
						self->s.frame = FRAME_death201-1;
						self->client->anim_end = FRAME_death206;
						break;
					case 2:
						self->s.frame = FRAME_death301-1;
						self->client->anim_end = FRAME_death308;
						break;
				}
				Server::PF_StartSound( self, CHAN_VOICE, Server::SV_SoundIndex( va( "*death%i.wav", ( rand( )%4 )+1 ) ), 1, ATTN_NORM, 0 );
			}
		}
	}
	self->deadflag = DEAD_DEAD;
	Server::SV_LinkEdict( self );
}

void Game::InitClientPersistant( gclient_t * client ) {
	GameItem * item;
	client->pers.Clear( );
	item = FindItem( "Blaster" );
	client->pers.selected_item = ITEM_INDEX( item );
	client->pers.inventory[ client->pers.selected_item ] = 1;
	client->pers.weapon = item;
	client->pers.health = 100;
	client->pers.max_health = 100;
	client->pers.max_bullets = 200;
	client->pers.max_shells = 100;
	client->pers.max_rockets = 50;
	client->pers.max_grenades = 50;
	client->pers.max_cells = 200;
	client->pers.max_slugs = 50;
	client->pers.connected = true;
}

void Game::InitClientResp( gclient_t * client ) {
	client->resp.Clear( );
	client->resp.enterframe = level.framenum;
	client->resp.coop_respawn = client->pers;
}

void Game::SaveClientData( ) {
	int i;
	Entity * ent;
	for( i = 0; i<maxClients; i++ ) {
		ent = &g_edicts[ 1+i ];
		if( !ent->inuse )
			continue;
		clients[ i ].pers.health = ent->health;
		clients[ i ].pers.max_health = ent->max_health;
		clients[ i ].pers.savedFlags =( ent->flags &( FL_GODMODE|FL_NOTARGET|FL_POWER_ARMOR ) );
		if( coop.GetBool( ) )
			clients[ i ].pers.score = ent->client->resp.score;
	}
}

void Game::FetchClientEntData( Entity * ent ) {
	ent->health = ent->client->pers.health;
	ent->max_health = ent->client->pers.max_health;
	ent->flags |= ent->client->pers.savedFlags;
	if( coop.GetBool( ) )
		ent->client->resp.score = ent->client->pers.score;
}

float Game::PlayersRangeFromSpot( Entity * spot ) {
	Entity * player;
	float bestplayerdistance;
	Vec3 v;
	int n;
	float playerdistance;
	bestplayerdistance = 9999999;
	for( n = 1; n <= Common::maxclients.GetInt( ); n++ ) {
		player = &g_edicts[ n ];
		if( !player->inuse )
			continue;
		if( player->health <= 0 )
			continue;
		v = spot->s.origin - player->s.origin;
		playerdistance = v.Length( );
		if( playerdistance < bestplayerdistance )
			bestplayerdistance = playerdistance;
	}
	return bestplayerdistance;
}

Entity * Game::SelectRandomDeathmatchSpawnPoint( ) {
	Entity * spot, * spot1, * spot2;
	int count = 0;
	int selection;
	float range, range1, range2;
	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;
	while( ( spot = G_Find( spot, FOFS( classname ), "info_player_deathmatch" ) ) != NULL ) {
		count++;
		range = PlayersRangeFromSpot( spot );
		if( range < range1 ) {
			range1 = range;
			spot1 = spot;
		} else if( range < range2 ) {
			range2 = range;
			spot2 = spot;
		}
	}
	if( !count )
		return NULL;
	if( count <= 2 )
		spot1 = spot2 = NULL;
	else
		count -= 2;
	selection = rand( ) % count;
	spot = NULL;
	do {
		spot = G_Find( spot, FOFS( classname ), "info_player_deathmatch" );
		if( spot == spot1 || spot == spot2 )
			selection++;
	} while( selection-- );
	return spot;
}

Entity * Game::SelectFarthestDeathmatchSpawnPoint( ) {
	Entity * bestspot;
	float bestdistance, bestplayerdistance;
	Entity * spot;
	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while( ( spot = G_Find( spot, FOFS( classname ), "info_player_deathmatch" ) ) != NULL ) {
		bestplayerdistance = PlayersRangeFromSpot( spot );
		if( bestplayerdistance > bestdistance ) {
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}
	}
	if( bestspot )
		return bestspot;
	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_Find( NULL, FOFS( classname ), "info_player_deathmatch" );
	return spot;
}

Entity * Game::SelectDeathmatchSpawnPoint( ) {
	if( dmflags.GetInt( ) & DF_SPAWN_FARTHEST )
		return SelectFarthestDeathmatchSpawnPoint( );
	else
		return SelectRandomDeathmatchSpawnPoint( );
}


Entity * Game::SelectCoopSpawnPoint( Entity * ent ) {
	int index;
	Entity * spot = NULL;
	Str target;
	index = ( int )( ent->client - clients );
	// player 0 starts in normal player spawn point
	if( !index )
		return NULL;
	spot = NULL;
	// assume there are four coop spots at each spawnpoint
	while( 1 ) {
		spot = G_Find( spot, FOFS( classname ), "info_player_coop" );
		if( !spot )
			return NULL; // we didn't have enough...
		target = spot->targetname;
		if( spawnpoint == target ) { // this is a coop spawn point for one of the clients here
			index--;
			if( !index )
				return spot; // this is it
		}
	}
	return spot;
}

void Game::SelectSpawnPoint( Entity * ent, Vec3 & origin, Vec3 & angles ) {
	Entity * spot = NULL;
	if( deathmatch.GetBool( ) )
		spot = SelectDeathmatchSpawnPoint( );
	else if( coop.GetBool( ) )
		spot = SelectCoopSpawnPoint( ent );
	// find a single player start spot
	if( !spot ) {
		while( ( spot = G_Find( spot, FOFS( classname ), "info_player_start" ) ) != NULL ) {
			if( !spawnpoint[ 0 ] && spot->targetname.IsEmpty( ) )
				break;
			if( !spawnpoint[ 0 ] || !spot->targetname.IsEmpty( ) )
				continue;
			if( spawnpoint.IcmpFast( spot->targetname ) )
				break;
		}
		if( !spot ) {
			if( spawnpoint.IsEmpty( ) ) // there wasn't a spawnpoint without a target, so use any
				spot = G_Find( spot, FOFS( classname ), "info_player_start" );
			if( !spot )
				Server::PF_error( "Couldn't find spawn point %s\n", spawnpoint.c_str( ) );
		}
	}
	origin = spot->s.origin;
	origin[ 2 ] += 9;
	angles = spot->s.angles;
}

void Game::InitBodyQue( ) {
	int i;
	Entity * ent;
	level.body_que = 0;
	for( i = 0; i<BODY_QUEUE_SIZE; i++ ) {
		ent = G_Spawn( );
		ent->classname = "bodyque";
	}
}

void Game::body_die( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point ) {
	int n;
	if( self->health < -40 ) {
		Server::PF_StartSound( self, CHAN_BODY, Server::SV_SoundIndex( "misc/udeath.wav" ), 1, ATTN_NORM, 0 );
		for( n = 0; n < 4; n++ )
			ThrowGib( self, meatGib, damage, GIB_ORGANIC );
		self->s.origin[ 2 ] -= 48;
		ThrowClientHead( self, damage );
		self->takedamage = DAMAGE_NO;
	}
}

void Game::CopyToBodyQue( Entity * ent ) {
	Entity * body;
	// grab a body que and cycle to the next one
	body = &g_edicts[ Common::maxclients.GetInt( ) + level.body_que + 1 ];
	level.body_que =( level.body_que + 1 ) % BODY_QUEUE_SIZE;
	// FIXME: send an effect on the removed body
	Server::SV_UnlinkEdict( ent );
	Server::SV_UnlinkEdict( body );
	body->s = ent->s;
	body->s.number = ( int )( body - g_edicts );
	body->svflags = ent->svflags;
	body->mins = ent->mins;
	body->maxs = ent->maxs;
	body->absmin = ent->absmin;
	body->absmax = ent->absmax;
	body->size = ent->size;
	body->solid = ent->solid;
	body->clipmask = ent->clipmask;
	body->owner = ent->owner;
	body->movetype = ent->movetype;
	body->die = body_die;
	body->takedamage = DAMAGE_YES;
	Server::SV_LinkEdict( body );
}

void Game::respawn( Entity * self ) {
	if( deathmatch.GetBool( ) || coop.GetBool( ) ) {
		// spectator's don't leave bodies
		if( self->movetype != MOVETYPE_NOCLIP )
			CopyToBodyQue( self );
		self->svflags &= ~SVF_NOCLIENT;
		PutClientInServer( self );
		// add a teleportation effect
		self->s.eventNum = EV_PLAYER_TELEPORT;
		// hold in place briefly
		self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		self->client->ps.pmove.pm_time = 14;
		self->client->respawn_time = level.time;
		return;
	}
	// restart the entire server
	CBuffer::Cbuf_AddText( "menu_loadgame\n" );
}

void Game::spectator_respawn( Entity * ent ) {
	int i, numspec;
	// if the user wants to become a spectator, make sure he doesn't
	// exceed max_spectators
	if( ent->client->pers.spectator ) {
		Str value = ClientServerInfo::Info_ValueForKey( ent->client->pers.userinfo, "spectator" );
		if( spectator_password.GetString( ) != "none" && spectator_password.GetString( ) != value ) {
				Server::PF_cprintf( ent, PRINT_HIGH, "Spectator password incorrect.\n" );
				ent->client->pers.spectator = false;
				Server::PF_WriteByte( svc_stufftext );
				Server::PF_WriteString( "spectator 0\n" );
				Server::PF_Unicast( ent, true );
				return;
		}
		// count spectators
		for( i = 1, numspec = 0; i <= Common::maxclients.GetInt( ); i++ ) {
			if( g_edicts[ i ].inuse && g_edicts[ i ].client->pers.spectator )
				numspec++;
		}
		if( numspec >= maxspectators.GetInt( ) ) {
			Server::PF_cprintf( ent, PRINT_HIGH, "Server spectator limit is full." );
			ent->client->pers.spectator = false;
			// reset his spectator var
			Server::PF_WriteByte( svc_stufftext );
			Server::PF_WriteString( "spectator 0\n" );
			Server::PF_Unicast( ent, true );
			return;
		}
	} else {
		// he was a spectator and wants to join the game
		// he must have the right password
		Str value = ClientServerInfo::Info_ValueForKey( ent->client->pers.userinfo, "password" );
		if( password.GetString( ) != "none" && password.GetString( ) != value ) {

				Server::PF_cprintf( ent, PRINT_HIGH, "Password incorrect.\n" );
				ent->client->pers.spectator = true;
				Server::PF_WriteByte( svc_stufftext );
				Server::PF_WriteString( "spectator 1\n" );
				Server::PF_Unicast( ent, true );
				return;
		}
	}
	// clear client on respawn
	ent->client->resp.score = ent->client->pers.score = 0;
	ent->svflags &= ~SVF_NOCLIENT;
	PutClientInServer( ent );
	// add a teleportation effect
	if( !ent->client->pers.spectator ) {
		// send effect
		Server::PF_WriteByte( svc_muzzleflash );
		Server::PF_WriteShort( ( int )( ent-g_edicts ) );
		Server::PF_WriteByte( MZ_LOGIN );
		Server::SV_Multicast( ent->s.origin, MULTICAST_PVS );
		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
	}
	ent->client->respawn_time = level.time;
	if( ent->client->pers.spectator )
		Server::SV_BroadcastPrintf( PRINT_HIGH, "%s has moved to the sidelines\n", ent->client->pers.netname.c_str( ) );
	else
		Server::SV_BroadcastPrintf( PRINT_HIGH, "%s joined the game\n", ent->client->pers.netname.c_str( ) );
}

void Game::PutClientInServer( Entity * ent ) {
	Vec3 mins = Vec3( -16, -16, -24 );
	Vec3 maxs = Vec3( 16, 16, 32 );
	Vec3 spawn_origin, spawn_angles;
	gclient_t * client;
	int i;
	client_persistant_t saved;
	client_respawn_t resp;
	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	SelectSpawnPoint( ent, spawn_origin, spawn_angles );
	client = ent->client;
	// deathmatch wipes most client data every spawn
	if( deathmatch.GetBool( ) ) {
		Str userinfo;

		resp = client->resp;
		userinfo = client->pers.userinfo;
		InitClientPersistant( client );
		ClientUserinfoChanged( ent, userinfo );
	} else if( coop.GetBool( ) ) {
		//		int			n;
		Str userinfo;
		resp = client->resp;
		userinfo = client->pers.userinfo;
		// this is kind of ugly, but it's how we want to handle keys in coop
		//		for( n = 0; n < game.num_items; n++ )
		//		{
		//			if( itemlist[ n ].flags & IT_KEY )
		//				resp.coop_respawn.inventory[ n ] = client->pers.inventory[ n ];
		//		}
		resp.coop_respawn.game_helpchanged = client->pers.game_helpchanged;
		resp.coop_respawn.helpchanged = client->pers.helpchanged;
		client->pers = resp.coop_respawn;
		ClientUserinfoChanged( ent, userinfo );
		if( resp.score > client->pers.score ) client->pers.score = resp.score;
	} else {
		resp.Clear( );
	}
	// clear everything but the persistant data
	saved = client->pers;
	client->Clear( );
	client->pers = saved;
	if( client->pers.health <= 0 )
		InitClientPersistant( client );
	client->resp = resp;
	// copy some data from the client to the entity
	FetchClientEntData( ent );
	// clear entity values
	ent->groundentity = NULL;
	ent->client = client;
	ent->takedamage = DAMAGE_AIM;
	ent->movetype = MOVETYPE_WALK;
	ent->viewheight = 22;
	ent->inuse = true;
	ent->classname = "player";
	ent->mass = 200;
	ent->solid = SOLID_BBOX;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->model = "players/male/tris.md2";
	ent->pain = player_pain;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags &= ~SVF_DEADMONSTER;
	ent->mins = mins;
	ent->maxs = maxs;
	ent->velocity = vec3_origin;
	// clear playerstate values
	ent->client->ps.Clear( );
	client->ps.pmove.origin[ 0 ] = ( short )( spawn_origin[ 0 ] * 8.0f );
	client->ps.pmove.origin[ 1 ] = ( short )( spawn_origin[ 1 ] * 8.0f );
	client->ps.pmove.origin[ 2 ] = ( short )( spawn_origin[ 2 ] * 8.0f );
	if( deathmatch.GetBool( ) &&( dmflags.GetInt( ) & DF_FIXED_FOV ) )
		client->ps.fov = 90;
	else {
		client->ps.fov = ( float )atoi( ClientServerInfo::Info_ValueForKey( client->pers.userinfo, "fov" ) );
		if( client->ps.fov < 1 )
			client->ps.fov = 90;
		else if( client->ps.fov > 160 )
			client->ps.fov = 160;
	}
	client->ps.gunindex = Server::SV_ModelIndex( client->pers.weapon->view_model );
	// clear entity state values
	ent->s.effects = 0;
	ent->s.modelindex = 255; // will use the skin specified model
	ent->s.modelindex2 = 255; // custom gun model
	// sknum is player num and weapon number
	// weapon number will be added in changeweapon
	ent->s.skinnum = ( int )( ent - g_edicts ) - 1;
	ent->s.frame = 0;
	ent->s.origin = spawn_origin;
	ent->s.origin[ 2 ] += 1; // make sure off ground
	ent->s.old_origin = ent->s.origin;
	// set the delta angle
	for( i = 0; i<3; i++ )
		client->ps.pmove.delta_angles[ i ] = ANGLE2SHORT( spawn_angles[ i ] - client->resp.cmd_angles[ i ] );
	ent->s.angles[ PITCH ] = 0;
	ent->s.angles[ YAW ] = spawn_angles[ YAW ];
	ent->s.angles[ ROLL ] = 0;
	client->ps.viewangles = ent->s.angles;
	client->v_angle = ent->s.angles;
	// spawn a spectator
	if( client->pers.spectator ) {
		client->chase_target = NULL;
		client->resp.spectator = true;
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->ps.gunindex = 0;
		Server::SV_LinkEdict( ent );
		return;
	} else
		client->resp.spectator = false;
	if( !KillBox( ent ) ) { // could't spawn in?
	}
	Server::SV_LinkEdict( ent );
	// force the current weapon up
	client->newweapon = client->pers.weapon;
	ChangeWeapon( ent );
}

void Game::ClientBeginDeathmatch( Entity * ent ) {
	G_InitEdict( ent );
	InitClientResp( ent->client );
	// locate ent at a spawn point
	PutClientInServer( ent );
	if( level.intermissiontime )
		MoveClientToIntermission( ent );
	else {
		// send effect
		Server::PF_WriteByte( svc_muzzleflash );
		Server::PF_WriteShort( ( int )( ent-g_edicts ) );
		Server::PF_WriteByte( MZ_LOGIN );
		Server::SV_Multicast( ent->s.origin, MULTICAST_PVS );
	}
	Server::SV_BroadcastPrintf( PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname.c_str( ) );
	// make sure all view stuff is valid
	ClientEndServerFrame( ent );
}

void Game::ClientBegin( Entity * ent ) {
	int i;
	//ent->client = clients +( ent - g_edicts - 1 );
	if( deathmatch.GetBool( ) ) {
		ClientBeginDeathmatch( ent );
		return;
	}
	// if there is already a body waiting for us( a loadgame ), just
	// take it, otherwise spawn one from scratch
	if( ent->inuse == true ) {
		// the client has cleared the client side viewangles upon
		// connecting to the server, which is different than the
		// state when the game is saved, so we need to compensate
		// with deltaangles
		for( i = 0; i<3; i++ )
			ent->client->ps.pmove.delta_angles[ i ] = ANGLE2SHORT( ent->client->ps.viewangles[ i ] );
	} else {
		// a spawn point will completely reinitialize the entity
		// except for the persistant data that was initialized at
		// ClientConnect( ) time
		G_InitEdict( ent );
		ent->classname = "player";
		InitClientResp( ent->client );
		PutClientInServer( ent );
	}
	if( level.intermissiontime )
		MoveClientToIntermission( ent );
	else {
		// send effect if in a multiplayer game
		if( maxClients > 1 ) {
			Server::PF_WriteByte( svc_muzzleflash );
			Server::PF_WriteShort( ( int )( ent-g_edicts ) );
			Server::PF_WriteByte( MZ_LOGIN );
			Server::SV_Multicast( ent->s.origin, MULTICAST_PVS );
			Server::SV_BroadcastPrintf( PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname.c_str( ) );
		}
	}
	// make sure all view stuff is valid
	ClientEndServerFrame( ent );
}

void Game::ClientUserinfoChanged( Entity * ent, Str & userinfo ) {
	Str s;
	int playernum;
	// check for malformed or illegal info strings
	if( !ClientServerInfo::Info_Validate( userinfo ) )
		userinfo = g_defplayer.GetString( );
	// set name
	s = ClientServerInfo::Info_ValueForKey( userinfo, "name" );
	ent->client->pers.netname = s;
	// set spectator
	s = ClientServerInfo::Info_ValueForKey( userinfo, "spectator" );
	// spectators are only supported in deathmatch
	if( deathmatch.GetBool( ) && * s && strcmp( s, "0" ) )
		ent->client->pers.spectator = true;
	else
		ent->client->pers.spectator = false;
	// set skin
	s = ClientServerInfo::Info_ValueForKey( userinfo, "skin" );
	playernum = ( int )( ent-g_edicts )-1;
	// combine name and skin into a configstring
	Server::PF_Configstring( CS_PLAYERSKINS + playernum, userinfo );
	// fov
	if( deathmatch.GetBool( ) &&( dmflags.GetInt( ) & DF_FIXED_FOV ) ) {
		ent->client->ps.fov = 90;
	} else {
		ent->client->ps.fov = ( float )atof( ClientServerInfo::Info_ValueForKey( userinfo, "fov" ) );
		if( ent->client->ps.fov < 1 )
			ent->client->ps.fov = 90;
		else if( ent->client->ps.fov > 160 )
			ent->client->ps.fov = 160;
	}
	// handedness
	s = ClientServerInfo::Info_ValueForKey( userinfo, "hand" );
	if( s.Length( ) )
		ent->client->pers.hand = atoi( s );
	// save off the userinfo in case we want to check something later
	ent->client->pers.userinfo = userinfo;
}

bool Game::ClientConnect( Entity * ent, Str & userinfo ) {
	// check to see if they are on the banned IP list
	Str value =  ClientServerInfo::Info_ValueForKey( userinfo, "ip" );
	if( SV_FilterPacket( value ) ) {
		ClientServerInfo::Info_SetValueForKey( userinfo, "rejmsg", "you are banned" );
		return false;
	}
	// check for a spectator
	value =  ClientServerInfo::Info_ValueForKey( userinfo, "spectator" );
	if( deathmatch.GetBool( ) && * value && strcmp( value, "0" ) ) {
		int i, numspec;
		if( spectator_password.GetString( ) != "none" && spectator_password.GetString( ) != value ) {
				 ClientServerInfo::Info_SetValueForKey( userinfo, "rejmsg", "Spectator password required or incorrect." );
				return false;
		}
		// count spectators
		for( i = numspec = 0; i < Common::maxclients.GetInt( ); i++ ) {
			if( g_edicts[ i+1 ].inuse && g_edicts[ i+1 ].client->pers.spectator )
				numspec++;
		}
		if( numspec >= maxspectators.GetInt( ) ) {
			ClientServerInfo::Info_SetValueForKey( userinfo, "rejmsg", "Server spectator limit is full." );
			return false;
		}
	} else {
		// check for a password
		value = ClientServerInfo::Info_ValueForKey( userinfo, "password" );
		if( password.GetString( ) != "none" && password.GetString( ) != value ) {
			ClientServerInfo::Info_SetValueForKey( userinfo, "rejmsg", "Password required or incorrect." );
			return false;
		}
	}
	// they can connect
	ent->client = &clients[ ent - g_edicts ];
	// if there is already a body waiting for us( a loadgame ), just
	// take it, otherwise spawn one from scratch
	if( ent->inuse == false ) {
		// clear the respawning variables
		InitClientResp( ent->client );
		if( !autosaved || !ent->client->pers.weapon )
			InitClientPersistant( ent->client );
	}
	ClientUserinfoChanged( ent, userinfo );
	if( maxClients > 1 )
		Common::Com_DPrintf( "%s connected\n", ent->client->pers.netname.c_str( ) );
	ent->svflags = 0; // make sure we start with known default
	ent->client->pers.connected = true;
	return true;
}

void Game::ClientDisconnect( Entity * ent ) {
	int playernum;
	if( !ent->client )
		return;
	Server::SV_BroadcastPrintf( PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname.c_str( ) );
	// send effect
	Server::PF_WriteByte( svc_muzzleflash );
	Server::PF_WriteShort( ( int )( ent-g_edicts ) );
	Server::PF_WriteByte( MZ_LOGOUT );
	Server::SV_Multicast( ent->s.origin, MULTICAST_PVS );
	Server::SV_UnlinkEdict( ent );
	ent->s.modelindex = 0;
	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->classname = "disconnected";
	ent->client->pers.connected = false;
	playernum = ( int )( ent-g_edicts )-1;
	Server::PF_Configstring( CS_PLAYERSKINS+playernum, "" );
}

CTrace Game::PM_trace( Vec3 & start, Vec3 & mins, Vec3 & maxs, Vec3 & end ) {
	return Server::SV_Trace( start, mins, maxs, end, pm_passent, ( pm_passent->health > 0 ) ? MASK_PLAYERSOLID : MASK_DEADSOLID );
}

unsigned Game::CheckBlock( void * b, int c ) {
	int v, i;
	v = 0;
	for( i = 0; i<c; i++ )
		v+=( ( byte * )b )[ i ];
	return v;
}

void Game::PrintPmove( pmove_t * pm ) {
	unsigned c1, c2;
	c1 = CheckBlock( &pm->s, sizeof( pm->s ) );
	c2 = CheckBlock( &pm->cmd, sizeof( pm->cmd ) );
	Common::Com_Printf( "sv %3i:%i %i\n", pm->cmd.impulse, c1, c2 );
}

void Game::ClientThink( Entity * ent, usercmd_t * ucmd ) {
	gclient_t * client;
	Entity * other;
	int i, j;
	pmove_t pm;
	level.current_entity = ent;
	client = ent->client;
	if( level.intermissiontime ) {
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
		if( level.time > level.intermissiontime + 5.0f
			&&( ucmd->buttons & BUTTON_ANY ) )
			level.exitintermission = true;
		return;
	}
	pm_passent = ent;
	if( ent->client->chase_target ) {
		client->resp.cmd_angles[ 0 ] = SHORT2ANGLE( ucmd->angles[ 0 ] );
		client->resp.cmd_angles[ 1 ] = SHORT2ANGLE( ucmd->angles[ 1 ] );
		client->resp.cmd_angles[ 2 ] = SHORT2ANGLE( ucmd->angles[ 2 ] );
	} else {
		// set up for pmove
		pm.Clear( );
		if( ent->movetype == MOVETYPE_NOCLIP )
			client->ps.pmove.pm_type = PM_SPECTATOR;
		else if( ent->s.modelindex != 255 )
			client->ps.pmove.pm_type = PM_GIB;
		else if( ent->deadflag )
			client->ps.pmove.pm_type = PM_DEAD;
		else
			client->ps.pmove.pm_type = PM_NORMAL;
		client->ps.pmove.gravity = sv_gravity.GetInt( );
		pm.s = client->ps.pmove;
		for( i = 0; i<3; i++ ) {
			pm.s.origin[ i ] = ( short )ent->s.origin[ i ] * 8;
			pm.s.velocity[ i ] = ( short )ent->velocity[ i ] * 8;
		}
		if( memcmp( &client->old_pmove, &pm.s, sizeof( pm.s ) ) ) {
			pm.snapinitial = true;
			//		Common::Com_DPrintf( "pmove changed!\n" );
		}
		pm.cmd = * ucmd;
		pm.trace = PM_trace; // adds default parms
		pm.pointcontents = Server::SV_PointContents;
		// perform a pmove
		PlayerMove::Pmove( &pm );
		// save results of pmove
		client->ps.pmove = pm.s;
		client->old_pmove = pm.s;
		for( i = 0; i<3; i++ ) {
			ent->s.origin[ i ] = ( float )pm.s.origin[ i ] * 0.125f;
			ent->velocity[ i ] = ( float )pm.s.velocity[ i ] * 0.125f;
		}
		ent->mins = pm.mins;
		ent->maxs = pm.maxs;
		client->resp.cmd_angles[ 0 ] = SHORT2ANGLE( ucmd->angles[ 0 ] );
		client->resp.cmd_angles[ 1 ] = SHORT2ANGLE( ucmd->angles[ 1 ] );
		client->resp.cmd_angles[ 2 ] = SHORT2ANGLE( ucmd->angles[ 2 ] );
		if( ent->groundentity && !pm.groundentity &&( pm.cmd.upmove >= 10 ) &&( pm.waterlevel == 0 ) ) {
			Server::PF_StartSound( ent, CHAN_VOICE, Server::SV_SoundIndex( "sounds/jump.wav" ), 1, ATTN_NORM, 0 );
			PlayerNoise( ent, ent->s.origin, PNOISE_SELF );
		}
		ent->viewheight = ( int )pm.viewheight;
		ent->waterlevel = pm.waterlevel;
		ent->watertype = pm.watertype;
		ent->groundentity = pm.groundentity;
		if( pm.groundentity )
			ent->groundentity_linkcount = pm.groundentity->linkcount;
		if( ent->deadflag ) {
			client->ps.viewangles[ ROLL ] = 40;
			client->ps.viewangles[ PITCH ] = -15;
			client->ps.viewangles[ YAW ] = client->killer_yaw;
		} else {
			client->v_angle = pm.viewangles;
			client->ps.viewangles = pm.viewangles;
		}
		Server::SV_LinkEdict( ent );
		if( ent->movetype != MOVETYPE_NOCLIP )
			G_TouchTriggers( ent );
		// touch other objects
		for( i = 0; i<pm.numtouch; i++ ) {
			other = pm.touchents[ i ];
			for( j = 0; j<i; j++ )
				if( pm.touchents[ j ] == other )
					break;
			if( j != i )
				continue; // duplicated
			if( !other->touch )
				continue;
			other->touch( other, ent, plane_origin, SURF_NULL );
		}
	}
	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;
	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;
	// fire weapon from final position if needed
	if( client->latched_buttons & BUTTON_ATTACK ) {
		if( client->resp.spectator ) {
			client->latched_buttons = 0;
			if( client->chase_target ) {
				client->chase_target = NULL;
				client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			} else
				GetChaseTarget( ent );
		} else if( !client->weapon_thunk ) {
			client->weapon_thunk = true;
			Think_Weapon( ent );
		}
	}
	if( client->resp.spectator ) {
		if( ucmd->upmove >= 10 ) {
			if( !( client->ps.pmove.pm_flags & PMF_JUMP_HELD ) ) {
				client->ps.pmove.pm_flags |= PMF_JUMP_HELD;
				if( client->chase_target )
					ChaseNext( ent );
				else
					GetChaseTarget( ent );
			}
		} else
			client->ps.pmove.pm_flags &= ~PMF_JUMP_HELD;
	}
	// update chase cam if being followed
	for( i = 1; i <= Common::maxclients.GetInt( ); i++ ) {
		other = g_edicts + i;
		if( other->inuse && other->client->chase_target == ent )
			UpdateChaseCam( other );
	}
}

void Game::ClientBeginServerFrame( Entity * ent ) {
	gclient_t * client;
	int buttonMask;
	if( level.intermissiontime )
		return;
	client = ent->client;
	if( deathmatch.GetBool( ) && client->pers.spectator != client->resp.spectator && ( level.time - client->respawn_time ) >= 5.0 ) {
		spectator_respawn( ent );
		return;
	}
	// run weapon animations if it hasn't been done by a ucmd_t
	if( !client->weapon_thunk && !client->resp.spectator )
		Think_Weapon( ent );
	else
		client->weapon_thunk = false;
	if( ent->deadflag ) {
		// wait for any button just going down
		if( level.time > client->respawn_time ) {
			// in deathmatch, only wait for attack button
			if( deathmatch.GetBool( ) )
				buttonMask = BUTTON_ATTACK;
			else
				buttonMask = -1;
			if( ( client->latched_buttons & buttonMask ) || ( deathmatch.GetBool( ) &&( dmflags.GetInt( ) & DF_FORCE_RESPAWN ) ) ) {
				respawn( ent );
				client->latched_buttons = 0;
			}
		}
		return;
	}
	// add player trail so monsters can follow
	if( !deathmatch.GetBool( ) ) {
		if( !visible( ent, PlayerTrail_LastSpot( ) ) )
			PlayerTrail_Add( ent->s.old_origin );
	}
	client->latched_buttons = 0;
}
