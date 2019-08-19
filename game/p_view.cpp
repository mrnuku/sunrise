#include "precompiled.h"
#pragma hdrstop

#include "m_player.h"

float Game::SV_CalcRoll( Vec3 & angles, Vec3 & velocity ) {
	float side = velocity * view_right;
	float sign = side < 0 ? -1.0f : 1.0f;
	side = fabs( side );
	if( side < sv_rollspeed.GetFloat( ) )
		side = side * sv_rollangle.GetFloat( ) / sv_rollspeed.GetFloat( );
	else
		side = sv_rollangle.GetFloat( );
	return side * sign;
}

void Game::P_DamageFeedback( Entity * player ) {
	gclient_t * client;
	float side;
	float realcount, count, kick;
	Vec3 v;
	int r, l;
	static Vec3			power_color = Vec3( 0.0f, 1.0f, 0.0f );
	static Vec3			acolor = Vec3( 1.0f, 1.0f, 1.0f );
	static Vec3			bcolor = Vec3( 1.0f, 0.0f, 0.0f );
	client = player->client;
	// flash the backgrounds behind the status numbers
	client->ps.stats[ STAT_FLASHES ] = 0;
	if( client->damage_blood )
		client->ps.stats[ STAT_FLASHES ] |= 1;
	if( client->damage_armor && !( player->flags & FL_GODMODE ) &&( client->invincible_framenum <= level.framenum ) )
		client->ps.stats[ STAT_FLASHES ] |= 2;
	// total points of damage shot at the player this frame
	count = ( float )( ( client->damage_blood + client->damage_armor + client->damage_parmor ) );
	if( count == 0 )
		return; // didn't take any damage
	// start a pain animation if still in the player model
	if( client->anim_priority < ANIM_PAIN && player->s.modelindex == 255 ) {
		static int			i;
		client->anim_priority = ANIM_PAIN;
		if( client->ps.pmove.pm_flags & PMF_DUCKED ) {
			player->s.frame = FRAME_crpain1-1;
			client->anim_end = FRAME_crpain4;
		} else {
			i = ( i + 1 ) % 3;
			switch( i ) {
				case 0:
					player->s.frame = FRAME_pain101-1;
					client->anim_end = FRAME_pain104;
					break;
				case 1:
					player->s.frame = FRAME_pain201-1;
					client->anim_end = FRAME_pain204;
					break;
				case 2:
					player->s.frame = FRAME_pain301-1;
					client->anim_end = FRAME_pain304;
					break;
			}
		}
	}
	realcount = count;
	if( count < 10 )
		count = 10; // always make a visible effect
	// play an apropriate pain sound
	if( ( level.time > player->pain_debounce_time ) && !( player->flags & FL_GODMODE ) &&( client->invincible_framenum <= level.framenum ) ) {
		r = 1 +( rand( )&1 );
		player->pain_debounce_time = level.time + 0.7f;
		if( player->health < 25 )
			l = 25;
		else if( player->health < 50 )
			l = 50;
		else if( player->health < 75 )
			l = 75;
		else
			l = 100;
		Server::PF_StartSound( player, CHAN_VOICE, Server::SV_SoundIndex( va( "*pain%i_%i.wav", l, r ) ), 1, ATTN_NORM, 0 );
	}
	// the total alpha of the blend is always proportional to count
	if( client->damage_alpha < 0 )
		client->damage_alpha = 0;
	client->damage_alpha += count* 0.01f;
	if( client->damage_alpha < 0.2f )
		client->damage_alpha = 0.2f;
	if( client->damage_alpha > 0.6f )
		client->damage_alpha = 0.6f; // don't go too saturated
	// the color of the blend will vary based on how much was absorbed
	// by different armors
	v = vec3_origin;
	if( client->damage_parmor )
		v = v.Magnitude( ( float )client->damage_parmor/realcount, power_color );
	if( client->damage_armor )
		v = v.Magnitude( ( float )client->damage_armor/realcount, acolor );
	if( client->damage_blood )
		v = v.Magnitude( ( float )client->damage_blood/realcount, bcolor );
	client->damage_blend = v;
	// calculate view angle kicks
	kick = ( float )( abs( client->damage_knockback ) );
	if( kick && player->health > 0 ) { // kick of 0 means no view adjust at all
		kick = kick * 100 / player->health;
		if( kick < count* 0.5f )
			kick = count* 0.5f;
		if( kick > 50 )
			kick = 50;
		v = client->damage_from - player->s.origin;
		v.Normalize( );
		side = v * view_right;
		client->v_dmg_roll = kick* side* 0.3f;
		side = -( v * view_forward );
		client->v_dmg_pitch = kick* side* 0.3f;
		client->v_dmg_time = level.time + DAMAGE_TIME;
	}
	// clear totals
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_parmor = 0;
	client->damage_knockback = 0;
}

