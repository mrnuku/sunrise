#include "precompiled.h"
#pragma hdrstop

glwstate_t Renderer::glw_state;

/*
* * VID_CreateWindow
*/
#define	WINDOW_CLASS_NAME	"Quake 2"

bool Renderer::VID_CreateWindow( int width, int height, bool fullscreen ) {

	WNDCLASS		wc;
	RECT			r;
	int				stylebits;
	int				x, y, w, h;
	int				exstyle;

	/* Register the frame class */
    wc.style         = 0;
    wc.lpfnWndProc   =( WNDPROC )glw_state.wndproc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = glw_state.hInstance;
    wc.hIcon         = 0;
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground =( HBRUSH )COLOR_GRAYTEXT;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = WINDOW_CLASS_NAME;

    if( !RegisterClass( &wc ) )
		Common::Com_Error( ERR_FATAL, "Couldn't register window class" );

	if( fullscreen )
	{
		exstyle = WS_EX_TOPMOST;
		stylebits = WS_POPUP|WS_VISIBLE;
	}
	else
	{
		exstyle = 0;
		stylebits = WINDOW_STYLE;
	}

	r.left = 0;
	r.top = 0;
	r.right  = width;
	r.bottom = height;

	AdjustWindowRect( &r, stylebits, FALSE );

	w = r.right - r.left;
	h = r.bottom - r.top;

	if( fullscreen ) {

		x = 0;
		y = 0;

	} else {

		x = Common::vid_xpos.GetInt( );
		y = Common::vid_ypos.GetInt( );
	}

	glw_state.hWnd = CreateWindowEx( exstyle, WINDOW_CLASS_NAME, "Quake 2", stylebits, x, y, w, h, NULL, NULL, glw_state.hInstance, NULL );

	if( !glw_state.hWnd ) Common::Com_Error( ERR_FATAL, "Couldn't create window" );
	
	ShowWindow( glw_state.hWnd, SW_SHOW );
	UpdateWindow( glw_state.hWnd );

	// init all the gl stuff for the window
	if( !GLimp_InitGL( ) )
	{
		Common::Com_Printf( "VID_CreateWindow( ) - GLimp_InitGL failed\n" );
		return false;
	}

	SetForegroundWindow( glw_state.hWnd );
	SetFocus( glw_state.hWnd );

	// let the sound and input subsystems know about the new window
	Video::VID_NewWindow( width, height );

	return true;
}


