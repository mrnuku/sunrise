#include "precompiled.h"
#pragma hdrstop

extern "C" {
CEGUI::XMLParser * createParser( ) {
	return new GUIxmlParser( );
}
void destroyParser( CEGUI::XMLParser * parser ) {
	delete parser;
}
CEGUI::ImageCodec * createImageCodec( ) {
	return new GUIImageCodec( );
}
void destroyImageCodec( CEGUI::ImageCodec * imageCodec ) {
	delete imageCodec;
}
}

// // //// // //// // //// //
// GUISystem
// //// // //// // //// //

CEGUI::System *					GUISystem::g_system				= NULL;
CEGUI::GUIRenderer *			GUISystem::g_renderer			= NULL;
CEGUI::WindowManager *			GUISystem::g_winMgr				= NULL;
CEGUI::SchemeManager *			GUISystem::g_schemeMgr			= NULL;
CEGUI::Scheme *					GUISystem::g_scheme				= NULL;
GUIFontManager *				GUISystem::g_fontManager		= NULL;
GUIResourceProvider *			GUISystem::g_resourceProvider	= NULL;
CEGUI::FalagardWRModule *		GUISystem::g_wrModule			= NULL;
GUILogger *						GUISystem::g_logger				= NULL;
Lock							GUISystem::g_guiLock;
GUIWindow *						GUISystem::g_guiBackGround;
ConsoleWindow *					GUISystem::g_guiConsole;
List< GUIWindow * >				GUISystem::g_windowList;
Lock							GUISystem::g_consoleLock;
List< CEGUI::ListboxTextItem * >GUISystem::g_consoleInputList;
int								GUISystem::g_consoleInputUploaded;
timeType						GUISystem::g_lastFrameTime = 0;

void GUISystem::Init( ) {
	g_renderer			= new CEGUI::GUIRenderer( );
	g_resourceProvider	= new GUIResourceProvider( );
	g_logger			= new GUILogger( );
	g_system			= &CEGUI::System::create( *g_renderer, g_resourceProvider );
	g_winMgr			= CEGUI::WindowManager::getSingletonPtr( );
	g_schemeMgr			= CEGUI::SchemeManager::getSingletonPtr( );
	g_scheme			= &g_schemeMgr->create( "TaharezLook.scheme" );
	g_fontManager		= ( GUIFontManager * )CEGUI::FontManager::getSingletonPtr( );
	g_wrModule			= new CEGUI::FalagardWRModule( );
	
	CEGUI::GUIFont * defFont = new CEGUI::GUIFont( "Lucida Console", 16 );
	g_fontManager->addFont( defFont );
	g_system->setDefaultFont( defFont );

	g_guiBackGround = new GUIWindow( "DefaultWindow", "GUIBackGround", 0.0f, 0.0f, 1.0f, 1.0f );
	g_guiBackGround->SetAlpha( 0.0f );
	g_system->setGUISheet( *g_guiBackGround );

	g_guiConsole = new ConsoleWindow( 0.01f, 0.01f, 0.98f, 0.48f );

	SoundSystem::InitMenu( );

	g_consoleInputUploaded = 0;
	g_lastFrameTime = System::Sys_Milliseconds( );
}

void GUISystem::Shutdown( ) {
	CEGUI::System::destroy( );

	delete g_resourceProvider;
	delete g_logger;
	delete g_renderer;
	delete g_wrModule;

	g_system				= NULL;
	g_renderer				= NULL;
	g_winMgr				= NULL;
	g_schemeMgr				= NULL;
	g_scheme				= NULL;
	g_fontManager			= NULL;
	g_resourceProvider		= NULL;
	g_logger				= NULL;
	g_wrModule				= NULL;

	g_consoleInputUploaded = 0;
	g_consoleInputList.Clear( );
}

