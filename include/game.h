// game.h -- game dll information visible to server

#define	GAME_API_VERSION	3

// edict->svflags

#define	SVF_NOCLIENT			0x00000001	// don't send entity to clients, even if it has effects
#define	SVF_DEADMONSTER			0x00000002	// treat as CONTENTS_CORPSE for collision
#define	SVF_MONSTER				0x00000004	// treat as CONTENTS_BODY for collision

//===============================================================

class spawn_t {
public:
	Str		name;
	void	( *spawn )( Entity * ent );
};

class Game {

private:

	static CVar		gun_x;
	static CVar		gun_y;
	static CVar		gun_z;

	//FIXME: sv_ prefix is wrong for these
	static CVar		sv_rollspeed;
	static CVar		sv_rollangle;
	static CVar		sv_maxvelocity;
	static CVar		sv_gravity;

	// latched vars
	static CVar		sv_cheats;
	static CVar		gamename;
	static CVar		gamedate;

	static CVar		maxspectators;
	static CVar		deathmatch;
	static CVar		coop;
	static CVar		skill;
	static CVar		maxentities;

	// change anytime vars
	static CVar		dmflags;
	static CVar		fraglimit;
	static CVar		timelimit;
	static CVar		password;
	static CVar		spectator_password;
	static CVar		needpass;
	static CVar		filterban;

	static CVar		g_select_empty;

	static CVar		run_pitch;
	static CVar		run_roll;
	static CVar		bob_pitch;
	static CVar		bob_roll;

	// flood control
	static CVar		flood_msgs;
	static CVar		flood_persecond;
	static CVar		flood_waitdelay;

	// dm map list
	static CVar		sv_maplist;

	static CVar		g_defplayer;

	static gitem_armor_t		jacketarmor_info;
	static gitem_armor_t		combatarmor_info;
	static gitem_armor_t		bodyarmor_info;

	static GameItem		itemlist[ ];
	static spawn_t		spawns[ ];

	static const field_t		fields[ ];
	static const field_t		levelfields[ ];
	static const field_t		clientfields[ ];

	static level_locals_t		level;
	static spawn_temp_t			st;

	static Str			helpmessage1;
	static Str			helpmessage2;
	static int			helpchanged;	// flash F1 icon if non 0, play sound
											// and increment only if 1, 2, or 3

	static gclient_t		clients[ 32 ];		// [ maxclients ]
	static Entity			g_edicts[ 1024 ];

	// can't store spawnpoint in level, because
	// it would get overwritten by the savegame restore
	static Str			spawnpoint;	// needed for coop respawns

	// store latched cvars here that we want to get at often
	static int				maxClients;
	static int				maxEntities;

	// cross level triggers
	static int				serverflags;

	// items
	static int				num_items;

	static bool				autosaved;

	// g_ai.cpp
	// Called once each frame to set level.sight_client to the player to be checked for in findtarget.
	// If all clients are either dead or in notarget, sight_client will be null. In coop games, sight_client will cycle between the clients.
	static void				AI_SetSightClient( );

	// Move the specified distance at current facing. This replaces the QC functions: ai_forward, ai_back, ai_pain, and ai_painforward
	static void				ai_move( Entity * self, float dist );
	// Used for standing around and looking for players. Distance is for slight position adjustments needed by the animations
	static void				ai_stand( Entity * self, float dist );
	// The monster is walking it's beat
	static void				ai_walk( Entity * self, float dist );
	// Turns towards target and advances. Use this call with a distnace of 0 to replace ai_face
	static void				ai_charge( Entity * self, float dist );
	// don't move, but turn towards eal_yaw. Distance is for slight position adjustments needed by the animations
	static void				ai_turn( Entity * self, float dist );
	static int				range( Entity * self, Entity * other );
	// returns 1 if the entity is visible to self, even if not infront( )
	static bool				visible( Entity * self, Entity * other );
	// returns 1 if the entity is in front( in sight ) of self
	static bool				infront( Entity * self, Entity * other );
	static void				HuntTarget( Entity * self );
	static void				FoundTarget( Entity * self );
	static bool				FindTarget( Entity * self );
	static bool				FacingIdeal( Entity * self );
	static bool				M_CheckAttack( Entity * self );
	// Turn and close until within an angle to launch a melee attack
	static void				ai_run_melee( Entity * self );
	// Turn in place until within an angle to launch a missile attack
	static void				ai_run_missile( Entity * self );
	// Strafe sideways, but stay at aproximately the same range
	static void				ai_run_slide( Entity * self, float distance );
	// Decides if we're going to attack or do something else used by ai_run and ai_stand
	static bool				ai_checkattack( Entity * self, float dist );
	// The monster has an enemy it is trying to kill
	static void				ai_run( Entity * self, float dist );

	// g_chase.cpp
	static void				UpdateChaseCam( Entity * ent );
	static void				ChaseNext( Entity * ent );
	static void				ChasePrev( Entity * ent );
	static void				GetChaseTarget( Entity * ent );

	// g_cmds.cpp
	static int				PlayerSort( void const * a, void const * b );

	static const Str		ClientTeam( Entity * ent );
	static bool				OnSameTeam( Entity * ent1, Entity * ent2 );
	static void				SelectNextItem( Entity * ent, int itflags );
	static void				SelectPrevItem( Entity * ent, int itflags );
	static void				ValidateSelectedItem( Entity * ent );
	static void				Cmd_Give_f( Entity * ent );
	static void				Cmd_God_f( Entity * ent );
	static void				Cmd_Notarget_f( Entity * ent );
	static void				Cmd_Noclip_f( Entity * ent );
	static void				Cmd_Use_f( Entity * ent );
	static void				Cmd_Drop_f( Entity * ent );
	static void				Cmd_Inven_f( Entity * ent );
	static void				Cmd_InvUse_f( Entity * ent );
	static void				Cmd_WeapPrev_f( Entity * ent );
	static void				Cmd_WeapNext_f( Entity * ent );
	static void				Cmd_WeapLast_f( Entity * ent );
	static void				Cmd_InvDrop_f( Entity * ent );
	static void				Cmd_Kill_f( Entity * ent );
	static void				Cmd_PutAway_f( Entity * ent );
	static void				Cmd_Players_f( Entity * ent );
	static void				Cmd_Wave_f( Entity * ent );
	static void				Cmd_Say_f( Entity * ent, bool team, bool arg0 );
	static void				Cmd_PlayerList_f( Entity * ent );

