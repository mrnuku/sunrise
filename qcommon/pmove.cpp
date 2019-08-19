#include "precompiled.h"
#pragma hdrstop

void pml_t::Clear( ) {
	origin.Zero( );
	velocity.Zero( );
	forward.Zero( );
	right.Zero( );
	up.Zero( );
	frametime		= 0;
	groundsurface	= NULL;
	groundplane.Zero( );
	groundcontents	= 0;
	previous_origin.Zero( );
	ladder			= false;
}

// movement parameters
float pm_stopspeed = 100;
float pm_maxspeed = 300;
float pm_duckspeed = 100;
float pm_accelerate = 10;
float pm_airaccelerate = 0;
float pm_wateraccelerate = 10;
float pm_friction = 6;
float pm_waterfriction = 1;
float pm_waterspeed = 400;

void usercmd_t::Clear( ) {
	msec		= 0;
	buttons		= 0;
	forwardmove	= 0;
	sidemove	= 0;
	upmove		= 0;
	impulse		= 0;
	lightlevel	= 0;	
	for( int i = 0; i < 3; i++ )
		angles[ i ]	= 0;
}

void pmove_t::Clear( ) {
	s.Clear( );
	cmd.Clear( );
	snapinitial		= false;
	numtouch		= 0;
	for( int i = 0; i < MAXTOUCH; i++ )
		touchents[ i ] = NULL;
	viewangles.Zero( );
	viewheight		= 0;
	mins.Zero( );
	maxs.Zero( );
	groundentity	= NULL;
	watertype		= 0;
	waterlevel		= 0;
	trace			= NULL;
	pointcontents	= NULL;
}

void pmove_state_t::Clear( ) {
	pm_type		= PM_NORMAL;
	pm_flags	= 0;
	pm_time		= 0;
	gravity		= 0;
	for( int i = 0; i < 3; i++ ) {
		origin[ i ]			= 0;
		velocity[ i ]		= 0;
		delta_angles[ i ]	= 0;
	}
}

void player_state_t::Clear( ) {
	pmove.Clear( );
	viewangles.Zero( );
	viewoffset.Zero( );
	kick_angles.Zero( );
	gunangles.Zero( );
	gunoffset.Zero( );
	gunindex	= 0;
	gunframe	= 0;
	fov			= 0;
	rdflags		= 0;
	for( int i = 0; i < 4; i++ )
		blend[ i ]	= 0;
	for( int i = 0; i < MAX_STATS; i++ )
		stats[ i ] = 0;
}

void entity_state_t::Clear( ) {
	number		= 0;
	origin.Zero( );
	angles.Zero( );
	old_origin.Zero( );
	modelindex	= 0;
	modelindex2	= 0;
	modelindex3	= 0;
	modelindex4	= 0;
	frame		= 0;
	skinnum		= 0;
	effects		= 0;
	renderfx	= 0;
	solid		= 0;
	sound		= 0;
	eventNum	= EV_NONE;
}

void PlayerMove::PM_ClipVelocity( const Vec3 & in, const Vec3 & normal, Vec3 & out, float overbounce ) {
	float backoff = in * normal * overbounce;
	for( int i = 0; i < 3; i++ ) {
		float change = normal[ i ] * backoff;
		out[ i ] = in[ i ] - change;
		if( out[ i ] > -0.1f && out[ i ] < 0.1f )
			out[ i ] = 0.0f;
	}
}

void PlayerMove::PM_StepSlideMove_( PmoveParams & pm_params ) {
	int bumpcount, numbumps;
	Vec3 dir;
	float d;
	int numplanes;
	Vec3 planes[ 5 ];
	Vec3 primal_velocity;
	int i, j;
	CTrace trace;
	Vec3 end;
	float time_left;
	numbumps = 4;
	primal_velocity = pm_params.d_pml.velocity;
	numplanes = 0;
	time_left = pm_params.d_pml.frametime;
	for( bumpcount = 0; bumpcount<numbumps; bumpcount++ ) {
		for( i = 0; i<3; i++ )
			end[ i ] = pm_params.d_pml.origin[ i ] + time_left * pm_params.d_pml.velocity[ i ];
		trace = pm_params.d_pm.trace( pm_params.d_pml.origin, pm_params.d_pm.mins, pm_params.d_pm.maxs, end );
		if( trace.allsolid ) { // entity is trapped in another solid
			pm_params.d_pml.velocity[ 2 ] = 0; // don't build up falling damage
			return;
		}
		if( trace.fraction > 0 ) { // actually covered some distance
			pm_params.d_pml.origin = trace.endpos;
			numplanes = 0;
		}
		if( trace.fraction == 1 )
			break; // moved the entire distance
		// save entity for contact
		if( pm_params.d_pm.numtouch < MAXTOUCH && trace.ent ) {
			pm_params.d_pm.touchents[ pm_params.d_pm.numtouch ] = trace.ent;
			pm_params.d_pm.numtouch++;
		}
		time_left -= time_left * trace.fraction;
		// slide along this plane
		if( numplanes >= 5 ) { // this shouldn't really happen
			pm_params.d_pml.velocity = vec3_origin;
			break;
		}
		planes[ numplanes ] = trace.plane.Normal( );
		numplanes++;
		// modify original_velocity so it parallels all of the clip planes
		for( i = 0; i<numplanes; i++ ) {
			PM_ClipVelocity( pm_params.d_pml.velocity, planes[ i ], pm_params.d_pml.velocity, 1.01f );
			for( j = 0; j < numplanes; j++ ) {
				if( j != i && pm_params.d_pml.velocity * planes[ j ] < 0.0f )
					break; // not ok
			}
			if( j == numplanes )
				break;
		}
		if( i == numplanes ) { // go along the crease
			if( numplanes != 2 ) {
				//				Con_Printf( "clip velocity, numplanes == %i\n", numplanes );
				pm_params.d_pml.velocity = vec3_origin;
				break;
			}
			dir = planes[ 0 ].Cross( planes[ 1 ] );
			d = dir * pm_params.d_pml.velocity;
			pm_params.d_pml.velocity = dir * d;
		}
		// if velocity is against the original velocity, stop dead
		// to avoid tiny occilations in sloping corners
		if( pm_params.d_pml.velocity * primal_velocity <= 0.0f ) {
			pm_params.d_pml.velocity = vec3_origin;
			break;
		}
	}
	if( pm_params.d_pm.s.pm_time )
		pm_params.d_pml.velocity = primal_velocity;
}

