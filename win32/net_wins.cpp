#include "precompiled.h"
#pragma hdrstop

#define u_long				WindowsNS::u_long
#define u_int				WindowsNS::u_int
#define fd_set				WindowsNS::fd_set
#define WORD				WindowsNS::WORD
#define DWORD_PTR			WindowsNS::DWORD_PTR
#define BYTE				WindowsNS::BYTE
typedef unsigned long ULONG;

#define IPPROTO_TCP             6               /* tcp */
#define IPPROTO_UDP             17              /* user datagram protocol */

bool		Network::old_config = false;
loopback_t	Network::loopbacks[ 2 ];
SOCKET		Network::ip_sockets[ 2 ] = { 0, 0 };

//=============================================================================

void Network::NetadrToSockadr( const netadr_t * a, struct WindowsNS::sockaddr * s ) {
	Common::Com_Memset( s, 0, sizeof( struct WindowsNS::sockaddr ) );
	if( a->type == NA_BROADCAST ) {
		( ( struct WindowsNS::sockaddr_in * )s )->sin_family = AF_INET;
		( ( struct WindowsNS::sockaddr_in * )s )->sin_port = a->port;
		( ( struct WindowsNS::sockaddr_in * )s )->sin_addr.s_addr = INADDR_BROADCAST;
	} else if( a->type == NA_IP ) {
		( ( struct WindowsNS::sockaddr_in * )s )->sin_family = AF_INET;
		( ( struct WindowsNS::sockaddr_in * )s )->sin_addr.s_addr = * ( int * )&a->ip;
		( ( struct WindowsNS::sockaddr_in * )s )->sin_port = a->port;
	}
}

void Network::SockadrToNetadr( struct WindowsNS::sockaddr * s, netadr_t * a ) {
	if( s->sa_family == AF_INET ) {
		a->type = NA_IP;
		* ( int * )&a->ip =( ( struct WindowsNS::sockaddr_in * )s )->sin_addr.s_addr;
		a->port =( ( struct WindowsNS::sockaddr_in * )s )->sin_port;
	}
}

bool Network::NET_CompareAdr( const netadr_t a, const netadr_t b ) {
	if( a.type != b.type )
		return false;
	if( a.type == NA_LOOPBACK )
		return true;
	if( a.type == NA_IP ) {
		if( a.ip[ 0 ] == b.ip[ 0 ] && a.ip[ 1 ] == b.ip[ 1 ] && a.ip[ 2 ] == b.ip[ 2 ] && a.ip[ 3 ] == b.ip[ 3 ] && a.port == b.port )
			return true;
		return false;
	}
	return false;
}

bool Network::NET_CompareBaseAdr( const netadr_t a, const netadr_t b ) {
	if( a.type != b.type )
		return false;
	if( a.type == NA_LOOPBACK )
		return true;
	if( a.type == NA_IP ) {
		if( a.ip[ 0 ] == b.ip[ 0 ] && a.ip[ 1 ] == b.ip[ 1 ] && a.ip[ 2 ] == b.ip[ 2 ] && a.ip[ 3 ] == b.ip[ 3 ] )
			return true;
		return false;
	}
	return false;
}

const Str Network::NET_AdrToString( netadr_t a ) {
	Str s;
	if( a.type == NA_LOOPBACK )
		sprintf( s, "loopback" );
	else if( a.type == NA_IP )
		sprintf( s, "%i.%i.%i.%i:%i", a.ip[ 0 ], a.ip[ 1 ], a.ip[ 2 ], a.ip[ 3 ], WindowsNS::ntohs( a.port ) );
	return s;
}

bool Network::NET_StringToSockaddr( const Str & string, struct WindowsNS::sockaddr * sadr ) {

	struct WindowsNS::hostent * host;
	unsigned long ulongAddress;
	bool isNumericIP = string[ 0 ] >= '0' && string[ 0 ] <= '9';
	bool containsPort = string.Find( ':' ) != -1;
	Str address = containsPort ? string.Left( string.Find( ':' ) ) : string;
	if( !isNumericIP ) {		
		host = WindowsNS::gethostbyname( address );
		if( !host )
			return false;
	} else {
		ulongAddress = WindowsNS::inet_addr( address );
		if( ulongAddress == INADDR_NONE )
			return false;
	}
	Common::Com_Memset( sadr, 0, sizeof( struct WindowsNS::sockaddr ) );
	( ( struct WindowsNS::sockaddr_in * )sadr )->sin_family = AF_INET;
	*( unsigned long * )&( ( struct WindowsNS::sockaddr_in * )sadr )->sin_addr = isNumericIP ? ulongAddress : *( unsigned long * )host->h_addr_list[ 0 ];
	if( containsPort ) {
		Str portNumber = string.Right( string.Length( ) - ( string.Find( ':' ) + 1 ) );
		( ( struct WindowsNS::sockaddr_in * )sadr )->sin_port = WindowsNS::htons( atoi( portNumber ) );
	}
	return true;
}

