// qcommon.h -- definitions common between client and server, but not game.dll

//============================================================================

typedef struct sizebuf_s {

	bool	allowoverflow;	// if false, do a Com_Error
	bool	overflowed;		// set to true if the buffer size failed
	byte	* data;
	size_t		maxsize;
	size_t		cursize;
	size_t		readcount;
} sizebuf_t;

//============================================================================

struct usercmd_s;
struct entity_state_s;

#if 0
class Message {

public:

	static void			SZ_Init( sizebuf_t * buf, byte * data, size_t length );
	static void			SZ_Clear( sizebuf_t * buf );
	static void *		SZ_GetSpace( sizebuf_t * buf, size_t length );
	static void			SZ_Write( sizebuf_t * buf, const void * data, size_t length );
	static void			SZ_Print( sizebuf_t * buf, const Str & data );	// strcats onto the sizebuf

	static void			MSG_WriteChar( sizebuf_t * sb, int c );
	static void			MSG_WriteByte( sizebuf_t * sb, int c );
	static void			MSG_WriteShort( sizebuf_t * sb, int c );
	static void			MSG_WriteLong( sizebuf_t * sb, int c );
	static void			MSG_WriteFloat( sizebuf_t * sb, float f );
	static void			MSG_WriteString( sizebuf_t * sb, const Str & str );
	static void			MSG_WriteCoord( sizebuf_t * sb, float f );
	static void			MSG_WritePos( sizebuf_t * sb, Vec3 & pos );
	static void			MSG_WriteAngle( sizebuf_t * sb, float f );
	static void			MSG_WriteAngle16( sizebuf_t * sb, float f );
	static void			MSG_WriteDeltaUsercmd( sizebuf_t * sb, struct usercmd_s * from, struct usercmd_s * cmd );
	static void			MSG_WriteDeltaEntity( struct entity_state_s * from, struct entity_state_s * to, sizebuf_t * msg, bool force, bool newentity );
	static void			MSG_WriteDir( sizebuf_t * sb, Vec3 & vector );

	static void			MSG_WriteVec( sizebuf_t * sb, Vec3 & vector );

	static void			MSG_BeginReading( sizebuf_t * sb );

	static int			MSG_ReadChar( sizebuf_t * sb );
	static int			MSG_ReadByte( sizebuf_t * sb );
	static int			MSG_ReadShort( sizebuf_t * sb );
	static int			MSG_ReadLong( sizebuf_t * sb );
	static float		MSG_ReadFloat( sizebuf_t * sb );
	static const Str	MSG_ReadString( sizebuf_t * sb );
	static const Str	MSG_ReadStringLine( sizebuf_t * sb );

	static float		MSG_ReadCoord( sizebuf_t * sb );
	static Vec3			MSG_ReadPos( sizebuf_t * sb );
	static float		MSG_ReadAngle( sizebuf_t * sb );
	static float		MSG_ReadAngle16( sizebuf_t * sb );
	static void			MSG_ReadDeltaUsercmd( sizebuf_t * sb, struct usercmd_s * from, struct usercmd_s * cmd );

	static Vec3			MSG_ReadDir( sizebuf_t * sb );

	static void			MSG_ReadData( sizebuf_t * sb, void * buffer, size_t size );

	static Vec3			MSG_ReadVec( sizebuf_t * sb );
};
#endif
//============================================================================

extern	bool		bigendien;

extern	short	BigShort( short l );
extern	short	LittleShort( short l );
extern	int		BigLong( int l );
extern	int		LittleLong( int l );
extern	float	BigFloat( float l );
extern	float	LittleFloat( float l );

//============================================================================

char * CopyString( char * in );

//============================================================================

/*
==============================================================

PROTOCOL

==============================================================
*/

// protocol.h -- communications protocols

#define	PROTOCOL_VERSION	2

//=========================================

#define	PORT_MASTER	25240
#define	PORT_CLIENT	25251
#define	PORT_SERVER	25250

//=========================================

#define	UPDATE_BACKUP	16	// copies of entity_state_t to keep buffered
							// must be power of two
#define	UPDATE_MASK		( UPDATE_BACKUP-1 )



//==================
// the svc_strings[] array in cl_parse.c should mirror this
//==================

//
// server to client
//
enum svc_ops_e {

	svc_bad,					// these ops are known to the game dll
	svc_muzzleflash,
	svc_muzzleflash2,
	svc_temp_entity,
	svc_layout,
	svc_inventory,				// the rest are private to the client and server
	svc_nop,
	svc_disconnect,
	svc_reconnect,
	svc_sound, 					// <see code>
	svc_print, 					// [ byte ]  [ string ] null terminated string
	svc_stufftext, 				// [ string ] stuffed into client's console buffer, should be \n terminated
	svc_serverdata, 			// [ long ] protocol ...
	svc_configstring, 			// [ short ] [ string ]
	svc_spawnbaseline, 		
	svc_centerprint, 			// [ string ] to put in center of the screen
	svc_download, 				// [ short ] size [size bytes]
	svc_playerinfo, 			// variable
	svc_packetentities, 		// [ ... ]
	svc_deltapacketentities, 	// [ ... ]
	svc_frame

};

//==============================================

//
// client to server
//
enum clc_ops_e {

	clc_bad,
	clc_nop,		
	clc_move, 				// [ [usercmd_t ]
	clc_userinfo, 			// [[userinfo string]
	clc_stringcmd			// [ string ] message
};

//==============================================

// plyer_state_t communication

#define	PS_M_TYPE			( 1<<0 )
#define	PS_M_ORIGIN			( 1<<1 )
#define	PS_M_VELOCITY		( 1<<2 )
#define	PS_M_TIME			( 1<<3 )
#define	PS_M_FLAGS			( 1<<4 )
#define	PS_M_GRAVITY		( 1<<5 )
#define	PS_M_DELTA_ANGLES	( 1<<6 )

#define	PS_VIEWOFFSET		( 1<<7 )
#define	PS_VIEWANGLES		( 1<<8 )
#define	PS_KICKANGLES		( 1<<9 )
#define	PS_BLEND			( 1<<10 )
#define	PS_FOV				( 1<<11 )
#define	PS_WEAPONINDEX		( 1<<12 )
#define	PS_WEAPONFRAME		( 1<<13 )
#define	PS_RDFLAGS			( 1<<14 )

//==============================================

// user_cmd_t communication

// ms and light always sent, the others are optional
#define	CM_ANGLE1 	( 1<<0 )
#define	CM_ANGLE2 	( 1<<1 )
#define	CM_ANGLE3 	( 1<<2 )
#define	CM_FORWARD	( 1<<3 )
#define	CM_SIDE		( 1<<4 )
#define	CM_UP		( 1<<5 )
#define	CM_BUTTONS	( 1<<6 )
#define	CM_IMPULSE	( 1<<7 )

//==============================================

// a sound without an ent or pos will be a local only sound
#define	SND_VOLUME		( 1<<0 )		// a byte
#define	SND_ATTENUATION	( 1<<1 )		// a byte
#define	SND_POS			( 1<<2 )		// three coordinates
#define	SND_ENT			( 1<<3 )		// a short 0-2: channel, 3-12: entity
#define	SND_OFFSET		( 1<<4 )		// a byte, msec offset from frame start

#define DEFAULT_SOUND_PACKET_VOLUME	1.0f
#define DEFAULT_SOUND_PACKET_ATTENUATION 1.0f

