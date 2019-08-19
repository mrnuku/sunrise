#include "precompiled.h"
#pragma hdrstop

/*

============

CanDamage



Returns true if the inflictor can directly damage the target.  Used for

explosions and melee attacks.

============

*/
bool Game::CanDamage( Entity * targ, Entity * inflictor ) {

	Vec3 dest;
	CTrace trace;

	// bmodels need special checking because their origin is 0, 0, 0
	if( targ->movetype == MOVETYPE_PUSH )
	{
		dest = targ->absmin + targ->absmax;
		dest *= 0.5f;
		trace = Server::SV_Trace( inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID );
		if( trace.fraction == 1.0f )
			return true;
		if( trace.ent == targ )
			return true;
		return false;
	}

	trace = Server::SV_Trace( inflictor->s.origin, vec3_origin, vec3_origin, targ->s.origin, inflictor, MASK_SOLID );
	if( trace.fraction == 1.0f )
		return true;

	dest = targ->s.origin;
	dest[ 0 ] += 15.0f;
	dest[ 1 ] += 15.0f;
	trace = Server::SV_Trace( inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID );
	if( trace.fraction == 1.0f )
		return true;

	dest = targ->s.origin;
	dest[ 0 ] += 15.0f;
	dest[ 1 ] -= 15.0f;
	trace = Server::SV_Trace( inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID );
	if( trace.fraction == 1.0f )
		return true;

	dest = targ->s.origin;
	dest[ 0 ] -= 15.0f;
	dest[ 1 ] += 15.0f;
	trace = Server::SV_Trace( inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID );
	if( trace.fraction == 1.0f )
		return true;

	dest = targ->s.origin;
	dest[ 0 ] -= 15.0f;
	dest[ 1 ] -= 15.0f;
	trace = Server::SV_Trace( inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID );
	if( trace.fraction == 1.0f )
		return true;


	return false;
}


/*

============

Killed

============

*/
void Game::Killed( Entity * targ, Entity * inflictor, Entity * attacker, int damage, Vec3 & point ) {

	if( targ->health < -999 )
		targ->health = -999;

	targ->enemy = attacker;

	if( ( targ->svflags & SVF_MONSTER ) &&( targ->deadflag != DEAD_DEAD ) ) {
		//		targ->svflags |= SVF_DEADMONSTER;	// now treat as a different content type
		if( !( targ->monsterinfo.aiflags & AI_GOOD_GUY ) )
		{
			level.killed_monsters++;
			if( coop.GetBool( ) && attacker->client )
				attacker->client->resp.score++;
			// medics won't heal monsters that they kill themselves
			if( strcmp( attacker->classname, "monster_medic" ) == 0 )
				targ->owner = attacker;
		}
	}

	if( targ->movetype == MOVETYPE_PUSH || targ->movetype == MOVETYPE_STOP || targ->movetype == MOVETYPE_NONE )
	{ // doors, triggers, etc
		targ->die( targ, inflictor, attacker, damage, point );
		return;
	}

	if( ( targ->svflags & SVF_MONSTER ) &&( targ->deadflag != DEAD_DEAD ) ) {
		targ->touch = NULL;
		monster_death_use( targ );
	}

	targ->die( targ, inflictor, attacker, damage, point );
}


/*

================

SpawnDamage

================

*/
void Game::SpawnDamage( int type, Vec3 & origin, Vec3 & normal, int damage ) {

	if( damage > 255 )
		damage = 255;
	Server::PF_WriteByte( svc_temp_entity );
	Server::PF_WriteByte( type );
	//	Server::PF_WriteByte( damage );
	Server::PF_WritePos( origin );
	Server::PF_WriteDir( normal );
	Server::SV_Multicast( origin, MULTICAST_PVS );
}


