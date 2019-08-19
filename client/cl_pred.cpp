#include "precompiled.h"
#pragma hdrstop

void Client::CL_CheckPredictionError( ) {
	if( !cl_predict.GetBool( ) || ( cl.frame.playerstate.pmove.pm_flags & PMF_NO_PREDICTION ) )
		return;
	// calculate the last usercmd_t we sent that the server has processed
	int frame = ( cls.framecount - 1 ) & CMD_BACKUP_MASK;
	// compare what the server returned with what we had predicted it to be
	int delta[ 3 ];
	delta[ 0 ] = cl.frame.playerstate.pmove.origin[ 0 ] - cl.predicted_origins[ frame ][ 0 ];
	delta[ 1 ] = cl.frame.playerstate.pmove.origin[ 1 ] - cl.predicted_origins[ frame ][ 1 ];
	delta[ 2 ] = cl.frame.playerstate.pmove.origin[ 2 ] - cl.predicted_origins[ frame ][ 2 ];
	// save the prediction error for interpolation
	int len = abs( delta[ 0 ] ) + abs( delta[ 1 ] ) + abs( delta[ 2 ] );
	if( len > 640 ) { // 80 world units .... a teleport or something
		cl.prediction_error = vec3_origin;
	} else {
		if( cl_showmiss.GetBool( ) && ( delta[ 0 ] || delta[ 1 ] || delta[ 2 ] ) )
			Common::Com_Printf( "prediction miss on %i: %i\n", cls.framecount - 1, delta[ 0 ] + delta[ 1 ] + delta[ 2 ] );
		cl.predicted_origins[ frame ][ 0 ] = cl.frame.playerstate.pmove.origin[ 0 ];
		cl.predicted_origins[ frame ][ 1 ] = cl.frame.playerstate.pmove.origin[ 1 ];
		cl.predicted_origins[ frame ][ 2 ] = cl.frame.playerstate.pmove.origin[ 2 ];
		// save for error itnerpolation
		for( int i = 0; i < 3; i++ )
			cl.prediction_error[ i ] = delta[ i ] * 0.125f;
	}
}

void Client::CL_ClipMoveToEntities( Vec3 & start, Vec3 & mins, Vec3 & maxs, Vec3 & end, CTrace & tr ) {
	for( int i = 0; i < cl.frame.num_entities; i++ ) {
		int num = ( cl.frame.parse_entities + i ) & ( MAX_PARSE_ENTITIES - 1 );
		const entity_state_t & ent = cl_parse_entities[ num ];
		if( !ent.solid )
			continue;
		if( ent.number == cl.playernum+1 )
			continue;
		int headnode;
		Vec3 angles;
		if( ent.solid == 31 ) { // special value for bmodel
			CModel * cmodel = cl.model_clip[ ent.modelindex ];
			if( !cmodel )
				continue;
			headnode = cmodel->headnode;
			angles = ent.angles;
		} else { // encoded bbox
			float x = 8.0f * ( ent.solid & 31 );
			float zd = 8.0f * ( ( ent.solid >> 5 ) & 31 );
			float zu = 8.0f * ( ( ent.solid >> 10 ) & 63 ) - 32;
			Vec3 bmins( -x, -x, -zd );
			Vec3 bmaxs( x, x, zu );
			headnode = CollisionModel::CM_HeadnodeForBox( bmins, bmaxs );
			angles = vec3_origin; // boxes don't rotate
		}
		if( tr.allsolid )
			return;
		CTrace trace = CollisionModel::CM_TransformedBoxTrace( start, end, mins, maxs, 1, MASK_PLAYERSOLID, cl_parse_entities[ num ].origin, angles );
		if( trace.allsolid || trace.startsolid || trace.fraction < tr.fraction ) {
			trace.ent = ( Entity * )&cl_parse_entities[ num ];
			if( tr.startsolid ) {
				tr = trace;
				tr.startsolid = true;
			} else {
				tr = trace;
			}
		} else if( trace.startsolid ) {
			tr.startsolid = true;
		}
	}
}

CTrace Client::CL_PMTrace( Vec3 & start, Vec3 & mins, Vec3 & maxs, Vec3 & end ) {
	// check against world
	CTrace t = CollisionModel::CM_BoundTrace( start, end, Bounds( mins, maxs ), 4, MASK_PLAYERSOLID );
	if( t.fraction < 1.0f )
		t.ent = ( Entity * )1;
	// check all other solid models
	CL_ClipMoveToEntities( start, mins, maxs, end, t );
	return t;
}

int Client::CL_PMpointcontents( Vec3 & point ) {
	int contents = CollisionModel::CM_PointContents( point, 0 );
	for( int i = 0; i < cl.frame.num_entities; i++ ) {
		int num = ( cl.frame.parse_entities + i ) & ( MAX_PARSE_ENTITIES - 1 );
		const entity_state_t & ent = cl_parse_entities[ num ];
		if( ent.solid != 31 ) // special value for bmodel
			continue;
		CModel * cmodel = cl.model_clip[ ent.modelindex ];
		if( !cmodel )
			continue;
		contents |= CollisionModel::CM_TransformedPointContents( point, cmodel->headnode, ent.origin, ent.angles );
	}
	return contents;
}