//==============================================

// entity_state_t communication

// try to pack the common update flags into the first byte
#define	U_ORIGIN1	( 1<<0 )
#define	U_ORIGIN2	( 1<<1 )
#define	U_ANGLE2	( 1<<2 )
#define	U_ANGLE3	( 1<<3 )
#define	U_FRAME8	( 1<<4 )		// frame is a byte
#define	U_EVENT		( 1<<5 )
#define	U_REMOVE	( 1<<6 )		// REMOVE this entity, don't add it
#define	U_MOREBITS1	( 1<<7 )		// read one additional byte

// second byte
#define	U_NUMBER16	( 1<<8 )		// NUMBER8 is implicit if not set
#define	U_ORIGIN3	( 1<<9 )
#define	U_ANGLE1	( 1<<10 )
#define	U_MODEL		( 1<<11 )
#define U_RENDERFX8	( 1<<12 )		// fullbright, etc
#define	U_EFFECTS8	( 1<<14 )		// autorotate, trails, etc
#define	U_MOREBITS2	( 1<<15 )		// read one additional byte

// third byte
#define	U_SKIN8		( 1<<16 )
#define	U_FRAME16	( 1<<17 )		// frame is a short
#define	U_RENDERFX16 ( 1<<18 )	// 8 + 16 = 32
#define	U_EFFECTS16	( 1<<19 )		// 8 + 16 = 32
#define	U_MODEL2	( 1<<20 )		// weapons, flags, etc
#define	U_MODEL3	( 1<<21 )
#define	U_MODEL4	( 1<<22 )
#define	U_MOREBITS3	( 1<<23 )		// read one additional byte

// fourth byte
#define	U_OLDORIGIN	( 1<<24 )		// FIXME: get rid of this
#define	U_SKIN16	( 1<<25 )
#define	U_SOUND		( 1<<26 )
#define	U_SOLID		( 1<<27 )


/*
==============================================================

CMD

Command text buffering and command execution

==============================================================
*/

/*

Any number of commands can be added in a frame, from several different sources.
Most commands come from either keybindings or console line input, but remote
servers can also send across commands and entire text files can be execed.

The + command line options are also added to the command buffer.

The game starts with a Cbuf_AddText( "exec quake.rc\n" ); Cbuf_Execute( );

*/

class CBuffer {

public:

	static void			Cbuf_Init( );
	// allocates an initial text buffer that will grow as needed

	static void			Cbuf_AddText( const Str & text );
	// as new commands are generated from the console or keybindings, // the text is added to the end of the command buffer.

	static void			Cbuf_InsertText( const Str & text );
	// when a command wants to issue other commands immediately, the text is
	// inserted at the beginning of the buffer, before any remaining unexecuted
	// commands.

	static void			Cbuf_ExecuteText( int exec_when, const Str & text );
	// this can be used in place of either Cbuf_AddText or Cbuf_InsertText

	static void			Cbuf_AddEarlyCommands( bool clear );
	// adds all the +set commands from the command line

	static bool			Cbuf_AddLateCommands( );
	// adds all the remaining + commands from the command line
	// Returns true if any late commands were added, which
	// will keep the demoloop from immediately starting

	static void			Cbuf_Execute( );
	// Pulls off \n terminated lines of text from the command buffer and sends
	// them through Cmd_ExecuteString.  Stops when the buffer is empty.
	// Normally called once per frame, but may be explicitly invoked.
	// Do not call inside a command function!

	static void			Cbuf_CopyToDefer( );
	static void			Cbuf_InsertFromDefer( );
	// These two functions are used to defer any pending commands while a map
	// is being loaded
};

//===========================================================================

/*

Command execution takes a null terminated string, breaks it into tokens, then searches for a command or variable that matches the first token.

*/

typedef void( *xcommand_t )( );

class Command {

private:

	static	List< Str >			cmd_argv;
	static	Str					cmd_args;

	static class cmd_function_t *		cmd_functions;		// possible commands to execute

public:

	static void	Cmd_Init( );

	static void	Cmd_AddCommand( const Str & cmd_name, xcommand_t function );
	// called by the init functions of other parts of the program to
	// register commands and functions to call for them.
	// The cmd_name is referenced later, so it should not be in temp memory
	// if function is NULL, the command will be forwarded to the server
	// as a clc_stringcmd instead of executed locally
	static void	Cmd_RemoveCommand( const Str & cmd_name );

	static bool			Cmd_Exists( const Str & cmd_name );
	// used by the cvar code to check for cvar / command name overlap

	static const Str	Cmd_CompleteCommand( const Str & partial );
	// attempts to match a partial command for automatic command line completion
	// returns NULL if nothing fits

	static int			Cmd_Argc( );
	static const Str &	Cmd_Argv( int arg );
	static const Str &	Cmd_Args( );
	// The functions that execute commands get their parameters with these
	// functions. Cmd_Argv( ) will return an empty string, not a NULL
	// if arg > argc, so string operations are always safe.

	static void	Cmd_TokenizeString( const Str & text, bool macroExpand );
	// Takes a null terminated string.  Does not need to be /n terminated.
	// breaks the string up into arg tokens.

	static void	Cmd_ExecuteString( const Str & text );
	// Parses a single line of text into arguments and tries to execute it
	// as if it was typed at the console
	
	static void			Cmd_Wait_f( );
	static void			Cmd_Exec_f( );
	static void			Cmd_Echo_f( );
	static void			Cmd_Alias_f( );
	static void			Cmd_List_f( );
};

/*
==============================================================

NET

==============================================================
*/

// net.h -- quake's interface to the networking layer

#define	PORT_ANY	-1

#define	MAX_MSGLEN		1400		// max length of a message
#define	PACKET_HEADER	10			// two ints and a short

typedef enum {
	NA_BAD,
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP
} netadrtype_t;

typedef enum {
	NS_CLIENT,
	NS_SERVER
} netsrc_t;

class netadr_t {
public:
	netadrtype_t	type;
	byte			ip[ 4 ];
	unsigned short	port;

	void			Clear( ) { type = NA_BAD; *( ( int * )ip ) = 0; port = 0; }
};

#define	MAX_LOOPBACK	4

typedef struct {
	byte		data[ MAX_MSGLEN ];
	size_t		datalen;
} loopmsg_t;

typedef struct {
	loopmsg_t	msgs[ MAX_LOOPBACK ];
	size_t		get, send;
} loopback_t;

typedef intptr_t        SOCKET;

#define MAX_UDP_MSG_SIZE	1400

typedef struct udpMsg_s {
	byte				d_data[ MAX_UDP_MSG_SIZE ];
	netadr_t			d_address;
	int					d_size;
	timeType			d_time;
	struct udpMsg_s *	d_next;
} udpMsg_t;

// // //// // //// // //// //
/// UDPPort
// //// // //// // //// //

class UDPPort {
public:
								UDPPort( );

	udpMsg_t *					d_sendFirst;
	udpMsg_t *					d_sendLast;
	udpMsg_t *					d_recieveFirst;
	udpMsg_t *					d_recieveLast;
	BlockAlloc< udpMsg_t, 64 >	d_udpMsgAllocator;
};

INLINE UDPPort::UDPPort( ) {
	d_sendFirst = d_sendLast = d_recieveFirst = d_recieveLast = NULL;
}

