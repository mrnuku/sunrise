// Copyright ( C) 2004 Id Software, Inc.
//

#ifndef __CMDSYSTEM_H__
#define __CMDSYSTEM_H__

/*
===============================================================================

	Console command execution and command text buffering.

	Any number of commands can be added in a frame from several different
	sources. Most commands come from either key bindings or console line input,
	but entire text files can be execed.

	Command execution takes a null terminated string, breaks it into tokens,
	then searches for a command or variable that matches the first token.

===============================================================================
*/

// command flags
typedef enum {
	CMD_FL_ALL				= -1,
	CMD_FL_CHEAT			= BIT( 0),	// command is considered a cheat
	CMD_FL_SYSTEM			= BIT( 1),	// system command
	CMD_FL_RENDERER			= BIT( 2),	// renderer command
	CMD_FL_SOUND			= BIT( 3),	// sound command
	CMD_FL_GAME				= BIT( 4),	// game command
	CMD_FL_TOOL				= BIT( 5)	// tool command
} cmdFlags_t;

// parameters for command buffer stuffing
typedef enum {
	CMD_EXEC_NOW,						// don't return until completed
	CMD_EXEC_INSERT,					// insert at current position, but don't run yet
	CMD_EXEC_APPEND						// add to end of the command buffer ( normal case)
} cmdExecution_t;

// command function
typedef void ( *cmdFunction_t)( const CmdArgs &args );

// argument completion function
typedef void ( *argCompletion_t)( const CmdArgs &args, void( *callback)( const char *s ) );


class CmdSystem {
public:
	virtual				~CmdSystem( ) {}

	virtual void		Init( ) = 0;
	virtual void		Shutdown( ) = 0;

						// Registers a command and the function to call for it.
	virtual void		AddCommand( const char *cmdName, cmdFunction_t function, int flags, const char *description, argCompletion_t argCompletion = NULL ) = 0;
						// Removes a command.
	virtual void		RemoveCommand( const char *cmdName ) = 0;
						// Remove all commands with one of the flags set.
	virtual void		RemoveFlaggedCommands( int flags ) = 0;

						// Command and argument completion using callback for each valid string.
	virtual void		CommandCompletion( void( *callback)( const char *s ) ) = 0;
	virtual void		ArgCompletion( const char *cmdString, void( *callback)( const char *s ) ) = 0;

						// Adds command text to the command buffer, does not add a final \n
	virtual void		BufferCommandText( cmdExecution_t exec, const char *text ) = 0;
						// Pulls off \n \r or ; terminated lines of text from the command buffer and
						// executes the commands. Stops when the buffer is empty.
						// Normally called once per frame, but may be explicitly invoked.
	virtual void		ExecuteCommandBuffer( ) = 0;

						// Base for path/file auto-completion.
	virtual void		ArgCompletion_FolderExtension( const CmdArgs &args, void( *callback)( const char *s ), const char *folder, bool stripFolder, ... ) = 0;
						// Base for decl name auto-completion.
	virtual void		ArgCompletion_DeclName( const CmdArgs &args, void( *callback)( const char *s ), int type ) = 0;

						// Adds to the command buffer in tokenized form ( CMD_EXEC_NOW or CMD_EXEC_APPEND only )
	virtual void		BufferCommandArgs( cmdExecution_t exec, const CmdArgs &args ) = 0;

						// Setup a reloadEngine to happen on next command run, and give a command to execute after reload
	virtual void		SetupReloadEngine( const CmdArgs &args ) = 0;
	virtual bool		PostReloadEngine( ) = 0;

