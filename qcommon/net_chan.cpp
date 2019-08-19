#include "precompiled.h"
#pragma hdrstop

/*

packet header
-------------
31	sequence
1	does this message contain a reliable payload
31	acknowledge sequence
1	acknowledge receipt of even/odd message
16	qport

The remote connection never knows if it missed a reliable message, the
local side detects that it has been dropped by seeing a sequence acknowledge
higher thatn the last reliable sequence, but without the correct evon/odd
bit for the reliable set.

If the sender notices that a reliable message has been dropped, it will be
retransmitted.  It will not be retransmitted again until a message after
the retransmit has been acknowledged and the reliable still failed to get thetpRenderer::

if the sequence number is -1, the packet should be handled without a netcon

The reliable message can be added to at any time by doing
MSG_Write* ( &netchan->message, <data> ).

If the message buffer is overflowed, either by a single message, or by
multiple frames worth piling up while the last reliable transmit goes
unacknowledged, the netchan signals a fatal error.

Reliable messages are always placed first in a packet, then the unreliable
message is included if there is sufficient room.

To the receiver, there is no distinction between the reliable and unreliable
parts of the message, they are just processed out as a single larger message.

Illogical packet sequence numbers cause the packet to be dropped, but do
not kill the connection.  This, combined with the tight window of valid
reliable acknowledgement numbers provides protection against malicious
address spoofing.


The qport field is a workaround for bad address translating routers that
sometimes remap the client's source port on a packet during gameplay.

If the base part of the net address matches and the qport matches, then the
channel matches even if the IP port differs.  The IP port should be updated
to the new value before sending out any replies.


If there is no information that needs to be transfered on a given frame, such as during the connection stage while waiting for the client to load, then a packet only needs to be delivered if there is something in the
unacknowledged reliable
*/

#if 0
/*
===============
Netchan_Init

===============
*/
void Netchan::Netchan_Init( ) {
	// pick a port value that should be nice and random
#ifdef MM_TIMER
	int port = WindowsNS::timeGetTime( ) & 0xFFFF;
#else
	srand( System::Sys_MillisecondsUint( ) );
	int port = rand( ) & 0xFFFF;
#endif
	CVarSystem::SetCVarInt( "qport", port );
}

/*
===============
Netchan_OutOfBand

Sends an out-of-band datagram
================
*/
void Netchan::Netchan_OutOfBand( int net_socket, netadr_t adr, size_t length, byte * data ) {
	sizebuf_t	send;
	byte		send_buf[ MAX_MSGLEN ];
	// write the packet header
	Message::SZ_Init( &send, send_buf, sizeof( send_buf ) );	
	Message::MSG_WriteLong( &send, -1 );	// -1 sequence means out of band
	Message::SZ_Write( &send, data, length );
	// send the datagram
	Network::NET_SendPacket( ( netsrc_t )net_socket, send.cursize, send.data, adr );
}

/*
===============
Netchan_OutOfBandPrint

Sends a text message in an out-of-band datagram
================
*/
void Netchan::Netchan_OutOfBandPrint( int net_socket, netadr_t adr, const char * format, ... ) {
	va_list		argptr;
	Str		string;	
	va_start( argptr, format );
	vsprintf( string, format, argptr );
	va_end( argptr );
	Netchan_OutOfBand( net_socket, adr, string.Length( ), ( byte * )string.c_str( ) );
}


/*
==============
Netchan_Setup

called to open a channel to a remote system
==============
*/
void Netchan::Netchan_Setup( netsrc_t sock, netchan_t * chan, netadr_t adr, int qport ) {
	Common::Com_Memset( chan, 0, sizeof( netchan_t ) );	
	chan->sock = sock;
	chan->remote_address = adr;
	chan->qport = qport;
	chan->last_received = System::Sys_LastTime( );
	chan->incoming_sequence = 0;
	chan->outgoing_sequence = 1;
	Message::SZ_Init( &chan->message, chan->message_buf, sizeof( chan->message_buf ) );
	chan->message.allowoverflow = true;
}


/*
===============
Netchan_CanReliable

Returns true if the last reliable message has acked
================
*/
bool Netchan::Netchan_CanReliable( netchan_t * chan ) {
	if( chan->reliable_length )
		return false;			// waiting for ack
	return true;
}


bool Netchan::Netchan_NeedReliable( netchan_t * chan ) {
	bool	send_reliable;
	// if the remote side dropped the last reliable message, resend it
	send_reliable = false;
	if( chan->incoming_acknowledged > chan->last_reliable_sequence && chan->incoming_reliable_acknowledged != chan->reliable_sequence )
		send_reliable = true;
	// if the reliable transmit buffer is empty, copy the current message out
	if( !chan->reliable_length && chan->message.cursize )
		send_reliable = true;
	return send_reliable;
}

