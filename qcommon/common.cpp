#include "precompiled.h"
#pragma hdrstop

#define	MAXPRINTMSG	10240

#define MAX_NUM_ARGVS	50


int		com_argc;
char	* com_argv[ MAX_NUM_ARGVS+1 ];

int		realtime;

jmp_buf abortframe;		// an ERR_DROP occured, exit the entire frame

int			server_state;

/*
============================================================================

CLIENT / SERVER interactions

============================================================================
*/

Str *	Common::g_redirectBuffer = NULL;

/*
=============
Common::Com_Printf

Both client and server can use this, and it will output
to the apropriate place.
=============
*/
void Common::Com_Printf( const char * fmt, ... ) {
	va_list		argptr;
	Str			msg;
	va_start( argptr, fmt );
	vsprintf( msg, fmt, argptr );
	va_end( argptr );
	if( g_redirectBuffer ) {
		g_redirectBuffer->Append( msg );
		return;
	}
	//Console::Con_Print( msg );
#ifndef DEDICATED_ONLY
	if( !dedicated.GetBool( ) )
	GUISystem::PrintConsole( msg );
#endif
		
	// also echo to debugging console
	//System::Sys_ConsoleOutput( msg );
	if( System::Sys_HaveConsole( ) )
		System::Sys_OutputConsole( msg );

	WindowsNS::OutputDebugStringA( msg );

	// logfile
	if( logfile_active.GetBool( ) ) {
		Str	name;		
		if( !logfile ) {
			if( logfile_active.GetInt( ) > 2 )
				logfile = FileSystem::OpenFileByMode( logfile_name, FS_APPEND );
			else
				logfile = FileSystem::OpenFileByMode( logfile_name, FS_WRITE );
		}
		if( logfile )
			logfile->Printf( "%s", msg.c_str( ) );
		if( logfile_active.GetInt( ) > 1 )
			logfile->Flush( );		// force it to save every time
	}
}


/*
================
Com_DPrintf

A Common::Com_Printf that only shows up if the "developer" cvar is set
================
*/
void Common::Com_DPrintf( const char * fmt, ... ) {
	if( !developer.GetBool( ) )
		return; // don't confuse non-developers with techie stuff...
	va_list		argptr;
	Str			msg;
	va_start( argptr, fmt );
	vsprintf( msg, fmt, argptr );
	va_end( argptr );
	
	//Common::Com_Printf( "%s", msg.c_str( ) );
#ifndef DEDICATED_ONLY
	if( !dedicated.GetBool( ) )
	GUISystem::PrintConsoleColor( msg, color_yellow );
#endif

	if( System::Sys_HaveConsole( ) )
		System::Sys_OutputConsole( msg );

	WindowsNS::OutputDebugStringA( msg );
}


/*
=============
Common::Com_Error

Both client and server can use this, and it will
do the apropriate things.
=============
*/
void Common::Com_Error( int code, const char * fmt, ... ) {
	va_list			argptr;
	Str				msg;
	static	bool	recursive = false;

	if( recursive )
		System::Sys_Error( "recursive error after: %s", msg.c_str( ) );
	recursive = true;
	va_start( argptr, fmt );
	vsprintf( msg, fmt, argptr );
	va_end( argptr );
	
	if( code == ERR_DISCONNECT ) {
#ifndef DEDICATED_ONLY
		if( !dedicated.GetBool( ) )
			Client::CL_Drop( );
#endif
		recursive = false;
		longjmp( abortframe, -1 );

	} else if( code == ERR_DROP ) {
		//Common::Com_Printf( "********************\nERROR: %s\n********************\n", msg.c_str( ) );
		Server::SV_Shutdown( va( "Server crashed: %s\n", msg.c_str( ) ), false );
#ifndef DEDICATED_ONLY
		if( !dedicated.GetBool( ) ) {
			GUISystem::PrintConsoleColor( msg, color_red );
			Client::CL_Drop( );
		}
#endif
		recursive = false;
		longjmp( abortframe, -1 );

	} else {
		Server::SV_Shutdown( va( "Server fatal crashed: %s\n", msg.c_str( ) ), false );
#ifndef DEDICATED_ONLY
		if( !dedicated.GetBool( ) )
			Client::CL_Shutdown( );
#endif
	}

	if( logfile )
		FileSystem::CloseFile( logfile );

	System::Sys_Error( "%s", msg.c_str( ) );
}


/*
=============
Com_Quit

Both client and server can use this, and it will
do the apropriate things.
=============
*/
void Common::Com_Quit( ) {

	Server::SV_Shutdown( "Server quit\n", false );
#ifndef DEDICATED_ONLY
	if( !dedicated.GetBool( ) )
		Client::CL_Shutdown( );
#endif
	if( logfile )
		FileSystem::CloseFile( logfile );

	Common::Shutdown( );
	System::Sys_Quit( );
}


/*
==================
Com_ServerState
==================
*/
int Common::Com_ServerState( ) {

	return server_state;
}

/*
==================
Com_SetServerState
==================
*/
void Common::Com_SetServerState( int state ) {

	server_state = state;
}

/*
==============================================================================

			MESSAGE IO FUNCTIONS

Handles byte ordering and avoids alignment errors
==============================================================================
*/

//
// writing functions
//
#if 0

void Message::MSG_WriteChar( sizebuf_t * sb, int c ) {

	byte	* buf;
	
#ifdef PARANOID
	if( c < -128 || c > 127 )
		Common::Com_Error( ERR_FATAL, "MSG_WriteChar: range error" );
#endif

	buf =( byte * )SZ_GetSpace( sb, 1 );
	buf[ 0 ] = c;
}

void Message::MSG_WriteByte( sizebuf_t * sb, int c ) {

	byte	* buf;
	
#ifdef PARANOID
	if( c < 0 || c > 255 )
		Common::Com_Error( ERR_FATAL, "MSG_WriteByte: range error" );
#endif

	buf =( byte * )SZ_GetSpace( sb, 1 );
	buf[ 0 ] = c;
}

void Message::MSG_WriteShort( sizebuf_t * sb, int c ) {

	byte	* buf;
	
#ifdef PARANOID
	if( c <( ( short )0x8000 ) || c >( short )0x7FFF )
		Common::Com_Error( ERR_FATAL, "MSG_WriteShort: range error" );
#endif

	buf =( byte * )SZ_GetSpace( sb, 2 );
	buf[ 0 ] = c&0xFF;
	buf[ 1 ] = c>>8;
}

void Message::MSG_WriteLong( sizebuf_t * sb, int c ) {

	byte	* buf;
	
	buf =( byte * )SZ_GetSpace( sb, 4 );
	buf[ 0 ] = c&0xFF;
	buf[ 1 ] =( c>>8 )&0xFF;
	buf[ 2 ] =( c>>16 )&0xFF;
	buf[ 3 ] = c>>24;
}

void Message::MSG_WriteFloat( sizebuf_t * sb, float f ) {

	union
	{
		float	f;
		int	l;
	} dat;
	
	
	dat.f = f;
	dat.l = LittleLong( dat.l );
	
	SZ_Write( sb, &dat.l, 4 );
}

void Message::MSG_WriteString( sizebuf_t * sb, const Str & str ) {

	int len = str.Length( );

	if( !len ) SZ_Write( sb, "", 1 );
	else SZ_Write( sb, str.c_str( ), len + 1 );
}

void Message::MSG_WriteCoord( sizebuf_t * sb, float f ) {

	MSG_WriteShort( sb, ( int )( f* 8 ) );
}

void Message::MSG_WritePos( sizebuf_t * sb, Vec3 & pos ) {

	MSG_WriteShort( sb, ( int )( pos[ 0 ]* 8 ) );
	MSG_WriteShort( sb, ( int )( pos[ 1 ]* 8 ) );
	MSG_WriteShort( sb, ( int )( pos[ 2 ]* 8 ) );
	//MSG_WriteVec( sb, pos );
}

void Message::MSG_WriteAngle( sizebuf_t * sb, float f ) {

	MSG_WriteByte( sb, ( int )( f* 256/360 ) & 255 );
}

void Message::MSG_WriteAngle16( sizebuf_t * sb, float f ) {

	MSG_WriteShort( sb, ANGLE2SHORT( f ) );
}


void Message::MSG_WriteDeltaUsercmd( sizebuf_t * buf, usercmd_t * from, usercmd_t * cmd ) {

	int		bits;

//
// send the movement message
//
	bits = 0;
	if( cmd->angles[ 0 ] != from->angles[ 0 ] )
		bits |= CM_ANGLE1;
	if( cmd->angles[ 1 ] != from->angles[ 1 ] )
		bits |= CM_ANGLE2;
	if( cmd->angles[ 2 ] != from->angles[ 2 ] )
		bits |= CM_ANGLE3;
	if( cmd->forwardmove != from->forwardmove )
		bits |= CM_FORWARD;
	if( cmd->sidemove != from->sidemove )
		bits |= CM_SIDE;
	if( cmd->upmove != from->upmove )
		bits |= CM_UP;
	if( cmd->buttons != from->buttons )
		bits |= CM_BUTTONS;
	if( cmd->impulse != from->impulse )
		bits |= CM_IMPULSE;

    MSG_WriteByte( buf, bits );

	if( bits & CM_ANGLE1 )
		MSG_WriteShort( buf, cmd->angles[ 0 ] );
	if( bits & CM_ANGLE2 )
		MSG_WriteShort( buf, cmd->angles[ 1 ] );
	if( bits & CM_ANGLE3 )
		MSG_WriteShort( buf, cmd->angles[ 2 ] );
	
	if( bits & CM_FORWARD )
		MSG_WriteShort( buf, cmd->forwardmove );
	if( bits & CM_SIDE )
	  	MSG_WriteShort( buf, cmd->sidemove );
	if( bits & CM_UP )
		MSG_WriteShort( buf, cmd->upmove );

 	if( bits & CM_BUTTONS )
	  	MSG_WriteByte( buf, cmd->buttons );
 	if( bits & CM_IMPULSE )
	    MSG_WriteByte( buf, cmd->impulse );

    MSG_WriteByte( buf, cmd->msec );
	MSG_WriteByte( buf, cmd->lightlevel );
}


