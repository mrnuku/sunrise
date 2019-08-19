#include "precompiled.h"
#pragma hdrstop

/* QUAKED target_temp_entity( 1 0 0 )( -8 -8 -8 )( 8 8 8 )

Fire an origin based temp entity event to the clients.

"style"		type byte

*/
void Game::Use_Target_Tent( Entity * ent, Entity * other, Entity * activator ) {

	Server::PF_WriteByte( svc_temp_entity );
	Server::PF_WriteByte( ent->style );
	Server::PF_WritePos( ent->s.origin );
	Server::SV_Multicast( ent->s.origin, MULTICAST_PVS );
}

void Game::SP_target_temp_entity( Entity * ent ) {

	ent->use = Use_Target_Tent;
}


//==========================================================

//==========================================================

/* QUAKED target_speaker( 1 0 0 )( -8 -8 -8 )( 8 8 8 ) looped-on looped-off reliable

"noise"		wav file to play

"attenuation"

-1 = none, send to whole level

1 = normal fighting sounds

2 = le sound level

3 = ambient sound level

"volume"	0.0f to 1.0f



Normal sounds play each time the target is used.  The reliable flag can be set for crucial voiceovers.



Looped sounds are always atten 3 / vol 1, and the use function toggles it on/off.

Multiple entical looping sounds will just increase volume without any speed cost.

*/
void Game::Use_Target_Speaker( Entity * ent, Entity * other, Entity * activator ) {

	int chan;

	if( ent->spawnflags & 3 )
	{ // looping sound toggles
		if( ent->s.sound )
			ent->s.sound = 0; // turn it off
		else
			ent->s.sound = ent->noise_index; // start it
	}
	else
	{ // normal sound
		if( ent->spawnflags & 4 )
			chan = CHAN_VOICE|CHAN_RELIABLE;
		else
			chan = CHAN_VOICE;
		// use a positioned_sound, because this entity won't normally be
		// sent to any clients because it is invisible
		Server::SV_StartSound( &ent->s.origin, ent, chan, ent->noise_index, ent->volume, ent->attenuation, 0 );
	}
}

void Game::SP_target_speaker( Entity * ent ) {

	Str buffer;

	if( !st.noise )
	{
		Common::Com_DPrintf( "target_speaker with no noise set at %s\n", vtos( ent->s.origin ).c_str( ) );
		return;
	}
	if( st.noise == ".wav" ) sprintf( buffer, "%s.wav", st.noise.c_str( ) );
	else buffer = st.noise;
	ent->noise_index = Server::SV_SoundIndex( buffer );

	if( !ent->volume )
		ent->volume = 1.0f;

	if( !ent->attenuation )
		ent->attenuation = 1.0f;
	else if( ent->attenuation == -1 ) // use -1 so 0 defaults to 1
		ent->attenuation = 0;

	// check for prestarted looping sound
	if( ent->spawnflags & 1 )
		ent->s.sound = ent->noise_index;

	ent->use = Use_Target_Speaker;

	// must link the entity so we get areas and clusters so
	// the server can determine who to send updates to
	Server::SV_LinkEdict( ent );
}


//==========================================================

void Game::Use_Target_Help( Entity * ent, Entity * other, Entity * activator ) {

	if( ent->spawnflags & 1 ) helpmessage1 = ent->message;
	else helpmessage2 = ent->message;

	helpchanged++;
}

/* QUAKED target_help( 1 0 1 )( -16 -16 -24 )( 16 16 24 ) help1

When fired, the "message" key becomes the current personal computer string, and the message light will be set on all clients status bars.

*/
void Game::SP_target_help( Entity * ent ) {

	if( deathmatch.GetBool( ) )
	{ // auto-remove for deathmatch
		G_FreeEdict( ent );
		return;
	}

	if( !ent->message )
	{
		Common::Com_DPrintf( "%s with no message at %s\n", ent->classname.c_str( ), vtos( ent->s.origin ).c_str( ) );
		G_FreeEdict( ent );
		return;
	}
	ent->use = Use_Target_Help;
}

//==========================================================

