#include "precompiled.h"
#pragma hdrstop

void level_locals_t::Clear( ) {
	framenum			= 0;
	time				= 0;
	level_name.Clear( );
	mapname.Clear( );
	nextmap.Clear( );
	intermissiontime	= 0;
	changemap.Clear( );
	exitintermission	= 0;
	intermission_origin.Zero( );
	intermission_angle.Zero( );
	sight_client		= NULL;
	sight_entity		= NULL;
	sight_entity_framenum	= 0;
	sound_entity		= NULL;
	sound_entity_framenum	= 0;
	sound2_entity		= NULL;
	sound2_entity_framenum	= 0;
	pic_health			= 0;
	total_secrets		= 0;
	found_secrets		= 0;
	total_goals			= 0;
	found_goals			= 0;
	total_monsters		= 0;
	killed_monsters		= 0;
	current_entity		= NULL;
	body_que			= 0;
	power_cubes			= 0;
}

void spawn_temp_t::Clear( ) {
	sky.Clear( );
	skyrotate;
	skyaxis.Zero( );
	nextmap.Clear( );
	lip			= 0;
	distance	= 0;
	height		= 0;
	noise.Clear( );
	pausetime	= 0;
	item.Clear( );
	gravity.Clear( );
	minyaw		= 0;
	maxyaw		= 0;
	minpitch	= 0;
	maxpitch	= 0;
}

/*===============
ED_CallSpawn

Finds the spawn function for the entity and calls it
===============*/
void Game::ED_CallSpawn( Entity * ent ) {

	spawn_t * s;
	GameItem * item;
	int i;

	if( !ent->classname )
	{
		Common::Com_DPrintf( "ED_CallSpawn: NULL classname\n" );
		return;
	}

	// check item spawn functions
	for( i = 0, item = itemlist; i<num_items; i++, item++ )
	{
		if( !item->classname )
			continue;
		if( item->classname == ent->classname )
		{ // found it
			SpawnItem( ent, item );
			return;
		}
	}

	// check normal spawn functions
	for( s = spawns; s->spawn; s++ )
	{
		if( s->name == ent->classname )
		{ // found it
			s->spawn( ent );
			return;
		}
	}
	Common::Com_DPrintf( "%s doesn't have a spawn function\n", ent->classname.c_str( ) );
}

/*

===============
ED_ParseField

Takes a key/value pair and sets the binary values
in an edict
===============

*/
void Game::ED_ParseField( const Str & key, const Str & value, Entity * ent ) {

	const field_t * f;
	byte * b;
	float v;
	Vec3 vec;

	for( f = fields; f->name; f++ )
	{
		if( !( f->flags & FFL_NOSPAWN ) && f->name.IcmpFast( key ) )
		{ // found it
			if( f->flags & FFL_SPAWNTEMP )
				b =( byte * )&st;
			else
				b =( byte * )ent;

			switch( f->type ) {

			case F_LSTRING:

				*( Str * )( b+f->ofs ) = value;
				break;
			case F_VECTOR:
				sscanf( value, "%f %f %f", &vec[ 0 ], &vec[ 1 ], &vec[ 2 ] );
				( ( float * )( b+f->ofs ) )[ 0 ] = vec[ 0 ];
				( ( float * )( b+f->ofs ) )[ 1 ] = vec[ 1 ];
				( ( float * )( b+f->ofs ) )[ 2 ] = vec[ 2 ];
				break;
			case F_INT:
				* ( int * )( b+f->ofs ) = atoi( value );
				break;
			case F_FLOAT:
				* ( float * )( b+f->ofs ) = ( float )atof( value );
				break;

			case F_DOUBLE:
				*( double * )( b+f->ofs ) = atof( value );
				break;

			case F_ANGLEHACK:
				v = ( float )atof( value );
				( ( float * )( b+f->ofs ) )[ 0 ] = 0;
				( ( float * )( b+f->ofs ) )[ 1 ] = v;
				( ( float * )( b+f->ofs ) )[ 2 ] = 0;
				break;
			case F_IGNORE:
				break;
			}
			return;
		}
	}
	Common::Com_DPrintf( "%s is not a field\n", key.c_str( ) );
}

