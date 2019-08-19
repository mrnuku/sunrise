#ifndef MOVIEEXPORT_H
#define MOVIEEXPORT_H

#pragma pack ( push, 1 )

class FLV_Header {

public:

	enum {

		FLV_Video				= BIT( 0 ),
		FLV_Audio				= BIT( 2 )
	};

	char						magic[ 3 ];	// 'FLV'
	byte						version;	// 1
	byte						flags;		// bit 3 = audio, bit 1 = video
	Int32< BO_BigEndian >		dataOffset; // sizeof( FLV_Header )

								FLV_Header( ) {

									magic[ 0 ] = 'F'; magic[ 1 ] = 'L'; magic[ 2 ] = 'V';
									version = 1;
									flags = 0;
									dataOffset = sizeof( FLV_Header );
								}
};

class FLV_Tag {

public:

	enum TagType : byte {

		TAG_Audio				= 0x08,
		TAG_Video				= 0x09,
		TAG_Meta				= 0x12
	};

	TagType						type;
	Int24< BO_BigEndian >		size;
	Int24< BO_BigEndian >		timeStamp;
	byte						TimeStampExt;
	Int24< BO_BigEndian >		streamID; // 0

								FLV_Tag( ) {

									TimeStampExt = 0;
									streamID = 0;
								}
};

#pragma pack ( pop )

class FLV_Writer {

private:

	FileBase *					outputFile;
	Int32< BO_BigEndian >		prevTagSize;
	byte						audioByte;
	byte						videoByte;
	int							durationOffset;
	int							filesizeOffset;

	enum AudioType {

		FLV_Mono				= 0,
		FLV_Stereo				= BIT( 0 ),

		FLV_8bit				= 0,
		FLV_16bit				= BIT( 1 ),

		FLV_5_5khz				= 0,
		FLV_11khz				= BIT( 2 ),
		FLV_22khz				= BIT( 3 ),
		FLV_44khz				= BIT( 2 ) | BIT( 3 ),

		FLV_Uncompressed		= 0,
		FLV_ADPCM				= BIT( 4 ),
		FLV_MP3					= BIT( 5 ),
		FLV_Nelly8khzMono		= BIT( 6 ) | BIT( 4 ),
		FLV_Nelly				= BIT( 6 ) | BIT( 5 ),
		FLV_Speex				= BIT( 6 ) | BIT( 5 ) | BIT( 4 ),
		FLV_AAC					= BIT( 7 ) | BIT( 5 )
	};

	enum VideoType {

		FLV_Sorensen_H263		= BIT( 1 ),
		FLV_ScreenVideo			= BIT( 1 ) | BIT( 0 ),
		FLV_On2_VP6				= BIT( 2 ),
		FLV_On2_VP6_Alpha		= BIT( 2 ) | BIT( 0 ),
		FLV_ScreenVideo2		= BIT( 2 ) | BIT( 1 ),
		FLV_H264				= BIT( 2 ) | BIT( 1 ) | BIT( 0 ),

		FLV_KeyFrame			= BIT( 4 ),
		FLV_InterFrame			= BIT( 5 ),
		FLV_DisposableFrame		= BIT( 5 ) | BIT( 4 ),
	};

	enum AMFDataType {

		AMF_Number				= 0x00,
		AMF_Bool				= 0x01,
		AMF_String				= 0x02,
		AMF_Object				= 0x03,
		AMF_Null				= 0x05,
		AMF_Undefined			= 0x06,
		AMF_Reference			= 0x07,
		AMF_MixedArray			= 0x08,
		AMF_ObjectEnd			= 0x09,
		AMF_Array				= 0x0a,
		AMF_Date				= 0x0b,
		AMF_LongString			= 0x0c,
		AMF_Unsupported			= 0x0d
	};

public:

	bool						SetOutputFile( const Str & file_name );
	void						CloseOutputFile( int duration );

	void						WriteHeader( bool have_audio, bool have_video );
	void						WriteVideoFrame( const void * data, int size, int time_stamp, bool is_keyfame );
	void						WriteAudioFrame( const void * data, int size, int time_stamp );
};

class MovieExportThread : public ThreadProcess {

private:

	ThreadBase *				thread;
	Signal						workSignal;
	Signal						workEndSignal;

public:

								MovieExportThread( ): thread( NULL ) { }

	void						StartThread( );
	void						StopThread( );

	void						SignalThread( ) { workSignal.Set( ); };
	void						WaitForThread( ) { workEndSignal.Wait( ); };

	virtual unsigned int		Run( void * param );
	virtual void				Stop( );
};

