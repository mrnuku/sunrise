struct sfx_s;

/*
====================================================================

  SYSTEM SPECIFIC FUNCTIONS

====================================================================
*/

class Sound {

private:

	static CVar				s_initsound;
	static CVar				s_volume;
	static CVar				s_khz;
	static CVar				s_loadas8bit;
	static CVar				s_mixahead;
	static CVar				s_show;
	static CVar				s_testsound;
	static CVar				s_primary;
	static CVar				s_wavonly;

	static sfx_t			known_sfx[ MAX_SOUNDS * 2 ];

public:

	static const Str		DSoundError( int error );
	static bool				DS_CreateBuffers( );
	static void				DS_DestroyBuffers( );
	static void				FreeSound( );
	static bool				SNDDMA_InitDirect( );
	static bool				SNDDMA_InitWav( );

	// initializes cycling through a DMA buffer and returns information on it
	static bool			SNDDMA_Init( );

	// gets the current DMA position
	static int		SNDDMA_GetDMAPos( );

	// shutdown the DMA xfer.
	static void	SNDDMA_Shutdown( );

	static void	SNDDMA_BeginPainting( );

	static void	SNDDMA_Submit( );

	static wavinfo_t			GetWavinfo( const Str & name, const byte * wav, int wavlength );

	static void			S_InitScaletable( );

	static sfxcache_t			* S_LoadSound( sfx_t * s );

	static void			S_IssuePlaysound( playsound_t * ps );

	static void			S_PaintChannels( int endtime );

	// picks a channel based on priorities, empty slots, number of channels
	static channel_t			* S_PickChannel( int entnum, int entchannel );

	// spatializes a channel
	static void			S_Spatialize( channel_t * ch );

	static void			S_Init( );
	static void			S_Shutdown( );

	// if origin is NULL, the sound will be dynamically sourced from the entity
	static void			S_StartSound( Vec3 * origin, int entnum, int entchannel, struct sfx_s * sfx, float fvol,  float attenuation, float timeofs );
	static void			S_StartLocalSound( const Str & s );

	static void			S_RawSamples( int samples, int rate, int width, int channels, byte * data );

	static void			S_StopAllSounds( );
	static void			S_Update( Vec3 & origin, Vec3 & v_forward, Vec3 & v_right, Vec3 & v_up );

	static void			S_Activate( bool active );

	static void			S_BeginRegistration( );
	static struct			sfx_s * S_RegisterSound( const Str & sample );
	static void			S_EndRegistration( );

	static struct			sfx_s * S_FindName( const Str & name, bool create );

	static void					S_SoundInfo_f( );
	static struct			sfx_s * 		S_AliasName( const Str & aliasname, const Str & truename );
	static void					S_SpatializeOrigin( Vec3 & origin, float master_vol, float dist_mult, int * left_vol, int * right_vol );
	static struct			playsound_s * 	S_AllocPlaysound( );
	static void					S_FreePlaysound( struct playsound_s * ps );
	static struct			sfx_s * 		S_RegisterInlineSound( entity_state_t * ent, const Str & base );
	static void					S_ClearBuffer( );
	static void					S_AddLoopSounds( );
	static void					GetSoundtime( );
	static void					S_Mix( );
	static void					S_Play( );
	static void					S_SoundList( );

	static void					ResampleSfx( sfx_t * sfx, int inrate, int inwidth, const byte * data );

	static void					S_WriteLinearBlastStereo16( );
	static void					S_TransferStereo16( unsigned long * pbuf, int endtime );
	static void					S_TransferPaintBuffer( int endtime );
	static void					S_PaintChannelFrom8( channel_t * ch, sfxcache_t * sc, int count, int offset );
	static void					S_PaintChannelFrom16( channel_t * ch, sfxcache_t * sc, int count, int offset );
};

