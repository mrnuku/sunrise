#include "precompiled.h"
#pragma hdrstop

// =======================================================================
// Internal sound data & structures
// =======================================================================

// only begin attenuating sound volumes when outside the FULLVOLUME range

int s_registration_sequence;

channel_t channels[ MAX_CHANNELS ];

bool snd_initialized = false;
int sound_started = 0;

dma_t dma;

Vec3 listener_origin;
Vec3 listener_forward;
Vec3 listener_right;
Vec3 listener_up;

bool s_registering;

int soundtime; // sample PAIRS
int paintedtime; // sample PAIRS

// during registration it is possible to have more sounds
// than could actually be referenced during gameplay, // because we don't want to free anything until we are
// sure we won't need it.

int num_sfx;

playsound_t s_playsounds[ 128 ];
playsound_t s_freeplays;
playsound_t s_pendingplays;

int s_beginofs;

int s_rawend;
portable_samplepair_t s_rawsamples[ MAX_RAW_SAMPLES ];


// ====================================================================
// User-setable variables
// ====================================================================


void Sound::S_SoundInfo_f( ) {

	if( !sound_started )
	{
		Common::Com_Printf( "sound system not started\n" );
		return;
	}

	Common::Com_Printf( "%5d stereo\n", dma.channels - 1 );
	Common::Com_Printf( "%5d samples\n", dma.samples );
	Common::Com_Printf( "%5d samplepos\n", dma.samplepos );
	Common::Com_Printf( "%5d samplebits\n", dma.samplebits );
	Common::Com_Printf( "%5d submission_chunk\n", dma.submission_chunk );
	Common::Com_Printf( "%5d speed\n", dma.speed );
	Common::Com_Printf( "0x%x dma buffer\n", dma.buffer );
}

/*

================

S_Init

================

*/
void Sound::S_Init( ) {

#if 0
	Common::Com_Printf( "\n------- sound initialization -------\n" );

	if( !s_initsound.GetBool( ) ) Common::Com_Printf( "not initializing.\n" );
	else {

		Command::Cmd_AddCommand( Str( "play" ), S_Play );
		Command::Cmd_AddCommand( Str( "stopsound" ), S_StopAllSounds );
		Command::Cmd_AddCommand( Str( "soundlist" ), S_SoundList );
		Command::Cmd_AddCommand( Str( "soundinfo" ), S_SoundInfo_f );

		if( !SNDDMA_Init( ) ) return;

		S_InitScaletable( );

		sound_started = 1;
		num_sfx = 0;

		soundtime = 0;
		paintedtime = 0;

		Common::Com_Printf( "sound sampling rate: %i\n", dma.speed );

		S_StopAllSounds( );
	}

	Common::Com_Printf( "------------------------------------\n" );
#endif
}


// =======================================================================
// Shutdown sound engine
// =======================================================================

void Sound::S_Shutdown( ) {

#if 0
	int i;
	sfx_t * sfx;

	if( !sound_started )
		return;

	SNDDMA_Shutdown( );

	sound_started = 0;

	Command::Cmd_RemoveCommand( Str( "play" ) );
	Command::Cmd_RemoveCommand( Str( "stopsound" ) );
	Command::Cmd_RemoveCommand( Str( "soundlist" ) );
	Command::Cmd_RemoveCommand( Str( "soundinfo" ) );

	// free all sounds
	for( i = 0, sfx = known_sfx; i < num_sfx; i++, sfx++ )
	{
		if( !sfx->name )
			continue;
		if( sfx->cache )
			Z_Free( sfx->cache );
		Common::Com_Memset( sfx, 0, sizeof( *sfx ) );
	}

	num_sfx = 0;
#endif
}


// =======================================================================
// Load a sound
// =======================================================================

