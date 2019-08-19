#ifndef BTREE_H
#define BTREE_H

// // //// // //// // //// //
/// Block based allocator for fixed size objects.
//
/// All objects of the 'type' are properly constructed.
/// However, the constructor is not called for re-used objects.
// //// // //// // //// //

template< class type, int block_size > class BlockAlloc {
private:
	template< class type > class Element {
	public:
		Element *				d_next;
		type					d_t;
	};
	typedef Element< type >	t_element;

	template< class type, int block_size > class Block {
	public:
		t_element				d_elements[ block_size ];
		Block *					d_next;
	};
	typedef Block< type, block_size >	t_block;

	t_block *				d_blocks;
	t_element *				d_free;
	int						d_total;
	int						d_active;
	Lock					d_lock;					// lock for thread safe memory allocation

public:
							BlockAlloc( );
							~BlockAlloc( );

	void					Shutdown( );

	type *					Alloc( );
	void					Free( type * element );

	int						GetTotalCount( ) const	{ return d_total; }
	int						GetAllocCount( ) const	{ return d_active; }
	int						GetFreeCount( ) const	{ return d_total - d_active; }
};

// // //// // //// // //// //
/// BTreeNode
// //// // //// // //// //

template< class type, class t_key > class BTreeNode {
public:
	/// key used for sorting
	t_key								d_key;
	/// if != NULL pointer to object stored in leaf node
	type *								d_object;
	/// parent node
	BTreeNode *							d_parent;
	/// next sibling
	BTreeNode *							d_next;
	/// prev sibling
	BTreeNode *							d_prev;
	/// number of children
	int									d_numChildren;
	/// first child
	BTreeNode *							d_firstChild;
	/// last child
	BTreeNode *							d_lastChild;
};

//#define BTREE_CHECK

// // //// // //// // //// //
/// BTree
//
/// Balanced Search Tree
//
/// t_key operators needed: = 0 for default value, ==, <=, and >= depending on FindLargestSmallerEqual is needed
// //// // //// // //// //

template< class type, class t_key, int num_child, int block_size = 128 > class BTree {
private:
	typedef BTreeNode< type, t_key >	t_node;

	t_node *							d_root;
	BlockAlloc< t_node, block_size >	d_nodeAllocator;

	t_node *							AllocNode( );
	void								FreeNode( t_node * node );
	void								SplitNode( t_node * node );
	t_node *							MergeNodes( t_node * node1, t_node * node2 );

	void								CheckTree_r( t_node * node, int & numNodes ) const;
	void								CheckTree( ) const;

public:
										BTree( );
										~BTree( );

	void								Init( );
	void								Shutdown( );

	void								Clear( );

	/// add an object to the tree
	t_node *							Add( type * object, t_key key );

	/// remove an object node from the tree
	void								Remove( t_node * node );

	/// find an object using the given key
	type *								Find( t_key key ) const;

	/// find an object with the smallest key larger equal the given key
	type *								FindSmallestLargerEqual( t_key key ) const;

	/// find an object with the largest key smaller equal the given key
	type *								FindLargestSmallerEqual( t_key key ) const;

	/// find an object using the given key
	t_node *							FindNode( t_key key ) const;

	/// find an object with the smallest key larger equal the given key
	t_node *							FindSmallestLargerEqualNode( t_key key ) const;

	/// find an object with the largest key smaller equal the given key
	t_node *							FindLargestSmallerEqualNode( t_key key ) const;

	/// returns the root node of the tree
	t_node *							GetRoot( ) const;

	/// returns the total number of nodes in the tree
	int									GetNodeCount( ) const;

	/// goes through all nodes of the tree
	t_node *							GetNext( t_node * node ) const;

	/// goes through all leaf nodes of the tree
	t_node *							GetNextLeaf( t_node * node ) const;

	t_node *							Begin( ) const;
};

// // //// // //// // //// //
// BlockAlloc
// //// // //// // //// //

template< class type, int block_size >
INLINE BlockAlloc< type, block_size >::BlockAlloc( ) {
	d_blocks = NULL;
	d_free = NULL;
	d_total = d_active = 0;
}

