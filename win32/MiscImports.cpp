#include "precompiled.h"
#pragma hdrstop

DynamicModule *							ZLibImports::zlibModule;

ZLIBINFLATEINIT2_						ZLibImports::zlInflateInit2_;
ZLIBINFLATE								ZLibImports::zlInflate;
ZLIBINFLATEEND							ZLibImports::zlInflateEnd;
ZLIBCRC32								ZLibImports::zlCrc32;

void ZLibImports::Init( ) {

	zlibModule					= new DynamicModule( "zlibwapi.dll" );

	zlInflateInit2_				= ( ZLIBINFLATEINIT2_ )				zlibModule->FindProc( "inflateInit2_" );
	zlInflate					= ( ZLIBINFLATE )					zlibModule->FindProc( "inflate" );
	zlInflateEnd				= ( ZLIBINFLATEEND )				zlibModule->FindProc( "inflateEnd" );
	zlCrc32						= ( ZLIBCRC32 )						zlibModule->FindProc( "crc32" );
}

void ZLibImports::Shutdown( ) {

	delete zlibModule;
}

#ifndef DEDICATED_ONLY

DynamicModule *							PngImports::pngModule = NULL;

PNG_CREATE_READ_STRUCT_PROC				PngImports::png_create_read_struct;
PNG_CREATE_INFO_STRUCT_PROC				PngImports::png_create_info_struct;
PNG_SET_READ_FN_PROC					PngImports::png_set_read_fn;
PNG_READ_INFO_PROC						PngImports::png_read_info;
PNG_GET_IHDR_PROC						PngImports::png_get_IHDR;
PNG_SET_EXPAND_PROC						PngImports::png_set_expand;
PNG_SET_SCALE_16_PROC					PngImports::png_set_scale_16;
PNG_READ_IMAGE_PROC						PngImports::png_read_image;
PNG_READ_ROW_PROC						PngImports::png_read_row;
PNG_READ_END_PROC						PngImports::png_read_end;
PNG_DESTROY_READ_STRUCT_PROC			PngImports::png_destroy_read_struct;
PNG_CREATE_WRITE_STRUCT_PROC			PngImports::png_create_write_struct;
PNG_SET_WRITE_FN_PROC					PngImports::png_set_write_fn;
PNG_SET_COMPRESSION_LEVEL_PROC			PngImports::png_set_compression_level;
PNG_SET_COMPRESSION_MEM_LEVEL_PROC		PngImports::png_set_compression_mem_level;
PNG_SET_COMPRESSION_STRATEGY_PROC		PngImports::png_set_compression_strategy;
PNG_SET_COMPRESSION_WINDOW_BITS_PROC	PngImports::png_set_compression_window_bits;
PNG_SET_COMPRESSION_METHOD_PROC			PngImports::png_set_compression_method;
PNG_SET_COMPRESSION_BUFFER_SIZE_PROC	PngImports::png_set_compression_buffer_size;
PNG_SET_IHDR_PROC						PngImports::png_set_IHDR;
PNG_SET_PLTE_PROC						PngImports::png_set_PLTE;
PNG_WRITE_INFO_PROC						PngImports::png_write_info;
PNG_WRITE_ROW_PROC						PngImports::png_write_row;
PNG_WRITE_END_PROC						PngImports::png_write_end;
PNG_DESTROY_WRITE_STRUCT_PROC			PngImports::png_destroy_write_struct;
PNG_ERROR_PROC							PngImports::png_error;
PNG_GET_IO_PTR_PROC						PngImports::png_get_io_ptr;

