// Copyright ( C) 2004 Id Software, Inc.
//

#ifndef __CVARSYSTEM_H__
#define __CVARSYSTEM_H__

/*
===============================================================================

	Console Variables ( CVars) are used to hold scalar or string variables
	that can be changed or displayed at the console as well as accessed
	directly in code.

	CVars are mostly used to hold settings that can be changed from the
	console or saved to and loaded from configuration files. CVars are also
	occasionally used to communicate information between different modules
	of the program.

	CVars are restricted from having the same names as console commands to
	keep the console interface from being ambiguous.

	CVars can be accessed from the console in three ways:
	cvarName			prints the current value
	cvarName X			sets the value to X if the variable exists
	set cvarName X		as above, but creates the CVar if not present

	CVars may be declared in the global namespace, in classes and in functions.
	However declarations in classes and functions should always be static to
	save space and time. Making CVars static does not change their
	functionality due to their global nature.

	CVars should be contructed only through one of the constructors with name,
	value, flags and description. The name, value and description parameters
	to the constructor have to be static strings, do not use va( ) or the like
	functions returning a string.

	CVars may be declared multiple times using the same name string. However,
	they will all reference the same value and changing the value of one CVar
	changes the value of all CVars with the same name.

	CVars should always be declared with the correct type flag: CVAR_BOOL,
	CVAR_INTEGER or CVAR_FLOAT. If no such flag is specified the CVar
	defaults to type string. If the CVAR_BOOL flag is used there is no need
	to specify an argument auto-completion function because the CVar gets
	one assigned automatically.

	CVars are automatically range checked based on their type and any min/max
	or valid string set specified in the constructor.

	CVars are always considered cheats except when CVAR_NOCHEAT, CVAR_INIT,
	CVAR_ROM, CVAR_ARCHIVE, CVAR_USERINFO, CVAR_SERVERINFO, CVAR_NETWORKSYNC
	is set.

===============================================================================
*/

typedef enum {

	CVAR_ALL				= -1,			// all flags
	CVAR_BOOL				= BIT( 0 ),		// variable is a boolean
	CVAR_INT				= BIT( 1 ),		// variable is an integer
	CVAR_FLOAT				= BIT( 2 ),		// variable is a float
	CVAR_LIMIT				= BIT( 3 ),
	CVAR_STRING				= BIT( 4 ),		// variable is a string
	CVAR_READONLY			= BIT( 5 ),		// display only, cannot be set by user at all
	CVAR_ARCHIVE			= BIT( 6 ),		// set to cause it to be saved to a config file
	CVAR_INIT				= BIT( 7 ),		// can only be set from the command-line
	CVAR_MODIFIED			= BIT( 8 ),		// set when the variable is modified
	CVAR_USERINFO			= BIT( 9 ),		// added to userinfo  when changed
	CVAR_SERVERINFO			= BIT( 10 ),	// added to serverinfo when changed
	CVAR_PRESET				= BIT( 11 ),
	CVAR_NOSET				= BIT( 12 ),	// don't allow change from console at all, // but can be set from the command line
	CVAR_NOREGISTER			= BIT( 13 ),
	CVAR_DYNAMIC			= BIT( 14 ),
	CVAR_LATCH				= BIT( 15 ),	// save changes until server restart
	CVAR_CHEAT				= BIT( 16 )

} cvarFlags_t;


/*
===============================================================================

	CVar

===============================================================================
*/

class CVar : public NamedObject {
	friend class			CVarSystem;
private:

	CVar *					d_latch;
	CVar *					d_original;				// for eliminating cheats
	Str						d_stringValue;			// value
	Str						d_description;			// description
	int						d_flags;				// CVAR_? flags
	float					d_valueMin;				// minimum value
	float					d_valueMax;				// maximum value
	argCompletion_t			d_valueCompletion;		// value auto-completion function
	int						d_integerValue;			// atoi( string )
	float					d_floatValue;			// atof( value )

	void					Init( const Str & name, const Str & value, int flags, const Str & description, float valueMin, float valueMax, argCompletion_t valueCompletion );
	void					SecInit( );

public:

							CVar( const Str & name, const Str & value, int flags, const Str & description = null_string, argCompletion_t valueCompletion = NULL );
							CVar( const Str & name, const Str & value, int flags, const Str & description, float valueMin, float valueMax, argCompletion_t valueCompletion = NULL );
							CVar( );
							~CVar( );