/* QUAKED target_secret( 1 0 1 )( -8 -8 -8 )( 8 8 8 )

Counts a secret found.

These are single use targets.

*/
void Game::use_target_secret( Entity * ent, Entity * other, Entity * activator ) {

	Server::PF_StartSound( ent, CHAN_VOICE, ent->noise_index, 1, ATTN_NORM, 0 );

	level.found_secrets++;

	G_UseTargets( ent, activator );
	G_FreeEdict( ent );
}

void Game::SP_target_secret( Entity * ent ) {

	if( deathmatch.GetBool( ) )
	{ // auto-remove for deathmatch
		G_FreeEdict( ent );
		return;
	}

	ent->use = use_target_secret;
	if( !st.noise )
		st.noise = "misc/secret.wav";
	ent->noise_index = Server::SV_SoundIndex( st.noise );
	ent->svflags = SVF_NOCLIENT;
	level.total_secrets++;
}

//==========================================================

/* QUAKED target_goal( 1 0 1 )( -8 -8 -8 )( 8 8 8 )

Counts a goal completed.

These are single use targets.

*/
void Game::use_target_goal( Entity * ent, Entity * other, Entity * activator ) {

	Server::PF_StartSound( ent, CHAN_VOICE, ent->noise_index, 1, ATTN_NORM, 0 );

	level.found_goals++;

	G_UseTargets( ent, activator );
	G_FreeEdict( ent );
}

void Game::SP_target_goal( Entity * ent ) {

	if( deathmatch.GetBool( ) ) { // auto-remove for deathmatch

		G_FreeEdict( ent );
		return;
	}

	ent->use = use_target_goal;
	if( !st.noise ) st.noise = "misc/secret.wav";
	ent->noise_index = Server::SV_SoundIndex( st.noise );
	ent->svflags = SVF_NOCLIENT;
	level.total_goals++;
}

//==========================================================


/* QUAKED target_explosion( 1 0 0 )( -8 -8 -8 )( 8 8 8 )

Spawns an explosion temporary entity when used.



"delay"		wait this long before going off

"dmg"		how much radius damage should be done, defaults to 0

*/
void Game::target_explosion_explode( Entity * self ) {

	float save;

	Server::PF_WriteByte( svc_temp_entity );
	Server::PF_WriteByte( TE_EXPLOSION1 );
	Server::PF_WritePos( self->s.origin );
	Server::SV_Multicast( self->s.origin, MULTICAST_PHS );

	T_RadiusDamage( self, self->activator, ( float )self->dmg, NULL, ( float )( self->dmg+40 ), MOD_EXPLOSIVE );

	save = self->delay;
	self->delay = 0;
	G_UseTargets( self, self->activator );
	self->delay = save;
}

void Game::use_target_explosion( Entity * self, Entity * other, Entity * activator ) {

	self->activator = activator;

	if( !self->delay )
	{
		target_explosion_explode( self );
		return;
	}

	self->think = target_explosion_explode;
	self->nextthink = level.time + self->delay;
}

void Game::SP_target_explosion( Entity * ent ) {

	ent->use = use_target_explosion;
	ent->svflags = SVF_NOCLIENT;
}


//==========================================================

/* QUAKED target_changelevel( 1 0 0 )( -8 -8 -8 )( 8 8 8 )

Changes level to "map" when fired

*/
void Game::use_target_changelevel( Entity * self, Entity * other, Entity * activator ) {

	if( level.intermissiontime )
		return; // already activated

	if( !deathmatch.GetBool( ) && !coop.GetBool( ) )
	{
		if( g_edicts[ 1 ].health <= 0 )
			return;
	}

	// if noexit, do a ton of damage to other
	if( deathmatch.GetBool( ) && !( dmflags.GetInt( ) & DF_ALLOW_EXIT ) && other != world )
	{
		T_Damage( other, self, self, vec3_origin, other->s.origin, vec3_origin, 10 * other->max_health, 1000, 0, MOD_EXIT );
		return;
	}

	// if multiplayer, let everyone know who hit the exit
	if( deathmatch.GetBool( ) )
	{
		if( activator && activator->client )
			Server::SV_BroadcastPrintf( PRINT_HIGH, "%s exited the level.\n", activator->client->pers.netname.c_str( ) );
	}

	// if going to a new unit, clear cross triggers
	if( strstr( self->map, "*" ) ) serverflags &= ~( SFL_CROSS_TRIGGER_MASK );

	BeginIntermission( self );
}