	// g_combat.cpp
	static bool				CanDamage( Entity * targ, Entity * inflictor );
	static void				Killed( Entity * targ, Entity * inflictor, Entity * attacker, int damage, Vec3 & point );
	static void				SpawnDamage( int type, Vec3 & origin, Vec3 & normal, int damage );
	static int				CheckPowerArmor( Entity * ent, Vec3 & point, Vec3 & normal, int damage, int dflags );
	static int				CheckArmor( Entity * ent, Vec3 & point, Vec3 & normal, int damage, int te_sparks, int dflags );
	static bool				CheckTeamDamage( Entity * targ, Entity * attacker );
	static void				M_ReactToDamage( Entity * targ, Entity * attacker );

	static void				T_Damage( Entity * targ, Entity * inflictor, Entity * attacker, Vec3 & dir, Vec3 & point, Vec3 & normal, int damage, int knockback, int dflags, int mod );
	static void				T_RadiusDamage( Entity * inflictor, Entity * attacker, float damage, Entity * ignore, float radius, int mod );


	// g_func.cpp
	static void				Move_Done( Entity * ent );
	static void				Move_Final( Entity * ent );
	static void				Move_Begin( Entity * ent );
	static void				Move_Calc( Entity * ent, Vec3 & dest, void( *func )( Entity * ) );
	static void				AngleMove_Done( Entity * ent );
	static void				AngleMove_Final( Entity * ent );
	static void				AngleMove_Begin( Entity * ent );
	static void				AngleMove_Calc( Entity * ent, void( *func )( Entity * ) );

	static void				plat_CalcAcceleratedMove( moveinfo_t * moveinfo );
	static void				plat_Accelerate( moveinfo_t * moveinfo );
	static void				Think_AccelMove( Entity * ent );
	static void				plat_hit_top( Entity * ent );
	static void				plat_hit_bottom( Entity * ent );
	static void				plat_go_down( Entity * ent );
	static void				plat_go_up( Entity * ent );
	static void				plat_blocked( Entity * self, Entity * other );
	static void				Use_Plat( Entity * ent, Entity * other, Entity * activator );
	static void				Touch_Plat_Center( Entity * ent, Entity * other, Plane & plane, int surf );
	static void				plat_spawn_inside_trigger( Entity * ent );
	static void				SP_func_plat( Entity * ent );
	static void				rotating_blocked( Entity * self, Entity * other );
	static void				rotating_touch( Entity * self, Entity * other, Plane & plane, int surf );
	static void				rotating_use( Entity * self, Entity * other, Entity * activator );
	static void				SP_func_rotating( Entity * ent );
	static void				button_done( Entity * self );
	static void				button_return( Entity * self );
	static void				button_wait( Entity * self );
	static void				button_fire( Entity * self );
	static void				button_use( Entity * self, Entity * other, Entity * activator );
	static void				button_touch( Entity * self, Entity * other, Plane & plane, int surf );
	static void				button_killed( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point );
	static void				SP_func_button( Entity * ent );
	static void				door_use_areaportals( Entity * self, bool open );
	static void				door_hit_top( Entity * self );
	static void				door_hit_bottom( Entity * self );
	static void				door_go_down( Entity * self );
	static void				door_go_up( Entity * self, Entity * activator );
	static void				door_use( Entity * self, Entity * other, Entity * activator );
	static void				Touch_DoorTrigger( Entity * self, Entity * other, Plane & plane, int surf );
	static void				Think_CalcMoveSpeed( Entity * self );
	static void				Think_SpawnDoorTrigger( Entity * ent );
	static void				door_blocked( Entity * self, Entity * other );
	static void				door_killed( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point );
	static void				door_touch( Entity * self, Entity * other, Plane & plane, int surf );
	static void				SP_func_door( Entity * ent );
	static void				SP_func_door_rotating( Entity * ent );
	static void				SP_func_water( Entity * self );
	static void				train_blocked( Entity * self, Entity * other );
	static void				train_wait( Entity * self );
	static void				train_next( Entity * self );
	static void				train_resume( Entity * self );
	static void				func_train_find( Entity * self );
	static void				train_use( Entity * self, Entity * other, Entity * activator );
	static void				SP_func_train( Entity * self );
	static void				trigger_elevator_use( Entity * self, Entity * other, Entity * activator );
	static void				trigger_elevator_init( Entity * self );
	static void				SP_trigger_elevator( Entity * self );
	static void				func_timer_think( Entity * self );
	static void				func_timer_use( Entity * self , Entity * other , Entity * activator );
	static void				SP_func_timer( Entity * self );
	static void				func_conveyor_use( Entity * self, Entity * other, Entity * activator );
	static void				SP_func_conveyor( Entity * self );
	static void				door_secret_use( Entity * self, Entity * other, Entity * activator );
	static void				door_secret_move1( Entity * self );
	static void				door_secret_move2( Entity * self );
	static void				door_secret_move3( Entity * self );
	static void				door_secret_move4( Entity * self );
	static void				door_secret_move5( Entity * self );
	static void				door_secret_move6( Entity * self );
	static void				door_secret_done( Entity * self );
	static void				door_secret_blocked( Entity * self, Entity * other );
	static void				door_secret_die( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point );
	static void				SP_func_door_secret( Entity * ent );
	static void				use_killbox( Entity * self, Entity * other, Entity * activator );
	static void				SP_func_killbox( Entity * ent );

	// g_items.cpp
	static GameItem *		GetItemByIndex( int index );
	static GameItem *		FindItemByClassname( const Str & classname );
	static GameItem *		FindItem( const Str & pickup_name );
	static void				DoRespawn( Entity * ent );
	static void				SetRespawn( Entity * ent, float delay );

