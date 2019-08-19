// g_local.h -- local definitions for game module

// define GAME_INCLUDE so that game.h does not define the
// short, server-visible gclient_t and Entity structures, // because we define the full size ones in this file

// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	"baseq2"

//==================================================================

// view pitching times
#define DAMAGE_TIME		0.5f
#define	FALL_TIME		0.3f


// edict->spawnflags
// these are set with checkboxes on each entity in the map editor
#define	SPAWNFLAG_NOT_EASY			0x00000100
#define	SPAWNFLAG_NOT_MEDIUM		0x00000200
#define	SPAWNFLAG_NOT_HARD			0x00000400
#define	SPAWNFLAG_NOT_DEATHMATCH	0x00000800
#define	SPAWNFLAG_NOT_COOP			0x00001000

// edict->flags
#define	FL_FLY					0x00000001
#define	FL_SWIM					0x00000002	// implied immunity to drowining
#define FL_IMMUNE_LASER			0x00000004
#define	FL_INWATER				0x00000008
#define	FL_GODMODE				0x00000010
#define	FL_NOTARGET				0x00000020
#define FL_IMMUNE_SLIME			0x00000040
#define FL_IMMUNE_LAVA			0x00000080
#define	FL_PARTIALGROUND		0x00000100	// not all corners are valid
#define	FL_WATERJUMP			0x00000200	// player jumping out of water
#define	FL_TEAMSLAVE			0x00000400	// not the first on the team
#define FL_NO_KNOCKBACK			0x00000800
#define FL_POWER_ARMOR			0x00001000	// power armor( if any ) is active
#define FL_RESPAWN				0x80000000	// used for item respawning


#define	FRAMETIME		0.1f

// memory tags to allow dynamic memory to be cleaned up
#define	TAG_GAME	765		// clear when unloading the dll
#define	TAG_LEVEL	766		// clear when loading a new level
#define TAG_CM		767


#define MELEE_DISTANCE	80

#define BODY_QUEUE_SIZE		8

typedef enum {
	DAMAGE_NO,
	DAMAGE_YES, 			// will take damage if hit
	DAMAGE_AIM			// auto targeting recognizes this
} damage_t;

typedef enum  {
	WEAPON_READY,
	WEAPON_ACTIVATING,
	WEAPON_DROPPING,
	WEAPON_FIRING
} weaponstate_t;

typedef enum {
	AMMO_BULLETS,
	AMMO_SHELLS,
	AMMO_ROCKETS,
	AMMO_GRENADES,
	AMMO_CELLS,
	AMMO_SLUGS
} ammo_t;


//deadflag
#define DEAD_NO					0
#define DEAD_DYING				1
#define DEAD_DEAD				2
#define DEAD_RESPAWNABLE		3

//range
#define RANGE_MELEE				0
#define RANGE_NEAR				1
#define RANGE_MID				2
#define RANGE_FAR				3

//gib types
#define GIB_ORGANIC				0
#define GIB_METALLIC			1

//monster ai flags
#define AI_STAND_GROUND			0x00000001
#define AI_TEMP_STAND_GROUND	0x00000002
#define AI_SOUND_TARGET			0x00000004
#define AI_LOST_SIGHT			0x00000008
#define AI_PURSUIT_LAST_SEEN	0x00000010
#define AI_PURSUE_NEXT			0x00000020
#define AI_PURSUE_TEMP			0x00000040
#define AI_HOLD_FRAME			0x00000080
#define AI_GOOD_GUY				0x00000100
#define AI_BRUTAL				0x00000200
#define AI_NOSTEP				0x00000400
#define AI_DUCKED				0x00000800
#define AI_COMBAT_POINT			0x00001000
#define AI_MEDIC				0x00002000
#define AI_RESURRECTING			0x00004000

//monster attack state
#define AS_STRAIGHT				1
#define AS_SLIDING				2
#define	AS_MELEE				3
#define	AS_MISSILE				4

// armor types
#define ARMOR_NONE				0
#define ARMOR_JACKET			1
#define ARMOR_COMBAT			2
#define ARMOR_BODY				3
#define ARMOR_SHARD				4

