#include "precompiled.h"
#pragma hdrstop

#define BYTE								WindowsNS::BYTE
#define WORD								WindowsNS::WORD
#define DWORD								WindowsNS::DWORD
#define LPSTR								WindowsNS::LPSTR
#define ULONG_PTR							WindowsNS::ULONG_PTR

DynamicModule *								GLImports::glModule;

PFNWGLGETPROCADDRESSPROC					GLImports::wglGetProcAddress;
PFNWGLCREATECONTEXTPROC						GLImports::wglCreateContext;
PFNWGLCREATECONTEXTATTRIBSPROC				GLImports::wglCreateContextAttribs;
PFNWGLDELETECONTEXTPROC						GLImports::wglDeleteContext;
PFNWGLMAKECURRENTPROC						GLImports::wglMakeCurrent;
PFNWGLGETCURRENTCONTEXTPROC					GLImports::wglGetCurrentContext;
PFNWGLSWAPINTERVALPROC						GLImports::wglSwapInterval;

PFNGLENABLEPROC								GLImports::glEnable;
PFNGLDISABLEPROC							GLImports::glDisable;

PFNGLGETERRORPROC							GLImports::glGetError;
PFNGLGETFLOATVPROC							GLImports::glGetFloatv;
PFNGLGETINTEGERVPROC						GLImports::glGetIntegerv;
PFNGLGETBOOLEANVPROC						GLImports::glGetBooleanv;

PFNGLREADBUFFERPROC							GLImports::glReadBuffer;
PFNGLREADPIXELSPROC							GLImports::glReadPixels;
PFNGLCLAMPCOLORPROC							GLImports::glClampColor;

PFNGLPOLYGONMODEPROC						GLImports::glPolygonMode;
PFNGLPOINTSIZEPROC							GLImports::glPointSize;
PFNGLCULLFACEPROC							GLImports::glCullFace;
PFNGLBLENDFUNCPROC							GLImports::glBlendFunc;
PFNGLDEPTHFUNCPROC							GLImports::glDepthFunc;
PFNGLDEPTHMASKPROC							GLImports::glDepthMask;
PFNGLCOLORMASKPROC							GLImports::glColorMask;
PFNGLVIEWPORTPROC							GLImports::glViewport;
PFNGLSCISSORPROC							GLImports::glScissor;
PFNGLCLEARPROC								GLImports::glClear;
PFNGLCLEARCOLORPROC							GLImports::glClearColor;
PFNGLCLEARDEPTHPROC							GLImports::glClearDepth;
PFNGLCLEARSTENCILPROC						GLImports::glClearStencil;

PFNGLBLENDFUNCSEPARATEPROC					GLImports::glBlendFuncSeparate;
PFNGLSTENCILFUNCSEPARATEPROC				GLImports::glStencilFuncSeparate;
PFNGLSTENCILMASKSEPARATEPROC				GLImports::glStencilMaskSeparate;
PFNGLSTENCILOPSEPARATEATIPROC				GLImports::glStencilOpSeparate;

PFNGLBINDTEXTUREEXTPROC						GLImports::glBindTexture;
PFNGLDELETETEXTURESEXTPROC					GLImports::glDeleteTextures;
PFNGLGENTEXTURESEXTPROC						GLImports::glGenTextures;
PFNGLACTIVETEXTUREARBPROC					GLImports::glActiveTexture;
PFNGLTEXIMAGE2DPROC							GLImports::glTexImage2D;
PFNGLTEXIMAGE2DMULTISAMPLEPROC				GLImports::glTexImage2DMultisample;
PFNGLTEXSUBIMAGE2DEXTPROC					GLImports::glTexSubImage2D;
PFNGLTEXPARAMETERIPROC						GLImports::glTexParameteri;
PFNGLCOPYTEXIMAGE1DEXTPROC					GLImports::glCopyTexImage1D;
PFNGLCOPYTEXIMAGE2DEXTPROC					GLImports::glCopyTexImage2D;
PFNGLCOPYTEXSUBIMAGE1DEXTPROC				GLImports::glCopyTexSubImage1D;
PFNGLCOPYTEXSUBIMAGE2DEXTPROC				GLImports::glCopyTexSubImage2D;
PFNGLCOPYTEXSUBIMAGE3DEXTPROC				GLImports::glCopyTexSubImage3D;
PFNGLCOMPRESSEDTEXIMAGE1DPROC				GLImports::glCompressedTexImage1D;
PFNGLCOMPRESSEDTEXIMAGE2DPROC				GLImports::glCompressedTexImage2D;
PFNGLCOMPRESSEDTEXIMAGE3DPROC				GLImports::glCompressedTexImage3D;

PFNGLCREATESHADERPROC						GLImports::glCreateShader;
PFNGLSHADERSOURCEPROC						GLImports::glShaderSource;
PFNGLCOMPILESHADERPROC						GLImports::glCompileShader;
PFNGLCREATEPROGRAMPROC						GLImports::glCreateProgram;
PFNGLATTACHSHADERPROC						GLImports::glAttachShader;
PFNGLLINKPROGRAMPROC						GLImports::glLinkProgram;
PFNGLUSEPROGRAMPROC							GLImports::glUseProgram;
PFNGLDELETESHADERPROC						GLImports::glDeleteShader;
PFNGLGETPROGRAMIVPROC						GLImports::glGetProgramiv;
PFNGLGETSHADERINFOLOGPROC					GLImports::glGetShaderInfoLog;
PFNGLGETSHADERIVPROC						GLImports::glGetShaderiv;
PFNGLDELETEPROGRAMPROC						GLImports::glDeleteProgram;