	static bool				Pickup_Powerup( Entity * ent, Entity * other );
	static void				Drop_General( Entity * ent, GameItem * item );
	static bool				Pickup_Adrenaline( Entity * ent, Entity * other );
	static bool				Pickup_AncientHead( Entity * ent, Entity * other );
	static bool				Pickup_Bandolier( Entity * ent, Entity * other );
	static bool				Pickup_Pack( Entity * ent, Entity * other );
	static void				Use_Quad( Entity * ent, GameItem * item );
	static void				Use_Breather( Entity * ent, GameItem * item );
	static void				Use_Envirosuit( Entity * ent, GameItem * item );
	static void				Use_Invulnerability( Entity * ent, GameItem * item );
	static void				Use_Silencer( Entity * ent, GameItem * item );
	static bool				Pickup_Key( Entity * ent, Entity * other );
	static bool				Add_Ammo( Entity * ent, GameItem * item, int count );
	static bool				Pickup_Ammo( Entity * ent, Entity * other );
	static void				Drop_Ammo( Entity * ent, GameItem * item );
	static void				MegaHealth_think( Entity * self );
	static bool				Pickup_Health( Entity * ent, Entity * other );
	static int				ArmorIndex( Entity * ent );
	static bool				Pickup_Armor( Entity * ent, Entity * other );
	static int				PowerArmorType( Entity * ent );
	static void				Use_PowerArmor( Entity * ent, GameItem * item );
	static bool				Pickup_PowerArmor( Entity * ent, Entity * other );
	static void				Drop_PowerArmor( Entity * ent, GameItem * item );
	static void				Touch_Item( Entity * ent, Entity * other, Plane & plane, int surf );
	static void				drop_temp_touch( Entity * ent, Entity * other, Plane & plane, int surf );
	static void				drop_make_touchable( Entity * ent );
	static Entity *			Drop_Item( Entity * ent, GameItem * item );
	static void				Use_Item( Entity * ent, Entity * other, Entity * activator );
	static void				droptofloor( Entity * ent );
	static void				PrecacheItem( GameItem * it );
	static void				SpawnItem( Entity * ent, GameItem * item );
	static void				SP_item_health( Entity * self );
	static void				SP_item_health_small( Entity * self );
	static void				SP_item_health_large( Entity * self );
	static void				SP_item_health_mega( Entity * self );
	static void				InitItems( );
	static void				SetItemNames( );

	// g_main.cpp
	static void				ClientEndServerFrames( );
	static Entity *			CreateTargetChangeLevel( const Str & map );
	static void				EndDMLevel( );
	static void				CheckNeedPass( );
	static void				CheckDMRules( );
	static void				ExitLevel( );

	// g_misc.cpp
	static void				Use_Areaportal( Entity * ent, Entity * other, Entity * activator );
	static void				SP_func_areaportal( Entity * ent );
	static Vec3				VelocityForDamage( int damage );
	static void				ClipGibVelocity( Entity * ent );
	static void				gib_think( Entity * self );
	static void				gib_touch( Entity * self, Entity * other, Plane & plane, int surf );
	static void				gib_die( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point );
	static void				ThrowGib( Entity * self, const Str & gibname, int damage, int type );
	static void				ThrowHead( Entity * self, const Str & gibname, int damage, int type );
	static void				ThrowClientHead( Entity * self, int damage );
	static void				debris_die( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point );
	static void				ThrowDebris( Entity * self, const Str & modelname, float speed, Vec3 & origin );
	static void				BecomeExplosion1( Entity * self );
	static void				BecomeExplosion2( Entity * self );
	static void				path_corner_touch( Entity * self, Entity * other, Plane & plane, int surf );
	static void				SP_path_corner( Entity * self );
	static void				point_combat_touch( Entity * self, Entity * other, Plane & plane, int surf );
	static void				SP_point_combat( Entity * self );
	static void				TH_viewthing( Entity * ent );
	static void				SP_viewthing( Entity * ent );
	static void				SP_info_null( Entity * self );
	static void				SP_info_notnull( Entity * self );
	static void				light_use( Entity * self, Entity * other, Entity * activator );
	static void				SP_light( Entity * self );
	static void				func_wall_use( Entity * self, Entity * other, Entity * activator );
	static void				SP_func_wall( Entity * self );
	static void				func_object_touch( Entity * self, Entity * other, Plane & plane, int surf );
	static void				func_object_release( Entity * self );
	static void				func_object_use( Entity * self, Entity * other, Entity * activator );
	static void				SP_func_object( Entity * self );
	static void				func_explosive_explode( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point );
	static void				func_explosive_use( Entity * self, Entity * other, Entity * activator );
	static void				func_explosive_spawn( Entity * self, Entity * other, Entity * activator );
	static void				SP_func_explosive( Entity * self );
	static void				barrel_touch( Entity * self, Entity * other, Plane & plane, int surf );
	static void				barrel_explode( Entity * self );
	static void				barrel_delay( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point );
	static void				SP_misc_explobox( Entity * self );
	static void				misc_blackhole_use( Entity * ent, Entity * other, Entity * activator );
	static void				misc_blackhole_think( Entity * self );
	static void				SP_misc_blackhole( Entity * ent );
	static void				misc_eastertank_think( Entity * self );
	static void				SP_misc_eastertank( Entity * ent );
	static void				misc_easterchick_think( Entity * self );
	static void				SP_misc_easterchick( Entity * ent );
	static void				misc_easterchick2_think( Entity * self );
	static void				SP_misc_easterchick2( Entity * ent );
	static void				commander_body_think( Entity * self );
	static void				commander_body_use( Entity * self, Entity * other, Entity * activator );
	static void				commander_body_drop( Entity * self );
	static void				SP_monster_commander_body( Entity * self );
	static void				misc_banner_think( Entity * ent );
	static void				SP_misc_banner( Entity * ent );
	static void				misc_deadsoldier_die( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point );
	static void				SP_misc_deadsoldier( Entity * ent );
	static void				misc_viper_use( Entity * self, Entity * other, Entity * activator );
	static void				SP_misc_viper( Entity * ent );
	static void				SP_misc_bigviper( Entity * ent );
	static void				misc_viper_bomb_touch( Entity * self, Entity * other, Plane & plane, int surf );
	static void				misc_viper_bomb_prethink( Entity * self );
	static void				misc_viper_bomb_use( Entity * self, Entity * other, Entity * activator );
	static void				SP_misc_viper_bomb( Entity * self );
	static void				misc_strogg_ship_use( Entity * self, Entity * other, Entity * activator );
	static void				SP_misc_strogg_ship( Entity * ent );
	static void				misc_satellite_dish_think( Entity * self );
	static void				misc_satellite_dish_use( Entity * self, Entity * other, Entity * activator );
	static void				SP_misc_satellite_dish( Entity * ent );
	static void				SP_light_mine1( Entity * ent );
	static void				SP_light_mine2( Entity * ent );
	static void				SP_misc_gib_arm( Entity * ent );
	static void				SP_misc_gib_leg( Entity * ent );
	static void				SP_misc_gib_head( Entity * ent );
	static void				SP_target_character( Entity * self );
	static void				target_string_use( Entity * self, Entity * other, Entity * activator );
	static void				SP_target_string( Entity * self );
	static void				func_clock_reset( Entity * self );
	static void				func_clock_format_countdown( Entity * self );
	static void				func_clock_think( Entity * self );
	static void				func_clock_use( Entity * self, Entity * other, Entity * activator );
	static void				SP_func_clock( Entity * self );
	static void				teleporter_touch( Entity * self, Entity * other, Plane & plane, int surf );
	static void				SP_misc_teleporter( Entity * ent );
	static void				SP_misc_teleporter_dest( Entity * ent );