void GUISystem::Key_Event( const KeyEvent & key_event ) {
	if( !g_system )
		return;
	ScopedLock< true > scopedLock( g_guiLock );
	if( key_event.IsDown( ) ) {
		for( int i = 0; i < g_windowList.Num( ); i++ )
			if( g_windowList[ i ]->GetBind( ) == key_event )
				g_windowList[ i ]->SwapVisible( );
	}

	switch( key_event ) {
		case mouse_left:
		case mouse_right:
			if( key_event.IsDown( ) ) g_system->injectMouseButtonDown( key_event == mouse_left ? CEGUI::LeftButton : CEGUI::RightButton );
			else g_system->injectMouseButtonUp( key_event == mouse_left ? CEGUI::LeftButton : CEGUI::RightButton );
			break;
		case mouse_wheelup:
			 g_system->injectMouseWheelChange( 1.0f );
			 break;
		case mouse_wheeldown:
			 g_system->injectMouseWheelChange( -1.0f );
			 break;
		case key_return:
			if( key_event.IsDown( ) )
				g_system->injectKeyDown( CEGUI::Key::Return );
			else
				g_system->injectKeyUp( CEGUI::Key::Return );
			break;
		case key_tab:
			if( key_event.IsDown( ) )
				g_system->injectKeyDown( CEGUI::Key::Tab );
			else
				g_system->injectKeyUp( CEGUI::Key::Tab );
			break;
		case key_backspace:
			if( key_event.IsDown( ) )
				g_system->injectKeyDown( CEGUI::Key::Backspace );
			else
				g_system->injectKeyUp( CEGUI::Key::Backspace );
			break;
		default:			
			uint unicode = key_event.GetUnicode( );
			if( unicode && key_event.IsDown( ) )
				g_system->injectChar( unicode );
	}
}

void GUISystem::SendMouseMovement( const Vec2 & coords ) {
	if( !g_system )
		return;
	ScopedLock< true > scopedLock( g_guiLock );
	g_system->injectMousePosition( coords[ 0 ], coords[ 1 ] );
}

void GUISystem::DrawGUI( ) {
	g_consoleLock.Acquire( );
	while( g_consoleInputUploaded != g_consoleInputList.Num( ) ) {
		g_guiConsole->GetTextWindow( )->addItem( g_consoleInputList[ g_consoleInputUploaded++ ] );
		g_guiConsole->GetTextWindow( )->ensureItemIsVisible( g_consoleInputUploaded );
	}
	g_consoleLock.Release( );
	ScopedLock< true > scopedLock( g_guiLock );
	timeType currentTime = System::Sys_Milliseconds( );
	g_system->injectTimePulse( ( currentTime - g_lastFrameTime ) / 1000.0f );
	g_lastFrameTime = currentTime;
	try {
		g_system->renderGUI( );
	}
	catch( CEGUI::Exception & except ) {
		Common::Com_Error( ERR_DROP, "GUISystem::DrawGUI: g_system->renderGUI( ) failed with \"%s\"\n", except.getMessage( ).c_str( ) );
	}
}

void GUISystem::PrintConsole( const Str & text ) {
	ScopedLock< true > scopedLock( g_consoleLock );
	CEGUI::ListboxTextItem * newItem = new CEGUI::ListboxTextItem( text.cegui_str( ), ( uint )g_consoleInputList.Num( ) );
	g_consoleInputList.Append( newItem );
	if( g_system )
		g_system->signalRedraw( );
}

void GUISystem::PrintConsoleColor( const Str & text, const Color & color ) {
	ScopedLock< true > scopedLock( g_consoleLock );
	CEGUI::ListboxTextItem * newItem = new CEGUI::ListboxTextItem( text.cegui_str( ), ( uint )g_consoleInputList.Num( ) );
	newItem->setTextColours( color );
	g_consoleInputList.Append( newItem );
	if( g_system )
		g_system->signalRedraw( );
}

// // //// // //// // //// //
// FormattedListboxTextItem
// //// // //// // //// //

FormattedListboxTextItem::FormattedListboxTextItem( const CEGUI::String & text, const CEGUI::HorizontalTextFormatting format, const uint item_id,
	void * const item_data, const bool disabled, const bool auto_delete ) :
	
	ListboxTextItem( text, item_id, item_data, disabled, auto_delete ), d_formatting( format ), d_formattedRenderedString( 0 ), d_formattingAreaSize( 0, 0 ) {
}

FormattedListboxTextItem::~FormattedListboxTextItem( ) {
	delete d_formattedRenderedString;
}

CEGUI::HorizontalTextFormatting FormattedListboxTextItem::getFormatting( ) const {
	return d_formatting;
}


void FormattedListboxTextItem::setFormatting( const CEGUI::HorizontalTextFormatting fmt ) {
	if( fmt == d_formatting )
		return;
	d_formatting = fmt;
	delete d_formattedRenderedString;
	d_formattedRenderedString = 0;
	d_formattingAreaSize = CEGUI::Size( 0, 0 );
}

