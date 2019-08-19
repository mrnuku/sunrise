// Copyright( C ) 2004 Id Software, Inc.
//

// This is real evil but allows the code to inspect arbitrary class variables.
#define private		public
#define protected	public

#include "System_portable.h"
#include "System_OS.h"
#include "System_Q2.h"

#ifdef ID_DEBUG_MEMORY
#include "GameTypeInfo.h"				// Make sure this is up to date!
#else
#include "NoGameTypeInfo.h"
#endif

// disabled because it's adds about 64MB to state dumps and takes a really long time
//#define DUMP_GAMELOCAL


typedef void( *WriteVariableType_t )( const char * varName, const char * varType, const char * scope, const char * prefix, const char * postfix, const char * value, const void * varPtr, int varSize );


class TypeInfoTools {
public:
	static const classTypeInfo_t * 	FindClassInfo( const char * typeName );
	static const enumTypeInfo_t * 	FindEnumInfo( const char * typeName );
	static bool						IsSubclassOf( const char * typeName, const char * superType );
	static void						PrintType( const void * typePtr, const char * typeName );
	static void						WriteTypeToFile( File * fp, const void * typePtr, const char * typeName );
	static void						InitTypeVariables( const void * typePtr, const char * typeName, int value );
	static void						WriteGameState( const char * fileName );
	static void						CompareGameState( const char * fileName );

private:
	static File			* 					fp;
	static int						initValue;
	static WriteVariableType_t		Write;
	static Lexer			* 				src;
	static bool						typeError;

	static const char * 				OutputString( const char * string );
	static bool						ParseTemplateArguments( Lexer &src, Str &arguments );
	static void						PrintVariable( const char * varName, const char * varType, const char * scope, const char * prefix, const char * postfix, const char * value, const void * varPtr, int varSize );
	static void						WriteVariable( const char * varName, const char * varType, const char * scope, const char * prefix, const char * postfix, const char * value, const void * varPtr, int varSize );
	static void						WriteGameStateVariable( const char * varName, const char * varType, const char * scope, const char * prefix, const char * postfix, const char * value, const void * varPtr, int varSize );
	static void						InitVariable( const char * varName, const char * varType, const char * scope, const char * prefix, const char * postfix, const char * value, const void * varPtr, int varSize );
	static void						VerifyVariable( const char * varName, const char * varType, const char * scope, const char * prefix, const char * postfix, const char * value, const void * varPtr, int varSize );
	static int						WriteVariable_r( const void * varPtr, const char * varName, const char * varType, const char * scope, const char * prefix, const int pointerDepth );
	static void						WriteClass_r( const void * classPtr, const char * className, const char * classType, const char * scope, const char * prefix, const int pointerDepth );
};

File * 							TypeInfoTools::fp			= NULL;
int									TypeInfoTools::initValue	= 0;
WriteVariableType_t					TypeInfoTools::Write		= NULL;
Lexer * 							TypeInfoTools::src		= NULL;
bool								TypeInfoTools::typeError	= false;


/*
================
GetTypeVariableName
================
*/
const char * GetTypeVariableName( const char * typeName, int offset ) {
	static char			varName[ 1024 ];
	int i;

	for( i = 0; classTypeInfo[ i ].typeName != NULL; i++ ) {
		if( Str::Cmp( typeName, classTypeInfo[ i ].typeName ) == 0 ) {
			if( classTypeInfo[ i ].variables[ 0 ].name != NULL && offset >= classTypeInfo[ i ].variables[ 0 ].offset ) {
				break;
			}
			typeName = classTypeInfo[ i ].superType;
			if( *typeName == '\0' ) {
				return "<unknown>";
			}
			i = -1;
		}
	}

	const classTypeInfo_t &classInfo = classTypeInfo[ i ];

	for( i = 0; classInfo.variables[ i ].name != NULL; i++ ) {
		if( offset <= classInfo.variables[ i ].offset ) {
			break;
		}
	}
	if( i == 0 ) {
		Str::snPrintf( varName, sizeof( varName ), "%s::<unknown>", classInfo.typeName );
	} else {
		Str::snPrintf( varName, sizeof( varName ), "%s::%s", classInfo.typeName, classInfo.variables[ i-1 ].name );
	}
	return varName;
}