	// g_monster.cpp
	static void				monster_fire_bullet( Entity * self, Vec3 & start, Vec3 & dir, int damage, int kick, int hspread, int vspread, int flashtype );
	static void				monster_fire_shotgun( Entity * self, Vec3 & start, Vec3 & aimdir, int damage, int kick, int hspread, int vspread, int count, int flashtype );
	static void				monster_fire_blaster( Entity * self, Vec3 & start, Vec3 & dir, int damage, int speed, int flashtype, int effect );
	static void				monster_fire_grenade( Entity * self, Vec3 & start, Vec3 & aimdir, int damage, int speed, int flashtype );
	static void				monster_fire_rocket( Entity * self, Vec3 & start, Vec3 & dir, int damage, int speed, int flashtype );
	static void				monster_fire_railgun( Entity * self, Vec3 & start, Vec3 & aimdir, int damage, int kick, int flashtype );
	static void				monster_fire_bfg( Entity * self, Vec3 & start, Vec3 & aimdir, int damage, int speed, int kick, float damage_radius, int flashtype );
	static void				M_FliesOff( Entity * self );
	static void				M_FliesOn( Entity * self );
	static void				M_FlyCheck( Entity * self );
	static void				AttackFinished( Entity * self, float time );
	static void				M_CheckGround( Entity * ent );
	static void				M_CatagorizePosition( Entity * ent );
	static void				M_WorldEffects( Entity * ent );
	static void				M_droptofloor( Entity * ent );
	static void				M_SetEffects( Entity * ent );
	static void				M_MoveFrame( Entity * self );
	static void				monster_think( Entity * self );
	static void				monster_use( Entity * self, Entity * other, Entity * activator );
	static void				monster_triggered_spawn( Entity * self );
	static void				monster_triggered_spawn_use( Entity * self, Entity * other, Entity * activator );
	static void				monster_triggered_start( Entity * self );
	static void				monster_death_use( Entity * self );
	static bool				monster_start( Entity * self );
	static void				monster_start_go( Entity * self );
	static void				walkmonster_start_go( Entity * self );
	static void				walkmonster_start( Entity * self );
	static void				flymonster_start_go( Entity * self );
	static void				flymonster_start( Entity * self );
	static void				swimmonster_start_go( Entity * self );
	static void				swimmonster_start( Entity * self );

	// g_phys.cpp
	static Entity * 		SV_TestEntityPosition( Entity * ent );
	static void				SV_CheckVelocity( Entity * ent );
	// Runs thinking code for this frame if necessary
	static bool				SV_RunThink( Entity * ent );
	// Two entities have touched, so run their touch functions
	static void				SV_Impact( Entity * e1, CTrace & trace );
	// Slide off of the impacting object. returns the blocked flags( 1 = floor, 2 = step / wall )
	static int				ClipVelocity( Vec3 & in, Vec3 & normal, Vec3 * out, float overbounce );
	// The basic solid body movement clip that slides along multiple planes. Returns the clipflags if the velocity was modified( hit something solid )
	// 1 = floor, 2 = wall / step, 4 = dead stop
	static int				SV_FlyMove( Entity * ent, float time, int mask );
	static void				SV_AddGravity( Entity * ent );
	// Does not change the entities velocity at all
	static CTrace			SV_PushEntity( Entity * ent, Vec3 & push );
	// Objects need to be moved back on a failed push, otherwise riders would continue to slide.
	static bool				SV_Push( Entity * pusher, Vec3 & move, Vec3 & amove );
	// Bmodel objects don't interact with each other, but push all box objects
	static void				SV_Physics_Pusher( Entity * ent );
	// Non moving objects can only think
	static void				SV_Physics_None( Entity * ent );
	// A moving object that doesn't obey physics
	static void				SV_Physics_Noclip( Entity * ent );
	// Toss, bounce, and fly movement.  When onground, do nothing.
	static void				SV_Physics_Toss( Entity * ent );
	// Monsters freefall when they don't have a ground entity, otherwise all movement is done with discrete steps.
	// This is also used for objects that have become still on the ground, but will fall if the floor is pulled out from under them. FIXME: is this true?
	static void				SV_AddRotationalFriction( Entity * ent );
	static void				SV_Physics_Step( Entity * ent );
	static void				G_RunEntity( Entity * ent );

	// g_save.cpp
	static void				WriteField1( FileBase * f, const field_t * field, byte * base );
	static void				WriteField2( FileBase * f, const field_t * field, byte * base );
	static void				ReadField( FileBase * f, const field_t * field, byte * base );
	static void				WriteClient( FileBase * f, gclient_t * client );
	static void				ReadClient( FileBase * f, gclient_t * client );
	static void				WriteEdict( FileBase * f, Entity * ent );
	static void				WriteLevelLocals( FileBase * f );
	static void				ReadEdict( FileBase * f, Entity * ent );
	static void				ReadLevelLocals( FileBase * f );

	// g_spawn.cpp
	static void				G_FindTeams( );
	static void				ED_CallSpawn( Entity * ent );
	static void				ED_ParseField( const Str & key, const Str & value, Entity * ent );
	//static void				ED_ParseEdict( Lexer & lexer, Entity * ent );
	static void				ED_ParseEdict( const MapEntity & mapEnt, Entity * gameEnt );
	static void				SP_worldspawn( Entity * ent );