/*

==================

S_FindName



==================

*/
sfx_t * Sound::S_FindName( const Str & name, bool create ) {

	return NULL;
#if 0
	sfx_t * sfx;

	if( name.IsEmpty( ) ) Common::Com_Error( ERR_FATAL, "S_FindName: empty name\n" );

	if( strlen( name ) >= MAX_QPATH ) Common::Com_Error( ERR_FATAL, "Sound name too long: %s", name.c_str( ) );

	// see if already loaded
	int i;
	for( i = 0; i < num_sfx; i++ ) if( known_sfx[ i ].name && known_sfx[ i ].name == name ) return &known_sfx[ i ];

	if( !create ) return NULL;

		// find a free sfx
	for( i = 0; i < num_sfx; i++ ) if( known_sfx[ i ].name.IsEmpty( ) ) break;

	if( i == num_sfx ) {

		if( num_sfx ==( MAX_SOUNDS * 2 ) ) Common::Com_Error( ERR_FATAL, "S_FindName: out of sfx_t" );
		num_sfx++;
	}

	sfx = &known_sfx[ i ];
	Common::Com_Memset( sfx, 0, sizeof( sfx_t ) );
	sfx->name = name;
	sfx->registration_sequence = s_registration_sequence;

	return sfx;
#endif
}


/*

==================

S_AliasName



==================

*/
sfx_t * Sound::S_AliasName( const Str & aliasname, const Str & truename ) {

	return NULL;

#if 0
	sfx_t * sfx;
	int i;

	// find a free sfx
	for( i = 0; i < num_sfx; i++ ) if( !known_sfx[ i ].name[ 0 ] ) break;

	if( i == num_sfx ) {

		if( num_sfx ==( MAX_SOUNDS * 2 ) ) Common::Com_Error( ERR_FATAL, "S_FindName: out of sfx_t" );
		num_sfx++;
	}

	sfx = &known_sfx[ i ];
	Common::Com_Memset( sfx, 0, sizeof( sfx_t ) );

	sfx->name = aliasname;
	sfx->registration_sequence = s_registration_sequence;
	sfx->truename = truename;

	return sfx;
#endif
}


/*

=====================

S_BeginRegistration



=====================

*/
void Sound::S_BeginRegistration( ) {

#if 0
	s_registration_sequence++;
	s_registering = true;
#endif
}

/*

==================

S_RegisterSound



==================

*/
sfx_t * Sound::S_RegisterSound( const Str & sample ) {

#if 0
	sfx_t * sfx;

	if( !sound_started ) return NULL;

	sfx = S_FindName( sample, true );
	sfx->registration_sequence = s_registration_sequence;

	if( !s_registering ) S_LoadSound( sfx );

	return sfx;
#endif

	return NULL;
}


/*

=====================

S_EndRegistration



=====================

*/
void Sound::S_EndRegistration( ) {

#if 0
	int i;
	sfx_t * sfx;
	int size;

	// free any sounds not from this registration sequence
	for( i = 0, sfx = known_sfx; i < num_sfx; i++, sfx++ ) {

		if( !sfx->name ) continue;
		if( sfx->registration_sequence != s_registration_sequence ) { // don't need this sound

			if( sfx->cache ) // it is possible to have a leftover
				Z_Free( sfx->cache ); // from a server that didn't finish loading

			Common::Com_Memset( sfx, 0, sizeof( *sfx ) );

		} else { // make sure it is paged in

			if( sfx->cache ) {

				size = sfx->cache->length* sfx->cache->width;
				Com_PageInMemory( ( byte * )sfx->cache, size );
			}
		}

	}

	// load everything in
	for( i = 0, sfx = known_sfx; i < num_sfx; i++, sfx++ ) {

		if( !sfx->name )
			continue;
		S_LoadSound( sfx );
	}

	s_registering = false;
#endif
}


//=============================================================================

/*

=================

S_PickChannel

=================

*/
channel_t * Sound::S_PickChannel( int entnum, int entchannel ) {

	int ch_idx;
	int first_to_die;
	int life_left;
	channel_t * ch;

	if( entchannel<0 )
		Common::Com_Error( ERR_DROP, "S_PickChannel: entchannel<0" );

	// Check for replacement sound, or find the best one to replace
	first_to_die = -1;
	life_left = 0x7FFFFFFF;
	for( ch_idx = 0; ch_idx < MAX_CHANNELS; ch_idx++ )
	{
		if( entchannel != 0 // channel 0 never overrides
			&& channels[ ch_idx ].entnum == entnum
			&& channels[ ch_idx ].entchannel == entchannel )
		{ // always override sound from same entity
			first_to_die = ch_idx;
			break;
		}

		// don't let monster sounds override player sounds
		if( channels[ ch_idx ].entnum == Client::cl.playernum+1 && entnum != Client::cl.playernum+1 && channels[ ch_idx ].sfx )
			continue;

		if( channels[ ch_idx ].end - paintedtime < life_left )
		{
			life_left = channels[ ch_idx ].end - paintedtime;
			first_to_die = ch_idx;
		}
	}

	if( first_to_die == -1 )
		return NULL;

	ch = &channels[ first_to_die ];
	Common::Com_Memset( ch, 0, sizeof( *ch ) );

	return ch;
}

