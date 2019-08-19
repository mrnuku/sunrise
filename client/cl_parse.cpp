#include "precompiled.h"
#pragma hdrstop

bool Client::CL_CheckOrDownloadFile( const Str & filename ) {
	int nameLen = filename.Length( );
	if( filename.Find( ".." ) != -1 ) {
		Common::Com_Printf( "Refusing to download a path with ..\n" );
		return true;
	}
	if( FileSystem::TouchFile( filename ) ) // it exists, no need to download
		return true;
	cls.downloadname = filename;
	// download to a temp name, and only rename
	// to the real name when done, so if interrupted
	// a runt file wont be left
	cls.downloadtempname = filename;
	cls.downloadtempname.Append( ".tmp" );
	// check to see if we already have a tmp for this file, if so, try to resume
	// open the file if not opened yet
	cl_messageBufferReliable.WriteByte( clc_stringcmd );
	if( FileSystem::TouchFile( cls.downloadtempname ) ) { // it exists
		cls.download = FileSystem::OpenFileByMode( cls.downloadtempname, FS_APPEND );
		int len = ( int )cls.download->Length( );
		// give the server an offset to start the download
		Common::Com_Printf( "Resuming %s\n", cls.downloadname.c_str( ) );
		cl_messageBufferReliable.WriteString( va( "download %s %i", cls.downloadname.c_str( ), len ) );
	} else {
		//cls.download = FileSystem::OpenFileByMode( cls.downloadtempname, FS_WRITE );
		Common::Com_Printf( "Downloading %s\n", cls.downloadname.c_str( ) );
		cl_messageBufferReliable.WriteString( va( "download %s", cls.downloadname.c_str( ) ) );
	}
	cls.downloadnumber++;
	return false;
}

void Client::CL_Download_f( ) {
	Str filename;
	if( Command::Cmd_Argc( ) != 2 ) {
		Common::Com_Printf( "Usage: download <filename>\n" );
		return;
	}
	sprintf( filename, "%s", Command::Cmd_Argv( 1 ).c_str( ) );
	if( filename == ".." ) {
		Common::Com_Printf( "Refusing to download a path with ..\n" );
		return;
	}
	if( FileSystem::TouchFile( filename ) ) { // it exists, no need to download
		Common::Com_Printf( "File already exists.\n" );
		return;
	}
	cls.downloadname = filename;
	Common::Com_Printf( "Downloading %s\n", cls.downloadname.c_str( ) );
	// download to a temp name, and only rename
	// to the real name when done, so if interrupted
	// a runt file wont be left
	//COM_StripExtension( cls.downloadname, cls.downloadtempname ); // FIXME
	cls.downloadtempname += ".tmp";
	cl_messageBufferReliable.WriteByte( clc_stringcmd );
	cl_messageBufferReliable.WriteString( va( "download %s %i", cls.downloadname.c_str( ) ) );
	cls.downloadnumber++;
}

void Client::CL_RegisterSounds( ) {
	//Sound::S_BeginRegistration( );
	SoundSystem::BeginRegistration( );
	CL_RegisterTEntSounds( );
	for( int i = 1; i < MAX_SOUNDS; i++ ) {
		if( cl.configstrings[ CS_SOUNDS + i ].IsEmpty( ) )
			break;
		//cl.sound_precache[ i ] = Sound::S_RegisterSound( cl.configstrings[ CS_SOUNDS + i ] );
		cl.sound_precache[ i ] = SoundSystem::RegisterSample( cl.configstrings[ CS_SOUNDS + i ] );
	}
	//Sound::S_EndRegistration( );
	SoundSystem::EndRegistration( );
}

void Client::CL_ParseDownload( MessageBuffer & msg_buffer ) {
	Str name;
	int r;
	// read the data
	int size = msg_buffer.ReadShort( );
	int percent = msg_buffer.ReadByte( );
	if( size == -1 ) {
		Common::Com_Printf( "Server does not have this file.\n" );
		if( cls.download ) // if here, we tried to resume a file but the server said no
			FileSystem::CloseFile( cls.download );
		CL_RequestNextDownload( );
		return;
	}
	// open the file if not opened yet
	if( !cls.download ) {
		name = cls.downloadtempname;
		cls.download = FileSystem::OpenFileByMode( name, FS_WRITE );
	}
	cls.download->Write( msg_buffer.GetData( ) + msg_buffer.GetReadCount( ), size );
	msg_buffer.SetReadCount( msg_buffer.GetReadCount( ) + size );
	if( percent != 100 ) {
		// request next block
		// change display routines by zoid
		cls.downloadpercent = percent;
		cl_messageBufferReliable.WriteByte( clc_stringcmd );
		cl_messageBufferReliable.WriteString( "nextdl" );
	} else {
		Str oldn;
		Str newn;
		//		Common::Com_Printf( "100%%\n" );
		FileSystem::CloseFile( cls.download );
		// rename the temp file to it's final name
		oldn = cls.downloadtempname;
		newn = cls.downloadname;
		r = rename( oldn, newn );
		if( r )
			Common::Com_Printf( "failed to rename.\n" );
		cls.downloadpercent = 0;
		// get another file if needed
		CL_RequestNextDownload( );
	}
}

