#ifndef MEMORY_H
#define MEMORY_H

class MemoryReader {

private:

	const byte *			d_data;
	size_t					d_size;
	size_t					d_offset;

public:

							MemoryReader( const void * data_ptr, size_t data_size );

	const byte &			operator[ ]( const size_t index ) const { assert( index < d_offset ); return d_data[ index ]; }
	
	size_t					Read( void * buffer, size_t size );
	size_t					Length( ) const	{ return d_size; }
	size_t					Tell( ) const	{ return d_offset; }
};

#if 0

class Memory2D {

public:

	typedef enum {

		align_ident,
		align_vertical,
		align_horizontal,
		align_maxnum

	} alignType_e;

							explicit Memory2D( const Bounds2D & b, bool flipped = false, alignType_e align = align_ident );

	bool					IsValid( ) const { return valid; }
	bool					IsFlipped( ) const { return flipped; }

							// determine flip status by image's bounds
	void					UpdateFlipped( const Bounds2D & imageBounds );

	const Bounds2D			GetBounds( ) const;
	Bounds2D				CutBoundsBy( const Bounds2D & imageBounds, alignType_e cutDirection ) const;

	alignType_e				GetAlign( ) const;

private:

	Bounds2D				bounds;
	alignType_e				align;
	bool					valid;
	bool					flipped;
	FrameTexture *	texture;
};

INLINE Memory2D::Memory2D( const Bounds2D & b, bool flipped, alignType_e align ) {

	this->bounds = b;
	this->align = align;
	this->valid = true;
	this->flipped = flipped;
}

INLINE void Memory2D::UpdateFlipped( const Bounds2D & imageBounds ) {

	Vec2 extents1 = bounds[ 1 ] - bounds[ 0 ];
	Vec2 extents2 = imageBounds[ 1 ] - imageBounds[ 0 ];

	flipped = ( extents1[ 0 ] < extents2[ 0 ] ) || ( extents1[ 1 ] < extents2[ 1 ] );
}

INLINE const Bounds2D Memory2D::GetBounds( ) const {

	return bounds;
}

INLINE Bounds2D Memory2D::CutBoundsBy( const Bounds2D & imageBounds, alignType_e cutDirection ) const {

	Bounds2D outBounds = this->bounds;
	Vec2 imageSize = imageBounds[ 1 ] - imageBounds[ 0 ]; // for translate bounds by image size

	switch( cutDirection ) {

		case align_ident:

			outBounds[ 0 ][ 0 ] += imageSize[ 0 ];
			outBounds[ 0 ][ 1 ] += imageSize[ 1 ];
			break;

		case align_vertical:

			outBounds[ 0 ][ 1 ] += imageSize[ 1 ];

			if( align != align_vertical )
				outBounds[ 1 ][ 0 ] = outBounds[ 0 ][ 0 ] + imageSize[ 0 ];

			break;

		case align_horizontal:

			outBounds[ 0 ][ 0 ] += imageSize[ 0 ];

			if( align != align_horizontal )
				outBounds[ 1 ][ 1 ] = outBounds[ 0 ][ 1 ] + imageSize[ 1 ];

			break;
	}

	assert( ( outBounds[ 0 ][ 0 ] <= outBounds[ 1 ][ 0 ] ) && ( outBounds[ 0 ][ 1 ] <= outBounds[ 1 ][ 1 ] ) );

	return outBounds;
}

INLINE Memory2D::alignType_e Memory2D::GetAlign( ) const {

	return align;
}

#endif

#endif
