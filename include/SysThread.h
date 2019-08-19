#ifndef SYSTHREAD_H
#define SYSTHREAD_H

// // //// // //// // //// //
/// threadPriority_e
// //// // //// // //// //

typedef enum {
	THREAD_LOWEST,
	THREAD_BELOW_NORMAL,
	THREAD_NORMAL,
	THREAD_ABOVE_NORMAL,
	THREAD_HIGHEST
} threadPriority_e;

// // //// // //// // //// //
/// SysThread
// //// // //// // //// //

class SysThread {
private:
#ifndef _WIN32
	static void *									g_setPriorityProc( void * params );
	static threadProc_t								g_createProc;
	static void *									g_createParms;
	static int										g_priority;
#endif
													SysThread( );

public:
	static bool										Create( threadProc_t proc, void * params, threadHandle_t & handle, threadPriority_e priority = THREAD_NORMAL, unsigned int stackSize = 0 );
	static bool										Start( threadHandle_t & handle );
	static bool										Suspend( threadHandle_t & handle );
	static bool										Resume( threadHandle_t & handle );
	static unsigned int								Exit( unsigned int ret_val );
	static void										Join( threadHandle_t & handle );
	static void										Destroy( threadHandle_t & handle );
	static void										SetPriority( threadHandle_t & handle, const threadPriority_e priority );
	static void										SetProcessor( threadHandle_t & handle, const unsigned int processor );
	static void										SetName( threadHandle_t & handle, const char * name );
};

#endif