/*

================

G_FindTeams



Chain together all entities with a matching team field.



All but the first will have the FL_TEAMSLAVE flag set.

All but the last will have the teamchain field set to the next one

================

*/
void Game::G_FindTeams( ) {

	Entity * e, * e2, * chain;
	int i, j;
	int c, c2;

	c = 0;
	c2 = 0;
	for( i = 1, e = g_edicts+i; i < num_edicts; i++, e++ )
	{
		if( !e->inuse )
			continue;
		if( !e->team )
			continue;
		if( e->flags & FL_TEAMSLAVE )
			continue;
		chain = e;
		e->teammaster = e;
		c++;
		c2++;
		for( j = i+1, e2 = e+1; j < num_edicts; j++, e2++ )
		{
			if( !e2->inuse )
				continue;
			if( !e2->team )
				continue;
			if( e2->flags & FL_TEAMSLAVE )
				continue;
			if( !strcmp( e->team, e2->team ) )
			{
				c2++;
				chain->teamchain = e2;
				e2->teammaster = e;
				chain = e2;
				e2->flags |= FL_TEAMSLAVE;
			}
		}
	}

	Common::Com_DPrintf( "%i teams with %i entities\n", c, c2 );
}

void Game::ED_ParseEdict( const MapEntity & mapEnt, Entity * gameEnt ) {
	bool init = false;
	const field_t * f;
	byte * b;
	Vec3 vec;
	Token token;
	const Dict & eDict = mapEnt.GetEpairs( );
	st.Clear( );
	// go through all the dictionary pairs
	for( int i = 0; i < eDict.GetNumKeyVals( ); i++ ) {
		init = true;
		// keynames with a leading underscore are used for utility comments, and are immediately discarded by quake
		if( eDict.GetKeyVal( i )->GetKey( )[ 0 ] == '_' )			
			continue;
		for( f = fields; f->name.Length( ); f++ ) {
			if( !( f->flags & FFL_NOSPAWN ) && f->name == eDict.GetKeyVal( i )->GetKey( ) ) { // found it

				if( f->flags & FFL_SPAWNTEMP )
					b = ( byte * )&st;
				else
					b = ( byte * )gameEnt;
				if( f->type == F_LSTRING ) {
					*( Str * )( b + f->ofs ) = eDict.GetKeyVal( i )->GetValue( );
					break;
				}
				if( f->type == F_VECTOR ) {
					Lexer lexer2( eDict.GetKeyVal( i )->GetValue( ).c_str( ), eDict.GetKeyVal( i )->GetValue( ).Length( ), "F_VECTOR" );
					lexer2.ParseFloatArray( 3, ( ( float * )( b + f->ofs ) ) );
					break;
				}
				if( f->type == F_INT ) {
					token = eDict.GetKeyVal( i )->GetKey( );
					token.type = TT_NUMBER;
					token.subtype = TT_DECIMAL;
					*( int * )( b + f->ofs ) = token.GetIntValue( );
					break;
				}
				if( f->type == F_FLOAT ) {
					token = eDict.GetKeyVal( i )->GetKey( );
					token.type = TT_NUMBER;
					token.subtype = TT_DECIMAL;
					*( float * )( b + f->ofs ) = token.GetFloatValue( );
					break;
				}
				if( f->type == F_DOUBLE ) {
					token = eDict.GetKeyVal( i )->GetKey( );
					token.type = TT_NUMBER;
					token.subtype = TT_DECIMAL;
					*( double * )( b + f->ofs ) = token.GetDoubleValue( );
					break;
				}
				if( f->type == F_ANGLEHACK ) {
					token = eDict.GetKeyVal( i )->GetKey( );
					token.type = TT_NUMBER;
					token.subtype = TT_DECIMAL;
					( ( float * )( b + f->ofs ) )[ 0 ] = 0;
					( ( float * )( b + f->ofs ) )[ 1 ] = token.GetFloatValue( );
					( ( float * )( b + f->ofs ) )[ 2 ] = 0;
					break;
				}
				if( f->type == F_IGNORE )
					break;
			}
		}
		if( !f->name )
			Common::Com_DPrintf( "Unknown key name: \"%s\" with value: \"%s\"\n", eDict.GetKeyVal( i )->GetKey( ).c_str( ),  eDict.GetKeyVal( i )->GetValue( ).c_str( ) );
	}
	if( !init )
		gameEnt->Clear( );
}