/*
================
TypeInfoTools::FindClassInfo
================
*/
const classTypeInfo_t * TypeInfoTools::FindClassInfo( const char * typeName ) {
	int i;

	for( i = 0; classTypeInfo[ i ].typeName != NULL; i++ ) {
		if( Str::Cmp( typeName, classTypeInfo[ i ].typeName ) == 0 ) {
			return &classTypeInfo[ i ];
		}
	}
	return NULL;
}

/*
================
TypeInfoTools::FindEnumInfo
================
*/
const enumTypeInfo_t * TypeInfoTools::FindEnumInfo( const char * typeName ) {
	int i;

	for( i = 0; enumTypeInfo[ i ].typeName != NULL; i++ ) {
		if( Str::Cmp( typeName, enumTypeInfo[ i ].typeName ) == 0 ) {
			return &enumTypeInfo[ i ];
		}
	}
	return NULL;
}

/*
================
TypeInfoTools::IsSubclassOf
================
*/
bool TypeInfoTools::IsSubclassOf( const char * typeName, const char * superType ) {
	int i;

	while( *typeName != '\0' ) {
		if( Str::Cmp( typeName, superType ) == 0 ) {
			return true;
		}
		for( i = 0; classTypeInfo[ i ].typeName != NULL; i++ ) {
			if( Str::Cmp( typeName, classTypeInfo[ i ].typeName ) == 0 ) {
				typeName = classTypeInfo[ i ].superType;
				break;
			}
		}
		if( classTypeInfo[ i ].typeName == NULL ) {
			Common::Com_DPrintf( "super class %s not found", typeName );
			break;
		}
	}
	return false;
}

/*
================
TypeInfoTools::OutputString
================
*/
const char * TypeInfoTools::OutputString( const char * string ) {
	static int			index = 0;
	static char			buffers[ 4 ][ 16384 ];
	char * out;
	int i, c;

	out = buffers[ index ];
	index =( index + 1 ) & 3;

	if( string == NULL ) {
		return NULL;
	}

	for( i = 0; i < sizeof( buffers[ 0 ] ) - 2; i++ ) {
		c = * string++;
		switch( c ) {
			case '\0': out[ i ] = '\0'; return out;
			case '\\': out[ i++ ] = '\\'; out[ i ] = '\\'; break;
			case '\n': out[ i++ ] = '\\'; out[ i ] = 'n'; break;
			case '\r': out[ i++ ] = '\\'; out[ i ] = 'r'; break;
			case '\t': out[ i++ ] = '\\'; out[ i ] = 't'; break;
			case '\v': out[ i++ ] = '\\'; out[ i ] = 'v'; break;
			default: out[ i ] = c; break;
		}
	}
	out[ i ] = '\0';
	return out;
}

/*
================
TypeInfoTools::ParseTemplateArguments
================
*/
bool TypeInfoTools::ParseTemplateArguments( Lexer &src, Str &arguments ) {
	int indent;
	Token token;

	arguments = "";

	if( !src.ExpectTokenString( "<" ) ) {
		return false;
	}

	indent = 1;
	while( indent ) {
		if( !src.ReadToken( &token ) ) {
			break;
		}
		if( token == "<" ) {
			indent++;
		} else if( token == ">" ) {
			indent--;
		} else {
			if( arguments.Length( ) ) {
				arguments += " ";
			}
			arguments += token;
		}
	}
	return true;
}

/*
================
TypeInfoTools::PrintType
================
*/
void TypeInfoTools::PrintType( const void * typePtr, const char * typeName ) {
	TypeInfoTools::fp = NULL;
	TypeInfoTools::initValue = 0;
	TypeInfoTools::Write = PrintVariable;
	WriteClass_r( typePtr, "", typeName, "", "", 0 );
}

/*
================
TypeInfoTools::InitTypeVariables
================
*/
void TypeInfoTools::InitTypeVariables( const void * typePtr, const char * typeName, int value ) {
	TypeInfoTools::fp = NULL;
	TypeInfoTools::initValue = value;
	TypeInfoTools::Write = InitVariable;
	WriteClass_r( typePtr, "", typeName, "", "", 0 );
}

