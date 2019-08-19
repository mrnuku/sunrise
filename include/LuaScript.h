#ifndef LUASCRIPT_H
#define LUASCRIPT_H

class LuaState;
typedef int ( *LuaFunction )( LuaState & state );
typedef void ( *LuaHook )( LuaState & state, Lua::lua_Debug & debug_state );

#define GETTOP ((int)(top-(ci->func+1)))

class LuaState : public Lua::lua_State {
private:
	static int			Writer( lua_State * L, const void * p, size_t sz, void * ud );
	static const char *	Reader( lua_State * L, void * data, size_t * size );

public:

	static LuaState *	New( );
	void				Close( );

	void				PushClone( int index )							{ Lua::lua_pushvalue( this, index ); }
	void				PushMetatable( const Str & mt_name )			{ Lua::luaL_newmetatable( this, mt_name ); }
	void				PushTable( const Str & t_name );
	void				PushVarPtr( const Str & var_name, void * ptr );
	void				PushString( const Str & s_name )				{ Lua::lua_pushlstring( this, s_name, s_name.Length( ) ); }

	void				Push( )											{ Lua::lua_pushnil( this ); }
	void				Push( const bool & b_val )						{ Lua::lua_pushboolean( this, ( int )b_val ); }
	void				Push( void * lu_val )							{ Lua::lua_pushlightuserdata( this, lu_val ); }
	void				Push( const Lua::lua_Number & n_val )			{ Lua::lua_pushnumber( this, n_val ); }
	void				Push( const Str & s_name )						{ Lua::lua_pushlstring( this, s_name, s_name.Length( ) ); }

	void				Pop( int count )								{ Lua::lua_pop( this, count ); }

	void				AddMethod( const Str & m_name, LuaFunction m_func, int parent_idx = -3 );
	void				AddMethod2( const Str & m_name, LuaFunction m_func, int parent_idx = -3 );

	LuaStateExtraData *	ED( )											{ return ( LuaStateExtraData * )( ( ( byte * )( this ) ) - LUAI_EXTRASPACE ); }

	int					GetTop( )										{ return GETTOP; }
	void				SetTop( int num )								{ Lua::lua_settop( this, num ); }
	void				SaveTop( )										{ ED( )->d_savedTopValue = GETTOP; }
	void				RestoreTop( )									{ assert( ED( )->d_savedTopValue == GETTOP ); }//Lua::lua_settop( this, ED( )->d_savedTopValue ); }
	bool				LoadString( const Str & inp )					{ return Lua::luaL_loadbuffer( this, inp.c_str( ), inp.Length( ), "inline-script" ) ? false : true; }
	const char *		GetErrorString( )								{ return Lua::lua_tostring( this, -1 ); }
	void				GetErrorString( Str & out )						{ out = Lua::lua_tostring( this, -1 ); }
	void				Execute( int args, int rets )					{ Lua::lua_call( this, args, rets ); }
	bool				ExecuteProt(int args, int rets, int errh )		{ return Lua::lua_pcall( this, args, rets, errh ) ? false : true; }
	void				PrintStack( bool one_line = true, const Str & msg = null_string );

	bool				DumpFunction( CAListBase< byte > & buffer )		{ return !Lua::lua_dump( this, Writer, &buffer ); }
	bool				LoadChunk( CAListBase< byte > & buffer, const Str & chunk = "inline-script" )	{ return Lua::lua_load( this, Reader, &buffer, chunk, NULL ) == LUA_OK; }

	int					GetType( int index )							{ return Lua::lua_type( this, index ); }
	const char *		GetTypename( int index )						{ return Lua::lua_typename( this, Lua::lua_type( this, index ) ); }
	void				GetTypename( int index, Str & out )				{ out = Lua::lua_typename( this, Lua::lua_type( this, index ) ); }

	bool				IsNil( int index )								{ return Lua::lua_type( this, index ) == LUA_TNIL; }
	bool				IsTable( int index )							{ return Lua::lua_type( this, index ) == LUA_TTABLE; }
	bool				IsBool( int index )								{ return Lua::lua_type( this, index ) == LUA_TBOOLEAN; }
	bool				IsNumber( int index )							{ return Lua::lua_isnumber( this, index ) ? true : false; }
	bool				IsString( int index )							{ int pType = Lua::lua_type( this, index ); return pType == LUA_TSTRING || pType == LUA_TNUMBER; }
	bool				IsUserdata( int index )							{ return Lua::lua_isuserdata( this, index ) ? true : false; }
	bool				IsFunction( int index )							{ return Lua::lua_iscfunction( this, index ) ? true : false; }

	bool				GetBool( int index )							{ return Lua::lua_toboolean( this, index ) ? true : false; }
	const char *		GetString( int index )							{ return Lua::lua_tostring( this, index ); }
	Lua::lua_Number		GetNumber( int index )							{ return Lua::lua_tonumber( this, index ); }
	int					GetInt( int index )								{ return ( int )Lua::lua_tonumber( this, index ); }
	float				GetFloat( int index )							{ return ( float )Lua::lua_tonumber( this, index ); }
	double				GetDouble( int index )							{ return ( double )Lua::lua_tonumber( this, index ); }
	void *				GetPtr( int index )								{ return Lua::lua_touserdata( this, index ); }
	LuaFunction			GetFunction( int index )						{ return ( LuaFunction )Lua::lua_tocfunction( this, index ); }

