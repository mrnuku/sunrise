#include "precompiled.h"
#pragma hdrstop

cinematics_t Screen::cin;

/*
=================================================================
PCX LOADING
=================================================================
*/
/*
==============
SCR_LoadPCX
==============
*/
void Screen::SCR_LoadPCX( const Str & filename, byte ** pic, byte ** palette, int * width, int * height ) {

	byte * raw;
	pcx_t * pcx;
	int x, y;
	int dataByte, runLength;
	byte * out, * pix;

	*pic = NULL;

	//
	// load the file
	//
	File_Memory fm = FileSystem::ReadFile( filename );

	//
	// parse the PCX file
	//
	pcx =( pcx_t * )fm.GetDataPtr( );
	raw = &pcx->data;

	if( pcx->manufacturer != 0x0A || pcx->version != 5 || pcx->encoding != 1 || pcx->bits_per_pixel != 8 || pcx->xmax >= 640 || pcx->ymax >= 480 ) {

		Common::Com_Printf( "Bad pcx file %s\n", filename.c_str( ) );
		return;
	}

	out =( byte * )Z_Malloc( ( pcx->ymax+1 ) * ( pcx->xmax+1 ) );

	*pic = out;

	pix = out;

	if( palette ) {

		*palette =( byte * )Z_Malloc( 768 );
		memcpy( *palette, ( byte * )pcx + fm.Length( ) - 768, 768 );
	}

	if( width ) * width = pcx->xmax+1;
	if( height ) * height = pcx->ymax+1;

	for( y = 0; y<= pcx->ymax; y++, pix += pcx->xmax+1 ) {

		for( x = 0; x<= pcx->xmax; ) {

			dataByte = * raw++;

			if( ( dataByte & 0xC0 ) == 0xC0 ) {

				runLength = dataByte & 0x3F;
				dataByte = * raw++;
			} else runLength = 1;

			while( runLength-- > 0 ) pix[ x++ ] = dataByte;
		}

	}

	if( ( size_t )( raw -( byte * )pcx ) > fm.Length( ) ) {

		Common::Com_Printf( "PCX file %s was malformed", filename.c_str( ) );
		Z_Free( *pic );
		* pic = NULL;
	}

	FileSystem::FreeFile( fm );
}

//=============================================================

/*

==================

SCR_StopCinematic

==================

*/
void Screen::SCR_StopCinematic( ) {

#if 0

	Client::cl.cinematictime = 0; // done
	if( cin.pic )
	{
		Z_Free( cin.pic );
		cin.pic = NULL;
	}
	if( cin.pic_pending )
	{
		Z_Free( cin.pic_pending );
		cin.pic_pending = NULL;
	}
	if( Client::cl.cinematicpalette_active )
	{
		Renderer::CinematicSetPalette( NULL );
		Client::cl.cinematicpalette_active = false;
	}
	if( Client::cl.cinematic_file.IsValid( ) ) {

		FileSystem::CloseFile( Client::cl.cinematic_file );
	}
	if( cin.hnodes1 )
	{
		Z_Free( cin.hnodes1 );
		cin.hnodes1 = NULL;
	}

	// switch back down to 11 khz sound if necessary
	if( cin.restart_sound )
	{
		cin.restart_sound = false;
		Client::CL_Snd_Restart_f( );
	}

#endif
}

/*

====================

SCR_FinishCinematic



Called when either the cinematic completes, or it is aborted

====================

*/
void Screen::SCR_FinishCinematic( ) {

	// tell the server to advance to the next map / cinematic
	Message::MSG_WriteByte( &Client::cls.netchan.message, clc_stringcmd );
	Message::SZ_Print( &Client::cls.netchan.message, Str( va( "nextserver %i\n", Client::cl.servercount ) ) );
}

//==========================================================================