void PlayerMove::PM_StepSlideMove( PmoveParams & pm_params ) {
	Vec3 start_o, start_v;
	Vec3 down_o, down_v;
	CTrace trace;
	float down_dist, up_dist;
	//	Vec3		delta;
	Vec3 up, down;
	start_o = pm_params.d_pml.origin;
	start_v = pm_params.d_pml.velocity;
	PM_StepSlideMove_( pm_params );
	down_o = pm_params.d_pml.origin;
	down_v = pm_params.d_pml.velocity;
	up = start_o;
	up[ 2 ] += 18;
	trace = pm_params.d_pm.trace( up, pm_params.d_pm.mins, pm_params.d_pm.maxs, up );
	if( trace.allsolid )
		return; // can't step up
	// try sliding above
	pm_params.d_pml.origin = up;
	pm_params.d_pml.velocity = start_v;
	PM_StepSlideMove_( pm_params );
	// push down the final amount
	down = pm_params.d_pml.origin;
	down[ 2 ] -= 18;
	trace = pm_params.d_pm.trace( pm_params.d_pml.origin, pm_params.d_pm.mins, pm_params.d_pm.maxs, down );
	if( !trace.allsolid ) {
		pm_params.d_pml.origin = trace.endpos;
	}
	up = pm_params.d_pml.origin;
	// decide which one went farther
	down_dist =( down_o[ 0 ] - start_o[ 0 ] ) * ( down_o[ 0 ] - start_o[ 0 ] ) +( down_o[ 1 ] - start_o[ 1 ] ) * ( down_o[ 1 ] - start_o[ 1 ] );
	up_dist =( up[ 0 ] - start_o[ 0 ] ) * ( up[ 0 ] - start_o[ 0 ] ) +( up[ 1 ] - start_o[ 1 ] ) * ( up[ 1 ] - start_o[ 1 ] );

	if( down_dist > up_dist || trace.plane.Normal( )[ 2 ] < 0.7f ) {
		pm_params.d_pml.origin = down_o;
		pm_params.d_pml.velocity = down_v;
		return;
	}
	//!! Special case
	// if we were walking along a plane, then we need to copy the Z over
	pm_params.d_pml.velocity[ 2 ] = down_v[ 2 ];
}

void PlayerMove::PM_Friction( PmoveParams & pm_params ) {
	float newspeed, control;
	float friction;
	float speed = pm_params.d_pml.velocity.Length( );
	if( speed < 1.0f ) {
		pm_params.d_pml.velocity[ 0 ] = 0.0f;
		pm_params.d_pml.velocity[ 1 ] = 0.0f;
		return;
	}
	float drop = 0;
	// apply ground friction
	if( ( pm_params.d_pm.groundentity && pm_params.d_pml.groundsurface ) || ( pm_params.d_pml.ladder ) ) {
		friction = pm_friction;
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control* friction* pm_params.d_pml.frametime;
	}
	// apply water friction
	if( pm_params.d_pm.waterlevel && !pm_params.d_pml.ladder )
		drop += speed * pm_waterfriction * pm_params.d_pm.waterlevel * pm_params.d_pml.frametime;
	// scale the velocity
	newspeed = speed - drop;
	if( newspeed < 0.0f ) newspeed = 0.0f;
	newspeed /= speed;
	pm_params.d_pml.velocity *= newspeed;
}

void PlayerMove::PM_Accelerate( PmoveParams & pm_params, const Vec3 & wishdir, float wishspeed, float accel ) {
	int i;
	float addspeed, accelspeed, currentspeed;
	currentspeed = pm_params.d_pml.velocity * wishdir;
	addspeed = wishspeed - currentspeed;
	if( addspeed <= 0 )
		return;
	accelspeed = accel* pm_params.d_pml.frametime * wishspeed;
	if( accelspeed > addspeed )
		accelspeed = addspeed;
	for( i = 0; i < 3; i++ )
		pm_params.d_pml.velocity[ i ] += accelspeed* wishdir[ i ];
}

