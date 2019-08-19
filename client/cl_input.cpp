#include "precompiled.h"
#pragma hdrstop

/*
===============================================================================
KEY BUTTONS

Continuous button event tracking is complicated by the fact that two different
input sources( say, mouse button 1 and the control key ) can both press the
same button, but the button should only be released when both of the
pressing key have been released.

When a key event issues a button command( +forward, +attack, etc ), it appends
its key number as a parameter to the command so it can be matched up with
the release.

state bit 0 is the current state of the key
state bit 1 is edge triggered on the up to down transition
state bit 2 is edge triggered on the down to up transition

Key_Event( int key, bool down, unsigned time );

+mlook src time
===============================================================================
*/

kbutton_t in_klook;
kbutton_t in_left, in_right, in_forward, in_back;
kbutton_t in_lookup, in_lookdown, in_moveleft, in_moveright;
kbutton_t in_strafe, in_speed, in_use, in_attack;
kbutton_t in_up, in_down;

int in_impulse;

void Input::IN_KeyDown( kbutton_t * b ) {
	int k;
	Str c;
	c = Command::Cmd_Argv( 1 );
	if( c[ 0 ] )
		k = atoi( c );
	else
		k = -1; // typed manually at the console for continuous down
	if( k == b->down[ 0 ] || k == b->down[ 1 ] )
		return; // repeating key
	if( !b->down[ 0 ] )
		b->down[ 0 ] = k;
	else if( !b->down[ 1 ] )
		b->down[ 1 ] = k;
	else {
		Common::Com_Printf( "Three keys down for a button!\n" );
		return;
	}
	if( b->state & 1 )
		return; // still down
	// save timestamp
	c = Command::Cmd_Argv( 2 );
	b->downtime = atoi( c );
	b->state |= 1 + 2; // down + impulse down
}

void Input::IN_KeyUp( kbutton_t * b ) {
	int k;
	Str c;
	unsigned uptime;
	c = Command::Cmd_Argv( 1 );
	if( c[ 0 ] )
		k = atoi( c );
	else { // typed manually at the console, assume for unsticking, so clear all
		b->down[ 0 ] = b->down[ 1 ] = 0;
		b->state = 4; // impulse up
		return;
	}
	if( b->down[ 0 ] == k )
		b->down[ 0 ] = 0;
	else if( b->down[ 1 ] == k )
		b->down[ 1 ] = 0;
	else
		return; // key up without coresponding down( menu pass through )
	if( b->down[ 0 ] || b->down[ 1 ] )
		return; // some other key is still holding it down
	if( !( b->state & 1 ) )
		return; // still up( this should not happen )
	// save timestamp
	if( b->state & 0x8 )
		b->msec += System::Sys_MillisecondsUint( ) - b->downtime;
	else {
		c = Command::Cmd_Argv( 2 );
		uptime = atoi( c );
		b->msec += uptime - b->downtime;
	}
	b->state &= ~1; // now up
	b->state |= 4; // impulse up
}

void Input::IN_KLookDown( ) {
	IN_KeyDown( &in_klook );
}

void Input::IN_KLookUp( ) {
	IN_KeyUp( &in_klook );
}

void Input::IN_UpDown( ) {	
	IN_KeyDown( &in_up );
}

void Input::IN_UpUp( ) {
	IN_KeyUp( &in_up );
}

void Input::IN_DownDown( ) {
	IN_KeyDown( &in_down );
}

void Input::IN_DownUp( ) {	
	IN_KeyUp( &in_down );
}

void Input::IN_LeftDown( ) {
	IN_KeyDown( &in_left );
}

void Input::IN_LeftUp( ) {	
	IN_KeyUp( &in_left );
}

void Input::IN_RightDown( ) {	
	IN_KeyDown( &in_right );
}

void Input::IN_RightUp( ) {	
	IN_KeyUp( &in_right );
}

void Input::IN_ForwardDown( ) {	
	IN_KeyDown( &in_forward );
}

void Input::IN_ForwardUp( ) {
	IN_KeyUp( &in_forward );
}

void Input::IN_BackDown( ) {
	IN_KeyDown( &in_back );
}

void Input::IN_BackUp( ) {
	IN_KeyUp( &in_back );
}

void Input::IN_LookupDown( ) {
	IN_KeyDown( &in_lookup );
}

void Input::IN_LookupUp( ) {	
	IN_KeyUp( &in_lookup );
}

void Input::IN_LookdownDown( ) {
	IN_KeyDown( &in_lookdown );
}

void Input::IN_LookdownUp( ) {	
	IN_KeyUp( &in_lookdown );
}

