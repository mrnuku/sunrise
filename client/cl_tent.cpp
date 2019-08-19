#include "precompiled.h"
#pragma hdrstop

typedef enum {
	ex_free,
	ex_explosion,
	ex_misc,
	ex_flash,
	ex_mflash,
	ex_poly,
	ex_poly2
} exptype_t;

typedef struct explosion_s {

	exptype_t		type;
	SceneEntity		ent;

	int				frames;
	float			light;
	Vec3			lightcolor;
	timeTypeReal	start;
	int				baseframe;

} explosion_t;

explosion_t cl_explosions[ 32 ];

typedef struct {

	int				entity;
	int				dest_entity;
	RenderModel *	model;
	timeType		endtime;
	Vec3			offset;
	Vec3			start, end;

} beam_t;

beam_t cl_beams[ 32 ];
//PMM - added this for player-linked beams.  Currently only used by the plasma beam
beam_t cl_playerbeams[ 32 ];

typedef struct {

	SceneEntity		ent;
	timeType		endtime;

} laser_t;

laser_t cl_lasers[ 32 ];

//ROGUE
cl_sustain_t cl_sustains[ MAX_SUSTAINS ];
//ROGUE

//PGM
extern void CL_TeleportParticles( Vec3 & org );
//PGM

void CL_BlasterParticles( Vec3 & org, Vec3 & dir );
void CL_ExplosionParticles( Vec3 & org );
void CL_BFGExplosionParticles( Vec3 & org );
// RAFAEL
void CL_BlueBlasterParticles( Vec3 & org, Vec3 & dir );

struct sfx_s * cl_sfx_ric1;
struct sfx_s * cl_sfx_ric2;
struct sfx_s * cl_sfx_ric3;
struct sfx_s * cl_sfx_lashit;
struct sfx_s * cl_sfx_spark5;
struct sfx_s * cl_sfx_spark6;
struct sfx_s * cl_sfx_spark7;
struct sfx_s * cl_sfx_railg;
struct sfx_s * cl_sfx_rockexp;
struct sfx_s * cl_sfx_grenexp;
struct sfx_s * cl_sfx_watrexp;
// RAFAEL
struct sfx_s * cl_sfx_plasexp;
struct sfx_s * cl_sfx_footsteps[ 4 ];

RenderModel * cl_mod_explode;
RenderModel * cl_mod_smoke;
RenderModel * cl_mod_flash;
RenderModel * cl_mod_parasite_segment;
RenderModel * cl_mod_grapple_cable;
RenderModel * cl_mod_parasite_tip;
RenderModel * cl_mod_explo4;
RenderModel * cl_mod_bfg_explo;
RenderModel * cl_mod_powerscreen;
// RAFAEL
RenderModel * cl_mod_plasmaexplo;

//ROGUE
struct sfx_s * cl_sfx_lightning;
struct sfx_s * cl_sfx_disrexp;
RenderModel * cl_mod_lightning;
RenderModel * cl_mod_heatbeam;
RenderModel * cl_mod_monster_heatbeam;
RenderModel * cl_mod_explo4_big;

//ROGUE

void Client::CL_RegisterTEntSounds( ) {
#if 0
	int i;
	Str name;

	// PMM - version stuff
	//	Common::Com_Printf( "%s\n", ROGUE_VERSION_STRING );
	// PMM
	cl_sfx_ric1 = Sound::S_RegisterSound( Str( "world/ric1.wav" ) );
	cl_sfx_ric2 = Sound::S_RegisterSound( Str( "world/ric2.wav" ) );
	cl_sfx_ric3 = Sound::S_RegisterSound( Str( "world/ric3.wav" ) );
	cl_sfx_lashit = Sound::S_RegisterSound( Str( "weapons/lashit.wav" ) );
	cl_sfx_spark5 = Sound::S_RegisterSound( Str( "world/spark5.wav" ) );
	cl_sfx_spark6 = Sound::S_RegisterSound( Str( "world/spark6.wav" ) );
	cl_sfx_spark7 = Sound::S_RegisterSound( Str( "world/spark7.wav" ) );
	cl_sfx_railg = Sound::S_RegisterSound( Str( "weapons/railgf1a.wav" ) );
	cl_sfx_rockexp = Sound::S_RegisterSound( Str( "weapons/rocklx1a.wav" ) );
	cl_sfx_grenexp = Sound::S_RegisterSound( Str( "weapons/grenlx1a.wav" ) );
	cl_sfx_watrexp = Sound::S_RegisterSound( Str( "weapons/xpld_wat.wav" ) );
	// RAFAEL
	// cl_sfx_plasexp = Sound::S_RegisterSound( "weapons/plasexpl.wav" );
	Sound::S_RegisterSound( Str( "player/land1.wav" ) );

	Sound::S_RegisterSound( Str( "player/fall2.wav" ) );
	Sound::S_RegisterSound( Str( "player/fall1.wav" ) );

	for( i = 0; i<4; i++ )
	{
		sprintf( name, "player/step%i.wav", i+1 );
		cl_sfx_footsteps[ i ] = Sound::S_RegisterSound( name );
	}

	//PGM
	cl_sfx_lightning = Sound::S_RegisterSound( Str( "weapons/tesla.wav" ) );
	cl_sfx_disrexp = Sound::S_RegisterSound( Str( "weapons/disrupthit.wav" ) );
	// version stuff
	//	sprintf( name, "weapons/sound%d.wav", ROGUE_VERSION_ID );
	//	if( name[ 0 ] == 'w' )
	//		name[ 0 ] = 'W';
	//PGM
#endif
}

void Client::CL_RegisterTEntModels( ) {
#if 0
	cl_mod_explode = Renderer::RegisterModel( Str( "models/objects/explode/tris.md2" ) );
	cl_mod_smoke = Renderer::RegisterModel( Str( "models/objects/smoke/tris.md2" ) );
	cl_mod_flash = Renderer::RegisterModel( Str( "models/objects/flash/tris.md2" ) );
	cl_mod_parasite_segment = Renderer::RegisterModel( Str( "models/monsters/parasite/segment/tris.md2" ) );
	cl_mod_grapple_cable = Renderer::RegisterModel( Str( "models/ctf/segment/tris.md2" ) );
	cl_mod_parasite_tip = Renderer::RegisterModel( Str( "models/monsters/parasite/tip/tris.md2" ) );
	cl_mod_explo4 = Renderer::RegisterModel( Str( "models/objects/r_explode/tris.md2" ) );
	cl_mod_bfg_explo = Renderer::RegisterModel( Str( "sprites/s_bfg2.sp2" ) );
	cl_mod_powerscreen = Renderer::RegisterModel( Str( "models/items/armor/effect/tris.md2" ) );

	Renderer::RegisterModel( Str( "models/objects/laser/tris.md2" ) );
	Renderer::RegisterModel( Str( "models/objects/grenade2/tris.md2" ) );
	Renderer::RegisterModel( Str( "models/weapons/v_machn/tris.md2" ) );
	Renderer::RegisterModel( Str( "models/weapons/v_handgr/tris.md2" ) );
	Renderer::RegisterModel( Str( "models/weapons/v_shotg2/tris.md2" ) );
	Renderer::RegisterModel( Game::boneGib );
	Renderer::RegisterModel( Game::meatGib );
	Renderer::RegisterModel( Str( "models/objects/gibs/bone2/tris.md2" ) );
	// RAFAEL
	// Renderer::RegisterModel( "models/objects/blaser/tris.md2" );

	//MaterialSystem::FindTexture( "pics/w_machinegun.pcx", "hud" );
	//MaterialSystem::FindTexture( "pics/a_bullets.pcx", "hud" );
	//MaterialSystem::FindTexture( "pics/i_health.pcx", "hud" );
	//MaterialSystem::FindTexture( "pics/a_grenades.pcx", "hud" );

	//ROGUE
	cl_mod_explo4_big = Renderer::RegisterModel( Str( "models/objects/r_explode2/tris.md2" ) );
	cl_mod_lightning = Renderer::RegisterModel( Str( "models/proj/lightning/tris.md2" ) );
	cl_mod_heatbeam = Renderer::RegisterModel( Str( "models/proj/beam/tris.md2" ) );
	cl_mod_monster_heatbeam = Renderer::RegisterModel( Str( "models/proj/widowbeam/tris.md2" ) );
	//ROGUE
#endif
}

