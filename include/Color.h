#ifndef COLOR_H
#define COLOR_H

#define COLOR_BLUE 2
#define COLOR_GREEN 1
#define COLOR_RED 0
#define COLOR_ALPHA 3

#define COLOR_SRC_BLUE 2
#define COLOR_SRC_GREEN 1
#define COLOR_SRC_RED 0
#define COLOR_SRC_ALPHA 3

#define COLOR_DEFAULT_ALPHA_VALUE 255

#define COLOR_FRACTION ( 255.0f )
#define COLOR_ONEOVERFRACTION ( 1.0f / 256.0f )

class Color {

private:

	byte				data[ 4 ];

public:

						Color( );
						Color( dword color );
						Color( byte color_red, byte color_green, byte color_blue, byte color_alpha );
						Color( float color_red, float color_green, float color_blue, float color_alpha, bool clamp );
						Color( const Vec4 & color, bool normalize );
						Color( const Vec3 & color, bool normalize );

	Color				operator+( const Color & c2 ) const;
	Color				operator-( const Color & c2 ) const;
	Color				operator*( const float f ) const;

	operator			CEGUI::colour( ) const { return CEGUI::colour( *( ( CEGUI::argb_t * )data ) ); }

	void				Zero( );
	void				SetRed( byte color_red ) { data[ COLOR_RED ] = color_red; }
	void				SetGreen( byte color_green ) { data[ COLOR_GREEN ] = color_green; }
	void				SetBlue( byte color_blue ) { data[ COLOR_BLUE ] = color_blue; }
	void				SetAlpha( byte color_alpha ) { data[ COLOR_ALPHA ] = color_alpha; }

	void				Lerp( const Color & c1, const Color & c2, const float f );
};

extern Color color_white;
extern Color color_black;
extern Color color_darkgray;
extern Color color_gray;
extern Color color_lightgray;
extern Color color_transparent;
extern Color color_skyblue;
extern Color color_blue;
extern Color color_green;
extern Color color_red;
extern Color color_orange;
extern Color color_yellow;
extern Color color_cyan;
extern Color color_purple;

INLINE Color::Color( ) {

	data[ COLOR_BLUE ] = data[ COLOR_GREEN ] = data[ COLOR_RED ] = 0;
	data[ COLOR_ALPHA ] = COLOR_DEFAULT_ALPHA_VALUE;
}

INLINE Color::Color( dword color ) {

	data[ COLOR_BLUE ] = ( ( byte * )&color )[ COLOR_SRC_BLUE ];
	data[ COLOR_GREEN ] = ( ( byte * )&color )[ COLOR_SRC_GREEN ];
	data[ COLOR_RED ] = ( ( byte * )&color )[ COLOR_SRC_RED ];
	data[ COLOR_ALPHA ] = ( ( byte * )&color )[ COLOR_SRC_ALPHA ];
}

INLINE Color::Color( byte color_red, byte color_green, byte color_blue, byte color_alpha ) {

	data[ COLOR_BLUE ] = color_blue;
	data[ COLOR_GREEN ] = color_green;
	data[ COLOR_RED ] = color_red;
	data[ COLOR_ALPHA ] = color_alpha;
}

INLINE Color::Color( float color_red, float color_green, float color_blue, float color_alpha, bool clamp ) {

	if( clamp ) {

		Clamp( color_red, 0.0f, 1.0f );
		Clamp( color_green, 0.0f, 1.0f );
		Clamp( color_blue, 0.0f, 1.0f );
		Clamp( color_alpha, 0.0f, 1.0f );
	}

	data[ COLOR_BLUE ] = ( byte )( color_blue * COLOR_FRACTION );
	data[ COLOR_GREEN ] = ( byte )( color_green * COLOR_FRACTION );
	data[ COLOR_RED ] = ( byte )( color_red * COLOR_FRACTION );
	data[ COLOR_ALPHA ] = ( byte )( color_alpha * COLOR_FRACTION );
}

INLINE Color::Color( const Vec4 & color, bool normalize ) {

	Vec4 colorVector = color;
	if( normalize ) colorVector.Normalize( );

	data[ COLOR_BLUE ] = ( byte )( colorVector[ COLOR_SRC_BLUE ] * COLOR_FRACTION );
	data[ COLOR_GREEN ] = ( byte )( colorVector[ COLOR_SRC_GREEN ] * COLOR_FRACTION );
	data[ COLOR_RED ] = ( byte )( colorVector[ COLOR_SRC_RED ] * COLOR_FRACTION );
	data[ COLOR_ALPHA ] = ( byte )( colorVector[ COLOR_SRC_ALPHA ] * COLOR_FRACTION );
}

INLINE Color::Color( const Vec3 & color, bool normalize ) {

	Vec3 colorVector = color;
	if( normalize ) colorVector.Normalize( );

	data[ COLOR_BLUE ] = ( byte )( colorVector[ COLOR_SRC_BLUE ] * COLOR_FRACTION );
	data[ COLOR_GREEN ] = ( byte )( colorVector[ COLOR_SRC_GREEN ] * COLOR_FRACTION );
	data[ COLOR_RED ] = ( byte )( colorVector[ COLOR_SRC_RED ] * COLOR_FRACTION );
	data[ COLOR_ALPHA ] = COLOR_DEFAULT_ALPHA_VALUE;
}

INLINE Color Color::operator+( const Color & c2 ) const {

	return Color(	data[ COLOR_RED ] + c2.data[ COLOR_RED ],
					data[ COLOR_GREEN ] + c2.data[ COLOR_GREEN ],
					data[ COLOR_BLUE ] + c2.data[ COLOR_BLUE ],
					data[ COLOR_ALPHA ] + c2.data[ COLOR_ALPHA ] );
}

INLINE Color Color::operator-( const Color & c2 ) const {

	return Color(	data[ COLOR_RED ] - c2.data[ COLOR_RED ],
					data[ COLOR_GREEN ] - c2.data[ COLOR_GREEN ],
					data[ COLOR_BLUE ] - c2.data[ COLOR_BLUE ],
					data[ COLOR_ALPHA ] - c2.data[ COLOR_ALPHA ] );
}

INLINE Color Color::operator*( const float f ) const {

	return Color(	( byte )( data[ COLOR_RED ] * f ),
					( byte )( data[ COLOR_GREEN ] * f ),
					( byte )( data[ COLOR_BLUE ] * f ),
					( byte )( data[ COLOR_ALPHA ] * f ) );
}

INLINE void Color::Zero( ) {

	data[ COLOR_BLUE ] = data[ COLOR_GREEN ] = data[ COLOR_RED ] = data[ COLOR_ALPHA ] = 0;
}

INLINE void Color::Lerp( const Color & c1, const Color & c2, const float f ) {

	( *this ) = c1 + ( ( c2 - c1 ) * f );
}

#endif