void Input::IN_MoveleftDown( ) {	
	IN_KeyDown( &in_moveleft );
}

void Input::IN_MoveleftUp( ) {	
	IN_KeyUp( &in_moveleft );
}

void Input::IN_MoverightDown( ) {	
	IN_KeyDown( &in_moveright );
}

void Input::IN_MoverightUp( ) {	
	IN_KeyUp( &in_moveright );
}

void Input::IN_SpeedDown( ) {	
	IN_KeyDown( &in_speed );
}

void Input::IN_SpeedUp( ) {	
	IN_KeyUp( &in_speed );
}

void Input::IN_StrafeDown( ) {	
	IN_KeyDown( &in_strafe );
}

void Input::IN_StrafeUp( ) {	
	IN_KeyUp( &in_strafe );
}

void Input::IN_AttackDown( ) {	
	IN_KeyDown( &in_attack );
}

void Input::IN_AttackUp( ) {	
	IN_KeyUp( &in_attack );
}

void Input::IN_UseDown( ) {	
	IN_KeyDown( &in_use );
}

void Input::IN_UseUp( ) {	
	IN_KeyUp( &in_use );
}

void Input::IN_Impulse( ) {	
	in_impulse = atoi( Command::Cmd_Argv( 1 ) );
}

float Client::CL_KeyState( kbutton_t * key ) {
	key->state &= 1; // clear impulses
	int msec = key->msec;
	key->msec = 0;
	if( key->state ) { // still down
		key->state |= 0x8;
		uint msgTime = System::Sys_MillisecondsUint( );
		msec += msgTime - key->downtime;
		key->downtime = msgTime;
	}
	return Min( 1.0f, Max( ( msec / 1000.0f ) / cls.frametime, 0.0f ) );
}

void Client::CL_AdjustAngles( ) {
	timeTypeReal speed;
	float up, down;
	if( in_speed.state & 1 )
		speed = Client::cls.frametime * cl_anglespeedkey.GetFloat( );
	else
		speed = Client::cls.frametime;
	if( !( in_strafe.state & 1 ) ) {
		Client::cl.viewangles[ YAW ] -= speed * cl_yawspeed.GetInt( ) * CL_KeyState( &in_right );
		Client::cl.viewangles[ YAW ] += speed * cl_yawspeed.GetInt( ) * CL_KeyState( &in_left );
	}
	if( in_klook.state & 1 ) {
		Client::cl.viewangles[ PITCH ] -= speed* cl_pitchspeed.GetInt( ) * CL_KeyState( &in_forward );
		Client::cl.viewangles[ PITCH ] += speed* cl_pitchspeed.GetInt( ) * CL_KeyState( &in_back );
	}
	up = CL_KeyState( &in_lookup );
	down = CL_KeyState( &in_lookdown );
	Client::cl.viewangles[ PITCH ] -= speed* cl_pitchspeed.GetInt( ) * up;
	Client::cl.viewangles[ PITCH ] += speed* cl_pitchspeed.GetInt( ) * down;
}

void Client::CL_BaseMove( usercmd_t * cmd ) {
	CL_AdjustAngles( );
	cmd->Clear( );
	cmd->angles[ 0 ] = ( short )Client::cl.viewangles[ 0 ];
	cmd->angles[ 1 ] = ( short )Client::cl.viewangles[ 1 ];
	cmd->angles[ 2 ] = ( short )Client::cl.viewangles[ 2 ];
	if( in_strafe.state & 1 ) {
		cmd->sidemove += ( short )( cl_sidespeed.GetInt( ) * CL_KeyState( &in_right ) );
		cmd->sidemove -= ( short )( cl_sidespeed.GetInt( ) * CL_KeyState( &in_left ) );
	}
	cmd->sidemove += ( short )( cl_sidespeed.GetInt( ) * CL_KeyState( &in_moveright ) );
	cmd->sidemove -= ( short )( cl_sidespeed.GetInt( ) * CL_KeyState( &in_moveleft ) );
	cmd->upmove += ( short )( cl_upspeed.GetInt( ) * CL_KeyState( &in_up ) );
	cmd->upmove -= ( short )( cl_upspeed.GetInt( ) * CL_KeyState( &in_down ) );
	if( !( in_klook.state & 1 ) ) {
		cmd->forwardmove += ( short )( cl_forwardspeed.GetInt( ) * CL_KeyState( &in_forward ) );
		cmd->forwardmove -= ( short )( cl_forwardspeed.GetInt( ) * CL_KeyState( &in_back ) );
	}
	// adjust for speed key / running
	if( ( in_speed.state & 1 ) ^ cl_run.GetInt( ) ) {
		cmd->forwardmove *= 2;
		cmd->sidemove *= 2;
		cmd->upmove *= 2;
	}
}

