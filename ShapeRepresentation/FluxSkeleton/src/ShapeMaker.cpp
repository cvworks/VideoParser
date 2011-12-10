/************************************************************************

File:		ShapeMaker.cpp

Author(s):		Pavel Dimitrov

Created:		23 Jun 2002

Last Revision:	$Date: 2002/07/25 20:50:49 $

Description:	

$Revision: 1.8 $

$Log: ShapeMaker.cpp,v $
Revision 1.8  2002/07/25 20:50:49  pdimit
Making release 0.1

Revision 1.7  2002/07/23 21:02:53  pdimit
The branch segmentation of the thinned DivMap is improved --
the two pixel branches are now included. Still, the degenerate case
of the square of junction-points is not taken care of.

Also, a DDSGraph is created which still does not know
of left and right for the branches, i.e. the contour has not been cut.

Revision 1.6  2002/06/26 12:00:53  pdimit
Fixed the zero length segment problem in ShapeMaker

Revision 1.5  2002/06/26 11:51:37  pdimit
Implemented the DivergenceMap class. It is supposed to be a base
class for other implementations. It has very dumb algorithms, but
which seem to work just fine. Look at testDivergenceMap to see how
to use it. Also, testSimpleShapeMaker has a much nicer interface --
exactly the same as for testDivergenceMap...

Revision 1.4  2002/06/26 07:47:27  pdimit
Just added the class SimpleShape to the mix.
It is precisely that, simple. It consist of a
single header file -- SimpleShape.h.
However, ShapeMaker does not compute the
xmin/xmax and ymin/ymax values yet.

Revision 1.3  2002/06/26 06:34:45  pdimit
After adding a more successful smoothing.
A point becomes the average of its previou nbr, next nbr and
itself. The first and end points (which are the same) do not change.
This smoothing primitive is applied 5 times to get a nicer result.

Revision 1.2  2002/06/26 04:30:44  pdimit
After the failed attempt at getting the spline smoothing to work

Revision 1.1  2002/06/24 09:04:41  pdimit
First commit of this. The contour tracing (Pavlidis) seems to be
working.

Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
McGill University, Montreal, QC.  Please see the copyright notice
included in this distribution for full details.
***********************************************************************/
#define SIMPLESHAPEMAKER_CPP

#include "ShapeMaker.h"
#include "DiscreteSegCurve.h"
#include "LineSeg.h"
#include "CircSeg.h"
#include "Vector.h"

#include "SplineSmooth.h"

#include <Tools/BasicUtils.h>
#include <Tools/UserArguments.h>

#define NORTH 0
#define SOUTH 1
#define EAST  2
#define WEST  3

extern vpl::UserArguments g_userArgs;

using namespace sg;

int ShapeMaker::s_curveClassId = 0;

void spit_it(std::vector<Point> ctr, int xs, int ys)
{
	std::cout << "P3\n" << xs << " " << ys << "\n256\n";

	bool *out = new bool[xs*ys];
	int i;

	for (i=0; i<xs*ys; i++)
		out[i] = false;

	std::vector<Point>::iterator I;

	for(I = ctr.begin(); I != ctr.end(); I++){
		Point p = *I;

		out[(int)p.y*xs + (int)p.x] = true;
	}

	for (i=0; i<xs*ys; i++){
		if (out[i])
			std::cout << " 200 0 0 ";
		else
			std::cout << " 0 0 200 ";
	}

	delete[] out;
}

/* static */
void ShapeMaker::ReadUserArguments()
{
	g_userArgs.ReadArg("FluxSkeleton", "boundaryCurve", 
		Tokenize("DiscretePointCurve,DiscreteSegCurve"),
		"Type of curve used to represent a shape boundary", 0, &s_curveClassId);

	g_userArgs.ReadArg("FluxSkeleton", "boundaryCurveSegment", 
		Tokenize("DiscreteSeg,LineSeg,CircSeg"),
		"Type of segments for a discrete boundary curve", 0, 
		&DiscreteSegCurve::s_curveSegmentClassId);
}

/*!
	Serializes the ShapeBoundary member variables except 
	the m_skeletalInfo, which must be recomputed by calling 
	setSkeletalPoints once the DDSGraph is deserialized.
*/
void ShapeBoundary::Serialize(OutputStream& os) const
{
	::Serialize(os, m_xmin);
	::Serialize(os, m_xmax);
	::Serialize(os, m_ymin);
	::Serialize(os, m_ymax);

	::Serialize(os, m_contourCurves.size());

	if (!m_contourCurves.empty())
	{
		::Serialize(os, vpl_TypeName(*m_contourCurves.front()));

		std_forall(it, m_contourCurves)
			(*it)->Serialize(os);
	}

	// The m_skeletalInfo is not serializied. It will have
	// to be recomputed by calling setSkeletalPoints.
}