class MessageBuffer;

// // //// // //// // //// //
/// NetworkPort
// //// // //// // //// //

class NetworkPort {
private:
	UDPPort			d_udpPort;
	netadr_t		d_boundTarget;		// interface and port
	SOCKET			d_socket;		// OS specific socket
public:
					NetworkPort( );				// this just zeros netSocket and port
					~NetworkPort( );

	// if the InitForPort fails, the idPort.port field will remain 0
	bool			InitForPort( int portNumber );
	int				GetPort( ) const { return d_boundTarget.port; }
	netadr_t		GetAdr( ) const { return d_boundTarget; }
	void			Close( );

	bool			GetPacket( netadr_t & from, void * data, int & size, int maxSize );
	bool			GetPacketBlocking( netadr_t & from, void * data, int & size, int maxSize, int timeout );
	void			SendPacket( const netadr_t to, const void * data, int size );

	void			SendPacket( const netadr_t to, const MessageBuffer & msg_buffer );
	bool			GetPacket( netadr_t & from, MessageBuffer & msg_buffer );

	int				d_packetsRead;
	int				d_bytesRead;

	int				d_packetsWritten;
	int				d_bytesWritten;
};

// // //// // //// // //// //
/// Network
// //// // //// // //// //

class Network {
	friend class		NetworkPort;
private:
	static bool			old_config;
	static loopback_t	loopbacks[ ];
	static SOCKET		ip_sockets[ ];

	static CVar			net_noudp;
	static CVar			net_ip;
	static CVar			net_hostport;
	static CVar			net_clientport;
	static CVar			net_forceLatency;
	static CVar			net_forceDrop;

public:
	static void			NET_Init( );
	static void			NET_Shutdown( );

	static void			NET_Config( bool multiplayer );

	static bool			NET_GetPacket( netsrc_t sock, netadr_t * net_from, sizebuf_t * net_message );
	static void			NET_SendPacket( netsrc_t sock, size_t length, void * data, netadr_t to );

	static bool			Net_GetUDPPacket( SOCKET netSocket, netadr_t & net_from, char * data, int & size, int maxSize );
	static void			Net_SendUDPPacket( SOCKET netSocket, int length, const void * data, const netadr_t to );
	static bool			Net_WaitForUDPPacket( SOCKET netSocket, int timeout );

	static bool			NET_CompareBaseAdr( const netadr_t a, const netadr_t b );
	static bool			NET_IsLocalAddress( netadr_t adr );
	static const Str	NET_AdrToString( netadr_t a );
	static bool			NET_StringToAdr( const Str & string, netadr_t * a );
	static bool			NET_StringToSockaddr( const Str & string, struct WindowsNS::sockaddr * sadr );
	static void			NET_Sleep( int msec );

	// net_wins.cpp
	static void			NetadrToSockadr( const netadr_t * a, struct WindowsNS::sockaddr * s );
	static void			SockadrToNetadr( struct WindowsNS::sockaddr * s, netadr_t * a );
	static bool			NET_CompareAdr( const netadr_t a, const netadr_t b );
	static bool			NET_GetLoopPacket( const netsrc_t sock, netadr_t * net_from, sizebuf_t * net_message );
	static void			NET_SendLoopPacket( netsrc_t sock, size_t length, void * data, netadr_t to );
	static SOCKET		NET_IPSocket( const Str & net_interface, int port, netadr_t * bound_to );
	static void			NET_OpenIP( );
	static const char *	NET_ErrorString( );
};

//============================================================================

#define	OLD_AVG		0.99f		// total = oldtotal* OLD_AVG + new* ( 1-OLD_AVG )

#define	MAX_LATENT	32

typedef struct {

	bool	fatal_error;

	netsrc_t	sock;

	int			dropped;			// between last packet and previous

	timeType	last_received;		// for timeouts
	timeType	last_sent;			// for retransmits

	netadr_t	remote_address;
	int			qport;				// qport value to write when transmitting

// sequencing variables
	uint		incoming_sequence;
	uint		incoming_acknowledged;
	uint		incoming_reliable_acknowledged;	// single bit

	uint		incoming_reliable_sequence;		// single bit, maintained local

	uint		outgoing_sequence;
	uint		reliable_sequence;			// single bit
	uint		last_reliable_sequence;		// sequence number of last send

// reliable staging and holding areas
	sizebuf_t	message;		// writing buffer to send to server
	byte		message_buf[ MAX_MSGLEN-16 ];		// leave space for header

// message is copied to this buffer when it is first transfered
	size_t		reliable_length;
	byte		reliable_buf[ MAX_MSGLEN-16 ];	// unacked reliable message

} netchan_t;

class Netchan {
	friend class		MessageChannel;
private:

	static CVar			showpackets;
	static CVar			showdrop;
	static CVar			qport;

public:
#if 0
	static void			Netchan_Init( );
	static void			Netchan_Setup( netsrc_t sock, netchan_t * chan, netadr_t adr, int qport );
	static bool			Netchan_NeedReliable( netchan_t * chan );
	static void			Netchan_Transmit( netchan_t * chan, size_t length, byte * data );
	static void			Netchan_OutOfBand( int net_socket, netadr_t adr, size_t length, byte * data );
	static void			Netchan_OutOfBandPrint( int net_socket, netadr_t adr, const char * format, ... );
	static bool			Netchan_Process( netchan_t * chan, sizebuf_t * msg );
	static bool			Netchan_CanReliable( netchan_t * chan );
#endif
};

/*
==============================================================

MISC

==============================================================
*/

enum {

	ERR_FATAL, 		// exit the entire game with a popup window
	ERR_DROP, 		// print to console and disconnect from game
	ERR_DISCONNECT	// not an error, just a normal exit
};

enum {

	EXEC_NOW, 		// don't return until completed
	EXEC_INSERT, 	// insert at current position, but don't run yet
	EXEC_APPEND		// add to end of the command buffer
};

enum {

	PRINT_ALL, PRINT_DEVELOPER, 		// only print when "developer 1"
	PRINT_ALERT
};

#define NUMVERTEXNORMALS	162

class Common {

public: // this is the place of engine-wide CVars

	static CVar			userinfo_modified;
	static CVar			developer;
	static CVar			showclamp;
	static CVar			paused;
	static CVar			timescale;
	static CVar			fixedtime;
	static CVar			logfile_active;
	static CVar			dedicated;
	static CVar			build;
	static CVar			maxclients;

	static CVar			vid_xpos;
	static CVar			vid_ypos;
	static CVar			vid_width;
	static CVar			vid_height;
	static CVar			vid_fullscreen;
	static CVar			vid_gamma;

	static const Str	logfile_name;
	static FileBase *	logfile;

	static Str *		g_redirectBuffer;

public:

	static const Vec3	bytedirs[ NUMVERTEXNORMALS ];

	static void			Com_BeginRedirect( Str & buffer );
	static void			Com_EndRedirect( );
	static void			Com_Printf( const char * fmt, ... );
	static void			Com_DPrintf( const char * fmt, ... );
	static void			Com_Error( int code, const char * fmt, ... );
	static void			Com_Quit( );

	static int			Com_ServerState( );		// this should have just been a cvar...
	static void			Com_SetServerState( int state );