CEGUI::Size FormattedListboxTextItem::getPixelSize( ) const {
	if( !d_owner )
		return CEGUI::Size( 0, 0 );
	if( !d_renderedStringValid )
		parseTextString( ); // reparse text if we need to.
	if( !d_formattedRenderedString )
		setupStringFormatter( ); // create formatter if needed
	// get size of render area from target window, to see if we need to reformat
	const CEGUI::Size area_sz( ( ( CEGUI::Listbox * )d_owner )->getListRenderArea( ).getSize( ) );
	if( area_sz != d_formattingAreaSize ) {
		d_formattedRenderedString->format( area_sz );
		d_formattingAreaSize = area_sz;
	}
	return CEGUI::Size( d_formattedRenderedString->getHorizontalExtent( ), d_formattedRenderedString->getVerticalExtent( ) );
}

void FormattedListboxTextItem::draw( CEGUI::GeometryBuffer & buffer, const CEGUI::Rect & targetRect, float alpha, const CEGUI::Rect * clipper ) const {
	if( !d_renderedStringValid )
		parseTextString( ); // reparse text if we need to.
	if( !d_formattedRenderedString )
		setupStringFormatter( ); // create formatter if needed
	// get size of render area from target window, to see if we need to reformat
	// NB: We do not use targetRect, since it may not represent the same area.
	const CEGUI::Size area_sz( ( ( CEGUI::Listbox * ) d_owner )->getListRenderArea( ).getSize( ) );
	if( area_sz != d_formattingAreaSize ) {
		d_formattedRenderedString->format( area_sz );
		d_formattingAreaSize = area_sz;
	}
	// draw selection imagery
	if( d_selected && d_selectBrush != 0 )
		d_selectBrush->draw( buffer, targetRect, clipper, getModulateAlphaColourRect( d_selectCols, alpha ) );
	// factor the window alpha into our colours.
	const CEGUI::ColourRect final_colours( getModulateAlphaColourRect( CEGUI::ColourRect( 0xFFFFFFFF ), alpha ) );
	// draw the formatted text
	d_formattedRenderedString->draw( buffer, targetRect.getPosition( ), &final_colours, clipper );
}

void FormattedListboxTextItem::setupStringFormatter( ) const {
	// delete any existing formatter
	delete d_formattedRenderedString;
	d_formattedRenderedString = 0;
	// create new formatter of whichever type...
	switch( d_formatting ) {
		case CEGUI::HTF_LEFT_ALIGNED:
			d_formattedRenderedString = new CEGUI::LeftAlignedRenderedString(d_renderedString);
			break;
		case CEGUI::HTF_RIGHT_ALIGNED:
			d_formattedRenderedString = new CEGUI::RightAlignedRenderedString(d_renderedString);
			break;
		case CEGUI::HTF_CENTRE_ALIGNED:
			d_formattedRenderedString = new CEGUI::CentredRenderedString(d_renderedString);
			break;
		case CEGUI::HTF_JUSTIFIED:
			d_formattedRenderedString = new CEGUI::JustifiedRenderedString(d_renderedString);
			break;
		case CEGUI::HTF_WORDWRAP_LEFT_ALIGNED:
			d_formattedRenderedString = new CEGUI::RenderedStringWordWrapper< CEGUI::LeftAlignedRenderedString >( d_renderedString );
			break;
		case CEGUI::HTF_WORDWRAP_RIGHT_ALIGNED:
			d_formattedRenderedString = new CEGUI::RenderedStringWordWrapper< CEGUI::RightAlignedRenderedString >( d_renderedString );
			break;
		case CEGUI::HTF_WORDWRAP_CENTRE_ALIGNED:
			d_formattedRenderedString = new CEGUI::RenderedStringWordWrapper< CEGUI::CentredRenderedString >( d_renderedString );
			break;
		case CEGUI::HTF_WORDWRAP_JUSTIFIED:
			d_formattedRenderedString = new CEGUI::RenderedStringWordWrapper< CEGUI::JustifiedRenderedString >( d_renderedString );
			break;
	}
}

// // //// // //// // //// //
// GUILogger
// //// // //// // //// //

GUILogger::GUILogger( ) {
	g_logBuffer.ReAllocate( 256*1024, false );
}

GUILogger::~GUILogger( ) {
}

void GUILogger::logEvent( const CEGUI::String & message, CEGUI::LoggingLevel level ) {
	g_logBuffer += message;
	g_logBuffer += '\n';
}