/*

==================

SmallestNode1

==================

*/
int Screen::SmallestNode1( int numhnodes ) {

	int i;
	int best, bestnode;

	best = 99999999;
	bestnode = -1;
	for( i = 0; i<numhnodes; i++ )
	{
		if( cin.h_used[ i ] )
			continue;
		if( !cin.h_count[ i ] )
			continue;
		if( cin.h_count[ i ] < best )
		{
			best = cin.h_count[ i ];
			bestnode = i;
		}
	}

	if( bestnode == -1 )
		return -1;

	cin.h_used[ bestnode ] = true;
	return bestnode;
}


/*
==================
Huff1TableInit

Reads the 64k counts table and initializes the node trees
==================
*/
void Screen::Huff1TableInit( ) {

#if 0

	int prev;
	int j;
	int * node, * nodebase;
	byte counts[ 256 ];
	int numhnodes;

	cin.hnodes1 =( int * )Z_Malloc( 256* 256* 2* 4 );
	Common::Com_Memset( cin.hnodes1, 0, 256* 256* 2* 4 );

	for( prev = 0; prev<256; prev++ )
	{
		Common::Com_Memset( cin.h_count, 0, sizeof( cin.h_count ) );
		Common::Com_Memset( cin.h_used, 0, sizeof( cin.h_used ) );

		// read a row of counts
		Client::cl.cinematic_file.Read( counts, sizeof( counts ) );
		for( j = 0; j<256; j++ )
			cin.h_count[ j ] = counts[ j ];

		// build the nodes
		numhnodes = 256;
		nodebase = cin.hnodes1 + prev* 256* 2;

		while( numhnodes != 511 )
		{
			node = nodebase +( numhnodes-256 )* 2;

			// pick two lowest counts
			node[ 0 ] = SmallestNode1( numhnodes );
			if( node[ 0 ] == -1 )
				break; // no more

			node[ 1 ] = SmallestNode1( numhnodes );
			if( node[ 1 ] == -1 )
				break;

			cin.h_count[ numhnodes ] = cin.h_count[node[ 0 ]] + cin.h_count[node[ 1 ]];
			numhnodes++;
		}

		cin.numhnodes1[ prev ] = numhnodes-1;
	}

#endif
}

/*
==================
Huff1Decompress
==================
*/
cblock_t Screen::Huff1Decompress( cblock_t in ) {

	byte * input;
	byte * out_p;
	int nodenum;
	int count;
	cblock_t out;
	int inbyte;
	int * hnodes, * hnodesbase;
	//int		i;

	// get decompressed count
	count = in.data[ 0 ] +( in.data[ 1 ]<<8 ) +( in.data[ 2 ]<<16 ) +( in.data[ 3 ]<<24 );
	input = in.data + 4;
	out_p = out.data =( byte * )Z_Malloc( count );

	// read bits

	hnodesbase = cin.hnodes1 - 256* 2; // nodes 0-255 aren't stored

	hnodes = hnodesbase;
	nodenum = cin.numhnodes1[ 0 ];
	while( count )
	{
		inbyte = * input++;
		//-----------
		if( nodenum < 256 )
		{
			hnodes = hnodesbase +( nodenum<<9 );
			* out_p++ = nodenum;
			if( !--count )
				break;
			nodenum = cin.numhnodes1[ nodenum ];
		}
		nodenum = hnodes[nodenum* 2 +( inbyte&1 )];
		inbyte >>= 1;
		//-----------
		if( nodenum < 256 )
		{
			hnodes = hnodesbase +( nodenum<<9 );
			* out_p++ = nodenum;
			if( !--count )
				break;
			nodenum = cin.numhnodes1[ nodenum ];
		}
		nodenum = hnodes[nodenum* 2 +( inbyte&1 )];
		inbyte >>= 1;
		//-----------
		if( nodenum < 256 )
		{
			hnodes = hnodesbase +( nodenum<<9 );
			* out_p++ = nodenum;
			if( !--count )
				break;
			nodenum = cin.numhnodes1[ nodenum ];
		}
		nodenum = hnodes[nodenum* 2 +( inbyte&1 )];
		inbyte >>= 1;
		//-----------
		if( nodenum < 256 )
		{
			hnodes = hnodesbase +( nodenum<<9 );
			* out_p++ = nodenum;
			if( !--count )
				break;
			nodenum = cin.numhnodes1[ nodenum ];
		}
		nodenum = hnodes[nodenum* 2 +( inbyte&1 )];
		inbyte >>= 1;
		//-----------
		if( nodenum < 256 )
		{
			hnodes = hnodesbase +( nodenum<<9 );
			* out_p++ = nodenum;
			if( !--count )
				break;
			nodenum = cin.numhnodes1[ nodenum ];
		}
		nodenum = hnodes[nodenum* 2 +( inbyte&1 )];
		inbyte >>= 1;
		//-----------
		if( nodenum < 256 )
		{
			hnodes = hnodesbase +( nodenum<<9 );
			* out_p++ = nodenum;
			if( !--count )
				break;
			nodenum = cin.numhnodes1[ nodenum ];
		}
		nodenum = hnodes[nodenum* 2 +( inbyte&1 )];
		inbyte >>= 1;
		//-----------
		if( nodenum < 256 )
		{
			hnodes = hnodesbase +( nodenum<<9 );
			* out_p++ = nodenum;
			if( !--count )
				break;
			nodenum = cin.numhnodes1[ nodenum ];
		}
		nodenum = hnodes[nodenum* 2 +( inbyte&1 )];
		inbyte >>= 1;
		//-----------
		if( nodenum < 256 )
		{
			hnodes = hnodesbase +( nodenum<<9 );
			* out_p++ = nodenum;
			if( !--count )
				break;
			nodenum = cin.numhnodes1[ nodenum ];
		}
		nodenum = hnodes[nodenum* 2 +( inbyte&1 )];
		inbyte >>= 1;
	}

	if( input - in.data != in.count && input - in.data != in.count+1 )
	{
		Common::Com_Printf( "Decompression overread by %i", ( input - in.data ) - in.count );
	}
	out.count = ( int )( out_p - out.data );

	return out;
}