void PngImports::Init( ) {

	pngModule						= new DynamicModule( "libpng15.dll" );

	png_create_read_struct			= ( PNG_CREATE_READ_STRUCT_PROC )			pngModule->FindProc( "png_create_read_struct" );
	png_create_info_struct			= ( PNG_CREATE_INFO_STRUCT_PROC )			pngModule->FindProc( "png_create_info_struct" );
	png_set_read_fn					= ( PNG_SET_READ_FN_PROC )					pngModule->FindProc( "png_set_read_fn" );
	png_read_info					= ( PNG_READ_INFO_PROC )					pngModule->FindProc( "png_read_info" );
	png_get_IHDR					= ( PNG_GET_IHDR_PROC )						pngModule->FindProc( "png_get_IHDR" );
	png_set_expand					= ( PNG_SET_EXPAND_PROC )					pngModule->FindProc( "png_set_expand" );
	png_set_scale_16				= ( PNG_SET_SCALE_16_PROC )					pngModule->FindProc( "png_set_scale_16" );
	png_read_image					= ( PNG_READ_IMAGE_PROC )					pngModule->FindProc( "png_read_image" );
	png_read_row					= ( PNG_READ_ROW_PROC )						pngModule->FindProc( "png_read_row" );
	png_read_end					= ( PNG_READ_END_PROC )						pngModule->FindProc( "png_read_end" );
	png_destroy_read_struct			= ( PNG_DESTROY_READ_STRUCT_PROC )			pngModule->FindProc( "png_destroy_read_struct" );
	png_create_write_struct			= ( PNG_CREATE_WRITE_STRUCT_PROC )			pngModule->FindProc( "png_create_write_struct" );
	png_set_write_fn				= ( PNG_SET_WRITE_FN_PROC )					pngModule->FindProc( "png_set_write_fn" );
	png_set_compression_level		= ( PNG_SET_COMPRESSION_LEVEL_PROC )		pngModule->FindProc( "png_set_compression_level" );
	png_set_compression_mem_level	= ( PNG_SET_COMPRESSION_MEM_LEVEL_PROC )	pngModule->FindProc( "png_set_compression_mem_level" );
	png_set_compression_strategy	= ( PNG_SET_COMPRESSION_STRATEGY_PROC )		pngModule->FindProc( "png_set_compression_strategy" );
	png_set_compression_window_bits	= ( PNG_SET_COMPRESSION_WINDOW_BITS_PROC )	pngModule->FindProc( "png_set_compression_window_bits" );
	png_set_compression_method		= ( PNG_SET_COMPRESSION_METHOD_PROC )		pngModule->FindProc( "png_set_compression_method" );
	png_set_compression_buffer_size	= ( PNG_SET_COMPRESSION_BUFFER_SIZE_PROC )	pngModule->FindProc( "png_set_compression_buffer_size" );
	png_set_IHDR					= ( PNG_SET_IHDR_PROC )						pngModule->FindProc( "png_set_IHDR" );
	png_set_PLTE					= ( PNG_SET_PLTE_PROC )						pngModule->FindProc( "png_set_PLTE" );
	png_write_info					= ( PNG_WRITE_INFO_PROC )					pngModule->FindProc( "png_write_info" );
	png_write_row					= ( PNG_WRITE_ROW_PROC )					pngModule->FindProc( "png_write_row" );
	png_write_end					= ( PNG_WRITE_END_PROC )					pngModule->FindProc( "png_write_end" );
	png_destroy_write_struct		= ( PNG_DESTROY_WRITE_STRUCT_PROC )			pngModule->FindProc( "png_destroy_write_struct" );
	png_error						= ( PNG_ERROR_PROC )						pngModule->FindProc( "png_error" );
	png_get_io_ptr					= ( PNG_GET_IO_PTR_PROC )					pngModule->FindProc( "png_get_io_ptr" );
}

void PngImports::Shutdown( ) {

	delete pngModule;
	pngModule = NULL;
}

bool PngImports::IsAvailable( ) {

	if( !pngModule ) Init( );
	return pngModule ? true : false;
}