void Client::CL_ClearTEnts( ) {

	Common::Com_Memset( cl_beams, 0, sizeof( cl_beams ) );
	Common::Com_Memset( cl_explosions, 0, sizeof( cl_explosions ) );
	Common::Com_Memset( cl_lasers, 0, sizeof( cl_lasers ) );

	//ROGUE
	Common::Com_Memset( cl_playerbeams, 0, sizeof( cl_playerbeams ) );
	Common::Com_Memset( cl_sustains, 0, sizeof( cl_sustains ) );
	//ROGUE
}

explosion_t * Client::CL_AllocExplosion( ) {
	int i;
	timeType time;
	int index;
	for( i = 0; i<32; i++ ) 	{
		if( cl_explosions[ i ].type == ex_free ) {
			Common::Com_Memset( &cl_explosions[ i ], 0, sizeof( cl_explosions[ i ] ) );
			return &cl_explosions[ i ];
		}
	}
	// find the oldest explosion
	time = cl.time;
	index = 0;
	for( i = 0; i<32; i++ ) {
		if( ( timeType )cl_explosions[ i ].start < time ) {
			time = ( timeType )cl_explosions[ i ].start;
			index = i;
		}
		Common::Com_Memset( &cl_explosions[ index ], 0, sizeof( cl_explosions[ index ] ) );
		return &cl_explosions[ index ];
	}
	return NULL;
}

void Client::CL_SmokeAndFlash( Vec3 & origin ) {

	explosion_t * ex;

	ex = CL_AllocExplosion( );
	ex->ent.origin = origin;
	ex->type = ex_misc;
	ex->frames = 4;
	ex->ent.flags = RF_TRANSLUCENT;
	ex->start = ( timeTypeReal )cl.frame.servertime - 100;
	ex->ent.model = cl_mod_smoke;

	ex = CL_AllocExplosion( );
	ex->ent.origin = origin;
	ex->type = ex_flash;
	ex->ent.flags = RF_FULLBRIGHT;
	ex->frames = 2;
	ex->start = ( timeTypeReal )cl.frame.servertime - 100;
	ex->ent.model = cl_mod_flash;
}

void Client::CL_ParseParticles( MessageBuffer & msg_buffer ) {

	int color, count;
	Vec3 pos, dir;

	msg_buffer.ReadVec3( pos );
	dir = msg_buffer.ReadDir( 24 );

	color = msg_buffer.ReadByte( );

	count = msg_buffer.ReadByte( );

	CL_ParticleEffect( pos, dir, color, count );
}

int Client::CL_ParseBeam( MessageBuffer & msg_buffer, RenderModel * model ) {

	int ent;
	Vec3 start, end;
	beam_t * b;
	int i;

	ent = msg_buffer.ReadShort( );

	msg_buffer.ReadVec3( start );
	msg_buffer.ReadVec3( end );

	// override any beam with the same entity
	for( i = 0, b = cl_beams; i< 32; i++, b++ )
		if( b->entity == ent )
		{
			b->entity = ent;
			b->model = model;
			b->endtime = cl.time + 200;
			b->start = start;
			b->end = end;
			b->offset = vec3_origin;
			return ent;
		}

		// find a free beam
		for( i = 0, b = cl_beams; i< 32; i++, b++ )
		{
			if( !b->model || b->endtime < cl.time )
			{
				b->entity = ent;
				b->model = model;
				b->endtime = cl.time + 200;
				b->start = start;
				b->end = end;
				b->offset = vec3_origin;
				return ent;
			}
		}
		Common::Com_Printf( "beam list overflow!\n" );
		return ent;
}

int Client::CL_ParseBeam2( MessageBuffer & msg_buffer, RenderModel * model ) {

	int ent;
	Vec3 start, end, offset;
	beam_t * b;
	int i;

	ent = msg_buffer.ReadShort( );

	msg_buffer.ReadVec3( start );
	msg_buffer.ReadVec3( end );
	msg_buffer.ReadVec3( offset );

	//	Common::Com_Printf( "end- %f %f %f\n", end[ 0 ], end[ 1 ], end[ 2 ] );

	// override any beam with the same entity

	for( i = 0, b = cl_beams; i< 32; i++, b++ )
		if( b->entity == ent )
		{
			b->entity = ent;
			b->model = model;
			b->endtime = cl.time + 200;
			b->start = start;
			b->end = end;
			b->offset = offset;
			return ent;
		}

		// find a free beam
		for( i = 0, b = cl_beams; i< 32; i++, b++ )
		{
			if( !b->model || b->endtime < cl.time )
			{
				b->entity = ent;
				b->model = model;
				b->endtime = cl.time + 200;
				b->start = start;
				b->end = end;
				b->offset = offset;
				return ent;
			}
		}
		Common::Com_Printf( "beam list overflow!\n" );
		return ent;
}

int Client::CL_ParsePlayerBeam( MessageBuffer & msg_buffer, RenderModel * model ) {

	int ent;
	Vec3 start, end, offset;
	beam_t * b;
	int i;

	ent = msg_buffer.ReadShort( );

	msg_buffer.ReadVec3( start );
	msg_buffer.ReadVec3( end );
	// PMM - network optimization
	if( model == cl_mod_heatbeam )
		offset = Vec3( 2, 7, -3 );
	else if( model == cl_mod_monster_heatbeam ) {
		model = cl_mod_heatbeam;
		offset = Vec3( 0, 0, 0 );
	} else
		msg_buffer.ReadVec3( offset );

	//	Common::Com_Printf( "end- %f %f %f\n", end[ 0 ], end[ 1 ], end[ 2 ] );

	// override any beam with the same entity
	// PMM - For player beams, we only want one per player( entity ) so..
	for( i = 0, b = cl_playerbeams; i< 32; i++, b++ ) {
		if( b->entity == ent ) {
			b->entity = ent;
			b->model = model;
			b->endtime = cl.time + 200;
			b->start = start;
			b->end = end;
			b->offset = offset;
			return ent;
		}
	}
	// find a free beam
	for( i = 0, b = cl_playerbeams; i< 32; i++, b++ ) {
		if( !b->model || b->endtime < cl.time ) {
			b->entity = ent;
			b->model = model;
			b->endtime = cl.time + 100; // PMM - this needs to be 100 to prevent multiple heatbeams
			b->start = start;
			b->end = end;
			b->offset = offset;
			return ent;
		}
	}
	Common::Com_Printf( "beam list overflow!\n" );
	return ent;
}