/*
================
IsAllowedToChangedFromSaveGames
================
*/
bool IsAllowedToChangedFromSaveGames( const char * varName, const char * varType, const char * scope, const char * prefix, const char * postfix, const char * value ) {
	if( Str::Icmp( scope, "Animator" ) == 0 ) {
		if( Str::Icmp( varName, "forceUpdate" ) == 0 ) {
			return true;
		}
		if( Str::Icmp( varName, "lastTransformTime" ) == 0 ) {
			return true;
		}
		if( Str::Icmp( varName, "AFPoseTime" ) == 0 ) {
			return true;
		}
		if( Str::Icmp( varName, "frameBounds" ) == 0 ) {
			return true;
		}
	} else if( Str::Icmp( scope, "ClipModel" ) == 0 ) {
		if( Str::Icmp( varName, "touchCount" ) == 0 ) {
			return true;
		}
	} else if( Str::Icmp( scope, "Entity" ) == 0 ) {
		if( Str::Icmp( varName, "numPVSAreas" ) == 0 ) {
			return true;
		}
		if( Str::Icmp( varName, "renderView" ) == 0 ) {
			return true;
		}
	} else if( Str::Icmp( scope, "BrittleFracture" ) == 0 ) {
		if( Str::Icmp( varName, "changed" ) == 0 ) {
			return true;
		}
	} else if( Str::Icmp( scope, "Physics_AF" ) == 0 ) {
		return true;
	} else if( Str::Icmp( scope, "renderEntity_t" ) == 0 ) {
		// These get fixed up when UpdateVisuals is called
		if( Str::Icmp( varName, "origin" ) == 0 ) {
			return true;
		}
		if( Str::Icmp( varName, "axis" ) == 0 ) {
			return true;
		}
		if( Str::Icmp( varName, "bounds" ) == 0 ) {
			return true;
		}
	}

	if( Str::Icmpn( prefix, "AFEntity_Base::af.AF::physicsObj.Physics_AF", 49 ) == 0 ) {
		return true;
	}

	return false;
}

/*
================
IsRenderHandleVariable
================
*/
bool IsRenderHandleVariable( const char * varName, const char * varType, const char * scope, const char * prefix, const char * postfix, const char * value ) {
	if( Str::Icmp( scope, "ClipModel" ) == 0 ) {
		if( Str::Icmp( varName, "renderModelHandle" ) == 0 ) {
			return true;
		}
	} else if( Str::Icmp( scope, "FXLocalAction" ) == 0 ) {
		if( Str::Icmp( varName, "lightDefHandle" ) == 0 ) {
			return true;
		}
		if( Str::Icmp( varName, "modelDefHandle" ) == 0 ) {
			return true;
		}
	} else if( Str::Icmp( scope, "Entity" ) == 0 ) {
		if( Str::Icmp( varName, "modelDefHandle" ) == 0 ) {
			return true;
		}
	} else if( Str::Icmp( scope, "Light" ) == 0 ) {
		if( Str::Icmp( varName, "lightDefHandle" ) == 0 ) {
			return true;
		}
	} else if( Str::Icmp( scope, "AFEntity_Gibbable" ) == 0 ) {
		if( Str::Icmp( varName, "skeletonModelDefHandle" ) == 0 ) {
			return true;
		}
	} else if( Str::Icmp( scope, "AFEntity_SteamPipe" ) == 0 ) {
		if( Str::Icmp( varName, "steamModelHandle" ) == 0 ) {
			return true;
		}
	} else if( Str::Icmp( scope, "Item" ) == 0 ) {
		if( Str::Icmp( varName, "itemShellHandle" ) == 0 ) {
			return true;
		}
	} else if( Str::Icmp( scope, "ExplodingBarrel" ) == 0 ) {
		if( Str::Icmp( varName, "particleModelDefHandle" ) == 0 ) {
			return true;
		}
		if( Str::Icmp( varName, "lightDefHandle" ) == 0 ) {
			return true;
		}
	} else if( Str::Icmp( scope, "Projectile" ) == 0 ) {
		if( Str::Icmp( varName, "lightDefHandle" ) == 0 ) {
			return true;
		}
	} else if( Str::Icmp( scope, "BFGProjectile" ) == 0 ) {
		if( Str::Icmp( varName, "secondModelDefHandle" ) == 0 ) {
			return true;
		}
	} else if( Str::Icmp( scope, "SmokeParticles" ) == 0 ) {
		if( Str::Icmp( varName, "renderEntityHandle" ) == 0 ) {
			return true;
		}
	} else if( Str::Icmp( scope, "Weapon" ) == 0 ) {
		if( Str::Icmp( varName, "muzzleFlashHandle" ) == 0 ) {
			return true;
		}
		if( Str::Icmp( varName, "worldMuzzleFlashHandle" ) == 0 ) {
			return true;
		}
		if( Str::Icmp( varName, "guiLightHandle" ) == 0 ) {
			return true;
		}
		if( Str::Icmp( varName, "nozzleGlowHandle" ) == 0 ) {
			return true;
		}
	}
	return false;
}