void Game::SV_CalcViewOffset( Entity * ent ) {
	float bob;
	float ratio;
	float delta;
	Vec3 v;
	// if dead, fix the angle and don't add any kick
	if( ent->deadflag ) {
		ent->client->ps.kick_angles = vec3_origin;
		ent->client->ps.viewangles[ ROLL ] = 40.0f;
		ent->client->ps.viewangles[ PITCH ] = -15.0f;
		ent->client->ps.viewangles[ YAW ] = ent->client->killer_yaw;
	} else {
		// add angles based on weapon kick
		ent->client->ps.kick_angles = ent->client->kick_angles;
		// add angles based on damage kick
		ratio =( ent->client->v_dmg_time - level.time ) / DAMAGE_TIME;
		if( ratio < 0.0f ) {
			ratio = 0.0f;
			ent->client->v_dmg_pitch = 0.0f;
			ent->client->v_dmg_roll = 0.0f;
		}
		ent->client->ps.kick_angles[ PITCH ] += ratio * ent->client->v_dmg_pitch;
		ent->client->ps.kick_angles[ ROLL ] += ratio * ent->client->v_dmg_roll;
		// add pitch based on fall kick
		ratio = ( ent->client->fall_time - level.time ) / FALL_TIME;
		if( ratio < 0.0f )
			ratio = 0.0f;
		ent->client->ps.kick_angles[ PITCH ] += ratio * ent->client->fall_value;
		// add angles based on velocity
		delta = ent->velocity * view_forward;
		ent->client->ps.kick_angles[ PITCH ] += delta * run_pitch.GetFloat( );
		delta = ent->velocity * view_right;
		ent->client->ps.kick_angles[ ROLL ] += delta * run_roll.GetFloat( );
		// add angles based on bob
		delta = view_bobfracsin * bob_pitch.GetFloat( ) * view_xyspeed;
		if( ent->client->ps.pmove.pm_flags & PMF_DUCKED )
			delta *= 6.0f; // crouching
		ent->client->ps.kick_angles[ PITCH ] += delta;
		delta = view_bobfracsin * bob_roll.GetFloat( ) * view_xyspeed;
		if( ent->client->ps.pmove.pm_flags & PMF_DUCKED )
			delta *= 6.0f; // crouching
		if( view_bobcycle & 1 )
			delta = -delta;
		ent->client->ps.kick_angles[ ROLL ] += delta;
	}
	// base origin
	v = vec3_origin;
	// add view height
	v[ 2 ] += ent->viewheight;
	// add fall height
	ratio =( ent->client->fall_time - level.time ) / FALL_TIME;
	if( ratio < 0.0f )
		ratio = 0.0f;
	v[ 2 ] -= ratio * ent->client->fall_value * 0.4f;
	// add bob height
	bob = view_bobfracsin * view_xyspeed * bob_roll.GetFloat( );
	if( bob > 6.0f )
		bob = 6.0f;
	//Screen::SCR_DebugGraph( bob * 2, 255 );
	v[ 2 ] += bob;
	// add kick offset
	v += ent->client->kick_origin;
	// absolutely bound offsets
	// so the view can never be outside the player box
	if( v[ 0 ] < -14.0f )
		v[ 0 ] = -14.0f;
	else if( v[ 0 ] > 14.0f )
		v[ 0 ] = 14.0f;
	if( v[ 1 ] < -14.0f )
		v[ 1 ] = -14.0f;
	else if( v[ 1 ] > 14.0f )
		v[ 1 ] = 14.0f;
	if( v[ 2 ] < -22.0f )
		v[ 2 ] = -22.0f;
	else if( v[ 2 ] > 30.0f )
		v[ 2 ] = 30.0f;
	ent->client->ps.viewoffset = v;
}