#if 0
void Client::CL_PredictMovement( ) {
	int ack, current;
	int frame;
	int oldframe;
	usercmd_t * cmd;
	pmove_t pm;
	int i;
	int step;
	int oldz;
	if( cls.state != ca_active )
		return;
	if( Common::paused.GetBool( ) )
		return;
	if( !cl_predict.GetBool( ) || ( cl.frame.playerstate.pmove.pm_flags & PMF_NO_PREDICTION ) ) { // just set angles
		for( i = 0; i < 3; i++ )
			cl.predicted_angles[ i ] = cl.viewangles[ i ] + SHORT2ANGLE( cl.frame.playerstate.pmove.delta_angles[ i ] );
		return;
	}
	ack = Client::cl.frame.deltaframe;
	current = Client::cl.frame.serverframe;
	// if we are too far out of date, just freeze
	if( current - ack >= CMD_BACKUP ) {
		if( cl_showmiss.GetBool( ) )
			Common::Com_Printf( "exceeded CMD_BACKUP\n" );
		return;
	}
	// copy current state to pmove
	pm.Clear( );
	pm.trace = CL_PMTrace;
	pm.pointcontents = CL_PMpointcontents;
	pm_airaccelerate = ( float )atof( cl.configstrings[ CS_AIRACCEL ] );
	pm.s = cl.frame.playerstate.pmove;
	//	SCR_DebugGraph( current - ack - 1, 0 );
	frame = 0;
	// run frames
	while( ack < current ) {
		ack++;
		frame = ack & ( CMD_BACKUP-1 );
		cmd = &cl.cmds[ frame ];
		pm.cmd = *cmd;
		PlayerMove::Pmove( &pm );
		// save for debug checking
		cl.predicted_origins[ frame ][ 0 ] = pm.s.origin[ 0 ];
		cl.predicted_origins[ frame ][ 1 ] = pm.s.origin[ 1 ];
		cl.predicted_origins[ frame ][ 2 ] = pm.s.origin[ 2 ];
	}
	oldframe = ( ack - 2 ) & ( CMD_BACKUP - 1 );
	oldz = cl.predicted_origins[ oldframe ][ 2 ];
	step = pm.s.origin[ 2 ] - oldz;
	if( step > 63 && step < 160 && ( pm.s.pm_flags & PMF_ON_GROUND ) ) {
		cl.predicted_step = step * 0.125f;
		cl.predicted_step_time = cls.realtime - ( timeType )( cls.frametime * 500 );
	}
	// copy results out for rendering
	cl.predicted_origin[ 0 ] = ( float )pm.s.origin[ 0 ] * 0.125f;
	cl.predicted_origin[ 1 ] = ( float )pm.s.origin[ 1 ] * 0.125f;
	cl.predicted_origin[ 2 ] = ( float )pm.s.origin[ 2 ] * 0.125f;
	cl.predicted_angles = pm.viewangles;
}
#endif

void Client::CL_PredictMovement( ) {
	if( Common::paused.GetBool( ) )
		return;
	if( !cl_predict.GetBool( ) || ( cl.frame.playerstate.pmove.pm_flags & PMF_NO_PREDICTION ) ) { // just set angles
		for( int i = 0; i < 3; i++ )
			cl.predicted_angles[ i ] = cl.viewangles[ i ] + SHORT2ANGLE( cl.frame.playerstate.pmove.delta_angles[ i ] );
		return;
	}
	// copy current state to pmove
	pmove_t pm;
	pm.Clear( );
	pm.trace = CL_PMTrace;
	pm.pointcontents = CL_PMpointcontents;
	pm_airaccelerate = ( float )atof( cl.configstrings[ CS_AIRACCEL ] );
	pm.s = cl.frame.playerstate.pmove;
	//	SCR_DebugGraph( current - ack - 1, 0 );
	// run frames
	int frame = cls.framecount & CMD_BACKUP_MASK;
	pm.cmd = cl.cmds[ frame ];
	PlayerMove::Pmove( &pm );
	// save for debug checking
	cl.predicted_origins[ frame ][ 0 ] = pm.s.origin[ 0 ];
	cl.predicted_origins[ frame ][ 1 ] = pm.s.origin[ 1 ];
	cl.predicted_origins[ frame ][ 2 ] = pm.s.origin[ 2 ];
	int oldFrame = ( cls.framecount - 2 ) & CMD_BACKUP_MASK;
	int oldz = cl.predicted_origins[ frame ][ 2 ];
	int step = pm.s.origin[ 2 ] - oldz;
	if( step > 63 && step < 160 && ( pm.s.pm_flags & PMF_ON_GROUND ) ) {
		cl.predicted_step = step * 0.125f;
		cl.predicted_step_time = cls.realtime - ( timeType )( cls.frametime * 500 );
	}
	// copy results out for rendering
	cl.predicted_origin[ 0 ] = ( float )pm.s.origin[ 0 ] * 0.125f;
	cl.predicted_origin[ 1 ] = ( float )pm.s.origin[ 1 ] * 0.125f;
	cl.predicted_origin[ 2 ] = ( float )pm.s.origin[ 2 ] * 0.125f;
	cl.predicted_angles = pm.viewangles;
}
