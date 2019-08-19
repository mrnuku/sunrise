#include "precompiled.h"
#pragma hdrstop

#include "m_player.h"

void Game::P_ProjectSource( gclient_t * client, const Vec3 & point, const Vec3 & distance, const Vec3 & forward, const Vec3 & right, Vec3 & result ) {
	Vec3 dist2 = distance;
	if( client->pers.hand == LEFT_HANDED )
		dist2[ 1 ] *= -1;
	else if( client->pers.hand == CENTER_HANDED )
		dist2[ 1 ] = 0;
	G_ProjectSource( point, distance, forward, right, result );
}


/*

===============

PlayerNoise



Each player can have two noise objects associated with it:

a personal noise( jumping, pain, weapon firing ), and a weapon

target noise( bullet wall impacts )



Monsters that don't directly see the player can move

to a noise in hopes of seeing the player from there.

===============

*/
void Game::PlayerNoise( Entity * who, Vec3 & where, int type ) {

	Entity * noise;

	if( type == PNOISE_WEAPON )
	{
		if( who->client->silencer_shots )
		{
			who->client->silencer_shots--;
			return;
		}
	}

	if( deathmatch.GetBool( ) )
		return;

	if( who->flags & FL_NOTARGET )
		return;


	if( !who->mynoise )
	{
		noise = G_Spawn( );
		noise->classname = "player_noise";
		noise->mins = Vec3( -8, -8, -8 );
		noise->maxs = Vec3( 8, 8, 8 );
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise = noise;

		noise = G_Spawn( );
		noise->classname = "player_noise";
		noise->mins = Vec3( -8, -8, -8 );
		noise->maxs = Vec3( 8, 8, 8 );
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise2 = noise;
	}

	if( type == PNOISE_SELF || type == PNOISE_WEAPON )
	{
		noise = who->mynoise;
		level.sound_entity = noise;
		level.sound_entity_framenum = level.framenum;
	}
	else // type == PNOISE_IMPACT
	{
		noise = who->mynoise2;
		level.sound2_entity = noise;
		level.sound2_entity_framenum = level.framenum;
	}

	noise->s.origin = where;
	noise->absmin = where - noise->maxs;
	noise->absmax = where + noise->maxs;
	noise->teleport_time = level.time;
	Server::SV_LinkEdict( noise );
}


bool Game::Pickup_Weapon( Entity * ent, Entity * other ) {

	int index;
	GameItem * ammo;

	index = ITEM_INDEX( ent->item );

	if( ( ( dmflags.GetInt( ) & DF_WEAPONS_STAY ) || coop.GetBool( ) )
		&& other->client->pers.inventory[ index ] )
	{
		if( !( ent->spawnflags &( DROPPED_ITEM | DROPPED_PLAYER_ITEM ) ) )
			return false; // leave the weapon for others to pickup
	}

	other->client->pers.inventory[ index ]++;

	if( !( ent->spawnflags & DROPPED_ITEM ) )
	{
		// give them some ammo with it
		ammo = FindItem( ent->item->ammo );
		if( dmflags.GetInt( ) & DF_INFINITE_AMMO )
			Add_Ammo( other, ammo, 1000 );
		else
			Add_Ammo( other, ammo, ammo->quantity );

		if( !( ent->spawnflags & DROPPED_PLAYER_ITEM ) )
		{
			if( deathmatch.GetBool( ) )
			{
				if( dmflags.GetInt( ) & DF_WEAPONS_STAY )
					ent->flags |= FL_RESPAWN;
				else
					SetRespawn( ent, 30 );
			}
			if( coop.GetBool( ) )
				ent->flags |= FL_RESPAWN;
		}
	}

	if( other->client->pers.weapon != ent->item &&
		( other->client->pers.inventory[ index ] == 1 ) &&
		( !deathmatch.GetBool( ) || other->client->pers.weapon == FindItem( "blaster" ) ) )
		other->client->newweapon = ent->item;

	return true;
}


/*

===============

ChangeWeapon



The old weapon has been dropped all the way, so make the new one

current

===============

*/
void Game::ChangeWeapon( Entity * ent ) {

	int i;

	if( ent->client->grenade_time )
	{
		ent->client->grenade_time = level.time;
		ent->client->weapon_sound = 0;
		weapon_grenade_fire( ent, false );
		ent->client->grenade_time = 0;
	}

	ent->client->pers.lastweapon = ent->client->pers.weapon;
	ent->client->pers.weapon = ent->client->newweapon;
	ent->client->newweapon = NULL;
	ent->client->machinegun_shots = 0;

	// set visible model
	if( ent->s.modelindex == 255 ) {
		if( ent->client->pers.weapon )
			i =( ( ent->client->pers.weapon->weapmodel & 0xFF ) << 8 );
		else
			i = 0;
		ent->s.skinnum =( ( int )( ent - g_edicts ) - 1 ) | i;
	}

	if( ent->client->pers.weapon && ent->client->pers.weapon->ammo.Length( ) )
		ent->client->ammo_index = ITEM_INDEX( FindItem( ent->client->pers.weapon->ammo ) );
	else
		ent->client->ammo_index = 0;

	if( !ent->client->pers.weapon )
	{ // dead
		ent->client->ps.gunindex = 0;
		return;
	}

	ent->client->weaponstate = WEAPON_ACTIVATING;
	ent->client->ps.gunframe = 0;
	ent->client->ps.gunindex = Server::SV_ModelIndex( ent->client->pers.weapon->view_model );

	ent->client->anim_priority = ANIM_PAIN;
	if( ent->client->ps.pmove.pm_flags & PMF_DUCKED )
	{
		ent->s.frame = FRAME_crpain1;
		ent->client->anim_end = FRAME_crpain4;
	}
	else
	{
		ent->s.frame = FRAME_pain301;
		ent->client->anim_end = FRAME_pain304;

	}
}

