#include "precompiled.h"
#pragma hdrstop

Color color_white( 1.0f, 1.0f, 1.0f, 1.0f, false );
Color color_black( 0.0f, 0.0f, 0.0f, 1.0f, false );
Color color_darkgray( 0.25f, 0.25f, 0.25f, 1.0f, false );
Color color_gray( 0.5f, 0.5f, 0.5f, 1.0f, false );
Color color_lightgray( 0.75f, 0.75f, 0.75f, 1.0f, false );
Color color_transparent( 0.0f, 0.0f, 0.0f, 0.0f, false );
Color color_skyblue( 0.25f, 0.5f, 0.5f, 1.0f, false );
Color color_blue( 0.0f, 0.0f, 1.0f, 1.0f, false );
Color color_green( 0.0f, 1.0f, 0.0f, 1.0f, false );
Color color_red( 1.0f, 0.0f, 0.0f, 1.0f, false );
Color color_orange( 1.0f, 0.5f, 0.0f, 1.0f, false );
Color color_yellow( 1.0f, 1.0f, 0.0f, 1.0f, false );
Color color_cyan( 0.0f, 1.0f, 1.0f, 1.0f, false );
Color color_purple( 1.0f, 0.0f, 1.0f, 1.0f, false );

const Str null_string		= "";
const Str true_string		= "true";
const Str false_string		= "false";

DynamicBlockAlloc< char, 1 << 18, 128 >	g_stringDataAllocator;

CANamedList< CVar *, 256 >	CVarSystem::g_CVarList;

ServerPersistantData	Server::svs;				// persistant server info
ServerLocalData			Server::sv;					// local server

#ifndef DEDICATED_ONLY
Str Screen::scr_centerstring;
#endif

//sfx_t Sound::known_sfx[ MAX_SOUNDS * 2 ];

/***************************************
		CVARS	-	Common
***************************************/

CVar Common::userinfo_modified				( "userinfo_modified",		"true",			CVAR_BOOL,		"userinfo transmit flag" );
CVar Common::developer						( "developer",				"true",			CVAR_BOOL );
CVar Common::showclamp						( "showclamp",				"false",		CVAR_BOOL );
CVar Common::paused							( "paused",					"false",		CVAR_BOOL | CVAR_CHEAT );
CVar Common::timescale						( "timescale",				"1.0",			CVAR_FLOAT | CVAR_CHEAT );
CVar Common::fixedtime						( "fixedtime",				"0",			CVAR_INT | CVAR_CHEAT );
CVar Common::logfile_active					( "logfile",				"0",			CVAR_INT );
CVar Common::maxclients						( "maxclients",				"4",			CVAR_INT | CVAR_SERVERINFO | CVAR_LATCH );
CVar Common::vid_xpos						( "vid_xpos",				"3",			CVAR_INT | CVAR_ARCHIVE );
CVar Common::vid_ypos						( "vid_ypos",				"22",			CVAR_INT | CVAR_ARCHIVE );
CVar Common::vid_width						( "vid_width",				"1024",			CVAR_INT | CVAR_ARCHIVE );
CVar Common::vid_height						( "vid_height",				"768",			CVAR_INT | CVAR_ARCHIVE );
CVar Common::vid_fullscreen					( "vid_fullscreen",			"false",		CVAR_BOOL | CVAR_ARCHIVE );
CVar Common::vid_gamma						( "vid_gamma",				"1.0",			CVAR_FLOAT | CVAR_ARCHIVE );

#ifdef DEDICATED_ONLY
CVar Common::dedicated						( "dedicated",				"true",			CVAR_BOOL | CVAR_NOSET );
#else
CVar Common::dedicated						( "dedicated",				"false",		CVAR_BOOL | CVAR_NOSET );
#endif

CVar Common::build							( "build", "ver: "VERSION", arch: "CPUSTRING", compile date: "__DATE__" "__TIME__", desc: "BUILDSTRING, CVAR_STRING | CVAR_SERVERINFO | CVAR_NOSET );

/***************************************
		CVARS	-	Client
***************************************/

#ifndef DEDICATED_ONLY
CVar Client::adr0							( "adr0",					null_string,			CVAR_STRING | CVAR_ARCHIVE );
CVar Client::adr1							( "adr1",					null_string,			CVAR_STRING | CVAR_ARCHIVE );
CVar Client::adr2							( "adr2",					null_string,			CVAR_STRING | CVAR_ARCHIVE );
CVar Client::adr3							( "adr3",					null_string,			CVAR_STRING | CVAR_ARCHIVE );
CVar Client::adr4							( "adr4",					null_string,			CVAR_STRING | CVAR_ARCHIVE );
CVar Client::adr5							( "adr5",					null_string,			CVAR_STRING | CVAR_ARCHIVE );
CVar Client::adr6							( "adr6",					null_string,			CVAR_STRING | CVAR_ARCHIVE );
CVar Client::adr7							( "adr7",					null_string,			CVAR_STRING | CVAR_ARCHIVE );
CVar Client::adr8							( "adr8",					null_string,			CVAR_STRING | CVAR_ARCHIVE );
CVar Client::cl_gun							( "cl_gun",					"true",						CVAR_BOOL );
CVar Client::cl_footsteps					( "cl_footsteps",			"true",						CVAR_BOOL );
CVar Client::cl_noskins						( "cl_noskins",				"false",					CVAR_BOOL );
CVar Client::cl_predict						( "cl_predict",				"true",						CVAR_BOOL );
CVar Client::cl_maxfps						( "cl_maxfps",				"90",						CVAR_FLOAT );
CVar Client::cl_upspeed						( "cl_upspeed",				"200",						CVAR_INT );
CVar Client::cl_forwardspeed				( "cl_forwardspeed",		"200",						CVAR_INT );
CVar Client::cl_sidespeed					( "cl_sidespeed",			"200",						CVAR_INT );
CVar Client::cl_yawspeed					( "cl_yawspeed",			"140",						CVAR_INT );
CVar Client::cl_pitchspeed					( "cl_pitchspeed",			"150",						CVAR_INT );
CVar Client::cl_anglespeedkey				( "cl_anglespeedkey",		"1.5",						CVAR_FLOAT );
CVar Client::cl_run							( "cl_run",					"false",					CVAR_BOOL | CVAR_ARCHIVE );
CVar Client::cl_shownet						( "cl_shownet",				"0",						CVAR_INT );
CVar Client::cl_showmiss					( "cl_showmiss",			"false",					CVAR_BOOL );
CVar Client::cl_timeout						( "cl_timeout",				"120",						CVAR_INT );
CVar Client::cl_timedemo					( "timedemo",				"false",					CVAR_BOOL | CVAR_CHEAT );

//
// userinfo
//
CVar Client::info_spectator					( "spectator",				"false",					CVAR_BOOL | CVAR_USERINFO );
CVar Client::name							( "name",					"Anonymous",				CVAR_STRING | CVAR_USERINFO | CVAR_ARCHIVE );
CVar Client::model							( "model",					"players/male/tris.md2",	CVAR_STRING | CVAR_USERINFO | CVAR_ARCHIVE );
CVar Client::weapon							( "weapon",					"players/male/weapon.md2",	CVAR_STRING | CVAR_USERINFO | CVAR_ARCHIVE );
CVar Client::skin							( "skin",					"players/male/grunt.pcx",	CVAR_STRING | CVAR_USERINFO | CVAR_ARCHIVE );
CVar Client::icon							( "icon",					"players/male/grunt_i.pcx",	CVAR_STRING | CVAR_USERINFO | CVAR_ARCHIVE );
CVar Client::rate							( "rate",					"25000",					CVAR_INT | CVAR_USERINFO | CVAR_ARCHIVE ); // FIXME
CVar Client::msg							( "msg",					"1",						CVAR_INT | CVAR_USERINFO | CVAR_ARCHIVE );
CVar Client::hand							( "hand",					"0",						CVAR_INT | CVAR_USERINFO | CVAR_ARCHIVE );
CVar Client::fov							( "fov",					"90.0",						CVAR_FLOAT | CVAR_USERINFO | CVAR_ARCHIVE );