/*
================
TypeInfoTools::PrintVariable
================
*/
void TypeInfoTools::PrintVariable( const char * varName, const char * varType, const char * scope, const char * prefix, const char * postfix, const char * value, const void * varPtr, int varSize ) {
	Common::Com_DPrintf( "%s%s::%s%s = \"%s\"\n", prefix, scope, varName, postfix, value );
}

/*
================
TypeInfoTools::WriteVariable
================
*/
void TypeInfoTools::WriteVariable( const char * varName, const char * varType, const char * scope, const char * prefix, const char * postfix, const char * value, const void * varPtr, int varSize ) {

	for( int i = Str::FindChar( value, '#', 0 ); i >= 0; i = Str::FindChar( value, '#', i+1 ) ) {
		if( 	Str::Icmpn( value+i+1, "INF", 3 ) == 0 ||
				Str::Icmpn( value+i+1, "IND", 3 ) == 0 ||
				Str::Icmpn( value+i+1, "NAN", 3 ) == 0 ||
				Str::Icmpn( value+i+1, "QNAN", 4 ) == 0 ||
				Str::Icmpn( value+i+1, "SNAN", 4 ) == 0 ) {
			Common::Com_DPrintf( "%s%s::%s%s = \"%s\"", prefix, scope, varName, postfix, value );
			break;
		}
	}
	fp->WriteFloatString( "%s%s::%s%s = \"%s\"\n", prefix, scope, varName, postfix, value );
}

/*
================
TypeInfoTools::WriteGameStateVariable
================
*/
void TypeInfoTools::WriteGameStateVariable( const char * varName, const char * varType, const char * scope, const char * prefix, const char * postfix, const char * value, const void * varPtr, int varSize ) {

	for( int i = Str::FindChar( value, '#', 0 ); i >= 0; i = Str::FindChar( value, '#', i+1 ) ) {
		if( 	Str::Icmpn( value+i+1, "INF", 3 ) == 0 ||
				Str::Icmpn( value+i+1, "IND", 3 ) == 0 ||
				Str::Icmpn( value+i+1, "NAN", 3 ) == 0 ||
				Str::Icmpn( value+i+1, "QNAN", 4 ) == 0 ||
				Str::Icmpn( value+i+1, "SNAN", 4 ) == 0 ) {
			Common::Com_DPrintf( "%s%s::%s%s = \"%s\"", prefix, scope, varName, postfix, value );
			break;
		}
	}

	if( IsRenderHandleVariable( varName, varType, scope, prefix, postfix, value ) ) {
		return;
	}

	if( IsAllowedToChangedFromSaveGames( varName, varType, scope, prefix, postfix, value ) ) {
		return;
	}

	fp->WriteFloatString( "%s%s::%s%s = \"%s\"\n", prefix, scope, varName, postfix, value );
}

/*
================
TypeInfoTools::InitVariable
================
*/
void TypeInfoTools::InitVariable( const char * varName, const char * varType, const char * scope, const char * prefix, const char * postfix, const char * value, const void * varPtr, int varSize ) {
	if( varPtr != NULL && varSize > 0 ) {
		// NOTE: skip renderer handles
		if( IsRenderHandleVariable( varName, varType, scope, prefix, postfix, value ) ) {
			return;
		}
		Common::Com_Memset( const_cast<void* >( varPtr ), initValue, varSize );
	}
}

/*
================
TypeInfoTools::VerifyVariable
================
*/
void TypeInfoTools::VerifyVariable( const char * varName, const char * varType, const char * scope, const char * prefix, const char * postfix, const char * value, const void * varPtr, int varSize ) {
	Token token;

	if( typeError ) {
		return;
	}

	src->SkipUntilString( "=" );
	src->ExpectTokenType( TT_STRING, 0, &token );
	if( token.Cmp( value ) != 0 ) {

		// NOTE: skip several things

		if( IsRenderHandleVariable( varName, varType, scope, prefix, postfix, value ) ) {
			return;
		}

		if( IsAllowedToChangedFromSaveGames( varName, varType, scope, prefix, postfix, value ) ) {
			return;
		}

		src->Warning( "state diff for %s%s::%s%s\n%s\n%s", prefix, scope, varName, postfix, token.c_str( ), value );
		typeError = true;
	}
}