void PlayerMove::PM_AirAccelerate( PmoveParams & pm_params, const Vec3 & wishdir, float wishspeed, float accel ) {
	int i;
	float addspeed, accelspeed, currentspeed, wishspd = wishspeed;
	if( wishspd > 30 )
		wishspd = 30;
	currentspeed = pm_params.d_pml.velocity * wishdir;
	addspeed = wishspd - currentspeed;
	if( addspeed <= 0 )
		return;
	accelspeed = accel * wishspeed * pm_params.d_pml.frametime;
	if( accelspeed > addspeed )
		accelspeed = addspeed;
	for( i = 0; i < 3; i++ )
		pm_params.d_pml.velocity[ i ] += accelspeed* wishdir[ i ];
}

void PlayerMove::PM_AddCurrents( PmoveParams & pm_params, Vec3 & wishvel ) {
	Vec3 v;
	// account for ladders
	if( pm_params.d_pml.ladder && fabs( pm_params.d_pml.velocity[ 2 ] ) <= 200 ) {
		if( ( pm_params.d_pm.viewangles[ PITCH ] <= -15 ) && ( pm_params.d_pm.cmd.forwardmove > 0 ) )
			wishvel[ 2 ] = 200;
		else if( ( pm_params.d_pm.viewangles[ PITCH ] >= 15 ) &&( pm_params.d_pm.cmd.forwardmove > 0 ) )
			wishvel[ 2 ] = -200;
		else if( pm_params.d_pm.cmd.upmove > 0 )
			wishvel[ 2 ] = 200;
		else if( pm_params.d_pm.cmd.upmove < 0 )
			wishvel[ 2 ] = -200;
		else
			wishvel[ 2 ] = 0;

		// limit horizontal speed when on a ladder
		if( wishvel[ 0 ] < -25 )
			wishvel[ 0 ] = -25;
		else if( wishvel[ 0 ] > 25 )
			wishvel[ 0 ] = 25;

		if( wishvel[ 1 ] < -25 )
			wishvel[ 1 ] = -25;
		else if( wishvel[ 1 ] > 25 )
			wishvel[ 1 ] = 25;
	}
#if 0
	// add water currents
	if( pm_params.d_pm.watertype & MASK_CURRENT ) {
		Vec3 v = vec3_origin;
		if( pm_params.d_pm.watertype & CONTENTS_CURRENT_0 )
			v[ 0 ] += 1;
		if( pm_params.d_pm.watertype & CONTENTS_CURRENT_90 )
			v[ 1 ] += 1;
		if( pm_params.d_pm.watertype & CONTENTS_CURRENT_180 )
			v[ 0 ] -= 1;
		if( pm_params.d_pm.watertype & CONTENTS_CURRENT_270 )
			v[ 1 ] -= 1;
		if( pm_params.d_pm.watertype & CONTENTS_CURRENT_UP )
			v[ 2 ] += 1;
		if( pm_params.d_pm.watertype & CONTENTS_CURRENT_DOWN )
			v[ 2 ] -= 1;
		s = pm_waterspeed;
		if( ( pm_params.d_pm.waterlevel == 1 ) &&( pm_params.d_pm.groundentity ) )
			s /= 2;
		wishvel = wishvel.Magnitude( s, v );
	}
	// add conveyor belt velocities
	if( pm_params.d_pm.groundentity ) {
		Vec3 v = vec3_origin;
		if( pm_params.d_pml.groundcontents & CONTENTS_CURRENT_0 )
			v[ 0 ] += 1;
		if( pm_params.d_pml.groundcontents & CONTENTS_CURRENT_90 )
			v[ 1 ] += 1;
		if( pm_params.d_pml.groundcontents & CONTENTS_CURRENT_180 )
			v[ 0 ] -= 1;
		if( pm_params.d_pml.groundcontents & CONTENTS_CURRENT_270 )
			v[ 1 ] -= 1;
		if( pm_params.d_pml.groundcontents & CONTENTS_CURRENT_UP )
			v[ 2 ] += 1;
		if( pm_params.d_pml.groundcontents & CONTENTS_CURRENT_DOWN )
			v[ 2 ] -= 1;
		wishvel = wishvel.Magnitude( 100.0f /* pm_params.d_pm.groundentity->speed */, v );
	}
#endif
}

void PlayerMove::PM_WaterMove( PmoveParams & pm_params ) {
	int i;
	Vec3 wishvel;
	float wishspeed;
	Vec3 wishdir;
	// user intentions
	for( i = 0; i<3; i++ )
		wishvel[ i ] = pm_params.d_pml.forward[ i ]* pm_params.d_pm.cmd.forwardmove + pm_params.d_pml.right[ i ]* pm_params.d_pm.cmd.sidemove;
	if( !pm_params.d_pm.cmd.forwardmove && !pm_params.d_pm.cmd.sidemove && !pm_params.d_pm.cmd.upmove )
		wishvel[ 2 ] -= 60; // drift towards bottom
	else
		wishvel[ 2 ] += pm_params.d_pm.cmd.upmove;
	PM_AddCurrents( pm_params, wishvel );
	wishdir = wishvel;
	wishspeed = wishdir.Normalize( );
	if( wishspeed > pm_maxspeed ) {
		wishvel *= pm_maxspeed / wishspeed;
		wishspeed = pm_maxspeed;
	}
	wishspeed *= 0.5f;
	PM_Accelerate( pm_params, wishdir, wishspeed, pm_wateraccelerate );
	PM_StepSlideMove( pm_params );
}