CVar Client::cl_vwep						( "cl_vwep",				"true",						CVAR_BOOL | CVAR_ARCHIVE );
CVar Client::cl_nodelta						( "cl_nodelta",				"false",					CVAR_BOOL );

#endif

/***************************************
		CVARS	-	Game
***************************************/

CVar Game::gun_x							( "gun_x",				"0.0",			CVAR_FLOAT,		"view_right" );
CVar Game::gun_y							( "gun_y",				"0.0",			CVAR_FLOAT,		"view_forward" );
CVar Game::gun_z							( "gun_z",				"0.0",			CVAR_FLOAT,		"view_up" );

//FIXME: sv_ prefix is wrong for these
CVar Game::sv_rollspeed						( "sv_rollspeed",		"200.0",		CVAR_FLOAT,		"roll hacking value" );
CVar Game::sv_rollangle						( "sv_rollangle",		"2.0",			CVAR_FLOAT,		"counterpart of sv_rollspeed?" );
CVar Game::sv_maxvelocity					( "sv_maxvelocity",		"2000.0",		CVAR_FLOAT,		"ranges all velocities to [ -sv_maxvelocity; sv_maxvelocity ]" );
CVar Game::sv_gravity						( "sv_gravity",			"800",			CVAR_INT,		"axis-z gravity value" );

// latched vars
CVar Game::sv_cheats						( "cheats",				"false",		CVAR_BOOL | CVAR_SERVERINFO | CVAR_LATCH,		"enables fun mode" );
CVar Game::gamename							( "gamename",			GAMEVERSION,	CVAR_SERVERINFO | CVAR_LATCH );
CVar Game::gamedate							( "gamedate",			__DATE__,		CVAR_SERVERINFO | CVAR_LATCH );

CVar Game::maxspectators					( "maxspectators",		"16",			CVAR_INT | CVAR_SERVERINFO );
CVar Game::deathmatch						( "deathmatch",			"false",		CVAR_BOOL | CVAR_LATCH,			"enables dm mode" );
CVar Game::coop								( "coop",				"false",		CVAR_BOOL | CVAR_LATCH,			"enables cooperative mode" );
CVar Game::skill							( "skill",				"1",			CVAR_INT | CVAR_LATCH,			"affects ai behaviour and response time", 0.0f, 4.0f );
CVar Game::maxentities						( "maxentities",		"1024",			CVAR_INT | CVAR_LATCH,			"maximum num of ents" );

// change anytime vars
CVar Game::dmflags							( "dmflags",			"16",			CVAR_INT | CVAR_SERVERINFO,		"a nice flaggable integer for disabling items" );
CVar Game::fraglimit						( "fraglimit",			"0",			CVAR_INT | CVAR_SERVERINFO );
CVar Game::timelimit						( "timelimit",			"0.0",			CVAR_FLOAT | CVAR_SERVERINFO,	"time limit in minutes" );
CVar Game::password							( "password",			"none",			CVAR_STRING | CVAR_USERINFO,	"user pass" );
CVar Game::spectator_password				( "spectator_password", "none",			CVAR_STRING | CVAR_USERINFO,	"spectator pass" );
CVar Game::needpass							( "needpass",			"0",			CVAR_INT | CVAR_SERVERINFO );
CVar Game::filterban						( "filterban",			"true",			CVAR_BOOL );

CVar Game::g_select_empty					( "g_select_empty",		"false",		CVAR_BOOL | CVAR_ARCHIVE );

CVar Game::run_pitch						( "run_pitch",			"0.002",		CVAR_FLOAT );
CVar Game::run_roll							( "run_roll",			"0.005",		CVAR_FLOAT );
CVar Game::bob_pitch						( "bob_pitch",			"0.002",		CVAR_FLOAT );
CVar Game::bob_roll							( "bob_roll",			"0.002",		CVAR_FLOAT );

// flood control
CVar Game::flood_msgs						( "flood_msgs",			"4",			CVAR_INT,			"flood protection msg count" );
CVar Game::flood_persecond					( "flood_persecond",	"4.0",			CVAR_FLOAT,			"check time for msg count in secs" );
CVar Game::flood_waitdelay					( "flood_waitdelay",	"10.0",			CVAR_FLOAT,			"flooder shut up time in secs" );

// dm map list
CVar Game::sv_maplist						( "sv_maplist",			null_string,	CVAR_STRING );

CVar Game::g_defplayer						( "g_defplayer",	"name = Anonymous, model = players/male/tris.md2, weapon = players/male/weapon.md2, skin = players/male/grunt.pcx, icon = players/male/grunt_i.pcx",	CVAR_STRING );

/***************************************
		CVARS	-	network, misc
***************************************/

//CVar Thread::debugThread					( "debug_thread",			"true",			CVAR_BOOL | CVAR_ARCHIVE,		"enabling thread creation trace messages" );

CVar FileSystem::fs_gamedir					( "fs_gamedir",				"fs",			CVAR_STRING | CVAR_INIT,		"directory of game files" );

CVar Network::net_noudp						( "noudp",					"false",		CVAR_BOOL | CVAR_NOSET );
CVar Network::net_ip						( "ip",						"localhost",	CVAR_STRING | CVAR_NOSET );
CVar Network::net_hostport					( "hostport",				"0",			CVAR_INT | CVAR_NOSET );
CVar Network::net_clientport				( "clientport",				"0",			CVAR_INT | CVAR_NOSET );
CVar Network::net_forceLatency				( "net_forceLatency",		"0",			CVAR_INT | CVAR_NOSET );
CVar Network::net_forceDrop					( "net_forceDrop",			"0",			CVAR_INT | CVAR_NOSET );

CVar Netchan::showpackets					( "showpackets",			"false",		CVAR_BOOL );
CVar Netchan::showdrop						( "showdrop",				"false",		CVAR_BOOL );
CVar Netchan::qport							( "qport",					"0",			CVAR_INT | CVAR_NOSET );

#ifndef DEDICATED_ONLY

/***************************************
		CVARS	-	MaterialSystem
***************************************/

//CVar MaterialSystem::g_matGuiDivisor			( "matGuiDivisor",			"2.0",							CVAR_FLOAT );

/***************************************
		CVARS	-	Renderer
***************************************/

//CVar Renderer::g_renSleep					( "renSleep",				"0",			CVAR_INT );
//CVar Renderer::g_pvsLock					( "pvsLock",				"false",		CVAR_BOOL );
//CVar Renderer::g_brushMark					( "brushMark",				"-1",			CVAR_INT );
//CVar Renderer::g_pvsMark					( "pvsMark",				null_string,	CVAR_STRING );
//CVar Renderer::g_shadowEnable				( "shadow",					"1",			CVAR_INT );
//CVar Renderer::g_ssaoEnable					( "ssao",					"1",			CVAR_INT );
//CVar Renderer::g_fsaaEnable					( "fsaa",					"1",			CVAR_INT );
//CVar Renderer::g_utilDraw					( "utilb",					"0",			CVAR_INT );
//CVar Renderer::g_shadowMapStaticSize		( "smss",					"0",			CVAR_INT | CVAR_ARCHIVE );
//CVar Renderer::g_shadowMapDynamicSize		( "smds",					"0",			CVAR_INT | CVAR_ARCHIVE );

