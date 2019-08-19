#include "precompiled.h"
#pragma hdrstop

Heap * 	FileSystem::fsHeap = NULL;
NamedList< ZipFileSystem * >		FileSystem::zipList;
CANamedList< FileBase *, 256 >			FileSystem::g_fileList;
NamedList< ResourceRecord * >	FileSystem::g_resourceList;

ZCALLBACK FileBase * ZipFileSystem::CB_open64_file_func( ZipFileSystem & zip_sys, const char * filename, int mode ) {

	zip_sys.d_zipFile = FileSystem::OpenFileByMode( filename, FS_READ );
	return zip_sys.d_zipFile;
}

int ZipFileSystem::CB_close_file_func( ZipFileSystem & zip_sys, FileBase * zip_file ) {

	FileSystem::CloseFile( zip_file );
	return 0;
}

ZLib::uLong ZipFileSystem::CB_zread_file( ZipFileSystem & zip_sys, FileBase * zip_file, void * buf, ZLib::uLong size ) {

	zip_sys.d_numRead++;
	zip_sys.d_allRead += ( int )size;
	return ( ZLib::uLong )zip_file->Read( buf, size );
}

ZLib::uLong ZipFileSystem::CB_zwrite_file( ZipFileSystem & zip_sys, FileBase * zip_file, const void * buf, ZLib::uLong size ) {

	zip_sys.d_numWrite++;
	zip_sys.d_allWrite += ( int )size;
	return ( ZLib::uLong )zip_file->Write( buf, size );
}

ZLib::ZPOS64_T ZipFileSystem::CB_ztell64_file( ZipFileSystem & zip_sys, FileBase * zip_file ) {

	zip_sys.d_numTell++;
	return zip_file->Tell( );
}

long ZipFileSystem::CB_zseek64_file( ZipFileSystem & zip_sys, FileBase * zip_file, ZLib::ZPOS64_T offset, int origin ) {

	zip_sys.d_numSeek++;
	return zip_file->Seek( ( long )offset, ( fsOrigin_t )origin );
}

int ZipFileSystem::CB_zerror_file( ZipFileSystem & zip_sys, FileBase * zip_file ) {

	Common::Com_Printf( "ZipFileSystem: zerror_file\n" );
	return 0;
}

ZipFileSystem::ZipFileSystem( const Str & path ) {
	d_name = path;
	d_zipFileLock = false;
	Init( );
}

ZipFileSystem::~ZipFileSystem( ) {

	Shutdown( );
}

void ZipFileSystem::Init( ) {

	ClearCounters( );
	ZLib::zlib_filefunc64_def	zipFileFunc;
	zipFileFunc.zopen64_file	= ( ZLib::open64_file_func )CB_open64_file_func;
	zipFileFunc.zclose_file		= ( ZLib::close_file_func )CB_close_file_func;
	zipFileFunc.zread_file		= ( ZLib::read_file_func )CB_zread_file;
	zipFileFunc.zwrite_file		= ( ZLib::write_file_func )CB_zwrite_file;
	zipFileFunc.ztell64_file	= ( ZLib::tell64_file_func )CB_ztell64_file;
	zipFileFunc.zseek64_file	= ( ZLib::seek64_file_func )CB_zseek64_file;
	zipFileFunc.zerror_file		= ( ZLib::testerror_file_func )CB_zerror_file;
	zipFileFunc.opaque			= this;

	d_zipFileHandle = ZLib::unzOpen2_64( GetName( ).c_str( ), &zipFileFunc );
	if( !d_zipFileHandle  ) {
		Common::Com_Printf( "Warning: ZipFileSystem: unable to open zip file\n" );
		return;
	}
	if( ZLib::unzGoToFirstFile( d_zipFileHandle ) != UNZ_OK ) {
		Common::Com_Printf( "Warning: ZipFileSystem: empty zip file\n" );
		Shutdown( );
		return;
	}
	ZipRecord zipRecord;
	char fileName[ 512 ];
	while( 1 ) {
		ZLib::unzGetFilePos64( d_zipFileHandle, &zipRecord.d_filePos );
		ZLib::unzGetCurrentFileInfo64( d_zipFileHandle, &zipRecord.d_fileInfo, fileName, 512, NULL, 0, NULL, 0 );
		if( zipRecord.d_fileInfo.uncompressed_size ) {
			zipRecord.SetName( fileName );
			d_zipFileList.Append( new ZipRecord( zipRecord ) );
		}
		if( ZLib::unzGoToNextFile( d_zipFileHandle ) == UNZ_END_OF_LIST_OF_FILE )	
			break;
	}
}