void Message::MSG_WriteDir( sizebuf_t * sb, Vec3 & dir ) {

	int		i, best;
	float	d, bestd;
	
	if( !dir.Length( ) ) {

		MSG_WriteByte( sb, 0 );
		return;
	}

	bestd = 0;
	best = 0;
	for( i = 0; i<NUMVERTEXNORMALS; i++ )
	{
		d = dir * Common::bytedirs[ i ];
		if( d > bestd )
		{
			bestd = d;
			best = i;
		}
	}
	MSG_WriteByte( sb, best );
}

Vec3 Message::MSG_ReadDir( sizebuf_t * sb ) {

	int		b;

	b = MSG_ReadByte( sb );
	if( b >= NUMVERTEXNORMALS )
		Common::Com_Error( ERR_DROP, "MSF_ReadDir: out of range" );
	return Common::bytedirs[ b ];
}

void Message::MSG_WriteVec( sizebuf_t * sb, Vec3 & vector ) {

	SZ_Write( sb, &vector, sizeof( float ) * 3 );
}

Vec3 Message::MSG_ReadVec( sizebuf_t * sb ) {
	
	Vec3 ret;
	size_t size = sizeof( float ) * 3;
	
	if( sb->readcount + size > sb->cursize ) return vec3_origin;
		
	memcpy( ret.ToFloatPtr( ), &sb->data[ sb->readcount ], size );

	sb->readcount += size;

	return ret;
}


/*
==================
MSG_WriteDeltaEntity

Writes part of a packetentities message.
Can delta from either a baseline or a previous packet_entity
==================
*/
void Message::MSG_WriteDeltaEntity( entity_state_t * from, entity_state_t * to, sizebuf_t * msg, bool force, bool newentity ) {

	if( !to->number ) Common::Com_Error( ERR_FATAL, "Unset entity number" );
	if( to->number >= MAX_EDICTS ) Common::Com_Error( ERR_FATAL, "Entity number >= MAX_EDICTS" );

// send an update
	int bits = 0;

	if( to->number >= 256 ) bits |= U_NUMBER16;		// number8 is implicit otherwise

	if( to->origin[ 0 ] != from->origin[ 0 ] ) bits |= U_ORIGIN1;
	if( to->origin[ 1 ] != from->origin[ 1 ] ) bits |= U_ORIGIN2;
	if( to->origin[ 2 ] != from->origin[ 2 ] ) bits |= U_ORIGIN3;

	if( to->angles[ 0 ] != from->angles[ 0 ] ) bits |= U_ANGLE1;		
	if( to->angles[ 1 ] != from->angles[ 1 ] ) bits |= U_ANGLE2;
	if( to->angles[ 2 ] != from->angles[ 2 ] ) bits |= U_ANGLE3;
		
	if( to->skinnum != from->skinnum ) {

		if( ( unsigned )to->skinnum < 256 )				bits |= U_SKIN8;
		else if( ( unsigned )to->skinnum < 0x10000 )	bits |= U_SKIN16;
		else											bits |=( U_SKIN8 | U_SKIN16 );
	}
		
	if( to->frame != from->frame ) {

		if( to->frame < 256 )				bits |= U_FRAME8;
		else								bits |= U_FRAME16;
	}

	if( to->effects != from->effects ) {

		if		( to->effects < 256 )		bits |= U_EFFECTS8;
		else if	( to->effects < 0x8000 )	bits |= U_EFFECTS16;
		else								bits |= U_EFFECTS8 | U_EFFECTS16;
	}
	
	if( to->renderfx != from->renderfx ) {

		if		( to->renderfx < 256 )		bits |= U_RENDERFX8;
		else if	( to->renderfx < 0x8000 )	bits |= U_RENDERFX16;
		else								bits |= U_RENDERFX8 | U_RENDERFX16;
	}
	
	if( to->solid != from->solid ) bits |= U_SOLID;

	// event is not delta compressed, just 0 compressed
	if( to->eventNum ) bits |= U_EVENT;
	
	if( to->modelindex  != from->modelindex  )	bits |= U_MODEL;
	if( to->modelindex2 != from->modelindex2 )	bits |= U_MODEL2;
	if( to->modelindex3 != from->modelindex3 )	bits |= U_MODEL3;
	if( to->modelindex4 != from->modelindex4 )	bits |= U_MODEL4;

	if( to->sound != from->sound ) bits |= U_SOUND;

	if( newentity ||( to->renderfx & RF_BEAM ) ) bits |= U_OLDORIGIN;

	//
	// write the message
	//
	if( !bits && !force ) return;		// nothing to send!

	//----------

	if		( bits & 0xFF000000 )	bits |= U_MOREBITS3 | U_MOREBITS2 | U_MOREBITS1;
	else if	( bits & 0x00FF0000 )	bits |= U_MOREBITS2 | U_MOREBITS1;
	else if	( bits & 0x0000FF00 )	bits |= U_MOREBITS1;

	MSG_WriteByte( msg, bits & 255 );

	if( bits & 0xFF000000 ) {

		MSG_WriteByte( msg, ( bits >> 8  ) & 255 );
		MSG_WriteByte( msg, ( bits >> 16 ) & 255 );
		MSG_WriteByte( msg, ( bits >> 24 ) & 255 );

	} else if( bits & 0x00FF0000 ) {

		MSG_WriteByte( msg, ( bits >> 8  ) & 255 );
		MSG_WriteByte( msg, ( bits >> 16 ) & 255 );

	} else if( bits & 0x0000FF00 ) {

		MSG_WriteByte( msg, ( bits >> 8  ) & 255 );
	}

	//----------

	if( bits & U_NUMBER16 )	MSG_WriteShort( msg, to->number );
	else					MSG_WriteByte( msg, to->number );

	if( bits & U_MODEL  )	MSG_WriteByte( msg, to->modelindex  );
	if( bits & U_MODEL2 )	MSG_WriteByte( msg, to->modelindex2 );
	if( bits & U_MODEL3 )	MSG_WriteByte( msg, to->modelindex3 );
	if( bits & U_MODEL4 )	MSG_WriteByte( msg, to->modelindex4 );

	if( bits & U_FRAME8  )	MSG_WriteByte( msg, to->frame );
	if( bits & U_FRAME16 )	MSG_WriteShort( msg, to->frame );

	if( ( bits & U_SKIN8 ) &&( bits & U_SKIN16 ) ) MSG_WriteLong( msg, to->skinnum ); //used for laser colors
	else if( bits & U_SKIN8 )
		MSG_WriteByte( msg, to->skinnum );
	else if( bits & U_SKIN16 )
		MSG_WriteShort( msg, to->skinnum );


	if( ( bits &( U_EFFECTS8|U_EFFECTS16 ) ) ==( U_EFFECTS8|U_EFFECTS16 ) )
		MSG_WriteLong( msg, to->effects );
	else if( bits & U_EFFECTS8 )
		MSG_WriteByte( msg, to->effects );
	else if( bits & U_EFFECTS16 )
		MSG_WriteShort( msg, to->effects );

	if( ( bits &( U_RENDERFX8|U_RENDERFX16 ) ) ==( U_RENDERFX8|U_RENDERFX16 ) )
		MSG_WriteLong( msg, to->renderfx );
	else if( bits & U_RENDERFX8 )
		MSG_WriteByte( msg, to->renderfx );
	else if( bits & U_RENDERFX16 )
		MSG_WriteShort( msg, to->renderfx );

	if( bits & U_ORIGIN1 )
		MSG_WriteCoord( msg, to->origin[ 0 ] );		
	if( bits & U_ORIGIN2 )
		MSG_WriteCoord( msg, to->origin[ 1 ] );
	if( bits & U_ORIGIN3 )
		MSG_WriteCoord( msg, to->origin[ 2 ] );

	if( bits & U_ANGLE1 )
		MSG_WriteAngle( msg, to->angles[ 0 ] );
	if( bits & U_ANGLE2 )
		MSG_WriteAngle( msg, to->angles[ 1 ] );
	if( bits & U_ANGLE3 )
		MSG_WriteAngle( msg, to->angles[ 2 ] );

	if( bits & U_OLDORIGIN )
	{
		MSG_WriteCoord( msg, to->old_origin[ 0 ] );
		MSG_WriteCoord( msg, to->old_origin[ 1 ] );
		MSG_WriteCoord( msg, to->old_origin[ 2 ] );
	}

	if( bits & U_SOUND ) MSG_WriteByte( msg, to->sound );
	if( bits & U_EVENT ) MSG_WriteByte( msg, to->eventNum );
	if( bits & U_SOLID ) MSG_WriteShort( msg, to->solid );
}


//============================================================

//
// reading functions
//

void Message::MSG_BeginReading( sizebuf_t * msg ) {

	msg->readcount = 0;
}

// returns -1 if no more characters are available
int Message::MSG_ReadChar( sizebuf_t * msg_read ) {

	int	c;
	
	if( msg_read->readcount+1 > msg_read->cursize )
		c = -1;
	else
		c =( signed char )msg_read->data[ msg_read->readcount ];
	msg_read->readcount++;
	
	return c;
}

int Message::MSG_ReadByte( sizebuf_t * msg_read ) {

	int	c;
	
	if( msg_read->readcount+1 > msg_read->cursize )
		c = -1;
	else
		c =( unsigned char )msg_read->data[ msg_read->readcount ];
	msg_read->readcount++;
	
	return c;
}

