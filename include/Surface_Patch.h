#ifndef __SURFACE_PATCH_H__
#define __SURFACE_PATCH_H__

/// * * * * * * * * * * * * * * *
///
/// Patch
///
/// Bezier patch surface.
///
/// * * * * * * * * * * * * * * *
class Patch : public Surface {

public:
						Patch( void );
						Patch( int maxPatchWidth, int maxPatchHeight );
						Patch( const Patch &patch );
						~Patch( void );

	void				SetSize( int patchWidth, int patchHeight );
	int					GetWidth( void ) const;
	int					GetHeight( void ) const;

	/// subdivide the patch mesh based on error
	void				Subdivide( float maxHorizontalError, float maxVerticalError, float maxLength, bool genNormals = false );

	/// subdivide the patch up to an explicit number of horizontal and vertical subdivisions
	void				SubdivideExplicit( int horzSubdivisions, int vertSubdivisions, bool genNormals, bool removeLinear = false );

	/// generate triangle indexes
	void				GenerateIndexes( void );

protected:

	/// width of patch
	int					width;

	/// height of patch
	int					height;

	/// maximum width allocated for
	int					maxWidth;

	/// maximum height allocated for
	int					maxHeight;

	/// true if vertices are spaced out
	bool				expanded;

private:

	/// put the approximation points on the curve
	void				PutOnCurve( void );

	/// remove columns and rows with all points on one line
	void				RemoveLinearColumnsRows( void );

	/// resize verts buffer
	void				ResizeExpanded( int height, int width );

	/// space points out over maxWidth * maxHeight buffer
	void				Expand( void );

	/// move all points to the start of the verts buffer
	void				Collapse( void );

	/// project a point onto a vector to calculate maximum curve error
	void				ProjectPointOntoVector( const Vec3 &point, const Vec3 &vStart, const Vec3 &vEnd, Vec3 &vProj );

	/// generate normals
	void				GenerateNormals( void );

	/// lerp point from two patch point
	void				LerpVert( const DrawVert &a, const DrawVert &b, DrawVert &out ) const;

	/// sample a single 3x3 patch
	void				SampleSinglePatchPoint( const DrawVert ctrl[3][3], float u, float v, DrawVert *out ) const;
	void				SampleSinglePatch( const DrawVert ctrl[3][3], int baseCol, int baseRow, int width, int horzSub, int vertSub, DrawVert *outVerts ) const;
};

INLINE Patch::Patch( void ) {

	height = width = maxHeight = maxWidth = 0;
	expanded = false;
}

INLINE Patch::Patch( int maxPatchWidth, int maxPatchHeight ) {

	width = height = 0;
	maxWidth = maxPatchWidth;
	maxHeight = maxPatchHeight;
	verts.SetNum( maxWidth * maxHeight );
	expanded = false;
}

INLINE Patch::Patch( const Patch & patch ) {

	(*this) = patch;
}

INLINE Patch::~Patch() {
}

INLINE int Patch::GetWidth( void ) const {

	return width;
}

INLINE int Patch::GetHeight( void ) const {

	return height;
}

#endif