void PlayerMove::PM_AirMove( PmoveParams & pm_params ) {
	int i;
	Vec3 wishvel;
	float fmove, smove;
	Vec3 wishdir;
	float wishspeed;
	float maxspeed;
	fmove = pm_params.d_pm.cmd.forwardmove;
	smove = pm_params.d_pm.cmd.sidemove;
	//!!!!! pitch should be 1/3 so this isn't needed??!
	for( i = 0; i < 2; i++ )
		wishvel[ i ] = pm_params.d_pml.forward[ i ]* fmove + pm_params.d_pml.right[ i ]* smove;
	wishvel[ 2 ] = 0;
	PM_AddCurrents( pm_params, wishvel );
	wishdir = wishvel;
	wishspeed = wishdir.Normalize( );
	// clamp to server defined max speed
	maxspeed = ( pm_params.d_pm.s.pm_flags & PMF_DUCKED ) ? pm_duckspeed : pm_maxspeed;
	if( wishspeed > maxspeed ) {
		wishvel *= maxspeed / wishspeed;
		wishspeed = maxspeed;
	}
	if( pm_params.d_pml.ladder ) {
		PM_Accelerate( pm_params, wishdir, wishspeed, pm_accelerate );
		if( !wishvel[ 2 ] ) {
			if( pm_params.d_pml.velocity[ 2 ] > 0 ) {
				pm_params.d_pml.velocity[ 2 ] -= pm_params.d_pm.s.gravity * pm_params.d_pml.frametime;
				if( pm_params.d_pml.velocity[ 2 ] < 0 )
					pm_params.d_pml.velocity[ 2 ] = 0;
			} else {
				pm_params.d_pml.velocity[ 2 ] += pm_params.d_pm.s.gravity * pm_params.d_pml.frametime;
				if( pm_params.d_pml.velocity[ 2 ] > 0 )
					pm_params.d_pml.velocity[ 2 ] = 0;
			}
		}
		PM_StepSlideMove( pm_params );
	} else if( pm_params.d_pm.groundentity ) { // walking on ground
		pm_params.d_pml.velocity[ 2 ] = 0; //!!! this is before the accel
		PM_Accelerate( pm_params, wishdir, wishspeed, pm_accelerate );
		// PGM	-- fix for negative trigger_gravity fields
		//		pm_params.d_pml.velocity[ 2 ] = 0;
		if( pm_params.d_pm.s.gravity > 0 )
			pm_params.d_pml.velocity[ 2 ] = 0;
		else
			pm_params.d_pml.velocity[ 2 ] -= pm_params.d_pm.s.gravity * pm_params.d_pml.frametime;
		// PGM
		if( !pm_params.d_pml.velocity[ 0 ] && !pm_params.d_pml.velocity[ 1 ] )
			return;
		PM_StepSlideMove( pm_params );
	} else { // not on ground, so little effect on velocity
		if( pm_airaccelerate )
			PM_AirAccelerate( pm_params, wishdir, wishspeed, pm_accelerate );
		else
			PM_Accelerate( pm_params, wishdir, wishspeed, 1 );
		// add gravity
		pm_params.d_pml.velocity[ 2 ] -= pm_params.d_pm.s.gravity * pm_params.d_pml.frametime;
		PM_StepSlideMove( pm_params );
	}
}