/*

=================

S_SpatializeOrigin



Used for spatializing channels and autosounds

=================

*/
void Sound::S_SpatializeOrigin( Vec3 & origin, float master_vol, float dist_mult, int * left_vol, int * right_vol ) {

	vec_t dot;
	vec_t dist;
	vec_t lscale, rscale, scale;
	Vec3 source_vec;

	if( Client::cls.state != ca_active )
	{
		* left_vol = * right_vol = 255;
		return;
	}

	// calculate stereo seperation and distance attenuation
	source_vec = origin - listener_origin;

	dist = source_vec.Normalize( );
	dist -= 80;
	if( dist < 0 )
		dist = 0; // close enough to be at full volume
	dist *= dist_mult; // different attenuation levels

	dot = listener_right * source_vec;

	if( dma.channels == 1 || !dist_mult )
	{ // no attenuation = no spatialization
		rscale = 1.0f;
		lscale = 1.0f;
	}
	else
	{
		rscale = 0.5f * ( 1.0f + dot );
		lscale = 0.5f* ( 1.0f - dot );
	}

	// add in distance effect
	scale =( 1.0f - dist ) * rscale;
	* right_vol =( int )( master_vol * scale );
	if( *right_vol < 0 )
		* right_vol = 0;

	scale =( 1.0f - dist ) * lscale;
	* left_vol =( int )( master_vol * scale );
	if( *left_vol < 0 )
		* left_vol = 0;
}

/*

=================

S_Spatialize

=================

*/
void Sound::S_Spatialize( channel_t * ch ) {

	Vec3 origin;

	// anything coming from the view entity will always be full volume
	if( ch->entnum == Client::cl.playernum+1 )
	{
		ch->leftvol = ch->master_vol;
		ch->rightvol = ch->master_vol;
		return;
	}

	if( ch->fixed_origin )
	{
		origin = ch->origin;
	}
	else
		origin = Client::CL_GetEntitySoundOrigin( ch->entnum );

	S_SpatializeOrigin( origin, ( float )ch->master_vol, ch->dist_mult, &ch->leftvol, &ch->rightvol );
}


/*

=================

S_AllocPlaysound

=================

*/
playsound_t * Sound::S_AllocPlaysound( ) {

	playsound_t * ps;

	ps = s_freeplays.next;
	if( ps == &s_freeplays )
		return NULL; // no free playsounds

	// unlink from freelist
	ps->prev->next = ps->next;
	ps->next->prev = ps->prev;

	return ps;
}


/*

=================

S_FreePlaysound

=================

*/
void Sound::S_FreePlaysound( playsound_t * ps ) {

	// unlink from channel
	ps->prev->next = ps->next;
	ps->next->prev = ps->prev;

	// add to free list
	ps->next = s_freeplays.next;
	s_freeplays.next->prev = ps;
	ps->prev = &s_freeplays;
	s_freeplays.next = ps;
}



