#ifndef GLIMPORTS_H
#define GLIMPORTS_H

#define WGL_CONTEXT_MAJOR_VERSION				0x2091
#define WGL_CONTEXT_MINOR_VERSION				0x2092
#define WGL_CONTEXT_LAYER_PLANE					0x2093
#define WGL_CONTEXT_FLAGS						0x2094
#define WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY	0x8256
#define WGL_NO_RESET_NOTIFICATION				0x8261
#define WGL_LOSE_CONTEXT_ON_RESET				0x8252
#define WGL_CONTEXT_PROFILE_MASK				0x9126
#define WGL_CONTEXT_DEBUG_BIT					0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT		0x0002
#define WGL_CONTEXT_CORE_PROFILE_BIT			0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT	0x00000002
#define WGL_CONTEXT_ROBUST_ACCESS_BIT			0x00000004

ENUM( GLenum, glEnumName ) {

	NONE = GL_NONE, TRUE_ = GL_TRUE,

	FRONT_LEFT = GL_FRONT_LEFT, FRONT_RIGHT, BACK_LEFT, BACK_RIGHT, FRONT, BACK, LEFT, RIGHT, FRONT_AND_BACK, AUX0, AUX1, AUX2, AUX3,

	TEXTURE0 = GL_TEXTURE0, TEXTURE1, TEXTURE2, TEXTURE3, TEXTURE4, TEXTURE5, TEXTURE6, TEXTURE7, TEXTURE8, TEXTURE9, TEXTURE10,
	TEXTURE11, TEXTURE12, TEXTURE13, TEXTURE14, TEXTURE15, TEXTURE16, TEXTURE17, TEXTURE18, TEXTURE19, TEXTURE20,
	TEXTURE21, TEXTURE22, TEXTURE23, TEXTURE24, TEXTURE25, TEXTURE26, TEXTURE27, TEXTURE28, TEXTURE29, TEXTURE30, TEXTURE31,

	FIXED_ONLY = GL_FIXED_ONLY,

	COLOR_ATTACHMENT0 = GL_COLOR_ATTACHMENT0, COLOR_ATTACHMENT1, COLOR_ATTACHMENT2, COLOR_ATTACHMENT3, COLOR_ATTACHMENT4, COLOR_ATTACHMENT5,
	COLOR_ATTACHMENT6, COLOR_ATTACHMENT7, COLOR_ATTACHMENT8, COLOR_ATTACHMENT9, COLOR_ATTACHMENT10, COLOR_ATTACHMENT11, COLOR_ATTACHMENT12,
	COLOR_ATTACHMENT13, COLOR_ATTACHMENT14, COLOR_ATTACHMENT15,

	ALREADY_SIGNALED = GL_ALREADY_SIGNALED, TIMEOUT_EXPIRED, CONDITION_SATISFIED, WAIT_FAILED_
};

typedef WindowsNS::PROC( APIENTRY * PFNWGLGETPROCADDRESSPROC )( WindowsNS::LPCSTR );
typedef WindowsNS::HGLRC( APIENTRY * PFNWGLCREATECONTEXTPROC )( WindowsNS::HDC );
typedef WindowsNS::HGLRC ( APIENTRY * PFNWGLCREATECONTEXTATTRIBSPROC )( WindowsNS::HDC hDC, WindowsNS::HGLRC hshareContext, const int * attribList );
typedef WindowsNS::BOOL( APIENTRY * PFNWGLDELETECONTEXTPROC )( WindowsNS::HGLRC );
typedef WindowsNS::BOOL( APIENTRY * PFNWGLMAKECURRENTPROC )( WindowsNS::HDC, WindowsNS::HGLRC );
typedef WindowsNS::HGLRC( APIENTRY * PFNWGLGETCURRENTCONTEXTPROC )( );
typedef bool( APIENTRY * PFNWGLSWAPINTERVALPROC )( int );

typedef void( APIENTRY * PFNGLENABLEPROC )( GLenum cap );
typedef void( APIENTRY * PFNGLDISABLEPROC )( GLenum cap );
typedef void( APIENTRY * PFNGLFINISHPROC )( );
typedef void( APIENTRY * PFNGLFLUSHPROC )( );

typedef GLenum( APIENTRY * PFNGLGETERRORPROC )( );
typedef void( APIENTRY * PFNGLGETFLOATVPROC )( GLenum pname, GLfloat * params );
typedef void( APIENTRY * PFNGLGETINTEGERVPROC )( GLenum pname, GLint * params );
typedef void( APIENTRY * PFNGLGETBOOLEANVPROC )( GLenum pname, GLboolean * params );