void ZipFileSystem::Shutdown( ) {

	d_zipFileList.DeleteContents( true );
	ZLib::unzClose( d_zipFileHandle );
}

size_t ZipFileSystem::GetSize( const Str & relative_path ) {

	ZipRecord ** zipRec = d_zipFileList.FindByName( relative_path );
	if( zipRec )
		return ( *zipRec )->d_fileInfo.uncompressed_size;
	return 0;
}

bool ZipFileSystem::ReadFile( const Str & relative_path, File_Memory & dest ) {

	ZipRecord ** zipRec = d_zipFileList.FindByName( relative_path );
	if( !zipRec )
		return false;
	ZLib::unzGoToFilePos64( d_zipFileHandle, &( *zipRec )->d_filePos );
	if( ZLib::unzOpenCurrentFile( d_zipFileHandle ) != UNZ_OK )
		return false;
	int readSucces = ZLib::unzReadCurrentFile( d_zipFileHandle, dest.GetDataPtr( ), ( unsigned int )dest.Length( ) );
	if( ZLib::unzCloseCurrentFile( d_zipFileHandle ) == UNZ_CRCERROR ) {
		Common::Com_Printf( "Warning: ZipFileSystem::ReadFile: CRC error in \"%s\" in zip \"%s\"\n", ( *zipRec )->GetName( ).c_str( ), GetName( ).c_str( ) );
		return false;
	}
	return ( readSucces < 0 ) ? false : true;
}

void FileSystem::ZipDir( ) {

	if( Command::Cmd_Argc( ) != 2 ) {
		Common::Com_Printf( "usage: zdir < zip file >\n" );
		return;
	}
	ZLib::unzFile zipFile = ZLib::unzOpen( RelativePathToOSPath( Command::Cmd_Argv( 1 ) ) );
	if( !zipFile ) {
		Common::Com_Printf( "zdir: unable to open zip file\n" );
		return;
	}
	if( ZLib::unzGoToFirstFile( zipFile ) != UNZ_OK ) {
		Common::Com_Printf( "zdir: empty zip file\n" );
		return;
	}
	ZLib::unz_file_info fileInfo;
	char fileName[ 512 ];
	while( 1 ) {
		ZLib::unzGetCurrentFileInfo( zipFile, &fileInfo, fileName, 512, NULL, 0, NULL, 0 );
		Common::Com_Printf( "%.2fKiB\t%.2fKiB\t%s\n", ( float )fileInfo.compressed_size / 1024.0f, ( float )fileInfo.uncompressed_size / 1024.0f, fileName );
		if( ZLib::unzGoToNextFile( zipFile ) == UNZ_END_OF_LIST_OF_FILE )			
			break;
	}
	ZLib::unzClose( zipFile );
}

void FileSystem::Dir( ) {

	Str findFilter = "*";

	if( Command::Cmd_Argc( ) >= 2 ) {

		findFilter = Command::Cmd_Args( );
	}

	List< Str > fileList = ListFiles( findFilter, 0, 0 );

	for( int i = 0; i < fileList.Num( ); i++ ) {

		Common::Com_Printf( "%s\n", fileList[ i ].c_str( ) );
	}
}

const Str FileSystem::OSPathToRelativePath( const Str & OSPath ) {

	return "not implemented";
}