/* CONFIG: VFW config */
typedef struct {

	int i_format_version;
	/* Basic */
	int i_preset;
	int i_tuning;
	int i_profile;
	int i_level;
	int b_fastdecode;
	int b_zerolatency;
	/* Rate control */
	int i_encoding_type;
	int i_qp;
	int i_rf_constant;  /* 1pass VBR, nominal QP */
	int i_passbitrate;
	int i_pass;
	int b_fast1pass;    /* Turns off some flags during 1st pass */
	int b_createstats;  /* Creates the statsfile in single pass mode */
	int b_updatestats;  /* Updates the statsfile during 2nd pass */
	char stats[ MAX_PATH ];
	/* Output */
	int i_output_mode;
	int i_fourcc;
	int b_vd_hack;
	char output_file[ MAX_PATH ];
	/* Sample Aspect Ratio */
	int i_sar_width;
	int i_sar_height;
	/* Debug */
	int i_log_level;
	int b_psnr;
	int b_ssim;
	int b_no_asm;
	/* Decoder && AVI Muxer */
	int b_disable_decoder;
	/* Extra command line */
	char extra_cmdline[ 4096 ];

} x264Config_t;

typedef struct {

	/* x264 handle */
	void * h;

	/* Configuration GUI params */
	x264Config_t config;

} x264Codec_t;

typedef WindowsNS::LRESULT ( WINAPI * MEDRIVERPROC )( x264Codec_t * dwDriverId, WindowsNS::HDRVR hDriver, WindowsNS::UINT uMsg, WindowsNS::DWORD_PTR lParam1, WindowsNS::DWORD_PTR lParam2 );

typedef int ( FAACAPI * FAACENCGETVERSIONPROC )( char ** faac_id_string, char ** faac_copyright_string );
typedef faac::faacEncConfigurationPtr( FAACAPI *  FAACENCGETCURRENTCONFIGURATIONPROC )( faac::faacEncHandle hEncoder );
typedef int ( FAACAPI *  FAACENCSETCONFIGURATIONPROC )( faac::faacEncHandle hEncoder, faac::faacEncConfigurationPtr config );
typedef faac::faacEncHandle ( FAACAPI *  FAACENCOPENPROC )( unsigned long sampleRate, unsigned int numChannels, unsigned long * inputSamples, unsigned long * maxOutputBytes );
typedef int ( FAACAPI *  FAACENCGETDECODERSPECIFICINFOPROC )( faac::faacEncHandle hEncoder, unsigned char ** ppBuffer, unsigned long * pSizeOfDecoderSpecificInfo );
typedef int ( FAACAPI *  FAACENCENCODEPROC )( faac::faacEncHandle hEncoder, int32_t * inputBuffer, unsigned int samplesInput, unsigned char * outputBuffer, unsigned int bufferSize );
typedef int ( FAACAPI *  FAACENCCLOSEPROC )( faac::faacEncHandle hEncoder );

class MovieExport {

	friend class									MovieExportThread;

private:

	static DynamicModule *							x264Module;
	static DynamicModule *							faccModule;

	static WindowsNS::BITMAPINFO					inputFormat;
	static WindowsNS::BITMAPINFO					outputFormat;
	static byte *									readBuffer;
	static byte *									inputBuffer;
	static byte *									outputBuffer;
	static WindowsNS::ICCOMPRESS					compParams;
	static WindowsNS::DWORD							compChunkId;
	static WindowsNS::DWORD							compFlags;
	static bool										isValid;
	static bool										isActive;
	static timeType									firstFrameTime;
	static timeType									lastFrameTime;
	static timeType									deltaFrameTime;
	static int										soundSampleCountRaw;
	static void *									soundSampleBufferRaw;
	static faac::faacEncHandle						faacHandle;
	static unsigned long							faacInputSamples;
	static unsigned long							faacMaxOutputBytes;
	static faac::faacEncConfigurationPtr			faacConfig;
	static int *									faacInputBuffer;
	static unsigned char *							faacOutputBuffer;
	static MovieExportThread						compThread;
	static x264Codec_t *							codecInstance;
	static FLV_Writer								outputFile;

	static MEDRIVERPROC								DriverProc;

	static FAACENCGETVERSIONPROC					faacEncGetVersion;
	static FAACENCGETCURRENTCONFIGURATIONPROC		faacEncGetCurrentConfiguration;
	static FAACENCSETCONFIGURATIONPROC				faacEncSetConfiguration;
	static FAACENCOPENPROC							faacEncOpen;
	static FAACENCGETDECODERSPECIFICINFOPROC		faacEncGetDecoderSpecificInfo;
	static FAACENCENCODEPROC						faacEncEncode;
	static FAACENCCLOSEPROC							faacEncClose;

public:

	static void										Init( );
	static void										Shutdown( );
	static void										Frame( );

	static void										Configure( );
	static void										Compress( );
};

INLINE dword FourCC( const char * code ) {

	return	( ( dword )( byte )( code[ 0 ] ) | ( ( dword )( byte )( code[ 1 ] ) << 8 ) |
			( ( dword )( byte )( code[ 2 ] ) << 16 ) | ( ( dword )( byte )( code[ 3 ] ) << 24 ) );
}

#endif
