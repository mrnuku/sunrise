#include "precompiled.h"
#pragma hdrstop

TextureBase *						MaterialSystem::g_missingTexture = NULL;
CANamedList< Image *, 256 >			MaterialSystem::g_loadedImages;
CANamedList< TextureBase *, 256 >	MaterialSystem::g_loadedTextures;
CANamedList< Material *, 256 >		MaterialSystem::g_loadedMaterials;

// // //// // //// // //// //
// MemoryReader
// //// // //// // //// //

MemoryReader::MemoryReader( const void * data_ptr, size_t data_size ) {
	d_data		= ( byte * )data_ptr;
	d_size		= data_size;
	d_offset	= 0;
}
	
size_t MemoryReader::Read( void * buffer, size_t size ) {
	size_t readSize = Min( d_size - d_offset, size );
	Common::Com_Memcpy( buffer, d_data + d_offset, readSize );
	d_offset += readSize;
	return readSize;
}

// // //// // //// // //// //
// ImageBlock
// //// // //// // //// //

ImageBlock::ImageBlock( ImagePixel * points, int pointsLength ) {
	d_points = points;
	d_pointsLength = pointsLength;
	for( int i = 0; i < PIXEL_DIMENSIONS; i++ ) {
		d_minCorner.x[ i ] = 0;
		d_maxCorner.x[ i ] = 0xFF;
	}
}

int ImageBlock::LongestSideIndex( ) const {
	int m = d_maxCorner.x[ 0 ] - d_minCorner.x[ 0 ];
	int maxIndex = 0;
	for( int i = 1; i < PIXEL_DIMENSIONS; i++ ) {
		int diff = d_maxCorner.x[ i ] - d_minCorner.x[ i ];
		if( diff > m ) {
			m = diff;
			maxIndex = i;
		}
	}
	return maxIndex;
}

void ImageBlock::Shrink( ) {

	for( int j = 0; j < PIXEL_DIMENSIONS; j++ )
		d_minCorner.x[j] = d_maxCorner.x[j] = d_points[0].x[j];
	for( int i = 1; i < d_pointsLength; i++ ) {
		for( int j = 0; j<PIXEL_DIMENSIONS; j++ ) {
			d_minCorner.x[ j ] = Min( d_minCorner.x[ j ], d_points[ i ].x[ j ] );
			d_maxCorner.x[ j ] = Max( d_maxCorner.x[ j ], d_points[ i ].x[ j ] );
		}
	}
}

// // //// // //// // //// //
// ImagePalette
// //// // //// // //// //

ImagePalette::ImagePalette( const CAListBase< ImagePixel > & palette ) {
	assert( !( palette.Num( ) % 2 ) );
	SSE_Data	palMem;
	d_imageMem.Clear( );
	palMem.Clear( );
	for( int pi = 0; pi < palette.Num( ); pi += 2 ) {
		palMem.WriteBytes( 0, palette[ pi + 0 ].x, 3 );
		palMem.WriteBytes( 8, palette[ pi + 1 ].x, 3 );
		d_paletteMemList.Append( palMem );
	}
}

void ImagePalette::LookupLine( const byte * input, bool alpha, CAListBase< byte > & output ) {

	int channels	= alpha ? 4 : 3;
	int inputIndex, bestIndex, bestDist;
	SSE_Data absDifference;

	absDifference.Clear( );

	for( int i = 0; i < output.Num( ); i++ ) {
		inputIndex	= i * channels;
		bestDist	= 0xFFFFFF; // something big
		bestIndex	= -1;
		// load pixel data
		d_imageMem.WriteBytes( 0, &input[ inputIndex ], 3 );
		d_imageMem.WriteBytes( 8, &input[ inputIndex ], 3 );
		// lookup nearest palette ref
		for( int pi = 0; pi < d_paletteMemList.Num( ); pi++ ) {
			absDifference.AbsoluteDifference( d_imageMem, d_paletteMemList[ pi ] );
			if( absDifference.GetIntRef( 0 ) < bestDist ) {
				bestDist = absDifference.GetIntRef( 0 );
				bestIndex = ( pi * 2 ) + 0;
			}
			if( absDifference.GetIntRef( 2 ) < bestDist ) {
				bestDist = absDifference.GetIntRef( 2 );
				bestIndex = ( pi * 2 ) + 1;
			}
		}

		assert( bestIndex != -1 );
		output[ i ] = ( byte )bestIndex;
	}
}

// // //// // //// // //// //
// Image
// //// // //// // //// //

Image::Image( ) {
	d_size.Zero( );
	d_alpha				= false;
	d_dxt				= false;
	d_numMips			= 0;
	d_internalFormat	= 0;
	d_format			= 0;
}

Image::~Image( ) {
}

byte * Image::Alloc( int width, int height, bool alpha ) {
	d_data.SetNum( width * height * ( alpha ? 4 : 3 ) );
	d_size.Set( width, height );
	d_alpha		= alpha;
	d_dxt		= false;
	d_numMips	= 1;
	d_internalFormat	= alpha ? GL_RGBA8 : GL_RGB8;
	d_format			= alpha ? GL_RGBA : GL_RGB;
	return d_data.Ptr( );
}

