#include "precompiled.h"
#pragma hdrstop

#define iDirectSoundCreate( a, b, c )	pDirectSoundCreate( a, b, c )

#define HRESULT				WindowsNS::HRESULT
#define UINT				WindowsNS::UINT
#define DWORD				WindowsNS::DWORD

#if 0
typedef HRESULT( WINAPI * PFNDIRECTSOUNDCREATEPROC )( WindowsNS::GUID FAR * lpGUID, WindowsNS::LPDIRECTSOUND FAR * lplpDS, WindowsNS::IUnknown FAR * pUnkOuter );
PFNDIRECTSOUNDCREATEPROC pDirectSoundCreate;
#endif

// 64K is > 1 second at 16-bit, 22050 Hz
#define	WAV_BUFFERS				64
#define	WAV_MASK				0x3F
#define	WAV_BUFFER_SIZE			0x0400
#define SECONDARY_BUFFER_SIZE	0x10000

typedef enum {SIS_SUCCESS, SIS_FAILURE, SIS_NOTAVAIL} sndinitstat;

static bool	dsound_init;
static bool	wav_init;
static bool	snd_firsttime = true, snd_isdirect, snd_iswave;
static bool	primary_format_set;

// starts at 0 for disabled
static int	snd_buffer_count = 0;
static int	sample16;
static int	snd_sent, snd_completed;

/* 
 * Global variables. Must be visible to window-procedure function 
 *  so it can unlock and free the data block after it has been played. 
 */ 


WindowsNS::HANDLE		hData;
WindowsNS::HPSTR		lpData, lpData2;

WindowsNS::HGLOBAL		hWaveHdr;
WindowsNS::LPWAVEHDR	lpWaveHdr;

WindowsNS::HWAVEOUT    hWaveOut; 

WindowsNS::WAVEOUTCAPS	wavecaps;

DWORD	gSndBufSize;

WindowsNS::MMTIME		mmstarttime;

#if 0
WindowsNS::LPDIRECTSOUND pDS;
WindowsNS::LPDIRECTSOUNDBUFFER pDSBuf, pDSPBuf;
#endif

WindowsNS::HINSTANCE hInstDS;

const Str Sound::DSoundError( int error ) {

	return "unknown";
}