/***************************************
		CVARS	-	Screen
***************************************/

CVar Screen::scr_viewsize					( "viewsize",				"100",		CVAR_INT | CVAR_ARCHIVE );
CVar Screen::scr_conspeed					( "scr_conspeed",			"3.0",		CVAR_FLOAT );
CVar Screen::scr_showpause					( "scr_showpause",			"true",		CVAR_BOOL );
CVar Screen::scr_centertime					( "scr_centertime",			"2.5",		CVAR_FLOAT );
CVar Screen::scr_netgraph					( "netgraph",				"false",	CVAR_BOOL );
CVar Screen::scr_timegraph					( "timegraph",				"false",	CVAR_BOOL );
CVar Screen::scr_debuggraph					( "debuggraph",				"false",	CVAR_BOOL );
CVar Screen::scr_graphheight				( "graphheight",			"32",		CVAR_INT );
CVar Screen::scr_graphscale					( "graphscale",				"1.0",		CVAR_FLOAT );
CVar Screen::scr_graphshift					( "graphshift",				"0.0",		CVAR_FLOAT );

/***************************************
		CVARS	-	Sound
***************************************/

CVar SoundSystem::snd_captureid				( "snd_captureid",		"0",			CVAR_INT | CVAR_ARCHIVE,	"OpenAL capture device id" );

/*CVar Sound::s_initsound						( "s_initsound",		"true",			CVAR_BOOL );
CVar Sound::s_volume						( "s_volume",			"0.7",			CVAR_FLOAT | CVAR_ARCHIVE );
CVar Sound::s_khz							( "s_khz",				"44",			CVAR_INT | CVAR_ARCHIVE );
CVar Sound::s_loadas8bit					( "s_loadas8bit",		"false",		CVAR_BOOL | CVAR_ARCHIVE );
CVar Sound::s_mixahead						( "s_mixahead",			"0.2",			CVAR_FLOAT | CVAR_ARCHIVE );
CVar Sound::s_show							( "s_show",				"false",		CVAR_BOOL );
CVar Sound::s_testsound						( "s_testsound",		"false",		CVAR_BOOL );
CVar Sound::s_primary						( "s_primary",			"false",		CVAR_BOOL | CVAR_ARCHIVE ); // win32 specific
CVar Sound::s_wavonly						( "s_wavonly",			"false",		CVAR_BOOL );*/

/***************************************
		CVARS	-	View
***************************************/

CVar View::crosshair						( "crosshair",			"0",		CVAR_INT | CVAR_ARCHIVE,	"2d crosshair", 0.0f, 3.0f );
CVar View::cl_testblend						( "cl_testblend",		"false",	CVAR_BOOL,					"full screen blend effect" );
CVar View::cl_testparticles					( "cl_testparticles",	"0.0",		CVAR_FLOAT,					"create 4096 particles in the view", 0.0f, 1.0f );
CVar View::cl_testentities					( "cl_testentities",	"false",	CVAR_BOOL,					"create 32 player models" );
CVar View::cl_testlights					( "cl_testlights",		"false",	CVAR_BOOL | CVAR_CHEAT,		"create 32 lights models" );
CVar View::cl_stats							( "cl_stats",			"false",	CVAR_BOOL,					"prints r_numentities, r_numdlights and r_numparticles per frame" );
CVar View::cl_add_blend						( "cl_blend",			"true",		CVAR_BOOL,					"enables blend" );
CVar View::cl_add_lights					( "cl_lights",			"true",		CVAR_BOOL,					"enables lights" );
CVar View::cl_add_particles					( "cl_particles",		"true",		CVAR_BOOL,					"enables particles" );
CVar View::cl_add_entities					( "cl_entities",		"true",		CVAR_BOOL,					"enables entities" );

/***************************************
		CVARS	-	Input
***************************************/

CVar Input::freelook						( "freelook",				"true",			CVAR_BOOL | CVAR_ARCHIVE );
CVar Input::lookspring						( "lookspring",				"false",		CVAR_BOOL | CVAR_ARCHIVE );
CVar Input::lookstrafe						( "lookstrafe",				"false",		CVAR_BOOL | CVAR_ARCHIVE );
CVar Input::sensitivity						( "sensitivity",			"3.0",			CVAR_FLOAT | CVAR_ARCHIVE );

CVar Input::m_pitch							( "m_pitch",				"0.022",		CVAR_FLOAT | CVAR_ARCHIVE );
CVar Input::m_yaw							( "m_yaw",					"0.022",		CVAR_FLOAT );
CVar Input::m_forward						( "m_forward",				"1",			CVAR_INT );
CVar Input::m_side							( "m_side",					"1",			CVAR_INT );

CVar Input::m_filter						( "m_filter",				"false",		CVAR_BOOL,		"interpolates mouse coords" );
CVar Input::in_mouse						( "in_mouse",				"true",			CVAR_BOOL | CVAR_ARCHIVE,	"enables mouse" );

// joystick variables
CVar Input::in_joystick						( "in_joystick",			"false",		CVAR_BOOL | CVAR_ARCHIVE,	"enables joystick" );

// none of these cvars are saved over a session
// this means that advanced controller configuration needs to be executed
// each time.  this avoids any problems with getting back to a default usage
// or when changing from one controller to another.  this way at least something
// works.

CVar Input::joy_name						( "joy_name",				"joystick",		CVAR_STRING );
CVar Input::joy_advanced					( "joy_advanced",			"false",		CVAR_BOOL );
CVar Input::joy_advaxisx					( "joy_advaxisx",			"0",			CVAR_INT );
CVar Input::joy_advaxisy					( "joy_advaxisy",			"0",			CVAR_INT );
CVar Input::joy_advaxisz					( "joy_advaxisz",			"0",			CVAR_INT );
CVar Input::joy_advaxisr					( "joy_advaxisr",			"0",			CVAR_INT );
CVar Input::joy_advaxisu					( "joy_advaxisu",			"0",			CVAR_INT );
CVar Input::joy_advaxisv					( "joy_advaxisv",			"0",			CVAR_INT );
CVar Input::joy_forwardthreshold			( "joy_forwardthreshold",	"0.15",			CVAR_FLOAT );
CVar Input::joy_sidethreshold				( "joy_sidethreshold",		"0.15",			CVAR_FLOAT );
CVar Input::joy_upthreshold					( "joy_upthreshold",		"0.15",			CVAR_FLOAT );
CVar Input::joy_pitchthreshold				( "joy_pitchthreshold",		"0.15",			CVAR_FLOAT );
CVar Input::joy_yawthreshold				( "joy_yawthreshold",		"0.15",			CVAR_FLOAT );
CVar Input::joy_forwardsensitivity			( "joy_forwardsensitivity",	"-1.0",			CVAR_FLOAT );
CVar Input::joy_sidesensitivity				( "joy_sidesensitivity",	"-1.0",			CVAR_FLOAT );
CVar Input::joy_upsensitivity				( "joy_upsensitivity",		"-1.0",			CVAR_FLOAT );
CVar Input::joy_pitchsensitivity			( "joy_pitchsensitivity",	"1.0",			CVAR_FLOAT );
CVar Input::joy_yawsensitivity				( "joy_yawsensitivity",		"-1.0",			CVAR_FLOAT );

#endif

/***************************************
		CVARS	-	Server
***************************************/

