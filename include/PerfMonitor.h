#ifndef PERFMONITOR_H
#define PERFMONITOR_H

class PerfCounter {

	friend class										PerfCounterBlock;

private:

	WindowsNS::PPERF_OBJECT_TYPE						d_parentObjectType;
	WindowsNS::PPERF_COUNTER_DEFINITION					d_counterDefinition;
	int													d_index;

public:

														PerfCounter( WindowsNS::PPERF_OBJECT_TYPE parent = NULL, WindowsNS::PPERF_COUNTER_DEFINITION counter = NULL, int index = 0 );

	int													GetNameIndex( )	{ return ( int )d_counterDefinition->CounterNameTitleIndex; }
	PerfCounter &										operator ++( int );
	bool												IsValid( )			{ return d_counterDefinition && ( d_index < ( int )d_parentObjectType->NumCounters ); }
	bool												IsLongLong( )		{ return d_counterDefinition->CounterSize == 8; }
};

class PerfCounterBlock {

private:

	WindowsNS::PPERF_INSTANCE_DEFINITION				d_parentInstanceDefinition;
	WindowsNS::PPERF_COUNTER_BLOCK						d_counterBlock;

public:

														PerfCounterBlock( WindowsNS::PPERF_INSTANCE_DEFINITION parent, WindowsNS::PPERF_COUNTER_BLOCK block );

	__int64 *											GetCounter( PerfCounter & counter )	{ return ( __int64 * )( ( byte * )d_counterBlock + counter.d_counterDefinition->CounterOffset ); }
	__int32 *											GetCounterDword( PerfCounter & counter )	{ return ( __int32 * )( ( byte * )d_counterBlock + counter.d_counterDefinition->CounterOffset ); }
};

class PerfInstance {

private:

	WindowsNS::PPERF_OBJECT_TYPE						d_parentObjectType;
	WindowsNS::PPERF_INSTANCE_DEFINITION				d_instanceDefinition;
	int													d_index;

public:

														PerfInstance( WindowsNS::PPERF_OBJECT_TYPE parent, WindowsNS::PPERF_INSTANCE_DEFINITION instance );

	PerfCounterBlock									GetBlock( )			{ return PerfCounterBlock( d_instanceDefinition, ( WindowsNS::PPERF_COUNTER_BLOCK )( ( byte * )d_instanceDefinition + d_instanceDefinition->ByteLength ) ); }
	PerfInstance &										operator ++( int );
	bool												IsValid( )			{ return d_instanceDefinition && ( d_index < ( int )d_parentObjectType->NumInstances ); }
};

class PerfObject {

private:

	WindowsNS::PPERF_DATA_BLOCK							d_parentDataBlock;
	WindowsNS::PPERF_OBJECT_TYPE						d_objectType;
	int													d_index;

public:

														PerfObject( WindowsNS::PPERF_DATA_BLOCK parent, WindowsNS::PPERF_OBJECT_TYPE object );

	PerfCounter											FirstCounter( )		{ return PerfCounter( d_objectType, ( WindowsNS::PPERF_COUNTER_DEFINITION )( ( byte * )d_objectType + d_objectType->HeaderLength ) ); }
	PerfCounter											FindCounter( int index );
	PerfInstance										FirstInstance( )	{ return PerfInstance( d_objectType, ( WindowsNS::PPERF_INSTANCE_DEFINITION )( ( byte * )d_objectType + d_objectType->DefinitionLength ) ); }
	int													GetNameIndex( )	{ return ( int )d_objectType->ObjectNameTitleIndex; }
	int													GetNumInstances( )	{ return ( int )d_objectType->NumInstances; }
	PerfObject &										operator ++( int );
	bool												IsValid( )			{ return d_index < ( int )d_parentDataBlock->NumObjectTypes; }
};

typedef WindowsNS::LONG ( APIENTRY * REGQUERYVALUEEXAPROC )( WindowsNS::HKEY hKey, WindowsNS::LPCSTR lpValueName, WindowsNS::LPDWORD lpReserved, WindowsNS::LPDWORD lpType, WindowsNS::LPBYTE lpData, WindowsNS::LPDWORD lpcbData );

class PerfData {

private:

	static const int									c_regBlockSize = 16 * 1024;
	static const int									c_regMulSize = 8;
	CAList< byte, c_regBlockSize * c_regMulSize >		d_regBuffer;
	int													d_regBufferSize;

	WindowsNS::PPERF_DATA_BLOCK							d_dataBlock;

	static std::map< int, Str > *						g_counterTable;
	static class DynamicModule *						g_advapi32Module;

public:

														PerfData( );

	PerfObject											Query( int index );
	__int64 *											GetTime100ns( )		{ return ( __int64 * )&d_dataBlock->PerfTime100nSec; }

	void												PrintHelp( int index );

	static void											Init( );
	static void											Shutdown( );

	static REGQUERYVALUEEXAPROC							RegQueryValueExA;
};

#endif