void GUILogger::setLogFilename( const CEGUI::String & filename, bool append ) {
}

// // //// // //// // //// //
// GUIResourceProvider
// //// // //// // //// //

GUIResourceProvider::GUIResourceProvider( ) {
	CEGUI::Scheme::setDefaultResourceGroup( "cegui/" );
	CEGUI::Imageset::setDefaultResourceGroup( "cegui/" );
	CEGUI::Font::setDefaultResourceGroup( "cegui/" );
	CEGUI::WidgetLookManager::setDefaultResourceGroup( "cegui/" );
}

GUIResourceProvider::~GUIResourceProvider( ) {
}

void GUIResourceProvider::loadRawDataContainer( const CEGUI::String & filename, CEGUI::RawDataContainer & output, const CEGUI::String & resourceGroup ) {
	Str fName = resourceGroup;
	fName += filename;
	File_Memory fm = FileSystem::ReadFile( fName );
	if( !fm.IsValid( ) ) {
		output.setData( 0 ); output.setSize( 0 );
		return;
	}
	output.setData( new uint8[ fm.Length( ) ] );
	Common::Com_Memcpy( output.getDataPtr( ), fm.GetDataPtr( ), fm.Length( ) );
	output.setSize( fm.Length( ) );
	FileSystem::FreeFile( fm );
}

void GUIResourceProvider::unloadRawDataContainer( CEGUI::RawDataContainer & data ) {
	uint8 * const ptr = data.getDataPtr( );
	if( ptr )
		delete[ ] ptr;
	data.setData( 0 );
	data.setSize( 0 );
}

size_t GUIResourceProvider::getResourceGroupFileNames( std::vector< CEGUI::String > & out_vec, const CEGUI::String & file_pattern, const CEGUI::String & resource_group ) {
	return 0;
}

// // //// // //// // //// //
// GUIxmlParser
// //// // //// // //// //

GUIxmlParser::GUIxmlParser( ) {
	d_identifierString = "GUIxmlParser";
}

GUIxmlParser::~GUIxmlParser( ) {
}

void GUIxmlParser::parseXMLFile( CEGUI::XMLHandler & handler, const CEGUI::String & filename, const CEGUI::String & /*schemaName*/, const CEGUI::String & resourceGroup ) {
	Str fName = resourceGroup;
	fName += filename;
	File_Memory fm = FileSystem::ReadFile( fName );
	if( !fm.IsValid( ) )
		return;
	Expat::XML_Parser parser = Expat::XML_ParserCreate( 0 );
	if( parser ) {
		Expat::XML_SetUserData( parser, ( void * )&handler );
		Expat::XML_SetElementHandler( parser, startElement, endElement );
		Expat::XML_SetCharacterDataHandler( parser, characterData );
		if( !Expat::XML_Parse( parser, ( char * )fm.GetDataPtr( ), ( int )fm.Length( ), true ) )
			Common::Com_Printf( "GUIxmlParser::parseXMLFile: error: %s at line %i\n", Expat::XML_ErrorString( Expat::XML_GetErrorCode( parser ) ), Expat::XML_GetCurrentLineNumber( parser ) );
		Expat::XML_ParserFree( parser );
	}
	FileSystem::FreeFile( fm );
}

bool GUIxmlParser::initialiseImpl( ) {
	return true;
}

void GUIxmlParser::cleanupImpl( ) {
}

void GUIxmlParser::startElement( void * data, const char * element, const char ** attr ) {
	CEGUI::XMLHandler * handler = static_cast< CEGUI::XMLHandler * >( data );
	CEGUI::XMLAttributes attrs;
	for( size_t i = 0 ; attr[ i ] ; i += 2 )
		attrs.add( ( const CEGUI::utf8 * )attr[ i ], ( const CEGUI::utf8 * )attr[ i + 1 ] );
	handler->elementStart( ( const CEGUI::utf8 * )element, attrs );
}

void GUIxmlParser::endElement( void * data, const char * element ) {
	CEGUI::XMLHandler * handler = static_cast< CEGUI::XMLHandler * >( data );
	handler->elementEnd( ( const CEGUI::utf8 * )element );
}

void GUIxmlParser::characterData( void * data, const char * text, int len ) {
	CEGUI::XMLHandler * handler = static_cast< CEGUI::XMLHandler * >( data );
	CEGUI::String str( ( const CEGUI::utf8 * )text, static_cast< CEGUI::String::size_type >( len ) );
	handler->text( str );
}