/*
===============
Netchan_Transmit

tries to send an unreliable message to a connection, and handles the
transmition / retransmition of the reliable messages.

A 0 length will still generate a packet and deal with the reliable messages.
================
*/
void Netchan::Netchan_Transmit( netchan_t * chan, size_t length, byte * data ) {
	sizebuf_t	send;
	byte		send_buf[ MAX_MSGLEN ];
	bool		send_reliable;
	unsigned	w1, w2;
	// check for message overflow
	if( chan->message.overflowed ) {
		chan->fatal_error = true;
		Common::Com_Printf( "%s:Outgoing message overflow\n", Network::NET_AdrToString( chan->remote_address ).c_str( ) );
		return;
	}
	send_reliable = Netchan_NeedReliable( chan );
	if( !chan->reliable_length && chan->message.cursize ) {
		Common::Com_Memcpy( chan->reliable_buf, chan->message_buf, chan->message.cursize );
		chan->reliable_length = chan->message.cursize;
		chan->message.cursize = 0;
		chan->reliable_sequence ^= 1;
	}
	// write the packet header
	Message::SZ_Init( &send, send_buf, sizeof( send_buf ) );
	w1 =( chan->outgoing_sequence & ~( 1 << 31 ) ) |( send_reliable << 31 );
	w2 =( chan->incoming_sequence & ~( 1 << 31 ) ) |( chan->incoming_reliable_sequence << 31 );
	chan->outgoing_sequence++;
	chan->last_sent = System::Sys_LastTime( );
	Message::MSG_WriteLong( &send, w1 );
	Message::MSG_WriteLong( &send, w2 );
	// send the qport if we are a client
	if( chan->sock == NS_CLIENT )
		Message::MSG_WriteShort( &send, qport.GetInt( ) );
	// copy the reliable message to the packet first
	if( send_reliable ) {
		Message::SZ_Write( &send, chan->reliable_buf, chan->reliable_length );
		chan->last_reliable_sequence = chan->outgoing_sequence;
	}	
	// add the unreliable part if space is available
	if( send.maxsize - send.cursize >= length )
		Message::SZ_Write( &send, data, length );
	else
		Common::Com_Printf( "Netchan_Transmit: dumped unreliable\n" );
	// send the datagram
	Network::NET_SendPacket( chan->sock, send.cursize, send.data, chan->remote_address );
	if( showpackets.GetBool( ) ) {
		if( send_reliable )
			Common::Com_Printf( "send %i : s =%i reliable =%i ack =%i rack =%i\n", send.cursize, chan->outgoing_sequence - 1, chan->reliable_sequence, chan->incoming_sequence, chan->incoming_reliable_sequence );
		else
			Common::Com_Printf( "send %i : s =%i ack =%i rack =%i\n", send.cursize, chan->outgoing_sequence - 1, chan->incoming_sequence, chan->incoming_reliable_sequence );
	}
}

/*
=================
Netchan_Process

called when the current net_message is from remote_address
modifies net_message so that it points to the packet payload
=================
*/
bool Netchan::Netchan_Process( netchan_t * chan, sizebuf_t * msg ) {

	unsigned	sequence, sequence_ack;
	unsigned	reliable_ack, reliable_message;
	int			qport;
	// get sequence numbers		
	Message::MSG_BeginReading( msg );
	sequence = Message::MSG_ReadLong( msg );
	sequence_ack = Message::MSG_ReadLong( msg );
	// read the qport if we are a server
	if( chan->sock == NS_SERVER )
		qport = Message::MSG_ReadShort( msg );
	reliable_message = sequence >> 31;
	reliable_ack = sequence_ack >> 31;
	sequence &= ~( 1 << 31 );
	sequence_ack &= ~( 1 << 31 );
	if( showpackets.GetBool( ) ) {
		if( reliable_message )
			Common::Com_Printf( "recv %i : s =%i reliable =%i ack =%i rack =%i\n", msg->cursize, sequence, chan->incoming_reliable_sequence ^ 1, sequence_ack, reliable_ack );
		else
			Common::Com_Printf( "recv %i : s =%i ack =%i rack =%i\n", msg->cursize, sequence, sequence_ack, reliable_ack );
	}
	//
	// discard stale or duplicated packets
	//
	if( sequence <= chan->incoming_sequence ) {
		if( showdrop.GetBool( ) )
			Common::Com_Printf( "%s:Out of order packet %i at %i\n", Network::NET_AdrToString( chan->remote_address ).c_str( ), sequence, chan->incoming_sequence );
		return false;
	}
	//
	// dropped packets don't keep the message from being used
	//
	chan->dropped = sequence - ( chan->incoming_sequence + 1 );
	if( chan->dropped > 0 && showdrop.GetBool( ) )
		Common::Com_Printf( "%s:Dropped %i packets at %i\n", Network::NET_AdrToString( chan->remote_address ).c_str( ), chan->dropped, sequence );
	//
	// if the current outgoing reliable message has been acknowledged
	// clear the buffer to make way for the next
	//
	if( reliable_ack == chan->reliable_sequence )
		chan->reliable_length = 0;	// it has been received	
	//
	// if this message contains a reliable message, bump incoming_reliable_sequence 
	//
	chan->incoming_sequence = sequence;
	chan->incoming_acknowledged = sequence_ack;
	chan->incoming_reliable_acknowledged = reliable_ack;
	if( reliable_message )
		chan->incoming_reliable_sequence ^= 1;
	//
	// the message can now be read from the current message pointer
	//
	chan->last_received = System::Sys_LastTime( );
	return true;
}

