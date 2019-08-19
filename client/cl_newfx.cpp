#include "precompiled.h"
#pragma hdrstop

/*
======
vectoangles2 - this is duplicated in the game DLL, but I need it
======
*/
void vectoangles2( Vec3 & value1, Vec3 * angles ) {

	float forward;
	float yaw, pitch;

	if( value1[ 1 ] == 0 && value1[ 0 ] == 0 )
	{
		yaw = 0;
		if( value1[ 2 ] > 0 )
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		// PMM - fixed to correct for pitch of 0
		if( value1[ 0 ] )
			yaw =( atan2( value1[ 1 ], value1[ 0 ] ) * 180 / M_PI );
		else if( value1[ 1 ] > 0 )
			yaw = 90;
		else
			yaw = 270;

		if( yaw < 0 )
			yaw += 360;

		forward = sqrt( value1[ 0 ]* value1[ 0 ] + value1[ 1 ]* value1[ 1 ] );
		pitch =( atan2( value1[ 2 ], forward ) * 180 / M_PI );
		if( pitch < 0 )
			pitch += 360;
	}

	angles->ToFloatPtr( )[ PITCH ] = -pitch;
	angles->ToFloatPtr( )[ YAW ] = yaw;
	angles->ToFloatPtr( )[ ROLL ] = 0;
}

//=============
//=============
void Client::CL_Flashlight( int ent, Vec3 & pos ) {

	cdlight_t * dl;

	dl = CL_AllocDlight( ent );
	dl->origin = pos;
	dl->radius = 400;
	dl->minlight = 250;
	dl->die = cl.time + 100;
	dl->color[ 0 ] = 1;
	dl->color[ 1 ] = 1;
	dl->color[ 2 ] = 1;
}

/*

======

CL_ColorFlash - flash of light

======

*/
void Client::CL_ColorFlash( Vec3 & pos, int ent, int intensity, float r, float g, float b ) {

	cdlight_t * dl;

	dl = CL_AllocDlight( ent );
	dl->origin = pos;
	dl->radius = ( float )intensity;
	dl->minlight = 250;
	dl->die = cl.time + 100;
	dl->color[ 0 ] = r;
	dl->color[ 1 ] = g;
	dl->color[ 2 ] = b;
}


/*

======

CL_DebugTrail

======

*/
void Client::CL_DebugTrail( Vec3 & start, Vec3 & end ) {

}

/*

===============

CL_SmokeTrail

===============

*/
void Client::CL_SmokeTrail( Vec3 & start, Vec3 & end, int colorStart, int colorRun, int spacing ) {

}

void Client::CL_ForceWall( Vec3 & start, Vec3 & end, int color ) {

}

void Client::CL_FlameEffects( centity_t * ent, Vec3 & origin ) {

}


/*

===============

CL_GenericParticleEffect

===============

*/
void Client::CL_GenericParticleEffect( Vec3 & org, Vec3 & dir, int color, int count, int numcolors, int dirspread, float alphavel ) {

}

/*

===============

CL_BubbleTrail2( lets you control the # of bubbles by setting the distance between the spawns )



===============

*/
void Client::CL_BubbleTrail2( Vec3 & start, Vec3 & end, int dist ) {

}

//#define CORKSCREW		1
//#define DOUBLE_SCREW	1

//#define	SPRAY		1
void Client::CL_Heatbeam( Vec3 & start, Vec3 & forward ) {

}
/*

===============

CL_ParticleSteamEffect



Puffs with velocity along direction, with some randomness thrown in

===============

*/
void Client::CL_ParticleSteamEffect( Vec3 & org, Vec3 & dir, int color, int count, int magnitude ) {

}

void Client::CL_ParticleSteamEffect2( cl_sustain_t * self ) {

}

/*

===============

CL_TrackerTrail

===============

*/
void Client::CL_TrackerTrail( Vec3 & start, Vec3 & end, int particleColor ) {

	
}

void Client::CL_Tracker_Shell( Vec3 & origin ) {


}

void Client::CL_MonsterPlasma_Shell( Vec3 & origin ) {

	
}

void Client::CL_Widowbeamout( cl_sustain_t * self ) {


}

void Client::CL_Nukeblast( cl_sustain_t * self ) {

	
}

void Client::CL_WidowSplash( Vec3 & org ) {

	
}

void Client::CL_Tracker_Explode( Vec3 & origin ) {

	

}

/*

===============

CL_TagTrail



===============

*/
void Client::CL_TagTrail( Vec3 & start, Vec3 & end, float color ) {

	
}

/*

===============

CL_ColorExplosionParticles

===============

*/
void Client::CL_ColorExplosionParticles( Vec3 & org, int color, int run ) {

}

/*

===============

CL_ParticleSmokeEffect - like the steam effect, but unaffected by gravity

===============

*/
void Client::CL_ParticleSmokeEffect( Vec3 & org, Vec3 & dir, int color, int count, int magnitude ) {


}

/*

===============

CL_BlasterParticles2



Wall impact puffs( Green )

===============

*/
void Client::CL_BlasterParticles2( Vec3 & org, Vec3 & dir, unsigned int color ) {

	
}

/*

===============

CL_BlasterTrail2



Green!

===============

*/
void Client::CL_BlasterTrail2( Vec3 & start, Vec3 & end ) {

}
