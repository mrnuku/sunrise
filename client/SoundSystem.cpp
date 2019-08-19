#include "precompiled.h"
#pragma hdrstop

DynamicModule *				ALImports::alModule;

LPALENABLE					ALImports::alEnable;
LPALDISABLE					ALImports::alDisable;
LPALISENABLED				ALImports::alIsEnabled;
LPALGETBOOLEAN				ALImports::alGetBoolean;
LPALGETINTEGER				ALImports::alGetInteger;
LPALGETFLOAT				ALImports::alGetFloat;
LPALGETDOUBLE				ALImports::alGetDouble;
LPALGETBOOLEANV				ALImports::alGetBooleanv;
LPALGETINTEGERV				ALImports::alGetIntegerv;
LPALGETFLOATV				ALImports::alGetFloatv;
LPALGETDOUBLEV				ALImports::alGetDoublev;
LPALGETSTRING				ALImports::alGetString;
LPALGETERROR				ALImports::alGetError;
LPALISEXTENSIONPRESENT		ALImports::alIsExtensionPresent;
LPALGETPROCADDRESS			ALImports::alGetProcAddress;
LPALGETENUMVALUE			ALImports::alGetEnumValue;
LPALLISTENERI				ALImports::alListeneri;
LPALLISTENERF				ALImports::alListenerf;
LPALLISTENER3F				ALImports::alListener3f;
LPALLISTENERFV				ALImports::alListenerfv;
LPALGETLISTENERI			ALImports::alGetListeneri;
LPALGETLISTENERF			ALImports::alGetListenerf;
LPALGETLISTENER3F			ALImports::alGetListener3f;
LPALGETLISTENERFV			ALImports::alGetListenerfv;
LPALGENSOURCES				ALImports::alGenSources;
LPALDELETESOURCES			ALImports::alDeleteSources;
LPALISSOURCE				ALImports::alIsSource;
LPALSOURCEI					ALImports::alSourcei;
LPALSOURCEF					ALImports::alSourcef;
LPALSOURCE3F				ALImports::alSource3f;
LPALSOURCEFV				ALImports::alSourcefv;
LPALGETSOURCEI				ALImports::alGetSourcei;
LPALGETSOURCEF				ALImports::alGetSourcef;
LPALGETSOURCEFV				ALImports::alGetSourcefv;
LPALSOURCEPLAYV				ALImports::alSourcePlayv;
LPALSOURCESTOPV				ALImports::alSourceStopv;
LPALSOURCEPLAY				ALImports::alSourcePlay;
LPALSOURCEPAUSE				ALImports::alSourcePause;
LPALSOURCESTOP				ALImports::alSourceStop;
LPALGENBUFFERS				ALImports::alGenBuffers;
LPALDELETEBUFFERS			ALImports::alDeleteBuffers;
LPALISBUFFER				ALImports::alIsBuffer;
LPALBUFFERDATA				ALImports::alBufferData;
LPALGETBUFFERI				ALImports::alGetBufferi;
LPALGETBUFFERF				ALImports::alGetBufferf;
LPALSOURCEQUEUEBUFFERS		ALImports::alSourceQueueBuffers;
LPALSOURCEUNQUEUEBUFFERS	ALImports::alSourceUnqueueBuffers;
LPALDISTANCEMODEL			ALImports::alDistanceModel;
LPALDOPPLERFACTOR			ALImports::alDopplerFactor;
LPALDOPPLERVELOCITY			ALImports::alDopplerVelocity;
LPALCGETSTRING				ALImports::alcGetString;
LPALCGETINTEGERV			ALImports::alcGetIntegerv;
LPALCOPENDEVICE				ALImports::alcOpenDevice;
LPALCCLOSEDEVICE			ALImports::alcCloseDevice;
LPALCCREATECONTEXT			ALImports::alcCreateContext;
LPALCMAKECONTEXTCURRENT		ALImports::alcMakeContextCurrent;
LPALCPROCESSCONTEXT			ALImports::alcProcessContext;
LPALCGETCURRENTCONTEXT		ALImports::alcGetCurrentContext;
LPALCGETCONTEXTSDEVICE		ALImports::alcGetContextsDevice;
LPALCSUSPENDCONTEXT			ALImports::alcSuspendContext;
LPALCDESTROYCONTEXT			ALImports::alcDestroyContext;
LPALCGETERROR				ALImports::alcGetError;
LPALCISEXTENSIONPRESENT		ALImports::alcIsExtensionPresent;
LPALCGETPROCADDRESS			ALImports::alcGetProcAddress;
LPALCGETENUMVALUE			ALImports::alcGetEnumValue;
LPALCCAPTUREOPENDEVICE		ALImports::alcCaptureOpenDevice;
LPALCCAPTURECLOSEDEVICE		ALImports::alcCaptureCloseDevice;
LPALCCAPTURESTART			ALImports::alcCaptureStart;
LPALCCAPTURESTOP			ALImports::alcCaptureStop;
LPALCCAPTURESAMPLES			ALImports::alcCaptureSamples;