const Str FileSystem::RelativePathToOSPath( const Str & relativePath ) {

	Str osPath = "../";
	osPath.Append( fs_gamedir.GetString( ) );
	osPath.Append( '/' );
	osPath.Append( relativePath );

#if defined(_WIN32) || defined(_WIN64)
	osPath.Replace( "/", "\\" ); // EVIL windows backslash
#endif

	return osPath;
}

bool FileSystem::TouchFile( const Str & relativePath ) {

	Str osPath = RelativePathToOSPath( relativePath );
	FILE * file = fopen( osPath, "rb" );
	bool retValue = false;

	if( file ) {

		retValue = true;
		fclose( file );
	}

	return retValue;
}

File_Memory FileSystem::ReadFile( const Str & relativePath ) {

	File_Memory mappedFile;
	FileBase * file = OpenFileRead( relativePath );

	if( !file )
		return mappedFile;

	mappedFile.d_size = file->Length( );
	mappedFile.d_filePtr = fsHeap->Allocate( mappedFile.d_size + 1 );
	file->Read( mappedFile.d_filePtr, mappedFile.d_size );
	( ( byte * )mappedFile.d_filePtr )[ mappedFile.d_size ] = 0;

	CloseFile( file );

	mappedFile.d_valid = true;
	return mappedFile;
}

void FileSystem::FreeFile( File_Memory & buffer ) {

	fsHeap->Free( buffer.d_filePtr );
	buffer.d_valid = false;
}

FileBase * FileSystem::OpenFileRead( const Str & relativePath ) {

	return OpenFileByMode( relativePath, FS_READ );
}

FileBase * FileSystem::OpenFileByMode( const Str & relativePath, fsMode_t mode ) {

	Str path = relativePath;
	Str name = relativePath;
	path.StripFilename( );
	name.StripPath( );

	ResourceRecord * resRecord;
	ZipRecord * zipRecord = NULL;
	ZipFileSystem * zipSystem = NULL;
	for( int i = 0; i < zipList.Num( ); i++ ) {
		zipRecord = zipList[ i ]->Find( relativePath );
		if( zipRecord ) {
			zipSystem = zipList[ i ];
			break;
		}
	}

	if( zipRecord ) {

		File_Zip * ret = new File_Zip( );
		ret->d_path = path;
		ret->d_name = name;
		ret->Zip_Fopen( zipSystem, zipRecord );
		g_fileList.Append( ret );
		return ret;

	} else if( ( resRecord = g_resourceList.FindByName2( relativePath ) ) ) {

		File_Resource * ret = new File_Resource( );
		ret->d_path = path;
		ret->d_name = name;
		ret->Resource_Fopen( resRecord );
		g_fileList.Append( ret );
		return ret;

	} else {

		File_Stdio * ret = new File_Stdio( );
		ret->d_path = path;
		ret->d_name = name;
		Str osPath = RelativePathToOSPath( relativePath );
		ret->Stdio_Fopen( osPath, mode );
		if( !ret->IsValid( ) ) {
			Common::Com_DPrintf( "FileSystem::OpenFileByMode: \"%s\" file open error.\n", relativePath.c_str( ) );
			delete ret;
			return NULL;
		}
		g_fileList.Append( ret );
		return ret;
	}
	return NULL;
}

void FileSystem::CloseFile( FileBase *& f ) {

	bool found = g_fileList.Remove( f );
	assert( found );

	switch( f->d_type ) {
		case FT_STDIO:
			dynamic_cast< File_Stdio * >( f )->Stdio_Fclose( );
			break;
		case FT_ZIP:
			dynamic_cast< File_Zip * >( f )->Zip_Fclose( );
			break;
		case FT_RESOURCE:
			dynamic_cast< File_Resource * >( f )->Resource_Fclose( );
			break;
	}

	delete f;
	f = NULL;
}