typedef void( APIENTRY * PFNGLREADBUFFERPROC )( GLenum mode );
typedef void( APIENTRY * PFNGLREADPIXELSPROC )( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid * pixels );

typedef void( APIENTRY * PFNGLPOLYGONMODEPROC )( GLenum face, GLenum mode );
typedef void( APIENTRY * PFNGLPOINTSIZEPROC )( GLfloat size );
typedef void( APIENTRY * PFNGLCULLFACEPROC )( GLenum mode );
typedef void( APIENTRY * PFNGLBLENDFUNCPROC )( GLenum sfactor, GLenum dfactor );
typedef void( APIENTRY * PFNGLDEPTHFUNCPROC )( GLenum func );
typedef void( APIENTRY * PFNGLDEPTHMASKPROC )( GLboolean flag );
typedef void( APIENTRY * PFNGLCOLORMASKPROC )( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha );
typedef void( APIENTRY * PFNGLVIEWPORTPROC )( GLint x, GLint y, GLsizei width, GLsizei height );
typedef void( APIENTRY * PFNGLSCISSORPROC )( GLint x, GLint y, GLsizei width, GLsizei height );
typedef void( APIENTRY * PFNGLCLEARPROC )( GLbitfield mask );
typedef void( APIENTRY * PFNGLCLEARCOLORPROC )( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha );
typedef void( APIENTRY * PFNGLCLEARDEPTHPROC )( GLclampd depth );
typedef void( APIENTRY * PFNGLCLEARSTENCILPROC )( GLint s );

typedef void( APIENTRY * PFNGLTEXIMAGE2DPROC )( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * pixels );
typedef void( APIENTRY * PFNGLTEXPARAMETERIPROC )( GLenum target, GLenum pname, GLint param );

// // //// // //// // //// //
/// GLContext
// //// // //// // //// //

class GLContext {
private:
	WindowsNS::HDC									d_deviceID;
	WindowsNS::HGLRC								d_contextID;
	bool											d_isChild;
	static int										g_attribList[ ];
	static WindowsNS::HGLRC							g_parentID;

public:
													GLContext( );

	WindowsNS::HDC &								DeviceID( ) { return d_deviceID; }
	WindowsNS::HGLRC &								ContextID( ) { return d_contextID; }

	bool											operator ==( const GLContext & other ) const { return d_contextID == other.d_contextID; }

	void											Init( bool child_context );
	void											Shutdown( );

	void											SwapBuffers( );

	void											Bind( );
	static void										BindDefault( );

	static void										InitParent( );
};

// // //// // //// // //// //
/// GLScopedContext
// //// // //// // //// //

class GLScopedContext : public GLContext {
public:
													GLScopedContext( );
													~GLScopedContext( );
};

// // //// // //// // //// //
/// GLOSLayer
//
/// Windows GL specific code
// //// // //// // //// //

class GLOSLayer {
private:
	static WindowsNS::HINSTANCE						hInstance;
	static WindowsNS::WNDPROC						wndProc;

	static WindowsNS::HDC							hDC;
	static WindowsNS::HWND							hWnd;
	static CAList< GLContext, 32 >					g_contextList;

	static bool										fullScreen;
	static Vec2										sizeScreen;

public:
	static bool										Create( );
	static bool										SetMode( const Vec2 & size, bool fs );
	static void										Shutdown( );
	static bool										Init( void * hinstance, void * wndproc );
	static bool										InitGL( );
	static void										BeginFrame( float camera_separation );
	static void										EndFrame( );
	static void										AppActivate( bool active );

	static GLContext &								CreateContext( bool child_context );
	static void										DeleteContext( GLContext & context );
	static GLContext &								GetContext( const int index );
};

// // //// // //// // //// //
/// enumNamePair
// //// // //// // //// //

typedef struct enumNamePair_s {
	GLenum								type;
	const Str							name;
} enumNamePair_t;

// // //// // //// // //// //
/// GLImports
//
/// Helps maintaining compatibility between various OS GL implementation and the engine
/// Currently this class contains all GL callbacks used by the engine
// //// // //// // //// //

class GLImports {
private:
	static DynamicModule *							glModule;
	static WindowsNS::PROC							GetProcCheck( const Str & func_name );
	static WindowsNS::PROC							GetProcSafe( const Str & func_name );
	static enumNamePair_t							glEnumNames[ ];

public:
	static PFNWGLGETPROCADDRESSPROC					wglGetProcAddress;
	static PFNWGLCREATECONTEXTPROC					wglCreateContext;
	static PFNWGLCREATECONTEXTATTRIBSPROC			wglCreateContextAttribs;
	static PFNWGLDELETECONTEXTPROC					wglDeleteContext;
	static PFNWGLMAKECURRENTPROC					wglMakeCurrent;
	static PFNWGLGETCURRENTCONTEXTPROC				wglGetCurrentContext;
	static PFNWGLSWAPINTERVALPROC					wglSwapInterval;

