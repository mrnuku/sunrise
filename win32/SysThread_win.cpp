#include "precompiled.h"
#pragma hdrstop

// // //// // //// // //// //
// SysThread
// //// // //// // //// //

SysThread::SysThread( ) {
}

bool SysThread::Create( threadProc_t proc, void * params, threadHandle_t & handle, threadPriority_e priority, unsigned int stackSize ) {
	handle.d_handle = reinterpret_cast< WindowsNS::HANDLE >( ::_beginthreadex( NULL, stackSize, proc, params, CREATE_SUSPENDED, &handle.d_id ) );
	if( !handle.d_handle )
		return false;
	SetPriority( handle, priority );
	return true;
}

bool SysThread::Start( threadHandle_t & handle ) {
	if( WindowsNS::ResumeThread( handle.d_handle ) == -1 )
		return false;
	return true;
}

bool SysThread::Suspend( threadHandle_t & handle ) {
	if( WindowsNS::SuspendThread( handle.d_handle ) == -1 )
		return false;
	return true;
}

bool SysThread::Resume( threadHandle_t & handle ) {
	if( WindowsNS::ResumeThread( handle.d_handle ) == -1 )
		return false;
	return true;
}

unsigned int SysThread::Exit( unsigned int ret_val ) {
	::_endthreadex( ret_val );
	return ret_val;
}

void SysThread::Join( threadHandle_t & handle ) {
	WindowsNS::WaitForSingleObject( handle.d_handle, INFINITE );
}

void SysThread::Destroy( threadHandle_t & handle ) {
	WindowsNS::CloseHandle( handle.d_handle );
}

void SysThread::SetPriority( threadHandle_t & handle, const threadPriority_e priority ) {
	switch( priority ) {
		case THREAD_LOWEST:
			WindowsNS::SetThreadPriority( handle.d_handle, THREAD_PRIORITY_LOWEST );
			break;
		case THREAD_BELOW_NORMAL:
			WindowsNS::SetThreadPriority( handle.d_handle, THREAD_PRIORITY_BELOW_NORMAL );
			break;
		default:
		case THREAD_NORMAL:
			WindowsNS::SetThreadPriority( handle.d_handle, THREAD_PRIORITY_NORMAL );
			break;
		case THREAD_ABOVE_NORMAL:
			WindowsNS::SetThreadPriority( handle.d_handle, THREAD_PRIORITY_ABOVE_NORMAL );
			break;
		case THREAD_HIGHEST:
			WindowsNS::SetThreadPriority( handle.d_handle, THREAD_PRIORITY_HIGHEST );
			break;
	}
}

void SysThread::SetProcessor( threadHandle_t & handle, const unsigned int processor ) {
#if defined( _XENON )
	::XSetThreadProcessor( handle.d_handle, processor );
#else
	WindowsNS::SetThreadAffinityMask( handle.d_handle, 1 << processor );
#endif	
}

void SysThread::SetName( threadHandle_t & handle, const char * name ) {
	struct THREADNAME_INFO {
		WindowsNS::DWORD	dwType;		// must be 0x1000
		WindowsNS::LPCSTR	szName;		// pointer to name (in user addr space)
		WindowsNS::DWORD	dwThreadID;	// thread ID (-1=caller thread)
		WindowsNS::DWORD	dwFlags;	// reserved for future use, must be zero
	};
	THREADNAME_INFO		info;
	info.dwType			= 0x1000;
	info.szName			= name;
	info.dwThreadID		= handle.d_id;
	info.dwFlags		= 0;
	if( info.dwThreadID ) {
#ifdef __GNUC__
		WindowsNS::RaiseException( 0x406D1388, 0, sizeof( info ) / sizeof( WindowsNS::DWORD ), ( WindowsNS::ULONG_PTR * )&info );
#else
		__try {
			WindowsNS::RaiseException( 0x406D1388, 0, sizeof( info ) / sizeof( WindowsNS::DWORD ), ( WindowsNS::ULONG_PTR * )&info );
		}
		__except( EXCEPTION_CONTINUE_EXECUTION ) {
		}
#endif
	}
}