void ALImports::Init( ) {

	alModule					= new DynamicModule( "openal32.dll" );

	alEnable					= ( LPALENABLE )				alModule->FindProc( "alEnable" );
	alDisable					= ( LPALDISABLE )				alModule->FindProc( "alDisable" );
	alIsEnabled					= ( LPALISENABLED )				alModule->FindProc( "alIsEnabled" );
	alGetBoolean				= ( LPALGETBOOLEAN )			alModule->FindProc( "alGetBoolean" );
	alGetInteger				= ( LPALGETINTEGER )			alModule->FindProc( "alGetInteger" );
	alGetFloat					= ( LPALGETFLOAT )				alModule->FindProc( "alGetFloat" );
	alGetDouble					= ( LPALGETDOUBLE )				alModule->FindProc( "alGetDouble" );
	alGetBooleanv				= ( LPALGETBOOLEANV )			alModule->FindProc( "alGetBooleanv" );
	alGetIntegerv				= ( LPALGETINTEGERV )			alModule->FindProc( "alGetIntegerv" );
	alGetFloatv					= ( LPALGETFLOATV )				alModule->FindProc( "alGetFloatv" );
	alGetDoublev				= ( LPALGETDOUBLEV )			alModule->FindProc( "alGetDoublev" );
	alGetString					= ( LPALGETSTRING )				alModule->FindProc( "alGetString" );
	alGetError					= ( LPALGETERROR )				alModule->FindProc( "alGetError" );
	alIsExtensionPresent		= ( LPALISEXTENSIONPRESENT )	alModule->FindProc( "alIsExtensionPresent" );
	alGetProcAddress			= ( LPALGETPROCADDRESS )		alModule->FindProc( "alGetProcAddress" );
	alGetEnumValue				= ( LPALGETENUMVALUE )			alModule->FindProc( "alGetEnumValue" );
	alListeneri					= ( LPALLISTENERI )				alModule->FindProc( "alListeneri" );
	alListenerf					= ( LPALLISTENERF )				alModule->FindProc( "alListenerf" );
	alListener3f				= ( LPALLISTENER3F )			alModule->FindProc( "alListener3f" );
	alListenerfv				= ( LPALLISTENERFV )			alModule->FindProc( "alListenerfv" );
	alGetListeneri				= ( LPALGETLISTENERI )			alModule->FindProc( "alGetListeneri" );
	alGetListenerf				= ( LPALGETLISTENERF )			alModule->FindProc( "alGetListenerf" );
	alGetListener3f				= ( LPALGETLISTENER3F )			alModule->FindProc( "alGetListener3f" );
	alGetListenerfv				= ( LPALGETLISTENERFV )			alModule->FindProc( "alGetListenerfv" );
	alGenSources				= ( LPALGENSOURCES )			alModule->FindProc( "alGenSources" );
	alDeleteSources				= ( LPALDELETESOURCES )			alModule->FindProc( "alDeleteSources" );
	alIsSource					= ( LPALISSOURCE )				alModule->FindProc( "alIsSource" );
	alSourcei					= ( LPALSOURCEI )				alModule->FindProc( "alSourcei" );
	alSourcef					= ( LPALSOURCEF )				alModule->FindProc( "alSourcef" );
	alSource3f					= ( LPALSOURCE3F )				alModule->FindProc( "alSource3f" );
	alSourcefv					= ( LPALSOURCEFV )				alModule->FindProc( "alSourcefv" );
	alGetSourcei				= ( LPALGETSOURCEI )			alModule->FindProc( "alGetSourcei" );
	alGetSourcef				= ( LPALGETSOURCEF )			alModule->FindProc( "alGetSourcef" );
	alGetSourcefv				= ( LPALGETSOURCEFV )			alModule->FindProc( "alGetSourcefv" );
	alSourcePlayv				= ( LPALSOURCEPLAYV )			alModule->FindProc( "alSourcePlayv" );
	alSourceStopv				= ( LPALSOURCESTOPV )			alModule->FindProc( "alSourceStopv" );
	alSourcePlay				= ( LPALSOURCEPLAY )			alModule->FindProc( "alSourcePlay" );
	alSourcePause				= ( LPALSOURCEPAUSE )			alModule->FindProc( "alSourcePause" );
	alSourceStop				= ( LPALSOURCESTOP )			alModule->FindProc( "alSourceStop" );
	alGenBuffers				= ( LPALGENBUFFERS )			alModule->FindProc( "alGenBuffers" );
	alDeleteBuffers				= ( LPALDELETEBUFFERS )			alModule->FindProc( "alDeleteBuffers" );
	alIsBuffer					= ( LPALISBUFFER )				alModule->FindProc( "alIsBuffer" );
	alBufferData				= ( LPALBUFFERDATA )			alModule->FindProc( "alBufferData" );
	alGetBufferi				= ( LPALGETBUFFERI )			alModule->FindProc( "alGetBufferi" );
	alGetBufferf				= ( LPALGETBUFFERF )			alModule->FindProc( "alGetBufferf" );
	alSourceQueueBuffers		= ( LPALSOURCEQUEUEBUFFERS )	alModule->FindProc( "alSourceQueueBuffers" );
	alSourceUnqueueBuffers		= ( LPALSOURCEUNQUEUEBUFFERS )	alModule->FindProc( "alSourceUnqueueBuffers" );
	alDistanceModel				= ( LPALDISTANCEMODEL )			alModule->FindProc( "alDistanceModel" );
	alDopplerFactor				= ( LPALDOPPLERFACTOR )			alModule->FindProc( "alDopplerFactor" );
	alDopplerVelocity			= ( LPALDOPPLERVELOCITY )		alModule->FindProc( "alDopplerVelocity" );
	alcGetString				= ( LPALCGETSTRING )			alModule->FindProc( "alcGetString" );
	alcGetIntegerv				= ( LPALCGETINTEGERV )			alModule->FindProc( "alcGetIntegerv" );
	alcOpenDevice				= ( LPALCOPENDEVICE )			alModule->FindProc( "alcOpenDevice" );
	alcCloseDevice				= ( LPALCCLOSEDEVICE )			alModule->FindProc( "alcCloseDevice" );
	alcCreateContext			= ( LPALCCREATECONTEXT )		alModule->FindProc( "alcCreateContext" );
	alcMakeContextCurrent		= ( LPALCMAKECONTEXTCURRENT )	alModule->FindProc( "alcMakeContextCurrent" );
	alcProcessContext			= ( LPALCPROCESSCONTEXT )		alModule->FindProc( "alcProcessContext" );
	alcGetCurrentContext		= ( LPALCGETCURRENTCONTEXT )	alModule->FindProc( "alcGetCurrentContext" );
	alcGetContextsDevice		= ( LPALCGETCONTEXTSDEVICE )	alModule->FindProc( "alcGetContextsDevice" );
	alcSuspendContext			= ( LPALCSUSPENDCONTEXT )		alModule->FindProc( "alcSuspendContext" );
	alcDestroyContext			= ( LPALCDESTROYCONTEXT )		alModule->FindProc( "alcDestroyContext" );
	alcGetError					= ( LPALCGETERROR )				alModule->FindProc( "alcGetError" );
	alcIsExtensionPresent		= ( LPALCISEXTENSIONPRESENT )	alModule->FindProc( "alcIsExtensionPresent" );
	alcGetProcAddress			= ( LPALCGETPROCADDRESS )		alModule->FindProc( "alcGetProcAddress" );
	alcGetEnumValue				= ( LPALCGETENUMVALUE )			alModule->FindProc( "alcGetEnumValue" );
	alcCaptureOpenDevice		= ( LPALCCAPTUREOPENDEVICE )	alModule->FindProc( "alcCaptureOpenDevice" );
	alcCaptureCloseDevice		= ( LPALCCAPTURECLOSEDEVICE )	alModule->FindProc( "alcCaptureCloseDevice" );
	alcCaptureStart				= ( LPALCCAPTURESTART )			alModule->FindProc( "alcCaptureStart" );
	alcCaptureStop				= ( LPALCCAPTURESTOP )			alModule->FindProc( "alcCaptureStop" );
	alcCaptureSamples			= ( LPALCCAPTURESAMPLES )		alModule->FindProc( "alcCaptureSamples" );
}

