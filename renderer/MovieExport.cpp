#include "precompiled.h"
#pragma hdrstop

DynamicModule *				MovieExport::x264Module;
DynamicModule *				MovieExport::faccModule;

MEDRIVERPROC				MovieExport::DriverProc;

FAACENCGETVERSIONPROC				MovieExport::faacEncGetVersion;
FAACENCGETCURRENTCONFIGURATIONPROC	MovieExport::faacEncGetCurrentConfiguration;
FAACENCSETCONFIGURATIONPROC			MovieExport::faacEncSetConfiguration;
FAACENCOPENPROC						MovieExport::faacEncOpen;
FAACENCGETDECODERSPECIFICINFOPROC	MovieExport::faacEncGetDecoderSpecificInfo;
FAACENCENCODEPROC					MovieExport::faacEncEncode;
FAACENCCLOSEPROC					MovieExport::faacEncClose;

WindowsNS::BITMAPINFO		MovieExport::inputFormat;
WindowsNS::BITMAPINFO		MovieExport::outputFormat;
byte *						MovieExport::readBuffer = NULL;
byte *						MovieExport::inputBuffer = NULL;
byte *						MovieExport::outputBuffer = NULL;
WindowsNS::ICCOMPRESS		MovieExport::compParams;
WindowsNS::DWORD			MovieExport::compChunkId = 0;
WindowsNS::DWORD			MovieExport::compFlags = 0;
bool						MovieExport::isValid = false;
bool						MovieExport::isActive = false;
timeType					MovieExport::firstFrameTime;
timeType					MovieExport::lastFrameTime;
timeType					MovieExport::deltaFrameTime;
int							MovieExport::soundSampleCountRaw = 0;
void *						MovieExport::soundSampleBufferRaw = NULL;
faac::faacEncHandle			MovieExport::faacHandle;
unsigned long				MovieExport::faacInputSamples;
unsigned long				MovieExport::faacMaxOutputBytes;
faac::faacEncConfigurationPtr	MovieExport::faacConfig;
int *						MovieExport::faacInputBuffer = NULL;
unsigned char *				MovieExport::faacOutputBuffer = NULL;
MovieExportThread			MovieExport::compThread;
x264Codec_t *				MovieExport::codecInstance;
FLV_Writer					MovieExport::outputFile;

bool FLV_Writer::SetOutputFile( const Str & file_name ) {

	outputFile = FileSystem::OpenFileByMode( file_name, FS_WRITE );
	return outputFile != NULL;
}

void FLV_Writer::CloseOutputFile( int duration ) {

	if( !this->outputFile )
		return;

	this->outputFile->Write( this->prevTagSize.Ptr( ), sizeof( Int32< BO_BigEndian > ) );

	Real64< BO_BigEndian > paramValue;
	int fileSize = this->outputFile->Tell( );

	paramValue = duration;
	this->outputFile->Seek( this->durationOffset, FS_SEEK_SET );
	this->outputFile->Write( paramValue.Ptr( ), sizeof( Real64< BO_BigEndian > ) );

	paramValue = fileSize;
	this->outputFile->Seek( this->filesizeOffset, FS_SEEK_SET );
	this->outputFile->Write( paramValue.Ptr( ), sizeof( Real64< BO_BigEndian > ) );
	
	FileSystem::CloseFile( outputFile );
}