CVar Server::hostname						( "hostname",					"noname",						CVAR_STRING | CVAR_SERVERINFO | CVAR_ARCHIVE );
CVar Server::timeout						( "timeout",					"125",							CVAR_INT );
CVar Server::zombietime						( "zombietime",					"2",							CVAR_INT );
CVar Server::sv_enforcetime					( "sv_enforcetime",				"false",						CVAR_BOOL );
CVar Server::allow_download					( "allow_download",				"true",							CVAR_BOOL | CVAR_ARCHIVE );
CVar Server::allow_download_players			( "allow_download_players",		"false",						CVAR_BOOL | CVAR_ARCHIVE );
CVar Server::allow_download_models			( "allow_download_models",		"true",							CVAR_BOOL | CVAR_ARCHIVE );
CVar Server::allow_download_sounds			( "allow_download_sounds",		"true",							CVAR_BOOL | CVAR_ARCHIVE );
CVar Server::allow_download_maps			( "allow_download_maps",		"true",							CVAR_BOOL | CVAR_ARCHIVE );
CVar Server::sv_noreload					( "sv_noreload",				"false",						CVAR_BOOL );
CVar Server::sv_airaccelerate				( "sv_airaccelerate",			"0.0",							CVAR_FLOAT | CVAR_LATCH );
CVar Server::public_server					( "public",						"false",						CVAR_BOOL );
CVar Server::sv_reconnect_limit				( "sv_reconnect_limit",			"3",							CVAR_INT | CVAR_ARCHIVE );
CVar Server::protocol						( "protocol",					va( "%i", PROTOCOL_VERSION ),	CVAR_INT | CVAR_SERVERINFO | CVAR_NOSET );
CVar Server::sv_mapname						( "mapname",					null_string,				CVAR_STRING | CVAR_SERVERINFO | CVAR_NOSET );

#ifndef DEDICATED_ONLY

/***************************************
		SVARS
***************************************/

DynamicBlockAlloc< byte, 1 << 18, 128 >	SVar::g_shaderDataAllocator;
CANamedList< SVar *, 256 >				SVar::g_shaderVariableList;

SVarMat4		Renderer::g_mvpMatrix		( "mvpMatrix" );
SVarMat4		Renderer::g_viewMatrix		( "viewMatrix" );
SVarMat4		Renderer::g_normalMatrix	( "normalMatrix" );
SVarMat4		Renderer::g_uiMatrix		( "uiMatrix" );
SVarVec3		Renderer::g_cameraOrigin	( "cameraOrigin" );
SVarVec3		Renderer::g_lightOrigin		( "lightOrigin" );
SVarVec3		Renderer::g_lightColor		( "lightColor", Vec3( 1.0f, 1.0f, 1.0f ) );
SVarFloat		Renderer::g_lightIntensity	( "lightIntensity" );
SVarPlane		Renderer::g_sunPlane		( "sunPlane" );
SVarVec4		Renderer::g_debugColor		( "debugColor", Vec4( 0.5f, 0.5f, 0.5f, 0.5f ) );

#endif

/***************************************
		ITEMS
***************************************/

gitem_armor_t Game::jacketarmor_info = { 25, 50, .30f, .00f, ARMOR_JACKET };
gitem_armor_t Game::combatarmor_info = { 50, 100, .60f, .30f, ARMOR_COMBAT };
gitem_armor_t Game::bodyarmor_info = { 100, 200, .80f, .60f, ARMOR_BODY };

GameItem Game::itemlist[ ] = {

	{ NULL }, // leave index 0 alone

	/* weapon_blaster( .3 .3 1 )( -16 -16 -16 )( 16 16 16 ) always owned, never in the world */
	{
		"weapon_blaster",
		NULL,
		Use_Weapon,
		NULL,
		Weapon_Blaster,
		NULL,//"misc/w_pkup.wav",
		NULL,
		0,
		NULL,//"models/weapons/v_blast/tris.md2",
		/* icon */ NULL,//"pics/w_blaster.pcx",
		/* pickup */ "Blaster",
		0,
		0,
		NULL,
		IT_WEAPON | IT_STAY_COOP,
		WEAP_BLASTER,
		NULL,
		0,
		/* precache */ NULL//"weapons/blastf1a.wav, misc/lasfly.wav"
	},	
	{
		NULL,
		Pickup_Health,
		NULL,
		NULL,
		NULL,
		NULL,//"items/pkup.wav",
		NULL,
		0,
		NULL,
		/* icon */ NULL,//"pics/i_health.pcx",
		/* pickup */ "Health",
		/* width */ 3,
		0,
		NULL,
		0,
		0,
		NULL,
		0,
		/* precache */ NULL//"items/s_health.wav, items/n_health.wav, items/l_health.wav, items/m_health.wav"
	},
	{ NULL } // end of list marker
};

void Game::InitItems( ) {

	num_items = sizeof( itemlist ) / sizeof( GameItem ) - 1;
}

/***************************************

		Str	Menu::constructors

***************************************/

Str Game::helpmessage1 = null_string;
Str Game::helpmessage2 = null_string;
Str Game::spawnpoint = null_string;

#ifndef DEDICATED_ONLY

ClientStaticData	Client::cls;
ClientStateData		Client::cl;

centity_t			Client::cl_entities[ MAX_EDICTS ];

const Str Client::env_suf[ 6 ] = { "rt", "bk", "lf", "ft", "up", "dn" };

const Str Client::svc_strings[ ] = {

	"svc_bad", 
	"svc_muzzleflash",
	"svc_muzzlflash2",
	"svc_temp_entity",
	"svc_layout",
	"svc_inventory",
	"svc_nop",
	"svc_disconnect",
	"svc_reconnect",
	"svc_sound",
	"svc_print",
	"svc_stufftext",
	"svc_serverdata",
	"svc_configstring",
	"svc_spawnbaseline",
	"svc_centerprint",
	"svc_download",
	"svc_playerinfo",
	"svc_packetentities",
	"svc_deltapacketentities",
	"svc_frame"
};

List< Str > View::cl_weaponmodels;

Str					Keyboard::g_bindings[ 256 ];

#endif

