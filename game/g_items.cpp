#include "precompiled.h"
#pragma hdrstop

static int jacket_armor_index;
static int combat_armor_index;
static int body_armor_index;
static int power_screen_index;
static int power_shield_index;

static timeTypeReal quad_drop_timeout_hack;

//======================================================================

/*

===============

GetItemByIndex

===============

*/
GameItem * Game::GetItemByIndex( int index ) {

	if( index == 0 || index >= num_items )
		return NULL;

	return &itemlist[ index ];
}

/*

===============

FindItemByClassname



===============

*/
GameItem * Game::FindItemByClassname( const Str & classname ) {

	int i;
	GameItem * it;

	it = itemlist;
	for( i = 0; i<num_items; i++, it++ ) {

		if( !it->classname ) continue;
		if( it->classname == classname ) return it;
	}

	return NULL;
}

/*

===============

FindItem



===============

*/
GameItem * Game::FindItem( const Str & pickup_name ) {

	int i;
	GameItem * it;

	it = itemlist;
	for( i = 0; i<num_items; i++, it++ ) {

		if( it->pickup_name.IsEmpty( ) ) continue;
		if( it->pickup_name == pickup_name )
			return it;
	}

	return NULL;
}

//======================================================================

void Game::DoRespawn( Entity * ent ) {

	if( ent->team )
	{
		Entity * master;
		int count;
		int choice;

		master = ent->teammaster;

		for( count = 0, ent = master; ent; ent = ent->chain, count++ )
		;

		choice = rand( ) % count;

		for( count = 0, ent = master; count < choice; ent = ent->chain, count++ )
		;
	}

	ent->svflags &= ~SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	Server::SV_LinkEdict( ent );

	// send an effect
	ent->s.eventNum = EV_ITEM_RESPAWN;
}

void Game::SetRespawn( Entity * ent, float delay ) {

	ent->flags |= FL_RESPAWN;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_NOT;
	ent->nextthink = level.time + delay;
	ent->think = DoRespawn;
	Server::SV_LinkEdict( ent );
}


//======================================================================

bool Game::Pickup_Powerup( Entity * ent, Entity * other ) {

	int quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX( ent->item )];
	if( ( skill.GetInt( ) == 1 && quantity >= 2 ) ||( skill.GetInt( ) >= 2 && quantity >= 1 ) ) return false;

	if( ( coop.GetBool( ) ) &&( ent->item->flags & IT_STAY_COOP ) &&( quantity > 0 ) ) return false;

	other->client->pers.inventory[ITEM_INDEX( ent->item )]++;

	if( deathmatch.GetBool( ) ) {

		if( !( ent->spawnflags & DROPPED_ITEM ) )
			SetRespawn( ent, ( float )( ent->item->quantity ) );
		if( ( dmflags.GetInt( ) & DF_INSTANT_ITEMS ) ||( ( ent->item->use == Use_Quad ) &&( ent->spawnflags & DROPPED_PLAYER_ITEM ) ) )
		{
			if( ( ent->item->use == Use_Quad ) &&( ent->spawnflags & DROPPED_PLAYER_ITEM ) )
				quad_drop_timeout_hack = ( ent->nextthink - level.time ) / FRAMETIME;
			ent->item->use( other, ent->item );
		}
	}

	return true;
}

void Game::Drop_General( Entity * ent, GameItem * item ) {

	Drop_Item( ent, item );
	ent->client->pers.inventory[ITEM_INDEX( item )]--;
	ValidateSelectedItem( ent );
}


//======================================================================

bool Game::Pickup_Adrenaline( Entity * ent, Entity * other ) {

	if( !deathmatch.GetBool( ) ) other->max_health += 1;

	if( other->health < other->max_health )
		other->health = other->max_health;

	if( !( ent->spawnflags & DROPPED_ITEM ) && deathmatch.GetBool( ) ) SetRespawn( ent, ( float )( ent->item->quantity ) );

	return true;
}

bool Game::Pickup_AncientHead( Entity * ent, Entity * other ) {

	other->max_health += 2;

	if( !( ent->spawnflags & DROPPED_ITEM ) && deathmatch.GetBool( ) ) SetRespawn( ent, ( float )( ent->item->quantity ) );

	return true;
}