void Game::SP_target_changelevel( Entity * ent ) {

	if( !ent->map )
	{
		Common::Com_DPrintf( "target_changelevel with no map at %s\n", vtos( ent->s.origin ).c_str( ) );
		G_FreeEdict( ent );
		return;
	}

	ent->use = use_target_changelevel;
	ent->svflags = SVF_NOCLIENT;
}


//==========================================================

/* QUAKED target_splash( 1 0 0 )( -8 -8 -8 )( 8 8 8 )

Creates a particle splash effect when used.



Set "sounds" to one of the following:

1 ) sparks

2 ) blue water

3 ) brown water

4 ) slime

5 ) lava

6 ) blood



"count"	how many pixels in the splash

"dmg"	if set, does a radius damage at this location when it splashes

useful for lava/sparks

*/
void Game::use_target_splash( Entity * self, Entity * other, Entity * activator ) {

	Server::PF_WriteByte( svc_temp_entity );
	Server::PF_WriteByte( TE_SPLASH );
	Server::PF_WriteByte( self->count );
	Server::PF_WritePos( self->s.origin );
	Server::PF_WriteDir( self->movedir );
	Server::PF_WriteByte( self->sounds );
	Server::SV_Multicast( self->s.origin, MULTICAST_PVS );

	if( self->dmg )
		T_RadiusDamage( self, activator, ( float )self->dmg, NULL, ( float )( self->dmg+40 ), MOD_SPLASH );
}

void Game::SP_target_splash( Entity * self ) {

	self->use = use_target_splash;
	G_SetMovedir( self->s.angles, &self->movedir );

	if( !self->count )
		self->count = 32;

	self->svflags = SVF_NOCLIENT;
}


//==========================================================

/* QUAKED target_spawner( 1 0 0 )( -8 -8 -8 )( 8 8 8 )

Set target to the type of entity you want spawned.

Useful for spawning monsters and gibs in the factory levels.



For monsters:

Set direction to the facing you want it to have.



For gibs:

Set direction if you want it moving and

speed how fast it should be moving otherwise it

will just be dropped

*/
void Game::use_target_spawner( Entity * self, Entity * other, Entity * activator ) {

	Entity * ent;

	ent = G_Spawn( );
	ent->classname = self->target;
	ent->s.origin = self->s.origin;
	ent->s.angles = self->s.angles;
	ED_CallSpawn( ent );
	Server::SV_UnlinkEdict( ent );
	KillBox( ent );
	Server::SV_LinkEdict( ent );
	if( self->speed )
		ent->velocity = self->movedir;
}

void Game::SP_target_spawner( Entity * self ) {

	self->use = use_target_spawner;
	self->svflags = SVF_NOCLIENT;
	if( self->speed )
	{
		G_SetMovedir( self->s.angles, &self->movedir );
		self->movedir *= self->speed;
	}
}

//==========================================================

/* QUAKED target_blaster( 1 0 0 )( -8 -8 -8 )( 8 8 8 ) NOTRAIL NOEFFECTS

Fires a blaster bolt in the set direction when triggered.



dmg		default is 15

speed	default is 1000

*/
void Game::use_target_blaster( Entity * self, Entity * other, Entity * activator ) {

	int effect;

	if( self->spawnflags & 2 )
		effect = 0;
	else if( self->spawnflags & 1 )
		effect = EF_HYPERBLASTER;
	else
		effect = EF_BLASTER;

	fire_blaster( self, self->s.origin, self->movedir, self->dmg, ( int )self->speed, EF_BLASTER, MOD_TARGET_BLASTER ? true : false );
	Server::PF_StartSound( self, CHAN_VOICE, self->noise_index, 1, ATTN_NORM, 0 );
}