void ALImports::Shutdown( ) {

	delete alModule;
}

bool								SoundSystem::isValid = false;
ALCdevice *							SoundSystem::alcDevice = NULL;
ALCcontext *						SoundSystem::alcContext = NULL;
ALCdevice *							SoundSystem::alcCaptureDevice = NULL;
bool								SoundSystem::isCaptureEnabled = false;
int									SoundSystem::captureBufferSize = 65536;
void *								SoundSystem::captureBuffer = NULL;
//SoundBuffer *						SoundSystem::captureTestBuffer[ 2 ];
//SoundSource *						SoundSystem::captureTestSource[ 2 ];
CANamedList< SoundBuffer *, 512 >	SoundSystem::g_bufferList;
CAList< SoundSource *, 256 >		SoundSystem::g_sourceList;
Blip_Buffer							SoundSystem::blipBuffer;
Blip_Synth< blip_good_quality, 40 >	SoundSystem::blipSynth;
SoundBuffer *						SoundSystem::blipTestBuffer;
SoundSource *						SoundSystem::blipTestSource;
Vec3								SoundSystem::listenerLastOrigin = vec3_origin;

WAV_Reader::WAV_Reader( const void * iff_ptr, size_t iff_size ) {
	d_iffData = ( byte * )iff_ptr;
	d_iffEnd = d_iffData + iff_size;
}

short WAV_Reader::GetLittleShort( ) {

	short val = 0;
	val = *d_dataPtr;
	val = val + ( *( d_dataPtr + 1 ) << 8 );
	d_dataPtr += 2;
	return val;
}

