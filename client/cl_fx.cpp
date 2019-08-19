#include "precompiled.h"
#pragma hdrstop

RenderModel * Client::cl_mod_smoke;
RenderModel * Client::cl_mod_flash;

/*

==============================================================



LIGHT STYLE MANAGEMENT



==============================================================

*/
typedef struct {

	int length;
	float value[ 3 ];
	float map[ MAX_QPATH ];
} clightstyle_t;

clightstyle_t cl_lightstyle[ MAX_LIGHTSTYLES ];
int lastofs;

/*

================

CL_ClearLightStyles

================

*/
void Client::CL_ClearLightStyles( ) {

	Common::Com_Memset( cl_lightstyle, 0, sizeof( cl_lightstyle ) );
	lastofs = -1;
}

/*

================

CL_RunLightStyles

================

*/
void Client::CL_RunLightStyles( ) {

	int ofs;
	int i;
	clightstyle_t * ls;

	ofs = ( int )( cl.time / 100 );
	if( ofs == lastofs )
		return;
	lastofs = ofs;

	for( i = 0, ls = cl_lightstyle; i<MAX_LIGHTSTYLES; i++, ls++ )
	{
		if( !ls->length )
		{
			ls->value[ 0 ] = ls->value[ 1 ] = ls->value[ 2 ] = 1.0f;
			continue;
		}
		if( ls->length == 1 )
			ls->value[ 0 ] = ls->value[ 1 ] = ls->value[ 2 ] = ls->map[ 0 ];
		else
			ls->value[ 0 ] = ls->value[ 1 ] = ls->value[ 2 ] = ls->map[ ofs%ls->length ];
	}
}


void Client::CL_SetLightstyle( int i ) {

	const char * s;
	int j, k;

	s = cl.configstrings[ i+CS_LIGHTS ];

	j = ( int )strlen( s );
	if( j >= MAX_QPATH )
		Common::Com_Error( ERR_DROP, "svc_lightstyle length =%i", j );

	cl_lightstyle[ i ].length = j;

	for( k = 0; k<j; k++ )
		cl_lightstyle[ i ].map[ k ] =( float )( s[ k ]-'a' )/( float )( 'm'-'a' );
}

/*

================

CL_AddLightStyles

================

*/
void Client::CL_AddLightStyles( ) {

	int i;
	clightstyle_t * ls;

	for( i = 0, ls = cl_lightstyle; i<MAX_LIGHTSTYLES; i++, ls++ )
		View::V_AddLightStyle( i, ls->value[ 0 ], ls->value[ 1 ], ls->value[ 2 ] );
}

/*

==============================================================



DLIGHT MANAGEMENT



==============================================================

*/

/*

================

CL_ClearDlights

================

*/
void Client::CL_ClearDlights( ) {

}

/*

===============

CL_AllocDlight



===============

*/
cdlight_t * Client::CL_AllocDlight( int key ) {

	return NULL;
}

/*

===============

CL_NewDlight

===============

*/
void Client::CL_NewDlight( int key, float x, float y, float z, float radius, timeType time ) {

	cdlight_t * dl;

	dl = CL_AllocDlight( key );
	dl->origin[ 0 ] = x;
	dl->origin[ 1 ] = y;
	dl->origin[ 2 ] = z;
	dl->radius = radius;
	dl->die = cl.time + time;
}


/*

===============

CL_RunDLights



===============

*/
void Client::CL_RunDLights( ) {

}