	static int			FloatToBits( const float & f, int exponentBits, int mantissaBits );
	static float		BitsToFloat( int i, int exponentBits, int mantissaBits );
	static unsigned		Com_BlockChecksum( const void * buffer, size_t length );
	static byte			COM_BlockSequenceCRCByte( byte * base, size_t length, int sequence );

	static int			COM_Argc( );
	static const Str	COM_Argv( int arg );	// range and null checked
	static void			COM_ClearArgv( int arg );
	static int			COM_CheckParm( const Str & parm );

	static void			COM_InitArgv( int argc, char ** argv );

	static void			Init( int argc, char ** argv );
	static void			Shutdown( );
	static void			Frame( timeType msec );

	static void			Com_Error_f( );
	static void			Adjust( );
	static void			Adjust2( );

	static timeType		GetClockTicks( );
	static timeType		ClockTicksPerSecond( );

	static void *		Com_Memcpy( void * Dest, const void * Source, size_t Size );
	static void *		Com_Memset( void * Dest, int Value, size_t Size );
	static void			MinMax( Vec3 & min, Vec3 & max, const Vec3 * src, const int count );
};

INLINE void Common::MinMax( Vec3 & min, Vec3 & max, const Vec3 * src, const int count ) {
	min[0] = min[1] = min[2] = INFINITY;
	max[0] = max[1] = max[2] = -INFINITY;
	for( int i = 0; i < count; i++ ) {
		const Vec3 & v = src[ i ];
		if( v[0] < min[0] )
			min[0] = v[0];
		if( v[0] > max[0] )
			max[0] = v[0];
		if( v[1] < min[1] )
			min[1] = v[1];
		if( v[1] > max[1] )
			max[1] = v[1];
		if( v[2] < min[2] )
			min[2] = v[2];
		if( v[2] > max[2] )
			max[2] = v[2];
	}
}

INLINE void * Common::Com_Memcpy( void * Dest, const void * Source, size_t Size ) {
	//return memcpy( Dest, Source, Size );
	SSE_Data xmmTemp;
	xmmword * dest2;
	const xmmword * source2;
	size_t byteCount = Size % 16;
	__movsb( ( byte * )Dest, ( byte * )Source, byteCount );
	if( !( Size - byteCount ) )
		return Dest;
	int copyCount = ( int )( Size - byteCount ) / 16;
	dest2 = ( xmmword * )( ( ( byte * )Dest ) + byteCount );
	source2 = ( xmmword * )( ( ( byte * )Source ) + byteCount );
	for( int i = 0; i < copyCount; i++ ) {
		xmmTemp.LoadUnaligned( source2 + i );
		xmmTemp.StoreUnaligned( dest2 + i );
	}
	return Dest;
}

INLINE void * Common::Com_Memset( void * Dest, int Value, size_t Size ) {
	//return memset( Dest, Value, Size );
	SSE_Data xmmTemp;
	xmmword * dest2;
	size_t byteCount = Size % 16;
	__stosb( ( byte * )Dest, ( byte )Value, byteCount );
	if( !( Size - byteCount ) )
		return Dest;
	int setValue = ( byte )Value;
	int setCount = ( int )( Size - byteCount ) / 16;
	dest2 = ( xmmword * )( ( ( byte * )Dest ) + byteCount );
	xmmTemp.GetIntRef( 0 ) = xmmTemp.GetIntRef( 1 ) = xmmTemp.GetIntRef( 2 ) = xmmTemp.GetIntRef( 3 ) =
		( setValue << 24 ) | ( setValue << 16 ) | ( setValue << 8 ) | ( setValue );
	for( int i = 0; i < setCount; i++ )
		xmmTemp.StoreUnaligned( dest2 + i );
	return Dest;
}

INLINE timeType Common::GetClockTicks( ) {
	return ( timeType )WindowsNS::GetTickCount( );
}

INLINE timeType Common::ClockTicksPerSecond( ) {
	return 1000;
}

INLINE void Common::Com_BeginRedirect( Str & buffer ) {
	assert( !g_redirectBuffer );
	g_redirectBuffer = &buffer;
}

INLINE void Common::Com_EndRedirect( ) {
	assert( g_redirectBuffer );
	g_redirectBuffer = NULL;
}

float	frand( );	// 0 ti 1
float	crand( );	// -1 to 1

void Z_Free( void * ptr );
void * Z_Malloc( size_t size );			// returns 0 filled memory
void * Z_TagMalloc( size_t size, int tag );
void Z_FreeTags( int tag );

/*
==============================================================

NON-PORTABLE SYSTEM SERVICES

==============================================================
*/

// directory searching
#define SFF_ARCH    0x01
#define SFF_HIDDEN  0x02
#define SFF_RDONLY  0x04
#define SFF_SUBDIR  0x08
#define SFF_SYSTEM  0x10

class System {
private:

	static Str					findbase;
	static Str					findpath;
	static intptr_t				findhandle;
	static Str					console_text;
	static bool					console_clearprev;

#ifdef MM_TIMER
	static timeType				baseTime;
#else
	static timeTypeSystem		baseTime;
	static timeTypeReal			baseFreq;
	static timeTypeSystem		prevCounter;
	static timeTypeSystem		prevPerformance;
	static int					cpuCount;
#endif
	static timeType				lastTime;
	static bool					initialized;

	static dword				g_msgTimeBase;
	static dword				g_msgLast;

	static class WinConData *	g_conData;

	static WindowsNS::LRESULT WINAPI	InputLineWndProc( WindowsNS::HWND hWnd, WindowsNS::UINT uMsg, WindowsNS::WPARAM wParam, WindowsNS::LPARAM lParam );
	static WindowsNS::LRESULT WINAPI	ConWndProc( WindowsNS::HWND hWnd, WindowsNS::UINT uMsg, WindowsNS::WPARAM wParam, WindowsNS::LPARAM lParam );

public:

	static void			Sys_CreateConsole( );
	static void			Sys_DestroyConsole( );
	static void			Sys_ShowConsole( int visLevel, bool quitOnClose );
	static const Str &	Sys_InputConsole( );
	static void			Sys_OutputConsole( const char * pMsg );
	static void			Win_SetErrorText( const char * buf );
	static bool			Sys_HaveConsole( )		{ return g_conData != NULL; }
	
	static void			Sys_Init( );
	static void			Sys_AppActivate( );
	static void			Sys_SendKeyEvents( );
	static void			Sys_Error( const char * error, ... );
	static void			Sys_Quit( );
	static const Str	Sys_GetClipboardData( );
	static void			Sys_SetClipboardData( const Str & text );
	static void			Sys_Mkdir( const Str & path );
	static const Str	Sys_FindFirst( const Str & path, unsigned musthave, unsigned canthave );
	static const Str	Sys_FindNext( unsigned musthave, unsigned canthave );
	static void			Sys_FindClose( );
	static timeType &	Sys_Milliseconds( );
	static timeType &	Sys_LastTime( );
	static unsigned int	Sys_MsgMilliseconds( ) { return g_msgLast; }

	static float		CpuUsage( );

#ifndef MM_TIMER
	static int			Sys_MillisecondsInt( );
	static uint			Sys_MillisecondsUint( );

	static void			Sys_ReadRawTime( timeTypeSystem & counter );
	static timeType		Sys_RawTimeToMs( const timeTypeSystem & counter );
#endif

