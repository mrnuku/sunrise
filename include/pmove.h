/*
==============================================================

PLAYER MOVEMENT CODE

Common between server and client so prediction matches

==============================================================
*/

// pmove_state_t is the information necessary for client side movement
// prediction
typedef enum  {

	PM_NORMAL,		// can accelerate and turn
	PM_SPECTATOR,	// no acceleration or turning
	PM_DEAD,
	PM_GIB, 		// different bounding box
	PM_FREEZE

} pmtype_t;

// pmove->pm_flags
#define	PMF_DUCKED			1
#define	PMF_JUMP_HELD		2
#define	PMF_ON_GROUND		4
#define	PMF_TIME_WATERJUMP	8	// pm_time is waterjump
#define	PMF_TIME_LAND		16	// pm_time is time before rejump
#define	PMF_TIME_TELEPORT	32	// pm_time is non-moving time
#define PMF_NO_PREDICTION	64	// temporarily disables prediction( used for grappling hook )

// this structure needs to be communicated bit-accurate
// from the server to the client to guarantee that
// prediction stays in sync, so no floats are used.
// if any part of the game code modifies this struct, it
// will result in a prediction error of some degree.
class pmove_state_t {
public:
	pmtype_t	pm_type;

	short		origin[ 3 ];		// 12.3
	short		velocity[ 3 ];	// 12.3
	byte		pm_flags;		// ducked, jump_held, etc
	byte		pm_time;		// each unit = 8 ms
	short		gravity;
	short		delta_angles[ 3 ];	// add to command angles to get view direction
									// changed by spawns, rotating objects, and teleporters
	void		Clear( );
};

//
// button bits
//
#define	BUTTON_ATTACK		1
#define	BUTTON_USE			2
#define	BUTTON_ANY			128			// any key whatsoever

// usercmd_t is sent to the server each client frame
class usercmd_t {
public:
	byte	msec;
	byte	buttons;
	short	angles[ 3 ];
	short	forwardmove, sidemove, upmove;
	byte	impulse;		// remove?
	byte	lightlevel;		// light level the player is standing on

			usercmd_t( ) { Clear( ); }
	void	Clear( );
};

#define	MAXTOUCH	32

class pmove_t {
public:
	// state( in / out )
	pmove_state_t	s;

	// command( in )
	usercmd_t		cmd;
	bool		snapinitial;	// if s has been changed outside pmove

	// results( out )
	int			numtouch;
	Entity *	touchents[ MAXTOUCH ];

	Vec3		viewangles;			// clamped
	float		viewheight;

	Vec3		mins, maxs;			// bounding box size

	Entity *	groundentity;
	int			watertype;
	int			waterlevel;

	// callbacks to test the world
	CTrace		( *trace )( Vec3 & start, Vec3 & mins, Vec3 & maxs, Vec3 & end );
	int			( *pointcontents )( Vec3 & point );

	void		Clear( );
};

// player_state_t is the information needed in addition to pmove_state_t
// to rendered a view.  There will only be 10 player_state_t sent each second, // but the number of pmove_state_t changes will be reletive to client
// frame rates
class player_state_t {
public:
	pmove_state_t	pmove;		// for prediction

	// these fields do not need to be communicated bit-precise

	Vec3		viewangles;		// for fixed views
	Vec3		viewoffset;		// add to pmovestate->origin
	Vec3		kick_angles;	// add to view direction to get render angles
								// set by weapon kicks, pain effects, etc

	Vec3		gunangles;
	Vec3		gunoffset;
	int			gunindex;
	int			gunframe;

	float		blend[ 4 ];		// rgba full screen effect
	
	float		fov;			// horizontal field of view

	int			rdflags;		// refdef flags

	short		stats[ MAX_STATS ];		// fast status bar updates

	void		Clear( );
};

// all of the locals will be zeroed before each
// pmove, just to make damn sure we don't have
// any differences when running on client or server

class pml_t {
public:
	Vec3		origin; // full float precision
	Vec3		velocity; // full float precision

	Vec3		forward;
	Vec3		right;
	Vec3		up;
	float		frametime;

	Surface *	groundsurface;
	Plane		groundplane;
	int			groundcontents;

	Vec3		previous_origin;
	bool		ladder;

				pml_t( ) { Clear( ); }
	void		Clear( );
};

class PmoveParams {
public:
	pml_t		d_pml;
	pmove_t &	d_pm;

				PmoveParams( pmove_t & pm ) : d_pm( pm ) { }
};

extern float pm_airaccelerate;

class PlayerMove {

private:

	// slide off of the impacting object. returns the blocked flags( 1 = floor, 2 = step / wall )
	static void			PM_ClipVelocity( const Vec3 & in, const Vec3 & normal, Vec3 & out, float overbounce );
	// Each intersection will try to step over the obstruction instead of sliding along it.
	// Returns a new origin, velocity, and contact entity Does not modify any world state?
	static void			PM_StepSlideMove_( PmoveParams & pm_params );
	static void			PM_StepSlideMove( PmoveParams & pm_params );
	// Handles both ground friction and water friction
	static void			PM_Friction( PmoveParams & pm_params );
	// Handles user intended acceleration
	static void			PM_Accelerate( PmoveParams & pm_params, const Vec3 & wishdir, float wishspeed, float accel );
	static void			PM_AirAccelerate( PmoveParams & pm_params, const Vec3 & wishdir, float wishspeed, float accel );
	static void			PM_AddCurrents( PmoveParams & pm_params, Vec3 & wishvel );
	static void			PM_WaterMove( PmoveParams & pm_params );
	static void			PM_AirMove( PmoveParams & pm_params );
	static void			PM_CatagorizePosition( PmoveParams & pm_params );
	static void			PM_CheckJump( PmoveParams & pm_params );
	static void			PM_CheckSpecialMovement( PmoveParams & pm_params );
	static void			PM_FlyMove( PmoveParams & pm_params, bool doclip );
	// Sets mins, maxs, and pm_params.d_pm.viewheight
	static void			PM_CheckDuck( PmoveParams & pm_params );
	static void			PM_DeadMove( PmoveParams & pm_params );
	static bool			PM_GoodPosition( PmoveParams & pm_params );
	// On exit, the origin will have a value that is pre-quantized to the 0.125f precision of the network channel and in a valid position.
	static void			PM_SnapPosition( PmoveParams & pm_params );
	static void			PM_InitialSnapPosition( PmoveParams & pm_params );
	static void			PM_ClampAngles( PmoveParams & pm_params );

public:
	// Can be called by either the server or the client
	static void			Pmove( pmove_t * pmove );
};