	// g_svcmds.cpp
	static void				Svcmd_Test_f( );
	static bool				StringToFilter( const Str & s, struct ipfilter_s * f );
	static bool				SV_FilterPacket( const Str & from );
	static void				SVCmd_AddIP_f( );
	static void				SVCmd_RemoveIP_f( );
	static void				SVCmd_ListIP_f( );
	static void				SVCmd_WriteIP_f( );

	// g_target.cpp
	static void				Use_Target_Tent( Entity * ent, Entity * other, Entity * activator );
	static void				SP_target_temp_entity( Entity * ent );
	static void				Use_Target_Speaker( Entity * ent, Entity * other, Entity * activator );
	static void				SP_target_speaker( Entity * ent );
	static void				Use_Target_Help( Entity * ent, Entity * other, Entity * activator );
	static void				SP_target_help( Entity * ent );
	static void				use_target_secret( Entity * ent, Entity * other, Entity * activator );
	static void				SP_target_secret( Entity * ent );
	static void				use_target_goal( Entity * ent, Entity * other, Entity * activator );
	static void				SP_target_goal( Entity * ent );
	static void				target_explosion_explode( Entity * self );
	static void				use_target_explosion( Entity * self, Entity * other, Entity * activator );
	static void				SP_target_explosion( Entity * ent );
	static void				use_target_changelevel( Entity * self, Entity * other, Entity * activator );
	static void				SP_target_changelevel( Entity * ent );
	static void				use_target_splash( Entity * self, Entity * other, Entity * activator );
	static void				SP_target_splash( Entity * self );
	static void				use_target_spawner( Entity * self, Entity * other, Entity * activator );
	static void				SP_target_spawner( Entity * self );
	static void				use_target_blaster( Entity * self, Entity * other, Entity * activator );
	static void				SP_target_blaster( Entity * self );
	static void				trigger_crosslevel_trigger_use( Entity * self, Entity * other, Entity * activator );
	static void				SP_target_crosslevel_trigger( Entity * self );
	static void				target_crosslevel_target_think( Entity * self );
	static void				SP_target_crosslevel_target( Entity * self );
	static void				target_laser_think( Entity * self );
	static void				target_laser_on( Entity * self );
	static void				target_laser_off( Entity * self );
	static void				target_laser_use( Entity * self, Entity * other, Entity * activator );
	static void				target_laser_start( Entity * self );
	static void				SP_target_laser( Entity * self );
	static void				target_lightramp_think( Entity * self );
	static void				target_lightramp_use( Entity * self, Entity * other, Entity * activator );
	static void				SP_target_lightramp( Entity * self );
	static void				target_earthquake_think( Entity * self );
	static void				target_earthquake_use( Entity * self, Entity * other, Entity * activator );
	static void				SP_target_earthquake( Entity * self );

	// g_tigger.cpp
	static void				InitTrigger( Entity * self );
	static void				multi_wait( Entity * ent );
	static void				multi_trigger( Entity * ent );
	static void				Use_Multi( Entity * ent, Entity * other, Entity * activator );
	static void				Touch_Multi( Entity * self, Entity * other, Plane & plane, int surf );
	static void				trigger_enable( Entity  * self, Entity * other, Entity * activator );
	static void				SP_trigger_multiple( Entity * ent );
	static void				SP_trigger_once( Entity * ent );
	static void				trigger_relay_use( Entity * self, Entity * other, Entity * activator );
	static void				SP_trigger_relay( Entity * self );
	static void				trigger_key_use( Entity * self, Entity * other, Entity * activator );
	static void				SP_trigger_key( Entity * self );
	static void				trigger_counter_use( Entity * self, Entity * other, Entity * activator );
	static void				SP_trigger_counter( Entity * self );
	static void				SP_trigger_always( Entity * ent );
	static void				trigger_push_touch( Entity * self, Entity * other, Plane & plane, int surf );
	static void				SP_trigger_push( Entity * self );
	static void				hurt_use( Entity * self, Entity * other, Entity * activator );
	static void				hurt_touch( Entity * self, Entity * other, Plane & plane, int surf );
	static void				SP_trigger_hurt( Entity * self );
	static void				trigger_gravity_touch( Entity * self, Entity * other, Plane & plane, int surf );
	static void				SP_trigger_gravity( Entity * self );
	static void				trigger_monsterjump_touch( Entity * self, Entity * other, Plane & plane, int surf );
	static void				SP_trigger_monsterjump( Entity * self );

	// g_utils.cpp
	static void				G_ProjectSource( const Vec3 & point, const Vec3 & distance, const Vec3 & forward, const Vec3 & right, Vec3 & result );
	static Entity *			G_Find( Entity * from, int fieldofs, const Str & match );
	static Entity *			findradius( Entity * from, Vec3 & org, float rad );
	static Entity *			G_PickTarget( const Str & targetname );
	static void				Think_Delay( Entity * ent );
	static void				G_UseTargets( Entity * ent, Entity * activator );
	static const Str		vtos( Vec3 & v );
	static void				G_SetMovedir( Vec3 & angles, Vec3 * movedir );
	static float			vectoyaw( Vec3 & vec );
	static void				vectoangles( Vec3 & value1, Vec3 * angles );
	static void				G_InitEdict( Entity * e );
	static Entity *			G_Spawn( );
	static void				G_FreeEdict( Entity * ed );
	static void				G_TouchTriggers( Entity * ent );
	static void				G_TouchSolids( Entity * ent );
	static bool				KillBox( Entity * ent );

