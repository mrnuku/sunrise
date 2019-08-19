#include "precompiled.h"
#pragma hdrstop

#include "m_player.h"

const Str Game::ClientTeam( Entity * ent ) {
	Str value;
	if( !ent->client )
		return value;
	value = ClientServerInfo::Info_ValueForKey( ent->client->pers.userinfo, Str( "skin" ) );
	if( value.Find( '/' ) != -1 )
		return value;
	if( dmflags.GetInt( ) & DF_MODELTEAMS )
		return value.Left( value.Find( '/' ) );
	return value.Right( value.Length( ) - 1 );
}

bool Game::OnSameTeam( Entity * ent1, Entity * ent2 ) {
	if( !( dmflags.GetInt( ) &( DF_MODELTEAMS | DF_SKINTEAMS ) ) )
		return false;
	if( ClientTeam( ent2 ) == ClientTeam( ent1 ) )
		return true;
	return false;
}

void Game::SelectNextItem( Entity * ent, int itflags ) {
	gclient_t * cl = ent->client;
	if( cl->chase_target ) {
		ChaseNext( ent );
		return;
	}
	// scan  for the next valid one
	for( int i = 1; i<= MAX_ITEMS; i++ ) {
		int index = ( cl->pers.selected_item + i ) % MAX_ITEMS;
		if( !cl->pers.inventory[ index ] )
			continue;
		GameItem * it = &itemlist[ index ];
		if( !it->use )
			continue;
		if( !( it->flags & itflags ) )
			continue;
		cl->pers.selected_item = index;
		return;
	}
	cl->pers.selected_item = -1;
}

void Game::SelectPrevItem( Entity * ent, int itflags ) {
	gclient_t * cl = ent->client;;
	if( cl->chase_target ) {
		ChasePrev( ent );
		return;
	}
	// scan  for the next valid one
	for( int i = 1; i<= MAX_ITEMS; i++ ) {
		int index =( cl->pers.selected_item + MAX_ITEMS - i )%MAX_ITEMS;
		if( !cl->pers.inventory[ index ] )
			continue;
		GameItem * it = &itemlist[ index ];
		if( !it->use )
			continue;
		if( !( it->flags & itflags ) )
			continue;
		cl->pers.selected_item = index;
		return;
	}
	cl->pers.selected_item = -1;
}

void Game::ValidateSelectedItem( Entity * ent ) {
	gclient_t * cl = ent->client;
	if( cl->pers.inventory[ cl->pers.selected_item ] )
		return; // valid
	SelectNextItem( ent, -1 );
}