PFNGLUNIFORM1FPROC							GLImports::glUniform1f;
PFNGLUNIFORM2FPROC							GLImports::glUniform2f;
PFNGLUNIFORM3FPROC							GLImports::glUniform3f;
PFNGLUNIFORM4FPROC							GLImports::glUniform4f;
PFNGLUNIFORM1IPROC							GLImports::glUniform1i;
PFNGLUNIFORM2IPROC							GLImports::glUniform2i;
PFNGLUNIFORM3IPROC							GLImports::glUniform3i;
PFNGLUNIFORM4IPROC							GLImports::glUniform4i;
PFNGLUNIFORM1FVPROC							GLImports::glUniform1fv;
PFNGLUNIFORM2FVPROC							GLImports::glUniform2fv;
PFNGLUNIFORM3FVPROC							GLImports::glUniform3fv;
PFNGLUNIFORM4FVPROC							GLImports::glUniform4fv;
PFNGLUNIFORM1IVPROC							GLImports::glUniform1iv;
PFNGLUNIFORM2IVPROC							GLImports::glUniform2iv;
PFNGLUNIFORM3IVPROC							GLImports::glUniform3iv;
PFNGLUNIFORM4IVPROC							GLImports::glUniform4iv;
PFNGLUNIFORMMATRIX2FVPROC					GLImports::glUniformMatrix2fv;
PFNGLUNIFORMMATRIX3FVPROC					GLImports::glUniformMatrix3fv;
PFNGLUNIFORMMATRIX4FVPROC					GLImports::glUniformMatrix4fv;
PFNGLGETUNIFORMLOCATIONPROC					GLImports::glGetUniformLocation;
PFNGLGETACTIVEUNIFORMPROC					GLImports::glGetActiveUniform;
PFNGLGETUNIFORMFVPROC						GLImports::glGetUniformfv;
PFNGLGETUNIFORMIVPROC						GLImports::glGetUniformiv;
PFNGLGETATTRIBLOCATIONPROC					GLImports::glGetAttribLocation;
PFNGLGETACTIVEATTRIBPROC					GLImports::glGetActiveAttrib;

PFNGLDRAWBUFFERSPROC						GLImports::glDrawBuffers;
PFNGLBINDFRAMEBUFFERPROC					GLImports::glBindFramebuffer;
PFNGLGENRENDERBUFFERSPROC					GLImports::glGenRenderbuffers;
PFNGLDELETERENDERBUFFERSPROC				GLImports::glDeleteRenderbuffers;
PFNGLFRAMEBUFFERTEXTURE2DPROC				GLImports::glFramebufferTexture2D;
PFNGLCHECKFRAMEBUFFERSTATUSPROC				GLImports::glCheckFramebufferStatus;
PFNGLGENFRAMEBUFFERSPROC					GLImports::glGenFramebuffers;
PFNGLFRAMEBUFFERRENDERBUFFERPROC			GLImports::glFramebufferRenderbuffer;
PFNGLBINDRENDERBUFFERPROC					GLImports::glBindRenderbuffer;
PFNGLRENDERBUFFERSTORAGEPROC				GLImports::glRenderbufferStorage;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC		GLImports::glRenderbufferStorageMultisample;
PFNGLDELETEFRAMEBUFFERSPROC					GLImports::glDeleteFramebuffers;
PFNGLBINDFRAGDATALOCATIONPROC				GLImports::glBindFragDataLocation;
PFNGLBLITFRAMEBUFFERPROC					GLImports::glBlitFramebuffer;
PFNGLBLENDEQUATIONPROC						GLImports::glBlendEquation;
PFNGLGENSAMPLERSPROC						GLImports::glGenSamplers;
PFNGLDELETESAMPLERSPROC						GLImports::glDeleteSamplers;
PFNGLBINDSAMPLERPROC						GLImports::glBindSampler;
PFNGLSAMPLERPARAMETERIPROC					GLImports::glSamplerParameteri;
PFNGLSAMPLERPARAMETERFPROC					GLImports::glSamplerParameterf;
PFNGLSAMPLERPARAMETERFVPROC					GLImports::glSamplerParameterfv;

PFNGLGENBUFFERSPROC							GLImports::glGenBuffers;
PFNGLBINDBUFFERPROC							GLImports::glBindBuffer;
PFNGLBUFFERDATAPROC							GLImports::glBufferData;
PFNGLMAPBUFFERRANGEPROC						GLImports::glMapBufferRange;
PFNGLFLUSHMAPPEDBUFFERRANGEPROC				GLImports::glFlushMappedBufferRange;
PFNGLBUFFERSUBDATAPROC						GLImports::glBufferSubData;
PFNGLDELETEBUFFERSPROC						GLImports::glDeleteBuffers;
PFNGLGETBUFFERPARAMETERIVPROC				GLImports::glGetBufferParameteriv;
PFNGLMAPBUFFERPROC							GLImports::glMapBuffer;
PFNGLUNMAPBUFFERPROC						GLImports::glUnmapBuffer;
PFNGLGENVERTEXARRAYSPROC					GLImports::glGenVertexArrays;
PFNGLDELETEVERTEXARRAYSPROC					GLImports::glDeleteVertexArrays;
PFNGLBINDVERTEXARRAYPROC					GLImports::glBindVertexArray;
PFNGLVERTEXATTRIBPOINTERPROC				GLImports::glVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC			GLImports::glEnableVertexAttribArray;
PFNGLGETVERTEXATTRIBIVPROC					GLImports::glGetVertexAttribiv;
PFNGLGETVERTEXATTRIBPOINTERVPROC			GLImports::glGetVertexAttribPointerv;
PFNGLDRAWELEMENTSBASEVERTEXPROC				GLImports::glDrawElementsBaseVertex;
PFNGLDRAWELEMENTSINSTANCEDPROC				GLImports::glDrawElementsInstanced;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC	GLImports::glDrawElementsInstancedBaseVertex;
PFNGLDRAWRANGEELEMENTSPROC					GLImports::glDrawRangeElements;
PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC		GLImports::glDrawRangeElementsBaseVertex;

