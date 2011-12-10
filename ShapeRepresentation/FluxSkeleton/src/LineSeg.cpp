/************************************************************************

File:		LineSeg.cpp

Author(s):		Pavel Dimitrov

Created:		11 Jun 2002

Last Revision:	$Date: 2002/07/25 20:50:49 $

Description:	

$Revision: 1.3 $

$Log: LineSeg.cpp,v $
Revision 1.3  2002/07/25 20:50:49  pdimit
Making release 0.1

Revision 1.2  2002/06/26 06:34:45  pdimit
After adding a more successful smoothing.
A point becomes the average of its previou nbr, next nbr and
itself. The first and end points (which are the same) do not change.
This smoothing primitive is applied 5 times to get a nicer result.

Revision 1.1.1.1  2002/06/23 05:42:08  pdimit
Initial import



Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
McGill University, Montreal, QC.  Please see the copyright notice
included in this distribution for full details.

***********************************************************************/

#define LINESEG_CPP

/**********************
Include Files
**********************/

#include "LineSeg.h"
#include "defines.h"

using namespace sg;

void LineSeg::computeDistance(const Point &p, const double& t1, 
							  const double& t2, Distance& ret)
{
	double s;
	double d;
	Vector v1(v.y, -v.x); // the perpendicular vector

	d = (v1.x * v.y - v.x * v1.y);

	if(d != 0)
	{
		s = (v1.x * (p.y - startPt.y) + v1.y * (startPt.x - p.x)) / d;
		
		double x = startPt.x + s*v.x;
		double y = startPt.y + s*v.y;

		if (s <= t1){ // closest pt is startPt
			ret.t = t1;
			ret.p = atT(t1);
			ret.dist = ret.p.distanceToPt(p);

		}
		else if (s <= t2){ // in between
			ret.t = s;
			ret.p.Set(x,y);
			ret.dist = p.distanceToPt(ret.p);
		}
		else{ // on the other end, i.e. closest to endPt
			ret.t = t2;
			ret.p = atT(t2);
			ret.dist = ret.p.distanceToPt(p);
		}
	}
	else { // here the point is actually on the line


		if(v.x !=0)
			ret.t = (p.x - startPt.x) / v.x;
		else
			ret.t = (p.y - startPt.y) / v.y;     

		if(ret.t < t1){ // the point comes before the starting point
			ret.dist = t1 - ret.t; // since the tangent vector v is of unit length
			ret.t = t1;
			ret.p = startPt;
		}
		else if (ret.t >= t2){ // point on the other side of the endPt
			ret.dist = ret.t - t2;
			ret.t = t2;
			ret.p = endPt;
		}
		else{ // the point is on the segment
			ret.dist = 0;
			// ret.t is the one computed
			ret.p = p; // closest pt to p on seg is p itself !
		}
	}
}