/*
* * DS_CreateBuffers
*/
bool Sound::DS_CreateBuffers( ) {

	return false;
#if 0
	WindowsNS::DSBUFFERDESC		dsbuf;
	WindowsNS::DSBCAPS			dsbcaps;
	WindowsNS::WAVEFORMATEX		pformat, format;
	DWORD			dwWrite;

	Common::Com_Memset( &format, 0, sizeof( format ) );
	format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = dma.channels;
    format.wBitsPerSample = dma.samplebits;
    format.nSamplesPerSec = dma.speed;
    format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
    format.cbSize = 0;
    format.nAvgBytesPerSec = format.nSamplesPerSec* format.nBlockAlign; 

	Common::Com_Printf( "Creating DS buffers\n" );

	Common::Com_DPrintf( "...setting EXCLUSIVE coop level: " );
	if( DS_OK != pDS->SetCooperativeLevel( cl_hwnd, DSSCL_EXCLUSIVE ) )
	{
		Common::Com_Printf( "failed\n" );
		FreeSound( );
		return false;
	}
	Common::Com_DPrintf( "ok\n" );

// get access to the primary buffer, if possible, so we can set the
// sound hardware format
	Common::Com_Memset( &dsbuf, 0, sizeof( dsbuf ) );
	dsbuf.dwSize = sizeof( WindowsNS::DSBUFFERDESC );
	dsbuf.dwFlags = DSBCAPS_PRIMARYBUFFER;
	dsbuf.dwBufferBytes = 0;
	dsbuf.lpwfxFormat = NULL;

	Common::Com_Memset( &dsbcaps, 0, sizeof( dsbcaps ) );
	dsbcaps.dwSize = sizeof( dsbcaps );
	primary_format_set = false;

	Common::Com_DPrintf( "...creating primary buffer: " );
	if( DS_OK == pDS->CreateSoundBuffer( &dsbuf, &pDSPBuf, NULL ) ) {
		pformat = format;

		Common::Com_DPrintf( "ok\n" );
		if( DS_OK != pDSPBuf->SetFormat( &pformat ) )
		{
			if( snd_firsttime )
				Common::Com_DPrintf( "...setting primary sound format: failed\n" );
		}
		else
		{
			if( snd_firsttime )
				Common::Com_DPrintf( "...setting primary sound format: ok\n" );

			primary_format_set = true;
		}
	}
	else
		Common::Com_Printf( "failed\n" );

	if( !primary_format_set || !s_primary.GetBool( ) )
	{
	// create the secondary buffer we'll actually work with
		Common::Com_Memset( &dsbuf, 0, sizeof( dsbuf ) );
		dsbuf.dwSize = sizeof( WindowsNS::DSBUFFERDESC );
		dsbuf.dwFlags = DSBCAPS_CTRLFREQUENCY | DSBCAPS_LOCSOFTWARE;
		dsbuf.dwBufferBytes = SECONDARY_BUFFER_SIZE;
		dsbuf.lpwfxFormat = &format;

		Common::Com_Memset( &dsbcaps, 0, sizeof( dsbcaps ) );
		dsbcaps.dwSize = sizeof( dsbcaps );

		Common::Com_DPrintf( "...creating secondary buffer: " );
		if( DS_OK != pDS->CreateSoundBuffer( &dsbuf, &pDSBuf, NULL ) )
		{
			Common::Com_Printf( "failed\n" );
			FreeSound( );
			return false;
		}
		Common::Com_DPrintf( "ok\n" );

		dma.channels = format.nChannels;
		dma.samplebits = format.wBitsPerSample;
		dma.speed = format.nSamplesPerSec;

		if( DS_OK != pDSBuf->GetCaps( &dsbcaps ) ) {

			Common::Com_Printf( "***GetCaps failed***\n" );
			FreeSound( );
			return false;
		}

		Common::Com_Printf( "...using secondary sound buffer\n" );
	}
	else
	{
		Common::Com_Printf( "...using primary buffer\n" );

		Common::Com_DPrintf( "...setting WRITEPRIMARY coop level: " );
		if( DS_OK != pDS->SetCooperativeLevel( cl_hwnd, DSSCL_WRITEPRIMARY ) ) {

			Common::Com_Printf( "failed\n" );
			FreeSound( );
			return false;
		}
		Common::Com_DPrintf( "ok\n" );

		if( DS_OK != pDSPBuf->GetCaps( &dsbcaps ) ) {

			Common::Com_Printf( "***GetCaps failed***\n" );
			return false;
		}

		pDSBuf = pDSPBuf;
	}

	// Make sure mixer is active
	pDSBuf->Play( 0, 0, DSBPLAY_LOOPING );

	if( snd_firsttime )
		Common::Com_Printf( "   %d channel( s )\n"
		               "   %d bits/sample\n"
					   "   %d bytes/sec\n", dma.channels, dma.samplebits, dma.speed );
	
	gSndBufSize = dsbcaps.dwBufferBytes;

	/* we don't want anyone to access the buffer directly w/o locking it first. */
	lpData = NULL; 

	pDSBuf->Stop( );
	pDSBuf->GetCurrentPosition( &mmstarttime.u.sample, &dwWrite );
	pDSBuf->Play( 0, 0, DSBPLAY_LOOPING );

	dma.samples = gSndBufSize/( dma.samplebits/8 );
	dma.samplepos = 0;
	dma.submission_chunk = 1;
	dma.buffer =( unsigned char * ) lpData;
	sample16 =( dma.samplebits/8 ) - 1;

	return true;
#endif
}

/*
* * DS_DestroyBuffers
*/
void Sound::DS_DestroyBuffers( ) {

#if 0
	Common::Com_DPrintf( "Destroying DS buffers\n" );
	if( pDS )
	{
		Common::Com_DPrintf( "...setting NORMAL coop level\n" );
		pDS->SetCooperativeLevel( cl_hwnd, DSSCL_NORMAL );
	}

	if( pDSBuf )
	{
		Common::Com_DPrintf( "...stopping and releasing sound buffer\n" );
		pDSBuf->Stop( );
		pDSBuf->Release( );
	}

	// only release primary buffer if it's not also the mixing buffer we just released
	if( pDSPBuf &&( pDSBuf != pDSPBuf ) )
	{
		Common::Com_DPrintf( "...releasing primary buffer\n" );
		pDSPBuf->Release( );
	}
	pDSBuf = NULL;
	pDSPBuf = NULL;

	dma.buffer = NULL;
#endif
}

