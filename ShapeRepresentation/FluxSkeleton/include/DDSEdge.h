/**************************************************************************

   File:                DDSGraph.h

   Author(s):           Pavel Dimitrov

   Created:             22 Jul 2002

   Last Revision:       $Date: 2002/07/25 20:50:47 $

   Description: 

   $Revision: 1.2 $

   $Log: DDSGraph.h,v $
   Revision 1.2  2002/07/25 20:50:47  pdimit
   Making release 0.1

   Revision 1.1  2002/07/23 21:02:51  pdimit
   The branch segmentation of the thinned DivMap is improved --
   the two pixel branches are now included. Still, the degenerate case
   of the square of junction-points is not taken care of.

   Also, a DDSGraph is created which still does not know
   of left and right for the branches, i.e. the contour has not been cut.

   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 **************************************************************************/

#ifndef DISCRETE_DIVERGENCE_SKELETON_EDGE_H
#define DISCRETE_DIVERGENCE_SKELETON_EDGE_H

#include <iostream>
#include "ContourCurve.h"
#include "FluxPoint.h"
#include "BoundaryInfo.h"
#include "BoundarySegmentArray.h"
#include "DDSNode.h"

namespace sg {

//typedef std::vector<FluxPoint> FluxPointArray;
//typedef std::vector<BoundaryInfo> BoundaryInfoArray;
//typedef std::vector<BoundarySegment> BoundarySegments;

/*!
	Edge of a DDSGraph graph 
*/
class DDSEdge
{
public: //should be protected but AFMMSkeleton.h must be corrected first
	DDSGraph* dds;   //!< Parent DDSGraph
	DDSNode* n1;     //!< branch terminal node 1
	DDSNode* n2;     //!< branch terminal node 2

	FluxPointArray flux_points;             //!< Branch skeletal points
	BoundaryInfoArray boundary_info_array;  //!< Boundary info associated with each flux point
	BoundarySegmentArray boundary_segments; //!< Array of boundary gaps. Empty if no gaps.

	unsigned int edgeIndex; //!< Edge's index on the node vector of the containter graph

public:
	/*!
		This value is used in the AFMMSkeleton to know the reconstruction error of
		a given edge during the simplification step. It must be initialized to -1.
	*/
	double m_dEgdeRecError;

protected:
	void InitNodeInfo(DDSGraph* pDDS, 
		DDSNode* pN1,
		DDSNode* pN2)
	{
		dds = pDDS;

		n1 = pN1; 
		n1->addEdge(this);

		n2 = pN2; 
		n2->addEdge(this);

		m_dEgdeRecError = -1;
	}

	void InitPoints(const FluxPointArray& pts)
	{
		flux_points = pts;
	}

public:
	DDSEdge(DDSGraph* pDDS,
		DDSNode* pN1,
		DDSNode* pN2) 
	{
		InitNodeInfo(pDDS, pN1, pN2);
	}

	DDSEdge(DDSGraph* pDDS,
		const FluxPointArray& pts,
		DDSNode* pN1,
		DDSNode* pN2) 
	{
		InitNodeInfo(pDDS, pN1, pN2);
		InitPoints(pts);
	}

	//! Used by vpl::GSG only
	/*DDSEdge(void* pData)
	{
		dds = NULL;
		n1  = NULL;
		n2  = NULL;
		pEdgeData = pData;
	}*/

	void setSkeleton(DDSGraph* sk){ dds = sk; }

	const FluxPointArray& getFluxPoints() const { return flux_points; }
	FluxPointArray& getFluxPoints() { return flux_points; }

	DDSNode* getN1() { return n1; }
	DDSNode* getN2() { return n2; }
	DDSGraph* getSkeleton() { return dds; }

	const DDSNode* getN1() const { return n1; }
	const DDSNode* getN2() const { return n2; }
	const DDSGraph* getSkeleton() const { return dds; }

	//void computeBoundaryInfoArray();
	const BoundaryInfoArray& getBoundaryInfoArray() const { return boundary_info_array; }
	BoundaryInfoArray& getBoundaryInfoArray() { return boundary_info_array; }
	void setBoundaryInfoArray(const BoundaryInfoArray& bia) { boundary_info_array = bia; }

	const BoundarySegmentArray& getBoundarySegments() const { return boundary_segments; }
	BoundarySegmentArray& getBoundarySegments() { return boundary_segments; }

	void computeValueApproximation(FluxPointArray& fpl, BoundaryInfoArray& bil);
	void computeTangents(FluxPointArray& fpl, BoundaryInfoArray& bil);

	unsigned int size() const { return flux_points.size(); }

	//! Inits boundary infor array. Only first and last points are "cleared"
	void initBoundaryInfoArray()
	{
		boundary_info_array.resize(flux_points.size());

		// Make sure that at least the first and last points are cleared
		boundary_info_array.front().clear();
		boundary_info_array.back().clear();
	}

	// Handy functions to retrieve flux points or a piece of their information
	const FluxPoint& fluxPoint(unsigned int i) const { return flux_points[i]; }

	const FluxPoint& firstFluxPoint() const { return flux_points.front(); }
	const FluxPoint& lastFluxPoint() const { return flux_points.back(); }

	const Point& firstXYPoint() const { return flux_points.front().p; }
	const Point& lastXYPoint() const { return flux_points.back().p; }

	double radius(unsigned int i) const { return flux_points[i].radius(); }

	const BoundaryInfo& boundaryInfo(unsigned int i) const { return boundary_info_array[i]; }

	const BoundaryInfo& firstBndryInfo() const { return boundary_info_array.front(); }
	const BoundaryInfo& lastBndryInfo() const { return boundary_info_array.back(); }

	int bndryPtIndex(unsigned int i, char side) const
	{
		return boundary_info_array[i][side].index;
	}

	/*!
		Distance between the endpoints of the spokes emanating from the i'th 
		branch point. ie, the distance between the two boundary points 
		associated with the i'th branch point.
	*/
	double spokeEndpointDistance(unsigned int i) const 
	{ 
		const BoundaryInfo& bi = boundaryInfo(i);

		return bi.first.pt.dist(bi.second.pt); 
	}

	//! Checks if its a terminal branch. ie, least one endpoint node has degree 1
	bool isTerminal() const
	{
		return (n1->degree() == 1 || n2->degree() == 1);
	}
};

} //namespace sg

#endif // DISCRETE_DIVERGENCE_SKELETON_EDGE_H