						// Default argument completion functions.
	static void			ArgCompletion_Boolean( const CmdArgs &args, void( *callback)( const char *s ) );
	template<int min,int max>
	static void			ArgCompletion_Integer( const CmdArgs &args, void( *callback)( const char *s ) );
	template<const char **strings>
	static void			ArgCompletion_String( const CmdArgs &args, void( *callback)( const char *s ) );
	template<int type>
	static void			ArgCompletion_Decl( const CmdArgs &args, void( *callback)( const char *s ) );
	static void			ArgCompletion_FileName( const CmdArgs &args, void( *callback)( const char *s ) );
	static void			ArgCompletion_MapName( const CmdArgs &args, void( *callback)( const char *s ) );
	static void			ArgCompletion_ModelName( const CmdArgs &args, void( *callback)( const char *s ) );
	static void			ArgCompletion_SoundName( const CmdArgs &args, void( *callback)( const char *s ) );
	static void			ArgCompletion_ImageName( const CmdArgs &args, void( *callback)( const char *s ) );
	static void			ArgCompletion_VideoName( const CmdArgs &args, void( *callback)( const char *s ) );
	static void			ArgCompletion_ConfigName( const CmdArgs &args, void( *callback)( const char *s ) );
	static void			ArgCompletion_SaveGame( const CmdArgs &args, void( *callback)( const char *s ) );
	static void			ArgCompletion_DemoName( const CmdArgs &args, void( *callback)( const char *s ) );
};

extern CmdSystem *	cmdSystem;


INLINE void CmdSystem::ArgCompletion_Boolean( const CmdArgs &args, void( *callback)( const char *s ) ) {
	callback( va( "%s 0", args.Argv( 0 ) ) );
	callback( va( "%s 1", args.Argv( 0 ) ) );
}

template<int min,int max> STATIC_TEMPLATE INLINE void CmdSystem::ArgCompletion_Integer( const CmdArgs &args, void( *callback)( const char *s ) ) {
	for ( int i = min; i <= max; i++ ) {
		callback( va( "%s %d", args.Argv( 0 ), i ) );
	}
}

template<const char **strings> STATIC_TEMPLATE INLINE void CmdSystem::ArgCompletion_String( const CmdArgs &args, void( *callback)( const char *s ) ) {
	for ( int i = 0; strings[i]; i++ ) {
		callback( va( "%s %s", args.Argv( 0 ), strings[i] ) );
	}
}

template<int type> STATIC_TEMPLATE INLINE void CmdSystem::ArgCompletion_Decl( const CmdArgs &args, void( *callback)( const char *s ) ) {
	cmdSystem->ArgCompletion_DeclName( args, callback, type );
}

INLINE void CmdSystem::ArgCompletion_FileName( const CmdArgs &args, void( *callback)( const char *s ) ) {
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "/", true, "", NULL );
}

INLINE void CmdSystem::ArgCompletion_MapName( const CmdArgs &args, void( *callback)( const char *s ) ) {
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "maps/", true, ".map", NULL );
}

INLINE void CmdSystem::ArgCompletion_ModelName( const CmdArgs &args, void( *callback)( const char *s ) ) {
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "models/", false, ".lwo", ".ase", ".md5mesh", ".ma", NULL );
}

INLINE void CmdSystem::ArgCompletion_SoundName( const CmdArgs &args, void( *callback)( const char *s ) ) {
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "sound/", false, ".wav", ".ogg", NULL );
}

INLINE void CmdSystem::ArgCompletion_ImageName( const CmdArgs &args, void( *callback)( const char *s ) ) {
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "/", false, ".tga", ".dds", ".jpg", ".pcx", NULL );
}

INLINE void CmdSystem::ArgCompletion_VideoName( const CmdArgs &args, void( *callback)( const char *s ) ) {
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "video/", false, ".roq", NULL );
}

INLINE void CmdSystem::ArgCompletion_ConfigName( const CmdArgs &args, void( *callback)( const char *s ) ) {
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "/", true, ".cfg", NULL );
}

INLINE void CmdSystem::ArgCompletion_SaveGame( const CmdArgs &args, void( *callback)( const char *s ) ) {
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "SaveGames/", true, ".save", NULL );
}

INLINE void CmdSystem::ArgCompletion_DemoName( const CmdArgs &args, void( *callback)( const char *s ) ) {
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "demos/", true, ".demo", NULL );
}

#endif /* !__CMDSYSTEM_H__ */