	static WindowsNS::LRESULT WINAPI	MainWndProc( WindowsNS::HWND hWnd, WindowsNS::UINT uMsg, WindowsNS::WPARAM wParam, WindowsNS::LPARAM lParam );
};

INLINE timeType & System::Sys_Milliseconds( ) {
#ifdef MM_TIMER
	lastTime = WindowsNS::timeGetTime( ) - baseTime;
#else
	__int64 longVal;
	WindowsNS::QueryPerformanceCounter( ( WindowsNS::LARGE_INTEGER * )&longVal );
	lastTime = ( timeType )( ( timeTypeReal )( longVal - baseTime ) / baseFreq );
#endif
	return lastTime;
}

#ifndef MM_TIMER
INLINE int System::Sys_MillisecondsInt( ) {
	__int64 longVal;
	WindowsNS::QueryPerformanceCounter( ( WindowsNS::LARGE_INTEGER * )&longVal );
	return ( int )( ( timeTypeReal )( longVal - baseTime ) / baseFreq );
}

INLINE uint System::Sys_MillisecondsUint( ) {
	__int64 longVal;
	WindowsNS::QueryPerformanceCounter( ( WindowsNS::LARGE_INTEGER * )&longVal );
	return ( uint )( ( timeTypeReal )( longVal - baseTime ) / baseFreq );
}

INLINE void System::Sys_ReadRawTime( timeTypeSystem & counter ) {
	WindowsNS::QueryPerformanceCounter( ( WindowsNS::LARGE_INTEGER * )&counter );
}

INLINE timeType System::Sys_RawTimeToMs( const timeTypeSystem & counter ) {
	return ( timeType )( ( timeTypeReal )( counter ) / baseFreq );
}
#endif

INLINE timeType & System::Sys_LastTime( ) {
	return lastTime;
}

#define	MAX_MESSAGE_SIZE				16384		// max length of a message, which may
													// be fragmented into multiple packets
#define MAX_MSG_QUEUE_SIZE				16384		// must be a power of 2

#define CONNECTIONLESS_MESSAGE_ID		-1

#define	MAX_PACKETLEN			1400		// max size of a network packet
#define	FRAGMENT_SIZE			( MAX_PACKETLEN - 100 )
#define	FRAGMENT_BIT			( 1 << 31 )

// // //// // //// // //// //
/// MessageQueue
// //// // //// // //// //

class MessageQueue {
private:
	byte			d_buffer[ MAX_MSG_QUEUE_SIZE ];
	int				d_first;			// sequence number of first message in queue
	int				d_last;			// sequence number of last message in queue
	int				d_startIndex;		// index pointing to the first byte of the first message
	int				d_endIndex;		// index pointing to the first byte after the last message

	void			WriteByte( byte b );
	byte			ReadByte( );
	void			WriteShort( int s );
	int				ReadShort( );
	void			WriteLong( int l );
	int				ReadLong( );
	void			WriteData( const byte * data, const int size );
	void			ReadData( byte * data, const int size );

public:
					MessageQueue( );

	void			Init( int sequence );

	bool			Add( const byte * data, const int size );
	bool			Get( byte * data, int & size );
	int				GetTotalSize( ) const;
	int				GetSpaceLeft( ) const;
	int				GetFirst( ) const { return d_first; }
	int				GetLast( ) const { return d_last; }
	void			CopyToBuffer( byte * buf ) const;
};

class Dict;

// // //// // //// // //// //
/// MessageBuffer
// //// // //// // //// //

class MessageBuffer {
	friend class	NetworkPort;
protected:
	byte *			d_writeData;			// pointer to data for writing
	const byte *	d_readData;			// pointer to data for reading
	int				d_maxSize;			// maximum size of message in bytes
	int				d_curSize;			// current size of message in bytes
	int				d_writeBit;			// number of bits written to the last written byte
	mutable int		d_readCount;			// number of bytes read so far
	mutable int		d_readBit;			// number of bits read from the last read byte
	bool			d_allowOverflow;		// if false, generate an error when the message is overflowed
	bool			d_overflowed;			// set to true if the buffer size failed (with allowOverflow set)

	bool			CheckOverflow( int numBits );
	byte *			GetByteSpace( int length );
	void			WriteDelta( int oldValue, int newValue, int numBits );
	int				ReadDelta( int oldValue, int numBits ) const;
	void			ZeroState( );

public:
					MessageBuffer( );

	void			Init( byte * data, int length );
	void			Init( const byte * data, int length );
	byte *			GetData( );						// get data for writing
	const byte *	GetData( ) const;					// get data for reading
	int				GetMaxSize( ) const;				// get the maximum message size
	void			SetAllowOverflow( bool set );			// generate error if not set and message is overflowed
	bool			IsOverflowed( ) const;				// returns true if the message was overflowed

	int				GetSize( ) const;					// size of the message in bytes
	void			SetSize( int size );					// set the message size
	int				GetWriteBit( ) const;				// get current write bit
	void			SetWriteBit( int bit );					// set current write bit
	int				GetNumBitsWritten( ) const;		// returns number of bits written
	int				GetRemainingWriteBits( ) const;	// space left in bits for writing
	void			SaveWriteState( int & s, int & b ) const;	// save the write state
	void			RestoreWriteState( int s, int b );		// restore the write state

	int				GetReadCount( ) const;				// bytes read so far
	void			SetReadCount( int bytes );				// set the number of bytes and bits read
	int				GetReadBit( ) const;				// get current read bit
	void			SetReadBit( int bit );					// set current read bit
	int				GetNumBitsRead( ) const;			// returns number of bits read
	int				GetRemainingReadBits( ) const;		// number of bits left to read
	void			SaveReadState( int & c, int & b ) const;	// save the read state
	void			RestoreReadState( int c, int b );		// restore the read state

	void			BeginWriting( );					// begin writing
	int				GetRemainingSpace( ) const;		// space left in bytes
	void			WriteByteAlign( );					// write up to the next byte boundary
	void			WriteBits( int value, int numBits );	// write the specified number of bits
	void			WriteChar( int c );
	void			WriteByte( int c );
	void			WriteShort( int c );
	void			WriteUShort( int c );
	void			WriteLong( int c );
	void			WriteFloat( const float & f );
	void			WriteFloat( const float & f, int exponentBits, int mantissaBits );
	void			WriteAngle8( const float & f );
	void			WriteAngle16( const float & f );
	void			WriteDir( const Vec3 & dir, int numBits );
	void			WriteVec3( const Vec3 & vec );
	void			WriteString( const Str & s );
	void			WriteData( const void * data, int length );
	void			WriteNetadr( const netadr_t adr );

	void			WriteDeltaChar( int oldValue, int newValue );
	void			WriteDeltaByte( int oldValue, int newValue );
	void			WriteDeltaShort( int oldValue, int newValue );
	void			WriteDeltaLong( int oldValue, int newValue );
	void			WriteDeltaFloat( const float & oldValue, const float & newValue );
	void			WriteDeltaFloat( const float & oldValue, const float & newValue, int exponentBits, int mantissaBits );
	void			WriteDeltaByteCounter( int oldValue, int newValue );
	void			WriteDeltaShortCounter( int oldValue, int newValue );
	void			WriteDeltaLongCounter( int oldValue, int newValue );
	bool			WriteDeltaDict( const Dict & dict, const Dict * base );