bool Game::Pickup_Bandolier( Entity * ent, Entity * other ) {

	GameItem * item;
	int index;

	if( other->client->pers.max_bullets < 250 )
		other->client->pers.max_bullets = 250;
	if( other->client->pers.max_shells < 150 )
		other->client->pers.max_shells = 150;
	if( other->client->pers.max_cells < 250 )
		other->client->pers.max_cells = 250;
	if( other->client->pers.max_slugs < 75 )
		other->client->pers.max_slugs = 75;

	item = FindItem( "Bullets" );
	if( item )
	{
		index = ITEM_INDEX( item );
		other->client->pers.inventory[ index ] += item->quantity;
		if( other->client->pers.inventory[ index ] > other->client->pers.max_bullets )
			other->client->pers.inventory[ index ] = other->client->pers.max_bullets;
	}

	item = FindItem( "Shells" );
	if( item )
	{
		index = ITEM_INDEX( item );
		other->client->pers.inventory[ index ] += item->quantity;
		if( other->client->pers.inventory[ index ] > other->client->pers.max_shells )
			other->client->pers.inventory[ index ] = other->client->pers.max_shells;
	}

	if( !( ent->spawnflags & DROPPED_ITEM ) && deathmatch.GetBool( ) ) SetRespawn( ent, ( float )( ent->item->quantity ) );

	return true;
}

bool Game::Pickup_Pack( Entity * ent, Entity * other ) {

	GameItem * item;
	int index;

	if( other->client->pers.max_bullets < 300 )
		other->client->pers.max_bullets = 300;
	if( other->client->pers.max_shells < 200 )
		other->client->pers.max_shells = 200;
	if( other->client->pers.max_rockets < 100 )
		other->client->pers.max_rockets = 100;
	if( other->client->pers.max_grenades < 100 )
		other->client->pers.max_grenades = 100;
	if( other->client->pers.max_cells < 300 )
		other->client->pers.max_cells = 300;
	if( other->client->pers.max_slugs < 100 )
		other->client->pers.max_slugs = 100;

	item = FindItem( "Bullets" );
	if( item )
	{
		index = ITEM_INDEX( item );
		other->client->pers.inventory[ index ] += item->quantity;
		if( other->client->pers.inventory[ index ] > other->client->pers.max_bullets )
			other->client->pers.inventory[ index ] = other->client->pers.max_bullets;
	}

	item = FindItem( "Shells" );
	if( item )
	{
		index = ITEM_INDEX( item );
		other->client->pers.inventory[ index ] += item->quantity;
		if( other->client->pers.inventory[ index ] > other->client->pers.max_shells )
			other->client->pers.inventory[ index ] = other->client->pers.max_shells;
	}

	item = FindItem( "Cells" );
	if( item )
	{
		index = ITEM_INDEX( item );
		other->client->pers.inventory[ index ] += item->quantity;
		if( other->client->pers.inventory[ index ] > other->client->pers.max_cells )
			other->client->pers.inventory[ index ] = other->client->pers.max_cells;
	}

	item = FindItem( "Grenades" );
	if( item )
	{
		index = ITEM_INDEX( item );
		other->client->pers.inventory[ index ] += item->quantity;
		if( other->client->pers.inventory[ index ] > other->client->pers.max_grenades )
			other->client->pers.inventory[ index ] = other->client->pers.max_grenades;
	}

	item = FindItem( "Rockets" );
	if( item )
	{
		index = ITEM_INDEX( item );
		other->client->pers.inventory[ index ] += item->quantity;
		if( other->client->pers.inventory[ index ] > other->client->pers.max_rockets )
			other->client->pers.inventory[ index ] = other->client->pers.max_rockets;
	}

	item = FindItem( "Slugs" );
	if( item )
	{
		index = ITEM_INDEX( item );
		other->client->pers.inventory[ index ] += item->quantity;
		if( other->client->pers.inventory[ index ] > other->client->pers.max_slugs )
			other->client->pers.inventory[ index ] = other->client->pers.max_slugs;
	}

	if( !( ent->spawnflags & DROPPED_ITEM ) && deathmatch.GetBool( ) ) SetRespawn( ent, ( float )( ent->item->quantity ) );

	return true;
}

//======================================================================

void Game::Use_Quad( Entity * ent, GameItem * item ) {

	int timeout;

	ent->client->pers.inventory[ITEM_INDEX( item )]--;
	ValidateSelectedItem( ent );

	if( quad_drop_timeout_hack )
	{
		timeout = ( int )quad_drop_timeout_hack;
		quad_drop_timeout_hack = 0;
	}
	else
	{
		timeout = 300;
	}

	if( ent->client->quad_framenum > level.framenum )
		ent->client->quad_framenum += timeout;
	else
		ent->client->quad_framenum = ( float )( level.framenum + timeout );

	Server::PF_StartSound( ent, CHAN_ITEM, Server::SV_SoundIndex( "items/damage.wav" ), 1, ATTN_NORM, 0 );
}