void Game::SV_CalcGunOffset( Entity * ent ) {
	int i;
	float delta;
	// gun angles from bobbing
	ent->client->ps.gunangles[ ROLL ] = view_xyspeed * view_bobfracsin * 0.005f;
	ent->client->ps.gunangles[ YAW ] = view_xyspeed * view_bobfracsin * 0.01f;
	if( view_bobcycle & 1 ) {
		ent->client->ps.gunangles[ ROLL ] = -ent->client->ps.gunangles[ ROLL ];
		ent->client->ps.gunangles[ YAW ] = -ent->client->ps.gunangles[ YAW ];
	}
	ent->client->ps.gunangles[ PITCH ] = view_xyspeed * view_bobfracsin * 0.005f;
	// gun angles from delta movement
	for( i = 0; i<3; i++ ) {
		delta = ent->client->oldviewangles[ i ] - ent->client->ps.viewangles[ i ];
		if( delta > 180 )
			delta -= 360;
		if( delta < -180 )
			delta += 360;
		if( delta > 45 )
			delta = 45;
		if( delta < -45 )
			delta = -45;
		if( i == YAW )
			ent->client->ps.gunangles[ ROLL ] += 0.1f* delta;
		ent->client->ps.gunangles[ i ] += 0.2f * delta;
	}
	// gun height
	ent->client->ps.gunoffset = vec3_origin;
	//	ent->ps->gunorigin[ 2 ] += bob;
	// gun_x / gun_y / gun_z are development tools
	for( i = 0; i<3; i++ ) {
		ent->client->ps.gunoffset[ i ] += view_forward[ i ] * gun_y.GetFloat( );
		ent->client->ps.gunoffset[ i ] += view_right[ i ] * gun_x.GetFloat( );
		ent->client->ps.gunoffset[ i ] += view_up[ i ] * -gun_z.GetFloat( );
	}
}

void Game::SV_AddBlend( float r, float g, float b, float a, float * v_blend ) {
	float a2, a3;
	if( a <= 0 )
		return;
	a2 = v_blend[ 3 ] +( 1-v_blend[ 3 ] )* a; // new total alpha
	a3 = v_blend[ 3 ]/a2; // fraction of color from old
	v_blend[ 0 ] = v_blend[ 0 ]* a3 + r* ( 1-a3 );
	v_blend[ 1 ] = v_blend[ 1 ]* a3 + g* ( 1-a3 );
	v_blend[ 2 ] = v_blend[ 2 ]* a3 + b* ( 1-a3 );
	v_blend[ 3 ] = a2;
}