template< class type, int block_size >
INLINE BlockAlloc< type, block_size >::~BlockAlloc( ) {
	Shutdown( );
}

template< class type, int block_size >
INLINE type * BlockAlloc< type, block_size >::Alloc( ) {
	ScopedLock< true > scopedLock( d_lock );
	if( !d_free ) {
		t_block * block = new t_block;
		block->d_next = d_blocks;
		d_blocks = block;
		for( int i = 0; i < block_size; i++ ) {
			block->d_elements[ i ].d_next = d_free;
			d_free = &block->d_elements[ i ];
		}
		d_total += block_size;
	}
	d_active++;
	t_element * element = d_free;
	d_free = d_free->d_next;
	element->d_next = NULL;
	return &element->d_t;
}

template< class type, int block_size >
INLINE void BlockAlloc< type, block_size >::Free( type * t ) {
	ScopedLock< true > scopedLock( d_lock );
	t_element * element = ( t_element * )( ( ( unsigned char * )t ) - offsetof( t_element, d_t ) );
	element->d_next = d_free;
	d_free = element;
	d_active--;
}

template< class type, int block_size >
INLINE void BlockAlloc< type, block_size >::Shutdown( ) {
	ScopedLock< true > scopedLock( d_lock );
	while( d_blocks ) {
		t_block * block = d_blocks;
		d_blocks = d_blocks->d_next;
		delete block;
	}
	d_blocks = NULL;
	d_free = NULL;
	d_total = d_active = 0;
}

// // //// // //// // //// //
// BTree
// //// // //// // //// //

template< class type, class t_key, int num_child, int block_size >
INLINE BTree< type, t_key, num_child, block_size >::BTree( ) {
	assert( num_child >= 4 );
	d_root = NULL;
	Init( );
}

template< class type, class t_key, int num_child, int block_size >
INLINE BTree< type, t_key, num_child, block_size >::~BTree( ) {
	Shutdown( );
}

template< class type, class t_key, int num_child, int block_size >
INLINE void BTree< type, t_key, num_child, block_size >::Init( ) {
	d_root = AllocNode( );
}

template< class type, class t_key, int num_child, int block_size >
INLINE void BTree< type, t_key, num_child, block_size >::Shutdown( ) {
	d_nodeAllocator.Shutdown( );
	d_root = NULL;
}

template< class type, class t_key, int num_child, int block_size >
INLINE void BTree< type, t_key, num_child, block_size >::Clear( ) {
	d_nodeAllocator.Shutdown( );
	d_root = AllocNode( );
}

template< class type, class t_key, int num_child, int block_size >
BTreeNode< type, t_key > * BTree< type, t_key, num_child, block_size >::Add( type * object, t_key key ) {
	t_node * newNode;
	if( d_root->d_numChildren >= num_child ) {
		newNode = AllocNode( );
		newNode->d_key = d_root->d_key;
		newNode->d_firstChild = d_root;
		newNode->d_lastChild = d_root;
		newNode->d_numChildren = 1;
		d_root->d_parent = newNode;
		SplitNode( d_root );
		d_root = newNode;
	}
	newNode = AllocNode( );
	newNode->d_key = key;
	newNode->d_object = object;
	for( t_node * child, * node = d_root; node->d_firstChild != NULL; node = child ) {
		if( key > node->d_key )
			node->d_key = key;
		// find the first child with a key larger equal to the key of the new node
		for( child = node->d_firstChild; child->d_next; child = child->d_next )
			if( key <= child->d_key )
				break;
		if( child->d_object ) {
			if( key <= child->d_key ) {
				// insert new node before child
				if( child->d_prev )
					child->d_prev->d_next = newNode;
				else
					node->d_firstChild = newNode;
				newNode->d_prev = child->d_prev;
				newNode->d_next = child;
				child->d_prev = newNode;
			} else {
				// insert new node after child
				if( child->d_next )
					child->d_next->d_prev = newNode;
				else
					node->d_lastChild = newNode;
				newNode->d_prev = child;
				newNode->d_next = child->d_next;
				child->d_next = newNode;
			}
			newNode->d_parent = node;
			node->d_numChildren++;
#ifdef BTREE_CHECK
			CheckTree( );
#endif
			return newNode;
		}
		// make sure the child has room to store another node
		if( child->d_numChildren >= num_child ) {
			SplitNode( child );
			if( key <= child->d_prev->d_key )
				child = child->d_prev;
		}
	}
	// we only end up here if the root node is empty
	newNode->d_parent = d_root;
	d_root->d_key = key;
	d_root->d_firstChild = newNode;
	d_root->d_lastChild = newNode;
	d_root->d_numChildren++;
#ifdef BTREE_CHECK
	CheckTree( );
#endif
	return newNode;
}