int WAV_Reader::GetLittleLong( ) {

	int val = 0;
	val = * d_dataPtr;
	val = val + ( *( d_dataPtr + 1 ) << 8 );
	val = val + ( *( d_dataPtr + 2 ) << 16 );
	val = val + ( *( d_dataPtr + 3 ) << 24 );
	d_dataPtr += 4;
	return val;
}

bool WAV_Reader::FindNextChunk( const Str & chunk_name ) {

	while( 1 ) {
		d_dataPtr = d_lastChunk;
		if( d_dataPtr >= d_iffEnd ) {
			d_dataPtr = NULL;
			return false;
		}		
		d_dataPtr += 4;
		d_iffChunkLen = GetLittleLong( );
		if( d_iffChunkLen < 0 ) {
			d_dataPtr = NULL;
			return false;
		}
		d_dataPtr -= 8;
		d_lastChunk = d_dataPtr + 8 + ( ( d_iffChunkLen + 1 ) & ~1 );
		if( !chunk_name.Cmpn( ( char * )d_dataPtr, chunk_name.Length( ) ) )
			return true;
	}
	return false;
}

bool WAV_Reader::FindChunk( const Str & chunk_name ) {

	d_lastChunk = d_iffData;
	return FindNextChunk( chunk_name );
}

SoundBuffer::SoundBuffer( ) {
	ALImports::alGenBuffers( 1, &d_bufferID );
}

SoundBuffer::~SoundBuffer( ) {
	ALImports::alDeleteBuffers( 1, &d_bufferID );
}

ALenum SoundBuffer::GetFormat( int channels, int bits ) {
	if( channels == 2 ) {
		if( bits == 16 )
			return AL_FORMAT_STEREO16;
		else if( bits == 8 )
			return AL_FORMAT_STEREO8;
		else
			return AL_FALSE;
	} else if( channels == 1 ) {
		if( bits == 16 )
			return AL_FORMAT_MONO16;
		else if( bits == 8 )
			return AL_FORMAT_MONO8;
		else
			return AL_FALSE;
	}
	return AL_FALSE;
}

bool SoundBuffer::LoadWAV( const void * data_ptr, size_t data_size ) {

	WAV_Reader wavReader( data_ptr, data_size );
	wavReader.FindChunk( "RIFF" );
	if( !wavReader.CompareDataPtr( "WAVE", 8 ) )
		return false;
	wavReader.SetIffPtr( wavReader.GetDataPtr( ) + 12 );	
	if( !wavReader.FindChunk( "fmt " ) )
		return false;
	wavReader.AdvanceDataPtr( 8 );
	if( wavReader.GetLittleShort( ) != 1 )
		return false;
	int channels = wavReader.GetLittleShort( );
	int rate = wavReader.GetLittleLong( );
	wavReader.AdvanceDataPtr( 4 + 2 );
	int bits = wavReader.GetLittleShort( );
	int width = bits / 8;
	ALenum sampleFormat = GetFormat( channels, bits );
	if( sampleFormat == AL_FALSE )
		return false;	
	int loopstart, samples = 0;
	if( wavReader.FindChunk( "cue " ) ) {
		wavReader.AdvanceDataPtr( 32 );
		loopstart = wavReader.GetLittleLong( );
		if( wavReader.FindNextChunk( "LIST" ) ) {
			if( wavReader.CompareDataPtr( "mark", 28 ) ) {
				wavReader.AdvanceDataPtr( 24 );
				samples = loopstart + wavReader.GetLittleLong( );
			}
		}
	}
	else
		loopstart = -1;
	if( !wavReader.FindChunk( "data" ) )
		return false;
	wavReader.AdvanceDataPtr( 4 );
	int samples2 = wavReader.GetLittleLong( ) / width;
	if( samples && samples2 < samples )
		return false;
	else
		samples = samples2;
	return UpLoad( sampleFormat, wavReader.GetDataPtr( ), samples * channels * width, rate );
}

bool SoundBuffer::Load( const Str & file_name ) {

	File_Memory		fm = FileSystem::ReadFile( file_name );
	Str				ext;
	if( !fm.IsValid( ) ) {
		Common::Com_Printf( "Unable to load sound: \"%s\"\n", file_name.c_str( ) );
		return false;
	}
	file_name.ExtractFileExtension( ext );
	d_name = file_name;
	bool succes;
	if( ext.IcmpFast( "wav" ) )
		succes = LoadWAV( fm.GetDataPtr( ), fm.Length( ) );
	FileSystem::FreeFile( fm );
	return succes;
}

SoundSource::SoundSource( ) {
	ALImports::alGenSources( 1, &d_sourceID );
	d_sourceBuffer = NULL;
}

SoundSource::~SoundSource( ) {
	if( d_sourceBuffer )
		UnBind( );
	ALImports::alDeleteSources( 1, &d_sourceID );
}

void SoundSystem::TestSoundSystem( ) {
}

class SoundEditorWindow : public GUIWindow {

private:

	GUIWindow *				d_windowEvents;
	GUIWindow *				d_windowTimeSlider;
	GUIWindow *				d_windowDeltaSlider;
	GUIWindow *				d_windowAddButton;
	GUIWindow *				d_windowRemoveButton;
	GUIWindow *				d_windowClearButton;
	GUIWindow *				d_windowLoopCheckbox;
	GUIWindow *				d_windowTestButton;

	bool					d_loopEnable;
	int						d_keyTime;
	int						d_keyDelta;

	class BlipPairs {

	public:

		int		keyTime;
		int		keyDelta;

				BlipPairs( int time = 0, int delta = 0 ) : keyTime( time ), keyDelta( delta ) { }
	};

	CAList< BlipPairs, 256 >	d_blipPairList;

	static const int								d_blipRange = 40;
	Blip_Buffer										d_blipBuffer;
	Blip_Synth< blip_good_quality, d_blipRange >	d_blipSynth;

	SoundBuffer *									d_blipSoundBuffer;
	SoundSource *									d_blipSoundSource;

	bool					TimeSliderChanged( const CEGUI::EventArgs & args );
	bool					DeltaSliderChanged( const CEGUI::EventArgs & args );
	bool					AddButtonClick( const CEGUI::EventArgs & args );
	bool					RemoveButtonClick( const CEGUI::EventArgs & args );
	bool					ClearButtonClick( const CEGUI::EventArgs & args );
	bool					TestButtonClick( const CEGUI::EventArgs & args );
	bool					LoopCheckboxClick( const CEGUI::EventArgs & args );

public:

							SoundEditorWindow( float rel_x, float rel_y, float rel_w, float rel_h );
};

SoundEditorWindow::SoundEditorWindow( float rel_x, float rel_y, float rel_w, float rel_h ) :
	GUIWindow( "TaharezLook/FrameWindow", "SoundEditor", rel_x, rel_y, rel_w, rel_h ) {

	SetText( "Sound Editor" );
	SetBind( key_f5 );
	Hide( );
	Register( );

	d_windowEvents = new GUIWindow( "TaharezLook/MultiColumnList", "SoundEditor/Events", 0.01f, 0.08f, 0.48f, 0.88f );
	d_windowEvents->SetProperty( "ColumnHeader", "text:Time width:{0.5,0} id:0" );
	d_windowEvents->SetProperty( "ColumnHeader", "text:Delta width:{0.5,0} id:1" );
	d_windowEvents->SetParent( this );

	d_windowTimeSlider = new GUIWindow( "TaharezLook/HorizontalScrollbar", "SoundEditor/TimeSlider", 0.51f, 0.08f, 0.48f, 0.05f );
	d_windowTimeSlider->SetProperty( "DocumentSize", "100" );
	d_windowTimeSlider->SetProperty( "StepSize", "1" );
	d_windowTimeSlider->SetParent( this );
	d_windowTimeSlider->AddEvent< SoundEditorWindow >( CEGUI::Scrollbar::EventScrollPositionChanged, this, &SoundEditorWindow::TimeSliderChanged );
	d_keyTime = 0;

	d_windowDeltaSlider = new GUIWindow( "TaharezLook/HorizontalScrollbar", "SoundEditor/DeltaSlider", 0.51f, 0.19f, 0.48f, 0.05f );
	d_windowDeltaSlider->SetProperty( "DocumentSize", Str( d_blipRange * 2 ).c_str( ) );
	d_windowDeltaSlider->SetProperty( "StepSize", "1" );
	d_windowDeltaSlider->SetParent( this );
	d_windowDeltaSlider->AddEvent< SoundEditorWindow >( CEGUI::Scrollbar::EventScrollPositionChanged, this, &SoundEditorWindow::DeltaSliderChanged );
	d_keyDelta = 0;

	d_windowAddButton = new GUIWindow( "TaharezLook/Button", "SoundEditor/AddButton", 0.72f, 0.29f, 0.2f, 0.1f );
	d_windowAddButton->SetText( "Add" );
	d_windowAddButton->SetParent( this );
	d_windowAddButton->AddEvent< SoundEditorWindow >( CEGUI::ButtonBase::EventMouseClick, this, &SoundEditorWindow::AddButtonClick );

	d_windowRemoveButton = new GUIWindow( "TaharezLook/Button", "SoundEditor/RemoveButton", 0.72f, 0.39f, 0.2f, 0.1f );
	d_windowRemoveButton->SetText( "Remove" );
	d_windowRemoveButton->SetParent( this );
	d_windowRemoveButton->AddEvent< SoundEditorWindow >( CEGUI::ButtonBase::EventMouseClick, this, &SoundEditorWindow::RemoveButtonClick );

	d_windowClearButton = new GUIWindow( "TaharezLook/Button", "SoundEditor/ClearButton", 0.72f, 0.49f, 0.2f, 0.1f );
	d_windowClearButton->SetText( "Clear" );
	d_windowClearButton->SetParent( this );
	d_windowClearButton->AddEvent< SoundEditorWindow >( CEGUI::ButtonBase::EventMouseClick, this, &SoundEditorWindow::ClearButtonClick );

	d_windowLoopCheckbox = new GUIWindow( "TaharezLook/Checkbox", "SoundEditor/LoopCheckbox", 0.51f, 0.86f, 0.2f, 0.05f );
	d_windowLoopCheckbox->SetText( "Loop" );
	d_windowLoopCheckbox->SetParent( this );
	d_windowLoopCheckbox->AddEvent< SoundEditorWindow >( CEGUI::ButtonBase::EventMouseClick, this, &SoundEditorWindow::LoopCheckboxClick );
	d_loopEnable = false;

	d_windowTestButton = new GUIWindow( "TaharezLook/Button", "SoundEditor/TestButton", 0.72f, 0.86f, 0.2f, 0.1f );
	d_windowTestButton->SetText( "Test" );
	d_windowTestButton->SetParent( this );
	d_windowTestButton->AddEvent< SoundEditorWindow >( CEGUI::ButtonBase::EventMouseClick, this, &SoundEditorWindow::TestButtonClick );

	d_blipBuffer.set_sample_rate( 44100, 1000 );
	d_blipBuffer.clock_rate( d_blipBuffer.sample_rate( ) );
	d_blipBuffer.bass_freq( 0 ); // makes waveforms perfectly flat
	d_blipSynth.volume( 0.50 );
	d_blipSynth.output( &d_blipBuffer );

	d_blipSoundBuffer = NULL;
	d_blipSoundSource = NULL;
}