PFNGLGENQUERIESPROC							GLImports::glGenQueries;
PFNGLDELETEQUERIESPROC						GLImports::glDeleteQueries;
PFNGLISQUERYPROC							GLImports::glIsQuery;
PFNGLBEGINQUERYPROC							GLImports::glBeginQuery;
PFNGLENDQUERYPROC							GLImports::glEndQuery;
PFNGLGETQUERYIVPROC							GLImports::glGetQueryiv;
PFNGLGETQUERYOBJECTIVPROC					GLImports::glGetQueryObjectiv;
PFNGLGETQUERYOBJECTUIVPROC					GLImports::glGetQueryObjectuiv;

PFNGLFENCESYNCPROC							GLImports::glFenceSync;
PFNGLISSYNCPROC								GLImports::glIsSync;
PFNGLDELETESYNCPROC							GLImports::glDeleteSync;
PFNGLCLIENTWAITSYNCPROC						GLImports::glClientWaitSync;
PFNGLWAITSYNCPROC							GLImports::glWaitSync;
PFNGLGETINTEGER64VPROC						GLImports::glGetInteger64v;
PFNGLGETSYNCIVPROC							GLImports::glGetSynciv;
PFNGLFINISHPROC								GLImports::glFinish;
PFNGLFLUSHPROC								GLImports::glFlush;

WindowsNS::HINSTANCE						GLOSLayer::hInstance;
WindowsNS::WNDPROC							GLOSLayer::wndProc;
WindowsNS::HDC								GLOSLayer::hDC;				// handle to device context
WindowsNS::HWND								GLOSLayer::hWnd;			// handle to window
CAList< GLContext, 32 >						GLOSLayer::g_contextList;	// handle to GL rendering context
bool										GLOSLayer::fullScreen;
Vec2										GLOSLayer::sizeScreen;

WindowsNS::HGLRC							GLContext::g_parentID = NULL;
int											GLContext::g_attribList[ ] = {	WGL_CONTEXT_MAJOR_VERSION,	3,
																			WGL_CONTEXT_MINOR_VERSION,	3,
																			WGL_CONTEXT_PROFILE_MASK,	WGL_CONTEXT_CORE_PROFILE_BIT,
																			WGL_CONTEXT_FLAGS,			0,//WGL_CONTEXT_DEBUG_BIT,
																			0,							0 };

void GLImports::PreInit( ) {

	glModule							= new DynamicModule( "opengl32.dll" );

	wglGetProcAddress					= ( PFNWGLGETPROCADDRESSPROC )					glModule->FindProc( "wglGetProcAddress" );
	wglCreateContext					= ( PFNWGLCREATECONTEXTPROC )					glModule->FindProc( "wglCreateContext" );
	wglDeleteContext					= ( PFNWGLDELETECONTEXTPROC )					glModule->FindProc( "wglDeleteContext" );
	wglMakeCurrent						= ( PFNWGLMAKECURRENTPROC )						glModule->FindProc( "wglMakeCurrent" );
	wglGetCurrentContext				= ( PFNWGLGETCURRENTCONTEXTPROC )				glModule->FindProc( "wglGetCurrentContext" );
}

