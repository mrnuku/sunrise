#include "precompiled.h"
#pragma hdrstop

CVar::CVar( const Str & name, const Str & value, int flags, const Str & description, argCompletion_t valueCompletion ) {
	if ( !valueCompletion && ( flags & CVAR_BOOL ) )
		valueCompletion = CmdSystem::ArgCompletion_Boolean;
	Init( name, value, flags, description, 1, -1, valueCompletion );
}

CVar::CVar( const Str & name, const Str & value, int flags, const Str & description, float valueMin, float valueMax, argCompletion_t valueCompletion ) {
	Init( name, value, flags | CVAR_LIMIT, description, valueMin, valueMax, valueCompletion );
}

CVar::CVar( ) {
	d_flags = CVAR_NOREGISTER;
}

CVar::~CVar( ) {
	if( d_flags & CVAR_DYNAMIC )
		CVarSystem::UnRegister( this );
}

CVar & CVar::operator =( const CVar & cvar ) {
	d_name				= cvar.d_name;
	d_latch				= cvar.d_latch;
	d_original			= cvar.d_original;
	d_stringValue		= cvar.d_stringValue;
	d_description		= cvar.d_description;
	d_flags				= cvar.d_flags & ~CVAR_NOREGISTER;
	d_valueMin			= cvar.d_valueMin;
	d_valueMax			= cvar.d_valueMax;
	d_valueCompletion	= cvar.d_valueCompletion;
	d_integerValue		= cvar.d_integerValue;
	d_floatValue		= cvar.d_floatValue;
	CVarSystem::Register( this );
	return *this;
}

void CVar::DRegister( const Str & name, const Str & value, int extra_flags, const Str & description ) {
	d_name				= name;
	d_flags				= CVAR_STRING | CVAR_DYNAMIC | extra_flags;
	d_description		= description;
	d_valueMin			= 0.0f;
	d_valueMax			= 1.0f;
	d_valueCompletion	= NULL;
	d_integerValue		= 0;
	d_floatValue		= 0.0f;
	d_stringValue		= value;
	SecInit( );
	CVarSystem::Register( this );
}

void CVar::DRegister( const Str & name, const bool value, int extra_flags, const Str & description ) {
	d_name				= name;
	d_flags				= CVAR_BOOL | CVAR_DYNAMIC | extra_flags;
	d_description		= description;
	d_valueMin			= 0.0f;
	d_valueMax			= 1.0f;
	d_valueCompletion	= NULL;
	d_integerValue		= ( int )value;
	d_floatValue		= ( float )value;
	d_stringValue		= Str( value );
	SecInit( );
	CVarSystem::Register( this );
}

void CVar::DRegister( const Str & name, const int value, int extra_flags, const Str & description ) {
	d_name				= name;
	d_flags				= CVAR_INT | CVAR_DYNAMIC | extra_flags;
	d_description		= description;
	d_valueMin			= 0.0f;
	d_valueMax			= 1.0f;
	d_valueCompletion	= NULL;
	d_integerValue		= value;
	d_floatValue		= ( float )value;
	d_stringValue		= Str( value );
	SecInit( );
	CVarSystem::Register( this );
}

void CVar::DRegister( const Str & name, const float value, int extra_flags, const Str & description ) {
	d_name				= name;
	d_flags				= CVAR_FLOAT | CVAR_DYNAMIC | extra_flags;
	d_description		= description;
	d_valueMin			= 0.0f;
	d_valueMax			= 1.0f;
	d_valueCompletion	= NULL;
	d_integerValue		= ( int )value;
	d_floatValue		= value;
	d_stringValue		= Str( value );
	SecInit( );
	CVarSystem::Register( this );
}

void CVar::SecInit( ) {
	if( d_flags & ( CVAR_USERINFO | CVAR_SERVERINFO ) )
		if( !IsInfoValid( ) )
			Common::Com_Error( ERR_FATAL, "CVar: \"%s\" have worng chars", d_name.c_str( ) );
	if( d_flags & CVAR_LATCH ) {
		d_latch = new CVar( *this );
		d_latch->d_latch = NULL;
	} else
		d_latch = NULL;
	if( d_flags & CVAR_CHEAT ) {
		d_original = new CVar( *this );
		d_original->d_original = NULL;
	} else
		d_original = NULL;
}