// power armor types
#define POWER_ARMOR_NONE		0
#define POWER_ARMOR_SCREEN		1
#define POWER_ARMOR_SHIELD		2

// handedness values
#define RIGHT_HANDED			0
#define LEFT_HANDED				1
#define CENTER_HANDED			2


// game.serverflags values
#define SFL_CROSS_TRIGGER_1		0x00000001
#define SFL_CROSS_TRIGGER_2		0x00000002
#define SFL_CROSS_TRIGGER_3		0x00000004
#define SFL_CROSS_TRIGGER_4		0x00000008
#define SFL_CROSS_TRIGGER_5		0x00000010
#define SFL_CROSS_TRIGGER_6		0x00000020
#define SFL_CROSS_TRIGGER_7		0x00000040
#define SFL_CROSS_TRIGGER_8		0x00000080
#define SFL_CROSS_TRIGGER_MASK	0x000000FF


// noise types for PlayerNoise
#define PNOISE_SELF				0
#define PNOISE_WEAPON			1
#define PNOISE_IMPACT			2


// edict->movetype values
typedef enum {

	MOVETYPE_NONE, 			// never moves
	MOVETYPE_NOCLIP, 		// origin and angles change with no interaction
	MOVETYPE_PUSH, 			// no clip to world, push on box contact
	MOVETYPE_STOP, 			// no clip to world, stops on box contact

	MOVETYPE_WALK, 			// gravity
	MOVETYPE_STEP, 			// gravity, special edge handling
	MOVETYPE_FLY,
	MOVETYPE_TOSS, 			// gravity
	MOVETYPE_FLYMISSILE, 	// extra size to monsters
	MOVETYPE_BOUNCE

} movetype_t;



class gitem_armor_t {
public:
	int		base_count;
	int		max_count;
	float	normal_protection;
	float	energy_protection;
	int		armor;
};


// GameItem->flags
#define	IT_WEAPON		1		// use makes active weapon
#define	IT_AMMO			2
#define IT_ARMOR		4
#define IT_STAY_COOP	8
#define IT_KEY			16
#define IT_POWERUP		32

// GameItem->weapmodel for weapons indicates model index
#define WEAP_BLASTER			1 
#define WEAP_SHOTGUN			2 
#define WEAP_SUPERSHOTGUN		3 
#define WEAP_MACHINEGUN			4 
#define WEAP_CHAINGUN			5 
#define WEAP_GRENADES			6 
#define WEAP_GRENADELAUNCHER	7 
#define WEAP_ROCKETLAUNCHER		8 
#define WEAP_HYPERBLASTER		9 
#define WEAP_RAILGUN			10
#define WEAP_BFG				11

class Entity;

class GameItem {
public:

	const Str		classname;	// spawning name
	bool			( *pickup )( Entity * ent, Entity * other );
	void			( *use )( Entity * ent, GameItem * item );
	void			( *drop )( Entity * ent, GameItem * item );
	void			( *weaponthink )( Entity * ent );
	const Str		pickup_sound;
	const Str		world_model;
	int				world_model_flags;
	const Str		view_model;

	// client side info
	const Str		icon;
	const Str		pickup_name;	// for printing on pickup
	int				count_width;	// number of digits to display by icon

	int				quantity;		// for ammo how much, for weapons how much is used per shot
	const Str		ammo;			// for weapons
	int				flags;			// IT_* flags

	int				weapmodel;		// weapon model index( for weapons )

	const void *	info;
	int				tag;

	const Str		precaches;		// string of all models, sounds, and images this item will use

};

class gclient_t;

//
// this structure is cleared as each map is entered
// it is read/written to the level.sav file for savegames
//
class level_locals_t {
public:
	int				framenum;
	timeTypeReal	time;

	Str			level_name;	// the descriptive name( Outer Base, etc )
	Str			mapname;		// the server name( base1, etc )
	Str			nextmap;		// go here when fraglimit is hit

	// intermission state
	timeTypeReal	intermissiontime;		// time the intermission was started
	Str			changemap;
	int			exitintermission;
	Vec3		intermission_origin;
	Vec3		intermission_angle;

	Entity *	sight_client;	// changed once each frame for coop games