// // //// // //// // //// //
// GUIImageCodec
// //// // //// // //// //

GUIImageCodec::GUIImageCodec( ) : ImageCodec( "GUIImageCodec" ) {
}

GUIImageCodec::~GUIImageCodec( ) {
}

CEGUI::Texture * GUIImageCodec::load( const CEGUI::RawDataContainer & data, CEGUI::Texture * result ) {
	assert( 0 );
	return NULL;
}

// // //// // //// // //// //
// GUIFontManager
// //// // //// // //// //

void GUIFontManager::addFont( CEGUI::GUIFont * font ) {		
	doExistingObjectAction( font->getProperty( "Name" ), font, CEGUI::XREA_RETURN );
}

// // //// // //// // //// //
// GUIWindow
// //// // //// // //// //

GUIWindow::GUIWindow( const Str & type, const Str & name, float rel_x, float rel_y, float rel_w, float rel_h ) {
	d_name		= name;
	d_type		= type;
	d_keyBind	= vs_none;
	d_handle	= GUISystem::g_winMgr->createWindow( type.cegui_str( ), name.cegui_str( ) );
	d_handle->setPosition( CEGUI::UVector2( cegui_reldim( rel_x ), cegui_reldim( rel_y ) ) );
	d_handle->setSize( CEGUI::UVector2( cegui_reldim( rel_w ), cegui_reldim( rel_h ) ) );
	d_handle->setInheritsAlpha( false );
}

GUIWindow::~GUIWindow( ) {
	d_childList.DeleteContents( false );
}

void GUIWindow::SwapVisible( ) {
	if( d_handle->isVisible( ) ) {
		Hide( );
	} else {		
		Show( );
		d_handle->moveToFront( );
	}
}

void GUIWindow::SetParent( GUIWindow * parent ) {	
	parent->d_childList.Append( this );
	parent->d_handle->addChildWindow( d_handle );
}

void GUIWindow::Register( ) {
	GUISystem::g_windowList.Append( this );
	SetParent( GUISystem::g_guiBackGround );
}

// // //// // //// // //// //
// ConsoleWindow
// //// // //// // //// //

ConsoleWindow::ConsoleWindow( float rel_x, float rel_y, float rel_w, float rel_h ) :
	GUIWindow( "TaharezLook/Editbox", "ConsoleFrame/Editbox", 0.01f, 0.885f, 0.98f, 0.1f ) {

	d_windowFrame = new GUIWindow( "TaharezLook/FrameWindow", "ConsoleFrame", rel_x, rel_y, rel_w, rel_h );
	d_windowFrame->SetText( "Console" );
	d_windowFrame->SetBind( mouse_middle );
	d_windowFrame->Register( );
	d_windowText = new GUIWindow( "TaharezLook/Listbox", "ConsoleFrame/Text", 0.01f, 0.08f, 0.98f, 0.795f );
	d_windowText->SetParent( d_windowFrame );
	SetParent( d_windowFrame );
	AddEvent< ConsoleWindow >( CEGUI::Editbox::EventKeyDown, &ConsoleWindow::handleKeyDown );
}

bool ConsoleWindow::handleKeyDown( const CEGUI::EventArgs & args ) {
	const CEGUI::KeyEventArgs & keyArgs = dynamic_cast< const CEGUI::KeyEventArgs & >( args );
	const CEGUI::WindowEventArgs & winArgs = dynamic_cast< const CEGUI::WindowEventArgs & >( args );
	CEGUI::Editbox * winEditBox = dynamic_cast< CEGUI::Editbox * >( winArgs.window );
	if( keyArgs.scancode == CEGUI::Key::Return ) {
		Str conText = winEditBox->getText( );
		CBuffer::Cbuf_AddText( conText + '\n' );
		winEditBox->setText( "" );
	} else if( keyArgs.scancode == CEGUI::Key::Tab ) {
		Str conText = winEditBox->getText( );
		Str completedText = Command::Cmd_CompleteCommand( conText );
		if( completedText.IsEmpty( ) )
			completedText = CVarSystem::CompleteVariable( conText );
		if( completedText.Length( ) ) {
			winEditBox->setText( completedText.cegui_str( ) );
			winEditBox->setCaratIndex( completedText.Length( ) );
		}
	} else return false;
	return true;
}