void PlayerMove::PM_CatagorizePosition( PmoveParams & pm_params ) {
	Vec3 point;
	int cont;
	CTrace trace;
	float sample1;
	float sample2;
	// if the player hull point one unit down is solid, the player is on ground see if standing on something solid	
	point[ 0 ] = pm_params.d_pml.origin[ 0 ];
	point[ 1 ] = pm_params.d_pml.origin[ 1 ];
	point[ 2 ] = pm_params.d_pml.origin[ 2 ] - 0.25f;
	if( pm_params.d_pml.velocity[ 2 ] > 180.0f ) { //!!ZOID changed from 100 to 180( ramp accel )
		pm_params.d_pm.s.pm_flags &= ~PMF_ON_GROUND;
		pm_params.d_pm.groundentity = NULL;
	} else {
		trace = pm_params.d_pm.trace( pm_params.d_pml.origin, pm_params.d_pm.mins, pm_params.d_pm.maxs, point );
		pm_params.d_pml.groundplane = trace.plane;
		pm_params.d_pml.groundsurface = trace.surface;
		pm_params.d_pml.groundcontents = trace.contents;
		if( !trace.ent ||( trace.plane.Normal( )[ 2 ] < 0.7f && !trace.startsolid ) ) {
			pm_params.d_pm.groundentity = NULL;
			pm_params.d_pm.s.pm_flags &= ~PMF_ON_GROUND;
		} else {
			pm_params.d_pm.groundentity = trace.ent;
			// hitting solid ground will end a waterjump
			if( pm_params.d_pm.s.pm_flags & PMF_TIME_WATERJUMP ) {
				pm_params.d_pm.s.pm_flags &= ~( PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT );
				pm_params.d_pm.s.pm_time = 0;
			}
			if( !( pm_params.d_pm.s.pm_flags & PMF_ON_GROUND ) ) { // just hit the ground
				pm_params.d_pm.s.pm_flags |= PMF_ON_GROUND;
				// don't do landing time if we were just going down a slope
				if( pm_params.d_pml.velocity[ 2 ] < -200.0f ) {
					pm_params.d_pm.s.pm_flags |= PMF_TIME_LAND;
					// don't allow another jump for a little while
					if( pm_params.d_pml.velocity[ 2 ] < -400.0f )
						pm_params.d_pm.s.pm_time = 25;
					else
						pm_params.d_pm.s.pm_time = 18;
				}
			}
		}
		if( pm_params.d_pm.numtouch < MAXTOUCH && trace.ent ) {
			pm_params.d_pm.touchents[ pm_params.d_pm.numtouch ] = trace.ent;
			pm_params.d_pm.numtouch++;
		}
	}
	// get waterlevel, accounting for ducking
	pm_params.d_pm.waterlevel = 0;
	pm_params.d_pm.watertype = 0;
	sample2 = pm_params.d_pm.viewheight - pm_params.d_pm.mins[ 2 ];
	sample1 = sample2 * 0.5f;
	point[ 2 ] = pm_params.d_pml.origin[ 2 ] + pm_params.d_pm.mins[ 2 ] + 1.0f;
	cont = pm_params.d_pm.pointcontents( point );
	if( cont & MASK_WATER ) {
		pm_params.d_pm.watertype = cont;
		pm_params.d_pm.waterlevel = 1;
		point[ 2 ] = pm_params.d_pml.origin[ 2 ] + pm_params.d_pm.mins[ 2 ] + sample1;
		cont = pm_params.d_pm.pointcontents( point );
		if( cont & MASK_WATER ) {
			pm_params.d_pm.waterlevel = 2;
			point[ 2 ] = pm_params.d_pml.origin[ 2 ] + pm_params.d_pm.mins[ 2 ] + sample2;
			cont = pm_params.d_pm.pointcontents( point );
			if( cont & MASK_WATER ) pm_params.d_pm.waterlevel = 3;
		}
	}
}

void PlayerMove::PM_CheckJump( PmoveParams & pm_params ) {
	if( pm_params.d_pm.s.pm_flags & PMF_TIME_LAND ) { // hasn't been long enough since landing to jump again
		return;
	}
	if( pm_params.d_pm.cmd.upmove < 10 ) { // not holding jump
		pm_params.d_pm.s.pm_flags &= ~PMF_JUMP_HELD;
		return;
	}
	// must wait for jump to be released
	if( pm_params.d_pm.s.pm_flags & PMF_JUMP_HELD )
		return;
	if( pm_params.d_pm.s.pm_type == PM_DEAD )
		return;
	if( pm_params.d_pm.waterlevel >= 2 ) { // swimming, not jumping
		pm_params.d_pm.groundentity = NULL;
		if( pm_params.d_pml.velocity[ 2 ] <= -300 )
			return;
		if( pm_params.d_pm.watertype == CONTENTS_WATER )
			pm_params.d_pml.velocity[ 2 ] = 100;
		else if( pm_params.d_pm.watertype == CONTENTS_SLIME )
			pm_params.d_pml.velocity[ 2 ] = 80;
		else
			pm_params.d_pml.velocity[ 2 ] = 50;
		return;
	}
	if( pm_params.d_pm.groundentity == NULL )
		return; // in air, so no effect
	pm_params.d_pm.s.pm_flags |= PMF_JUMP_HELD;
	pm_params.d_pm.groundentity = NULL;
	pm_params.d_pml.velocity[ 2 ] += 270;
	if( pm_params.d_pml.velocity[ 2 ] < 270 )
		pm_params.d_pml.velocity[ 2 ] = 270;
}

void PlayerMove::PM_CheckSpecialMovement( PmoveParams & pm_params ) {
	Vec3 spot;
	int cont;
	Vec3 flatforward;
	CTrace trace;
	if( pm_params.d_pm.s.pm_time )
		return;
	pm_params.d_pml.ladder = false;
	// check for ladder
	flatforward[ 0 ] = pm_params.d_pml.forward[ 0 ];
	flatforward[ 1 ] = pm_params.d_pml.forward[ 1 ];
	flatforward[ 2 ] = 0;
	flatforward.Normalize( );
	spot = pm_params.d_pml.origin.Magnitude( 1.0f, flatforward );
	trace = pm_params.d_pm.trace( pm_params.d_pml.origin, pm_params.d_pm.mins, pm_params.d_pm.maxs, spot );
	if( ( trace.fraction < 1 ) && ( trace.contents & CONTENTS_LADDER ) )
		pm_params.d_pml.ladder = true;
	// check for water jump
	if( pm_params.d_pm.waterlevel != 2 )
		return;
	spot = pm_params.d_pml.origin.Magnitude( 30.0f, flatforward );
	spot[ 2 ] += 4;
	cont = pm_params.d_pm.pointcontents( spot );
	if( !( cont & CONTENTS_SOLID ) )
		return;
	spot[ 2 ] += 16;
	cont = pm_params.d_pm.pointcontents( spot );
	if( cont )
		return;
	// jump out of water
	pm_params.d_pml.velocity = flatforward * 50.0f;
	pm_params.d_pml.velocity[ 2 ] = 350.0f;
	pm_params.d_pm.s.pm_flags |= PMF_TIME_WATERJUMP;
	pm_params.d_pm.s.pm_time = 255;
}