void Game::SV_CalcBlend( Entity * ent ) {
	int contents;
	Vec3 vieworg;
	int remaining;
	ent->client->ps.blend[ 0 ] = ent->client->ps.blend[ 1 ] =
		ent->client->ps.blend[ 2 ] = ent->client->ps.blend[ 3 ] = 0;
	// add for contents
	vieworg = ent->s.origin + ent->client->ps.viewoffset;
	contents = Server::SV_PointContents( vieworg );
	if( contents &( CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WATER ) )
		ent->client->ps.rdflags |= RDF_UNDERWATER;
	else
		ent->client->ps.rdflags &= ~RDF_UNDERWATER;
	if( contents &( CONTENTS_SOLID|CONTENTS_LAVA ) ) SV_AddBlend( 1.0f, 0.3f, 0.0f, 0.6f, ent->client->ps.blend );
	else if( contents & CONTENTS_SLIME )
		SV_AddBlend( 0.0f, 0.1f, 0.05f, 0.6f, ent->client->ps.blend );
	else if( contents & CONTENTS_WATER )
		SV_AddBlend( 0.5f, 0.3f, 0.2f, 0.4f, ent->client->ps.blend );
	// add for powerview_ups
	if( ent->client->quad_framenum > level.framenum ) {
		remaining = ( int )ent->client->quad_framenum - level.framenum;
		if( remaining == 30 ) // beginning to fade
			Server::PF_StartSound( ent, CHAN_ITEM, Server::SV_SoundIndex( "items/damage2.wav" ), 1, ATTN_NORM, 0 );
		if( remaining > 30 ||( remaining & 4 ) )
			SV_AddBlend( 0, 0, 1, 0.08f, ent->client->ps.blend );
	} else if( ent->client->invincible_framenum > level.framenum ) {
		remaining = ( int )ent->client->invincible_framenum - level.framenum;
		if( remaining == 30 ) // beginning to fade
			Server::PF_StartSound( ent, CHAN_ITEM, Server::SV_SoundIndex( "items/protect2.wav" ), 1, ATTN_NORM, 0 );
		if( remaining > 30 ||( remaining & 4 ) )
			SV_AddBlend( 1, 1, 0, 0.08f, ent->client->ps.blend );
	} else if( ent->client->enviro_framenum > level.framenum ) {
		remaining = ( int )ent->client->enviro_framenum - level.framenum;
		if( remaining == 30 ) // beginning to fade
			Server::PF_StartSound( ent, CHAN_ITEM, Server::SV_SoundIndex( "items/airout.wav" ), 1, ATTN_NORM, 0 );
		if( remaining > 30 ||( remaining & 4 ) )
			SV_AddBlend( 0, 1, 0, 0.08f, ent->client->ps.blend );
	} else if( ent->client->breather_framenum > level.framenum ) {
		remaining = ( int )ent->client->breather_framenum - level.framenum;
		if( remaining == 30 ) // beginning to fade
			Server::PF_StartSound( ent, CHAN_ITEM, Server::SV_SoundIndex( "items/airout.wav" ), 1, ATTN_NORM, 0 );
		if( remaining > 30 ||( remaining & 4 ) )
			SV_AddBlend( 0.4f, 1, 0.4f, 0.04f, ent->client->ps.blend );
	}
	// add for damage
	if( ent->client->damage_alpha > 0 )
		SV_AddBlend( ent->client->damage_blend[ 0 ], ent->client->damage_blend[ 1 ], ent->client->damage_blend[ 2 ], ent->client->damage_alpha, ent->client->ps.blend );
	if( ent->client->bonus_alpha > 0 )
		SV_AddBlend( 0.85f, 0.7f, 0.3f, ent->client->bonus_alpha, ent->client->ps.blend );
	// drop the damage value
	ent->client->damage_alpha -= 0.06f;
	if( ent->client->damage_alpha < 0 )
		ent->client->damage_alpha = 0;
	// drop the bonus value
	ent->client->bonus_alpha -= 0.1f;
	if( ent->client->bonus_alpha < 0 )
		ent->client->bonus_alpha = 0;
}

void Game::P_FallingDamage( Entity * ent ) {
	float delta;
	int damage;
	Vec3 dir;
	if( ent->s.modelindex != 255 )
		return; // not in the player model
	if( ent->movetype == MOVETYPE_NOCLIP )
		return;
	if( ( ent->client->oldvelocity[ 2 ] < 0 ) &&( ent->velocity[ 2 ] > ent->client->oldvelocity[ 2 ] ) &&( !ent->groundentity ) )
		delta = ent->client->oldvelocity[ 2 ];
	else {
		if( !ent->groundentity )
			return;
		delta = ent->velocity[ 2 ] - ent->client->oldvelocity[ 2 ];
	}
	delta = delta* delta * 0.0001f;
	// never take falling damage if completely underwater
	if( ent->waterlevel == 3 )
		return;
	if( ent->waterlevel == 2 )
		delta *= 0.25f;
	if( ent->waterlevel == 1 )
		delta *= 0.5f;
	if( delta < 1 )
		return;
	if( delta < 15 ) {
		ent->s.eventNum = EV_FOOTSTEP;
		return;
	}
	ent->client->fall_value = delta* 0.5f;
	if( ent->client->fall_value > 40 )
		ent->client->fall_value = 40;
	ent->client->fall_time = level.time + FALL_TIME;
	if( delta > 30 ) {
		if( ent->health > 0 ) {
			if( delta >= 55 )
				ent->s.eventNum = EV_FALLFAR;
			else
				ent->s.eventNum = EV_FALL;
		}
		ent->pain_debounce_time = level.time; // no normal pain sound
		damage = ( int )( ( delta-30 )/2 );
		if( damage < 1 )
			damage = 1;
		dir = Vec3( 0, 0, 1 );
		if( !deathmatch.GetBool( ) || !( dmflags.GetInt( ) & DF_NO_FALLING ) )
			T_Damage( ent, world, world, dir, ent->s.origin, vec3_origin, damage, 0, 0, MOD_FALLING );
	} else {
		ent->s.eventNum = EV_FALLSHORT;
		return;
	}
}