bool Image::LoadTGA( const void * data_ptr, size_t data_size ) {

	const byte * buffer = ( byte * )data_ptr;
	short width = 0, height = 0;	// The dimensions of the image
	byte length = 0;				// The length in bytes to the pixels
	byte imageType = 0;	     		// The image type (RLE, RGB, Alpha...)

	byte bits = 0;	    			// The bits per pixel for the image (16, 24, 32)
	int channels = 0;				// The channels of the image (3 = RGA : 4 = RGBA)
	int stride = 0;					// The stride (channels * width)

	Common::Com_Memcpy( &length, buffer, sizeof( byte ) ); // Read in the length in bytes from the header to the pixel data
	buffer += 2;
	Common::Com_Memcpy( &imageType, buffer, sizeof( byte ) ); // Read in the imageType (RLE, RGB, etc...)
	buffer += 10;

	// Read the width, height and bits per pixel (16, 24 or 32)
	Common::Com_Memcpy( &width, buffer, sizeof( short ) );
	buffer += sizeof( short );
	Common::Com_Memcpy( &height, buffer, sizeof( short ) );
	buffer += sizeof( short );
	Common::Com_Memcpy( &bits, buffer, sizeof( byte ) );
	buffer += sizeof( byte );

    // see where and what we might need to flip
    bool flip_horz = ( buffer[ 0 ] & 16 ) == 16;
    bool flip_vert = !( ( buffer[ 0 ] & 32) == 32 );

	buffer += length + 1; // Now we move the file pointer to the pixel data

	if( imageType != 10 ) { // Check if the image is RLE compressed or not

		if( bits == 24 || bits == 32 ) { // Check if the image is a 24 or 32-bit image

			// Calculate the channels (3 or 4) - (use bits >> 3 for more speed).
			// Next, we calculate the stride and allocate enough memory for the pixels.
			channels = bits / 8;
			stride = channels * width;
			Alloc( width, height, channels == 4 );

			for( int y = 0; y < height; y++ ) { // Load in all the pixel data line by line

				byte * pLine = &d_data[ stride * y ]; // Store a pointer to the current line of pixels

				Common::Com_Memcpy( pLine, buffer, stride ); // Read in the current line of pixels
				buffer += stride;

				// Go through all of the pixels and swap the B and R values since TGA
				// files are stored as BGR instead of RGB (or use GL_BGR_EXT verses GL_RGB)
				for( int i = 0; i < stride; i += channels ) Swap( pLine[ i ], pLine[ i + 2 ] );
			}

		} else if( bits == 16 ) { // Check if the image is a 16 bit image (RGB stored in 1 unsigned short)

			unsigned short pixels = 0;

			// Since we convert 16-bit images to 24 bit, we hardcode the channels to 3.
			// We then calculate the stride and allocate memory for the pixels.
			channels = 3;
			stride = channels * width;
			Alloc( width, height, false );

			for( int i = 0; i < width * height; i++ ) { // Load in all the pixel data pixel by pixel

				Common::Com_Memcpy( &pixels, buffer, sizeof( unsigned short ) ); // Read in the current pixel
				buffer += sizeof( unsigned short );

				// To convert a 16-bit pixel into an R, G, B, we need to
				// do some masking and such to isolate each color value.
				// 0x1f = 11111 in binary, so since 5 bits are reserved in
				// each unsigned short for the R, G and B, we bit shift and mask
				// to find each value.  We then bit shift up by 3 to get the full color.
				d_data[ i * 3 + 0 ] = static_cast< byte >( ( ( pixels >> 10 ) & 0x1f ) << 3 );
				d_data[ i * 3 + 1 ] = static_cast< byte >( ( ( pixels >> 5 ) & 0x1f ) << 3 );
				d_data[ i * 3 + 2 ] = static_cast< byte >( ( pixels & 0x1f ) << 3 );
			}

		} else { // Else return a NULL for a bad or unsupported pixel format

			return false;
        }

	} else { // Else, it must be Run-Length Encoded (RLE)

		// Create some variables to hold the rleID, current colors read, channels, & stride.
		byte rleID = 0;
		int colorsRead = 0;
		channels = bits / 8;
		stride = channels * width;

		// Next we want to allocate the memory for the pixels and create an array,
		// depending on the channel count, to read in for each pixel.
		Alloc( width, height, channels == 4 );
		byte pColors[ 4 ];

		int i = 0;

		// Load in all the pixel data
		while( i < width * height ) {

			Common::Com_Memcpy( &rleID, buffer, sizeof( byte ) ); // Read in the current color count + 1
			buffer += sizeof( byte );

			if( rleID < 128 ) { // Check if we don't have an encoded string of colors

				rleID++; // Increase the count by 1

				while( rleID ) { // Go through and read all the unique colors found

					Common::Com_Memcpy( pColors, buffer, sizeof( byte ) * channels ); // Read in the current color
					buffer += sizeof( byte ) * channels;

					// Store the current pixel in our image array
					d_data[ colorsRead + 0 ] = pColors[ 2 ];
					d_data[ colorsRead + 1 ] = pColors[ 1 ];
					d_data[ colorsRead + 2 ] = pColors[ 0 ];

					if( bits == 32 ) d_data[ colorsRead + 3 ] = pColors[ 3 ]; // If we have a 4 channel 32-bit image, assign one more for the alpha

					// Increase the current pixels read, decrease the amount
					// of pixels left, and increase the starting index for the next pixel.
					i++;
					rleID--;
					colorsRead += channels;
				}

			} else { // Else, let's read in a string of the same character

				rleID -= 127; // Minus the 128 ID + 1 (127) to get the color count that needs to be read

				Common::Com_Memcpy( pColors, buffer, sizeof( byte ) * channels ); // Read in the current color, which is the same for a while
				buffer += sizeof( byte ) * channels;

				while( rleID ) { // Go and read as many pixels as are the same

					// Assign the current pixel to the current index in our pixel array
					d_data[ colorsRead + 0 ] = pColors[ 2 ];
					d_data[ colorsRead + 1 ] = pColors[ 1 ];
					d_data[ colorsRead + 2 ] = pColors[ 0 ];

					if( bits == 32 ) d_data[ colorsRead + 3 ] = pColors[ 3 ]; // If we have a 4 channel 32-bit image, assign one more for the alpha

					// Increase the current pixels read, decrease the amount
					// of pixels left, and increase the starting index for the next pixel.
					i++;
					rleID--;
					colorsRead += channels;
				}
			}
		}
	}

    // now we've read the data, do required flips so that origin is top-left
    if( flip_vert )
		Flip( );
    if( flip_horz )
		Flop( );

	return true;
}