void PngImports::PngReadCallback( Png::png_structp png_ptr, Png::png_bytep data, Png::png_size_t length ) {

	MemoryReader * reader = ( MemoryReader * )PngImports::png_get_io_ptr( png_ptr );
	if( !reader->Read( data, length ) ) PngImports::png_error( png_ptr, "PngReadCallback: MemoryReader out of data" );
}

void PngImports::PngWriteCallback( Png::png_structp png_ptr, Png::png_bytep data, Png::png_size_t length ) {

	FileBase * writer = ( FileBase * )PngImports::png_get_io_ptr( png_ptr );
	writer->Write( data, length );
}

void PngImports::PngFlushCallback( Png::png_structp png_ptr ) {

	FileBase * writer = ( FileBase * )PngImports::png_get_io_ptr( png_ptr );
	writer->Flush( );
}

DynamicModule *					DWImports::dwModule;
WindowsNS::DWRITECREATEFACTORY	DWImports::DWriteCreateFactory;
WindowsNS::IDWriteFactory *		DWImports::DWriteFactory;
WindowsNS::GUID					DWImports::GUIDDWrite = { 0xb859ee5aL, 0xd838, 0x4b5b, 0xa2, 0xe8, 0x1a, 0xdc, 0x7d, 0x93, 0xdb, 0x48 };
DWRenderer						DWImports::dwRenderer;

void DWImports::Init( ) {

	dwModule					= new DynamicModule( "dwrite.dll" );

	DWriteCreateFactory			= ( WindowsNS::DWRITECREATEFACTORY )		dwModule->FindProc( "DWriteCreateFactory" );

	DWriteCreateFactory( WindowsNS::DWRITE_FACTORY_TYPE_SHARED, GUIDDWrite, reinterpret_cast< WindowsNS::IUnknown ** >( &DWriteFactory ) );
}

void DWImports::Shutdown( ) {

	DWriteFactory->Release( );
	delete dwModule;
}

DWFontRenderer::DWFontRenderer( const Str & font_name, float font_pts ) {

	DWImports::DWriteFactory->CreateTextFormat( WStr( font_name ), NULL, WindowsNS::DWRITE_FONT_WEIGHT_DEMI_BOLD, WindowsNS::DWRITE_FONT_STYLE_NORMAL,
		WindowsNS::DWRITE_FONT_STRETCH_ULTRA_CONDENSED, font_pts, L"en-EN", &d_textFormat );

	d_textFormat->GetFontCollection( &d_fontCollection );
	d_fontCollection->GetFontFamily( 0, &d_fontFamily );
	d_fontFamily->GetFont( 0, &d_fontInternal );
	d_fontInternal->GetMetrics( &d_fontMetrics );
	d_fontPts = font_pts;
	d_fontEmDivisor = ( d_fontPts / 72.0f ) * 96.0f;

	Vec2 fontBounds( font_pts, d_fontEmDivisor );
	fontBounds.Ceil( );
	d_bmpWidth = ( int )fontBounds[ 0 ];
	d_bmpHeight = ( int )fontBounds[ 1 ];

	DWImports::DWriteFactory->GetGdiInterop( &d_gdiInterop );
	d_gdiInterop->CreateBitmapRenderTarget( WindowsNS::GetDC( NULL ), d_bmpWidth, d_bmpHeight, &d_renderTarget );
	DWImports::DWriteFactory->CreateRenderingParams( &d_renderParams );
	d_renderTarget->SetCurrentTransform( NULL );
	d_renderer.SetTarget( d_renderTarget );
	d_renderer.SetParams( d_renderParams );
 
	d_hdcMem = d_renderTarget->GetMemoryDC( );
	d_hdcMemComp = WindowsNS::CreateCompatibleDC( d_hdcMem );
	d_hbmMemComp = WindowsNS::CreateCompatibleBitmap( d_hdcMem, d_bmpWidth, d_bmpHeight );
	WindowsNS::SelectObject( d_hdcMemComp, d_hbmMemComp );
	WindowsNS::GetObject( d_hbmMemComp, sizeof( WindowsNS::BITMAP ), &d_bmpMemComp );
	d_brush = WindowsNS::CreateSolidBrush( 0 );
	WindowsNS::SelectObject( d_hdcMem, d_brush );
}