void Game::P_WorldEffects( ) {
	bool breather;
	bool envirosuit;
	int waterlevel, old_waterlevel;
	if( view_current_player->movetype == MOVETYPE_NOCLIP ) {
		view_current_player->air_finished = level.time + 12; // don't need air
		return;
	}
	waterlevel = view_current_player->waterlevel;
	old_waterlevel = view_current_client->old_waterlevel;
	view_current_client->old_waterlevel = waterlevel;
	breather = view_current_client->breather_framenum > level.framenum;
	envirosuit = view_current_client->enviro_framenum > level.framenum;
	// if just entered a water volume, play a sound
	if( !old_waterlevel && waterlevel ) {
		PlayerNoise( view_current_player, view_current_player->s.origin, PNOISE_SELF );
		if( view_current_player->watertype & CONTENTS_LAVA )
			Server::PF_StartSound( view_current_player, CHAN_BODY, Server::SV_SoundIndex( "player/lava_in.wav" ), 1, ATTN_NORM, 0 );
		else if( view_current_player->watertype & CONTENTS_SLIME )
			Server::PF_StartSound( view_current_player, CHAN_BODY, Server::SV_SoundIndex( "player/watr_in.wav" ), 1, ATTN_NORM, 0 );
		else if( view_current_player->watertype & CONTENTS_WATER )
			Server::PF_StartSound( view_current_player, CHAN_BODY, Server::SV_SoundIndex( "player/watr_in.wav" ), 1, ATTN_NORM, 0 );
		view_current_player->flags |= FL_INWATER;
		// clear damage_debounce, so the pain sound will play immediately
		view_current_player->damage_debounce_time = level.time - 1.0f;
	}
	// if just completely exited a water volume, play a sound
	if( old_waterlevel && ! waterlevel ) {
		PlayerNoise( view_current_player, view_current_player->s.origin, PNOISE_SELF );
		Server::PF_StartSound( view_current_player, CHAN_BODY, Server::SV_SoundIndex( "player/watr_out.wav" ), 1, ATTN_NORM, 0 );
		view_current_player->flags &= ~FL_INWATER;
	}
	// check for head just going under water
	if( old_waterlevel != 3 && waterlevel == 3 )
		Server::PF_StartSound( view_current_player, CHAN_BODY, Server::SV_SoundIndex( "player/watr_un.wav" ), 1, ATTN_NORM, 0 );
	// check for head just coming out of water
	if( old_waterlevel == 3 && waterlevel != 3 ) {
		if( view_current_player->air_finished < level.time )
		{ // gasp for air
			Server::PF_StartSound( view_current_player, CHAN_VOICE, Server::SV_SoundIndex( "player/gasp1.wav" ), 1, ATTN_NORM, 0 );
			PlayerNoise( view_current_player, view_current_player->s.origin, PNOISE_SELF );
		} else if( view_current_player->air_finished < level.time + 11 ) { // just break surface
			Server::PF_StartSound( view_current_player, CHAN_VOICE, Server::SV_SoundIndex( "player/gasp2.wav" ), 1, ATTN_NORM, 0 );
		}
	}
	// check for drowning
	if( waterlevel == 3 ) {
		// breather or envirosuit give air
		if( breather || envirosuit ) {
			view_current_player->air_finished = level.time + 10;
			if( ( ( int )( view_current_client->breather_framenum - level.framenum ) % 25 ) == 0 ) {
				if( !view_current_client->breather_sound )
					Server::PF_StartSound( view_current_player, CHAN_AUTO, Server::SV_SoundIndex( "player/u_breath1.wav" ), 1, ATTN_NORM, 0 );
				else
					Server::PF_StartSound( view_current_player, CHAN_AUTO, Server::SV_SoundIndex( "player/u_breath2.wav" ), 1, ATTN_NORM, 0 );
				view_current_client->breather_sound ^= 1;
				PlayerNoise( view_current_player, view_current_player->s.origin, PNOISE_SELF );
				//FIXME: release a bubble?
			}
		}
		// if out of air, start drowning
		if( view_current_player->air_finished < level.time ) { // drown!
			if( view_current_player->client->next_drown_time < level.time && view_current_player->health > 0 ) {
				view_current_player->client->next_drown_time = level.time + 1.0f;
				// take more damage the longer underwater
				view_current_player->dmg += 2;
				if( view_current_player->dmg > 15 )
					view_current_player->dmg = 15;
				// play a gurp sound instead of a normal pain sound
				if( view_current_player->health <= view_current_player->dmg )
					Server::PF_StartSound( view_current_player, CHAN_VOICE, Server::SV_SoundIndex( "player/drown1.wav" ), 1, ATTN_NORM, 0 );
				else if( rand( )&1 )
					Server::PF_StartSound( view_current_player, CHAN_VOICE, Server::SV_SoundIndex( "*gurp1.wav" ), 1, ATTN_NORM, 0 );
				else
					Server::PF_StartSound( view_current_player, CHAN_VOICE, Server::SV_SoundIndex( "*gurp2.wav" ), 1, ATTN_NORM, 0 );
				view_current_player->pain_debounce_time = level.time;
				T_Damage( view_current_player, world, world, vec3_origin, view_current_player->s.origin, vec3_origin, view_current_player->dmg, 0, DAMAGE_NO_ARMOR, MOD_WATER );
			}
		}
	} else {
		view_current_player->air_finished = level.time + 12;
		view_current_player->dmg = 2;
	}
	// check for sizzle damage
	if( waterlevel &&( view_current_player->watertype&( CONTENTS_LAVA|CONTENTS_SLIME ) ) ) {
		if( view_current_player->watertype & CONTENTS_LAVA ) {
			if( view_current_player->health > 0 && view_current_player->pain_debounce_time <= level.time && view_current_client->invincible_framenum < level.framenum ) {
				if( rand( )&1 )
					Server::PF_StartSound( view_current_player, CHAN_VOICE, Server::SV_SoundIndex( "player/burn1.wav" ), 1, ATTN_NORM, 0 );
				else
					Server::PF_StartSound( view_current_player, CHAN_VOICE, Server::SV_SoundIndex( "player/burn2.wav" ), 1, ATTN_NORM, 0 );
				view_current_player->pain_debounce_time = level.time + 1.0f;
			}
			if( envirosuit ) // take 1/3 damage with envirosuit
				T_Damage( view_current_player, world, world, vec3_origin, view_current_player->s.origin, vec3_origin, 1* waterlevel, 0, 0, MOD_LAVA );
			else
				T_Damage( view_current_player, world, world, vec3_origin, view_current_player->s.origin, vec3_origin, 3* waterlevel, 0, 0, MOD_LAVA );
		}
		if( view_current_player->watertype & CONTENTS_SLIME ) {
			if( !envirosuit ) { // no damage from slime with envirosuit
				T_Damage( view_current_player, world, world, vec3_origin, view_current_player->s.origin, vec3_origin, 1* waterlevel, 0, 0, MOD_SLIME );
			}
		}
	}
}