/*
==================
FreeSound
==================
*/
void Sound::FreeSound( ) {

#if 0
	int		i;

	Common::Com_DPrintf( "Shutting down sound system\n" );

	if( pDS )
		DS_DestroyBuffers( );

	if( hWaveOut )
	{
		Common::Com_DPrintf( "...resetting waveOut\n" );
		WindowsNS::waveOutReset( hWaveOut );

		if( lpWaveHdr )
		{
			Common::Com_DPrintf( "...unpreparing headers\n" );
			for( i = 0; i< WAV_BUFFERS; i++ )
				waveOutUnprepareHeader( hWaveOut, lpWaveHdr+i, sizeof( WindowsNS::WAVEHDR ) );
		}

		Common::Com_DPrintf( "...closing waveOut\n" );
		WindowsNS::waveOutClose( hWaveOut );

		if( hWaveHdr )
		{
			Common::Com_DPrintf( "...freeing WAV header\n" );
			WindowsNS::GlobalUnlock( hWaveHdr );
			WindowsNS::GlobalFree( hWaveHdr );
		}

		if( hData )
		{
			Common::Com_DPrintf( "...freeing WAV buffer\n" );
			WindowsNS::GlobalUnlock( hData );
			WindowsNS::GlobalFree( hData );
		}

	}

	if( pDS )
	{
		Common::Com_DPrintf( "...releasing DS object\n" );
		pDS->Release( );
	}

	if( hInstDS )
	{
		Common::Com_DPrintf( "...freeing DSOUND.DLL\n" );
		WindowsNS::FreeLibrary( hInstDS );
		hInstDS = NULL;
	}

	pDS = NULL;
	pDSBuf = NULL;
	pDSPBuf = NULL;
	hWaveOut = 0;
	hData = 0;
	hWaveHdr = 0;
	lpData = NULL;
	lpWaveHdr = NULL;
	dsound_init = false;
	wav_init = false;
#endif
}

/*
==================
SNDDMA_InitDirect

Direct-Sound support
==================
*/
bool Sound::SNDDMA_InitDirect( ) {

	return false;

#if 0
	WindowsNS::DSCAPS			dscaps;
	HRESULT			hresult;

	dma.channels = 2;
	dma.samplebits = 16;

	if		( s_khz.GetInt( ) == 11 )	dma.speed = 11025;
	else if	( s_khz.GetInt( ) == 22 )	dma.speed = 22050;
	else								dma.speed = 44100;

	Common::Com_Printf( "Initializing DirectSound\n" );

	if( !hInstDS )
	{
		Common::Com_DPrintf( "...loading dsound.dll: " );

		hInstDS = WindowsNS::LoadLibrary( "dsound.dll" );
		
		if( hInstDS == NULL )
		{
			Common::Com_Printf( "failed\n" );
			return false;
		}

		Common::Com_DPrintf( "ok\n" );
		pDirectSoundCreate =( PFNDIRECTSOUNDCREATEPROC )GetProcAddress( hInstDS, "DirectSoundCreate" );

		if( !pDirectSoundCreate ) {
			Common::Com_Printf( "***couldn't get DS proc addr*** \n" );
			return false;
		}
	}

	Common::Com_DPrintf( "...creating DS object: " );
	while( ( hresult = iDirectSoundCreate( NULL, &pDS, NULL ) ) != DS_OK )
	{
		if( hresult != DSERR_ALLOCATED )
		{
			Common::Com_Printf( "failed\n" );
			return false;
		}

		if( WindowsNS::MessageBox( NULL, "The sound hardware is in use by another app.\n\n"
					    "Select Retry to try to start sound again or Cancel to run Quake with no sound.", "Sound not available", MB_RETRYCANCEL | MB_SETFOREGROUND | MB_ICONEXCLAMATION ) != IDRETRY )
		{
			Common::Com_Printf( "failed, hardware already in use\n" );
			return false;
		}
	}
	Common::Com_DPrintf( "ok\n" );

	dscaps.dwSize = sizeof( dscaps );

	if( DS_OK != pDS->GetCaps( &dscaps ) )
	{
		Common::Com_Printf( "***couldn't get DS caps*** \n" );
	}

	if( dscaps.dwFlags & DSCAPS_EMULDRIVER )
	{
		Common::Com_DPrintf( "...no DSound driver found\n" );
		FreeSound( );
		return false;
	}

	if( !DS_CreateBuffers( ) )
		return false;

	dsound_init = true;

	Common::Com_DPrintf( "...completed successfully\n" );

	return true;
#endif
}


