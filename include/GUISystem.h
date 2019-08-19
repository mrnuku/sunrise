#ifndef GUISYSTEM_H
#define GUISYSTEM_H

// // //// // //// // //// //
/// GUILogger
// //// // //// // //// //

class GUILogger : public CEGUI::Logger {
private:
	Str												g_logBuffer;

public:
													GUILogger( );
	virtual											~GUILogger( );

	virtual void									logEvent( const CEGUI::String & message, CEGUI::LoggingLevel level = CEGUI::Standard );
	virtual void									setLogFilename( const CEGUI::String & filename, bool append = false );

	const Str &										getLogBuffer( ) const;
};

// // //// // //// // //// //
/// GUIResourceProvider
// //// // //// // //// //

class GUIResourceProvider : public CEGUI::ResourceProvider {
public:
													GUIResourceProvider( );
	virtual											~GUIResourceProvider( );

	virtual void									loadRawDataContainer( const CEGUI::String & filename, CEGUI::RawDataContainer & output, const CEGUI::String & resourceGroup );
	virtual void									unloadRawDataContainer( CEGUI::RawDataContainer & data );
	virtual size_t									getResourceGroupFileNames( std::vector< CEGUI::String > & out_vec, const CEGUI::String & file_pattern, const CEGUI::String & resource_group );
};

// // //// // //// // //// //
/// GUIxmlParser
// //// // //// // //// //

class GUIxmlParser : public CEGUI::XMLParser {
protected:

	virtual bool									initialiseImpl( );
	virtual void									cleanupImpl( );

	static void										startElement( void * data, const char * element, const char ** attr );
	static void										endElement( void * data, const char * element );
	static void										characterData( void * data, const char * text, int len );

public:
													GUIxmlParser( );
	virtual											~GUIxmlParser( );

	virtual void									parseXMLFile( CEGUI::XMLHandler & handler, const CEGUI::String & filename, const CEGUI::String & schemaName, const CEGUI::String & resourceGroup );
};

// // //// // //// // //// //
/// GUIImageCodec
// //// // //// // //// //

class GUIImageCodec : public CEGUI::ImageCodec {
public:
													GUIImageCodec( );
	virtual											~GUIImageCodec( );

	virtual CEGUI::Texture *						load( const CEGUI::RawDataContainer & data, CEGUI::Texture * result );
};

// // //// // //// // //// //
/// GUIFontManager
// //// // //// // //// //

class GUIFontManager : CEGUI::FontManager {	
public:		
	void											addFont( CEGUI::GUIFont * font );
};

// // //// // //// // //// //
/// FormattedListboxTextItem
//
/// A ListboxItem based class that can do horizontal text formatiing.
// //// // //// // //// //

class FormattedListboxTextItem : public CEGUI::ListboxTextItem {
protected:
	/// Helper to create a FormattedRenderedString of an appropriate type.
	void											setupStringFormatter( ) const;
	/// Current formatting set
	CEGUI::HorizontalTextFormatting					d_formatting;
	/// Class that renders RenderedString with some formatting.
	mutable CEGUI::FormattedRenderedString *		d_formattedRenderedString;
	/// Tracks target area for rendering so we can reformat when needed
	mutable CEGUI::Size								d_formattingAreaSize;

public:
													FormattedListboxTextItem( const CEGUI::String & text,
														const CEGUI::HorizontalTextFormatting format = CEGUI::HTF_LEFT_ALIGNED, const uint item_id = 0,
														void * const item_data = 0, const bool disabled = false, const bool auto_delete = true );

													~FormattedListboxTextItem( );

	/// Return the current formatting set.
	CEGUI::HorizontalTextFormatting					getFormatting( ) const;
	/// Set the formatting.  You should call Listbox::handleUpdatedItemData
	/// after setting the formatting in order to update the listbox.  We do not
	/// do it automatically since you may wish to batch changes to multiple
	/// items and multiple calls to handleUpdatedItemData is wasteful.
	void											setFormatting( const CEGUI::HorizontalTextFormatting fmt );
	/// overriden functions.
	CEGUI:: Size									getPixelSize( ) const;
	void											draw( CEGUI::GeometryBuffer & buffer, const CEGUI::Rect & targetRect, float alpha, const CEGUI::Rect * clipper ) const;
};

// // //// // //// // //// //
/// GUIWindow
// //// // //// // //// //

class GUIWindow : public NamedObject {
protected:
	CEGUI::Window *									d_handle;
	Str												d_type;
	List< GUIWindow * >								d_childList;
	virtualCode_t									d_keyBind;

public:
													GUIWindow( const Str & type, const Str & name, float rel_x, float rel_y, float rel_w, float rel_h );
													~GUIWindow( );

	void											SetPos( float rel_x, float rel_y );
	void											SetSize( float rel_w, float rel_h );
	void											SetText( const Str & text );
	void											SetAlpha( float value );
	void											SetProperty( const Str & name, const Str & value );
	void											SetBind( virtualCode_t bind );

	virtualCode_t									GetBind( ) const;

	template< typename T > void						AddEvent( const CEGUI::String & evnt, bool ( T::*function )( const CEGUI::EventArgs & args ) );
	template< typename T > void						AddEvent( const CEGUI::String & evnt, GUIWindow * window, bool ( T::*function )( const CEGUI::EventArgs & args ) );

