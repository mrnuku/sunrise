#include "precompiled.h"
#pragma hdrstop

using namespace Lua;

LuaState *			LuaScript::g_state = NULL;

void * LuaStateExtraData::Allocator( void * ud, void * ptr, size_t osize, size_t nsize ) {

	LuaStateAllocator * memAllocator = ( LuaStateAllocator * )ud;
	if( !nsize ) {
		memAllocator->Free( ( char * )ptr );
		ptr = NULL;
	} else {
		if( ptr )
			ptr = memAllocator->Resize( ( char * )ptr, ( int )nsize );
		else
			ptr = memAllocator->Alloc( ( int )nsize );
	}
	return ptr;
}

LuaState * LuaState::New( ) {
	
	LuaStateAllocator * memAllocator = new LuaStateAllocator;
	LuaState * luaScriptPtr = ( LuaState * )lua_newstate( LuaStateExtraData::Allocator, memAllocator );
	int extraSize = LUAI_EXTRASPACE;
	Common::Com_Memset( luaScriptPtr->ED( ), 0, extraSize );
	luaScriptPtr->ED( )->d_memAllocator = memAllocator;
	luaL_openlibs( luaScriptPtr );
	return luaScriptPtr;
}

void LuaState::Close( ) {
	
	LuaStateAllocator * memAllocator = ED( )->d_memAllocator;
	Lua::lua_close( this );
	delete memAllocator;
}

void LuaState::PushTable( const Str & t_name ) {

	Lua::lua_pushstring( this, t_name );
	Lua::lua_newtable( this );
}

void LuaState::PushVarPtr( const Str & var_name, void * ptr ) {

	Lua::lua_pushstring( this, var_name );
	Lua::lua_pushlightuserdata( this, ptr );
}

void LuaState::AddMethod( const Str & m_name, LuaFunction m_func, int parent_idx ) {

	Lua::lua_pushstring( this, m_name );
	Lua::lua_pushcfunction( this, ( Lua::lua_CFunction )m_func );
	Lua::lua_rawset( this, parent_idx );
}

void LuaState::AddMethod2( const Str & m_name, LuaFunction m_func, int parent_idx ) {

	Lua::lua_pushstring( this, m_name );
	Lua::lua_pushcfunction( this, ( Lua::lua_CFunction )m_func );
	Lua::lua_settable( this, parent_idx );
}

int LuaState::Writer( lua_State * L, const void * p, size_t sz, void * ud ) {

	CAListBase< byte > * buffer = ( CAListBase< byte > * )ud;
	buffer->Append( ( byte * )p, ( int )sz );
	return 0;
}

const char * LuaState::Reader( lua_State * L, void * data, size_t * size ) {

	const CAListBase< byte > * buffer = ( CAListBase< byte > * )data;
	*size = ( size_t )buffer->Num( );
	return ( const char * )buffer->Ptr( );
}

int LuaScript::print( LuaState & state ) {

	Str outputStr;
	char ptrBuffer[ 32 ];
	for( state.BeginItr( ); state; state++ ) {
		if( state.IsNumber( ) ) {
			float fVal = state.GetFloat( );
			int iVal = ( int )fVal;
			if( fabs( fVal ) > fabs( ( float )iVal ) )
				outputStr += fVal;
			else
				outputStr += iVal;
		}
		else if( state.IsString( ) )
			outputStr += state.GetString( );
		else if( state.IsNil( ) )
			outputStr += "nil";
		else if( state.IsBool( ) )
			outputStr += state.GetBool( );
		else {
			outputStr += state.GetTypename( );
			sprintf( ptrBuffer, ":%p", state.GetPtr( ) );
			outputStr += ptrBuffer;
		}
	}
	Common::Com_Printf( outputStr );
	return 0;
}

void LuaScript::debugHook( LuaState & state, Lua::lua_Debug & debug_state ) {

	int iVal;

	switch( debug_state.event ) {

		case LUA_HOOKCALL:

			iVal = Lua::lua_getinfo( &state, "nlSu", &debug_state );
			if( debug_state.currentline == -1 ) break;
			Common::Com_Printf( "1>%s(%i): %s entry\t", debug_state.source, debug_state.currentline, debug_state.name ? debug_state.name : debug_state.what );
			state.PrintStack( );
			break;

		case LUA_HOOKRET:

			iVal = Lua::lua_getinfo( &state, "nlSu", &debug_state );
			if( debug_state.currentline == -1 ) break;
			Common::Com_Printf( "1>%s(%i): %s ret  \t", debug_state.source, debug_state.currentline, debug_state.name ? debug_state.name : debug_state.what );
			state.PrintStack( );
			break;

		case LUA_HOOKLINE:

			iVal = Lua::lua_getinfo( &state, "nlSu", &debug_state );
			Common::Com_Printf( "1>%s(%i): %s step \t", debug_state.source, debug_state.currentline, debug_state.name ? debug_state.name : debug_state.what );
			state.PrintStack( );
			break;

		case 3:

			iVal = debug_state.event;
			return;
	}

	return;
}

void LuaScript::Init( ) {

	g_state = LuaState::New( );
	Register( "print", print );
	ExecuteInline( "print(_VERSION,' initialized.\\\n')" );
}

void LuaScript::Shutdown( ) {

	g_state->Close( );
	g_state = NULL;
}

bool LuaScript::ExecuteInternal( LuaFunction funcp, int args, int rets ) {

	//g_state->SaveTop( );
	assert( g_state->GetTop( ) == args );
	lua_pushcclosure( g_state, ( Lua::lua_CFunction )funcp, 0 );
	if( args )
		Lua::lua_insert( g_state, 1 );
	lua_call( g_state, args, rets );
	//g_state->RestoreTop( );
	return true;
}