void GLImports::Init( ) {

	wglCreateContextAttribs				= ( PFNWGLCREATECONTEXTATTRIBSPROC )			GetProcCheck( "wglCreateContextAttribsARB" );
	wglSwapInterval						= ( PFNWGLSWAPINTERVALPROC )					GetProcCheck( "wglSwapIntervalEXT" );

	glEnable							= ( PFNGLENABLEPROC )							glModule->FindProc( "glEnable" );
	glDisable							= ( PFNGLDISABLEPROC )							glModule->FindProc( "glDisable" );

	glGetError							= ( PFNGLGETERRORPROC )							glModule->FindProc( "glGetError" );
	glGetFloatv							= ( PFNGLGETFLOATVPROC )						glModule->FindProc( "glGetFloatv" );
	glGetIntegerv						= ( PFNGLGETINTEGERVPROC )						glModule->FindProc( "glGetIntegerv" );
	glGetBooleanv						= ( PFNGLGETBOOLEANVPROC )						glModule->FindProc( "glGetBooleanv" );

	glReadBuffer						= ( PFNGLREADBUFFERPROC )						glModule->FindProc( "glReadBuffer" );
	glReadPixels						= ( PFNGLREADPIXELSPROC )						glModule->FindProc( "glReadPixels" );
	glClampColor						= ( PFNGLCLAMPCOLORPROC )						GetProcCheck( "glClampColor" );

	glPolygonMode						= ( PFNGLPOLYGONMODEPROC )						glModule->FindProc( "glPolygonMode" );
	glPointSize							= ( PFNGLPOINTSIZEPROC )						glModule->FindProc( "glPointSize" );
	glCullFace							= ( PFNGLCULLFACEPROC )							glModule->FindProc( "glCullFace" );
	glBlendFunc							= ( PFNGLBLENDFUNCPROC )						glModule->FindProc( "glBlendFunc" );
	glDepthFunc							= ( PFNGLDEPTHFUNCPROC )						glModule->FindProc( "glDepthFunc" );
	glDepthMask							= ( PFNGLDEPTHMASKPROC )						glModule->FindProc( "glDepthMask" );
	glColorMask							= ( PFNGLCOLORMASKPROC )						glModule->FindProc( "glColorMask" );
	glViewport							= ( PFNGLVIEWPORTPROC )							glModule->FindProc( "glViewport" );
	glScissor							= ( PFNGLSCISSORPROC )							glModule->FindProc( "glScissor" );
	glClear								= ( PFNGLCLEARPROC )							glModule->FindProc( "glClear" );
	glClearColor						= ( PFNGLCLEARCOLORPROC )						glModule->FindProc( "glClearColor" );
	glClearDepth						= ( PFNGLCLEARDEPTHPROC )						glModule->FindProc( "glClearDepth" );
	glClearStencil						= ( PFNGLCLEARSTENCILPROC )						glModule->FindProc( "glClearStencil" );

	glBlendFuncSeparate					= ( PFNGLBLENDFUNCSEPARATEPROC )				GetProcCheck( "glBlendFuncSeparate" );
	glStencilFuncSeparate				= ( PFNGLSTENCILFUNCSEPARATEPROC )				GetProcCheck( "glStencilFuncSeparate" );
	glStencilMaskSeparate				= ( PFNGLSTENCILMASKSEPARATEPROC )				GetProcCheck( "glStencilMaskSeparate" );
	glStencilOpSeparate					= ( PFNGLSTENCILOPSEPARATEATIPROC )				GetProcCheck( "glStencilOpSeparate" );

	glBindTexture						= ( PFNGLBINDTEXTUREEXTPROC )					GetProcSafe( "glBindTexture" );
	glDeleteTextures					= ( PFNGLDELETETEXTURESEXTPROC )				GetProcSafe( "glDeleteTextures" );
	glGenTextures						= ( PFNGLGENTEXTURESEXTPROC )					GetProcSafe( "glGenTextures" );
	glActiveTexture						= ( PFNGLACTIVETEXTUREPROC )					GetProcSafe( "glActiveTexture" );
	glTexImage2D						= ( PFNGLTEXIMAGE2DPROC )						GetProcSafe( "glTexImage2D" );
	glTexImage2DMultisample				= ( PFNGLTEXIMAGE2DMULTISAMPLEPROC )			GetProcSafe( "glTexImage2DMultisample" );
	glTexSubImage2D						= ( PFNGLTEXSUBIMAGE2DEXTPROC )					GetProcSafe( "glTexSubImage2D" );
	glTexParameteri						= ( PFNGLTEXPARAMETERIPROC )					GetProcSafe( "glTexParameteri" );
	glCopyTexImage1D					= ( PFNGLCOPYTEXIMAGE1DEXTPROC )				GetProcSafe( "glCopyTexImage1D" );
	glCopyTexImage2D					= ( PFNGLCOPYTEXIMAGE2DEXTPROC )				GetProcSafe( "glCopyTexImage2D" );
	glCopyTexSubImage1D					= ( PFNGLCOPYTEXSUBIMAGE1DEXTPROC )				GetProcSafe( "glCopyTexSubImage1D" );
	glCopyTexSubImage2D					= ( PFNGLCOPYTEXSUBIMAGE2DEXTPROC )				GetProcSafe( "glCopyTexSubImage2D" );
	glCopyTexSubImage3D					= ( PFNGLCOPYTEXSUBIMAGE3DEXTPROC )				GetProcSafe( "glCopyTexSubImage3D" );
	glCompressedTexImage1D				= ( PFNGLCOMPRESSEDTEXIMAGE1DPROC )				GetProcSafe( "glCompressedTexImage1D" );
	glCompressedTexImage2D				= ( PFNGLCOMPRESSEDTEXIMAGE2DPROC )				GetProcSafe( "glCompressedTexImage2D" );
	glCompressedTexImage3D				= ( PFNGLCOMPRESSEDTEXIMAGE3DPROC )				GetProcSafe( "glCompressedTexImage3D" );

	glCreateShader						= ( PFNGLCREATESHADERPROC )						GetProcCheck( "glCreateShader" );
	glShaderSource						= ( PFNGLSHADERSOURCEPROC )						GetProcCheck( "glShaderSource" );
	glCompileShader						= ( PFNGLCOMPILESHADERPROC )					GetProcCheck( "glCompileShader" );
	glCreateProgram						= ( PFNGLCREATEPROGRAMPROC )					GetProcCheck( "glCreateProgram" );
	glAttachShader						= ( PFNGLATTACHSHADERPROC )						GetProcCheck( "glAttachShader" );
	glLinkProgram						= ( PFNGLLINKPROGRAMPROC )						GetProcCheck( "glLinkProgram" );
	glUseProgram						= ( PFNGLUSEPROGRAMPROC )						GetProcCheck( "glUseProgram" );
	glDeleteShader						= ( PFNGLDELETESHADERPROC )						GetProcCheck( "glDeleteShader" );
	glGetProgramiv						= ( PFNGLGETPROGRAMIVPROC )						GetProcCheck( "glGetProgramiv" );
	glGetShaderInfoLog					= ( PFNGLGETSHADERINFOLOGPROC )					GetProcCheck( "glGetShaderInfoLog" );
	glGetShaderiv						= ( PFNGLGETSHADERIVPROC )						GetProcCheck( "glGetShaderiv" );
	glDeleteProgram						= ( PFNGLDELETEPROGRAMPROC )					GetProcCheck( "glDeleteProgram" );

	glUniform1f							= ( PFNGLUNIFORM1FPROC )						GetProcCheck( "glUniform1f" );
	glUniform2f							= ( PFNGLUNIFORM2FPROC )						GetProcCheck( "glUniform2f" );
	glUniform3f							= ( PFNGLUNIFORM3FPROC )						GetProcCheck( "glUniform3f" );
	glUniform4f							= ( PFNGLUNIFORM4FPROC )						GetProcCheck( "glUniform4f" );
	glUniform1i							= ( PFNGLUNIFORM1IPROC )						GetProcCheck( "glUniform1i" );
	glUniform2i							= ( PFNGLUNIFORM2IPROC )						GetProcCheck( "glUniform2i" );
	glUniform3i							= ( PFNGLUNIFORM3IPROC )						GetProcCheck( "glUniform3i" );
	glUniform4i							= ( PFNGLUNIFORM4IPROC )						GetProcCheck( "glUniform4i" );
	glUniform1fv						= ( PFNGLUNIFORM1FVPROC )						GetProcCheck( "glUniform1fv" );
	glUniform2fv						= ( PFNGLUNIFORM2FVPROC )						GetProcCheck( "glUniform2fv" );
	glUniform3fv						= ( PFNGLUNIFORM3FVPROC )						GetProcCheck( "glUniform3fv" );
	glUniform4fv						= ( PFNGLUNIFORM4FVPROC )						GetProcCheck( "glUniform4fv" );
	glUniform1iv						= ( PFNGLUNIFORM1IVPROC )						GetProcCheck( "glUniform1iv" );
	glUniform2iv						= ( PFNGLUNIFORM2IVPROC )						GetProcCheck( "glUniform2iv" );
	glUniform3iv						= ( PFNGLUNIFORM3IVPROC )						GetProcCheck( "glUniform3iv" );
	glUniform4iv						= ( PFNGLUNIFORM4IVPROC )						GetProcCheck( "glUniform4iv" );
	glUniformMatrix2fv					= ( PFNGLUNIFORMMATRIX2FVPROC )					GetProcCheck( "glUniformMatrix2fv" );
	glUniformMatrix3fv					= ( PFNGLUNIFORMMATRIX3FVPROC )					GetProcCheck( "glUniformMatrix3fv" );
	glUniformMatrix4fv					= ( PFNGLUNIFORMMATRIX4FVPROC )					GetProcCheck( "glUniformMatrix4fv" );
	glGetUniformLocation				= ( PFNGLGETUNIFORMLOCATIONPROC )				GetProcCheck( "glGetUniformLocation" );
	glGetActiveUniform					= ( PFNGLGETACTIVEUNIFORMPROC )					GetProcCheck( "glGetActiveUniform" );
	glGetUniformfv						= ( PFNGLGETUNIFORMFVPROC )						GetProcCheck( "glGetUniformfv" );
	glGetUniformiv						= ( PFNGLGETUNIFORMIVPROC )						GetProcCheck( "glGetUniformiv" );
	glGetAttribLocation					= ( PFNGLGETATTRIBLOCATIONPROC )				GetProcCheck( "glGetAttribLocation" );
	glGetActiveAttrib					= ( PFNGLGETACTIVEATTRIBPROC )					GetProcCheck( "glGetActiveAttrib" );

	glDrawBuffers						= ( PFNGLDRAWBUFFERSPROC )						GetProcCheck( "glDrawBuffers" );
	glBindFramebuffer					= ( PFNGLBINDFRAMEBUFFERPROC )					GetProcCheck( "glBindFramebuffer" );
	glGenRenderbuffers					= ( PFNGLGENRENDERBUFFERSPROC )					GetProcCheck( "glGenRenderbuffers" );
	glDeleteRenderbuffers				= ( PFNGLDELETERENDERBUFFERSPROC )				GetProcCheck( "glDeleteRenderbuffers" );
	glFramebufferTexture2D				= ( PFNGLFRAMEBUFFERTEXTURE2DPROC )				GetProcCheck( "glFramebufferTexture2D" );
	glCheckFramebufferStatus			= ( PFNGLCHECKFRAMEBUFFERSTATUSPROC )			GetProcCheck( "glCheckFramebufferStatus" );
	glGenFramebuffers					= ( PFNGLGENFRAMEBUFFERSPROC )					GetProcCheck( "glGenFramebuffers" );
	glFramebufferRenderbuffer			= ( PFNGLFRAMEBUFFERRENDERBUFFERPROC )			GetProcCheck( "glFramebufferRenderbuffer" );
	glBindRenderbuffer					= ( PFNGLBINDRENDERBUFFERPROC )					GetProcCheck( "glBindRenderbuffer" );
	glRenderbufferStorage				= ( PFNGLRENDERBUFFERSTORAGEPROC )				GetProcCheck( "glRenderbufferStorage" );
	glRenderbufferStorageMultisample	= ( PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC )	GetProcCheck( "glRenderbufferStorageMultisample" );
	glDeleteFramebuffers				= ( PFNGLDELETEFRAMEBUFFERSPROC )				GetProcCheck( "glDeleteFramebuffers" );
	glBindFragDataLocation				= ( PFNGLBINDFRAGDATALOCATIONPROC )				GetProcCheck( "glBindFragDataLocation" );
	glBlitFramebuffer					= ( PFNGLBLITFRAMEBUFFERPROC )					GetProcCheck( "glBlitFramebuffer" );
	glBlendEquation						= ( PFNGLBLENDEQUATIONPROC )					GetProcCheck( "glBlendEquation" );
	glGenSamplers						= ( PFNGLGENSAMPLERSPROC )						GetProcCheck( "glGenSamplers" );
	glDeleteSamplers					= ( PFNGLDELETESAMPLERSPROC )					GetProcCheck( "glDeleteSamplers" );
	glBindSampler						= ( PFNGLBINDSAMPLERPROC )						GetProcCheck( "glBindSampler" );
	glSamplerParameteri					= ( PFNGLSAMPLERPARAMETERIPROC )				GetProcCheck( "glSamplerParameteri" );
	glSamplerParameterf					= ( PFNGLSAMPLERPARAMETERFPROC )				GetProcCheck( "glSamplerParameterf" );
	glSamplerParameterfv				= ( PFNGLSAMPLERPARAMETERFVPROC )				GetProcCheck( "glSamplerParameterfv" );

	glGenBuffers						= ( PFNGLGENBUFFERSPROC )						GetProcCheck( "glGenBuffers" );
	glBindBuffer						= ( PFNGLBINDBUFFERPROC )						GetProcCheck( "glBindBuffer" );
	glBufferData						= ( PFNGLBUFFERDATAPROC )						GetProcCheck( "glBufferData" );
	glMapBufferRange					= ( PFNGLMAPBUFFERRANGEPROC )					GetProcCheck( "glMapBufferRange" );
	glFlushMappedBufferRange			= ( PFNGLFLUSHMAPPEDBUFFERRANGEPROC )			GetProcCheck( "glFlushMappedBufferRange" );
	glBufferSubData						= ( PFNGLBUFFERSUBDATAPROC )					GetProcCheck( "glBufferSubData" );
	glDeleteBuffers						= ( PFNGLDELETEBUFFERSPROC )					GetProcCheck( "glDeleteBuffers" );
	glGetBufferParameteriv				= ( PFNGLGETBUFFERPARAMETERIVPROC )				GetProcCheck( "glGetBufferParameteriv" );
	glMapBuffer							= ( PFNGLMAPBUFFERPROC )						GetProcCheck( "glMapBuffer" );
	glUnmapBuffer						= ( PFNGLUNMAPBUFFERPROC )						GetProcCheck( "glUnmapBuffer" );
	glGenVertexArrays					= ( PFNGLGENVERTEXARRAYSPROC )					GetProcCheck( "glGenVertexArrays" );
	glDeleteVertexArrays				= ( PFNGLDELETEVERTEXARRAYSPROC )				GetProcCheck( "glDeleteVertexArrays" );
	glBindVertexArray					= ( PFNGLBINDVERTEXARRAYPROC )					GetProcCheck( "glBindVertexArray" );
	glVertexAttribPointer				= ( PFNGLVERTEXATTRIBPOINTERPROC )				GetProcCheck( "glVertexAttribPointer" );
	glEnableVertexAttribArray			= ( PFNGLENABLEVERTEXATTRIBARRAYPROC )			GetProcCheck( "glEnableVertexAttribArray" );
	glGetVertexAttribiv					= ( PFNGLGETVERTEXATTRIBIVPROC )				GetProcCheck( "glGetVertexAttribiv" );
	glGetVertexAttribPointerv			= ( PFNGLGETVERTEXATTRIBPOINTERVPROC )			GetProcCheck( "glGetVertexAttribPointerv" );
	glDrawElementsBaseVertex			= ( PFNGLDRAWELEMENTSBASEVERTEXPROC )			GetProcCheck( "glDrawElementsBaseVertex" );
	glDrawElementsInstanced				= ( PFNGLDRAWELEMENTSINSTANCEDPROC )			GetProcCheck( "glDrawElementsInstanced" );
	glDrawElementsInstancedBaseVertex	= ( PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC )	GetProcCheck( "glDrawElementsInstancedBaseVertex" );
	glDrawRangeElements					= ( PFNGLDRAWRANGEELEMENTSPROC )				GetProcCheck( "glDrawRangeElements" );
	glDrawRangeElementsBaseVertex		= ( PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC )		GetProcCheck( "glDrawRangeElementsBaseVertex" );

	glGenQueries						= ( PFNGLGENQUERIESPROC )						GetProcCheck( "glGenQueries" );
	glDeleteQueries						= ( PFNGLDELETEQUERIESPROC )					GetProcCheck( "glDeleteQueries" );
	glIsQuery							= ( PFNGLISQUERYPROC )							GetProcCheck( "glIsQuery" );
	glBeginQuery						= ( PFNGLBEGINQUERYPROC )						GetProcCheck( "glBeginQuery" );
	glEndQuery							= ( PFNGLENDQUERYPROC )							GetProcCheck( "glEndQuery" );
	glGetQueryiv						= ( PFNGLGETQUERYIVPROC )						GetProcCheck( "glGetQueryiv" );
	glGetQueryObjectiv					= ( PFNGLGETQUERYOBJECTIVPROC )					GetProcCheck( "glGetQueryObjectiv" );
	glGetQueryObjectuiv					= ( PFNGLGETQUERYOBJECTUIVPROC )				GetProcCheck( "glGetQueryObjectuiv" );

	glFenceSync							= ( PFNGLFENCESYNCPROC )						GetProcCheck( "glFenceSync" );
	glIsSync							= ( PFNGLISSYNCPROC )							GetProcCheck( "glIsSync" );
	glDeleteSync						= ( PFNGLDELETESYNCPROC )						GetProcCheck( "glDeleteSync" );
	glClientWaitSync					= ( PFNGLCLIENTWAITSYNCPROC )					GetProcCheck( "glClientWaitSync" );
	glWaitSync							= ( PFNGLWAITSYNCPROC )							GetProcCheck( "glWaitSync" );
	glGetInteger64v						= ( PFNGLGETINTEGER64VPROC )					GetProcCheck( "glGetInteger64v" );
	glGetSynciv							= ( PFNGLGETSYNCIVPROC )						GetProcCheck( "glGetSynciv" );
	glFinish							= ( PFNGLFINISHPROC )							glModule->FindProc( "glFinish" );
	glFlush								= ( PFNGLFLUSHPROC )							glModule->FindProc( "glFlush" );
}

