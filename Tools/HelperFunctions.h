/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "STLUtils.h"

namespace vpl {

/*!
	Retrieves the parameterized point coordinate along a line segment.
	
	For example, the midpoint of the line segment can be computed by
	letting t = 0.5. Interestingly, as stated by the Midpoint Theorem, 
	the midpoint can also be found by computing the average of the 
	coordinates of its endpoints. ie, we could compute it as (p0 + p1) / 2
*/
inline Point LineSegmentPoint(const Point& p0, const Point& p1, const double& t)
{
	return p0 + (p1 - p0) * t;
}

//! Computes the area of the triangle with side lengths a, b and c
inline double TriangleArea(const double& a, const double& b, const double& c)
{
	double s = (a + b + c) / 2; //semiperimeter

	return sqrt(s * (s - a) * (s - b) * (s - c));
}

//! Computes the cosine of the angle between two vectors 
inline double VectorCosine(const double& a_x, const double& a_y, 
						   const double& b_x, const double& b_y,
						   const double& a_norm, const double& b_norm)
{
	ASSERT(a_norm > 0 && b_norm > 0);

	double cosine = (a_x * b_x + a_y * b_y) / (a_norm * b_norm);

	// Rounding errors might make dot product out of range for cosine
	if (cosine > 1) 
		return 1;
	else if (cosine < -1) 
		return -1;
	else
		return cosine;
}

double SignedVectorAngle(const double& a_x, const double& a_y, 
						 const double& b_x, const double& b_y,
						 const double& a_norm, const double& b_norm);

//! Computes the signed angle between two vectors
inline double SignedVectorAngle(const double& a_x, const double& a_y, 
								const double& b_x, const double& b_y)
{
	double a_norm = sqrt(a_x * a_x + a_y * a_y);
	double b_norm = sqrt(b_x * b_x + b_y * b_y);

	return SignedVectorAngle(a_x, a_y, b_x, b_y, a_norm, b_norm);
}

int FindLineSegmentIntersection(const Point& S1_P0, const Point& S1_P1,
							const Point& S2_P0, const Point& S2_P1,
							Point* I0, Point* I1 = NULL);

bool IsInSegment(const Point& P, const Point& S_P0, const Point& S_P1);

double LinearTotalLeastSquares(const Point* vertices, int n, double& ax, double& by, double& c);

Point GetClosestPointOnLine(const double& a, const double& b, const double& c, const Point& p);

bool DoSegmentCircleIntersect(const Point& p1, const Point& p2, const Point& sc, const double& r);

bool FindLineCircleIntersection(const Point& p1, const Point& p2, const Point& sc, 
								const double& r, double* mu1, double* mu2);

/*!
	Returns true iff point P is in triangle AB, BC, CA
	from: http://www.blackpawn.com/texts/pointinpoly/default.html
*/
inline bool IsInTriangle(const Point& P, const Point& A, const Point& B, const Point& C)
{
	// Compute vectors        
	Vector2D v0 = C - A;
	Vector2D v1 = B - A;
	Vector2D v2 = P - A;

	// Compute dot products
	double dot00 = v0.Dot(v0);
	double dot01 = v0.Dot(v1);
	double dot02 = v0.Dot(v2);
	double dot11 = v1.Dot(v1);
	double dot12 = v1.Dot(v2);

	// Compute barycentric coordinates
	double invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
	double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	double v = (dot00 * dot12 - dot01 * dot02) * invDenom;

	// Check if point is in triangle
	return (u > 0) && (v > 0) && (u + v < 1);
}

void ComputeLogSpace(std::vector<double>& logSpace, 
					 const double& lowBoundary, double highBoundary);

void RasterizeLine(const Point& pt0, const Point& pt1, PointArray* linePts);

/*!
	Returns the array [0,..., N - 1].

	@param N the number of elements in the array.
*/
inline std::vector<unsigned> TrivialIndexMap(const unsigned N)
{
	std::vector<unsigned> indexMap(N);

	for (unsigned i = 0; i < N; i++)
		indexMap[i] = i;

	return indexMap;
}

void RandomArrayPermutation(std::vector<unsigned>& indexMap, 
	unsigned seed = 0);

/*!
	It computes an array with N elements formed by randomly
	permuting the array [0,..., N - 1].

	@param N the number of elements in the array.

	@param seed if it's greater than 0, srand is called 
	with (seed - 1) prior to shuffling the array.
*/
inline std::vector<unsigned> RandomArrayPermutation(const unsigned N, 
	unsigned seed = 0)
{
	std::vector<unsigned> indexMap = TrivialIndexMap(N);

	RandomArrayPermutation(indexMap);

	return indexMap;
}

} // namespace vpl