int Message::MSG_ReadShort( sizebuf_t * msg_read ) {

	int	c;
	
	if( msg_read->readcount+2 > msg_read->cursize )
		c = -1;
	else		
		c =( short )( msg_read->data[ msg_read->readcount ]
		+( msg_read->data[ msg_read->readcount+1 ]<<8 ) );
	
	msg_read->readcount += 2;
	
	return c;
}

int Message::MSG_ReadLong( sizebuf_t * msg_read ) {

	int	c;
	
	if( msg_read->readcount+4 > msg_read->cursize )
		c = -1;
	else
		c = msg_read->data[ msg_read->readcount ]
		+( msg_read->data[ msg_read->readcount+1 ]<<8 )
		+( msg_read->data[ msg_read->readcount+2 ]<<16 )
		+( msg_read->data[ msg_read->readcount+3 ]<<24 );
	
	msg_read->readcount += 4;
	
	return c;
}

float Message::MSG_ReadFloat( sizebuf_t * msg_read ) {

	union
	{
		byte	b[ 4 ];
		float	f;
		int	l;
	} dat;
	
	if( msg_read->readcount+4 > msg_read->cursize )
		dat.f = -1;
	else
	{
		dat.b[ 0 ] =	msg_read->data[ msg_read->readcount ];
		dat.b[ 1 ] =	msg_read->data[ msg_read->readcount+1 ];
		dat.b[ 2 ] =	msg_read->data[ msg_read->readcount+2 ];
		dat.b[ 3 ] =	msg_read->data[ msg_read->readcount+3 ];
	}
	msg_read->readcount += 4;
	
	dat.l = LittleLong( dat.l );

	return dat.f;
}

const Str Message::MSG_ReadString( sizebuf_t * msg_read ) {

	Str	string;
	int i = 0;
	
	while( 1 ) {

		int c = MSG_ReadChar( msg_read );
		if( c == -1 || c == 0 ) break;
		i++;
		string.Append( c );
	}
	
	if( !i )
		return null_string;
	return string;
}

const Str Message::MSG_ReadStringLine( sizebuf_t * msg_read ) {

	Str	string;
	
	while( 1 ) {

		int c = MSG_ReadChar( msg_read );
		string.Append( c );
		if( c == -1 || c == 0 || c == '\n' ) break;
	}
	
	return string;
}

float Message::MSG_ReadCoord( sizebuf_t * msg_read ) {

	return MSG_ReadShort( msg_read ) * ( 1.0f / 8.0f );
}

Vec3 Message::MSG_ReadPos( sizebuf_t * msg_read ) {

	Vec3 pos;
	pos[ 0 ] = MSG_ReadShort( msg_read ) * ( 1.0f / 8.0f );
	pos[ 1 ] = MSG_ReadShort( msg_read ) * ( 1.0f / 8.0f );
	pos[ 2 ] = MSG_ReadShort( msg_read ) * ( 1.0f / 8.0f );
	return pos;
	//return MSG_ReadVec( msg_read );
}

float Message::MSG_ReadAngle( sizebuf_t * msg_read ) {

	return MSG_ReadChar( msg_read ) * ( 360.0f / 256.0f );
}

float Message::MSG_ReadAngle16( sizebuf_t * msg_read ) {

	return SHORT2ANGLE( Message::MSG_ReadShort( msg_read ) );
}

void Message::MSG_ReadDeltaUsercmd( sizebuf_t * msg_read, usercmd_t * from, usercmd_t * move ) {

	int bits;

	memcpy( move, from, sizeof( usercmd_t ) );

	bits = MSG_ReadByte( msg_read );
		
	// read current angles
	if( bits & CM_ANGLE1 ) move->angles[ 0 ] = MSG_ReadShort( msg_read );
	if( bits & CM_ANGLE2 ) move->angles[ 1 ] = MSG_ReadShort( msg_read );
	if( bits & CM_ANGLE3 ) move->angles[ 2 ] = MSG_ReadShort( msg_read );
		
	// read movement
	if( bits & CM_FORWARD )	move->forwardmove	= MSG_ReadShort( msg_read );
	if( bits & CM_SIDE )	move->sidemove		= MSG_ReadShort( msg_read );
	if( bits & CM_UP )		move->upmove		= MSG_ReadShort( msg_read );
	
	// read buttons
	if( bits & CM_BUTTONS ) move->buttons = MSG_ReadByte( msg_read );

	if( bits & CM_IMPULSE ) move->impulse = MSG_ReadByte( msg_read );

	// read time to run command
	move->msec = MSG_ReadByte( msg_read );

	// read the light level
	move->lightlevel = MSG_ReadByte( msg_read );
}

void Message::MSG_ReadData( sizebuf_t * msg_read, void * data, size_t len ) {

	for( size_t i = 0; i < len; i++ )( ( byte * )data )[ i ] = MSG_ReadByte( msg_read );
}

//===========================================================================

void Message::SZ_Init( sizebuf_t * buf, byte * data, size_t length ) {

	Common::Com_Memset( buf, 0, sizeof( sizebuf_t ) );
	buf->data = data;
	buf->maxsize = length;
}

void Message::SZ_Clear( sizebuf_t * buf ) {

	buf->cursize = 0;
	buf->overflowed = false;
}

void * Message::SZ_GetSpace( sizebuf_t * buf, size_t length ) {

	void * 		data;
	
	if( buf->cursize + length > buf->maxsize ) {

		if( !buf->allowoverflow ) Common::Com_Error( ERR_FATAL, "SZ_GetSpace: overflow without allowoverflow set" );
		
		if( length > buf->maxsize ) Common::Com_Error( ERR_FATAL, "SZ_GetSpace: %i is > full buffer size", length );
			
		Common::Com_Printf( "SZ_GetSpace: overflow\n" );
		SZ_Clear( buf ); 
		buf->overflowed = true;
	}

	data = buf->data + buf->cursize;
	buf->cursize += length;
	
	return data;
}

void Message::SZ_Write( sizebuf_t * buf, const void * data, size_t length ) {

	memcpy( SZ_GetSpace( buf, length ), data, length );
}

void Message::SZ_Print( sizebuf_t * buf, const Str & data ) {

	int		len;
	
	len = data.Length( ) + 1;

	if( buf->cursize ) {

		if( buf->data[ buf->cursize - 1 ] )	memcpy( ( byte * )SZ_GetSpace( buf, len ), 			data, len ); // no trailing 0
		else								memcpy( ( byte * )SZ_GetSpace( buf, len - 1 ) - 1, 	data, len ); // write over trailing 0
	}
	else									memcpy( ( byte * )SZ_GetSpace( buf, len ), 			data, len );
}
#endif

//============================================================================

/*
================
COM_CheckParm

Returns the position( 1 to argc-1 ) in the program's argument list
where the given parameter apears, or 0 if not present
================
*/
int Common::COM_CheckParm( const Str & parm ) {
	
	for( int i = 1; i < com_argc; i++ ) {

		if( parm == com_argv[ i ] ) return i;
	}
		
	return 0;
}

int Common::COM_Argc( ) {

	return com_argc;
}

const Str Common::COM_Argv( int arg ) {

	if( arg < 0 || arg >= com_argc || !com_argv[ arg ] ) return "";
	return com_argv[ arg ];
}

void Common::COM_ClearArgv( int arg ) {

	if( arg < 0 || arg >= com_argc || !com_argv[ arg ] ) return;
	com_argv[ arg ] = "";
}

/*
================
COM_InitArgv
================
*/
void Common::COM_InitArgv( int argc, char ** argv ) {

	if( argc > MAX_NUM_ARGVS ) Common::Com_Error( ERR_FATAL, "argc > MAX_NUM_ARGVS" );

	com_argc = argc;

	for( int i = 0; i<argc; i++ ) {

		if( !argv[ i ] || strlen( argv[ i ] ) >= MAX_TOKEN_CHARS ) com_argv[ i ] = "";
		else com_argv[ i ] = argv[ i ];
	}
}

char * CopyString( char * in ) {

	char	* out;
	
	out =( char * )Z_Malloc( strlen( in )+1 );
	strcpy( out, in );
	return out;
}

/*
==============================================================================

						ZONE MEMORY ALLOCATION

just cleared malloc with counters now...

==============================================================================
*/

#define	Z_MAGIC		0x1d1d


typedef struct zhead_s {

	struct zhead_s	* prev, * next;
	short		magic;
	short		tag;			// for group free
	size_t		size;

} zhead_t;

zhead_t		z_chain;
int			z_count;
size_t		z_bytes;

/*
========================
Z_Free
========================
*/
void Z_Free( void * ptr ) {

	zhead_t	* z;

	z =( ( zhead_t * )ptr ) - 1;

	if( z->magic != Z_MAGIC )
		Common::Com_Error( ERR_FATAL, "Z_Free: bad magic" );

	z->prev->next = z->next;
	z->next->prev = z->prev;

	z_count--;
	z_bytes -= z->size;
	free( z );
}


/*
========================
Z_Stats_f
========================
*/
void Z_Stats_f( ) {

	Common::Com_Printf( "%i bytes in %i blocks\n", z_bytes, z_count );
}

/*
========================
Z_FreeTags
========================
*/
void Z_FreeTags( int tag ) {

	zhead_t	* z, * next;

	for( z = z_chain.next; z != &z_chain; z = next )
	{
		next = z->next;
		if( z->tag == tag )
			Z_Free( ( void * )( z+1 ) );
	}
}

/*
========================
Z_TagMalloc
========================
*/
void * Z_TagMalloc( size_t size, int tag ) {

	zhead_t	* z;
	
	size = size + sizeof( zhead_t );
	z =( zhead_t * )malloc( size );
	if( !z )
		Common::Com_Error( ERR_FATAL, "Z_Malloc: failed on allocation of %i bytes", size );
	Common::Com_Memset( z, 0, size );
	z_count++;
	z_bytes += size;
	z->magic = Z_MAGIC;
	z->tag = tag;
	z->size = size;

	z->next = z_chain.next;
	z->prev = &z_chain;
	z_chain.next->prev = z;
	z_chain.next = z;

	return( void * )( z+1 );
}