void Client::CL_ParseServerData( MessageBuffer & msg_buffer ) {
	Common::Com_DPrintf( "Serverdata packet received.\n" );
	// wipe the client_state_t struct
	CL_ClearState( );
	cls.state = ca_connected;
	// parse protocol version number
	cls.serverProtocol = msg_buffer.ReadByte( );
	cl.servercount = msg_buffer.ReadLong( );
	cl.attractloop = msg_buffer.ReadByte( ) != 0;
	// game directory
	msg_buffer.ReadString( cl.gamedir );
	// set gamedir
	//if( ( str.Length( ) &&( !fs_gamedirvar->string || !* fs_gamedirvar->string || strcmp( fs_gamedirvar->string, str ) ) ) ||( !* str &&( fs_gamedirvar->string || * fs_gamedirvar->string ) ) )
	//	Cvar_Set( "game", str ); // FIXME LATER
	// parse player entity number
	cl.playernum = msg_buffer.ReadShort( );
	// get the full level name
	Str str;
	msg_buffer.ReadString( str );
	if( cl.playernum != -1 ) {
		// seperate the printfs so the server message can have a color
		Common::Com_Printf( "\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n" );
		Common::Com_Printf( "%c%s\n", 2, str.c_str( ) );
		// need to prep refresh at next oportunity
		cl.refresh_prepped = false;
	}
}

void Client::CL_ParseBaseline( MessageBuffer & msg_buffer ) {
	entity_state_t * es;
	uint bits;
	int newnum;
	entity_state_t nullstate;
	nullstate.Clear( );
	newnum = CL_ParseEntityBits( msg_buffer, &bits );
	es = &cl_entities[ newnum ].baseline;
	CL_ParseDelta( msg_buffer, &nullstate, es, newnum, bits );
}

void Client::CL_LoadClientinfo( clientinfo_t * ci, const Str & s ) {
	Str model_filename;
	Str skin_filename;
	Str weapon_filename;
	ci->cinfo = s;
	// isolate the player's name
	ci->name = ClientServerInfo::Info_ValueForKey( s, "name" );
	if( cl_noskins.GetBool( ) || !s.Length( ) ) {
		sprintf( model_filename, "players/male/tris.md2" );
		sprintf( weapon_filename, "players/male/weapon.md2" );
		sprintf( skin_filename, "players/male/grunt.pcx" );
		sprintf( ci->iconname, "players/male/grunt_i.pcx" );
	} else {		
		model_filename = ClientServerInfo::Info_ValueForKey( s, "model" );
		weapon_filename = ClientServerInfo::Info_ValueForKey( s, "weapon" );
		skin_filename = ClientServerInfo::Info_ValueForKey( s, "skin" );
		ci->iconname = ClientServerInfo::Info_ValueForKey( s, "icon" );
	}
	//ci->model = Renderer::RegisterModel( model_filename );
	ci->weaponmodel.SetGranularity( );
	ci->weaponmodel.Append( NULL );
	//ci->weaponmodel.Append( Renderer::RegisterModel( weapon_filename ) );
	//ci->skin = MaterialSystem::FindTexture( skin_filename, "model" );
	//ci->icon = MaterialSystem::FindTexture( ci->iconname, "hud" );
	// must have loaded all data types to be valud
	if( !ci->skin || !ci->icon || !ci->model || !ci->weaponmodel[ 0 ] ) {
		ci->skin = NULL;
		ci->icon = NULL;
		ci->model = NULL;
		ci->weaponmodel[ 0 ] = NULL;
		return;
	}
}

