#include "precompiled.h"
#pragma hdrstop

// cl_view.c -- player rendering positioning

int					View::gun_frame;
RenderModel *		View::gun_model;

//=============

List< SceneEntity >		View::r_entities;
List< SceneLight >		View::r_dlights;

/*
====================
V_ClearScene

Specifies the model that will be used as the world
====================
*/
void View::V_ClearScene( ) {

	r_entities.Clear( );
	r_dlights.Clear( );
}

/*
=====================
V_AddEntity
=====================
*/
void View::V_AddEntity( const SceneEntity & ent ) {

	r_entities.Append( ent );
}


/*

=====================

V_AddParticle

=====================

*/
void View::V_AddParticle( Vec3 & org, int color, float alpha ) {

}

/*

=====================

V_AddLight



=====================

*/
void View::V_AddLight( Vec3 & org, float intensity, float r, float g, float b ) {

	r_dlights.Append( SceneLight( org, Vec3( r, g, b ), intensity ) );
}


/*

=====================

V_AddLightStyle



=====================

*/
void View::V_AddLightStyle( int style, float r, float g, float b ) {

}

/*

================
View::V_TestParticles

If cl_testparticles is set, create 4096 particles in the view
================

*/
void View::V_TestParticles( ) {

}

/*

================
V_TestEntities

If cl_testentities is set, create 32 player models
================

*/
void View::V_TestEntities( ) {

}

/*

================
V_TestLights

If cl_testlights is set, create 32 lights models
================

*/
void View::V_TestLights( ) {

}

//===================================================================

/*

=================

CL_PrepRefresh



Call before entering a new level, or after changing dlls

=================

*/
void Client::CL_PrepRefresh( ) {

	Str mapname;
	int i;
	Str name;
	float rotate;
	Vec3 axis;

	if( Client::cl.configstrings[ CS_MODELS + 1 ].IsEmpty( ) )
		return; // no map loaded

	Screen::SCR_AddDirtyPoint( 0, 0 );
	Screen::SCR_AddDirtyPoint( Video::viddef.width-1, Video::viddef.height-1 );

	// let the render dll load the map
	mapname = Client::cl.configstrings[ CS_MODELS + 1 ].Right( Client::cl.configstrings[ CS_MODELS + 1 ].Length( ) - 5 ); // skip "maps/"
	mapname = mapname.Left( mapname.Length( ) - 4 ); // cut off ".bsp"

	// register models, pics, and skins
	Common::Com_Printf( "Map: %s\r", mapname.c_str( ) );
	Screen::SCR_UpdateScreen( );
	Renderer::BeginRegistration( mapname );
	Common::Com_Printf( "                                     \r" );

	// precache status bar pics
	Common::Com_Printf( "pics\r" );
	Screen::SCR_UpdateScreen( );
	Screen::SCR_TouchPics( );
	Common::Com_Printf( "                                     \r" );

	CL_RegisterTEntModels( );

	View::cl_weaponmodels.Append( "weapon.md2" );

	for( i = 2; i < MAX_MODELS && Client::cl.configstrings[ CS_MODELS + i ].Length( ); i++ )
	{
		name = Client::cl.configstrings[ CS_MODELS+i ];
		if( name[ 0 ] != '*' ) Common::Com_Printf( "%s\r", name.c_str( ) );
		Screen::SCR_UpdateScreen( );
		if( name[ 0 ] == '#' ) {
			// special player weapon model
			
		} else {
			Client::cl.model_draw[ i ] = Renderer::RegisterModel( Str( Client::cl.configstrings[ CS_MODELS + i ] ) );
			if( name[ 0 ] == '*' ) Client::cl.model_clip[ i ] = CollisionModel::CM_InlineModel( Client::cl.configstrings[ CS_MODELS + i ] );
			else Client::cl.model_clip[ i ] = NULL;
		}
		if( name[ 0 ] != '*' ) Common::Com_Printf( "                                     \r" );
	}

	Common::Com_Printf( "images\r", i );
	Screen::SCR_UpdateScreen( );
	for( i = 1; i<MAX_IMAGES && Client::cl.configstrings[ CS_IMAGES + i ].Length( ); i++ ) {

		Client::cl.image_precache[ i ] = MaterialSystem::FindTexture( Client::cl.configstrings[ CS_IMAGES + i ], "hud" );
	}

	Common::Com_Printf( "                                     \r" );
	for( i = 0; i < MAX_CLIENTS; i++ ) {

		if( Client::cl.configstrings[ CS_PLAYERSKINS + i ].IsEmpty( ) ) continue;
		Common::Com_Printf( "client %i\r", i );
		Screen::SCR_UpdateScreen( );
		CL_ParseClientinfo( i );
		Common::Com_Printf( "                                     \r" );
	}

	CL_LoadClientinfo( &Client::cl.baseclientinfo, CVarSystem::GetCVarStr( "g_defplayer" ) );

	// set sky textures and speed
	Common::Com_Printf( "sky\r", i );
	Screen::SCR_UpdateScreen( );
	rotate = ( float )atof( Client::cl.configstrings[ CS_SKYROTATE ] );
	sscanf( Client::cl.configstrings[ CS_SKYAXIS ], "%f %f %f", &axis[ 0 ], &axis[ 1 ], &axis[ 2 ] );
	Renderer::SetSky( Client::cl.configstrings[ CS_SKY ], rotate, axis );
	Common::Com_Printf( "                                     \r" );

	// the renderer can now free unneeded stuff
	Renderer::EndRegistration( );

	// clear any lines of console text
	//Console::Con_ClearNotify( );

	Screen::SCR_UpdateScreen( );
	Client::cl.refresh_prepped = true;
	Client::cl.force_refdef = true; // make sure we have a valid refdef
}