/*
==================
SNDDM_InitWav

Crappy windows multimedia base
==================
*/
bool Sound::SNDDMA_InitWav( ) {

	return false;
#if 0
	WindowsNS::WAVEFORMATEX  format; 
	int				i;
	HRESULT			hr;

	Common::Com_Printf( "Initializing wave sound\n" );
	
	snd_sent = 0;
	snd_completed = 0;

	dma.channels = 2;
	dma.samplebits = 16;

	if		( s_khz.GetInt( ) == 11 )	dma.speed = 11025;
	else if	( s_khz.GetInt( ) == 22 )	dma.speed = 22050;
	else								dma.speed = 44100;

	Common::Com_Memset( &format, 0, sizeof( format ) );
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = dma.channels;
	format.wBitsPerSample = dma.samplebits;
	format.nSamplesPerSec = dma.speed;
	format.nBlockAlign = format.nChannels
		* format.wBitsPerSample / 8;
	format.cbSize = 0;
	format.nAvgBytesPerSec = format.nSamplesPerSec
		* format.nBlockAlign; 
	
	/* Open a waveform device for output using window callback. */ 
	Common::Com_DPrintf( "...opening waveform device: " );
	while( ( hr = WindowsNS::waveOutOpen( ( WindowsNS::LPHWAVEOUT )&hWaveOut, WAVE_MAPPER, &format, 0, 0L, CALLBACK_NULL ) ) != MMSYSERR_NOERROR )
	{
		if( hr != MMSYSERR_ALLOCATED )
		{
			Common::Com_Printf( "failed\n" );
			return false;
		}

		if( WindowsNS::MessageBox( NULL, "The sound hardware is in use by another app.\n\n"
					    "Select Retry to try to start sound again or Cancel to run Quake 2 with no sound.", "Sound not available", MB_RETRYCANCEL | MB_SETFOREGROUND | MB_ICONEXCLAMATION ) != IDRETRY )
		{
			Common::Com_Printf( "hw in use\n" );
			return false;
		}
	} 
	Common::Com_DPrintf( "ok\n" );

	/* 
	 * Allocate and lock memory for the waveform data. The memory 
	 * for waveform data must be globally allocated with 
	 * GMEM_MOVEABLE and GMEM_SHARE flags. 

	*/ 
	Common::Com_DPrintf( "...allocating waveform buffer: " );
	gSndBufSize = WAV_BUFFERS * WAV_BUFFER_SIZE;
	hData = WindowsNS::GlobalAlloc( GMEM_MOVEABLE | GMEM_SHARE, gSndBufSize ); 
	if( !hData ) {

		Common::Com_Printf( " failed\n" );
		FreeSound( );
		return false; 
	}
	Common::Com_DPrintf( "ok\n" );

	Common::Com_DPrintf( "...locking waveform buffer: " );
	lpData =( WindowsNS::HPSTR )WindowsNS::GlobalLock( hData );
	if( !lpData ) { 

		Common::Com_Printf( " failed\n" );
		FreeSound( );
		return false; 
	} 
	Common::Com_Memset( lpData, 0, gSndBufSize );
	Common::Com_DPrintf( "ok\n" );

	/* 
	 * Allocate and lock memory for the header. This memory must 
	 * also be globally allocated with GMEM_MOVEABLE and 
	 * GMEM_SHARE flags. 
	 */ 
	Common::Com_DPrintf( "...allocating waveform header: " );
	hWaveHdr = WindowsNS::GlobalAlloc( GMEM_MOVEABLE | GMEM_SHARE, ( DWORD ) sizeof( WindowsNS::WAVEHDR ) * WAV_BUFFERS ); 

	if( hWaveHdr == NULL )
	{ 
		Common::Com_Printf( "failed\n" );
		FreeSound( );
		return false; 
	} 
	Common::Com_DPrintf( "ok\n" );

	Common::Com_DPrintf( "...locking waveform header: " );
	lpWaveHdr =( WindowsNS::LPWAVEHDR ) WindowsNS::GlobalLock( hWaveHdr ); 

	if( lpWaveHdr == NULL )
	{ 
		Common::Com_Printf( "failed\n" );
		FreeSound( );
		return false; 
	}
	Common::Com_Memset( lpWaveHdr, 0, sizeof( WindowsNS::WAVEHDR ) * WAV_BUFFERS );
	Common::Com_DPrintf( "ok\n" );

	/* After allocation, set up and prepare headers. */ 
	Common::Com_DPrintf( "...preparing headers: " );
	for( i = 0; i<WAV_BUFFERS; i++ )
	{
		lpWaveHdr[ i ].dwBufferLength = WAV_BUFFER_SIZE; 
		lpWaveHdr[ i ].lpData = lpData + i* WAV_BUFFER_SIZE;

		if( waveOutPrepareHeader( hWaveOut, lpWaveHdr+i, sizeof( WindowsNS::WAVEHDR ) ) !=
				MMSYSERR_NOERROR )
		{
			Common::Com_Printf( "failed\n" );
			FreeSound( );
			return false;
		}
	}
	Common::Com_DPrintf( "ok\n" );

	dma.samples = gSndBufSize/( dma.samplebits/8 );
	dma.samplepos = 0;
	dma.submission_chunk = 512;
	dma.buffer =( unsigned char * ) lpData;
	sample16 =( dma.samplebits/8 ) - 1;

	wav_init = true;

	return true;
#endif
}