/*

==============

CL_ParseMuzzleFlash

==============

*/
void Client::CL_ParseMuzzleFlash( MessageBuffer & msg_buffer ) {

	int i, weapon;
	i = msg_buffer.ReadShort( );
	weapon = msg_buffer.ReadByte( );

#if 0

	Vec3 fv, rv;
	cdlight_t * dl;
	int i, weapon;
	centity_t * pl;
	int silenced;
	float volume;
	Str soundname;

	i = msg_buffer.ReadShort( );
	if( i < 1 || i >= MAX_EDICTS )
		Common::Com_Error( ERR_DROP, "CL_ParseMuzzleFlash: bad entity" );

	weapon = msg_buffer.ReadByte( );
	silenced = weapon & MZ_SILENCED;
	weapon &= ~MZ_SILENCED;

	pl = &cl_entities[ i ];

	dl = CL_AllocDlight( i );
	dl->origin = pl->current.origin;
	pl->current.angles.AngleVectors( &fv, &rv, NULL );
	dl->origin = dl->origin.Magnitude( 18.0f, fv );
	dl->origin = dl->origin.Magnitude( 16.0f, rv );
	if( silenced )
		dl->radius = 100.0f +( rand( )&31 );
	else
		dl->radius = 200.0f +( rand( )&31 );
	dl->minlight = 32.0f;
	dl->die = cl.time; // + 0.1;

	if( silenced )
		volume = 0.2f;
	else
		volume = 1.0f;

	switch( weapon )
	{
	case MZ_BLASTER:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( Str( "weapons/blastf1a.wav" ) ), volume, ATTN_NORM, 0 );
		break;
	case MZ_BLUEHYPERBLASTER:
		dl->color = Vec3( 0.0f, 0.0f, 1.0f );
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( Str( "weapons/hyprbf1a.wav" ) ), volume, ATTN_NORM, 0 );
		break;
	case MZ_HYPERBLASTER:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( Str( "weapons/hyprbf1a.wav" ) ), volume, ATTN_NORM, 0 );
		break;
	case MZ_MACHINEGUN:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		sprintf( soundname, "weapons/machgf%ib.wav", ( rand( ) % 5 ) + 1 );
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( soundname ), volume, ATTN_NORM, 0 );
		break;
	case MZ_SHOTGUN:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( Str( "weapons/shotgf1b.wav" ) ), volume, ATTN_NORM, 0.0f );
		Sound::S_StartSound( NULL, i, CHAN_AUTO, Sound::S_RegisterSound( Str( "weapons/shotgr1b.wav" ) ), volume, ATTN_NORM, 0.1f );
		break;
	case MZ_SSHOTGUN:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( Str( "weapons/sshotf1b.wav" ) ), volume, ATTN_NORM, 0.0f );
		break;
	case MZ_CHAINGUN1:
		dl->radius = 200.0f +( rand( )&31 );
		dl->color = Vec3( 1.0f, 0.25f, 0.0f );
		sprintf( soundname, "weapons/machgf%ib.wav", ( rand( ) % 5 ) + 1 );
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( soundname ), volume, ATTN_NORM, 0.0f );
		break;
	case MZ_CHAINGUN2:
		dl->radius = 225.0f +( rand( )&31 );
		dl->color = Vec3( 1.0f, 0.5f, 0.0f );
		dl->die = cl.time + 0.1f; // long delay
		sprintf( soundname, "weapons/machgf%ib.wav", ( rand( ) % 5 ) + 1 );
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( soundname ), volume, ATTN_NORM, 0.0f );
		sprintf( soundname, "weapons/machgf%ib.wav", ( rand( ) % 5 ) + 1 );
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( soundname ), volume, ATTN_NORM, 0.05f );
		break;
	case MZ_CHAINGUN3:
		dl->radius = 250.0f +( rand( )&31 );
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		dl->die = cl.time + 0.1f; // long delay
		sprintf( soundname, "weapons/machgf%ib.wav", ( rand( ) % 5 ) + 1 );
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( soundname ), volume, ATTN_NORM, 0.0f );
		sprintf( soundname, "weapons/machgf%ib.wav", ( rand( ) % 5 ) + 1 );
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( soundname ), volume, ATTN_NORM, 0.033f );
		sprintf( soundname, "weapons/machgf%ib.wav", ( rand( ) % 5 ) + 1 );
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( soundname ), volume, ATTN_NORM, 0.066f );
		break;
	case MZ_RAILGUN:
		dl->color = Vec3( 0.5f, 0.5f, 1.0f );
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( Str( "weapons/railgf1a.wav" ) ), volume, ATTN_NORM, 0.0f );
		break;
	case MZ_ROCKET:
		dl->color = Vec3( 1.0f, 0.5f, 0.2f );
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( Str( "weapons/rocklf1a.wav" ) ), volume, ATTN_NORM, 0.0f );
		Sound::S_StartSound( NULL, i, CHAN_AUTO, Sound::S_RegisterSound( Str( "weapons/rocklr1b.wav" ) ), volume, ATTN_NORM, 0.1f );
		break;
	case MZ_GRENADE:
		dl->color = Vec3( 1.0f, 0.5f, 0.0f );
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( Str( "weapons/grenlf1a.wav" ) ), volume, ATTN_NORM, 0.0f );
		Sound::S_StartSound( NULL, i, CHAN_AUTO, Sound::S_RegisterSound( Str( "weapons/grenlr1b.wav" ) ), volume, ATTN_NORM, 0.1f );
		break;
	case MZ_BFG:
		dl->color = Vec3( 0.0f, 1.0f, 0.0f );
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( Str( "weapons/bfg__f1y.wav" ) ), volume, ATTN_NORM, 0.0f );
		break;

	case MZ_LOGIN:
		dl->color = Vec3( 0.0f, 1.0f, 0.0f );
		dl->die = cl.time + 1.0f;
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( Str( "weapons/grenlf1a.wav" ) ), 1, ATTN_NORM, 0.0f );
		CL_LogoutEffect( pl->current.origin, weapon );
		break;
	case MZ_LOGOUT:
		dl->color = Vec3( 1.0f, 0.0f, 0.0f );
		dl->die = cl.time + 1.0f;
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( Str( "weapons/grenlf1a.wav" ) ), 1, ATTN_NORM, 0.0f );
		CL_LogoutEffect( pl->current.origin, weapon );
		break;
	case MZ_RESPAWN:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		dl->die = cl.time + 1.0f;
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( Str( "weapons/grenlf1a.wav" ) ), 1, ATTN_NORM, 0.0f );
		CL_LogoutEffect( pl->current.origin, weapon );
		break;
		// RAFAEL
	case MZ_PHALANX:
		dl->color = Vec3( 1.0f, 0.5f, 0.5f );
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( Str( "weapons/plasshot.wav" ) ), volume, ATTN_NORM, 0.0f );
		break;
		// RAFAEL
	case MZ_IONRIPPER:
		dl->color = Vec3( 1.0f, 0.5f, 0.5f );
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( Str( "weapons/rippfitpRenderer::wav" ) ), volume, ATTN_NORM, 0.0f );
		break;

		// ======================
		// PGM
	case MZ_ETF_RIFLE:
		dl->color = Vec3( 0.9f, 0.7f, 0.0f );
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( Str( "weapons/nail1.wav" ) ), volume, ATTN_NORM, 0 );
		break;
	case MZ_SHOTGUN2:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( Str( "weapons/shotg2.wav" ) ), volume, ATTN_NORM, 0 );
		break;
	case MZ_HEATBEAM:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		dl->die = cl.time + 100;
		//		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( "weapons/bfg__l1a.wav" ), volume, ATTN_NORM, 0 );
		break;
	case MZ_BLASTER2:
		dl->color = Vec3( 0.0f, 1.0f, 0.0f );
		// FIXME - different sound for blaster2 ??
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( Str( "weapons/blastf1a.wav" ) ), volume, ATTN_NORM, 0 );
		break;
	case MZ_TRACKER:
		// negative flashes handled the same in gl/soft until CL_AddDLights
		dl->color = Vec3( -1.0f, -1.0f, -1.0f );
		Sound::S_StartSound( NULL, i, CHAN_WEAPON, Sound::S_RegisterSound( Str( "weapons/disint2.wav" ) ), volume, ATTN_NORM, 0 );
		break;
	case MZ_NUKE1:
		dl->color = Vec3( 1.0f, 0.0f, 0.0f );
		dl->die = cl.time + 100;
		break;
	case MZ_NUKE2:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		dl->die = cl.time + 100;
		break;
	case MZ_NUKE4:
		dl->color = Vec3( 0.0f, 0.0f, 1.0f );
		dl->die = cl.time + 100;
		break;
	case MZ_NUKE8:
		dl->color = Vec3( 0.0f, 1.0f, 1.0f );
		dl->die = cl.time + 100;
		break;
		// PGM
		// ======================
	}