/*
================
TypeInfoTools::WriteVariable_r
================
*/
int TypeInfoTools::WriteVariable_r( const void * varPtr, const char * varName, const char * varType, const char * scope, const char * prefix, const int pointerDepth ) {
	int i, isPointer, typeSize;
	Lexer typeSrc;
	Token token;
	Str typeString, templateArgs;

	isPointer = 0;
	typeSize = -1;

	// create a type string without 'const', 'mutable', 'class', 'struct', 'union'
	typeSrc.LoadMemory( varType, Str::Length( varType ), varName );
	while( typeSrc.ReadToken( &token ) ) {
		if( token != "const" && token != "mutable" && token != "class" && token != "struct" && token != "union" ) {
			typeString += token + " ";
		}
	}
	typeString.StripTrailing( ' ' );
	typeSrc.FreeSource( );

	// if this is an array
	if( typeString[typeString.Length( ) - 1] == ']' ) {
		for( i = typeString.Length( ); i > 0 && typeString[i - 1] != '['; i-- ) {
		}
		int num = atoi( &typeString[ i ] );
		Str listVarType = typeString;
		listVarType.CapLength( i - 1 );
		typeSize = 0;
		for( i = 0; i < num; i++ ) {
			Str listVarName = va( "%s[ %d ]", varName, i );
			int size = WriteVariable_r( varPtr, listVarName, listVarType, scope, prefix, pointerDepth );
			typeSize += size;
			if( size == -1 ) {
				break;
			}
			varPtr =( void * )( ( ( byte * ) varPtr ) + size );
		}
		return typeSize;
	}

	// if this is a pointer
	isPointer = 0;
	for( i = typeString.Length( ); i > 0 && typeString[i - 1] == '*'; i -= 2 ) {
		if( varPtr ==( void * )0xCDCDCDCD ||( varPtr != NULL && * ( ( unsigned long * )varPtr ) == 0xcdcdcdcd ) ) {
			Common::Com_DPrintf( "%s%s::%s%s references uninitialized memory", prefix, scope, varName, "" );
			return typeSize;
		}
		if( varPtr != NULL  ) {
			varPtr = * ( ( void * * )varPtr );
		}
		isPointer++;
	}

	if( varPtr == NULL ) {
		Write( varName, varType, scope, prefix, "", "<NULL>", varPtr, 0 );
		return sizeof( void * );
	}

	typeSrc.LoadMemory( typeString, typeString.Length( ), varName );

	if( !typeSrc.ReadToken( &token ) ) {
		Write( varName, varType, scope, prefix, "", va( "<unknown type '%s'>", varType ), varPtr, 0 );
		return -1;
	}

	// get full type
	while( typeSrc.CheckTokenString( "::" ) ) {
		Token newToken;
		typeSrc.ExpectTokenType( TT_NAME, 0, &newToken );
		token += "::" + newToken;
	}

	if( token == "signed" ) {

		if( !typeSrc.ReadToken( &token ) ) {
			Write( varName, varType, scope, prefix, "", va( "<unknown type '%s'>", varType ), varPtr, 0 );
			return -1;
		}
		if( token == "char" ) {

			typeSize = sizeof( signed char );
			Write( varName, varType, scope, prefix, "", va( "%d", * ( ( signed char * )varPtr ) ), varPtr, typeSize );

		} else if( token == "short" ) {

			typeSize = sizeof( signed short );
			Write( varName, varType, scope, prefix, "", va( "%d", * ( ( signed short * )varPtr ) ), varPtr, typeSize );

		} else if( token == "int" ) {

			typeSize = sizeof( signed int );
			Write( varName, varType, scope, prefix, "", va( "%d", * ( ( signed int * )varPtr ) ), varPtr, typeSize );

		} else if( token == "long" ) {

			typeSize = sizeof( signed long );
			Write( varName, varType, scope, prefix, "", va( "%ld", * ( ( signed long * )varPtr ) ), varPtr, typeSize );

		} else {

			Write( varName, varType, scope, prefix, "", va( "<unknown type '%s'>", varType ), varPtr, 0 );
			return -1;
		}

	} else if( token == "unsigned" ) {

		if( !typeSrc.ReadToken( &token ) ) {
			Write( varName, varType, scope, prefix, "", va( "<unknown type '%s'>", varType ), varPtr, 0 );
			return -1;
		}
		if( token == "char" ) {

			typeSize = sizeof( unsigned char );
			Write( varName, varType, scope, prefix, "", va( "%d", * ( ( unsigned char * )varPtr ) ), varPtr, typeSize );

		} else if( token == "short" ) {

			typeSize = sizeof( unsigned short );
			Write( varName, varType, scope, prefix, "", va( "%d", * ( ( unsigned short * )varPtr ) ), varPtr, typeSize );

		} else if( token == "int" ) {

			typeSize = sizeof( unsigned int );
			Write( varName, varType, scope, prefix, "", va( "%d", * ( ( unsigned int * )varPtr ) ), varPtr, typeSize );

		} else if( token == "long" ) {

			typeSize = sizeof( unsigned long );
			Write( varName, varType, scope, prefix, "", va( "%lu", * ( ( unsigned long * )varPtr ) ), varPtr, typeSize );

		} else {

			Write( varName, varType, scope, prefix, "", va( "<unknown type '%s'>", varType ), varPtr, 0 );
			return -1;
		}

	} else if( token == "byte" ) {

		typeSize = sizeof( byte );
		Write( varName, varType, scope, prefix, "", va( "%d", * ( ( byte * )varPtr ) ), varPtr, typeSize );

	} else if( token == "word" ) {

		typeSize = sizeof( word );
		Write( varName, varType, scope, prefix, "", va( "%d", * ( ( word * )varPtr ) ), varPtr, typeSize );

	} else if( token == "dword" ) {

		typeSize = sizeof( dword );
		Write( varName, varType, scope, prefix, "", va( "%d", * ( ( dword * )varPtr ) ), varPtr, typeSize );

	} else if( token == "bool" ) {

		typeSize = sizeof( bool );
		Write( varName, varType, scope, prefix, "", va( "%d", * ( ( bool * )varPtr ) ), varPtr, typeSize );

	} else if( token == "char" ) {

		typeSize = sizeof( char );
		Write( varName, varType, scope, prefix, "", va( "%d", * ( ( char * )varPtr ) ), varPtr, typeSize );

	} else if( token == "short" ) {

		typeSize = sizeof( short );
		Write( varName, varType, scope, prefix, "", va( "%d", * ( ( short * )varPtr ) ), varPtr, typeSize );

	} else if( token == "int" ) {

		typeSize = sizeof( int );
		Write( varName, varType, scope, prefix, "", va( "%d", * ( ( int * )varPtr ) ), varPtr, typeSize );

	} else if( token == "long" ) {

		typeSize = sizeof( long );
		Write( varName, varType, scope, prefix, "", va( "%ld", * ( ( long * )varPtr ) ), varPtr, typeSize );

	} else if( token == "float" ) {

		typeSize = sizeof( float );
		Write( varName, varType, scope, prefix, "", Str( *( ( float * )varPtr ) ).c_str( ), varPtr, typeSize );

	} else if( token == "double" ) {

		typeSize = sizeof( double );
		Write( varName, varType, scope, prefix, "", Str( ( float )* ( ( double * )varPtr ) ).c_str( ), varPtr, typeSize );

	} else if( token == "List" ) {

		List<int> * list =( ( List<int> * )varPtr );
		Write( varName, varType, scope, prefix, ".num", va( "%d", list->Num( ) ), NULL, 0 );
		// NOTE: we don't care about the amount of memory allocated
		//Write( varName, varType, scope, prefix, ".size", va( "%d", list->Size( ) ), NULL, 0 );
		Write( varName, varType, scope, prefix, ".granularity", va( "%d", list->GetGranularity( ) ), NULL, 0 );

		if( list->Num( ) && ParseTemplateArguments( typeSrc, templateArgs ) ) {
			void * listVarPtr = list->Ptr( );
			for( i = 0; i < list->Num( ); i++ ) {
				Str listVarName = va( "%s[ %d ]", varName, i );
				int size = WriteVariable_r( listVarPtr, listVarName, templateArgs, scope, prefix, pointerDepth );
				if( size == -1 ) {
					break;
				}
				listVarPtr =( void * )( ( ( byte * ) listVarPtr ) + size );
			}
		}

		typeSize = sizeof( List<int> );

	} else if( token == "StaticList" ) {

		StaticList<int, 1> * list =( ( StaticList<int, 1> * )varPtr );
		Write( varName, varType, scope, prefix, ".num", va( "%d", list->Num( ) ), NULL, 0 );

		int totalSize = 0;
		if( list->Num( ) && ParseTemplateArguments( typeSrc, templateArgs ) ) {
			void * listVarPtr = list->Ptr( );
			for( i = 0; i < list->Num( ); i++ ) {
				Str listVarName = va( "%s[ %d ]", varName, i );
				int size = WriteVariable_r( listVarPtr, listVarName, templateArgs, scope, prefix, pointerDepth );
				if( size == -1 ) {
					break;
				}
				totalSize += size;
				listVarPtr =( void * )( ( ( byte * ) listVarPtr ) + size );
			}
		}

		typeSize = sizeof( int ) + totalSize;

	} else if( token == "LinkList" ) {

		// FIXME: implement
		typeSize = sizeof( LinkList<Str> );
		Write( varName, varType, scope, prefix, "", va( "<unknown type '%s'>", varType ), NULL, 0 );

	} else if( token == "Str" ) {

		typeSize = sizeof( Str );

		const Str * str =( ( Str * )varPtr );
		Write( varName, varType, scope, prefix, "", OutputString( str->c_str( ) ), str->c_str( ), str->Length( ) );

	} else if( token == "StrList" ) {

		typeSize = sizeof( StrList );

		const StrList * list =( ( StrList * )varPtr );
		if( list->Num( ) ) {
			for( i = 0; i < list->Num( ); i++ ) {
				Write( varName, varType, scope, prefix, va( "[ %d ]", i ), OutputString( ( *list )[ i ].c_str( ) ), ( *list )[ i ].c_str( ), ( *list )[ i ].Length( ) );
			}
		} else {
			Write( varName, varType, scope, prefix, "", "<empty>", NULL, 0 );
		}

	} else if( token == "Dict" ) {

		typeSize = sizeof( Dict );

		const Dict * dict =( ( Dict * )varPtr );
		if( dict->GetNumKeyVals( ) ) {
			for( i = 0; i < dict->GetNumKeyVals( ); i++ ) {
				const KeyValue * kv = dict->GetKeyVal( i );
				Write( varName, varType, scope, prefix, va( "[ %d ]", i ), va( "\'%s\'  \'%s\'", OutputString( kv->GetKey( ).c_str( ) ), OutputString( kv->GetValue( ).c_str( ) ) ), NULL, 0 );
			}
		} else {
			Write( varName, varType, scope, prefix, "", "<empty>", NULL, 0 );
		}

	} else if( token == "ScriptBool" ) {

		typeSize = sizeof( ScriptBool );

		const ScriptBool * scriptBool =( ( ScriptBool * )varPtr );
		if( scriptBool->IsLinked( ) ) {
			Write( varName, varType, scope, prefix, "", ( *scriptBool != 0 ) ? "true" : "false", varPtr, typeSize );
		} else {
			Write( varName, varType, scope, prefix, "", "<not linked>", varPtr, typeSize );
		}

	} else {

		const classTypeInfo_t * classTypeInfo = FindClassInfo( scope +( "::" + token ) );
		if( classTypeInfo == NULL ) {
			classTypeInfo = FindClassInfo( token );
		}
		if( classTypeInfo != NULL ) {

			typeSize = classTypeInfo->size;

			if( !isPointer ) {

				char newPrefix[ 1024 ];
				Str::snPrintf( newPrefix, sizeof( newPrefix ), "%s%s::%s.", prefix, scope, varName );
				WriteClass_r( varPtr, "", token, token, newPrefix, pointerDepth );

			} else {

				Write( varName, varType, scope, prefix, "", va( "<pointer type '%s' not listed>", varType ), NULL, 0 );
				return -1;
			}
		} else {
			const enumTypeInfo_t * enumTypeInfo = FindEnumInfo( scope +( "::" + token ) );
			if( enumTypeInfo == NULL ) {
				enumTypeInfo = FindEnumInfo( token );
			}
			if( enumTypeInfo != NULL ) {

				typeSize = sizeof( int );	// NOTE: assuming sizeof( enum ) is sizeof( int )

				for( i = 0; enumTypeInfo->values[ i ].name != NULL; i++ ) {
					if( *( ( int * )varPtr ) == enumTypeInfo->values[ i ].value ) {
						break;
					}
				}
				if( enumTypeInfo->values[ i ].name != NULL ) {
					Write( varName, varType, scope, prefix, "", enumTypeInfo->values[ i ].name, NULL, 0 );
				} else {
					Write( varName, varType, scope, prefix, "", va( "%d", * ( ( int * )varPtr ) ), NULL, 0 );
				}

			} else {
				Write( varName, varType, scope, prefix, "", va( "<unknown type '%s'>", varType ), NULL, 0 );
				return -1;
			}
		}
	}

	i = 0;
	do {
		if( *( ( unsigned long * )varPtr ) == 0xCDCDCDCD ) {
			Common::Com_DPrintf( "%s%s::%s%s uses uninitialized memory", prefix, scope, varName, "" );
			break;
		}
	} while( ++i < typeSize );

	if( isPointer ) {
		return sizeof( void * );
	}
	return typeSize;
}