/*
================
CopyFile
================
*/
void FileSystem::FileCopy( const Str & fromOSPath, const Str & toOSPath ) {

	FILE	* f1, * f2;
	size_t	l;
	byte	buffer[ 65536 ];

	Common::Com_DPrintf( "CopyFile( %s, %s )\n", fromOSPath.c_str( ), toOSPath.c_str( ) );

	f1 = fopen( fromOSPath.c_str( ), "rb" );
	if( !f1 ) return;
	f2 = fopen( toOSPath.c_str( ), "wb" );
	if( !f2 ) {

		fclose( f1 );
		return;
	}

	while( 1 ) {

		l = fread( buffer, 1, sizeof( buffer ), f1 );
		if( !l ) break;
		fwrite( buffer, 1, l, f2 );
	}

	fclose( f1 );
	fclose( f2 );
}

List< Str >	FileSystem::ListFiles( const Str & findname, unsigned musthave, unsigned canthave ) {

	List< Str > list;

	//Str s;
	//s = System::Sys_FindFirst( RelativePathToOSPath( findname ), musthave, canthave );

	struct _finddata_t findInfo;
	Str path = RelativePathToOSPath( findname );
	intptr_t findHandle = _findfirst( path, &findInfo );
	if( findHandle == -1 ) return list;

	while( 1 ) {

		if( ( findInfo.name[ 0 ] != '.' && findInfo.name[ 1 ] != 0 ) || ( findInfo.name[ 0 ] != '.' && findInfo.name[ 1 ] != '.' && findInfo.name[ 2 ] != 0 ) ) {

			list.Append( ( !findInfo.size ) ? Str( findInfo.name ) += "/" : findInfo.name );
		}

		//s = System::Sys_FindNext( musthave, canthave );
		if( _findnext( findHandle, &findInfo ) == -1 ) break;
	}

	//System::Sys_FindClose( );
	_findclose( findHandle );

	return list;
}

bool FileSystem::EnumResNameProc( void * mod, const char * type, const char * name, const Str & type_str ) {

	ResourceRecord * resRecord = new ResourceRecord( );
	resRecord->SetName( type_str + ( int )name );
	resRecord->d_resourceType = type;
	resRecord->d_resourceName = name;
	g_resourceList.Append( resRecord );
	return true;
}

bool FileSystem::EnumResTypeProc( void * mod, const char * type, long * param ) {

	Str typeStr;

	if( ( __int64 )type >> 16 ) {
		typeStr = type;
	} else switch( ( int )type ) {
		case 1:
			typeStr = "CURSOR/";
			break;
		case 2:
			typeStr = "BITMAP/";
			break;
		case 3:
			typeStr = "ICON/";
			break;
		case 6:
			typeStr = "STRING/";
			break;
		case 12:
			typeStr = "GROUPCURSOR/";
			break;
		case 14:
			typeStr = "GROUPICON/";
			break;
		case 16:
			typeStr = "VERSION/";
			break;
		default:
			return true;
	}

	WindowsNS::EnumResourceNamesA( NULL, type, ( WindowsNS::ENUMRESNAMEPROCA )EnumResNameProc, ( WindowsNS::LONG_PTR )&typeStr );
	return true;
}

void FileSystem::Init( ) {

	ZLibImports::Init( );
	fsHeap = new Heap;

	List< Str > zipMounts = ListFiles( "*.zip", 0, 0 );

	for( int i = 0; i < zipMounts.Num( ); i++ ) {
		zipList.Append( new ZipFileSystem( zipMounts[ i ] ) );
	}

	WindowsNS::EnumResourceTypesA( NULL, ( WindowsNS::ENUMRESTYPEPROCA )EnumResTypeProc, NULL );

	/*FileBase * testResFile = OpenFileByMode( "ICON/1", FS_READ );

	char buff[ 512 ];
	for( int i = 1; ; i++ ) {
		testResFile->Read( buff, 512 );
		long readPos = testResFile->Tell( );
		if( readPos != ( i * 512 ) )
			break;
	}

	CloseFile( testResFile );*/

	/*FileBase * testZipFile = OpenFileByMode( "cegui/TaharezLook.looknfeel", FS_READ );

	char buff[ 512 ];
	for( int i = 1; ; i++ ) {
		testZipFile->Read( buff, 512 );
		long readPos = testZipFile->Tell( );
		if( readPos != ( i * 512 ) )
			break;
	}

	CloseFile( testZipFile );*/
}