keyTranslation_t KeyEvent::keyTranslationTable[ ] = {

	"key_esc",				key_esc,				VK_ESCAPE,

	"key_f1",				key_f1,					VK_F1,
	"key_f2",				key_f2,					VK_F2,
	"key_f3",				key_f3,					VK_F3,
	"key_f4",				key_f4,					VK_F4,
	"key_f5",				key_f5,					VK_F5,
	"key_f6",				key_f6,					VK_F6,
	"key_f7",				key_f7,					VK_F7,
	"key_f8",				key_f8,					VK_F8,
	"key_f9",				key_f9,					VK_F9,
	"key_f10",				key_f10,				VK_F10,
	"key_f11",				key_f11,				VK_F11,
	"key_f12",				key_f12,				VK_F12,

	"key_space",			key_space,				VK_SPACE,	
	"key_tab",				key_tab,				VK_TAB,
	"key_backspace",		key_backspace,			VK_BACK,
	"key_return",			key_return,				VK_RETURN,

	"key_caps_lock",		key_caps_lock,			VK_CAPITAL,
	"key_shift",			key_shift,				VK_SHIFT,
	"key_ctrl",				key_ctrl,				VK_CONTROL,
	"key_alt",				key_alt,				VK_MENU,
	"key_left_shift",		key_left_shift,			VK_LSHIFT,
	"key_left_ctrl",		key_left_ctrl,			VK_LCONTROL,
	"key_left_alt",			key_left_alt,			VK_LMENU,
	"key_right_shift",		key_right_shift,		VK_RSHIFT,
	"key_right_ctrl",		key_right_ctrl,			VK_RCONTROL,
	"key_right_alt",		key_right_alt,			VK_RMENU,

	"key_comma",			key_comma,				VK_OEM_COMMA,
	"key_dot",				key_dot,				VK_OEM_PERIOD,
	"key_minus",			key_minus,				VK_OEM_MINUS,
	"key_plus",				key_plus,				VK_OEM_PLUS,

	"key_oem_1",			key_oem_1,				VK_OEM_1,
	"key_oem_2",			key_oem_2,				VK_OEM_2,
	"key_oem_3",			key_oem_3,				VK_OEM_3,
	"key_oem_4",			key_oem_4,				VK_OEM_4,
	"key_oem_5",			key_oem_5,				VK_OEM_5,
	"key_oem_6",			key_oem_6,				VK_OEM_6,
	"key_oem_7",			key_oem_7,				VK_OEM_7,
	"key_oem_8",			key_oem_8,				VK_OEM_8,
	"key_oem_ax",			key_oem_ax,				VK_OEM_AX,
	"key_oem_102",			key_oem_102,			VK_OEM_102,

	"key_0",				key_0,					'0',
	"key_1",				key_1,					'1',
	"key_2",				key_2,					'2',
	"key_3",				key_3,					'3',
	"key_4",				key_4,					'4',
	"key_5",				key_5,					'5',
	"key_6",				key_6,					'6',
	"key_7",				key_7,					'7',
	"key_8",				key_8,					'8',
	"key_9",				key_9,					'9',

	"key_a",				key_a,					'A',
	"key_b",				key_b,					'B',
	"key_c",				key_c,					'C',
	"key_d",				key_d,					'D',
	"key_e",				key_e,					'E',
	"key_f",				key_f,					'F',
	"key_g",				key_g,					'G',
	"key_h",				key_h,					'H',
	"key_i",				key_i,					'I',
	"key_j",				key_j,					'J',
	"key_k",				key_k,					'K',
	"key_l",				key_l,					'L',
	"key_m",				key_m,					'M',
	"key_n",				key_n,					'N',
	"key_o",				key_o,					'O',
	"key_p",				key_p,					'P',
	"key_q",				key_q,					'Q',
	"key_r",				key_r,					'R',
	"key_s",				key_s,					'S',
	"key_t",				key_t,					'T',
	"key_u",				key_u,					'U',
	"key_v",				key_v,					'V',
	"key_w",				key_w,					'W',
	"key_x",				key_x,					'X',
	"key_y",				key_y,					'Y',
	"key_z",				key_z,					'Z',

	"key_printscreen",		key_printscreen,		VK_SNAPSHOT,
	"key_scroll_lock",		key_scroll_lock,		VK_SCROLL,
	"key_pause",			key_pause,				VK_PAUSE,

	"key_insert",			key_insert,				VK_INSERT,
	"key_delete",			key_delete,				VK_DELETE,
	"key_home",				key_home,				VK_HOME,
	"key_end",				key_end,				VK_END,
	"key_pageup",			key_pageup,				VK_PRIOR,
	"key_pagedown",			key_pagedown,			VK_NEXT,

	"key_left",				key_left,				VK_LEFT,
	"key_up",				key_up,					VK_UP,
	"key_down",				key_down,				VK_DOWN,
	"key_right",			key_right,				VK_RIGHT,

	"key_num_lock",			key_num_lock,			VK_NUMLOCK,
	"key_num_0",			key_num_0,				VK_NUMPAD0,
	"key_num_1",			key_num_1,				VK_NUMPAD1,
	"key_num_2",			key_num_2,				VK_NUMPAD2,
	"key_num_3",			key_num_3,				VK_NUMPAD3,
	"key_num_4",			key_num_4,				VK_NUMPAD4,
	"key_num_5",			key_num_5,				VK_NUMPAD5,
	"key_num_6",			key_num_6,				VK_NUMPAD6,
	"key_num_7",			key_num_7,				VK_NUMPAD7,
	"key_num_8",			key_num_8,				VK_NUMPAD8,
	"key_num_9",			key_num_9,				VK_NUMPAD9,
	"key_num_slash",		key_num_slash,			VK_DIVIDE,
	"key_num_star",			key_num_star,			VK_MULTIPLY,
	"key_num_minus",		key_num_minus,			VK_SUBTRACT,
	"key_num_plus",			key_num_plus,			VK_ADD,
	"key_num_return",		key_num_return,			VK_SEPARATOR,
	"key_num_comma",		key_num_comma,			VK_DECIMAL,

	"mouse_left",			mouse_left,				VK_LBUTTON,
	"mouse_right",			mouse_right,			VK_RBUTTON,
	"mouse_middle",			mouse_middle,			VK_MBUTTON,
	"mouse_button_4",		mouse_button_4,			0,
	"mouse_button_5",		mouse_button_5,			0,
	"mouse_wheelup",		mouse_wheelup,			VK_XBUTTON1,
	"mouse_wheeldown",		mouse_wheeldown,		VK_XBUTTON2,
	"mouse_wheelleft",		mouse_wheelleft,		0,
	"mouse_wheelright",		mouse_wheelright,		0,

	null_string,			vs_none,				0
};