	// g_weapon.cpp
	static void				check_dodge( Entity * self, Vec3 & start, Vec3 & dir, int speed );
	static bool				fire_hit( Entity * self, Vec3 & aim, int damage, int kick );
	static void				fire_lead( Entity * self, Vec3 & start, Vec3 & aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod );
	static void				fire_bullet( Entity * self, Vec3 & start, Vec3 & aimdir, int damage, int kick, int hspread, int vspread, int mod );
	static void				fire_shotgun( Entity * self, Vec3 & start, Vec3 & aimdir, int damage, int kick, int hspread, int vspread, int count, int mod );
	static void				blaster_touch( Entity * self, Entity * other, Plane & plane, int surf );
	static void				fire_blaster( Entity * self, Vec3 & start, Vec3 & dir, int damage, int speed, int effect, bool vhyper );
	static void				Grenade_Explode( Entity * ent );
	static void				Grenade_Touch( Entity * ent, Entity * other, Plane & plane, int surf );
	static void				fire_grenade( Entity * self, Vec3 & start, Vec3 & aimdir, int damage, int speed, float timer, float damage_radius );
	static void				fire_grenade2( Entity * self, Vec3 & start, Vec3 & aimdir, int damage, int speed, float timer, float damage_radius, bool held );
	static void				rocket_touch( Entity * ent, Entity * other, Plane & plane, int surf );
	static void				fire_rocket( Entity * self, Vec3 & start, Vec3 & dir, int damage, int speed, float damage_radius, int radius_damage );
	static void				fire_rail( Entity * self, Vec3 & start, Vec3 & aimdir, int damage, int kick );
	static void				bfg_explode( Entity * self );
	static void				bfg_touch( Entity * self, Entity * other, Plane & plane, int surf );
	static void				bfg_think( Entity * self );
	static void				fire_bfg( Entity * self, Vec3 & start, Vec3 & dir, int damage, int speed, float damage_radius );

	// p_client.cpp
	static void				SP_FixCoopSpots( Entity * self );
	static void				SP_CreateCoopSpots( Entity * self );
	// info_player_start( 1 0 0 )( -16 -16 -24 )( 16 16 32 )
	// The normal starting point for a level.
	static void				SP_info_player_start( Entity * self );
	// info_player_deathmatch( 1 0 1 )( -16 -16 -24 )( 16 16 32 )
	// potential spawning position for deathmatch games
	static void				SP_info_player_deathmatch( Entity * self );
	// info_player_coop( 1 0 1 )( -16 -16 -24 )( 16 16 32 )
	// potential spawning position for coop games
	static void				SP_info_player_coop( Entity * self );
	//i nfo_player_intermission( 1 0 1 )( -16 -16 -24 )( 16 16 32 )
	// The deathmatch intermission point will be at one of these
	// Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
	static void				SP_info_player_intermission( Entity * ent );
	static void				player_pain( Entity * self, Entity * other, float kick, int damage );
	static bool				IsFemale( Entity * ent );
	static bool				IsNeutral( Entity * ent );
	static void				ClientObituary( Entity * self, Entity * inflictor, Entity * attacker );
	static void				TossClientWeapon( Entity * self );
	static void				LookAtKiller( Entity * self, Entity * inflictor, Entity * attacker );
	static void				player_die( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point );
	// This is only called when the game first initializes in single player, but is called after each death and level change in deathmatch
	static void				InitClientPersistant( gclient_t * client );
	static void				InitClientResp( gclient_t * client );
	// Some information that should be persistant, like health, is still stored in the edict structure, so it needs to
	// be mirrored out to the client structure before all the edicts are wiped.
	static void				SaveClientData( );
	static void				FetchClientEntData( Entity * ent );
	// Returns the distance to the nearest player from the given spot
	static float			PlayersRangeFromSpot( Entity * spot );
	// go to a random point, but NOT the two points closest to other players
	static Entity *			SelectRandomDeathmatchSpawnPoint( );
	static Entity *			SelectFarthestDeathmatchSpawnPoint( );
	static Entity *			SelectDeathmatchSpawnPoint( );
	static Entity *			SelectCoopSpawnPoint( Entity * ent );
	// Chooses a player start, deathmatch start, coop start, etc
	static void				SelectSpawnPoint( Entity * ent, Vec3 & origin, Vec3 & angles );
	static void				InitBodyQue( );
	static void				body_die( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point );
	static void				CopyToBodyQue( Entity * ent );
	static void				respawn( Entity * self );
	// only called when pers.spectator changes, note that resp.spectator should be the opposite of pers.spectator here
	static void				spectator_respawn( Entity * ent );
	// Called when a player connects to a server or respawns in a deathmatch.
	static void				PutClientInServer( Entity * ent );
	// A client has just connected to the server in deathmatch mode, so clear everything out before starting them.
	static void				ClientBeginDeathmatch( Entity * ent );
	static Entity *			pm_passent;
	static CTrace			PM_trace( Vec3 & start, Vec3 & mins, Vec3 & maxs, Vec3 & end );
	static unsigned			CheckBlock( void * b, int c );
	static void				PrintPmove( pmove_t * pm );
	// This will be called once for each server frame, before running any other entities in the world.
	static void				ClientBeginServerFrame( Entity * ent );

	// p_hud.cpp
	static void				MoveClientToIntermission( Entity * ent );
	static void				BeginIntermission( Entity * targ );
	static void				DeathmatchScoreboardMessage( Entity * ent, Entity * killer );
	static void				DeathmatchScoreboard( Entity * ent );
	static void				Cmd_Score_f( Entity * ent );
	static void				HelpComputer( Entity * ent );
	static void				Cmd_Help_f( Entity * ent );
	static void				G_SetStats( Entity * ent );
	static void				G_CheckChaseStats( Entity * ent );
	static void				G_SetSpectatorStats( Entity * ent );

	// p_trail.cpp
	#define	TRAIL_LENGTH	8
	static Entity * 		trail[ TRAIL_LENGTH ];
	static int				trail_head;
	static bool				trail_active;
	static void				PlayerTrail_Init( );
	static void				PlayerTrail_Add( Vec3 & spot );
	static void				PlayerTrail_New( Vec3 & spot );
	static Entity *			PlayerTrail_PickFirst( Entity * self );
	static Entity *			PlayerTrail_PickNext( Entity * self );
	static Entity *			PlayerTrail_LastSpot( );

	// p_view.cpp
	static Entity * 		view_current_player;
	static gclient_t *		view_current_client;
	static Vec3				view_forward;
	static Vec3				view_right;
	static Vec3				view_up;
	static float			view_xyspeed;
	static float			view_bobmove;
	static int				view_bobcycle;
	static float			view_bobfracsin;

	static float			SV_CalcRoll( Vec3 & angles, Vec3 & velocity );
	// Handles color blends and view kicks
	static void				P_DamageFeedback( Entity * player );
	static void				SV_CalcViewOffset( Entity * ent );
	static void				SV_CalcGunOffset( Entity * ent );
	static void				SV_AddBlend( float r, float g, float b, float a, float * v_blend );
	static void				SV_CalcBlend( Entity * ent );
	static void				P_FallingDamage( Entity * ent );
	static void				P_WorldEffects( );
	static void				G_SetClientEffects( Entity * ent );
	static void				G_SetClientEvent( Entity * ent );
	static void				G_SetClientSound( Entity * ent );
	static void				G_SetClientFrame( Entity * ent );
	// Called for each player at the end of the server frame and view_right after spawning
	static void				ClientEndServerFrame( Entity * ent );

