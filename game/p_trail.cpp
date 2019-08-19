#include "precompiled.h"
#pragma hdrstop

/*

==============================================================================



PLAYER TRAIL



==============================================================================



This is a circular list containing the a list of points of where

the player has been recently.  It is used by monsters for pursuit.



.origin		the spot

.owner		forward link

.aiment		backward link

*/
bool Game::trail_active = false;





void Game::PlayerTrail_Init( ) {

	int n;

	if( deathmatch.GetBool( ) /* FIXME || coop */ )
		return;

	for( n = 0; n < TRAIL_LENGTH; n++ )
	{
		trail[ n ] = G_Spawn( );
		trail[ n ]->classname = "player_trail";
	}

	trail_head = 0;
	trail_active = true;
}


void Game::PlayerTrail_Add( Vec3 & spot ) {

	Vec3 temp;

	if( !trail_active )
		return;

	trail[ trail_head ]->s.origin = spot;

	trail[ trail_head ]->timestamp = level.time;

	temp = spot - trail[( ( ( trail_head ) - 1 ) &( TRAIL_LENGTH - 1 ) )]->s.origin;
	trail[ trail_head ]->s.angles[ 1 ] = vectoyaw( temp );

	trail_head =( ( ( trail_head ) + 1 ) &( TRAIL_LENGTH - 1 ) );
}


void Game::PlayerTrail_New( Vec3 & spot ) {

	if( !trail_active )
		return;

	PlayerTrail_Init( );
	PlayerTrail_Add( spot );
}


Entity * Game::PlayerTrail_PickFirst( Entity * self ) {

	int marker;
	int n;

	if( !trail_active )
		return NULL;

	for( marker = trail_head, n = TRAIL_LENGTH; n; n-- )
	{
		if( trail[ marker ]->timestamp <= self->monsterinfo.trail_time )
			marker =( ( ( marker ) + 1 ) &( TRAIL_LENGTH - 1 ) );
		else
			break;
	}

	if( visible( self, trail[ marker ] ) ) {
		return trail[ marker ];
	}

	if( visible( self, trail[( ( ( marker ) - 1 ) &( TRAIL_LENGTH - 1 ) )] ) ) {
		return trail[( ( ( marker ) - 1 ) &( TRAIL_LENGTH - 1 ) )];
	}

	return trail[ marker ];
}

Entity * Game::PlayerTrail_PickNext( Entity * self ) {

	int marker;
	int n;

	if( !trail_active )
		return NULL;

	for( marker = trail_head, n = TRAIL_LENGTH; n; n-- )
	{
		if( trail[ marker ]->timestamp <= self->monsterinfo.trail_time )
			marker =( ( ( marker ) + 1 ) &( TRAIL_LENGTH - 1 ) );
		else
			break;
	}

	return trail[ marker ];
}

Entity * Game::PlayerTrail_LastSpot( ) {

	return trail[( ( ( trail_head ) - 1 ) &( TRAIL_LENGTH - 1 ) ) ];
}