	Entity *	sight_entity;
	int			sight_entity_framenum;
	Entity *	sound_entity;
	int			sound_entity_framenum;
	Entity *	sound2_entity;
	int			sound2_entity_framenum;

	int			pic_health;

	int			total_secrets;
	int			found_secrets;

	int			total_goals;
	int			found_goals;

	int			total_monsters;
	int			killed_monsters;

	Entity *	current_entity;	// entity running from G_RunFrame
	int			body_que;			// dead bodies

	int			power_cubes;		// ugly necessity for coop

	void		Clear( );
};


// spawn_temp_t is only used to hold entity field values that
// can be set from the editor, but aren't actualy present
// in Entity during gameplay
class spawn_temp_t {
public:
	// world vars
	Str			sky;
	float		skyrotate;
	Vec3		skyaxis;
	Str			nextmap;

	int			lip;
	int			distance;
	int			height;
	Str			noise;
	timeTypeReal	pausetime;
	Str			item;
	Str			gravity;

	float		minyaw;
	float		maxyaw;
	float		minpitch;
	float		maxpitch;

	void		Clear( );
};

class moveinfo_t {
public:
	// fixed data
	Vec3		start_origin;
	Vec3		start_angles;
	Vec3		end_origin;
	Vec3		end_angles;

	int			sound_start;
	int			sound_middle;
	int			sound_end;

	float		accel;
	float		speed;
	float		decel;
	float		distance;

	float		wait;

	// state data
	int			state;
	Vec3		dir;
	float		current_speed;
	float		move_speed;
	float		next_speed;
	float		remaining_distance;
	float		decel_distance;
	void		( *endfunc )( Entity * );

	void		Clear( );
};

class mframe_t {
public:
	void	( *aifunc )( Entity * self, float dist );
	float	dist;
	void	( *thinkfunc )( Entity * self );
};

class mmove_t {
public:
	int			firstframe;
	int			lastframe;
	mframe_t	* frame;
	void		( *endfunc )( Entity * self );
};

class monsterinfo_t {
public:
	mmove_t *	currentmove;
	int			aiflags;
	int			nextframe;
	float		scale;

	void		( *stand )( Entity * self );
	void		( *le )( Entity * self );
	void		( *search )( Entity * self );
	void		( *walk )( Entity * self );
	void		( *run )( Entity * self );
	void		( *dodge )( Entity * self, Entity * other, float eta );
	void		( *attack )( Entity * self );
	void		( *melee )( Entity * self );
	void		( *sight )( Entity * self, Entity * other );
	bool		( *checkattack )( Entity * self );

	timeTypeReal	pausetime;
	float		attack_finished;

	Vec3		saved_goal;
	float		search_time;
	float		trail_time;
	Vec3		last_sighting;
	int			attack_state;
	int			lefty;
	float		le_time;
	int			linkcount;

	int			power_armor_type;
	int			power_armor_power;

	void		Clear( );
};

// means of death
#define MOD_UNKNOWN			0
#define MOD_BLASTER			1
#define MOD_SHOTGUN			2
#define MOD_SSHOTGUN		3
#define MOD_MACHINEGUN		4
#define MOD_CHAINGUN		5
#define MOD_GRENADE			6
#define MOD_G_SPLASH		7
#define MOD_ROCKET			8
#define MOD_R_SPLASH		9
#define MOD_HYPERBLASTER	10
#define MOD_RAILGUN			11
#define MOD_BFG_LASER		12
#define MOD_BFG_BLAST		13
#define MOD_BFG_EFFECT		14
#define MOD_HANDGRENADE		15
#define MOD_HG_SPLASH		16
#define MOD_WATER			17
#define MOD_SLIME			18
#define MOD_LAVA			19
#define MOD_CRUSH			20
#define MOD_TELEFRAG		21
#define MOD_FALLING			22
#define MOD_SUICIDE			23
#define MOD_HELD_GRENADE	24
#define MOD_EXPLOSIVE		25
#define MOD_BARREL			26
#define MOD_BOMB			27
#define MOD_EXIT			28
#define MOD_SPLASH			29
#define MOD_TARGET_LASER	30
#define MOD_TRIGGER_HURT	31
#define MOD_HIT				32
#define MOD_TARGET_BLASTER	33
#define MOD_FRIENDLY_FIRE	0x8000000