	CVar &					operator =( const CVar & cvar );
	void					DRegister( const Str & name, const Str & value, int extra_flags = 0, const Str & description = null_string );
	void					DRegister( const Str & name, const bool value, int extra_flags = 0, const Str & description = null_string );
	void					DRegister( const Str & name, const int value, int extra_flags = 0, const Str & description = null_string );
	void					DRegister( const Str & name, const float value, int extra_flags = 0, const Str & description = null_string );

	int						GetFlags( ) const			{ return d_flags; }
	const Str &				GetDescription( ) const		{ return d_description; }
	float					GetMinValue( ) const		{ return d_valueMin; }
	float					GetMaxValue( ) const		{ return d_valueMax; }
	argCompletion_t			GetValueCompletion( ) const	{ return d_valueCompletion; }

	bool					IsModified( ) const			{ return ( d_flags & CVAR_MODIFIED ) != 0; }
	void					SetModified( )				{ d_flags |= CVAR_MODIFIED; }
	void					ClearModified( )			{ d_flags &= ~CVAR_MODIFIED; }

	const Str &				GetString( ) const;
	bool					GetBool( ) const			{ return ( d_integerValue != 0 ); }
	int						GetInt( ) const				{ return d_integerValue; }
	float					GetFloat( ) const			{ return d_floatValue; }

	void					SetString( const Str & value );
	void					SetBool( const bool value );
	void					SetInt( const int value );
	void					SetFloat( const float value );

	void					LimitValue( );
	void					FlushLatch( );
	void					FlushCheat( );

	bool					IsInfoValid( );
};

/*
===============================================================================

	CVarSystem

===============================================================================
*/

class CVarSystem {

private:

	static CANamedList< CVar *, 256 >		g_CVarList;

	static int				lm_Name( LuaState & state );
	static int				lm_Set( LuaState & state );
	static int				lm_Get( LuaState & state );
	static int				lm_Desc( LuaState & state );
	static int				lm___index( LuaState & state );
	static int				lg_registerCvars( LuaState & state );

	static void				LuaSetCVar( CVar * cvar, const char * value );
	static void				LuaSetCVar( CVar * cvar, const bool value );
	static void				LuaSetCVar( CVar * cvar, const int value );
	static void				LuaSetCVar( CVar * cvar, const float value );

public:

	static void				Init( );
	static void				Shutdown( );
	//virtual bool			IsInitialized( ) const;

							// Registers a CVar.
	static void				Register( CVar * cvar );
	static void				UnRegister( CVar * cvar );

							// Finds the CVar with the given name.
							// Returns NULL if there is no CVar with the given name.
	static CVar *			Find( const Str & name );

	static CVar *			CreatePreset( const Str & name );

							// Sets the value of a CVar by name.
	static void				SetCVarStr( const Str & name, const Str & value, int flags = 0 );
	static void				SetCVarBool( const Str & name, const bool value, int flags = 0 );
	static void				SetCVarInt( const Str & name, const int value, int flags = 0 );
	static void				SetCVarFloat( const Str & name, const float value, int flags = 0 );

							// Gets the value of a CVar by name.
	static const Str &		GetCVarStr( const Str & name );
	static bool				GetCVarBool( const Str & name );
	static int				GetCVarInt( const Str & name );
	static float			GetCVarFloat( const Str & name );

							// Called by the command system when argv( 0) doesn't match a known command.
							// Returns true if argv( 0) is a variable reference and prints or changes the CVar.
	//virtual bool			Command( const CmdArgs &args );
	static bool				Command( );

							// Command and argument completion using callback for each valid string.
	//virtual void			CommandCompletion( void( *callback)( const char *s ) );
	//virtual void			ArgCompletion( const char *cmdString, void( *callback)( const char *s ) );

							// Sets/gets/clears modified flags that tell what kind of CVars have changed.
	//virtual void			SetModifiedFlags( int flags );
	//virtual int				GetModifiedFlags( ) const;
	//virtual void			ClearModifiedFlags( int flags );

							// Resets variables with one of the given flags set.
	//virtual void			ResetFlaggedVariables( int flags );

							// Removes auto-completion from the flagged variables.
	//virtual void			RemoveFlaggedAutoCompletion( int flags );

							// Writes variables with one of the given flags set to the given file.
	static void				WriteCVars( FileBase * f );

							// Moves CVars to and from dictionaries.
	//virtual const Dict *	MoveCVarsToDict( int flags ) const;
	//virtual void			SetCVarsFromDict( const Dict &dict );

	static const Str		BitInfo( int flags );