//======================================================================

void Game::Use_Breather( Entity * ent, GameItem * item ) {

	ent->client->pers.inventory[ITEM_INDEX( item )]--;
	ValidateSelectedItem( ent );

	if( ent->client->breather_framenum > level.framenum )
		ent->client->breather_framenum += 300;
	else
		ent->client->breather_framenum = ( float )( level.framenum + 300 );

	//	Server::PF_StartSound( ent, CHAN_ITEM, Server::SV_SoundIndex( "items/damage.wav" ), 1, ATTN_NORM, 0 );
}

//======================================================================

void Game::Use_Envirosuit( Entity * ent, GameItem * item ) {

	ent->client->pers.inventory[ITEM_INDEX( item )]--;
	ValidateSelectedItem( ent );

	if( ent->client->enviro_framenum > level.framenum )
		ent->client->enviro_framenum += 300;
	else
		ent->client->enviro_framenum = ( float )( level.framenum + 300 );

	//	Server::PF_StartSound( ent, CHAN_ITEM, Server::SV_SoundIndex( "items/damage.wav" ), 1, ATTN_NORM, 0 );
}

//======================================================================

void Game::Use_Invulnerability( Entity * ent, GameItem * item ) {

	ent->client->pers.inventory[ITEM_INDEX( item )]--;
	ValidateSelectedItem( ent );

	if( ent->client->invincible_framenum > level.framenum )
		ent->client->invincible_framenum += 300;
	else
		ent->client->invincible_framenum = ( float )( level.framenum + 300 );

	Server::PF_StartSound( ent, CHAN_ITEM, Server::SV_SoundIndex( "items/protect.wav" ), 1, ATTN_NORM, 0 );
}

//======================================================================

void Game::Use_Silencer( Entity * ent, GameItem * item ) {

	ent->client->pers.inventory[ITEM_INDEX( item )]--;
	ValidateSelectedItem( ent );
	ent->client->silencer_shots += 30;

	//	Server::PF_StartSound( ent, CHAN_ITEM, Server::SV_SoundIndex( "items/damage.wav" ), 1, ATTN_NORM, 0 );
}

//======================================================================

bool Game::Pickup_Key( Entity * ent, Entity * other ) {

	if( coop.GetBool( ) )
	{
		if( strcmp( ent->classname, "key_power_cube" ) == 0 )
		{
			if( other->client->pers.power_cubes &( ( ent->spawnflags & 0x0000FF00 )>> 8 ) ) 		return false;
			other->client->pers.inventory[ITEM_INDEX( ent->item )]++;
			other->client->pers.power_cubes |=( ( ent->spawnflags & 0x0000FF00 ) >> 8 );
		}
		else
		{
			if( other->client->pers.inventory[ITEM_INDEX( ent->item )] )
				return false;
			other->client->pers.inventory[ITEM_INDEX( ent->item )] = 1;
		}
		return true;
	}
	other->client->pers.inventory[ITEM_INDEX( ent->item )]++;
	return true;
}

//======================================================================

bool Game::Add_Ammo( Entity * ent, GameItem * item, int count ) {

	int index;
	int max;

	if( !ent->client )
		return false;

	if( item->tag == AMMO_BULLETS )
		max = ent->client->pers.max_bullets;
	else if( item->tag == AMMO_SHELLS )
		max = ent->client->pers.max_shells;
	else if( item->tag == AMMO_ROCKETS )
		max = ent->client->pers.max_rockets;
	else if( item->tag == AMMO_GRENADES )
		max = ent->client->pers.max_grenades;
	else if( item->tag == AMMO_CELLS )
		max = ent->client->pers.max_cells;
	else if( item->tag == AMMO_SLUGS )
		max = ent->client->pers.max_slugs;
	else
		return false;

	index = ITEM_INDEX( item );

	if( ent->client->pers.inventory[ index ] == max )
		return false;

	ent->client->pers.inventory[ index ] += count;

	if( ent->client->pers.inventory[ index ] > max )
		ent->client->pers.inventory[ index ] = max;

	return true;
}