void Client::CL_ClampPitch( ) {
	float pitch;
	pitch = SHORT2ANGLE( Client::cl.frame.playerstate.pmove.delta_angles[ PITCH ] );
	if( pitch > 180 )
		pitch -= 360;
	if( Client::cl.viewangles[ PITCH ] + pitch < -360 )
		Client::cl.viewangles[ PITCH ] += 360; // wrapped
	if( Client::cl.viewangles[ PITCH ] + pitch > 360 )
		Client::cl.viewangles[ PITCH ] -= 360; // wrapped
	if( Client::cl.viewangles[ PITCH ] + pitch > 89 )
		Client::cl.viewangles[ PITCH ] = 89 - pitch;
	if( Client::cl.viewangles[ PITCH ] + pitch < -89 )
		Client::cl.viewangles[ PITCH ] = -89 - pitch;
}

void Client::CL_FinishMove( usercmd_t * cmd ) {
	timeType ms;
	int i;
	// figure button bits
	if( in_attack.state & 3 )
		cmd->buttons |= BUTTON_ATTACK;
	in_attack.state &= ~2;
	if( in_use.state & 3 )
		cmd->buttons |= BUTTON_USE;
	in_use.state &= ~2;
	if( /*Keyboard::Key_AnyKeyDown( ) &&*/ Client::cls.key_dest == key_game )
		cmd->buttons |= BUTTON_ANY;
	// send milliseconds of time to apply the move
	ms = ( timeType )( Client::cls.frametime * 1000 );
	if( ms > 250 )
		ms = 100; // time was unreasonable
	cmd->msec = ( byte )ms;
	CL_ClampPitch( );
	for( i = 0; i<3; i++ )
		cmd->angles[ i ] = ANGLE2SHORT( Client::cl.viewangles[ i ] );
	cmd->impulse = in_impulse;
	in_impulse = 0;
	// send the ambient light level at the player's current position
	cmd->lightlevel = 0; // FIXME LATER remove this
}

void Client::CL_CreateCmd( usercmd_t & cmd ) {
	// get basic movement from keyboard
	CL_BaseMove( &cmd );
	// allow mice or other external controllers to add to the move
	Input::IN_Move( &cmd );
	CL_FinishMove( &cmd );
	//cmd.impulse = Client::cls.framecount;
}

void Input::IN_CenterView( ) {
	Client::cl.viewangles[ PITCH ] = -SHORT2ANGLE( Client::cl.frame.playerstate.pmove.delta_angles[ PITCH ] );
}

void Client::CL_InitInput( ) {
	Command::Cmd_AddCommand( "centerview", Input::IN_CenterView );
	Command::Cmd_AddCommand( "+moveup", Input::IN_UpDown );
	Command::Cmd_AddCommand( "-moveup", Input::IN_UpUp );
	Command::Cmd_AddCommand( "+movedown", Input::IN_DownDown );
	Command::Cmd_AddCommand( "-movedown", Input::IN_DownUp );
	Command::Cmd_AddCommand( "+left", Input::IN_LeftDown );
	Command::Cmd_AddCommand( "-left", Input::IN_LeftUp );
	Command::Cmd_AddCommand( "+right", Input::IN_RightDown );
	Command::Cmd_AddCommand( "-right", Input::IN_RightUp );
	Command::Cmd_AddCommand( "+forward", Input::IN_ForwardDown );
	Command::Cmd_AddCommand( "-forward", Input::IN_ForwardUp );
	Command::Cmd_AddCommand( "+back", Input::IN_BackDown );
	Command::Cmd_AddCommand( "-back", Input::IN_BackUp );
	Command::Cmd_AddCommand( "+lookup", Input::IN_LookupDown );
	Command::Cmd_AddCommand( "-lookup", Input::IN_LookupUp );
	Command::Cmd_AddCommand( "+lookdown", Input::IN_LookdownDown );
	Command::Cmd_AddCommand( "-lookdown", Input::IN_LookdownUp );
	Command::Cmd_AddCommand( "+strafe", Input::IN_StrafeDown );
	Command::Cmd_AddCommand( "-strafe", Input::IN_StrafeUp );
	Command::Cmd_AddCommand( "+moveleft", Input::IN_MoveleftDown );
	Command::Cmd_AddCommand( "-moveleft", Input::IN_MoveleftUp );
	Command::Cmd_AddCommand( "+moveright", Input::IN_MoverightDown );
	Command::Cmd_AddCommand( "-moveright", Input::IN_MoverightUp );
	Command::Cmd_AddCommand( "+speed", Input::IN_SpeedDown );
	Command::Cmd_AddCommand( "-speed", Input::IN_SpeedUp );
	Command::Cmd_AddCommand( "+attack", Input::IN_AttackDown );
	Command::Cmd_AddCommand( "-attack", Input::IN_AttackUp );
	Command::Cmd_AddCommand( "+use", Input::IN_UseDown );
	Command::Cmd_AddCommand( "-use", Input::IN_UseUp );
	Command::Cmd_AddCommand( "impulse", Input::IN_Impulse );
	Command::Cmd_AddCommand( "+klook", Input::IN_KLookDown );
	Command::Cmd_AddCommand( "-klook", Input::IN_KLookUp );
}