void Game::SP_target_blaster( Entity * self ) {

	self->use = use_target_blaster;
	G_SetMovedir( self->s.angles, &self->movedir );
	self->noise_index = Server::SV_SoundIndex( "weapons/laser2.wav" );

	if( !self->dmg )
		self->dmg = 15;
	if( !self->speed )
		self->speed = 1000;

	self->svflags = SVF_NOCLIENT;
}


//==========================================================

/* QUAKED target_crosslevel_trigger( .5 .5 .5 )( -8 -8 -8 )( 8 8 8 ) trigger1 trigger2 trigger3 trigger4 trigger5 trigger6 trigger7 trigger8

Once this trigger is touched/used, any trigger_crosslevel_target with the same trigger number is automatically used when a level is started within the same unit.  It is OK to check multiple triggers.  Message, delay, target, and killtarget also work.

*/
void Game::trigger_crosslevel_trigger_use( Entity * self, Entity * other, Entity * activator ) {

	serverflags |= self->spawnflags;
	G_FreeEdict( self );
}

void Game::SP_target_crosslevel_trigger( Entity * self ) {

	self->svflags = SVF_NOCLIENT;
	self->use = trigger_crosslevel_trigger_use;
}

/* QUAKED target_crosslevel_target( .5 .5 .5 )( -8 -8 -8 )( 8 8 8 ) trigger1 trigger2 trigger3 trigger4 trigger5 trigger6 trigger7 trigger8

Triggered by a trigger_crosslevel elsewhere within a unit.  If multiple triggers are checked, all must be true.  Delay, target and

killtarget also work.



"delay"		delay before using targets if the trigger has been activated( default 1 )

*/
void Game::target_crosslevel_target_think( Entity * self ) {

	if( self->spawnflags ==( serverflags & SFL_CROSS_TRIGGER_MASK & self->spawnflags ) ) {
		G_UseTargets( self, self );
		G_FreeEdict( self );
	}
}

void Game::SP_target_crosslevel_target( Entity * self ) {

	if( ! self->delay )
		self->delay = 1;
	self->svflags = SVF_NOCLIENT;

	self->think = target_crosslevel_target_think;
	self->nextthink = level.time + self->delay;
}

//==========================================================

/* QUAKED target_laser( 0 .5 .8 )( -8 -8 -8 )( 8 8 8 ) START_ON RED GREEN BLUE YELLOW ORANGE FAT

When triggered, fires a laser.  You can either set a target

or a direction.

*/
void Game::target_laser_think( Entity * self ) {

	Entity * ignore;
	Vec3 start;
	Vec3 end;
	CTrace tr;
	Vec3 point;
	Vec3 last_movedir;
	int count;

	if( self->spawnflags & 0x80000000 )
		count = 8;
	else
		count = 4;

	if( self->enemy ) {

		last_movedir = self->movedir;
		point = self->enemy->absmin.Magnitude( 0.5f, self->enemy->size );
		self->movedir = point - self->s.origin;
		self->movedir.Normalize( );
		if( self->movedir != last_movedir ) self->spawnflags |= 0x80000000;
	}

	ignore = self;
	start = self->s.origin;
	end = start.Magnitude( 2048.0f, self->movedir );
	while( 1 )
	{
		tr = Server::SV_Trace( start, vec3_origin, vec3_origin, end, ignore, CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_CORPSE );

		if( !tr.ent )
			break;

		// hurt it if we can
		if( ( tr.ent->takedamage ) && !( tr.ent->flags & FL_IMMUNE_LASER ) ) 	T_Damage( tr.ent, self, self->activator, self->movedir, tr.endpos, vec3_origin, self->dmg, 1, DAMAGE_ENERGY, MOD_TARGET_LASER );

		// if we hit something that's not a monster or player or is immune to lasers, we're done
		if( !( tr.ent->svflags & SVF_MONSTER ) &&( !tr.ent->client ) )
		{
			if( self->spawnflags & 0x80000000 )
			{
				self->spawnflags &= ~0x80000000;
				Server::PF_WriteByte( svc_temp_entity );
				Server::PF_WriteByte( TE_LASER_SPARKS );
				Server::PF_WriteByte( count );
				Server::PF_WritePos( tr.endpos );
				Server::PF_WriteDir( tr.plane.Normal( ) );
				Server::PF_WriteByte( self->s.skinnum );
				Server::SV_Multicast( tr.endpos, MULTICAST_PVS );
			}
			break;
		}

		ignore = tr.ent;
		start = tr.endpos;
	}

	self->s.old_origin = tr.endpos;

	self->nextthink = level.time + FRAMETIME;
}