	void			BeginReading( ) const;				// begin reading.
	int				GetRemaingData( ) const;			// number of bytes left to read
	void			ReadByteAlign( ) const;			// read up to the next byte boundary
	int				ReadBits( int numBits ) const;			// read the specified number of bits
	int				ReadChar( ) const;
	int				ReadByte( ) const;
	int				ReadShort( ) const;
	int				ReadUShort( ) const;
	int				ReadLong( ) const;
	float			ReadFloat( ) const;
	float			ReadFloat( int exponentBits, int mantissaBits ) const;
	float			ReadAngle8( ) const;
	float			ReadAngle16( ) const;
	Vec3			ReadDir( int numBits ) const;
	void			ReadVec3( Vec3 & vec ) const;
	int				ReadString( Str & s ) const;
	int				ReadData( void * data, int length ) const;
	void			ReadNetadr( netadr_t * adr ) const;

	int				ReadDeltaChar( int oldValue ) const;
	int				ReadDeltaByte( int oldValue ) const;
	int				ReadDeltaShort( int oldValue ) const;
	int				ReadDeltaLong( int oldValue ) const;
	float			ReadDeltaFloat( const float & oldValue ) const;
	float			ReadDeltaFloat( const float & oldValue, int exponentBits, int mantissaBits ) const;
	int				ReadDeltaByteCounter( int oldValue ) const;
	int				ReadDeltaShortCounter( int oldValue ) const;
	int				ReadDeltaLongCounter( int oldValue ) const;
	bool			ReadDeltaDict( Dict & dict, const Dict * base ) const;

	static int		DirToBits( const Vec3 & dir, int numBits );
	static Vec3		BitsToDir( int bits, int numBits );
};

// // //// // //// // //// //
/// MessageBufferT
// //// // //// // //// //

template< int static_size > class MessageBufferT : public MessageBuffer {
private:
	byte			d_staticData[ static_size ];

public:
	void			Init( );
};

template< int static_size >
INLINE void MessageBufferT< static_size >::Init( ) {
	ZeroState( );
	Common::Com_Memset( d_staticData, 0, static_size );
	d_writeData = d_staticData;
	d_readData = d_staticData;
	d_maxSize = static_size;
}

// // //// // //// // //// //
/// MessageChannel
// //// // //// // //// //

class MessageChannel {
private:
	netadr_t		d_remoteAddress;	// address of remote host
	int				d_id;				// our identification used instead of port number
	int				d_maxRate;		// maximum number of bytes that may go out per second
	// variables to control the outgoing rate
	timeType		d_lastSendTime;	// last time data was sent out
	int				d_lastDataBytes;	// bytes left to send at last send time
	// variables to keep track of the rate
	timeType		d_outgoingRateTime;
	int				d_outgoingRateBytes;
	timeType		d_incomingRateTime;
	int				d_incomingRateBytes;
	// variables to keep track of the compression ratio
	float			d_outgoingCompression;
	float			d_incomingCompression;
	// variables to keep track of the incoming packet loss
	float			d_incomingReceivedPackets;
	float			d_incomingDroppedPackets;
	timeType		d_incomingPacketLossTime;
	// sequencing variables
	int				d_outgoingSequence;
	int				d_incomingSequence;
	// outgoing fragment buffer
	bool			d_unsentFragments;
	int				d_unsentFragmentStart;
	byte			d_unsentBuffer[ MAX_MESSAGE_SIZE ];
	MessageBuffer	d_unsentMsg;
	// incoming fragment assembly buffer
	int				d_fragmentSequence;
	int				d_fragmentLength;
	byte			d_fragmentBuffer[ MAX_MESSAGE_SIZE ];
	// reliable messages
	MessageQueue		d_reliableSend;
	MessageQueue		d_reliableReceive;

	void			WriteMessageData( MessageBuffer & out, const MessageBuffer & msg );
	bool			ReadMessageData( MessageBuffer & out, const MessageBuffer & msg );
	void			UpdateOutgoingRate( timeType time, const int size );
	void			UpdateIncomingRate( timeType time, const int size );
	void			UpdatePacketLoss( timeType time, const int numReceived, const int numDropped );

public:
					MessageChannel( );

	void			Init( const netadr_t adr, const int id );
	void			Shutdown( );
	void			ResetRate( );
	int				GetID( ) const { return d_id; }
	int				GetOutgoingSequence( ) const { return d_outgoingSequence; }
	int				GetIncomingSequence( ) const { return d_incomingSequence; }
	timeType		GetLastOutgoingTime( ) const { return d_outgoingRateTime + 1000; }
	timeType		GetLastIncomingTime( ) const { return d_incomingRateTime + 1000; }
	// Sets the maximum outgoing rate.
	void			SetMaxOutgoingRate( int rate ) { d_maxRate = rate; }
	// Gets the maximum outgoing rate.
	int				GetMaxOutgoingRate( ) { return d_maxRate; }
	// Returns the address of the entity at the other side of the channel.
	netadr_t		GetRemoteAddress( ) const { return d_remoteAddress; }
	// Returns the average outgoing rate over the last second.
	int				GetOutgoingRate( ) const { return d_outgoingRateBytes; }
	// Returns the average incoming rate over the last second.
	int				GetIncomingRate( ) const { return d_incomingRateBytes; }
	// Returns the average outgoing compression ratio over the last second.
	float			GetOutgoingCompression( ) const { return d_outgoingCompression; }
	// Returns the average incoming compression ratio over the last second.
	float			GetIncomingCompression( ) const { return d_incomingCompression; }
	// Returns the average incoming packet loss over the last 5 seconds.
	float			GetIncomingPacketLoss( ) const;
	// Returns true if the channel is ready to send new data based on the maximum rate.
	bool			ReadyToSend( timeType time ) const;
	// Sends an unreliable message, in order and without duplicates.
	int				SendMessage( NetworkPort & port, timeType time, const MessageBuffer & msg );
	// Sends the next fragment if the last message was too large to send at once.
	void			SendNextFragment( NetworkPort & port, timeType time );
	// Returns true if there are unsent fragments left.
	bool			UnsentFragmentsLeft( ) const { return d_unsentFragments; }
	// Processes the incoming message. Returns true when a complete message
	// is ready for further processing. In that case the read pointer of msg
	// points to the first byte ready for reading, and sequence is set to
	// the sequence number of the message.
	bool			Process( const netadr_t from, timeType time, MessageBuffer & msg, int & sequence );
	// Sends a reliable message, in order and without duplicates.
	bool			SendReliableMessage( const MessageBuffer & msg );
	// Returns true if a new reliable message is available and stores the message.
	bool			GetReliableMessage( MessageBuffer & msg );
	// Removes any pending outgoing or incoming reliable messages.
	void			ClearReliableMessages( );
};

// // //// // //// // //// //
// MessageQueue
// //// // //// // //// //

INLINE void MessageQueue::Init( int sequence ) {
	d_first = d_last = sequence;
	d_startIndex = d_endIndex = 0;
}

INLINE int MessageQueue::GetTotalSize( ) const {
	return ( d_startIndex <= d_endIndex ) ? ( d_endIndex - d_startIndex ) : ( sizeof( d_buffer ) - d_startIndex + d_endIndex );
}

INLINE int MessageQueue::GetSpaceLeft( ) const {
	return ( ( d_startIndex <= d_endIndex ) ? ( sizeof( d_buffer ) - ( d_endIndex - d_startIndex ) ) : ( d_startIndex - d_endIndex ) ) - 1;
}