#undef DO

bool Network::NET_StringToAdr( const Str & string, netadr_t * a ) {
	struct WindowsNS::sockaddr sadr;	
	if( 0 && string == "localhost" )	{
		Common::Com_Memset( a, 0, sizeof( netadr_t ) );
		a->type = NA_LOOPBACK;
		return true;
	}
	if( !NET_StringToSockaddr( string, &sadr ) )
		return false;	
	SockadrToNetadr( &sadr, a );
	return true;
}

bool Network::NET_IsLocalAddress( netadr_t adr ) {
	return adr.type == NA_LOOPBACK;
}

bool Network::NET_GetLoopPacket( const netsrc_t sock, netadr_t * net_from, sizebuf_t * net_message ) {
	loopback_t	* loop = &loopbacks[ sock ];
	if( loop->send - loop->get > MAX_LOOPBACK )
		loop->get = loop->send - MAX_LOOPBACK;
	if( loop->get >= loop->send )
		return false;
	size_t i = loop->get &( MAX_LOOPBACK-1 );
	loop->get++;
	Common::Com_Memcpy( net_message->data, loop->msgs[ i ].data, loop->msgs[ i ].datalen );
	net_message->cursize = loop->msgs[ i ].datalen;
	Common::Com_Memset( net_from, 0, sizeof( *net_from ) );
	net_from->type = NA_LOOPBACK;
	return true;

}

void Network::NET_SendLoopPacket( netsrc_t sock, size_t length, void * data, netadr_t to ) {
	loopback_t	* loop = &loopbacks[ sock ^ 1 ];
	int i = loop->send &( MAX_LOOPBACK - 1 );
	loop->send++;
	Common::Com_Memcpy( loop->msgs[ i ].data, data, length );
	loop->msgs[ i ].datalen = length;
}

bool Network::NET_GetPacket( netsrc_t sock, netadr_t * net_from, sizebuf_t * net_message ) {
	int 	ret;
	struct WindowsNS::sockaddr from;
	int		fromlen;
	intptr_t	net_socket;
	int		protocol;
	int		err;
	if( NET_GetLoopPacket( sock, net_from, net_message ) )
		return true;
	for( protocol = 0; protocol < 2; protocol++ ) {
		if( protocol == 0 )
			net_socket = ip_sockets[ sock ];
		if( !net_socket )
			continue;
		fromlen = sizeof( from );
		ret = recvfrom( net_socket, ( char * )net_message->data, ( int )net_message->maxsize, 0, &from, &fromlen );
		SockadrToNetadr( &from, net_from );
		if( ret == -1 ) {
			err = WindowsNS::WSAGetLastError( );
			if( err == WSAEWOULDBLOCK )
				continue;
			if( err == WSAEMSGSIZE ) {
				Common::Com_Printf( "Warning:  Oversize packet from %s\n", NET_AdrToString( *net_from ).c_str( ) );
				continue;
			}
			if( Common::dedicated.GetBool( ) )	// let dedicated servers continue after errors
				Common::Com_Printf( "NET_GetPacket: %s from %s\n", NET_ErrorString( ), NET_AdrToString( *net_from ).c_str( ) );
			else
				Common::Com_Error( ERR_DROP, "NET_GetPacket: %s from %s", NET_ErrorString( ), NET_AdrToString( *net_from ).c_str( ) );
			continue;
		}
		if( ret == net_message->maxsize ) {
			Common::Com_Printf( "Oversize packet from %s\n", NET_AdrToString( *net_from ).c_str( ) );
			continue;
		}
		net_message->cursize = ret;
		return true;
	}
	return false;
}

