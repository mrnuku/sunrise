#include "precompiled.h"
#pragma hdrstop

using namespace WindowsNS;

PerfCounter::PerfCounter( PPERF_OBJECT_TYPE parent, PPERF_COUNTER_DEFINITION counter, int index ) {

	d_parentObjectType = parent;
	d_counterDefinition = counter;
	d_index = index;
}

PerfCounter & PerfCounter::operator ++( int ) {

	d_index++;
	if( IsValid( ) ) d_counterDefinition = ( PPERF_COUNTER_DEFINITION )( ( LPBYTE )d_counterDefinition + d_counterDefinition->ByteLength );
	return *this;
}

PerfCounterBlock::PerfCounterBlock( PPERF_INSTANCE_DEFINITION parent, PPERF_COUNTER_BLOCK block ) {

	d_parentInstanceDefinition = parent;
	d_counterBlock = block;
}

PerfInstance::PerfInstance( PPERF_OBJECT_TYPE parent, PPERF_INSTANCE_DEFINITION instance ) {

	d_parentObjectType = parent;
	d_instanceDefinition = instance;
	d_index = 0;
}

PerfInstance & PerfInstance::operator ++( int ) {

	d_index++;
	if( IsValid( ) ) {
		PPERF_COUNTER_BLOCK perfCounterBlock = ( PPERF_COUNTER_BLOCK )( ( PBYTE )d_instanceDefinition + d_instanceDefinition->ByteLength );
		d_instanceDefinition = ( PPERF_INSTANCE_DEFINITION )( ( PBYTE )perfCounterBlock + perfCounterBlock->ByteLength );
	}
	return *this;
}

PerfObject::PerfObject( PPERF_DATA_BLOCK parent, PPERF_OBJECT_TYPE object ) {

	d_parentDataBlock = parent;
	d_objectType = object;
	d_index = 0;
}

PerfCounter PerfObject::FindCounter( int index ) {

	PerfCounter counter = FirstCounter( );
	for( ; counter.IsValid( ); counter++ ) {
		if( counter.GetNameIndex( ) == index ) return counter;
	}
	return counter;
}

PerfObject & PerfObject::operator ++( int ) {

	d_index++;
	if( IsValid( ) ) d_objectType = ( PPERF_OBJECT_TYPE )( ( LPBYTE )d_objectType + d_objectType->TotalByteLength );
	return *this;
}

std::map< int, Str > *	PerfData::g_counterTable = NULL;
DynamicModule *			PerfData::g_advapi32Module = NULL;
REGQUERYVALUEEXAPROC	PerfData::RegQueryValueExA;

PerfData::PerfData( ) {

	d_regBufferSize = c_regBlockSize * ( c_regMulSize - 1 );
}

PerfObject PerfData::Query( int index ) {

	if( !g_advapi32Module ) Init( );

	Str indexStr( index );

	d_regBufferSize = c_regBlockSize * ( c_regMulSize - 1 );
	do {
		d_regBufferSize += c_regBlockSize;
		d_regBuffer.SetNum( d_regBufferSize );

	} while( PerfData::RegQueryValueExA( HKEY_PERFORMANCE_DATA, indexStr.c_str( ), NULL, NULL, d_regBuffer.Ptr( ), ( DWORD * )&d_regBufferSize ) == ERROR_MORE_DATA );

	d_regBuffer.SetNum( d_regBufferSize );
	d_dataBlock = ( PPERF_DATA_BLOCK )d_regBuffer.Ptr( );

	return PerfObject( d_dataBlock, ( PPERF_OBJECT_TYPE )( d_regBuffer.Ptr( ) + d_dataBlock->HeaderLength ) );
}

void PerfData::PrintHelp( int index ) {

	if( !g_advapi32Module ) Init( );

	if( !g_counterTable ) {

		d_regBufferSize = c_regBlockSize * ( c_regMulSize - 1 );
		do {
			d_regBufferSize += c_regBlockSize;
			d_regBuffer.SetNum( d_regBufferSize );
		} while( PerfData::RegQueryValueExA( HKEY_PERFORMANCE_DATA, "Counter", NULL, NULL, d_regBuffer.Ptr( ), ( DWORD * )&d_regBufferSize ) == ERROR_MORE_DATA );
		d_regBuffer.SetNum( d_regBufferSize );

		g_counterTable = new std::map< int, Str >;

		const char * hText = ( char * )d_regBuffer.Ptr( );
		while( *hText ) {

			int key = atoi( hText );
			hText += strlen( hText ) + 1;
			( *g_counterTable )[ key ] = hText;
			hText += strlen( hText ) + 1;
		}
	}

	Str indexStr( index );

	d_regBufferSize = c_regBlockSize * ( c_regMulSize - 1 );
	do {
		d_regBufferSize += c_regBlockSize;
		d_regBuffer.SetNum( d_regBufferSize );

	} while( PerfData::RegQueryValueExA( HKEY_PERFORMANCE_DATA, indexStr.c_str( ), NULL, NULL, d_regBuffer.Ptr( ), ( DWORD * )&d_regBufferSize ) == ERROR_MORE_DATA );

	d_regBuffer.SetNum( d_regBufferSize );
	d_dataBlock = ( PPERF_DATA_BLOCK )d_regBuffer.Ptr( );

	PerfObject perfObject = PerfObject( d_dataBlock, ( PPERF_OBJECT_TYPE )( d_regBuffer.Ptr( ) + d_dataBlock->HeaderLength ) );
	int i = 0, j;

	Common::Com_Printf( "PerfData::PrintHelp: Counter List %s (%i->%i) count %i {\n\n", ( *g_counterTable )[ perfObject.GetNameIndex( ) ].c_str( ), index, perfObject.GetNameIndex( ), perfObject.GetNumInstances( ) );

	for( PerfCounter perfCounter = perfObject.FirstCounter( ); perfCounter.IsValid( ); perfCounter++, i++ ) {

		Common::Com_Printf( "\t%i: (%i) %s (", i, perfCounter.GetNameIndex( ), ( *g_counterTable )[ perfCounter.GetNameIndex( ) ].c_str( ) );
		j = 0;
		for( PerfInstance perfInstance = perfObject.FirstInstance( ); perfInstance.IsValid( ) && j < 4; perfInstance++, j++ ) {

			PerfCounterBlock perfCounterBlock = perfInstance.GetBlock( );
			if( perfCounter.IsLongLong( ) ) Common::Com_Printf( " __%lu", *perfCounterBlock.GetCounter( perfCounter ) );
			else Common::Com_Printf( " %u", *perfCounterBlock.GetCounterDword( perfCounter ) );
		}
		Common::Com_Printf( " )\n" );
	}

	Common::Com_Printf( "}\n" );
}

void PerfData::Init( ) {

	g_advapi32Module = new DynamicModule( "advapi32.dll" );
	RegQueryValueExA = ( REGQUERYVALUEEXAPROC )g_advapi32Module->FindProc( "RegQueryValueExA" );
}

void PerfData::Shutdown( ) {

	delete g_advapi32Module;
}