void Client::CL_ParseClientinfo( int player ) {
	Str s;
	clientinfo_t * ci;
	s = cl.configstrings[ CS_PLAYERSKINS + player ];
	ci = &cl.clientinfo[ player ];
	CL_LoadClientinfo( ci, s );
}

void Client::CL_ParseConfigString( MessageBuffer & msg_buffer ) {
	int i;
	Str s;
	Str olds;
	i = msg_buffer.ReadShort( );
	if( i < 0 || i >= MAX_CONFIGSTRINGS )
		Common::Com_Error( ERR_DROP, "configstring > MAX_CONFIGSTRINGS" );
	msg_buffer.ReadString( s );
	olds = cl.configstrings[ i ];
	cl.configstrings[ i ] = s;
	// do something apropriate 
	if( i >= CS_LIGHTS && i < CS_LIGHTS + MAX_LIGHTSTYLES ) {
		CL_SetLightstyle( i - CS_LIGHTS );
	} else if( i >= CS_MODELS && i < CS_MODELS+MAX_MODELS && cl.refresh_prepped ) {
		cl.model_draw[ i - CS_MODELS ] = Renderer::RegisterModel( cl.configstrings[ i ] );
		if( cl.configstrings[ i ][ 0 ] == '*' )
			cl.model_clip[ i-CS_MODELS ] = CollisionModel::CM_InlineModel( cl.configstrings[ i ] );
		else
			cl.model_clip[ i - CS_MODELS ] = NULL;
	} else if( i >= CS_SOUNDS && i < CS_SOUNDS+MAX_MODELS && cl.refresh_prepped )
		cl.sound_precache[ i - CS_SOUNDS ] = SoundSystem::RegisterSample( cl.configstrings[ i ] );
		//cl.sound_precache[ i - CS_SOUNDS ] = Sound::S_RegisterSound( cl.configstrings[ i ] );
	else if( i >= CS_IMAGES && i < CS_IMAGES + MAX_MODELS && cl.refresh_prepped )
		cl.image_precache[ i - CS_IMAGES ] = MaterialSystem::FindTexture( cl.configstrings[ i ], "hud" );
	else if( i >= CS_PLAYERSKINS && i < CS_PLAYERSKINS + MAX_CLIENTS && cl.refresh_prepped && strcmp( olds, s ) )
		CL_ParseClientinfo( i - CS_PLAYERSKINS );
}

void Client::CL_ParseStartSoundPacket( MessageBuffer & msg_buffer ) {
	Vec3 pos;
	int channel, ent;
	int sound_num;
	float volume;
	float attenuation;
	int flags;
	float ofs;
	flags = msg_buffer.ReadByte( );
	sound_num = msg_buffer.ReadByte( );
	if( flags & SND_VOLUME )
		volume = msg_buffer.ReadByte( ) / 255.0f;
	else
		volume = DEFAULT_SOUND_PACKET_VOLUME;
	if( flags & SND_ATTENUATION )
		attenuation = msg_buffer.ReadByte( ) / 64.0f;
	else
		attenuation = DEFAULT_SOUND_PACKET_ATTENUATION;
	if( flags & SND_OFFSET )
		ofs = msg_buffer.ReadByte( ) / 1000.0f;
	else
		ofs = 0;
	if( flags & SND_ENT ) { // entity reletive
		channel = msg_buffer.ReadShort( );
		ent = channel>>3;
		if( ent > MAX_EDICTS )
			Common::Com_Error( ERR_DROP, "CL_ParseStartSoundPacket: ent = %i", ent );
		channel &= 7;
	} else {
		ent = 0;
		channel = 0;
	}
	if( flags & SND_POS )
		msg_buffer.ReadVec3( pos );
	if( !cl.sound_precache[ sound_num ] )
		return;
	//Sound::S_StartSound( ( flags & SND_POS ) ? &pos : NULL, ent, channel, cl.sound_precache[ sound_num ], volume, attenuation, ofs );
	if( flags & SND_ENT )
		SoundSystem::PlaySampleOnEnt( cl.sound_precache[ sound_num ], ent, volume, attenuation, ofs );
	else if( flags & SND_POS )
		SoundSystem::PlaySampleOnPos( cl.sound_precache[ sound_num ], pos, volume, attenuation, ofs );
	else
		SoundSystem::PlaySampleOnLocal( cl.sound_precache[ sound_num ], volume, attenuation, ofs );
}

