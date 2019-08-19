#ifndef SVAR_H
#define SVAR_H

enum {
	SVar_None = 0,
	SVar_Float,
	SVar_Vec2,
	SVar_Vec3,
	SVar_Vec4,
	SVar_Plane,
	SVar_Mat2,
	SVar_Mat3,
	SVar_Mat4,
	SVar_Sampler2D,
};

class GLTexture;

// // //// // //// // //// //
/// SVar
// //// // //// // //// //

class SVar : public NamedObject {
protected:
	int									d_svarType;
	bool								d_svarIsGlobal;
	int									d_svarDataSize;
	byte *								d_svarData;

	void								Register( );

	/// global SVar lookup table
	static DynamicBlockAlloc< byte, 1 << 18, 128 >	g_shaderDataAllocator;
	static CANamedList< SVar *, 256 >	g_shaderVariableList;

public:
										~SVar( );

	bool								operator ==( int with ) const		{ return d_svarType == with; }
	bool								operator !=( int with ) const		{ return d_svarType != with; }
	bool								operator ==( SVar * with ) const	{ return d_svarType == with->d_svarType; }

	void								operator =( SVar * src );
	/// sets the internal data
	void								operator =( const float src );
	void								operator =( const Vec2 & src );
	void								operator =( const Vec3 & src );
	void								operator =( const Vec4 & src );
	void								operator =( const Plane & src );
	void								operator =( const Mat2 & src );
	void								operator =( const Mat3 & src );
	void								operator =( const Mat4 & src );
	void								operator =( GLTexture * src );
	/// creates a copy from 'this'
	virtual SVar *						CreateParam( ) = 0;
	/// uploads the SVar data into the VGA
	virtual void						Write( int localtion ) = 0;
	/// resets states to default
	virtual void						Reset( int location ) = 0;
	/// finds an SVar with the given name on the global list
	static SVar &						Find( const Str & name );
	/// checks the gives svar is globally avaible
	static bool							CheckGlobal( const Str & name );
	/// used for sorting surfaces local shader variable list
	static int							TypeAndNameCompare( SVar * const * a,  SVar * const * b );

	static int							Sampler2DComapre( SVar * const * a,  SVar * const * b );
};

// // //// // //// // //// //
/// SVarT
// //// // //// // //// //

template< class type, int type_enum, bool reg_global > class SVarT : public SVar {
private:
	void								AllocInternal( );
public:
										SVarT( const Str & name, const type & initial_value, bool global_register = reg_global );
										SVarT( const Str & name, bool global_register = reg_global );
										SVarT( const SVarT & other );

	void								operator =( const type & src );

	type &								GetDataRef( );
	const type &						GetDataRef( ) const;

	virtual SVar *						CreateParam( );
	virtual void						Write( int localtion );
	virtual void						Reset( int location );
};

// // //// // //// // //// //
/// SVarT template table
// //// // //// // //// //

typedef SVarT< float, SVar_Float, true >						SVarFloat;
typedef SVarT< Vec2, SVar_Vec2, true >							SVarVec2;
typedef SVarT< Vec3, SVar_Vec3, true >							SVarVec3;
typedef SVarT< Vec4, SVar_Vec4, true >							SVarVec4;
typedef SVarT< Plane, SVar_Plane, true >						SVarPlane;
typedef SVarT< Mat2, SVar_Mat2, true >							SVarMat2;
typedef SVarT< Mat3, SVar_Mat3, true >							SVarMat3;
typedef SVarT< Mat4, SVar_Mat4, true >							SVarMat4;
typedef SVarT< GLTexture, SVar_Sampler2D, false >				SVarSampler2D;

// // //// // //// // //// //
// SVar
// //// // //// // //// //

INLINE void SVar::operator =( SVar * src ) {
	assert( d_svarType == src->d_svarType );
	Common::Com_Memcpy( d_svarData, src->d_svarData, d_svarDataSize );
}

INLINE SVar & SVar::Find( const Str & name ) {
#ifdef _DEBUG
	SVar * svar = g_shaderVariableList.FindByName( name );
	assert( svar );
	return *svar;
#else
	return *g_shaderVariableList.FindByName( name );
#endif
}

INLINE bool SVar::CheckGlobal( const Str & name ) {
	return g_shaderVariableList.FindByName( name ) != NULL;
}

// // //// // //// // //// //
// SVarT
// //// // //// // //// //

template< class type, int type_enum, bool reg_global >
SVar * SVarT< type, type_enum, reg_global >::CreateParam( ) {
	SVarT * svarCopy = new SVarT( *this );
	return ( SVar * )svarCopy;
}

template< class type, int type_enum, bool reg_global >
INLINE void SVarT< type, type_enum, reg_global >::AllocInternal( ) {
	d_svarType = type_enum;
	d_svarDataSize = sizeof( type );
	d_svarData = g_shaderDataAllocator.Alloc( d_svarDataSize );
	Common::Com_Memset( d_svarData, 0, d_svarDataSize );
}

template< class type, int type_enum, bool reg_global >
INLINE type & SVarT< type, type_enum, reg_global >::GetDataRef( ) {
	return *( ( type * )d_svarData );
}

template< class type, int type_enum, bool reg_global >
INLINE const type & SVarT< type, type_enum, reg_global >::GetDataRef( ) const {
	return *( ( type * )d_svarData );
}

template< class type, int type_enum, bool reg_global >
INLINE void SVarT< type, type_enum, reg_global >::operator =( const type & src ) {
	*( ( type * )d_svarData ) = src;
}

template< class type, int type_enum, bool reg_global >
SVarT< type, type_enum, reg_global >::SVarT( const Str & name, const type & initial_value, bool global_register ) {
	d_name = name;
	AllocInternal( );
	GetDataRef( ) = initial_value;
	if( global_register )
		Register( );
	d_svarIsGlobal = global_register;
}

template< class type, int type_enum, bool reg_global >
SVarT< type, type_enum, reg_global >::SVarT( const Str & name, bool global_register ) {
	d_name = name;
	AllocInternal( );
	if( global_register )
		Register( );
	d_svarIsGlobal = global_register;
}

template< class type, int type_enum, bool reg_global >
SVarT< type, type_enum, reg_global >::SVarT( const SVarT & other ) {
	d_name = other.d_name;
	AllocInternal( );
	*( ( type * )d_svarData ) = other.GetDataRef( );
	d_svarIsGlobal = false;
}

// // //// // //// // //// //
// GLShader
// //// // //// // //// //

INLINE void GLShader::WriteSVars( ) {
	for( int i = 0, j = 0; i < d_inputSVars.Num( ); i++ ) {
		if( ( j < d_inputSamplers.Num( ) ) && ( d_inputSamplers[ j ]->GetUniformLocation( ) == i ) )
			d_inputSamplers[ j++ ]->ActivateTextureUnit( );
		d_inputSVars[ i ]->Write( i );
	}
}

#endif