bool SoundEditorWindow::TimeSliderChanged( const CEGUI::EventArgs & args ) {

	const CEGUI::WindowEventArgs & winArgs = dynamic_cast< const CEGUI::WindowEventArgs & >( args );
	d_keyTime = ( int )( ( CEGUI::Scrollbar * )winArgs.window )->getScrollPosition( );

	return true;
}

bool SoundEditorWindow::DeltaSliderChanged( const CEGUI::EventArgs & args ) {

	const CEGUI::WindowEventArgs & winArgs = dynamic_cast< const CEGUI::WindowEventArgs & >( args );
	d_keyDelta = ( int )( ( CEGUI::Scrollbar * )winArgs.window )->getScrollPosition( ) - d_blipRange;

	return true;
}

bool SoundEditorWindow::AddButtonClick( const CEGUI::EventArgs & args ) {

	CEGUI::MultiColumnList * mcl = d_windowEvents->GetHandle< CEGUI::MultiColumnList >( );
	uint rowIndex = mcl->addRow( );

	int keyTime = d_keyTime;
	if( d_blipPairList.Num( ) ) keyTime += d_blipPairList[ d_blipPairList.Num( ) - 1 ].keyTime;

	mcl->setItem( new CEGUI::ListboxTextItem( Str( keyTime ).c_str( ) ), 0, rowIndex );
	mcl->setItem( new CEGUI::ListboxTextItem( Str( d_keyDelta ).c_str( ) ), 1, rowIndex );

	mcl->getVertScrollbar( )->setScrollPosition( mcl->getVertScrollbar( )->getDocumentSize( ) );

	d_blipPairList.Append( BlipPairs( keyTime, d_keyDelta ) );

	return true;
}

bool SoundEditorWindow::RemoveButtonClick( const CEGUI::EventArgs & args ) {

	int rowNum = d_blipPairList.Num( ) - 1;
	if( rowNum < 0 ) return true;
	CEGUI::MultiColumnList * mcl = d_windowEvents->GetHandle< CEGUI::MultiColumnList >( );
	mcl->removeRow( ( uint )rowNum );
	d_blipPairList.RemoveIndex( rowNum );
	return true;
}

bool SoundEditorWindow::ClearButtonClick( const CEGUI::EventArgs & args ) {

	if( !d_blipPairList.Num( ) ) return true;
	CEGUI::MultiColumnList * mcl = d_windowEvents->GetHandle< CEGUI::MultiColumnList >( );
	for( int i = d_blipPairList.Num( ) - 1; i >= 0; i-- ) mcl->removeRow( ( uint )i );
	d_blipPairList.Clear( );
	return true;
}

