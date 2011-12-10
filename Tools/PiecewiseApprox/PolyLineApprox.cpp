/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include <stdlib.h>
#include <iostream>
#include "PolyLineApprox.h"
#include "../HelperFunctions.h"
#include "../BasicUtils.h"

using namespace vpl;

/*!
	@brief Finds the best fit line mx + b, where

	 m = (n Sum(xy) - Sum(x) Sum(y)) / (n Sum(x^2) - (Sum(x))^2)
	 b = mean(y) - m mean(x)

	 we could also find it by the projecton matrix: P = A (A^t A)^-1 A^t
*/
double PolyLineApprox::LeastSquares(const Point* vertices, int n, LineSegment& s)
{
	int i;
	double x, y, sumx = 0, sumy = 0, sumx2 = 0, sumxy = 0, sum2x = 0;

	ASSERT(n > 1);
	
	for (i = 0; i < n; i++)
	{
		x = vertices[i].x;
		y = vertices[i].y;
		
		sumx += x;
		sumy += y;
		sumx2 += x * x;
		sumxy += x * y;
	}
	
	sum2x = sumx * sumx;
	
	// Set the parameters of the segment
	s.m = (n * sumxy - sumx * sumy) / (n * sumx2 - sum2x);
	s.b = (sumy - s.m * sumx) / n; // == (sumx2 * sumy - sumxy * sumx) / double(n * sumx2 - sum2x);
	s.p0 = vertices[0];
	s.p1 = vertices[n - 1];

	ASSERT_VALID_NUM(s.m);
	ASSERT_VALID_NUM(s.b);
	
	// Compute the squared error
	double e, sume2 = 0;
	
	for(i = 0; i < n; i++)
	{
		e = s.m * vertices[i].x + s.b - vertices[i].y;
		sume2 += e * e;
	}
	
	return sume2;
}

/*!
	@brief Computes a relative measure of the line slope and compares
	it to the m_dMinSlope value.

	@returns 0 if the relative slope is in [-m_dMinSlope, m_dMinSlope]
	         otherwise, it returns the sign of the slope: 1 or -1
*/
int PolyLineApprox::GetSegmentDirection(const LineSegment& s) const
{
	double mx, dx, r, my;
	
	mx = (s.p0.x + s.p1.x) / 2;
	dx = fabs(s.p0.x - s.p1.x);
	
	my = s.m * mx + s.b;

	// Compute dy / mean y if possible
	r = (my > 0) ? (fabs(s.m * dx) / my) : 0; 
	
	ASSERT(r >= 0.0);
		
	return (r < m_dMinSlope) ? 0:SIGN(s.m);
}

void PolyLineApprox::PlotKnots(int seg_num) const
{
	static int dataidx = 0;	
		
	std::cout << "\n% DATA " << ++dataidx << "...\na = ";
	m_points.Print(std::cout, "\n");
	std::cout << ";\n% Fitting data " << dataidx << "...\n\n";
	
	MEMDATA d = m_minerrors[seg_num][0][m_points.GetSize() - 1];
	
	std::cout << "\n% For " << seg_num << " lines: \n";
	std::cout << "plot(a(:,1),a(:,2),'x');\nhold on;\n";
		
	for (int j = 0; j < m_knots.GetSize(); j++)
	{
		const KNOT& k = m_knots[j];
		const char* c = k.dir == 0 ? "g":(j % 2) ? "b":"r";
			
		std::cout << "x = " << k.seg.p0.x << ':' << k.seg.p1.x << ";\n";
		std::cout << "y = " << k.seg.m << " * x + " << k.seg.b << ";\n";
		std::cout << "plot(x,y,'" << c << "');\n";
		
		std::cout << "% p0.y = " << k.seg.p0.y << " and p1.y = " << k.seg.p1.y << ";\n";
		
		//if (j < m_knots.GetSize() - 1)
		//	std::cout << "disp('Angle: " << CompAcuteAngle(j, j + 1) << "');\n";			
	}
	
	std::cout << "title('Num of segments: " << seg_num
		<< ", error: " << d.GetMinError() << ",  min error: " << m_dMinError
		<< ", max segs: " << m_nMaxSegments	<< "');\n";

	std::cout << "ylabel('Radius');\n";
	std::cout << "xlabel('Shock point index');\n";
				
	std::cout << "hold off;\npause;\n\n";
	std::cout << "% End fitting...\n";
}
