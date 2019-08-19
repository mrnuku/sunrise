#include "precompiled.h"
#pragma hdrstop

/*
=============
Signal::Signal
=============
*/
Signal::Signal( ) {

	SysSignal::Create( handle );
}

/*
=============
Signal:~:Signal
=============
*/
Signal::~Signal( ) {

	SysSignal::Destroy( handle );
}

/*
=============
Signal::Set
=============
*/
void Signal::Set( ) {

	SysSignal::Set( handle );
}

/*
=============
Signal::Clear
=============
*/
void Signal::Clear( ) {

	SysSignal::Clear( handle );
}

/*
=============
Signal::Wait
=============
*/
bool Signal::Wait( int timeout ) {

	return SysSignal::Wait( handle, timeout );
}

/*
=============
Signal::Wait
=============
*/
bool Signal::SignalAndWait( Signal &signal, int timeout ) {

	return SysSignal::SignalAndWait( signal.handle, handle, timeout );
}