void GLImports::Shutdown( ) {
	delete glModule;
}

/*
* * VID_CreateWindow
*/
#define	WINDOW_CLASS_NAME	"chaos0"

bool GLOSLayer::Create( ) {
	WindowsNS::WNDCLASS		wc;
	WindowsNS::RECT			r;
	int						stylebits, x, y, w, h, exstyle;
	/* Register the frame class */
    wc.style         = 0;
    wc.lpfnWndProc   = wndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = 0;
    wc.hCursor       = WindowsNS::LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = ( WindowsNS::HBRUSH )0;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = WINDOW_CLASS_NAME;

    if( !RegisterClass( &wc ) )
		Common::Com_Error( ERR_FATAL, "Couldn't register window class" );

	if( fullScreen ) {
		exstyle = WS_EX_TOPMOST;
		stylebits = WS_POPUP | WS_VISIBLE;
	} else {
		exstyle = 0;
		stylebits = WINDOW_STYLE;
	}

	r.left		= 0;
	r.top		= 0;
	r.right		= ( WindowsNS::LONG )sizeScreen[ 0 ];
	r.bottom	= ( WindowsNS::LONG )sizeScreen[ 1 ];
	WindowsNS::AdjustWindowRect( &r, stylebits, FALSE );
	w = r.right - r.left;
	h = r.bottom - r.top;
	if( fullScreen ) {
		x = 0;
		y = 0;
	} else {
		x = Common::vid_xpos.GetInt( );
		y = Common::vid_ypos.GetInt( );
	}
	hWnd = WindowsNS::CreateWindowExA( exstyle, WINDOW_CLASS_NAME, "SunRise Engine "VERSION, stylebits, x, y, w, h, NULL, NULL, hInstance, NULL );
	if( !hWnd )
		Common::Com_Error( ERR_FATAL, "CreateWindowEx failed" );	
	WindowsNS::ShowWindow( hWnd, SW_SHOW );
	WindowsNS::UpdateWindow( hWnd );
	if( !InitGL( ) ) { // init all the gl stuff for the window
		Common::Com_Printf( "GLOSLayer::InitGL failed\n" );
		return false;
	}
	WindowsNS::SetForegroundWindow( hWnd );
	WindowsNS::SetFocus( hWnd );
	Video::VID_NewWindow( ( int )sizeScreen[ 0 ], ( int )sizeScreen[ 1 ] ); // let the sound and input subsystems know about the new window
	return true;
}

