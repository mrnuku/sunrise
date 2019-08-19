// server.h


//define	PARANOID			// speed sapping error checking

//=============================================================================

#define	MAX_MASTERS	8				// max recipients for heartbeat packets

class moveclip_t {
public:
	Vec3		boxmins, boxmaxs;// enclose the test object along entire move
	Vec3		mins, maxs;	// size of the moving object
	Vec3		mins2, maxs2;	// size when clipping against mosnters
	Vec3		start, end;
	CTrace		trace;
	Entity *	passedict;
	int			contentmask;
};

typedef enum {
	ss_dead, 			// no map loaded
	ss_loading, 			// spawning level edicts
	ss_game, 			// actively running
	ss_cinematic,
	ss_demo,
	ss_pic
} server_state_t;
// some qc commands are only valid before the server has finished
// initializing( precache commands, static sounds / objects, etc )

class ServerLocalData {
public:
	server_state_t	state;			// precache commands are only valid during load

	bool			attractloop;		// running cinematics and demos for the local system only
	bool			loadgame;			// client begins should reuse existing entity

	timeType		time;				// always sv.framenum * 100 msec
	int				framenum;

	Str				name;			// map name, or cinematic name
	CModel *		models[ MAX_MODELS ];

	Dict			d_config;
	Str				configstrings[ MAX_CONFIGSTRINGS ];
	entity_state_t	baselines[ MAX_EDICTS ];

	// the multicast buffer is used to send a message to a set of clients
	// it is only used to marshall data until SV_Multicast is called
	MessageBufferT< MAX_MSGLEN >	d_multicastBuffer;

	// demo server information
	FileBase *		demofile;
	bool			timedemo;		// don't time sync

	void			Clear( );
};

//#define EDICT_NUM( n )( ( Entity * )( ( byte * )Game::edicts + Game::edict_size* ( n ) ) )
#define EDICT_NUM( n )( ( Entity * )( ( byte * )Game::edicts + Game::edict_size * ( n ) ) )
#define NUM_FOR_EDICT( e )( ( int )( ( byte * )( e )-( byte * )Game::edicts ) / Game::edict_size )


typedef enum {
	cs_free, 		// can be reused for a new connection
	cs_zombie, 		// client has been disconnected, but don't reuse
					// connection for a couple seconds
	cs_connected, 	// has been assigned to a client_t, but not in game yet
	cs_spawned		// client is fully in game
} client_state_e;

class client_frame_t {
public:
	int					areabytes;
	byte				areabits[ MAX_MAP_AREAS/8 ];		// portalarea visibility bits
	player_state_t		ps;
	int					num_entities;
	int					first_entity;		// into the circular sv_packet_entities[]
	timeType			senttime;			// for ping calculations

	void				Clear( );
};

#define	LATENCY_COUNTS	16
#define	RATE_MESSAGES	10

class client_t {
public:
	client_state_e	state;

	Str				userinfo;		// name, etc

	int				lastframe;			// for delta compression
	usercmd_t		lastcmd;			// for filling in big drops

	int				commandMsec;		// every seconds this is reset, if user
										// commands exhaust it, assume time cheating

	timeType		frame_latency[ LATENCY_COUNTS ];
	timeType		ping;

	int				surpressCount;		// number of messages rate supressed

	Entity *		edict;				// EDICT_NUM( clientnum+1 )
	Str				name;			// extracted from userinfo, high bits masked
	int				messagelevel;		// for filtering printed messages

	// The datagram is written to by sound calls, prints, temp ents, etc.
	// It can be harmlessly overflowed.

	client_frame_t	frames[ UPDATE_BACKUP ];	// updates can be delta'd from here

	File_Memory		download;			// file being downloaded
	size_t			downloadsize;		// total bytes( can't use EOF because of paks )
	size_t			downloadcount;		// bytes sent

	timeType		lastmessage;		// sv.framenum when packet was last received
	timeType		lastconnect;

	int				challenge;			// challenge of this user, randomly generated

	MessageChannel					d_clientChannel;
	MessageBufferT< MAX_MSGLEN >	d_clientBuffer;
	MessageBufferT< MAX_MSGLEN >	d_clientBufferReliable;

					client_t( ) { Clear( ); }
	void			Clear( );
};

// a client can leave the server in one of four ways:
// dropping properly by quiting or disconnecting
// timing out if no valid messages are received for timeout.value seconds
// getting kicked off by the server operator
// a program error, like an overflowed reliable buffer

//=============================================================================

// MAX_CHALLENGES is made large to prevent a denial
// of service attack that could cycle all of them
// out before legitimate users connected
#define	MAX_CHALLENGES	1024

class challenge_t {
public:
	netadr_t		adr;
	int				challenge;
	timeType		time;

