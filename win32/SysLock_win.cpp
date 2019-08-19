#include "precompiled.h"
#pragma hdrstop

/*
=============
SysLock::Create
=============
*/
void SysLock::Init( lockHandle_t& handle ) {

	WindowsNS::InitializeCriticalSection( &handle );
}

/*
=============
SysLock::Destroy
=============
*/
void SysLock::Destroy( lockHandle_t& handle ) {

	WindowsNS::DeleteCriticalSection( &handle );
}

/*
=============
SysLock::Acquire
=============
*/
bool SysLock::Acquire( lockHandle_t& handle, bool blocking ) {

	if ( WindowsNS::TryEnterCriticalSection( &handle ) == 0 ) {

		if ( !blocking ) {

			return false;
		}

		WindowsNS::EnterCriticalSection( &handle );
	}

	return true;
}

/*
=============
SysLock::Release
=============
*/
void SysLock::Release( lockHandle_t& handle ) {

	WindowsNS::LeaveCriticalSection( &handle );
}
