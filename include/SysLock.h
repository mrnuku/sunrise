#ifndef __SYSLOCK_H__
#define __SYSLOCK_H__

class SysLock {

public:

	static void				Init( lockHandle_t& handle );
	static void				Destroy( lockHandle_t& handle );
	static bool				Acquire( lockHandle_t& handle, bool blocking );
	static void				Release( lockHandle_t& handle );

private:
							SysLock( ) {}
};

#endif /* !__SYSLOCK_H__ */