	void			Clear( ) { adr.Clear( ); challenge = 0; time = 0; }
};

class ServerPersistantData {
public:
	bool				initialized;				// sv_init has completed
	timeType			realtime;					// always increasing, no clamping, etc

	Str					mapcmd;	// ie: * intro.cin+base 

	int					spawncount;					// incremented each server start
											// used to check late spawns

	client_t *			clients;					// [ maxclients->value ];
	int					num_client_entities;		// maxclients->value* UPDATE_BACKUP* MAX_PACKET_ENTITIES
	int					next_client_entities;		// next client_entity to use
	entity_state_t *	client_entities;		// [ num_client_entities ]

	timeType			last_heartbeat;

	NetworkPort			d_serverPort;

	challenge_t			challenges[ MAX_CHALLENGES ];	// to prevent invalid IPs from connecting

	// serverrecord values
	FileBase *			demofile;
	MessageBufferT< MAX_MSGLEN >	d_demoBuffer;

	void				Clear( );
};

//=============================================================================

typedef enum {
	RD_NONE,
	RD_CLIENT,
	RD_PACKET
} redirect_t;

#define	STRUCT_FROM_LINK( l, t, m )( ( t * )( ( byte * )l -( int )&( ( ( t * )0 )->m ) ) )
#define	EDICT_FROM_AREA( l ) STRUCT_FROM_LINK( l, Entity, area )
#define	AREA_DEPTH	4
#define	AREA_NODES	32
#define MAX_TOTAL_ENT_LEAFS		128

class areanode_t {
public:
	int				axis;		// -1 = leaf node
	float			dist;
	areanode_t *	children[ 2 ];
	link_t			trigger_edicts;
	link_t			solid_edicts;
};

//===========================================================

class ucmd_t {
public:
	const Str		d_name;
	void			( *d_func )( MessageBuffer & msg_buffer );
};

class Server {
private:
	static CVar		hostname;
	static CVar		timeout;
	static CVar		zombietime;
	static CVar		sv_paused;
	static CVar		sv_timedemo;
	static CVar		sv_enforcetime;
	static CVar		allow_download;
	static CVar		allow_download_players;
	static CVar		allow_download_models;
	static CVar		allow_download_sounds;
	static CVar		allow_download_maps;
	static CVar		sv_noreload;
	static CVar		sv_airaccelerate;
	static CVar		public_server;
	static CVar		sv_reconnect_limit;
	static CVar		protocol;
	static CVar		sv_mapname;

	static areanode_t	sv_areanodes[ AREA_NODES ];
	static int			sv_numareanodes;
	static Vec3			area_mins, area_maxs;
	static Entity **	area_list;
	static int			area_count, area_maxcount;
	static int			area_type;

	static ServerPersistantData		svs;				// persistant server info
	static ServerLocalData			sv;					// local server

	static netadr_t			master_adr[ MAX_MASTERS ];	// address of the master server

	static client_t *		sv_client;
	static Entity *			sv_player;

	static const ucmd_t		ucmds[ ];

	static int				lg_Map( LuaState & state );
	static int				lg_GameMap( LuaState & state );

public:
	// sv_main.c
	static void			SV_Init( );
	static void			SV_Frame( timeType msec );
	static void			SV_FinalMessage( const Str & message, bool reconnect );
	static void			SV_DropClient( client_t * drop );
	static int			SV_ModelIndex( const Str & name );
	static int			SV_SoundIndex( const Str & name );
	static int			SV_ImageIndex( const Str & name );
	static void			SV_WriteClientdataToMessage( client_t * client, sizebuf_t * msg );
	static void			SV_ExecuteUserCommand( const Str & uc, MessageBuffer & msg_buffer );
	static void			SV_InitOperatorCommands( );
	static void			SV_SendServerinfo( client_t * client );
	static void			SV_UserinfoChanged( client_t * cl );
	static void			Master_Heartbeat( );
	static void			Master_Shutdown( );
	static void			Master_Packet( );
	static void			SV_Shutdown( const Str & finalmsg, bool reconnect );
	static void			SV_PrepWorldFrame( );
	static const Str	SV_StatusString( );

	static void		SVC_Status( MessageBuffer & msg_buffer, netadr_t & msg_from );
	static void		SVC_Ack( MessageBuffer & msg_buffer, netadr_t & msg_from );
	static void		SVC_Info( MessageBuffer & msg_buffer, netadr_t & msg_from );
	static void		SVC_Ping( MessageBuffer & msg_buffer, netadr_t & msg_from );
	static void		SVC_GetChallenge( MessageBuffer & msg_buffer, netadr_t & msg_from );
	static void		SVC_DirectConnect( MessageBuffer & msg_buffer, netadr_t & msg_from );

