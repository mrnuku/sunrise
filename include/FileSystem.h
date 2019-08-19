// Copyright ( C) 2004 Id Software, Inc.
//

#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

/*
===============================================================================

	File System

	No stdio calls should be used by any part of the game, because of all sorts
	of directory and separator char issues. Throughout the game a forward slash
	should be used as a separator. The file system takes care of the conversion
	to an OS specific separator. The file system treats all file and directory
	names as case insensitive.

	The following cvars store paths used by the file system:

	"fs_basepath"		path to local install, read-only
	"fs_savepath"		path to config, save game, etc. files, read & write
	"fs_cdpath"			path to cd, read-only
	"fs_devpath"		path to files created during development, read & write

	The base path for file saving can be set to "fs_savepath" or "fs_devpath".

===============================================================================
*/

static const unsigned	FILE_NOT_FOUND_TIMESTAMP	= 0xFFFFFFFF;
static const int		MAX_PURE_PAKS				= 128;
static const int		MAX_OSPATH					= 256;

// modes for OpenFileByMode. used as bit mask internally
typedef enum {

	FS_READ		= 0,
	FS_WRITE	= 1,
	FS_APPEND	= 2

} fsMode_t;

class ZipRecord : public NamedObject {

public:

	ZLib::unz_file_info64			d_fileInfo;
	ZLib::unz64_file_pos			d_filePos;
};

class ZipFileSystem : public NamedObject {

private:

	ZLib::unzFile					d_zipFileHandle;
	NamedList< ZipRecord * >		d_zipFileList;
	FileBase *						d_zipFile;
	bool							d_zipFileLock;

	int								d_numRead;
	int								d_allRead;
	int								d_numWrite;
	int								d_allWrite;
	int								d_numTell;
	int								d_numSeek;

	void							ClearCounters( )	{ d_numRead = d_allRead = d_numWrite = d_allWrite = d_numTell = d_numSeek = 0; }

	static ZCALLBACK FileBase *		CB_open64_file_func( ZipFileSystem & zip_sys, const char * filename, int mode );
	static int						CB_close_file_func( ZipFileSystem & zip_sys, FileBase * zip_file );
	static ZLib::uLong				CB_zread_file( ZipFileSystem & zip_sys, FileBase * zip_file, void * buf, ZLib::uLong size );
	static ZLib::uLong				CB_zwrite_file( ZipFileSystem & zip_sys, FileBase * zip_file, const void * buf, ZLib::uLong size );
	static ZLib::ZPOS64_T			CB_ztell64_file( ZipFileSystem & zip_sys, FileBase * zip_file );
	static long						CB_zseek64_file( ZipFileSystem & zip_sys, FileBase * zip_file, ZLib::ZPOS64_T offset, int origin );
	static int						CB_zerror_file( ZipFileSystem & zip_sys, FileBase * zip_file );

public:

									ZipFileSystem( const Str & path );
									~ZipFileSystem( );

	void							Init( );
	void							Shutdown( );

	ZipRecord *						Find( const Str & rec_name ) { return d_zipFileList.FindByName2( rec_name ); }
	ZLib::unzFile					Lock( ) { assert( !d_zipFileLock ); d_zipFileLock = true; return d_zipFileHandle; }
	void							UnLock( ) { d_zipFileLock = false; }

	size_t							GetSize( const Str & relative_path );
	bool							ReadFile( const Str & relative_path, File_Memory & dest );
};

class ResourceRecord : public NamedObject {

public:

	const char *					d_resourceType;
	const char *					d_resourceName;
};

class FileSystem {

private:

	static class Heap *		fsHeap;
	static class CVar		fs_gamedir;
	static NamedList< ZipFileSystem * >		zipList;
	static CANamedList< FileBase *, 256 >	g_fileList;
	static NamedList< ResourceRecord * >	g_resourceList;

	static bool CALLBACK	EnumResNameProc( void * mod, const char * type, const char * name, const Str & type_str );
	static bool CALLBACK	EnumResTypeProc( void * mod, const char * type, long * param );

public:
							// Initializes the file system.
	static void				Init( );

							// Restarts the file system.
	static void				Restart( );

							// Shutdown the file system.
	static void				Shutdown( );

	static void				Register( );

							// true if file exists
	static bool				TouchFile( const Str & relativePath );

							// Returns true if the file system is initialized.
	//virtual bool			IsInitialized( ) const;

							// Returns true if we are doing an fs_copyfiles.
	//virtual bool			PerformingCopyFiles( ) const;