void Game::G_SetClientEffects( Entity * ent ) {
	int pa_type;
	int remaining;
	ent->s.effects = 0;
	ent->s.renderfx = 0;
	if( ent->health <= 0 || level.intermissiontime )
		return;
	if( ent->powerarmor_time > level.time ) {
		pa_type = PowerArmorType( ent );
		if( pa_type == POWER_ARMOR_SCREEN ) {
			ent->s.effects |= EF_POWERSCREEN;
		} else if( pa_type == POWER_ARMOR_SHIELD ) {
			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_GREEN;
		}
	}
	if( ent->client->quad_framenum > level.framenum ) {
		remaining = ( int )ent->client->quad_framenum - level.framenum;
		if( remaining > 30 ||( remaining & 4 ) )
			ent->s.effects |= EF_QUAD;
	}
	if( ent->client->invincible_framenum > level.framenum ) {
		remaining = ( int )ent->client->invincible_framenum - level.framenum;
		if( remaining > 30 ||( remaining & 4 ) )
			ent->s.effects |= EF_PENT;
	}
	// show cheaters!!!
	if( ent->flags & FL_GODMODE ) {
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |=( RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE );
	}
}

void Game::G_SetClientEvent( Entity * ent ) {
	if( ent->s.eventNum )
		return;
	if( ent->groundentity && view_xyspeed > 225 ) {
		if( ( int )( view_current_client->bobtime+view_bobmove ) != view_bobcycle )
			ent->s.eventNum = EV_FOOTSTEP;
	}
}

