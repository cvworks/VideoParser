/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
//#define _USE_MATH_DEFINES
//#include <cmath>

#include "HelperFunctions.h"
#include <algorithm>

#define SMALL_NUM  0.00000001 // anything that avoids division overflow

using namespace vpl;

/*!
	@brief Computes the signed angle x between vectors (a_x, a_y) and (b_x, b_y)
	such that (a_x, a_y) rotated by x is aligned with (b_x, b_y)

	code from: http://gpwiki.org/index.php/Math:Vector

	@see doc/SignedVectorAngle.pdf
*/
double vpl::SignedVectorAngle(const double& a_x, const double& a_y, 
							  const double& b_x, const double& b_y,
							  const double& a_norm, const double& b_norm)
{
	ASSERT(a_norm > 0 && b_norm > 0);

	double cosine = (a_x * b_x + a_y * b_y) / (a_norm * b_norm);

	// Rounding errors might make dot product out of range for cosine
	if (cosine > 1) cosine = 1;
	else if (cosine < -1) cosine = -1;

	if ((a_x * b_y - a_y * b_x) < 0)
		return -acos(cosine);
	else
		return acos(cosine);
}


/*!
	@brief The intersection of 2 finite 2D segments

	code from: http://softsurfer.com/Archive/algorithm_0104/algorithm_0104B.htm

    The parametric equation of a line defined by two points P0 and P1 can be written as

	   P(s) = P0 + s (P1-P0) = P0 + s u,

    where the parameter s is a real number and u=P1-P0 is a line direction vector.
	Using this representation P(0)=P0, P(1)=P1, and when 0<s<1, P(s) is a point on the 
	finite segment between P0 and P1 where s is the fraction of P(s)'s distance along 
	the whole P0P1 line segment.  That is, s = d(P0,P(s)) / d(P0,P1).  Further, if s<0 
	then P(s) is outside the segment on the P0 side, and if s>1 then P(s) is outside 
	the segment on the P1 side.

	@param S1_P0 first point of finite segment S1
	@param S1_P1 last point of finite segment S1
	@param S2_P0 first point of finite segment S2
	@param S2_P1 last point of finite segment S2

    @param I0 intersect point (when it exists). Cannot be NULL.
    @param I1 endpoint of intersect segment [I0,I1] (when it exists). May be NULL.

    @return 0 disjoint (no intersect of segments or lines)
            1 both segments intersect in unique point I0 inside both segments
            2 segments overlap in the interval I0 to I1
			3 first segments is intersected by second line (in point I0)
			4 second segments is intersected by first line (in point I0)
			5 segments don't intersect but the lines do (in point I0)
*/
int vpl::FindLineSegmentIntersection(const Point& S1_P0, const Point& S1_P1,
							    const Point& S2_P0, const Point& S2_P1,
							    Point* I0, Point* I1 /*=NULL*/)
{
	vpl::Vector2D u = S1_P1 - S1_P0;
    vpl::Vector2D v = S2_P1 - S2_P0;
    vpl::Vector2D w = S1_P0 - S2_P0;
    double D = u.Perp(v);

	// test if they are parallel (includes either being a point)
    if (fabs(D) < SMALL_NUM)            // S1 and S2 are parallel
	{          
        if (u.Perp(w) != 0 || v.Perp(w) != 0) 
		{
            return 0;                   // they are NOT collinear
        }

        // they are collinear or degenerate
        // check if they are degenerate points
        double du = u.Dot(u);
        double dv = v.Dot(v);

        if (du == 0 && dv == 0)         // both segments are points
		{
            if (S1_P0 != S2_P0)         // they are distinct points
                return 0;

            *I0 = S1_P0;                // they are the same point
            return 1;
        }

        if (du == 0)                    // S1 is a single point
		{
            if (!IsInSegment(S1_P0, S2_P0, S2_P1))  // but is not in S2
                return 0;

            *I0 = S1_P0;
            return 1;
        }

        if (dv == 0)                    // S2 a single point
		{
            if (!IsInSegment(S2_P0, S1_P0, S1_P1))  // but is not in S1
                return 0;

            *I0 = S2_P0;
            return 1;
        }

        // they are collinear segments - get overlap (or not)
        double t0, t1;                   // endpoints of S1 in eqn for S2
		vpl::Vector2D w2 = S1_P1 - S2_P0;

        if (v.x != 0) 
		{
                t0 = w.x / v.x;
                t1 = w2.x / v.x;
        }
        else 
		{
                t0 = w.y / v.y;
                t1 = w2.y / v.y;
        }

        if (t0 > t1)                    // must have t0 smaller than t1
		{
                double t = t0; 

				t0 = t1; 
				t1 = t;    // swap if not
        }

        if (t0 > 1 || t1 < 0) 
		{
            return 0;     // NO overlap
        }

        t0 = (t0 < 0) ? 0 : t0;        // clip to min 0
        t1 = (t1 > 1) ? 1 : t1;        // clip to max 1

        if (t0 == t1)                  // intersect is a point
		{
            *I0 = S2_P0 + v * t0;
            return 1;
        }

        // they overlap in a valid subsegment
        *I0 = S2_P0 + v * t0;
		
		if (I1 != NULL)
			*I1 = S2_P0 + v * t1;

        return 2;
    }

    // the segments are skew and may intersect in a point
    // get the intersect parameter for S1
    double sI = v.Perp(w) / D;

	*I0 = S1_P0 + u * sI;               // compute S1 intersect point

	bool bS1Intersect = (sI >= 0 && sI <= 1);

    //if (sI < 0 || sI > 1)               // no intersect with S1
    //   return 3;                       // but lines do intersect

    // get the intersect parameter for S2
    double tI = u.Perp(w) / D;

	bool bS2Intersect = (tI >= 0 && tI <= 1);

    //if (tI < 0 || tI > 1)               // no intersect with S2
    //    return 3;                       // but lines do intersect

	if (bS1Intersect && !bS2Intersect)
		return 3;
	else if (!bS1Intersect && bS2Intersect)
		return 4;
	else if (!bS1Intersect && !bS2Intersect)
		return 5;

    return 1;
}