/*
========================
Z_Malloc
========================
*/
void * Z_Malloc( size_t size ) {

	return Z_TagMalloc( size, 0 );
}


//============================================================================

int Common::FloatToBits( const float & f, int exponentBits, int mantissaBits ) {
	int i, sign, exponent, mantissa, value;
	assert( exponentBits >= 2 && exponentBits <= 8 );
	assert( mantissaBits >= 2 && mantissaBits <= 23 );
	int maxBits = ( ( ( 1 << ( exponentBits - 1 ) ) - 1 ) << mantissaBits ) | ( ( 1 << mantissaBits ) - 1 );
	int minBits = ( ( ( 1 <<   exponentBits       ) - 2 ) << mantissaBits ) | 1;
	float max = BitsToFloat( maxBits, exponentBits, mantissaBits );
	float min = BitsToFloat( minBits, exponentBits, mantissaBits );
	if( f >= 0.0f ) {
		if( f >= max )
			return maxBits;
		else if( f <= min )
			return minBits;
	} else {
		if( f <= -max )
			return ( maxBits | ( 1 << ( exponentBits + mantissaBits ) ) );
		else if( f >= -min )
			return ( minBits | ( 1 << ( exponentBits + mantissaBits ) ) );
	}
	exponentBits--;
	i = *reinterpret_cast< const int * >( &f );
	sign = ( i >> IEEE_FLT_SIGN_BIT ) & 1;
	exponent = ( ( i >> IEEE_FLT_MANTISSA_BITS ) & ( ( 1 << IEEE_FLT_EXPONENT_BITS ) - 1 ) ) - IEEE_FLT_EXPONENT_BIAS;
	mantissa = i & ( ( 1 << IEEE_FLT_MANTISSA_BITS ) - 1 );
	value = sign << ( 1 + exponentBits + mantissaBits );
	value |= ( ( INTSIGNBITSET( exponent ) << exponentBits ) | ( abs( exponent ) & ( ( 1 << exponentBits ) - 1 ) ) ) << mantissaBits;
	value |= mantissa >> ( IEEE_FLT_MANTISSA_BITS - mantissaBits );
	return value;
}

float Common::BitsToFloat( int i, int exponentBits, int mantissaBits ) {
	static int exponentSign[2] = { 1, -1 };
	int sign, exponent, mantissa, value;
	assert( exponentBits >= 2 && exponentBits <= 8 );
	assert( mantissaBits >= 2 && mantissaBits <= 23 );
	exponentBits--;
	sign = i >> ( 1 + exponentBits + mantissaBits );
	exponent = ( ( i >> mantissaBits ) & ( ( 1 << exponentBits ) - 1 ) ) * exponentSign[( i >> ( exponentBits + mantissaBits ) ) & 1];
	mantissa = ( i & ( ( 1 << mantissaBits ) - 1 ) ) << ( IEEE_FLT_MANTISSA_BITS - mantissaBits );
	value = sign << IEEE_FLT_SIGN_BIT | ( exponent + IEEE_FLT_EXPONENT_BIAS ) << IEEE_FLT_MANTISSA_BITS | mantissa;
	return *reinterpret_cast< float * >( &value );
}

