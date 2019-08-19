#include "precompiled.h"
#pragma hdrstop


/*
=============
DrawVert::Normalize
=============
*/
void DrawVert::Normalize( void ) {
	normal.Normal( ).Normalize( );
#if 0
	tangents[1].Cross( normal, tangents[0] );
	tangents[1].Normalize();
	tangents[0].Cross( tangents[1], normal );
	tangents[0].Normalize();
#endif
}