void FLV_Writer::WriteHeader( bool have_audio, bool have_video ) {

	if( !this->outputFile )
		return;

	FLV_Header header;

	if( have_audio ) header.flags |= FLV_Header::FLV_Audio;
	if( have_video ) header.flags |= FLV_Header::FLV_Video;

	this->outputFile->Write( &header, sizeof( FLV_Header ) );

	//this->audioByte = FLV_Stereo | FLV_16bit | FLV_44khz | FLV_AAC;
	this->audioByte = FLV_Stereo | FLV_16bit | FLV_44khz;
	this->videoByte = FLV_H264;

	FLV_Tag tag;
	tag.type = FLV_Tag::TAG_Meta;
	tag.size = 0;
	tag.timeStamp = 0;

	prevTagSize = 0; // why flv need this
	this->outputFile->Write( this->prevTagSize.Ptr( ), sizeof( Int32< BO_BigEndian > ) );

	int tagOffset = this->outputFile->Tell( );
	this->outputFile->Write( &tag, sizeof( FLV_Tag ) );

	tag.size += this->outputFile->WriteByte( AMF_String );
	tag.size += this->outputFile->WriteAMFString( "onMetaData" );

	Int32< BO_BigEndian > numOfParams = 7;
	tag.size += this->outputFile->WriteByte( AMF_MixedArray );
	tag.size += ( int )this->outputFile->Write( numOfParams.Ptr( ), sizeof( Int32< BO_BigEndian > ) );

	Real64< BO_BigEndian > paramValue;

	paramValue = Video::viddef.width;
	tag.size += this->outputFile->WriteAMFString( "width" );
	tag.size += this->outputFile->WriteByte( AMF_Number );
	tag.size += ( int )this->outputFile->Write( paramValue.Ptr( ), sizeof( Real64< BO_BigEndian > ) );

	paramValue = Video::viddef.height;
	tag.size += this->outputFile->WriteAMFString( "height" );
	tag.size += this->outputFile->WriteByte( AMF_Number );
	tag.size += ( int )this->outputFile->Write( paramValue.Ptr( ), sizeof( Real64< BO_BigEndian > ) );

	paramValue = 25;
	tag.size += this->outputFile->WriteAMFString( "framerate" );
	tag.size += this->outputFile->WriteByte( AMF_Number );
	tag.size += ( int )this->outputFile->Write( paramValue.Ptr( ), sizeof( Real64< BO_BigEndian > ) );

	paramValue = FLV_H264;
	tag.size += this->outputFile->WriteAMFString( "videocodecid" );
	tag.size += this->outputFile->WriteByte( AMF_Number );
	tag.size += ( int )this->outputFile->Write( paramValue.Ptr( ), sizeof( Real64< BO_BigEndian > ) );

	//paramValue = FLV_AAC >> 4;
	paramValue = 0;
	tag.size += this->outputFile->WriteAMFString( "audiocodecid" );
	tag.size += this->outputFile->WriteByte( AMF_Number );
	tag.size += ( int )this->outputFile->Write( paramValue.Ptr( ), sizeof( Real64< BO_BigEndian > ) );

	paramValue = 0;
	tag.size += this->outputFile->WriteAMFString( "duration" );
	tag.size += this->outputFile->WriteByte( AMF_Number );
	this->durationOffset = tagOffset + sizeof( FLV_Tag ) + tag.size.GetInt( );
	tag.size += ( int )this->outputFile->Write( paramValue.Ptr( ), sizeof( Real64< BO_BigEndian > ) );

	paramValue = 0;
	tag.size += this->outputFile->WriteAMFString( "filesize" );
	tag.size += this->outputFile->WriteByte( AMF_Number );
	this->filesizeOffset = tagOffset + sizeof( FLV_Tag ) + tag.size.GetInt( );
	tag.size += ( int )this->outputFile->Write( paramValue.Ptr( ), sizeof( Real64< BO_BigEndian > ) );

	tag.size += this->outputFile->WriteAMFString( "" );
	tag.size += this->outputFile->WriteByte( AMF_ObjectEnd );

	this->outputFile->Seek( tagOffset, FS_SEEK_SET );
	this->outputFile->Write( &tag, sizeof( FLV_Tag ) );
	this->outputFile->Seek( 0, FS_SEEK_END );

	this->prevTagSize = tag.size.GetInt( ) + sizeof( FLV_Tag );
}

void FLV_Writer::WriteVideoFrame( const void * data, int size, int time_stamp, bool is_keyfame ) {

	if( !size || !outputFile ) return;

	this->outputFile->Write( this->prevTagSize.Ptr( ), sizeof( Int32< BO_BigEndian > ) );

	FLV_Tag tag;
	tag.type = FLV_Tag::TAG_Video;
	tag.size = size + 1;
	tag.timeStamp = time_stamp;

	byte frameByte = is_keyfame ? FLV_KeyFrame : FLV_InterFrame;

	this->outputFile->Write( &tag, sizeof( FLV_Tag ) );
	this->outputFile->WriteByte( frameByte );
	this->outputFile->Write( data, size );

	this->prevTagSize = tag.size.GetInt( ) + sizeof( FLV_Tag );
}

void FLV_Writer::WriteAudioFrame( const void * data, int size, int time_stamp ) {

	if( !size || !outputFile->IsValid( ) ) return;

	this->outputFile->Write( this->prevTagSize.Ptr( ), sizeof( Int32< BO_BigEndian > ) );

	FLV_Tag tag;
	tag.type = FLV_Tag::TAG_Audio;
	tag.size = size + 1;
	tag.timeStamp = time_stamp;

	this->outputFile->Write( &tag, sizeof( FLV_Tag ) );
	this->outputFile->WriteByte( this->audioByte );
	this->outputFile->Write( data, size );

	this->prevTagSize = tag.size.GetInt( ) + sizeof( FLV_Tag );
}