//#define	FOFS( x )( int )&( ( ( Entity * )0 )->x )
//#define	STOFS( x )( int )&( ( ( spawn_temp_t * )0 )->x )
//#define	LLOFS( x )( int )&( ( ( level_locals_t * )0 )->x )
//#define	CLOFS( x )( int )&( ( ( gclient_t * )0 )->x )

#define	FOFS( x )	offsetof( Entity, x )
#define	STOFS( x )	offsetof( spawn_temp_t, x )
#define	LLOFS( x )	offsetof( level_locals_t, x )
#define	CLOFS( x )	offsetof( gclient_t, x )

#define random( )	( ( rand( ) & 0x7FFF ) / ( ( float )0x7FFF ) )
#define crandom( )	( 2.0f * ( random( ) - 0.5f ) )

#define world	( &g_edicts[ 0 ] )

// item spawnflags
#define ITEM_TRIGGER_SPAWN		0x00000001
#define ITEM_NO_TOUCH			0x00000002
// 6 bits reserved for editor flags
// 8 bits used as power cube  bits for coop games
#define DROPPED_ITEM			0x00010000
#define	DROPPED_PLAYER_ITEM		0x00020000
#define ITEM_TARGETS_USED		0x00040000

//
// fields are needed for spawning from the entity string
// and saving / loading games
//
enum {

	FFL_SPAWNTEMP	= BIT( 0 ),
	FFL_NOSPAWN		= BIT( 1 )

};

typedef enum {

	F_INT,
	F_FLOAT,
	F_DOUBLE,
	F_LSTRING, 			// string on disk, pointer in memory, TAG_LEVEL
	F_GSTRING, 			// string on disk, pointer in memory, TAG_GAME
	F_VECTOR,
	F_ANGLEHACK,
	F_EDICT, 			// index on disk, pointer in memory
	F_ITEM, 				// index on disk, pointer in memory
	F_CLIENT, 			// index on disk, pointer in memory
	F_FUNCTION,
	F_MMOVE,
	F_IGNORE

} fieldtype_t;

class field_t {
public:
	Str				name;
	int				ofs;
	fieldtype_t		type;
	int				flags;
};

#define	ITEM_INDEX( x )( ( int )( ( x )-Game::itemlist ) )

// damage flags
#define DAMAGE_RADIUS			0x00000001	// damage was indirect
#define DAMAGE_NO_ARMOR			0x00000002	// armour does not protect from this damage
#define DAMAGE_ENERGY			0x00000004	// damage is from an energy based weapon
#define DAMAGE_NO_KNOCKBACK		0x00000008	// do not affect velocity, just view angles
#define DAMAGE_BULLET			0x00000010  // damage is from a bullet( used for ricochets )
#define DAMAGE_NO_PROTECTION	0x00000020  // armor, shields, invulnerability, and godmode have no effect

#define DEFAULT_BULLET_HSPREAD	300
#define DEFAULT_BULLET_VSPREAD	500
#define DEFAULT_SHOTGUN_HSPREAD	1000
#define DEFAULT_SHOTGUN_VSPREAD	500
#define DEFAULT_DEATHMATCH_SHOTGUN_COUNT	12
#define DEFAULT_SHOTGUN_COUNT	12
#define DEFAULT_SSHOTGUN_COUNT	20

//============================================================================

// client_t->anim_priority
#define	ANIM_BASIC		0		// stand / run
#define	ANIM_WAVE		1
#define	ANIM_JUMP		2
#define	ANIM_PAIN		3
#define	ANIM_ATTACK		4
#define	ANIM_DEATH		5
#define	ANIM_REVERSE	6


// client data that stays across multiple level loads
class client_persistant_t {
public:
	Str			userinfo;
	Str			netname;
	int			hand;

	bool		connected;			// a loadgame will leave valid entities that
									// just don't have a connection yet

	// values saved and restored from edicts when changing levels
	int			health;
	int			max_health;
	int			savedFlags;