INLINE void MessageQueue::WriteByte( byte b ) {
	d_buffer[ d_endIndex ] = b;
	d_endIndex = ( d_endIndex + 1 ) & ( MAX_MSG_QUEUE_SIZE - 1 );
}

INLINE byte MessageQueue::ReadByte( ) {
	byte b = d_buffer[ d_startIndex ];
	d_startIndex = ( d_startIndex + 1 ) & ( MAX_MSG_QUEUE_SIZE - 1 );
	return b;
}

INLINE void MessageQueue::WriteShort( int s ) {
	WriteByte( ( s >>  0 ) & 255 );
	WriteByte( ( s >>  8 ) & 255 );
}

INLINE int MessageQueue::ReadShort( ) {
	return ReadByte( ) | ( ReadByte( ) << 8 );
}

INLINE void MessageQueue::WriteLong( int l ) {
	WriteByte( ( l >>  0 ) & 255 );
	WriteByte( ( l >>  8 ) & 255 );
	WriteByte( ( l >> 16 ) & 255 );
	WriteByte( ( l >> 24 ) & 255 );
}

INLINE int MessageQueue::ReadLong( ) {
	return ReadByte( ) | ( ReadByte( ) << 8 ) | ( ReadByte( ) << 16 ) | ( ReadByte( ) << 24 );
}

INLINE void MessageQueue::WriteData( const byte * data, const int size ) {
	for( int i = 0; i < size; i++ )
		WriteByte( data[ i ] );
}

INLINE void MessageQueue::ReadData( byte * data, const int size ) {
	if( data ) {
		for( int i = 0; i < size; i++ )
			data[ i ] = ReadByte( );
	} else {
		for( int i = 0; i < size; i++ )
			ReadByte( );
	}
}

// // //// // //// // //// //
// MessageBuffer
// //// // //// // //// //

INLINE void MessageBuffer::ZeroState( ) {
	Common::Com_Memset( this, 0, sizeof( MessageBuffer ) );
}

INLINE void MessageBuffer::Init( byte * data, int length ) {
	ZeroState( );
	d_writeData = data;
	d_readData = data;
	d_maxSize = length;
}

INLINE void MessageBuffer::Init( const byte * data, int length ) {
	ZeroState( );
	d_writeData = NULL;
	d_readData = data;
	d_maxSize = length;
}

INLINE byte * MessageBuffer::GetData( ) {
	return d_writeData;
}

INLINE const byte * MessageBuffer::GetData( ) const {
	return d_readData;
}

INLINE int MessageBuffer::GetMaxSize( ) const {
	return d_maxSize;
}

INLINE void MessageBuffer::SetAllowOverflow( bool set ) {
	d_allowOverflow = set;
}

INLINE bool MessageBuffer::IsOverflowed( ) const {
	return d_overflowed;
}

INLINE int MessageBuffer::GetSize( ) const {
	return d_curSize;
}

INLINE void MessageBuffer::SetSize( int size ) {
	if( size > d_maxSize )
		d_curSize = d_maxSize;
	else
		d_curSize = size;
}

INLINE int MessageBuffer::GetWriteBit( ) const {
	return d_writeBit;
}

INLINE void MessageBuffer::SetWriteBit( int bit ) {
	d_writeBit = bit & 7;
	if( d_writeBit )
		d_writeData[ d_curSize - 1 ] &= ( 1 << d_writeBit ) - 1;
}

INLINE int MessageBuffer::GetNumBitsWritten( ) const {
	return ( ( d_curSize << 3 ) - ( ( 8 - d_writeBit ) & 7 ) );
}

INLINE int MessageBuffer::GetRemainingWriteBits( ) const {
	return ( d_maxSize << 3 ) - GetNumBitsWritten( );
}

INLINE void MessageBuffer::SaveWriteState( int & s, int & b ) const {
	s = d_curSize;
	b = d_writeBit;
}

INLINE void MessageBuffer::RestoreWriteState( int s, int b ) {
	d_curSize = s;
	d_writeBit = b & 7;
	if( d_writeBit )
		d_writeData[ d_curSize - 1 ] &= ( 1 << d_writeBit ) - 1;
}

INLINE int MessageBuffer::GetReadCount( ) const {
	return d_readCount;
}

INLINE void MessageBuffer::SetReadCount( int bytes ) {
	d_readCount = bytes;
}

INLINE int MessageBuffer::GetReadBit( ) const {
	return d_readBit;
}

INLINE void MessageBuffer::SetReadBit( int bit ) {
	d_readBit = bit & 7;
}

INLINE int MessageBuffer::GetNumBitsRead( ) const {
	return ( ( d_readCount << 3 ) - ( ( 8 - d_readBit ) & 7 ) );
}

INLINE int MessageBuffer::GetRemainingReadBits( ) const {
	return ( d_curSize << 3 ) - GetNumBitsRead( );
}

INLINE void MessageBuffer::SaveReadState( int & c, int & b ) const {
	c = d_readCount;
	b = d_readBit;
}

INLINE void MessageBuffer::RestoreReadState( int c, int b ) {
	d_readCount = c;
	d_readBit = b & 7;
}

INLINE void MessageBuffer::BeginWriting( ) {
	d_curSize = 0;
	d_overflowed = false;
	d_writeBit = 0;
}

INLINE int MessageBuffer::GetRemainingSpace( ) const {
	return d_maxSize - d_curSize;
}

INLINE void MessageBuffer::WriteByteAlign( ) {
	d_writeBit = 0;
}

INLINE void MessageBuffer::WriteChar( int c ) {
	WriteBits( c, -8 );
}

INLINE void MessageBuffer::WriteByte( int c ) {
	WriteBits( c, 8 );
}

INLINE void MessageBuffer::WriteShort( int c ) {
	WriteBits( c, -16 );
}

INLINE void MessageBuffer::WriteUShort( int c ) {
	WriteBits( c, 16 );
}

INLINE void MessageBuffer::WriteLong( int c ) {
	WriteBits( c, 32 );
}

INLINE void MessageBuffer::WriteFloat( const float & f ) {
	WriteBits( *reinterpret_cast< const int * >( &f ), 32 );
}

INLINE void MessageBuffer::WriteFloat( const float & f, int exponentBits, int mantissaBits ) {
	int bits = Common::FloatToBits( f, exponentBits, mantissaBits );
	WriteBits( bits, 1 + exponentBits + mantissaBits );
}

INLINE void MessageBuffer::WriteAngle8( const float & f ) {
	WriteByte( ANGLE2BYTE( f ) );
}

INLINE void MessageBuffer::WriteAngle16( const float & f ) {
	WriteShort( ANGLE2SHORT(f) );
}

INLINE void MessageBuffer::WriteDir( const Vec3 & dir, int numBits ) {
	WriteBits( DirToBits( dir, numBits ), numBits );
}

INLINE void MessageBuffer::WriteVec3( const Vec3 & vec ) {
	WriteBits( *reinterpret_cast< const int * >( &vec.x ), 32 );
	WriteBits( *reinterpret_cast< const int * >( &vec.y ), 32 );
	WriteBits( *reinterpret_cast< const int * >( &vec.z ), 32 );
}