bool GLOSLayer::SetMode( const Vec2 & size, bool fs ) {
	if( hWnd )
		Shutdown( );
	fullScreen = fs;
	if( fullScreen ) {
		WindowsNS::DEVMODE dm;
		Common::Com_Memset( &dm, 0, sizeof( dm ) );
		dm.dmSize = sizeof( dm );
		dm.dmPelsWidth  = ( DWORD )size[ 0 ];
		dm.dmPelsHeight = ( DWORD )size[ 1 ];
		dm.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT;
		if( ChangeDisplaySettings( &dm, CDS_FULLSCREEN ) == DISP_CHANGE_SUCCESSFUL ) {
			sizeScreen = size;
			if( !Create( ) )
				return false;
			return true;
		}
	} else {		
		WindowsNS::ChangeDisplaySettings( 0, 0 );
		sizeScreen = size;
		if( !Create( ) )
			return false;
	}
	return true;
}

void GLOSLayer::Shutdown( ) {
	for( int i = 0; i < g_contextList.Num( ); i++ )
		DeleteContext( g_contextList[ i ] );

	if( hDC ) {
		WindowsNS::ReleaseDC( hWnd, hDC );
		hDC = NULL;
	}
	if( hWnd ) {
		WindowsNS::DestroyWindow( hWnd );
		hWnd = NULL;
	}
	UnregisterClass( WINDOW_CLASS_NAME, hInstance );
	if( fullScreen ) {
		WindowsNS::ChangeDisplaySettings( 0, 0 );
		fullScreen = false;
	}
}