void PlayerMove::PM_FlyMove( PmoveParams & pm_params, bool doclip ) {
	float speed, drop, friction, control, newspeed;
	float currentspeed, addspeed, accelspeed;
	int i;
	Vec3 wishvel;
	float fmove, smove;
	Vec3 wishdir;
	float wishspeed;
	Vec3 end;
	CTrace trace;
	pm_params.d_pm.viewheight = 22;
	// friction
	speed = pm_params.d_pml.velocity.Length( );
	if( speed < 1.0f )
		pm_params.d_pml.velocity = vec3_origin;
	else {
		drop = 0;
		friction = pm_friction * 1.5f; // extra friction
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control* friction* pm_params.d_pml.frametime;
		// scale the velocity
		newspeed = speed - drop;
		if( newspeed < 0 )
			newspeed = 0;
		newspeed /= speed;
		pm_params.d_pml.velocity *= newspeed;
	}
	// accelerate
	fmove = pm_params.d_pm.cmd.forwardmove;
	smove = pm_params.d_pm.cmd.sidemove;
	pm_params.d_pml.forward.Normalize( );
	pm_params.d_pml.right.Normalize( );
	for( i = 0; i < 3; i++ )
		wishvel[ i ] = pm_params.d_pml.forward[ i ] * fmove + pm_params.d_pml.right[ i ] * smove;
	wishvel[ 2 ] += pm_params.d_pm.cmd.upmove;
	wishdir = wishvel;
	wishspeed = wishdir.Normalize( );
	// clamp to server defined max speed
	if( wishspeed > pm_maxspeed ) {
		wishvel *= pm_maxspeed / wishspeed;
		wishspeed = pm_maxspeed;
	}
	currentspeed = pm_params.d_pml.velocity * wishdir;
	addspeed = wishspeed - currentspeed;
	if( addspeed <= 0 )
		return;
	accelspeed = pm_accelerate * pm_params.d_pml.frametime* wishspeed;
	if( accelspeed > addspeed )
		accelspeed = addspeed;

	for( i = 0; i < 3; i++ )
		pm_params.d_pml.velocity[ i ] += accelspeed * wishdir[ i ];
	if( doclip ) {
		for( i = 0; i < 3; i++ )
			end[ i ] = pm_params.d_pml.origin[ i ] + pm_params.d_pml.frametime * pm_params.d_pml.velocity[ i ];
		trace = pm_params.d_pm.trace( pm_params.d_pml.origin, pm_params.d_pm.mins, pm_params.d_pm.maxs, end );
		pm_params.d_pml.origin = trace.endpos;
	} else { // move
		pm_params.d_pml.origin = pm_params.d_pml.origin.Magnitude( pm_params.d_pml.frametime, pm_params.d_pml.velocity );
	}
}

void PlayerMove::PM_CheckDuck( PmoveParams & pm_params ) {
	CTrace trace;
	pm_params.d_pm.mins[ 0 ] = -16;
	pm_params.d_pm.mins[ 1 ] = -16;
	pm_params.d_pm.maxs[ 0 ] = 16;
	pm_params.d_pm.maxs[ 1 ] = 16;
	if( pm_params.d_pm.s.pm_type == PM_GIB ) {
		pm_params.d_pm.mins[ 2 ] = 0;
		pm_params.d_pm.maxs[ 2 ] = 16;
		pm_params.d_pm.viewheight = 8;
		return;
	}
	pm_params.d_pm.mins[ 2 ] = -24;
	if( pm_params.d_pm.s.pm_type == PM_DEAD ) {
		pm_params.d_pm.s.pm_flags |= PMF_DUCKED;
	} else if( pm_params.d_pm.cmd.upmove < 0 &&( pm_params.d_pm.s.pm_flags & PMF_ON_GROUND ) ) { // duck
		pm_params.d_pm.s.pm_flags |= PMF_DUCKED;
	} else { // stand up if possible
		if( pm_params.d_pm.s.pm_flags & PMF_DUCKED )
		{
			// try to stand up
			pm_params.d_pm.maxs[ 2 ] = 32;
			trace = pm_params.d_pm.trace( pm_params.d_pml.origin, pm_params.d_pm.mins, pm_params.d_pm.maxs, pm_params.d_pml.origin );
			if( !trace.allsolid )
				pm_params.d_pm.s.pm_flags &= ~PMF_DUCKED;
		}
	}
	if( pm_params.d_pm.s.pm_flags & PMF_DUCKED ) {
		pm_params.d_pm.maxs[ 2 ] = 4;
		pm_params.d_pm.viewheight = -2;
	} else {
		pm_params.d_pm.maxs[ 2 ] = 32;
		pm_params.d_pm.viewheight = 22;
	}
}