/*!
	Deserializes the ShapeBoundary member variables except 
	the m_skeletalInfo, which must be recomputed by calling 
	setSkeletalPoints once the DDSGraph is deserialized.
*/
void ShapeBoundary::Deserialize(InputStream& is)
{
	::Deserialize(is, m_xmin);
	::Deserialize(is, m_xmax);
	::Deserialize(is, m_ymin);
	::Deserialize(is, m_ymax);

	unsigned numCurves;

	::Deserialize(is, numCurves);

	m_contourCurves.resize(numCurves);

	if (numCurves > 0)
	{
		std::string type;

		::Deserialize(is, type);

		unsigned typeId;
		
		if (type == vpl_TypeName(DiscretePointCurve))
			typeId = 0;
		else if (type == vpl_TypeName(DiscreteSegCurve))
			typeId = 1;
		else
			ASSERT(false);

		ContourCurve* curve = NULL;

		for (unsigned i = 0; i < numCurves; ++i)
		{
			switch (typeId)
			{
				case 0: curve = new DiscretePointCurve; break;
				case 1: curve = new DiscreteSegCurve; break;
			}

			curve->Deserialize(is);

			m_contourCurves[i] = curve;
		}
	}

	// The m_skeletalInfo is not deserializied. It must be 
	// recomputed by calling setSkeletalPoints.
}

/*!
	The shape is 0 and background is 1 (or false and true resp.)
	A point P has the following 8-contour
		1 2 3
		8 P 4
		7 6 5

	This also computes xmin/xmax and ymin/ymax.
*/
ContourCurve *ShapeMaker::getContour(bool foreground)
{
	int dir = NORTH; // current trace direction

	// these are the offsets for P1,P2,P3 for the four directions
	Vector front_pt[][3] = {
		{Vector(-1,1), Vector(0,1), Vector(1,1)},    // north
		{Vector(1,-1), Vector(0,-1), Vector(-1,-1)}, // south
		{Vector(1,1), Vector(1,0), Vector(1,-1)},    // east
		{Vector(-1,-1), Vector(-1,0), Vector(-1,1)}  // west
	};

	int dir_change [][3] = {
		{WEST, NORTH, NORTH},   // current is NORTH
		{EAST, SOUTH, SOUTH},   // current is SOUTH
		{NORTH, EAST, EAST},  // current is EAST
		{SOUTH, WEST, WEST},  // current is WEST
	};

	// rotating in the clockwise dir
	int dir_rot [] = { 
		EAST,   // current is NORTH
		WEST,   // current is SOUTH
		SOUTH,  // current is EAST
		NORTH  // current is WEST
	};

	// offsets for the 8-neighbours of point P
	//    Vector nbrs[] = {Vector(-1,1), Vector(0,1), Vector(1,1),   // 1 2 3
	//  		   Vector(1,0), Vector(1,-1), Vector(0,-1),  // 4 5 6
	//  		   Vector(-1,-1), Vector(-1,0)               // 7 8
	//    };

	std::vector<Point> contour_pts;

	Point p;  // current pixel
	Point sp; // starting pixel

	// start by finding a suitable first point
	int i = 0;
	while (arr[i] != foreground && i < xSize*ySize)
		i++;

	// there may not be a single foreground pixel !
	if(arr[i] == foreground){
		int x,y;

		y = i / xSize;
		x = i % xSize;

		p = Point((double)x, (double)y);
		contour_pts.push_back(p); // insert the first point into the list
		sp = p; // the current and start point are the same ...

		// now determine a good starting direction
		dir = NORTH;
		if(x!=0){
			if(operator()(x-1,y) != foreground)
				dir = NORTH;
		}
		else if(x<xSize-1){
			if(operator()(x+1,y) != foreground)
				dir = SOUTH;
		}
		else if(y>1){
			if(operator()(x,y-1) != foreground)
				dir = WEST;
		} 
		else if(y<ySize-1)
			if(operator()(x,y+1) != foreground)
				dir = EAST;

		// now find the next pixel

		Point np;
		bool found_pix = false;

		// try rotating
		for(int j=0; j < 4 && (found_pix==false); j++)
		{
			if(j!=0)
				dir = dir_rot[dir];

			// try the three pixels in front (according to dir)
			for (int i=0; i<3 && found_pix == false; i++)
			{
				np.x = p.x+front_pt[dir][i].x; // set the new pt to P(i+1)
				np.y = p.y+front_pt[dir][i].y; 

				if(withinBounds((int)np.x, (int)np.y))
				{
					if(operator()((int)np.x, (int)np.y) == foreground)
					{
						found_pix = true;

						p = np;
						contour_pts.push_back(p);
						// update the new direction
						dir = dir_change[dir][i];
					}
				}
			} // end of P1,P2,P3 tries
		} // end of 90deg rotations

		while (p.x != sp.x || p.y != sp.y)
		{
			found_pix = false;

			// try rotating
			for(int j=0; j < 4 && (found_pix==false); j++)
			{
				if(j!=0)
					dir = dir_rot[dir];

				// try the three pixels in front (according to dir)
				for (int i=0; i<3 && (found_pix == false); i++)
				{
					np.x = p.x+front_pt[dir][i].x; // set the new pt to P(i+1)
					np.y = p.y+front_pt[dir][i].y; 

					if(withinBounds((int)np.x, (int)np.y))
					{
						if(operator()((int)np.x, (int)np.y) == foreground)
						{
							found_pix = true;

							p = np;
							contour_pts.push_back(p);
							// update the new direction
							dir = dir_change[dir][i];

						}
					}
				} // end of P1,P2,P3 tries
			} // end of 90deg rotations
		} // end of while loop

		return getContour(contour_pts, m_bbox, m_nSmoothIterations);
	}
	else
	{
		return NULL;
	}
}

