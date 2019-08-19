#ifndef SOUNDSYSTEM_H
#define SOUNDSYSTEM_H

class ALImports {

private:

	static DynamicModule *				alModule;

public:

	static LPALENABLE					alEnable;
	static LPALDISABLE					alDisable;
	static LPALISENABLED				alIsEnabled;
	static LPALGETBOOLEAN				alGetBoolean;
	static LPALGETINTEGER				alGetInteger;
	static LPALGETFLOAT					alGetFloat;
	static LPALGETDOUBLE				alGetDouble;
	static LPALGETBOOLEANV				alGetBooleanv;
	static LPALGETINTEGERV				alGetIntegerv;
	static LPALGETFLOATV				alGetFloatv;
	static LPALGETDOUBLEV				alGetDoublev;
	static LPALGETSTRING				alGetString;
	static LPALGETERROR					alGetError;
	static LPALISEXTENSIONPRESENT		alIsExtensionPresent;
	static LPALGETPROCADDRESS			alGetProcAddress;
	static LPALGETENUMVALUE				alGetEnumValue;
	static LPALLISTENERI				alListeneri;
	static LPALLISTENERF				alListenerf;
	static LPALLISTENER3F				alListener3f;
	static LPALLISTENERFV				alListenerfv;
	static LPALGETLISTENERI				alGetListeneri;
	static LPALGETLISTENERF				alGetListenerf;
	static LPALGETLISTENER3F			alGetListener3f;
	static LPALGETLISTENERFV			alGetListenerfv;
	static LPALGENSOURCES				alGenSources;
	static LPALDELETESOURCES			alDeleteSources;
	static LPALISSOURCE					alIsSource;
	static LPALSOURCEI					alSourcei;
	static LPALSOURCEF					alSourcef;
	static LPALSOURCE3F					alSource3f;
	static LPALSOURCEFV					alSourcefv;
	static LPALGETSOURCEI				alGetSourcei;
	static LPALGETSOURCEF				alGetSourcef;
	static LPALGETSOURCEFV				alGetSourcefv;
	static LPALSOURCEPLAYV				alSourcePlayv;
	static LPALSOURCESTOPV				alSourceStopv;
	static LPALSOURCEPLAY				alSourcePlay;
	static LPALSOURCEPAUSE				alSourcePause;
	static LPALSOURCESTOP				alSourceStop;
	static LPALGENBUFFERS				alGenBuffers;
	static LPALDELETEBUFFERS			alDeleteBuffers;
	static LPALISBUFFER					alIsBuffer;
	static LPALBUFFERDATA				alBufferData;
	static LPALGETBUFFERI				alGetBufferi;
	static LPALGETBUFFERF				alGetBufferf;
	static LPALSOURCEQUEUEBUFFERS		alSourceQueueBuffers;
	static LPALSOURCEUNQUEUEBUFFERS		alSourceUnqueueBuffers;
	static LPALDISTANCEMODEL			alDistanceModel;
	static LPALDOPPLERFACTOR			alDopplerFactor;
	static LPALDOPPLERVELOCITY			alDopplerVelocity;
	static LPALCGETSTRING				alcGetString;
	static LPALCGETINTEGERV				alcGetIntegerv;
	static LPALCOPENDEVICE				alcOpenDevice;
	static LPALCCLOSEDEVICE				alcCloseDevice;
	static LPALCCREATECONTEXT			alcCreateContext;
	static LPALCMAKECONTEXTCURRENT		alcMakeContextCurrent;
	static LPALCPROCESSCONTEXT			alcProcessContext;
	static LPALCGETCURRENTCONTEXT		alcGetCurrentContext;
	static LPALCGETCONTEXTSDEVICE		alcGetContextsDevice;
	static LPALCSUSPENDCONTEXT			alcSuspendContext;
	static LPALCDESTROYCONTEXT			alcDestroyContext;
	static LPALCGETERROR				alcGetError;
	static LPALCISEXTENSIONPRESENT		alcIsExtensionPresent;
	static LPALCGETPROCADDRESS			alcGetProcAddress;
	static LPALCGETENUMVALUE			alcGetEnumValue;
	static LPALCCAPTUREOPENDEVICE		alcCaptureOpenDevice;
	static LPALCCAPTURECLOSEDEVICE		alcCaptureCloseDevice;
	static LPALCCAPTURESTART			alcCaptureStart;
	static LPALCCAPTURESTOP				alcCaptureStop;
	static LPALCCAPTURESAMPLES			alcCaptureSamples;

	static void							Init( );
	static void							Shutdown( );
};

class WAV_Reader {
private:
	const byte *									d_iffData;
	const byte *									d_iffEnd;
	int												d_iffChunkLen;
	const byte *									d_dataPtr;
	const byte *									d_lastChunk;

public:

													WAV_Reader( const void * iff_ptr, size_t iff_size );

	bool											FindChunk( const Str & chunk_name );
	bool											FindNextChunk( const Str & chunk_name );
	short											GetLittleShort( );
	int												GetLittleLong( );

	const byte *									GetDataPtr( ) const					{ return d_dataPtr; }
	void											AdvanceDataPtr( int offs )			{ d_dataPtr += offs; }
	bool											CompareDataPtr( const Str & str, int offs = 0 )	{ return d_dataPtr ? !str.Cmpn( ( char * )( d_dataPtr + offs ), str.Length( ) ) : false; }
	void											SetIffPtr( const byte * iff_ptr )	{ d_iffData = iff_ptr; }
};

