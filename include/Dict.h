// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __DICT_H__
#define __DICT_H__

/*
===============================================================================

Key/value dictionary

This is a dictionary class that tracks an arbitrary number of key / value
pair combinations. It is used for map entity spawning, GUI state management,
and other things.

Keys are compared case-insensitive.

Does not allocate memory until the first key/value pair is added.

===============================================================================
*/

class KeyValue {
	friend class Dict;

public:
	const Str &		GetKey( void ) const { return *key; }
	const Str &		GetValue( void ) const { return *value; }

	size_t				Allocated( void ) const { return key->Allocated() + value->Allocated(); }
	size_t				Size( void ) const { return sizeof( *this ) + key->Size() + value->Size(); }

	bool				operator==( const KeyValue &kv ) const { return ( key == kv.key && value == kv.value ); }

private:
	const PoolStr *	key;
	const PoolStr *	value;
};

class Dict {
public:
						Dict( void );
						Dict( const Dict &other );	// allow declaration with assignment
						~Dict( void );

						// set the granularity for the index
	void				SetGranularity( int granularity );
						// set hash size
	void				SetHashSize( int hashSize );
						// clear existing key/value pairs and copy all key/value pairs from other
	Dict &				operator=( const Dict &other );
						// copy from other while leaving existing key/value pairs in place
	void				Copy( const Dict &other );
						// clear existing key/value pairs and transfer key/value pairs from other
	void				TransferKeyValues( Dict &other );
						// parse dict from parser
	bool				Parse( Parser &parser );
						// copy key/value pairs from other dict not present in this dict
	void				SetDefaults( const Dict *dict );
						// clear dict freeing up memory
	void				Clear( void );
						// print the dict
	void				Print() const;

	size_t				Allocated( void ) const;
	size_t				Size( void ) const { return sizeof( *this ) + Allocated(); }

	void				Set( const char *key, const char *value );
	void				SetFloat( const char *key, float val );
	void				SetInt( const char *key, int val );
	void				SetBool( const char *key, bool val );
	void				SetVector( const char *key, const Vec3 &val );
	void				SetVec2( const char *key, const Vec2 &val );
	void				SetVec4( const char *key, const Vec4 &val );
	void				SetAngles( const char *key, const Angles &val );
	void				SetMatrix( const char *key, const Mat3 &val );

						// these return default values of 0.0, 0 and false
	const char *		GetString( const char *key, const char *defaultString = "" ) const;
	float				GetFloat( const char *key, const char *defaultString = "0" ) const;
	int					GetInt( const char *key, const char *defaultString = "0" ) const;
	bool				GetBool( const char *key, const char *defaultString = "0" ) const;
	Vec3				GetVector( const char *key, const char *defaultString = NULL ) const;
	Vec2				GetVec2( const char *key, const char *defaultString = NULL ) const;
	Vec4				GetVec4( const char *key, const char *defaultString = NULL ) const;
	Angles				GetAngles( const char *key, const char *defaultString = NULL ) const;
	Mat3				GetMatrix( const char *key, const char *defaultString = NULL ) const;

	bool				GetString( const char *key, const char *defaultString, const char **out ) const;
	bool				GetString( const char *key, const char *defaultString, Str &out ) const;
	bool				GetFloat( const char *key, const char *defaultString, float &out ) const;
	bool				GetInt( const char *key, const char *defaultString, int &out ) const;
	bool				GetBool( const char *key, const char *defaultString, bool &out ) const;
	bool				GetVector( const char *key, const char *defaultString, Vec3 &out ) const;
	bool				GetVec2( const char *key, const char *defaultString, Vec2 &out ) const;
	bool				GetVec4( const char *key, const char *defaultString, Vec4 &out ) const;
	bool				GetAngles( const char *key, const char *defaultString, Angles &out ) const;
	bool				GetMatrix( const char *key, const char *defaultString, Mat3 &out ) const;

	int					GetNumKeyVals( void ) const;
	const KeyValue *	GetKeyVal( int index ) const;
						// returns the key/value pair with the given key
						// returns NULL if the key/value pair does not exist
	const KeyValue *	FindKey( const char *key ) const;
						// returns the index to the key/value pair with the given key
						// returns -1 if the key/value pair does not exist
	int					FindKeyIndex( const char *key ) const;
						// delete the key/value pair with the given key
	void				Delete( const char *key );
						// finds the next key/value pair with the given key prefix.
						// lastMatch can be used to do additional searches past the first match.
	const KeyValue *	MatchPrefix( const char *prefix, const KeyValue *lastMatch = NULL ) const;
						// randomly chooses one of the key/value pairs with the given key prefix and returns it's value
	//const char *		RandomPrefix( const char *prefix, idRandom &random ) const;