bool Game::Pickup_Ammo( Entity * ent, Entity * other ) {

	int oldcount;
	int count;
	bool weapon;

	weapon =( ent->item->flags & IT_WEAPON );
	if( ( weapon ) &&( dmflags.GetInt( ) & DF_INFINITE_AMMO ) )
		count = 1000;
	else if( ent->count )
		count = ent->count;
	else
		count = ent->item->quantity;

	oldcount = other->client->pers.inventory[ITEM_INDEX( ent->item )];

	if( !Add_Ammo( other, ent->item, count ) ) return false;

	if( weapon && !oldcount )
	{
		if( other->client->pers.weapon != ent->item && ( !deathmatch.GetBool( ) || other->client->pers.weapon == FindItem( "blaster" ) ) )
			other->client->newweapon = ent->item;
	}

	if( !( ent->spawnflags &( DROPPED_ITEM | DROPPED_PLAYER_ITEM ) ) &&( deathmatch.GetBool( ) ) ) SetRespawn( ent, 30 );
	return true;
}

void Game::Drop_Ammo( Entity * ent, GameItem * item ) {

	Entity * dropped;
	int index;

	index = ITEM_INDEX( item );
	dropped = Drop_Item( ent, item );
	if( ent->client->pers.inventory[ index ] >= item->quantity )
		dropped->count = item->quantity;
	else
		dropped->count = ent->client->pers.inventory[ index ];

	if( ent->client->pers.weapon &&
		ent->client->pers.weapon->tag == AMMO_GRENADES &&
		item->tag == AMMO_GRENADES &&
		ent->client->pers.inventory[ index ] - dropped->count <= 0 ) {
			Server::PF_cprintf( ent, PRINT_HIGH, "Can't drop current weapon\n" );
			G_FreeEdict( dropped );
			return;
	}

	ent->client->pers.inventory[ index ] -= dropped->count;
	ValidateSelectedItem( ent );
}


//======================================================================

void Game::MegaHealth_think( Entity * self ) {

	if( self->owner->health > self->owner->max_health )
	{
		self->nextthink = level.time + 1;
		self->owner->health -= 1;
		return;
	}

	if( !( self->spawnflags & DROPPED_ITEM ) && deathmatch.GetBool( ) ) SetRespawn( self, 20 );
	else
		G_FreeEdict( self );
}

bool Game::Pickup_Health( Entity * ent, Entity * other ) {

	if( !( ent->style & 1 ) )
		if( other->health >= other->max_health )
			return false;

	other->health += ent->count;

	if( !( ent->style & 1 ) ) {
		if( other->health > other->max_health )
			other->health = other->max_health;
	}

	if( ent->style & 2 )
	{
		ent->think = MegaHealth_think;
		ent->nextthink = level.time + 5;
		ent->owner = other;
		ent->flags |= FL_RESPAWN;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
	}
	else
	{
		if( !( ent->spawnflags & DROPPED_ITEM ) && deathmatch.GetBool( ) ) SetRespawn( ent, 30 );
	}

	return true;
}

//======================================================================

int Game::ArmorIndex( Entity * ent ) {

	if( !ent->client )
		return 0;

	//if( ent->client->pers.inventory[ jacket_armor_index ] > 0 )
	//	return jacket_armor_index;

	//if( ent->client->pers.inventory[ combat_armor_index ] > 0 )
	//	return combat_armor_index;

	//if( ent->client->pers.inventory[ body_armor_index ] > 0 )
	//	return body_armor_index;

	return 0;
}