	void											Show( );
	void											Hide( );
	void											SwapVisible( );

	void											SetParent( GUIWindow * parent );
	void											Register( );

	template< typename T > T *						GetHandle( );
	CEGUI::Window *									GetHandle( );
	operator										CEGUI::Window *( );
};

// // //// // //// // //// //
/// ConsoleWindow
// //// // //// // //// //

class ConsoleWindow : public GUIWindow {
private:
	GUIWindow *										d_windowFrame;
	GUIWindow *										d_windowText;

public:
													ConsoleWindow( float rel_x, float rel_y, float rel_w, float rel_h );

	CEGUI::Listbox *								GetTextWindow( );
	bool											handleKeyDown( const CEGUI::EventArgs & args );
};

// // //// // //// // //// //
/// GUISystem
// //// // //// // //// //

class GUISystem {
	friend class									GUIWindow;

private:
	static CEGUI::System *							g_system;
	static CEGUI::GUIRenderer *						g_renderer;
	static CEGUI::WindowManager *					g_winMgr;
	static CEGUI::SchemeManager *					g_schemeMgr;
	static CEGUI::Scheme *							g_scheme;
	static GUIFontManager *							g_fontManager;
	static GUIResourceProvider *					g_resourceProvider;
	static CEGUI::FalagardWRModule *				g_wrModule;
	static GUILogger *								g_logger;
	static Lock										g_guiLock;
	static GUIWindow *								g_guiBackGround;
	static ConsoleWindow *							g_guiConsole;
	static List< GUIWindow * >						g_windowList;
	static Lock										g_consoleLock;
	static List< CEGUI::ListboxTextItem * >			g_consoleInputList;
	static int										g_consoleInputUploaded;
	static timeType									g_lastFrameTime;

public:
	static void										Init( );
	static void										Shutdown( );

	static void										Key_Event( const KeyEvent & key_event );
	static void										SendMouseMovement( const Vec2 & coords );

	static bool										NeedRedraw( );
	static void										DrawGUI( );

	static void										PrintConsole( const Str & text );
	static void										PrintConsoleColor( const Str & text, const Color & color );

	static int										GetNumWindows( );
	static GUIWindow *								GetWindow( int index );
};

// // //// // //// // //// //
// GUILogger
// //// // //// // //// //

INLINE const Str & GUILogger::getLogBuffer( ) const {
	return g_logBuffer;
}

// // //// // //// // //// //
// GUIWindow
// //// // //// // //// //

INLINE void GUIWindow::SetPos( float rel_x, float rel_y ) {
	d_handle->setPosition( CEGUI::UVector2( cegui_reldim( rel_x ), cegui_reldim( rel_y ) ) );
}

INLINE void GUIWindow::SetSize( float rel_w, float rel_h ) {
	d_handle->setSize( CEGUI::UVector2( cegui_reldim( rel_w ), cegui_reldim( rel_h ) ) );
}

INLINE void GUIWindow::SetText( const Str & text ) {
	d_handle->setText( text.cegui_str( ) );
}

INLINE void GUIWindow::SetAlpha( float value ) {
	d_handle->setAlpha( value );
}

INLINE void GUIWindow::SetProperty( const Str & name, const Str & value ) {
	d_handle->setProperty( name.cegui_str( ), value.cegui_str( ) );
}

INLINE void GUIWindow::SetBind( virtualCode_t bind ) {
	d_keyBind = bind;
}

INLINE virtualCode_t GUIWindow::GetBind( ) const {
	return d_keyBind;
}

template< typename T > INLINE void GUIWindow::AddEvent( const CEGUI::String & evnt, bool ( T::*function )( const CEGUI::EventArgs & args ) ) {
	d_handle->subscribeEvent( evnt, CEGUI::Event::Subscriber( function, ( T * )this ) );
}

template< typename T > INLINE void GUIWindow::AddEvent( const CEGUI::String & evnt, GUIWindow * window, bool ( T::*function )( const CEGUI::EventArgs & args ) ) {
	d_handle->subscribeEvent( evnt, CEGUI::Event::Subscriber( function, ( T * )window ) );
}

INLINE void GUIWindow::Show( ) {
	d_handle->show( );
}

INLINE void GUIWindow::Hide( ) {
	d_handle->hide( );
}

template< typename T > INLINE T * GUIWindow::GetHandle( ) {
	return ( T * )d_handle;
}

INLINE CEGUI::Window * GUIWindow::GetHandle( ) {
	return d_handle;
}

INLINE GUIWindow::operator CEGUI::Window *( ) {
	return d_handle;
}

// // //// // //// // //// //
// ConsoleWindow
// //// // //// // //// //

INLINE CEGUI::Listbox * ConsoleWindow::GetTextWindow( ) {
	return ( CEGUI::Listbox * )( ( CEGUI::Window * )*d_windowText );
}

// // //// // //// // //// //
// GUISystem
// //// // //// // //// //

INLINE bool GUISystem::NeedRedraw( ) {
	return g_system->isRedrawRequested( );
}

INLINE int GUISystem::GetNumWindows( ) {
	return g_windowList.Num( );
}

INLINE GUIWindow * GUISystem::GetWindow( int index ) {
	return g_windowList[ index ];
}

#endif