void MovieExport::Init( ) {

	char asciiName[ 16 ];
	char asciiDesc[ 128 ];
	WindowsNS::LRESULT isOk;
	WindowsNS::ICINFO compInfo;
	WindowsNS::ICOPEN openInfo = { 0 };
	openInfo.fccType = FourCC( "vidc" );

	x264Module = new DynamicModule( "x264vfw.dll" );
	faccModule = new DynamicModule( "libfaac.dll" );

	DriverProc = ( MEDRIVERPROC )x264Module->FindProc( "DriverProc" );

	faacEncGetVersion				= ( FAACENCGETVERSIONPROC )					faccModule->FindProc( "faacEncGetVersion" );
	faacEncGetCurrentConfiguration	= ( FAACENCGETCURRENTCONFIGURATIONPROC )	faccModule->FindProc( "faacEncGetCurrentConfiguration" );
	faacEncSetConfiguration			= ( FAACENCSETCONFIGURATIONPROC )			faccModule->FindProc( "faacEncSetConfiguration" );
	faacEncOpen						= ( FAACENCOPENPROC )						faccModule->FindProc( "faacEncOpen" );
	faacEncGetDecoderSpecificInfo	= ( FAACENCGETDECODERSPECIFICINFOPROC )		faccModule->FindProc( "faacEncGetDecoderSpecificInfo" );
	faacEncEncode					= ( FAACENCENCODEPROC )						faccModule->FindProc( "faacEncEncode" );
	faacEncClose					= ( FAACENCCLOSEPROC )						faccModule->FindProc( "faacEncClose" );

	faacHandle = faacEncOpen( 44100, 2, &faacInputSamples, &faacMaxOutputBytes );
	faacConfig = faacEncGetCurrentConfiguration( faacHandle );
	faacConfig->inputFormat = FAAC_INPUT_16BIT;
	//faacConfig->outputFormat = 0;
	faacEncSetConfiguration( faacHandle, faacConfig );
	faacInputBuffer = ( int * )Mem_Alloc( faacInputSamples * sizeof( int ) );
	faacOutputBuffer = ( unsigned char * )Mem_Alloc( faacMaxOutputBytes );

	DriverProc( 0, 0, DRV_LOAD, 0, 0 );
	codecInstance = ( x264Codec_t * )DriverProc( 0, 0, DRV_OPEN, 0, ( WindowsNS::DWORD_PTR )&openInfo );
	DriverProc( codecInstance, 0, ICM_GETINFO, ( WindowsNS::DWORD_PTR )&compInfo, sizeof( WindowsNS::ICINFO ) );

	wcstombs( asciiName, compInfo.szName, 16 );
	wcstombs( asciiDesc, compInfo.szDescription, 128 );
	Common::Com_Printf( "MovieExport {\n" );
	Common::Com_Printf( "\tVideo Encoder Name: %s\n", asciiName );
	Common::Com_Printf( "\tVideo Encoder Desc: %s\n", asciiDesc );
	Common::Com_Printf( "\tVideo Encoder Flags:" );
	if( compInfo.dwFlags & VIDCF_QUALITY )			Common::Com_Printf( " VIDCF_QUALITY" );
	if( compInfo.dwFlags & VIDCF_CRUNCH )			Common::Com_Printf( " VIDCF_CRUNCH" );
	if( compInfo.dwFlags & VIDCF_TEMPORAL )			Common::Com_Printf( " VIDCF_TEMPORAL" );
	if( compInfo.dwFlags & VIDCF_COMPRESSFRAMES )	Common::Com_Printf( " VIDCF_COMPRESSFRAMES" );
	if( compInfo.dwFlags & VIDCF_DRAW )				Common::Com_Printf( " VIDCF_DRAW" );
	if( compInfo.dwFlags & VIDCF_FASTTEMPORALC )	Common::Com_Printf( " VIDCF_FASTTEMPORALC" );
	if( compInfo.dwFlags & VIDCF_FASTTEMPORALD )	Common::Com_Printf( " VIDCF_FASTTEMPORALD" );
	Common::Com_Printf( "\n}\n" );

	Common::Com_Memset( &inputFormat, 0, sizeof( WindowsNS::BITMAPINFO ) );
	inputFormat.bmiHeader.biSize = sizeof( WindowsNS::BITMAPINFOHEADER );
	inputFormat.bmiHeader.biWidth = Video::viddef.width;
	inputFormat.bmiHeader.biHeight = Video::viddef.height;
	inputFormat.bmiHeader.biPlanes = 1;
	inputFormat.bmiHeader.biBitCount = 24;
	inputFormat.bmiHeader.biCompression = BI_RGB;
	inputFormat.bmiHeader.biSizeImage = Video::viddef.width * Video::viddef.height * 3;

	isOk = DriverProc( codecInstance, 0, ICM_COMPRESS_GET_FORMAT, ( WindowsNS::DWORD_PTR )&inputFormat, ( WindowsNS::DWORD_PTR )&outputFormat );

	if( isOk != ICERR_OK ) {

		Common::Com_Printf( "MovieExport: ICM_COMPRESS_GET_FORMAT failed\n" );
		Shutdown( );
		return;
	}

	if( ( int )DriverProc( codecInstance, 0, ICM_GETSTATE, 0, 0 ) != sizeof( x264Config_t ) ) {

		Common::Com_Printf( "MovieExport: ICM_GETSTATE returns unfamiliar size\n" );
		Shutdown( );
		return;
	}

	readBuffer = ( byte * )Mem_Alloc( inputFormat.bmiHeader.biSizeImage );
	inputBuffer = ( byte * )Mem_Alloc( inputFormat.bmiHeader.biSizeImage );
	outputBuffer = ( byte * )Mem_Alloc( outputFormat.bmiHeader.biSizeImage );

	Common::Com_Memset( &compParams, 0, sizeof( WindowsNS::ICCOMPRESS ) );
	compParams.lpbiOutput = &outputFormat.bmiHeader;
	compParams.lpbiInput = &inputFormat.bmiHeader;
	compParams.lpckid = &compChunkId;
	compParams.lpdwFlags = &compFlags;
	compParams.lpInput = inputBuffer;
	compParams.lpOutput = outputBuffer;
	compParams.dwFrameSize = outputFormat.bmiHeader.biSizeImage;

	Command::Cmd_AddCommand( "x-conf", Configure );
	Command::Cmd_AddCommand( "x-add", Compress );

	compThread.StartThread( );

	deltaFrameTime = 1000 / 25;

	isValid = true;
	isActive = false;
}

