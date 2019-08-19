#include "precompiled.h"
#pragma hdrstop

template< int mat_rows, int mat_columns, class mat_class >
class MatrixOrderSwap {

public:
	static mat_class					Swap( const mat_class & src );
};

template< int mat_rows, int mat_columns, class mat_class >
mat_class MatrixOrderSwap< mat_rows, mat_columns, mat_class >::Swap( const mat_class & src ) {

	mat_class dest;
	for( int i = 0; i < mat_rows; i++ )
		for( int j = 0; j < mat_columns; j++ )
			dest[ j ][ i ] = src[ i ][ j ];
	return dest;
}

SVar::~SVar( ) {
	g_shaderDataAllocator.Free( d_svarData );
	d_svarType = SVar_None;
	d_svarDataSize = 0;
	if( d_svarIsGlobal )
		g_shaderVariableList.Remove( this );
}

int SVar::TypeAndNameCompare( SVar * const * a,  SVar * const * b ) {
	if( ( *a )->d_svarType > ( *b )->d_svarType )
		return -1;
	if( ( *a )->d_svarType < ( *b )->d_svarType )
		return 1;
	return ( ( *a )->d_name != ( *b )->d_name ) ? -1 : 0;
}

void SVar::Register( ) {

	SVar * sVar = g_shaderVariableList.FindByName( d_name );
	if( sVar ) {
		if( *sVar == this )
			Common::Com_DPrintf( "MaterialSystem::RegisterSVar: shader variable \"%s\" already registered.\n", this->GetName( ).c_str( ) );
		else
			Common::Com_DPrintf( "MaterialSystem::RegisterSVar: shader variable \"%s\" cant be registered by multiple types.\n", this->GetName( ).c_str( ) );
		return;
	}
	g_shaderVariableList.Append( this );
}

int SVar::Sampler2DComapre( SVar * const * a,  SVar * const * b ) {

	assert( ( *a )->d_svarType == SVar_Sampler2D && ( *b )->d_svarType == SVar_Sampler2D );
	GLuint aTexID = dynamic_cast< SVarSampler2D * >( *a )->GetDataRef( ).GetObjectID( );
	GLuint bTexID = dynamic_cast< SVarSampler2D * >( *b )->GetDataRef( ).GetObjectID( );
	if( aTexID > bTexID )
		return -1;
	if( aTexID < bTexID )
		return 1;
	return 0;
}

// // //// // //// // //// //
// SVarFloat
// //// // //// // //// //
void SVar::operator =( const float src ) {
	dynamic_cast< SVarFloat * >( this )->GetDataRef( ) = src;
}
template< > void SVarFloat::Reset( int location ) {
}
template< > void SVarFloat::Write( int localtion ) {
	GLImports::glUniform1fv( localtion, 1, ( GLfloat * )d_svarData );
}

// // //// // //// // //// //
// SVarVec2
// //// // //// // //// //
void SVar::operator =( const Vec2 & src ) {
	dynamic_cast< SVarVec2 * >( this )->GetDataRef( ) = src;
}
template< > void SVarVec2::Reset( int location ) {
}
template< > void SVarVec2::Write( int localtion ) {
	GLImports::glUniform2fv( localtion, 1, ( GLfloat * )d_svarData );
}

// // //// // //// // //// //
// SVarVec3
// //// // //// // //// //
void SVar::operator =( const Vec3 & src ) {
	dynamic_cast< SVarVec3 * >( this )->GetDataRef( ) = src;
}
template< > void SVarVec3::Reset( int location ) {
}
template< > void SVarVec3::Write( int localtion ) {
	GLImports::glUniform3fv( localtion, 1, ( GLfloat * )d_svarData );
}

// // //// // //// // //// //
// SVarVec4
// //// // //// // //// //
void SVar::operator =( const Vec4 & src ) {
	dynamic_cast< SVarVec4 * >( this )->GetDataRef( ) = src;
}
template< > void SVarVec4::Reset( int location ) {
}
template< > void SVarVec4::Write( int localtion ) {
	GLImports::glUniform4fv( localtion, 1, ( GLfloat * )d_svarData );
}

// // //// // //// // //// //
// SVarPlane
// //// // //// // //// //
void SVar::operator =( const Plane & src ) {
	dynamic_cast< SVarPlane * >( this )->GetDataRef( ) = src;
}
template< > void SVarPlane::Reset( int location ) {
}
template< > void SVarPlane::Write( int localtion ) {
	GLImports::glUniform4fv( localtion, 1, ( GLfloat * )d_svarData );
}

// // //// // //// // //// //
// SVarMat2
// //// // //// // //// //
void SVar::operator =( const Mat2 & src ) {
	dynamic_cast< SVarMat2 * >( this )->GetDataRef( ) = src;
}
template< > void SVarMat2::Reset( int location ) {
}
template< > void SVarMat2::Write( int localtion ) {
	GLImports::glUniformMatrix2fv( localtion, 1, false, ( GLfloat * )d_svarData );
}

// // //// // //// // //// //
// SVarMat3
// //// // //// // //// //
void SVar::operator =( const Mat3 & src ) {
	dynamic_cast< SVarMat3 * >( this )->GetDataRef( ) = src;
}
template< > void SVarMat3::Reset( int location ) {
}
template< > void SVarMat3::Write( int localtion ) {
	float temp[ 9 ];
	GetDataRef( )[ 0 ].CopyTo( &temp[ 0 ] );
	GetDataRef( )[ 1 ].CopyTo( &temp[ 3 ] );
	GetDataRef( )[ 2 ].CopyTo( &temp[ 6 ] );
	GLImports::glUniformMatrix3fv( localtion, 1, false, temp );
}

// // //// // //// // //// //
// SVarMat4
// //// // //// // //// //
void SVar::operator =( const Mat4 & src ) {
	dynamic_cast< SVarMat4 * >( this )->GetDataRef( ) = src;
}
template< > void SVarMat4::Reset( int location ) {
}
template< > void SVarMat4::Write( int localtion ) {
	Mat4 temp = MatrixOrderSwap< 4, 4, Mat4 >::Swap( GetDataRef( ) );
	GLImports::glUniformMatrix4fv( localtion, 1, false, temp.ToFloatPtr( ) );
}

// // //// // //// // //// //
// SVarSampler2D
// //// // //// // //// //
void SVar::operator =( GLTexture * src ) {
	if( !this )
		Common::Com_DPrintf( "SVar::operator = attempted to write NULL input with texture num %i!\n", src ? src->GetObjectID( ) : 0 );
	else
		dynamic_cast< SVarSampler2D * >( this )->GetDataRef( ) = *src;
}
template< > void SVarSampler2D::Reset( int location ) {
	GetDataRef( ).UnBind( );
}
template< > void SVarSampler2D::Write( int location ) {
	GetDataRef( ).Bind( );
}