void Network::NET_SendPacket( netsrc_t sock, size_t length, void * data, netadr_t to ) {	
	if( to.type == NA_LOOPBACK ) {
		NET_SendLoopPacket( sock, length, data, to );
		return;
	}
	SOCKET net_socket = ip_sockets[ sock ];
	if( !net_socket )
		return;
	struct WindowsNS::sockaddr	addr;
	NetadrToSockadr( &to, &addr );
	int ret = sendto( net_socket, ( char * )data, ( int )length, 0, &addr, sizeof( addr ) );
	if( ret == -1 ) {
		int err = WindowsNS::WSAGetLastError( );
		// wouldblock is silent
		if( err == WSAEWOULDBLOCK )
			return;
		// some PPP links dont allow broadcasts
		if( ( err == WSAEADDRNOTAVAIL ) &&( ( to.type == NA_BROADCAST ) ) )
			return;
		if( Common::dedicated.GetBool( ) ) {	// let dedicated servers continue after errors
			Common::Com_Printf( "NET_SendPacket ERROR: %s to %s\n", NET_ErrorString( ), NET_AdrToString( to ).c_str( ) );
		} else {
			if( err == WSAEADDRNOTAVAIL )
				Common::Com_DPrintf( "NET_SendPacket Warning: %s : %s\n", NET_ErrorString( ), NET_AdrToString( to ).c_str( ) );
			else
				Common::Com_Error( ERR_DROP, "NET_SendPacket ERROR: %s to %s\n", NET_ErrorString( ), NET_AdrToString( to ).c_str( ) );
		}
	}
}

SOCKET Network::NET_IPSocket( const Str & net_interface, int port, netadr_t * bound_to ) {
	SOCKET	newsocket;
	if( ( newsocket = WindowsNS::socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) == INVALID_SOCKET ) {
		int err = WindowsNS::WSAGetLastError( );
		if( err != WSAEAFNOSUPPORT )
			Common::Com_Printf( "WARNING: UDP_OpenSocket: socket: %s", NET_ErrorString( ) );
		return 0;
	}
	u_long i = 1;
	// make it non-blocking
	if( WindowsNS::ioctlsocket( newsocket, FIONBIO, &i ) == -1 ) {
		Common::Com_Printf( "WARNING: UDP_OpenSocket: ioctl FIONBIO: %s\n", NET_ErrorString( ) );
		return 0;
	}
	// make it broadcast capable
	if( WindowsNS::setsockopt( newsocket, SOL_SOCKET, SO_BROADCAST, ( char * )&i, sizeof( u_long ) ) == -1 ) {
		Common::Com_Printf( "WARNING: UDP_OpenSocket: setsockopt SO_BROADCAST: %s\n", NET_ErrorString( ) );
		return 0;
	}
	struct WindowsNS::sockaddr_in	address;
	if( 0 && ( !net_interface || !net_interface[ 0 ] || !Str::Icmp( net_interface, "localhost" ) ) )
		address.sin_addr.s_addr = INADDR_ANY;
	else
		NET_StringToSockaddr( net_interface, ( struct WindowsNS::sockaddr * )&address );
	address.sin_port = port == PORT_ANY ? 0 : WindowsNS::htons( ( short )port );
	address.sin_family = AF_INET;
	if( WindowsNS::bind( newsocket, ( WindowsNS::sockaddr * )&address, sizeof( address ) ) == -1 ) {
		Common::Com_Printf( "WARNING: UDP_OpenSocket: bind: %s\n", NET_ErrorString( ) );
		WindowsNS::closesocket( newsocket );
		return 0;
	}
	if( bound_to ) {
		int len = sizeof( address );
		WindowsNS::getsockname( newsocket, ( WindowsNS::sockaddr * )&address, &len );
		SockadrToNetadr( ( WindowsNS::sockaddr * )&address, bound_to );
	}
	return newsocket;
}

void Network::NET_OpenIP( ) {
	if( !ip_sockets[ NS_SERVER ] ) {
		int port = net_hostport.GetInt( );
		if( !port )
			port = PORT_SERVER;
		ip_sockets[ NS_SERVER ] = NET_IPSocket( net_ip.GetString( ), port, NULL );
		if( !ip_sockets[ NS_SERVER ] && Common::dedicated.GetBool( ) )
			Common::Com_Error( ERR_FATAL, "Couldn't allocate dedicated server IP port" );
	}
	// dedicated servers don't need client ports
	if( Common::dedicated.GetBool( ) )
		return;
	if( !ip_sockets[ NS_CLIENT ] ) {
		int port = net_clientport.GetInt( );
		if( !port )
			port = PORT_CLIENT;
		ip_sockets[ NS_CLIENT ] = NET_IPSocket( net_ip.GetString( ), port, NULL );
		if( !ip_sockets[ NS_CLIENT ] )
			ip_sockets[ NS_CLIENT ] = NET_IPSocket( net_ip.GetString( ), PORT_ANY, NULL );
	}
}