							// returns an info string containing all the CVAR_USERINFO cvars
	static const Str		Userinfo( );
	
							// returns an info string containing all the CVAR_SERVERINFO cvars
	static const Str		Serverinfo( );

	static void				GetLatchedVars( );	
	static void				GetCheatVars( );

	static const Str &		CompleteVariable( const Str & partial );

	static void				Set_f( );
	static void				List_f( );
	static void				Find_f( );
};

//
// CVAR INLINEs
//

INLINE void CVar::LimitValue( ) {
	if( d_flags & CVAR_LIMIT ) {
		if( d_flags & CVAR_FLOAT )
			d_floatValue = Max( Min( d_floatValue, d_valueMax ), d_valueMin );
		else if( d_flags & CVAR_INT )
			d_integerValue = Max( Min( d_integerValue, ( int )d_valueMax ), ( int )d_valueMin );
	}
}

INLINE const Str & CVar::GetString( ) const {
	return d_stringValue;
}

INLINE void CVar::SetBool( const bool value ) {
	if( d_flags & CVAR_USERINFO )
		Common::userinfo_modified.SetBool( true );
	CVar * workon = ( d_flags & CVAR_LATCH && d_latch ) ? d_latch : this;
	workon->d_integerValue = ( int )value;
	workon->d_floatValue = ( float )workon->d_integerValue;
	workon->d_stringValue = workon->d_integerValue ? true_string : false_string;
}

INLINE void CVar::SetInt( const int value ) {
	if( d_flags & CVAR_USERINFO )
		Common::userinfo_modified.SetBool( true );
	CVar * workon = ( d_flags & CVAR_LATCH && d_latch ) ? d_latch : this;

	workon->d_integerValue = value;
	workon->LimitValue( );
	workon->d_floatValue = ( float )workon->d_integerValue;
	sprintf( workon->d_stringValue, "%i", workon->d_integerValue );
}

INLINE void CVar::SetFloat( const float value ) {

	if( d_flags & CVAR_USERINFO )
		Common::userinfo_modified.SetBool( true );
	CVar * workon = ( d_flags & CVAR_LATCH && d_latch ) ? d_latch : this;
	workon->d_floatValue = value;
	workon->LimitValue( );
	workon->d_integerValue = ( int )workon->d_floatValue;
	sprintf( workon->d_stringValue, "%f", workon->d_floatValue );
}

//
// CVARSYSTEM INLINEs
//

INLINE CVar * CVarSystem::Find( const Str & name ) {
	return g_CVarList.FindByName( name );
}

INLINE CVar * CVarSystem::CreatePreset( const Str & name ) {
	return new CVar( name, "", CVAR_PRESET, "preset cvar" );
}

INLINE void CVarSystem::SetCVarStr( const Str & name, const Str & value, int flags ) {
	CVar * cvar = g_CVarList.FindByName( name );
	if( cvar )
		cvar->SetString( value );
}

INLINE void CVarSystem::SetCVarBool( const Str & name, const bool value, int flags ) {
	CVar * cvar = g_CVarList.FindByName( name );
	if( cvar )
		cvar->SetBool( value );
}

INLINE void CVarSystem::SetCVarInt( const Str & name, const int value, int flags ) {
	CVar * cvar = g_CVarList.FindByName( name );
	if( cvar )
		cvar->SetInt( value );
}

INLINE void CVarSystem::SetCVarFloat( const Str & name, const float value, int flags ) {
	CVar * cvar = g_CVarList.FindByName( name );
	if( cvar )
		cvar->SetFloat( value );
}

INLINE const Str & CVarSystem::GetCVarStr( const Str & name ) {
	CVar * cvar = g_CVarList.FindByName( name );
	if( cvar )
		return cvar->GetString( );
	return null_string;
}

INLINE bool CVarSystem::GetCVarBool( const Str & name ) {
	CVar * cvar = g_CVarList.FindByName( name );
	if( cvar )
		return cvar->GetBool( );
	return false;
}

INLINE int CVarSystem::GetCVarInt( const Str & name ) {
	CVar * cvar = g_CVarList.FindByName( name );
	if( cvar )
		return cvar->GetInt( );
	return 0;
}

INLINE float CVarSystem::GetCVarFloat( const Str & name ) {
	CVar * cvar = g_CVarList.FindByName( name );
	if( cvar )
		return cvar->GetFloat( );
	return 0.0f;
}

#endif /* !__CVARSYSTEM_H__ */