static byte chktbl[ 1024 ] = {
0x84, 0x47, 0x51, 0xC1, 0x93, 0x22, 0x21, 0x24, 0x2F, 0x66, 0x60, 0x4D, 0xB0, 0x7C, 0xDA, 0x88, 0x54, 0x15, 0x2B, 0xC6, 0x6C, 0x89, 0xC5, 0x9D, 0x48, 0xEE, 0xE6, 0x8A, 0xB5, 0xF4, 0xCB, 0xFB, 0xF1, 0x0C, 0x2E, 0xA0, 0xD7, 0xC9, 0x1F, 0xD6, 0x06, 0x9A, 0x09, 0x41, 0x54, 0x67, 0x46, 0xC7, 0x74, 0xE3, 0xC8, 0xB6, 0x5D, 0xA6, 0x36, 0xC4, 0xAB, 0x2C, 0x7E, 0x85, 0xA8, 0xA4, 0xA6, 0x4D, 0x96, 0x19, 0x19, 0x9A, 0xCC, 0xD8, 0xAC, 0x39, 0x5E, 0x3C, 0xF2, 0xF5, 0x5A, 0x72, 0xE5, 0xA9, 0xD1, 0xB3, 0x23, 0x82, 0x6F, 0x29, 0xCB, 0xD1, 0xCC, 0x71, 0xFB, 0xEA, 0x92, 0xEB, 0x1C, 0xCA, 0x4C, 0x70, 0xFE, 0x4D, 0xC9, 0x67, 0x43, 0x47, 0x94, 0xB9, 0x47, 0xBC, 0x3F, 0x01, 0xAB, 0x7B, 0xA6, 0xE2, 0x76, 0xEF, 0x5A, 0x7A, 0x29, 0x0B, 0x51, 0x54, 0x67, 0xD8, 0x1C, 0x14, 0x3E, 0x29, 0xEC, 0xE9, 0x2D, 0x48, 0x67, 0xFF, 0xED, 0x54, 0x4F, 0x48, 0xC0, 0xAA, 0x61, 0xF7, 0x78, 0x12, 0x03, 0x7A, 0x9E, 0x8B, 0xCF, 0x83, 0x7B, 0xAE, 0xCA, 0x7B, 0xD9, 0xE9, 0x53, 0x2A, 0xEB, 0xD2, 0xD8, 0xCD, 0xA3, 0x10, 0x25, 0x78, 0x5A, 0xB5, 0x23, 0x06, 0x93, 0xB7, 0x84, 0xD2, 0xBD, 0x96, 0x75, 0xA5, 0x5E, 0xCF, 0x4E, 0xE9, 0x50, 0xA1, 0xE6, 0x9D, 0xB1, 0xE3, 0x85, 0x66, 0x28, 0x4E, 0x43, 0xDC, 0x6E, 0xBB, 0x33, 0x9E, 0xF3, 0x0D, 0x00, 0xC1, 0xCF, 0x67, 0x34, 0x06, 0x7C, 0x71, 0xE3, 0x63, 0xB7, 0xB7, 0xDF, 0x92, 0xC4, 0xC2, 0x25, 0x5C, 0xFF, 0xC3, 0x6E, 0xFC, 0xAA, 0x1E, 0x2A, 0x48, 0x11, 0x1C, 0x36, 0x68, 0x78, 0x86, 0x79, 0x30, 0xC3, 0xD6, 0xDE, 0xBC, 0x3A, 0x2A, 0x6D, 0x1E, 0x46, 0xDD, 0xE0, 0x80, 0x1E, 0x44, 0x3B, 0x6F, 0xAF, 0x31, 0xDA, 0xA2, 0xBD, 0x77, 0x06, 0x56, 0xC0, 0xB7, 0x92, 0x4B, 0x37, 0xC0, 0xFC, 0xC2, 0xD5, 0xFB, 0xA8, 0xDA, 0xF5, 0x57, 0xA8, 0x18, 0xC0, 0xDF, 0xE7, 0xAA, 0x2A, 0xE0, 0x7C, 0x6F, 0x77, 0xB1, 0x26, 0xBA, 0xF9, 0x2E, 0x1D, 0x16, 0xCB, 0xB8, 0xA2, 0x44, 0xD5, 0x2F, 0x1A, 0x79, 0x74, 0x87, 0x4B, 0x00, 0xC9, 0x4A, 0x3A, 0x65, 0x8F, 0xE6, 0x5D, 0xE5, 0x0A, 0x77, 0xD8, 0x1A, 0x14, 0x41, 0x75, 0xB1, 0xE2, 0x50, 0x2C, 0x93, 0x38, 0x2B, 0x6D, 0xF3, 0xF6, 0xDB, 0x1F, 0xCD, 0xFF, 0x14, 0x70, 0xE7, 0x16, 0xE8, 0x3D, 0xF0, 0xE3, 0xBC, 0x5E, 0xB6, 0x3F, 0xCC, 0x81, 0x24, 0x67, 0xF3, 0x97, 0x3B, 0xFE, 0x3A, 0x96, 0x85, 0xDF, 0xE4, 0x6E, 0x3C, 0x85, 0x05, 0x0E, 0xA3, 0x2B, 0x07, 0xC8, 0xBF, 0xE5, 0x13, 0x82, 0x62, 0x08, 0x61, 0x69, 0x4B, 0x47, 0x62, 0x73, 0x44, 0x64, 0x8E, 0xE2, 0x91, 0xA6, 0x9A, 0xB7, 0xE9, 0x04, 0xB6, 0x54, 0x0C, 0xC5, 0xA9, 0x47, 0xA6, 0xC9, 0x08, 0xFE, 0x4E, 0xA6, 0xCC, 0x8A, 0x5B, 0x90, 0x6F, 0x2B, 0x3F, 0xB6, 0x0A, 0x96, 0xC0, 0x78, 0x58, 0x3C, 0x76, 0x6D, 0x94, 0x1A, 0xE4, 0x4E, 0xB8, 0x38, 0xBB, 0xF5, 0xEB, 0x29, 0xD8, 0xB0, 0xF3, 0x15, 0x1E, 0x99, 0x96, 0x3C, 0x5D, 0x63, 0xD5, 0xB1, 0xAD, 0x52, 0xB8, 0x55, 0x70, 0x75, 0x3E, 0x1A, 0xD5, 0xDA, 0xF6, 0x7A, 0x48, 0x7D, 0x44, 0x41, 0xF9, 0x11, 0xCE, 0xD7, 0xCA, 0xA5, 0x3D, 0x7A, 0x79, 0x7E, 0x7D, 0x25, 0x1B, 0x77, 0xBC, 0xF7, 0xC7, 0x0F, 0x84, 0x95, 0x10, 0x92, 0x67, 0x15, 0x11, 0x5A, 0x5E, 0x41, 0x66, 0x0F, 0x38, 0x03, 0xB2, 0xF1, 0x5D, 0xF8, 0xAB, 0xC0, 0x02, 0x76, 0x84, 0x28, 0xF4, 0x9D, 0x56, 0x46, 0x60, 0x20, 0xDB, 0x68, 0xA7, 0xBB, 0xEE, 0xAC, 0x15, 0x01, 0x2F, 0x20, 0x09, 0xDB, 0xC0, 0x16, 0xA1, 0x89, 0xF9, 0x94, 0x59, 0x00, 0xC1, 0x76, 0xBF, 0xC1, 0x4D, 0x5D, 0x2D, 0xA9, 0x85, 0x2C, 0xD6, 0xD3, 0x14, 0xCC, 0x02, 0xC3, 0xC2, 0xFA, 0x6B, 0xB7, 0xA6, 0xEF, 0xDD, 0x12, 0x26, 0xA4, 0x63, 0xE3, 0x62, 0xBD, 0x56, 0x8A, 0x52, 0x2B, 0xB9, 0xDF, 0x09, 0xBC, 0x0E, 0x97, 0xA9, 0xB0, 0x82, 0x46, 0x08, 0xD5, 0x1A, 0x8E, 0x1B, 0xA7, 0x90, 0x98, 0xB9, 0xBB, 0x3C, 0x17, 0x9A, 0xF2, 0x82, 0xBA, 0x64, 0x0A, 0x7F, 0xCA, 0x5A, 0x8C, 0x7C, 0xD3, 0x79, 0x09, 0x5B, 0x26, 0xBB, 0xBD, 0x25, 0xDF, 0x3D, 0x6F, 0x9A, 0x8F, 0xEE, 0x21, 0x66, 0xB0, 0x8D, 0x84, 0x4C, 0x91, 0x45, 0xD4, 0x77, 0x4F, 0xB3, 0x8C, 0xBC, 0xA8, 0x99, 0xAA, 0x19, 0x53, 0x7C, 0x02, 0x87, 0xBB, 0x0B, 0x7C, 0x1A, 0x2D, 0xDF, 0x48, 0x44, 0x06, 0xD6, 0x7D, 0x0C, 0x2D, 0x35, 0x76, 0xAE, 0xC4, 0x5F, 0x71, 0x85, 0x97, 0xC4, 0x3D, 0xEF, 0x52, 0xBE, 0x00, 0xE4, 0xCD, 0x49, 0xD1, 0xD1, 0x1C, 0x3C, 0xD0, 0x1C, 0x42, 0xAF, 0xD4, 0xBD, 0x58, 0x34, 0x07, 0x32, 0xEE, 0xB9, 0xB5, 0xEA, 0xFF, 0xD7, 0x8C, 0x0D, 0x2E, 0x2F, 0xAF, 0x87, 0xBB, 0xE6, 0x52, 0x71, 0x22, 0xF5, 0x25, 0x17, 0xA1, 0x82, 0x04, 0xC2, 0x4A, 0xBD, 0x57, 0xC6, 0xAB, 0xC8, 0x35, 0x0C, 0x3C, 0xD9, 0xC2, 0x43, 0xDB, 0x27, 0x92, 0xCF, 0xB8, 0x25, 0x60, 0xFA, 0x21, 0x3B, 0x04, 0x52, 0xC8, 0x96, 0xBA, 0x74, 0xE3, 0x67, 0x3E, 0x8E, 0x8D, 0x61, 0x90, 0x92, 0x59, 0xB6, 0x1A, 0x1C, 0x5E, 0x21, 0xC1, 0x65, 0xE5, 0xA6, 0x34, 0x05, 0x6F, 0xC5, 0x60, 0xB1, 0x83, 0xC1, 0xD5, 0xD5, 0xED, 0xD9, 0xC7, 0x11, 0x7B, 0x49, 0x7A, 0xF9, 0xF9, 0x84, 0x47, 0x9B, 0xE2, 0xA5, 0x82, 0xE0, 0xC2, 0x88, 0xD0, 0xB2, 0x58, 0x88, 0x7F, 0x45, 0x09, 0x67, 0x74, 0x61, 0xBF, 0xE6, 0x40, 0xE2, 0x9D, 0xC2, 0x47, 0x05, 0x89, 0xED, 0xCB, 0xBB, 0xB7, 0x27, 0xE7, 0xDC, 0x7A, 0xFD, 0xBF, 0xA8, 0xD0, 0xAA, 0x10, 0x39, 0x3C, 0x20, 0xF0, 0xD3, 0x6E, 0xB1, 0x72, 0xF8, 0xE6, 0x0F, 0xEF, 0x37, 0xE5, 0x09, 0x33, 0x5A, 0x83, 0x43, 0x80, 0x4F, 0x65, 0x2F, 0x7C, 0x8C, 0x6A, 0xA0, 0x82, 0x0C, 0xD4, 0xD4, 0xFA, 0x81, 0x60, 0x3D, 0xDF, 0x06, 0xF1, 0x5F, 0x08, 0x0D, 0x6D, 0x43, 0xF2, 0xE3, 0x11, 0x7D, 0x80, 0x32, 0xC5, 0xFB, 0xC5, 0xD9, 0x27, 0xEC, 0xC6, 0x4E, 0x65, 0x27, 0x76, 0x87, 0xA6, 0xEE, 0xEE, 0xD7, 0x8B, 0xD1, 0xA0, 0x5C, 0xB0, 0x42, 0x13, 0x0E, 0x95, 0x4A, 0xF2, 0x06, 0xC6, 0x43, 0x33, 0xF4, 0xC7, 0xF8, 0xE7, 0x1F, 0xDD, 0xE4, 0x46, 0x4A, 0x70, 0x39, 0x6C, 0xD0, 0xED, 0xCA, 0xBE, 0x60, 0x3B, 0xD1, 0x7B, 0x57, 0x48, 0xE5, 0x3A, 0x79, 0xC1, 0x69, 0x33, 0x53, 0x1B, 0x80, 0xB8, 0x91, 0x7D, 0xB4, 0xF6, 0x17, 0x1A, 0x1D, 0x5A, 0x32, 0xD6, 0xCC, 0x71, 0x29, 0x3F, 0x28, 0xBB, 0xF3, 0x5E, 0x71, 0xB8, 0x43, 0xAF, 0xF8, 0xB9, 0x64, 0xEF, 0xC4, 0xA5, 0x6C, 0x08, 0x53, 0xC7, 0x00, 0x10, 0x39, 0x4F, 0xDD, 0xE4, 0xB6, 0x19, 0x27, 0xFB, 0xB8, 0xF5, 0x32, 0x73, 0xE5, 0xCB, 0x32
};

byte Common::COM_BlockSequenceCRCByte( byte * base, size_t length, int sequence ) {
	int		n;
	byte	* p;
	int		x;
	byte chkb[60 + 4];
	unsigned short crc;
	if( sequence < 0 )
		System::Sys_Error( "sequence < 0, this shouldn't happen\n" );
	p = chktbl +( sequence %( sizeof( chktbl ) - 4 ) );
	if( length > 60 )
		length = 60;
	memcpy( chkb, base, length );
	chkb[ length ] = p[ 0 ];
	chkb[ length+1 ] = p[ 1 ];
	chkb[ length+2 ] = p[ 2 ];
	chkb[ length+3 ] = p[ 3 ];
	length += 4;
	crc = ( unsigned short )CRC32_BlockChecksum( chkb, ( int )length );
	for( x = 0, n = 0; n<length; n++ )
		x += chkb[ n ];
	crc =( crc ^ x ) & 0xFF;
	return ( byte )crc;
}

//========================================================

float	frand( ) {

	return ( rand( ) & 32767 ) * ( 1.0f / 32767.0f );
}

float	crand( ) {

	return ( rand( ) & 32767 ) * ( 2.0f / 32767.0f ) - 1.0f;
}

/*
=============
Common::Com_Error_f

Just throw a fatal error to
test error shutdown procedures
=============
*/
void Common::Com_Error_f( ) {

	Common::Com_Error( ERR_FATAL, "%s", Command::Cmd_Args( ).c_str( ) );
}