#endif

// // //// // //// // //// //
// MessageQueue
// //// // //// // //// //

MessageQueue::MessageQueue( ) {
	Init( 0 );
}

bool MessageQueue::Add( const byte * data, const int size ) {
	if( GetSpaceLeft( ) < size + 8 )
		return false;
	int sequence = d_last;
	WriteShort( size );
	WriteLong( sequence );
	WriteData( data, size );
	d_last++;
	return true;
}

bool MessageQueue::Get( byte * data, int & size ) {
	if( d_first == d_last ) {
		size = 0;
		return false;
	}
	int sequence;
	size = ReadShort( );
	sequence = ReadLong( );
	ReadData( data, size );
	assert( sequence == d_first );
	d_first++;
	return true;
}

void MessageQueue::CopyToBuffer( byte * buf ) const {
	if( d_startIndex <= d_endIndex ) {
		Common::Com_Memcpy( buf, d_buffer + d_startIndex, d_endIndex - d_startIndex );
	} else {
		Common::Com_Memcpy( buf, d_buffer + d_startIndex, sizeof( d_buffer ) - d_startIndex );
		Common::Com_Memcpy( buf + sizeof( d_buffer ) - d_startIndex, d_buffer, d_endIndex );
	}
}

// // //// // //// // //// //
// MessageBuffer
// //// // //// // //// //

MessageBuffer::MessageBuffer() {
	/*d_writeData = NULL;
	d_readData = NULL;
	d_maxSize = 0;
	d_curSize = 0;
	d_writeBit = 0;
	d_readCount = 0;
	d_readBit = 0;
	d_allowOverflow = false;
	d_overflowed = false;*/
	Common::Com_Memset( this, 0, sizeof( MessageBuffer ) );
}

bool MessageBuffer::CheckOverflow( int numBits ) {
	assert( numBits >= 0 );
	if( numBits > GetRemainingWriteBits( ) ) {
		if( !d_allowOverflow )
			Common::Com_Error( ERR_DROP, "MessageBuffer: overflow without allowOverflow set" );
		if( numBits > ( d_maxSize << 3 ) )
			Common::Com_Error( ERR_DROP, "MessageBuffer: %i bits is > full message size", numBits );
		Common::Com_Printf( "MessageBuffer: overflow\n" );
		BeginWriting( );
		d_overflowed = true;
		return true;
	}
	return false;
}

byte * MessageBuffer::GetByteSpace( int length ) {
	if( !d_writeData )
		Common::Com_Error( ERR_DROP, "MessageBuffer::GetByteSpace: cannot write to message" );
	// round up to the next byte
	WriteByteAlign( );
	// check for overflow
	CheckOverflow( length << 3 );
	byte * ptr = d_writeData + d_curSize;
	d_curSize += length;
	return ptr;
}

void MessageBuffer::WriteBits( int value, int numBits ) {
	if( !d_writeData )
		Common::Com_Error( ERR_DROP, "MessageBuffer::WriteBits: cannot write to message" );
	// check if the number of bits is valid
	if( numBits == 0 || numBits < -31 || numBits > 32 )
		Common::Com_Error( ERR_DROP, "MessageBuffer::WriteBits: bad numBits %i", numBits );
	// check for value overflows
	// this should be an error really, as it can go unnoticed and cause either bandwidth or corrupted data transmitted
	if( numBits != 32 ) {
		if( numBits > 0 ) {
			if( value > ( 1 << numBits ) - 1 )
				Common::Com_DPrintf( "MessageBuffer::WriteBits: value overflow %d %d", value, numBits );
			else if( value < 0 )
				Common::Com_DPrintf( "MessageBuffer::WriteBits: value overflow %d %d", value, numBits );
		} else {
			int r = 1 << ( - 1 - numBits );
			if( value > r - 1 )
				Common::Com_DPrintf( "MessageBuffer::WriteBits: value overflow %d %d", value, numBits );
			else if( value < -r )
				Common::Com_DPrintf( "MessageBuffer::WriteBits: value overflow %d %d", value, numBits );
		}
	}
	if( numBits < 0 )
		numBits = -numBits;
	// check for msg overflow
	if( CheckOverflow( numBits ) )
		return;
	// write the bits
	while( numBits ) {
		if( d_writeBit == 0 ) {
			d_writeData[ d_curSize ] = 0;
			d_curSize++;
		}
		int put = 8 - d_writeBit;
		if( put > numBits ) {
			put = numBits;
		}
		int fraction = value & ( ( 1 << put ) - 1 );
		d_writeData[ d_curSize - 1 ] |= fraction << d_writeBit;
		numBits -= put;
		value >>= put;
		d_writeBit = ( d_writeBit + put ) & 7;
	}
}

