// Copyright ( C) 2004 Id Software, Inc.
//

#ifndef __SYS_TYPEINFO_H__
#define __SYS_TYPEINFO_H__

/*
===================================================================================

	Game Type Info

===================================================================================
*/

const char *	GetTypeVariableName( const char *typeName, int offset );

void			PrintType( const void *typePtr, const char *typeName );
void			InitTypeVariables( const void *typePtr, const char *typeName, int value );

void			ListTypeInfo_f( const CmdArgs &args );

void			WriteGameState_f( const CmdArgs &args );
void			CompareGameState_f( const CmdArgs &args );

#endif /* !__SYS_TYPEINFO_H__ */