/*

====================

CalcFov

====================

*/
float View::CalcFov( float fov_x, float width, float height ) {

	float a;
	float x;

	if( fov_x < 1 || fov_x > 179 ) Common::Com_Error( ERR_DROP, "Bad fov: %f", fov_x );

	x = width/tan( fov_x/360* M_PI );

	a = atan( height/x );

	a = a* 360/M_PI;

	return a;
}

//============================================================================

// gun frame debugging functions
void View::V_Gun_Next_f( ) {

	gun_frame++;
	Common::Com_Printf( "frame %i\n", gun_frame );
}

void View::V_Gun_Prev_f( ) {

	gun_frame--;
	if( gun_frame < 0 )
		gun_frame = 0;
	Common::Com_Printf( "frame %i\n", gun_frame );
}

void View::V_Gun_Model_f( ) {
	Str name;

	if( Command::Cmd_Argc( ) != 2 )
	{
		gun_model = NULL;
		return;
	}
	sprintf( name, "models/%s/tris.md2", Command::Cmd_Argv( 1 ).c_str( ) );
	gun_model = Renderer::RegisterModel( name );
}

//============================================================================


/*

=================

SCR_DrawCrosshair

=================

*/
void View::SCR_DrawCrosshair( ) {

	if( !crosshair.GetBool( ) ) return;

	if( crosshair.IsModified( ) ) {

		crosshair.ClearModified( );
		Screen::SCR_TouchPics( );
	}

#if 0
	if( !Screen::crosshair_pic.Length( ) ) return;

	Renderer::DrawPic( Screen::scr_vrect.x +( ( Screen::scr_vrect.width - Screen::crosshair_width ) >> 1 ), Screen::scr_vrect.y +( ( Screen::scr_vrect.height - Screen::crosshair_height ) >> 1 ), Screen::crosshair_pic );
#endif
}

/*
==================

V_RenderView

==================
*/

int entitycmpfnc( const SceneEntity * a, const SceneEntity * b ) {

	// all other models are sorted by model then skin
	if( a->model == b->model ) {

		return( ( int )a->skin - ( int )b->skin );

	} else {

		return( ( int )a->model - ( int )b->model );
	}
}