	void				GetBool( int index, bool & out )				{ out = Lua::lua_toboolean( this, index ) ? true : false; }
	void				GetString( int index, Str & out )				{ out = Lua::lua_tostring( this, index ); }
	void				GetNumber( int index, Lua::lua_Number & out )	{ out = Lua::lua_tonumber( this, index ); }
	void				GetInt( int index, int & out )					{ out = ( int )Lua::lua_tonumber( this, index ); }
	void				GetFloat( int index, float & out )				{ out = ( float )Lua::lua_tonumber( this, index ); }
	void				GetDouble( int index, double & out )			{ out = ( double )Lua::lua_tonumber( this, index ); }
	void				GetPtr( int index, void *& out )				{ out = Lua::lua_touserdata( this, index ); }
	void				GetFunction( int index, LuaFunction & out )		{ out = ( LuaFunction )Lua::lua_tocfunction( this, index ); }

	void				BeginItr( )										{ ED( )->d_paramCount = GETTOP; ED( )->d_paramIndex = 1; }
	operator			bool( )											{ return ED( )->d_paramIndex <= ED( )->d_paramCount; }
	LuaState &			operator ++(int)								{ ED( )->d_paramIndex++; return *this; }

	int					GetType( )										{ return Lua::lua_type( this, ED( )->d_paramIndex ); }
	const char *		GetTypename( )									{ return Lua::lua_typename( this, Lua::lua_type( this, ED( )->d_paramIndex ) ); }
	void				GetTypename( Str & out )						{ out = Lua::lua_typename( this, Lua::lua_type( this, ED( )->d_paramIndex ) ); }

	bool				IsNil( )										{ return Lua::lua_type( this, ED( )->d_paramIndex ) == LUA_TNIL; }
	bool				IsTable( )										{ return Lua::lua_type( this, ED( )->d_paramIndex ) == LUA_TTABLE; }
	bool				IsBool( )										{ return Lua::lua_type( this, ED( )->d_paramIndex ) == LUA_TBOOLEAN; }
	bool				IsNumber( )										{ return Lua::lua_isnumber( this, ED( )->d_paramIndex ) ? true : false; }
	bool				IsString( )										{ int pType = Lua::lua_type( this, ED( )->d_paramIndex ); return pType == LUA_TSTRING || pType == LUA_TNUMBER; }
	bool				IsUserdata( )									{ return Lua::lua_isuserdata( this, ED( )->d_paramIndex ) ? true : false; }
	bool				IsFunction( )									{ return Lua::lua_iscfunction( this, ED( )->d_paramIndex ) ? true : false; }

	bool				GetBool( )										{ return Lua::lua_toboolean( this, ED( )->d_paramIndex ) ? true : false; }
	const char *		GetString( )									{ return Lua::lua_tostring( this, ED( )->d_paramIndex ); }
	Lua::lua_Number		GetNumber( )									{ return Lua::lua_tonumber( this, ED( )->d_paramIndex ); }
	int					GetInt( )										{ return ( int )Lua::lua_tonumber( this, ED( )->d_paramIndex ); }
	float				GetFloat( )										{ return ( float )Lua::lua_tonumber( this, ED( )->d_paramIndex ); }
	double				GetDouble( )									{ return ( double )Lua::lua_tonumber( this, ED( )->d_paramIndex ); }
	void *				GetPtr( )										{ return Lua::lua_touserdata( this, ED( )->d_paramIndex ); }
	LuaFunction			GetFunction( )									{ return ( LuaFunction )Lua::lua_tocfunction( this, ED( )->d_paramIndex ); }

	void				GetBool( bool & out )							{ out = Lua::lua_toboolean( this, ED( )->d_paramIndex ) ? true : false; }
	void				GetString( Str & out )							{ out = Lua::lua_tostring( this, ED( )->d_paramIndex ); }
	void				GetNumber( Lua::lua_Number & out )				{ out = Lua::lua_tonumber( this, ED( )->d_paramIndex ); }
	void				GetInt( int & out )								{ out = ( int )Lua::lua_tonumber( this, ED( )->d_paramIndex ); }
	void				GetFloat( float & out )							{ out = ( float )Lua::lua_tonumber( this, ED( )->d_paramIndex ); }
	void				GetDouble( double & out )						{ out = ( double )Lua::lua_tonumber( this, ED( )->d_paramIndex ); }
	void				GetPtr( void *& out )							{ out = Lua::lua_touserdata( this, ED( )->d_paramIndex ); }
	void				GetFunction( LuaFunction & out )				{ out = ( LuaFunction )Lua::lua_tocfunction( this, ED( )->d_paramIndex ); }
};

#undef GETTOP

class LuaScript {

private:

	static LuaState *								g_state;

	static int										print( LuaState & state );
	static void										debugHook( LuaState & state, Lua::lua_Debug & debug_state );

public:

	static void										Init( );
	static void										Shutdown( );

	static bool										ExecuteInternal( LuaFunction funcp, int args = 0, int rets = 0 );
	static bool										ExecuteFile( const Str & script_file, int args = 0, int rets = 0 );
	static bool										ExecuteFileProt( const Str & script_file, int args = 0, int rets = 0, int errh = 0 );
	static bool										ExecuteInline( const Str & script_data, int args = 0, int rets = 0 );
	static bool										ExecuteInlineProt( const Str & script_data, int args = 0, int rets = 0, int errh = 0 );
	static void										Register( const Str & name, LuaFunction funcp );
	static void										RegisterModule( const Str & mod_name, LuaFunction mod_open_func );
};

#define LCF_ENTRY_CHECK()	{state.PrintStack(true,"1>"__FILE__"("_CRT_STRINGIZE(__LINE__)"): "__FUNCTION__" entry\t");}
#define LCF_LINE_CHECK()	{state.PrintStack(true,"1>"__FILE__"("_CRT_STRINGIZE(__LINE__)"): "__FUNCTION__" step \t");}
#define LCF_RETURN_CHECK(x)	{state.PrintStack(true,"1>"__FILE__"("_CRT_STRINGIZE(__LINE__)"): "__FUNCTION__" ret_"#x"\t");return(x);}

#endif