void MessageBuffer::WriteDeltaByteCounter( int oldValue, int newValue ) {
	int i, x = oldValue ^ newValue;
	for( i = 7; i > 0; i-- ) {
		if( x & ( 1 << i ) ) {
			i++;
			break;
		}
	}
	WriteBits( i, 3 );
	if( i )
		WriteBits( ( ( 1 << i ) - 1 ) & newValue, i );
}

void MessageBuffer::WriteDeltaShortCounter( int oldValue, int newValue ) {
	int i, x = oldValue ^ newValue;
	for( i = 15; i > 0; i-- ) {
		if( x & ( 1 << i ) ) {
			i++;
			break;
		}
	}
	WriteBits( i, 4 );
	if( i )
		WriteBits( ( ( 1 << i ) - 1 ) & newValue, i );
}

void MessageBuffer::WriteDeltaLongCounter( int oldValue, int newValue ) {
	int i, x = oldValue ^ newValue;
	for( i = 31; i > 0; i-- ) {
		if( x & ( 1 << i ) ) {
			i++;
			break;
		}
	}
	WriteBits( i, 5 );
	if( i )
		WriteBits( ( ( 1 << i ) - 1 ) & newValue, i );
}

bool MessageBuffer::WriteDeltaDict( const Dict &dict, const Dict *base ) {
	bool changed = false;
	if( base != NULL ) {
		for( int i = 0; i < dict.GetNumKeyVals(); i++ ) {
			const KeyValue * kv = dict.GetKeyVal( i );
			const KeyValue * basekv = base->FindKey( kv->GetKey() );
			if( basekv == NULL || basekv->GetValue().Icmp( kv->GetValue() ) != 0 ) {
				WriteString( kv->GetKey() );
				WriteString( kv->GetValue() );
				changed = true;
			}
		}
		WriteString( "" );
		for( int i = 0; i < base->GetNumKeyVals( ); i++ ) {
			const KeyValue * basekv = base->GetKeyVal( i );
			const KeyValue * kv = dict.FindKey( basekv->GetKey( ) );
			if( kv == NULL ) {
				WriteString( basekv->GetKey() );
				changed = true;
			}
		}
		WriteString( "" );
	} else {
		for( int i = 0; i < dict.GetNumKeyVals( ); i++ ) {
			const KeyValue * kv = dict.GetKeyVal( i );
			WriteString( kv->GetKey() );
			WriteString( kv->GetValue() );
			changed = true;
		}
		WriteString( "" );
		WriteString( "" );
	}
	return changed;
}

int MessageBuffer::ReadBits( int numBits ) const {
	if( !d_readData )
		Common::Com_Error( ERR_DROP, "MessageBuffer::ReadBits: cannot read from message" );
	// check if the number of bits is valid
	if( numBits == 0 || numBits < -31 || numBits > 32 )
		Common::Com_Error( ERR_DROP, "MessageBuffer::ReadBits: bad numBits %i", numBits );
	bool sgn = false;
	int value = 0;
	int valueBits = 0;
	if( numBits < 0 ) {
		numBits = -numBits;
		sgn = true;
	}
	// check for overflow
	if( numBits > GetRemainingReadBits( ) )
		return -1;
	while( valueBits < numBits ) {
		if( d_readBit == 0 )
			d_readCount++;
		int get = 8 - d_readBit;
		if( get > ( numBits - valueBits ) )
			get = numBits - valueBits;
		int fraction = d_readData[ d_readCount - 1 ];
		fraction >>= d_readBit;
		fraction &= ( 1 << get ) - 1;
		value |= fraction << valueBits;
		valueBits += get;
		d_readBit = ( d_readBit + get ) & 7;
	}
	if( sgn ) {
		if( value & ( 1 << ( numBits - 1 ) ) )
			value |= -1 ^ ( ( 1 << numBits ) - 1 );
	}
	return value;
}

int MessageBuffer::ReadData( void * data, int length ) const {
	ReadByteAlign( );
	int cnt = d_readCount;
	if( d_readCount + length > d_curSize ) {
		if( data )
			Common::Com_Memcpy( data, d_readData + d_readCount, GetRemaingData() );
		d_readCount = d_curSize;
	} else {
		if( data )
			Common::Com_Memcpy( data, d_readData + d_readCount, length );
		d_readCount += length;
	}
	return ( d_readCount - cnt );
}

void MessageBuffer::ReadNetadr( netadr_t * adr ) const {
	adr->type = NA_IP;
	for( int i = 0; i < 4; i++ )
		adr->ip[ i ] = ReadByte( );
	adr->port = ReadUShort( );
}