void CVar::Init( const Str & name, const Str & value, int flags, const Str & description, float valueMin, float valueMax, argCompletion_t valueCompletion ) {

	d_name = name;
	if( !( flags &( CVAR_BOOL | CVAR_INT | CVAR_FLOAT ) ) )
		flags |= CVAR_STRING;
	d_flags = flags;
	d_description = description;
	d_valueMin = valueMin;
	d_valueMax = valueMax;
	d_valueCompletion = valueCompletion;
	d_integerValue = 0;
	d_floatValue = 0.0f;
	SetString( value );
	SecInit( );
	CVarSystem::Register( this );
}

bool CVar::IsInfoValid( ) {

	if( GetName( ).Find( "\\" ) != -1 )
		return false;
	if( GetName( ).Find( "\"" ) != -1 )
		return false;
	if( GetName( ).Find( ";" ) != -1 )
		return false;
	if( GetName( ).Find( "\\" ) != -1 )
		return false;
	if( GetName( ).Find( "\"" ) != -1 )
		return false;
	if( GetName( ).Find( ";" ) != -1 )
		return false;
	return true;
}

void CVar::SetString( const Str & value ) {

	if( d_flags & CVAR_LATCH && d_latch ) {
		d_latch->SetString( value );
		return;
	}
	if( d_flags & CVAR_USERINFO )
		Common::userinfo_modified.SetBool( true );
	if( !value ) {
		Common::Com_DPrintf( "CVar::SetString: NULL value, passed to '%s'\n", GetName( ).c_str( ) );
		return;
	}
	Token token;
	if( d_flags & CVAR_STRING )
		d_stringValue = value;
	else if( d_flags & CVAR_BOOL ) {
		token = value;
		bool boolean = token.IcmpFast( true_string );
		if( !boolean ) {
			if( token.IcmpFast( false_string ) )
				boolean = false;
			else {
				Common::Com_DPrintf( "CVar::SetString? type mismatch, expected bool, but '%s' = '%s'\n", GetName( ).c_str( ), value.c_str( ) );
				return;
			}
		}
		SetBool( boolean );
	} else if( d_flags & CVAR_INT ) {
		token = value[ 0 ] == '-' ? value.Right( value.Length( ) - 1 ) : value;
		token.type = TT_NUMBER;
		token.subtype = TT_DECIMAL;
		if( !token.IsNumeric( ) ) {
			Common::Com_DPrintf( "CVar::SetString: type mismatch, expected int, but '%s' = '%s'\n", GetName( ).c_str( ), value.c_str( ) );
			return;
		}
		int interger = token.GetIntValue( );
		if( value[ 0 ] == '-' ) interger *= -1;
		SetInt( interger );
	} else if( d_flags & CVAR_FLOAT ) {
		token = value;
		token.type = TT_NUMBER;
		token.subtype = TT_FLOAT;
		if( !token.IsNumeric( ) ) {
			Common::Com_DPrintf( "CVar::SetString: type mismatch, expected float, but '%s' = '%s'\n", GetName( ).c_str( ), value.c_str( ) );
			return;
		}
		float real = token.GetFloatValue( );
		SetFloat( real );
	} else
		Common::Com_Error( ERR_DISCONNECT, "CVar::SetString: type error in '%s'\n", GetName( ).c_str( ) );
}

void CVar::FlushLatch( ) {
	if( d_flags & ( CVAR_LATCH | CVAR_STRING ) )
		SetString( d_latch->GetString( ) );
	else if	( d_flags & ( CVAR_LATCH | CVAR_BOOL ) )
		SetBool( d_latch->GetBool( ) );
	else if	( d_flags & ( CVAR_LATCH | CVAR_INT ) )
		SetInt( d_latch->GetInt( ) );
	else if	( d_flags & ( CVAR_LATCH | CVAR_FLOAT ) )
		SetFloat( d_latch->GetFloat( ) );
}

