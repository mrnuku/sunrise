#include <stdio.h>
#include <string.h>

#define __stosb( dest, fill_value, count ) __asm__( "rep stosb" : : "c" ( count ), "a" ( fill_value ), "D" ( dest ) : )
#define __stosd( dest, fill_value, count ) __asm__( "rep stosl" : : "c" ( count ), "a" ( fill_value ), "D" ( dest ) : )

#define __movsb( dest, src, count ) __asm__( "rep movsb" : : "c" ( count ), "S" ( src ), "D" ( dest ) : )
#define __movsd( dest, src, count ) __asm__( "rep movsl" : : "c" ( count ), "S" ( src ), "D" ( dest ) : )

int main( int argc, char * argv[ ] ) {

	char tests[ 32 ];
	
	strcpy( tests, "aabbccddeeffgghh\n" );
	printf( tests );
	
	__stosd( tests, 0x21222324, 1 );
	printf( tests );
	
	__stosb( tests, 'x', 4 );
	printf( tests );
	
	const char * tts = "meow";
	__movsb( tests, tts, 4 );
	printf( tests );
	
	int tti = 0x24232221;
	__movsd( tests, &tti, 1 );
	printf( tests );
	
	return 0;
}