/*

===============

S_IssuePlaysound



Take the next playsound and begin it on the channel

This is never called directly by S_Play* , but only

by the update loop.

===============

*/
void Sound::S_IssuePlaysound( playsound_t * ps ) {

	channel_t * ch;
	sfxcache_t * sc;

	if( s_show.GetBool( ) ) Common::Com_Printf( "Issue %i\n", ps->begin );
	// pick a channel to play on
	ch = S_PickChannel( ps->entnum, ps->entchannel );
	if( !ch )
	{
		S_FreePlaysound( ps );
		return;
	}

	// spatialize
	if( ps->attenuation == ATTN_STATIC )
		ch->dist_mult = ps->attenuation * 0.001f;
	else
		ch->dist_mult = ps->attenuation * 0.0005f;
	ch->master_vol = ( int )ps->volume;
	ch->entnum = ps->entnum;
	ch->entchannel = ps->entchannel;
	ch->sfx = ps->sfx;
	ch->origin = ps->origin;
	ch->fixed_origin = ps->fixed_origin;

	S_Spatialize( ch );

	ch->pos = 0;
	sc = S_LoadSound( ch->sfx );
	ch->end = paintedtime + sc->length;

	// free the playsound
	S_FreePlaysound( ps );
}

struct sfx_s * Sound::S_RegisterInlineSound( entity_state_t * ent, const Str & base ) {

	struct sfx_s * sfx;
	Str sdir;

	// determine what model the client is using
	/*int n = CS_PLAYERSKINS + ent->number - 1;
	if( Client::cl.configstrings[ n ] ) {

		sdir = ClientServerInfo::Info_ValueForKey( Client::cl.configstrings[ n ], "model" );
		sdir.StripFilename( );

	} else*/ sdir = "player/male";

	sdir.Append( '/' );
	sdir.Append( base );

	// see if we already know of the model specific sound
	sfx = S_FindName( sdir, false );

	if( sfx ) return sfx;

	sfx = S_RegisterSound( sdir );

	return sfx;
}


// =======================================================================
// Start a sound effect
// =======================================================================

/*

====================

S_StartSound



Validates the parms and ques the sound up

if pos is NULL, the sound will be dynamically sourced from the entity

Entchannel 0 will never override a playing sound

====================

*/
void Sound::S_StartSound( Vec3 * origin, int entnum, int entchannel, sfx_t * sfx, float fvol, float attenuation, float timeofs ) {

#if 0
	sfxcache_t * sc;
	int vol;
	playsound_t * ps, * sort;
	int start;

	if( !sound_started ) return;

	if( !sfx ) return;

	if( sfx->name[ 0 ] == '*' ) sfx = S_RegisterInlineSound( &Client::cl_entities[ entnum ].current, sfx->name.Right( sfx->name.Length( ) - 1 ) );

	// make sure the sound is loaded
	sc = S_LoadSound( sfx );
	if( !sc ) return; // couldn't load the sound's data

	vol = ( int )fvol * 255;

	// make the playsound_t
	ps = S_AllocPlaysound( );
	if( !ps ) return;

	if( origin ) {

		ps->origin = * origin;
		ps->fixed_origin = true;

	} else {
		ps->origin = vec3_origin;
		ps->fixed_origin = false;
	}

	ps->entnum = entnum;
	ps->entchannel = entchannel;
	ps->attenuation = attenuation;
	ps->volume = ( float )vol;
	ps->sfx = sfx;

	// drift s_beginofs
	start = ( int )( Client::cl.frame.servertime * 0.001f ) * dma.speed + s_beginofs;
	if( start < paintedtime ) {

		start = paintedtime;
		s_beginofs = start - ( int )( Client::cl.frame.servertime * 0.001f * dma.speed );

	} else if( start > paintedtime + 0.3f * dma.speed ) {

		start = paintedtime + ( int )( 0.1f * dma.speed );
		s_beginofs = start - ( int )( Client::cl.frame.servertime * 0.001f * dma.speed );

	} else s_beginofs -= 10;

	if( !timeofs ) ps->begin = paintedtime;
	else ps->begin = ( unsigned int )( start + timeofs * dma.speed );

	// sort into the pending sound list
	for( sort = s_pendingplays.next; sort != &s_pendingplays && sort->begin < ps->begin; sort = sort->next );

	ps->next = sort;
	ps->prev = sort->prev;

	ps->next->prev = ps;
	ps->prev->next = ps;
#endif
}


/*

==================

S_StartLocalSound

==================

*/
void Sound::S_StartLocalSound( const Str & s ) {

	sfx_t * sfx;

	if( !sound_started ) return;

	sfx = S_RegisterSound( s );
	if( !sfx ) {

		Common::Com_Printf( "S_StartLocalSound: can't cache %s\n", s.c_str( ) );
		return;
	}

	S_StartSound( NULL, Client::cl.playernum+1, 0, sfx, 1, 1, 0 );
}