class SoundBuffer : public NamedObject {
	friend class									SoundSource;
private:

	ALuint											d_bufferID;
	CAList< SoundSource *, 64 >						d_sourceList;

	ALenum											GetFormat( int channels, int bits );
	bool											LoadWAV( const void * data_ptr, size_t data_size );

public:

													SoundBuffer( );
													~SoundBuffer( );

	bool											UpLoad( ALenum buffer_format, const ALvoid * buffer_data, ALsizei buffer_size, ALsizei buffer_frequency );
	bool											Load( const Str & file_name );
	SoundSource *									FindUnused( );
};

class SoundSource {

private:

	ALuint											d_sourceID;
	SoundBuffer *									d_sourceBuffer;

public:

													SoundSource( );
													~SoundSource( );

	int												GetParamInt( ALenum p_name ) const			{ int pValue; ALImports::alGetSourcei( d_sourceID, p_name, &pValue ); return pValue; }
	void											SetParamInt( ALenum p_name, int p_value )	{ ALImports::alSourcei( d_sourceID, p_name, p_value ); }
	float											GetParamFloat( ALenum p_name ) const		{ float pValue; ALImports::alGetSourcef( d_sourceID, p_name, &pValue ); return pValue; }
	void											SetParamFloat( ALenum p_name, float val )	{ ALImports::alSourcef( d_sourceID, p_name, val ); }
	Vec3											GetParamVec( ALenum p_name ) const			{ Vec3 pValue; ALImports::alGetSourcefv( d_sourceID, p_name, pValue.ToFloatPtr( ) ); return pValue; }
	void											SetParamVec( ALenum p_name, Vec3 & val )	{ ALImports::alSourcefv( d_sourceID, p_name, val.ToFloatPtr( ) ); }

	void											Play( )										{ ALImports::alSourcePlay( d_sourceID ); }
	void											Pause( )									{ ALImports::alSourcePause( d_sourceID ); }
	void											Stop( )										{ ALImports::alSourceStop( d_sourceID ); }
	//void											Rewind( )									{ ALImports::alSourceRewind( d_sourceID ); }

	bool											IsPlaying( ) const							{ return GetParamInt( AL_SOURCE_STATE ) == AL_PLAYING; }

	void											Bind( SoundBuffer * sound_buffer )			{ d_sourceBuffer = sound_buffer; sound_buffer->d_sourceList.Append( this ); SetParamInt( AL_BUFFER, sound_buffer->d_bufferID ); }
	void											UnBind( )									{ d_sourceBuffer->d_sourceList.Remove( this ); SetParamInt( AL_BUFFER, AL_NONE ); }
	void											Looping( bool status )						{ SetParamInt( AL_LOOPING, status ); }
};

class SoundSystem {

private:

	static CVar										snd_captureid;

	static bool										isValid;

	static ALCdevice *								alcDevice;
	static ALCcontext *								alcContext;
	static ALCdevice *								alcCaptureDevice;

	static bool										isCaptureEnabled;
	static int										captureBufferSize;
	static void *									captureBuffer;
	//static SoundBuffer *							captureTestBuffer[ 2 ];
	//static SoundSource *							captureTestSource[ 2 ];

	static CANamedList< SoundBuffer *, 512 >		g_bufferList;
	static CAList< SoundSource *, 256 >				g_sourceList;

	static Blip_Buffer								blipBuffer;
	static Blip_Synth< blip_good_quality, 40 >		blipSynth;

	static SoundBuffer *							blipTestBuffer;
	static SoundSource *							blipTestSource;

	static Vec3										listenerLastOrigin;

	static void										TestSoundSystem( );

public:

	static void										InitMenu( );
	static void										Init( );
	static void										Shutdown( );

	static void										BeginRegistration( );
	static void										EndRegistration( );
	static SoundBuffer *							RegisterSample( const Str & file_name );

	static SoundSource *							PlaySampleOnPos( SoundBuffer * sample_buffer, const Vec3 & sample_origin, float volume, float attenuation, float start_ofs );
	static SoundSource *							PlaySampleOnEnt( SoundBuffer * sample_buffer, int sample_entnum, float volume, float attenuation, float start_ofs );
	static SoundSource *							PlaySampleOnLocal( SoundBuffer * sample_buffer, float volume, float attenuation, float start_ofs );
	static SoundSource *							PlaySampleOnLocal( const Str & file_name );

	static void										StopAllSounds( );
	static void										Activate( );
	static void										Deactivate( );
	static void										Frame( const Vec3 & listener_origin, const Vec3 & listener_forward, const Vec3 & listener_right, const Vec3 & listener_up );

	static void										EnableCapture( );
	static void										DisableCapture( );
	static void *									Capture( int & sample_count );

	static SoundBuffer *							AllocBuffer( );
	static SoundSource *							AllocSource( );
};

INLINE bool SoundBuffer::UpLoad( ALenum buffer_format, const ALvoid * buffer_data, ALsizei buffer_size, ALsizei buffer_frequency ) {

	ALImports::alBufferData( d_bufferID, buffer_format, buffer_data, buffer_size, buffer_frequency );
	return ( ALImports::alGetError( ) == AL_NO_ERROR );
}

INLINE SoundSource * SoundBuffer::FindUnused( ) {
	for( int i = 0; i < d_sourceList.Num( ); i++ )
		if( !d_sourceList[ i ]->IsPlaying( ) )
			return d_sourceList[ i ];
	return NULL;
}

#endif