void CVar::FlushCheat( ) {
	if( d_flags & ( CVAR_CHEAT | CVAR_STRING ) )
		SetString( d_original->GetString( ) );
	else if( d_flags & ( CVAR_CHEAT | CVAR_BOOL ) )
		SetBool( d_original->GetBool( ) );
	else if( d_flags & ( CVAR_CHEAT | CVAR_INT ) )
		SetInt( d_original->GetInt( ) );
	else if( d_flags & ( CVAR_CHEAT | CVAR_FLOAT ) )
		SetFloat( d_original->GetFloat( ) );
}

void CVarSystem::LuaSetCVar( CVar * cvar, const char * value ) {
	bool isPreset = ( cvar->d_flags & CVAR_PRESET ) > 0;
	if( isPreset )
		cvar->d_flags = CVAR_PRESET | CVAR_STRING;
	if( isPreset || value != cvar->GetString( ) ) {
		cvar->SetString( value );
		cvar->SetModified( );
	}
}

void CVarSystem::LuaSetCVar( CVar * cvar, const bool value ) {
	bool isPreset = ( cvar->d_flags & CVAR_PRESET ) > 0;
	if( isPreset )
		cvar->d_flags = CVAR_PRESET | CVAR_BOOL;
	if( isPreset || value != cvar->GetBool( ) ) {
		cvar->SetBool( value );
		cvar->SetModified( );
	}
}

void CVarSystem::LuaSetCVar( CVar * cvar, const int value ) {
	bool isPreset = ( cvar->d_flags & CVAR_PRESET ) > 0;
	if( isPreset )
		cvar->d_flags = CVAR_PRESET | CVAR_INT;
	if( isPreset || value != cvar->GetInt( ) ) {
		cvar->SetInt( value );
		cvar->SetModified( );
	}
}

void CVarSystem::LuaSetCVar( CVar * cvar, const float value ) {
	bool isPreset = ( cvar->d_flags & CVAR_PRESET ) > 0;
	if( isPreset )
		cvar->d_flags = CVAR_PRESET | CVAR_FLOAT;
	if( isPreset || value != cvar->GetFloat( ) ) {
		cvar->SetFloat( value );
		cvar->SetModified( );
	}
}

#define CV_ENTRY_CHECK()
#define CV_LINE_CHECK()
#define	CV_RETURN_CHECK(x)	return(x);

int CVarSystem::lm_Name( LuaState & state ) {

	CV_ENTRY_CHECK();
	if( !state.IsUserdata( 1 ) ) CV_RETURN_CHECK(0);
	CVar * cVar = ( CVar * )state.GetPtr( 1 );
	if( !cVar ) {
		Lua::lua_pushnil( &state );
		CV_RETURN_CHECK(1);
	}
	Lua::lua_pushstring( &state, cVar->GetName( ) ); CV_LINE_CHECK();
	CV_RETURN_CHECK(1);
}

int CVarSystem::lm_Set( LuaState & state ) {

	CV_ENTRY_CHECK();
	if( !state.IsUserdata( 1 ) ) CV_RETURN_CHECK(0);
	CVar * cVar = ( CVar * )state.GetPtr( 1 );
	if( !cVar || cVar->GetFlags( ) & CVAR_READONLY ) CV_RETURN_CHECK(0);
	state.BeginItr( ); state++;
	if( state.IsNumber( ) ) {
		float fVal = state.GetFloat( );
		int iVal = ( int )fVal;
		if( fabs( fVal ) != fabs( ( float )iVal ) )	
			LuaSetCVar( cVar, fVal );
		else
			LuaSetCVar( cVar, iVal );
	} else if( state.IsString( ) )
		LuaSetCVar( cVar, state.GetString( ) );
	else if( state.IsBool( ) )
		LuaSetCVar( cVar, state.GetBool( ) );
	CV_RETURN_CHECK(0);
}