#endif
}


/*

==============

CL_ParseMuzzleFlash2

==============

*/
void Client::CL_ParseMuzzleFlash2( MessageBuffer & msg_buffer ) {

	int ent = msg_buffer.ReadShort( );
	int flash_number = msg_buffer.ReadByte( );

#if 0
	int ent;
	Vec3 origin;
	int flash_number;
	cdlight_t * dl;
	Vec3 forward, right;
	Str soundname;

	ent = msg_buffer.ReadShort( );
	if( ent < 1 || ent >= MAX_EDICTS )
		Common::Com_Error( ERR_DROP, "CL_ParseMuzzleFlash2: bad entity" );

	flash_number = msg_buffer.ReadByte( );

	// locate the origin
	cl_entities[ ent ].current.angles.AngleVectors( &forward, &right, NULL );
	origin[ 0 ] = cl_entities[ ent ].current.origin[ 0 ] + forward[ 0 ] * monster_flash_offset[ flash_number ][ 0 ] + right[ 0 ] * monster_flash_offset[ flash_number ][ 1 ];
	origin[ 1 ] = cl_entities[ ent ].current.origin[ 1 ] + forward[ 1 ] * monster_flash_offset[ flash_number ][ 0 ] + right[ 1 ] * monster_flash_offset[ flash_number ][ 1 ];
	origin[ 2 ] = cl_entities[ ent ].current.origin[ 2 ] + forward[ 2 ] * monster_flash_offset[ flash_number ][ 0 ] + right[ 2 ] * monster_flash_offset[ flash_number ][ 1 ] + monster_flash_offset[ flash_number ][ 2 ];

	dl = CL_AllocDlight( ent );
	dl->origin = origin;
	dl->radius = ( float )( 200 +( rand( )&31 ) );
	dl->minlight = 32;
	dl->die = cl.time; // + 0.1;

	switch( flash_number )
	{
	case MZ2_INFANTRY_MACHINEGUN_1:
	case MZ2_INFANTRY_MACHINEGUN_2:
	case MZ2_INFANTRY_MACHINEGUN_3:
	case MZ2_INFANTRY_MACHINEGUN_4:
	case MZ2_INFANTRY_MACHINEGUN_5:
	case MZ2_INFANTRY_MACHINEGUN_6:
	case MZ2_INFANTRY_MACHINEGUN_7:
	case MZ2_INFANTRY_MACHINEGUN_8:
	case MZ2_INFANTRY_MACHINEGUN_9:
	case MZ2_INFANTRY_MACHINEGUN_10:
	case MZ2_INFANTRY_MACHINEGUN_11:
	case MZ2_INFANTRY_MACHINEGUN_12:
	case MZ2_INFANTRY_MACHINEGUN_13:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		CL_ParticleEffect( origin, vec3_origin, 0, 40 );
		CL_SmokeAndFlash( origin );
		Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( Str( "infantry/infatck1.wav" ) ), 1, ATTN_NORM, 0 );
		break;

	case MZ2_SOLDIER_MACHINEGUN_1:
	case MZ2_SOLDIER_MACHINEGUN_2:
	case MZ2_SOLDIER_MACHINEGUN_3:
	case MZ2_SOLDIER_MACHINEGUN_4:
	case MZ2_SOLDIER_MACHINEGUN_5:
	case MZ2_SOLDIER_MACHINEGUN_6:
	case MZ2_SOLDIER_MACHINEGUN_7:
	case MZ2_SOLDIER_MACHINEGUN_8:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		CL_ParticleEffect( origin, vec3_origin, 0, 40 );
		CL_SmokeAndFlash( origin );
		Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( Str( "soldier/solatck3.wav" ) ), 1, ATTN_NORM, 0 );
		break;

	case MZ2_GUNNER_MACHINEGUN_1:
	case MZ2_GUNNER_MACHINEGUN_2:
	case MZ2_GUNNER_MACHINEGUN_3:
	case MZ2_GUNNER_MACHINEGUN_4:
	case MZ2_GUNNER_MACHINEGUN_5:
	case MZ2_GUNNER_MACHINEGUN_6:
	case MZ2_GUNNER_MACHINEGUN_7:
	case MZ2_GUNNER_MACHINEGUN_8:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		CL_ParticleEffect( origin, vec3_origin, 0, 40 );
		CL_SmokeAndFlash( origin );
		Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( Str( "gunner/gunatck2.wav" ) ), 1, ATTN_NORM, 0 );
		break;

	case MZ2_ACTOR_MACHINEGUN_1:
	case MZ2_SUPERTANK_MACHINEGUN_1:
	case MZ2_SUPERTANK_MACHINEGUN_2:
	case MZ2_SUPERTANK_MACHINEGUN_3:
	case MZ2_SUPERTANK_MACHINEGUN_4:
	case MZ2_SUPERTANK_MACHINEGUN_5:
	case MZ2_SUPERTANK_MACHINEGUN_6:
	case MZ2_TURRET_MACHINEGUN: // PGM
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );

		CL_ParticleEffect( origin, vec3_origin, 0, 40 );
		CL_SmokeAndFlash( origin );
		Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( Str( "infantry/infatck1.wav" ) ), 1, ATTN_NORM, 0 );
		break;

	case MZ2_BOSS2_MACHINEGUN_L1:
	case MZ2_BOSS2_MACHINEGUN_L2:
	case MZ2_BOSS2_MACHINEGUN_L3:
	case MZ2_BOSS2_MACHINEGUN_L4:
	case MZ2_BOSS2_MACHINEGUN_L5:
	case MZ2_CARRIER_MACHINEGUN_L1: // PMM
	case MZ2_CARRIER_MACHINEGUN_L2: // PMM
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );

		CL_ParticleEffect( origin, vec3_origin, 0, 40 );
		CL_SmokeAndFlash( origin );
		Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( Str( "infantry/infatck1.wav" ) ), 1, ATTN_NONE, 0 );
		break;

	case MZ2_SOLDIER_BLASTER_1:
	case MZ2_SOLDIER_BLASTER_2:
	case MZ2_SOLDIER_BLASTER_3:
	case MZ2_SOLDIER_BLASTER_4:
	case MZ2_SOLDIER_BLASTER_5:
	case MZ2_SOLDIER_BLASTER_6:
	case MZ2_SOLDIER_BLASTER_7:
	case MZ2_SOLDIER_BLASTER_8:
	case MZ2_TURRET_BLASTER: // PGM
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( Str( "soldier/solatck2.wav" ) ), 1, ATTN_NORM, 0 );
		break;

	case MZ2_FLYER_BLASTER_1:
	case MZ2_FLYER_BLASTER_2:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( Str( "flyer/flyatck3.wav" ) ), 1, ATTN_NORM, 0 );
		break;

	case MZ2_MEDIC_BLASTER_1:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( Str( "medic/medatck1.wav" ) ), 1, ATTN_NORM, 0 );
		break;

	case MZ2_HOVER_BLASTER_1:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( Str( "hover/hovatck1.wav" ) ), 1, ATTN_NORM, 0 );
		break;

	case MZ2_FLOAT_BLASTER_1:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( Str( "floater/fltatck1.wav" ) ), 1, ATTN_NORM, 0 );
		break;

	case MZ2_SOLDIER_SHOTGUN_1:
	case MZ2_SOLDIER_SHOTGUN_2:
	case MZ2_SOLDIER_SHOTGUN_3:
	case MZ2_SOLDIER_SHOTGUN_4:
	case MZ2_SOLDIER_SHOTGUN_5:
	case MZ2_SOLDIER_SHOTGUN_6:
	case MZ2_SOLDIER_SHOTGUN_7:
	case MZ2_SOLDIER_SHOTGUN_8:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		CL_SmokeAndFlash( origin );
		Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( Str( "soldier/solatck1.wav" ) ), 1, ATTN_NORM, 0 );
		break;

	case MZ2_TANK_BLASTER_1:
	case MZ2_TANK_BLASTER_2:
	case MZ2_TANK_BLASTER_3:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( Str( "tank/tnkatck3.wav" ) ), 1, ATTN_NORM, 0 );
		break;

	case MZ2_TANK_MACHINEGUN_1:
	case MZ2_TANK_MACHINEGUN_2:
	case MZ2_TANK_MACHINEGUN_3:
	case MZ2_TANK_MACHINEGUN_4:
	case MZ2_TANK_MACHINEGUN_5:
	case MZ2_TANK_MACHINEGUN_6:
	case MZ2_TANK_MACHINEGUN_7:
	case MZ2_TANK_MACHINEGUN_8:
	case MZ2_TANK_MACHINEGUN_9:
	case MZ2_TANK_MACHINEGUN_10:
	case MZ2_TANK_MACHINEGUN_11:
	case MZ2_TANK_MACHINEGUN_12:
	case MZ2_TANK_MACHINEGUN_13:
	case MZ2_TANK_MACHINEGUN_14:
	case MZ2_TANK_MACHINEGUN_15:
	case MZ2_TANK_MACHINEGUN_16:
	case MZ2_TANK_MACHINEGUN_17:
	case MZ2_TANK_MACHINEGUN_18:
	case MZ2_TANK_MACHINEGUN_19:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		CL_ParticleEffect( origin, vec3_origin, 0, 40 );
		CL_SmokeAndFlash( origin );
		sprintf( soundname, "tank/tnkatk2%c.wav", 'a' + rand( ) % 5 );
		Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( soundname ), 1, ATTN_NORM, 0 );
		break;

	case MZ2_CHICK_ROCKET_1:
	case MZ2_TURRET_ROCKET: // PGM
		dl->color = Vec3( 1.0f, 0.5f, 0.2f );
		Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( Str( "chick/chkatck2.wav" ) ), 1, ATTN_NORM, 0 );
		break;

	case MZ2_TANK_ROCKET_1:
	case MZ2_TANK_ROCKET_2:
	case MZ2_TANK_ROCKET_3:
		dl->color = Vec3( 1.0f, 0.5f, 0.2f );
		Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( Str( "tank/tnkatck1.wav" ) ), 1, ATTN_NORM, 0 );
		break;

	case MZ2_SUPERTANK_ROCKET_1:
	case MZ2_SUPERTANK_ROCKET_2:
	case MZ2_SUPERTANK_ROCKET_3:
	case MZ2_BOSS2_ROCKET_1:
	case MZ2_BOSS2_ROCKET_2:
	case MZ2_BOSS2_ROCKET_3:
	case MZ2_BOSS2_ROCKET_4:
	case MZ2_CARRIER_ROCKET_1:
		//	case MZ2_CARRIER_ROCKET_2:
		//	case MZ2_CARRIER_ROCKET_3:
		//	case MZ2_CARRIER_ROCKET_4:
		dl->color = Vec3( 1.0f, 0.5f, 0.2f );
		Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( Str( "tank/rocket.wav" ) ), 1, ATTN_NORM, 0 );
		break;

	case MZ2_GUNNER_GRENADE_1:
	case MZ2_GUNNER_GRENADE_2:
	case MZ2_GUNNER_GRENADE_3:
	case MZ2_GUNNER_GRENADE_4:
		dl->color = Vec3( 1.0f, 0.5f, 0.0f );
		Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( Str( "gunner/gunatck3.wav" ) ), 1, ATTN_NORM, 0 );
		break;

	case MZ2_GLADIATOR_RAILGUN_1:
		// PMM
	case MZ2_CARRIER_RAILGUN:
	case MZ2_WIDOW_RAIL:
		// pmm
		dl->color = Vec3( 0.5f, 0.5f, 1.0f );
		break;

		// --- Xian's shit starts ---
	case MZ2_MAKRON_BFG:
		dl->color = Vec3( 0.5f, 1.0f, 0.5f );
		//Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( "makron/bfg_fitpRenderer::wav" ), 1, ATTN_NORM, 0 );
		break;

	case MZ2_MAKRON_BLASTER_1:
	case MZ2_MAKRON_BLASTER_2:
	case MZ2_MAKRON_BLASTER_3:
	case MZ2_MAKRON_BLASTER_4:
	case MZ2_MAKRON_BLASTER_5:
	case MZ2_MAKRON_BLASTER_6:
	case MZ2_MAKRON_BLASTER_7:
	case MZ2_MAKRON_BLASTER_8:
	case MZ2_MAKRON_BLASTER_9:
	case MZ2_MAKRON_BLASTER_10:
	case MZ2_MAKRON_BLASTER_11:
	case MZ2_MAKRON_BLASTER_12:
	case MZ2_MAKRON_BLASTER_13:
	case MZ2_MAKRON_BLASTER_14:
	case MZ2_MAKRON_BLASTER_15:
	case MZ2_MAKRON_BLASTER_16:
	case MZ2_MAKRON_BLASTER_17:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( Str( "makron/blaster.wav" ) ), 1, ATTN_NORM, 0 );
		break;

	case MZ2_JORG_MACHINEGUN_L1:
	case MZ2_JORG_MACHINEGUN_L2:
	case MZ2_JORG_MACHINEGUN_L3:
	case MZ2_JORG_MACHINEGUN_L4:
	case MZ2_JORG_MACHINEGUN_L5:
	case MZ2_JORG_MACHINEGUN_L6:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		CL_ParticleEffect( origin, vec3_origin, 0, 40 );
		CL_SmokeAndFlash( origin );
		Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( Str( "boss3/xfitpRenderer::wav" ) ), 1, ATTN_NORM, 0 );
		break;

	case MZ2_JORG_MACHINEGUN_R1:
	case MZ2_JORG_MACHINEGUN_R2:
	case MZ2_JORG_MACHINEGUN_R3:
	case MZ2_JORG_MACHINEGUN_R4:
	case MZ2_JORG_MACHINEGUN_R5:
	case MZ2_JORG_MACHINEGUN_R6:
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		CL_ParticleEffect( origin, vec3_origin, 0, 40 );
		CL_SmokeAndFlash( origin );
		break;

	case MZ2_JORG_BFG_1:
		dl->color = Vec3( 0.5f, 1.0f, 0.5f );
		break;

	case MZ2_BOSS2_MACHINEGUN_R1:
	case MZ2_BOSS2_MACHINEGUN_R2:
	case MZ2_BOSS2_MACHINEGUN_R3:
	case MZ2_BOSS2_MACHINEGUN_R4:
	case MZ2_BOSS2_MACHINEGUN_R5:
	case MZ2_CARRIER_MACHINEGUN_R1: // PMM
	case MZ2_CARRIER_MACHINEGUN_R2: // PMM

		dl->color = Vec3( 1.0f, 1.0f, 0.0f );

		CL_ParticleEffect( origin, vec3_origin, 0, 40 );
		CL_SmokeAndFlash( origin );
		break;

		// ======
		// ROGUE
	case MZ2_STALKER_BLASTER:
	case MZ2_DAEDALUS_BLASTER:
	case MZ2_MEDIC_BLASTER_2:
	case MZ2_WIDOW_BLASTER:
	case MZ2_WIDOW_BLASTER_SWEEP1:
	case MZ2_WIDOW_BLASTER_SWEEP2:
	case MZ2_WIDOW_BLASTER_SWEEP3:
	case MZ2_WIDOW_BLASTER_SWEEP4:
	case MZ2_WIDOW_BLASTER_SWEEP5:
	case MZ2_WIDOW_BLASTER_SWEEP6:
	case MZ2_WIDOW_BLASTER_SWEEP7:
	case MZ2_WIDOW_BLASTER_SWEEP8:
	case MZ2_WIDOW_BLASTER_SWEEP9:
	case MZ2_WIDOW_BLASTER_100:
	case MZ2_WIDOW_BLASTER_90:
	case MZ2_WIDOW_BLASTER_80:
	case MZ2_WIDOW_BLASTER_70:
	case MZ2_WIDOW_BLASTER_60:
	case MZ2_WIDOW_BLASTER_50:
	case MZ2_WIDOW_BLASTER_40:
	case MZ2_WIDOW_BLASTER_30:
	case MZ2_WIDOW_BLASTER_20:
	case MZ2_WIDOW_BLASTER_10:
	case MZ2_WIDOW_BLASTER_0:
	case MZ2_WIDOW_BLASTER_10L:
	case MZ2_WIDOW_BLASTER_20L:
	case MZ2_WIDOW_BLASTER_30L:
	case MZ2_WIDOW_BLASTER_40L:
	case MZ2_WIDOW_BLASTER_50L:
	case MZ2_WIDOW_BLASTER_60L:
	case MZ2_WIDOW_BLASTER_70L:
	case MZ2_WIDOW_RUN_1:
	case MZ2_WIDOW_RUN_2:
	case MZ2_WIDOW_RUN_3:
	case MZ2_WIDOW_RUN_4:
	case MZ2_WIDOW_RUN_5:
	case MZ2_WIDOW_RUN_6:
	case MZ2_WIDOW_RUN_7:
	case MZ2_WIDOW_RUN_8:
		dl->color = Vec3( 0.0f, 1.0f, 0.0f );
		Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( Str( "tank/tnkatck3.wav" ) ), 1, ATTN_NORM, 0 );
		break;

	case MZ2_WIDOW_DISRUPTOR:
		dl->color = Vec3( -1.0f, -1.0f, -1.0f );
		Sound::S_StartSound( NULL, ent, CHAN_WEAPON, Sound::S_RegisterSound( Str( "weapons/disint2.wav" ) ), 1, ATTN_NORM, 0 );
		break;

	case MZ2_WIDOW_PLASMABEAM:
	case MZ2_WIDOW2_BEAMER_1:
	case MZ2_WIDOW2_BEAMER_2:
	case MZ2_WIDOW2_BEAMER_3:
	case MZ2_WIDOW2_BEAMER_4:
	case MZ2_WIDOW2_BEAMER_5:
	case MZ2_WIDOW2_BEAM_SWEEP_1:
	case MZ2_WIDOW2_BEAM_SWEEP_2:
	case MZ2_WIDOW2_BEAM_SWEEP_3:
	case MZ2_WIDOW2_BEAM_SWEEP_4:
	case MZ2_WIDOW2_BEAM_SWEEP_5:
	case MZ2_WIDOW2_BEAM_SWEEP_6:
	case MZ2_WIDOW2_BEAM_SWEEP_7:
	case MZ2_WIDOW2_BEAM_SWEEP_8:
	case MZ2_WIDOW2_BEAM_SWEEP_9:
	case MZ2_WIDOW2_BEAM_SWEEP_10:
	case MZ2_WIDOW2_BEAM_SWEEP_11:
		dl->radius = ( float )( 300 +( rand( )&100 ) );
		dl->color = Vec3( 1.0f, 1.0f, 0.0f );
		dl->die = cl.time + 200;
		break;
		// ROGUE
		// ======

		// --- Xian's shit ends ---

	}