	// p_weapon.cpp
	static bool				is_quad;
	static byte				is_silenced;

	static int				sm_meat_index;
	static int				snd_fry;
	static int				meansOfDeath;

	static void				P_ProjectSource( gclient_t * client, const Vec3 & point, const Vec3 & distance, const Vec3 & forward, const Vec3 & right, Vec3 & result );
	static void				PlayerNoise( Entity * who, Vec3 & where, int type );
	static bool				Pickup_Weapon( Entity * ent, Entity * other );
	static void				ChangeWeapon( Entity * ent );
	static void				NoAmmoWeaponChange( Entity * ent );
	static void				Think_Weapon( Entity * ent );
	static void				Use_Weapon( Entity * ent, GameItem * item );
	static void				Drop_Weapon( Entity * ent, GameItem * item );
	static void				Weapon_Generic( Entity * ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int * pause_frames, int * fire_frames, void( *fire )( Entity * ent ) );
	static void				weapon_grenade_fire( Entity * ent, bool held );
	static void				Weapon_Grenade( Entity * ent );
	static void				weapon_grenadelauncher_fire( Entity * ent );
	static void				Weapon_GrenadeLauncher( Entity * ent );
	static void				Weapon_RocketLauncher_Fire( Entity * ent );
	static void				Weapon_RocketLauncher( Entity * ent );
	static void				Blaster_Fire( Entity * ent, Vec3 & g_offset, int damage, bool vhyper, int effect );
	static void				Weapon_Blaster_Fire( Entity * ent );
	static void				Weapon_Blaster( Entity * ent );
	static void				Weapon_HyperBlaster_Fire( Entity * ent );
	static void				Weapon_HyperBlaster( Entity * ent );
	static void				Machinegun_Fire( Entity * ent );
	static void				Weapon_Machinegun( Entity * ent );
	static void				Chaingun_Fire( Entity * ent );
	static void				Weapon_Chaingun( Entity * ent );
	static void				weapon_shotgun_fire( Entity * ent );
	static void				Weapon_Shotgun( Entity * ent );
	static void				weapon_supershotgun_fire( Entity * ent );
	static void				Weapon_SuperShotgun( Entity * ent );
	static void				weapon_railgun_fire( Entity * ent );
	static void				Weapon_Railgun( Entity * ent );
	static void				weapon_bfg_fire( Entity * ent );
	static void				Weapon_BFG( Entity * ent );

	// m_actor.cpp
	static const Str		actor_names[ ];
	static const Str		actor_messages[ ];

	static mframe_t			actor_frames_stand[ ];
	static mmove_t			actor_move_stand;
	static mframe_t			actor_frames_walk[ ];
	static mmove_t			actor_move_walk;
	static mframe_t			actor_frames_run[ ];
	static mmove_t			actor_move_run;
	static mframe_t			actor_frames_pain1[ ];
	static mmove_t			actor_move_pain1;
	static mframe_t			actor_frames_pain2[ ];
	static mmove_t			actor_move_pain2;
	static mframe_t			actor_frames_pain3[ ];
	static mmove_t			actor_move_pain3;
	static mframe_t			actor_frames_flipoff[ ];
	static mmove_t			actor_move_flipoff;
	static mframe_t			actor_frames_taunt[ ];
	static mmove_t			actor_move_taunt;
	static mframe_t			actor_frames_death1[ ];
	static mmove_t			actor_move_death1;
	static mframe_t			actor_frames_death2[ ];
	static mmove_t			actor_move_death2;
	static mframe_t			actor_frames_attack[ ];
	static mmove_t			actor_move_attack;

	static void				actor_stand( Entity * self );
	static void				actor_walk( Entity * self );
	static void				actor_run( Entity * self );
	static void				actor_pain( Entity * self, Entity * other, float kick, int damage );
	static void				actor_dead( Entity * self );
	static void				actor_die( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point );
	static void				actor_fire( Entity * self );
	static void				actor_attack( Entity * self );
	static void				actor_use( Entity * self, Entity * other, Entity * activator );
	static void				SP_misc_actor( Entity * self );
	static void				target_actor_touch( Entity * self, Entity * other, Plane & plane, int surf );
	static void				SP_target_actor( Entity * self );

	static void				actorMachineGun( Entity * self );

	// m_move.cpp
	static bool				M_CheckBottom( Entity * ent );
	static bool				SV_movestep( Entity * ent, Vec3 & move, bool relink );
	static void				M_ChangeYaw( Entity * ent );
	static bool				SV_StepDirection( Entity * ent, float yaw, float dist );
	static void				SV_FixCheckBottom( Entity * ent );
	static void				SV_NewChaseDir( Entity * actor, Entity * enemy, float dist );
	static bool				SV_CloseEnough( Entity * ent, Entity * goal, float dist );
	static void				M_MoveToGoal( Entity * ent, float dist );
	static bool				M_walkmove( Entity * ent, float yaw, float dist );

	// m_soldier.cpp
	static int				soldier_sound_idle;
	static int				soldier_sound_sight1;
	static int				soldier_sound_sight2;
	static int				soldier_sound_pain_light;
	static int				soldier_sound_pain;
	static int				soldier_sound_pain_ss;
	static int				soldier_sound_death_light;
	static int				soldier_sound_death;
	static int				soldier_sound_death_ss;
	static int				soldier_sound_cock;

