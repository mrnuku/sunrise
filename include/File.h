// Copyright ( C) 2004 Id Software, Inc.
//

#ifndef __FILE_H__
#define __FILE_H__

/*
==============================================================

  File Streams.

==============================================================
*/

// mode parm for Seek
typedef enum {

	FS_SEEK_SET = SEEK_SET,
	FS_SEEK_CUR = SEEK_CUR,
	FS_SEEK_END = SEEK_END,

} fsOrigin_t;

typedef __int64 timeStamp_t;

typedef enum {

	FT_NONE,
	FT_STDIO,
	FT_ZIP,
	FT_RESOURCE

} fileType_t;

class FileBase : public NamedObject {

	friend class				FileSystem;

protected:

	Str							d_path;
	fileType_t					d_type;

public:

								FileBase( ) : d_type( FT_NONE ) { }

	/// Read data from the file to the buffer.
	virtual size_t				Read( void * buffer, size_t len ) = 0;

	/// Write data from the buffer to the file.
	virtual size_t				Write( const void * buffer, size_t len ) = 0;

	/// Returns the length of the file.
	virtual size_t				Length( ) = 0;

	/// Return a time value for reload operations.
	virtual timeStamp_t			Timestamp( ) = 0;

	/// Returns offset in file.
	virtual long				Tell( ) = 0;

	/// Causes any buffered data to be written to the file.
	virtual void				Flush( ) = 0;

	/// Seek on a file.
	virtual int					Seek( long offset, fsOrigin_t origin ) = 0;

	/// Go back to the beginning of the file.
	virtual void				Rewind( ) = 0;


	/// returns true if file opened
	bool						IsValid( ) const		{ return d_type != FT_NONE; }

	/// Get the full file path.
	const Str &					GetFullPath( ) const	{  return d_path; }

	/// Like fprintf.
	int							Printf( const char * fmt, ... ) id_attribute( ( format( printf, 2, 3) ) );

	/// Like fprintf but with argument pointer
	int							VPrintf( const char * fmt, va_list arg );

	/// Write a string with high precision floating point numbers to the file.
	int							WriteFloatString( const char *fmt, ... ) id_attribute( ( format( printf, 2, 3) ) );
	
	// Endian portable alternatives to Read( ...)
	//virtual int				ReadInt( int &value );
	//virtual int				ReadUnsignedInt( unsigned int &value );
	//virtual int				ReadShort( short &value );
	//virtual int				ReadUnsignedShort( unsigned short &value );
	//virtual int				ReadChar( char &value );
	//virtual int				ReadUnsignedChar( unsigned char &value );
	//virtual int				ReadFloat( float &value );
	//virtual int				ReadBool( bool &value );
	//virtual int				ReadString( Str &string );
	
	// Endian portable alternatives to Write( ...)
	//virtual int				WriteInt( const int value );
	//virtual int				WriteUnsignedInt( const unsigned int value );
	//virtual int				WriteShort( const short value );
	//virtual int				WriteUnsignedShort( unsigned short value );
	//virtual int				WriteChar( const char value );
	int							WriteByte( const byte & value );
	//virtual int				WriteFloat( const float value );
	//virtual int				WriteBool( const bool value );
	int 						WriteAMFString( const Str & string );
	//virtual int				WriteString( const Str & string );
};

class File_Stdio : public FileBase {

private:

	FILE *						d_handle;

public:

	virtual size_t				Read( void * buffer, size_t len );
	virtual size_t				Write( const void * buffer, size_t len );
	virtual size_t				Length( );
	virtual timeStamp_t			Timestamp( );
	virtual long				Tell( );
	virtual void				Flush( );
	virtual int					Seek( long offset, fsOrigin_t origin );
	virtual void				Rewind( );

	void						Stdio_Fopen( const Str & path, int mode );
	void						Stdio_Fclose( );
};

class File_Zip : public FileBase {

private:

	class ZipRecord *			d_zipRecord;
	ZLib::unzFile				d_zipHandle;
	class ZipFileSystem *		d_zipSystem;

public:

	virtual size_t				Read( void * buffer, size_t len );
	virtual size_t				Write( const void * buffer, size_t len );
	virtual size_t				Length( );
	virtual timeStamp_t			Timestamp( );
	virtual long				Tell( );
	virtual void				Flush( );
	virtual int					Seek( long offset, fsOrigin_t origin );
	virtual void				Rewind( );

	void						Zip_Fopen( ZipFileSystem * zip_sys, ZipRecord * zip_rec );
	void						Zip_Fclose( );
};

class File_Resource : public FileBase {

private:

	class ResourceRecord *		d_resourceRecord;
	WindowsNS::HGLOBAL			d_resourceObject;
	byte *						d_resourcePtr;
	size_t						d_resourceSize;
	long						d_offset;

public:

	virtual size_t				Read( void * buffer, size_t len );
	virtual size_t				Write( const void * buffer, size_t len );
	virtual size_t				Length( );
	virtual timeStamp_t			Timestamp( );
	virtual long				Tell( );
	virtual void				Flush( );
	virtual int					Seek( long offset, fsOrigin_t origin );
	virtual void				Rewind( );

	void						Resource_Fopen( ResourceRecord * res_rec );
	void						Resource_Fclose( );
};

class File_Memory {

	friend class			FileSystem;

public:

							File_Memory( ) { d_size = 0; d_filePtr = NULL; d_valid = false; }

							// returns true if file opened
	bool					IsValid( ) { return d_valid; }

							// Returns the length of the file.
	size_t					Length( ) { return d_size; }

							// returns const pointer to the memory buffer
	void *					GetDataPtr( ) { return d_filePtr; }

private:

	size_t					d_size;		// allocated size
	void *					d_filePtr;	// buffer holding the file data
	bool					d_valid;
};

#endif /* !__FILE_H__ */