	static PFNGLENABLEPROC							glEnable;
	static PFNGLDISABLEPROC							glDisable;

	static PFNGLGETERRORPROC						glGetError;
	static PFNGLGETFLOATVPROC						glGetFloatv;
	static PFNGLGETINTEGERVPROC						glGetIntegerv;
	static PFNGLGETBOOLEANVPROC						glGetBooleanv;

	static PFNGLREADBUFFERPROC						glReadBuffer;
	static PFNGLREADPIXELSPROC						glReadPixels;
	static PFNGLCLAMPCOLORPROC						glClampColor;

	static PFNGLPOLYGONMODEPROC						glPolygonMode;
	static PFNGLPOINTSIZEPROC						glPointSize;
	static PFNGLCULLFACEPROC						glCullFace;
	static PFNGLBLENDFUNCPROC						glBlendFunc;
	static PFNGLDEPTHFUNCPROC						glDepthFunc;
	static PFNGLDEPTHMASKPROC						glDepthMask;
	static PFNGLCOLORMASKPROC						glColorMask;
	static PFNGLVIEWPORTPROC						glViewport;
	static PFNGLSCISSORPROC							glScissor;
	static PFNGLCLEARPROC							glClear;
	static PFNGLCLEARCOLORPROC						glClearColor;
	static PFNGLCLEARDEPTHPROC						glClearDepth;
	static PFNGLCLEARSTENCILPROC					glClearStencil;

	static PFNGLBLENDFUNCSEPARATEPROC				glBlendFuncSeparate;
	static PFNGLSTENCILFUNCSEPARATEPROC				glStencilFuncSeparate;
	static PFNGLSTENCILMASKSEPARATEPROC				glStencilMaskSeparate;
	static PFNGLSTENCILOPSEPARATEATIPROC			glStencilOpSeparate;

	static PFNGLBINDTEXTUREEXTPROC					glBindTexture;
	static PFNGLDELETETEXTURESEXTPROC				glDeleteTextures;
	static PFNGLGENTEXTURESEXTPROC					glGenTextures;
	static PFNGLACTIVETEXTUREPROC					glActiveTexture;
	static PFNGLTEXIMAGE2DPROC						glTexImage2D;
	static PFNGLTEXIMAGE2DMULTISAMPLEPROC			glTexImage2DMultisample;
	static PFNGLTEXSUBIMAGE2DEXTPROC				glTexSubImage2D;
	static PFNGLTEXPARAMETERIPROC					glTexParameteri;
	static PFNGLCOPYTEXIMAGE1DEXTPROC				glCopyTexImage1D;
	static PFNGLCOPYTEXIMAGE2DEXTPROC				glCopyTexImage2D;
	static PFNGLCOPYTEXSUBIMAGE1DEXTPROC			glCopyTexSubImage1D;
	static PFNGLCOPYTEXSUBIMAGE2DEXTPROC			glCopyTexSubImage2D;
	static PFNGLCOPYTEXSUBIMAGE3DEXTPROC			glCopyTexSubImage3D;
	static PFNGLCOMPRESSEDTEXIMAGE1DPROC			glCompressedTexImage1D;
	static PFNGLCOMPRESSEDTEXIMAGE2DPROC			glCompressedTexImage2D;
	static PFNGLCOMPRESSEDTEXIMAGE3DPROC			glCompressedTexImage3D;

	static PFNGLCREATESHADERPROC					glCreateShader;
	static PFNGLSHADERSOURCEPROC					glShaderSource;
	static PFNGLCOMPILESHADERPROC					glCompileShader;
	static PFNGLCREATEPROGRAMPROC					glCreateProgram;
	static PFNGLATTACHSHADERPROC					glAttachShader;
	static PFNGLLINKPROGRAMPROC						glLinkProgram;
	static PFNGLUSEPROGRAMPROC						glUseProgram;
	static PFNGLDELETESHADERPROC					glDeleteShader;
	static PFNGLGETPROGRAMIVPROC					glGetProgramiv;
	static PFNGLGETSHADERINFOLOGPROC				glGetShaderInfoLog;
	static PFNGLGETSHADERIVPROC						glGetShaderiv;
	static PFNGLDELETEPROGRAMPROC					glDeleteProgram;