/*

==================

S_ClearBuffer

==================

*/
void Sound::S_ClearBuffer( ) {

	int clear;

	if( !sound_started ) return;

	s_rawend = 0;

	if( dma.samplebits == 8 ) clear = 0x80;
	else clear = 0;

	SNDDMA_BeginPainting( );

	if( dma.buffer ) Common::Com_Memset( dma.buffer, clear, dma.samples * dma.samplebits / 8 );
	SNDDMA_Submit( );
}

/*

==================

S_StopAllSounds

==================

*/
void Sound::S_StopAllSounds( ) {

	if( !sound_started ) return;

	// clear all the playsounds
	Common::Com_Memset( s_playsounds, 0, sizeof( s_playsounds ) );
	s_freeplays.next = s_freeplays.prev = &s_freeplays;
	s_pendingplays.next = s_pendingplays.prev = &s_pendingplays;

	for( int i = 0; i < 128; i++ ) {

		s_playsounds[ i ].prev = &s_freeplays;
		s_playsounds[ i ].next = s_freeplays.next;
		s_playsounds[ i ].prev->next = &s_playsounds[ i ];
		s_playsounds[ i ].next->prev = &s_playsounds[ i ];
	}

	// clear all the channels
	Common::Com_Memset( channels, 0, sizeof( channels ) );

	S_ClearBuffer( );
}

/*

==================

S_AddLoopSounds



Entities with a ->sound field will generated looped sounds

that are automatically started, stopped, and merged together

as the entities are sent to the client

==================

*/
void Sound::S_AddLoopSounds( ) {

	int i, j;
	int sounds[ MAX_EDICTS ];
	int left, right, left_total, right_total;
	channel_t * ch;
	sfx_t * sfx;
	sfxcache_t * sc;
	int num;
	entity_state_t * ent;

	if( Common::paused.GetBool( ) ) return;

	if( Client::cls.state != ca_active ) return;

	if( !Client::cl.sound_prepped ) return;

	for( i = 0; i<Client::cl.frame.num_entities; i++ ) {

		num =( Client::cl.frame.parse_entities + i )&( MAX_PARSE_ENTITIES-1 );
		ent = &Client::cl_parse_entities[ num ];
		sounds[ i ] = ent->sound;
	}

	for( i = 0; i<Client::cl.frame.num_entities; i++ ) {

		if( !sounds[ i ] )
			continue;

		sfx = Client::cl.sound_precache[sounds[ i ]];
		if( !sfx )
			continue; // bad sound effect
		sc = sfx->cache;
		if( !sc )
			continue;

		num =( Client::cl.frame.parse_entities + i )&( MAX_PARSE_ENTITIES-1 );
		ent = &Client::cl_parse_entities[ num ];

		// find the total contribution of all sounds of this type
		S_SpatializeOrigin( ent->origin, 255.0f, 0.003f, &left_total, &right_total );
		for( j = i+1; j<Client::cl.frame.num_entities; j++ ) {

			if( sounds[ j ] != sounds[ i ] )
				continue;
			sounds[ j ] = 0; // don't check this again later

			num =( Client::cl.frame.parse_entities + j )&( MAX_PARSE_ENTITIES-1 );
			ent = &Client::cl_parse_entities[ num ];

			S_SpatializeOrigin( ent->origin, 255.0f, 0.003f, &left, &right );
			left_total += left;
			right_total += right;
		}

		if( left_total == 0 && right_total == 0 ) continue; // not audible

		// allocate a channel
		ch = S_PickChannel( 0, 0 );
		if( !ch ) return;

		if( left_total > 255 ) left_total = 255;
		if( right_total > 255 ) right_total = 255;

		ch->leftvol = left_total;
		ch->rightvol = right_total;
		ch->autosound = true; // remove next frame
		ch->sfx = sfx;
		ch->pos = paintedtime % sc->length;
		ch->end = paintedtime + sc->length - ch->pos;
	}
}

//=============================================================================