int MessageBuffer::ReadDelta( int oldValue, int numBits ) const {
	if( ReadBits( 1 ) )
		return ReadBits( numBits );
	return oldValue;
}

int MessageBuffer::ReadDeltaByteCounter( int oldValue ) const {
	int i = ReadBits( 3 );
	if( !i )
		return oldValue;
	int newValue = ReadBits( i );
	return ( oldValue & ~( ( 1 << i ) - 1 ) | newValue );
}

int MessageBuffer::ReadDeltaShortCounter( int oldValue ) const {
	int i = ReadBits( 4 );
	if( !i )
		return oldValue;
	int newValue = ReadBits( i );
	return ( oldValue & ~( ( 1 << i ) - 1 ) | newValue );
}

int MessageBuffer::ReadDeltaLongCounter( int oldValue ) const {
	int i = ReadBits( 5 );
	if( !i )
		return oldValue;
	int newValue = ReadBits( i );
	return ( oldValue & ~( ( 1 << i ) - 1 ) | newValue );
}

bool MessageBuffer::ReadDeltaDict( Dict & dict, const Dict * base ) const {
	Str		key;
	Str		value;
	bool		changed = false;
	if( base != NULL )
		dict = *base;
	else
		dict.Clear( );
	while( ReadString( key ) != 0 ) {
		ReadString( value );
		dict.Set( key, value );
		changed = true;
	}
	while( ReadString( key ) != 0 ) {
		dict.Delete( key );
		changed = true;
	}
	return changed;
}

int MessageBuffer::DirToBits( const Vec3 & dir, int numBits ) {
	assert( numBits >= 6 && numBits <= 32 );
	assert( dir.LengthSqr( ) - 1.0f < 0.01f );
	numBits /= 3;
	int max = ( 1 << ( numBits - 1 ) ) - 1;
	float bias = 0.5f / max;
	int bits = FLOATSIGNBITSET( dir.x ) << ( numBits * 3 - 1 );
	bits |= ( ( int )( ( fabs( dir.x ) + bias ) * max ) ) << ( numBits * 2 );
	bits |= FLOATSIGNBITSET( dir.y ) << ( numBits * 2 - 1 );
	bits |= ( ( int )( ( fabs( dir.y ) + bias ) * max ) ) << ( numBits * 1 );
	bits |= FLOATSIGNBITSET( dir.z ) << ( numBits * 1 - 1 );
	bits |= ( ( int )( ( fabs( dir.z ) + bias ) * max ) ) << ( numBits * 0 );
	return bits;
}

Vec3 MessageBuffer::BitsToDir( int bits, int numBits ) {
	static float sign[ 2 ] = { 1.0f, -1.0f };
	assert( numBits >= 6 && numBits <= 32 );
	numBits /= 3;
	int max = ( 1 << ( numBits - 1 ) ) - 1;
	float invMax = 1.0f / max;
	Vec3 dir;
	dir.x = sign[( bits >> ( numBits * 3 - 1 ) ) & 1] * ( ( bits >> ( numBits * 2 ) ) & max ) * invMax;
	dir.y = sign[( bits >> ( numBits * 2 - 1 ) ) & 1] * ( ( bits >> ( numBits * 1 ) ) & max ) * invMax;
	dir.z = sign[( bits >> ( numBits * 1 - 1 ) ) & 1] * ( ( bits >> ( numBits * 0 ) ) & max ) * invMax;
	dir.NormalizeFast( );
	return dir;
}

void MessageBuffer::WriteString( const Str & s ) {
	WriteByteAlign( );
	int len = s.Length( ) + 1;
	Common::Com_Memcpy( GetByteSpace( len ), s.c_str( ), len );
}

int MessageBuffer::ReadString( Str & s ) const {
	ReadByteAlign( );
	int len = Str::Length( ( char * )&d_writeData[ d_readCount ] );
	int lenp1 = len + 1;
	Common::Com_Memcpy( s.ReSize( lenp1, len ), &d_writeData[ d_readCount ], lenp1 );
	d_readCount += lenp1;
	return len;
}

// // //// // //// // //// //
// MessageChannel
// //// // //// // //// //

MessageChannel::MessageChannel( ) {
	Shutdown( );
}

void MessageChannel::Init( const netadr_t adr, const int id ) {
	d_remoteAddress = adr;
	d_id = id;
	d_maxRate = 50000;
	d_lastSendTime = 0;
	d_lastDataBytes = 0;
	d_outgoingRateTime = 0;
	d_outgoingRateBytes = 0;
	d_incomingRateTime = 0;
	d_incomingRateBytes = 0;
	d_incomingReceivedPackets = 0.0f;
	d_incomingDroppedPackets = 0.0f;
	d_incomingPacketLossTime = 0;
	d_outgoingCompression = 0.0f;
	d_incomingCompression = 0.0f;
	d_outgoingSequence = 1;
	d_incomingSequence = 0;
	d_unsentFragments = false;
	d_unsentFragmentStart = 0;
	d_fragmentSequence = 0;
	d_fragmentLength = 0;
	d_reliableSend.Init( 1 );
	d_reliableReceive.Init( 0 );
}