int CVarSystem::lm_Get( LuaState & state ) {

	CV_ENTRY_CHECK();
	if( !state.IsUserdata( 1 ) ) CV_RETURN_CHECK(0);
	CVar * cVar = ( CVar * )state.GetPtr( 1 );
	if( !cVar ) { Lua::lua_pushnil( &state ); CV_RETURN_CHECK(1); }
	switch( cVar->GetFlags( ) & ( CVAR_BOOL | CVAR_INT | CVAR_FLOAT | CVAR_STRING ) ) {
		case CVAR_BOOL:		Lua::lua_pushboolean( &state, cVar->GetInt( ) ); break;
		case CVAR_INT:		Lua::lua_pushinteger( &state, cVar->GetInt( ) ); break;
		case CVAR_FLOAT:	Lua::lua_pushnumber( &state, cVar->GetFloat( ) ); break;
		case CVAR_STRING:	Lua::lua_pushstring( &state, cVar->GetString( ) ); break;
		default:			Lua::lua_pushnil( &state );
	}
	CV_RETURN_CHECK(1);
}

int CVarSystem::lm_Desc( LuaState & state ) {

	CV_ENTRY_CHECK();
	if( !state.IsUserdata( 1 ) )
		CV_RETURN_CHECK(0);
	CVar * cVar = ( CVar * )state.GetPtr( 1 );
	if( !cVar || cVar->GetDescription( ).IsEmpty( ) ) {
		Lua::lua_pushnil( &state );
		CV_LINE_CHECK();
	}
	else {
		Lua::lua_pushstring( &state, cVar->GetDescription( ) );
		CV_LINE_CHECK();
	}
	CV_RETURN_CHECK(1);
}

int CVarSystem::lm___index( LuaState & state ) {

	CV_ENTRY_CHECK();
	CVar * cVar;
	Str cVarName;
	int t1 = state.GetType( 1 );
	switch( state.GetType( 1 ) ) {
		case LUA_TTABLE:
			cVarName = ( state.GetString( 2 ) );
			cVar = CVarSystem::Find( cVarName );
			if( !cVar ) {
				if( cVarName.IsEmpty( ) ) {
					Lua::lua_pushnil( &state );
					CV_RETURN_CHECK(1);
				}
				cVar = CVarSystem::CreatePreset( cVarName );
			}
			lua_pushlightuserdata( &state, cVar ); CV_LINE_CHECK();
			Lua::lua_getmetatable( &state, 1 ); CV_LINE_CHECK();
			Lua::lua_setmetatable( &state, -2 ); CV_LINE_CHECK();
			CV_RETURN_CHECK(1);

		case LUA_TLIGHTUSERDATA:
			Lua::lua_getmetatable( &state, -2 ); CV_LINE_CHECK();
			Lua::lua_pushvalue( &state, -2 ); CV_LINE_CHECK();
			Lua::lua_rawget( &state, -2 ); CV_LINE_CHECK();
			Lua::lua_replace( &state, -2 ); CV_LINE_CHECK();
			CV_RETURN_CHECK(1);
	}
	CV_RETURN_CHECK(0);
}

template< class T, class T_func_type, T_func_type T_func > class LuaAutoMethod {
public:
	static int			GetValue( LuaState & state ) {

		if( !state.IsUserdata( 1 ) ) {
			Lua::lua_pushstring( &state, "wrong parameter" );
			Lua::lua_error( &state );
			return 0;
		}
		T * userDataPtr = ( T * )state.GetPtr( 1 );
		auto userDataValue = ( userDataPtr->*T_func )( );
		state.Push( userDataValue );
		return 1;
	}
};
#define AutoMethodOf(type,func) LuaAutoMethod<type,decltype(func),func>