/*

============

T_Damage



targ		entity that is being damaged

inflictor	entity that is causing the damage

attacker	entity that caused the inflictor to damage targ

example: targ = monster, inflictor = rocket, attacker = player



dir			direction of the attack

point		point at which the damage is being inflicted

normal		normal vector from that point

damage		amount of damage being inflicted

knockback	force to be applied against targ as a result of the damage



dflags		these flags are used to control how T_Damage works

DAMAGE_RADIUS			damage was indirect( from a nearby explosion )

DAMAGE_NO_ARMOR			armor does not protect from this damage

DAMAGE_ENERGY			damage is from an energy based weapon

DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles

DAMAGE_BULLET			damage is from a bullet( used for ricochets )

DAMAGE_NO_PROTECTION	kills godmode, armor, everything

============

*/
int Game::CheckPowerArmor( Entity * ent, Vec3 & point, Vec3 & normal, int damage, int dflags ) {

	gclient_t * client;
	int save;
	int power_armor_type;
	int index;
	int damagePerCell;
	int pa_te_type;
	int power;
	int power_used;

	if( !damage )
		return 0;

	client = ent->client;

	if( dflags & DAMAGE_NO_ARMOR )
		return 0;

	if( client )
	{
		power_armor_type = PowerArmorType( ent );
		if( power_armor_type != POWER_ARMOR_NONE )
		{
			index = ITEM_INDEX( FindItem( "Cells" ) );
			power = client->pers.inventory[ index ];
		}
	}
	else if( ent->svflags & SVF_MONSTER )
	{
		power_armor_type = ent->monsterinfo.power_armor_type;
		power = ent->monsterinfo.power_armor_power;
	}
	else
		return 0;

	if( power_armor_type == POWER_ARMOR_NONE )
		return 0;
	if( !power )
		return 0;

	if( power_armor_type == POWER_ARMOR_SCREEN )
	{
		Vec3 vec;
		float dot;
		Vec3 forward;

		// only works if damage point is in front
		ent->s.angles.AngleVectors( &forward, NULL, NULL );
		vec = point - ent->s.origin;
		vec.Normalize( );
		dot = vec * forward;
		if( dot <= 0.3f )
			return 0;

		damagePerCell = 1;
		pa_te_type = TE_SCREEN_SPARKS;
		damage = damage / 3;
	}
	else
	{
		damagePerCell = 2;
		pa_te_type = TE_SHIELD_SPARKS;
		damage =( 2 * damage ) / 3;
	}

	save = power * damagePerCell;
	if( !save )
		return 0;
	if( save > damage )
		save = damage;

	SpawnDamage( pa_te_type, point, normal, save );
	ent->powerarmor_time = level.time + 0.2f;

	power_used = save / damagePerCell;

	if( client )
		client->pers.inventory[ index ] -= power_used;
	else
		ent->monsterinfo.power_armor_power -= power_used;
	return save;
}

int Game::CheckArmor( Entity * ent, Vec3 & point, Vec3 & normal, int damage, int te_sparks, int dflags ) {

	gclient_t * client;
	int save;
	int index;
	GameItem * armor;

	if( !damage )
		return 0;

	client = ent->client;

	if( !client )
		return 0;

	if( dflags & DAMAGE_NO_ARMOR )
		return 0;

	index = ArmorIndex( ent );
	if( !index )
		return 0;

	armor = GetItemByIndex( index );

	if( dflags & DAMAGE_ENERGY )
		save = ( int )ceil( ( ( gitem_armor_t * )armor->info )->energy_protection* damage );
	else
		save = ( int )ceil( ( ( gitem_armor_t * )armor->info )->normal_protection* damage );
	if( save >= client->pers.inventory[ index ] )
		save = client->pers.inventory[ index ];

	if( !save )
		return 0;

	client->pers.inventory[ index ] -= save;
	SpawnDamage( te_sparks, point, normal, save );

	return save;
}