void Common::Adjust( ) {

	if( Command::Cmd_Argc( ) != 4 && Command::Cmd_Argc( ) != 2 ) {
		Common::Com_Printf( "usage: adj < int/float cvar > < +-/* > < value >\nusage: adj < bool cvar >\n" );
		return;
	}
	CVar * cVar = CVarSystem::Find( Command::Cmd_Argv( 1 ) );
	if( !cVar ) {
		Common::Com_Printf( "adj: cvar \"%s\" not found\n", Command::Cmd_Argv( 1 ).c_str( ) );
		return;
	}
	if( ( ( cVar->GetFlags( ) & ( CVAR_INT | CVAR_FLOAT | CVAR_STRING ) ) && ( Command::Cmd_Argc( ) != 4 ) ) ||
		( ( cVar->GetFlags( ) & CVAR_BOOL ) && Command::Cmd_Argc( ) != 2 ) ) {
		Common::Com_Printf(  "adj: wrong parameters\n" );
		return;
	}
	if( cVar->GetFlags( ) & CVAR_BOOL ) {
		cVar->SetBool( !cVar->GetBool( ) );
		return;
	}
	if( Command::Cmd_Argv( 2 ).Length( ) != 1 ) {
		Common::Com_Printf( "adj: operator \"%s\" invalid\n", Command::Cmd_Argv( 2 ).c_str( ) );
		return;
	}
	Token token;
	int sLen;
	int sSub;
	if( cVar->GetFlags( ) & CVAR_STRING ) {
		switch( Command::Cmd_Argv( 2 )[ 0 ] ) {
			case '+':
				cVar->SetString( cVar->GetString( ) + Command::Cmd_Argv( 3 ) );
				break;
			case '-':
				sLen = cVar->GetString( ).Length( );
				sSub = token.StrIntValue( Command::Cmd_Argv( 3 ) );
				if( sLen < sSub ) {
					if( !sLen )
						break;
					sSub = sLen;
				}
				cVar->SetString( cVar->GetString( ).Left( sLen - sSub ) );
				break;
			default:
				Common::Com_Printf( "adj: string operator \"%s\" invalid\n", Command::Cmd_Argv( 2 ).c_str( ) );
				return;
		}
		return;
	}
	if( !( cVar->GetFlags( ) & ( CVAR_INT | CVAR_FLOAT ) ) ) {
		Common::Com_Printf( "adj: cvar \"%s\" is not a number\n", Command::Cmd_Argv( 1 ).c_str( ) );
		return;
	}
	float cVal = ( cVar->GetFlags( ) & CVAR_INT ) ? ( float )cVar->GetInt( ) : cVar->GetFloat( );
	float optVal = token.StrFloatValue( Command::Cmd_Argv( 3 ) );
	float outVal;
	switch( Command::Cmd_Argv( 2 )[ 0 ] ) {
		case '+':
			outVal = cVal + optVal;
			break;
		case '-':
			outVal = cVal - optVal;
			break;
		case '/':
			outVal = cVal / optVal;
			break;
		case '*':
			outVal = cVal * optVal;
			break;
		default:
			Common::Com_Printf( "adj: arithmetic operator \"%s\" invalid\n", Command::Cmd_Argv( 2 ).c_str( ) );
			return;
	}
	if( cVar->GetFlags( ) & CVAR_INT )
		cVar->SetInt( ( int )outVal );
	else
		cVar->SetFloat( outVal );
}

void Common::Adjust2( ) {

	if( Command::Cmd_Argc( ) != 4 && Command::Cmd_Argc( ) != 2 ) {
		Adjust( );
		Common::Com_Printf( "adj2 sets the modified flag\n" );
		return;
	}
	Adjust( );
	CVar * cVar = CVarSystem::Find( Command::Cmd_Argv( 1 ) );
	if( cVar )
		cVar->SetModified( );
}

#if 0
template< typename T > class MemberFunctionSlot {
private:
	typedef bool		( T::*MemberFunctionType )( const Str & );
    MemberFunctionType	d_function;
    T &					d_object;

public:

						MemberFunctionSlot( MemberFunctionType func, T & obj ) : d_function( func ),  d_object( obj ) { }

    bool				operator( )( const Str & args ) {
		return ( d_object.*d_function )( args );
    }
};

class DummyTest {
public:
	bool				ACoolFunc( const Str & args ) {
		Common::Com_Printf( args );
		return true;
	}
};
#endif

#if 0
enum {
	TokenEOS,
	TokenCChar,
	TokenNumber,
	TokenName,
	TokenLiteral,
};

class StreamToken : public Str {
private:
	int						d_tokenType;

public:
							StreamToken( int type = TokenEOS );

	int						GetTokenType( ) const;
	void					SetTokenType( int type, bool clear = true );
};

INLINE StreamToken::StreamToken( int type ) {
	d_tokenType = type;
}

INLINE int StreamToken::GetTokenType( ) const {
	return d_tokenType;
}

INLINE void StreamToken::SetTokenType( int type, bool clear ) {
	d_tokenType = type;
	if( clear )
		Clear( );
}

class StreamLexer {
protected:
	StreamToken				d_currentToken;
	StreamToken				d_nextToken;
	Str						d_internalBuffer;
	FileBase *				d_sourceFile;
	int						d_lineNumber;
	bool					d_inputIsUnicode;
	bool					d_streamError;

	int						ReadChar( );
	void					ReadNextToken( );

public:

	const StreamToken &		GetCurrentToken( ) const;
	const StreamToken &		GetNextToken( ) const;
	int						GetLineNumber( ) const;
};

INLINE int StreamLexer::ReadChar( ) {
	int nextChar = 0;
	return ( !d_sourceFile->Read( &nextChar, d_inputIsUnicode ? 2 : 1 ) ) ? 0 : nextChar;
}

void StreamLexer::ReadNextToken( ) {
	int tokenType = TokenEOS, tokenChar = ReadChar( ), tokenCharNext;
	int commentType = 0; // 1 C-style, 2 Cpp-style
	d_nextToken.SetTokenType( TokenEOS );
	while( tokenChar ) {
		tokenCharNext = ReadChar( );
		if( tokenType && commentType ) {
			d_streamError = true;
			return;
		}
		switch( commentType ) {
			case 0:
			if( tokenChar == '/' ) {
				if( tokenCharNext == '/' )
					commentType = 1;
				else if( tokenCharNext == '*' )
					commentType = 2;
			}
			continue;
			case 1:
				if( tokenChar == '\n' )
					commentType = 0;
			continue;
			case 2:
				if( tokenChar == '*' && tokenCharNext == '/' )
					commentType = 3;
			continue;
		}
		if( tokenChar >= '0' && tokenChar <= '9' ) {
			tokenType = TokenNumber;
		} else if( tokenChar >= 'a' && tokenChar <= 'z' ) {
			tokenType = TokenName;
		} else if( tokenChar >= 'A' && tokenChar <= 'Z' ) {
			tokenType = TokenName;
		} else if( tokenChar == '"' ) {
			tokenType = TokenLiteral;
		} else {
			tokenType = TokenCChar;
		}
		if( d_nextToken.GetTokenType( ) && tokenType != d_nextToken.GetTokenType( ) ) {
			return;
		}
		if( tokenType ) {
			if( d_nextToken.IsEmpty( ) )
				d_nextToken.SetTokenType( tokenType, false );
			d_nextToken.Append( tokenChar );
		}
		tokenChar = tokenCharNext;
	}
}
#endif

// // //// // //// // //// //
/// SR3_StateMap
//
/// A SR3_StateMap maps a secondary context to a probability. Creates a StateMap with n contexts using 4*n bytes memory.
// //// // //// // //// //
class SR3_StateMap {
private:
	List< uint >	d_context;	// cxt -> prediction in high 25 bits, count in low 7 bits
	//int				dt[ 128 ];	// i -> 1K/(i+2)
	int				GetDT( int n ) const;

public:
					SR3_StateMap( int n );

	void			Clear( );
	/// predicts next bit (0..4K-1) in context cxt (0..n-1)
	int				Predict( int cxt ) const;
	///updates model for actual bit y (0..1) in cxt (0..n-1)
	void			Update( int cxt, int y );
};

SR3_StateMap::SR3_StateMap( int n ) {
	d_context.SetNum( n );
	//for( int i = 0; i < 128; i++ )
	//	dt[ i ] = 512 / ( i + 2 );
	Clear( );
}

INLINE int SR3_StateMap::GetDT( int n ) const {
	return 512 / ( n + 2 );
}

INLINE void SR3_StateMap::Clear( ) {
	for( int i = 0; i < d_context.Num( ); i++ )
		d_context[ i ] = 1 << 31;
}

INLINE int SR3_StateMap::Predict( int cxt ) const {
	assert( cxt >= 0 && cxt < d_context.Num( ) );
	return d_context[ cxt ] >> 20;
}

INLINE void SR3_StateMap::Update( int cxt, int y ) {
	assert( cxt >= 0 && cxt < d_context.Num( ) );
	assert( y == 0 || y == 1 );
	int n = d_context[ cxt ] & 127, p = d_context[ cxt ] >> 9;  // count, prediction
	if( n < 127 )
		d_context[ cxt ]++;
	d_context[ cxt ] += ( ( y << 23 ) - p ) * GetDT( n ) & 0xffffff80;
}

class SR3_Hash {
private:
	uint		d_hashValue;
public:
				SR3_Hash( ) : d_hashValue( 0 )	{ }
	uint &	operator =( const uint & v )		{ d_hashValue = v; return d_hashValue; }
	operator uint &( )							{ return d_hashValue; }
	operator const uint &( ) const				{ return d_hashValue; }
};

#define SR3_CONTEXT_NUM ( ( 1024 + 1024 ) * 258 )
#define SR3_ENCODER_BUFF_SIZE 0x1000000
#define SR3_ENCODER_BUFF_MASK ( SR3_ENCODER_BUFF_SIZE - 1 )
// // //// // //// // //// //
/// SR3_EncoderBase
//
/// An Encoder does arithmetic encoding in n contexts. Creates encoder for compression to archive f,
/// which must be open past any header for writing in binary mode.
// //// // //// // //// //
class SR3_EncoderBase {
private:
	SR3_StateMap	d_sm;			// cxt -> p
	const int		d_num;			// number of contexts
	//List< uint >	d_encoderContext;
	KeyMap< SR3_Hash, ( SR3_ENCODER_BUFF_SIZE >> 9 ) >	d_encoderContext;
	int				d_c;
	int				d_cxt;
	int				d_channel;
	uint			d_r;
	uint			d_x1, d_x2;		// Range, initially [0, 1), scaled by 2^32
	uint			d_hc[ 16 ];

protected:
	virtual void	PutByte( int c ) = 0;

					SR3_EncoderBase( int n = SR3_CONTEXT_NUM );
	/// compresses bit y (0 or 1) to output modeled in context cxt (0..n-1)
	void			EncodeBit( int cxt, int y );
	/// Encode a byte c in context cxt to encoder e
	void			EncodeByte( int cxt, int c );

public:
	/// call this when compression is finished
	void			Flush( );
	virtual int		GetOutputSize( ) const = 0;
	int				CompressChunk( const void * ptr, int size );
	bool			CompressFile( FileBase * input );
};