/*!
	It smoothes the given countour points and then creates 
	a DiscreteSegCurve that representes the contour. It also
	computes its bounding box after smoothing.
*/
/* static */
ContourCurve* ShapeMaker::getContour(std::vector<Point>& contour_pts, BBox& bb,
									int smoothIter)
{
	// Smooth out the contour
	for (int k = 0; k < smoothIter; k++)
		smooth_closed_curve_pts(contour_pts);

	// Compute xmin/xmax ymin/ymax
	std::vector<Point>::iterator I = contour_pts.begin();

	// Init the bounding box
	bb.xmin = bb.xmax = I->x; 
	bb.ymin = bb.ymax = I->y; 

	for(++I; I != contour_pts.end(); ++I)
	{
		if(I->x < bb.xmin) bb.xmin = I->x;
		if(I->x > bb.xmax) bb.xmax = I->x;
		if(I->y < bb.ymin) bb.ymin = I->y;
		if(I->y > bb.ymax) bb.ymax = I->y;
	}

	// Get some object of ContourCurve-derived class
	return CreateCurve(contour_pts);
}

ShapeBoundary* ShapeMaker::getShape(bool foreground)
{
	ContourCurve* pContour = getContour(foreground);

	ShapeBoundary* pShape = new ShapeBoundary(pContour); // pShape owns countour

	pShape->setBounds(m_bbox.xmin, m_bbox.xmax, m_bbox.ymin, m_bbox.ymax);

	return pShape;
}

/* static */
ShapeBoundary* ShapeMaker::getShape(std::vector<Point>& contour_pts, int smoothIter)
{
	BBox bb;

	ContourCurve* pContour = getContour(contour_pts, bb, smoothIter);

	ShapeBoundary* pShape = new ShapeBoundary(pContour); // pShape owns countour

	pShape->setBounds(bb.xmin, bb.xmax, bb.ymin, bb.ymax);

	return pShape;
}

// Here we average the points. Diego: Bug fixed. it was "eating" one point.
void sg::smooth_closed_curve_pts(std::vector<Point>& pts)
{
	std::vector<Point> tpts;
	std::vector<Point>::iterator it0;
	std::vector<Point>::iterator itN;

	tpts.reserve(pts.size()); //Diego: makes it faster

	// Everything is easier if the first and last point are the same, so make sure they are
	if (pts.front() != pts.back())
		pts.push_back(pts.front());

	// Read the first point and its right neighbour
	it0 = pts.begin();
	Point pt0 = *it0; it0++;
	Point nextPt = *it0;

	// Read the left neighbour of the first point, i.e., the "true" last point
	itN = pts.end(); itN--; itN--;
	Point prevPt = *itN;

	// Compute the avg centered at the first point -- which is a special case
	pt0.x = (prevPt.x + pt0.x + nextPt.x) / 3.0;
	pt0.y = (prevPt.y + pt0.y + nextPt.y) / 3.0;

	// Save the new smoothed point
	tpts.push_back(pt0);

	// Now do all the other points, until (but including) the "true" last point
	std::vector<Point>::iterator left, right;
	Point currPt;

	itN++; // itN is now the repeated version of the first point

	for(; it0 != itN; it0++)
	{
		left  = it0; left--;
		right = it0; right++;

		prevPt = *left;
		nextPt = *right;
		currPt = *it0;

		// Average the current point
		currPt.x = (prevPt.x + currPt.x + nextPt.x) / 3.0;
		currPt.y = (prevPt.y + currPt.y + nextPt.y) / 3.0;

		tpts.push_back(currPt);
	}

	// Finally add the last point -- which is the same as the first
	tpts.push_back(pt0);

	pts = tpts;
}

ContourCurve* sg::compute_curve_from_contour(std::vector<Point>& contour_pts, int nSmoothIter /*=5*/)
{
	if (contour_pts.empty())
		return NULL;

	// smooth contour
	for (int j = 0; j < nSmoothIter; j++)
		smooth_closed_curve_pts(contour_pts);

	return ShapeMaker::CreateCurve(contour_pts);

	// Finally, make the shape
	/*std::vector<Point>::iterator I = contour_pts.begin();

	Point pp = *I; 

	I++;

	Point fp = pp; // the first point

	std::vector<CurveSeg*> segs;

	for(int i = 0; I != contour_pts.end(); I++, i++)
	{
		segs.push_back(ShapeMaker::CreateCurveSegment(pp, *I));

		pp = *I;
	}

	if(pp.x != fp.x || pp.y != fp.y)
	{
		segs.push_back(ShapeMaker::CreateCurveSegment(pp, fp));
	}

	if (segs.empty())
		return NULL;

	return ShapeMaker::CreateCurve(segs, true);*/
}