/*
* * GLimp_SetMode
*/
uint Renderer::GLimp_SetMode( uint * pwidth, uint * pheight, int mode, bool fullscreen ) {

	int width, height;
	const char * win_fs[] = { "W", "FS" };

	Common::Com_Printf( "Initializing OpenGL display\n" );

	Common::Com_Printf( "...setting mode %d:", mode );

	if( !Video::VID_GetModeInfo( &width, &height, mode ) )
	{
		Common::Com_Printf( " invalid mode\n" );
		return rserr_invalid_mode;
	}

	Common::Com_Printf( " %d %d %s\n", width, height, win_fs[ fullscreen ] );

	// destroy the existing window
	if( glw_state.hWnd )
	{
		GLimp_Shutdown( );
	}

	// do a CDS if needed
	if( fullscreen )
	{
		DEVMODE dm;

		Common::Com_Printf( "...attempting fullscreen\n" );

		memset( &dm, 0, sizeof( dm ) );

		dm.dmSize = sizeof( dm );

		dm.dmPelsWidth  = width;
		dm.dmPelsHeight = height;
		dm.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT;

		if( gl_bitdepth.GetFloat( ) != 0 )
		{
			dm.dmBitsPerPel = gl_bitdepth.GetInt( );
			dm.dmFields |= DM_BITSPERPEL;
			Common::Com_Printf( "...using gl_bitdepth of %d\n", gl_bitdepth.GetInt( ) );
		}
		else
		{
			HDC hdc = GetDC( NULL );
			int bitspixel = GetDeviceCaps( hdc, BITSPIXEL );

			Common::Com_Printf( "...using desktop display depth of %d\n", bitspixel );

			ReleaseDC( 0, hdc );
		}

		Common::Com_Printf( "...calling CDS: " );
		if( ChangeDisplaySettings( &dm, CDS_FULLSCREEN ) == DISP_CHANGE_SUCCESSFUL )
		{
			* pwidth = width;
			* pheight = height;

			gl_state.fullscreen = true;

			Common::Com_Printf( "ok\n" );

			if( !VID_CreateWindow( width, height, true ) )
				return rserr_invalid_mode;

			return rserr_ok;
		}
		else
		{
			* pwidth = width;
			* pheight = height;

			Common::Com_Printf( "failed\n" );

			Common::Com_Printf( "...calling CDS assuming dual monitors:" );

			dm.dmPelsWidth = width * 2;
			dm.dmPelsHeight = height;
			dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

			if( gl_bitdepth.GetFloat( ) != 0 )
			{
				dm.dmBitsPerPel = gl_bitdepth.GetInt( );
				dm.dmFields |= DM_BITSPERPEL;
			}

			/*
			* * our first CDS failed, so maybe we're running on some weird dual monitor
			* * system 
			*/
			if( ChangeDisplaySettings( &dm, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL )
			{
				Common::Com_Printf( " failed\n" );

				Common::Com_Printf( "...setting windowed mode\n" );

				ChangeDisplaySettings( 0, 0 );

				* pwidth = width;
				* pheight = height;
				gl_state.fullscreen = false;
				if( !VID_CreateWindow( width, height, false ) )
					return rserr_invalid_mode;
				return rserr_invalid_fullscreen;
			}
			else
			{
				Common::Com_Printf( " ok\n" );
				if( !VID_CreateWindow( width, height, true ) )
					return rserr_invalid_mode;

				gl_state.fullscreen = true;
				return rserr_ok;
			}
		}
	}
	else
	{
		Common::Com_Printf( "...setting windowed mode\n" );

		ChangeDisplaySettings( 0, 0 );

		* pwidth = width;
		* pheight = height;
		gl_state.fullscreen = false;
		if( !VID_CreateWindow( width, height, false ) )
			return rserr_invalid_mode;
	}

	return rserr_ok;
}

/*
* * GLimp_Shutdown
* *
* * This routine does all OS specific shutdown procedures for the OpenGL
* * subsystem.  Under OpenGL this means NULLing out the current DC and
* * HGLRC, deleting the rendering context, and releasing the DC acquired
* * for the window.  The state structure is also nulled out.
* *
*/
void Renderer::GLimp_Shutdown( ) {

	if( !wglMakeCurrent( NULL, NULL ) )
		Common::Com_Printf( "ref_gl::R_Shutdown( ) - wglMakeCurrent failed\n" );
	if( glw_state.hGLRC )
	{
		if( !wglDeleteContext( glw_state.hGLRC ) )
			Common::Com_Printf( "ref_gl::R_Shutdown( ) - wglDeleteContext failed\n" );
		glw_state.hGLRC = NULL;
	}
	if( glw_state.hDC )
	{
		if( !ReleaseDC( glw_state.hWnd, glw_state.hDC ) )
			Common::Com_Printf( "ref_gl::R_Shutdown( ) - ReleaseDC failed\n" );
		glw_state.hDC   = NULL;
	}
	if( glw_state.hWnd )
	{
		DestroyWindow( 	glw_state.hWnd );
		glw_state.hWnd = NULL;
	}

	UnregisterClass( WINDOW_CLASS_NAME, glw_state.hInstance );

	if( gl_state.fullscreen )
	{
		ChangeDisplaySettings( 0, 0 );
		gl_state.fullscreen = false;
	}
}


/*
* * GLimp_Init
* *
* * This routine is responsible for initializing the OS specific portions
* * of OpenGL.  Under Win32 this means dealing with the pixelformats and
* * doing the wgl interface stuff.
*/
bool Renderer::GLimp_Init( void * hinstance, void * wndproc ) {

	glw_state.hInstance =( HINSTANCE ) hinstance;
	glw_state.wndproc = wndproc;

	return true;
}

bool Renderer::GLimp_InitGL( ) {

    PIXELFORMATDESCRIPTOR pfd = 
	{
		sizeof( PIXELFORMATDESCRIPTOR ), 	// size of this pfd
		1, 								// version number
		PFD_DRAW_TO_WINDOW |			// support window
		PFD_SUPPORT_OPENGL |			// support OpenGL
		PFD_DOUBLEBUFFER, 				// double buffered
		PFD_TYPE_RGBA, 					// RGBA type
		24, 								// 24-bit color depth
		0, 0, 0, 0, 0, 0, 				// color bits ignored
		0, 								// no alpha buffer
		0, 								// shift bit ignored
		0, 								// no accumulation buffer
		0, 0, 0, 0, 					// accum bits ignored
		32, 								// 32-bit z-buffer	
		0, 								// no stencil buffer
		0, 								// no auxiliary buffer
		PFD_MAIN_PLANE, 					// main layer
		0, 								// reserved
		0, 0, 0							// layer masks ignored
    };
    int  pixelformat;

	/*
	* * set PFD_STEREO if necessary
	*/
	if( CVarSystem::GetCVarBool( "cl_stereo" ) ) {

		Common::Com_Printf( "...attempting to use stereo\n" );
		pfd.dwFlags |= PFD_STEREO;
		gl_state.stereo_enabled = true;

	} else gl_state.stereo_enabled = false;
	/*
	* * figure out if we're running on a minidriver or not
	*/
	glw_state.minidriver = true;

	/*
	* * Get a DC for the specified window
	*/
	if( glw_state.hDC != NULL )
		Common::Com_Printf( "GLimp_Init( ) - non-NULL DC exists\n" );

    if( ( glw_state.hDC = GetDC( glw_state.hWnd ) ) == NULL )
	{
		Common::Com_Printf( "GLimp_Init( ) - GetDC failed\n" );
		return false;
	}

	if( glw_state.minidriver )
	{
		if( ( pixelformat = ChoosePixelFormat( glw_state.hDC, &pfd ) ) == 0 )
		{
			Common::Com_Printf( "GLimp_Init( ) - wglChoosePixelFormat failed\n" );
			return false;
		}
		if( SetPixelFormat( glw_state.hDC, pixelformat, &pfd ) == FALSE )
		{
			Common::Com_Printf( "GLimp_Init( ) - wglSetPixelFormat failed\n" );
			return false;
		}
		DescribePixelFormat( glw_state.hDC, pixelformat, sizeof( pfd ), &pfd );
	}
	else {

		if( ( pixelformat = ChoosePixelFormat( glw_state.hDC, &pfd ) ) == 0 ) {

			Common::Com_Printf( "GLimp_Init( ) - ChoosePixelFormat failed\n" );
			return false;
		}
		if( SetPixelFormat( glw_state.hDC, pixelformat, &pfd ) == FALSE ) {

			Common::Com_Printf( "GLimp_Init( ) - SetPixelFormat failed\n" );
			return false;
		}
		DescribePixelFormat( glw_state.hDC, pixelformat, sizeof( pfd ), &pfd );

		if( !( pfd.dwFlags & PFD_GENERIC_ACCELERATED ) ) glw_state.mcd_accelerated = false;
		else glw_state.mcd_accelerated = true;
	}

	/*
	* * report if stereo is desired but unavailable
	*/
	if( !( pfd.dwFlags & PFD_STEREO ) && CVarSystem::GetCVarBool( "cl_stereo" ) )  {

		Common::Com_Printf( "...failed to select stereo pixel format\n" );

		CVarSystem::SetCVarBool( "cl_stereo", false );
		gl_state.stereo_enabled = false;
	}

	/*
	* * startup the OpenGL subsystem by creating a context and making
	* * it current
	*/
	if( ( glw_state.hGLRC = wglCreateContext( glw_state.hDC ) ) == 0 )
	{
		Common::Com_Printf( "GLimp_Init( ) - wglCreateContext failed\n" );

		goto fail;
	}

    if( !wglMakeCurrent( glw_state.hDC, glw_state.hGLRC ) )
	{
		Common::Com_Printf( "GLimp_Init( ) - wglMakeCurrent failed\n" );

		goto fail;
	}

	/*
	* * print out PFD specifics 
	*/
	Common::Com_Printf( "GL PFD: color( %d-bits ) Z( %d-bit )\n", ( int ) pfd.cColorBits, ( int ) pfd.cDepthBits );

	return true;

fail:
	if( glw_state.hGLRC )
	{
		wglDeleteContext( glw_state.hGLRC );
		glw_state.hGLRC = NULL;
	}

	if( glw_state.hDC )
	{
		ReleaseDC( glw_state.hWnd, glw_state.hDC );
		glw_state.hDC = NULL;
	}
	return false;
}

/*
* * GLimp_BeginFrame
*/
void Renderer::GLimp_BeginFrame( float camera_separation ) {

	if( gl_bitdepth.IsModified( ) ) gl_bitdepth.ClearModified( ); // FIXME

	if( camera_separation < 0 && gl_state.stereo_enabled )			glDrawBuffer( GL_BACK_LEFT );

	else if( camera_separation > 0 && gl_state.stereo_enabled )		glDrawBuffer( GL_BACK_RIGHT );

	else															glDrawBuffer( GL_BACK );
}

/*
* * GLimp_EndFrame
* * 
* * Responsible for doing a swapbuffers and possibly for other stuff
* * as yet to be determined.  Probably better not to make this a GLimp
* * function and instead do a call to GLimp_SwapBuffers.
*/
void Renderer::EndFrame( ) {

	int		err;

	err = glGetError( );
	assert( err == GL_NO_ERROR );

	if( Str::Icmp( gl_drawbuffer.GetString( ), "GL_BACK" ) == 0 ) {

		if( !SwapBuffers( glw_state.hDC ) ) Common::Com_Error( ERR_FATAL, "GLimp_EndFrame( ) - SwapBuffers( ) failed!\n" );
	}
}

/*
* * GLimp_AppActivate
*/
void Renderer::AppActivate( bool active ) {

	if( active ) {

		SetForegroundWindow( glw_state.hWnd );
		ShowWindow( glw_state.hWnd, SW_RESTORE );

	} else if( Common::vid_fullscreen.GetBool( ) ) ShowWindow( glw_state.hWnd, SW_MINIMIZE );
}
