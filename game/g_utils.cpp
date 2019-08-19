#include "precompiled.h"
#pragma hdrstop


void Game::G_ProjectSource( const Vec3 & point, const Vec3 & distance, const Vec3 & forward, const Vec3 & right, Vec3 & result ) {
	result[ 0 ] = point[ 0 ] + forward[ 0 ] * distance[ 0 ] + right[ 0 ] * distance[ 1 ];
	result[ 1 ] = point[ 1 ] + forward[ 1 ] * distance[ 0 ] + right[ 1 ] * distance[ 1 ];
	result[ 2 ] = point[ 2 ] + forward[ 2 ] * distance[ 0 ] + right[ 2 ] * distance[ 1 ] + distance[ 2 ];
}


/*

=============

G_Find



Searches all active entities for the next one that holds

the matching string at fieldofs( use the FOFS( ) macro ) in the structure.



Searches beginning at the edict after from, or the beginning if NULL

NULL will be returned if the end of the list is reached.



=============

*/
Entity * Game::G_Find( Entity * from, int fieldofs, const Str & match ) {

	Str * s;

	if( !from ) from = g_edicts;
	else from++;

	for( ; from < &g_edicts[ num_edicts ]; from++ ) {

		if( !from->inuse ) continue;
		s = ( Str * )( ( byte * )from + fieldofs );
		if( !*s ) continue;
		if( *s == match ) 	return from;
	}

	return NULL;
}


/*

=================

findradius



Returns entities that have origins within a spherical area



findradius( origin, radius )

=================

*/
Entity * Game::findradius( Entity * from, Vec3 & org, float rad ) {

	Vec3 eorg;
	int j;

	if( !from )
		from = g_edicts;
	else
		from++;
	for( ; from < &g_edicts[ num_edicts ]; from++ )
	{
		if( !from->inuse )
			continue;
		if( from->solid == SOLID_NOT )
			continue;
		for( j = 0; j<3; j++ )
			eorg[ j ] = org[ j ] -( from->s.origin[ j ] +( from->mins[ j ] + from->maxs[ j ] )* 0.5f );
		if( eorg.Length( ) > rad )
			continue;
		return from;
	}

	return NULL;
}


/*

=============

G_PickTarget



Searches all active entities for the next one that holds

the matching string at fieldofs( use the FOFS( ) macro ) in the structure.



Searches beginning at the edict after from, or the beginning if NULL

NULL will be returned if the end of the list is reached.



=============

*/
Entity * Game::G_PickTarget( const Str & targetname ) {

	Entity * ent = NULL;
	int num_choices = 0;
	Entity * choice[ 8 ];

	if( !targetname ) {

		Common::Com_DPrintf( "G_PickTarget called with NULL targetname\n" );
		return NULL;
	}

	while( 1 ) {

		ent = G_Find( ent, FOFS( targetname ), targetname );
		if( !ent ) break;
		choice[ num_choices++ ] = ent;
		if( num_choices == 8 ) break;
	}

	if( !num_choices ) {

		Common::Com_DPrintf( "G_PickTarget: target %s not found\n", targetname.c_str( ) );
		return NULL;
	}

	return choice[ rand( ) % num_choices ];
}



void Game::Think_Delay( Entity * ent ) {

	G_UseTargets( ent, ent->activator );
	G_FreeEdict( ent );
}