void Network::NET_Config( bool multiplayer ) {
	if( old_config == multiplayer )
		return;
	old_config = multiplayer;
	if( !multiplayer ) {	// shut down any existing sockets
		for( int i = 0; i < 2; i++ ) {
			if( ip_sockets[ i ] ) {
				WindowsNS::closesocket( ip_sockets[ i ] );
				ip_sockets[ i ] = 0;
			}
		}
	} else {	// open sockets
		if( !net_noudp.GetBool( ) )
			NET_OpenIP( );
	}
}

void Network::NET_Sleep( int msec ) {
	WindowsNS::timeval timeout;
	fd_set	fdset;
	if( !Common::dedicated.GetBool( ) )
		return; // we're not a server, just run full speed
	FD_ZERO( &fdset );
	if( ip_sockets[ NS_SERVER ] )
		FD_SET( ip_sockets[ NS_SERVER ], &fdset ); // network socket
	timeout.tv_sec = msec/1000;
	timeout.tv_usec = ( msec % 1000 ) * 1000;
	select( 0, &fdset, NULL, NULL, &timeout );
}

void Network::NET_Init( ) {
	WindowsNS::WSADATA wsaData;
	int r = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
	if( r || LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 )
		Common::Com_Error( ERR_FATAL, "Winsock initialization failed." );
	//Common::Com_Printf( "Winsock Initialized\n" );
}

void Network::NET_Shutdown( ) {
	NET_Config( false );	// close sockets
	WindowsNS::WSACleanup( );
}

const char * Network::NET_ErrorString( ) {

	switch( WindowsNS::WSAGetLastError( ) ) {

	case WSAEINTR: return "wsaeintr";
	case WSAEBADF: return "wsaebadf";
	case WSAEACCES: return "wsaeacces";
	case WSAEDISCON: return "wsaediscon";
	case WSAEFAULT: return "wsaefault";
	case WSAEINVAL: return "wsaeinval";
	case WSAEMFILE: return "wsaemfile";
	case WSAEWOULDBLOCK: return "wsaewouldblock";
	case WSAEINPROGRESS: return "wsaeinprogress";
	case WSAEALREADY: return "wsaealready";
	case WSAENOTSOCK: return "wsaenotsock";
	case WSAEDESTADDRREQ: return "wsaedestaddrreq";
	case WSAEMSGSIZE: return "wsaemsgsize";
	case WSAEPROTOTYPE: return "wsaeprototype";
	case WSAENOPROTOOPT: return "wsaenoprotoopt";
	case WSAEPROTONOSUPPORT: return "WSAEPROTONOSUPPORT";
	case WSAESOCKTNOSUPPORT: return "wsaesocktnosupport";
	case WSAEOPNOTSUPP: return "wsaeopnotsupp";
	case WSAEPFNOSUPPORT: return "wsaepfnosupport";
	case WSAEAFNOSUPPORT: return "wsaeafnosupport";
	case WSAEADDRINUSE: return "wsaeaddrinuse";
	case WSAEADDRNOTAVAIL: return "wsaeaddrnotavail";
	case WSAENETDOWN: return "wsaenetdown";
	case WSAENETUNREACH: return "wsaenetunreach";
	case WSAENETRESET: return "wsaenetreset";
	case WSAECONNABORTED: return "wswsaeconnabortedaeintr";
	case WSAECONNRESET: return "wsaeconnreset";
	case WSAENOBUFS: return "wsaenobufs";
	case WSAEISCONN: return "wsaeisconn";
	case WSAENOTCONN: return "wsaenotconn";
	case WSAESHUTDOWN: return "wsaeshutdown";
	case WSAETOOMANYREFS: return "wsaetoomanyrefs";
	case WSAETIMEDOUT: return "wsaetimedout";
	case WSAECONNREFUSED: return "wsaeconnrefused";
	case WSAELOOP: return "wsaeloop";
	case WSAENAMETOOLONG: return "wsaenametoolong";
	case WSAEHOSTDOWN: return "wsaehostdown";
	case WSASYSNOTREADY: return "wsasysnotready";
	case WSAVERNOTSUPPORTED: return "wsavernotsupported";
	case WSANOTINITIALISED: return "wsanotinitialised";
	case WSAHOST_NOT_FOUND: return "wsahost_not_found";
	case WSATRY_AGAIN: return "wsatry_again";
	case WSANO_RECOVERY: return "wsano_recovery";
	case WSANO_DATA: return "wsano_data";
	}
	return "no error";
}