void MovieExport::Shutdown( ) {

	if( isValid ) {

		if( isActive ) Compress( );

		Command::Cmd_RemoveCommand( "x-conf" );
		Command::Cmd_RemoveCommand( "x-add" );
	}

	compThread.StopThread( );

	isActive = isValid = false;

	if( readBuffer ) Mem_Free( readBuffer );
	if( inputBuffer ) Mem_Free( inputBuffer );
	if( outputBuffer ) Mem_Free( outputBuffer );
	readBuffer = NULL;
	inputBuffer = NULL;
	outputBuffer = NULL;

	if( faacInputBuffer )Mem_Free( faacInputBuffer );
	if( faacOutputBuffer ) Mem_Free( faacOutputBuffer );
	faacInputBuffer = NULL;
	faacOutputBuffer = NULL;

	DriverProc( codecInstance, 0, DRV_CLOSE, 0, 0 );
	DriverProc( 0, 0, DRV_FREE, 0, 0 );

	faacEncClose( faacHandle );

	delete x264Module;
	delete faccModule;
}

void MovieExport::Frame( ) {

	// recording not enabled
	if( !isActive ) return;

	if( System::Sys_LastTime( ) < ( lastFrameTime + deltaFrameTime ) ) return;

	lastFrameTime = System::Sys_LastTime( );

	soundSampleBufferRaw = SoundSystem::Capture( soundSampleCountRaw );

	GLImports::glReadPixels( 0, 0, inputFormat.bmiHeader.biWidth,inputFormat.bmiHeader.biHeight, GL_RGB, GL_UNSIGNED_BYTE, readBuffer );

	compThread.WaitForThread( );
	compThread.SignalThread( );
}

void MovieExport::Configure( ) {

	WindowsNS::LRESULT isOk;

	if( isActive ) {

		Common::Com_Printf( "MovieExport: recording in progress\n" );
		return;
	}

	Input::IN_DeactivateMouse( );

	isOk = DriverProc( codecInstance, 0, ICM_CONFIGURE, ( WindowsNS::DWORD_PTR )cl_hwnd, 0 );

	Input::IN_ActivateMouse( );
}