	int			selected_item;
	int			inventory[ MAX_ITEMS ];

	// ammo capacities
	int			max_bullets;
	int			max_shells;
	int			max_rockets;
	int			max_grenades;
	int			max_cells;
	int			max_slugs;

	GameItem *	weapon;
	GameItem *	lastweapon;

	int			power_cubes;	// used for tracking the cubes in coop games
	int			score;			// for calculating total unit score in coop games

	int			game_helpchanged;
	int			helpchanged;

	bool		spectator;			// client is a spectator

	void		Clear( );
};

// client data that stays across deathmatch respawns
class client_respawn_t {
public:
	client_persistant_t		coop_respawn;	// what to set client->pers to on a respawn
	int						enterframe;			// level.framenum the client entered the game
	int						score;				// frags, etc
	Vec3					cmd_angles;			// angles sent over in the last command

	bool					spectator;			// client is a spectator

	void					Clear( );
};

// this structure is cleared on each PutClientInServer( ), // except for 'client->pers'
class gclient_t {
public:
	// known to server
	player_state_t			ps;				// communicated by server to clients
	timeType				ping;

	// private to game
	client_persistant_t		pers;
	client_respawn_t		resp;
	pmove_state_t			old_pmove;	// for detecting out-of-pmove changes

	bool		showscores;			// set layout stat
	bool		showinventory;		// set layout stat
	bool		showhelp;
	bool		showhelpicon;

	int			ammo_index;

	int			buttons;
	int			oldbuttons;
	int			latched_buttons;

	bool		weapon_thunk;

	GameItem *	newweapon;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int			damage_armor;		// damage absorbed by armor
	int			damage_parmor;		// damage absorbed by power armor
	int			damage_blood;		// damage taken out of health
	int			damage_knockback;	// impact damage
	Vec3		damage_from;		// origin for vector calculation

	float		killer_yaw;			// when dead, look at killer

	weaponstate_t	weaponstate;
	Vec3		kick_angles;	// weapon kicks
	Vec3		kick_origin;
	float		v_dmg_roll, v_dmg_pitch;
	timeTypeReal	v_dmg_time;	// damage kicks
	timeTypeReal	fall_time;
	float		fall_value;		// for view drop on fall
	float		damage_alpha;
	float		bonus_alpha;
	Vec3		damage_blend;
	Vec3		v_angle;			// aiming direction
	float		bobtime;			// so off-ground doesn't change it
	Vec3		oldviewangles;
	Vec3		oldvelocity;

	timeTypeReal	next_drown_time;
	int			old_waterlevel;
	int			breather_sound;

	int			machinegun_shots;	// for weapon raising

	// animation vars
	int			anim_end;
	int			anim_priority;
	bool		anim_duck;
	bool		anim_run;

	// powerup timers
	float		quad_framenum;
	float		invincible_framenum;
	float		breather_framenum;
	float		enviro_framenum;

	bool		grenade_blew_up;
	timeTypeReal	grenade_time;
	int			silencer_shots;
	int			weapon_sound;

	timeTypeReal	pickup_msg_time;

	timeTypeReal	flood_locktill;		// locked from talking
	timeTypeReal	flood_when[ 10 ];		// when messages were said
	int			flood_whenhead;		// head pointer for when said

	timeTypeReal	respawn_time;		// can respawn when time > this

	Entity *	chase_target;		// player we are chasing
	bool		update_chase;		// need to update chase info?

	void		Clear( );
};

// link_t is only used for entity area links now

class link_t {
public:
	link_t *		prev;
	link_t *		next;

	void			ClearLink( );
	void			RemoveLink( );
	void			InsertLinkBefore( link_t * before );

	void			Clear( ) { prev = next = NULL; }
};

INLINE void link_t::ClearLink( ) {
	prev = next = this;
}

INLINE void link_t::RemoveLink( ) {
	next->prev = prev;
	prev->next = next;
}

INLINE void link_t::InsertLinkBefore( link_t * before ) {
	next = before;
	prev = before->prev;
	prev->next = this;
	next->prev = this;
}

#define	MAX_ENT_CLUSTERS	16