/*! 
	Determines if a point is inside a segment.

    @param P a point
	@param S_P0 first point of a collinear segment S
	@param S_P1 last point of a collinear segment S
    @return true if P is inside S, and false otherwise
*/
bool vpl::IsInSegment(const Point& P, const Point& S_P0, const Point& S_P1)
{
    if (S_P0.x != S_P1.x)     // S is not vertical
	{
        if (S_P0.x <= P.x && P.x <= S_P1.x)
            return true;

        if (S_P0.x >= P.x && P.x >= S_P1.x)
            return true;
    }
    else     // S is vertical, so test y coordinate
	{
        if (S_P0.y <= P.y && P.y <= S_P1.y)
            return true;

        if (S_P0.y >= P.y && P.y >= S_P1.y)
            return true;
    }

    return false;
}

/*!
	Compute the solution to the total least squares problem in terms
	of the solution of the related eigenvalue problem.
	See details in Forsyth & Ponce book (p. 334)

	returns the parameters of the line:

		cx * x + cy * y + c0 = 0

	such that (cx, cy) is a UNIT vector perpendicular to the line.

	@params vertices array of x-y point coordinates
	@param n number of poinst in the array
	@param cx the coefficent that multiplies x in the equation of the line above
	@param cy the coefficent that multiplies y in the equation of the line above
	@param c0 the independent coefficent in the equation of the line above
	@return the TLS fit error
	        
*/
double vpl::LinearTotalLeastSquares(const Point* vertices, int n, 
									double& cx, double& cy, double& c0)
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
	// which says that the eigenvector is simple (b2,-b1) for any non-zero row (b1,b2) of [a1-l a2; a2 a3-l]
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
	j = (dist[0] < dist[1]) ? 0:1;
	
	// Set the parameters of the line segment
	cx = V[0][j];
	cy = V[1][j];
	c0 = V[2][j];
	
	return dist[j];
}

/*
	@brief Finds the closest point on the line to point P. a, b, and c are the parameters
	of the 2D implicit line: a * x + b * y + c = 0
	It is assumed that (a, b) is a UNIT vector normal to the line. That is, 
			a * a + b * b == 1.
*/
Point vpl::GetClosestPointOnLine(const double& a, const double& b, const double& c, const Point& p)
{
	ASSERT((a * a + b * b) > 0.99 && (a * a + b * b) < 1.01);

	Point p0;

	// Choose a valid point in the line (use the least "non-zero" coeff)
	if (fabs(a) > fabs(b))
		p0.Set(-c / a, 0);
	else
		p0.Set(0, -c / b);

	double d = fabs(a * p.x + b * p.y + c); // distance between p and the line, assuming a^2 + b^2 = 1

	double h = (p - p0).L2(); // distance from p0 to p

	double u = fabs(h * cos(asin(d / h))); // distance from p0 to the projection of p onto the line

	Point L(b, -a); // get a unit vector in the direction of the line

	// L may be pointing towars the wrong direction, so we either scale by u or -u
	Point p1 = p0 + L * u;
	Point p2 = p0 + L * -u;

	return (p1.SqDist(p) < p2.SqDist(p)) ? p1 : p2;
}

