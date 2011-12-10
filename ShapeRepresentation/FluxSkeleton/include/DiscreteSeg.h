/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include "LineSeg.h"

#ifndef _DISCRETE_SEG_H_
#define _DISCRETE_SEG_H_

namespace sg {

class DiscreteSeg : public LineSeg
{
public:
	DiscreteSeg(const Point& sP, const Point& eP, const double& sl = 0)
		: LineSeg(sP, eP, sl)
	{
	}

	virtual void computeDistance(const Point& p, Distance& ret)
	{
		double d0 = p.SqDist(startPt);
		double d1 = p.SqDist(endPt);

		if (d0 <= d1)
		{
			ret.dist = sqrt(d0);
			ret.t = 0;
			ret.p = startPt;
		}
		else
		{
			ret.dist = sqrt(d1);
			ret.t = length;
			ret.p = endPt;
		}
	}

	Point atT(const double& t)
	{
		ASSERT(t == 0 || t == length);

		if (t == 0)
			return startPt;
		else
			return endPt;
	}

	virtual CurveSeg* clone()
	{
      DiscreteSeg *ls = new DiscreteSeg(startPt, endPt, startLength);

      ls->v = v;
      ls->length = length;

      return ls;
    }
};

} // namespace sg 

#endif // _DISCRETE_SEG_H_