							// Lists files with the given extension in the given directory.
							// Directory should not have either a leading or trailing '/'
							// The returned files will not include any directories or '/' unless fullRelativePath is set.
							// The extension must include a leading dot and may not contain wildcards.
							// If extension is "/", only subdirectories will be returned.
	//virtual FileList *	ListFiles( const char *relativePath, const char *extension, bool sort = false, bool fullRelativePath = false, const char* gamedir = NULL );

							// Lists files in the given directory and all subdirectories with the given extension.
							// Directory should not have either a leading or trailing '/'
							// The returned files include a full relative path.
							// The extension must include a leading dot and may not contain wildcards.
	//virtual FileList *	ListFilesTree( const char *relativePath, const char *extension, bool sort = false, const char* gamedir = NULL );
	static List< Str >		ListFiles( const Str & findname, unsigned musthave, unsigned canthave );

							// Frees the given file list.
	//virtual void			FreeFileList( FileList *fileList );

							// Converts a relative path to a full OS path.
	static const Str		OSPathToRelativePath( const Str & OSPath );

							// Converts a full OS path to a relative path.
	static const Str		RelativePathToOSPath( const Str & relativePath );

							// Builds a full OS path from the given components.
	//virtual const char *	BuildOSPath( const char *base, const char *game, const char *relativePath );

							// Creates the given OS path for as far as it doesn't exist already.
	//virtual void			CreateOSPath( const char *OSPath );

							// Reads a complete file.
							// Returns the length of the file, or -1 on failure.
							// A null buffer will just return the file length without loading.
							// A null timestamp will be ignored.
							// As a quick check for existance. -1 length == not present.
							// A 0 byte will always be appended at the end, so string ops are safe.
							// The buffer should be considered read-only, because it may be cached for other uses.
	static File_Memory		ReadFile( const Str & relativePath );

							// Frees the memory allocated by ReadFile.
	static void				FreeFile( File_Memory & buffer );

							// Writes a complete file, will create any needed subdirectories.
							// Returns the length of the file, or -1 on failure.
	//virtual int				WriteFile( const char *relativePath, const void *buffer, int size, const char *basePath = "fs_savepath" );

							// Removes the given file.
	//virtual void			RemoveFile( const char *relativePath );

							// Opens a file for reading.
	static FileBase *		OpenFileRead( const Str & relativePath );

							// Opens a file for writing, will create any needed subdirectories.
	//virtual File *		OpenFileWrite( const char *relativePath, const char *basePath = "fs_savepath" );

							// Opens a file for writing at the end.
	//virtual File *		OpenFileAppend( const char *filename, bool sync = false, const char *basePath = "fs_basepath" );

							// Opens a file for reading, writing, or appending depending on the value of mode.
	static FileBase *		OpenFileByMode( const Str & relativePath, fsMode_t mode );

							// Opens a file for writing to a full OS path.
	//virtual File *		OpenExplicitFileWrite( const char *OSPath );

							// Closes a file.
	static void				CloseFile( FileBase *& f );

							// resets the bytes read counter
	//virtual void			ResetReadCount( );

							// retrieves the current read count
	//virtual int			GetReadCount( );

							// adds to the read count
	//virtual void			AddToReadCount( int c );

							// look for a dynamic module
	//virtual void			FindDLL( const char *basename, char dllPath[ MAX_OSPATH ], bool updateChecksum );

							// case sensitive filesystems use an internal directory cache
							// the cache is cleared when calling OpenFileWrite and RemoveFile
							// in some cases you may need to use this directly
	//virtual void			ClearDirCache( );

							// don't use for large copies - allocates a single memory block for the copy
	static void				FileCopy( const Str & fromOSPath, const Str & toOSPath );

	//virtual File *		MakeTemporaryFile( );

							// look for a file in the loaded paks or the addon paks
							// if the file is found in addons, FS's internal structures are ready for a reloadEngine
	//virtual findFile_t	FindFile( const char *path, bool scheduleAddons = false );

							// ignore case and seperator char distinctions
	//virtual bool			FilenameCompare( const char *s1, const char *s2 ) const;

	static void				ZipDir( );
	static void				Dir( );
};

// FIXME: Better place ?
template< class type >
void CAListBase< type >::SaveFile( const Str & file_name ) const {

	File listFile = FileSystem::OpenFileByMode( file_name, FS_WRITE );
	listFile.Write( d_list, d_num * sizeof( type ) );
	FileSystem::CloseFile( listFile );
}

template< class type >
void CAListBase< type >::LoadFile( const Str & file_name ) {

	File listFile = FileSystem::OpenFileByMode( file_name, FS_READ );
	size_t fileSize = listFile.Length( );
	assert( !( fileSize % sizeof( type ) ) );
	SetNum( fileSize % sizeof( type ) );
	listFile.Read( d_list, fileSize );
	FileSystem::CloseFile( listFile );
}

#endif /* !__FILESYSTEM_H__ */
