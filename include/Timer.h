#ifndef TIMER_H
#define TIMER_H

class Timer {
private:
	timeTypeSystem		d_clockTicks;
	timeTypeSystem		d_start;

	enum {
		TS_STARTED,
		TS_STOPPED
	}					d_state;

public:
						Timer( );
						Timer( timeTypeSystem clockTicks );
						~Timer( );

	Timer				operator+( const Timer & t ) const;
	Timer				operator-( const Timer & t ) const;
	Timer &				operator+=( const Timer & t );
	Timer &				operator-=( const Timer & t );

	void				Start( );
	void				Stop( );
	void				Clear( );
	timeTypeSystem		ClockTicks( ) const;
	timeType			Milliseconds( ) const;
	Str					ToString( ) const { return Str( ( int )Milliseconds( ) ); }
};

INLINE Timer::Timer( ) {
	d_state = TS_STOPPED;
	d_clockTicks = 0;
}

INLINE Timer::Timer( timeType _clockTicks ) {
	d_state = TS_STOPPED;
	d_clockTicks = _clockTicks;
}

INLINE Timer::~Timer( ) {
}

INLINE Timer Timer::operator+( const Timer & t ) const {
	assert( d_state == TS_STOPPED && t.d_state == TS_STOPPED );
	return Timer( d_clockTicks + t.d_clockTicks );
}

INLINE Timer Timer::operator-( const Timer & t ) const {
	assert( d_state == TS_STOPPED && t.d_state == TS_STOPPED );
	return Timer( d_clockTicks - t.d_clockTicks );
}

INLINE Timer & Timer::operator+=( const Timer & t ) {
	assert( d_state == TS_STOPPED && t.d_state == TS_STOPPED );
	d_clockTicks += t.d_clockTicks;
	return *this;
}

INLINE Timer & Timer::operator-=( const Timer & t ) {
	assert( d_state == TS_STOPPED && t.d_state == TS_STOPPED );
	d_clockTicks -= t.d_clockTicks;
	return *this;
}

INLINE void Timer::Start( ) {
	assert( d_state == TS_STOPPED );
	d_state = TS_STARTED;
	System::Sys_ReadRawTime( d_start );
}

INLINE void Timer::Stop( ) {
	assert( d_state == TS_STARTED );
	timeTypeSystem end;
	System::Sys_ReadRawTime( end );
	d_clockTicks += end - d_start;
	d_state = TS_STOPPED;
}

INLINE void Timer::Clear( ) {
	d_clockTicks = 0;
}

INLINE timeTypeSystem Timer::ClockTicks( ) const {
	assert( d_state == TS_STOPPED );
	return d_clockTicks;
}

INLINE timeType Timer::Milliseconds( ) const {
	assert( d_state == TS_STOPPED );
	return System::Sys_RawTimeToMs( d_clockTicks );
}

class TimerReport {
private:
	List< Timer* >		d_timers;
	StrList				d_names;
	Str					d_reportName;

public:
						TimerReport( );
						~TimerReport( );

	void				SetReportName( const char * name );
	int					AddReport( const char * name );
	void				Clear( );
	void				Reset( );
	void				PrintReport( );
	void				AddTime( const char * name, Timer * time );
};

#endif