// edict->solid values
typedef enum {

SOLID_NOT, 			// no interaction with other objects
SOLID_TRIGGER, 		// only touch when inside, after moving
SOLID_BBOX, 			// touch on edge
SOLID_BSP			// bsp clip, touch on edge

} solid_t;

class Entity {
public:

	entity_state_t			s;
	gclient_t *				client;	// NULL if not a player
									// the server expects the first part
									// of gclient_s to be a player_state_t
									// but the rest of it is opaque

	bool					inuse;
	int						linkcount;

	// FIXME: move these fields to a server private sv_entity_t
	link_t					area;				// linked to a division node or leaf
	
	int						num_clusters;		// if -1, use headnode instead
	int						clusternums[ MAX_ENT_CLUSTERS ];
	int						headnode;			// unused if num_clusters != -1
	int						areanum, areanum2;

	//================================

	int						svflags;
	Vec3					mins, maxs;
	Vec3					absmin, absmax, size;
	solid_t					solid;
	int						clipmask;
	Entity *				owner;


	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!

	//================================
	int						movetype;
	int						flags;

	Str						model;
	timeTypeReal			freetime;			// sv.time when the object was freed
	
	//
	// only used locally in game, not by server
	//
	Str						message;
	Str						classname;
	Str						name;
	int						spawnflags;

	timeTypeReal			timestamp;

	float					angle;			// set in qe3, -1 = up, -2 = down
	Str						target;
	Str						targetname;
	Str						killtarget;
	Str						team;
	Str						pathtarget;
	Str						deathtarget;
	Str						combattarget;
	Entity *				target_ent;

	float					speed, accel, decel;
	Vec3					movedir;
	Vec3					pos1, pos2;

	Vec3					velocity;
	Vec3					avelocity;
	int						mass;
	timeTypeReal			air_finished;
	float					gravity;		// per entity gravity multiplier( 1.0f is normal )
											// use for lowgrav artifact, flares

	Entity *				goalentity;
	Entity *				movetarget;
	float					yaw_speed;
	float					eal_yaw;

	timeTypeReal			nextthink;
	void					( *prethink )( Entity * ent );
	void					( *think )( Entity * self );
	void					( *blocked )( Entity * self, Entity * other );	//move to moveinfo?
	void					( *touch )( Entity * self, Entity * other, Plane & plane, int surf );
	void					( *use )( Entity * self, Entity * other, Entity * activator );
	void					( *pain )( Entity * self, Entity * other, float kick, int damage );
	void					( *die )( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point );

	timeTypeReal			touch_debounce_time;		// are all these legit?  do we need more/less of them?
	timeTypeReal			pain_debounce_time;
	timeTypeReal			damage_debounce_time;
	timeTypeReal			fly_sound_debounce_time;	//move to clientinfo
	timeTypeReal			last_move_time;

	int						health;
	int						max_health;
	int						gib_health;
	int						deadflag;
	bool					show_hostile;

	timeTypeReal			powerarmor_time;

	Str						map;			// target_changelevel

	int						viewheight;		// height above origin where eyesight is determined
	int						takedamage;
	int						dmg;
	int						radius_dmg;
	float					dmg_radius;
	int						sounds;			//make this a spawntemp var?
	int						count;

	Entity *				chain;
	Entity *				enemy;
	Entity *				oldenemy;
	Entity *				activator;
	Entity *				groundentity;
	int						groundentity_linkcount;
	Entity *				teamchain;
	Entity *				teammaster;

	Entity *				mynoise;		// can go in client only
	Entity *				mynoise2;

	int						noise_index;
	int						noise_index2;
	float					volume;
	float					attenuation;

	// timing variables
	timeTypeReal			wait;
	timeTypeReal			delay;			// before firing targets
	timeTypeReal			random;

	timeTypeReal			teleport_time;

	int						watertype;
	int						waterlevel;

	Vec3					move_origin;
	Vec3					move_angles;

	// move this to clientinfo?
	int						light_level;

	int						style;			// also used as areaportal number

	GameItem *				item;			// for bonus items

	// common data blocks
	moveinfo_t				moveinfo;
	monsterinfo_t			monsterinfo;

	void					Clear( );
};