/*
==================
SCR_ReadNextFrame
==================
*/
byte * Screen::SCR_ReadNextFrame( ) {

#if 0

	int r;
	int command;
	byte samples[22050/14* 4];
	byte compressed[ 0x20000 ];
	int size;
	byte * pic;
	cblock_t in, huf1;
	int start, end, count;

	// read the next frame
	r = Client::cl.cinematic_file.Read( &command, 4 );

	if( r != 1 ) return NULL;

	command = LittleLong( command );
	if( command == 2 )
		return NULL; // last frame marker

	if( command == 1 )
	{ // read palette
		Client::cl.cinematic_file.Read( Client::cl.cinematicpalette, sizeof( Client::cl.cinematicpalette ) );
		Client::cl.cinematicpalette_active = 0; // dubious....  exposes an edge case
	}

	// decompress the next frame
	Client::cl.cinematic_file.Read( &size, 4 );
	size = LittleLong( size );
	if( size > sizeof( compressed ) || size < 1 ) Common::Com_Error( ERR_DROP, "Bad compressed frame size" );

	Client::cl.cinematic_file.Read( compressed, size );

	// read sound
	start = Client::cl.cinematicframe* cin.s_rate/14;
	end =( Client::cl.cinematicframe+1 )* cin.s_rate/14;
	count = end - start;

	Client::cl.cinematic_file.Read( samples, count * cin.s_width * cin.s_channels );

	Sound::S_RawSamples( count, cin.s_rate, cin.s_width, cin.s_channels, samples );

	in.data = compressed;
	in.count = size;

	huf1 = Huff1Decompress( in );

	pic = huf1.data;

	Client::cl.cinematicframe++;

	return pic;

#endif

	return NULL;
}

