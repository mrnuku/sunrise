#include "precompiled.h"
#pragma hdrstop

// this file is included in both the game dll and quake2, // the game needs it to source shot locations, the client
// needs it to position muzzle flashes
Vec3 monster_flash_offset [] = {

	// flash 0 is not used
	Vec3( 0.0f, 0.0f, 0.0f ), 
	// MZ2_TANK_BLASTER_1				1
	Vec3( 20.7f, -18.5f, 28.7f ), // MZ2_TANK_BLASTER_2				2
	Vec3( 16.6f, -21.5f, 30.1f ), // MZ2_TANK_BLASTER_3				3
	Vec3( 11.8f, -23.9f, 32.1f ), // MZ2_TANK_MACHINEGUN_1			4
	Vec3( 22.9f, -0.7f, 25.3f ), // MZ2_TANK_MACHINEGUN_2			5
	Vec3( 22.2f, 6.2f, 22.3f ), // MZ2_TANK_MACHINEGUN_3			6
	Vec3( 19.4f, 13.1f, 18.6f ), // MZ2_TANK_MACHINEGUN_4			7
	Vec3( 19.4f, 18.8f, 18.6f ), // MZ2_TANK_MACHINEGUN_5			8
	Vec3( 17.9f, 25.0f, 18.6f ), // MZ2_TANK_MACHINEGUN_6			9
	Vec3( 14.1f, 30.5f, 20.6f ), // MZ2_TANK_MACHINEGUN_7			10
	Vec3( 9.3f, 35.3f, 22.1f ), // MZ2_TANK_MACHINEGUN_8			11
	Vec3( 4.7f, 38.4f, 22.1f ), // MZ2_TANK_MACHINEGUN_9			12
	Vec3( -1.1f, 40.4f, 24.1f ), // MZ2_TANK_MACHINEGUN_10			13
	Vec3( -6.5f, 41.2f, 24.1f ), // MZ2_TANK_MACHINEGUN_11			14
	Vec3( 3.2f, 40.1f, 24.7f ), // MZ2_TANK_MACHINEGUN_12			15
	Vec3( 11.7f, 36.7f, 26.0f ), // MZ2_TANK_MACHINEGUN_13			16
	Vec3( 18.9f, 31.3f, 26.0f ), // MZ2_TANK_MACHINEGUN_14			17
	Vec3( 24.4f, 24.4f, 26.4f ), // MZ2_TANK_MACHINEGUN_15			18
	Vec3( 27.1f, 17.1f, 27.2f ), // MZ2_TANK_MACHINEGUN_16			19
	Vec3( 28.5f, 9.1f, 28.0f ), // MZ2_TANK_MACHINEGUN_17			20
	Vec3( 27.1f, 2.2f, 28.0f ), // MZ2_TANK_MACHINEGUN_18			21
	Vec3( 24.9f, -2.8f, 28.0f ), // MZ2_TANK_MACHINEGUN_19			22
	Vec3( 21.6f, -7.0f, 26.4f ), // MZ2_TANK_ROCKET_1				23
	Vec3( 6.2f, 29.1f, 49.1f ), // MZ2_TANK_ROCKET_2				24
	Vec3( 6.9f, 23.8f, 49.1f ), // MZ2_TANK_ROCKET_3				25
	Vec3( 8.3f, 17.8f, 49.5f ), 
	// MZ2_INFANTRY_MACHINEGUN_1		26
	Vec3( 26.6f, 7.1f, 13.1f ), // MZ2_INFANTRY_MACHINEGUN_2		27
	Vec3( 18.2f, 7.5f, 15.4f ), // MZ2_INFANTRY_MACHINEGUN_3		28
	Vec3( 17.2f, 10.3f, 17.9f ), // MZ2_INFANTRY_MACHINEGUN_4		29
	Vec3( 17.0f, 12.8f, 20.1f ), // MZ2_INFANTRY_MACHINEGUN_5		30
	Vec3( 15.1f, 14.1f, 21.8f ), // MZ2_INFANTRY_MACHINEGUN_6		31
	Vec3( 11.8f, 17.2f, 23.1f ), // MZ2_INFANTRY_MACHINEGUN_7		32
	Vec3( 11.4f, 20.2f, 21.0f ), // MZ2_INFANTRY_MACHINEGUN_8		33
	Vec3( 9.0f, 23.0f, 18.9f ), // MZ2_INFANTRY_MACHINEGUN_9		34
	Vec3( 13.9f, 18.6f, 17.7f ), // MZ2_INFANTRY_MACHINEGUN_10		35
	Vec3( 15.4f, 15.6f, 15.8f ), // MZ2_INFANTRY_MACHINEGUN_11		36
	Vec3( 10.2f, 15.2f, 25.1f ), // MZ2_INFANTRY_MACHINEGUN_12		37
	Vec3( -1.9f, 15.1f, 28.2f ), // MZ2_INFANTRY_MACHINEGUN_13		38
	Vec3( -12.4f, 13.0f, 20.2f ), 
	// MZ2_SOLDIER_BLASTER_1			39
	Vec3( 10.6f * 1.2f, 7.7f * 1.2f, 7.8f * 1.2f ), // MZ2_SOLDIER_BLASTER_2			40
	Vec3( 21.1f * 1.2f, 3.6f * 1.2f, 19.0f * 1.2f ), // MZ2_SOLDIER_SHOTGUN_1			41
	Vec3( 10.6f * 1.2f, 7.7f * 1.2f, 7.8f * 1.2f ), // MZ2_SOLDIER_SHOTGUN_2			42
	Vec3( 21.1f * 1.2f, 3.6f * 1.2f, 19.0f * 1.2f ), // MZ2_SOLDIER_MACHINEGUN_1			43
	Vec3( 10.6f * 1.2f, 7.7f * 1.2f, 7.8f * 1.2f ), // MZ2_SOLDIER_MACHINEGUN_2			44
	Vec3( 21.1f * 1.2f, 3.6f * 1.2f, 19.0f * 1.2f ), 
	// MZ2_GUNNER_MACHINEGUN_1			45
	Vec3( 30.1f * 1.15f, 3.9f * 1.15f, 19.6f * 1.15f ), // MZ2_GUNNER_MACHINEGUN_2			46
	Vec3( 29.1f * 1.15f, 2.5f * 1.15f, 20.7f * 1.15f ), // MZ2_GUNNER_MACHINEGUN_3			47
	Vec3( 28.2f * 1.15f, 2.5f * 1.15f, 22.2f * 1.15f ), // MZ2_GUNNER_MACHINEGUN_4			48
	Vec3( 28.2f * 1.15f, 3.6f * 1.15f, 22.0f * 1.15f ), // MZ2_GUNNER_MACHINEGUN_5			49
	Vec3( 26.9f * 1.15f, 2.0f * 1.15f, 23.4f * 1.15f ), // MZ2_GUNNER_MACHINEGUN_6			50
	Vec3( 26.5f * 1.15f, 0.6f * 1.15f, 20.8f * 1.15f ), // MZ2_GUNNER_MACHINEGUN_7			51
	Vec3( 26.9f * 1.15f, 0.5f * 1.15f, 21.5f * 1.15f ), // MZ2_GUNNER_MACHINEGUN_8			52
	Vec3( 29.0f * 1.15f, 2.4f * 1.15f, 19.5f * 1.15f ), // MZ2_GUNNER_GRENADE_1				53
	Vec3( 4.6f * 1.15f, -16.8f * 1.15f, 7.3f * 1.15f ), // MZ2_GUNNER_GRENADE_2				54
	Vec3( 4.6f * 1.15f, -16.8f * 1.15f, 7.3f * 1.15f ), // MZ2_GUNNER_GRENADE_3				55
	Vec3( 4.6f * 1.15f, -16.8f * 1.15f, 7.3f * 1.15f ), // MZ2_GUNNER_GRENADE_4				56
	Vec3( 4.6f * 1.15f, -16.8f * 1.15f, 7.3f * 1.15f ), 
	// MZ2_CHICK_ROCKET_1				57
	//	-24.8f, -9.0f, 39.0f, Vec3( 24.8f, -9.0f, 39.0f ), // PGM - this was incorrect in Q2

	// MZ2_FLYER_BLASTER_1				58
	Vec3( 12.1f, 13.4f, -14.5f ), // MZ2_FLYER_BLASTER_2				59
	Vec3( 12.1f, -7.4f, -14.5f ), 
	// MZ2_MEDIC_BLASTER_1				60
	Vec3( 12.1f, 5.4f, 16.5f ), 
	// MZ2_GLADIATOR_RAILGUN_1			61
	Vec3( 30.0f, 18.0f, 28.0f ), 
	// MZ2_HOVER_BLASTER_1				62
	Vec3( 32.5f, -0.8f, 10.0f ), 
	// MZ2_ACTOR_MACHINEGUN_1			63
	Vec3( 18.4f, 7.4f, 9.6f ), 
	// MZ2_SUPERTANK_MACHINEGUN_1		64
	Vec3( 30.0f, 30.0f, 88.5f ), // MZ2_SUPERTANK_MACHINEGUN_2		65
	Vec3( 30.0f, 30.0f, 88.5f ), // MZ2_SUPERTANK_MACHINEGUN_3		66
	Vec3( 30.0f, 30.0f, 88.5f ), // MZ2_SUPERTANK_MACHINEGUN_4		67
	Vec3( 30.0f, 30.0f, 88.5f ), // MZ2_SUPERTANK_MACHINEGUN_5		68
	Vec3( 30.0f, 30.0f, 88.5f ), // MZ2_SUPERTANK_MACHINEGUN_6		69
	Vec3( 30.0f, 30.0f, 88.5f ), // MZ2_SUPERTANK_ROCKET_1			70
	Vec3( 16.0f, -22.5f, 91.2f ), // MZ2_SUPERTANK_ROCKET_2			71
	Vec3( 16.0f, -33.4f, 86.7f ), // MZ2_SUPERTANK_ROCKET_3			72
	Vec3( 16.0f, -42.8f, 83.3f ), 
	// --- Start Xian Stuff ---
	// MZ2_BOSS2_MACHINEGUN_L1			73
	Vec3( 32, -40, 70 ), // MZ2_BOSS2_MACHINEGUN_L2			74
	Vec3( 32, -40, 70 ), // MZ2_BOSS2_MACHINEGUN_L3			75
	Vec3( 32, -40, 70 ), // MZ2_BOSS2_MACHINEGUN_L4			76
	Vec3( 32, -40, 70 ), // MZ2_BOSS2_MACHINEGUN_L5			77
	Vec3( 32, -40, 70 ), // --- End Xian Stuff

	// MZ2_BOSS2_ROCKET_1				78
	Vec3( 22.0f, 16.0f, 10.0f ), // MZ2_BOSS2_ROCKET_2				79
	Vec3( 22.0f, 8.0f, 10.0f ), // MZ2_BOSS2_ROCKET_3				80
	Vec3( 22.0f, -8.0f, 10.0f ), // MZ2_BOSS2_ROCKET_4				81
	Vec3( 22.0f, -16.0f, 10.0f ), 
	// MZ2_FLOAT_BLASTER_1				82
	Vec3( 32.5f, -0.8f, 10 ), 
	// MZ2_SOLDIER_BLASTER_3			83
	Vec3( 20.8f * 1.2f, 10.1f * 1.2f, -2.7f * 1.2f ), // MZ2_SOLDIER_SHOTGUN_3			84
	Vec3( 20.8f * 1.2f, 10.1f * 1.2f, -2.7f * 1.2f ), // MZ2_SOLDIER_MACHINEGUN_3			85
	Vec3( 20.8f * 1.2f, 10.1f * 1.2f, -2.7f * 1.2f ), // MZ2_SOLDIER_BLASTER_4			86
	Vec3( 7.6f * 1.2f, 9.3f * 1.2f, 0.8f * 1.2f ), // MZ2_SOLDIER_SHOTGUN_4			87
	Vec3( 7.6f * 1.2f, 9.3f * 1.2f, 0.8f * 1.2f ), // MZ2_SOLDIER_MACHINEGUN_4			88
	Vec3( 7.6f * 1.2f, 9.3f * 1.2f, 0.8f * 1.2f ), // MZ2_SOLDIER_BLASTER_5			89
	Vec3( 30.5f * 1.2f, 9.9f * 1.2f, -18.7f * 1.2f ), // MZ2_SOLDIER_SHOTGUN_5			90
	Vec3( 30.5f * 1.2f, 9.9f * 1.2f, -18.7f * 1.2f ), // MZ2_SOLDIER_MACHINEGUN_5			91
	Vec3( 30.5f * 1.2f, 9.9f * 1.2f, -18.7f * 1.2f ), // MZ2_SOLDIER_BLASTER_6			92
	Vec3( 27.6f * 1.2f, 3.4f * 1.2f, -10.4f * 1.2f ), // MZ2_SOLDIER_SHOTGUN_6			93
	Vec3( 27.6f * 1.2f, 3.4f * 1.2f, -10.4f * 1.2f ), // MZ2_SOLDIER_MACHINEGUN_6			94
	Vec3( 27.6f * 1.2f, 3.4f * 1.2f, -10.4f * 1.2f ), // MZ2_SOLDIER_BLASTER_7			95
	Vec3( 28.9f * 1.2f, 4.6f * 1.2f, -8.1f * 1.2f ), // MZ2_SOLDIER_SHOTGUN_7			96
	Vec3( 28.9f * 1.2f, 4.6f * 1.2f, -8.1f * 1.2f ), // MZ2_SOLDIER_MACHINEGUN_7			97
	Vec3( 28.9f * 1.2f, 4.6f * 1.2f, -8.1f * 1.2f ), // MZ2_SOLDIER_BLASTER_8			98
	//	34.5f * 1.2f, 9.6f * 1.2f, 6.1f * 1.2f, Vec3( 31.5f * 1.2f, 9.6f * 1.2f, 10.1f * 1.2f ), // MZ2_SOLDIER_SHOTGUN_8			99
	Vec3( 34.5f * 1.2f, 9.6f * 1.2f, 6.1f * 1.2f ), // MZ2_SOLDIER_MACHINEGUN_8			100
	Vec3( 34.5f * 1.2f, 9.6f * 1.2f, 6.1f * 1.2f ), 
	// --- Xian shit below ---
	// MZ2_MAKRON_BFG					101
	Vec3( 17, -19.5f, 62.9f ), // MZ2_MAKRON_BLASTER_1				102
	Vec3( -3.6f, -24.1f, 59.5f ), // MZ2_MAKRON_BLASTER_2				103
	Vec3( -1.6f, -19.3f, 59.5f ), // MZ2_MAKRON_BLASTER_3				104
	Vec3( -0.1f, -14.4f, 59.5f ), // MZ2_MAKRON_BLASTER_4				105
	Vec3( 2.0f, -7.6f, 59.5f ), // MZ2_MAKRON_BLASTER_5				106
	Vec3( 3.4f, 1.3f, 59.5f ), // MZ2_MAKRON_BLASTER_6				107
	Vec3( 3.7f, 11.1f, 59.5f ), // MZ2_MAKRON_BLASTER_7				108
	Vec3( -0.3f, 22.3f, 59.5f ), // MZ2_MAKRON_BLASTER_8				109
	Vec3( -6, 33, 59.5f ), // MZ2_MAKRON_BLASTER_9				110
	Vec3( -9.3f, 36.4f, 59.5f ), // MZ2_MAKRON_BLASTER_10			111
	Vec3( -7, 35, 59.5f ), // MZ2_MAKRON_BLASTER_11			112
	Vec3( -2.1f, 29, 59.5f ), // MZ2_MAKRON_BLASTER_12			113
	Vec3( 3.9f, 17.3f, 59.5f ), // MZ2_MAKRON_BLASTER_13			114
	Vec3( 6.1f, 5.8f, 59.5f ), // MZ2_MAKRON_BLASTER_14			115
	Vec3( 5.9f, -4.4f, 59.5f ), // MZ2_MAKRON_BLASTER_15			116
	Vec3( 4.2f, -14.1f, 59.5f ), // MZ2_MAKRON_BLASTER_16			117
	Vec3( 2.4f, -18.8f, 59.5f ), // MZ2_MAKRON_BLASTER_17			118
	Vec3( -1.8f, -25.5f, 59.5f ), // MZ2_MAKRON_RAILGUN_1				119
	Vec3( -17.3f, 7.8f, 72.4f ), 
	// MZ2_JORG_MACHINEGUN_L1			120
	Vec3( 78.5f, -47.1f, 96 ), // MZ2_JORG_MACHINEGUN_L2			121
	Vec3( 78.5f, -47.1f, 96 ), // MZ2_JORG_MACHINEGUN_L3			122
	Vec3( 78.5f, -47.1f, 96 ), // MZ2_JORG_MACHINEGUN_L4			123
	Vec3( 78.5f, -47.1f, 96 ), // MZ2_JORG_MACHINEGUN_L5			124
	Vec3( 78.5f, -47.1f, 96 ), // MZ2_JORG_MACHINEGUN_L6			125
	Vec3( 78.5f, -47.1f, 96 ), // MZ2_JORG_MACHINEGUN_R1			126
	Vec3( 78.5f, 46.7f, 96 ), // MZ2_JORG_MACHINEGUN_R2			127
	Vec3( 78.5f, 46.7f, 96 ), // MZ2_JORG_MACHINEGUN_R3			128
	Vec3( 78.5f, 46.7f, 96 ), // MZ2_JORG_MACHINEGUN_R4			129
	Vec3( 78.5f, 46.7f, 96 ), // MZ2_JORG_MACHINEGUN_R5			130
	Vec3( 78.5f, 46.7f, 96 ), // MZ2_JORG_MACHINEGUN_R6			131
	Vec3( 78.5f, 46.7f, 96 ), // MZ2_JORG_BFG_1					132
	Vec3( 6.3f, -9, 111.2f ), 
	// MZ2_BOSS2_MACHINEGUN_R1			73
	Vec3( 32, 40, 70 ), // MZ2_BOSS2_MACHINEGUN_R2			74
	Vec3( 32, 40, 70 ), // MZ2_BOSS2_MACHINEGUN_R3			75
	Vec3( 32, 40, 70 ), // MZ2_BOSS2_MACHINEGUN_R4			76
	Vec3( 32, 40, 70 ), // MZ2_BOSS2_MACHINEGUN_R5			77
	Vec3( 32, 40, 70 ), 
	// --- End Xian Shit ---

	// ROGUE
	// note that the above really ends at 137
	// carrier machineguns
	// MZ2_CARRIER_MACHINEGUN_L1
	Vec3( 56, -32, 32 ), // MZ2_CARRIER_MACHINEGUN_R1
	Vec3( 56, 32, 32 ), // MZ2_CARRIER_GRENADE
	Vec3( 42, 24, 50 ), // MZ2_TURRET_MACHINEGUN			141
	Vec3( 16, 0, 0 ), // MZ2_TURRET_ROCKET				142
	Vec3( 16, 0, 0 ), // MZ2_TURRET_BLASTER				143
	Vec3( 16, 0, 0 ), // MZ2_STALKER_BLASTER				144
	Vec3( 24, 0, 6 ), // MZ2_DAEDALUS_BLASTER				145
	Vec3( 32.5f, -0.8f, 10.0f ), // MZ2_MEDIC_BLASTER_2				146
	Vec3( 12.1f, 5.4f, 16.5f ), // MZ2_CARRIER_RAILGUN				147
	Vec3( 32, 0, 6 ), // MZ2_WIDOW_DISRUPTOR				148
	Vec3( 57.72f, 14.50f, 88.81f ), // MZ2_WIDOW_BLASTER				149
	Vec3( 56, 32, 32 ), // MZ2_WIDOW_RAIL					150
	Vec3( 62, -20, 84 ), // MZ2_WIDOW_PLASMABEAM				151		// PMM - not used!
	Vec3( 32, 0, 6 ), // MZ2_CARRIER_MACHINEGUN_L2		152
	Vec3( 61, -32, 12 ), // MZ2_CARRIER_MACHINEGUN_R2		153
	Vec3( 61, 32, 12 ), // MZ2_WIDOW_RAIL_LEFT				154
	Vec3( 17, -62, 91 ), // MZ2_WIDOW_RAIL_RIGHT				155
	Vec3( 68, 12, 86 ), // MZ2_WIDOW_BLASTER_SWEEP1			156			pmm - the sweeps need to be in sequential order
	Vec3( 47.5f, 56, 89 ), // MZ2_WIDOW_BLASTER_SWEEP2			157
	Vec3( 54, 52, 91 ), // MZ2_WIDOW_BLASTER_SWEEP3			158
	Vec3( 58, 40, 91 ), // MZ2_WIDOW_BLASTER_SWEEP4			159
	Vec3( 68, 30, 88 ), // MZ2_WIDOW_BLASTER_SWEEP5			160
	Vec3( 74, 20, 88 ), // MZ2_WIDOW_BLASTER_SWEEP6			161
	Vec3( 73, 11, 87 ), // MZ2_WIDOW_BLASTER_SWEEP7			162
	Vec3( 73, 3, 87 ), // MZ2_WIDOW_BLASTER_SWEEP8			163
	Vec3( 70, -12, 87 ), // MZ2_WIDOW_BLASTER_SWEEP9			164
	Vec3( 67, -20, 90 ), // MZ2_WIDOW_BLASTER_100			165
	Vec3( -20, 76, 90 ), // MZ2_WIDOW_BLASTER_90				166
	Vec3( -8, 74, 90 ), // MZ2_WIDOW_BLASTER_80				167
	Vec3( 0, 72, 90 ), // MZ2_WIDOW_BLASTER_70				168		d06
	Vec3( 10, 71, 89 ), // MZ2_WIDOW_BLASTER_60				169		d07
	Vec3( 23, 70, 87 ), // MZ2_WIDOW_BLASTER_50				170		d08
	Vec3( 32, 64, 85 ), // MZ2_WIDOW_BLASTER_40				171
	Vec3( 40, 58, 84 ), // MZ2_WIDOW_BLASTER_30				172		d10
	Vec3( 48, 50, 83 ), // MZ2_WIDOW_BLASTER_20				173
	Vec3( 54, 42, 82 ), // MZ2_WIDOW_BLASTER_10				174		d12
	Vec3( 56, 34, 82 ), // MZ2_WIDOW_BLASTER_0				175
	Vec3( 58, 26, 82 ), // MZ2_WIDOW_BLASTER_10L			176		d14
	Vec3( 60, 16, 82 ), // MZ2_WIDOW_BLASTER_20L			177
	Vec3( 59, 6, 81 ), // MZ2_WIDOW_BLASTER_30L			178		d16
	Vec3( 58, -2, 80 ), // MZ2_WIDOW_BLASTER_40L			179
	Vec3( 57, -10, 79 ), // MZ2_WIDOW_BLASTER_50L			180		d18
	Vec3( 54, -18, 78 ), // MZ2_WIDOW_BLASTER_60L			181
	Vec3( 42, -32, 80 ), // MZ2_WIDOW_BLASTER_70L			182		d20
	Vec3( 36, -40, 78 ), // MZ2_WIDOW_RUN_1					183
	Vec3( 68.4f, 10.88f, 82.08f ), // MZ2_WIDOW_RUN_2					184
	Vec3( 68.51f, 8.64f, 85.14f ), // MZ2_WIDOW_RUN_3					185
	Vec3( 68.66f, 6.38f, 88.78f ), // MZ2_WIDOW_RUN_4					186
	Vec3( 68.73f, 5.1f, 84.47f ), // MZ2_WIDOW_RUN_5					187
	Vec3( 68.82f, 4.79f, 80.52f ), // MZ2_WIDOW_RUN_6					188
	Vec3( 68.77f, 6.11f, 85.37f ), // MZ2_WIDOW_RUN_7					189
	Vec3( 68.67f, 7.99f, 90.24f ), // MZ2_WIDOW_RUN_8					190
	Vec3( 68.55f, 9.54f, 87.36f ), // MZ2_CARRIER_ROCKET_1				191
	Vec3( 0, 0, -5 ), // MZ2_CARRIER_ROCKET_2				192
	Vec3( 0, 0, -5 ), // MZ2_CARRIER_ROCKET_3				193
	Vec3( 0, 0, -5 ), // MZ2_CARRIER_ROCKET_4				194
	Vec3( 0, 0, -5 ), // MZ2_WIDOW2_BEAMER_1				195
	//	72.13f, -17.63f, 93.77f, Vec3( 69.00f, -17.63f, 93.77f ), // MZ2_WIDOW2_BEAMER_2				196
	//	71.46f, -17.08f, 89.82f, Vec3( 69.00f, -17.08f, 89.82f ), // MZ2_WIDOW2_BEAMER_3				197
	//	71.47f, -18.40f, 90.70f, Vec3( 69.00f, -18.40f, 90.70f ), // MZ2_WIDOW2_BEAMER_4				198
	//	71.96f, -18.34f, 94.32f, Vec3( 69.00f, -18.34f, 94.32f ), // MZ2_WIDOW2_BEAMER_5				199
	//	72.25f, -18.30f, 97.98f, Vec3( 69.00f, -18.30f, 97.98f ), // MZ2_WIDOW2_BEAM_SWEEP_1			200
	Vec3( 45.04f, -59.02f, 92.24f ), // MZ2_WIDOW2_BEAM_SWEEP_2			201
	Vec3( 50.68f, -54.70f, 91.96f ), // MZ2_WIDOW2_BEAM_SWEEP_3			202
	Vec3( 56.57f, -47.72f, 91.65f ), // MZ2_WIDOW2_BEAM_SWEEP_4			203
	Vec3( 61.75f, -38.75f, 91.38f ), // MZ2_WIDOW2_BEAM_SWEEP_5			204
	Vec3( 65.55f, -28.76f, 91.24f ), // MZ2_WIDOW2_BEAM_SWEEP_6			205
	Vec3( 67.79f, -18.90f, 91.22f ), // MZ2_WIDOW2_BEAM_SWEEP_7			206
	Vec3( 68.60f, -9.52f, 91.23f ), // MZ2_WIDOW2_BEAM_SWEEP_8			207
	Vec3( 68.08f, 0.18f, 91.32f ), // MZ2_WIDOW2_BEAM_SWEEP_9			208
	Vec3( 66.14f, 9.79f, 91.44f ), // MZ2_WIDOW2_BEAM_SWEEP_10			209
	Vec3( 62.77f, 18.91f, 91.65f ), // MZ2_WIDOW2_BEAM_SWEEP_11			210
	Vec3( 58.29f, 27.11f, 92.00f ), 
	// end of table
	Vec3( 0.0f, 0.0f, 0.0f )
};