void Game::G_SetClientSound( Entity * ent ) {
	Str weap;
	if( ent->client->pers.game_helpchanged != helpchanged ) {
		ent->client->pers.game_helpchanged = helpchanged;
		ent->client->pers.helpchanged = 1;
	}
	// help beep( no more than three times )
	if( ent->client->pers.helpchanged && ent->client->pers.helpchanged <= 3 && !( level.framenum&63 ) ) {
		ent->client->pers.helpchanged++;
		Server::PF_StartSound( ent, CHAN_VOICE, Server::SV_SoundIndex( "misc/pc_view_up.wav" ), 1, ATTN_STATIC, 0 );
	}
	if( ent->client->pers.weapon )
		weap = ent->client->pers.weapon->classname;
	else
		weap = "";
	if( ent->waterlevel &&( ent->watertype&( CONTENTS_LAVA|CONTENTS_SLIME ) ) )
		ent->s.sound = snd_fry;
	else if( strcmp( weap, "weapon_railgun" ) == 0 )
		ent->s.sound = Server::SV_SoundIndex( "weapons/rg_hum.wav" );
	else if( strcmp( weap, "weapon_bfg" ) == 0 )
		ent->s.sound = Server::SV_SoundIndex( "weapons/bfg_hum.wav" );
	else if( ent->client->weapon_sound )
		ent->s.sound = ent->client->weapon_sound;
	else
		ent->s.sound = 0;
}

void Game::G_SetClientFrame( Entity * ent ) {
	gclient_t * client;
	bool duck, run;
	if( ent->s.modelindex != 255 )
		return; // not in the player model
	client = ent->client;
	if( client->ps.pmove.pm_flags & PMF_DUCKED )
		duck = true;
	else
		duck = false;
	if( view_xyspeed )
		run = true;
	else
		run = false;
	// check for stand/duck and stop/go transitions
	if( duck != client->anim_duck && client->anim_priority < ANIM_DEATH )
		goto newanim;
	if( run != client->anim_run && client->anim_priority == ANIM_BASIC )
		goto newanim;
	if( !ent->groundentity && client->anim_priority <= ANIM_WAVE )
		goto newanim;
	if( client->anim_priority == ANIM_REVERSE ) {
		if( ent->s.frame > client->anim_end ) {
			ent->s.frame--;
			return;
		}
	} else if( ent->s.frame < client->anim_end ) { // continue an animation
		ent->s.frame++;
		return;
	}
	if( client->anim_priority == ANIM_DEATH )
		return; // stay there
	if( client->anim_priority == ANIM_JUMP ) {
		if( !ent->groundentity )
			return; // stay there
		ent->client->anim_priority = ANIM_WAVE;
		ent->s.frame = FRAME_jump3;
		ent->client->anim_end = FRAME_jump6;
		return;
	}
newanim:
	// return to either a running or standing frame
	client->anim_priority = ANIM_BASIC;
	client->anim_duck = duck;
	client->anim_run = run;

	if( !ent->groundentity ) {
		client->anim_priority = ANIM_JUMP;
		if( ent->s.frame != FRAME_jump2 )
			ent->s.frame = FRAME_jump1;
		client->anim_end = FRAME_jump2;
	} else if( run ) { // running
		if( duck ) {
			ent->s.frame = FRAME_crwalk1;
			client->anim_end = FRAME_crwalk6;
		} else {
			ent->s.frame = FRAME_run1;
			client->anim_end = FRAME_run6;
		}
	} else { // standing
		if( duck ) {
			ent->s.frame = FRAME_crstnd01;
			client->anim_end = FRAME_crstnd19;
		} else {
			ent->s.frame = FRAME_stand01;
			client->anim_end = FRAME_stand40;
		}
	}
}

