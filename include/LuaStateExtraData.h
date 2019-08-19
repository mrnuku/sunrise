#ifndef LUASTATEEXTRADATA_H
#define LUASTATEEXTRADATA_H

typedef DynamicBlockAlloc< char, 1 << 18, 128 > LuaStateAllocator;

class LuaStateExtraData {
public:
	LuaStateAllocator *	d_memAllocator;
	int					d_paramCount;
	int					d_paramIndex;
	int					d_savedTopValue;

	static void *		Allocator( void * ud, void * ptr, size_t osize, size_t nsize );
};

#define LUAI_EXTRASPACE sizeof( LuaStateExtraData )

#endif
