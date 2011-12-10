/************************************************************************

File:		CircSeg.cpp

Author(s):		Pavel Dimitrov

Created:		22 Jun 2002

Last Revision:	$Date: 2002/06/23 05:42:08 $

Description:	

$Revision: 1.1.1.1 $

$Log: CircSeg.cpp,v $
Revision 1.1.1.1  2002/06/23 05:42:08  pdimit
Initial import



Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
McGill University, Montreal, QC.  Please see the copyright notice
included in this distribution for full details.

***********************************************************************/

#define CIRCSEG_CPP

#include "CircSeg.h"

using namespace sg;


// constructors
CircSeg::CircSeg(Point p0, Point p1, Point c, double sign){
	this->p0 = p0;
	this->p1 = p1;
	centre = c;
	this->sign = sign;

	r = p0.distanceToPt(c);

	t0 = point_to_rad(p0);
	t1 = point_to_rad(p1);

	// note: sign should be 1 or -1
	if (t0 < t1){
		if(sign > 0)
			length = r*(t1-t0);
		else
			length = r*(2*PI - (t1-t0)); // the complement of the previous one
	}
	else{ //i.e. when t0 > t1 -- this is the exact opposite of the first case
		if(sign < 0)
			length = r*(-(t1-t0));
		else
			length = r*(2*PI + (t1-t0)); // the complement of the previous one
	}
}


bool CircSeg::rad_in_arc(double t){

	if (t0 < t1){
		if(sign == CCW)
			if (t0<=t && t<=t1)
				return true;
			else
				return false;
		else // if CW
			if (t0<=t && t<=t1)
				return false;
			else
				return true;

	}
	else { // t0 > t1 -- here the exact opposite of the above holds
		if(sign == CW) // <-- note that this is CW as opposed to CCW
			if (t1<=t && t<=t0)
				return true;
			else
				return false;
		else // if CCW
			if (t1<=t && t<=t0)
				return false;
			else
				return true;
	}

	std::cerr << "IT GOT HERE !!!\n";
	return true;
}


// the quadrants are in CCW, i.e.
//     |
//  II | I
// ----+----
// III | IV
//     |
// takes a point on the circle and returns the rad value of it.
double CircSeg::point_to_rad(const Point &p){

	double dx = p.x - centre.x;
	double dy = p.y - centre.y;

	if (dx >=0 && dy >= 0){ // I quadrant
		return atan(dy / dx);
	}

	if (dx < 0 && dy >= 0){ // II quadrant
		return PI - atan( dy / (-dx) );
	}

	if (dx < 0 && dy < 0){ // III quadrant
		return atan( (-dy) / (-dx) ) + PI;
	}

	if (dx >= 0 && dy < 0){ // IV quadrant
		return atan( (dx) / (-dy) ) + 3.0*PI/2.0;
	}

	return 0;
}

// distance to closest pt on arc
// The idea here is to use the equation of the circle
//    C: (x - cx)^2 + (y - cy)^2 = r^2
// and substitute for x and y the parametrized line going
// from the point p to the centre of the circle (cx,cy),
//  L: x(s) = px + s*(cx-px), x(s) = py + s*(cy-py), 
// and
// solve the quadratic equation for s. Naturally, there will 
// always be exactly two (distinct) real solutions s1 and s2. The smaller
// of the two gives the closest point to p on the circle. However,
// this need not be the closest point to p on the arc !

void CircSeg::computeDistance(const Point& p, Distance& d)
{
	Vector v(centre.x - p.x, centre.y - p.y);

	if (v.x == 0 && v.y == 0)
	{
		d.dist = r;
		d.t = 0;
		d.p = p0;

		return;
	}

	double A = SQR(v.x) + SQR(v.y);
	double B = 2*(p.x*v.x - v.y*centre.y + p.y*v.y - v.x*centre.x);
	double C = SQR(centre.x) - 2*p.x*centre.x + SQR(p.x) + SQR(centre.y) \
		- 2*centre.y*p.y + SQR(p.y) - SQR(r);

	// the discriminant should always be >0 since the line goes through
	// the centre of the circle
	double D = sqrt(SQR(B) - 4*A*C); 


	double s1 = (-B + D) / (2*A);
	double s2 = (-B - D) / (2*A);

	// first make s1 < s2;
	double tmp;
	if (s2 < s1) {
		tmp = s2;
		s2  = s1;
		s1  = tmp;
	}

	// try the closer point first -- is it on the arc
	Point pt(p.x + s1*v.x,
		p.y + s1*v.y);

	double st = point_to_rad(pt);

	if(rad_in_arc(st)){
		d.dist = p.distanceToPt(pt);
		d.t = (st - t0)*r / sign;
		d.p = pt;   

		return;
	}

	// now try the other one
	pt.x = p.x + s2*v.x;
	pt.y = p.y + s2*v.y;
	st = point_to_rad(pt);

	if(rad_in_arc(st)){
		d.dist = p.distanceToPt(pt);
		d.t = (st - t0)*r / sign;
		d.p = pt;   

		return;
	}

	// the line does not intersect the arc
	// so we must return either p0 or p1
	double d0 = p0.distanceToPt(p);
	double d1 = p1.distanceToPt(p);

	if(d0 < d1){
		d.dist = d0;
		d.t = 0;
		d.p = p0;
	}
	else {
		d.dist = d1;
		d.t = length;
		d.p = p1;
	}
}