/*!
   @brief Calculate the intersection of a ray and a circle

   code from: http://local.wasp.uwa.edu.au/~pbourke/geometry/sphereline/

   The line segment is defined from p1 to p2
   The sphere is of radius r and centered at sc

   There are potentially two points of intersection given by
   p = p1 + mu1 (p2 - p1)
   p = p1 + mu2 (p2 - p1)

   @return false if the ray doesn't intersect the circle.
*/
bool vpl::FindLineCircleIntersection(const Point& p1, const Point& p2, const Point& sc,
									 const double& r, double* mu1, double* mu2)
{
	double a, b, c;
	double bb4ac;
	Point dp;

	dp.x = p2.x - p1.x;
	dp.y = p2.y - p1.y;

	a = dp.x * dp.x + dp.y * dp.y;
	b = 2 * (dp.x * (p1.x - sc.x) + dp.y * (p1.y - sc.y));
	c = sc.x * sc.x + sc.y * sc.y;
	c += p1.x * p1.x + p1.y * p1.y;
	c -= 2 * (sc.x * p1.x + sc.y * p1.y);
	c -= r * r;

	bb4ac = b * b - 4 * a * c;

	if (fabs(a) < 0.0001 || bb4ac < 0) 
		return false;

	*mu1 = (-b + sqrt(bb4ac)) / (2 * a);
	*mu2 = (-b - sqrt(bb4ac)) / (2 * a);

	return true;
}

/*!
   @brief Checks if the a line segment and a circle intersect

   The line segment is defined from p1 to p2
   The sphere is of radius r and centered at sc

   @return true iff the segment intersects the circle.
*/
bool vpl::DoSegmentCircleIntersect(const Point& p1, const Point& p2, const Point& sc,
								   const double& r)
{
	double mu1, mu2;

	if (FindLineCircleIntersection(p1, p2, sc, r, &mu1, &mu2))
		return (mu1 >= 0 && mu1 <= 1) || (mu2 >= 0 && mu2 <= 1);
	else
		return false;
}

/*!
	@param logSpace N-dimensional array to store the N-dimensional log space
	@param lowBoundary low bounadry
	@param highBoundary high boundary
*/
void vpl::ComputeLogSpace(std::vector<double>& logSpace, 
						  const double& lowBoundary, double highBoundary)
{
	ASSERT(!logSpace.empty());

	if (highBoundary == M_PI)
		highBoundary = log10(M_PI);

	double distance = highBoundary - lowBoundary;

	const int numOfSlices = logSpace.size() - 1;

	for (int i = 0; i < numOfSlices; ++i)
		logSpace[i] = pow(10, lowBoundary + i * distance / numOfSlices);

	logSpace[numOfSlices] = pow(10, highBoundary);
}

/*!
	Rasterizes a line using Bresenham's line algorithm.

	If the given points are not connected, the points in between them are interpolated 
	by a line. 
	
	@return A list of connected points.
*/
void vpl::RasterizeLine(const Point& pt0, const Point& pt1, PointArray* linePts)
{
	ASSERT(linePts);
	ASSERT_VALID_POINT(pt0);
	ASSERT_VALID_POINT(pt1);

	linePts->clear();
	linePts->reserve((int)ceil(pt0.dist(pt1) * 1.1));

	// Add the first pt
	linePts->push_back(pt0);

	// See if the points are already connected (a common case easy to check)
	if (fabs(pt1.x - pt0.x) <= 1 && fabs(pt1.y - pt0.y) <= 1)
	{
		// Make sure that the points are different
		if (pt0 != pt1)
			linePts->push_back(pt1);

		return;
	}

	// Interpolate the given points with a rasterized line, so that the result 
	// is a connected set of points

	// Rasterize a line using Bresenham's line algorithm
	int x0 = ROUND_NUM(pt0.x);
	int y0 = ROUND_NUM(pt0.y);
	const int x1 = ROUND_NUM(pt1.x);
	const int y1 = ROUND_NUM(pt1.y);

	int dy = y1 - y0;
	int dx = x1 - x0;
	int stepx, stepy;

	if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	dy <<= 1;                                                  // dy is now 2*dy
	dx <<= 1;                                                  // dx is now 2*dx

	// do not add the first pt, because it's already there. 

	if (dx > dy) {
		int fraction = dy - (dx >> 1);                         // same as 2*dy - dx
		while (x0 != x1) {
			if (fraction >= 0) {
				y0 += stepy;
				fraction -= dx;                                // same as fraction -= 2*dx
			}
			x0 += stepx;
			fraction += dy;                                    // same as fraction -= 2*dy

			if (x0 != x1 || y0 != y1)                          // add last pt
				linePts->push_back(Point(x0, y0));
		}
	} else {
		int fraction = dx - (dy >> 1);
		while (y0 != y1) {
			if (fraction >= 0) {
				x0 += stepx;
				fraction -= dy;
			}
			y0 += stepy;
			fraction += dx;

			if (x0 != x1 || y0 != y1)                          // add last pt
				linePts->push_back(Point(x0, y0));
		}
	}

	// See if the the last point is already included and add it if not
	if (linePts->back().x != x1 || linePts->back().y != y1)
		linePts->push_back(Point(x1, y1));
}

/*!
	It shuffles the elements of the given array.

	@param seed if it's greater than 0, srand is called 
	with (seed - 1) prior to shuffling the array.
*/
void vpl::RandomArrayPermutation(std::vector<unsigned>& indexMap, 
	unsigned seed)
{
	if (seed > 0)
		std::srand(seed - 1);

	std::random_shuffle(indexMap.begin(), indexMap.end());
}