#endif
}


/*

===============

CL_AddDLights



===============

*/
void Client::CL_AddDLights( ) {

}



/*
==============================================================
PARTICLE MANAGEMENT
==============================================================
*/

/*
===============
CL_ClearParticles
===============
*/
void Client::CL_ClearParticles( ) {

}


/*
===============
CL_ParticleEffect
Wall impact puffs
===============
*/
void Client::CL_ParticleEffect( Vec3 & org, Vec3 & dir, int color, int count ) {

	
}


/*

===============

CL_ParticleEffect2

===============

*/
void Client::CL_ParticleEffect2( Vec3 & org, Vec3 & dir, int color, int count ) {

	
}


// RAFAEL
/*

===============

CL_ParticleEffect3

===============

*/
void Client::CL_ParticleEffect3( Vec3 & org, Vec3 & dir, int color, int count ) {

}

/*

===============

CL_TeleporterParticles

===============

*/
void Client::CL_TeleporterParticles( entity_state_t * ent ) {


}


/*

===============

CL_LogoutEffect



===============

*/
void Client::CL_LogoutEffect( Vec3 & org, int type ) {

	
}


/*

===============

CL_ItemRespawnParticles



===============

*/
void Client::CL_ItemRespawnParticles( Vec3 & org ) {

}