bool Game::Pickup_Armor( Entity * ent, Entity * other ) {

	int old_armor_index;
	gitem_armor_t * oldinfo;
	gitem_armor_t * newinfo;
	int newcount;
	float salvage;
	int salvagecount;

	// get info on new armor
	newinfo =( gitem_armor_t * )ent->item->info;

	old_armor_index = ArmorIndex( other );

	// handle armor shards specially
	if( ent->item->tag == ARMOR_SHARD )
	{
		if( !old_armor_index )
			other->client->pers.inventory[ jacket_armor_index ] = 2;
		else
			other->client->pers.inventory[ old_armor_index ] += 2;
	}

	// if player has no armor, just use it
	else if( !old_armor_index )
	{
		other->client->pers.inventory[ITEM_INDEX( ent->item )] = newinfo->base_count;
	}

	// use the better armor
	else
	{
		// get info on old armor
		if( old_armor_index == jacket_armor_index )
			oldinfo = &jacketarmor_info;
		else if( old_armor_index == combat_armor_index )
			oldinfo = &combatarmor_info;
		else //( old_armor_index == body_armor_index )
			oldinfo = &bodyarmor_info;

		if( newinfo->normal_protection > oldinfo->normal_protection )
		{
			// calc new armor values
			salvage = oldinfo->normal_protection / newinfo->normal_protection;
			salvagecount = ( int )salvage * other->client->pers.inventory[ old_armor_index ];
			newcount = newinfo->base_count + salvagecount;
			if( newcount > newinfo->max_count )
				newcount = newinfo->max_count;

			// zero count of old armor so it goes away
			other->client->pers.inventory[ old_armor_index ] = 0;

			// change armor to new item with computed value
			other->client->pers.inventory[ITEM_INDEX( ent->item )] = newcount;
		}
		else
		{
			// calc new armor values
			salvage = newinfo->normal_protection / oldinfo->normal_protection;
			salvagecount = ( int )salvage * newinfo->base_count;
			newcount = other->client->pers.inventory[ old_armor_index ] + salvagecount;
			if( newcount > oldinfo->max_count )
				newcount = oldinfo->max_count;

			// if we're already maxed out then we don't need the new armor
			if( other->client->pers.inventory[ old_armor_index ] >= newcount )
				return false;

			// update current armor value
			other->client->pers.inventory[ old_armor_index ] = newcount;
		}
	}

	if( !( ent->spawnflags & DROPPED_ITEM ) && deathmatch.GetBool( ) ) SetRespawn( ent, 20 );

	return true;
}

//======================================================================

int Game::PowerArmorType( Entity * ent ) {

	if( !ent->client )
		return POWER_ARMOR_NONE;

	if( !( ent->flags & FL_POWER_ARMOR ) ) return POWER_ARMOR_NONE;

	if( ent->client->pers.inventory[ power_shield_index ] > 0 )
		return POWER_ARMOR_SHIELD;

	if( ent->client->pers.inventory[ power_screen_index ] > 0 )
		return POWER_ARMOR_SCREEN;

	return POWER_ARMOR_NONE;
}

void Game::Use_PowerArmor( Entity * ent, GameItem * item ) {

	int index;

	if( ent->flags & FL_POWER_ARMOR )
	{
		ent->flags &= ~FL_POWER_ARMOR;
		Server::PF_StartSound( ent, CHAN_AUTO, Server::SV_SoundIndex( "misc/power2.wav" ), 1, ATTN_NORM, 0 );
	}
	else
	{
		index = ITEM_INDEX( FindItem( "cells" ) );
		if( !ent->client->pers.inventory[ index ] )
		{
			Server::PF_cprintf( ent, PRINT_HIGH, "No cells for power armor.\n" );
			return;
		}
		ent->flags |= FL_POWER_ARMOR;
		Server::PF_StartSound( ent, CHAN_AUTO, Server::SV_SoundIndex( "misc/power1.wav" ), 1, ATTN_NORM, 0 );
	}
}

bool Game::Pickup_PowerArmor( Entity * ent, Entity * other ) {

	int quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX( ent->item )];

	other->client->pers.inventory[ITEM_INDEX( ent->item )]++;

	if( deathmatch.GetBool( ) ) {

		if( !( ent->spawnflags & DROPPED_ITEM ) )
			SetRespawn( ent, ( float )( ent->item->quantity ) );
		// auto-use for DM only if we didn't already have one
		if( !quantity )
			ent->item->use( other, ent->item );
	}

	return true;
}

void Game::Drop_PowerArmor( Entity * ent, GameItem * item ) {

	if( ( ent->flags & FL_POWER_ARMOR ) &&( ent->client->pers.inventory[ITEM_INDEX( item )] == 1 ) ) Use_PowerArmor( ent, item );
	Drop_General( ent, item );
}

//======================================================================