void FileSystem::Restart( ) {

	Shutdown( );
	Init( );
}

void FileSystem::Shutdown( ) {

	ZLibImports::Shutdown( );
	delete fsHeap;
	zipList.DeleteContents( true );
	g_resourceList.DeleteContents( true );
	assert( !g_fileList.Num( ) );
}

void FileSystem::Register( ) {

	Command::Cmd_AddCommand( "zdir", ZipDir );
	Command::Cmd_AddCommand( "dir", Dir );
}

// // //// // //// // //// //
// File
// //// // //// // //// //

int FileBase::WriteByte( const byte & value ) {

	return ( int )Write( &value, 1 );
}

int FileBase::WriteAMFString( const Str & string ) {

	Int16< BO_BigEndian > len = string.Length( );
	int lenWrite = ( int )Write( len.Ptr( ), sizeof( Int16< BO_BigEndian > ) );
	return ( int )Write( string.c_str( ), string.Length( ) ) + lenWrite;
}

int FileBase::Printf( const char * fmt, ... ) {

 	Str theBuff;
	va_list argptr;

	va_start( argptr, fmt );
	int len = vsprintf( theBuff, fmt, argptr );
	va_end( argptr );

	return ( int )Write( theBuff.c_str( ), len );
}

int FileBase::VPrintf( const char * fmt, va_list arg ) {

	Str theBuff;

	int len = vsprintf( theBuff, fmt, arg );

	return ( int )Write( theBuff.c_str( ), len );
}

int FileBase::WriteFloatString( const char * fmt, ... ) {

	Str theBuff;
	va_list argptr;

	va_start( argptr, fmt );
	int len = vsprintf( theBuff, fmt, argptr );
	va_end( argptr );

	return ( int )Write( theBuff.c_str( ), len );
}

// // //// // //// // //// //
// File_Stdio
// //// // //// // //// //

size_t File_Stdio::Read( void * buffer, size_t len ) {
	return fread( buffer, 1, len, ( FILE * )d_handle );
}

size_t File_Stdio::Write( const void * buffer, size_t len ) {
	return fwrite( buffer, 1, len, ( FILE * )d_handle );
}

size_t File_Stdio::Length( ) {

	long cPos = Tell( );
	Seek( 0, FS_SEEK_END );
	int fSize = Tell( );
	Seek( cPos, FS_SEEK_SET );
	return fSize;
}

timeStamp_t File_Stdio::Timestamp( ) {

	struct _stat64 st;
	_fstat64( _fileno( ( FILE * )d_handle ), &st );
	return st.st_mtime;
}

long File_Stdio::Tell( ) {
	return ftell( ( FILE * )d_handle );
}

void File_Stdio::Flush( ) {
	fflush( ( FILE * )d_handle );
}

int File_Stdio::Seek( long offset, fsOrigin_t origin ) {
	return fseek( ( FILE * )d_handle, offset, origin );
}

void File_Stdio::Rewind( ) {
	fseek( ( FILE * )d_handle, 0, FS_SEEK_CUR );
}

void File_Stdio::Stdio_Fopen( const Str & path, int mode ) {

	switch( mode ) {

		case FS_READ:
			d_handle = fopen( path, "rb" );
			break;

		case FS_WRITE:		
			d_handle = fopen( path, "wb" );
			break;

		case FS_APPEND:
			d_handle = fopen( path, "ab" );
			break;
	}

	if( d_handle )
		d_type = FT_STDIO;
}

void File_Stdio::Stdio_Fclose( ) {
	fclose( d_handle );
}

