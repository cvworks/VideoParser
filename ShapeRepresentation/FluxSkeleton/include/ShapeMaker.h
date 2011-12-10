/**************************************************************************

File:                ShapeMaker.h

Author(s):           Pavel Dimitrov

Created:             22 Jun 2002

Last Revision:       $Date: 2002/07/25 20:50:47 $

Description: Implements ShapeMaker. This is here both for testing
purposes and as a sample implementation. It is
actually a bollean array that knows how to create a
SimpleShape. It also provides the containing contour
of the binary image as a closed ContourCurve. 

It is reasonably fast.

$Revision: 1.6 $

$Log: ShapeMaker.h,v $
Revision 1.6  2002/07/25 20:50:47  pdimit
Making release 0.1

Revision 1.5  2002/06/27 14:14:40  pdimit
Just updated the descriptions in the header files.

Revision 1.4  2002/06/26 11:51:35  pdimit
Implemented the DivergenceMap class. It is supposed to be a base
class for other implementations. It has very dumb algorithms, but
which seem to work just fine. Look at testDivergenceMap to see how
to use it. Also, testSimpleShapeMaker has a much nicer interface --
exactly the same as for testDivergenceMap...

Revision 1.3  2002/06/26 07:47:25  pdimit
Just added the class SimpleShape to the mix.
It is precisely that, simple. It consist of a
single header file -- SimpleShape.h.
However, ShapeMaker does not compute the
xmin/xmax and ymin/ymax values yet.

Revision 1.2  2002/06/26 04:30:41  pdimit
After the failed attempt at getting the spline smoothing to work

Revision 1.1.1.1  2002/06/23 05:42:08  pdimit
Initial import



Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
McGill University, Montreal, QC.  Please see the copyright notice
included in this distribution for full details.

**************************************************************************/

#ifndef _SHAPE_MAKER_H_
#define _SHAPE_MAKER_H_

#define DEF_SMOOTH_ITER 5

#include <vector>
#include "defines.h"
#include "ShapeBoundary.h"
#include "DiscretePointCurve.h"
#include "DiscreteSegCurve.h"

namespace sg{

void smooth_closed_curve_pts(std::vector<Point>& pts);

ContourCurve* compute_curve_from_contour(std::vector<Point>& contour_pts, 
								  int nSmoothIter = DEF_SMOOTH_ITER);

class ShapeMaker
{
	struct BBox
	{
		double xmin, xmax, ymin, ymax;
	};

	BBox m_bbox;

	int xSize, ySize;
	bool* arr;

	bool bl;

	int m_nSmoothIterations;

	static int s_curveClassId;

public:

	// constructors/destructors
	ShapeMaker(int xs, int ys, int smoothIter = DEF_SMOOTH_ITER, 
		bool bInit = true) 
	{
		xSize = xs;
		ySize = ys;

		m_nSmoothIterations = smoothIter;

		arr = new bool[xs*ys];

		if (bInit)
			for(int i=0; i< xs * ys; i++)
				arr[i] = false;

		bl = false;
	}

	ShapeMaker(int xs, int ys, bool *array, 
		int smoothIter = DEF_SMOOTH_ITER) 
	{
		xSize = xs;
		ySize = ys;

		m_nSmoothIterations = smoothIter;

		arr = new bool[xs*ys];

		for(int i=0; i<xs*ys; i++)
			arr[i] = array[i];

		bl = false;
	}

	~ShapeMaker() {
		delete[] arr;
	}

	/////////////////////////////////////////////////////////////////////
	// Static function that deal with user arguments and class selection 
	//! Reads user arguments for the class

	static void ReadUserArguments();

	//! @see ReadUserArguments() for id order
	enum CurveType {DISC_PT_CURVE, DISC_SEG_CURVE};

	//! @see ReadUserArguments() for id order
	static ContourCurve* CreateCurve(const std::vector<Point>& pts)
	{
		// ID order: DiscretePointCurve,DiscreteSegCurve
		switch (s_curveClassId)
		{
			case DISC_PT_CURVE: 
				return new DiscretePointCurve(pts);
			case DISC_SEG_CURVE:
				new DiscreteSegCurve(pts);
		}
		
		// Something is wrong if we are here
		return NULL;
	}

	/////////////////////////////////////////////////////////////////////
	// Regular methods
	bool operator()(int x, int y) const
	{
		if (x>=0 && x<xSize && y>=0 && y<ySize)
			return arr[y*xSize + x];
		else
			return false;
	}

	bool& operator()(int x, int y)
	{
		if (x>=0 && x<xSize && y>=0 && y<ySize)
			return arr[y*xSize + x];
		else
			return (bool &)bl;
	}

	static ShapeBoundary* getShape(std::vector<Point>& contour_pts,
		int smoothIter = DEF_SMOOTH_ITER);

	static ContourCurve* getContour(std::vector<Point>& contour_pts, BBox& bbox,
		int smoothIter);

	ShapeBoundary* getShape(bool foreground=false);
	ContourCurve* getContour(bool foreground=false);

	bool withinBounds(int x, int y) {
		return (x>=0 && x < xSize && y>=0 && y < ySize);

	}
};

}

#endif  // _SHAPE_MAKER_H_