bool SoundEditorWindow::TestButtonClick( const CEGUI::EventArgs & args ) {

	if( !d_blipSoundSource ) {		
		d_blipSoundBuffer = SoundSystem::AllocBuffer( );
		d_blipSoundSource = SoundSystem::AllocSource( );
		if( !d_blipSoundSource )
			return true;
	} else if( d_blipSoundSource->IsPlaying( ) ) {
		d_blipSoundSource->Stop( );
		return true;
	}
	if( !d_blipPairList.Num( ) )
		return true;
	d_blipBuffer.set_sample_rate( 44100, d_blipPairList[ d_blipPairList.Num( ) - 1 ].keyTime );
	for( int i = 0; i < d_blipPairList.Num( ); i++ )
		d_blipSynth.offset( d_blipPairList[ i ].keyTime, d_blipPairList[ i ].keyDelta );
	d_blipBuffer.end_frame( d_blipPairList[ d_blipPairList.Num( ) - 1 ].keyTime );
	int blipSamplesSize = d_blipBuffer.samples_avail( );
	if( !blipSamplesSize )
		return true;
	CAList< blip_sample_t, 32768 > blipSamplesBuffer;
	blipSamplesBuffer.SetNum( blipSamplesSize );
	Blip_Reader blipReader;
	int bass = blipReader.begin( d_blipBuffer );	
	blip_sample_t * out = blipSamplesBuffer.Ptr( );
	for( long n = blipSamplesSize; n--; ) {
		long s = blipReader.read( );
		blipReader.next( bass );
		if( ( short ) s != s )
			s = 0x7fff - ( s >> 24 );		
		*out++ = ( blip_sample_t )( s + 0x8000 );
	}	
	blipReader.end( d_blipBuffer );
	d_blipBuffer.remove_samples( blipSamplesSize );
	d_blipSoundSource->UnBind( );
	if( !d_blipSoundBuffer->UpLoad( AL_FORMAT_MONO16, blipSamplesBuffer.Ptr( ), ( ALsizei )blipSamplesBuffer.MemoryUsed( ), d_blipBuffer.sample_rate( ) ) ) {
		Common::Com_Printf( "SoundEditorWindow::TestButtonClick( ) : sound sample upload failed\n" );
		return true;
	}
	d_blipSoundSource->Bind( d_blipSoundBuffer );
	d_blipSoundSource->Looping( d_loopEnable );
	d_blipSoundSource->Play( );
	return true;
}

bool SoundEditorWindow::LoopCheckboxClick( const CEGUI::EventArgs & args ) {

	d_loopEnable = !d_loopEnable;
	return true;
}

void SoundSystem::InitMenu( ) {

	SoundEditorWindow * soundEditor = new SoundEditorWindow( 0.01f, 0.31f, 0.48f, 0.48f );
}

void SoundSystem::Init( ) {

	ALImports::Init( );
	alcDevice = ALImports::alcOpenDevice( NULL );
	if( !alcDevice ) {
		Common::Com_Printf( "SoundSystem: alcOpenDevice failed\n" );
		return;
	}
	alcContext = ALImports::alcCreateContext( alcDevice, NULL );
	if( !alcContext ) {
		Common::Com_Printf( "SoundSystem: alcCreateContext failed\n" );
		Shutdown( );
		return;
	}
	ALImports::alcMakeContextCurrent( alcContext );
	if( ALImports::alcGetError( alcDevice ) != ALC_NO_ERROR ) {
		Common::Com_Printf( "SoundSystem: alcMakeContextCurrent failed\n" );
		Shutdown( );
		return;
	}
	const ALchar * captDevices = ALImports::alcGetString( NULL, ALC_CAPTURE_DEVICE_SPECIFIER );
	const ALchar * captDeviceDesired = captDevices;
	for( int i = 0; i < snd_captureid.GetInt( ); i++ ) {
		if( *captDeviceDesired ) {
			while( *captDeviceDesired ) captDeviceDesired++;
			captDeviceDesired++;
		} else
			snd_captureid.SetInt( 0 );
	}
	alcCaptureDevice = ALImports::alcCaptureOpenDevice( captDeviceDesired, 44100, AL_FORMAT_STEREO16, captureBufferSize );
	if( !alcCaptureDevice ) {
		Common::Com_Printf( "SoundSystem: alcCaptureOpenDevice failed\n" );
		Shutdown( );
		return;
	}
	captureBuffer = Mem_Alloc( captureBufferSize * 4 );
	//for( int i = 0; ; i++ ) {
		//if( *captDevices ) {
			//if( i == snd_captureid.GetInt( ) )
				//Common::Com_Printf( "*" );
			//Common::Com_Printf( "\tCapture Device %i: %s\n", i, captDevices );
			//while( *captDevices )
				//captDevices++;
			//captDevices++;

		//} else break;
	//}
	//Common::Com_Printf( "}\n" );
	blipBuffer.set_sample_rate( 44100, 1000 );
	blipBuffer.clock_rate( blipBuffer.sample_rate( ) );
	blipBuffer.bass_freq( 0 );
	blipSynth.volume( 0.50 );
	blipSynth.output( &blipBuffer );
	blipTestBuffer = AllocBuffer( );
	blipTestSource = AllocSource( );
	Command::Cmd_AddCommand( "testss", TestSoundSystem );
	isValid = true;
}

void SoundSystem::Shutdown( ) {

	g_sourceList.DeleteContents( true );
	g_bufferList.DeleteContents( true );
	if( alcContext ) {
		ALImports::alcMakeContextCurrent( NULL );
		ALImports::alcDestroyContext( alcContext );
		alcContext = NULL;
	}
	if( alcDevice ) {
		ALImports::alcCloseDevice( alcDevice );
		alcDevice = NULL;
	}
	if( isCaptureEnabled ) {
		ALImports::alcCaptureStop( alcCaptureDevice );
		isCaptureEnabled = false;
	}
	DisableCapture( );
	if( captureBuffer ) {
		Mem_Free( captureBuffer );
		captureBuffer = NULL;
	}
	ALImports::Shutdown( );
	Command::Cmd_RemoveCommand( "testss" );
	isValid = false;
}