void Game::M_ReactToDamage( Entity * targ, Entity * attacker ) {

	if( !( attacker->client ) && !( attacker->svflags & SVF_MONSTER ) ) return;

	if( attacker == targ || attacker == targ->enemy )
		return;

	// if we are a good guy monster and our attacker is a player
	// or another good guy, do not get mad at them
	if( targ->monsterinfo.aiflags & AI_GOOD_GUY )
	{
		if( attacker->client ||( attacker->monsterinfo.aiflags & AI_GOOD_GUY ) ) 	return;
	}

	// we now know that we are not both good guys

	// if attacker is a client, get mad at them because he's good and we're not
	if( attacker->client )
	{
		targ->monsterinfo.aiflags &= ~AI_SOUND_TARGET;

		// this can only happen in coop( both new and old enemies are clients )
		// only switch if can't see the current enemy
		if( targ->enemy && targ->enemy->client )
		{
			if( visible( targ, targ->enemy ) )
			{
				targ->oldenemy = attacker;
				return;
			}
			targ->oldenemy = targ->enemy;
		}
		targ->enemy = attacker;
		if( !( targ->monsterinfo.aiflags & AI_DUCKED ) ) 	FoundTarget( targ );
		return;
	}

	// it's the same base( walk/swim/fly ) type and a different classname and it's not a tank
	//( they spray too much ), get mad at them
	if( ( ( targ->flags &( FL_FLY|FL_SWIM ) ) ==( attacker->flags &( FL_FLY|FL_SWIM ) ) ) &&
		( strcmp( targ->classname, attacker->classname ) != 0 ) &&
		( strcmp( attacker->classname, "monster_tank" ) != 0 ) &&
		( strcmp( attacker->classname, "monster_supertank" ) != 0 ) &&
		( strcmp( attacker->classname, "monster_makron" ) != 0 ) &&
		( strcmp( attacker->classname, "monster_jorg" ) != 0 ) )
	{
		if( targ->enemy && targ->enemy->client )
			targ->oldenemy = targ->enemy;
		targ->enemy = attacker;
		if( !( targ->monsterinfo.aiflags & AI_DUCKED ) ) 	FoundTarget( targ );
	}
	// if they * meant* to shoot us, then shoot back
	else if( attacker->enemy == targ )
	{
		if( targ->enemy && targ->enemy->client )
			targ->oldenemy = targ->enemy;
		targ->enemy = attacker;
		if( !( targ->monsterinfo.aiflags & AI_DUCKED ) ) 	FoundTarget( targ );
	}
	// otherwise get mad at whoever they are mad at( help our buddy ) unless it is us!
	else if( attacker->enemy && attacker->enemy != targ )
	{
		if( targ->enemy && targ->enemy->client )
			targ->oldenemy = targ->enemy;
		targ->enemy = attacker->enemy;
		if( !( targ->monsterinfo.aiflags & AI_DUCKED ) ) 	FoundTarget( targ );
	}
}

bool Game::CheckTeamDamage( Entity * targ, Entity * attacker ) {

	//FIXME make the next line real and uncomment this block
	// if( ( ability to damage a teammate == OFF ) &&( targ's team == attacker's team ) ) return false;
	return false;
}