/*

===============

CL_ExplosionParticles

===============

*/
void Client::CL_ExplosionParticles( Vec3 & org ) {

	
}


/*

===============

CL_BigTeleportParticles

===============

*/
void Client::CL_BigTeleportParticles( Vec3 & org ) {


}


/*

===============

CL_BlasterParticles



Wall impact puffs

===============

*/
void Client::CL_BlasterParticles( Vec3 & org, Vec3 & dir ) {

	
}


/*

===============

CL_BlasterTrail



===============

*/
void Client::CL_BlasterTrail( Vec3 & start, Vec3 & end ) {

	
}

/*

===============

CL_QuadTrail



===============

*/
void Client::CL_QuadTrail( Vec3 & start, Vec3 & end ) {


}

/*

===============

CL_FlagTrail



===============

*/
void Client::CL_FlagTrail( Vec3 & start, Vec3 & end, float color ) {


}

/*

===============

CL_DiminishingTrail



===============

*/
void Client::CL_DiminishingTrail( Vec3 & start, Vec3 & end, centity_t * old, int flags ) {

	
}

/*
===============
CL_RocketTrail
===============
*/
void Client::CL_RocketTrail( Vec3 & start, Vec3 & end, centity_t * old ) {


}

/*

===============

CL_RailTrail



===============

*/
void Client::CL_RailTrail( Vec3 & start, Vec3 & end ) {


}