bool Network::Net_GetUDPPacket( SOCKET netSocket, netadr_t & net_from, char * data, int & size, int maxSize ) {
	int 			ret;
	struct WindowsNS::sockaddr	from;
	int				fromlen;
	int				err;
	if( !netSocket )
		return false;
	fromlen = sizeof( from );
	ret =  WindowsNS::recvfrom( netSocket, data, maxSize, 0, ( struct WindowsNS::sockaddr * )&from, &fromlen );
	if( ret == SOCKET_ERROR ) {
		err = WindowsNS::WSAGetLastError( );
		if( err == WSAEWOULDBLOCK || err == WSAECONNRESET )
			return false;
		Common::Com_DPrintf( "Net_GetUDPPacket: %s\n", NET_ErrorString( ) );
		return false;
	}
	SockadrToNetadr( &from, &net_from );
	if( ret == maxSize ) {
		Common::Com_DPrintf( "Net_GetUDPPacket: oversize packet from %s\n", NET_AdrToString( net_from ).c_str( ) );
		return false;
	}
	size = ret;
	return true;
}

void Network::Net_SendUDPPacket( SOCKET netSocket, int length, const void * data, const netadr_t to ) {
	int				ret;
	struct WindowsNS::sockaddr	addr;
	if( !netSocket )
		return;
	NetadrToSockadr( &to, &addr );
	ret = WindowsNS::sendto( netSocket, ( const char * )data, length, 0, &addr, sizeof( addr ) );
	if( ret == SOCKET_ERROR ) {
		int err = WindowsNS::WSAGetLastError( );
		// wouldblock is silent
		if( err == WSAEWOULDBLOCK )
			return;
		// some PPP links do not allow broadcasts and return an error
		if( ( err == WSAEADDRNOTAVAIL ) && ( to.type == NA_BROADCAST ) )
			return;
		Common::Com_DPrintf( "Net_SendUDPPacket: %s\n", NET_ErrorString( ) );
	}
}

bool Network::Net_WaitForUDPPacket( SOCKET netSocket, int timeout ) {
	int					ret;
	fd_set				set;
	struct WindowsNS::timeval		tv;
	if( !netSocket )
		return false;
	if( timeout <= 0 )
		return true;
	FD_ZERO( &set );
	FD_SET( netSocket, &set );
	tv.tv_sec = 0;
	tv.tv_usec = timeout * 1000;
	ret = WindowsNS::select( ( int )netSocket + 1, &set, NULL, NULL, &tv );
	if( ret == -1 ) {
		Common::Com_DPrintf( "Net_WaitForUPDPacket select(): %s\n", strerror( errno ) );
		return false;
	}
	// timeout with no data
	if( ret == 0 )
		return false;
	return true;
}

NetworkPort::NetworkPort ( ) {
	d_socket = 0;
	Common::Com_Memset( &d_boundTarget, 0, sizeof( netadr_t ) );
}

NetworkPort::~NetworkPort( ) {
	Close( );
}

bool NetworkPort::InitForPort( int portNumber ) {
	int len = sizeof( struct WindowsNS::sockaddr_in );
	d_socket = Network::NET_IPSocket( Network::net_ip.GetString( ), portNumber, &d_boundTarget );
	if( d_socket <= 0 ) {
		d_socket = 0;
		Common::Com_Memset( &d_boundTarget, 0, sizeof( netadr_t ) );
		return false;
	}
	//udpPorts[ bound_to.port ] = new idUDPLag;
	return true;
}

void NetworkPort::Close( ) {
	if( d_socket ) {
		//if( udpPorts[ bound_to.port ] ) {
		//	delete udpPorts[ bound_to.port ];
		//	udpPorts[ bound_to.port ] = NULL;
		//}
		WindowsNS::closesocket( d_socket );
		d_socket = 0;
		Common::Com_Memset( &d_boundTarget, 0, sizeof( netadr_t ) );
	}
}