/*

============

S_RawSamples



Cinematic streaming and voice over network

============

*/
void Sound::S_RawSamples( int samples, int rate, int width, int channels, byte * data ) {

	int i;
	int src, dst;
	float scale;

	if( !sound_started )
		return;

	if( s_rawend < paintedtime )
		s_rawend = paintedtime;
	scale =( float )rate / dma.speed;

	//Common::Com_Printf( "%i < %i < %i\n", soundtime, paintedtime, s_rawend );
	if( channels == 2 && width == 2 )
	{
		if( scale == 1.0f )
		{ // optimized case
			for( i = 0; i<samples; i++ )
			{
				dst = s_rawend&( MAX_RAW_SAMPLES-1 );
				s_rawend++;
				s_rawsamples[ dst ].left =
					LittleShort( ( ( short * )data )[i* 2] ) << 8;
				s_rawsamples[ dst ].right =
					LittleShort( ( ( short * )data )[i* 2+1] ) << 8;
			}
		}
		else
		{
			for( i = 0;; i++ )
			{
				src = ( int )( i * scale );
				if( src >= samples )
					break;
				dst = s_rawend&( MAX_RAW_SAMPLES-1 );
				s_rawend++;
				s_rawsamples[ dst ].left =
					LittleShort( ( ( short * )data )[src* 2] ) << 8;
				s_rawsamples[ dst ].right =
					LittleShort( ( ( short * )data )[src* 2+1] ) << 8;
			}
		}
	}
	else if( channels == 1 && width == 2 )
	{
		for( i = 0;; i++ )
		{
			src = ( int )( i * scale );
			if( src >= samples )
				break;
			dst = s_rawend&( MAX_RAW_SAMPLES-1 );
			s_rawend++;
			s_rawsamples[ dst ].left =
				LittleShort( ( ( short * )data )[ src ] ) << 8;
			s_rawsamples[ dst ].right =
				LittleShort( ( ( short * )data )[ src ] ) << 8;
		}
	}
	else if( channels == 2 && width == 1 )
	{
		for( i = 0;; i++ )
		{
			src = ( int )( i * scale );
			if( src >= samples )
				break;
			dst = s_rawend&( MAX_RAW_SAMPLES-1 );
			s_rawend++;
			s_rawsamples[ dst ].left =
				( ( char * )data )[src* 2] << 16;
			s_rawsamples[ dst ].right =
				( ( char * )data )[src* 2+1] << 16;
		}
	}
	else if( channels == 1 && width == 1 )
	{
		for( i = 0;; i++ )
		{
			src = ( int )( i * scale );
			if( src >= samples )
				break;
			dst = s_rawend&( MAX_RAW_SAMPLES-1 );
			s_rawend++;
			s_rawsamples[ dst ].left =
				( ( ( byte * )data )[ src ]-128 ) << 16;
			s_rawsamples[ dst ].right =( ( ( byte * )data )[ src ]-128 ) << 16;
		}
	}
}

//=============================================================================

/*

============

S_Update



Called once each time through the main loop

============

*/
void Sound::S_Update( Vec3 & origin, Vec3 & forward, Vec3 & right, Vec3 & up ) {

	int i;
	int total;
	channel_t * ch;
	channel_t * combine;

	if( !sound_started )
		return;

	// if the laoding plaque is up, clear everything
	// out to make sure we aren't looping a dirty
	// dma buffer while loading
	if( Client::cls.disable_screen )
	{
		S_ClearBuffer( );
		return;
	}

	// rebuild scale tables if volume is modified
	if( s_volume.IsModified( ) ) S_InitScaletable( );

	listener_origin = origin;
	listener_forward = forward;
	listener_right = right;
	listener_up = up;

	combine = NULL;

	// update spatialization for dynamic sounds	
	ch = channels;
	for( i = 0; i<MAX_CHANNELS; i++, ch++ )
	{
		if( !ch->sfx )
			continue;
		if( ch->autosound )
		{ // autosounds are regenerated fresh each frame
			Common::Com_Memset( ch, 0, sizeof( *ch ) );
			continue;
		}
		S_Spatialize( ch ); // respatialize channel
		if( !ch->leftvol && !ch->rightvol )
		{
			Common::Com_Memset( ch, 0, sizeof( *ch ) );
			continue;
		}
	}

	// add loopsounds
	S_AddLoopSounds( );

	//
	// debugging output
	//
	if( s_show.GetBool( ) ) {

		total = 0;
		ch = channels;
		for( i = 0; i<MAX_CHANNELS; i++, ch++ )
			if( ch->sfx &&( ch->leftvol || ch->rightvol ) )
			{
				Common::Com_Printf( "%3i %3i %s\n", ch->leftvol, ch->rightvol, ch->sfx->name.c_str( ) );
				total++;
			}

			Common::Com_Printf( "----( %i )---- painted: %i\n", total, paintedtime );
	}

	// mix some sound
	S_Mix( );
}