void Game::target_laser_on( Entity * self ) {

	if( !self->activator )
		self->activator = self;
	self->spawnflags |= 0x80000001;
	self->svflags &= ~SVF_NOCLIENT;
	target_laser_think( self );
}

void Game::target_laser_off( Entity * self ) {

	self->spawnflags &= ~1;
	self->svflags |= SVF_NOCLIENT;
	self->nextthink = 0;
}

void Game::target_laser_use( Entity * self, Entity * other, Entity * activator ) {

	self->activator = activator;
	if( self->spawnflags & 1 )
		target_laser_off( self );
	else
		target_laser_on( self );
}

void Game::target_laser_start( Entity * self ) {

	Entity * ent;

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	self->s.modelindex = 1; // must be non-zero

	// set the beam diameter
	if( self->spawnflags & 64 )
		self->s.frame = 16;
	else
		self->s.frame = 4;

	// set the color
	if( self->spawnflags & 2 )
		self->s.skinnum = 0xF2F2F0F0;
	else if( self->spawnflags & 4 )
		self->s.skinnum = 0xD0D1D2D3;
	else if( self->spawnflags & 8 )
		self->s.skinnum = 0xF3F3F1F1;
	else if( self->spawnflags & 16 )
		self->s.skinnum = 0xDCDDDEDF;
	else if( self->spawnflags & 32 )
		self->s.skinnum = 0xE0E1E2E3;

	if( !self->enemy )
	{
		if( self->target )
		{
			ent = G_Find( NULL, FOFS( targetname ), self->target );
			if( !ent )
				Common::Com_DPrintf( "%s at %s: %s is a bad target\n", self->classname.c_str( ), vtos( self->s.origin ).c_str( ), self->target.c_str( ) );
			self->enemy = ent;
		}
		else
		{
			G_SetMovedir( self->s.angles, &self->movedir );
		}
	}
	self->use = target_laser_use;
	self->think = target_laser_think;

	if( !self->dmg )
		self->dmg = 1;

	self->mins = Vec3( -8, -8, -8 );
	self->maxs = Vec3( 8, 8, 8 );
	Server::SV_LinkEdict( self );

	if( self->spawnflags & 1 )
		target_laser_on( self );
	else
		target_laser_off( self );
}

void Game::SP_target_laser( Entity * self ) {

	// let everything else get spawned before we start firing
	self->think = target_laser_start;
	self->nextthink = level.time + 1;
}

//==========================================================

/* QUAKED target_lightramp( 0 .5 .8 )( -8 -8 -8 )( 8 8 8 ) TOGGLE

speed		How many seconds the ramping will take

message		two letters; starting lightlevel and ending lightlevel

*/
void Game::target_lightramp_think( Entity * self ) {

	char style[ 2 ];

	style[ 0 ] = 'a' + ( char )( self->movedir[ 0 ] +( level.time - self->timestamp ) / FRAMETIME * self->movedir[ 2 ] );
	style[ 1 ] = 0;
	Server::PF_Configstring( CS_LIGHTS+self->enemy->style, style );

	if( ( level.time - self->timestamp ) < self->speed )
	{
		self->nextthink = level.time + FRAMETIME;
	}
	else if( self->spawnflags & 1 )
	{
		char temp;

		temp = ( char )( self->movedir[ 0 ] );
		self->movedir[ 0 ] = self->movedir[ 1 ];
		self->movedir[ 1 ] = temp;
		self->movedir[ 2 ] *= -1;
	}
}