	static PFNGLUNIFORM1FPROC						glUniform1f;
	static PFNGLUNIFORM2FPROC						glUniform2f;
	static PFNGLUNIFORM3FPROC						glUniform3f;
	static PFNGLUNIFORM4FPROC						glUniform4f;
	static PFNGLUNIFORM1IPROC						glUniform1i;
	static PFNGLUNIFORM2IPROC						glUniform2i;
	static PFNGLUNIFORM3IPROC						glUniform3i;
	static PFNGLUNIFORM4IPROC						glUniform4i;
	static PFNGLUNIFORM1FVPROC						glUniform1fv;
	static PFNGLUNIFORM2FVPROC						glUniform2fv;
	static PFNGLUNIFORM3FVPROC						glUniform3fv;
	static PFNGLUNIFORM4FVPROC						glUniform4fv;
	static PFNGLUNIFORM1IVPROC						glUniform1iv;
	static PFNGLUNIFORM2IVPROC						glUniform2iv;
	static PFNGLUNIFORM3IVPROC						glUniform3iv;
	static PFNGLUNIFORM4IVPROC						glUniform4iv;
	static PFNGLUNIFORMMATRIX2FVPROC				glUniformMatrix2fv;
	static PFNGLUNIFORMMATRIX3FVPROC				glUniformMatrix3fv;
	static PFNGLUNIFORMMATRIX4FVPROC				glUniformMatrix4fv;
	static PFNGLGETUNIFORMLOCATIONPROC				glGetUniformLocation;
	static PFNGLGETACTIVEUNIFORMPROC				glGetActiveUniform;
	static PFNGLGETUNIFORMFVPROC					glGetUniformfv;
	static PFNGLGETUNIFORMIVPROC					glGetUniformiv;
	static PFNGLGETATTRIBLOCATIONPROC				glGetAttribLocation;
	static PFNGLGETACTIVEATTRIBPROC					glGetActiveAttrib;

	static PFNGLDRAWBUFFERSPROC						glDrawBuffers;
	static PFNGLBINDFRAMEBUFFERPROC					glBindFramebuffer;
	static PFNGLGENRENDERBUFFERSPROC				glGenRenderbuffers;
	static PFNGLDELETERENDERBUFFERSPROC				glDeleteRenderbuffers;
	static PFNGLFRAMEBUFFERTEXTURE2DPROC			glFramebufferTexture2D;
	static PFNGLCHECKFRAMEBUFFERSTATUSPROC			glCheckFramebufferStatus;
	static PFNGLGENFRAMEBUFFERSPROC					glGenFramebuffers;
	static PFNGLFRAMEBUFFERRENDERBUFFERPROC			glFramebufferRenderbuffer;
	static PFNGLBINDRENDERBUFFERPROC				glBindRenderbuffer;
	static PFNGLRENDERBUFFERSTORAGEPROC				glRenderbufferStorage;
	static PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC	glRenderbufferStorageMultisample;
	static PFNGLDELETEFRAMEBUFFERSPROC				glDeleteFramebuffers;
	static PFNGLBINDFRAGDATALOCATIONPROC			glBindFragDataLocation;
	static PFNGLBLITFRAMEBUFFERPROC					glBlitFramebuffer;
	static PFNGLBLENDEQUATIONPROC					glBlendEquation;
	static PFNGLGENSAMPLERSPROC						glGenSamplers;
	static PFNGLDELETESAMPLERSPROC					glDeleteSamplers;
	static PFNGLBINDSAMPLERPROC						glBindSampler;
	static PFNGLSAMPLERPARAMETERIPROC				glSamplerParameteri;
	static PFNGLSAMPLERPARAMETERFPROC				glSamplerParameterf;
	static PFNGLSAMPLERPARAMETERFVPROC				glSamplerParameterfv;