bool GLOSLayer::Init( void * hinstance, void * wndproc ) {
	hInstance = ( WindowsNS::HINSTANCE )hinstance;
	wndProc = ( WindowsNS::WNDPROC )wndproc;
	return true;
}

bool GLOSLayer::InitGL( ) {
    WindowsNS::PIXELFORMATDESCRIPTOR	pfd;
	int									bestPixelFormat = 0;
	WindowsNS::HDC rootHdc = WindowsNS::GetDC( NULL );
	BYTE desiredColorBits = WindowsNS::GetDeviceCaps( rootHdc, BITSPIXEL );
	WindowsNS::ReleaseDC( 0, rootHdc );
	Common::Com_Memset( &pfd, 0, sizeof( WindowsNS::PIXELFORMATDESCRIPTOR ) );
	pfd.nSize = sizeof( WindowsNS::PIXELFORMATDESCRIPTOR );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER | PFD_DEPTH_DONTCARE;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = desiredColorBits;
    if( ( hDC = WindowsNS::GetDC( hWnd ) ) == NULL ) {
		Common::Com_Printf( "GLOSLayer::InitGL( ) - GetDC failed\n" );
		return false;
	}
	bestPixelFormat = WindowsNS::ChoosePixelFormat( hDC, &pfd );
	if( !bestPixelFormat ) {
		Common::Com_Printf( "GLOSLayer::InitGL( ) - cant get suitable PFD\n" );
		return false;
	}
	if( WindowsNS::SetPixelFormat( hDC, bestPixelFormat, &pfd ) == FALSE ) {
		Common::Com_Printf( "GLOSLayer::InitGL( ) - SetPixelFormat failed\n" );
		return false;
	}
	CreateContext( false ).Bind( );
	return true;
}