void Sound::GetSoundtime( ) {

	int samplepos;
	static int			buffers;
static int			oldsamplepos;
	int fullsamples;

	fullsamples = dma.samples / dma.channels;

	// it is possible to miscount buffers if it has wrapped twice between
	// calls to S_Update.  Oh well.
	samplepos = SNDDMA_GetDMAPos( );

	if( samplepos < oldsamplepos ) {

		buffers++; // buffer wrapped

		if( paintedtime > 0x40000000 )
		{ // time to chop things off to avoid 32 bit limits
			buffers = 0;
			paintedtime = fullsamples;
			S_StopAllSounds( );
		}
	}
	oldsamplepos = samplepos;

	soundtime = buffers* fullsamples + samplepos/dma.channels;
}


void Sound::S_Mix( ) {

	unsigned endtime;
	int samps;

	if( !sound_started )
		return;

	SNDDMA_BeginPainting( );

	if( !dma.buffer )
		return;

	// Updates DMA time
	GetSoundtime( );

	// check to make sure that we haven't overshot
	if( paintedtime < soundtime )
	{
		Common::Com_DPrintf( "S_Update_ : overflow\n" );
		paintedtime = soundtime;
	}

	// mix ahead of current position
	endtime = soundtime + ( int )( s_mixahead.GetFloat( ) * ( float )dma.speed );
	//endtime =( soundtime + 4096 ) & ~4095;

	// mix to an even submission block size
	endtime =( endtime + dma.submission_chunk-1 )
		& ~( dma.submission_chunk-1 );
	samps = dma.samples >>( dma.channels-1 );
	if( ( int )endtime - soundtime > samps )
		endtime = ( unsigned int )( soundtime + samps );

	S_PaintChannels( endtime );

	SNDDMA_Submit( );
}

/*

===============================================================================



console functions



===============================================================================

*/
void Sound::S_Play( ) {

	int i;
	Str name;
	sfx_t * sfx;

	i = 1;
	while( i < Command::Cmd_Argc( ) ) {

		if( Command::Cmd_Argv( i ).Find( '.' ) == -1 ) {

			name = Command::Cmd_Argv( i );
			name.Append( ".wav" );

		} else name = Command::Cmd_Argv( i );

		sfx = S_RegisterSound( name );
		S_StartSound( NULL, Client::cl.playernum+1, 0, sfx, 1.0f, 1.0f, 0 );
		i++;
	}
}

void Sound::S_SoundList( ) {

	int i;
	sfx_t * sfx;
	sfxcache_t * sc;
	int size, total;

	total = 0;
	for( sfx = known_sfx, i = 0; i < num_sfx; i++, sfx++ ) {

		if( !sfx->registration_sequence ) continue;
		sc = sfx->cache;
		if( sc ) {

			size = sc->length * sc->width * ( sc->stereo + 1 );
			total += size;

			if( sc->loopstart >= 0 ) Common::Com_Printf( "L" );
			else Common::Com_Printf( " " );

			Common::Com_Printf( "( %2db ) %6i : %s\n", sc->width * 8, size, sfx->name.c_str( ) );

		} else {

			if( sfx->name[ 0 ] == '*' ) Common::Com_Printf( "  placeholder : %s\n", sfx->name.c_str( )  );
			else Common::Com_Printf( "  not loaded  : %s\n", sfx->name.c_str( )  );
		}
	}
	Common::Com_Printf( "Total resident: %i\n", total );
}