	static PFNGLGENBUFFERSPROC						glGenBuffers;
	static PFNGLBINDBUFFERPROC						glBindBuffer;
	static PFNGLBUFFERDATAPROC						glBufferData;
	static PFNGLMAPBUFFERRANGEPROC					glMapBufferRange;
	static PFNGLFLUSHMAPPEDBUFFERRANGEPROC			glFlushMappedBufferRange;
	static PFNGLBUFFERSUBDATAPROC					glBufferSubData;
	static PFNGLDELETEBUFFERSPROC					glDeleteBuffers;
	static PFNGLGETBUFFERPARAMETERIVPROC			glGetBufferParameteriv;
	static PFNGLMAPBUFFERPROC						glMapBuffer;
	static PFNGLUNMAPBUFFERPROC						glUnmapBuffer;
	static PFNGLGENVERTEXARRAYSPROC					glGenVertexArrays;
	static PFNGLDELETEVERTEXARRAYSPROC				glDeleteVertexArrays;
	static PFNGLBINDVERTEXARRAYPROC					glBindVertexArray;
	static PFNGLVERTEXATTRIBPOINTERPROC				glVertexAttribPointer;
	static PFNGLENABLEVERTEXATTRIBARRAYPROC			glEnableVertexAttribArray;
	static PFNGLGETVERTEXATTRIBIVPROC				glGetVertexAttribiv;
	static PFNGLGETVERTEXATTRIBPOINTERVPROC			glGetVertexAttribPointerv;
	static PFNGLDRAWELEMENTSBASEVERTEXPROC			glDrawElementsBaseVertex;
	static PFNGLDRAWELEMENTSINSTANCEDPROC			glDrawElementsInstanced;
	static PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC	glDrawElementsInstancedBaseVertex;
	static PFNGLDRAWRANGEELEMENTSPROC				glDrawRangeElements;
	static PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC		glDrawRangeElementsBaseVertex;

	static PFNGLGENQUERIESPROC						glGenQueries;
	static PFNGLDELETEQUERIESPROC					glDeleteQueries;
	static PFNGLISQUERYPROC							glIsQuery;
	static PFNGLBEGINQUERYPROC						glBeginQuery;
	static PFNGLENDQUERYPROC						glEndQuery;
	static PFNGLGETQUERYIVPROC						glGetQueryiv;
	static PFNGLGETQUERYOBJECTIVPROC				glGetQueryObjectiv;
	static PFNGLGETQUERYOBJECTUIVPROC				glGetQueryObjectuiv;

	static PFNGLFENCESYNCPROC						glFenceSync;
	static PFNGLISSYNCPROC							glIsSync;
	static PFNGLDELETESYNCPROC						glDeleteSync;
	static PFNGLCLIENTWAITSYNCPROC					glClientWaitSync;
	static PFNGLWAITSYNCPROC						glWaitSync;
	static PFNGLGETINTEGER64VPROC					glGetInteger64v;
	static PFNGLGETSYNCIVPROC						glGetSynciv;
	static PFNGLFINISHPROC							glFinish;
	static PFNGLFLUSHPROC							glFlush;

	static void										PreInit( );
	static void										Init( );
	static void										Shutdown( );
	static void										GLErrorCheck( );
	static void										FramebufferErrorCheck( );

	/// shader help
	static const Str &								GetEnumString( GLenum type );
	static GLenum									GetEnumValue( const Str & name, bool insensitive = true );
};

// // //// // //// // //// //
// GLImports
// //// // //// // //// //

INLINE WindowsNS::PROC GLImports::GetProcCheck( const Str & func_name ) {
	WindowsNS::PROC func = GLImports::wglGetProcAddress( func_name );
	if( !func )
		Common::Com_Error( ERR_DROP, "Missing GL function \"%s\"", func_name.c_str( ) );
	return func;
}

INLINE WindowsNS::PROC GLImports::GetProcSafe( const Str & func_name ) {
	WindowsNS::PROC func = GLImports::wglGetProcAddress( func_name );
	if( !func )
		func = glModule->FindProc( func_name );
	return func;
}

INLINE void GLImports::FramebufferErrorCheck( ) {
	GLenum glFramebufferStatus = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	assert( glFramebufferStatus == GL_FRAMEBUFFER_COMPLETE );
}

INLINE const Str & GLImports::GetEnumString( GLenum type ) {
	for( int i = 0; glEnumNames[ i ].name.Length( ); i++ )
		if( type == glEnumNames[ i ].type )
			return glEnumNames[ i ].name;
	return null_string;
}

INLINE GLenum GLImports::GetEnumValue( const Str & name, bool insensitive ) {
	for( int i = 0; glEnumNames[ i ].name.Length( ); i++ )
		if( insensitive ? !name.Icmp( glEnumNames[ i ].name ) : !name.Cmp( glEnumNames[ i ].name ) )
			return glEnumNames[ i ].type;
	return 0;
}

INLINE void GLImports::GLErrorCheck( ) {
	GLenum	glErrorValue = GLImports::glGetError( );
	Str		enumName = GetEnumString( glErrorValue );
	if( glErrorValue != GL_NO_ERROR ) {
		Common::Com_Printf( "GLImports::GLErrorCheck - %s\n", enumName.c_str( ) );
		assert( glErrorValue == GL_NO_ERROR );
	}
}

#endif