DWFontRenderer::~DWFontRenderer( ) {

	WindowsNS::DeleteObject( d_brush );
	WindowsNS::DeleteObject( d_hbmMemComp );
	WindowsNS::ReleaseDC( NULL, d_hdcMemComp );

	d_fontInternal->Release( );
	d_fontFamily->Release( );
	d_fontCollection->Release( );

	d_renderTarget->Release( );
	d_gdiInterop->Release( );
	d_textFormat->Release( );

	for( int i = 0; i < d_imageSets.Num( ); i++ )
		CEGUI::ImagesetManager::getSingleton( ).destroy( d_imageSets[ i ]->getName( ) );

	//d_imageSets.DeleteContents( true );
	d_imageSets.Clear( );
}

CEGUI::utf32 DWFontRenderer::GetCodepointMap( std::map< CEGUI::utf32, CEGUI::FontGlyph > & codepoint_map ) {

	CEGUI::utf32 maxCodepoint = 0;

	for( CEGUI::utf32 i = 0; i < 65536; i++ ) {

		WindowsNS::BOOL exists;
		d_fontInternal->HasCharacter( i, &exists );

		if( exists ) {

			codepoint_map[ i ] = CEGUI::FontGlyph( );
			maxCodepoint = i;
		}
	}

	return maxCodepoint;
}

void DWFontRenderer::ResizeDIB( WindowsNS::DWRITE_TEXT_METRICS & glyph_metrics, byte * image_buffer ) {

	byte * bmpBuffer = ( byte * )d_bmpMemComp.bmBits;
	Vec2 ceilSize( glyph_metrics.width, glyph_metrics.height );
	ceilSize.Ceil( );
	int glyphHeight = ( int )( glyph_metrics.height = ceilSize[ 1 ] );
	int glyphWidth = ( int )( glyph_metrics.width = ceilSize[ 0 ] );

	for( int y = 0; y < glyphHeight; y++ ) {

		int bmpIndex = ( d_bmpHeight - ( y + 1 ) ) * d_bmpMemComp.bmWidthBytes;

		for( int x = 0; x < glyphWidth; x++, bmpIndex += 4 ) {

			image_buffer[ ( y * glyphWidth + x ) * 4 + 0 ] = bmpBuffer[ bmpIndex + 0 ];
			image_buffer[ ( y * glyphWidth + x ) * 4 + 1 ] = bmpBuffer[ bmpIndex + 1 ];
			image_buffer[ ( y * glyphWidth + x ) * 4 + 2 ] = bmpBuffer[ bmpIndex + 2 ];
			image_buffer[ ( y * glyphWidth + x ) * 4 + 3 ] = ( bmpBuffer[ bmpIndex + 0 ] + bmpBuffer[ bmpIndex + 1 ] + bmpBuffer[ bmpIndex + 2 ] ) / 3;
		}
	}

}