/*

==============================

G_UseTargets



the global "activator" should be set to the entity that initiated the firing.



If self.delay is set, a DelayedUse entity will be created that will actually

do the SUB_UseTargets after that many seconds have passed.



Centerprints any self.message to the activator.



Search for( string )targetname in all entities that

match( string )self.target and call their .use function



==============================

*/
void Game::G_UseTargets( Entity * ent, Entity * activator ) {

	Entity * t;

	//
	// check for a delay
	//
	if( ent->delay )
	{
		// create a temp object to fire at a later time
		t = G_Spawn( );
		t->classname = "DelayedUse";
		t->nextthink = level.time + ent->delay;
		t->think = Think_Delay;
		t->activator = activator;
		if( !activator )
			Common::Com_DPrintf( "Think_Delay with no activator\n" );
		t->message = ent->message;
		t->target = ent->target;
		t->killtarget = ent->killtarget;
		return;
	}


	//
	// print the message
	//
	if( ( ent->message ) && !( activator->svflags & SVF_MONSTER ) ) {
		Server::PF_centerprintf( activator, "%s", ent->message.c_str( ) );
		if( ent->noise_index )
			Server::PF_StartSound( activator, CHAN_AUTO, ent->noise_index, 1, ATTN_NORM, 0 );
		else
			Server::PF_StartSound( activator, CHAN_AUTO, Server::SV_SoundIndex( "misc/talk1.wav" ), 1, ATTN_NORM, 0 );
	}

	//
	// kill killtargets
	//
	if( ent->killtarget )
	{
		t = NULL;
		while( ( t = G_Find( t, FOFS( targetname ), ent->killtarget ) ) )
		{
			G_FreeEdict( t );
			if( !ent->inuse )
			{
				Common::Com_DPrintf( "entity was removed while using killtargets\n" );
				return;
			}
		}
	}

	//
	// fire targets
	//
	if( ent->target )
	{
		t = NULL;
		while( ( t = G_Find( t, FOFS( targetname ), ent->target ) ) )
		{
			// doors fire area portals in a specific way
			if( t->classname.IcmpFast( "func_areaportal" ) &&
				( ent->classname.IcmpFast( "func_door" ) || ent->classname.IcmpFast( "func_door_rotating" ) ) )
				continue;

			if( t == ent )
			{
				Common::Com_DPrintf( "WARNING: Entity used itself.\n" );
			}
			else
			{
				if( t->use )
					t->use( t, ent, activator );
			}
			if( !ent->inuse )
			{
				Common::Com_DPrintf( "entity was removed while using targets\n" );
				return;
			}
		}
	}
}

/*

=============

VectorToString



This is just a convenience function

for printing vectors

=============

*/
const Str Game::vtos( Vec3 & v ) {

	Str str;

	sprintf( str, "( %i %i %i )", ( int )v[ 0 ], ( int )v[ 1 ], ( int )v[ 2 ] );

	return str;
}

Vec3 VEC_UP = Vec3( 0, -1, 0 );
Vec3 MOVEDIR_UP = Vec3( 0, 0, 1 );
Vec3 VEC_DOWN = Vec3( 0, -2, 0 );
Vec3 MOVEDIR_DOWN = Vec3( 0, 0, -1 );

void Game::G_SetMovedir( Vec3 & angles, Vec3 * movedir ) {

	if( angles == VEC_UP ) * movedir = MOVEDIR_UP;
	else if( angles == VEC_DOWN ) * movedir = MOVEDIR_DOWN;
	else angles.AngleVectors( movedir, NULL, NULL );

	angles = vec3_origin;
}


float Game::vectoyaw( Vec3 & vec ) {

	float yaw;

	if( /* vec[ YAW ] == 0 &&*/ vec[ PITCH ] == 0 )
	{
		yaw = 0;
		if( vec[ YAW ] > 0 )
			yaw = 90;
		else if( vec[ YAW ] < 0 )
			yaw = -90;
	}
	else
	{
		yaw = ( atan2( vec[ YAW ], vec[ PITCH ] ) * 180 / M_PI );
		if( yaw < 0 )
			yaw += 360;
	}

	return yaw;
}


void Game::vectoangles( Vec3 & value1, Vec3 * angles ) {

	float forward;
	float yaw, pitch;

	if( value1[ 1 ] == 0 && value1[ 0 ] == 0 )
	{
		yaw = 0;
		if( value1[ 2 ] > 0 )
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		if( value1[ 0 ] )
			yaw = ( atan2( value1[ 1 ], value1[ 0 ] ) * 180 / M_PI );
		else if( value1[ 1 ] > 0 )
			yaw = 90;
		else
			yaw = -90;
		if( yaw < 0 )
			yaw += 360;

		forward = sqrt( value1[ 0 ]* value1[ 0 ] + value1[ 1 ]* value1[ 1 ] );
		pitch = ( atan2( value1[ 2 ], forward ) * 180 / M_PI );
		if( pitch < 0 )
			pitch += 360;
	}

	angles->ToFloatPtr( )[ PITCH ] = -pitch;
	angles->ToFloatPtr( )[ YAW ] = yaw;
	angles->ToFloatPtr( )[ ROLL ] = 0;
}