class SR3_Encoder : public SR3_EncoderBase {
private:
	CAList< byte, 1024 >	d_buffer;
	virtual void			PutByte( int c );
	virtual int				GetOutputSize( ) const;

public:
							SR3_Encoder( );
};

SR3_Encoder::SR3_Encoder( ) {
	d_buffer.SetGranularity( 1024 );
}

void SR3_Encoder::PutByte( int c ) {
	d_buffer.Append( c );
}

int SR3_Encoder::GetOutputSize( ) const {
	return d_buffer.Num( );
}

SR3_EncoderBase::SR3_EncoderBase( int n ) : d_num( n ), d_x1( 0 ), d_x2( 0xffffffff ), d_sm( n ) {
	//d_encoderContext.AssureSize( SR3_ENCODER_BUFF_SIZE, 0 );
	d_c = 0;
	d_cxt = 0;
	d_channel = 0;
	d_r = 0;
	for( int i = 0; i < 16; i++ )
		d_hc[ i ] = 0;
}

void SR3_EncoderBase::EncodeBit( int cxt, int y ) {
	assert( y == 0 || y == 1 );
	assert( cxt >= 0 && cxt < d_num );
	int p = d_sm.Predict( cxt );
	assert( p >= 0 && p < 4096 );
	uint xmid = d_x1 + ( ( d_x2 - d_x1 ) >> 12 ) * p;
	assert( xmid >= d_x1 && xmid < d_x2 );
	y ? ( d_x2 = xmid ) : ( d_x1 = xmid + 1 );
	d_sm.Update( cxt, y );
	while( ( ( d_x1 ^ d_x2 ) & 0xff000000 ) == 0 ) {  // pass equal leading bytes of range
		PutByte( d_x2 >> 24 );
		d_x1 <<= 8;
		d_x2 = ( d_x2 << 8 ) + 255;
	}
}

void SR3_EncoderBase::EncodeByte( int cxt, int c ) {
	// code high 4 bits in contexts cxt+1..15
	int b = ( c >> 4 ) + 16;
	EncodeBit( cxt + 1, b >> 3 & 1 );
	EncodeBit( cxt + ( b >> 3 ), b >> 2 & 1 );
	EncodeBit( cxt + ( b >> 2 ), b >> 1 & 1);
	EncodeBit( cxt + ( b >> 1 ), b & 1);
	// code low 4 bits in one of 16 blocks of 15 cxts (to reduce cache misses)
	cxt += 15 * ( b - 15 );
	b = c & 15 | 16;
	EncodeBit( cxt + 1, b >> 3 & 1 );
	EncodeBit( cxt + ( b >> 3 ), b >> 2 & 1 );
	EncodeBit( cxt + ( b >> 2 ), b >> 1 & 1);
	EncodeBit( cxt + ( b >> 1 ), b & 1);
}

void SR3_EncoderBase::Flush( ) {
	EncodeBit( d_cxt, 1 );
	EncodeBit( d_cxt + 1, 0 );
	EncodeByte( d_cxt + 2, d_r & 0xff );
	PutByte( d_x1 >> 24 );  // Flush first unequal byte of range
	d_encoderContext.Clear( );
}

bool SR3_EncoderBase::CompressFile( FileBase * input ) {
	const int cshft = 20; // Bit shift for context
	const int hmult = 5 << 5;       // Multiplier for hashes
	const int maxc = 1;       // Number of channels
	int channel = 0;                          // Channel for WAV files
	//List< uint > t4;  // context -> last 3 bytes in bits 0..23, count in 24..29
	//uint hc[ 16 ];                     // Hash of last 4 bytes by channel
	//t4.AssureSize( 0x1000000, 0 );
	while( 1 ) {
		const uint h = d_hc[ channel ];
		d_r = d_encoderContext[ h ];  // last byte count, last 3 bytes in this context
		if( d_r >= 0x4000000 )
			d_cxt = 1024 + ( d_r >> cshft );
		else
			d_cxt = d_c | d_r >> 16 & 0x3f00;
		d_cxt *= 258;
		int c = 0;
		if( !input->Read( &c, 1 ) )  // Mark EOF by coding first match as a literal
			break;
		int comp3 = c * 0x10101 ^ d_r;  // bytes 0, 1, 2 are 0 if equal
		if( !( comp3 & 0xff ) ) {  // match first?
			EncodeBit( d_cxt, 0 );
			if( d_r < 0x3f000000 )
				d_encoderContext[ h ] += 0x1000000;  // increment count
		} else if( !( comp3 & 0xff00 ) ) {  // match second?
			EncodeBit( d_cxt, 1 );
			EncodeBit( d_cxt + 1, 1 );
			EncodeBit( d_cxt + 2, 0 );
			d_encoderContext[ h ] = d_r & 0xff0000 | d_r << 8 & 0xff00 | c | 0x1000000;
		} else if( !( comp3 & 0xff0000 ) ) {  // match third?
			EncodeBit( d_cxt, 1 );
			EncodeBit( d_cxt + 1, 1 );
			EncodeBit( d_cxt + 2, 1 );
			d_encoderContext[ h ] = d_r << 8 & 0xffff00 | c | 0x1000000;
		} else {  // literal?
			EncodeBit( d_cxt, 1 );
			EncodeBit( d_cxt + 1, 0 );
			EncodeByte( d_cxt + 2, c );
			d_encoderContext[ h ] = d_r << 8 & 0xffff00 | c;
		}
		d_c = c;
		d_hc[ channel ] = ( h * hmult + c + 1 ) & SR3_ENCODER_BUFF_MASK;
		if( ++channel >= maxc )
			channel = 0;
	}
	Flush( );
	return true;
}

// Compress from in to out.  out should be positioned past the header.
int SR3_EncoderBase::CompressChunk( const void * ptr, int size ) {
	const int cshft = 20; // Bit shift for context
	const int hmult = 5 << 5;       // Multiplier for hashes
	const int maxc = 1;       // Number of channels
	//int channel = 0;                          // Channel for WAV files
	//List< uint > t4;  // context -> last 3 bytes in bits 0..23, count in 24..29
	//uint hc[ 16 ];                     // Hash of last 4 bytes by channel
	//t4.AssureSize( 0x1000000, 0 );
	if( !ptr || size <= 0 )
		return 0;
	int startOffset = GetOutputSize( );
	for( int i = 0; i < size; i++ ) {
		const uint h = d_hc[ d_channel ];
		d_r = d_encoderContext[ h ];  // last byte count, last 3 bytes in this context
		if( d_r >= 0x4000000 )
			d_cxt = 1024 + ( d_r >> cshft );
		else
			d_cxt = d_c | d_r >> 16 & 0x3f00;
		d_cxt *= 258;
		d_c = ( int )( ( byte * )ptr )[ i ];
		int comp3 = d_c * 0x10101 ^ d_r;  // bytes 0, 1, 2 are 0 if equal
		if( !( comp3 & 0xff ) ) {  // match first?
			EncodeBit( d_cxt, 0 );
			if( d_r < 0x3f000000 )
				d_encoderContext[ h ] += 0x1000000;  // increment count
		} else if( !( comp3 & 0xff00 ) ) {  // match second?
			EncodeBit( d_cxt, 1 );
			EncodeBit( d_cxt + 1, 1 );
			EncodeBit( d_cxt + 2, 0 );
			d_encoderContext[ h ] = d_r & 0xff0000 | d_r << 8 & 0xff00 | d_c | 0x1000000;
		} else if( !( comp3 & 0xff0000 ) ) {  // match third?
			EncodeBit( d_cxt, 1 );
			EncodeBit( d_cxt + 1, 1 );
			EncodeBit( d_cxt + 2, 1 );
			d_encoderContext[ h ] = d_r << 8 & 0xffff00 | d_c | 0x1000000;
		} else {  // literal?
			EncodeBit( d_cxt, 1 );
			EncodeBit( d_cxt + 1, 0 );
			EncodeByte( d_cxt + 2, d_c );
			d_encoderContext[ h ] = d_r << 8 & 0xffff00 | d_c;
		}
		d_hc[ d_channel ] = ( h * hmult + d_c + 1 ) & SR3_ENCODER_BUFF_MASK;
		if( ++d_channel >= maxc )
			d_channel = 0;
	}
	return GetOutputSize( ) - startOffset;
}

// // //// // //// // //// //
/// SR3_DecoderBase
//
/// A Decoder does arithmetic decoding in n contexts.  Methods:
/// Decoder(f, n) creates decoder for decompression from archive f,
///     which must be open past any header for reading in binary mode.
/// code(cxt) returns the next decompressed bit from file f
///   with context cxt in 0..n-1.
// //// // //// // //// //
class SR3_DecoderBase {
private:
	const int		d_num;     // number of contexts
	uint			d_x1, d_x2;      // Range, initially [0, 1), scaled by 2^32
	uint			d_x;           // Decompress mode: last 4 input bytes of archive
	SR3_StateMap	d_sm;     // cxt -> p

	virtual int		GetByte( ) = 0;

public:
					//SR3_DecoderBase( int n = SR3_CONTEXT_NUM );
	/// Return decompressed bit (0..1) in context cxt (0..n-1)
	int				DecodeBit( int cxt );
	/// Decode one byte
	int				DecodeByte( int cxt );
};

//SR3_DecoderBase::SR3_DecoderBase( int n ): d_num( n ), d_x1( 0 ), d_x2( 0xffffffff ), d_x( 0 ), d_sm( n ) {
//	for( int i = 0; i < 4; i++ )
//		d_x = ( d_x << 8 ) + ( GetByte( ) & 255 );
//}

