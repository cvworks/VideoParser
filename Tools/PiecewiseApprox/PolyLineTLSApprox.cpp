/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include <stdlib.h>
#include <iostream>
#include "PolyLineTLSApprox.h"
#include "../HelperFunctions.h"
#include "../BasicUtils.h"

using namespace vpl;

/*!
	Compute the solution to the total least squares problem in terms
	of the solution of the related eigenvalue problem.
	See details in Forsyth & Ponce book (p. 334)
*/
double PolyLineTLSApprox::LeastSquares(const Point* vertices, int n, EuclideanLineSegment& s)
{
	int i, j;
	double x, y, sumx = 0, sumy = 0, sumx2 = 0, sumy2 = 0, sumxy = 0;
	double mx, my, a1, a2, a3, b0, b1, v0, v1, vecNorm;
	double V[3][2], D[2], dist[2];
	
	ASSERT(n > 1);
	
	// Compute the means needed to form the matrix of the eigenvalue problem
	for (i = 0; i < n; i++)
	{
		x = vertices[i].x;
		y = vertices[i].y;

		sumx += x;
		sumy += y;
		sumx2 += x * x;
		sumxy += x * y;
		sumy2 += y * y;
	}
	
	mx = sumx / n;
	my = sumy / n;
	
	a1 = sumx2 / n - mx * mx;
	a2 = sumxy / n - mx * my;
	a3 = sumy2 / n - my * my;
	
	// Find roots of characteristic polynomial
	b0 = -a1 - a3;
	b1 = sqrt(b0 * b0 - 4 * (a1 * a3 - a2 * a2));
	D[0] = (-b0 - b1) / 2;
	D[1] = (-b0 + b1) / 2;
		
	// Find the two eigenvectors of the "quadratic" polynomial using Strang's trick (p. 243)
	// which says that the eigenvector is simply (b2,-b1) for any non-zero row (b1,b2) of [a1-l a2; a2 a3-l]
	for (j = 0; j < 2; j++)
	{
		// use the first row...
		v0 = a2;
		v1 = -a1 + D[j];
		
		// ...but if it's a zero row, then choose the other row
		if (v0 == 0 && v1 == 0) 
		{
			v0 = a3 - D[j];
			v1 = -a2;
		}
		
		// make the eigenvector have unit length
		vecNorm = sqrt(v0 * v0 + v1 * v1);
		V[0][j] = v0 / vecNorm;
		V[1][j] = v1 / vecNorm;
		
		// add the last parameter c of the line ax + by + c = 0 as a 3rd dimension in V
		V[2][j] = -V[0][j] * mx - V[1][j] * my;
	}
	
	// Compute distances to line using Forsyth & Ponce's trick (p. 334) and sum them
	dist[0] = dist[1] = 0;
	
	for(i = 0; i < n; i++)
		for(j = 0; j < 2; j++)
			dist[j] += fabs(V[0][j] * vertices[i].x + V[1][j] * vertices[i].y + V[2][j]);
			
	// Find the index of the eigenvector that best fits the line
	j = (dist[0] < dist[1]) ? 0 : 1;
	
	/*if (fabs(V[1][j]) < 0.00001) 
		V[1][j] = 0.00001;
	
	// Set the parameters of the line segment
	s.m = -V[0][j] / V[1][j];
	s.b = -V[2][j] / V[1][j];
	
	ASSERT_VALID_NUM(s.m);
	ASSERT_VALID_NUM(s.b);*/

	// The line has the form ax + by + c = 0 and may not go throu any of
	// the given vertices, so the first and last points of the line should be
	// those that are closer to the first and last vertices given, respectively.
	s.p0 = vertices[0];
	s.p1 = vertices[n - 1];
	
	// Return squared error to be consisten with function in the base class
	return dist[j] /* * dist[j]*/;
}