void CL_WriteDeltaUsercmd( MessageBuffer & msg_buffer, usercmd_t & from, usercmd_t & cmd ) {
#if 0
	int		bits = 0;
	if( cmd.angles[ 0 ] != from.angles[ 0 ] )
		bits |= CM_ANGLE1;
	if( cmd.angles[ 1 ] != from.angles[ 1 ] )
		bits |= CM_ANGLE2;
	if( cmd.angles[ 2 ] != from.angles[ 2 ] )
		bits |= CM_ANGLE3;
	if( cmd.forwardmove != from.forwardmove )
		bits |= CM_FORWARD;
	if( cmd.sidemove != from.sidemove )
		bits |= CM_SIDE;
	if( cmd.upmove != from.upmove )
		bits |= CM_UP;
	if( cmd.buttons != from.buttons )
		bits |= CM_BUTTONS;
	if( cmd.impulse != from.impulse )
		bits |= CM_IMPULSE;
    msg_buffer.WriteByte( bits );
	if( bits & CM_ANGLE1 )
		msg_buffer.WriteShort( cmd.angles[ 0 ] );
	if( bits & CM_ANGLE2 )
		msg_buffer.WriteShort( cmd.angles[ 1 ] );
	if( bits & CM_ANGLE3 )
		msg_buffer.WriteShort( cmd.angles[ 2 ] );	
	if( bits & CM_FORWARD )
		msg_buffer.WriteShort( cmd.forwardmove );
	if( bits & CM_SIDE )
	  	msg_buffer.WriteShort( cmd.sidemove );
	if( bits & CM_UP )
		msg_buffer.WriteShort( cmd.upmove );
 	if( bits & CM_BUTTONS )
	  	msg_buffer.WriteByte( cmd.buttons );
 	if( bits & CM_IMPULSE )
	    msg_buffer.WriteByte( cmd.impulse );
    msg_buffer.WriteByte( cmd.msec );
	msg_buffer.WriteByte( cmd.lightlevel );
#endif
	msg_buffer.WriteDeltaShort( from.angles[ 0 ], cmd.angles[ 0 ] );
	msg_buffer.WriteDeltaShort( from.angles[ 1 ], cmd.angles[ 1 ] );
	msg_buffer.WriteDeltaShort( from.angles[ 2 ], cmd.angles[ 2 ] );	
	msg_buffer.WriteDeltaShort( from.forwardmove, cmd.forwardmove );
	msg_buffer.WriteDeltaShort( from.sidemove, cmd.sidemove );
	msg_buffer.WriteDeltaShort( from.upmove, cmd.upmove );
	msg_buffer.WriteDeltaByte( from.buttons, cmd.buttons );
	msg_buffer.WriteDeltaByte( from.impulse, cmd.impulse );
    msg_buffer.WriteDeltaByte( from.msec, cmd.msec );
	msg_buffer.WriteDeltaByte( from.lightlevel, cmd.lightlevel );
}

void CL_WriteUsercmd( MessageBuffer & msg_buffer, usercmd_t & cmd ) {
	msg_buffer.WriteDeltaShort( 0, cmd.angles[ 0 ] );
	msg_buffer.WriteDeltaShort( 0, cmd.angles[ 1 ] );
	msg_buffer.WriteDeltaShort( 0, cmd.angles[ 2 ] );	
	msg_buffer.WriteDeltaShort( 0, cmd.forwardmove );
	msg_buffer.WriteDeltaShort( 0, cmd.sidemove );
	msg_buffer.WriteDeltaShort( 0, cmd.upmove );
	msg_buffer.WriteDeltaByte( 0, cmd.buttons );
	msg_buffer.WriteDeltaByte( 0, cmd.impulse );
    msg_buffer.WriteDeltaByte( 0, cmd.msec );
	msg_buffer.WriteDeltaByte( 0, cmd.lightlevel );
}

