// Copyright ( C) 2004 Id Software, Inc.
//

#ifndef __HIERARCHY_H__
#define __HIERARCHY_H__

/*
==============================================================================

	Hierarchy

==============================================================================
*/

template< class type >
class Hierarchy {
public:

						Hierarchy( );
						~Hierarchy( );
	
	void				SetOwner( type *object );
	type *				Owner( ) const;
	void				ParentTo( Hierarchy &node );
	void				MakeSiblingAfter( Hierarchy &node );
	bool				ParentedBy( const Hierarchy &node ) const;
	void				RemoveFromParent( );
	void				RemoveFromHierarchy( );

	type *				GetParent( ) const;		// parent of this node
	type *				GetChild( ) const;			// first child of this node
	type *				GetSibling( ) const;		// next node with the same parent
	type *				GetPriorSibling( ) const;	// previous node with the same parent
	type *				GetNext( ) const;			// goes through all nodes of the hierarchy
	type *				GetNextLeaf( ) const;		// goes through all leaf nodes of the hierarchy

private:
	Hierarchy *		parent;
	Hierarchy *		sibling;
	Hierarchy *		child;
	type *				owner;

	Hierarchy<type>	*GetPriorSiblingNode( ) const;	// previous node with the same parent
};

/*
================
Hierarchy<type>::Hierarchy
================
*/
template< class type >
Hierarchy<type>::Hierarchy( ) {
	owner	= NULL;
	parent	= NULL;	
	sibling	= NULL;
	child	= NULL;
}

/*
================
Hierarchy<type>::~Hierarchy
================
*/
template< class type >
Hierarchy<type>::~Hierarchy( ) {
	RemoveFromHierarchy( );
}

/*
================
Hierarchy<type>::Owner

Gets the object that is associated with this node.
================
*/
template< class type >
type *Hierarchy<type>::Owner( ) const {
	return owner;
}

/*
================
Hierarchy<type>::SetOwner

Sets the object that this node is associated with.
================
*/
template< class type >
void Hierarchy<type>::SetOwner( type *object ) {
	owner = object;
}

/*
================
Hierarchy<type>::ParentedBy
================
*/
template< class type >
bool Hierarchy<type>::ParentedBy( const Hierarchy &node ) const {
	if ( parent == &node ) {
		return true;
	} else if ( parent ) {
		return parent->ParentedBy( node );
	}
	return false;
}

/*
================
Hierarchy<type>::ParentTo

Makes the given node the parent.
================
*/
template< class type >
void Hierarchy<type>::ParentTo( Hierarchy &node ) {
	RemoveFromParent( );

	parent		= &node;
	sibling		= node.child;
	node.child	= this;
}

/*
================
Hierarchy<type>::MakeSiblingAfter

Makes the given node a sibling after the passed in node.
================
*/
template< class type >
void Hierarchy<type>::MakeSiblingAfter( Hierarchy &node ) {
	RemoveFromParent( );
	parent	= node.parent;
	sibling = node.sibling;
	node.sibling = this;
}

/*
================
Hierarchy<type>::RemoveFromParent
================
*/
template< class type >
void Hierarchy<type>::RemoveFromParent( ) {
	Hierarchy<type> *prev;

	if ( parent ) {
		prev = GetPriorSiblingNode( );
		if ( prev ) {
			prev->sibling = sibling;
		} else {
			parent->child = sibling;
		}
	}

	parent = NULL;
	sibling = NULL;
}

/*
================
Hierarchy<type>::RemoveFromHierarchy

Removes the node from the hierarchy and adds it's children to the parent.
================
*/
template< class type >
void Hierarchy<type>::RemoveFromHierarchy( ) {
	Hierarchy<type> *parentNode;
	Hierarchy<type> *node;

	parentNode = parent;
	RemoveFromParent( );

	if ( parentNode ) {
		while( child ) {
			node = child;
			node->RemoveFromParent( );
			node->ParentTo( *parentNode );
		}
	} else {
		while( child ) {
			child->RemoveFromParent( );
		}
	}
}

/*
================
Hierarchy<type>::GetParent
================
*/
template< class type >
type *Hierarchy<type>::GetParent( ) const {
	if ( parent ) {
		return parent->owner;
	}
	return NULL;
}

/*
================
Hierarchy<type>::GetChild
================
*/
template< class type >
type *Hierarchy<type>::GetChild( ) const {
	if ( child ) {
		return child->owner;
	}
	return NULL;
}

/*
================
Hierarchy<type>::GetSibling
================
*/
template< class type >
type *Hierarchy<type>::GetSibling( ) const {
	if ( sibling ) {
		return sibling->owner;
	}
	return NULL;
}

/*
================
Hierarchy<type>::GetPriorSiblingNode

Returns NULL if no parent, or if it is the first child.
================
*/
template< class type >
Hierarchy<type> *Hierarchy<type>::GetPriorSiblingNode( ) const {
	if ( !parent || ( parent->child == this ) ) {
		return NULL;
	}

	Hierarchy<type> *prev;
	Hierarchy<type> *node;

	node = parent->child;
	prev = NULL;
	while( ( node != this ) && ( node != NULL ) ) {
		prev = node;
		node = node->sibling;
	}

	assert( node == this );

	return prev;
}

/*
================
Hierarchy<type>::GetPriorSibling

Returns NULL if no parent, or if it is the first child.
================
*/
template< class type >
type *Hierarchy<type>::GetPriorSibling( ) const {
	Hierarchy<type> *prior;

	prior = GetPriorSiblingNode( );
	if ( prior ) {
		return prior->owner;
	}

	return NULL;
}

/*
================
Hierarchy<type>::GetNext

Goes through all nodes of the hierarchy.
================
*/
template< class type >
type *Hierarchy<type>::GetNext( ) const {
	const Hierarchy<type> *node;

	if ( child ) {
		return child->owner;
	} else {
		node = this;
		while( node && node->sibling == NULL ) {
			node = node->parent;
		}
		if ( node ) {
			return node->sibling->owner;
		} else {
			return NULL;
		}
	}
}

/*
================
Hierarchy<type>::GetNextLeaf

Goes through all leaf nodes of the hierarchy.
================
*/
template< class type >
type *Hierarchy<type>::GetNextLeaf( ) const {
	const Hierarchy<type> *node;

	if ( child ) {
		node = child;
		while ( node->child ) {
			node = node->child;
		}
		return node->owner;
	} else {
		node = this;
		while( node && node->sibling == NULL ) {
			node = node->parent;
		}
		if ( node ) {
			node = node->sibling;
			while ( node->child ) {
				node = node->child;
			}
			return node->owner;
		} else {
			return NULL;
		}
	}
}

#endif /* !__HIERARCHY_H__ */