bool Image::LoadPNG( const void * data_ptr, size_t data_size ) {

	if( !PngImports::IsAvailable( ) ) return false;

	MemoryReader		dataStream( data_ptr, data_size );
	Png::png_uint_32	width, height, pitch;
	int					bit_depth, color_type, interlace_type;

	Png::png_structp	png_ptr			= PngImports::png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	Png::png_infop		info_ptr		= PngImports::png_create_info_struct( png_ptr );

	PngImports::png_set_read_fn( png_ptr, &dataStream, PngImports::PngReadCallback );
	PngImports::png_read_info( png_ptr, info_ptr );

	PngImports::png_get_IHDR( png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL );

	d_alpha	= ( color_type & PNG_COLOR_MASK_ALPHA ) > 0;
	pitch		= width * ( d_alpha ? 4 : 3 );
	Alloc( width, height, d_alpha );

	PngImports::png_set_expand( png_ptr );
	PngImports::png_set_scale_16( png_ptr );

	if( interlace_type ) {

		Png::png_bytepp rowOffsets = ( Png::png_bytepp )Mem_Alloc( sizeof( Png::png_bytep ) * height );

		for( Png::png_uint_32 i = 0; i < height; i++ )
			rowOffsets[ i ] = d_data.Ptr( ) + ( i * pitch );

		PngImports::png_read_image( png_ptr, rowOffsets );
		Mem_Free( rowOffsets );

	} else {

		for( Png::png_uint_32 i = 0; i < height; i++ )
			PngImports::png_read_row( png_ptr, d_data.Ptr( ) + ( i * pitch ), NULL );
	}

	PngImports::png_read_end( png_ptr, info_ptr );
	PngImports::png_destroy_read_struct( &png_ptr, &info_ptr, NULL );
	return true;
}

bool Image::LoadDXT( const void * data_ptr, size_t data_size ) {

	ddsFileHeader_t * header = ( ddsFileHeader_t * )data_ptr;
	d_dxt = false;
	if( header->ddspf.dwFlags & DDSF_FOURCC ) {
		d_alpha = false;
		d_dxt = true;
		switch( header->ddspf.dwFourCC ) {
			case DDS_MAKEFOURCC( 'D', 'X', 'T', '1' ):
				d_internalFormat = ( header->ddspf.dwFlags & DDSF_ALPHAPIXELS ) ? GL_COMPRESSED_RGBA_S3TC_DXT1_EXT : GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
				break;
			case DDS_MAKEFOURCC( 'D', 'X', 'T', '3' ):
				d_internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
				break;
			case DDS_MAKEFOURCC( 'D', 'X', 'T', '5' ):
			case DDS_MAKEFOURCC( 'R', 'X', 'G', 'B' ):
				d_internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				break;
			default:
				return false;
		}
	} else if( ( header->ddspf.dwFlags & DDSF_RGBA ) && header->ddspf.dwRGBBitCount == 32 ) {
		d_alpha = true;
		d_format = GL_BGRA_EXT;
		d_internalFormat = GL_RGBA8;
	} else if( ( header->ddspf.dwFlags & DDSF_RGB ) && header->ddspf.dwRGBBitCount == 32 ) {
		d_alpha = true;
		d_format = GL_BGRA_EXT;
		d_internalFormat = GL_RGBA8;
	} else if( ( header->ddspf.dwFlags & DDSF_RGB ) && header->ddspf.dwRGBBitCount == 24 ) {
		d_alpha = false;
		d_format = GL_BGR_EXT;
		d_internalFormat = GL_RGB8;
	} else if( header->ddspf.dwRGBBitCount == 8 ) {
		d_alpha = true;
		d_format = GL_ALPHA;
		d_internalFormat = GL_ALPHA8;
	} else
		return false;
	d_size[ 0 ] = header->dwWidth;
	d_size[ 1 ] = header->dwHeight;
	d_numMips = ( header->dwFlags & DDSF_MIPMAPCOUNT ) ? header->dwMipMapCount : 1;
	//if( d_dxt ) {
	//	d_data.SetNum( data_size );
	//	Common::Com_Memcpy( d_data.Ptr( ), data_ptr, data_size );
	//} else {
		d_data.SetNum( ( int )( data_size - sizeof( ddsFileHeader_t ) ) );
		Common::Com_Memcpy( d_data.Ptr( ), ( ( byte * )data_ptr ) + sizeof( ddsFileHeader_t ), data_size - sizeof( ddsFileHeader_t ) );
	//}
	return true;
}

