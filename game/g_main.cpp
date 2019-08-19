#include "precompiled.h"
#pragma hdrstop

int Game::maxClients;
int Game::maxEntities;
int Game::serverflags;
int Game::num_items;
bool Game::autosaved;
Entity * Game::edicts;
int Game::edict_size;
int Game::num_edicts;
int Game::max_edicts;
int Game::helpchanged;

// p_client.cpp
Entity * Game::pm_passent;

// p_trail.cpp
int Game::trail_head;
Entity * Game::trail[ TRAIL_LENGTH ];

// p_view.cpp
Entity * Game::view_current_player;
gclient_t * Game::view_current_client;
Vec3 Game::view_forward;
Vec3 Game::view_right;
Vec3 Game::view_up;
float Game::view_xyspeed;
float Game::view_bobmove;
int Game::view_bobcycle;
float Game::view_bobfracsin;

// p_weapon.cpp
byte Game::is_silenced;
bool Game::is_quad;

int Game::sm_meat_index;
int Game::snd_fry;
int Game::meansOfDeath;

void moveinfo_t::Clear( ) {
	start_origin.Zero( );
	start_angles.Zero( );
	end_origin.Zero( );
	end_angles.Zero( );
	sound_start		= 0;
	sound_middle	= 0;
	sound_end		= 0;
	accel			= 0;
	speed			= 0;
	decel			= 0;
	distance		= 0;
	wait			= 0;
	state			= 0;
	dir.Zero( );
	current_speed	= 0;
	move_speed		= 0;
	next_speed		= 0;
	remaining_distance	= 0;
	decel_distance	= 0;
	endfunc			= NULL;
}

void monsterinfo_t::Clear( ) {
	currentmove		= NULL;
	aiflags			= 0;
	nextframe		= 0;
	scale			= 0;
	stand			= NULL;
	le				= NULL;
	search			= NULL;
	walk			= NULL;
	run				= NULL;
	dodge			= NULL;
	attack			= NULL;
	melee			= NULL;
	sight			= NULL;
	checkattack		= NULL;
	pausetime		= 0;
	attack_finished	= 0;
	saved_goal.Zero( );
	search_time		= 0;
	trail_time		= 0;
	last_sighting.Zero( );
	attack_state	= 0;
	lefty			= 0;
	le_time			= 0;
	linkcount		= 0;
	power_armor_type	= 0;
	power_armor_power	= 0;
}

void Entity::Clear( ) {
	s.Clear( );
	client			= NULL;
	inuse			= false;
	linkcount		= 0;
	area.Clear( );	
	num_clusters	= 0;
	for( int i = 0; i < MAX_ENT_CLUSTERS; i++ )
		clusternums[ i ] = 0;
	headnode		= 0;
	areanum			= 0;
	areanum2		= 0;
	svflags			= 0;
	mins.Zero( );
	maxs.Zero( );
	absmin.Zero( );
	absmax.Zero( );
	size.Zero( );
	solid			= SOLID_NOT;
	clipmask		= 0;
	owner			= NULL;
	movetype		= 0;
	flags			= 0;
	model.Clear( );
	freetime		= 0;
	message.Clear( );
	classname.Clear( );
	name.Clear( );
	spawnflags		= 0;
	timestamp		= 0;
	angle			= 0;
	target.Clear( );
	targetname.Clear( );
	killtarget.Clear( );
	team.Clear( );
	pathtarget.Clear( );
	deathtarget.Clear( );
	combattarget.Clear( );
	target_ent		= NULL;
	speed			= 0;
	accel			= 0;
	decel			= 0;
	movedir.Zero( );
	pos1.Zero( );
	pos2.Zero( );
	velocity.Zero( );
	avelocity.Zero( );
	mass			= 0;
	air_finished	= 0;
	gravity			= 0;
	goalentity		= NULL;
	movetarget		= NULL;
	yaw_speed		= 0;
	eal_yaw			= 0;
	nextthink		= 0;
	prethink		= NULL;
	think			= NULL;
	blocked			= NULL;
	touch			= NULL;
	use				= NULL;
	pain			= NULL;
	die				= NULL;
	touch_debounce_time			= 0;
	pain_debounce_time			= 0;
	damage_debounce_time		= 0;
	fly_sound_debounce_time		= 0;
	last_move_time				= 0;
	health			= 0;
	max_health		= 0;
	gib_health		= 0;
	deadflag		= 0;
	show_hostile	= false;
	powerarmor_time	= 0;
	map.Clear( );
	viewheight		= 0;
	takedamage		= 0;
	dmg				= 0;
	radius_dmg		= 0;
	dmg_radius		= 0;
	sounds			= 0;
	count			= 0;
	chain			= NULL;
	enemy			= NULL;
	oldenemy		= NULL;
	activator		= NULL;
	groundentity	= NULL;
	groundentity_linkcount	= 0;
	teamchain		= NULL;
	teammaster		= NULL;
	mynoise			= NULL;
	mynoise2		= NULL;
	noise_index		= 0;
	noise_index2	= 0;
	volume			= 0;
	attenuation		= 0;
	wait			= 0;
	delay			= 0;
	random			= 0;
	teleport_time	= 0;
	watertype		= 0;
	waterlevel		= 0;
	move_origin.Zero( );
	move_angles.Zero( );
	light_level		= 0;
	style			= 0;
	item			= NULL;
	moveinfo.Clear( );
	monsterinfo.Clear( );
}