// // //// // //// // //// //
// File_Zip
// //// // //// // //// //

size_t File_Zip::Read( void * buffer, size_t len ) {
	return ( size_t )ZLib::unzReadCurrentFile( d_zipHandle, buffer, ( unsigned int )len );
}

size_t File_Zip::Write( const void * buffer, size_t len ) {
	return 0;
}

size_t File_Zip::Length( ) {
	return ( size_t )d_zipRecord->d_fileInfo.uncompressed_size;
}

timeStamp_t File_Zip::Timestamp( ) {
	return ( timeStamp_t )d_zipRecord->d_fileInfo.dosDate;
}

long File_Zip::Tell( ) {
	return ( long )ZLib::unztell64( d_zipHandle );
}

void File_Zip::Flush( ) {
}

int File_Zip::Seek( long offset, fsOrigin_t origin ) {
	return 0;
}

void File_Zip::Rewind( ) {
}

void File_Zip::Zip_Fopen( ZipFileSystem * zip_sys, ZipRecord * zip_rec ) {

	d_zipRecord = zip_rec;
	d_zipHandle = zip_sys->Lock( );
	d_zipSystem = zip_sys;
	ZLib::unzGoToFilePos64( d_zipHandle, &zip_rec->d_filePos );
	int succes = ZLib::unzOpenCurrentFile( d_zipHandle );
	assert( succes == UNZ_OK );
	d_type = FT_ZIP;
}

void File_Zip::Zip_Fclose( ) {

	int succes = ZLib::unzCloseCurrentFile( d_zipHandle );
	assert( succes != UNZ_CRCERROR );
	d_zipSystem->UnLock( );
}

// // //// // //// // //// //
// File_Resource
// //// // //// // //// //

size_t File_Resource::Read( void * buffer, size_t len ) {

	size_t readCount = Min( ( size_t )len, d_resourceSize - ( size_t )d_offset );
	Common::Com_Memcpy( buffer, &d_resourcePtr[ d_offset ], readCount );
	d_offset += ( long )readCount;
	return readCount;
}

size_t File_Resource::Write( const void * buffer, size_t len ) {
	return 0;
}

size_t File_Resource::Length( ) {
	return d_resourceSize;
}

timeStamp_t File_Resource::Timestamp( ) {
	return 0;
}

long File_Resource::Tell( ) {
	return d_offset;
}

void File_Resource::Flush( ) {
}

int File_Resource::Seek( long offset, fsOrigin_t origin ) {

	switch( origin ) {
		case FS_SEEK_SET:
			d_offset = offset;
			break;
		case FS_SEEK_CUR:
			d_offset += offset;
			break;
		case FS_SEEK_END:
			d_offset = ( long )d_resourceSize - offset;
			break;
	}
	if( d_offset < 0 ) {
		d_offset = 0;
		return 0;
	}
	if( d_offset > ( long )d_resourceSize ) {
		d_offset = 0;
		return 0;
	}
	return 1;
}

void File_Resource::Rewind( ) {
	d_offset = 0;
}

void File_Resource::Resource_Fopen( ResourceRecord * res_rec ) {

	d_resourceObject = NULL;
	d_resourceRecord = res_rec;
	WindowsNS::HRSRC resourceHandle	= WindowsNS::FindResourceA( NULL, res_rec->d_resourceName, res_rec->d_resourceType );

	if( !resourceHandle )
		return;

	d_resourceSize			= ( size_t )WindowsNS::SizeofResource( NULL, resourceHandle );
	d_resourceObject		= WindowsNS::LoadResource( NULL, resourceHandle );
	d_resourcePtr			= ( byte * )WindowsNS::LockResource( d_resourceObject );
	d_offset				= 0;
	d_type					= FT_RESOURCE;
}

void File_Resource::Resource_Fclose( ) {

	if( d_resourceObject )
		WindowsNS::FreeResource( d_resourceObject );
}