void Game::target_lightramp_use( Entity * self, Entity * other, Entity * activator ) {

	if( !self->enemy )
	{
		Entity * e;

		// check all the targets
		e = NULL;
		while( 1 )
		{
			e = G_Find( e, FOFS( targetname ), self->target );
			if( !e )
				break;
			if( strcmp( e->classname, "light" ) != 0 )
			{
				Common::Com_DPrintf( "%s at %s ", self->classname.c_str( ), vtos( self->s.origin ).c_str( ) );
				Common::Com_DPrintf( "target %s( %s at %s ) is not a light\n", self->target.c_str( ), e->classname.c_str( ), vtos( e->s.origin ).c_str( ) );
			}
			else
			{
				self->enemy = e;
			}
		}

		if( !self->enemy )
		{
			Common::Com_DPrintf( "%s target %s not found at %s\n", self->classname.c_str( ), self->target.c_str( ), vtos( self->s.origin ).c_str( ) );
			G_FreeEdict( self );
			return;
		}
	}

	self->timestamp = level.time;
	target_lightramp_think( self );
}

void Game::SP_target_lightramp( Entity * self ) {

	if( !self->message || strlen( self->message ) != 2 || self->message[ 0 ] < 'a' || self->message[ 0 ] > 'z' || self->message[ 1 ] < 'a' || self->message[ 1 ] > 'z' || self->message[ 0 ] == self->message[ 1 ] )
	{
		Common::Com_DPrintf( "target_lightramp has bad ramp( %s ) at %s\n", self->message.c_str( ), vtos( self->s.origin ).c_str( ) );
		G_FreeEdict( self );
		return;
	}

	if( deathmatch.GetBool( ) )
	{
		G_FreeEdict( self );
		return;
	}

	if( !self->target )
	{
		Common::Com_DPrintf( "%s with no target at %s\n", self->classname.c_str( ), vtos( self->s.origin ).c_str( ) );
		G_FreeEdict( self );
		return;
	}

	self->svflags |= SVF_NOCLIENT;
	self->use = target_lightramp_use;
	self->think = target_lightramp_think;

	self->movedir[ 0 ] = ( float )( self->message[ 0 ] - 'a' );
	self->movedir[ 1 ] = ( float )( self->message[ 1 ] - 'a' );
	self->movedir[ 2 ] =( self->movedir[ 1 ] - self->movedir[ 0 ] ) /( self->speed / FRAMETIME );
}

//==========================================================

/* QUAKED target_earthquake( 1 0 0 )( -8 -8 -8 )( 8 8 8 )

When triggered, this initiates a level-wide earthquake.

All players and monsters are affected.

"speed"		severity of the quake( default:200 )

"count"		duration of the quake( default:5 )

*/
void Game::target_earthquake_think( Entity * self ) {

	int i;
	Entity * e;

	if( self->last_move_time < level.time )
	{
		Server::SV_StartSound( &self->s.origin, self, CHAN_AUTO, self->noise_index, 1.0f, ATTN_NONE, 0 );
		self->last_move_time = level.time + 0.5f;
	}

	for( i = 1, e = g_edicts+i; i < num_edicts; i++, e++ )
	{
		if( !e->inuse )
			continue;
		if( !e->client )
			continue;
		if( !e->groundentity )
			continue;

		e->groundentity = NULL;
		e->velocity[ 0 ] += crandom( )* 150;
		e->velocity[ 1 ] += crandom( )* 150;
		e->velocity[ 2 ] = self->speed * ( 100.0f / e->mass );
	}

	if( level.time < self->timestamp )
		self->nextthink = level.time + FRAMETIME;
}

void Game::target_earthquake_use( Entity * self, Entity * other, Entity * activator ) {

	self->timestamp = level.time + self->count;
	self->nextthink = level.time + FRAMETIME;
	self->activator = activator;
	self->last_move_time = 0.0;
}

void Game::SP_target_earthquake( Entity * self ) {

	if( !self->targetname )
		Common::Com_DPrintf( "untargeted %s at %s\n", self->classname.c_str( ), vtos( self->s.origin ).c_str( ) );

	if( !self->count )
		self->count = 5;

	if( !self->speed )
		self->speed = 200;

	self->svflags |= SVF_NOCLIENT;
	self->think = target_earthquake_think;
	self->use = target_earthquake_use;

	self->noise_index = Server::SV_SoundIndex( "world/quake.wav" );
}