void Game::ClientEndServerFrame( Entity * ent ) {
	float bobtime;
	int i;
	view_current_player = ent;
	view_current_client = ent->client;
	// If the origin or velocity have changed since ClientThink( ), // view_update the pmove values.  This will happen when the client
	// is pushed by a bmodel or kicked by an explosion.
	// 
	// If it wasn't view_updated here, the view position would lag a frame
	// behind the body position when pushed -- "sinking into plats"
	for( i = 0; i<3; i++ ) {
		view_current_client->ps.pmove.origin[ i ] = ( short )( ent->s.origin[ i ] * 8.0f );
		view_current_client->ps.pmove.velocity[ i ] = ( short )( ent->velocity[ i ] * 8.0f );
	}
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	if( level.intermissiontime ) {
		// FIXME: add view drifting here?
		view_current_client->ps.blend[ 3 ] = 0;
		view_current_client->ps.fov = 90;
		G_SetStats( ent );
		return;
	}
	ent->client->v_angle.AngleVectors( &view_forward, &view_right, &view_up );
	// burn from lava, etc
	P_WorldEffects( );
	// set model angles from view angles so other things in
	// the world can tell which direction you are looking
	if( ent->client->v_angle[ PITCH ] > 180 )
		ent->s.angles[ PITCH ] =( -360 + ent->client->v_angle[ PITCH ] )/3;
	else
		ent->s.angles[ PITCH ] = ent->client->v_angle[ PITCH ]/3;
	ent->s.angles[ YAW ] = ent->client->v_angle[ YAW ];
	ent->s.angles[ ROLL ] = SV_CalcRoll( ent->s.angles, ent->velocity )* 4;
	// calculate speed and cycle to be used for
	// all cyclic walking effects
	view_xyspeed = sqrt( ent->velocity[ 0 ]* ent->velocity[ 0 ] + ent->velocity[ 1 ]* ent->velocity[ 1 ] );
	if( view_xyspeed < 5 ) {
		view_bobmove = 0;
		view_current_client->bobtime = 0; // start at beginning of cycle again
	} else if( ent->groundentity ) { // so bobbing only cycles when on ground
		if( view_xyspeed > 210 )
			view_bobmove = 0.25f;
		else if( view_xyspeed > 100 )
			view_bobmove = 0.125f;
		else
			view_bobmove = 0.0625f;
	}
	bobtime =( view_current_client->bobtime += view_bobmove );
	if( view_current_client->ps.pmove.pm_flags & PMF_DUCKED )
		bobtime *= 4;
	view_bobcycle =( int )bobtime;
	view_bobfracsin = fabs( sin( bobtime* M_PI ) );
	// detect hitting the floor
	P_FallingDamage( ent );
	// apply all the damage taken this frame
	P_DamageFeedback( ent );
	// determine the view offsets
	SV_CalcViewOffset( ent );
	// determine the gun offsets
	SV_CalcGunOffset( ent );
	// determine the full screen color blend
	// must be after viewoffset, so eye contents can be
	// accurately determined
	// FIXME: with client prediction, the contents
	// should be determined by the client
	SV_CalcBlend( ent );
	// chase cam stuff
	if( ent->client->resp.spectator )
		G_SetSpectatorStats( ent );
	else
		G_SetStats( ent );
	G_CheckChaseStats( ent );
	G_SetClientEvent( ent );
	G_SetClientEffects( ent );
	G_SetClientSound( ent );
	G_SetClientFrame( ent );
	ent->client->oldvelocity = ent->velocity;
	ent->client->oldviewangles = ent->client->ps.viewangles;
	// clear weapon kicks
	ent->client->kick_origin = vec3_origin;
	ent->client->kick_angles = vec3_origin;
	// if the scoreboard is view_up, view_update it
	if( ent->client->showscores && !( level.framenum & 31 ) ) {
		DeathmatchScoreboardMessage( ent, ent->enemy );
		Server::PF_Unicast( ent, false );
	}
}