void PlayerMove::PM_DeadMove( PmoveParams & pm_params ) {
	float forward;
	if( !pm_params.d_pm.groundentity )
		return;
	// extra friction
	forward = pm_params.d_pml.velocity.Length( );
	forward -= 20.0f;
	if( forward <= 0 ) {
		pm_params.d_pml.velocity = vec3_origin;
	} else {
		pm_params.d_pml.velocity.Normalize( );
		pm_params.d_pml.velocity *= forward;
	}
}


bool PlayerMove::PM_GoodPosition( PmoveParams & pm_params ) {
	CTrace trace;
	Vec3 origin, end;
	if( pm_params.d_pm.s.pm_type == PM_SPECTATOR )
		return true;
	for( int i = 0; i < 3; i++ )
		origin[ i ] = end[ i ] = ( float )pm_params.d_pm.s.origin[ i ] * 0.125f;
	trace = pm_params.d_pm.trace( origin, pm_params.d_pm.mins, pm_params.d_pm.maxs, end );
	return !trace.allsolid;
}

void PlayerMove::PM_SnapPosition( PmoveParams & pm_params ) {
	int sign[ 3 ];
	int i, j, bits;
	short base[ 3 ];
	// try all single bits first
	static int			jitterbits[ 8 ] = {0, 4, 1, 2, 3, 5, 6, 7};
	// snap velocity to eigths
	for( i = 0; i<3; i++ )
		pm_params.d_pm.s.velocity[ i ] =( int )( pm_params.d_pml.velocity[ i ]* 8 );
	for( i = 0; i<3; i++ ) {
		if( pm_params.d_pml.origin[ i ] >= 0 )
			sign[ i ] = 1;
		else
			sign[ i ] = -1;
		pm_params.d_pm.s.origin[ i ] =( int )( pm_params.d_pml.origin[ i ]* 8 );
		if( ( ( float )pm_params.d_pm.s.origin[ i ] * 0.125f ) == pm_params.d_pml.origin[ i ] )
			sign[ i ] = 0;
	}
	base[ 0 ] = pm_params.d_pm.s.origin[ 0 ];
	base[ 1 ] = pm_params.d_pm.s.origin[ 1 ];
	base[ 2 ] = pm_params.d_pm.s.origin[ 2 ];
	// try all combinations
	for( j = 0; j<8; j++ ) {
		bits = jitterbits[ j ];
		pm_params.d_pm.s.origin[ 0 ] = base[ 0 ];
		pm_params.d_pm.s.origin[ 1 ] = base[ 1 ];
		pm_params.d_pm.s.origin[ 2 ] = base[ 2 ];
		for( i = 0; i<3; i++ )
			if( bits &( 1<<i ) )
				pm_params.d_pm.s.origin[ i ] += sign[ i ];
		if( PM_GoodPosition( pm_params ) )
			return;
	}
	// go back to the last position
	pm_params.d_pm.s.origin[ 0 ] = ( short )pm_params.d_pml.previous_origin[ 0 ];
	pm_params.d_pm.s.origin[ 1 ] = ( short )pm_params.d_pml.previous_origin[ 1 ];
	pm_params.d_pm.s.origin[ 2 ] = ( short )pm_params.d_pml.previous_origin[ 2 ];
	//	Com_DPrintf( "using previous_origin\n" );
}

void PlayerMove::PM_InitialSnapPosition( PmoveParams & pm_params ) {
	int x, y, z;
	short base[ 3 ];
	static int			offset[ 3 ] = { 0, -1, 1 };
	memcpy( base, pm_params.d_pm.s.origin, sizeof( short ) * 3 );
	for( z = 0; z < 3; z++ ) {
		pm_params.d_pm.s.origin[ 2 ] = base[ 2 ] + offset[ z ];
		for( y = 0; y < 3; y++ ) {
			pm_params.d_pm.s.origin[ 1 ] = base[ 1 ] + offset[ y ];
			for( x = 0; x < 3; x++ ) {
				pm_params.d_pm.s.origin[ 0 ] = base[ 0 ] + offset[ x ];
				if( PM_GoodPosition( pm_params ) ) {
					pm_params.d_pml.origin = Vec3( pm_params.d_pm.s.origin[ 0 ], pm_params.d_pm.s.origin[ 1 ], pm_params.d_pm.s.origin[ 2 ] ) * 0.125f;
					pm_params.d_pml.previous_origin = Vec3( pm_params.d_pm.s.origin[ 0 ], pm_params.d_pm.s.origin[ 1 ], pm_params.d_pm.s.origin[ 2 ] );
					return;
				}
			}
		}
	}
	Common::Com_DPrintf( "Bad InitialSnapPosition\n" );
}