void DWFontRenderer::Render( const CEGUI::String & font_name, const std::map< CEGUI::utf32, CEGUI::FontGlyph > & codepoint_map, CEGUI::utf32 start_codepoint, CEGUI::utf32 end_codepoint ) {

	WindowsNS::IDWriteTextLayout * textLayout;
	WindowsNS::DWRITE_TEXT_METRICS textMetrics;

	byte * imageBuffer = ( byte * )Mem_Alloc( d_bmpWidth * d_bmpHeight * 4 );

	std::map< CEGUI::utf32, CEGUI::FontGlyph >::const_iterator itr = codepoint_map.lower_bound( start_codepoint );
	std::map< CEGUI::utf32, CEGUI::FontGlyph >::const_iterator endItr = codepoint_map.upper_bound( end_codepoint );

	while( itr != codepoint_map.end( ) ) {

		CEGUI::Imageset * imageSet = &CEGUI::ImagesetManager::getSingleton( ).create( font_name + "_glyph_" + Str( int( itr->first ) ).c_str( ), CEGUI::System::getSingleton( ).getRenderer( )->createTexture( ) );
		d_imageSets.Append( imageSet );

		WindowsNS::HRESULT res = DWImports::DWriteFactory->CreateTextLayout( ( wchar_t * )&itr->first, 1, d_textFormat, ( float )d_bmpWidth, ( float )d_bmpHeight, &textLayout );
		res = textLayout->GetMetrics( &textMetrics );

		textMetrics.width = Min( textMetrics.width, ( float )d_bmpWidth );
		textMetrics.height = Min( textMetrics.height, ( float )d_bmpHeight );

		bool zeroWidth = textMetrics.width == 0.0f;

		if( !zeroWidth ) {

			WindowsNS::PatBlt( d_hdcMem, 0, 0, d_bmpWidth, d_bmpHeight, 0x00F00021 );
			WindowsNS::HRESULT res = textLayout->Draw( NULL, reinterpret_cast< WindowsNS::IDWriteTextRenderer * >( &d_renderer ), 0.0f, 0.0f );
			WindowsNS::BitBlt( d_hdcMemComp, 0, 0, d_bmpWidth, d_bmpHeight, d_hdcMem, 0, 0, 0x00CC0020 );
			ResizeDIB( textMetrics, imageBuffer );
		}

		CEGUI::Rect area( 0, 0, zeroWidth ? 0 : textMetrics.width, zeroWidth ? 0 : textMetrics.height );
		CEGUI::Point offset( 0, zeroWidth ? 0 : -( textMetrics.height + GetDescender( ) ) );
		CEGUI::String name;
		name += itr->first;
		imageSet->defineImage( name, area, offset );
		( ( CEGUI::FontGlyph & )itr->second ).setImage( &imageSet->getImage( name ) );
		( ( CEGUI::FontGlyph & )itr->second ).setAdvance( ceil( textMetrics.widthIncludingTrailingWhitespace ) );

		if( !zeroWidth )
			imageSet->getTexture( )->loadFromMemory( imageBuffer, CEGUI::Size( textMetrics.width, textMetrics.height ), CEGUI::Texture::PF_RGBA );
		//reinterpret_cast< CEGUI::GUITexture * >( imageSet->getTexture( ) )->Name( name.c_str( ) );

		textLayout->Release( );

		itr++;
		if( itr == endItr ) break;
	}

	Mem_Free( imageBuffer );
}