/*

===============

Touch_Item

===============

*/
void Game::Touch_Item( Entity * ent, Entity * other, Plane & plane, int surf ) {

	bool taken;

	if( !other->client )
		return;
	if( other->health < 1 )
		return; // dead people can't pickup
	if( !ent->item->pickup )
		return; // not a grabbable item?

	taken = ent->item->pickup( ent, other );

	if( taken )
	{
		// flash the screen
		other->client->bonus_alpha = 0.25f;

		// show icon and name on status bar
		other->client->ps.stats[ STAT_PICKUP_ICON ] = Server::SV_ImageIndex( ent->item->icon );
		other->client->ps.stats[ STAT_PICKUP_STRING ] = CS_ITEMS+ITEM_INDEX( ent->item );
		other->client->pickup_msg_time = level.time + 3.0f;

		// change selected item
		if( ent->item->use )
			other->client->pers.selected_item = other->client->ps.stats[ STAT_SELECTED_ITEM ] = ITEM_INDEX( ent->item );

		if( ent->item->pickup == Pickup_Health )
		{
			if( ent->count == 2 )
				Server::PF_StartSound( other, CHAN_ITEM, Server::SV_SoundIndex( "items/s_health.wav" ), 1, ATTN_NORM, 0 );
			else if( ent->count == 10 )
				Server::PF_StartSound( other, CHAN_ITEM, Server::SV_SoundIndex( "items/n_health.wav" ), 1, ATTN_NORM, 0 );
			else if( ent->count == 25 )
				Server::PF_StartSound( other, CHAN_ITEM, Server::SV_SoundIndex( "items/l_health.wav" ), 1, ATTN_NORM, 0 );
			else //( ent->count == 100 )
				Server::PF_StartSound( other, CHAN_ITEM, Server::SV_SoundIndex( "items/m_health.wav" ), 1, ATTN_NORM, 0 );
		}
		else if( ent->item->pickup_sound )
		{
			Server::PF_StartSound( other, CHAN_ITEM, Server::SV_SoundIndex( ent->item->pickup_sound ), 1, ATTN_NORM, 0 );
		}
	}

	if( !( ent->spawnflags & ITEM_TARGETS_USED ) ) {
		G_UseTargets( ent, other );
		ent->spawnflags |= ITEM_TARGETS_USED;
	}

	if( !taken )
		return;

	if( !( ( coop.GetBool( ) ) &&( ent->item->flags & IT_STAY_COOP ) ) ||( ent->spawnflags &( DROPPED_ITEM | DROPPED_PLAYER_ITEM ) ) )
	{
		if( ent->flags & FL_RESPAWN )
			ent->flags &= ~FL_RESPAWN;
		else
			G_FreeEdict( ent );
	}
}

//======================================================================

void Game::drop_temp_touch( Entity * ent, Entity * other, Plane & plane, int surf ) {

	if( other == ent->owner )
		return;

	Touch_Item( ent, other, plane, surf );
}

void Game::drop_make_touchable( Entity * ent ) {
	ent->touch = Touch_Item;
	if( deathmatch.GetBool( ) ) {

		ent->nextthink = level.time + 29;
		ent->think = G_FreeEdict;
	}
}

Entity * Game::Drop_Item( Entity * ent, GameItem * item ) {

	Entity * dropped;
	Vec3 forward, right;
	Vec3 offset;

	dropped = G_Spawn( );

	dropped->classname = item->classname;
	dropped->item = item;
	dropped->spawnflags = DROPPED_ITEM;
	dropped->s.effects = item->world_model_flags;
	dropped->s.renderfx = RF_GLOW;
	dropped->mins = Vec3( -15, -15, -15 );
	dropped->maxs = Vec3( 15, 15, 15 );
	Server::PF_setmodel( dropped, dropped->item->world_model );
	dropped->solid = SOLID_TRIGGER;
	dropped->movetype = MOVETYPE_TOSS;
	dropped->touch = drop_temp_touch;
	dropped->owner = ent;

	if( ent->client )
	{
		CTrace trace;

		ent->client->v_angle.AngleVectors( &forward, &right, NULL );
		offset = Vec3( 24, 0, -16 );
		G_ProjectSource( ent->s.origin, offset, forward, right, dropped->s.origin );
		trace = Server::SV_Trace( ent->s.origin, dropped->mins, dropped->maxs, dropped->s.origin, ent, CONTENTS_SOLID );
		dropped->s.origin = trace.endpos;
	}
	else
	{
		ent->s.angles.AngleVectors( &forward, &right, NULL );
		dropped->s.origin = ent->s.origin;
	}

	dropped->velocity = forward * 100.0f;
	dropped->velocity[ 2 ] = 300.0f;

	dropped->think = drop_make_touchable;
	dropped->nextthink = level.time + 1;

	Server::SV_LinkEdict( dropped );

	return dropped;
}

void Game::Use_Item( Entity * ent, Entity * other, Entity * activator ) {

	ent->svflags &= ~SVF_NOCLIENT;
	ent->use = NULL;

	if( ent->spawnflags & ITEM_NO_TOUCH )
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
	}
	else
	{
		ent->solid = SOLID_TRIGGER;
		ent->touch = Touch_Item;
	}

	Server::SV_LinkEdict( ent );
}