void Game::T_Damage( Entity * targ, Entity * inflictor, Entity * attacker, Vec3 & dir, Vec3 & point, Vec3 & normal, int damage, int knockback, int dflags, int mod ) {

	gclient_t * client;
	int take;
	int save;
	int asave;
	int psave;
	int te_sparks;

	if( !targ->takedamage )
		return;

	// friendly fire avoidance
	// if enabled you can't hurt teammates( but you can hurt yourself )
	// knockback still occurs
	if( ( targ != attacker ) &&( ( deathmatch.GetBool( ) &&( dmflags.GetInt( ) &( DF_MODELTEAMS | DF_SKINTEAMS ) ) ) || coop.GetBool( ) ) ) {
		if( OnSameTeam( targ, attacker ) )
		{
			if( dmflags.GetInt( ) & DF_NO_FRIENDLY_FIRE )
				damage = 0;
			else
				mod |= MOD_FRIENDLY_FIRE;
		}
	}
	meansOfDeath = mod;

	// easy mode takes half damage
	if( !skill.GetBool( ) && deathmatch.GetBool( )== 0 && targ->client )
	{
		damage /= 2;
		if( !damage )
			damage = 1;
	}

	client = targ->client;

	if( dflags & DAMAGE_BULLET )
		te_sparks = TE_BULLET_SPARKS;
	else
		te_sparks = TE_SPARKS;

	dir.Normalize( );

	// bonus damage for suprising a monster
	if( !( dflags & DAMAGE_RADIUS ) &&( targ->svflags & SVF_MONSTER ) &&( attacker->client ) &&( !targ->enemy ) &&( targ->health > 0 ) ) damage *= 2;

	if( targ->flags & FL_NO_KNOCKBACK )
		knockback = 0;

	// figure momentum add
	if( !( dflags & DAMAGE_NO_KNOCKBACK ) ) {
		if( ( knockback ) &&( targ->movetype != MOVETYPE_NONE ) &&( targ->movetype != MOVETYPE_BOUNCE ) &&( targ->movetype != MOVETYPE_PUSH ) &&( targ->movetype != MOVETYPE_STOP ) )
		{
			Vec3 kvel;
			float mass;

			if( targ->mass < 50 )
				mass = 50;
			else
				mass = ( float )targ->mass;

			if( targ->client && attacker == targ )
				kvel = dir * ( 1600.0f * ( float )knockback / mass ); // the rocket jump hack...
			else
				kvel = dir * ( 500.0f * ( float )knockback / mass );

			targ->velocity = targ->velocity + kvel;
		}
	}

	take = damage;
	save = 0;

	// check for godmode
	if( ( targ->flags & FL_GODMODE ) && !( dflags & DAMAGE_NO_PROTECTION ) )
	{
		take = 0;
		save = damage;
		SpawnDamage( te_sparks, point, normal, save );
	}

	// check for invincibility
	if( ( client && client->invincible_framenum > level.framenum ) && !( dflags & DAMAGE_NO_PROTECTION ) ) {
		if( targ->pain_debounce_time < level.time )
		{
			Server::PF_StartSound( targ, CHAN_ITEM, Server::SV_SoundIndex( "items/protect4.wav" ), 1, ATTN_NORM, 0 );
			targ->pain_debounce_time = level.time + 2.0f;
		}
		take = 0;
		save = damage;
	}

	psave = CheckPowerArmor( targ, point, normal, take, dflags );
	take -= psave;

	asave = CheckArmor( targ, point, normal, take, te_sparks, dflags );
	take -= asave;

	//treat cheat/powerup savings the same as armor
	asave += save;

	// team damage avoidance
	if( !( dflags & DAMAGE_NO_PROTECTION ) && CheckTeamDamage( targ, attacker ) ) return;

	// do the damage
	if( take )
	{
		if( ( targ->svflags & SVF_MONSTER ) ||( client ) ) 	SpawnDamage( TE_BLOOD, point, normal, take );
		else
			SpawnDamage( te_sparks, point, normal, take );


		targ->health = targ->health - take;

		if( targ->health <= 0 )
		{
			if( ( targ->svflags & SVF_MONSTER ) ||( client ) ) 		targ->flags |= FL_NO_KNOCKBACK;
			Killed( targ, inflictor, attacker, take, point );
			return;
		}
	}

	if( targ->svflags & SVF_MONSTER )
	{
		M_ReactToDamage( targ, attacker );
		if( !( targ->monsterinfo.aiflags & AI_DUCKED ) &&( take ) )
		{
			targ->pain( targ, attacker, ( float )knockback, take );
			// nightmare mode monsters don't go into pain frames often
			if( skill.GetInt( ) == 3 )
				targ->pain_debounce_time = level.time + 5.0f;
		}
	}
	else if( client )
	{
		if( !( targ->flags & FL_GODMODE ) &&( take ) ) 	targ->pain( targ, attacker, ( float )knockback, take );
	}
	else if( take )
	{
		if( targ->pain )
			targ->pain( targ, attacker, ( float )knockback, take );
	}

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if( client )
	{
		client->damage_parmor += psave;
		client->damage_armor += asave;
		client->damage_blood += take;
		client->damage_knockback += knockback;
		client->damage_from = point;
	}
}


/*

============

T_RadiusDamage

============

*/
void Game::T_RadiusDamage( Entity * inflictor, Entity * attacker, float damage, Entity * ignore, float radius, int mod ) {

	float points;
	Entity * ent = NULL;
	Vec3 v;
	Vec3 dir;

	while( ( ent = findradius( ent, inflictor->s.origin, radius ) ) != NULL )
	{
		if( ent == ignore )
			continue;
		if( !ent->takedamage )
			continue;

		v = ent->mins + ent->maxs;
		v = ent->s.origin.Magnitude( 0.5f, v );
		v = inflictor->s.origin - v;
		points = damage - 0.5f * v.Length( );
		if( ent == attacker )
			points = points * 0.5f;
		if( points > 0 )
		{
			if( CanDamage( ent, inflictor ) )
			{
				dir = ent->s.origin - inflictor->s.origin;
				T_Damage( ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, ( int )points, ( int )points, DAMAGE_RADIUS, mod );
			}
		}
	}
}