//===================================================================


void Game::Shutdown( ) {

	Common::Com_DPrintf( "==== ShutdownGame ====\n" );

	Z_FreeTags( TAG_LEVEL );
	Z_FreeTags( TAG_GAME );
}

//======================================================================


/*

=================

ClientEndServerFrames

=================

*/
void Game::ClientEndServerFrames( ) {

	int i;
	Entity * ent;

	// calc the player views now that all pushing
	// and damage has been added
	for( i = 0; i < Common::maxclients.GetInt( ); i++ )
	{
		ent = g_edicts + 1 + i;
		if( !ent->inuse || !ent->client )
			continue;
		ClientEndServerFrame( ent );
	}

}

/*

=================

CreateTargetChangeLevel



Returns the created target changelevel

=================

*/
Entity * Game::CreateTargetChangeLevel( const Str & map ) {

	Entity * ent;

	ent = G_Spawn( );
	ent->classname = "target_changelevel";
	level.nextmap = map;
	ent->map = level.nextmap;
	return ent;
}

/*

=================

EndDMLevel



The timelimit or fraglimit has been exceeded

=================

*/
void Game::EndDMLevel( ) {

	Entity * ent;
	char * s = NULL, * t, * f;
	static const char * seps = " , \n\r";

	// stay on same level flag
	if( dmflags.GetInt( ) & DF_SAME_LEVEL )
	{
		BeginIntermission( CreateTargetChangeLevel( level.mapname ) );
		return;
	}

	// see if it's in the map list
	if( sv_maplist.GetString( ).Length( ) ) {

		s = ( char * )Mem_Alloc( sv_maplist.GetString( ).Length( ) );
		strcpy( s, sv_maplist.GetString( ) );
		f = NULL;
		t = strtok( s, seps );
		while( t != NULL ) {
			if( level.mapname.IcmpFast( t ) ) {
				// it's in the list, go to the next one
				t = strtok( NULL, seps );
				if( t == NULL ) { // end of list, go to first one
					if( f == NULL ) // there isn't a first one, same level
						BeginIntermission( CreateTargetChangeLevel( level.mapname ) );
					else
						BeginIntermission( CreateTargetChangeLevel( f ) );
				} else
					BeginIntermission( CreateTargetChangeLevel( t ) );
				free( s );
				return;
			}
			if( !f )
				f = t;
			t = strtok( NULL, seps );
		}
		free( s );
	}

	if( level.nextmap[ 0 ] ) // go to a specific map
		BeginIntermission( CreateTargetChangeLevel( level.nextmap ) );
	else { // search for a changelevel
		ent = G_Find( NULL, FOFS( classname ), "target_changelevel" );
		if( !ent )
		{ // the map designer didn't include a changelevel, // so create a fake ent that goes back to the same level
			BeginIntermission( CreateTargetChangeLevel( level.mapname ) );
			Mem_Free( s );
			return;
		}
		BeginIntermission( ent );
	}

	Mem_Free( s );
}