void DWImports::RenderFont( const Str & font_name, float font_pts, Bounds2D & font_bounds, int char_start, int char_range, class Image ** char_set_ptr ) {

	WindowsNS::IDWriteTextFormat *			textFormat;
	List< WindowsNS::IDWriteTextLayout * >	textLayouts;
	WindowsNS::IDWriteGdiInterop *			gdiInterop;
	WindowsNS::IDWriteBitmapRenderTarget *	renderTarget;
	WindowsNS::IDWriteRenderingParams *		renderParams;
	WindowsNS::DWRITE_TEXT_METRICS			textMetrics;
	WindowsNS::BITMAP						bmpMemComp;
	WindowsNS::HDC							hdcMem, hdcMemComp;
	WindowsNS::HBITMAP						hbmMemComp;
	WindowsNS::HBRUSH						bBrush;
	WStr									faceName = font_name, charName;
	int										bmpWidth, bmpHeight, charEnd = char_start + char_range;
	Str										imageName;
	byte									*imageBuffer, *bmpBuffer;

	DWriteFactory->CreateTextFormat( faceName, NULL, WindowsNS::DWRITE_FONT_WEIGHT_DEMI_BOLD, WindowsNS::DWRITE_FONT_STYLE_NORMAL,
		WindowsNS::DWRITE_FONT_STRETCH_ULTRA_CONDENSED, font_pts, L"en-EN", &textFormat );

	textLayouts.SetNum( char_range );
	font_bounds.Zero( );
	font_bounds[ 1 ][ 0 ] = font_pts;
	for( int i = 0; i < char_range; i++ ) {

		charName = ( char )( i + char_start );

		DWriteFactory->CreateTextLayout( charName, 1, textFormat, 64.0f, 64.0f, &textLayouts[ i ] );
		textLayouts[ i ]->GetMetrics( &textMetrics );

		if( textMetrics.width > font_bounds[ 1 ][ 0 ] ) font_bounds[ 1 ][ 0 ] = textMetrics.width;
		if( textMetrics.height > font_bounds[ 1 ][ 1 ] ) font_bounds[ 1 ][ 1 ] = textMetrics.height;
	}

	font_bounds[ 1 ].Ceil( );
	bmpWidth = ( int )font_bounds[ 1 ][ 0 ];    
	bmpHeight = ( int )font_bounds[ 1 ][ 1 ]; 

	DWriteFactory->GetGdiInterop( &gdiInterop );
	gdiInterop->CreateBitmapRenderTarget( WindowsNS::GetDC( NULL ), bmpWidth, bmpHeight, &renderTarget );
	DWriteFactory->CreateRenderingParams( &renderParams );
	renderTarget->SetCurrentTransform( NULL );
	dwRenderer.SetTarget( renderTarget );
	dwRenderer.SetParams( renderParams );
 
	hdcMem = renderTarget->GetMemoryDC( );
	hdcMemComp = WindowsNS::CreateCompatibleDC( hdcMem );
	hbmMemComp = WindowsNS::CreateCompatibleBitmap( hdcMem, bmpWidth, bmpHeight );
	WindowsNS::SelectObject( hdcMemComp, hbmMemComp );
	WindowsNS::GetObject( hbmMemComp, sizeof( WindowsNS::BITMAP ), &bmpMemComp );
	bBrush = WindowsNS::CreateSolidBrush( 0 );
	WindowsNS::SelectObject( hdcMem, bBrush );

	*char_set_ptr = new Image[ char_range ];
	imageBuffer = ( byte * )Mem_Alloc( bmpWidth * bmpHeight * 4 );
	bmpBuffer = ( byte * )bmpMemComp.bmBits;

	for( int i = 0; i < char_range; i++ ) {

		WindowsNS::PatBlt( hdcMem, 0, 0, bmpWidth, bmpHeight, 0x00F00021 );
		WindowsNS::HRESULT res = textLayouts[ i ]->Draw( NULL, reinterpret_cast< WindowsNS::IDWriteTextRenderer * >( &dwRenderer ), 0.0f, 0.0f );
		WindowsNS::BitBlt( hdcMemComp, 0, 0, bmpWidth, bmpHeight, hdcMem, 0, 0, 0x00CC0020 );

		for( int y = 0; y < bmpHeight; y++ ) {

			int bmpIndex = y * bmpMemComp.bmWidthBytes;

			for( int x = 0; x < bmpWidth; x++, bmpIndex += 4 ) {

				imageBuffer[ ( y * bmpWidth + x ) * 4 + 0 ] = bmpBuffer[ bmpIndex + 0 ];
				imageBuffer[ ( y * bmpWidth + x ) * 4 + 1 ] = bmpBuffer[ bmpIndex + 1 ];
				imageBuffer[ ( y * bmpWidth + x ) * 4 + 2 ] = bmpBuffer[ bmpIndex + 2 ];
				imageBuffer[ ( y * bmpWidth + x ) * 4 + 3 ] = ( bmpBuffer[ bmpIndex + 0 ] + bmpBuffer[ bmpIndex + 1 ] + bmpBuffer[ bmpIndex + 2 ] ) / 3;
			}
		}

		Image * fontImage = &( *char_set_ptr )[ i ];
		fontImage->LoadRaw( imageBuffer, bmpWidth * bmpHeight * 4, "rgba", VecT2i( ( int )font_bounds[1][0], ( int )font_bounds[1][1] ), true );
		textLayouts[ i ]->GetMetrics( &textMetrics );
		Bounds2D charBounds = Bounds2D( 0.0f, 0.0f, textMetrics.width ? textMetrics.width : i ? 1 : font_pts * 0.5f, textMetrics.height );
		charBounds[ 1 ].Ceil( );
		fontImage->Crop( BoundsT2i( vecT2i_origin, VecT2i( ( int )charBounds[1][0], ( int )charBounds[1][1] ) ) );
		sprintf( imageName, "%s_%.2f_%c", font_name.c_str( ), font_pts, i + char_start );
		fontImage->SetName( imageName );
	}

	Mem_Free( imageBuffer );

	WindowsNS::DeleteObject( bBrush );
	WindowsNS::DeleteObject( hbmMemComp );
	WindowsNS::ReleaseDC( NULL, hdcMemComp );

	renderTarget->Release( );
	gdiInterop->Release( );
	for( int i = 0; i < char_range; i++ ) textLayouts[ i ]->Release( );
	textFormat->Release( );
}