const field_t Game::fields[ ] = {

	{ "classname", FOFS( classname ), F_LSTRING },
	{ "name", FOFS( name ), F_LSTRING },
	{ "model", FOFS( model ), F_LSTRING },
	{ "spawnflags", FOFS( spawnflags ), F_INT },
	{ "speed", FOFS( speed ), F_FLOAT },
	{ "accel", FOFS( accel ), F_FLOAT },
	{ "decel", FOFS( decel ), F_FLOAT },
	{ "target", FOFS( target ), F_LSTRING },
	{ "targetname", FOFS( targetname ), F_LSTRING },
	{ "pathtarget", FOFS( pathtarget ), F_LSTRING },
	{ "deathtarget", FOFS( deathtarget ), F_LSTRING },
	{ "killtarget", FOFS( killtarget ), F_LSTRING },
	{ "combattarget", FOFS( combattarget ), F_LSTRING },
	{ "message", FOFS( message ), F_LSTRING },
	{ "team", FOFS( team ), F_LSTRING },
	{ "wait", FOFS( wait ), F_DOUBLE },
	{ "delay", FOFS( delay ), F_DOUBLE },
	{ "random", FOFS( random ), F_DOUBLE }, 
	{ "move_origin", FOFS( move_origin ), F_VECTOR },
	{ "move_angles", FOFS( move_angles ), F_VECTOR },
	{ "style", FOFS( style ), F_INT },
	{ "count", FOFS( count ), F_INT },
	{ "health", FOFS( health ), F_INT },
	{ "sounds", FOFS( sounds ), F_INT },
	{ "light", 0, F_IGNORE },
	{ "dmg", FOFS( dmg ), F_INT },
	{ "mass", FOFS( mass ), F_INT },
	{ "volume", FOFS( volume ), F_FLOAT },
	{ "attenuation", FOFS( attenuation ), F_FLOAT },
	{ "map", FOFS( map ), F_LSTRING },
	{ "origin", FOFS( s.origin ), F_VECTOR },
	{ "angles", FOFS( s.angles ), F_VECTOR },
	{ "angle", FOFS( s.angles ), F_ANGLEHACK },
	{ "goalentity", FOFS( goalentity ), F_EDICT, FFL_NOSPAWN },
	{ "movetarget", FOFS( movetarget ), F_EDICT, FFL_NOSPAWN },
	{ "enemy", FOFS( enemy ), F_EDICT, FFL_NOSPAWN },
	{ "oldenemy", FOFS( oldenemy ), F_EDICT, FFL_NOSPAWN },
	{ "activator", FOFS( activator ), F_EDICT, FFL_NOSPAWN },
	{ "groundentity", FOFS( groundentity ), F_EDICT, FFL_NOSPAWN },
	{ "teamchain", FOFS( teamchain ), F_EDICT, FFL_NOSPAWN },
	{ "teammaster", FOFS( teammaster ), F_EDICT, FFL_NOSPAWN },
	{ "owner", FOFS( owner ), F_EDICT, FFL_NOSPAWN },
	{ "mynoise", FOFS( mynoise ), F_EDICT, FFL_NOSPAWN },
	{ "mynoise2", FOFS( mynoise2 ), F_EDICT, FFL_NOSPAWN },
	{ "target_ent", FOFS( target_ent ), F_EDICT, FFL_NOSPAWN },
	{ "chain", FOFS( chain ), F_EDICT, FFL_NOSPAWN }, 
	{ "prethink", FOFS( prethink ), F_FUNCTION, FFL_NOSPAWN },
	{ "think", FOFS( think ), F_FUNCTION, FFL_NOSPAWN },
	{ "blocked", FOFS( blocked ), F_FUNCTION, FFL_NOSPAWN },
	{ "touch", FOFS( touch ), F_FUNCTION, FFL_NOSPAWN },
	{ "use", FOFS( use ), F_FUNCTION, FFL_NOSPAWN },
	{ "pain", FOFS( pain ), F_FUNCTION, FFL_NOSPAWN },
	{ "die", FOFS( die ), F_FUNCTION, FFL_NOSPAWN }, 
	{ "stand", FOFS( monsterinfo.stand ), F_FUNCTION, FFL_NOSPAWN },
	{ "le", FOFS( monsterinfo.le ), F_FUNCTION, FFL_NOSPAWN },
	{ "search", FOFS( monsterinfo.search ), F_FUNCTION, FFL_NOSPAWN },
	{ "walk", FOFS( monsterinfo.walk ), F_FUNCTION, FFL_NOSPAWN },
	{ "run", FOFS( monsterinfo.run ), F_FUNCTION, FFL_NOSPAWN },
	{ "dodge", FOFS( monsterinfo.dodge ), F_FUNCTION, FFL_NOSPAWN },
	{ "attack", FOFS( monsterinfo.attack ), F_FUNCTION, FFL_NOSPAWN },
	{ "melee", FOFS( monsterinfo.melee ), F_FUNCTION, FFL_NOSPAWN },
	{ "sight", FOFS( monsterinfo.sight ), F_FUNCTION, FFL_NOSPAWN },
	{ "checkattack", FOFS( monsterinfo.checkattack ), F_FUNCTION, FFL_NOSPAWN },
	{ "currentmove", FOFS( monsterinfo.currentmove ), F_MMOVE, FFL_NOSPAWN }, 
	{ "endfunc", FOFS( moveinfo.endfunc ), F_FUNCTION, FFL_NOSPAWN },

	// temp spawn vars -- only valid when the spawn function is called
	{ "lip", STOFS( lip ), F_INT, FFL_SPAWNTEMP },
	{ "distance", STOFS( distance ), F_INT, FFL_SPAWNTEMP },
	{ "height", STOFS( height ), F_INT, FFL_SPAWNTEMP },
	{ "noise", STOFS( noise ), F_LSTRING, FFL_SPAWNTEMP },
	{ "pausetime", STOFS( pausetime ), F_DOUBLE, FFL_SPAWNTEMP },
	{ "item", STOFS( item ), F_LSTRING, FFL_SPAWNTEMP },

	//need for item field in edict struct, FFL_SPAWNTEMP item will be skipped on saves
	{ "item", FOFS( item ), F_ITEM }, 
	{ "gravity", STOFS( gravity ), F_LSTRING, FFL_SPAWNTEMP },
	{ "sky", STOFS( sky ), F_LSTRING, FFL_SPAWNTEMP },
	{ "skyrotate", STOFS( skyrotate ), F_FLOAT, FFL_SPAWNTEMP },
	{ "skyaxis", STOFS( skyaxis ), F_VECTOR, FFL_SPAWNTEMP },
	{ "minyaw", STOFS( minyaw ), F_FLOAT, FFL_SPAWNTEMP },
	{ "maxyaw", STOFS( maxyaw ), F_FLOAT, FFL_SPAWNTEMP },
	{ "minpitch", STOFS( minpitch ), F_FLOAT, FFL_SPAWNTEMP },
	{ "maxpitch", STOFS( maxpitch ), F_FLOAT, FFL_SPAWNTEMP },
	{ "nextmap", STOFS( nextmap ), F_LSTRING, FFL_SPAWNTEMP }, 
	{ 0, 0, F_INT, 0 }
};

const field_t Game::levelfields[ ] = {

	{ "changemap", LLOFS( changemap ), F_LSTRING }, 
	{ "sight_client", LLOFS( sight_client ), F_EDICT },
	{ "sight_entity", LLOFS( sight_entity ), F_EDICT },
	{ "sound_entity", LLOFS( sound_entity ), F_EDICT },
	{ "sound2_entity", LLOFS( sound2_entity ), F_EDICT }, 
	{ NULL, 0, F_INT }
};

const field_t Game::clientfields[ ] = {

	{ "pers.weapon", CLOFS( pers.weapon ), F_ITEM },
	{ "pers.lastweapon", CLOFS( pers.lastweapon ), F_ITEM },
	{ "newweapon", CLOFS( newweapon ), F_ITEM }, 
	{ NULL, 0, F_INT }
};