	void				WriteToFileHandle( FileBase * f ) const;
	void				ReadFromFileHandle( FileBase * f );

						// returns a unique checksum for this dictionary's content
	int					Checksum( void ) const;

	static void			Init( void );
	static void			Shutdown( void );

	static void			ShowMemoryUsage_f( const CmdArgs &args );
	static void			ListKeys_f( const CmdArgs &args );
	static void			ListValues_f( const CmdArgs &args );

private:
	List<KeyValue>		args;
	HashIndex			argHash;

	static StrPool		globalKeys;
	static StrPool		globalValues;
};


INLINE Dict::Dict( void ) {
	args.SetGranularity( 16 );
	argHash.SetGranularity( 16 );
	argHash.Clear( 128, 16 );
}

INLINE Dict::Dict( const Dict &other ) {
	*this = other;
}

INLINE Dict::~Dict( void ) {
	Clear();
}

INLINE void Dict::SetGranularity( int granularity ) {
	args.SetGranularity( granularity );
	argHash.SetGranularity( granularity );
}

INLINE void Dict::SetHashSize( int hashSize ) {
	if ( args.Num() == 0 ) {
		argHash.Clear( hashSize, 16 );
	}
}

INLINE void Dict::SetFloat( const char *key, float val ) {
	Set( key, va( "%f", val ) );
}

INLINE void Dict::SetInt( const char *key, int val ) {
	Set( key, va( "%i", val ) );
}

INLINE void Dict::SetBool( const char *key, bool val ) {
	Set( key, va( "%i", val ) );
}

INLINE void Dict::SetVector( const char *key, const Vec3 &val ) {
	Set( key, val.ToString() );
}

INLINE void Dict::SetVec4( const char *key, const Vec4 &val ) {
	Set( key, val.ToString() );
}

INLINE void Dict::SetVec2( const char *key, const Vec2 &val ) {
	Set( key, val.ToString() );
}

INLINE void Dict::SetAngles( const char *key, const Angles &val ) {
	Set( key, val.ToString() );
}

INLINE void Dict::SetMatrix( const char *key, const Mat3 &val ) {
	Set( key, val.ToString() );
}

INLINE bool Dict::GetString( const char *key, const char *defaultString, const char **out ) const {
	const KeyValue *kv = FindKey( key );
	if ( kv ) {
		*out = kv->GetValue();
		return true;
	}
	*out = defaultString;
	return false;
}

INLINE bool Dict::GetString( const char *key, const char *defaultString, Str &out ) const {
	const KeyValue *kv = FindKey( key );
	if ( kv ) {
		out = kv->GetValue();
		return true;
	}
	out = defaultString;
	return false;
}

INLINE const char *Dict::GetString( const char *key, const char *defaultString ) const {
	const KeyValue *kv = FindKey( key );
	if ( kv ) {
		return kv->GetValue();
	}
	return defaultString;
}

INLINE float Dict::GetFloat( const char *key, const char *defaultString ) const {
	return ( float )atof( GetString( key, defaultString ) );
}

INLINE int Dict::GetInt( const char *key, const char *defaultString ) const {
	return atoi( GetString( key, defaultString ) );
}

INLINE bool Dict::GetBool( const char *key, const char *defaultString ) const {
	return ( atoi( GetString( key, defaultString ) ) != 0 );
}

INLINE Vec3 Dict::GetVector( const char *key, const char *defaultString ) const {
	Vec3 out;
	GetVector( key, defaultString, out );
	return out;
}

INLINE Vec2 Dict::GetVec2( const char *key, const char *defaultString ) const {
	Vec2 out;
	GetVec2( key, defaultString, out );
	return out;
}

INLINE Vec4 Dict::GetVec4( const char *key, const char *defaultString ) const {
	Vec4 out;
	GetVec4( key, defaultString, out );
	return out;
}

INLINE Angles Dict::GetAngles( const char *key, const char *defaultString ) const {
	Angles out;
	GetAngles( key, defaultString, out );
	return out;
}

INLINE Mat3 Dict::GetMatrix( const char *key, const char *defaultString ) const {
	Mat3 out;
	GetMatrix( key, defaultString, out );
	return out;
}

INLINE int Dict::GetNumKeyVals( void ) const {
	return args.Num();
}

INLINE const KeyValue *Dict::GetKeyVal( int index ) const {
	if ( index >= 0 && index < args.Num() ) {
		return &args[ index ];
	}
	return NULL;
}

#endif /* !__DICT_H__ */