void Game::Cmd_Give_f( Entity * ent ) {
	GameItem * it;
	int index;
	int i;
	bool give_all = false;
	Entity * it_ent;
	if( deathmatch.GetBool( ) && !sv_cheats.GetBool( ) ) {
		Server::PF_cprintf( ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n" );
		return;
	}
	Str name = Command::Cmd_Args( );
	if( name.IcmpFast( "all" ) )
		give_all = true;
	if( give_all || Command::Cmd_Argv( 1 ).IcmpFast( "health" ) ) {
		if( Command::Cmd_Argc( ) == 3 )
			ent->health = atoi( Command::Cmd_Argv( 2 ) );
		else
			ent->health = ent->max_health;
		if( !give_all )
			return;
	}
	if( give_all || name.IcmpFast( "weapons" ) ) {
		for( i = 0; i<num_items; i++ ) {
			it = itemlist + i;
			if( !it->pickup )
				continue;
			if( !( it->flags & IT_WEAPON ) )
				continue;
			ent->client->pers.inventory[ i ] += 1;
		}
		if( !give_all )
			return;
	}
	if( give_all || name.IcmpFast( "ammo" ) ) {
		for( i = 0; i<num_items; i++ ) {
			it = itemlist + i;
			if( !it->pickup )
				continue;
			if( !( it->flags & IT_AMMO ) )
				continue;
			Add_Ammo( ent, it, 1000 );
		}
		if( !give_all )
			return;
	}
	if( give_all || name.IcmpFast( "armor" ) ) {
		gitem_armor_t * info;
		it = FindItem( "Jacket Armor" );
		ent->client->pers.inventory[ITEM_INDEX( it )] = 0;
		it = FindItem( "Combat Armor" );
		ent->client->pers.inventory[ITEM_INDEX( it )] = 0;
		it = FindItem( "Body Armor" );
		info =( gitem_armor_t * )it->info;
		ent->client->pers.inventory[ITEM_INDEX( it )] = info->max_count;
		if( !give_all )
			return;
	}
	if( give_all || name.IcmpFast( "Power Shield" ) ) {
		it = FindItem( "Power Shield" );
		it_ent = G_Spawn( );
		it_ent->classname = it->classname;
		SpawnItem( it_ent, it );
		Touch_Item( it_ent, ent, plane_origin, SURF_NULL );
		if( it_ent->inuse )
			G_FreeEdict( it_ent );
		if( !give_all )
			return;
	}
	if( give_all ) {
		for( i = 0; i<num_items; i++ ) {
			it = itemlist + i;
			if( !it->pickup )
				continue;
			if( it->flags &( IT_ARMOR|IT_WEAPON|IT_AMMO ) ) 		continue;
			ent->client->pers.inventory[ i ] = 1;
		}
		return;
	}
	it = FindItem( name );
	if( !it ) {
		name = Command::Cmd_Argv( 1 );
		it = FindItem( name );
		if( !it ) {
			Server::PF_cprintf( ent, PRINT_HIGH, "unknown item\n" );
			return;
		}
	}
	if( !it->pickup ) {
		Server::PF_cprintf( ent, PRINT_HIGH, "non-pickup item\n" );
		return;
	}
	index = ITEM_INDEX( it );
	if( it->flags & IT_AMMO ) {
		if( Command::Cmd_Argc( ) == 3 )
			ent->client->pers.inventory[ index ] = atoi( Command::Cmd_Argv( 2 ) );
		else
			ent->client->pers.inventory[ index ] += it->quantity;
	} else {
		it_ent = G_Spawn( );
		it_ent->classname = it->classname;
		SpawnItem( it_ent, it );
		Touch_Item( it_ent, ent, plane_origin, 0 );
		if( it_ent->inuse )
			G_FreeEdict( it_ent );
	}
}

void Game::Cmd_God_f( Entity * ent ) {
	Str msg;
	if( deathmatch.GetBool( ) && !sv_cheats.GetBool( ) ) {
		Server::PF_cprintf( ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n" );
		return;
	}
	ent->flags ^= FL_GODMODE;
	if( !( ent->flags & FL_GODMODE ) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";
	Server::PF_cprintf( ent, PRINT_HIGH, msg.c_str( ) );
}

void Game::Cmd_Notarget_f( Entity * ent ) {
	Str msg;
	if( deathmatch.GetBool( ) && !sv_cheats.GetBool( ) ) {
		Server::PF_cprintf( ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n" );
		return;
	}
	ent->flags ^= FL_NOTARGET;
	if( !( ent->flags & FL_NOTARGET ) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";
	Server::PF_cprintf( ent, PRINT_HIGH, msg.c_str( ) );
}

void Game::Cmd_Noclip_f( Entity * ent ) {
	Str msg;
	if( deathmatch.GetBool( ) && !sv_cheats.GetBool( ) ) {
		Server::PF_cprintf( ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n" );
		return;
	}
	if( ent->movetype == MOVETYPE_NOCLIP ) {
		ent->movetype = MOVETYPE_WALK;
		msg = "noclip OFF\n";
	} else {
		ent->movetype = MOVETYPE_NOCLIP;
		msg = "noclip ON\n";
	}
	Server::PF_cprintf( ent, PRINT_HIGH, msg.c_str( ) );
}

void Game::Cmd_Use_f( Entity * ent ) {
	int index;
	GameItem * it;
	Str s;
	s = Command::Cmd_Args( );
	it = FindItem( s );
	if( !it ) {
		Server::PF_cprintf( ent, PRINT_HIGH, "unknown item: %s\n", s.c_str( ) );
		return;
	}
	if( !it->use ) {
		Server::PF_cprintf( ent, PRINT_HIGH, "Item is not usable.\n" );
		return;
	}
	index = ITEM_INDEX( it );
	if( !ent->client->pers.inventory[ index ] ) {
		Server::PF_cprintf( ent, PRINT_HIGH, "Out of item: %s\n", s.c_str( ) );
		return;
	}
	it->use( ent, it );
}

void Game::Cmd_Drop_f( Entity * ent ) {
	Str s = Command::Cmd_Args( );
	GameItem * it = FindItem( s );
	if( !it ) {
		Server::PF_cprintf( ent, PRINT_HIGH, "unknown item: %s\n", s.c_str( ) );
		return;
	}
	if( !it->drop ) {
		Server::PF_cprintf( ent, PRINT_HIGH, "Item is not dropable.\n" );
		return;
	}
	int index = ITEM_INDEX( it );
	if( !ent->client->pers.inventory[ index ] ) {
		Server::PF_cprintf( ent, PRINT_HIGH, "Out of item: %s\n", s.c_str( ) );
		return;
	}
	it->drop( ent, it );
}

void Game::Cmd_Inven_f( Entity * ent ) {
	int i;
	gclient_t * cl;
	cl = ent->client;
	cl->showscores = false;
	cl->showhelp = false;
	if( cl->showinventory ) {
		cl->showinventory = false;
		return;
	}
	cl->showinventory = true;
	Server::PF_WriteByte( svc_inventory );
	for( i = 0; i<MAX_ITEMS; i++ ) {
		Server::PF_WriteShort( cl->pers.inventory[ i ] );
	}
	Server::PF_Unicast( ent, true );
}

void Game::Cmd_InvUse_f( Entity * ent ) {
	GameItem * it;
	ValidateSelectedItem( ent );
	if( ent->client->pers.selected_item == -1 ) {
		Server::PF_cprintf( ent, PRINT_HIGH, "No item to use.\n" );
		return;
	}
	it = &itemlist[ ent->client->pers.selected_item ];
	if( !it->use ) {
		Server::PF_cprintf( ent, PRINT_HIGH, "Item is not usable.\n" );
		return;
	}
	it->use( ent, it );
}

void Game::Cmd_WeapPrev_f( Entity * ent ) {
	gclient_t * cl;
	int i, index;
	GameItem * it;
	int selected_weapon;
	cl = ent->client;
	if( !cl->pers.weapon )
		return;
	selected_weapon = ITEM_INDEX( cl->pers.weapon );
	// scan  for the next valid one
	for( i = 1; i<= MAX_ITEMS; i++ ) {
		index =( selected_weapon + i )%MAX_ITEMS;
		if( !cl->pers.inventory[ index ] )
			continue;
		it = &itemlist[ index ];
		if( !it->use )
			continue;
		if( !( it->flags & IT_WEAPON ) )
			continue;
		it->use( ent, it );
		if( cl->pers.weapon == it )
			return; // successful
	}
}

void Game::Cmd_WeapNext_f( Entity * ent ) {
	gclient_t * cl;
	int i, index;
	GameItem * it;
	int selected_weapon;
	cl = ent->client;
	if( !cl->pers.weapon )
		return;
	selected_weapon = ITEM_INDEX( cl->pers.weapon );
	// scan  for the next valid one
	for( i = 1; i<= MAX_ITEMS; i++ ) {
		index =( selected_weapon + MAX_ITEMS - i )%MAX_ITEMS;
		if( !cl->pers.inventory[ index ] )
			continue;
		it = &itemlist[ index ];
		if( !it->use )
			continue;
		if( !( it->flags & IT_WEAPON ) )
			continue;
		it->use( ent, it );
		if( cl->pers.weapon == it )
			return; // successful
	}
}

void Game::Cmd_WeapLast_f( Entity * ent ) {
	gclient_t * cl;
	int index;
	GameItem * it;
	cl = ent->client;
	if( !cl->pers.weapon || !cl->pers.lastweapon )
		return;
	index = ITEM_INDEX( cl->pers.lastweapon );
	if( !cl->pers.inventory[ index ] )
		return;
	it = &itemlist[ index ];
	if( !it->use )
		return;
	if( !( it->flags & IT_WEAPON ) )
		return;
	it->use( ent, it );
}

void Game::Cmd_InvDrop_f( Entity * ent ) {
	GameItem * it;
	ValidateSelectedItem( ent );
	if( ent->client->pers.selected_item == -1 ) {
		Server::PF_cprintf( ent, PRINT_HIGH, "No item to drop.\n" );
		return;
	}
	it = &itemlist[ ent->client->pers.selected_item ];
	if( !it->drop ) {
		Server::PF_cprintf( ent, PRINT_HIGH, "Item is not dropable.\n" );
		return;
	}
	it->drop( ent, it );
}

void Game::Cmd_Kill_f( Entity * ent ) {
	if( ( level.time - ent->client->respawn_time ) < 5.0 )
		return;
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;
	player_die( ent, ent, ent, 100000, vec3_origin );
}

void Game::Cmd_PutAway_f( Entity * ent ) {
	ent->client->showscores = false;
	ent->client->showhelp = false;
	ent->client->showinventory = false;
}

int Game::PlayerSort( void const * a, void const * b ) {
	int anum, bnum;
	anum = * ( int * )a;
	bnum = * ( int * )b;
	anum = clients[ anum].ps.stats[STAT_FRAGS ];
	bnum = clients[ bnum].ps.stats[STAT_FRAGS ];
	if( anum < bnum )
		return -1;
	if( anum > bnum )
		return 1;
	return 0;
}

void Game::Cmd_Players_f( Entity * ent ) {
	int i;
	int count;
	Str ssmall;
	Str large;
	int index[ 256 ];
	count = 0;
	for( i = 0; i < Common::maxclients.GetInt( ); i++ )
		if( clients[ i ].pers.connected ) {
			index[ count ] = i;
			count++;
		}
		// sort by frags
		qsort( index, count, sizeof( index[ 0 ] ), PlayerSort );
		// print information
		large[ 0 ] = 0;
		for( i = 0; i < count; i++ ) {
			sprintf( ssmall, "%3i %s\n", clients[index[ i ]].ps.stats[ STAT_FRAGS ], clients[index[ i ]].pers.netname.c_str( ) );
			large.Append( ssmall );
		}
		Server::PF_cprintf( ent, PRINT_HIGH, "%s\n%i players\n", large.c_str( ), count );
}

void Game::Cmd_Wave_f( Entity * ent ) {
	int i = atoi( Command::Cmd_Argv( 1 ) );
	// can't wave when ducked
	if( ent->client->ps.pmove.pm_flags & PMF_DUCKED )
		return;
	if( ent->client->anim_priority > ANIM_WAVE )
		return;
	ent->client->anim_priority = ANIM_WAVE;
	switch( i ) {
	case 0:
		Server::PF_cprintf( ent, PRINT_HIGH, "flipoff\n" );
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		Server::PF_cprintf( ent, PRINT_HIGH, "salute\n" );
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		Server::PF_cprintf( ent, PRINT_HIGH, "taunt\n" );
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
		break;
	case 3:
		Server::PF_cprintf( ent, PRINT_HIGH, "wave\n" );
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;
		break;
	case 4:
	default:
		Server::PF_cprintf( ent, PRINT_HIGH, "point\n" );
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;
		break;
	}
}

void Game::Cmd_Say_f( Entity * ent, bool team, bool arg0 ) {
	int i, j;
	Entity * other;
	Str text;
	gclient_t * cl;
	if( Command::Cmd_Argc( ) < 2 && !arg0 )
		return;
	if( !( dmflags.GetInt( ) &( DF_MODELTEAMS | DF_SKINTEAMS ) ) )
		team = false;
	if( team )
		sprintf( text, "( %s ): ", ent->client->pers.netname.c_str( ) );
	else
		sprintf( text, "%s: ", ent->client->pers.netname.c_str( ) );

	if( arg0 ) {
		text.Append( Command::Cmd_Argv( 0 ) );
		text.Append( " " );
		text.Append( Command::Cmd_Args( ) );
	} else {
		text = Command::Cmd_Args( );
	}
	// don't let text be too long for malicious reasons
	//if( strlen( text ) > 150 ) text[ 150 ] = 0; // FIXME LATER
	text.Append( "\n" );
	if( flood_msgs.GetBool( ) ) {
		cl = ent->client;
		if( level.time < cl->flood_locktill ) {
			Server::PF_cprintf( ent, PRINT_HIGH, "You can't talk for %d more seconds\n", ( int )( cl->flood_locktill - level.time ) );
			return;
		}
		i = cl->flood_whenhead - flood_msgs.GetInt( ) + 1;
		if( i < 0 )
			i =( sizeof( cl->flood_when )/sizeof( cl->flood_when[ 0 ] ) ) + i;
		if( cl->flood_when[ i ] && level.time - cl->flood_when[ i ] < flood_persecond.GetFloat( ) ) {
				cl->flood_locktill = level.time + flood_waitdelay.GetFloat( );
				Server::PF_cprintf( ent, PRINT_CHAT, "Flood protection:  You can't talk for %s seconds.\n", flood_waitdelay.GetString( ).c_str( ) );
				return;
		}
		cl->flood_whenhead =( cl->flood_whenhead + 1 ) % ( sizeof( cl->flood_when )/sizeof( cl->flood_when[ 0 ] ) );
		cl->flood_when[ cl->flood_whenhead ] = level.time;
	}

	if( Common::dedicated.GetBool( ) )
		Server::PF_cprintf( NULL, PRINT_CHAT, "%s", text.c_str( ) );
	for( j = 1; j <= maxClients; j++ ) {
		other = &g_edicts[ j ];
		if( !other->inuse )
			continue;
		if( !other->client )
			continue;
		if( team ) if( !OnSameTeam( ent, other ) )
			continue;
		Server::PF_cprintf( other, PRINT_CHAT, "%s", text.c_str( ) );
	}
}

void Game::Cmd_PlayerList_f( Entity * ent ) {
	int i;
	Str st;
	Str text;
	Entity * e2;
	// connect time, ping, score, name
	for( i = 0, e2 = g_edicts + 1; i < Common::maxclients.GetInt( ); i++, e2++ ) {
		if( !e2->inuse )
			continue;
		sprintf( st, "%02d:%02d %4d %3d %s%s\n", ( level.framenum - e2->client->resp.enterframe ) / 600, ( ( level.framenum - e2->client->resp.enterframe ) % 600 )/10, e2->client->ping, e2->client->resp.score, e2->client->pers.netname.c_str( ), e2->client->resp.spectator ? "( spectator )" : "" );
		text.Append( st );
	}
	Server::PF_cprintf( ent, PRINT_HIGH, "%s", text.c_str( ) );
}

void Game::ClientCommand( Entity * ent ) {
	if( !ent->client )
		return; // not fully in game yet
	Str cmd = Command::Cmd_Argv( 0 );
	if     ( cmd == "players" )
		Cmd_Players_f( ent );
	else if( cmd == "say" )
		Cmd_Say_f( ent, false, false );
	else if( cmd == "say_team" )
		Cmd_Say_f( ent, true, false );
	else if( cmd == "score" )
		Cmd_Score_f( ent );
	else if( cmd == "help" )
		Cmd_Help_f( ent );
	else {
		if( level.intermissiontime )
			return;
		if     ( cmd == "use" )
			Cmd_Use_f( ent );
		else if( cmd == "drop" )
			Cmd_Drop_f( ent );
		else if( cmd == "give" )
			Cmd_Give_f( ent );
		else if( cmd == "god" )
			Cmd_God_f( ent );
		else if( cmd == "notarget" )
			Cmd_Notarget_f( ent );
		else if( cmd == "noclip" )
			Cmd_Noclip_f( ent );
		else if( cmd == "inven" )
			Cmd_Inven_f( ent );
		else if( cmd == "invnext" )
			SelectNextItem( ent, -1 );
		else if( cmd == "invprev" )
			SelectPrevItem( ent, -1 );
		else if( cmd == "invnextw" )
			SelectNextItem( ent, IT_WEAPON );
		else if( cmd == "invprevw" )
			SelectPrevItem( ent, IT_WEAPON );
		else if( cmd == "invnextp" )
			SelectNextItem( ent, IT_POWERUP );
		else if( cmd == "invprevp" )
			SelectPrevItem( ent, IT_POWERUP );
		else if( cmd == "invuse" )
			Cmd_InvUse_f( ent );
		else if( cmd == "invdrop" )
			Cmd_InvDrop_f( ent );
		else if( cmd == "weapprev" )
			Cmd_WeapPrev_f( ent );
		else if( cmd == "weapnext" )
			Cmd_WeapNext_f( ent );
		else if( cmd == "weaplast" )
			Cmd_WeapLast_f( ent );
		else if( cmd == "kill" )
			Cmd_Kill_f( ent );
		else if( cmd == "putaway" )
			Cmd_PutAway_f( ent );
		else if( cmd == "wave" )
			Cmd_Wave_f( ent );
		else if( cmd == "playerlist" )
			Cmd_PlayerList_f( ent );
		else
			Cmd_Say_f( ent, false, true );
	}
}