/*

=================

NoAmmoWeaponChange

=================

*/
void Game::NoAmmoWeaponChange( Entity * ent ) {

	if( ent->client->pers.inventory[ITEM_INDEX( FindItem( "Slugs" ) )]
	&& ent->client->pers.inventory[ITEM_INDEX( FindItem( "Railgun" ) )] )
	{
		ent->client->newweapon = FindItem( "Railgun" );
		return;
	}
	if( ent->client->pers.inventory[ITEM_INDEX( FindItem( "Cells" ) )]
	&& ent->client->pers.inventory[ITEM_INDEX( FindItem( "Hyperblaster" ) )] )
	{
		ent->client->newweapon = FindItem( "Hyperblaster" );
		return;
	}
	if( ent->client->pers.inventory[ITEM_INDEX( FindItem( "Bullets" ) )]
	&& ent->client->pers.inventory[ITEM_INDEX( FindItem( "Chaingun" ) )] )
	{
		ent->client->newweapon = FindItem( "Chaingun" );
		return;
	}
	if( ent->client->pers.inventory[ITEM_INDEX( FindItem( "Bullets" ) )]
	&& ent->client->pers.inventory[ITEM_INDEX( FindItem( "Machinegun" ) )] )
	{
		ent->client->newweapon = FindItem( "Machinegun" );
		return;
	}
	if( ent->client->pers.inventory[ITEM_INDEX( FindItem( "Shells" ) )] > 1
		&& ent->client->pers.inventory[ITEM_INDEX( FindItem( "Super Shotgun" ) )] )
	{
		ent->client->newweapon = FindItem( "Super Shotgun" );
		return;
	}
	if( ent->client->pers.inventory[ITEM_INDEX( FindItem( "Shells" ) )]
	&& ent->client->pers.inventory[ITEM_INDEX( FindItem( "Shotgun" ) )] )
	{
		ent->client->newweapon = FindItem( "Shotgun" );
		return;
	}
	ent->client->newweapon = FindItem( "Blaster" );
}

/*

=================

Think_Weapon



Called by ClientBeginServerFrame and ClientThink

=================

*/
void Game::Think_Weapon( Entity * ent ) {

	// if just died, put the weapon away
	if( ent->health < 1 )
	{
		ent->client->newweapon = NULL;
		ChangeWeapon( ent );
	}

	// call active weapon think routine
	if( ent->client->pers.weapon && ent->client->pers.weapon->weaponthink )
	{
		is_quad =( ent->client->quad_framenum > level.framenum );
		if( ent->client->silencer_shots )
			is_silenced = MZ_SILENCED;
		else
			is_silenced = 0;
		ent->client->pers.weapon->weaponthink( ent );
	}
}


/*

================

Use_Weapon



Make the weapon ready if there is ammo

================

*/
void Game::Use_Weapon( Entity * ent, GameItem * item ) {
	// see if we're already using it
	if( item == ent->client->pers.weapon )
		return;
	if( item->ammo.Length( ) && !g_select_empty.GetBool( ) && !( item->flags & IT_AMMO ) ) {
		GameItem * ammo_item = FindItem( item->ammo );
		int ammo_index = ITEM_INDEX( ammo_item );
		if( !ent->client->pers.inventory[ ammo_index ] ) {
			Server::PF_cprintf( ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name.c_str( ), item->pickup_name.c_str( ) );
			return;
		}
		if( ent->client->pers.inventory[ ammo_index ] < item->quantity ) {
			Server::PF_cprintf( ent, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickup_name.c_str( ), item->pickup_name.c_str( ) );
			return;
		}
	}
	// change to this weapon when down
	ent->client->newweapon = item;
}

/*

================

Drop_Weapon

================

*/
void Game::Drop_Weapon( Entity * ent, GameItem * item ) {

	int index;

	if( dmflags.GetInt( ) & DF_WEAPONS_STAY )
		return;

	index = ITEM_INDEX( item );
	// see if we're already using it
	if( ( ( item == ent->client->pers.weapon ) ||( item == ent->client->newweapon ) )&&( ent->client->pers.inventory[ index ] == 1 ) )
	{
		Server::PF_cprintf( ent, PRINT_HIGH, "Can't drop current weapon\n" );
		return;
	}

	Drop_Item( ent, item );
	ent->client->pers.inventory[ index ]--;
}