spawn_t Game::spawns[ ] = {

	{ "item_health", SP_item_health },
	{ "item_health_small", SP_item_health_small },
	{ "item_health_large", SP_item_health_large },
	{ "item_health_mega", SP_item_health_mega },

	{ "info_player_start", SP_info_player_start },
	{ "info_player_deathmatch", SP_info_player_deathmatch },
	{ "info_player_coop", SP_info_player_coop },
	{ "info_player_intermission", SP_info_player_intermission },

	{ "func_plat", SP_func_plat },
	{ "func_button", SP_func_button },
	{ "func_door", SP_func_door },
	{ "func_door_secret", SP_func_door_secret },
	{ "func_door_rotating", SP_func_door_rotating },
	{ "func_rotating", SP_func_rotating },
	{ "func_train", SP_func_train },
	{ "func_water", SP_func_water },
	{ "func_conveyor", SP_func_conveyor },
	{ "func_areaportal", SP_func_areaportal },
	{ "func_clock", SP_func_clock },
	{ "func_wall", SP_func_wall },
	{ "func_object", SP_func_object },
	{ "func_timer", SP_func_timer },
	{ "func_explosive", SP_func_explosive },
	{ "func_killbox", SP_func_killbox }, 

	{ "trigger_always", SP_trigger_always },
	{ "trigger_once", SP_trigger_once },
	{ "trigger_multiple", SP_trigger_multiple },
	{ "trigger_relay", SP_trigger_relay },
	{ "trigger_push", SP_trigger_push },
	{ "trigger_hurt", SP_trigger_hurt },
	{ "trigger_key", SP_trigger_key },
	{ "trigger_counter", SP_trigger_counter },
	{ "trigger_elevator", SP_trigger_elevator },
	{ "trigger_gravity", SP_trigger_gravity },
	{ "trigger_monsterjump", SP_trigger_monsterjump }, 

	{ "target_temp_entity", SP_target_temp_entity },
	{ "target_speaker", SP_target_speaker },
	{ "target_explosion", SP_target_explosion },
	{ "target_changelevel", SP_target_changelevel },
	{ "target_secret", SP_target_secret },
	{ "target_goal", SP_target_goal },
	{ "target_splash", SP_target_splash },
	{ "target_spawner", SP_target_spawner },
	{ "target_blaster", SP_target_blaster },
	{ "target_crosslevel_trigger", SP_target_crosslevel_trigger },
	{ "target_crosslevel_target", SP_target_crosslevel_target },
	{ "target_laser", SP_target_laser },
	{ "target_help", SP_target_help },
	{ "target_actor", SP_target_actor },
	{ "target_lightramp", SP_target_lightramp },
	{ "target_earthquake", SP_target_earthquake },
	{ "target_character", SP_target_character },
	{ "target_string", SP_target_string },

	{ "worldspawn", SP_worldspawn },
	{ "viewthing", SP_viewthing },
	{ "light", SP_light },
	{ "light_mine1", SP_light_mine1 },
	{ "light_mine2", SP_light_mine2 },
	{ "info_null", SP_info_null },
	{ "func_group", SP_info_null },
	{ "info_notnull", SP_info_notnull },
	{ "path_corner", SP_path_corner },
	{ "point_combat", SP_point_combat },
	{ "misc_explobox", SP_misc_explobox },

	{ "misc_banner", SP_misc_banner },
	{ "misc_satellite_dish", SP_misc_satellite_dish },
	{ "misc_actor", SP_misc_actor },
	{ "misc_gib_arm", SP_misc_gib_arm },
	{ "misc_gib_leg", SP_misc_gib_leg },
	{ "misc_gib_head", SP_misc_gib_head },
	{ "misc_deadsoldier", SP_misc_deadsoldier },
	{ "misc_viper", SP_misc_viper },
	{ "misc_viper_bomb", SP_misc_viper_bomb },
	{ "misc_bigviper", SP_misc_bigviper },
	{ "misc_strogg_ship", SP_misc_strogg_ship },
	{ "misc_teleporter", SP_misc_teleporter },
	{ "misc_teleporter_dest", SP_misc_teleporter_dest },
	{ "misc_blackhole", SP_misc_blackhole },
	{ "misc_eastertank", SP_misc_eastertank },
	{ "misc_easterchick", SP_misc_easterchick },
	{ "misc_easterchick2", SP_misc_easterchick2 },

	{ "monster_soldier", SP_monster_soldier },

	{ NULL, NULL }

};

const Str	Game::boneGib = "models/objects/gibs/bone/tris.md2";
const Str	Game::meatGib = "models/objects/gibs/sm_meat/tris.md2";
const Str	Game::headGib = "models/objects/gibs/head2/tris.md2";

const Str Game::actor_names[ ] = { "Hellrot", "Tokay", "Killme", "Disruptor", "Adrianator", "Rambear", "Titus", "Bitterman" };

const Str Game::actor_messages[ ] = { "Watch it", "#$@* &", "Idiot", "Check your targets" };

const Str	Common::logfile_name = "log.txt";
FileBase *		Common::logfile = NULL;

#ifndef DEDICATED_ONLY

enumNamePair_t GLImports::glEnumNames[ ] = {

	GL_NO_ERROR,						"no error",
	GL_INVALID_ENUM,					"invalid enum",
	GL_INVALID_VALUE,					"invalid value",
	GL_INVALID_OPERATION,				"invalid operation",
	GL_INVALID_FRAMEBUFFER_OPERATION,	"invalid framebuffer operation",
	GL_OUT_OF_MEMORY,					"out of memory",

	GL_BYTE,							"byte",
	GL_UNSIGNED_BYTE,					"unsigned byte",
	GL_SHORT,							"short",
	GL_UNSIGNED_SHORT,					"unsigned short",
	GL_UNSIGNED_INT,					"unsigned int",
	GL_2_BYTES,							"2 bytes",
	GL_3_BYTES,							"3 bytes",
	GL_4_BYTES,							"4 bytes",
	GL_DOUBLE,							"double",
	GL_FLOAT,							"float",
	GL_FLOAT_VEC2,						"vec2",
	GL_FLOAT_VEC3,						"vec3",
	GL_FLOAT_VEC4,						"vec4",
	GL_INT,								"int",
	GL_INT_VEC2,						"ivec2",
	GL_INT_VEC3,						"ivec3",
	GL_INT_VEC4,						"ivec4",
	GL_BOOL,							"bool",
	GL_BOOL_VEC2,						"bvec2",
	GL_BOOL_VEC3,						"bvec3",
	GL_BOOL_VEC4,						"bvec4",
	GL_FLOAT_MAT2,						"mat2",
	GL_FLOAT_MAT3,						"mat3",
	GL_FLOAT_MAT4,						"mat4",
	GL_SAMPLER_1D,						"sampler1D",
	GL_SAMPLER_2D,						"sampler2D",
	GL_SAMPLER_3D,						"sampler3D",
	GL_SAMPLER_2D_SHADOW,				"sampler2DShadow",

	GL_FRONT,							"front",
	GL_BACK,							"back",
	GL_FRONT_AND_BACK,					"frontback",

	GL_NEVER,							"never",
	GL_LESS,							"less",
	GL_LEQUAL,							"lessequal",
	GL_GREATER,							"greater",
	GL_GEQUAL,							"greaterequal",
	GL_EQUAL,							"equal",
	GL_NOTEQUAL,						"notequal",
	GL_ALWAYS,							"always",

	GL_KEEP,							"keep",
	GL_ZERO,							"zero",
	GL_REPLACE,							"replace",
	GL_INCR,							"increment",
	GL_INCR_WRAP,						"incrementwrap",
	GL_DECR,							"decrement",
	GL_DECR_WRAP,						"decrementwrap",
	GL_INVERT,							"invert",

	0,									null_string
};

#endif

const ucmd_t Server::ucmds[ ] = {

	// auto issued
	{ "new", Server::SV_New_f },
	{ "configstrings", Server::SV_Configstrings_f },
	{ "baselines", Server::SV_Baselines_f },
	{ "begin", Server::SV_Begin_f },
	{ "nextserver", Server::SV_Nextserver_f },
	{ "disconnect", Server::SV_Disconnect_f },

	// issued by hand at client consoles	
	{ "info", Server::SV_ShowServerinfo_f },
	{ "download", Server::SV_BeginDownload_f },
	{ "nextdl", Server::SV_NextDownload_f },

	{ NULL, NULL }

};

List< Str >			Command::cmd_argv;
Str					Command::cmd_args;

level_locals_t		Game::level;
spawn_temp_t		Game::st;
Entity				Game::g_edicts[ 1024 ];
gclient_t			Game::clients[ 32 ];

Str		System::findbase;
Str		System::findpath;
Str		System::console_text;

#if 0
FrameTextureParam_t FrameBuffer::g_depthTextureParams[ ] = {

	GL_TEXTURE_WRAP_S,			GL_CLAMP_TO_EDGE,
	GL_TEXTURE_WRAP_T,			GL_CLAMP_TO_EDGE,
	GL_TEXTURE_MIN_FILTER,		GL_NEAREST,
	GL_TEXTURE_MAG_FILTER,		GL_NEAREST,
	//GL_TEXTURE_COMPARE_MODE,	GL_COMPARE_REF_TO_TEXTURE,
	//GL_TEXTURE_COMPARE_FUNC,	GL_LEQUAL,
	0, 0
};

FrameTextureParam_t FrameBuffer::g_colorTextureParams[ ] = {

	GL_TEXTURE_WRAP_S,			GL_REPEAT,
	GL_TEXTURE_WRAP_T,			GL_REPEAT,
	GL_TEXTURE_MIN_FILTER,		GL_NEAREST,
	GL_TEXTURE_MAG_FILTER,		GL_NEAREST,
	0, 0
};