template< class type, class t_key, int num_child, int block_size >
void BTree< type, t_key, num_child, block_size >::Remove( t_node * node ) {
	assert( node->d_object != NULL );
	if( node->d_prev ) // unlink the node from it's parent
		node->d_prev->d_next = node->d_next;
	else
		node->d_parent->d_firstChild = node->d_next;
	if( node->d_next )
		node->d_next->d_prev = node->d_prev;
	else
		node->d_parent->d_lastChild = node->d_prev;
	node->d_parent->d_numChildren--;
	t_node * parent;
	for( parent = node->d_parent; parent != d_root && parent->d_numChildren <= 1; parent = parent->d_parent ) { // make sure there are no parent nodes with a single child
		if( parent->d_next )
			parent = MergeNodes( parent, parent->d_next );
		else if( parent->d_prev )
			parent = MergeNodes( parent->d_prev, parent );
		if( parent->d_key > parent->d_lastChild->d_key ) // a parent may not use a key higher than the key of it's last child
			parent->d_key = parent->d_lastChild->d_key;
		if( parent->d_numChildren > num_child ) {
			SplitNode( parent );
			break;
		}
	}
	for( ; parent != NULL && parent->d_lastChild != NULL; parent = parent->d_parent )
		if( parent->d_key > parent->d_lastChild->d_key ) // a parent may not use a key higher than the key of it's last child
			parent->d_key = parent->d_lastChild->d_key;
	FreeNode( node ); // free the node
	if( d_root->d_numChildren == 1 && d_root->d_firstChild->d_object == NULL ) { // remove the root node if it has a single internal node as child
		t_node * oldRoot = d_root;
		d_root->d_firstChild->d_parent = NULL;
		d_root = d_root->d_firstChild;
		FreeNode( oldRoot );
	}
#ifdef BTREE_CHECK
	CheckTree( );
#endif
}

template< class type, class t_key, int num_child, int block_size >
INLINE type * BTree< type, t_key, num_child, block_size >::Find( t_key key ) const {
	for( t_node * node = d_root->d_firstChild; node != NULL; node = node->d_firstChild ) {
		while( node->d_next ) {
			if( node->d_key >= key )
				break;
			node = node->d_next;
		}
		if( node->d_object ) 
			return ( node->d_key == key ) ? node->d_object : NULL;
	}
	return NULL;
}

template< class type, class t_key, int num_child, int block_size >
INLINE type * BTree< type, t_key, num_child, block_size >::FindSmallestLargerEqual( t_key key ) const {
	for( t_node * node = d_root->d_firstChild; node != NULL; node = node->d_firstChild ) {
		while( node->d_next ) {
			if( node->d_key >= key )
				break;
			node = node->d_next;
		}
		if( node->d_object )
			return ( node->d_key >= key ) ? node->d_object : NULL;
	}
	return NULL;
}

template< class type, class t_key, int num_child, int block_size >
INLINE type * BTree< type, t_key, num_child, block_size >::FindLargestSmallerEqual( t_key key ) const {
	for( t_node * node = d_root->d_lastChild; node != NULL; node = node->d_lastChild ) {
		while( node->d_prev ) {
			if( node->d_key <= key )
				break;
			node = node->d_prev;
		}
		if( node->d_object )
			return ( node->d_key <= key ) ? node->d_object : NULL;
	}
	return NULL;
}