int Client::CL_ParseLightning( MessageBuffer & msg_buffer, RenderModel * model ) {

	int srcEnt, destEnt;
	Vec3 start, end;
	beam_t * b;
	int i;

	srcEnt = msg_buffer.ReadShort( );
	destEnt = msg_buffer.ReadShort( );

	msg_buffer.ReadVec3( start );
	msg_buffer.ReadVec3( end );

	// override any beam with the same source AND destination entities
	for( i = 0, b = cl_beams; i< 32; i++, b++ )
		if( b->entity == srcEnt && b->dest_entity == destEnt )
		{
			//			Common::Com_Printf( "%d: OVERRIDE  %d -> %d\n", cl.time, srcEnt, destEnt );
			b->entity = srcEnt;
			b->dest_entity = destEnt;
			b->model = model;
			b->endtime = cl.time + 200;
			b->start = start;
			b->end = end;
			b->offset = vec3_origin;
			return srcEnt;
		}

		// find a free beam
		for( i = 0, b = cl_beams; i< 32; i++, b++ )
		{
			if( !b->model || b->endtime < cl.time )
			{
				//			Common::Com_Printf( "%d: NORMAL  %d -> %d\n", cl.time, srcEnt, destEnt );
				b->entity = srcEnt;
				b->dest_entity = destEnt;
				b->model = model;
				b->endtime = cl.time + 200;
				b->start = start;
				b->end = end;
				b->offset = vec3_origin;
				return srcEnt;
			}
		}
		Common::Com_Printf( "beam list overflow!\n" );
		return srcEnt;
}

void Client::CL_ParseLaser( MessageBuffer & msg_buffer, int colors ) {

	Vec3 start;
	Vec3 end;
	laser_t * l;
	int i;

	msg_buffer.ReadVec3( start );
	msg_buffer.ReadVec3( end );

	for( i = 0, l = cl_lasers; i< 32; i++, l++ )
	{
		if( l->endtime < cl.time )
		{
			l->ent.flags = RF_TRANSLUCENT | RF_BEAM;
			l->ent.origin = start;
			l->ent.oldorigin = end;
			l->ent.alpha = 0.30f;
			l->ent.skinnum =( colors >>( ( rand( ) % 4 )* 8 ) ) & 0xFF;
			l->ent.model = NULL;
			l->ent.frame = 4;
			l->endtime = cl.time + 100;
			return;
		}
	}
}

void Client::CL_ParseSteam( MessageBuffer & msg_buffer ) {

	Vec3 pos, dir;
	int id, i;
	int r;
	int cnt;
	int color;
	int magnitude;
	cl_sustain_t * s, * free_sustain;

	id = msg_buffer.ReadShort( ); // an  of -1 is an instant effect
	if( id != -1 ) // sustains
	{
		//			Common::Com_Printf( "Sustain effect  %d\n",  );
		free_sustain = NULL;
		for( i = 0, s = cl_sustains; i<MAX_SUSTAINS; i++, s++ )
		{
			if( s->id == 0 )
			{
				free_sustain = s;
				break;
			}
		}
		if( free_sustain )
		{
			s->id = id;
			s->count = msg_buffer.ReadByte( );
			msg_buffer.ReadVec3( s->org );
			s->dir = msg_buffer.ReadDir( 24 );
			r = msg_buffer.ReadByte( );
			s->color = r & 0xFF;
			s->magnitude = msg_buffer.ReadShort( );
			s->endtime = cl.time + msg_buffer.ReadLong( );
			s->think = CL_ParticleSteamEffect2;
			s->thinkinterval = 100;
			s->nextthink = cl.time;
		}
		else
		{
			//				Common::Com_Printf( "No free sustains!\n" );
			// FIXME - read the stuff anyway
			cnt = msg_buffer.ReadByte( );
			msg_buffer.ReadVec3( pos );
			dir = msg_buffer.ReadDir( 24 );
			r = msg_buffer.ReadByte( );
			magnitude = msg_buffer.ReadShort( );
			magnitude = msg_buffer.ReadLong( ); // really interval
		}
	}
	else // instant
	{
		cnt = msg_buffer.ReadByte( );
		msg_buffer.ReadVec3( pos );
		dir = msg_buffer.ReadDir( 24 );
		r = msg_buffer.ReadByte( );
		magnitude = msg_buffer.ReadShort( );
		color = r & 0xFF;
		CL_ParticleSteamEffect( pos, dir, color, cnt, magnitude );
		//		Sound::S_StartSound( pos,  0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0 );
	}
}

void Client::CL_ParseWidow( MessageBuffer & msg_buffer ) {

	Vec3 pos;
	int id, i;
	cl_sustain_t * s, * free_sustain;

	id = msg_buffer.ReadShort( );

	free_sustain = NULL;
	for( i = 0, s = cl_sustains; i<MAX_SUSTAINS; i++, s++ )
	{
		if( s->id == 0 )
		{
			free_sustain = s;
			break;
		}
	}
	if( free_sustain )
	{
		s->id = id;
		msg_buffer.ReadVec3( s->org );
		s->endtime = cl.time + 2100;
		s->think = CL_Widowbeamout;
		s->thinkinterval = 1;
		s->nextthink = cl.time;
	}
	else // no free sustains
	{
		// FIXME - read the stuff anyway
		msg_buffer.ReadVec3( pos );
	}
}

void Client::CL_ParseNuke( MessageBuffer & msg_buffer ) {

	Vec3 pos;
	int i;
	cl_sustain_t * s, * free_sustain;

	free_sustain = NULL;
	for( i = 0, s = cl_sustains; i<MAX_SUSTAINS; i++, s++ )
	{
		if( s->id == 0 )
		{
			free_sustain = s;
			break;
		}
	}
	if( free_sustain )
	{
		s->id = 21000;
		msg_buffer.ReadVec3( s->org );
		s->endtime = cl.time + 1000;
		s->think = CL_Nukeblast;
		s->thinkinterval = 1;
		s->nextthink = cl.time;
	}
	else // no free sustains
	{
		// FIXME - read the stuff anyway
		msg_buffer.ReadVec3( pos );
	}
}

static byte splash_color[ ] = { 0x00, 0xE0, 0xB0, 0x50, 0xD0, 0xE0, 0xE8 };