FrameAttachmentType_t FrameBuffer::g_attachmentTypes[ ] = {

	"color0",	true,	0,		GL_COLOR_ATTACHMENT0,			g_colorTextureParams,	GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,
	"color1",	true,	0,		GL_COLOR_ATTACHMENT1,			g_colorTextureParams,	GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,
	"color2",	true,	0,		GL_COLOR_ATTACHMENT2,			g_colorTextureParams,	GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,
	"color3",	true,	0,		GL_COLOR_ATTACHMENT3,			g_colorTextureParams,	GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,
	"color4",	true,	0,		GL_COLOR_ATTACHMENT4,			g_colorTextureParams,	GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,
	"color5",	true,	0,		GL_COLOR_ATTACHMENT5,			g_colorTextureParams,	GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,
	"color6",	true,	0,		GL_COLOR_ATTACHMENT6,			g_colorTextureParams,	GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,
	"color7",	true,	0,		GL_COLOR_ATTACHMENT7,			g_colorTextureParams,	GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,
	"color8",	true,	0,		GL_COLOR_ATTACHMENT8,			g_colorTextureParams,	GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,
	"color9",	true,	0,		GL_COLOR_ATTACHMENT9,			g_colorTextureParams,	GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,
	"color10",	true,	0,		GL_COLOR_ATTACHMENT10,			g_colorTextureParams,	GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,
	"color11",	true,	0,		GL_COLOR_ATTACHMENT11,			g_colorTextureParams,	GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,
	"color12",	true,	0,		GL_COLOR_ATTACHMENT12,			g_colorTextureParams,	GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,
	"color13",	true,	0,		GL_COLOR_ATTACHMENT13,			g_colorTextureParams,	GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,
	"color14",	true,	0,		GL_COLOR_ATTACHMENT14,			g_colorTextureParams,	GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,
	"color15",	true,	0,		GL_COLOR_ATTACHMENT15,			g_colorTextureParams,	GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,
	"dpst",		false,	0,		GL_DEPTH_STENCIL_ATTACHMENT,	g_depthTextureParams,	GL_DEPTH24_STENCIL8,	GL_DEPTH_STENCIL,	GL_UNSIGNED_INT_24_8,
	"dp",		false,	0,		GL_DEPTH_ATTACHMENT,			g_depthTextureParams,	GL_DEPTH_COMPONENT24,	GL_DEPTH_COMPONENT,	GL_UNSIGNED_BYTE,

	"c0rgba32",	true,	0,		GL_COLOR_ATTACHMENT0,			g_colorTextureParams,	GL_RGBA32F,				GL_RGBA,			GL_FLOAT,
	"c3rgba32",	true,	0,		GL_COLOR_ATTACHMENT3,			g_colorTextureParams,	GL_RGBA32F,				GL_RGBA,			GL_FLOAT,
	"c2rgba16",	true,	0,		GL_COLOR_ATTACHMENT2,			g_colorTextureParams,	GL_RGBA16F,				GL_RGBA,			GL_HALF_FLOAT,
	"c3rgba16",	true,	0,		GL_COLOR_ATTACHMENT3,			g_colorTextureParams,	GL_RGBA16F,				GL_RGBA,			GL_HALF_FLOAT,

	"c0ms1",	true,	1,		GL_COLOR_ATTACHMENT0,			NULL,				GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,
	"dsms1",	false,	1,		GL_DEPTH_STENCIL_ATTACHMENT,	NULL,				GL_DEPTH24_STENCIL8,	GL_DEPTH_STENCIL,	GL_UNSIGNED_INT_24_8,
	"c0ms2",	true,	2,		GL_COLOR_ATTACHMENT0,			NULL,				GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,
	"dsms2",	false,	2,		GL_DEPTH_STENCIL_ATTACHMENT,	NULL,				GL_DEPTH24_STENCIL8,	GL_DEPTH_STENCIL,	GL_UNSIGNED_INT_24_8,
	"c0ms4",	true,	4,		GL_COLOR_ATTACHMENT0,			NULL,				GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,
	"dsms4",	false,	4,		GL_DEPTH_STENCIL_ATTACHMENT,	NULL,				GL_DEPTH24_STENCIL8,	GL_DEPTH_STENCIL,	GL_UNSIGNED_INT_24_8,
	"c0ms8",	true,	8,		GL_COLOR_ATTACHMENT0,			NULL,				GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,
	"dsms8",	false,	8,		GL_DEPTH_STENCIL_ATTACHMENT,	NULL,				GL_DEPTH24_STENCIL8,	GL_DEPTH_STENCIL,	GL_UNSIGNED_INT_24_8,
	"c0ms16",	true,	16,		GL_COLOR_ATTACHMENT0,			NULL,				GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,
	"dsms16",	false,	16,		GL_DEPTH_STENCIL_ATTACHMENT,	NULL,				GL_DEPTH24_STENCIL8,	GL_DEPTH_STENCIL,	GL_UNSIGNED_INT_24_8,

	null_string, false, 0,		0,							NULL,				0,						0,					0
};
#endif

#ifndef DEDICATED_ONLY

VertexBufferFormat VertexUI::g_vertexBufferFormat[ ] = {

	2,	GL_HALF_FLOAT,		GL_FALSE,	sizeof( VertexUI ),	( GLvoid * )offsetof( VertexUI, xy ),
	2,	GL_HALF_FLOAT,		GL_FALSE,	sizeof( VertexUI ),	( GLvoid * )offsetof( VertexUI, st ),
	4,	GL_UNSIGNED_BYTE,	GL_FALSE,	sizeof( VertexUI ),	( GLvoid * )offsetof( VertexUI, color ),

	0,	0,					0,			0,					( GLvoid * )0
};

VertexBufferFormat VertexColorOnlyUI::g_vertexBufferFormat[ ] = {

	2,	GL_FLOAT,			GL_FALSE,	sizeof( VertexColorOnlyUI ),	( GLvoid * )offsetof( VertexColorOnlyUI, xy ),
	4,	GL_UNSIGNED_BYTE,	GL_FALSE,	sizeof( VertexColorOnlyUI ),	( GLvoid * )offsetof( VertexColorOnlyUI, color ),

	0,	0,					0,			0,								( GLvoid * )0
};

VertexBufferFormat DrawVert::g_vertexBufferFormat[ ] = {

	4,	GL_FLOAT,	GL_FALSE,	sizeof( DrawVert ),	( GLvoid * )offsetof( DrawVert, xyz ),
	2,	GL_FLOAT,	GL_FALSE,	sizeof( DrawVert ),	( GLvoid * )offsetof( DrawVert, st ),
	4,	GL_FLOAT,	GL_FALSE,	sizeof( DrawVert ),	( GLvoid * )offsetof( DrawVert, normal ),
	0,	0,			0,			0,					( GLvoid * )0
};

VertexBufferFormat Vertex3D::g_vertexBufferFormat[ ] = {

	3,	GL_FLOAT,		GL_FALSE,	sizeof( Vertex3D ),	( GLvoid * )offsetof( Vertex3D, xyz ),
	2,	GL_HALF_FLOAT,	GL_FALSE,	sizeof( Vertex3D ),	( GLvoid * )offsetof( Vertex3D, st ),
	4,	GL_HALF_FLOAT,	GL_FALSE,	sizeof( Vertex3D ),	( GLvoid * )offsetof( Vertex3D, normal ),
	0,	0,				0,			0,					( GLvoid * )0
};

#endif