void View::V_RenderView( ) {

	if( Client::cls.state != ca_active )
		return;

	if( !Client::cl.refresh_prepped )
		return; // still loading

	if( CVarSystem::GetCVarBool( "cl_timedemo" ) )
	{
		if( !Client::cl.timedemo_start )
			Client::cl.timedemo_start = System::Sys_Milliseconds( );
		Client::cl.timedemo_frames++;
	}

	// an invalid frame will just use the exact previous refdef
	// we can't use the old frame if the video mode has changed, though...
	if( Client::cl.frame.valid &&( Client::cl.force_refdef || !Common::paused.GetBool( ) ) )
	{
		Client::cl.force_refdef = false;

		V_ClearScene( );

		// build a refresh entity list and calc Client::cl.sim*
		// this also calls CL_CalcViewValues which loads
		// v_forward, etc.
		Client::CL_AddEntities( );

		if( cl_testparticles.GetBool( ) ) V_TestParticles( );
		if( cl_testentities.GetBool( ) ) V_TestEntities( );
		if( cl_testlights.GetBool( ) ) V_TestLights( );
		if( cl_testblend.GetBool( ) ) {

			Client::cl.refdef.blend[ 0 ] = 1;
			Client::cl.refdef.blend[ 1 ] = 0.5f;
			Client::cl.refdef.blend[ 2 ] = 0.25f;
			Client::cl.refdef.blend[ 3 ] = 0.5f;
		}

		// never let it sit exactly on a node line, because a water plane can
		// dissapear when viewed with the eye exactly on it.
		// the server protocol only specifies to 1/8 pixel, so add 1/16 in each axis
		Client::cl.refdef.vieworg[ 0 ] += 1.0f/16;
		Client::cl.refdef.vieworg[ 1 ] += 1.0f/16;
		Client::cl.refdef.vieworg[ 2 ] += 1.0f/16;

		Client::cl.refdef.x = Screen::scr_vrect.x;
		Client::cl.refdef.y = Screen::scr_vrect.y;
		Client::cl.refdef.width = Screen::scr_vrect.width;
		Client::cl.refdef.height = Screen::scr_vrect.height;
		Client::cl.refdef.fov_y = CalcFov( Client::cl.refdef.fov_x, ( float )Client::cl.refdef.width, ( float )Client::cl.refdef.height );
		Client::cl.refdef.time = Client::cl.time* 0.001f;

		//Client::cl.refdef.areabits = Client::cl.frame.areabits;

		if( !cl_add_entities.GetBool( ) ) r_entities.Clear( );
		if( !cl_add_lights.GetBool( ) )r_dlights.Clear( );
		if( !cl_add_blend.GetBool( ) ) vec3_origin.CopyTo( Client::cl.refdef.blend );

		Client::cl.refdef.entities = r_entities;
		Client::cl.refdef.dlights = r_dlights;

		Client::cl.refdef.rdflags = Client::cl.frame.playerstate.rdflags;

		// sort entities for better cache locality
		//qsort( Client::cl.refdef.entities.Ptr( ), Client::cl.refdef.entities.Num( ), sizeof( Client::cl.refdef.entities[ 0 ] ), ( int( *)( const void * , const void * ) )entitycmpfnc );
		Client::cl.refdef.entities.Sort( entitycmpfnc );
	}

	Renderer::RenderFrame( Client::cl.refdef );

	Screen::SCR_AddDirtyPoint( Screen::scr_vrect.x, Screen::scr_vrect.y );
	Screen::SCR_AddDirtyPoint( Screen::scr_vrect.x + Screen::scr_vrect.width - 1, Screen::scr_vrect.y + Screen::scr_vrect.height - 1 );

	SCR_DrawCrosshair( );
}


/*

=============

V_Viewpos_f

=============

*/
void View::V_Viewpos_f( ) {

	Common::Com_Printf( "( %i %i %i ) : %i\n", ( int )Client::cl.refdef.vieworg[ 0 ], ( int )Client::cl.refdef.vieworg[ 1 ], ( int )Client::cl.refdef.vieworg[ 2 ], ( int )Client::cl.refdef.viewangles[ YAW ] );
}

/*

=============

V_Init

=============

*/
void View::V_Init( ) {

	Command::Cmd_AddCommand( "gun_next", V_Gun_Next_f );
	Command::Cmd_AddCommand( "gun_prev", V_Gun_Prev_f );
	Command::Cmd_AddCommand( "gun_model", V_Gun_Model_f );

	Command::Cmd_AddCommand( "viewpos", V_Viewpos_f );
}