void MessageChannel::Shutdown( ) {
	d_remoteAddress.Clear( );
	d_id = -1;
	Init( d_remoteAddress, d_id );
}

void MessageChannel::ResetRate( ) {
	d_lastSendTime = 0;
	d_lastDataBytes = 0;
	d_outgoingRateTime = 0;
	d_outgoingRateBytes = 0;
	d_incomingRateTime = 0;
	d_incomingRateBytes = 0;
}

void MessageChannel::WriteMessageData( MessageBuffer & out, const MessageBuffer & msg ) {
	MessageBuffer tmp;
	byte tmpBuf[ MAX_MESSAGE_SIZE ];
	tmp.Init( tmpBuf, sizeof( tmpBuf ) );
	// write acknowledgement of last received reliable message
	tmp.WriteLong( d_reliableReceive.GetLast( ) );
	// write reliable messages
	d_reliableSend.CopyToBuffer( tmp.GetData( ) + tmp.GetSize( ) );
	tmp.SetSize( tmp.GetSize( ) + d_reliableSend.GetTotalSize( ) );
	tmp.WriteShort( 0 );
	// write data
	tmp.WriteData( msg.GetData( ), msg.GetSize( ) );
	// write message size
	out.WriteShort( tmp.GetSize( ) );
	// compress message
	out.WriteData( tmp.GetData( ), tmp.GetSize( ) );
	//idFile_BitMsg file( out );
	//compressor->Init( &file, true, 3 );
	//compressor->Write( tmp.GetData( ), tmp.GetSize( ) );
	//compressor->FinishCompress( );
	//outgoingCompression = compressor->GetCompressionRatio( );
}

bool MessageChannel::ReadMessageData( MessageBuffer & out, const MessageBuffer & msg ) {
	int reliableAcknowledge, reliableMessageSize, reliableSequence;
	// read message size
	//out.SetSize( msg.ReadShort( ) );
	int msgSize = msg.ReadShort( );
	// decompress message
	out.BeginWriting( );
	out.WriteData( msg.GetData( ) + msg.GetReadCount( ), msgSize );
	//idFile_BitMsg file( msg );
	//compressor->Init( &file, false, 3 );
	//compressor->Read( out.GetData( ), out.GetSize( ) );
	//incomingCompression = compressor->GetCompressionRatio( );
	out.BeginReading( );
	// read acknowledgement of sent reliable messages
	reliableAcknowledge = out.ReadLong( );
	// remove acknowledged reliable messages
	while( d_reliableSend.GetFirst( ) <= reliableAcknowledge ) {
		if( !d_reliableSend.Get( NULL, reliableMessageSize ) )
			break;
	}
	// read reliable messages
	reliableMessageSize = out.ReadShort( );
	while( reliableMessageSize != 0 ) {
		if( reliableMessageSize <= 0 || reliableMessageSize > out.GetSize( ) - out.GetReadCount( ) ) {
			Common::Com_DPrintf( "%s: bad reliable message\n", Network::NET_AdrToString( d_remoteAddress ).c_str( ) );
			return false;
		}
		reliableSequence = out.ReadLong( );
		if( reliableSequence == d_reliableReceive.GetLast( ) + 1 )
			d_reliableReceive.Add( out.GetData( ) + out.GetReadCount( ), reliableMessageSize );
		out.ReadData( NULL, reliableMessageSize );
		reliableMessageSize = out.ReadShort( );
	}
	return true;
}

void MessageChannel::SendNextFragment( NetworkPort & port, timeType time ) {
	MessageBuffer	msg;
	byte		msgBuf[ MAX_PACKETLEN ];
	int			fragLength;
	if( d_remoteAddress.type == NA_BAD )
		return;
	if( !d_unsentFragments )
		return;
	// write the packet
	msg.Init( msgBuf, sizeof( msgBuf ) );
	msg.WriteShort( d_id );
	msg.WriteLong( d_outgoingSequence | FRAGMENT_BIT );
	fragLength = FRAGMENT_SIZE;
	if( d_unsentFragmentStart + fragLength > d_unsentMsg.GetSize( ) )
		fragLength = d_unsentMsg.GetSize( ) - d_unsentFragmentStart;
	msg.WriteShort( d_unsentFragmentStart );
	msg.WriteShort( fragLength );
	msg.WriteData( d_unsentMsg.GetData( ) + d_unsentFragmentStart, fragLength );
	// send the packet
	port.SendPacket( d_remoteAddress, msg.GetData( ), msg.GetSize( ) );
	// update rate control variables
	UpdateOutgoingRate( time, msg.GetSize( ) );
	if( Netchan::showpackets.GetBool( ) )
		Common::Com_DPrintf( "%d send %4i : s = %i fragment = %i,%i\n", d_id, msg.GetSize( ), d_outgoingSequence - 1, d_unsentFragmentStart, fragLength );
	d_unsentFragmentStart += fragLength;
	// this exit condition is a little tricky, because a packet
	// that is exactly the fragment length still needs to send
	// a second packet of zero length so that the other side
	// can tell there aren't more to follow
	if( d_unsentFragmentStart == d_unsentMsg.GetSize( ) && fragLength != FRAGMENT_SIZE ) {
		d_outgoingSequence++;
		d_unsentFragments = false;
	}
}