	static void				soldier_idle( Entity * self );
	static void				soldier_cock( Entity * self );
	static mframe_t			soldier_frames_stand1[ ];
	static mmove_t			soldier_move_stand1;
	static mframe_t			soldier_frames_stand3[ ];
	static mmove_t			soldier_move_stand3;
	static mframe_t			soldier_frames_stand4[ ];
	static mmove_t			soldier_move_stand4;
	static void				soldier_stand( Entity * self );
	static void				soldier_walk1_random( Entity * self );
	static mframe_t			soldier_frames_walk1[ ];
	static mmove_t			soldier_move_walk1;
	static mframe_t			soldier_frames_walk2[ ];
	static mmove_t			soldier_move_walk2;
	static void				soldier_walk( Entity * self );
	static mframe_t			soldier_frames_start_run[ ];
	static mmove_t			soldier_move_start_run;
	static mframe_t			soldier_frames_run[ ];
	static mmove_t			soldier_move_run;
	static void				soldier_run( Entity * self );
	static mframe_t			soldier_frames_pain1[ ];
	static mmove_t			soldier_move_pain1;
	static mframe_t			soldier_frames_pain2[ ];
	static mmove_t			soldier_move_pain2;
	static mframe_t			soldier_frames_pain3[ ];
	static mmove_t			soldier_move_pain3;
	static mframe_t			soldier_frames_pain4[ ];
	static mmove_t			soldier_move_pain4;
	static void				soldier_pain( Entity * self, Entity * other, float kick, int damage );
	static int				blaster_flash[ ];
	static int				shotgun_flash[ ];
	static int				machinegun_flash[ ];
	static void				soldier_fire( Entity * self, int flash_number );
	static void				soldier_fire1( Entity * self );
	static void				soldier_attack1_refire1( Entity * self );
	static void				soldier_attack1_refire2( Entity * self );
	static mframe_t			soldier_frames_attack1[ ];
	static mmove_t			soldier_move_attack1;
	static void				soldier_fire2( Entity * self );
	static void				soldier_attack2_refire1( Entity * self );
	static void				soldier_attack2_refire2( Entity * self );
	static mframe_t			soldier_frames_attack2[ ];
	static mmove_t			soldier_move_attack2;
	static void				soldier_duck_down( Entity * self );
	static void				soldier_duck_up( Entity * self );
	static void				soldier_fire3( Entity * self );
	static void				soldier_attack3_refire( Entity * self );
	static mframe_t			soldier_frames_attack3[ ];
	static mmove_t			soldier_move_attack3;
	static void				soldier_fire4( Entity * self );
	static mframe_t			soldier_frames_attack4[ ];
	static mmove_t			soldier_move_attack4;
	static void				soldier_fire5( Entity * self );
	static void				soldier_attack5_refire( Entity * self );
	static mframe_t			soldier_frames_attack5[ ];
	static mmove_t			soldier_move_attack5;
	static void				soldier_fire8( Entity * self );
	static void				soldier_attack6_refire( Entity * self );
	static mframe_t			soldier_frames_attack6[ ];
	static mmove_t			soldier_move_attack6;
	static void				soldier_attack( Entity * self );
	static void				soldier_sight( Entity * self, Entity * other );
	static void				soldier_duck_hold( Entity * self );
	static mframe_t			soldier_frames_duck[ ];
	static mmove_t			soldier_move_duck;
	static void				soldier_dodge( Entity * self, Entity * attacker, float eta );
	static void				soldier_fire6( Entity * self );
	static void				soldier_fire7( Entity * self );
	static void				soldier_dead( Entity * self );
	static mframe_t			soldier_frames_death1[ ];
	static mmove_t			soldier_move_death1;
	static mframe_t			soldier_frames_death2[ ];
	static mmove_t			soldier_move_death2;
	static mframe_t			soldier_frames_death3[ ];
	static mmove_t			soldier_move_death3;
	static mframe_t			soldier_frames_death4[ ];
	static mmove_t			soldier_move_death4;
	static mframe_t			soldier_frames_death5[ ];
	static mmove_t			soldier_move_death5;
	static mframe_t			soldier_frames_death6[ ];
	static mmove_t			soldier_move_death6;
	static void				soldier_die( Entity * self, Entity * inflictor, Entity * attacker, int damage, Vec3 & point );
	static void				SP_monster_soldier_x( Entity * self );
	static void				SP_monster_soldier_light( Entity * self );
	static void				SP_monster_soldier( Entity * self );
	static void				SP_monster_soldier_ss( Entity * self );

public:

	static const Str		boneGib;
	static const Str		meatGib;
	static const Str		headGib;
	
	// The edict array is allocated in the game dll so it
	// can vary in size from one game to another.
	// 
	// The size will be fixed when ge->Init( ) is called

	static Entity * 		edicts;
	static int				edict_size;
	static int				num_edicts;		// current number, <= max_edicts
	static int				max_edicts;

	// the init function will only be called when a game starts, // not each time a level is loaded.  Persistant data for clients
	// and the server can be allocated in init

	static bool				Init( );
	static void				Shutdown( );

	// each new level entered will cause a call to SpawnEntities

	static void				SpawnEntities( const Str & mapname, Str & spoint );

	// Read/Write Game is for storing persistant cross level information
	// about the world state and the clients.
	// WriteGame is called every time a level is exited.
	// ReadGame is called on a loadgame.

	static void				WriteGame( const Str & filename, bool autosave );
	static void				ReadGame( const Str & filename );

	// ReadLevel is called after the default map information has been
	// loaded with SpawnEntities

	static void				WriteLevel( const Str & filename );
	static void				ReadLevel( const Str & filename );

	// Called when a player begins connecting to the server. The game can refuse entrance to a client by returning false.
	// If the client is allowed, the connection process will continue and eventually get to ClientBegin( )
	// Changing levels will NOT cause this to be called again, but loadgames will.
	static bool				ClientConnect( Entity * ent, Str & userinfo );
	// called when a client has finished connecting, and is ready to be placed into the game.  This will happen every level load.
	static void				ClientBegin( Entity * ent );
	// called whenever the player updates a userinfo variable. The game can override any of the settings in place ( forcing skins or names, etc ) before copying it off.
	static void				ClientUserinfoChanged( Entity * ent, Str & userinfo );
	// Called when a player drops from the server. Will not be called between levels.
	static void				ClientDisconnect( Entity * ent );
	static void				ClientCommand( Entity * ent );
	// This will be called once for each client frame, which will usually be a couple times for each server frame.
	static void				ClientThink( Entity * ent, usercmd_t * cmd );

	static void				RunFrame( );

	// ServerCommand will be called when an "sv <command>" command is issued on the
	// server console.
	// The game can issue Command::Cmd_Argc( ) / Command::Cmd_Argv( ) commands to get the rest
	// of the parameters

	static void				ServerCommand( );
};