	static bool		Rcon_Validate( );
	static void		SVC_RemoteCommand( MessageBuffer & msg_buffer, netadr_t & msg_from );
	static void		SV_ConnectionlessPacket( MessageBuffer & msg_buffer, netadr_t & msg_from );
	static void		SV_CalcPings( );
	static void		SV_GiveMsec( );
	static void		SV_ReadPackets( );
	static void		SV_CheckTimeouts( );
	static void		SV_RunGameFrame( );

	// sv_init.c
	static void		SV_InitGame( );
	static void		SV_Map( bool attractloop, const Str & levelstring, bool loadgame );
	static int		SV_FindIndex( const Str & name, int start, int max, bool create );
	static Str		SV_GetConfigName( int index );
	static void		SV_CreateBaseline( );
	static void		SV_CheckForSavegame( );
	static void		SV_SpawnServer( const Str & server, Str & spawnpoint, server_state_t serverstate, bool attractloop, bool loadgame );

	// sv_send.c
	//static void			SV_FlushRedirect( int sv_redirected, const Str & outputbuf );
	static void			SV_DemoCompleted( );
	static void			SV_SendClientMessages( );
	static void			SV_Multicast( Vec3 & origin, multicast_t to );
	static void			SV_StartSound( Vec3 * origin, Entity * entity, int channel, int soundindex, float volume, float attenuation, float timeofs );
	static void			SV_ClientPrintf( client_t * cl, int level, const char * fmt, ... );
	static void			SV_BroadcastPrintf( int level, const char * fmt, ...  );
	static void			SV_BroadcastCommand( const char * fmt, ... );
	static bool		SV_SendClientDatagram( client_t * client );
	static bool		SV_RateDrop( client_t * c );

	// sv_user.c
	static void		SV_Nextserver( );
	static void		SV_ExecuteClientMessage( client_t * cl, MessageBuffer & msg_buffer );
	static void		SV_BeginDemoserver( );
	static void		SV_New_f( MessageBuffer & msg_buffer );
	static void		SV_Configstrings_f( MessageBuffer & msg_buffer );
	static void		SV_Baselines_f( MessageBuffer & msg_buffer );
	static void		SV_Begin_f( MessageBuffer & msg_buffer );
	static void		SV_NextDownload_f( MessageBuffer & msg_buffer );
	static void		SV_BeginDownload_f( MessageBuffer & msg_buffer );
	static void		SV_Disconnect_f( MessageBuffer & msg_buffer );
	static void		SV_ShowServerinfo_f( MessageBuffer & msg_buffer );
	static void		SV_Nextserver_f( MessageBuffer & msg_buffer );
	static void		SV_ClientThink( client_t * cl, usercmd_t * cmd );

	// sv_ccmds.c
	static void			SV_ReadLevelFile( );
	static void			SV_Status_f( );
	// Specify a list of master servers
	static void		SV_SetMaster_f( );
	// Sets sv_client and sv_player to the player with num Cmd_Argv( 1 )
	static bool		SV_SetPlayer( );
	// Delete save/<XXX>/
	static void		SV_WipeSavegame( const Str & savename );
	static void		SV_CopySaveGame( const Str & src, const Str & dst );
	static void		SV_WriteLevelFile( );
	static void		SV_WriteServerFile( bool autosave );
	static void		SV_ReadServerFile( );
	// Puts the server in demo mode on a specific map/cinematic
	static void		SV_DemoMap_f( );
	// Saves the state of the map just being exited and goes to a new map. If the initial character of the map string is '*', the next map is
	// in a new unit, so the current savegame directory is cleared of map files.
	// Example: * inter.cin+jail
	// Clears the archived maps, plays the inter.cin cinematic, then goes to map jail.bsp.
	static void		SV_GameMap_f( );
	// Goes directly to a given map without any savegame archiving. For development work
	static void		SV_Map_f( );
	static void		SV_Loadgame_f( );
	static void		SV_Savegame_f( );
	// Kick a user off of the server
	static void		SV_Kick_f( );
	static void		SV_ConSay_f( );
	static void		SV_Heartbeat_f( );
	// Examine or change the serverinfo string
	static void		SV_Serverinfo_f( );
	// Examine all a users info strings
	static void		SV_DumpUser_f( );
	// Begins server demo recording.  Every entity and every message will be recorded, but no playerinfo will be stored.  Primarily for demo merging.
	static void		SV_ServerRecord_f( );
	// Ends server demo recording
	static void		SV_ServerStop_f( );
	// Kick everyone off, possibly in preparation for a new game
	static void		SV_KillServer_f( );
	// Let the game dll handle a command
	static void		SV_ServerCommand_f( );