template< class type, class t_key, int num_child, int block_size >
INLINE BTreeNode< type, t_key > * BTree< type, t_key, num_child, block_size >::FindNode( t_key key ) const {
	for( t_node * node = d_root->d_firstChild; node != NULL; node = node->d_firstChild ) {
		while( node->d_next ) {
			if( node->d_key >= key )
				break;
			node = node->d_next;
		}
		if( node->d_object )
			return ( node->d_key == key ) ? node : NULL;
	}
	return NULL;
}

template< class type, class t_key, int num_child, int block_size >
INLINE BTreeNode< type, t_key > * BTree< type, t_key, num_child, block_size >::FindSmallestLargerEqualNode( t_key key ) const {
	for( t_node * node = d_root->d_firstChild; node != NULL; node = node->d_firstChild ) {
		while( node->d_next ) {
			if( node->d_key >= key )
				break;
			node = node->d_next;
		}
		if( node->d_object )
			return ( node->d_key >= key ) ? node : NULL;
	}
	return NULL;
}

template< class type, class t_key, int num_child, int block_size >
INLINE BTreeNode< type, t_key > * BTree< type, t_key, num_child, block_size >::FindLargestSmallerEqualNode( t_key key ) const {
	for( t_node * node = d_root->d_lastChild; node != NULL; node = node->d_lastChild ) {
		while( node->d_prev ) {
			if( node->d_key <= key )
				break;
			node = node->d_prev;
		}
		if( node->d_object )
			return ( node->d_key <= key ) ? node : NULL;
	}
	return NULL;
}

template< class type, class t_key, int num_child, int block_size >
INLINE BTreeNode< type, t_key > * BTree< type, t_key, num_child, block_size >::GetRoot( ) const {
	return d_root;
}

template< class type, class t_key, int num_child, int block_size >
INLINE int BTree< type, t_key, num_child, block_size >::GetNodeCount( ) const {
	return d_nodeAllocator.GetAllocCount( );
}

template< class type, class t_key, int num_child, int block_size >
INLINE BTreeNode< type, t_key > * BTree< type, t_key, num_child, block_size >::GetNext( t_node * node ) const {
	if( node->d_firstChild )
		return node->d_firstChild;
	else {
		while( node && node->d_next == NULL )
			node = node->d_parent;
		return node;
	}
}

template< class type, class t_key, int num_child, int block_size >
INLINE BTreeNode< type, t_key > * BTree< type, t_key, num_child, block_size >::Begin( ) const {
	//return GetNextLeaf( GetRoot( ) );
	t_node * node = d_root;
	if( node->d_firstChild ) {
		while ( node->d_firstChild )
			node = node->d_firstChild;
		return node;
	} else {
		while( node && node->d_next == NULL )
			node = node->d_parent;
		if( node ) {
			node = node->d_next;
			while ( node->d_firstChild )
				node = node->d_firstChild;
			return node;
		} else
			return NULL;
	}
}

template< class type, class t_key, int num_child, int block_size >
INLINE BTreeNode< type, t_key > * BTree< type, t_key, num_child, block_size >::GetNextLeaf( t_node * node ) const {
	if( node->d_firstChild ) {
		while ( node->d_firstChild )
			node = node->d_firstChild;
		return node;
	} else {
		while( node && node->d_next == NULL )
			node = node->d_parent;
		if( node ) {
			node = node->d_next;
			while ( node->d_firstChild )
				node = node->d_firstChild;
			return node;
		} else
			return NULL;
	}
}

template< class type, class t_key, int num_child, int block_size >
BTreeNode< type, t_key > * BTree< type, t_key, num_child, block_size >::AllocNode( ) {
	t_node * node = d_nodeAllocator.Alloc( );
	node->d_key = 0;
	node->d_parent = NULL;
	node->d_next = NULL;
	node->d_prev = NULL;
	node->d_numChildren = 0;
	node->d_firstChild = NULL;
	node->d_lastChild = NULL;
	node->d_object = NULL;
	return node;
}

template< class type, class t_key, int num_child, int block_size >
INLINE void BTree< type, t_key, num_child, block_size >::FreeNode( t_node * node ) {
	d_nodeAllocator.Free( node );
}