/*
==============
SpawnEntities

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.
==============
*/

void Game::SpawnEntities( const Str & mapname, Str & spoint ) {

	Entity *	ent;
	int			inhibit;
	int			i;
	float		skill_level;
	Str			fullname;

	sprintf( fullname, "maps/%s.map", mapname.c_str( ) );

	skill_level = floor( skill.GetFloat( ) );
	if( skill_level < 0 ) skill_level = 0;
	if( skill_level > 3 ) skill_level = 3;
	if( skill.GetInt( ) != skill_level ) skill.SetInt( ( int )skill_level );

	SaveClientData( );

	Z_FreeTags( TAG_LEVEL );

	level.Clear( );
	for( int i = 0; i < maxEntities; i++ )
		g_edicts[ i ].Clear( );

	level.mapname = mapname;
	spawnpoint = spawnpoint;

	// set client fields on player ents
	for( i = 0; i < maxClients; i++ ) g_edicts[ i + 1 ].client = clients + i;

	ent = NULL;
	inhibit = 0;

	MapData * mapData = MapSystem::LoadMapFile( fullname );

	for( int i = 0; i < mapData->GetMapFile( )->GetNumEntities( ); i++ ) {

		if( !ent ) ent = g_edicts;
		else ent = G_Spawn( );

		ED_ParseEdict( mapData->GetMapFile( )->GetEntity( i ), ent );
		ED_CallSpawn( ent );

	}

	Common::Com_DPrintf( "%i entities inhibited\n", inhibit );
	G_FindTeams( );

	PlayerTrail_Init( );
}


//===================================================================
const char * single_statusbar =
	"yb	-24 "

	// health
	"xv	0 "
	"hnum "
	"xv	50 "
	"pic 0 "

	// ammo
	"if 2 "
	"	xv	100 "
	"	anum "
	"	xv	150 "
	"	pic 2 "
	"endif "

	// armor
	"if 4 "
	"	xv	200 "
	"	rnum "
	"	xv	250 "
	"	pic 4 "
	"endif "

	// selected item
	"if 6 "
	"	xv	296 "
	"	pic 6 "
	"endif "

	"yb	-50 "

	// picked up item
	"if 7 "
	"	xv	0 "
	"	pic 7 "
	"	xv	26 "
	"	yb	-42 "
	"	stat_string 8 "
	"	yb	-50 "
	"endif "

	// timer
	"if 9 "
	"	xv	262 "
	"	num	2	10 "
	"	xv	296 "
	"	pic	9 "
	"endif "

	//  help / weapon icon 
	"if 11 "
	"	xv	148 "
	"	pic	11 "
	"endif "
;

const char * dm_statusbar =
	"yb	-24 "

	// health
	"xv	0 "
	"hnum "
	"xv	50 "
	"pic 0 "

	// ammo
	"if 2 "
	"	xv	100 "
	"	anum "
	"	xv	150 "
	"	pic 2 "
	"endif "

	// armor
	"if 4 "
	"	xv	200 "
	"	rnum "
	"	xv	250 "
	"	pic 4 "
	"endif "

	// selected item
	"if 6 "
	"	xv	296 "
	"	pic 6 "
	"endif "

	"yb	-50 "

	// picked up item
	"if 7 "
	"	xv	0 "
	"	pic 7 "
	"	xv	26 "
	"	yb	-42 "
	"	stat_string 8 "
	"	yb	-50 "
	"endif "

	// timer
	"if 9 "
	"	xv	246 "
	"	num	2	10 "
	"	xv	296 "
	"	pic	9 "
	"endif "

	//  help / weapon icon 
	"if 11 "
	"	xv	148 "
	"	pic	11 "
	"endif "

	//  frags
	"xr	-50 "
	"yt 2 "
	"num 3 14 "

	// spectator
	"if 17 "
	"xv 0 "
	"yb -58 "
	"string2 \"SPECTATOR MODE\" "
	"endif "

	// chase camera
	"if 16 "
	"xv 0 "
	"yb -68 "
	"string \"Chasing\" "
	"xv 64 "
	"stat_string 16 "
	"endif "
;