void Client::CL_ParseTEnt( MessageBuffer & msg_buffer ) {

	int type;
	Vec3 pos, pos2, dir;
	explosion_t * ex;
	int cnt;
	int color;
	int r;
	int ent;
	int magnitude;

	type = msg_buffer.ReadByte( );

	switch( type )
	{
	case TE_BLOOD: // bullet hitting flesh
		msg_buffer.ReadVec3( pos );
		dir = msg_buffer.ReadDir( 24 );
		CL_ParticleEffect( pos, dir, 0xE8, 60 );
		break;

	case TE_GUNSHOT: // bullet hitting wall
	case TE_SPARKS:
	case TE_BULLET_SPARKS:
		msg_buffer.ReadVec3( pos );
		dir = msg_buffer.ReadDir( 24 );
		if( type == TE_GUNSHOT )
			CL_ParticleEffect( pos, dir, 0, 40 );
		else
			CL_ParticleEffect( pos, dir, 0xE0, 6 );

		if( type != TE_SPARKS )
		{
			CL_SmokeAndFlash( pos );

			// impact sound
			cnt = rand( )&15;
			/*if( cnt == 1 )
				//Sound::S_StartSound( &pos, 0, 0, cl_sfx_ric1, 1, ATTN_NORM, 0 );
			else if( cnt == 2 )
				//Sound::S_StartSound( &pos, 0, 0, cl_sfx_ric2, 1, ATTN_NORM, 0 );
			else if( cnt == 3 )
				//Sound::S_StartSound( &pos, 0, 0, cl_sfx_ric3, 1, ATTN_NORM, 0 );*/
		}

		break;

	case TE_SCREEN_SPARKS:
	case TE_SHIELD_SPARKS:
		msg_buffer.ReadVec3( pos );
		dir = msg_buffer.ReadDir( 24 );
		if( type == TE_SCREEN_SPARKS )
			CL_ParticleEffect( pos, dir, 0xD0, 40 );
		else
			CL_ParticleEffect( pos, dir, 0xB0, 40 );
		//FIXME : replace or remove this sound
		//Sound::S_StartSound( &pos, 0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0 );
		break;

	case TE_SHOTGUN: // bullet hitting wall
		msg_buffer.ReadVec3( pos );
		dir = msg_buffer.ReadDir( 24 );
		CL_ParticleEffect( pos, dir, 0, 20 );
		CL_SmokeAndFlash( pos );
		break;

	case TE_SPLASH: // bullet hitting water
		cnt = msg_buffer.ReadByte( );
		msg_buffer.ReadVec3( pos );
		dir = msg_buffer.ReadDir( 24 );
		r = msg_buffer.ReadByte( );
		if( r > 6 )
			color = 0x00;
		else
			color = splash_color[ r ];
		CL_ParticleEffect( pos, dir, color, cnt );

		if( r == SPLASH_SPARKS )
		{
			r = rand( ) & 3;
			/*if( r == 0 )
				//Sound::S_StartSound( &pos, 0, 0, cl_sfx_spark5, 1, ATTN_STATIC, 0 );
			else if( r == 1 )
				//Sound::S_StartSound( &pos, 0, 0, cl_sfx_spark6, 1, ATTN_STATIC, 0 );
			else
				//Sound::S_StartSound( &pos, 0, 0, cl_sfx_spark7, 1, ATTN_STATIC, 0 );*/
		}
		break;

	case TE_LASER_SPARKS:
		cnt = msg_buffer.ReadByte( );
		msg_buffer.ReadVec3( pos );
		dir = msg_buffer.ReadDir( 24 );
		color = msg_buffer.ReadByte( );
		CL_ParticleEffect2( pos, dir, color, cnt );
		break;

		// RAFAEL
	case TE_BLUEHYPERBLASTER:
		msg_buffer.ReadVec3( pos );
		msg_buffer.ReadVec3( dir );
		CL_BlasterParticles( pos, dir );
		break;

	case TE_BLASTER: // blaster hitting wall
		msg_buffer.ReadVec3( pos );
		dir = msg_buffer.ReadDir( 24 );
		CL_BlasterParticles( pos, dir );

		ex = CL_AllocExplosion( );
		ex->ent.origin = pos;
		ex->ent.angles[ 0 ] = acos( dir[ 2 ] )/M_PI* 180;
		// PMM - fixed to correct for pitch of 0
		if( dir[ 0 ] )
			ex->ent.angles[ 1 ] = atan2( dir[ 1 ], dir[ 0 ] )/M_PI* 180;
		else if( dir[ 1 ] > 0 )
			ex->ent.angles[ 1 ] = 90;
		else if( dir[ 1 ] < 0 )
			ex->ent.angles[ 1 ] = 270;
		else
			ex->ent.angles[ 1 ] = 0;

		ex->type = ex_misc;
		ex->ent.flags = RF_FULLBRIGHT|RF_TRANSLUCENT;
		ex->start = ( timeTypeReal )cl.frame.servertime - 100;
		ex->light = 150;
		ex->lightcolor[ 0 ] = 1;
		ex->lightcolor[ 1 ] = 1;
		ex->ent.model = cl_mod_explode;
		ex->frames = 4;
		//Sound::S_StartSound( &pos, 0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0 );
		break;

	case TE_RAILTRAIL: // railgun effect
		msg_buffer.ReadVec3( pos );
		msg_buffer.ReadVec3( dir );
		CL_RailTrail( pos, pos2 );
		//Sound::S_StartSound( &pos2, 0, 0, cl_sfx_railg, 1, ATTN_NORM, 0 );
		break;

	case TE_EXPLOSION2:
	case TE_GRENADE_EXPLOSION:
	case TE_GRENADE_EXPLOSION_WATER:
		msg_buffer.ReadVec3( pos );

		ex = CL_AllocExplosion( );
		ex->ent.origin = pos;
		ex->type = ex_poly;
		ex->ent.flags = RF_FULLBRIGHT;
		ex->start = ( timeTypeReal )cl.frame.servertime - 100;
		ex->light = 350;
		ex->lightcolor[ 0 ] = 1.0f;
		ex->lightcolor[ 1 ] = 0.5f;
		ex->lightcolor[ 2 ] = 0.5f;
		ex->ent.model = cl_mod_explo4;
		ex->frames = 19;
		ex->baseframe = 30;
		ex->ent.angles[ 1 ] = ( float )( rand( ) % 360 );
		CL_ExplosionParticles( pos );
		/*if( type == TE_GRENADE_EXPLOSION_WATER )
			//Sound::S_StartSound( &pos, 0, 0, cl_sfx_watrexp, 1, ATTN_NORM, 0 );
		else
			//Sound::S_StartSound( &pos, 0, 0, cl_sfx_grenexp, 1, ATTN_NORM, 0 );*/
		break;

		// RAFAEL
	case TE_PLASMA_EXPLOSION:
		msg_buffer.ReadVec3( pos );
		ex = CL_AllocExplosion( );
		ex->ent.origin = pos;
		ex->type = ex_poly;
		ex->ent.flags = RF_FULLBRIGHT;
		ex->start = ( timeTypeReal )cl.frame.servertime - 100;
		ex->light = 350;
		ex->lightcolor[ 0 ] = 1.0f;
		ex->lightcolor[ 1 ] = 0.5f;
		ex->lightcolor[ 2 ] = 0.5f;
		ex->ent.angles[ 1 ] = ( float )( rand( ) % 360 );
		ex->ent.model = cl_mod_explo4;
		if( frand( ) < 0.5f )
			ex->baseframe = 15;
		ex->frames = 15;
		CL_ExplosionParticles( pos );
		//Sound::S_StartSound( &pos, 0, 0, cl_sfx_rockexp, 1, ATTN_NORM, 0 );
		break;

	case TE_EXPLOSION1:
	case TE_EXPLOSION1_BIG: // PMM
	case TE_ROCKET_EXPLOSION:
	case TE_ROCKET_EXPLOSION_WATER:
	case TE_EXPLOSION1_NP: // PMM
		msg_buffer.ReadVec3( pos );

		ex = CL_AllocExplosion( );
		ex->ent.origin = pos;
		ex->type = ex_poly;
		ex->ent.flags = RF_FULLBRIGHT;
		ex->start = ( timeTypeReal )cl.frame.servertime - 100;
		ex->light = 350;
		ex->lightcolor[ 0 ] = 1.0f;
		ex->lightcolor[ 1 ] = 0.5f;
		ex->lightcolor[ 2 ] = 0.5f;
		ex->ent.angles[ 1 ] = ( float )( rand( ) % 360 );
		if( type != TE_EXPLOSION1_BIG ) // PMM
			ex->ent.model = cl_mod_explo4; // PMM
		else
			ex->ent.model = cl_mod_explo4_big;
		if( frand( ) < 0.5f )
			ex->baseframe = 15;
		ex->frames = 15;
		if( ( type != TE_EXPLOSION1_BIG ) &&( type != TE_EXPLOSION1_NP ) ) // PMM
			CL_ExplosionParticles( pos ); // PMM
		/*if( type == TE_ROCKET_EXPLOSION_WATER )
			//Sound::S_StartSound( &pos, 0, 0, cl_sfx_watrexp, 1, ATTN_NORM, 0 );
		else
			//Sound::S_StartSound( &pos, 0, 0, cl_sfx_rockexp, 1, ATTN_NORM, 0 );*/
		break;

	case TE_BFG_EXPLOSION:
		msg_buffer.ReadVec3( pos );
		ex = CL_AllocExplosion( );
		ex->ent.origin = pos;
		ex->type = ex_poly;
		ex->ent.flags = RF_FULLBRIGHT;
		ex->start = ( timeTypeReal )cl.frame.servertime - 100;
		ex->light = 350;
		ex->lightcolor[ 0 ] = 0.0f;
		ex->lightcolor[ 1 ] = 1.0f;
		ex->lightcolor[ 2 ] = 0.0f;
		ex->ent.model = cl_mod_bfg_explo;
		ex->ent.flags |= RF_TRANSLUCENT;
		ex->ent.alpha = 0.30f;
		ex->frames = 4;
		break;

	case TE_BFG_BIGEXPLOSION:
		msg_buffer.ReadVec3( pos );
		CL_BFGExplosionParticles( pos );
		break;

	case TE_BFG_LASER:
		CL_ParseLaser( msg_buffer, 0xD0D1D2D3 );
		break;

	case TE_BUBBLETRAIL:
		msg_buffer.ReadVec3( pos );
		msg_buffer.ReadVec3( pos2 );
		CL_BubbleTrail( pos, pos2 );
		break;

	case TE_PARASITE_ATTACK:
	case TE_MEDIC_CABLE_ATTACK:
		ent = CL_ParseBeam( msg_buffer, cl_mod_parasite_segment );
		break;

	case TE_BOSSTPORT: // boss teleporting to station
		msg_buffer.ReadVec3( pos );
		CL_BigTeleportParticles( pos );
		//Sound::S_StartSound( &pos, 0, 0, Sound::S_RegisterSound( Str( "misc/bigtele.wav" ) ), 1, ATTN_NONE, 0 );
		break;

	case TE_GRAPPLE_CABLE:
		ent = CL_ParseBeam2( msg_buffer, cl_mod_grapple_cable );
		break;

		// RAFAEL
	case TE_WELDING_SPARKS:
		cnt = msg_buffer.ReadByte( );
		msg_buffer.ReadVec3( pos );
		dir = msg_buffer.ReadDir( 24 );
		color = msg_buffer.ReadByte( );
		CL_ParticleEffect2( pos, dir, color, cnt );

		ex = CL_AllocExplosion( );
		ex->ent.origin = pos;
		ex->type = ex_flash;
		// note to self
		// we need a better no draw flag
		ex->ent.flags = RF_BEAM;
		ex->start = cl.frame.servertime - 0.1f;
		ex->light = ( float )( 100 +( rand( )%75 ) );
		ex->lightcolor[ 0 ] = 1.0f;
		ex->lightcolor[ 1 ] = 1.0f;
		ex->lightcolor[ 2 ] = 0.3f;
		ex->ent.model = cl_mod_flash;
		ex->frames = 2;
		break;

	case TE_GREENBLOOD:
		msg_buffer.ReadVec3( pos );
		dir = msg_buffer.ReadDir( 24 );
		CL_ParticleEffect2( pos, dir, 0xDF, 30 );
		break;

		// RAFAEL
	case TE_TUNNEL_SPARKS:
		cnt = msg_buffer.ReadByte( );
		msg_buffer.ReadVec3( pos );
		dir = msg_buffer.ReadDir( 24 );
		color = msg_buffer.ReadByte( );
		CL_ParticleEffect3( pos, dir, color, cnt );
		break;

		//=============
		//PGM
		// PMM -following code integrated for flechette( different color )
	case TE_BLASTER2: // green blaster hitting wall
	case TE_FLECHETTE: // flechette
		msg_buffer.ReadVec3( pos );
		dir = msg_buffer.ReadDir( 24 );

		// PMM
		if( type == TE_BLASTER2 )
			CL_BlasterParticles2( pos, dir, 0xD0 );
		else
			CL_BlasterParticles2( pos, dir, 0x6F ); // 75

		ex = CL_AllocExplosion( );
		ex->ent.origin = pos;
		ex->ent.angles[ 0 ] = acos( dir[ 2 ] )/M_PI* 180;
		// PMM - fixed to correct for pitch of 0
		if( dir[ 0 ] )
			ex->ent.angles[ 1 ] = atan2( dir[ 1 ], dir[ 0 ] )/M_PI* 180;
		else if( dir[ 1 ] > 0 )
			ex->ent.angles[ 1 ] = 90;
		else if( dir[ 1 ] < 0 )
			ex->ent.angles[ 1 ] = 270;
		else
			ex->ent.angles[ 1 ] = 0;

		ex->type = ex_misc;
		ex->ent.flags = RF_FULLBRIGHT|RF_TRANSLUCENT;

		// PMM
		if( type == TE_BLASTER2 )
			ex->ent.skinnum = 1;
		else // flechette
			ex->ent.skinnum = 2;

		ex->start = ( timeTypeReal )cl.frame.servertime - 100;
		ex->light = 150;
		// PMM
		if( type == TE_BLASTER2 )
			ex->lightcolor[ 1 ] = 1;
		else // flechette
		{
			ex->lightcolor[ 0 ] = 0.19f;
			ex->lightcolor[ 1 ] = 0.41f;
			ex->lightcolor[ 2 ] = 0.75f;
		}
		ex->ent.model = cl_mod_explode;
		ex->frames = 4;
		//Sound::S_StartSound( &pos, 0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0 );
		break;


	case TE_LIGHTNING:
		ent = CL_ParseLightning( msg_buffer, cl_mod_lightning );
		//Sound::S_StartSound( NULL, ent, CHAN_WEAPON, cl_sfx_lightning, 1, ATTN_NORM, 0 );
		break;

	case TE_DEBUGTRAIL:
		msg_buffer.ReadVec3( pos );
		msg_buffer.ReadVec3( pos2 );
		CL_DebugTrail( pos, pos2 );
		break;

	case TE_PLAIN_EXPLOSION:
		msg_buffer.ReadVec3( pos );

		ex = CL_AllocExplosion( );
		ex->ent.origin = pos;
		ex->type = ex_poly;
		ex->ent.flags = RF_FULLBRIGHT;
		ex->start = ( timeTypeReal )cl.frame.servertime - 100;
		ex->light = 350;
		ex->lightcolor[ 0 ] = 1.0f;
		ex->lightcolor[ 1 ] = 0.5f;
		ex->lightcolor[ 2 ] = 0.5f;
		ex->ent.angles[ 1 ] = ( float )( rand( ) % 360 );
		ex->ent.model = cl_mod_explo4;
		if( frand( ) < 0.5f )
			ex->baseframe = 15;
		ex->frames = 15;
		/*if( type == TE_ROCKET_EXPLOSION_WATER )
			//Sound::S_StartSound( &pos, 0, 0, cl_sfx_watrexp, 1, ATTN_NORM, 0 );
		else
			//Sound::S_StartSound( &pos, 0, 0, cl_sfx_rockexp, 1, ATTN_NORM, 0 );*/
		break;

	case TE_FLASHLIGHT:
		msg_buffer.ReadVec3( pos );
		ent = msg_buffer.ReadShort( );
		CL_Flashlight( ent, pos );
		break;

	case TE_FORCEWALL:
		msg_buffer.ReadVec3( pos );
		msg_buffer.ReadVec3( pos2 );
		color = msg_buffer.ReadByte( );
		CL_ForceWall( pos, pos2, color );
		break;

	case TE_HEATBEAM:
		ent = CL_ParsePlayerBeam( msg_buffer, cl_mod_heatbeam );
		break;

	case TE_MONSTER_HEATBEAM:
		ent = CL_ParsePlayerBeam( msg_buffer, cl_mod_monster_heatbeam );
		break;

	case TE_HEATBEAM_SPARKS:
		//		cnt = msg_buffer.ReadByte( );
		cnt = 50;
		msg_buffer.ReadVec3( pos );
		dir = msg_buffer.ReadDir( 24 );
		//		r = msg_buffer.ReadByte( );
		//		magnitude = msg_buffer.ReadShort( );
		r = 8;
		magnitude = 60;
		color = r & 0xFF;
		CL_ParticleSteamEffect( pos, dir, color, cnt, magnitude );
		//Sound::S_StartSound( &pos, 0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0 );
		break;

	case TE_HEATBEAM_STEAM:
		//		cnt = msg_buffer.ReadByte( );
		cnt = 20;
		msg_buffer.ReadVec3( pos );
		dir = msg_buffer.ReadDir( 24 );
		//		r = msg_buffer.ReadByte( );
		//		magnitude = msg_buffer.ReadShort( );
		//		color = r & 0xFF;
		color = 0xE0;
		magnitude = 60;
		CL_ParticleSteamEffect( pos, dir, color, cnt, magnitude );
		//Sound::S_StartSound( &pos, 0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0 );
		break;

	case TE_STEAM:
		CL_ParseSteam( msg_buffer );
		break;

	case TE_BUBBLETRAIL2:
		//		cnt = msg_buffer.ReadByte( );
		cnt = 8;
		msg_buffer.ReadVec3( pos );
		msg_buffer.ReadVec3( pos2 );
		CL_BubbleTrail2( pos, pos2, cnt );
		//Sound::S_StartSound( &pos, 0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0 );
		break;

	case TE_MOREBLOOD:
		msg_buffer.ReadVec3( pos );
		dir = msg_buffer.ReadDir( 24 );
		CL_ParticleEffect( pos, dir, 0xE8, 250 );
		break;

	case TE_CHAINFIST_SMOKE:
		dir[ 0 ]= 0; dir[ 1 ]= 0; dir[ 2 ]= 1;
		msg_buffer.ReadVec3( pos );
		CL_ParticleSmokeEffect( pos, dir, 0, 20, 20 );
		break;

	case TE_ELECTRIC_SPARKS:
		msg_buffer.ReadVec3( pos );
		dir = msg_buffer.ReadDir( 24 );
		//		CL_ParticleEffect( pos, dir, 109, 40 );
		CL_ParticleEffect( pos, dir, 0x75, 40 );
		//FIXME : replace or remove this sound
		//Sound::S_StartSound( &pos, 0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0 );
		break;

	case TE_TRACKER_EXPLOSION:
		msg_buffer.ReadVec3( pos );
		CL_ColorFlash( pos, 0, 150, -1, -1, -1 );
		CL_ColorExplosionParticles( pos, 0, 1 );
		//		CL_Tracker_Explode( pos );
		//Sound::S_StartSound( &pos, 0, 0, cl_sfx_disrexp, 1, ATTN_NORM, 0 );
		break;

	case TE_TELEPORT_EFFECT:
	case TE_DBALL_GOAL:
		msg_buffer.ReadVec3( pos );
		CL_TeleportParticles( pos );
		break;

	case TE_WIDOWBEAMOUT:
		CL_ParseWidow( msg_buffer );
		break;

	case TE_NUKEBLAST:
		CL_ParseNuke( msg_buffer );
		break;

	case TE_WIDOWSPLASH:
		msg_buffer.ReadVec3( pos );
		CL_WidowSplash( pos );
		break;
		//PGM
		//==============

	default:
		Common::Com_Error( ERR_DROP, "CL_ParseTEnt: bad type" );
	}
}

