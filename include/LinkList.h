#ifndef LINKLIST_H
#define LINKLIST_H

// // //// // //// // //// //
/// LLNode
//
/// Circular linked list template
// //// // //// // //// //
template< class type > class LLNode {
protected:
	type *				d_head;
	type *				d_next;
	type *				d_prev;

	bool				Check( const type * parent ) const;

public:
						/// Node is initialized to be the head of an empty list
						LLNode( );

						/// Removes the node from the list, or if it's the head of a list, removes all the nodes from the list.
						~LLNode( );

	/// Returns true if the list is empty.
	bool				IsListEmpty( ) const;

	/// Returns true if the node is in a list.  If called on the head of a list, will always return false.
	bool				InList( ) const;

	/// Returns the number of nodes in the list.
	int					Num( ) const;

	/// If node is the head of the list, clears the list.  Otherwise it just removes the node from the list.
	void				Clear( );
	
	/// Places the node before the existing node in the list.  If the existing node is the head, then the new node is placed at the end of the list.
	void				InsertBefore( type * node );

	/// Places the node after the existing node in the list.  If the existing node is the head, then the new node is placed at the beginning of the list.
	void				InsertAfter( type * node );

	/// Adds node at the end of the list
	void				AddToEnd( type * node );

	/// Adds node at the beginning of the list
	void				AddToFront( type * node );

	/// Removes node from list
	void				Remove( );

	/// Returns the next object in the list, or NULL if at the end.
	type *				Next( ) const;

	/// Returns the previous object in the list, or NULL if at the beginning.
	type *				Prev( ) const;

	/// Returns the head of the list.  If the node isn't in a list, it returns a pointer to itself.
	type *				Head( ) const;
};

template< class type >
INLINE LLNode< type >::LLNode( ) {
	d_head	= ( type * )this;	
	d_next	= ( type * )this;
	d_prev	= ( type * )this;
}

template< class type >
INLINE LLNode< type >::~LLNode( ) {
	Clear( );
}

template< class type >
INLINE bool LLNode< type >::Check( const type * parent ) const {
	bool parentIsEqual = true;
	assert( ( parentIsEqual = ( ( type * )this == parent ) ) );
	return parentIsEqual;
}

template< class type >
INLINE bool LLNode< type >::IsListEmpty( ) const {
	return d_head->d_next == d_head;
}

template< class type >
INLINE bool LLNode< type >::InList( ) const {
	return d_head != ( type * )this;
}

template< class type >
INLINE int LLNode< type >::Num( ) const {
	int num = 0;
	for( LLNode< type > * node = d_head->d_next; node != d_head; node = d_node->d_next )
		num++;
	return num;
}

template< class type >
INLINE void LLNode< type >::Clear( ) {
	if( d_head == ( type * )this ) {
		while( d_next != ( type * )this )
			d_next->Remove( );
	} else
		Remove( );
}

template< class type >
INLINE void LLNode< type >::Remove( ) {
	d_prev->d_next = d_next;
	d_next->d_prev = d_prev;
	d_next = ( type * )this;
	d_prev = ( type * )this;
	d_head = ( type * )this;
}

template< class type >
INLINE void LLNode< type >::InsertBefore( type * node ) {
	Remove( );
	d_next			= node;
	d_prev			= node->d_prev;
	node->d_prev	= ( type * )this;
	d_prev->d_next	= ( type * )this;
	d_head			= node->d_head;
}

template< class type >
INLINE void LLNode< type >::InsertAfter( type * node ) {
	Remove( );
	d_prev			= node;
	d_next			= node->d_next;
	node->d_next	= ( type * )this;
	d_next->d_prev	= ( type * )this;
	d_head			= node->d_head;
}

template< class type >
INLINE void LLNode< type >::AddToEnd( type * node ) {
	InsertBefore( node->d_head );
}

template< class type >
INLINE void LLNode< type >::AddToFront( type * node ) {
	InsertAfter( node->d_head );
}

template< class type >
INLINE type * LLNode< type >::Head( ) const {
	return d_head;
}

template< class type >
INLINE type * LLNode< type >::Next( ) const {
	if( d_next == d_head )
		return NULL;
	return d_next;
}

template< class type >
INLINE type * LLNode< type >::Prev( ) const {
	if( d_prev == d_head )
		return NULL;
	return d_prev;
}

#endif