/* QUAKED worldspawn( 0 0 0 ) ?



Only used for the world.

"sky"	environment map name

"skyaxis"	vector axis for rotating sky

"skyrotate"	speed of rotation in degrees/second

"sounds"	music cd track number

"gravity"	800 is default gravity

"message"	text to print at user logon

*/
void Game::SP_worldspawn( Entity * ent ) {

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	ent->inuse = true; // since the world doesn't use G_Spawn( )
	ent->s.modelindex = 1; // world model is always index 1

	//---------------

	// reserve some spots for dead player bodies for coop / deathmatch
	InitBodyQue( );

	// set configstrings for items
	SetItemNames( );

	if( st.nextmap.Length( ) ) level.nextmap = st.nextmap;

	// make some data visible to the server

	if( ent->message && ent->message[ 0 ] ) {

		Server::PF_Configstring( CS_NAME, ent->message );
		level.level_name = ent->message;

	} else level.level_name = level.mapname;

	if( st.sky && st.sky[ 0 ] ) Server::PF_Configstring( CS_SKY, st.sky );
	else Server::PF_Configstring( CS_SKY, "unit1_" );

	Server::PF_Configstring( CS_SKYROTATE, va( "%f", st.skyrotate ) );

	Server::PF_Configstring( CS_SKYAXIS, va( "%f %f %f", st.skyaxis[ 0 ], st.skyaxis[ 1 ], st.skyaxis[ 2 ] ) );

	Server::PF_Configstring( CS_MAXCLIENTS, va( "%i", Common::maxclients.GetInt( ) ) );

	// status bar program
	if( deathmatch.GetBool( ) )
		Server::PF_Configstring( CS_STATUSBAR, dm_statusbar );
	else
		Server::PF_Configstring( CS_STATUSBAR, single_statusbar );

	//---------------


	// help icon for statusbar
	/*Server::SV_ImageIndex( "pics/i_help.pcx" );
	level.pic_health = Server::SV_ImageIndex( "pics/i_health.pcx" );
	Server::SV_ImageIndex( "pics/help.pcx" );
	Server::SV_ImageIndex( "pics/field_3.pcx" );*/

	if( st.gravity.IsEmpty( ) )
		CVarSystem::SetCVarInt( "sv_gravity", 800 );
	else
		CVarSystem::SetCVarStr( "sv_gravity", st.gravity );

	/*snd_fry = Server::SV_SoundIndex( "player/fry.wav" ); // standing in lava / slime

	PrecacheItem( FindItem( "Blaster" ) );

	Server::SV_SoundIndex( "player/lava1.wav" );
	Server::SV_SoundIndex( "player/lava2.wav" );

	Server::SV_SoundIndex( "misc/pc_up.wav" );
	Server::SV_SoundIndex( "misc/talk1.wav" );

	Server::SV_SoundIndex( "misc/udeath.wav" );

	// gibs
	Server::SV_SoundIndex( "items/respawn1.wav" );

	// sexed sounds
	Server::SV_SoundIndex( "*death1.wav" );
	Server::SV_SoundIndex( "*death2.wav" );
	Server::SV_SoundIndex( "*death3.wav" );
	Server::SV_SoundIndex( "*death4.wav" );
	Server::SV_SoundIndex( "*fall1.wav" );
	Server::SV_SoundIndex( "*fall2.wav" );
	Server::SV_SoundIndex( "*gurp1.wav" ); // drowning damage
	Server::SV_SoundIndex( "*gurp2.wav" );
	Server::SV_SoundIndex( "*jump1.wav" ); // player jump
	Server::SV_SoundIndex( "*pain25_1.wav" );
	Server::SV_SoundIndex( "*pain25_2.wav" );
	Server::SV_SoundIndex( "*pain50_1.wav" );
	Server::SV_SoundIndex( "*pain50_2.wav" );
	Server::SV_SoundIndex( "*pain75_1.wav" );
	Server::SV_SoundIndex( "*pain75_2.wav" );
	Server::SV_SoundIndex( "*pain100_1.wav" );
	Server::SV_SoundIndex( "*pain100_2.wav" );

	// sexed models
	// THIS ORDER MUST MATCH THE DEFINES IN g_local.h
	// you can add more, max 15
	Server::SV_ModelIndex( "#w_blaster.md2" );
	Server::SV_ModelIndex( "#w_shotgun.md2" );
	Server::SV_ModelIndex( "#w_sshotgun.md2" );
	Server::SV_ModelIndex( "#w_machinegun.md2" );
	Server::SV_ModelIndex( "#w_chaingun.md2" );
	Server::SV_ModelIndex( "#a_grenades.md2" );
	Server::SV_ModelIndex( "#w_glauncher.md2" );
	Server::SV_ModelIndex( "#w_rlauncher.md2" );
	Server::SV_ModelIndex( "#w_hyperblaster.md2" );
	Server::SV_ModelIndex( "#w_railgun.md2" );
	Server::SV_ModelIndex( "#w_bfg.md2" );

	//-------------------

	Server::SV_SoundIndex( "player/gasp1.wav" ); // gasping for air
	Server::SV_SoundIndex( "player/gasp2.wav" ); // head breaking surface, not gasping

	Server::SV_SoundIndex( "player/watr_in.wav" ); // feet hitting water
	Server::SV_SoundIndex( "player/watr_out.wav" ); // feet leaving water

	Server::SV_SoundIndex( "player/watr_un.wav" ); // head going underwater

	Server::SV_SoundIndex( "player/u_breath1.wav" );
	Server::SV_SoundIndex( "player/u_breath2.wav" );

	Server::SV_SoundIndex( "items/pkup.wav" ); // bonus item pickup
	Server::SV_SoundIndex( "world/land.wav" ); // landing thud
	Server::SV_SoundIndex( "misc/h2ohit1.wav" ); // landing splash

	Server::SV_SoundIndex( "items/damage.wav" );
	Server::SV_SoundIndex( "items/protect.wav" );
	Server::SV_SoundIndex( "items/protect4.wav" );
	Server::SV_SoundIndex( "weapons/noammo.wav" );

	Server::SV_SoundIndex( "infantry/inflies1.wav" );

	sm_meat_index = Server::SV_ModelIndex( meatGib );
	Server::SV_ModelIndex( "models/objects/gibs/arm/tris.md2" );
	Server::SV_ModelIndex( boneGib );
	Server::SV_ModelIndex( "models/objects/gibs/bone2/tris.md2" );
	Server::SV_ModelIndex( "models/objects/gibs/chest/tris.md2" );
	Server::SV_ModelIndex( "models/objects/gibs/skull/tris.md2" );
	Server::SV_ModelIndex( headGib );*/

	//
	// Setup light animation tables. 'a' is total darkness, 'z' is doublebright.
	//

	// 0 normal
	Server::PF_Configstring( CS_LIGHTS+0, "m" );

	// 1 FLICKER( first variety )
	Server::PF_Configstring( CS_LIGHTS+1, "mmnmmommommnonmmonqnmmo" );

	// 2 SLOW STRONG PULSE
	Server::PF_Configstring( CS_LIGHTS+2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba" );

	// 3 CANDLE( first variety )
	Server::PF_Configstring( CS_LIGHTS+3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg" );

	// 4 FAST STROBE
	Server::PF_Configstring( CS_LIGHTS+4, "mamamamamama" );

	// 5 GENTLE PULSE 1
	Server::PF_Configstring( CS_LIGHTS+5, "jklmnopqrstuvwxyzyxwvutsrqponmlkj" );

	// 6 FLICKER( second variety )
	Server::PF_Configstring( CS_LIGHTS+6, "nmonqnmomnmomomno" );

	// 7 CANDLE( second variety )
	Server::PF_Configstring( CS_LIGHTS+7, "mmmaaaabcdefgmmmmaaaammmaamm" );

	// 8 CANDLE( third variety )
	Server::PF_Configstring( CS_LIGHTS+8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa" );

	// 9 SLOW STROBE( fourth variety )
	Server::PF_Configstring( CS_LIGHTS+9, "aaaaaaaazzzzzzzz" );

	// 10 FLUORESCENT FLICKER
	Server::PF_Configstring( CS_LIGHTS+10, "mmamammmmammamamaaamammma" );

	// 11 SLOW PULSE NOT FADE TO BLACK
	Server::PF_Configstring( CS_LIGHTS+11, "abcdefghijklmnopqrrqponmlkjihgfedcba" );

	// styles 32-62 are assigned by the light program for switchable lights

	// 63 testing
	Server::PF_Configstring( CS_LIGHTS+63, "a" );
}