void Image::GLTexUpLoad( GLenum tex_target ) const {
	const byte * uploadPtr = d_data.Ptr( );
	VecT2i uploadSize = d_size;
	int dataSize;
	int channels	= d_dxt ? ( d_internalFormat <= GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ? 8 : 16 ) : d_alpha ? 4 : 3;
	for( int i = 0; i < d_numMips; i++ ) {
		assert( uploadSize[0]&&uploadSize[1] );
		if( d_dxt ) {
			dataSize = ((uploadSize[0]+3)/4) * ((uploadSize[1]+3)/4) * channels;
			GLImports::glCompressedTexImage2D( tex_target, i, d_internalFormat, uploadSize[ 0 ], uploadSize[ 1 ], 0, dataSize, uploadPtr );
		} else {
			dataSize = uploadSize[ 0 ] * uploadSize[ 1 ] * channels;
			GLImports::glTexImage2D( tex_target, i, d_internalFormat, uploadSize[ 0 ], uploadSize[ 1 ], 0, d_format, GL_UNSIGNED_BYTE, uploadPtr );
		}
		uploadPtr += dataSize;
		uploadSize *= 0.5f;
	}
	if( d_numMips > 1 ) {
		GLImports::glTexParameteri( tex_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		GLImports::glTexParameteri( tex_target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16 );
	}
}

void Image::MakePalette( CAListBase< ImagePixel > & result ) {

	int numPoints		= d_size[ 0 ] * d_size[ 1 ];
	int channels		= d_alpha ? 4 : 3;

	CAList< ImagePixel > dataTemp;

	if( !d_alpha ) dataTemp.Append( ( ImagePixel * )d_data.Ptr( ), numPoints );
	else { dataTemp.SetNum( numPoints ); for( int i = 0; i < numPoints; i++ ) Common::Com_Memcpy( &dataTemp[ i ], &d_data[ i * channels ], 3 ); }
	
	CAList< ImageBlock, 256 > blockList;
	blockList.Resize( result.NumAllocated( ) );

	ImageBlock initialBlock( dataTemp.Ptr( ), numPoints );
	initialBlock.Shrink( );
	blockList.Append( initialBlock );

	while( blockList.Num( ) < result.NumAllocated( ) && blockList[ blockList.Num( ) - 1 ].NumPoints( ) > 1 ) {

		ImageBlock longestBlock = blockList[ blockList.Num( ) - 1 ];
		blockList.SetNum( blockList.Num( ) - 1, false );

		ImagePixel * begin  = longestBlock.GetPoints( );
		ImagePixel * median = longestBlock.GetPoints( ) + ( longestBlock.NumPoints( ) + 1 ) / 2;
		ImagePixel * end    = longestBlock.GetPoints( ) + longestBlock.NumPoints( );

		switch( longestBlock.LongestSideIndex( ) ) {

			case 0: std::nth_element( begin, median, end, ImagePixelCoordComp< 0 >( ) ); break;
			case 1: std::nth_element( begin, median, end, ImagePixelCoordComp< 1 >( ) ); break;
			case 2: std::nth_element( begin, median, end, ImagePixelCoordComp< 2 >( ) ); break;
		}

		ImageBlock block1( begin, ( int )( median - begin ) ), block2( median, ( int )( end - median ) );
		block1.Shrink( );
		block2.Shrink( );

		blockList.Append( block1 );
		blockList.Append( block2 );
		blockList.Sort( );
	}

	for( int i = 0; i < blockList.Num( ); i++ ) {

		int sum[ PIXEL_DIMENSIONS ] = { 0 };
		ImageBlock block = blockList[ i ];
		ImagePixel * points = block.GetPoints( );

		for( int i = 0; i < block.NumPoints( ); i++ ) {
			for( int j = 0; j < PIXEL_DIMENSIONS; j++ ) sum[ j ] += points[ i ].x[ j ];
		}

		ImagePixel averagePoint;

		for( int j = 0; j < PIXEL_DIMENSIONS; j++ ) averagePoint.x[ j ] = sum[ j ] / block.NumPoints( );

		result.Append( averagePoint );
	}

	result.Sort( );
}

bool Image::SavePNG( const Str & file_name ) {
	assert( !d_dxt );
	if( !PngImports::IsAvailable( ) )
		return false;
	FileBase * output = FileSystem::OpenFileByMode( file_name, FS_WRITE );
	if( !output )
		return false;

	int width		= ( Png::png_uint_32 )d_size[ 0 ];
	int height		= ( Png::png_uint_32 )d_size[ 1 ];
	int pitch		= width * ( d_alpha ? 4 : 3 );

	CAList< ImagePixel, 256 > palette;
	MakePalette( palette );
	ImagePalette palRange( palette );

	Png::png_structp	png_ptr			= PngImports::png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	Png::png_infop		info_ptr		= PngImports::png_create_info_struct( png_ptr );

	PngImports::png_set_write_fn( png_ptr, output, PngImports::PngWriteCallback, PngImports::PngFlushCallback );

	PngImports::png_set_compression_level( png_ptr, Z_BEST_COMPRESSION );
	PngImports::png_set_compression_mem_level( png_ptr, 8 );
	PngImports::png_set_compression_strategy( png_ptr, Z_DEFAULT_STRATEGY );
	PngImports::png_set_compression_window_bits( png_ptr, 15 );
	PngImports::png_set_compression_method( png_ptr, 8 );
	PngImports::png_set_compression_buffer_size( png_ptr, 8192 );

	PngImports::png_set_IHDR( png_ptr, info_ptr, ( Png::png_uint_32 )width, ( Png::png_uint_32 )height, 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT );
	PngImports::png_set_PLTE( png_ptr, info_ptr, ( Png::png_const_colorp )palette.Ptr( ), 256 );
	PngImports::png_write_info( png_ptr, info_ptr );

	CAList< byte, 2048 > cache;
	cache.SetNum( width );

	for( int i = 0; i < height; i++ ) {

		palRange.LookupLine( &d_data[ i * pitch ], d_alpha, cache );
		PngImports::png_write_row( png_ptr, cache.Ptr( ) );
	}

	PngImports::png_write_end( png_ptr, info_ptr );
	PngImports::png_destroy_write_struct( &png_ptr, &info_ptr );
	FileSystem::CloseFile( output );
	return true;
}

bool Image::Load( const Str & filename ) {
	File_Memory		fm = FileSystem::ReadFile( filename );
	Str				ext;
	if( !fm.IsValid( ) ) {
		Common::Com_Printf( "Unable to load image: \"%s\"\n", filename.c_str( ) );
		return false;
	}
	d_name = filename;
	filename.ExtractFileExtension( ext );
	LoadMem( fm.GetDataPtr( ), fm.Length( ), ext );
	if( d_data.Num( ) == 1 )
		Common::Com_Printf( "Failed to load image file \"%s\"\n", filename.c_str( ) );
	FileSystem::FreeFile( fm );
	return ( d_data.Num( ) != 1 ) ? true : false;
}

bool Image::Save( const Str & filename ) {
	Str				ext;
	filename.ExtractFileExtension( ext );
	if( ext.IcmpFast( "png" ) )
		return SavePNG( filename );
	else
		Common::Com_Printf( "Unsupported image output: \"%s\"\n", ext.c_str( ) );
	return false;
}

void Image::LoadMem( const void * data_ptr, size_t data_size, const Str & image_type ) {
	if		( image_type.IcmpFast( "tga" ) )
		LoadTGA( data_ptr, data_size );
	else if	( image_type.IcmpFast( "png" ) )
		LoadPNG( data_ptr, data_size );
	else if	( image_type.IcmpFast( "dds" ) )
		LoadDXT( data_ptr, data_size );
	else
		Common::Com_Printf( "Corrupted/unsupported image format: \"%s\"\n", image_type.c_str( ) );
}

void Image::LoadRaw( const void * data_ptr, size_t data_size, const Str & image_type, const VecT2i & image_size, bool image_flip ) {
	bool alpha;
	if( image_type.IcmpFast( "rgba" ) )
		alpha = true;
	else if( image_type.IcmpFast( "rgb" ) )
		alpha = false;
	else {
		Common::Com_Printf( "Unknown image format: \"%s\"\n", image_type.c_str( ) );
		return;
	}
	assert( data_size == ( image_size[ 0 ] * image_size[ 1 ] * ( alpha ? 4 : 3 ) ) );
	Alloc( image_size[ 0 ], image_size[ 1 ], alpha );
	Common::Com_Memcpy( d_data.Ptr( ), data_ptr, data_size );
	d_size = image_size;
	if( image_flip )
		Flip( );
}

void Image::Crop( const BoundsT2i & region ) {
	assert( !d_dxt );
	BoundsT2i bounds;
	bounds.Zero( );
	bounds[ 1 ] = d_size;
	if( !bounds.ContainsPoint( region[ 0 ] ) )
		return;

	BoundsT2i cutRange		= region;

	if( !bounds.ContainsPoint( region[ 1 ] ) )
		cutRange[ 1 ] = d_size;
	if( cutRange.GetArea( ) == 0.0f )
		return;

	int width				= d_size[ 0 ];
	int cutSkipX			= cutRange[ 0 ][ 0 ];
	int cutSkipY			= cutRange[ 0 ][ 1 ];
	int cutEndY				= cutRange[ 1 ][ 1 ];
	int cutWidth			= cutRange[ 1 ][ 0 ] - cutSkipX;
	int cutHeight			= d_size[ 1 ] - cutSkipY;
	int channels			= d_alpha ? 4 : 3;
	int pitch				= width * channels;
	int cutPitch			= cutWidth * channels;
	byte * cutData			= ( byte * )Mem_Alloc( cutWidth * cutHeight * channels );

	for( int line = cutSkipY; line < cutEndY; line++ ) {
		int srcOffset = line * pitch + cutSkipX * channels;
		int dstOffest = ( line - cutSkipY ) * cutPitch;
		Common::Com_Memcpy( &cutData[ dstOffest ], &d_data[ srcOffset ], cutPitch );
	}

	d_data.SetNum( 0, true );
	d_data.Append( cutData, cutWidth * cutHeight * channels );
	d_size.Set( cutWidth, cutHeight );
}

void Image::Flip( ) {
	assert( !d_dxt );
	int pitch		= d_size[ 0 ] * ( d_alpha ? 4 : 3 );
	int height		= d_size[ 1 ];    
	for( int line = 0; line < height / 2; line++ ) {
		int srcOffset = line * pitch;
		int dstOffest = ( height - line - 1 ) * pitch;
		for( int colBit = 0; colBit < pitch; colBit++ )
			Swap( d_data[ srcOffset + colBit ], d_data[ dstOffest + colBit ] );
	}
}

void Image::Flop( ) {
	assert( !d_dxt );
	int width		= d_size[ 0 ];
	int height		= d_size[ 1 ];
	int channels	= d_alpha ? 4 : 3;
	int pitch		= width * channels;    
	for( int line = 0; line < height; line++ ) {
		int srcOffset = line * pitch;
		int dstOffest = srcOffset + pitch - channels;
		for( int pix = 0; pix < width / 2; pix++, srcOffset += channels, dstOffest -= channels )
			for( int c = 0; c < channels; c++ ) Swap( d_data[ srcOffset + c ], d_data[ dstOffest + c ] );
	}
}

void Image::ForceAlpha( ) {
	assert( !d_dxt );
	if( !d_alpha ) {
		int width		= d_size[ 0 ];
		int height		= d_size[ 1 ];
		byte * data		= ( byte * )Mem_Alloc( width * height * 4 );
		int buffer		= width * height * 3;
		int newI		= 0;    
		for( int i = 0 ; i < buffer ; ) {
			data[ newI++ ] = d_data[ i++ ];
			data[ newI++ ] = d_data[ i++ ];
			data[ newI++ ] = d_data[ i++ ];
			data[ newI++ ] = 0xff;
		}
		d_data.SetNum( 0, false );
		d_data.Append( data, width * height * 4 );
		d_alpha = true;
	}
}

// // //// // //// // //// //
// TextureBase
// //// // //// // //// //

TextureBase::TextureBase( const Str & name, const Str & tag, Image * image ) : GLTexture( GL_TEXTURE_2D ) {
	d_name	= name;
	d_tag	= tag;
	d_image	= image;
	LoadFromImage( image );
}

// // //// // //// // //// //
// MaterialSystem
// //// // //// // //// //

void MaterialSystem::Init( ) {
	g_missingTexture = FindTexture( "textures/common/shadernotex.tga", "map" );
	if( !g_missingTexture )
		Common::Com_Error( ERR_DROP, "Missing error texture" );
}

void MaterialSystem::Shutdown( ) {
	g_loadedImages.DeleteContents( true );
	g_loadedTextures.DeleteContents( true );
	g_loadedMaterials.DeleteContents( true );
}

Image * MaterialSystem::FindImage( const Str & filename ) {
	Image * img = g_loadedImages.FindByName( filename );
	if( img )
		return img;
	Image * newImage = new Image( );
	if( newImage->Load( filename ) ) {
		g_loadedImages.Append( newImage );
		return newImage;
	} else {
		delete newImage;
		return NULL;
	}
}

TextureBase * MaterialSystem::FindTexture( const Str & filename, const Str & tag ) {
	TextureBase * text = g_loadedTextures.FindByName( filename );
	if( text && text->GetTag( ) == tag )
		return text;
	Image * image = FindImage( filename );
	if( !image )
		return g_missingTexture;
	return LoadTexture( image, tag );
}

Material * MaterialSystem::FindMaterial( const Str & mat_name ) {
	Material * mat = g_loadedMaterials.FindByName( mat_name );
	if( mat )
		return mat;
	Lexer				lexer( LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );
	Token				token;
	int					braceDepth = 0;
	int					parsePhase = 0;
	Material *			newMat = NULL;
	TextureBase *		entryTexture;
	Str					entryClassName;

	lexer.LoadFile( "materials/mats.mtr" );

	while( 1 ) {
		if( lexer.EndOfFile( ) )
			break;
		if( !lexer.ReadToken( &token ) || braceDepth > 1 || braceDepth < 0 ) {
			Common::Com_Printf( "MaterialSystem::FindMaterial: Parser error\n", token.c_str( ) );
			if( newMat ) { delete newMat; newMat = NULL; }
			break;
		}
		if( token == '{' ) {
			braceDepth++;
			continue;
		}
		else if( token == '}' ) {
			braceDepth--;
			continue;
		}
		if( newMat ) {
			if( braceDepth ) {
				if( !parsePhase ) { // entry class name first
					entryClassName = token;
					if( entryClassName == "qer_editorimage" )
						parsePhase--;
					else
						parsePhase++;
				} else if( parsePhase == 1 ) {
					if( entryClassName == "shader" ) {
						GLShader * matShader = GLShader::FindByPath( token );
						if( !matShader )
							matShader = new GLShader( token );
						newMat->SetShader( matShader );
					} else {
						entryTexture = FindTexture( token, "map" );
						if( !entryTexture ) {
							Common::Com_Printf( "MaterialSystem::FindMaterial: No such texture \"%s\".\n", token.c_str( ) );
							entryTexture = g_missingTexture;
						}
						newMat->AddEntry( entryClassName, entryTexture );
					}
					parsePhase--;
				} else
					parsePhase++;
			} else
				break;
		} else if( !braceDepth && mat_name == token )
			newMat = new Material( token );
	}
	if( newMat ) {		
		if( !newMat->GetShader( ) )
			Common::Com_Error( ERR_DROP, "MaterialSystem::FindMaterial: Unresolved shader in material \"%s\"", mat_name.c_str( ) );
		g_loadedMaterials.Append( newMat );
	}
	return newMat;
}

TextureBase * MaterialSystem::LoadTexture( Image * image, const Str & tag ) {
	TextureBase * texture = new TextureBase( image->GetName( ), tag, image );
	g_loadedTextures.Append( texture );
	return texture;
}

// // //// // //// // //// //
// MaterialEntry
// //// // //// // //// //

MaterialEntry::MaterialEntry( const Str & entry_name, SVar & entry_svar, TextureBase * entry_text ) : d_entrySVar( entry_svar ) {
	d_name = entry_name;
	d_entryTexture = entry_text;
}

MaterialEntry::~MaterialEntry( ) {
}

// // //// // //// // //// //
// Material
// //// // //// // //// //

Material::Material( const Str & mat_name ) {
	d_name = mat_name;
	d_matShader = NULL;
}

Material::~Material( ) {
}

void Material::AddEntry( const Str & entry_name, TextureBase * entry_text ) {
	MaterialEntry * entry = d_entries.FindByName( entry_name );
	if( !entry ) {
		SVar & sVar = d_matShader ? d_matShader->FindInput( entry_name ) : SVar::Find( entry_name );
		MaterialEntry * newEntry = new MaterialEntry( entry_name, sVar, entry_text );
		d_entries.Append( newEntry );
	}
}

void Material::ConfigRenderSurface( RenderSurfaceMap * ren_surf ) {
	for( int i = 0; i < d_entries.Num( ); i++ ) {
		SVar * entrySVar = d_entries[ i ]->GetSVar( ).CreateParam( );
		*entrySVar = d_entries[ i ]->GetTexture( );
		ren_surf->AddParam( entrySVar );
	}
}

void Material::ConfigRenderSurface( RenderSurfaceModel * ren_surf ) {
	for( int i = 0; i < d_entries.Num( ); i++ ) {
		SVar * entrySVar = d_entries[ i ]->GetSVar( ).CreateParam( );
		*entrySVar = d_entries[ i ]->GetTexture( );
		ren_surf->AddParam( entrySVar );
	}
}

// // //// // //// // //// //
// DynamicTexture
// //// // //// // //// //

DynamicTexture::DynamicTexture( ) {
}

DynamicTexture::DynamicTexture( GLTexture * frame_texture, const RectTi & frame_rect ) : GLTexture( *frame_texture ) {
	d_rect		= frame_rect;
}

void DynamicTexture::SetupSplit( const VecT2i & frame_size, const VecT2i & image_size, CAListBase< RectTi > & remainders ) {

	VecT2i origSize		= d_rect.GetExtents( ) * 2.0f;
	d_90ccw				= ( origSize[0] < image_size[0] ) || ( origSize[1] < image_size[1] );
	VecT2i imgSize		= d_90ccw ? VecT2i( image_size[1], image_size[0] ) : image_size;
	VecT2i texExtents	= d_rect.GetExtents( ) - ( imgSize * 0.5f );
	VecT2i texOrigin	= d_rect.GetCenter( ) - texExtents;
	d_rect				= RectTi( texOrigin, d_rect.GetExtents( ) - texExtents );
	VecT2i fracOrigin	= texOrigin - d_rect.GetExtents( );
	if( !d_90ccw ) {
		d_relocationMatrix[ 0 ].Set( (float)imgSize[0]/(float)frame_size[0],	0.0f,															0.0f );
		d_relocationMatrix[ 1 ].Set( 0.0f,										(float)imgSize[1]/(float)frame_size[1],							0.0f );
		d_relocationMatrix[ 2 ].Set( (float)fracOrigin[0]/(float)frame_size[0],	1.0f-(float)(imgSize[1]+fracOrigin[1])/(float)frame_size[1],	1.0f );
	} else {
		d_relocationMatrix[ 0 ].Set( 0.0f,										(float)imgSize[0]/(float)frame_size[0],							0.0f );
		d_relocationMatrix[ 1 ].Set( (float)imgSize[1]/(float)frame_size[1],	0.0f,															0.0f );
		d_relocationMatrix[ 2 ].Set( (float)fracOrigin[0]/(float)frame_size[0],	1.0f-(float)(imgSize[0]+fracOrigin[1])/(float)frame_size[1],	1.0f );
	}
	if( texExtents[ 0 ] )
		remainders.Append( RectTi( texOrigin + VecT2i( d_rect.GetExtents( )[0] + texExtents[0], 0 ), VecT2i( texExtents[0], d_rect.GetExtents( )[1] ) ) );
	if( texExtents[ 1 ] )
		remainders.Append( RectTi( texOrigin + VecT2i( 0, d_rect.GetExtents( )[1] + texExtents[1] ), VecT2i( d_rect.GetExtents( )[0], texExtents[1] ) ) );
	if( texExtents[ 0 ] && texExtents[ 1 ] )
		remainders.Append( RectTi( texOrigin + d_rect.GetExtents( ) + texExtents, texExtents ) );
}

// // //// // //// // //// //
// Overlay
// //// // //// // //// //

Overlay::Overlay( ) {
	d_size.Set( 512, 512 );
	d_shader		= new GLShader( "shaders/ui.csp" );
	d_frameBuffer	= new GLFrameBuffer( "textureAllocator", d_size, true, 0 );
	d_frameBuffer->SingleSetup( "c0,rgba8", "color" );
	d_frameTexture	= d_frameBuffer->GetTextureBuffer( "color" );
	AddFreeTexture( RectTi( d_size * 0.5f, d_size * 0.5f ) );
	d_uploadTexture	= new GLTexture( GL_TEXTURE_2D );
	d_projectionMatrix.OrthogonalSelf( 0, (float)d_size[0], (float)d_size[1], 0, 0, 99 );
}

Overlay::~Overlay( ) {
	delete d_shader;
	delete d_frameBuffer;
	delete d_uploadTexture;
}

DynamicTexture * Overlay::LoadFromImage( Image * image ) {

	CAList< RectTi, 4 >							remainderRects;
	VecT2i										imgSize	= image->GetSize( );
	imgSize[0]	+= imgSize[0] % 2;	// because the image box's extents stored
	imgSize[1]	+= imgSize[1] % 2;
	RectTi										imageBox( vecT2i_origin, imgSize * 0.5f );
	DynamicTexture *							texture;
	node_t *									node;
	BufferPrimitiveUI *							textPrimitive;

	image->ForceAlpha( );
	for( node = d_freeAreas.Begin( ); !node->d_key.CanFit( imageBox ); node = d_freeAreas.GetNextLeaf( node ) ) {
		assert( node );
	}
	texture = node->d_object;
	d_freeAreas.Remove( node );
	texture->SetupSplit( d_size, imgSize, remainderRects );
	for( int i = 0; i < remainderRects.Num( ); i++ ) {
		AddFreeTexture( remainderRects[i] );
	}
	d_uploadTexture->LoadFromImage( image );
	d_shader->FindInput( "uiMatrix" ) = d_projectionMatrix;
	d_shader->FindInput( "uiTexture" ) = d_uploadTexture;
	d_frameBuffer->Bind( );
	d_frameBuffer->AdjustViewport( );
	textPrimitive = GUIThread::GetPrimitiveBuffer( )->TexturePrimitive( texture->GetRect( ), texture->Is90ccw( ) );
	d_shader->Bind( );
	d_shader->WriteSVars( );
	GUIThread::GetPrimitiveBuffer( )->BeginDraw( );
	textPrimitive->Draw( 1 );
	GUIThread::GetPrimitiveBuffer( )->EndDraw( );
	GUIThread::GetPrimitiveBuffer( )->FreePrimitive( textPrimitive );
	d_shader->UnBind( );
	d_frameBuffer->UnBind( );

	d_textureList.Append( texture );
	return texture;
}

DynamicTexture * Overlay::LoadFromFile( const Str & file_name ) {
	Image * image = MaterialSystem::FindImage( file_name );
	return LoadFromImage( image );
}

void Overlay::FreeTexture( DynamicTexture * texture ) {
	bool r = d_textureList.Remove( texture );
	assert( r );
	d_freeAreas.Add( texture, texture->GetRect( ) );
}