/*

=================

CL_AddBeams

=================

*/
void Client::CL_AddBeams( ) {

	int i, j;
	beam_t * b;
	Vec3 dist, org;
	float d;
	SceneEntity ent;
	float yaw, pitch;
	float forward;
	float len, steps;
	float model_length;

	// update beams
	for( i = 0, b = cl_beams; i< 32; i++, b++ )
	{
		if( !b->model || b->endtime < cl.time )
			continue;

		// if coming from the player, update the start position
		if( b->entity == cl.playernum+1 ) // entity 0 is the world
		{
			b->start = cl.refdef.vieworg;
			b->start[ 2 ] -= 22; // adjust for view height
		}
		org = b->start + b->offset;

		// calculate pitch and yaw
		dist = b->end - org;

		if( dist[ 1 ] == 0 && dist[ 0 ] == 0 )
		{
			yaw = 0;
			if( dist[ 2 ] > 0 )
				pitch = 90;
			else
				pitch = 270;
		}
		else
		{
			// PMM - fixed to correct for pitch of 0
			if( dist[ 0 ] )
				yaw =( atan2( dist[ 1 ], dist[ 0 ] ) * 180 / M_PI );
			else if( dist[ 1 ] > 0 )
				yaw = 90;
			else
				yaw = 270;
			if( yaw < 0 )
				yaw += 360;

			forward = sqrt( dist[ 0 ]* dist[ 0 ] + dist[ 1 ]* dist[ 1 ] );
			pitch =( atan2( dist[ 2 ], forward ) * -180.0f / M_PI );
			if( pitch < 0 )
				pitch += 360.0f;
		}

		// add new entities for the beams
		d = dist.Normalize( );

		ent.Clear( );
		if( b->model == cl_mod_lightning )
		{
			model_length = 35.0f;
			d-= 20.0f; // correction so it doesn't end in middle of tesla
		}
		else
		{
			model_length = 30.0f;
		}
		steps = ceil( d/model_length );
		len =( d-model_length )/( steps-1 );

		// PMM - special case for lightning model .. if the real length is shorter than the model, // flip it around & draw it from the end to the start.  This prevents the model from going
		// through the tesla mine( instead it goes through the target )
		if( ( b->model == cl_mod_lightning ) &&( d <= model_length ) )
		{
			//			Common::Com_Printf( "special case\n" );
			ent.origin = b->end;
			// offset to push beam outside of tesla model( negative because dist is from end to start
			// for this beam )
			//			for( j = 0; j<3; j++ )
			//				ent.origin[ j ] -= dist[ j ]* 10.0f;
			ent.model = b->model;
			ent.flags = RF_FULLBRIGHT;
			ent.angles[ 0 ] = pitch;
			ent.angles[ 1 ] = yaw;
			ent.angles[ 2 ] = ( float )( rand( ) % 360 );
			View::V_AddEntity( ent );
			return;
		}
		while( d > 0 )
		{
			ent.origin = org;
			ent.model = b->model;
			if( b->model == cl_mod_lightning )
			{
				ent.flags = RF_FULLBRIGHT;
				ent.angles[ 0 ] = -pitch;
				ent.angles[ 1 ] = yaw + 180.0f;
				ent.angles[ 2 ] = ( float )( rand( ) % 360 );
			}
			else
			{
				ent.angles[ 0 ] = pitch;
				ent.angles[ 1 ] = yaw;
				ent.angles[ 2 ] = ( float )( rand( ) % 360 );
			}

			//			Common::Com_Printf( "B: %d -> %d\n", b->entity, b->dest_entity );
			View::V_AddEntity( ent );

			for( j = 0; j<3; j++ )
				org[ j ] += dist[ j ]* len;
			d -= model_length;
		}
	}
}

