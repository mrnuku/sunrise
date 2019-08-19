#ifndef ENDIANUTIL_H
#define ENDIANUTIL_H

enum ByteOrder {

	BO_LittleEndian,
	BO_BigEndian
};

#pragma pack ( push, 1 )

template< ByteOrder store_be > class Int16 {

private:

	byte						data[ 2 ];

public:

								Int16( ) { data[ 0 ] = data[ 1 ] = 0; }
								Int16( const int & value ) { SetInt( value ); }

	const byte &				operator[ ]( int index ) const;
	byte &						operator[ ]( int index );

	bool						IsNegative( ) const { return ( ( *this )[ 1 ] & 0x80 ) ? true : false; }

	Int16< store_be >			operator-( ) const { return Int16< store_be >( -GetInt( ) ); }
	Int16< store_be >			operator*( const int & value ) const { return Int16< store_be >( GetInt( ) * value ); }
	Int16< store_be >			operator/( const int & value ) const { return Int16< store_be >( GetInt( ) / value ); }
	Int16< store_be >			operator+( const int & value ) const { return Int16< store_be >( GetInt( ) + value ); }
	Int16< store_be >			operator-( const int & value ) const { return Int16< store_be >( GetInt( ) - value ); }
	Int16< store_be > &			operator+=( const int & value ) { SetInt( GetInt( ) + value ); return *this; }
	Int16< store_be > &			operator-=( const int & value ) { SetInt( GetInt( ) - value ); return *this; }
	Int16< store_be > &			operator/=( const int & value ) { SetInt( GetInt( ) / value ); return *this; }
	Int16< store_be > &			operator*=( const int & value ) { SetInt( GetInt( ) * value ); return *this; }

	void						SetInt( const int & value );
	int							GetInt( ) const;

	operator					int( ) const { return GetInt( ); }
	Int16< store_be > &			operator =( const int & value ) { SetInt( value ); return *this; }

	const byte *				Ptr( ) const { return data; }
	byte *						Ptr( ) { return data; }
};

template< ByteOrder store_be > class Int24 {

private:

	byte						data[ 3 ];

public:

								Int24( ) { data[ 0 ] = data[ 1 ] = data[ 2 ] = 0; }
								Int24( const int & value ) { SetInt( value ); }

	const byte &				operator[ ]( int index ) const;
	byte &						operator[ ]( int index );

	bool						IsNegative( ) const { return ( ( *this )[ 2 ] & 0x80 ) ? true : false; }

	Int24< store_be >			operator-( ) const { return Int24< store_be >( -GetInt( ) ); }
	Int24< store_be >			operator*( const int & value ) const { return Int24< store_be >( GetInt( ) * value ); }
	Int24< store_be >			operator/( const int & value ) const { return Int24< store_be >( GetInt( ) / value ); }
	Int24< store_be >			operator+( const int & value ) const { return Int24< store_be >( GetInt( ) + value ); }
	Int24< store_be >			operator-( const int & value ) const { return Int24< store_be >( GetInt( ) - value ); }
	Int24< store_be > &			operator+=( const int & value ) { SetInt( GetInt( ) + value ); return *this; }
	Int24< store_be > &			operator-=( const int & value ) { SetInt( GetInt( ) - value ); return *this; }
	Int24< store_be > &			operator/=( const int & value ) { SetInt( GetInt( ) / value ); return *this; }
	Int24< store_be > &			operator*=( const int & value ) { SetInt( GetInt( ) * value ); return *this; }

	void						SetInt( const int & value );
	int							GetInt( ) const;

	operator					int( ) const { return GetInt( ); }
	Int24< store_be > &			operator =( const int & value ) { SetInt( value ); return *this; }

	const byte *				Ptr( ) const { return data; }
	byte *						Ptr( ) { return data; }
};

