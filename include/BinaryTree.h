#ifndef BINARYTREE_H
#define BINARYTREE_H

template< class objType, class keyType > class BinaryTreeNode {

public:

	keyType	*								d_key;
	objType *								d_object;
	BinaryTreeNode *						d_childs[ 2 ];
};

template< class objType, class keyType > class BinaryTree {

protected:

	BinaryTreeNode< objType, keyType > *								d_root;
	DynamicBlockAlloc< BinaryTreeNode< objType, keyType >, 256, 32 >	d_nodeAllocator;
	DynamicBlockAlloc< BinaryTreeNode< objType, keyType >, 256, 32 >	d_leafAllocator;

public:

															BinaryTree( );
															~BinaryTree( );

	BinaryTreeNode< objType, keyType > *					operator[ ]( int index ) const;

	BinaryTreeNode< objType, keyType > *					AllocNode( );
	void													FreeNode( BinaryTreeNode< objType, keyType > * node );

	BinaryTreeNode< objType, keyType > *					AllocLeaf( );
	void													FreeLeaf( BinaryTreeNode< objType, keyType > * node );

	BinaryTreeNode< objType, keyType > *					GetRoot( ) const;
	int														GetNodeCount( ) const;
	int														GetLeafCount( ) const;

	void													GetDepth_r( BinaryTreeNode< objType, keyType > * node, int & depth ) const;
	int														GetDepth( ) const;

	BinaryTreeNode< objType, keyType > *					Find( keyType * key );
};

template< class objType, class keyType >
BinaryTree< objType, keyType >::BinaryTree( ) {

	d_root = NULL;
}

template< class objType, class keyType >
BinaryTree< objType, keyType >::~BinaryTree( ) {

	d_nodeAllocator.Shutdown( );
	d_leafAllocator.Shutdown( );
}

template< class objType, class keyType >
BinaryTreeNode< objType, keyType > * BinaryTree< objType, keyType >::operator[ ]( int index ) const {
}

template< class objType, class keyType >
BinaryTreeNode< objType, keyType > * BinaryTree< objType, keyType >::AllocNode( ) {

	BinaryTreeNode< objType, keyType > * node = d_nodeAllocator.Alloc( 1 );

	node->d_key = NULL;
	node->d_object = NULL;
	node->d_childs[ 0 ] = NULL;
	node->d_childs[ 1 ] = NULL;

	return node;
}

template< class objType, class keyType >
INLINE void BinaryTree< objType, keyType >::FreeNode( BinaryTreeNode< objType, keyType > * node ) {

	d_nodeAllocator.Free( node );
}

template< class objType, class keyType >
BinaryTreeNode< objType, keyType > * BinaryTree< objType, keyType >::AllocLeaf( ) {

	BinaryTreeNode< objType, keyType > * node = d_leafAllocator.Alloc( 1 );

	node->d_key = NULL;
	node->d_object = NULL;
	node->d_childs[ 0 ] = NULL;
	node->d_childs[ 1 ] = NULL;

	return node;
}

template< class objType, class keyType >
INLINE void BinaryTree< objType, keyType >::FreeLeaf( BinaryTreeNode< objType, keyType > * node ) {

	d_leafAllocator.Free( node );
}

template< class objType, class keyType >
INLINE BinaryTreeNode< objType, keyType > * BinaryTree< objType, keyType >::GetRoot( ) const {

	return d_root;
}

template< class objType, class keyType >
INLINE int BinaryTree< objType, keyType >::GetNodeCount( ) const {

	return d_nodeAllocator.GetAllocCount( );
}

template< class objType, class keyType >
INLINE int BinaryTree< objType, keyType >::GetLeafCount( ) const {

	return d_leafAllocator.GetAllocCount( );
}

template< class objType, class keyType >
int BinaryTree< objType, keyType >::GetDepth( ) const {

	assert( d_root );
	int depth = 0;
	GetDepth_r( d_root, depth );
	return depth;
}

template< class objType, class keyType >
void BinaryTree< objType, keyType >::GetDepth_r( BinaryTreeNode< objType, keyType > * node, int & depth ) const {

	depth++;

	int leftDepth = depth;
	int rightDepth = depth;

	if( node->d_childs[ 0 ] ) GetDepth_r( node->d_childs[ 0 ], leftDepth );
	if( node->d_childs[ 1 ] ) GetDepth_r( node->d_childs[ 1 ], rightDepth );

	depth = Max( leftDepth, rightDepth );
}

#endif