// RAFAEL
/*

===============

CL_IonripperTrail

===============

*/
void Client::CL_IonripperTrail( Vec3 & start, Vec3 & ent ) {


}


/*

===============

CL_BubbleTrail



===============

*/
void Client::CL_BubbleTrail( Vec3 & start, Vec3 & end ) {

}


/*

===============

CL_FlyParticles

===============

*/
void Client::CL_FlyParticles( Vec3 & origin, int count ) {

	
}

void Client::CL_FlyEffect( centity_t * ent, Vec3 & origin ) {

	
}

/*

===============

CL_BFGExplosionParticles

===============

*/
//FIXME combined with CL_ExplosionParticles
void Client::CL_BFGExplosionParticles( Vec3 & org ) {

	
}


/*

===============

CL_TeleportParticles



===============

*/
void Client::CL_TeleportParticles( Vec3 & org ) {

}


/*

===============

CL_AddParticles

===============

*/
void Client::CL_AddParticles( ) {


}


/*

==============

CL_EntityEvent



An entity has just been parsed that has an event value



the female events are there for backwards compatability

==============

*/
extern struct sfx_s * cl_sfx_footsteps[ 4 ];

void Client::CL_EntityEvent( entity_state_t * ent ) {
#if 0
	switch( ent->eventNum ) {

	case EV_ITEM_RESPAWN:
		Sound::S_StartSound( NULL, ent->number, CHAN_WEAPON, Sound::S_RegisterSound( Str( "items/respawn1.wav" ) ), 1, ATTN_IDLE, 0 );
		CL_ItemRespawnParticles( ent->origin );
		break;

	case EV_PLAYER_TELEPORT:
		Sound::S_StartSound( NULL, ent->number, CHAN_WEAPON, Sound::S_RegisterSound( Str( "misc/tele1.wav" ) ), 1, ATTN_IDLE, 0 );
		CL_TeleportParticles( ent->origin );
		break;

	case EV_FOOTSTEP:
		if( cl_footsteps.GetBool( ) ) Sound::S_StartSound( NULL, ent->number, CHAN_BODY, cl_sfx_footsteps[rand( )&3], 1, ATTN_NORM, 0 );
		break;

	case EV_FALLSHORT:
		Sound::S_StartSound( NULL, ent->number, CHAN_AUTO, Sound::S_RegisterSound( Str( "player/land1.wav" ) ), 1, ATTN_NORM, 0 );
		break;

	case EV_FALL:
		Sound::S_StartSound( NULL, ent->number, CHAN_AUTO, Sound::S_RegisterSound( Str( "*fall2.wav" ) ), 1, ATTN_NORM, 0 );
		break;

	case EV_FALLFAR:
		Sound::S_StartSound( NULL, ent->number, CHAN_AUTO, Sound::S_RegisterSound( Str( "*fall1.wav" ) ), 1, ATTN_NORM, 0 );
		break;
	}
#endif
}


/*

==============

CL_ClearEffects



==============

*/
void Client::CL_ClearEffects( ) {

	CL_ClearParticles( );
	CL_ClearDlights( );
	CL_ClearLightStyles( );
}