template< ByteOrder store_be > class Int32 {

private:

	byte						data[ 4 ];

public:

								Int32( ) { data[ 0 ] = data[ 1 ] = data[ 2 ] = data[ 3 ] = 0; }
								Int32( const int & value ) { SetInt( value ); }

	const byte &				operator[ ]( int index ) const;
	byte &						operator[ ]( int index );

	bool						IsNegative( ) const { return ( ( *this )[ 3 ] & 0x80 ) ? true : false; }

	Int32< store_be >			operator-( ) const { return Int32< store_be >( -GetInt( ) ); }
	Int32< store_be >			operator*( const int & value ) const { return Int32< store_be >( GetInt( ) * value ); }
	Int32< store_be >			operator/( const int & value ) const { return Int32< store_be >( GetInt( ) / value ); }
	Int32< store_be >			operator+( const int & value ) const { return Int32< store_be >( GetInt( ) + value ); }
	Int32< store_be >			operator-( const int & value ) const { return Int32< store_be >( GetInt( ) - value ); }
	Int32< store_be > &			operator+=( const int & value ) { SetInt( GetInt( ) + value ); return *this; }
	Int32< store_be > &			operator-=( const int & value ) { SetInt( GetInt( ) - value ); return *this; }
	Int32< store_be > &			operator/=( const int & value ) { SetInt( GetInt( ) / value ); return *this; }
	Int32< store_be > &			operator*=( const int & value ) { SetInt( GetInt( ) * value ); return *this; }

	void						SetInt( const int & value );
	int							GetInt( ) const;

	operator					int( ) const { return GetInt( ); }
	Int32< store_be > &			operator =( const int & value ) { SetInt( value ); return *this; }

	const byte *				Ptr( ) const { return data; }
	byte *						Ptr( ) { return data; }
};

template< ByteOrder store_be > class Real64 {

private:

	byte						data[ 8 ];

public:

								Real64( ) { data[ 0 ] = data[ 1 ] = data[ 2 ] = data[ 3 ] = data[ 4 ] = data[ 5 ] = data[ 6 ] = data[ 7 ] = 0; }
								Real64( const double & value ) { SetDouble( value ); }

	const byte &				operator[ ]( int index ) const;
	byte &						operator[ ]( int index );

	void						SetDouble( const double & value );
	double						GetDouble( ) const;

	operator					double( ) const { return GetDouble( ); }
	Real64< store_be > &		operator =( const double & value ) { SetDouble( value ); return *this; }

	const byte *				Ptr( ) const { return data; }
	byte *						Ptr( ) { return data; }
};

#pragma pack ( pop )

template< > INLINE const byte & Int16< BO_LittleEndian >::operator[ ]( int index ) const {

	assert( index >= 0 && index < 2 );
	return data[ index ];
}

template< > INLINE const byte & Int16< BO_BigEndian >::operator[ ]( int index ) const {

	assert( index >= 0 && index < 2 );
	return data[ 1 - index ];
}

template< > INLINE byte & Int16< BO_LittleEndian >::operator[ ]( int index ) {

	assert( index >= 0 && index < 2 );
	return data[ index ];
}

template< > INLINE byte & Int16< BO_BigEndian >::operator[ ]( int index ) {

	assert( index >= 0 && index < 2 );
	return data[ 1 - index ];
}

template< ByteOrder store_be > INLINE void Int16< store_be >::SetInt( const int & value ) {

	( *this )[ 0 ] = ( ( byte * )&value )[ 0 ];
	( *this )[ 1 ] = ( ( byte * )&value )[ 1 ];
}

template< ByteOrder store_be > INLINE int Int16< store_be >::GetInt( ) const {

	byte bPtr[ 4 ];
	bPtr[ 0 ] = ( *this )[ 0 ];
	bPtr[ 1 ] = ( *this )[ 1 ];
	bPtr[ 2 ] = IsNegative( ) ? 0xFF : 0x00;
	bPtr[ 3 ] = IsNegative( ) ? 0xFF : 0x00;
	return *( int * )bPtr;
}

template< > INLINE const byte & Int24< BO_LittleEndian >::operator[ ]( int index ) const {

	assert( index >= 0 && index < 3 );
	return data[ index ];
}

template< > INLINE const byte & Int24< BO_BigEndian >::operator[ ]( int index ) const {

	assert( index >= 0 && index < 3 );
	return data[ 2 - index ];
}

template< > INLINE byte & Int24< BO_LittleEndian >::operator[ ]( int index ) {

	assert( index >= 0 && index < 3 );
	return data[ index ];
}

template< > INLINE byte & Int24< BO_BigEndian >::operator[ ]( int index ) {

	assert( index >= 0 && index < 3 );
	return data[ 2 - index ];
}