int CVarSystem::lg_registerCvars( LuaState & state ) {

	CV_ENTRY_CHECK();
	//state.PushGlobal( );
	Lua::lua_newtable( &state );
	state.PushMetatable( "CVar" );
	state.AddMethod2( "Name", &AutoMethodOf( CVar, &CVar::GetName )::GetValue );
	state.AddMethod2( "Set", lm_Set );
	state.AddMethod2( "Get", lm_Get );
	state.AddMethod2( "Desc", &AutoMethodOf( CVar, &CVar::GetDescription )::GetValue );
	state.AddMethod( "__index", lm___index );
	//state.PushTable( "CVar" );
	//Lua::lua_pushvalue( &state, 2 );
	Lua::lua_setmetatable( &state, -2 );
	//Lua::lua_setglobal( &state, "CVar" );
	//Lua::lua_rawset( &state, 1 );
	//state.Pop( 2 );
	//CV_RETURN_CHECK(0);
	CV_RETURN_CHECK(1);
}

void CVarSystem::Init( ) {

	Command::Cmd_AddCommand( "set", Set_f );
	Command::Cmd_AddCommand( "cvarlist", List_f );
	Command::Cmd_AddCommand( "find", Find_f );
	LuaScript::RegisterModule( "CVar", lg_registerCvars );
}

void CVarSystem::Register( CVar * cvar ) {
	if( cvar->d_flags & CVAR_NOREGISTER )
		return;
	int cvari = g_CVarList.IndexByName( cvar->d_name );
	if( cvari != -1 ) {
		CVar * cvar2 = g_CVarList[ cvari ];
		if( cvar2->d_flags & CVAR_PRESET ) {
			switch( cvar->d_flags & ( CVAR_BOOL | CVAR_INT | CVAR_FLOAT | CVAR_STRING ) ) {
				case CVAR_BOOL:
					cvar->SetBool( cvar2->GetBool( ) ); break;
				case CVAR_INT:
					cvar->SetInt( cvar2->GetInt( ) ); break;
				case CVAR_FLOAT:
					cvar->SetFloat( cvar2->GetFloat( ) ); break;
				case CVAR_STRING:				
					cvar->SetString( cvar2->GetString( ) ); break;
			}
			delete cvar2;
			g_CVarList[ cvari ] = cvar;
		} else
			Common::Com_DPrintf( "CVarSystem::Register: cvar '%s' already registered\n", cvar->d_name.c_str( ) );
	} else
		g_CVarList.Append( cvar );
}

void CVarSystem::UnRegister( CVar * cvar ) {
	g_CVarList.Remove( cvar );
}

void CVarSystem::WriteCVars( FileBase * f ) {

	Str cVal;
	for( int i = 0; i < g_CVarList.Num( ); i++ ) {
		CVar * cVar = g_CVarList[ i ];
		int cFlags = cVar->GetFlags( );
		if( !( cFlags & CVAR_ARCHIVE ) || ( cFlags & CVAR_PRESET ) )
			continue;
		cVal = "CVar.";
		cVal += cVar->GetName( );
		cVal += ":Set( ";
		switch( cFlags & ( CVAR_BOOL | CVAR_INT | CVAR_FLOAT | CVAR_STRING ) ) {
			case CVAR_BOOL:				
				cVal += cVar->GetBool( );
				break;

			case CVAR_INT:				
				cVal += cVar->GetInt( );
				break;

			case CVAR_FLOAT:				
				cVal += cVar->GetFloat( );
				for( int iS = cVal.Length( ); iS > 3 && cVal[ iS - 1 ] == '0' && cVal[ iS - 2 ] != '.'; cVal.CapLength( --iS ) );
				break;

			case CVAR_STRING:				
				cVal += '\'';
				cVal += cVar->GetString( );
				cVal += '\'';
				break;
		}
		if( cVar->GetDescription( ).Length( ) ) {
			cVal += " )\t\t-- ";
			cVal += cVar->GetDescription( );
			cVal += '\n';
		} else
			cVal += " )\n";
		f->Write( cVal.c_str( ), cVal.Length( ) );
	}
}

const Str CVarSystem::BitInfo( int flags ) {

	Str info;
	for( int i = 0; i < g_CVarList.Num( ); i++ )
		if( g_CVarList[ i ]->GetFlags( ) & flags )
			ClientServerInfo::Info_SetValueForKey( info, g_CVarList[ i ]->GetName( ), g_CVarList[ i ]->GetString( ) );
	return info;
}

