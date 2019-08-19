#include "precompiled.h"
#pragma hdrstop

/*
=============
Lock::Lock
=============
*/
Lock::Lock( ) {

	SysLock::Init( handle );
}

/*
=============
Lock:~:Lock
=============
*/
Lock::~Lock( ) {

	SysLock::Destroy( handle );
}

/*
=============
Lock::Acquire
=============
*/
bool Lock::Acquire( bool blocking ) {

	return SysLock::Acquire( handle, blocking );
}

/*
=============
Lock::Release
=============
*/
void Lock::Release( ) {

	SysLock::Release( handle );
}