template< ByteOrder store_be > INLINE void Int24< store_be >::SetInt( const int & value ) {

	( *this )[ 0 ] = ( ( byte * )&value )[ 0 ];
	( *this )[ 1 ] = ( ( byte * )&value )[ 1 ];
	( *this )[ 2 ] = ( ( byte * )&value )[ 2 ];
}

template< ByteOrder store_be > INLINE int Int24< store_be >::GetInt( ) const {

	byte bPtr[ 4 ];
	bPtr[ 0 ] = ( *this )[ 0 ];
	bPtr[ 1 ] = ( *this )[ 1 ];
	bPtr[ 2 ] = ( *this )[ 2 ];
	bPtr[ 3 ] = IsNegative( ) ? 0xFF : 0x00;
	return *( int * )bPtr;
}

template< > INLINE const byte & Int32< BO_LittleEndian >::operator[ ]( int index ) const {

	assert( index >= 0 && index < 4 );
	return data[ index ];
}

template< > INLINE const byte & Int32< BO_BigEndian >::operator[ ]( int index ) const {

	assert( index >= 0 && index < 4 );
	return data[ 3 - index ];
}

template< > INLINE byte & Int32< BO_LittleEndian >::operator[ ]( int index ) {

	assert( index >= 0 && index < 4 );
	return data[ index ];
}

template< > INLINE byte & Int32< BO_BigEndian >::operator[ ]( int index ) {

	assert( index >= 0 && index < 4 );
	return data[ 3 - index ];
}

template< ByteOrder store_be > INLINE void Int32< store_be >::SetInt( const int & value ) {

	( *this )[ 0 ] = ( ( byte * )&value )[ 0 ];
	( *this )[ 1 ] = ( ( byte * )&value )[ 1 ];
	( *this )[ 2 ] = ( ( byte * )&value )[ 2 ];
	( *this )[ 3 ] = ( ( byte * )&value )[ 3 ];
}

template< ByteOrder store_be > INLINE int Int32< store_be >::GetInt( ) const {

	byte bPtr[ 4 ];
	bPtr[ 0 ] = ( *this )[ 0 ];
	bPtr[ 1 ] = ( *this )[ 1 ];
	bPtr[ 2 ] = ( *this )[ 2 ];
	bPtr[ 3 ] = ( *this )[ 3 ];
	return *( int * )bPtr;
}

template< > INLINE const byte & Real64< BO_LittleEndian >::operator[ ]( int index ) const {

	assert( index >= 0 && index < 8 );
	return data[ index ];
}

template< > INLINE const byte & Real64< BO_BigEndian >::operator[ ]( int index ) const {

	assert( index >= 0 && index < 8 );
	return data[ 7 - index ];
}

template< > INLINE byte & Real64< BO_LittleEndian >::operator[ ]( int index ) {

	assert( index >= 0 && index < 8 );
	return data[ index ];
}

template< > INLINE byte & Real64< BO_BigEndian >::operator[ ]( int index ) {

	assert( index >= 0 && index < 8 );
	return data[ 7 - index ];
}

template< ByteOrder store_be > INLINE void Real64< store_be >::SetDouble( const double & value ) {

	( *this )[ 0 ] = ( ( byte * )&value )[ 0 ];
	( *this )[ 1 ] = ( ( byte * )&value )[ 1 ];
	( *this )[ 2 ] = ( ( byte * )&value )[ 2 ];
	( *this )[ 3 ] = ( ( byte * )&value )[ 3 ];
	( *this )[ 4 ] = ( ( byte * )&value )[ 4 ];
	( *this )[ 5 ] = ( ( byte * )&value )[ 5 ];
	( *this )[ 6 ] = ( ( byte * )&value )[ 6 ];
	( *this )[ 7 ] = ( ( byte * )&value )[ 7 ];
}

template< ByteOrder store_be > INLINE double Real64< store_be >::GetDouble( ) const {

	byte bPtr[ 8 ];
	bPtr[ 0 ] = ( *this )[ 0 ];
	bPtr[ 1 ] = ( *this )[ 1 ];
	bPtr[ 2 ] = ( *this )[ 2 ];
	bPtr[ 3 ] = ( *this )[ 3 ];
	bPtr[ 4 ] = ( *this )[ 4 ];
	bPtr[ 5 ] = ( *this )[ 5 ];
	bPtr[ 6 ] = ( *this )[ 6 ];
	bPtr[ 7 ] = ( *this )[ 7 ];
	return *( double * )bPtr;
}

#endif