int MessageChannel::SendMessage( NetworkPort & port, timeType time, const MessageBuffer & msg ) {
	if( d_remoteAddress.type == NA_BAD )
		return -1;
	if( d_unsentFragments ) {
		Common::Com_Error( ERR_DROP, "MessageChannel::SendMessage: called with unsent fragments left" );
		return -1;
	}
	int totalLength = 4 + d_reliableSend.GetTotalSize( ) + 4 + msg.GetSize( );
	if( totalLength > MAX_MESSAGE_SIZE ) {
		Common::Com_DPrintf( "MessageChannel::SendMessage: message too large, length = %i\n", totalLength );
		return -1;
	}
	d_unsentMsg.Init( d_unsentBuffer, sizeof( d_unsentBuffer ) );
	d_unsentMsg.BeginWriting( );
	// fragment large messages
	if( totalLength >= FRAGMENT_SIZE ) {
		d_unsentFragments = true;
		d_unsentFragmentStart = 0;
		// write out the message data
		WriteMessageData( d_unsentMsg, msg );
		// send the first fragment now
		SendNextFragment( port, time );
		return d_outgoingSequence;
	}
	// write the header
	d_unsentMsg.WriteShort( d_id );
	d_unsentMsg.WriteLong( d_outgoingSequence );
	// write out the message data
	WriteMessageData( d_unsentMsg, msg );
	// send the packet
	port.SendPacket( d_remoteAddress, d_unsentMsg.GetData( ), d_unsentMsg.GetSize( ) );
	// update rate control variables
	UpdateOutgoingRate( time, d_unsentMsg.GetSize( ) );
	if( Netchan::showpackets.GetBool( ) )
		Common::Com_DPrintf( "%d send %4i : s = %i ack = %i\n", d_id, d_unsentMsg.GetSize( ), d_outgoingSequence - 1, d_incomingSequence );
	d_outgoingSequence++;
	return ( d_outgoingSequence - 1 );
}

bool MessageChannel::Process( const netadr_t from, timeType time, MessageBuffer & msg, int & sequence ) {
	int			fragStart = 0, fragLength = 0, dropped;
	bool		fragmented = false;
	MessageBuffer	fragMsg;
	// the IP port can't be used to differentiate them, because
	// some address translating routers periodically change UDP
	// port assignments
	if( d_remoteAddress.port != from.port ) {
		Common::Com_DPrintf( "MessageChannel::Process: fixing up a translated port\n" );
		d_remoteAddress.port = from.port;
	}
	// update incoming rate
	UpdateIncomingRate( time, msg.GetSize( ) );
	// get sequence numbers
	sequence = msg.ReadLong( );
	// check for fragment information
	if( sequence & FRAGMENT_BIT ) {
		sequence &= ~FRAGMENT_BIT;
		fragmented = true;
	}
	// read the fragment information
	if( fragmented ) {
		fragStart = msg.ReadShort( );
		fragLength = msg.ReadShort( );
	}
	if( Netchan::showpackets.GetBool( ) ) {
		if( fragmented )
			Common::Com_DPrintf( "%d recv %4i : s = %i fragment = %i,%i\n", d_id, msg.GetSize( ), sequence, fragStart, fragLength );
		else
			Common::Com_DPrintf( "%d recv %4i : s = %i\n", d_id, msg.GetSize( ), sequence );
	}
	// discard out of order or duplicated packets
	if( sequence <= d_incomingSequence ) {
		if( Netchan::showdrop.GetBool( ) || Netchan::showpackets.GetBool( ) )
			Common::Com_DPrintf( "%s: out of order packet %i at %i\n", Network::NET_AdrToString( d_remoteAddress ).c_str( ), sequence, d_incomingSequence );
		return false;
	}
	// dropped packets don't keep this message from being used
	dropped = sequence - ( d_incomingSequence + 1 );
	if( dropped > 0 )
		if( Netchan::showdrop.GetBool( ) || Netchan::showpackets.GetBool( ) ) {
			Common::Com_DPrintf( "%s: dropped %i packets at %i\n", Network::NET_AdrToString( d_remoteAddress ).c_str( ), dropped, sequence );
		UpdatePacketLoss( time, 0, dropped );
	}
	// if the message is fragmented
	if( fragmented ) {
		// make sure we have the correct sequence number
		if( sequence != d_fragmentSequence ) {
			d_fragmentSequence = sequence;
			d_fragmentLength = 0;
		}
		// if we missed a fragment, dump the message
		if( fragStart != d_fragmentLength ) {
			if( Netchan::showdrop.GetBool( ) || Netchan::showpackets.GetBool( ) )
				Common::Com_DPrintf( "%s: dropped a message fragment at seq %d\n", Network::NET_AdrToString( d_remoteAddress ).c_str( ), sequence );
			// we can still keep the part that we have so far,
			// so we don't need to clear fragmentLength
			UpdatePacketLoss( time, 0, 1 );
			return false;
		}

		// copy the fragment to the fragment buffer
		if( fragLength < 0 || fragLength > msg.GetRemaingData( ) || d_fragmentLength + fragLength > sizeof( d_fragmentBuffer ) ) {
			if( Netchan::showdrop.GetBool( ) || Netchan::showpackets.GetBool( ) )
				Common::Com_DPrintf( "%s: illegal fragment length\n", Network::NET_AdrToString( d_remoteAddress ).c_str( ) );
			UpdatePacketLoss( time, 0, 1 );
			return false;
		}
		Common::Com_Memcpy( d_fragmentBuffer + d_fragmentLength, msg.GetData( ) + msg.GetReadCount( ), fragLength );
		d_fragmentLength += fragLength;
		UpdatePacketLoss( time, 1, 0 );
		// if this wasn't the last fragment, don't process anything
		if( fragLength == FRAGMENT_SIZE )
			return false;
	} else {
		Common::Com_Memcpy( d_fragmentBuffer, msg.GetData( ) + msg.GetReadCount( ), msg.GetRemaingData( ) );
		d_fragmentLength = msg.GetRemaingData( );
		UpdatePacketLoss( time, 1, 0 );
	}
	fragMsg.Init( d_fragmentBuffer, d_fragmentLength );
	fragMsg.SetSize( d_fragmentLength );
	fragMsg.BeginReading( );
	d_incomingSequence = sequence;
	// read the message data
	if( !ReadMessageData( msg, fragMsg ) )
		return false;
	return true;
}