bool LuaScript::ExecuteFile( const Str & script_file, int args, int rets ) {

	Str luaLocation = "..\\..\\fs\\";
	luaLocation += script_file;
	File_Memory fMem = FileSystem::ReadFile( script_file );
	if( !fMem.IsValid( ) )
		return false;
	g_state->SaveTop( );
	bool succeded = Lua::luaL_loadbuffer( g_state, ( char * )fMem.GetDataPtr( ), fMem.Length( ), luaLocation ) ? false : true;
	if( !succeded )
		Common::Com_Printf( "LuaScript::Load: %\n", g_state->GetErrorString( ) );
	if( succeded )
		g_state->Execute( args, rets );
	g_state->RestoreTop( );
	FileSystem::FreeFile( fMem );
	return succeded;
}

bool LuaScript::ExecuteFileProt( const Str & script_file, int args, int rets, int errh ) {

	Str luaLocation = "..\\..\\fs\\";
	luaLocation += script_file;
	File_Memory fMem = FileSystem::ReadFile( script_file );
	if( !fMem.IsValid( ) )
		return false;
	g_state->SaveTop( );
	bool succeded = Lua::luaL_loadbuffer( g_state, ( char * )fMem.GetDataPtr( ), fMem.Length( ), luaLocation ) ? false : true;
	if( !succeded )
		Common::Com_Printf( "LuaScript::Load: %\n", g_state->GetErrorString( ) );
	if( errh )
		Lua::lua_sethook( g_state, ( Lua::lua_Hook )debugHook, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0 );
	if( succeded && !( succeded = g_state->ExecuteProt( args, rets, 0 ) ) )
		Common::Com_Printf( "LuaScript::Execute: %s\n", g_state->GetErrorString( ) );
	if( errh )
		Lua::lua_sethook( g_state, ( Lua::lua_Hook )NULL, 0, 0 );
	g_state->RestoreTop( );
	FileSystem::FreeFile( fMem );
	return succeded;
}

bool LuaScript::ExecuteInline( const Str & script_data, int args, int rets ) {

	g_state->SaveTop( );
	bool succeded = g_state->LoadString( script_data );
	if( !succeded )
		Common::Com_Printf( "LuaScript::Load: %\n", g_state->GetErrorString( ) );
	if( succeded )
		g_state->Execute( args, rets );
	g_state->RestoreTop( );
	return succeded;
}

bool LuaScript::ExecuteInlineProt( const Str & script_data, int args, int rets, int errh ) {

	g_state->SaveTop( );
	bool succeded = g_state->LoadString( script_data );
	if( !succeded )
		Common::Com_Printf( "LuaScript::Load: %\n", g_state->GetErrorString( ) );
	if( errh )
		Lua::lua_sethook( g_state, ( Lua::lua_Hook )debugHook, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0 );
	if( succeded && !( succeded = g_state->ExecuteProt( args, rets, 0 ) ) )
		Common::Com_Printf( "LuaScript::Execute: %s\n", g_state->GetErrorString( ) );
	if( errh )
		Lua::lua_sethook( g_state, ( Lua::lua_Hook )NULL, 0, 0 );
	assert( succeded );
	g_state->RestoreTop( );
	return succeded;
}

void LuaScript::Register( const Str & name, LuaFunction funcp ) {
	
	lua_register( g_state, name.c_str( ), ( lua_CFunction )funcp );
}

void LuaScript::RegisterModule( const Str & mod_name, LuaFunction mod_open_func ) {

	luaL_requiref( g_state, mod_name.c_str( ), ( lua_CFunction )mod_open_func, 1 );
	g_state->Pop( 1 );
}

void LuaState::PrintStack( bool one_line, const Str & msg ) {

	Str outputStr;
	if( msg.Length( ) ) {
		outputStr += msg;
		outputStr += ' ';
	}
	//outputStr += "FRAMEINFO:";
	if( !GetTop( ) ) outputStr += "empty\n";
	else {

		outputStr += GetTop( );
		if( one_line ) outputStr += " {";
		else outputStr += " {\n";
		Str ptrStr;
		for( int i = 1; i <= GetTop( ); i++ ) {

			if( one_line ) outputStr += " [ ";
			else outputStr += '\t';
			outputStr += GetTypename( i );
			if( one_line ) outputStr += ' ';
			else outputStr += '\t';
			if( IsNumber( i ) ) {

				float fVal = GetFloat( i );
				int iVal = ( int )fVal;
				if( fabs( fVal ) > fabs( ( float )iVal ) ) outputStr += fVal;
				else outputStr += iVal;

			}
			else if( IsString( i ) ) {
				
				ptrStr = GetString( i );
				ptrStr.Strip( '\n' );
				outputStr += ptrStr;

			} else if( IsBool( i ) ) outputStr += GetBool( i );
			else {

				void * sPtr = GetPtr( i );
				if( sPtr ) { sprintf( ptrStr, "0x%p", GetPtr( i ) ); /*if( one_line ) ptrStr.StripLeading( '0' );*/ outputStr += ptrStr; }
				else if( !IsNil( i ) ) outputStr += "0";
			}
			if( IsNil( i ) ) outputStr += "]";
			else if( one_line ) outputStr += " ]";
			else outputStr += '\n';
		}

		if( one_line ) outputStr += " }\n";
		else outputStr += "}\n";
	}

	Common::Com_Printf( outputStr );
}