int SR3_DecoderBase::DecodeBit( int cxt ) {
	assert( cxt >= 0 && cxt < d_num );
	int p = d_sm.Predict( cxt );
	assert( p >= 0 && p < 4096 );
	uint xmid = d_x1 + ( ( d_x2 - d_x1 ) >> 12 ) * p;
	assert( xmid >= d_x1 && xmid < d_x2 );
	int y = d_x <= xmid;
	y ? ( d_x2 = xmid ) : ( d_x1 = xmid + 1 );
	d_sm.Update( cxt, y );
	while( ( ( d_x1 ^ d_x2 ) & 0xff000000 ) == 0 ) {  // pass equal leading bytes of range
		d_x1 <<= 8;
		d_x2 = ( d_x2 << 8 ) + 255;
		d_x = ( d_x << 8 ) + ( GetByte( ) & 255 );  // EOF is OK
	}
	return y;
}

int SR3_DecoderBase::DecodeByte( int cxt ) {
	int hi = 1, lo = 1;  // high and low nibbles
	hi += hi + DecodeBit( cxt + hi );
	hi += hi + DecodeBit( cxt + hi );
	hi += hi + DecodeBit( cxt + hi );
	hi += hi + DecodeBit( cxt + hi );
	cxt += 15 * ( hi - 15 );
	lo += lo + DecodeBit( cxt + lo );
	lo += lo + DecodeBit( cxt + lo );
	lo += lo + DecodeBit( cxt + lo );
	lo += lo + DecodeBit( cxt + lo );
	return ( hi - 16 ) << 4 | ( lo - 16 );
}
#if 0
// ----------------------------------------------------------------------------
// Decompress from in to out.  in should be positioned past the header.
void decompress( FILE * in, FILE * out ) {
	Decoder e( in, ( 1024 + 1024 ) * 258 );
	const int cshft = 20; // Bit shift for context
	const int hmask = 0xffffff;  // Hash mask
	const int hmult = 5 << 5;       // Multiplier for hashes
	const int maxc = 1;       // Number of channels
	int channel = 0;                          // Channel for WAV files
	int c1 = 0; // previous byte
	List< uint > t4;  // context -> last 3 bytes in bits 0..23, count in 24..29
	uint hc[ 16 ];                     // Hash of last 4 bytes by channel
	t4.AssureSize( 0x1000000, 0 );
	while( 1 ) {
		const uint h = hc [channel ];
		const uint r = t4[ h ];  // last byte count, last 3 bytes in this context
		int cxt;  // context
		if( r >= 0x4000000 )
			cxt = 1024 + ( r >> cshft );
		else
			cxt = c1 | r >> 16 & 0x3f00;
		cxt *= 258;
		// Decompress: 0=p[1], 110=p[2], 111=p[3], 10xxxxxxxx=literal.
		// EOF is marked by p[1] coded as a literal.
		if( e.code( cxt ) ) {
			if( e.code( cxt + 1 ) ) {
				if( e.code( cxt + 2 ) ) {  // match third?
					c1 = r >> 16 & 0xff;
					t4[ h ] = r << 8 & 0xffff00 | c1 | 0x1000000;
				} else {  // match second?
					c1 = r >> 8 & 0xff;
					t4[ h ] = r & 0xff0000 | r << 8 & 0xff00 | c1 | 0x1000000;
				}
			} else {  // literal?
				c1 = decode( e, cxt + 2 );
				if( c1 == int( r & 0xff ) )
					break;  // EOF?
				t4[ h ] = r << 8 & 0xffff00 | c1;
			}
		} else {  // match first?
			c1 = r & 0xff;
			if( r < 0x3f000000 )
				t4[ h ] += 0x1000000;  // increment count
		}
		putc( c1, out );
		hc[ channel ] = ( h * hmult + c1 + 1 ) & hmask;
		if( ++channel >= maxc )
			channel=0;
	}
}
#endif
void Common::Init( int argc, char ** argv ) {

	/*Winding2D tw2;
	tw2.AddPoint( Vec2(0,0) );
	tw2.AddPoint( Vec2(0,256) );
	tw2.AddPoint( Vec2(512,256) );
	tw2.AddPoint( Vec2(512,0) );
	Winding2D tw23( tw2 );
	tw2.Print( );
	Plane2D tp;
	tp.FromPoints( Vec2(512,256), Vec2(0,0), true );
	tw2.ClipInPlace( tp );
	tw2.Print( );
	tp.FromPoints( Vec2(0,0), Vec2(512,256), true );
	tw23.ClipInPlace( tp );
	tw23.Print( );*/

	//DummyTest dmy;
	//MemberFunctionSlot< DummyTest > mfs( &DummyTest::ACoolFunc, dmy );

	FileSystem::Init( );
	LuaScript::Init( );

	File_Memory fm = FileSystem::ReadFile( "maps/perspective.map" );
	File_Memory fm2 = FileSystem::ReadFile( "maps/perspective0.map" );
	Str jingleBells = "abcdefgh abcdefg abcdef abcde abcd abc ab a";
	SR3_Encoder sr3Enc;
	sr3Enc.CompressChunk( jingleBells.c_str( ), jingleBells.Length( ) + 1 );
	sr3Enc.CompressChunk( jingleBells.c_str( ), jingleBells.Length( ) + 1 );
	sr3Enc.CompressChunk( fm.GetDataPtr( ), fm.Length( ) );
	sr3Enc.CompressChunk( jingleBells.c_str( ), jingleBells.Length( ) + 1 );
	sr3Enc.CompressChunk( jingleBells.c_str( ), jingleBells.Length( ) + 1 );
	sr3Enc.CompressChunk( fm2.GetDataPtr( ), fm2.Length( ) );
	sr3Enc.Flush( );
	FileSystem::FreeFile( fm );
	FileSystem::FreeFile( fm2 );

	if( setjmp( abortframe ) )
		System::Sys_Error( "Error during initialization" );

	z_chain.next = z_chain.prev = &z_chain;

	// prepare enough of the subsystems to handle
	// cvar and command buffer management
	Common::COM_InitArgv( argc, argv );

	Swap_Init( );
	CBuffer::Cbuf_Init( );

	Command::Cmd_Init( );
	CVarSystem::Init( );
	FileSystem::Register( );
	KeyEvent::Init( );

#ifndef DEDICATED_ONLY
	if( !dedicated.GetBool( ) ) {
		Keyboard::Key_Init( );
	}
#endif

	// we need to add the early commands twice, because
	// a basedir or cddir needs to be set before execing
	// config files, but we want other parms to override
	// the settings of the config files
#if 0
	CBuffer::Cbuf_AddEarlyCommands( true );
	CBuffer::Cbuf_Execute( );

	CBuffer::Cbuf_AddText( "exec default.cfg\n" );
	CBuffer::Cbuf_AddText( "exec config.cfg\n" );

	CBuffer::Cbuf_AddEarlyCommands( true );
	CBuffer::Cbuf_Execute( );
#endif

	if( !dedicated.GetBool( ) ) {
		LuaScript::ExecuteFile( "default.lua" );
		LuaScript::ExecuteFile( "config.lua" );
	} else {
		LuaScript::ExecuteFile( "dedicated.lua" );
	}

	CBuffer::Cbuf_AddEarlyCommands( true );
	CBuffer::Cbuf_Execute( );

	//
	// init commands and vars
	//
	Command::Cmd_AddCommand( "z_stats", Z_Stats_f );
	Command::Cmd_AddCommand( "error", Com_Error_f );
	Command::Cmd_AddCommand( "adj", Adjust );
	Command::Cmd_AddCommand( "adj2", Adjust2 );

	//Str s = va( "%4.2f %s %s %s", VERSION, CPUSTRING, __DATE__, BUILDSTRING );
	//Cvar_Get( "version", s, CVAR_SERVERINFO|CVAR_NOSET );

	if( dedicated.GetBool( ) )
		Command::Cmd_AddCommand( "quit", Common::Com_Quit );

	System::Sys_Init( );

	Network::NET_Init( );
	//Netchan::Netchan_Init( );

	Dict::Init( );
	Server::SV_Init( );
#ifndef DEDICATED_ONLY
	if( !dedicated.GetBool( ) )
		Client::CL_Init( );
#endif
	// add + commands from command line
	if( !CBuffer::Cbuf_AddLateCommands( ) ) {	// if the user didn't give any commands, run default action

		//if( !dedicated.GetBool( ) ) CBuffer::Cbuf_AddText( Str( "map perspective\n" ) );
		if( !dedicated.GetBool( ) )
			LuaScript::ExecuteInline( "DefaultAction()" );
		else {			
			//CBuffer::Cbuf_AddText( Str( "dedicated_start\n" ) );
			CBuffer::Cbuf_Execute( );
		}
	} else {	// the user asked for something explicit
		// so drop the loading plaque
#ifndef DEDICATED_ONLY
		if( !dedicated.GetBool( ) )
			Screen::SCR_EndLoadingPlaque( );
#endif
	}
	//Common::Com_Printf( "Game system initialized\n" );	
}

void Common::Frame( timeType msec ) {
	if( setjmp( abortframe ) )
		return; // an ERR_DROP was thrown
	if( fixedtime.GetBool( ) )
		msec = ( timeType )fixedtime.GetInt( );
	else {
		msec = ( timeType )( ( float )msec * timescale.GetFloat( ) );
		if( msec < 1 )
			msec = 1;
	}
	if( System::Sys_HaveConsole( ) )
		CBuffer::Cbuf_AddText( System::Sys_InputConsole( ) );
	CBuffer::Cbuf_Execute( );
	Server::SV_Frame( msec );
#ifndef DEDICATED_ONLY
	if( !dedicated.GetBool( ) )
		Client::CL_Frame( msec );
#else
	System::Sys_SendKeyEvents( );
#endif
}

void Common::Shutdown( ) {

	Dict::Shutdown( );
	KeyEvent::Shutdown( );
	LuaScript::Shutdown( );
	FileSystem::Shutdown( );
}