	// sv_ents.c
	static void			SV_WriteFrameToClient( client_t * client, MessageBuffer & msg_buffer );
	static void			SV_RecordDemoMessage( );
	static void			SV_BuildClientFrame( client_t * client );
	static void			SV_WriteDeltaEntity( entity_state_t * from, entity_state_t * to, MessageBuffer & msg_buffer, bool force, bool newentity );
	static void			SV_EmitPacketEntities( client_frame_t * from, client_frame_t * to, MessageBuffer & msg_buffer );
	static void			SV_WritePlayerstateToClient( client_frame_t * from, client_frame_t * to, MessageBuffer & msg_buffer );
	static void			SV_FatPVS( Vec3 & org );

	// sv_game.c
	/// Sends the contents of the mutlicast buffer to a single client
	static void			PF_Unicast( Entity * ent, bool reliable );
	/// Print to a single client
	static void			PF_cprintf( Entity * ent, int level, const char * fmt, ... );
	/// centerprint to a single client
	static void			PF_centerprintf( Entity * ent, const char * fmt, ... );
	/// Abort the server with a game error
	static void			PF_error( const char * fmt, ... );
	/// Also sets mins and maxs for INLINE bmodels
	static void			PF_setmodel( Entity * ent, const Str & name );
	static void			PF_Configstring( int index, const Str & val );
	static void			PF_WriteChar( int c );
	static void			PF_WriteByte( int c );
	static void			PF_WriteShort( int c );
	static void			PF_WriteLong( int c );
	static void			PF_WriteFloat( float f );
	static void			PF_WriteString( const Str & s );
	static void			PF_WritePos( Vec3 & pos );
	static void			PF_WriteDir( Vec3 & dir );
	static void			PF_WriteAngle( float f );
	/// Also checks portalareas so that doors block sight
	static bool			PF_inPVS( Vec3 & p1, Vec3 & p2 );
	/// Also checks portalareas so that doors block sound
	static bool			PF_inPHS( Vec3 & p1, Vec3 & p2 );
	static void			PF_StartSound( Entity * entity, int channel, int sound_num, float volume, float attenuation, float timeofs );
	/// Init the game subsystem for a new map
	static void			SV_InitGameProgs( );
	/// Called when either the entire server is being killed, or it is changing to a different game directory.
	static void			SV_ShutdownGameProgs( );
	static void			SV_InitEdict( Entity * e );

	// sv_world.cpp
	// Builds a uniformly subdivided tree for the given world size
	static areanode_t *	SV_CreateAreaNode( int depth, Vec3 & mins, Vec3 & maxs );
	static void			SV_AreaEdicts_r( areanode_t * node );
	// Returns a headnode that can be used for testing or clipping an object of mins/maxs size.
	// Offset is filled in to contain the adjustment that must be added to the testing object's origin to get a point to use with the returned hull.
	static int			SV_HullForEntity( Entity * ent );
	static void			SV_ClipMoveToEntities( moveclip_t * clip );
	static void			SV_TraceBounds( Vec3 & start, Vec3 & mins, Vec3 & maxs, Vec3 & end, Vec3 * boxmins, Vec3 * boxmaxs );

	//============================================================

	//
	// high level object sorting to reduce interaction tests
	//

	static void			SV_ClearWorld( );
	// called after the world model has been loaded, before linking any entities

	static void			SV_UnlinkEdict( Entity * ent );
	// call before removing an entity, and before trying to move one, // so it doesn't clip against itself

	static void			SV_LinkEdict( Entity * ent );
	// Needs to be called any time an entity changes origin, mins, maxs, // or solid.  Automatically unlinks if needed.
	// sets ent->v.absmin and ent->v.absmax
	// sets ent->leafnums[] for pvs determination even if the entity
	// is not solid

	static int			SV_AreaEdicts( Vec3 & mins, Vec3 & maxs, Entity * *list, int maxcount, int areatype );
	// fills in a table of edict pointers with edicts that have bounding boxes
	// that intersect the given area.  It is possible for a non-axial bmodel
	// to be returned that doesn't actually intersect the area on an exact
	// test.
	// returns the number of pointers filled in
	// ??? does this always return the world?

	//===================================================================

	//
	// functions that interact with everything apropriate
	//
	static int			SV_PointContents( Vec3 & p );
	// returns the CONTENTS_* value from the world at the given point.
	// Quake 2 extends this to also check entities, to allow moving liquids

	// Moves the given mins/maxs volume through the world from start to end. Passedict and edicts owned by passedict are explicitly not checked.
	static CTrace			SV_Trace( Vec3 & start, Vec3 & mins, Vec3 & maxs, Vec3 & end, Entity * passedict, int contentmask );
	// mins and maxs are relative

	// if the entire move stays in a solid volume, trace.allsolid will be set, // trace.startsolid will be set, and trace.fraction will be 0

	// if the starting point is in a solid, it will be allowed to move out
	// to an open area

	// passedict is explicitly excluded from clipping checks( normally NULL )
};