void MovieExport::Compress( ) {

	WindowsNS::LRESULT isOk;

	if( !isValid ) return;

	isActive = !isActive;

	if( isActive ) {

		if( !outputFile.SetOutputFile( "dummy.flv" ) ) {

			Common::Com_Printf( "MovieExport: unable to open output file\n" );
			isActive = false;
			return;
		}
		
		outputFile.WriteHeader( true, true );
		firstFrameTime = lastFrameTime = System::Sys_LastTime( );
		isOk = DriverProc( codecInstance, 0, ICM_COMPRESS_BEGIN, ( WindowsNS::DWORD_PTR )&inputFormat, ( WindowsNS::DWORD_PTR )&outputFormat );

		if( isOk != ICERR_OK ) {

			Common::Com_Printf( "MovieExport: ICM_COMPRESS_BEGIN failed\n" );
			isActive = false;
			return;
		}

		SoundSystem::EnableCapture( );

	} else {

		SoundSystem::DisableCapture( );

		compThread.WaitForThread( );

		outputFile.CloseOutputFile( ( int )( lastFrameTime - firstFrameTime ) );

		isOk = DriverProc( codecInstance, 0, ICM_COMPRESS_END, 0, 0 );

		if( isOk != ICERR_OK ) Common::Com_Printf( "MovieExport: ICM_COMPRESS_END failed\n" );
	}
}

void MovieExportThread::StartThread( ) {

	if( thread != NULL ) {

		return;
	}

	thread = new ThreadBase( this, THREAD_NORMAL );
	thread->SetName( "MovieExportThread" );

	if( !thread->Start( ) ) {

		Common::Com_Printf( "MovieExportThread: failed to start\n" );
	}
	
	// let first MovieExport::Frame( ) pass
	workEndSignal.Set( );
}

void MovieExportThread::StopThread( ) {

	if( thread != NULL ) {

		thread->Stop( );
		workSignal.Set( );
		thread->Join( );
		thread->Destroy( );
		thread = NULL;
	}
}

unsigned int MovieExportThread::Run( void * param ) {

	while( 1 ) {

		workSignal.Wait( );
		if( Terminating( ) ) break;

		// performs red <-> blue swap, assumes this loop ends before next video frame comes in ;)
		for( int i = 0; i < ( int )MovieExport::inputFormat.bmiHeader.biSizeImage; i += 3 ) {

			MovieExport::inputBuffer[ i + 0 ] = MovieExport::readBuffer[ i + 2 ];
			MovieExport::inputBuffer[ i + 1 ] = MovieExport::readBuffer[ i + 1 ];
			MovieExport::inputBuffer[ i + 2 ] = MovieExport::readBuffer[ i + 0 ];
		}

		MovieExport::compParams.dwFlags = MovieExport::compParams.lFrameNum ? 0 : ICCOMPRESS_KEYFRAME;
		MovieExport::DriverProc( MovieExport::codecInstance, 0, ICM_COMPRESS, ( WindowsNS::DWORD_PTR )&MovieExport::compParams, sizeof( WindowsNS::ICCOMPRESS ) );
		MovieExport::outputFile.WriteVideoFrame( MovieExport::outputBuffer,  MovieExport::outputFormat.bmiHeader.biSizeImage, ( int )( MovieExport::lastFrameTime - MovieExport::firstFrameTime ), !MovieExport::compParams.lFrameNum );
		MovieExport::compParams.lFrameNum++;

		MovieExport::outputFile.WriteAudioFrame( MovieExport::soundSampleBufferRaw, MovieExport::soundSampleCountRaw * 4, ( int )( MovieExport::lastFrameTime - MovieExport::firstFrameTime ) );

		/*int compressedAudioSize = 0;
		int soundSamplesNum = MovieExport::soundSampleCountRaw * 2;
		unsigned short * soundSamples = ( unsigned short * )MovieExport::soundSampleBufferRaw;
		while( soundSamplesNum ) {

			int step = Min( soundSamplesNum, ( int )MovieExport::faacInputSamples );
			//for( int i = 0; i < step; i++ ) MovieExport::faacInputBuffer[ i ] = soundSamples[ i ];
			int encodedSize = MovieExport::faacEncEncode( MovieExport::faacHandle, ( int * )soundSamples, step, MovieExport::faacOutputBuffer, MovieExport::faacMaxOutputBytes );
			compressedAudioSize += encodedSize;
			MovieExport::outputFile.WriteAudioFrame( MovieExport::faacOutputBuffer, encodedSize, MovieExport::lastFrameTime - MovieExport::firstFrameTime );
			soundSamplesNum -= step;
			soundSamples += step;
		}*/

		workEndSignal.Set( );
	}
	
	return 0;
}

void MovieExportThread::Stop( ) {

	ThreadProcess::Stop( );
}