//======================================================================

/*

================

droptofloor

================

*/
void Game::droptofloor( Entity * ent ) {

	CTrace tr;
	Vec3 dest;
	Vec3 v;

	v = Vec3( -15, -15, -15 );
	ent->mins = v;
	v = Vec3( 15, 15, 15 );
	ent->maxs = v;

	if( ent->model )
		Server::PF_setmodel( ent, ent->model );
	else
		Server::PF_setmodel( ent, ent->item->world_model );
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;
	ent->touch = Touch_Item;

	v = Vec3( 0, 0, -128 );
	dest = ent->s.origin + v;

	tr = Server::SV_Trace( ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID );
	if( tr.startsolid )
	{
		Common::Com_DPrintf( "droptofloor: %s startsolid at %s\n", ent->classname.c_str( ), ent->s.origin.ToString( ).c_str( ) );
		G_FreeEdict( ent );
		return;
	}

	ent->s.origin = tr.endpos;

	if( ent->team )
	{
		ent->flags &= ~FL_TEAMSLAVE;
		ent->chain = ent->teamchain;
		ent->teamchain = NULL;

		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		if( ent == ent->teammaster )
		{
			ent->nextthink = level.time + FRAMETIME;
			ent->think = DoRespawn;
		}
	}

	if( ent->spawnflags & ITEM_NO_TOUCH )
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
		ent->s.effects &= ~EF_ROTATE;
		ent->s.renderfx &= ~RF_GLOW;
	}

	if( ent->spawnflags & ITEM_TRIGGER_SPAWN )
	{
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		ent->use = Use_Item;
	}

	Server::SV_LinkEdict( ent );
}

/*
===============
PrecacheItem

Precaches all data needed for a given item.

This will be called for each item spawned in a level, 
and for each item in each client's inventory.
===============
*/
void Game::PrecacheItem( GameItem * it ) {

	GameItem * ammo;

	if( !it ) return;

	if( it->pickup_sound )	Server::SV_SoundIndex( it->pickup_sound );
	if( it->world_model )	Server::SV_ModelIndex( it->world_model );
	if( it->view_model )	Server::SV_ModelIndex( it->view_model );
	if( it->icon )			Server::SV_ImageIndex( it->icon );

	// parse everything for its ammo
	if( it->ammo && it->ammo[ 0 ] ) {

		ammo = FindItem( it->ammo );
		if( ammo != it ) PrecacheItem( ammo );
	}

	// parse the space seperated precache string for other items
	if( it->precaches.IsEmpty( ) ) return;

	Lexer lexer( it->precaches.c_str( ), it->precaches.Length( ), "Game::PrecacheItem", LEXFL_ALLOWBACKSLASHSTRINGCONCAT | LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );
	Token token;

	while( 1 ) {

		lexer.ReadToken( &token );

		Str ext = token.Right( 4 );

		// determine type based on extension
		if		( ext == ".md2" ) 	Server::SV_ModelIndex( token );
		else if	( ext == ".sp2" ) 	Server::SV_ModelIndex( token );
		else if	( ext == ".wav" ) 	Server::SV_SoundIndex( token );
		if		( ext == ".pcx" ) 	Server::SV_ImageIndex( token );

		if( lexer.EndOfFile( ) ) break;

		lexer.ExpectTokenType( TT_PUNCTUATION, P_COMMA, &token );
	}
}