void Client::CL_AddPlayerBeams( ) {

	int i, j;
	beam_t * b;
	Vec3 dist, org;
	float d;
	SceneEntity ent;
	float yaw, pitch;
	float forward;
	float len, steps;
	int framenum;
	float model_length;

	float hand_multiplier;
	frame_t * oldframe;
	player_state_t * ps, * ops;

	//PMM
	if( hand.GetBool( ) ) {
		if( hand.GetInt( ) == 2 )
			hand_multiplier = 0;
		else if( hand.GetInt( ) == 1 )
			hand_multiplier = -1;
		else
			hand_multiplier = 1;
	}
	else hand_multiplier = 1;
	//PMM

	// update beams
	for( i = 0, b = cl_playerbeams; i< 32; i++, b++ )
	{
		Vec3 f, r, u;
		if( !b->model || b->endtime < cl.time )
			continue;

		if( cl_mod_heatbeam &&( b->model == cl_mod_heatbeam ) )
		{

			// if coming from the player, update the start position
			if( b->entity == cl.playernum+1 ) // entity 0 is the world
			{
				// set up gun position
				// code straight out of CL_AddViewWeapon
				ps = &cl.frame.playerstate;
				j =( cl.frame.serverframe - 1 ) & UPDATE_MASK;
				oldframe = &cl.frames[ j ];
				if( oldframe->serverframe != cl.frame.serverframe-1 || !oldframe->valid )
					oldframe = &cl.frame; // previous frame was dropped or involid
				ops = &oldframe->playerstate;
				for( j = 0; j<3; j++ )
				{
					b->start[ j ] = cl.refdef.vieworg[ j ] + ops->gunoffset[ j ]
					+ cl.lerpfrac * ( ps->gunoffset[ j ] - ops->gunoffset[ j ] );
				}
				org = b->start.Magnitude( hand_multiplier * b->offset[ 0 ], cl.v_right );
				org = org.Magnitude( b->offset[ 1 ], cl.v_forward );
				org = org.Magnitude( b->offset[ 2 ], cl.v_up );

				if( hand.GetInt( ) == 2 ) org = org.Magnitude( -1.0f, cl.v_up );

				// FIXME - take these out when final
				r = cl.v_right;
				f = cl.v_forward;
				u = cl.v_up;

			}
			else
				org = b->start;
		}
		else
		{
			// if coming from the player, update the start position
			if( b->entity == cl.playernum+1 ) // entity 0 is the world
			{
				b->start = cl.refdef.vieworg;
				b->start[ 2 ] -= 22; // adjust for view height
			}
			org = b->start + b->offset;
		}

		// calculate pitch and yaw
		dist = b->end - org;

		//PMM
		if( cl_mod_heatbeam &&( b->model == cl_mod_heatbeam ) &&( b->entity == cl.playernum+1 ) )
		{
			vec_t len;

			len = dist.Length( );
			dist = f * len;
			dist = dist.Magnitude( hand_multiplier * b->offset[ 0 ], r );
			dist = dist.Magnitude( b->offset[ 1 ], f );
			dist = dist.Magnitude( b->offset[ 2 ], u );
			if( hand.GetInt( ) == 2 ) org = org.Magnitude( -1.0f, cl.v_up );
		}
		//PMM

		if( dist[ 1 ] == 0 && dist[ 0 ] == 0 )
		{
			yaw = 0;
			if( dist[ 2 ] > 0 )
				pitch = 90;
			else
				pitch = 270;
		}
		else
		{
			// PMM - fixed to correct for pitch of 0
			if( dist[ 0 ] )
				yaw =( atan2( dist[ 1 ], dist[ 0 ] ) * 180 / M_PI );
			else if( dist[ 1 ] > 0 )
				yaw = 90;
			else
				yaw = 270;
			if( yaw < 0 )
				yaw += 360;

			forward = sqrt( dist[ 0 ]* dist[ 0 ] + dist[ 1 ]* dist[ 1 ] );
			pitch =( atan2( dist[ 2 ], forward ) * -180.0f / M_PI );
			if( pitch < 0 )
				pitch += 360.0f;
		}

		if( cl_mod_heatbeam &&( b->model == cl_mod_heatbeam ) )
		{
			if( b->entity != cl.playernum+1 )
			{
				framenum = 2;
				//				Common::Com_Printf( "Third person\n" );
				ent.angles[ 0 ] = -pitch;
				ent.angles[ 1 ] = yaw + 180.0f;
				ent.angles[ 2 ] = 0;
				//				Common::Com_Printf( "%f %f - %f %f %f\n", -pitch, yaw+180.0f, b->offset[ 0 ], b->offset[ 1 ], b->offset[ 2 ] );
				ent.angles.AngleVectors( &f, &r, &u );

				// if it's a non-origin offset, it's a player, so use the hardcoded player offset
				if( b->offset != vec3_origin ) {

					org = org.Magnitude( -( b->offset[ 0 ] ) + 1.0f, r );
					org = org.Magnitude( -( b->offset[ 1 ] ), f );
					org = org.Magnitude( -( b->offset[ 2 ] ) - 10.0f, u );
				} else {

					// if it's a monster, do the particle effect
					CL_MonsterPlasma_Shell( b->start );
				}
			}
			else
			{
				framenum = 1;
			}
		}

		// if it's the heatbeam, draw the particle effect
		if( ( cl_mod_heatbeam &&( b->model == cl_mod_heatbeam ) &&( b->entity == cl.playernum+1 ) ) )
		{
			CL_Heatbeam( org, dist );
		}

		// add new entities for the beams
		d = dist.Normalize( );

		ent.Clear( );
		if( b->model == cl_mod_heatbeam )
		{
			model_length = 32.0f;
		}
		else if( b->model == cl_mod_lightning )
		{
			model_length = 35.0f;
			d-= 20.0f; // correction so it doesn't end in middle of tesla
		}
		else
		{
			model_length = 30.0f;
		}
		steps = ceil( d/model_length );
		len =( d-model_length )/( steps-1 );

		// PMM - special case for lightning model .. if the real length is shorter than the model, // flip it around & draw it from the end to the start.  This prevents the model from going
		// through the tesla mine( instead it goes through the target )
		if( ( b->model == cl_mod_lightning ) &&( d <= model_length ) )
		{
			//			Common::Com_Printf( "special case\n" );
			ent.origin = b->end;
			// offset to push beam outside of tesla model( negative because dist is from end to start
			// for this beam )
			//			for( j = 0; j<3; j++ )
			//				ent.origin[ j ] -= dist[ j ]* 10.0f;
			ent.model = b->model;
			ent.flags = RF_FULLBRIGHT;
			ent.angles[ 0 ] = pitch;
			ent.angles[ 1 ] = yaw;
			ent.angles[ 2 ] = ( float )( rand( ) % 360 );
			View::V_AddEntity( ent );
			return;
		}
		while( d > 0 )
		{
			ent.origin = org;
			ent.model = b->model;
			if( cl_mod_heatbeam &&( b->model == cl_mod_heatbeam ) )
			{
				//				ent.flags = RF_FULLBRIGHT|RF_TRANSLUCENT;
				//				ent.alpha = 0.3f;
				ent.flags = RF_FULLBRIGHT;
				ent.angles[ 0 ] = -pitch;
				ent.angles[ 1 ] = yaw + 180.0f;
				ent.angles[ 2 ] = ( float )( ( ( int )cl.time ) % 360 );
				//				ent.angles[ 2 ] = rand( )%360;
				ent.frame = framenum;
			}
			else if( b->model == cl_mod_lightning )
			{
				ent.flags = RF_FULLBRIGHT;
				ent.angles[ 0 ] = -pitch;
				ent.angles[ 1 ] = yaw + 180.0f;
				ent.angles[ 2 ] = ( float )( rand( ) % 360 );
			}
			else
			{
				ent.angles[ 0 ] = pitch;
				ent.angles[ 1 ] = yaw;
				ent.angles[ 2 ] = ( float )( rand( ) % 360 );
			}

			//			Common::Com_Printf( "B: %d -> %d\n", b->entity, b->dest_entity );
			View::V_AddEntity( ent );

			for( j = 0; j<3; j++ )
				org[ j ] += dist[ j ]* len;
			d -= model_length;
		}
	}
}