void PlayerMove::PM_ClampAngles( PmoveParams & pm_params ) {
	short temp;
	int i;
	if( pm_params.d_pm.s.pm_flags & PMF_TIME_TELEPORT ) {
		pm_params.d_pm.viewangles[ YAW ] = SHORT2ANGLE( pm_params.d_pm.cmd.angles[ YAW ] + pm_params.d_pm.s.delta_angles[ YAW ] );
		pm_params.d_pm.viewangles[ PITCH ] = 0;
		pm_params.d_pm.viewangles[ ROLL ] = 0;
	} else {
		// circularly clamp the angles with deltas
		for( i = 0; i<3; i++ ) {
			temp = pm_params.d_pm.cmd.angles[ i ] + pm_params.d_pm.s.delta_angles[ i ];
			pm_params.d_pm.viewangles[ i ] = SHORT2ANGLE( temp );
		}
		// don't let the player look up or down more than 90 degrees
		if( pm_params.d_pm.viewangles[ PITCH ] > 89 && pm_params.d_pm.viewangles[ PITCH ] < 180 )
			pm_params.d_pm.viewangles[ PITCH ] = 89;
		else if( pm_params.d_pm.viewangles[ PITCH ] < 271 && pm_params.d_pm.viewangles[ PITCH ] >= 180 )
			pm_params.d_pm.viewangles[ PITCH ] = 271;
	}
	pm_params.d_pm.viewangles.AngleVectors( &pm_params.d_pml.forward, &pm_params.d_pml.right, &pm_params.d_pml.up );
}

void PlayerMove::Pmove( pmove_t * pmove ) {
	PmoveParams pm_params( *pmove );
	// clear results
	pm_params.d_pm.numtouch = 0;
	pm_params.d_pm.viewangles.Zero( );
	pm_params.d_pm.viewheight = 0;
	pm_params.d_pm.groundentity = 0;
	pm_params.d_pm.watertype = 0;
	pm_params.d_pm.waterlevel = 0;
	// convert origin and velocity to float values
	pm_params.d_pml.origin = Vec3( pm_params.d_pm.s.origin[ 0 ], pm_params.d_pm.s.origin[ 1 ], pm_params.d_pm.s.origin[ 2 ] ) * 0.125f;
	pm_params.d_pml.velocity = Vec3( pm_params.d_pm.s.velocity[ 0 ], pm_params.d_pm.s.velocity[ 1 ], pm_params.d_pm.s.velocity[ 2 ] ) * 0.125f;
	// save old org in case we get stuck
	pm_params.d_pml.previous_origin = Vec3( pm_params.d_pm.s.origin[ 0 ], pm_params.d_pm.s.origin[ 1 ], pm_params.d_pm.s.origin[ 2 ] );
	pm_params.d_pml.frametime = pm_params.d_pm.cmd.msec * 0.001f;
	PM_ClampAngles( pm_params );
	if( pm_params.d_pm.s.pm_type == PM_SPECTATOR ) {
		PM_FlyMove( pm_params, false );
		PM_SnapPosition( pm_params );
		return;
	}
	if( pm_params.d_pm.s.pm_type >= PM_DEAD )	{
		pm_params.d_pm.cmd.forwardmove = 0;
		pm_params.d_pm.cmd.sidemove = 0;
		pm_params.d_pm.cmd.upmove = 0;
	}
	if( pm_params.d_pm.s.pm_type == PM_FREEZE )
		return; // no movement at all
	// set mins, maxs, and viewheight
	PM_CheckDuck( pm_params );
	if( pm_params.d_pm.snapinitial )
		PM_InitialSnapPosition( pm_params );
	// set groundentity, watertype, and waterlevel
	PM_CatagorizePosition( pm_params );
	if( pm_params.d_pm.s.pm_type == PM_DEAD )
		PM_DeadMove( pm_params );
	PM_CheckSpecialMovement( pm_params );
	// drop timing counter
	if( pm_params.d_pm.s.pm_time ) {
		int msec;
		msec = pm_params.d_pm.cmd.msec >> 3;
		if( !msec )
			msec = 1;
		if( msec >= pm_params.d_pm.s.pm_time ) {
			pm_params.d_pm.s.pm_flags &= ~( PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT );
			pm_params.d_pm.s.pm_time = 0;
		} else
			pm_params.d_pm.s.pm_time -= msec;
	}
	if( pm_params.d_pm.s.pm_flags & PMF_TIME_WATERJUMP ) { // waterjump has no control, but falls
		pm_params.d_pml.velocity[ 2 ] -= pm_params.d_pm.s.gravity * pm_params.d_pml.frametime;
		if( pm_params.d_pml.velocity[ 2 ] < 0 ) { // cancel as soon as we are falling down again
			pm_params.d_pm.s.pm_flags &= ~( PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT );
			pm_params.d_pm.s.pm_time = 0;
		}
		PM_StepSlideMove( pm_params );
	} else {
		PM_CheckJump( pm_params );
		PM_Friction( pm_params );
		if( pm_params.d_pm.waterlevel >= 2 )
			PM_WaterMove( pm_params );
		else {
			Vec3 angles;
			angles = pm_params.d_pm.viewangles;
			if( angles[ PITCH ] > 180.0f )
				angles[ PITCH ] = angles[ PITCH ] - 360.0f;
			angles[ PITCH ] /= 3.0f;
			angles.AngleVectors( &pm_params.d_pml.forward, &pm_params.d_pml.right, &pm_params.d_pml.up );
			PM_AirMove( pm_params );
		}
	}
	// set groundentity, watertype, and waterlevel for final spot
	PM_CatagorizePosition( pm_params );
	PM_SnapPosition( pm_params );
}