bool NetworkPort::GetPacket( netadr_t & from, void * data, int & size, int maxSize ) {
	udpMsg_t * msg;
	bool ret;
	while( 1 ) {
		ret = Network::Net_GetUDPPacket( d_socket, from, ( char * )data, size, maxSize );
		if( !ret )
			break;
		if( Network::net_forceDrop.GetInt( ) > 0 ) {
			if( rand( ) < Network::net_forceDrop.GetInt( ) * RAND_MAX / 100 )
				continue;
		}
		d_packetsRead++;
		d_bytesRead += size;
		if( Network::net_forceLatency.GetInt( ) > 0 ) {
			assert( size <= MAX_UDP_MSG_SIZE );
			msg = d_udpPort.d_udpMsgAllocator.Alloc( );
			Common::Com_Memcpy( msg->d_data, data, size );
			msg->d_size = size;
			msg->d_address = from;
			msg->d_time = System::Sys_LastTime( );
			msg->d_next = NULL;
			if( d_udpPort.d_recieveLast )
				d_udpPort.d_recieveLast->d_next = msg;
			else
				d_udpPort.d_recieveFirst = msg;
			d_udpPort.d_recieveLast = msg;
		} else
			break;
	}
	if( Network::net_forceLatency.GetInt( ) > 0 || d_udpPort.d_recieveFirst ) {
		msg = d_udpPort.d_recieveFirst;
		if( msg && msg->d_time <= System::Sys_LastTime( ) - Network::net_forceLatency.GetInt( ) ) {
			Common::Com_Memcpy( data, msg->d_data, msg->d_size );
			size = msg->d_size;
			from = msg->d_address;
			d_udpPort.d_recieveFirst = d_udpPort.d_recieveFirst->d_next;
			if( !d_udpPort.d_recieveFirst )
				d_udpPort.d_recieveLast = NULL;
			d_udpPort.d_udpMsgAllocator.Free( msg );
			return true;
		}
		return false;
	} else
		return ret;
}

bool NetworkPort::GetPacketBlocking( netadr_t & from, void * data, int & size, int maxSize, int timeout ) {
	Network::Net_WaitForUDPPacket( d_socket, timeout );
	if( GetPacket( from, data, size, maxSize ) )
		return true;
	return false;
}

void NetworkPort::SendPacket( const netadr_t to, const void *data, int size ) {
	udpMsg_t * msg;
	if( to.type == NA_BAD ) {
		Common::Com_DPrintf( "NetworkPort::SendPacket: bad address type NA_BAD - ignored" );
		return;
	}
	d_packetsWritten++;
	d_bytesWritten += size;
	if( Network::net_forceDrop.GetInt( ) > 0 ) {
		if( rand( ) <  Network::net_forceDrop.GetInt( ) * RAND_MAX / 100 )
			return;
	}
	if( Network::net_forceLatency.GetInt( ) > 0 || d_udpPort.d_sendFirst ) {
		assert( size <= MAX_UDP_MSG_SIZE );
		msg = d_udpPort.d_udpMsgAllocator.Alloc( );
		Common::Com_Memcpy( msg->d_data, data, size );
		msg->d_size = size;
		msg->d_address = to;
		msg->d_time = System::Sys_LastTime( );
		msg->d_next = NULL;
		if( d_udpPort.d_sendLast )
			d_udpPort.d_sendLast->d_next = msg;
		else
			d_udpPort.d_sendFirst = msg;
		d_udpPort.d_sendLast = msg;
		for( msg = d_udpPort.d_sendFirst; msg && msg->d_time <= System::Sys_LastTime( ) - Network:: net_forceLatency.GetInt( ); msg = d_udpPort.d_sendFirst ) {
			Network::Net_SendUDPPacket( d_socket, msg->d_size, msg->d_data, msg->d_address );
			d_udpPort.d_sendFirst = d_udpPort.d_sendFirst->d_next;
			if( !d_udpPort.d_sendFirst )
				d_udpPort.d_sendLast = NULL;
			d_udpPort.d_udpMsgAllocator.Free( msg );
		}
	} else
		Network::Net_SendUDPPacket( d_socket, size, data, to );
}