/*

=================

CheckNeedPass

=================

*/
void Game::CheckNeedPass( ) {

	int need;

	// if password or spectator_password has changed, update needpass
	// as needed
	if( password.IsModified( ) || spectator_password.IsModified( ) ) {

		password.ClearModified( );
		spectator_password.ClearModified( );

		need = 0;

		if( password.GetString( ) != "none" ) 			need |= 1;
		if( spectator_password.GetString( ) != "none" ) need |= 2;

		needpass.SetInt( need );
	}
}

/*

=================

CheckDMRules

=================

*/
void Game::CheckDMRules( ) {

	int i;
	gclient_t * cl;

	if( level.intermissiontime )
		return;

	if( !deathmatch.GetBool( ) ) return;

	if( timelimit.GetBool( ) ) {

		if( level.time >= timelimit.GetFloat( ) * 60.0f ) {

			Server::SV_BroadcastPrintf( PRINT_HIGH, "Timelimit hit.\n" );
			EndDMLevel( );
			return;
		}
	}

	if( fraglimit.GetInt( ) ) {

		for( i = 0; i < Common::maxclients.GetInt( ); i++ ) {

			cl = clients + i;
			if( !g_edicts[ i+1 ].inuse )
				continue;

			if( cl->resp.score >= fraglimit.GetInt( ) ) {

				Server::SV_BroadcastPrintf( PRINT_HIGH, "Fraglimit hit.\n" );
				EndDMLevel( );
				return;
			}
		}
	}
}


/*

=============

ExitLevel

=============

*/
void Game::ExitLevel( ) {

	int i;
	Entity * ent;
	Str command;

	sprintf( command, "gamemap \"%s\"\n", level.changemap.c_str( ) );
	CBuffer::Cbuf_AddText( command );
	level.changemap = NULL;
	level.exitintermission = 0;
	level.intermissiontime = 0;
	ClientEndServerFrames( );

	// clear some things before going to next level
	for( i = 0; i < Common::maxclients.GetInt( ); i++ )
	{
		ent = g_edicts + 1 + i;
		if( !ent->inuse )
			continue;
		if( ent->health > ent->client->pers.max_health )
			ent->health = ent->client->pers.max_health;
	}

}

/*

================

G_RunFrame



Advances the world by 0.1 seconds

================

*/
void Game::RunFrame( ) {

	int i;
	Entity * ent;

	level.framenum++;
	level.time = ( timeType )level.framenum * FRAMETIME;

	// choose a client for monsters to target this frame
	AI_SetSightClient( );

	// exit intermissions

	if( level.exitintermission )
	{
		ExitLevel( );
		return;
	}

	//
	// treat each object in turn
	// even the world gets a chance to think
	//
	ent = g_edicts;
	for( i = 0; i<num_edicts; i++, ent++ )
	{
		if( !ent->inuse )
			continue;

		level.current_entity = ent;

		ent->s.old_origin = ent->s.origin;

		// if the ground entity moved, make sure we are still on it
		if( ( ent->groundentity ) &&( ent->groundentity->linkcount != ent->groundentity_linkcount ) )
		{
			ent->groundentity = NULL;
			if( !( ent->flags &( FL_SWIM|FL_FLY ) ) &&( ent->svflags & SVF_MONSTER ) )
			{
				M_CheckGround( ent );
			}
		}

		if( i > 0 && i <= Common::maxclients.GetInt( ) )
		{
			ClientBeginServerFrame( ent );
			continue;
		}

		G_RunEntity( ent );
	}

	// see if it is time to end a deathmatch
	CheckDMRules( );

	// see if needpass needs updated
	CheckNeedPass( );

	// build the playerstate_t structures for all players
	ClientEndServerFrames( );
}
