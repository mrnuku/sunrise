#ifndef __SIGNAL_H__
#define __SIGNAL_H__

class Signal {

public:

	static const int	WAIT_INFINITE = -1;

						Signal( );
						~Signal( );

	void				Set( );
	void				Clear( );
	bool				Wait( int timeout = WAIT_INFINITE );
	bool				SignalAndWait( Signal &signal, int timeout = WAIT_INFINITE );

protected:

	signalHandle_t		handle;
};

#endif /* !__SIGNAL_H__ */