using namespace WindowsNS;

HRESULT DWRenderer::DrawGlyphRun( void * clientDrawingContext, float baselineOriginX, float baselineOriginY, DWRITE_MEASURING_MODE measuringMode,
	DWRITE_GLYPH_RUN const * glyphRun, DWRITE_GLYPH_RUN_DESCRIPTION const * glyphRunDescription, IUnknown * clientDrawingEffect ) {

	HRESULT res = renderTarget->DrawGlyphRun( baselineOriginX, baselineOriginY, measuringMode, glyphRun, renderParams, 0xFFFFFFFF );

	return S_OK;
}

HRESULT DWRenderer::DrawUnderline( void * clientDrawingContext, float baselineOriginX, float baselineOriginY, DWRITE_UNDERLINE const * underline, IUnknown * clientDrawingEffect ) {

	return E_NOTIMPL;
}

HRESULT DWRenderer::DrawStrikethrough( void * clientDrawingContext, float baselineOriginX, float baselineOriginY, DWRITE_STRIKETHROUGH const * strikethrough, IUnknown * clientDrawingEffect ) {

	return E_NOTIMPL;
}

HRESULT DWRenderer::DrawInlineObject( void * clientDrawingContext, float originX, float originY, IDWriteInlineObject * inlineObject, BOOL isSideways, BOOL isRightToLeft, IUnknown * clientDrawingEffect ) {

	return E_NOTIMPL;
}

HRESULT DWRenderer::QueryInterface( REFIID riid, void ** ppvObject ) {

	*ppvObject = NULL;
	return E_NOTIMPL;
}

ULONG DWRenderer::AddRef( ) {

	return 0;
}

ULONG DWRenderer::Release( ) {

	return 0;
}

HRESULT DWRenderer::IsPixelSnappingDisabled( void * clientDrawingContext, BOOL * isDisabled ) {

	*isDisabled = FALSE;
	return S_OK;
}

HRESULT DWRenderer::GetCurrentTransform( void * clientDrawingContext, DWRITE_MATRIX * transform ) {

	transform->m11 = 1.0f;
	transform->m12 = 0.0f;
	transform->m21 = 0.0f;
	transform->m22 = 1.0f;
	transform->dx = 0.0f;
	transform->dy = 0.0f;
	return S_OK;
}

HRESULT DWRenderer::GetPixelsPerDip( void * clientDrawingContext, FLOAT * pixelsPerDip ) {

	*pixelsPerDip = GetDeviceCaps( GetDC( NULL ), LOGPIXELSY ) / 96.0f;
	return S_OK;
}

#endif
