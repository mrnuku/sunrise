#ifndef THREAD_H
#define THREAD_H

class ThreadBase;
class ThreadProcess;

// // //// // //// // //// //
/// ThreadParams
// //// // //// // //// //

struct ThreadParams {
	ThreadBase *									d_thread;
	ThreadProcess *									d_process;
	void *											d_param;

													ThreadParams( );
};

// // //// // //// // //// //
/// ThreadBase
// //// // //// // //// //

class ThreadBase {
protected:
#if defined( _WIN32 )
	static unsigned int								ThreadProc( ThreadParams * params );
#else
	static void *									ThreadProc( void * params );
#endif

	ThreadParams									d_params;
	threadPriority_e								d_priority;
	threadHandle_t									d_handle;
	bool											d_isWorker;
	bool											d_isRunning;
	bool											d_isStopping;
	Str												d_tname;

#ifdef _WIN32
	WindowsNS::HANDLE								d_hEventWorkerDone;
	WindowsNS::HANDLE								d_hEventMoreWorkToDo;
#endif
													~ThreadBase( );

public:
													ThreadBase( ThreadProcess * process, threadPriority_e priority = THREAD_NORMAL, unsigned int stackSize = 0 );

	void											Destroy( );

	bool											Start( const void * param = NULL, size_t size = 0 );
	bool											StartWorker( const void * param = NULL, size_t size = 0 );
	void											SignalWork( );
	void											Stop( );
	void											Join( );

	void											SetPriority( const threadPriority_e priority );
	void											SetProcessor( const unsigned int processor );

	bool											IsRunning( ) const;

	const char *									GetName( ) const;
	void											SetName( const char * name );
};

// // //// // //// // //// //
/// WorkerThreadBase
// //// // //// // //// //

class WorkerThreadBase : public ThreadProcess, public NamedObject {
protected:
	ThreadBase *									d_thread;
	Signal											d_signalStart;
	Signal											d_signalEnd;

public:
	void											Init( );
	void											Shutdown( );

	virtual unsigned int							Run( void * param ) = 0;
	void											DoWork( );
	void											Wait( );
};

// // //// // //// // //// //
// ThreadBase
// //// // //// // //// //

INLINE bool ThreadBase::IsRunning( ) const {
	return d_isRunning;
}

// // //// // //// // //// //
// WorkerThreadBase
// //// // //// // //// //

INLINE void WorkerThreadBase::DoWork( ) {
	d_signalStart.Set( );
}

INLINE void WorkerThreadBase::Wait( ) {
	d_signalEnd.Wait( );
}

#endif