void Client::CL_AddExplosions( ) {

	SceneEntity ent;
	int i;
	explosion_t * ex;
	timeType frac;
	int f;

	ent.Clear( );

	for( i = 0, ex = cl_explosions; i< 32; i++, ex++ ) {
		if( ex->type == ex_free )
			continue;
		frac = ( cl.time - ( timeType )ex->start ) / 100;
		f = ( int )floorf( ( float )frac );
		ent = ex->ent;
		switch( ex->type ) {
		case ex_mflash:
			if( f >= ex->frames-1 )
				ex->type = ex_free;
			break;
		case ex_misc:
			if( f >= ex->frames-1 )
			{
				ex->type = ex_free;
				break;
			}
			ent.alpha = 1.0f - frac/( ex->frames-1 );
			break;
		case ex_flash:
			if( f >= 1 )
			{
				ex->type = ex_free;
				break;
			}
			ent.alpha = 1.0f;
			break;
		case ex_poly:
			if( f >= ex->frames-1 )
			{
				ex->type = ex_free;
				break;
			}

			ent.alpha =( 16.0f -( float )f )/16.0f;

			if( f < 10 )
			{
				ent.skinnum =( f>>1 );
				if( ent.skinnum < 0 )
					ent.skinnum = 0;
			}
			else
			{
				ent.flags |= RF_TRANSLUCENT;
				if( f < 13 )
					ent.skinnum = 5;
				else
					ent.skinnum = 6;
			}
			break;
		case ex_poly2:
			if( f >= ex->frames-1 )
			{
				ex->type = ex_free;
				break;
			}

			ent.alpha =( 5.0f -( float )f )/5.0f;
			ent.skinnum = 0;
			ent.flags |= RF_TRANSLUCENT;
			break;
		}

		if( ex->type == ex_free )
			continue;
		if( ex->light )
		{
			View::V_AddLight( ent.origin, ex->light* ent.alpha, ex->lightcolor[ 0 ], ex->lightcolor[ 1 ], ex->lightcolor[ 2 ] );
		}

		ent.oldorigin = ent.origin;

		if( f < 0 )
			f = 0;
		ent.frame = ex->baseframe + f + 1;
		ent.oldframe = ex->baseframe + f;
		ent.backlerp = 1.0f - cl.lerpfrac;

		View::V_AddEntity( ent );
	}
}

void Client::CL_AddLasers( ) {

	laser_t * l;
	int i;

	for( i = 0, l = cl_lasers; i< 32; i++, l++ )
	{
		if( l->endtime >= cl.time )
			View::V_AddEntity( l->ent );
	}
}

void Client::CL_ProcessSustain( ) {

	cl_sustain_t * s;
	int i;

	for( i = 0, s = cl_sustains; i< MAX_SUSTAINS; i++, s++ )
	{
		if( s->id )
			if( ( s->endtime >= cl.time ) &&( cl.time >= s->nextthink ) )
			{
				//				Common::Com_Printf( "think %d %d %d\n", cl.time, s->nextthink, s->thinkinterval );
				s->think( s );
			}
			else if( s->endtime < cl.time )
				s->id = 0;
	}
}

void Client::CL_AddTEnts( ) {

	CL_AddBeams( );
	// PMM - draw plasma beams
	CL_AddPlayerBeams( );
	CL_AddExplosions( );
	CL_AddLasers( );
	// PMM - set up sustain
	CL_ProcessSustain( );
}