void SoundSystem::BeginRegistration( ) {
}

void SoundSystem::EndRegistration( ) {
}

SoundBuffer * SoundSystem::RegisterSample( const Str & file_name ) {

	SoundBuffer * sBuffer = g_bufferList.FindByName( file_name );
	if( sBuffer )
		return sBuffer;
	sBuffer = new SoundBuffer;
	bool succes = sBuffer->Load( file_name );
	if( succes ) {
		g_bufferList.Append( sBuffer );
		return sBuffer;
	}
	delete sBuffer;
	return NULL;
}

SoundSource * SoundSystem::PlaySampleOnPos( SoundBuffer * sample_buffer, const Vec3 & sample_origin, float volume, float attenuation, float start_ofs ) {
	return NULL;
}

SoundSource * SoundSystem::PlaySampleOnEnt( SoundBuffer * sample_buffer, int sample_entnum, float volume, float attenuation, float start_ofs ) {
	Vec3 soundOrigin = Client::CL_GetEntitySoundOrigin( sample_entnum );
	SoundSource * sSource = sample_buffer->FindUnused( );
	if( !sSource ) {
		sSource = AllocSource( );
		sSource->Bind( sample_buffer );
	}
	sSource->SetParamInt( AL_SOURCE_RELATIVE, AL_TRUE );
	//sSource->SetParamVec( AL_POSITION, soundOrigin );
	sSource->SetParamFloat( AL_GAIN, volume );
	sSource->SetParamFloat( AL_MAX_GAIN, attenuation );
	sSource->SetParamFloat( AL_SEC_OFFSET, start_ofs );
	sSource->Play( );
	return sSource;
}

SoundSource * SoundSystem::PlaySampleOnLocal( SoundBuffer * sample_buffer, float volume, float attenuation, float start_ofs ) {
	return NULL;
}

SoundSource * SoundSystem::PlaySampleOnLocal( const Str & file_name ) {

	SoundBuffer * sBuffer = RegisterSample( file_name );
	if( !sBuffer )
		return NULL;
	SoundSource * sSource = sBuffer->FindUnused( );
	if( !sSource ) {
		sSource = AllocSource( );
		sSource->Bind( sBuffer );
	}
	sSource->Play( );
	return sSource;
}

void SoundSystem::StopAllSounds( ) {
}

void SoundSystem::Activate( ) {
}

void SoundSystem::Deactivate( ) {
}

void SoundSystem::Frame( const Vec3 & listener_origin, const Vec3 & listener_forward, const Vec3 & listener_right, const Vec3 & listener_up ) {

	if( !isValid ) return;

	ALImports::alListenerfv( AL_POSITION, listener_origin.ToFloatPtr( ) );

	Vec3 listenerVelocity = listener_origin - listenerLastOrigin;
	ALImports::alListenerfv( AL_VELOCITY, listenerVelocity.ToFloatPtr( ) );
	listenerLastOrigin = listener_origin;

	float orientation[ 6 ];
	Common::Com_Memcpy( &orientation[ 0 ], listener_forward.ToFloatPtr( ), NumSizeOf( float, listener_forward.GetDimension( ) ) );
	Common::Com_Memcpy( &orientation[ 3 ], listener_up.ToFloatPtr( ), NumSizeOf( float, listener_up.GetDimension( ) ) );
	ALImports::alListenerfv( AL_ORIENTATION, orientation );

	assert( ALImports::alGetError( ) == AL_NO_ERROR );
}

void SoundSystem::EnableCapture( ) {

	if( isCaptureEnabled ) return;
	ALImports::alcCaptureStart( alcCaptureDevice );
	isCaptureEnabled = true;
}

void SoundSystem::DisableCapture( ) {

	if( !isCaptureEnabled ) return;
	ALImports::alcCaptureStop( alcCaptureDevice );
	isCaptureEnabled = false;
}

void * SoundSystem::Capture( int & sample_count ) {

	if( !isCaptureEnabled ) return NULL;
	ALImports::alcGetIntegerv( alcCaptureDevice, ALC_CAPTURE_SAMPLES, 1, &sample_count );
	if( sample_count ) ALImports::alcCaptureSamples( alcCaptureDevice, captureBuffer, sample_count );

	return captureBuffer;
}

SoundBuffer * SoundSystem::AllocBuffer( ) {

	if( !isValid ) return NULL;
	SoundBuffer * soundBuffer = new SoundBuffer( );
	g_bufferList.Append( soundBuffer );
	return soundBuffer;
}

SoundSource * SoundSystem::AllocSource( ) {

	if( !isValid ) return NULL;
	SoundSource * soundSource = new SoundSource( );
	g_sourceList.Append( soundSource );
	return soundSource;
}