/*
* * GLimp_BeginFrame
*/
void GLOSLayer::BeginFrame( float camera_separation ) {
}

/*
* * GLimp_EndFrame
* * 
* * Responsible for doing a swapbuffers and possibly for other stuff
* * as yet to be determined.  Probably better not to make this a GLimp
* * function and instead do a call to GLimp_SwapBuffers.
*/
void GLOSLayer::EndFrame( ) {
	//GLImports::GLErrorCheck( );
	//if( !WindowsNS::SwapBuffers( hDC ) ) Common::Com_Error( ERR_FATAL, "GLOSLayer::EndFrame - SwapBuffers failed!\n" );
}

/*
* * GLimp_AppActivate
*/
void GLOSLayer::AppActivate( bool active ) {
	if( active ) {
		WindowsNS::SetForegroundWindow( hWnd );
		WindowsNS::ShowWindow( hWnd, SW_RESTORE );
	} else if( Common::vid_fullscreen.GetBool( ) )
		WindowsNS::ShowWindow( hWnd, SW_MINIMIZE );
}

GLContext & GLOSLayer::CreateContext( bool child_context ) {
	int contextIndex = g_contextList.Append( GLContext( ) );
	g_contextList[ contextIndex ].DeviceID( ) = hDC;
	g_contextList[ contextIndex ].Init( child_context );
	return g_contextList[ contextIndex ];
}
	
void GLOSLayer::DeleteContext( GLContext & context ) {
	for( int i = 0; i < g_contextList.Num( ); i++ ) {
		if( g_contextList[ i ] == context ) {
			g_contextList[ i ].Shutdown( );
			g_contextList.RemoveIndex( i );
			return;
		}
	}
	Common::Com_Printf( "GLOSLayer::DeleteContext( ) - context not found\n" );
	return;
}

GLContext & GLOSLayer::GetContext( const int index ) {
	return g_contextList[ index ];
}

// // //// // //// // //// //
// GLContext
// //// // //// // //// //

GLContext::GLContext( ) {
	d_deviceID		= NULL;
	d_contextID		= NULL;
	d_isChild		= false;
}

void GLContext::Init( bool child_context ) {
	d_isChild = child_context;
	if( d_isChild ) {		
		WindowsNS::HGLRC prevContext = GLImports::wglGetCurrentContext( );
		if( prevContext )
			BindDefault( );
		d_contextID = GLImports::wglCreateContextAttribs( d_deviceID, g_parentID, g_attribList );
		if( d_contextID )			
			Bind( );
		if( !d_contextID ) {
			DWORD errorCode = WindowsNS::GetLastError( );
			Common::Com_Printf( "GLContext::Init - wglCreateContextAttribs failed, GetLastError returned with 0x%X\n", errorCode );
		}
	} else {
		d_contextID = GLImports::wglCreateContext( d_deviceID );
		if( d_contextID )
			Bind( );
	}
	if( !d_contextID )
		Common::Com_Error( ERR_DROP, "GLContext::Init - context creation failed\n" );
}

void GLContext::Shutdown( ) {
	for( GLuint vertexArrayID = 1; ; vertexArrayID++ ) {
		GLint vertexParam;
		GLImports::glBindVertexArray( vertexArrayID );
		GLImports::glGetVertexAttribiv(	0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &vertexParam );
		glEnumName err = ( glEnumName )GLImports::glGetError( );
		if( err ) {
			GLImports::glBindVertexArray( 0 );
			GLImports::glGetVertexAttribiv( 0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &vertexParam );
			err = ( glEnumName )GLImports::glGetError( );
			break;
		}
		GLImports::glDeleteVertexArrays( 1, &vertexArrayID );
	}
	GLContext::BindDefault( );
	GLImports::wglDeleteContext( d_contextID );
}

void GLContext::SwapBuffers( ) {
	if( !WindowsNS::SwapBuffers( d_deviceID ) )		
		Common::Com_Error( ERR_DROP, "GLContext::SwapBuffers - SwapBuffers failed!\n" );
}

void GLContext::Bind( ) {
	if( !GLImports::wglMakeCurrent( d_deviceID, d_contextID ) )
		Common::Com_Error( ERR_DROP, "GLContext::Bind - wglMakeCurrent failed\n" );
}

void GLContext::BindDefault( ) {
	GLImports::wglMakeCurrent( NULL, NULL );
}

void GLContext::InitParent( ) {
	GLOSLayer::DeleteContext( GLOSLayer::GetContext( 0 ) );
	GLContext parentContext = GLOSLayer::CreateContext( true );
	g_parentID = parentContext.ContextID( );
}