void Client::CL_BuildCmd( MessageBuffer & msg_buffer ) {
	//sizebuf_t buf;
	//byte data[ 128 ];
	// build a command even if not connected
	// save this command off for prediction
	//int outSeq = Client::cl.frame.serverframe;
	//int i = outSeq & ( CMD_BACKUP-1 );
	//usercmd_t & cmd0 = cl.cmds[ i ];
	//cl.cmd_time[ i ] = cls.realtime; // for netgraph ping calculation
	//cmd0 = CL_CreateCmd( );
	//cl.cmd = cmd0;
	//if( Client::cls.state == ca_disconnected || Client::cls.state == ca_connecting )
	//	return;
	//if( Client::cls.state == ca_connected ) {
	//	if( Client::cls.netchan.message.cursize || System::Sys_LastTime( ) - Client::cls.netchan.last_sent > 1000 )
	//		Netchan::Netchan_Transmit( &Client::cls.netchan, 0, NULL );
	//	return;
	//}
	// send a userinfo update if needed
	//if( Common::userinfo_modified.GetBool( ) ) {
	//	Common::userinfo_modified.SetBool( false );
		//Message::MSG_WriteByte( &Client::cls.netchan.message, clc_userinfo );
		//Message::MSG_WriteString( &Client::cls.netchan.message, CVarSystem::Userinfo( ) );
	//	cl_messageBufferReliable.WriteByte( clc_userinfo );
	//	cl_messageBufferReliable.WriteString( CVarSystem::Userinfo( ) );
	//}
	//Message::SZ_Init( &buf, data, sizeof( data ) );
#if 0
	if( cmd->buttons && 
		Client::cl.cinematictime > 0 &&
		Client::cls.realtime - Client::cl.cinematictime > 1000 &&
		!Client::cl.attractloop )
	{ // skip the rest of the cinematic
		Screen::SCR_FinishCinematic( );
	}
#endif
	// begin a client move command
	msg_buffer.WriteByte( clc_move );
	// save the position for a checksum byte
	int checksumIndex = msg_buffer.GetSize( );
	msg_buffer.WriteByteAlign( );
	msg_buffer.WriteByte( 0 );
	// let the server know what the last frame we
	// got was, so the next message can be delta compressed
	if( cl_nodelta.GetBool( ) || !Client::cl.frame.valid || Client::cls.demowaiting )
		msg_buffer.WriteLong( -1 ); // no compression
	else
		msg_buffer.WriteLong( Client::cl.frame.serverframe );
	//CL_CreateCmd( cl.cmd );
	int cmdOldest = ( cls.framecount - 2 ) & CMD_BACKUP_MASK;
	int cmdOlder = ( cls.framecount - 1 ) & CMD_BACKUP_MASK;
	int cmdCurrent = cls.framecount & CMD_BACKUP_MASK;
	CL_WriteUsercmd( msg_buffer, cl.cmds[ cmdOldest ] );
	CL_WriteDeltaUsercmd( msg_buffer, cl.cmds[ cmdOldest ], cl.cmds[ cmdOlder ] );
	CL_WriteDeltaUsercmd( msg_buffer, cl.cmds[ cmdOlder ], cl.cmds[ cmdCurrent ] );
	// send this and the previous cmds in the message, so
	// if the last packet was dropped, it can be recovered
	//i = ( outSeq - 2 ) & ( CMD_BACKUP - 1 );
	//usercmd_t & cmd1 = cl.cmds[ i ];
	//usercmd_t nullcmd;
	//nullcmd.Clear( );
	//CL_WriteDeltaUsercmd( msg_buffer, nullcmd, cmd1 );
	//usercmd_t oldcmd = cmd1;
	//i = ( outSeq - 1 ) & ( CMD_BACKUP - 1 );
	//usercmd_t & cmd2 = cl.cmds[ i ];
	//CL_WriteDeltaUsercmd( msg_buffer, oldcmd, cmd2 );
	//oldcmd = cmd2;
	//i = ( outSeq ) & ( CMD_BACKUP - 1 );
	//usercmd_t & cmd3 = cl.cmds[ i ];
	//CL_WriteDeltaUsercmd( msg_buffer, oldcmd, cmd3 );
	// calculate a checksum over the move commands
	msg_buffer.WriteByteAlign( );
	msg_buffer.GetData( )[ checksumIndex ] = Common::COM_BlockSequenceCRCByte( msg_buffer.GetData( ) + checksumIndex + 1, msg_buffer.GetSize( ) - checksumIndex - 1, cl.frame.serverframe );
	// deliver the message
	//Netchan::Netchan_Transmit( &Client::cls.netchan, buf.cursize, buf.data );
}