void Game::G_InitEdict( Entity * e ) {

	e->inuse = true;
	e->classname = "noclass";
	e->gravity = 1.0f;
	e->s.number = ( int )( e - g_edicts );
}

/*
=================
G_Spawn

Either finds a free edict, or allocates a new one.
Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
Entity * Game::G_Spawn( ) {

	int i;
	Entity * e;

	e = &g_edicts[ Common::maxclients.GetInt( ) + 1 ];
	for( i = Common::maxclients.GetInt( ) + 1; i < num_edicts; i++, e++ )
	{
		// the first couple seconds of server time can involve a lot of
		// freeing and allocating, so relax the replacement policy
		if( !e->inuse &&( e->freetime < 2.0 || level.time - e->freetime > 0.5 ) ) {

			G_InitEdict( e );
			return e;
		}
	}

	if( i == maxEntities ) Server::PF_error( "ED_Alloc: no free edicts" );

	num_edicts++;
	G_InitEdict( e );
	return e;
}

/*

=================

G_FreeEdict



Marks the edict as free

=================

*/
void Game::G_FreeEdict( Entity * ed ) {
	Server::SV_UnlinkEdict( ed ); // unlink from world
	if( ( ed - g_edicts ) <=( Common::maxclients.GetInt( ) + BODY_QUEUE_SIZE ) ) {
		//		Common::Com_DPrintf( "tried to free special edict\n" );
		return;
	}
	ed->Clear( );
	ed->classname = "freed";
	ed->freetime = level.time;
	ed->inuse = false;
}


/*

============

G_TouchTriggers



============

*/
void Game::G_TouchTriggers( Entity * ent ) {

	int i, num;
	Entity * touch[ MAX_EDICTS ], * hit;

	// dead things don't activate triggers!
	if( ( ent->client ||( ent->svflags & SVF_MONSTER ) ) &&( ent->health <= 0 ) ) return;

	num = Server::SV_AreaEdicts( ent->absmin, ent->absmax, touch, MAX_EDICTS, AREA_TRIGGERS );

	// be careful, it is possible to have an entity in this
	// list removed before we get to it( killtriggered )
	for( i = 0; i<num; i++ )
	{
		hit = touch[ i ];
		if( !hit->inuse )
			continue;
		if( !hit->touch )
			continue;
		hit->touch( hit, ent, plane_origin, SURF_NULL );
	}
}

/*

============

G_TouchSolids



Call after linking a new trigger in during gameplay

to force all entities it covers to immediately touch it

============

*/
void Game::G_TouchSolids( Entity * ent ) {

	int i, num;
	Entity * touch[ MAX_EDICTS ], * hit;

	num = Server::SV_AreaEdicts( ent->absmin, ent->absmax, touch
		, MAX_EDICTS, AREA_SOLID );

	// be careful, it is possible to have an entity in this
	// list removed before we get to it( killtriggered )
	for( i = 0; i<num; i++ )
	{
		hit = touch[ i ];
		if( !hit->inuse )
			continue;
		if( ent->touch )
			ent->touch( hit, ent, plane_origin, SURF_NULL );
		if( !ent->inuse )
			break;
	}
}




/*

==============================================================================



Kill box



==============================================================================

*/
/*

=================

KillBox



Kills all entities that would touch the proposed new positioning

of ent.  Ent should be unlinked before calling this!

=================

*/
bool Game::KillBox( Entity * ent ) {

	CTrace tr;

	while( 1 )
	{
		tr = Server::SV_Trace( ent->s.origin, ent->mins, ent->maxs, ent->s.origin, NULL, MASK_PLAYERSOLID );
		if( !tr.ent )
			break;

		// nail it
		T_Damage( tr.ent, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_TELEFRAG );

		// if we didn't kill it, fail
		if( tr.ent->solid )
			return false;
	}

	return true; // all clear
}