bool MessageChannel::SendReliableMessage( const MessageBuffer & msg ) {
	assert( d_remoteAddress.type != NA_BAD );
	if( d_remoteAddress.type == NA_BAD )
		return false;
	bool result = d_reliableSend.Add( msg.GetData( ), msg.GetSize( ) );
	if( !result ) {
		Common::Com_DPrintf( "MessageChannel::SendReliableMessage: overflowed" );
		return false;
	}
	return result;
}

bool MessageChannel::GetReliableMessage( MessageBuffer & msg ) {
	int size;
	bool result = d_reliableReceive.Get( msg.GetData( ), size );
	msg.SetSize( size );
	msg.BeginReading( );
	return result;
}

void MessageChannel::UpdateOutgoingRate( timeType time, const int size ) {
	// update the outgoing rate control variables
	timeType deltaTime = time - d_lastSendTime;
	if( deltaTime > 1000 ) {
		d_lastDataBytes = 0;
	} else {
		d_lastDataBytes -= ( ( int )deltaTime * d_maxRate ) / 1000;
		if( d_lastDataBytes < 0 )
			d_lastDataBytes = 0;
	}
	d_lastDataBytes += size;
	d_lastSendTime = time;
	// update outgoing rate variables
	if( time - d_outgoingRateTime > 1000 ) {
		d_outgoingRateBytes -= d_outgoingRateBytes * ( int )( time - d_outgoingRateTime - 1000 ) / 1000;
		if( d_outgoingRateBytes < 0 )
			d_outgoingRateBytes = 0;
	}
	d_outgoingRateTime = time - 1000;
	d_outgoingRateBytes += size;
}

void MessageChannel::UpdateIncomingRate( timeType time, const int size ) {
	// update incoming rate variables
	if( time - d_incomingRateTime > 1000 ) {
		d_incomingRateBytes -= d_incomingRateBytes * ( int )( time - d_incomingRateTime - 1000 ) / 1000;
		if( d_incomingRateBytes < 0 )
			d_incomingRateBytes = 0;
	}
	d_incomingRateTime = time - 1000;
	d_incomingRateBytes += size;
}

void MessageChannel::UpdatePacketLoss( timeType time, const int numReceived, const int numDropped ) {
	// update incoming packet loss variables
	if( time - d_incomingPacketLossTime > 5000 ) {
		float scale = ( time - d_incomingPacketLossTime - 5000 ) * ( 1.0f / 5000.0f );
		d_incomingReceivedPackets -= d_incomingReceivedPackets * scale;
		if( d_incomingReceivedPackets < 0.0f )
			d_incomingReceivedPackets = 0.0f;
		d_incomingDroppedPackets -= d_incomingDroppedPackets * scale;
		if( d_incomingDroppedPackets < 0.0f )
			d_incomingDroppedPackets = 0.0f;
	}
	d_incomingPacketLossTime = time - 5000;
	d_incomingReceivedPackets += numReceived;
	d_incomingDroppedPackets += numDropped;
}