/*
==================
SCR_RunCinematic
==================
*/
void Screen::SCR_RunCinematic( ) {

#if 0

	int frame;

	if( Client::cl.cinematictime <= 0 )
	{
		SCR_StopCinematic( );
		return;
	}

	if( Client::cl.cinematicframe == -1 )
		return; // static image

	if( Client::cls.key_dest != key_game )
	{ // pause if menu or console is up
		Client::cl.cinematictime = Client::cls.realtime - Client::cl.cinematicframe* 1000/14;
		return;
	}

	frame = ( Client::cls.realtime - Client::cl.cinematictime ) * 14.0f / 1000.0f;
	if( frame <= Client::cl.cinematicframe )
		return;
	if( frame > Client::cl.cinematicframe+1 )
	{
		Common::Com_Printf( "Dropped frame: %i > %i\n", frame, Client::cl.cinematicframe+1 );
		Client::cl.cinematictime = Client::cls.realtime - Client::cl.cinematicframe* 1000/14;
	}
	if( cin.pic )
		Z_Free( cin.pic );
	cin.pic = cin.pic_pending;
	cin.pic_pending = NULL;
	cin.pic_pending = SCR_ReadNextFrame( );
	if( !cin.pic_pending )
	{
		SCR_StopCinematic( );
		SCR_FinishCinematic( );
		Client::cl.cinematictime = 1; // hack to get the black screen behind loading
		SCR_BeginLoadingPlaque( );
		Client::cl.cinematictime = 0;
		return;
	}

#endif
}

/*

==================

SCR_DrawCinematic



Returns true if a cinematic is active, meaning the view rendering

should be skipped

==================

*/
bool Screen::SCR_DrawCinematic( ) {

#if 0

	if( Client::cl.cinematictime <= 0 )
	{
		return false;
	}

	if( Client::cls.key_dest == key_menu )
	{ // blank screen and pause if menu is up
		Renderer::CinematicSetPalette( NULL );
		Client::cl.cinematicpalette_active = false;
		return true;
	}

	if( !Client::cl.cinematicpalette_active )
	{
		Renderer::CinematicSetPalette( ( unsigned char * )Client::cl.cinematicpalette );
		Client::cl.cinematicpalette_active = true;
	}

	if( !cin.pic )
		return true;

	Renderer::DrawStretchRaw( 0, 0, Video::viddef.width, Video::viddef.height, cin.width, cin.height, cin.pic );

	return true;

#endif

	return true;
}

/*

==================

SCR_PlayCinematic



==================

*/
void Screen::SCR_PlayCinematic( const Str & arg ) {

#if 0

	int width, height;
	byte * palette;
	Str name, dot;

	Client::cl.cinematicframe = 0;
	dot = strstr( arg, "." );
	if( dot && dot == ".pcx" ) { // static pcx image

		sprintf( name, "pics/%s", arg.c_str( ) );
		SCR_LoadPCX( name, &cin.pic, &palette, &cin.width, &cin.height );
		Client::cl.cinematicframe = -1;
		Client::cl.cinematictime = 1;
		SCR_EndLoadingPlaque( );
		Client::cls.state = ca_active;
		if( !cin.pic )
		{
			Common::Com_Printf( "%s not found.\n", name.c_str( ) );
			Client::cl.cinematictime = 0;
		}
		else
		{
			memcpy( Client::cl.cinematicpalette, palette, sizeof( Client::cl.cinematicpalette ) );
			Z_Free( palette );
		}
		return;
	}

	sprintf( name, "video/%s", arg.c_str( ) );
	Client::cl.cinematic_file = FileSystem::OpenFileRead( name );

	SCR_EndLoadingPlaque( );

	Client::cls.state = ca_active;

	Client::cl.cinematic_file.Read( &width, 4 );
	Client::cl.cinematic_file.Read( &height, 4 );
	cin.width = LittleLong( width );
	cin.height = LittleLong( height );

	Client::cl.cinematic_file.Read( &cin.s_rate, 4 );
	cin.s_rate = LittleLong( cin.s_rate );
	Client::cl.cinematic_file.Read( &cin.s_width, 4 );
	cin.s_width = LittleLong( cin.s_width );
	Client::cl.cinematic_file.Read( &cin.s_channels, 4 );
	cin.s_channels = LittleLong( cin.s_channels );

	Huff1TableInit( );

	Client::cl.cinematicframe = 0;
	cin.pic = SCR_ReadNextFrame( );
	Client::cl.cinematictime = System::Sys_Milliseconds( );

#endif
}