/*
==================
SNDDMA_Init

Try to find a sound device to mix for.
Returns false if nothing is found.
==================
*/
bool Sound::SNDDMA_Init( ) {

	return false;
#if 0
	Common::Com_Memset( &dma, 0, sizeof( dma ) );

	dsound_init = wav_init = 0;

	bool	stat = false;	// assume DirectSound won't initialize

	/* Init DirectSound */
	if( !s_wavonly.GetBool( ) ) {

		if( snd_firsttime || snd_isdirect ) {

			stat = SNDDMA_InitDirect( );

			if( stat ) {

				snd_isdirect = true;
				if( snd_firsttime ) Common::Com_Printf( "dsound init succeeded\n" );
			}
			else {

				snd_isdirect = false;
				Common::Com_Printf( "***dsound init failed***\n" );
			}
		}
	}

// if DirectSound didn't succeed in initializing, try to initialize
// waveOut sound, unless DirectSound failed because the hardware is
// already allocated( in which case the user has already chosen not
// to have sound )
	if( !dsound_init && stat ) {

		if( snd_firsttime || snd_iswave ) {

			snd_iswave = SNDDMA_InitWav( );

			if( snd_iswave ) {

				if( snd_firsttime ) Common::Com_Printf( "Wave sound init succeeded\n" );
			}
			else Common::Com_Printf( "Wave sound init failed\n" );
		}
	}

	snd_firsttime = false;

	snd_buffer_count = 1;

	if( !dsound_init && !wav_init ) {

		if( snd_firsttime ) Common::Com_Printf( "***No sound device initialized*** \n" );
		return false;
	}

	return true;
#endif
}

/*
==============
SNDDMA_GetDMAPos

return the current sample position( in mono samples read )
inside the recirculating dma buffer, so the mixing code will know
how many sample are required to fill it up.
===============
*/
int Sound::SNDDMA_GetDMAPos( ) {

#if 0
	WindowsNS::MMTIME	mmtime;
	int		s = 0;
	DWORD	dwWrite;

	if( dsound_init )  {
		mmtime.wType = TIME_SAMPLES;
		pDSBuf->GetCurrentPosition( &mmtime.u.sample, &dwWrite );
		s = mmtime.u.sample - mmstarttime.u.sample;
	}
	else if( wav_init )
	{
		s = snd_sent * WAV_BUFFER_SIZE;
	}


	s >>= sample16;

	s &=( dma.samples-1 );

	return s;
#endif

	return 0;
}