/*
================
TypeInfoTools::WriteClass_r
================
*/
void TypeInfoTools::WriteClass_r( const void * classPtr, const char * className, const char * classType, const char * scope, const char * prefix, const int pointerDepth ) {
	int i;

	const classTypeInfo_t * classInfo = FindClassInfo( classType );
	if( !classInfo ) {
		return;
	}
	if( *classInfo->superType != '\0' ) {
		WriteClass_r( classPtr, className, classInfo->superType, scope, prefix, pointerDepth );
	}

	for( i = 0; classInfo->variables[ i ].name != NULL; i++ ) {
		const classVariableInfo_t &classVar = classInfo->variables[ i ];

		void * varPtr =( void * )( ( ( byte * )classPtr ) + classVar.offset );

		WriteVariable_r( varPtr, classVar.name, classVar.type, classType, prefix, pointerDepth );
	}
}

/*
================
TypeInfoTools::WriteGameState
================
*/
void TypeInfoTools::WriteGameState( const char * fileName ) {
	Common::Com_DPrintf( "w00t\n" );
}

/*
================
TypeInfoTools::CompareGameState
================
*/
void TypeInfoTools::CompareGameState( const char * fileName ) {
	Common::Com_DPrintf( "w00t\n" );
}

/*
================
WriteGameState_f
================
*/
void WriteGameState_f( const CmdArgs &args ) {
	Str fileName;

	if( args.Argc( ) > 1 ) {
		fileName = args.Argv( 1 );
	} else {
		fileName = "GameState.txt";
	}
	fileName.SetFileExtension( "gameState.txt" );

	TypeInfoTools::WriteGameState( fileName );
}

