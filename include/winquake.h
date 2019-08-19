// winquake.h: Win32-specific Quake header file

#define	WINDOW_STYLE	( WS_OVERLAPPED|WS_BORDER|WS_CAPTION|WS_VISIBLE )

extern	WindowsNS::HINSTANCE	global_hInstance;

#if 0
extern WindowsNS::LPDIRECTSOUND pDS;
extern WindowsNS::LPDIRECTSOUNDBUFFER pDSBuf;
#endif

extern WindowsNS::DWORD gSndBufSize;

extern WindowsNS::HWND			cl_hwnd;
extern bool		ActiveApp, Minimized;

extern int		window_center_x, window_center_y;
extern WindowsNS::RECT		window_rect;
