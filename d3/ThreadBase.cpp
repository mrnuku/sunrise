#include "precompiled.h"
#pragma hdrstop

// // //// // //// // //// //
// ThreadParms
// //// // //// // //// //

ThreadParams::ThreadParams( ) : d_thread( NULL ), d_process( NULL ), d_param( NULL ) {
}

// // //// // //// // //// //
// ThreadBase
// //// // //// // //// //

ThreadBase::ThreadBase( ThreadProcess * process, threadPriority_e priority, unsigned int stackSize ) {
	d_priority			= priority;
	d_isWorker			= false;
	d_isRunning			= false;
	d_isStopping		= false;
	d_params.d_process	= process;
	d_params.d_thread	= this;
	SysThread::Create( ( threadProc_t )ThreadProc, &d_params, d_handle, d_priority, stackSize );
}

ThreadBase::~ThreadBase( ) {
	Mem_Free( d_params.d_param );
	d_params.d_param = NULL;
	SysThread::Destroy( d_handle );
}

void ThreadBase::Destroy( ) {
	delete this;
}

bool ThreadBase::Start( const void * param, size_t size ) {
	if( !d_params.d_process || d_isRunning )
		return false;
	if( param ) {
		d_params.d_param = Mem_Alloc( size );
		memcpy( d_params.d_param, param, size );
	} else
		d_params.d_param = NULL;
	d_params.d_process->Start( );	
	if( !SysThread::Start( d_handle ) )
		return false;
	d_isRunning = true;
	return true;
}

bool ThreadBase::StartWorker( const void * param, size_t size ) {
	d_isWorker = true;
#ifdef _WIN32
	d_hEventWorkerDone = WindowsNS::CreateEvent( NULL, FALSE, FALSE, NULL );
	d_hEventMoreWorkToDo = WindowsNS::CreateEvent( NULL, FALSE, FALSE, NULL );
#endif
	bool result = Start( param, size );
#ifdef _WIN32
	WindowsNS::WaitForSingleObject( d_hEventWorkerDone, INFINITE );
#endif
	return result;
}

void ThreadBase::SignalWork( ) {
#ifdef _WIN32
	WindowsNS::SetEvent( d_hEventMoreWorkToDo );
#endif
}

void ThreadBase::Stop( ) {
	d_isStopping = true;
	d_params.d_process->Stop( );
}

void ThreadBase::Join( ) {
	if ( d_isWorker ) {
#ifdef _WIN32
		WindowsNS::WaitForSingleObject( d_hEventWorkerDone, INFINITE );
#endif
	} else {
		SysThread::Join( d_handle );
		Mem_Free( d_params.d_param );
		d_params.d_param = NULL;
	}
}

void ThreadBase::SetPriority( const threadPriority_e priority ) {
	d_priority = priority;
	SysThread::SetPriority( d_handle, d_priority );
}

void ThreadBase::SetProcessor( const unsigned int processor ) {
	SysThread::SetProcessor( d_handle, processor );
}

const char* ThreadBase::GetName( ) const {
	return d_tname.c_str( );
}

void ThreadBase::SetName( const char * name ) {
	d_tname = name;
	SysThread::SetName( d_handle, name );
}

#if defined( _WIN32 )
unsigned int ThreadBase::ThreadProc( ThreadParams * params ) {
	// This doesn't appear to work for suspended threads - so we call it once more on thread start
	SysThread::SetName( params->d_thread->d_handle, params->d_thread->d_tname.c_str( ) );
	unsigned int retVal;
	if( params->d_thread->d_isWorker ) {
		do {
#ifdef _WIN32
			WindowsNS::SignalObjectAndWait( params->d_thread->d_hEventWorkerDone, params->d_thread->d_hEventMoreWorkToDo, INFINITE, FALSE );
#endif
			retVal = params->d_process->Run( params->d_param );
		} while( !params->d_thread->d_isStopping );
	} else
		retVal = params->d_process->Run( params->d_param );
	params->d_thread->d_isRunning = false;
	return SysThread::Exit( retVal );
}
#else
void * ThreadBase::ThreadProc( void * param ) {
	ThreadParams * param = static_cast< ThreadParams * >( param );
	unsigned int retVal = param->d_process->Run( param->d_param );
	return SysThread::Exit( retVal );
}
#endif

// // //// // //// // //// //
// WorkerThreadBase
// //// // //// // //// //

void WorkerThreadBase::Init( ) {
	if( d_name.IsEmpty( ) )
		d_name = "UnnamedThread";
	d_thread = new ThreadBase( this, THREAD_NORMAL );
	d_thread->SetName( d_name );
	d_thread->Start( );
}

void WorkerThreadBase::Shutdown( ) {
	d_thread->Stop( );
	d_signalStart.Set( );
	d_thread->Join( );
	d_thread->Destroy( );
}
