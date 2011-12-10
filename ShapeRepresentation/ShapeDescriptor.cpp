/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ShapeDescriptor.h"

using namespace vpl;

/*!
	[static] Computes an affine homography.
*/
Matrix ShapeDescriptor::ComputeHomography(const PointArray& srcPts, const PointArray& tgtPts)
{
	// Form the m x n matrix A
	const unsigned m = srcPts.size() * 2; // two rows per point
	const unsigned n = 9;          // the number of columns is always 9

	Matrix A(m, n);

	for (unsigned j = 0, i = 0; j < srcPts.size(); j++)
	{
		const Point& p = srcPts[j];
		const Point& q = tgtPts[j];

		A.row_ref(i++) << p.x << p.y << 1 << 0 << 0 << 0 
			<< -q.x * p.x << -q.x * p.y << -q.x;

		A.row_ref(i++) << 0 << 0 << 0 << p.x << p.y << 1
			<< -q.y * p.x << -q.y * p.y << -q.y;
	}

	// Compute the singular value decomposition of A
	vnl_svd<double> svd(A);

	// The column of V corresponding to the smallest singular value in D will be the
	// solution to H * P = 0. With 8 degrees of freedom, the 9th column should be
	// zero. However, due to noise H * P will likely be only close to zero.
	// Also, reshape the solution vector V(:,9) to get the homography matrix.
	VPLVector<double> h(svd.V().get_column(8));
	
	Matrix H = h.reshape(3, 3);

	// Normalize H using the fact that H(3,3) should be equal to 1
	H /= H(2, 2);

	return H;
}

/*!
	[static] Applies the homography H to the points.
*/
PointArray ShapeDescriptor::ApplyHomography(const Matrix& H, const PointArray& srcPts)
{
	Matrix P(3, srcPts.size());

	for (unsigned i = 0; i < srcPts.size(); i++)
	{
		P(0, i) = srcPts[i].x;
		P(1, i) = srcPts[i].y;
		P(2, i) = 1;
	}

	P = H * P;

	PointArray wrpPts(srcPts.size());

	for (unsigned i = 0; i < srcPts.size(); i++)
	{
		wrpPts[i].x = P(0, i) / P(2, i);
		wrpPts[i].y = P(1, i) / P(2, i);
	}

	return wrpPts;
}

/*!
	[static] Applies the homography H to the points.

	Point matrix must be  2 x N.

	{
		return ApplyHomography(H, ToPointArray(srcPts));
	}
*/
Matrix ShapeDescriptor::ApplyHomography(const Matrix& H, const Matrix& srcPts)
{
	ASSERT(srcPts.rows() == 2);
	ASSERT(H.rows() == 3 && H.cols() == 3);

	Matrix P;

	P.Set2x1(srcPts, Ones(1, srcPts.cols()));
	
	P = H * P;
	
	Matrix wrpPts(2, srcPts.cols());

	for (unsigned i = 0; i < srcPts.cols(); i++)
	{
		wrpPts(0, i) = P(0, i) / P(2, i);
		wrpPts(1, i) = P(1, i) / P(2, i);
	}
	
	return wrpPts;
}