/*INLINE void MessageBuffer::WriteString( const Str & s ) {
	int i, l = s.Length( );
	byte * dataPtr = GetByteSpace( l + 1 );
	const byte * bytePtr = reinterpret_cast< const byte * >( s.c_str( ) );
	for( i = 0; i < l; i++ )
		dataPtr[ i ] = bytePtr[ i ];
	dataPtr[ i ] = '\0';
}*/

INLINE void MessageBuffer::WriteData( const void * data, int length ) {
	Common::Com_Memcpy( GetByteSpace( length ), data, length );
}

INLINE void MessageBuffer::WriteNetadr( const netadr_t adr ) {
	byte * dataPtr = GetByteSpace( 4 );
	Common::Com_Memcpy( dataPtr, adr.ip, 4 );
	WriteUShort( adr.port );
}

INLINE void MessageBuffer::WriteDelta( int oldValue, int newValue, int numBits ) {
	if( oldValue == newValue ) {
		WriteBits( 0, 1 );
		return;
	}
	WriteBits( 1, 1 );
	WriteBits( newValue, numBits );
}

INLINE void MessageBuffer::WriteDeltaChar( int oldValue, int newValue ) {
	WriteDelta( oldValue, newValue, -8 );
}

INLINE void MessageBuffer::WriteDeltaByte( int oldValue, int newValue ) {
	WriteDelta( oldValue, newValue, 8 );
}

INLINE void MessageBuffer::WriteDeltaShort( int oldValue, int newValue ) {
	WriteDelta( oldValue, newValue, -16 );
}

INLINE void MessageBuffer::WriteDeltaLong( int oldValue, int newValue ) {
	WriteDelta( oldValue, newValue, 32 );
}

INLINE void MessageBuffer::WriteDeltaFloat( const float & oldValue, const float & newValue ) {
	WriteDelta( *reinterpret_cast< const int * >( &oldValue ), *reinterpret_cast< const int * >( &newValue ), 32 );
}

INLINE void MessageBuffer::WriteDeltaFloat( const float & oldValue, const float & newValue, int exponentBits, int mantissaBits ) {
	int oldBits = Common::FloatToBits( oldValue, exponentBits, mantissaBits );
	int newBits = Common::FloatToBits( newValue, exponentBits, mantissaBits );
	WriteDelta( oldBits, newBits, 1 + exponentBits + mantissaBits );
}

INLINE void MessageBuffer::BeginReading( ) const {
	d_readCount = 0;
	d_readBit = 0;
}

INLINE int MessageBuffer::GetRemaingData( ) const {
	return d_curSize - d_readCount;
}

INLINE void MessageBuffer::ReadByteAlign( ) const {
	d_readBit = 0;
}

INLINE int MessageBuffer::ReadChar( ) const {
	return ( signed char )ReadBits( -8 );
}

INLINE int MessageBuffer::ReadByte( ) const {
	return ( unsigned char )ReadBits( 8 );
}

INLINE int MessageBuffer::ReadShort( ) const {
	return ( short )ReadBits( -16 );
}

INLINE int MessageBuffer::ReadUShort( ) const {
	return ( unsigned short )ReadBits( 16 );
}

INLINE int MessageBuffer::ReadLong( ) const {
	return ReadBits( 32 );
}

INLINE float MessageBuffer::ReadFloat( ) const {
	float value;
	*reinterpret_cast< int * >( &value ) = ReadBits( 32 );
	return value;
}

INLINE float MessageBuffer::ReadFloat( int exponentBits, int mantissaBits ) const {
	int bits = ReadBits( 1 + exponentBits + mantissaBits );
	return Common::BitsToFloat( bits, exponentBits, mantissaBits );
}

INLINE float MessageBuffer::ReadAngle8( ) const {
	return BYTE2ANGLE( ReadByte( ) );
}

INLINE float MessageBuffer::ReadAngle16( ) const {
	return SHORT2ANGLE( ReadShort( ) );
}

INLINE Vec3 MessageBuffer::ReadDir( int numBits ) const {
	return BitsToDir( ReadBits( numBits ), numBits );
}

INLINE void MessageBuffer::ReadVec3( Vec3 & vec ) const {
	*reinterpret_cast< int * >( &vec.x ) = ReadBits( 32 );
	*reinterpret_cast< int * >( &vec.y ) = ReadBits( 32 );
	*reinterpret_cast< int * >( &vec.z ) = ReadBits( 32 );
}

/*INLINE int MessageBuffer::ReadString( Str & s ) const {
	ReadByteAlign( );
	while( 1 ) {
		int c = ReadByte( );
		if( c <= 0 || c >= 255 )
			break;
		s.Append( c );
	}	
	return s.Length( );
}*/

INLINE int MessageBuffer::ReadDeltaChar( int oldValue ) const {
	return ( signed char )ReadDelta( oldValue, -8 );
}

INLINE int MessageBuffer::ReadDeltaByte( int oldValue ) const {
	return ( unsigned char )ReadDelta( oldValue, 8 );
}

INLINE int MessageBuffer::ReadDeltaShort( int oldValue ) const {
	return ( short )ReadDelta( oldValue, -16 );
}

INLINE int MessageBuffer::ReadDeltaLong( int oldValue ) const {
	return ReadDelta( oldValue, 32 );
}

INLINE float MessageBuffer::ReadDeltaFloat( const float & oldValue ) const {
	float value;
	*reinterpret_cast< int * >( &value ) = ReadDelta( *reinterpret_cast< const int * >( &oldValue ), 32 );
	return value;
}

INLINE float MessageBuffer::ReadDeltaFloat( const float & oldValue, int exponentBits, int mantissaBits ) const {
	int oldBits = Common::FloatToBits( oldValue, exponentBits, mantissaBits );
	int newBits = ReadDelta( oldBits, 1 + exponentBits + mantissaBits );
	return Common::BitsToFloat( newBits, exponentBits, mantissaBits );
}

// // //// // //// // //// //
// MessageChannel
// //// // //// // //// //

INLINE void MessageChannel::ClearReliableMessages( ) {
	d_reliableSend.Init( 1 );
	d_reliableReceive.Init( 0 );
}

INLINE float MessageChannel::GetIncomingPacketLoss( ) const {
	if( d_incomingReceivedPackets == 0.0f && d_incomingDroppedPackets == 0.0f )
		return 0.0f;
	return d_incomingDroppedPackets * 100.0f / ( d_incomingReceivedPackets + d_incomingDroppedPackets );
}

INLINE bool MessageChannel::ReadyToSend( timeType time ) const {
	if( !d_maxRate )
		return true;
	timeType deltaTime = time - d_lastSendTime;
	if( deltaTime > 1000 )
		return true;
	return ( d_lastDataBytes - ( deltaTime * d_maxRate ) / 1000 ) <= 0;
}

// // //// // //// // //// //
// NetworkPort
// //// // //// // //// //

INLINE void NetworkPort::SendPacket( const netadr_t to, const MessageBuffer & msg_buffer ) {
	SendPacket( to, msg_buffer.d_readData, msg_buffer.d_curSize );
}

INLINE bool NetworkPort::GetPacket( netadr_t & from, MessageBuffer & msg_buffer ) {
	msg_buffer.d_writeBit = msg_buffer.d_readCount = msg_buffer.d_readBit = msg_buffer.d_overflowed = 0;
	return GetPacket( from, msg_buffer.d_writeData, msg_buffer.d_curSize, msg_buffer.d_maxSize );
}