void Client::SHOWNET( MessageBuffer & msg_buffer, const Str & str ) {
	if( cl_shownet.GetInt( ) >= 2 )
		Common::Com_Printf( "%3i:%s\n", msg_buffer.GetReadCount( ) - 1, str.c_str( ) );
}

void Client::CL_ParseServerMessage( MessageBuffer & msg_buffer ) {
	int cmd;
	Str s;
	int i;
	// if recording demos, copy the message out
	if( cl_shownet.GetInt( ) == 1 )
		Common::Com_Printf( "%i ", msg_buffer.GetSize( ) );
	else if( cl_shownet.GetInt( ) >= 2 )
		Common::Com_Printf( "------------------\n" );
	// parse the message
	while( 1 ) {
		if( msg_buffer.GetReadCount( ) > msg_buffer.GetSize( ) ) {
			Common::Com_Error( ERR_DROP, "CL_ParseServerMessage: Bad server message" );
			break;
		}
		if( !msg_buffer.GetRemaingData( ) ) {
			SHOWNET( msg_buffer, "END OF MESSAGE" );
			break;
		}
		cmd = msg_buffer.ReadByte( );
		if( cl_shownet.GetInt( ) >= 2 ) {
			if( !svc_strings[ cmd ] )
				Common::Com_Printf( "%3i:BAD CMD %i\n", msg_buffer.GetReadCount( ) - 1, cmd );
			else
				SHOWNET( msg_buffer, svc_strings[ cmd ] );
		}
		// other commands
		switch( cmd ) {
			default:
				Common::Com_Error( ERR_DROP, "CL_ParseServerMessage: Illegible server message\n" );
				break;
			case svc_nop:
				//			Common::Com_Printf( "svc_nop\n" );
				break;
			case svc_disconnect:
				Common::Com_Error( ERR_DISCONNECT, "Server disconnected\n" );
				break;
			case svc_reconnect:
				Common::Com_Printf( "Server disconnected, reconnecting\n" );
				if( cls.download ) {
					//ZOID, close download
					FileSystem::CloseFile( cls.download );
				}
				cls.state = ca_challenge;
				cls.connect_time = -99999; // CL_CheckForResend( ) will fire immediately
				break;
			case svc_print:
				i = msg_buffer.ReadByte( ); // level
				if( i == PRINT_CHAT ) {
					//Sound::S_StartLocalSound( Str( "misc/talk.wav" ) );
					SoundSystem::PlaySampleOnLocal( "misc/talk.wav" );
					//con.ormask = 128;
				}
				msg_buffer.ReadString( s );
				Common::Com_Printf( "%s", s.c_str( ) );
				//con.ormask = 0;
				break;
			case svc_centerprint:
				msg_buffer.ReadString( s );
				Screen::SCR_CenterPrint( s.c_str( ) );
				break;
			case svc_stufftext:
				msg_buffer.ReadString( s );
				Common::Com_DPrintf( "stufftext: %s\n", s.c_str( ) );
				CBuffer::Cbuf_AddText( s );
				break;
			case svc_serverdata:
				CBuffer::Cbuf_Execute( ); // make sure any stuffed commands are done
				CL_ParseServerData( msg_buffer );
				break;
			case svc_configstring:
				CL_ParseConfigString( msg_buffer );
				break;
			case svc_sound:
				CL_ParseStartSoundPacket( msg_buffer );
				break;
			case svc_spawnbaseline:
				CL_ParseBaseline( msg_buffer );
				break;
			case svc_temp_entity:
				CL_ParseTEnt( msg_buffer );
				break;
			case svc_muzzleflash:
				CL_ParseMuzzleFlash( msg_buffer );
				break;
			case svc_muzzleflash2:
				CL_ParseMuzzleFlash2( msg_buffer );
				break;
			case svc_download:
				CL_ParseDownload( msg_buffer );
				break;
			case svc_frame:
				CL_ParseFrame( msg_buffer );
				break;
			case svc_inventory:
				CL_ParseInventory( msg_buffer );
				break;
			case svc_layout:
				msg_buffer.ReadString( cl.layout );
				break;
			case svc_playerinfo:
			case svc_packetentities:
			case svc_deltapacketentities:
				Common::Com_Error( ERR_DROP, "Out of place frame data" );
				break;
		}
	}
	CL_AddNetgraph( );
	//
	// we don't know if it is ok to save a demo message until
	// after we have parsed the frame
	//
	if( cls.demorecording && !cls.demowaiting )
		CL_WriteDemoMessage( msg_buffer );
}
