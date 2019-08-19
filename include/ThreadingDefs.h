#ifndef THREADINGDEFS_H
#define THREADINGDEFS_H

#if defined( _WIN32 )

typedef unsigned int ( __stdcall *threadProc_t )( void * );

typedef WindowsNS::HANDLE			signalHandle_t;
typedef WindowsNS::CRITICAL_SECTION	lockHandle_t;

struct threadHandle_t {
	signalHandle_t						d_handle;
	unsigned int						d_id;
};

#else

typedef void * ( *threadProc_t )( void * );

typedef pthread_t					threadHandle_t;
typedef pthread_mutex_t				lockHandle_t;

struct signalHandle_t {
	lockHandle_t						d_mutex;
	pthread_cond_t						d_cond;
	bool								d_signaled;
	bool								d_waiting;
};

#endif

#endif
