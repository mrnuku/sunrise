#ifndef __LOCK_H__
#define __LOCK_H__

class Lock {

public:

						Lock();
						~Lock();

	bool				Acquire( bool blocking = true );
	void				Release();

#ifndef _WIN32
	lockHandle_t*		GetHandle() const { return &handle; }
#endif

protected:

	lockHandle_t		handle;
};

template< bool doLock >
class ScopedLock {

public:

};

template< >
class ScopedLock< true > {

public:

	ScopedLock( Lock& lock, bool blocking = true ) : lock( lock ) {
		lock.Acquire( blocking );
	}

	~ScopedLock() {
		lock.Release();
	}

private:

	ScopedLock( const ScopedLock& rhs );
	ScopedLock& operator=( const ScopedLock& rhs );
	Lock& lock;
};


template< >
class ScopedLock< false > {

public:

	ScopedLock( Lock& lock, bool blocking = true ) { }
	~ScopedLock() { }

private:

	ScopedLock( const ScopedLock& rhs );
	ScopedLock& operator=( const ScopedLock& rhs );
};


#endif /* !__LOCK_H__ */
