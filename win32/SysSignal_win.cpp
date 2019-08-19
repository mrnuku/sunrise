#include "precompiled.h"
#pragma hdrstop

#define DWORD WindowsNS::DWORD

/*
=============
SysSignal::Create
=============
*/
void SysSignal::Create( signalHandle_t &handle ) {

	handle = WindowsNS::CreateEvent( NULL, FALSE, FALSE, NULL );
}

/*
=============
SysSignal::Destroy
=============
*/
void SysSignal::Destroy( signalHandle_t& handle ) {

	WindowsNS::CloseHandle( handle );
}

/*
=============
SysSignal::Set
=============
*/
void SysSignal::Set( signalHandle_t& handle ) {

	WindowsNS::SetEvent( handle );
}

/*
=============
SysSignal::Clear
=============
*/
void SysSignal::Clear( signalHandle_t& handle ) {

	// events are created as auto-reset so this should never be needed
	assert( false );
	WindowsNS::ResetEvent( handle );
}

/*
=============
SysSignal::Wait
=============
*/
bool SysSignal::Wait( signalHandle_t& handle, int timeout ) {

	return ( WindowsNS::WaitForSingleObject( handle, timeout == Signal::WAIT_INFINITE ? INFINITE : timeout ) != WAIT_TIMEOUT );
}

/*
=============
SysSignal::SignalAndWait
=============
*/
bool SysSignal::SignalAndWait( signalHandle_t& signal, signalHandle_t& handle, int timeout ) {

	return ( WindowsNS::SignalObjectAndWait( signal, handle, timeout == Signal::WAIT_INFINITE ? INFINITE : timeout, FALSE ) != WAIT_TIMEOUT );
}
