/**************************************************************************

File:                DivergenceMap.h

Author(s):           Pavel Dimitrov

Created:             26 Jun 2002

Last Revision:       $Date: 2002/06/30 01:22:30 $

Description: Base class. As with the DistanceTransform, this is a
simple implementation of the divergence map
computation. It uses the DT to get the grad(DT) and
approximates the divergence value at any point in the
plane by taking an epsilon(user specified) radius
circle around it and using 8 points on this circle.

It can return the shape this is the DivMap of -- it
returns a clone of it, actually. Similarly for the DT.

$Revision: 1.3 $

$Log: DivergenceMap.h,v $
Revision 1.3  2002/06/30 01:22:30  pdimit
Added DivergenceSkeletonMaker and a test for it. Incomplete functionality
and the testing is mostly in DivergenceSkeletonMaker.cpp. Only the
thinning of the DivergenceMap is implemented.

Revision 1.2  2002/06/27 14:14:40  pdimit
Just updated the descriptions in the header files.

Revision 1.1  2002/06/26 11:51:35  pdimit
Implemented the DivergenceMap class. It is supposed to be a base
class for other implementations. It has very dumb algorithms, but
which seem to work just fine. Look at testDivergenceMap to see how
to use it. Also, testSimpleShapeMaker has a much nicer interface --
exactly the same as for testDivergenceMap...



Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
McGill University, Montreal, QC.  Please see the copyright notice
included in this distribution for full details.

**************************************************************************/

#ifndef DIVERGENCEMAP_H
#define DIVERGENCEMAP_H

#include "ShapeBoundary.h"
#include "DistanceTransform.h"
#include "Point.h"

#ifdef M_PI
#define SG_PI M_PI
#else
#define SG_PI 3.141592653589793
#endif

#define DIV_MAP_DEF_SAMP_RES 8 // min value that seems to yield good results
#define DIV_MAP_MAX_VAL 10000.0

namespace sg{

	/*!
		It does not have virtual functions anymore
	*/
	class DivergenceMap 
	{
		DistanceTransform* dt;

		const int m_nSamplingResolution;
		const double m_dPiFactor;
		std::vector<Point> m_alphas;

	public:

		DivergenceMap(int sampRes = DIV_MAP_DEF_SAMP_RES)
			: m_nSamplingResolution(sampRes), m_dPiFactor(2.0 * SG_PI / sampRes)
		{ 
		
		}

		DivergenceMap(DistanceTransform* pGivenDT, int sampRes = DIV_MAP_DEF_SAMP_RES)
			: m_nSamplingResolution(sampRes), m_dPiFactor(2.0 * SG_PI / sampRes)
		{
			dt = pGivenDT;
		}

		DivergenceMap(const DistanceTransform& givenDT, int sampRes = DIV_MAP_DEF_SAMP_RES)
			: m_nSamplingResolution(sampRes), m_dPiFactor(2.0 * SG_PI / sampRes)
		{
			dt = givenDT.clone();
		}

		void InitAlphas()
		{
			double alpha;

			m_alphas.resize(m_nSamplingResolution);

			for(int i = 0; i < m_nSamplingResolution; i++)
			{
				alpha = i * m_dPiFactor;

				m_alphas[i].x = cos(alpha);
				m_alphas[i].y = sin(alpha);
			}
		}

		~DivergenceMap() 
		{ 
			delete dt;
		}

		/*!
			eps is the half side of the square
		*/
		double getValue(const Point& p, const double& eps = 0.1) const
		{
			// Ugly but it gets the job done!
			if (m_alphas.size() != m_nSamplingResolution)
				((DivergenceMap*)this)->InitAlphas();

			double ret = 0;

			Point pp;
			Vector N, v;

			for(int i = 0; i < m_nSamplingResolution; i++)
			{
				pp.x = m_alphas[i].x;
				pp.y = m_alphas[i].y;

				N.x = pp.x; 
				N.y = pp.y;

				pp.x = p.x + eps * pp.x;
				pp.y = p.y + eps * pp.y;

				v = dt->getGradValue(pp);

				ret += N.dot(v);
			}

			return m_dPiFactor * ret;
		}
		
		ShapeBoundary* releaseShape() 
		{
			return dt->releaseShape();
		}

		ShapeBoundary* getShapeCopy() const 
		{
			return dt->getShapeCopy();
		} 

		void getShapeBounds (double *xmin, double *xmax, 
			double *ymin, double *ymax) const
		{
				dt->getShapeBounds(xmin,xmax, ymin,ymax);
		}

		DistanceTransform* getDTCopy() const{ return dt->clone();} 

		DistanceTransform* getMemberDT() { return dt;} 

		DivergenceMap *clone() const
		{ 
			return new DivergenceMap(*dt, m_nSamplingResolution);
		}
	};

}


#endif  /* DIVERGENCEMAP_H */
