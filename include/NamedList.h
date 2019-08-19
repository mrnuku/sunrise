#ifndef NAMEDLIST_H
#define NAMEDLIST_H

// // //// // //// // //// //
/// SingleObject
// //// // //// // //// //

template< class type > class SingleObject {
protected:
	static type *									g_objectPtr;

													SingleObject( );
													~SingleObject( );

public:
	type *											GetSinglePtr( );
	type &											GetSingleRef( );
};

// // //// // //// // //// //
/// NamedObject
//
/// Common base class for classes with a run-time name
// //// // //// // //// //

class NamedObject {
protected:
	Str												d_name;

public:
	const Str &										GetName( ) const;
	void											SetName( const Str & str );
};

// // //// // //// // //// //
/// NamedList
//
/// NamedList can lookup run-time named classes with a hash
// //// // //// // //// //

template< class type > class NamedList : public List< type > {
private:
	HashIndex										d_nameHash;

public:
	/// append element
	int												Append( const type & obj );
	bool											Remove( type const & obj );

	void											Clear( );
	void											DeleteContents( bool clear );

	/// special list find function with a fast hash lookup table
	type *											FindByName( const Str & name );
	type											FindByName2( const Str & name );
};

// // //// // //// // //// //
/// CANamedList
// //// // //// // //// //

template< class type, int static_size = 1 > class CANamedList : public CAList< type, static_size > {
private:
	HashIndex										d_nameHash;

public:
	/// append element
	int												Append( const type & obj );
	bool											Remove( type const & obj );

	void											Clear( );
	void											DeleteContents( bool clear );

	/// special list find function with a fast hash lookup table
	type											FindByName( const Str & name ) const;
	int												IndexByName( const Str & name ) const;
};

// // //// // //// // //// //
// SingleObject
// //// // //// // //// //

template< class type > INLINE SingleObject< type >::SingleObject( ) {
	assert( !g_objectPtr );
	g_objectPtr = static_cast< type * >( this );
}

template< class type > INLINE SingleObject< type >::~SingleObject( ) {
	assert( g_objectPtr );
	g_objectPtr = NULL;
}

template< class type > INLINE type * SingleObject< type >::GetSinglePtr( ) {
	return g_objectPtr;
}

template< class type > INLINE type & SingleObject< type >::GetSingleRef( ) {
	return *g_objectPtr;
}

// // //// // //// // //// //
// NamedObject
// //// // //// // //// //

INLINE const Str & NamedObject::GetName( ) const {
	return d_name;
}

INLINE void NamedObject::SetName( const Str & str ) {
	d_name = str;
}

// // //// // //// // //// //
// NamedList
// //// // //// // //// //

template< class type >
INLINE int NamedList< type >::Append( const type & obj ) {
	int index = List< type >::Append( obj );
	d_nameHash.Add( d_nameHash.GenerateKey( dynamic_cast< NamedObject * >( obj )->GetName( ), true ), index );
	return index;
}

template< class type >
INLINE bool NamedList< type >::Remove( type const & obj ) {
	int index = List< type >::FindIndex( obj );
	if( index != -1 ) {
		List< type >::RemoveIndex( index );
		d_nameHash.RemoveIndex( d_nameHash.GenerateKey( dynamic_cast< NamedObject * >( obj )->GetName( ), true ), index );
		return true;
	}
	return false;
}

template< class type >
INLINE void NamedList< type >::Clear( ) {
	List< type >::Clear( );
	d_nameHash.Clear( );
}

template< class type >
INLINE void NamedList< type >::DeleteContents( bool clear ) {
	List< type >::DeleteContents( clear );
	if( clear )
		d_nameHash.Clear( );
}

template< class type >
INLINE type * NamedList< type >::FindByName( const Str & name ) {
	for( int i = d_nameHash.First( d_nameHash.GenerateKey( name, true ) ); i != -1; i = d_nameHash.Next( i ) ) {
		type * data = &List< type >::d_list[ i ];
		if( Str::Cmp( dynamic_cast< NamedObject * >( *data )->GetName( ), name ) == 0 )
			return data;
	}
	return NULL;
}

template< class type > INLINE type NamedList< type >::FindByName2( const Str & name ) {
	for( int i = d_nameHash.First( d_nameHash.GenerateKey( name, true ) ); i != -1; i = d_nameHash.Next( i ) ) {
		type data = List< type >::d_list[ i ];
		if( dynamic_cast< NamedObject * >( data )->GetName( ) == name )
			return data;
	}
	return NULL;
}

// // //// // //// // //// //
// CANamedList
// //// // //// // //// //

template< class type, int static_size >
INLINE int CANamedList< type, static_size >::Append( const type & obj ) {
	int index = CAList< type, static_size >::Append( obj );
	d_nameHash.Add( d_nameHash.GenerateKey( dynamic_cast< NamedObject * >( obj )->GetName( ), true ), index );
	return index;
}

template< class type, int static_size >
INLINE bool CANamedList< type, static_size >::Remove( type const & obj ) {
	int index = CAList< type, static_size >::FindIndex( obj );
	if( index != -1 ) {
		CAList< type, static_size >::RemoveIndex( index );
		d_nameHash.RemoveIndex( d_nameHash.GenerateKey( dynamic_cast< NamedObject * >( obj )->GetName( ), true ), index );
		return true;
	}
	return false;
}

template< class type, int static_size >
INLINE void CANamedList< type, static_size >::Clear( ) {
	CAList< type, static_size >::Clear( );
	d_nameHash.Clear( );
}

template< class type, int static_size >
INLINE void CANamedList< type, static_size >::DeleteContents( bool clear ) {
	CAList< type, static_size >::DeleteContents( clear );
	if( clear )
		d_nameHash.Clear( );
}

template< class type, int static_size >
INLINE type CANamedList< type, static_size >::FindByName( const Str & name ) const {
	for( int i = d_nameHash.First( d_nameHash.GenerateKey( name, true ) ); i != -1; i = d_nameHash.Next( i ) ) {
		type data = CAList< type, static_size >::d_list[ i ];
		if( dynamic_cast< NamedObject * >( data )->GetName( ) == name )
			return data;
	}
	return NULL;
}

template< class type, int static_size >
INLINE int CANamedList< type, static_size >::IndexByName( const Str & name ) const {
	for( int i = d_nameHash.First( d_nameHash.GenerateKey( name, true ) ); i != -1; i = d_nameHash.Next( i ) ) {
		if( dynamic_cast< NamedObject * >( CAList< type, static_size >::d_list[ i ] )->GetName( ) == name )
			return i;
	}
	return -1;
}

#endif