const Str CVarSystem::Userinfo( ) {
	return BitInfo( CVAR_USERINFO );
}

const Str CVarSystem::Serverinfo( ) {
	return BitInfo( CVAR_SERVERINFO );
}

void CVarSystem::GetLatchedVars( ) {

	for( int i = 0; i < g_CVarList.Num( ); i++ )
		if( g_CVarList[ i ]->GetFlags( ) & CVAR_LATCH )
			g_CVarList[ i ]->FlushLatch( );
}

void CVarSystem::GetCheatVars( ) {
	for( int i = 0; i < g_CVarList.Num( ); i++ )
		if( g_CVarList[ i ]->GetFlags( ) & CVAR_CHEAT )
			g_CVarList[ i ]->FlushCheat( );
}

bool CVarSystem::Command( ) {

	CVar * var = Find( Command::Cmd_Argv( 0 ) );
	if( !var )
		return false;		
	// perform a variable print or set
	if( Command::Cmd_Argc( ) == 1 ) {
		Common::Com_Printf( "\"%s\" is \"%s\"\n", var->GetName( ).c_str( ), var->GetString( ).c_str( ) );
		return true;
	}
	var->SetString( Command::Cmd_Argv( 1 ) );
	var->SetModified( );
	return true;
}

const Str & CVarSystem::CompleteVariable( const Str & partial ) {
	
	int len = partial.Length( );	
	if( !len )
		return null_string;
	CVar * var = Find( partial );
	if( var )
		return var->GetName( );
	// check partial match
	for( int i = 0; i < g_CVarList.Num( ); i++ )
		if( !Str::Cmpn( partial, g_CVarList[ i ]->GetName( ), len ) )
			return g_CVarList[ i ]->GetName( );
	return null_string;
}

void CVarSystem::Set_f( ) {

	int c = Command::Cmd_Argc( );
	if( c != 3 ) {
		Common::Com_Printf( "usage: set <variable> <value>\n" );
		return;
	}
	SetCVarStr( Command::Cmd_Argv( 1 ), Command::Cmd_Argv( 2 ) );
}

void CVarSystem::List_f( ) {

	for( int i = 0; i < g_CVarList.Num( ); i++ ) {

		int flags = g_CVarList[ i ]->GetFlags( );

		if( flags & CVAR_ARCHIVE )
			Common::Com_Printf( "*" );
		else
			Common::Com_Printf( " " );
		if( flags & CVAR_USERINFO )
			Common::Com_Printf( "U" );
		else
			Common::Com_Printf( " " );
		if( flags & CVAR_SERVERINFO )
			Common::Com_Printf( "S" );
		else
			Common::Com_Printf( " " );
		if( flags & CVAR_NOSET )
			Common::Com_Printf( "-" );
		else if( flags & CVAR_LATCH )
			Common::Com_Printf( "L" );
		else
			Common::Com_Printf( " " );

		Common::Com_Printf( " %s \"%s\" \"%s\"\n", g_CVarList[ i ]->GetName( ).c_str( ), g_CVarList[ i ]->GetString( ).c_str( ), g_CVarList[ i ]->GetDescription( ).c_str( ) );
	}
}

void CVarSystem::Find_f( ) {

	for( int i = 0; i < g_CVarList.Num( ); i++ ) {
		for( int j = 1; j < Command::Cmd_Argc( ); j++ ) {
			if( g_CVarList[ i ]->GetName( ).Find( Command::Cmd_Argv( j ), false ) == -1 )
				if( g_CVarList[ i ]->GetDescription( ).Find( Command::Cmd_Argv( j ), false ) == -1 )
					continue;
			Common::Com_Printf( "%s \"%s\"\n", g_CVarList[ i ]->GetName( ).c_str( ), g_CVarList[ i ]->GetDescription( ).c_str( ) );
			break;
		}
	}
}