/*

============

SpawnItem



Sets the clipping size and plants the object on the floor.



Items can't be immediately dropped to floor, because they might

be on an entity that hasn't spawned yet.

============

*/
void Game::SpawnItem( Entity * ent, GameItem * item ) {

	PrecacheItem( item );

	if( ent->spawnflags ) {

		if( strcmp( ent->classname, "key_power_cube" ) != 0 ) {

			ent->spawnflags = 0;
			Common::Com_DPrintf( "%s at %s has invalid spawnflags set\n", ent->classname.c_str( ), vtos( ent->s.origin ).c_str( ) );
		}
	}

	// some items will be prevented in deathmatch
	if( deathmatch.GetBool( ) ) {

		if( dmflags.GetInt( ) & DF_NO_ARMOR ) {

			if( item->pickup == Pickup_Armor || item->pickup == Pickup_PowerArmor ) {

				G_FreeEdict( ent );
				return;
			}
		}

		if( dmflags.GetInt( ) & DF_NO_ITEMS ) {

			if( item->pickup == Pickup_Powerup ) {

				G_FreeEdict( ent );
				return;
			}
		}

		if( dmflags.GetInt( ) & DF_NO_HEALTH ) {

			if( item->pickup == Pickup_Health || item->pickup == Pickup_Adrenaline || item->pickup == Pickup_AncientHead ) {

				G_FreeEdict( ent );
				return;
			}
		}

		if( dmflags.GetInt( ) & DF_INFINITE_AMMO ) {

			if( ( item->flags == IT_AMMO ) ||( strcmp( ent->classname, "weapon_bfg" ) == 0 ) ) {

				G_FreeEdict( ent );
				return;
			}
		}
	}

	if( coop.GetBool( ) &&( strcmp( ent->classname, "key_power_cube" ) == 0 ) ) {
		ent->spawnflags |=( 1 <<( 8 + level.power_cubes ) );
		level.power_cubes++;
	}

	// don't let them drop items that stay in a coop game
	if( ( coop.GetBool( ) ) &&( item->flags & IT_STAY_COOP ) ) {
		item->drop = NULL;
	}

	ent->item = item;
	ent->nextthink = level.time + 2.0f * FRAMETIME; // items start after other solids
	ent->think = droptofloor;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
	if( ent->model )
		Server::SV_ModelIndex( ent->model );
}

//======================================================================

/* QUAKED item_health( .3 .3 1 )( -16 -16 -16 )( 16 16 16 )

*/
void Game::SP_item_health( Entity * self ) {

	if( deathmatch.GetBool( ) &&( dmflags.GetInt( ) & DF_NO_HEALTH ) )
	{
		G_FreeEdict( self );
		return;
	}

	self->model = "models/items/healing/medium/tris.md2";
	self->count = 10;
	SpawnItem( self, FindItem( "Health" ) );
	Server::SV_SoundIndex( "items/n_health.wav" );
}

/* QUAKED item_health_small( .3 .3 1 )( -16 -16 -16 )( 16 16 16 )

*/
void Game::SP_item_health_small( Entity * self ) {

	if( deathmatch.GetBool( ) &&( dmflags.GetInt( ) & DF_NO_HEALTH ) )
	{
		G_FreeEdict( self );
		return;
	}

	self->model = "models/items/healing/stimpack/tris.md2";
	self->count = 2;
	SpawnItem( self, FindItem( "Health" ) );
	self->style = 1;
	Server::SV_SoundIndex( "items/s_health.wav" );
}

/* QUAKED item_health_large( .3 .3 1 )( -16 -16 -16 )( 16 16 16 )

*/
void Game::SP_item_health_large( Entity * self ) {

	if( deathmatch.GetBool( ) &&( dmflags.GetInt( ) & DF_NO_HEALTH ) )
	{
		G_FreeEdict( self );
		return;
	}

	self->model = "models/items/healing/large/tris.md2";
	self->count = 25;
	SpawnItem( self, FindItem( "Health" ) );
	Server::SV_SoundIndex( "items/l_health.wav" );
}

/* QUAKED item_health_mega( .3 .3 1 )( -16 -16 -16 )( 16 16 16 )

*/
void Game::SP_item_health_mega( Entity * self ) {

	if( deathmatch.GetBool( ) &&( dmflags.GetInt( ) & DF_NO_HEALTH ) )
	{
		G_FreeEdict( self );
		return;
	}

	self->model = "models/items/mega_h/tris.md2";
	self->count = 100;
	SpawnItem( self, FindItem( "Health" ) );
	Server::SV_SoundIndex( "items/m_health.wav" );
	self->style = 1|2;
}

/*

===============

SetItemNames



Called by worldspawn

===============

*/
void Game::SetItemNames( ) {

	int i;
	GameItem * it;

	for( i = 0; i<num_items; i++ )
	{
		it = &itemlist[ i ];
		Server::PF_Configstring( CS_ITEMS+i, it->pickup_name );
	}

	jacket_armor_index = ITEM_INDEX( FindItem( "Jacket Armor" ) );
	combat_armor_index = ITEM_INDEX( FindItem( "Combat Armor" ) );
	body_armor_index = ITEM_INDEX( FindItem( "Body Armor" ) );
	power_screen_index = ITEM_INDEX( FindItem( "Power Screen" ) );
	power_shield_index = ITEM_INDEX( FindItem( "Power Shield" ) );
}