template< class type, class t_key, int num_child, int block_size >
void BTree< type, t_key, num_child, block_size >::SplitNode( t_node * node ) {
	t_node * newNode = AllocNode( ); // allocate a new node
	newNode->d_parent = node->d_parent;
	t_node * child = node->d_firstChild; // divide the children over the two nodes
	child->d_parent = newNode;
	for( int i = 3; i < node->d_numChildren; i += 2 ) {
		child = child->d_next;
		child->d_parent = newNode;
	}
	newNode->d_key = child->d_key;
	newNode->d_numChildren = node->d_numChildren / 2;
	newNode->d_firstChild = node->d_firstChild;
	newNode->d_lastChild = child;
	node->d_numChildren -= newNode->d_numChildren;
	node->d_firstChild = child->d_next;
	child->d_next->d_prev = NULL;
	child->d_next = NULL;
	assert( node->d_parent->d_numChildren < num_child );
	if( node->d_prev ) // add the new child to the parent before the split node
		node->d_prev->d_next = newNode;
	else
		node->d_parent->d_firstChild = newNode;
	newNode->d_prev = node->d_prev;
	newNode->d_next = node;
	node->d_prev = newNode;
	node->d_parent->d_numChildren++;
}

template< class type, class t_key, int num_child, int block_size >
BTreeNode< type,t_key > * BTree< type, t_key, num_child, block_size >::MergeNodes( t_node * node1, t_node * node2 ) {
	assert( node1->d_parent == node2->d_parent );
	assert( node1->d_next == node2 && node2->d_prev == node1 );
	assert( node1->d_object == NULL && node2->d_object == NULL );
	assert( node1->d_numChildren >= 1 && node2->d_numChildren >= 1 );
	t_node * child;
	for( child = node1->d_firstChild; child->d_next; child = child->d_next )
		child->d_parent = node2;
	child->d_parent = node2;
	child->d_next = node2->d_firstChild;
	node2->d_firstChild->d_prev = child;
	node2->d_firstChild = node1->d_firstChild;
	node2->d_numChildren += node1->d_numChildren;
	if( node1->d_prev ) // unlink the first node from the parent
		node1->d_prev->d_next = node2;
	else
		node1->d_parent->d_firstChild = node2;
	node2->d_prev = node1->d_prev;
	node2->d_parent->d_numChildren--;
	FreeNode( node1 );
	return node2;
}

template< class type, class t_key, int num_child, int block_size >
void BTree< type, t_key, num_child, block_size >::CheckTree_r( t_node * node, int & numNodes ) const {
	numNodes++;
	// the root node may have zero children and leaf nodes always have zero children, all other nodes should have at least 2 and at most num_child children
	assert( ( node == d_root ) || ( node->d_object != NULL && node->d_numChildren == 0 ) || ( node->d_numChildren >= 2 && node->d_numChildren <= num_child ) );
	// the key of a node may never be larger than the key of it's last child
	assert( ( node->d_lastChild == NULL ) || ( node->d_key <= node->d_lastChild->d_key ) );
	int numChildren = 0;
	for( t_node * child = node->d_firstChild; child; child = child->d_next ) {
		numChildren++;
		if( child->d_prev == NULL ) // make sure the children are properly linked
			assert( node->d_firstChild == child );
		else
			assert( child->d_prev->d_next == child );
		if( child->d_next == NULL )
			assert( node->d_lastChild == child );
		else
			assert( child->d_next->d_prev == child );
		CheckTree_r( child, numNodes ); // recurse down the tree
	}
	assert( numChildren == node->d_numChildren ); // the number of children should equal the number of linked children
}

template< class type, class t_key, int num_child, int block_size >
void BTree< type, t_key, num_child, block_size >::CheckTree( ) const {
	int numNodes = 0;
	CheckTree_r( d_root, numNodes );
	assert( numNodes == d_nodeAllocator.GetAllocCount( ) ); // the number of nodes in the tree should equal the number of allocated nodes
	t_node * lastNode = GetNextLeaf( GetRoot( ) ); // all the leaf nodes should be ordered
	if( lastNode )
		for( t_node * node = GetNextLeaf( lastNode ); node; lastNode = node, node = GetNextLeaf( node ) )
			assert( lastNode->d_key <= node->d_key );
}

#endif /* !__BTREE_H__ */
