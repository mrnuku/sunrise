// Copyright ( C) 2004 Id Software, Inc.
//

#ifndef __STRLIST_H__
#define __STRLIST_H__

/*
===============================================================================

	StrList

===============================================================================
*/

typedef List<Str> StrList;
typedef List<Str*> StrPtrList;
typedef Str *StrPtr;

/*
================
ListSortCompare<StrPtr>

Compares two pointers to strings. Used to sort a list of string pointers alphabetically in List<Str>::Sort.
================
*/
template<>
INLINE int ListSortCompare<StrPtr>( const StrPtr *a, const StrPtr *b ) {
	return ( *a )->Icmp( **b );
}

/*
================
StrList::Sort

Sorts the list of strings alphabetically. Creates a list of pointers to the actual strings and sorts the
pointer list. Then copies the strings into another list using the ordered list of pointers.
================
*/
template<>
INLINE void StrList::Sort( cmp_t *compare ) {
	int i;

	if ( !d_num ) {
		return;
	}

	List<Str>		other;
	List<StrPtr>	pointerList;

	pointerList.SetNum( d_num );
	for( i = 0; i < d_num; i++ ) {
		pointerList[ i ] = &( *this )[ i ];
	}

	pointerList.Sort( ListSortCompare );

	other.SetNum( d_num );
	other.SetGranularity( d_granularity );
	for( i = 0; i < other.Num( ); i++ ) {
		other[ i ] = *pointerList[ i ];
	}

	this->Swap( other );
}

/*
================
StrList::SortSubSection

Sorts a subsection of the list of strings alphabetically.
================
*/
template<>
INLINE void StrList::SortSubSection( int startIndex, int endIndex, cmp_t *compare ) {
	int i, s;

	if ( !d_num ) {
		return;
	}
	if ( startIndex < 0 ) {
		startIndex = 0;
	}
	if ( endIndex >= d_num ) {
		endIndex = d_num - 1;
	}
	if ( startIndex >= endIndex ) {
		return;
	}

	List<Str>		other;
	List<StrPtr>	pointerList;

	s = endIndex - startIndex + 1;
	other.SetNum( s );
	pointerList.SetNum( s );
	for( i = 0; i < s; i++ ) {
		other[ i ] = ( *this )[ startIndex + i ];
		pointerList[ i ] = &other[ i ];
	}

	pointerList.Sort( ListSortCompare );

	for( i = 0; i < s; i++ ) {
		( *this)[ startIndex + i ] = *pointerList[ i ];
	}
}

/*
================
StrList::Size
================
*/
template<>
INLINE size_t StrList::Size( ) const {
	size_t s;
	int i;

	s = sizeof( *this );
	for( i = 0; i < Num( ); i++ ) {
		s += ( *this )[ i ].Size( );
	}

	return s;
}

/*
===============================================================================

	StrList path sorting

===============================================================================
*/

/*
================
ListSortComparePaths

Compares two pointers to strings. Used to sort a list of string pointers alphabetically in List<Str>::Sort.
================
*/
template<class StrPtr>
INLINE int ListSortComparePaths( const StrPtr *a, const StrPtr *b ) {
	return ( *a )->IcmpPath( **b );
}

/*
================
StrListSortPaths

Sorts the list of path strings alphabetically and makes sure folders come first.
================
*/
INLINE void StrListSortPaths( StrList &list ) {
	int i;

	if ( !list.Num( ) ) {
		return;
	}

	List<Str>		other;
	List<StrPtr>	pointerList;

	pointerList.SetNum( list.Num( ) );
	for( i = 0; i < list.Num( ); i++ ) {
		pointerList[ i ] = &list[ i ];
	}

	pointerList.Sort( ListSortComparePaths<StrPtr> );

	other.SetNum( list.Num( ) );
	other.SetGranularity( list.GetGranularity( ) );
	for( i = 0; i < other.Num( ); i++ ) {
		other[ i ] = *pointerList[ i ];
	}

	list.Swap( other );
}

#endif /* !__STRLIST_H__ */
