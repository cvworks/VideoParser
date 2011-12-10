#ifndef _DIV_PT_H_
#define _DIV_PT_H_

#include "DivergenceMap.h"

namespace sg {

	enum DivPtColor {FOREGROUND_COL, BACKGROUND_COL, SKELETON_COL, 
		SK_POINT_COL, END_POINT_COL, BRANCHING_POINT_COL};

	struct DivPt 
	{
		Point p;      //!< coordinates of the point in the image point
		double val;   //!< divergence value
		double dist;  //!< closest distance to boundary

		char col;     //!< color used for the thinning and DDS graph building

		//int idx;      //!< index in a DivArr
		int x;        //!< x position in a DivArr
		int y;        //!< y position in a DivArr

		bool visited; //!< used for the DDS graph building (branches)

		//! Default constructor
		DivPt() : p(0, 0)
		{
			val  = DIV_MAP_MAX_VAL;
			dist = 0;
			col  = FOREGROUND_COL;
		}

		//! Basic constructor
		DivPt(const Point& pt, const double& div)
		{
			Set(pt, div);
		}

		//! Complete constructor
		DivPt(const Point& pt, const double& divVal, const double& bndryDist, 
			char c, int xIdx, int yIdx, bool vState)
		{ 
			Set(pt, divVal, bndryDist, c);
			SetOptInfo(xIdx, yIdx, vState);
		}

		void Set(const Point& pt, const double& div, 
			const double& d = 0, char c = FOREGROUND_COL)
		{
			p    = pt;
			val  = div;
			dist = d;
			col  = c;			
		}

		void SetOptInfo(int xIdx, int yIdx, bool vState)
		{
			x = xIdx;
			y = yIdx;
			visited = vState;
		}

		bool operator<(const DivPt &dp) const
		{
			return val < dp.val; // since max-heap
		}
	};
}

#endif // _DIV_PT_H_