/*
==============
SNDDMA_BeginPainting

Makes sure dma.buffer is valid
===============
*/
DWORD	locksize;
void Sound::SNDDMA_BeginPainting( ) {

#if 0
	int		reps;
	DWORD	dwSize2;
	DWORD	* pbuf, * pbuf2;
	HRESULT	hresult;
	DWORD	dwStatus;

	if( !pDSBuf )
		return;

	// if the buffer was lost or stopped, restore it and/or restart it
	if( pDSBuf->GetStatus( &dwStatus ) != DS_OK )
		Common::Com_Printf( "Couldn't get sound buffer status\n" );
	
	if( dwStatus & DSBSTATUS_BUFFERLOST )
		pDSBuf->Restore( );
	
	if( !( dwStatus & DSBSTATUS_PLAYING ) ) pDSBuf->Play( 0, 0, DSBPLAY_LOOPING );

	// lock the dsound buffer

	reps = 0;
	dma.buffer = NULL;

	while( ( hresult = pDSBuf->Lock( 0, gSndBufSize, ( WindowsNS::LPVOID * )&pbuf, &locksize, ( WindowsNS::LPVOID * )&pbuf2, &dwSize2, 0 ) ) != DS_OK )
	{
		if( hresult != DSERR_BUFFERLOST )
		{
			Common::Com_Printf( "S_TransferStereo16: Lock failed with error '%s'\n", DSoundError( hresult ).c_str( ) );
			S_Shutdown( );
			return;
		}
		else
		{
			pDSBuf->Restore( );
		}

		if( ++reps > 2 )
			return;
	}
	dma.buffer =( unsigned char * )pbuf;
#endif
}

/*
==============
SNDDMA_Submit

Send sound to device if buffer isn't really the dma buffer
Also unlocks the dsound buffer
===============
*/
void Sound::SNDDMA_Submit( ) {

#if 0
	WindowsNS::LPWAVEHDR	h;
	int			wResult;

	if( !dma.buffer ) return;

	// unlock the dsound buffer
	if( pDSBuf ) pDSBuf->Unlock( dma.buffer, locksize, NULL, 0 );

	if( !wav_init ) return;

	//
	// find which sound blocks have completed
	//
	while( true ) {

		if( snd_completed == snd_sent ) {

			Common::Com_DPrintf( "Sound overrun\n" );
			break;
		}

		if( !( lpWaveHdr[ snd_completed & WAV_MASK].dwFlags & WHDR_DONE ) ) {

			break;
		}

		snd_completed++;	// this buffer has been played
	}

//Common::Com_Printf( "completed %i\n", snd_completed );
	//
	// submit a few new sound blocks
	//
	while( ( ( snd_sent - snd_completed ) >> sample16 ) < 8 )
	{
		h = lpWaveHdr +( snd_sent&WAV_MASK );
	if( paintedtime/256 <= snd_sent )
		break;	//	Common::Com_Printf( "submit overrun\n" );
//Common::Com_Printf( "send %i\n", snd_sent );
		snd_sent++;
		/* 
		 * Now the data block can be sent to the output device. The 
		 * waveOutWrite function returns immediately and waveform 
		 * data is sent to the output device in the background. 
		 */ 
		wResult = waveOutWrite( hWaveOut, h, sizeof( WindowsNS::WAVEHDR ) ); 

		if( wResult != MMSYSERR_NOERROR )
		{ 
			Common::Com_Printf( "Failed to write block to device\n" );
			FreeSound( );
			return; 
		} 
	}
#endif
}

/*
==============
SNDDMA_Shutdown

Reset the sound device for exiting
===============
*/
void Sound::SNDDMA_Shutdown( ) {

	FreeSound( );
}


/*
===========
S_Activate

Called when the main window gains or loses focus.
The window have been destroyed and recreated
between a deactivate and an activate.
===========
*/
void Sound::S_Activate( bool active ) {

#if 0
	if( active )
	{
		if( pDS && cl_hwnd && snd_isdirect )
		{
			DS_CreateBuffers( );
		}
	}
	else
	{
		if( pDS && cl_hwnd && snd_isdirect )
		{
			DS_DestroyBuffers( );
		}
	}
#endif
}

