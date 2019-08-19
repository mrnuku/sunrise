#include "precompiled.h"
#pragma hdrstop

void Server::PF_Unicast( Entity * ent, bool reliable ) {
	int		p;
	client_t	* client;
	if( !ent )
		return;
	p = NUM_FOR_EDICT( ent );
	if( p < 1 || p > Common::maxclients.GetInt( ) )
		return;
	client = svs.clients + ( p - 1 );
	if( reliable )
		client->d_clientBufferReliable.WriteData( sv.d_multicastBuffer.GetData( ), sv.d_multicastBuffer.GetSize( ) );
	else
		client->d_clientBuffer.WriteData( sv.d_multicastBuffer.GetData( ), sv.d_multicastBuffer.GetSize( ) );
	sv.d_multicastBuffer.BeginWriting( );
}

void Server::PF_cprintf( Entity * ent, int level, const char * fmt, ... ) {
	char		msg[ 1024 ];
	va_list		argptr;
	int			n;
	if( ent ) {
		n = NUM_FOR_EDICT( ent );
		if( n < 1 || n > Common::maxclients.GetInt( ) )
			Common::Com_Error( ERR_DROP, "cprintf to a non-client" );
	}
	va_start( argptr, fmt );
	vsprintf( msg, fmt, argptr );
	va_end( argptr );
	if( ent )
		SV_ClientPrintf( svs.clients + ( n - 1 ), level, "%s", msg );
	else
		Common::Com_Printf( "%s", msg );
}

void Server::PF_centerprintf( Entity * ent, const char * fmt, ... ) {
	char		msg[ 1024 ];
	va_list		argptr;
	int			n;	
	n = NUM_FOR_EDICT( ent );
	if( n < 1 || n > Common::maxclients.GetInt( ) )
		return;	// Common::Com_Error( ERR_DROP, "centerprintf to a non-client" );
	va_start( argptr, fmt );
	vsprintf( msg, fmt, argptr );
	va_end( argptr );
	sv.d_multicastBuffer.WriteByte( svc_centerprint );
	sv.d_multicastBuffer.WriteString( msg );
	PF_Unicast( ent, true );
}

void Server::PF_error( const char * fmt, ... ) {
	char		msg[ 1024 ];
	va_list		argptr;	
	va_start( argptr, fmt );
	vsprintf( msg, fmt, argptr );
	va_end( argptr );
	Common::Com_Error( ERR_DROP, "Game Error: %s", msg );
}

void Server::PF_setmodel( Entity * ent, const Str & name ) {
	int		i;
	CModel *	mod;
	if( name.IsEmpty( ) )
		Common::Com_Error( ERR_DROP, "PF_setmodel: NULL" );
	i = SV_ModelIndex( name );		
//	ent->model = name;
	ent->s.modelindex = i;
// if it is an INLINE model, get the size information for it
	if( name[ 0 ] == '*' ) {
		mod = CollisionModel::CM_InlineModel( name );
		ent->mins = mod->bounds[ 0 ];
		ent->maxs = mod->bounds[ 1 ];
		SV_LinkEdict( ent );
	}
}

void Server::PF_Configstring( int index, const Str & val ) {
	if( index < 0 || index >= MAX_CONFIGSTRINGS )
		Common::Com_Error( ERR_DROP, "configstring: bad index %i\n", index );
	// change the string in sv
	sv.configstrings[ index ] = val;
	sv.d_config.Set( SV_GetConfigName( index ), val );
	if( sv.state != ss_loading ) {	// send the update to everyone
		//Message::SZ_Clear( &sv.multicast );
		assert( !sv.d_multicastBuffer.GetSize( ) );
		sv.d_multicastBuffer.WriteChar( svc_configstring );
		sv.d_multicastBuffer.WriteShort( index );
		sv.d_multicastBuffer.WriteString( val );
		SV_Multicast( vec3_origin, MULTICAST_ALL_R );
	}
}

void Server::PF_WriteChar( int c ) {	
	sv.d_multicastBuffer.WriteChar( c );
}

void Server::PF_WriteByte( int c ) {	
	sv.d_multicastBuffer.WriteByte( c );
}

void Server::PF_WriteShort( int c ) {	
	sv.d_multicastBuffer.WriteShort( c );
}

void Server::PF_WriteLong( int c ) {	
	sv.d_multicastBuffer.WriteLong( c );
}

void Server::PF_WriteFloat( float f ) {	
	sv.d_multicastBuffer.WriteFloat( f );
}

void Server::PF_WriteString( const Str & s ) {	
	sv.d_multicastBuffer.WriteString( s );
}

void Server::PF_WritePos( Vec3 & pos ) {	
	sv.d_multicastBuffer.WriteVec3( pos );
}

void Server::PF_WriteDir( Vec3 & dir ) {	
	sv.d_multicastBuffer.WriteDir( dir, 24 );
}

void Server::PF_WriteAngle( float f ) {	
	sv.d_multicastBuffer.WriteFloat( f );
}

bool Server::PF_inPVS( Vec3 & p1, Vec3 & p2 ) {
	int leafnum = CollisionModel::CM_PointLeafnum( p1 );
	int cluster = CollisionModel::CM_LeafCluster( leafnum );
	int area1 = CollisionModel::CM_LeafArea( leafnum );
	byte * mask = CollisionModel::CM_ClusterPVS( cluster );
	leafnum = CollisionModel::CM_PointLeafnum( p2 );
	cluster = CollisionModel::CM_LeafCluster( leafnum );
	int area2 = CollisionModel::CM_LeafArea( leafnum );
	if( mask &&( !( mask[ cluster >> 3 ] &( 1 <<( cluster & 7 ) ) ) ) )
		return false;
	if( !CollisionModel::CM_AreasConnected( area1, area2 ) )
		return false;		// a door blocks sight
	return true;
}

bool Server::PF_inPHS( Vec3 & p1, Vec3 & p2 ) {
	int leafnum = CollisionModel::CM_PointLeafnum( p1 );
	int cluster = CollisionModel::CM_LeafCluster( leafnum );
	int area1 = CollisionModel::CM_LeafArea( leafnum );
	byte * mask = CollisionModel::CM_ClusterPHS( cluster );
	leafnum = CollisionModel::CM_PointLeafnum( p2 );
	cluster = CollisionModel::CM_LeafCluster( leafnum );
	int area2 = CollisionModel::CM_LeafArea( leafnum );
	if( mask &&( !( mask[ cluster >> 3 ] &( 1 <<( cluster & 7 ) ) ) ) )
		return false;		// more than one bounce away
	if( !CollisionModel::CM_AreasConnected( area1, area2 ) )
		return false;		// a door blocks hearing
	return true;
}

void Server::PF_StartSound( Entity * entity, int channel, int sound_num, float volume, float attenuation, float timeofs ) {
	if( !entity )
		return;
	SV_StartSound( NULL, entity, channel, sound_num, volume, attenuation, timeofs );
}

void Server::SV_ShutdownGameProgs( ) {
	Game::Shutdown( );
}

void Server::SV_InitGameProgs( ) {
	// unload anything we have now
	SV_ShutdownGameProgs( );
	if( !Game::Init( ) )
		Common::Com_Error( ERR_DROP, "failed to init game" );
}