/*
================
CompareGameState_f
================
*/
void CompareGameState_f( const CmdArgs &args ) {
	Str fileName;

	if( args.Argc( ) > 1 ) {
		fileName = args.Argv( 1 );
	} else {
		fileName = "GameState.txt";
	}
	fileName.SetFileExtension( "gameState.txt" );

	TypeInfoTools::CompareGameState( fileName );
}

/*
================
PrintType
================
*/
void PrintType( const void * typePtr, const char * typeName ) {
	TypeInfoTools::PrintType( typePtr, typeName );
}

/*
================
InitTypeVariables
================
*/
void InitTypeVariables( const void * typePtr, const char * typeName, int value ) {
	TypeInfoTools::InitTypeVariables( typePtr, typeName, value );
}

/*
================
ListTypeInfo_f
================
*/
int SortTypeInfoByName( const int * a, const int * b ) {
	return Str::Icmp( classTypeInfo[* a].typeName, classTypeInfo[* b].typeName );
}

int SortTypeInfoBySize( const int * a, const int * b ) {
	if( classTypeInfo[* a].size < classTypeInfo[* b].size ) {
		return -1;
	}
	if( classTypeInfo[* a].size > classTypeInfo[* b].size ) {
		return 1;
	}
	return 0;
}

void ListTypeInfo_f( const CmdArgs &args ) {
	int i, j;
	List<int> index;

	Common::Com_DPrintf( "%-32s : %-32s size( B )\n", "type name", "super type name" );
	for( i = 0; classTypeInfo[ i ].typeName != NULL; i++ ) {
		index.Append( i );
	}

	if( args.Argc( ) > 1 && Str::Icmp( args.Argv( 1 ), "size" ) == 0 ) {
		index.Sort( SortTypeInfoBySize );
	} else {
		index.Sort( SortTypeInfoByName );
	}

	for( i = 0; classTypeInfo[ i ].typeName != NULL; i++ ) {
		j = index[ i ];
		Common::Com_DPrintf( "%-32s : %-32s %d\n", classTypeInfo[ j ].typeName, classTypeInfo[ j ].superType, classTypeInfo[ j ].size );
	}
}
