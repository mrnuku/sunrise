#include "precompiled.h"
#pragma hdrstop

TimerReport::TimerReport( ) {
}

void TimerReport::SetReportName( const char * name ) {
	d_reportName = ( name ) ? name : "Timer Report";
}

TimerReport::~TimerReport( ) {
	Clear( );
}

int TimerReport::AddReport( const char * name ) {
	if( name && *name ) {
		d_names.Append( name );
		return d_timers.Append( new Timer( ) );
	}
	return -1;
}

void TimerReport::Clear( ) {
	d_timers.DeleteContents( true );
	d_names.Clear( );
	d_reportName.Clear( );
}

void TimerReport::Reset( ) {
	assert( d_timers.Num( ) == d_names.Num( ) );
	for( int i = 0; i < d_timers.Num( ); i++ )
		d_timers[ i ]->Clear( );
}

void TimerReport::AddTime( const char * name, Timer * time ) {
	assert( d_timers.Num( ) == d_names.Num( ) );
	int i;
	for( i = 0; i < d_names.Num( ); i++ ) {
		if( d_names[ i ].Icmp( name ) == 0 ) {
			*d_timers[ i ] += * time;
			break;
		}
	}
	if( i == d_names.Num( ) ) {
		int index = AddReport( name );
		if( index >= 0 ) {
			d_timers[ index ]->Clear( );
			*d_timers[ index ] += *time;
		}
	}
}

void TimerReport::PrintReport( ) {
	assert( d_timers.Num( ) == d_names.Num( ) );
	Common::Com_DPrintf( "%s", d_reportName.c_str( ) );
	timeType total = 0;
	for( int i = 0; i < d_names.Num( ); i++ ) {
		Common::Com_DPrintf( "%s - %ims\n", d_names[ i ].c_str( ), ( int )d_timers[ i ]->Milliseconds( ) );
		total += d_timers[ i ]->Milliseconds( );
	}
	Common::Com_DPrintf( "Total: %ims ( %5.3fs )\n", ( int )total, total * 0.001f );
}