/*

================

Weapon_Generic



A generic function to handle the basics of weapon thinking

================

*/
void Game::Weapon_Generic( Entity * ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int * pause_frames, int * fire_frames, void( *fire )( Entity * ent ) ) {

	int n;

	if( ent->deadflag || ent->s.modelindex != 255 ) // VWep animations screw up corpses
	{
		return;
	}

	if( ent->client->weaponstate == WEAPON_DROPPING )
	{
		if( ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST )
		{
			ChangeWeapon( ent );
			return;
		}
		else if( ( FRAME_DEACTIVATE_LAST - ent->client->ps.gunframe ) == 4 )
		{
			ent->client->anim_priority = ANIM_REVERSE;
			if( ent->client->ps.pmove.pm_flags & PMF_DUCKED )
			{
				ent->s.frame = FRAME_crpain4+1;
				ent->client->anim_end = FRAME_crpain1;
			}
			else
			{
				ent->s.frame = FRAME_pain304+1;
				ent->client->anim_end = FRAME_pain301;

			}
		}

		ent->client->ps.gunframe++;
		return;
	}

	if( ent->client->weaponstate == WEAPON_ACTIVATING )
	{
		if( ent->client->ps.gunframe == FRAME_ACTIVATE_LAST )
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe =( FRAME_FIRE_LAST + 1 );
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if( ( ent->client->newweapon ) &&( ent->client->weaponstate != WEAPON_FIRING ) ) {
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe =( FRAME_IDLE_LAST + 1 );

		if( ( FRAME_DEACTIVATE_LAST -( FRAME_IDLE_LAST + 1 ) ) < 4 )
		{
			ent->client->anim_priority = ANIM_REVERSE;
			if( ent->client->ps.pmove.pm_flags & PMF_DUCKED )
			{
				ent->s.frame = FRAME_crpain4+1;
				ent->client->anim_end = FRAME_crpain1;
			}
			else
			{
				ent->s.frame = FRAME_pain304+1;
				ent->client->anim_end = FRAME_pain301;

			}
		}
		return;
	}

	if( ent->client->weaponstate == WEAPON_READY )
	{
		if( ( ( ent->client->latched_buttons|ent->client->buttons ) & BUTTON_ATTACK ) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if( ( !ent->client->ammo_index ) ||
				( ent->client->pers.inventory[ ent->client->ammo_index ] >= ent->client->pers.weapon->quantity ) )
			{
				ent->client->ps.gunframe =( FRAME_ACTIVATE_LAST + 1 );
				ent->client->weaponstate = WEAPON_FIRING;

				// start the animation
				ent->client->anim_priority = ANIM_ATTACK;
				if( ent->client->ps.pmove.pm_flags & PMF_DUCKED )
				{
					ent->s.frame = FRAME_crattak1-1;
					ent->client->anim_end = FRAME_crattak9;
				}
				else
				{
					ent->s.frame = FRAME_attack1-1;
					ent->client->anim_end = FRAME_attack8;
				}
			}
			else
			{
				if( level.time >= ent->pain_debounce_time )
				{
					Server::PF_StartSound( ent, CHAN_VOICE, Server::SV_SoundIndex( "weapons/noammo.wav" ), 1, ATTN_NORM, 0 );
					ent->pain_debounce_time = level.time + 1.0f;
				}
				NoAmmoWeaponChange( ent );
			}
		}
		else
		{
			if( ent->client->ps.gunframe == FRAME_IDLE_LAST )
			{
				ent->client->ps.gunframe =( FRAME_FIRE_LAST + 1 );
				return;
			}

			if( pause_frames )
			{
				for( n = 0; pause_frames[ n ]; n++ )
				{
					if( ent->client->ps.gunframe == pause_frames[ n ] )
					{
						if( rand( )&15 )
							return;
					}
				}
			}

			ent->client->ps.gunframe++;
			return;
		}
	}

	if( ent->client->weaponstate == WEAPON_FIRING )
	{
		for( n = 0; fire_frames[ n ]; n++ )
		{
			if( ent->client->ps.gunframe == fire_frames[ n ] )
			{
				if( ent->client->quad_framenum > level.framenum )
					Server::PF_StartSound( ent, CHAN_ITEM, Server::SV_SoundIndex( "items/damage3.wav" ), 1, ATTN_NORM, 0 );

				fire( ent );
				break;
			}
		}

		if( !fire_frames[ n ] )
			ent->client->ps.gunframe++;

		if( ent->client->ps.gunframe ==( FRAME_FIRE_LAST + 1 )+1 )
			ent->client->weaponstate = WEAPON_READY;
	}
}


/*

======================================================================



GRENADE



======================================================================

*/
void Game::weapon_grenade_fire( Entity * ent, bool held ) {

	Vec3 offset;
	Vec3 forward, right;
	Vec3 start;
	int damage = 125;
	float timer;
	int speed;
	float radius;

	radius = damage + 40.0f;
	if( is_quad )
		damage *= 4;

	offset = Vec3( 8, 8, ( float )( ent->viewheight-8 ) );
	ent->client->v_angle.AngleVectors( &forward, &right, NULL );
	P_ProjectSource( ent->client, ent->s.origin, offset, forward, right, start );

	timer = ent->client->grenade_time - level.time;
	speed = ( int )( 1600 +( 3.0f - timer ) * ( ( 3200 - 1600 ) / 3.0f ) );
	fire_grenade2( ent, start, forward, damage, speed, timer, radius, held );

	if( !( dmflags.GetInt( ) & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ ent->client->ammo_index ]--;

	ent->client->grenade_time = level.time + 1.0f;

	if( ent->deadflag || ent->s.modelindex != 255 ) // VWep animations screw up corpses
	{
		return;
	}

	if( ent->health <= 0 )
		return;

	if( ent->client->ps.pmove.pm_flags & PMF_DUCKED )
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
}

void Game::Weapon_Grenade( Entity * ent ) {

	if( ( ent->client->newweapon ) &&( ent->client->weaponstate == WEAPON_READY ) ) {
		ChangeWeapon( ent );
		return;
	}

	if( ent->client->weaponstate == WEAPON_ACTIVATING )
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 16;
		return;
	}

	if( ent->client->weaponstate == WEAPON_READY )
	{
		if( ( ( ent->client->latched_buttons|ent->client->buttons ) & BUTTON_ATTACK ) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if( ent->client->pers.inventory[ ent->client->ammo_index ] )
			{
				ent->client->ps.gunframe = 1;
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->grenade_time = 0;
			}
			else
			{
				if( level.time >= ent->pain_debounce_time )
				{
					Server::PF_StartSound( ent, CHAN_VOICE, Server::SV_SoundIndex( "weapons/noammo.wav" ), 1, ATTN_NORM, 0 );
					ent->pain_debounce_time = level.time + 1.0f;
				}
				NoAmmoWeaponChange( ent );
			}
			return;
		}

		if( ( ent->client->ps.gunframe == 29 ) ||( ent->client->ps.gunframe == 34 ) ||( ent->client->ps.gunframe == 39 ) ||( ent->client->ps.gunframe == 48 ) )
		{
			if( rand( )&15 )
				return;
		}

		if( ++ent->client->ps.gunframe > 48 )
			ent->client->ps.gunframe = 16;
		return;
	}

	if( ent->client->weaponstate == WEAPON_FIRING )
	{
		if( ent->client->ps.gunframe == 5 )
			Server::PF_StartSound( ent, CHAN_WEAPON, Server::SV_SoundIndex( "weapons/hgrena1b.wav" ), 1, ATTN_NORM, 0 );

		if( ent->client->ps.gunframe == 11 )
		{
			if( !ent->client->grenade_time )
			{
				ent->client->grenade_time = level.time + 3.2f;
				ent->client->weapon_sound = Server::SV_SoundIndex( "weapons/hgrenc1b.wav" );
			}

			// they waited too long, detonate it in their hand
			if( !ent->client->grenade_blew_up && level.time >= ent->client->grenade_time )
			{
				ent->client->weapon_sound = 0;
				weapon_grenade_fire( ent, true );
				ent->client->grenade_blew_up = true;
			}

			if( ent->client->buttons & BUTTON_ATTACK )
				return;

			if( ent->client->grenade_blew_up )
			{
				if( level.time >= ent->client->grenade_time )
				{
					ent->client->ps.gunframe = 15;
					ent->client->grenade_blew_up = false;
				}
				else
				{
					return;
				}
			}
		}

		if( ent->client->ps.gunframe == 12 )
		{
			ent->client->weapon_sound = 0;
			weapon_grenade_fire( ent, false );
		}

		if( ( ent->client->ps.gunframe == 15 ) &&( level.time < ent->client->grenade_time ) ) 	return;

		ent->client->ps.gunframe++;

		if( ent->client->ps.gunframe == 16 )
		{
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}

/*

======================================================================



GRENADE LAUNCHER



======================================================================

*/
void Game::weapon_grenadelauncher_fire( Entity * ent ) {

	Vec3 offset;
	Vec3 forward, right;
	Vec3 start;
	int damage = 120;
	float radius;

	radius = ( float )( damage+40 );
	if( is_quad )
		damage *= 4;

	offset = Vec3( 8, 8, ( float )( ent->viewheight-8 ) );
	ent->client->v_angle.AngleVectors( &forward, &right, NULL );
	P_ProjectSource( ent->client, ent->s.origin, offset, forward, right, start );

	ent->client->kick_origin = forward * -2.0f;
	ent->client->kick_angles[ 0 ] = -1.0f;

	fire_grenade( ent, start, forward, damage, 600, 2.5f, radius );

	Server::PF_WriteByte( svc_muzzleflash );
	Server::PF_WriteShort( ( int )( ent-g_edicts ) );
	Server::PF_WriteByte( MZ_GRENADE | is_silenced );
	Server::SV_Multicast( ent->s.origin, MULTICAST_PVS );

	ent->client->ps.gunframe++;

	PlayerNoise( ent, start, PNOISE_WEAPON );

	if( !( dmflags.GetInt( ) & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ ent->client->ammo_index ]--;
}

void Game::Weapon_GrenadeLauncher( Entity * ent ) {

	static int			pause_frames[] = {34, 51, 59, 0};
	static int			fire_frames[] = {6, 0};

	Weapon_Generic( ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_grenadelauncher_fire );
}

/*

======================================================================



ROCKET



======================================================================

*/
void Game::Weapon_RocketLauncher_Fire( Entity * ent ) {

	Vec3 offset, start;
	Vec3 forward, right;
	int damage;
	float damage_radius;
	int radius_damage;

	damage = 100 +( int )( random( ) * 20.0f );
	radius_damage = 120;
	damage_radius = 120;
	if( is_quad )
	{
		damage *= 4;
		radius_damage *= 4;
	}

	ent->client->v_angle.AngleVectors( &forward, &right, NULL );

	ent->client->kick_origin = forward * -2.0f;
	ent->client->kick_angles[ 0 ] = -1.0f;

	offset = Vec3( 8, 8, ( float )( ent->viewheight-8 ) );
	P_ProjectSource( ent->client, ent->s.origin, offset, forward, right, start );
	fire_rocket( ent, start, forward, damage, 650, damage_radius, radius_damage );

	// send muzzle flash
	Server::PF_WriteByte( svc_muzzleflash );
	Server::PF_WriteShort( ( int )( ent-g_edicts ) );
	Server::PF_WriteByte( MZ_ROCKET | is_silenced );
	Server::SV_Multicast( ent->s.origin, MULTICAST_PVS );

	ent->client->ps.gunframe++;

	PlayerNoise( ent, start, PNOISE_WEAPON );

	if( !( dmflags.GetInt( ) & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ ent->client->ammo_index ]--;
}

void Game::Weapon_RocketLauncher( Entity * ent ) {

	static int			pause_frames[] = {25, 33, 42, 50, 0};
	static int			fire_frames[] = {5, 0};

	Weapon_Generic( ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire );
}


/*

======================================================================



BLASTER / HYPERBLASTER



======================================================================

*/
void Game::Blaster_Fire( Entity * ent, Vec3 & g_offset, int damage, bool vhyper, int effect ) {

	Vec3 forward, right;
	Vec3 start;
	Vec3 offset;

	if( is_quad )
		damage *= 4;
	ent->client->v_angle.AngleVectors( &forward, &right, NULL );
	offset = Vec3( 24, 8, ( float )( ent->viewheight-8 ) );
	offset = offset + g_offset;
	P_ProjectSource( ent->client, ent->s.origin, offset, forward, right, start );

	ent->client->kick_origin = forward * -2.0f;
	ent->client->kick_angles[ 0 ] = -1;

	fire_blaster( ent, start, forward, damage, 1000, effect, vhyper );

	// send muzzle flash
	Server::PF_WriteByte( svc_muzzleflash );
	Server::PF_WriteShort( ( int )( ent-g_edicts ) );
	if( vhyper )
		Server::PF_WriteByte( MZ_HYPERBLASTER | is_silenced );
	else
		Server::PF_WriteByte( MZ_BLASTER | is_silenced );
	Server::SV_Multicast( ent->s.origin, MULTICAST_PVS );

	PlayerNoise( ent, start, PNOISE_WEAPON );
}


void Game::Weapon_Blaster_Fire( Entity * ent ) {

	int damage;

	if( deathmatch.GetBool( ) )
		damage = 15;
	else
		damage = 10;
	Blaster_Fire( ent, vec3_origin, damage, false, EF_BLASTER );
	ent->client->ps.gunframe++;
}

void Game::Weapon_Blaster( Entity * ent ) {

	static int			pause_frames[] = {19, 32, 0};
	static int			fire_frames[] = {5, 0};

	Weapon_Generic( ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire );
}


void Game::Weapon_HyperBlaster_Fire( Entity * ent ) {

	float rotation;
	Vec3 offset;
	int effect;
	int damage;

	ent->client->weapon_sound = Server::SV_SoundIndex( "weapons/hyprbl1a.wav" );

	if( !( ent->client->buttons & BUTTON_ATTACK ) ) {
		ent->client->ps.gunframe++;
	}
	else
	{
		if( ! ent->client->pers.inventory[ ent->client->ammo_index ] )
		{
			if( level.time >= ent->pain_debounce_time )
			{
				Server::PF_StartSound( ent, CHAN_VOICE, Server::SV_SoundIndex( "weapons/noammo.wav" ), 1, ATTN_NORM, 0 );
				ent->pain_debounce_time = level.time + 1.0f;
			}
			NoAmmoWeaponChange( ent );
		}
		else
		{
			rotation =( ent->client->ps.gunframe - 5 ) * 2* M_PI/6;
			offset[ 0 ] = -4 * sin( rotation );
			offset[ 1 ] = 0;
			offset[ 2 ] = 4 * cos( rotation );

			if( ( ent->client->ps.gunframe == 6 ) ||( ent->client->ps.gunframe == 9 ) ) 		effect = EF_HYPERBLASTER;
			else
				effect = 0;
			if( deathmatch.GetBool( ) )
				damage = 15;
			else
				damage = 20;
			Blaster_Fire( ent, offset, damage, true, effect );
			if( !( dmflags.GetInt( ) & DF_INFINITE_AMMO ) )
				ent->client->pers.inventory[ ent->client->ammo_index ]--;

			ent->client->anim_priority = ANIM_ATTACK;
			if( ent->client->ps.pmove.pm_flags & PMF_DUCKED )
			{
				ent->s.frame = FRAME_crattak1 - 1;
				ent->client->anim_end = FRAME_crattak9;
			}
			else
			{
				ent->s.frame = FRAME_attack1 - 1;
				ent->client->anim_end = FRAME_attack8;
			}
		}

		ent->client->ps.gunframe++;
		if( ent->client->ps.gunframe == 12 && ent->client->pers.inventory[ ent->client->ammo_index ] )
			ent->client->ps.gunframe = 6;
	}

	if( ent->client->ps.gunframe == 12 )
	{
		Server::PF_StartSound( ent, CHAN_AUTO, Server::SV_SoundIndex( "weapons/hyprbd1a.wav" ), 1, ATTN_NORM, 0 );
		ent->client->weapon_sound = 0;
	}

}

void Game::Weapon_HyperBlaster( Entity * ent ) {

	static int			pause_frames[] = {0};
	static int			fire_frames[] = {6, 7, 8, 9, 10, 11, 0};

	Weapon_Generic( ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_HyperBlaster_Fire );
}

/*

======================================================================



MACHINEGUN / CHAINGUN



======================================================================

*/
void Game::Machinegun_Fire( Entity * ent ) {

	int i;
	Vec3 start;
	Vec3 forward, right;
	Vec3 angles;
	int damage = 8;
	int kick = 2;
	Vec3 offset;

	if( !( ent->client->buttons & BUTTON_ATTACK ) ) {
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if( ent->client->ps.gunframe == 5 )
		ent->client->ps.gunframe = 4;
	else
		ent->client->ps.gunframe = 5;

	if( ent->client->pers.inventory[ ent->client->ammo_index ] < 1 )
	{
		ent->client->ps.gunframe = 6;
		if( level.time >= ent->pain_debounce_time )
		{
			Server::PF_StartSound( ent, CHAN_VOICE, Server::SV_SoundIndex( "weapons/noammo.wav" ), 1, ATTN_NORM, 0 );
			ent->pain_debounce_time = level.time + 1.0f;
		}
		NoAmmoWeaponChange( ent );
		return;
	}

	if( is_quad )
	{
		damage *= 4;
		kick *= 4;
	}

	for( i = 1; i<3; i++ )
	{
		ent->client->kick_origin[ i ] = crandom( ) * 0.35f;
		ent->client->kick_angles[ i ] = crandom( ) * 0.7f;
	}
	ent->client->kick_origin[ 0 ] = crandom( ) * 0.35f;
	ent->client->kick_angles[ 0 ] = ent->client->machinegun_shots * -1.5f;

	// raise the gun as it is firing
	if( !deathmatch.GetBool( ) )
	{
		ent->client->machinegun_shots++;
		if( ent->client->machinegun_shots > 9 )
			ent->client->machinegun_shots = 9;
	}

	// get start / end positions
	angles = ent->client->v_angle + ent->client->kick_angles;
	angles.AngleVectors( &forward, &right, NULL );
	offset = Vec3( 0, 8, ( float )( ent->viewheight-8 ) );
	P_ProjectSource( ent->client, ent->s.origin, offset, forward, right, start );
	fire_bullet( ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN );

	Server::PF_WriteByte( svc_muzzleflash );
	Server::PF_WriteShort( ( int )( ent-g_edicts ) );
	Server::PF_WriteByte( MZ_MACHINEGUN | is_silenced );
	Server::SV_Multicast( ent->s.origin, MULTICAST_PVS );

	PlayerNoise( ent, start, PNOISE_WEAPON );

	if( !( dmflags.GetInt( ) & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ ent->client->ammo_index ]--;

	ent->client->anim_priority = ANIM_ATTACK;
	if( ent->client->ps.pmove.pm_flags & PMF_DUCKED )
	{
		ent->s.frame = FRAME_crattak1 -( int )( random( )+0.25f );
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 -( int )( random( )+0.25f );
		ent->client->anim_end = FRAME_attack8;
	}
}

void Game::Weapon_Machinegun( Entity * ent ) {

	static int			pause_frames[] = {23, 45, 0};
	static int			fire_frames[] = {4, 5, 0};

	Weapon_Generic( ent, 3, 5, 45, 49, pause_frames, fire_frames, Machinegun_Fire );
}

void Game::Chaingun_Fire( Entity * ent ) {

	int i;
	int shots;
	Vec3 start;
	Vec3 forward, right, up;
	float r, u;
	Vec3 offset;
	int damage;
	int kick = 2;

	if( deathmatch.GetBool( ) )
		damage = 6;
	else
		damage = 8;

	if( ent->client->ps.gunframe == 5 )
		Server::PF_StartSound( ent, CHAN_AUTO, Server::SV_SoundIndex( "weapons/chngnu1a.wav" ), 1, ATTN_IDLE, 0 );

	if( ( ent->client->ps.gunframe == 14 ) && !( ent->client->buttons & BUTTON_ATTACK ) ) {
		ent->client->ps.gunframe = 32;
		ent->client->weapon_sound = 0;
		return;
	}
	else if( ( ent->client->ps.gunframe == 21 ) &&( ent->client->buttons & BUTTON_ATTACK )
		&& ent->client->pers.inventory[ ent->client->ammo_index ] )
	{
		ent->client->ps.gunframe = 15;
	}
	else
	{
		ent->client->ps.gunframe++;
	}

	if( ent->client->ps.gunframe == 22 )
	{
		ent->client->weapon_sound = 0;
		Server::PF_StartSound( ent, CHAN_AUTO, Server::SV_SoundIndex( "weapons/chngnd1a.wav" ), 1, ATTN_IDLE, 0 );
	}
	else
	{
		ent->client->weapon_sound = Server::SV_SoundIndex( "weapons/chngnl1a.wav" );
	}

	ent->client->anim_priority = ANIM_ATTACK;
	if( ent->client->ps.pmove.pm_flags & PMF_DUCKED )
	{
		ent->s.frame = FRAME_crattak1 -( ent->client->ps.gunframe & 1 );
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 -( ent->client->ps.gunframe & 1 );
		ent->client->anim_end = FRAME_attack8;
	}

	if( ent->client->ps.gunframe <= 9 )
		shots = 1;
	else if( ent->client->ps.gunframe <= 14 )
	{
		if( ent->client->buttons & BUTTON_ATTACK )
			shots = 2;
		else
			shots = 1;
	}
	else
		shots = 3;

	if( ent->client->pers.inventory[ ent->client->ammo_index ] < shots )
		shots = ent->client->pers.inventory[ ent->client->ammo_index ];

	if( !shots )
	{
		if( level.time >= ent->pain_debounce_time )
		{
			Server::PF_StartSound( ent, CHAN_VOICE, Server::SV_SoundIndex( "weapons/noammo.wav" ), 1, ATTN_NORM, 0 );
			ent->pain_debounce_time = level.time + 1.0f;
		}
		NoAmmoWeaponChange( ent );
		return;
	}

	if( is_quad )
	{
		damage *= 4;
		kick *= 4;
	}

	for( i = 0; i<3; i++ )
	{
		ent->client->kick_origin[ i ] = crandom( ) * 0.35f;
		ent->client->kick_angles[ i ] = crandom( ) * 0.7f;
	}

	for( i = 0; i<shots; i++ )
	{
		// get start / end positions
		ent->client->v_angle.AngleVectors( &forward, &right, &up );
		r = 7 + crandom( )* 4;
		u = crandom( )* 4;
		offset = Vec3( 0, r, u + ent->viewheight-8 );
		P_ProjectSource( ent->client, ent->s.origin, offset, forward, right, start );

		fire_bullet( ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_CHAINGUN );
	}

	// send muzzle flash
	Server::PF_WriteByte( svc_muzzleflash );
	Server::PF_WriteShort( ( int )( ent-g_edicts ) );
	Server::PF_WriteByte( ( MZ_CHAINGUN1 + shots - 1 ) | is_silenced );
	Server::SV_Multicast( ent->s.origin, MULTICAST_PVS );

	PlayerNoise( ent, start, PNOISE_WEAPON );

	if( !( dmflags.GetInt( ) & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ ent->client->ammo_index ] -= shots;
}


void Game::Weapon_Chaingun( Entity * ent ) {

	static int			pause_frames[] = {38, 43, 51, 61, 0};
	static int			fire_frames[] = {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

	Weapon_Generic( ent, 4, 31, 61, 64, pause_frames, fire_frames, Chaingun_Fire );
}


/*

======================================================================



SHOTGUN / SUPERSHOTGUN



======================================================================

*/
void Game::weapon_shotgun_fire( Entity * ent ) {

	Vec3 start;
	Vec3 forward, right;
	Vec3 offset;
	int damage = 4;
	int kick = 8;

	if( ent->client->ps.gunframe == 9 )
	{
		ent->client->ps.gunframe++;
		return;
	}

	ent->client->v_angle.AngleVectors( &forward, &right, NULL );

	ent->client->kick_origin = forward * -2.0f;
	ent->client->kick_angles[ 0 ] = -2.0f;

	offset = Vec3( 0, 8, ( float )( ent->viewheight-8 ) );
	P_ProjectSource( ent->client, ent->s.origin, offset, forward, right, start );

	if( is_quad )
	{
		damage *= 4;
		kick *= 4;
	}

	if( deathmatch.GetBool( ) )
		fire_shotgun( ent, start, forward, damage, kick, 500, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN );
	else
		fire_shotgun( ent, start, forward, damage, kick, 500, 500, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN );

	// send muzzle flash
	Server::PF_WriteByte( svc_muzzleflash );
	Server::PF_WriteShort( ( int )( ent-g_edicts ) );
	Server::PF_WriteByte( MZ_SHOTGUN | is_silenced );
	Server::SV_Multicast( ent->s.origin, MULTICAST_PVS );

	ent->client->ps.gunframe++;
	PlayerNoise( ent, start, PNOISE_WEAPON );

	if( !( dmflags.GetInt( ) & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ ent->client->ammo_index ]--;
}

void Game::Weapon_Shotgun( Entity * ent ) {

	static int			pause_frames[] = {22, 28, 34, 0};
	static int			fire_frames[] = {8, 9, 0};

	Weapon_Generic( ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_shotgun_fire );
}


void Game::weapon_supershotgun_fire( Entity * ent ) {

	Vec3 start;
	Vec3 forward, right;
	Vec3 offset;
	Vec3 v;
	int damage = 6;
	int kick = 12;

	ent->client->v_angle.AngleVectors( &forward, &right, NULL );

	ent->client->kick_origin = forward * -2.0f;
	ent->client->kick_angles[ 0 ] = -2.0f;

	offset = Vec3( 0, 8, ( float )( ent->viewheight-8 ) );
	P_ProjectSource( ent->client, ent->s.origin, offset, forward, right, start );

	if( is_quad )
	{
		damage *= 4;
		kick *= 4;
	}

	v[ PITCH ] = ent->client->v_angle[ PITCH ];
	v[ YAW ] = ent->client->v_angle[ YAW ] - 5;
	v[ ROLL ] = ent->client->v_angle[ ROLL ];
	v.AngleVectors( &forward, NULL, NULL );
	fire_shotgun( ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN );
	v[ YAW ] = ent->client->v_angle[ YAW ] + 5;
	v.AngleVectors( &forward, NULL, NULL );
	fire_shotgun( ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN );

	// send muzzle flash
	Server::PF_WriteByte( svc_muzzleflash );
	Server::PF_WriteShort( ( int )( ent-g_edicts ) );
	Server::PF_WriteByte( MZ_SSHOTGUN | is_silenced );
	Server::SV_Multicast( ent->s.origin, MULTICAST_PVS );

	ent->client->ps.gunframe++;
	PlayerNoise( ent, start, PNOISE_WEAPON );

	if( !( dmflags.GetInt( ) & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ ent->client->ammo_index ] -= 2;
}

void Game::Weapon_SuperShotgun( Entity * ent ) {

	static int			pause_frames[] = {29, 42, 57, 0};
	static int			fire_frames[] = {7, 0};

	Weapon_Generic( ent, 6, 17, 57, 61, pause_frames, fire_frames, weapon_supershotgun_fire );
}



/*

======================================================================



RAILGUN



======================================================================

*/
void Game::weapon_railgun_fire( Entity * ent ) {

	Vec3 start;
	Vec3 forward, right;
	Vec3 offset;
	int damage;
	int kick;

	if( deathmatch.GetBool( ) )
	{ // normal damage is too extreme in dm
		damage = 100;
		kick = 200;
	}
	else
	{
		damage = 150;
		kick = 250;
	}

	if( is_quad )
	{
		damage *= 4;
		kick *= 4;
	}

	ent->client->v_angle.AngleVectors( &forward, &right, NULL );

	ent->client->kick_origin = forward * -3.0f;
	ent->client->kick_angles[ 0 ] = -3.0f;

	offset = Vec3( 0, 7, ( float )( ent->viewheight-8 ) );
	P_ProjectSource( ent->client, ent->s.origin, offset, forward, right, start );
	fire_rail( ent, start, forward, damage, kick );

	// send muzzle flash
	Server::PF_WriteByte( svc_muzzleflash );
	Server::PF_WriteShort( ( int )( ent-g_edicts ) );
	Server::PF_WriteByte( MZ_RAILGUN | is_silenced );
	Server::SV_Multicast( ent->s.origin, MULTICAST_PVS );

	ent->client->ps.gunframe++;
	PlayerNoise( ent, start, PNOISE_WEAPON );

	if( !( dmflags.GetInt( ) & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ ent->client->ammo_index ]--;
}


void Game::Weapon_Railgun( Entity * ent ) {

	static int			pause_frames[] = {56, 0};
	static int			fire_frames[] = {4, 0};

	Weapon_Generic( ent, 3, 18, 56, 61, pause_frames, fire_frames, weapon_railgun_fire );
}


/*

======================================================================



BFG10K



======================================================================

*/
void Game::weapon_bfg_fire( Entity * ent ) {

	Vec3 offset, start;
	Vec3 forward, right;
	int damage;
	float damage_radius = 1000;

	if( deathmatch.GetBool( ) )
		damage = 200;
	else
		damage = 500;

	if( ent->client->ps.gunframe == 9 )
	{
		// send muzzle flash
		Server::PF_WriteByte( svc_muzzleflash );
		Server::PF_WriteShort( ( int )( ent-g_edicts ) );
		Server::PF_WriteByte( MZ_BFG | is_silenced );
		Server::SV_Multicast( ent->s.origin, MULTICAST_PVS );

		ent->client->ps.gunframe++;

		PlayerNoise( ent, start, PNOISE_WEAPON );
		return;
	}

	// cells can go down during windup( from power armor hits ), so
	// check again and abort firing if we don't have enough now
	if( ent->client->pers.inventory[ ent->client->ammo_index ] < 50 )
	{
		ent->client->ps.gunframe++;
		return;
	}

	if( is_quad )
		damage *= 4;

	ent->client->v_angle.AngleVectors( &forward, &right, NULL );

	ent->client->kick_origin = forward * -2.0f;

	// make a big pitch kick with an inverse fall
	ent->client->v_dmg_pitch = -40;
	ent->client->v_dmg_roll = crandom( )* 8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	offset = Vec3( 8, 8, ( float )( ent->viewheight-8 ) );
	P_ProjectSource( ent->client, ent->s.origin, offset, forward, right, start );
	fire_bfg( ent, start, forward, damage, 400, damage_radius );

	ent->client->ps.gunframe++;

	PlayerNoise( ent, start, PNOISE_WEAPON );

	if( !( dmflags.GetInt( ) & DF_INFINITE_AMMO ) ) ent->client->pers.inventory[ ent->client->ammo_index ] -= 50;
}

void Game::Weapon_BFG( Entity * ent ) {

	static int			pause_frames[] = {39, 45, 50, 55, 0};
	static int			fire_frames[] = {9, 17, 0};

	Weapon_Generic( ent, 8, 32, 55, 58, pause_frames, fire_frames, weapon_bfg_fire );
}


//======================================================================
